//
//  Poller.m
//  MacDerbyReplay
//
//  Created by Jeff Piazza on 7/13/14.
//  Copyright (c) 2014 Jeff Piazza. All rights reserved.
//

#import "Poller.h"
#import "CommandListener.h"

@implementation Poller

@synthesize listener;

- (id) initWithDelegate:(AppDelegate *)theAppDelegate andListener:(CommandListener *)theListener
{
    if ((self = [super init]))
    {
        _appDelegate = theAppDelegate;
        listener = theListener;
    }
    return self;
}

- (void) setUrlAndPoll: (NSString*) urlString
{
    if (![urlString hasPrefix:@"http:"]) {
        urlString = [@"http://" stringByAppendingString: urlString];
    }

    if (![urlString hasSuffix:@"/action.php"]) {
        if (![urlString hasSuffix: @"/"]) {
            urlString = [urlString stringByAppendingString: @"/"];
        }
        urlString = [urlString stringByAppendingString: @"action.php"];
    }
    url = [NSURL URLWithString:urlString];
    [self poll];
}

- (void) poll
{
    //  action=register-replay&port=<port>
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    
    [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
    
    [request setHTTPMethod:@"POST"];

    NSString* bodyData = [NSString stringWithFormat:@"action=register-replay"];  // TODO
    [request setHTTPBody:[NSData dataWithBytes:[bodyData UTF8String] length:strlen([bodyData UTF8String])]];
    
    
    // The connection delegate could sensibly be another object
    responseData = [NSMutableData data];
    connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
}

// These four are for connection delegate behavior
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    [responseData setLength:0];
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    [responseData appendData:data];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    [[NSAlert alertWithError:error] runModal];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
    // Once this method is invoked, "responseData" contains the complete result
    NSError *error;
    NSXMLDocument *document =
    [[NSXMLDocument alloc] initWithData:responseData options:NSXMLDocumentTidyHTML error:&error];
    
    // Deliberately ignoring the error variable: with most HTML it will be filled with
    // numerous "tidy" warnings.
    
    if (document) {
        NSArray* elements = [[document rootElement] elementsForName:@"replay-message"];
        // (NSArray *)nodesForXPath:(NSString *)xpath error:(NSError **)error;
        for (NSXMLElement* elt in elements) {
            NSString* msg = [elt stringValue];
            if ([listener interpret_command: msg]) {
            } else {
                NSString *strData = [[NSString alloc]initWithData:responseData encoding:NSUTF8StringEncoding];
                NSLog(@"  PARSING FAILS: %@", strData);
            }
        }
    } else {
        NSString *strData = [[NSString alloc]initWithData:responseData encoding:NSUTF8StringEncoding];
        NSLog(@"Poller response document failed to parse: %@", strData);
        // [[NSAlert alertWithMessageText:@"Document failed to parse" defaultButton:@"Dismiss" alternateButton:nil otherButton:nil informativeTextWithFormat:nil] runModal];
    }
    
    // Delay execution of my block for 500 msec (1/2 second)
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 500 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
        [self poll];
    });
}

@end
