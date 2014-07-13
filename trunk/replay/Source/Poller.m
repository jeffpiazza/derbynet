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
    // TODO: fix up url, adding http:// on the front and /action.php?query=replay-poll on the end.  Maybe remove /index.php from the end, if present.
    url = [NSURL URLWithString:urlString];
    
    [self poll];
}

- (void) poll
{
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    
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
    
    /*
     NSXMLElement *rootNode = [document rootElement];
     
     NSString *xpathQueryString =
     @"//div[@id='newtothestore']/div[@class='modulecontent']/div[@id='new-to-store']/div[@class='list_content']/ul/li/a";
     NSArray *newItemsNodes = [rootNode nodesForXPath:xpathQueryString error:&error];
     if (error)
     {
     [[NSAlert alertWithError:error] runModal];
     return;
     }
     */
    if (!document) {
        NSLog(@"Document failed to parse");
        // [[NSAlert alertWithMessageText:@"Document failed to parse" defaultButton:@"Dismiss" alternateButton:nil otherButton:nil informativeTextWithFormat:nil] runModal];
    } else {
        NSLog(@"PARSED OK!");
        // [[NSAlert alertWithMessageText:@"PARSED OK!" defaultButton:@"Dismiss" alternateButton:nil otherButton:nil informativeTextWithFormat:nil] runModal];
    }
    
    // Delay execution of my block for 10 seconds.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"dispatch_after");
        [self poll];
    });
}

@end
