//
//  Poller.m
//  MacDerbyReplay
//
//  Created by Jeff Piazza on 7/13/14.
//  Copyright (c) 2014 Jeff Piazza. All rights reserved.
//

#import "Poller.h"

@implementation Poller

- (void) setUrlAndPoll: (NSString*) urlString
{
    NSLog(@"setUrlAndPoll %@", urlString);

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

- (void) setPort: (int) port
{
    _port = port;
}


- (void) poll
{
    //  action=register-replay&port=<port>
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    
    [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
    
    [request setHTTPMethod:@"POST"];

    NSString* bodyData = [NSString stringWithFormat:@"action=register-replay&port=%i", _port];
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
    // NSString *strData = [[NSString alloc]initWithData:responseData encoding:NSUTF8StringEncoding];
    // NSLog(@"%@",strData);
    
    // Once this method is invoked, "responseData" contains the complete result
    NSError *error;
    NSXMLDocument *document =
    [[NSXMLDocument alloc] initWithData:responseData options:NSXMLDocumentTidyHTML error:&error];
    
    // Deliberately ignore the error: with most HTML it will be filled with
    // numerous "tidy" warnings.
    
    if (!document) {
        NSLog(@"Document failed to parse");
        // [[NSAlert alertWithMessageText:@"Document failed to parse" defaultButton:@"Dismiss" alternateButton:nil otherButton:nil informativeTextWithFormat:nil] runModal];
    } else {
        NSLog(@"PARSED OK!");
        // [[NSAlert alertWithMessageText:@"PARSED OK!" defaultButton:@"Dismiss" alternateButton:nil otherButton:nil informativeTextWithFormat:nil] runModal];
        NSString *strData = [[NSString alloc]initWithData:responseData encoding:NSUTF8StringEncoding];
        NSLog(@"%@",strData);
    }
    
    // Delay execution of my block for 10 seconds.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"dispatch_after");
        [self poll];
    });
}

@end
