//
//  Poller.h
//  MacDerbyReplay
//
//  Created by Jeff Piazza on 7/13/14.
//  Copyright (c) 2014 Jeff Piazza. All rights reserved.
//
// Poller takes a URL to the server, and proceeds to poll the server for new commands.
// Polling at least has the advantage that it works through firewalls.

#import <Foundation/Foundation.h>
#import "AppDelegate.h"

@class CommandListener;

@interface Poller : NSObject
{
    // cf _urlSheet, _urlField that are expressed as properties
    NSURL* url;
    NSMutableData* responseData;
    NSURLConnection* connection;
}

@property (weak) AppDelegate* appDelegate;
@property (weak) CommandListener* listener;

- (id)initWithDelegate: (AppDelegate*) theAppDelegate andListener: (CommandListener*) theListener;
- (void) setUrlAndPoll: (NSString*) urlString;

@end
