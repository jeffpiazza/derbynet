//
//  CommandListener.m
//  MacDerbyReplay
//
//  Created by Jeff Piazza on 7/17/14.
//  Copyright (c) 2014 Jeff Piazza. All rights reserved.
//

#import "CommandListener.h"

@implementation CommandListener

@synthesize appDelegate;

- (id)initWithDelegate: (AppDelegate*) theAppDelegate;
{
	if ((self = [super init])) {
        appDelegate = theAppDelegate;
	}
	return self;
}

- (NSString*) usage
{
    return @"Available commands:\r\n"
    "HELP  -- this message\r\n"
    "TEST skipback showings rate  -- playback test video\r\n"
    "DEMO skipback showings rate  -- playback test video\r\n"
    "START video_name_root  -- start recording video\r\n"
    "CANCEL  -- cancel recording\r\n"
    "REPLAY skipback showings rate  -- stop recording if recording; playback\r\n"
    "Please send a command:\r\n";
}

- (BOOL) interpret_command: (NSString*) msg
{
    NSLog(@"interpret_command: [[%@]]", msg);  // TODO
    @autoreleasepool {
        NSScanner* scanner = [NSScanner scannerWithString: msg];
        NSString* cmd;
        __block BOOL succeeded = (msg != NULL);
        if (succeeded) {
            succeeded = [scanner scanCharactersFromSet:[NSCharacterSet alphanumericCharacterSet]
                                            intoString:&cmd];
        }
        if (succeeded) {
            NSString* selector =
                [@"docommand_" stringByAppendingString:[[cmd lowercaseString] stringByAppendingString:@":"]];
            @try {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
                SEL sel = NSSelectorFromString(selector);
                id result = [self performSelector:sel withObject:scanner];
                succeeded = [result boolValue];
#pragma clang diagnostic pop
            }
            @catch (NSException * e) {
                succeeded = NO;
                NSLog(@"Exception: %@", e);
            }
        }
        
        return succeeded;
    }
}

- (NSNumber*) docommand_hello: (NSScanner*) scanner
{
    [[self appDelegate] setStatusMessage:@"Poller connected"];
    return [NSNumber numberWithBool: YES];
}

// TEST <skipback_seconds> <repeat> <rate>
// Plays the "film leader" clip built in to the app
- (NSNumber*) docommand_test: (NSScanner*) scanner
{
    int num_secs;
    if (![scanner scanInt: &num_secs]) return [NSNumber numberWithBool: NO];
    int num_times;
    if (![scanner scanInt: &num_times]) return [NSNumber numberWithBool: NO];
    float rate;
    if (![scanner scanFloat:&rate]) return [NSNumber numberWithBool: NO];
    
    NSLog(@"docommand_test num_secs=%i, num_times=%i, rate=%f", num_secs, num_times, rate);  // TODO
    
    // doPlaybackOf opens the asset file and does the rest asynchronously
    [[self appDelegate] doPlaybackOf: [[NSBundle mainBundle] URLForResource:@"FilmLeader" withExtension:@"mp4"]
                            skipback: num_secs duration: num_secs showings: num_times rate: rate];
    return [NSNumber numberWithBool: YES];
}

// DEMO <skipback_seconds> <repeat> <rate>
// Plays DEMO.mov from the current recording folder.  This is for demonstrating the operation of the app.
- (NSNumber*) docommand_demo: (NSScanner*) scanner
{
    int num_secs;
    if (![scanner scanInt: &num_secs]) return [NSNumber numberWithBool: NO];
    int num_times;
    if (![scanner scanInt: &num_times]) return [NSNumber numberWithBool: NO];
    float rate;
    if (![scanner scanFloat:&rate]) return [NSNumber numberWithBool: NO];

    NSLog(@"docommand_demo num_secs=%i, num_times=%i, rate=%f", num_secs, num_times, rate);  // TODO
    

    NSURL* directory = [[self appDelegate] moviesDirectory];
    NSLog(@"Movies directory: %@", directory);
    NSLog(@"Demo movie file: %@", [directory URLByAppendingPathComponent:@"Demo.m4v"]);
    [[self appDelegate] doPlaybackOf: [directory URLByAppendingPathComponent:@"Demo.m4v"]
                            skipback: num_secs duration: num_secs showings: num_times rate: rate];
    return [NSNumber numberWithBool: YES];
}

// "START video_name_root  -- start recording video"
- (NSNumber*) docommand_start: (NSScanner*) scanner
{
    // We want to consume all of the rest of the line as the filename root.  There's no "empty" character set, but this is close enough.
    [scanner setCharactersToBeSkipped:[NSCharacterSet illegalCharacterSet]];
    NSString* filename = NULL;
    if (![scanner scanUpToCharactersFromSet:[NSCharacterSet illegalCharacterSet] intoString:&filename]) return [NSNumber numberWithBool: NO];

    [[self appDelegate] setMovieFileName:[filename stringByTrimmingCharactersInSet:
                                          [NSCharacterSet whitespaceCharacterSet]]];

    [[self appDelegate] startRecording];

    return [NSNumber numberWithBool: YES];
}

// "CANCEL  -- cancel recording"
- (NSNumber*) docommand_cancel: (NSScanner*) scanner
{
    [[self appDelegate] cancelRecording];
    return [NSNumber numberWithBool: YES];
}

// "REPLAY skipback showings rate  -- stop recording if recording; playback"
- (NSNumber*) docommand_replay: (NSScanner*) scanner
{
    int num_secs;
    if (![scanner scanInt: &num_secs]) return [NSNumber numberWithBool: NO];
    int num_times;
    if (![scanner scanInt: &num_times]) return [NSNumber numberWithBool: NO];
    float rate;
    if (![scanner scanFloat:&rate]) return [NSNumber numberWithBool: NO];
    
    [[self appDelegate] cancelRecording];

    [[self appDelegate] doPlaybackOf: [[self appDelegate] moviePath]
                            skipback: num_secs duration: num_secs showings: num_times rate: rate];
    return [NSNumber numberWithBool: YES];
}
@end
