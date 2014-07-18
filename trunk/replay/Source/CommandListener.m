//
//  CommandListener.m
//  MacDerbyReplay
//
//  Created by Jeff Piazza on 7/17/14.
//  Copyright (c) 2014 Jeff Piazza. All rights reserved.
//

#import "CommandListener.h"
#import "GCDAsyncSocket.h"

// TODO: These constants are from EchoServer
#define WELCOME_MSG  0
#define ECHO_MSG     1
#define WARNING_MSG  2

#define READ_TIMEOUT 90.0


@implementation CommandListener

@synthesize appDelegate;

- (id)initWithDelegate: (AppDelegate*) theAppDelegate;
{
	if ((self = [super init]))
	{
        appDelegate = theAppDelegate;
        
		// Setup our socket.
		// The socket will invoke our delegate methods using the usual delegate paradigm.
		// However, it will invoke the delegate methods on a specified GCD delegate dispatch queue.
		//
		// Now we can configure the delegate dispatch queues however we want.
		// We could simply use the main dispatc queue, so the delegate methods are invoked on the main thread.
		// Or we could use a dedicated dispatch queue, which could be helpful if we were doing a lot of processing.
		//
		// The best approach for your application will depend upon convenience, requirements and performance.
		
		socketQueue = dispatch_queue_create("socketQueue", NULL);
		
		listenSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:socketQueue];
		
		// Setup an array to store all accepted client connections
		connectedSockets = [[NSMutableArray alloc] initWithCapacity:1];
		
		isRunning = NO;
	}
	return self;
}

- (void)startOnPort: (int) port
{
    NSLog(@"Starting command listener on port %i", port);
    if (port < 0 || port > 65535)
    {
        port = 0;
    }
    
    NSError *error = nil;
    if (![listenSocket acceptOnPort:port error:&error])
    {
        NSLog(@"Error starting server: %@", error);
        return;
    }
    
    // TODO: Need to tell someone what port we started on!
    // [self logInfo:FORMAT(@"Echo server started on port %hu", [listenSocket localPort])];
    NSLog(@"Server started on port %hu", [listenSocket localPort]);
    [[self appDelegate] setPortMessage:[NSString stringWithFormat:@"Listening on port %hu", [listenSocket localPort]]];
    [[self appDelegate] setStatus:@"Disconnected/Listening"];
    isRunning = YES;
}

- (void) stop
{
    // Stop accepting connections
    [listenSocket disconnect];
    
    // Stop any client connections
    @synchronized(connectedSockets)
    {
        NSUInteger i;
        for (i = 0; i < [connectedSockets count]; i++)
        {
            // Call disconnect on the socket,
            // which will invoke the socketDidDisconnect: method,
            // which will remove the socket from the list.
            [[connectedSockets objectAtIndex:i] disconnect];
        }
    }
    
    NSLog(@"Stopped Echo server");
    isRunning = false;
}


- (void)socket:(GCDAsyncSocket *)sock didAcceptNewSocket:(GCDAsyncSocket *)newSocket
{
	// This method is executed on the socketQueue (not the main thread)
	
	@synchronized(connectedSockets)
	{
		[connectedSockets addObject:newSocket];
	}
	
	NSString *host = [newSocket connectedHost];
	UInt16 port = [newSocket connectedPort];
	
	dispatch_async(dispatch_get_main_queue(), ^{
		@autoreleasepool {
            // TODO: UI feedback for what's going on
			// [self logInfo:FORMAT(@"Accepted client %@:%hu", host, port)];
            NSLog(@"Accepted client %@:%hu", host, port);
            [[self appDelegate] setStatus:[NSString stringWithFormat:@"Connected to %@:%hu", host, port]];
		}
	});
	
    // TODO?
    if (false) {
        NSString *welcomeMsg = @"Welcome to the AsyncSocket Echo Server\r\n";
        NSData *welcomeData = [welcomeMsg dataUsingEncoding:NSUTF8StringEncoding];
	
        [newSocket writeData:welcomeData withTimeout:-1 tag:WELCOME_MSG];
	}

	[newSocket readDataToData:[GCDAsyncSocket CRLFData] withTimeout:READ_TIMEOUT tag:0];
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
	// This method is executed on the socketQueue (not the main thread)
	
	if (tag == ECHO_MSG)
	{
		[sock readDataToData:[GCDAsyncSocket CRLFData] withTimeout:READ_TIMEOUT tag:0];
	}
}

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag
{
	// This method is executed on the socketQueue (not the main thread)
	
	dispatch_async(dispatch_get_main_queue(), ^{
		@autoreleasepool {
            
			NSData *strData = [data subdataWithRange:NSMakeRange(0, [data length] - 2)];
			NSString *msg = [[NSString alloc] initWithData:strData encoding:NSUTF8StringEncoding];
			if (msg)
			{
                dispatch_async(dispatch_get_main_queue(), ^{
                    @autoreleasepool {
                        // TODO: Respond to the message, duh!
                        NSLog(@"Read %lu characters: [[%@]]", (unsigned long)[msg length], msg);
                        [[self appDelegate] replayRecording];
                    }
                });
			}
			else
			{
                NSLog(@"Error converting received data into UTF-8 String");
			}
            
		}
	});
	
	// Echo message back to client
	[sock writeData:[@"OK\r\n" dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:ECHO_MSG];
}

// TODO: What if this just isn't provided?
- (NSTimeInterval)socket:(GCDAsyncSocket *)sock shouldTimeoutReadWithTag:(long)tag
                 elapsed:(NSTimeInterval)elapsed
               bytesDone:(NSUInteger)length
{
	return 0.0;
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
	if (sock != listenSocket)
	{
		dispatch_async(dispatch_get_main_queue(), ^{
			@autoreleasepool {
                [[self appDelegate] setStatus:@"Disconnected/Listening"];
				NSLog(@"Client Disconnected");
			}
		});
		
		@synchronized(connectedSockets)
		{
			[connectedSockets removeObject:sock];
		}
	}
}
@end
