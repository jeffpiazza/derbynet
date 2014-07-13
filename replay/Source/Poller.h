//
//  Poller.h
//  MacDerbyReplay
//
//  Created by Jeff Piazza on 7/13/14.
//  Copyright (c) 2014 Jeff Piazza. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Poller : NSObject
{
    // cf _urlSheet, _urlField that are expressed as properties
    NSURL* url;
    NSMutableData* responseData;
    NSURLConnection* connection;
}

- (void) setUrlAndPoll: (NSString*) urlString;

@end
