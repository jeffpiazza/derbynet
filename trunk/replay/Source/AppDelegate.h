//
//  AppDelegate.h
//
//  Created by Jeff Piazza on 4/21/14.
//  Copyright (c) 2014 ___FULLUSERNAME___. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Poller;
@class CommandListener;

#define STATUS_RECORDING_ERROR  (-4)
#define STATUS_NO_AUDIO_CHOSEN  (-3)
#define STATUS_NO_VIDEO_CHOSEN  (-2)
#define STATUS_NOT_CONNECTED    (-1)
#define STATUS_READY            0
#define STATUS_RECORDING        1
#define STATUS_PLAYING          2


@interface AppDelegate : NSObject <NSApplicationDelegate>
{
}

- (void) startRecording;
- (void) cancelRecording;
- (void) doPlaybackOf: (NSURL*) url skipback: (int) num_secs duration: (int) duration showings: (int) showings rate: (float) rate;

- (void) setPortMessage: (NSString*) msg;
- (void) setStatusMessage: (NSString*) msg;

- (void) setMovieFileName: (NSString*) name;
- (NSURL*) moviesDirectory;

@property BOOL isPlaying;

@property int status;

// URL for the web server for polling
@property (retain) NSString* url;

@property (retain) NSArray* videoDevices;
@property (retain) NSArray* audioDevices;
@property (assign) AVCaptureDevice* selectedVideoDevice;
@property (assign) AVCaptureDevice* selectedAudioDevice;

// Full path to the current movie file
@property (retain) NSURL* moviePath;
@property (retain) Poller* poller;

@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSView *previewView;
@property (weak) IBOutlet NSTextField* portView;
@property (weak) IBOutlet NSTextField* statusView;

@property (assign) IBOutlet NSWindow* urlSheet;
@property (weak) IBOutlet NSTextField* urlField;

// Playback:
@property (weak) IBOutlet NSView* controlContainerView;
@property (weak) IBOutlet AVPlayerView *playerView;

@end
