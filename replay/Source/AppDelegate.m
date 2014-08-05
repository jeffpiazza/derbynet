//
//  AppDelegate.m
//
//  Created by Jeff Piazza on 4/21/14.
//  Copyright (c) 2014 ___FULLUSERNAME___. All rights reserved.
//
// TODO: Show connection status (green/red)
// TODO: Allow changing connection, especially if connection fails.  Verify there's no leak.
// TODO: Instead of polling, listen on port for commands!!
// TODO: Preview on/off
//

#import "AppDelegate.h"
#import "CommandListener.h"

@interface AppDelegate () <AVCaptureFileOutputDelegate, AVCaptureFileOutputRecordingDelegate>
{
    // cf _urlSheet, _urlField that are expressed as properties
    NSMutableData* responseData;
    NSURLConnection* connection;
}

// Capture-related stuff
@property (retain) AVCaptureSession* session;
@property (retain) AVCaptureDeviceInput* videoDeviceInput;
@property (retain) AVCaptureDeviceInput* audioDeviceInput;
@property (retain) AVCaptureMovieFileOutput *movieFileOutput;
@property (retain) AVCaptureVideoPreviewLayer *previewLayer;

@property (retain) CommandListener* listener;

// Playback-related stuff

- (void) refreshDevices;
- (BOOL) selectedVideoDeviceProvidesAudio;
@end

@implementation AppDelegate
- (id)init
{
	self = [super init];
	if (self) {
        _listener = [[CommandListener alloc] initWithDelegate: self];
        
		// Create a capture session
		_session = [[AVCaptureSession alloc] init];
        
		// Attach outputs to session
		_movieFileOutput = [[AVCaptureMovieFileOutput alloc] init];
		[_movieFileOutput setDelegate:self];
		[_session addOutput: _movieFileOutput];
        
		NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
		//id deviceWasConnectedObserver =
        [notificationCenter addObserverForName:AVCaptureDeviceWasConnectedNotification
                                        object:nil
                                         queue:[NSOperationQueue mainQueue]
                                    usingBlock:^(NSNotification *note) {
                                        [self refreshDevices];
                                        // TODO: [note object] is the new device
                                    }];
		//id deviceWasDisconnectedObserver =
        [notificationCenter addObserverForName:AVCaptureDeviceWasDisconnectedNotification
                                        object:nil
                                         queue:[NSOperationQueue mainQueue]
                                    usingBlock:^(NSNotification *note) {
                                        [self refreshDevices];
                                    }];

        [self setIsPlaying: NO];
        
        [self setMovieFileName:@"Unspecified Movie"];
    }
    return self;
}

- (void) refreshDevices
{
    [self setVideoDevices:[[AVCaptureDevice devicesWithMediaType: AVMediaTypeMuxed]
        arrayByAddingObjectsFromArray:[AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]]];
    [self setAudioDevices: [AVCaptureDevice devicesWithMediaType: AVMediaTypeAudio]];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[self listener] startOnPort:0];

    // Insert code here to initialize your application
    [self refreshDevices];

	// Attach preview to session
	CALayer *previewViewLayer = [[self previewView] layer];
	[previewViewLayer setBackgroundColor:CGColorGetConstantColor(kCGColorBlack)];
	AVCaptureVideoPreviewLayer *newPreviewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:[self session]];
	[newPreviewLayer setFrame:[previewViewLayer bounds]];
	[newPreviewLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
	[previewViewLayer addSublayer:newPreviewLayer];
	[self setPreviewLayer:newPreviewLayer];
	
	// Start the session
	[[self session] startRunning];
    
    if (!([[self window] styleMask] & NSFullScreenWindowMask)) {
        [[self window] toggleFullScreen: nil];
    }
    
    // TODO: If we want the Replay process to register itself with the web server, this would be the way to go.
    // For now, just tell the operator what port we're listening on, and let them type it in to the web server.
    [self showUrlSheet];
}

- (void)showUrlSheet
{
    if (!_urlSheet) {
        NSLog(@"Loading UrlDialog nib");
        [NSBundle loadNibNamed:@"UrlDialog" owner: self];
    }
    
    [_urlSheet setPreventsApplicationTerminationWhenModal:NO];
    
    [NSApp beginSheet: _urlSheet
       modalForWindow: [self window]
        modalDelegate: self
       didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
          contextInfo: nil];

    NSLog(@"urlSheet says %d", [_urlSheet canBecomeKeyWindow]);
    // These both appear to be necessary, but it's not really clear why.
    NSLog(@"Making URL sheet key window");
    [_urlSheet makeKeyWindow];
    [_urlField becomeFirstResponder];
    NSLog(@"showUrlSheet exits");
}

- (IBAction) tryNewUrl: (id) sender
{
    NSLog(@"tryNewUrl");
    self.poller = [[Poller alloc] init];
    [self.poller setPort: [[self listener] port]];
    [self.poller setUrlAndPoll: [_urlField stringValue]];

    [NSApp endSheet:_urlSheet];
}

- (IBAction) closeUrlSheet: (id) sender
{
    NSLog(@"closeUrlSheet");
    [NSApp endSheet:_urlSheet];
}

- (void)didEndSheet:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"didEndSheet!");
    [sheet orderOut:self];
}

- (void)windowWillClose:(NSNotification *)notification
{
	// Stop the session
	[[self session] stopRunning];
	
	// Set movie file output delegate to nil to avoid a dangling pointer
	[[self movieFileOutput] setDelegate:nil];
	
    /*
	// Remove Observers
	NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
	for (id observer in [self observers])
		[notificationCenter removeObserver:observer];
	[observers release];
     */
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication *) theApplication
{
    return YES;
}

- (AVCaptureDevice *)selectedVideoDevice
{
    NSLog(@"selectedVideoDevice");
	return [_videoDeviceInput device];
}

- (void)setSelectedVideoDevice:(AVCaptureDevice *)selectedVideoDevice
{
    NSLog(@"setSelectedVideoDevice");
	[[self session] beginConfiguration];
	
	if ([self videoDeviceInput]) {
		// Remove the old device input from the session
		[[self session] removeInput:[self videoDeviceInput]];
		[self setVideoDeviceInput:nil];
	}
	
	if (selectedVideoDevice) {
		NSError *error = nil;
		
		// Create a device input for the device and add it to the session
		AVCaptureDeviceInput *newVideoDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:selectedVideoDevice error:&error];
		if (newVideoDeviceInput == nil) {
			dispatch_async(dispatch_get_main_queue(), ^(void) {
				[[NSApplication sharedApplication] presentError:error];
			});
		} else {
			if (![selectedVideoDevice supportsAVCaptureSessionPreset:[[self session] sessionPreset]])
				[[self session] setSessionPreset:AVCaptureSessionPresetHigh];
			
			[[self session] addInput:newVideoDeviceInput];
			[self setVideoDeviceInput:newVideoDeviceInput];
		}
	}
	
	// If this video device also provides audio, don't use another audio device
	if ([self selectedVideoDeviceProvidesAudio])
		[self setSelectedAudioDevice:nil];
	
	[[self session] commitConfiguration];
}

- (BOOL)selectedVideoDeviceProvidesAudio
{
	return ([[self selectedVideoDevice] hasMediaType:AVMediaTypeMuxed] || [[self selectedVideoDevice] hasMediaType:AVMediaTypeAudio]);
}

- (AVCaptureDevice *)selectedAudioDevice
{
	return [_audioDeviceInput device];
}

- (void)setSelectedAudioDevice:(AVCaptureDevice *)selectedAudioDevice
{
    NSLog(@"setSelectedAudioDevice");
	[[self session] beginConfiguration];
	
	if ([self audioDeviceInput]) {
		// Remove the old device input from the session
		[[self session] removeInput:[self audioDeviceInput]];
		[self setAudioDeviceInput:nil];
	}
	
	if (selectedAudioDevice && ![self selectedVideoDeviceProvidesAudio]) {
		NSError *error = nil;
		
		// Create a device input for the device and add it to the session
		AVCaptureDeviceInput *newAudioDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:selectedAudioDevice error:&error];
		if (newAudioDeviceInput == nil) {
			dispatch_async(dispatch_get_main_queue(), ^(void) {
				[[NSApplication sharedApplication] presentError:error];
			});
		} else {
			if (![selectedAudioDevice supportsAVCaptureSessionPreset:[[self session] sessionPreset]])
				[[self session] setSessionPreset:AVCaptureSessionPresetHigh];
			
			[[self session] addInput:newAudioDeviceInput];
			[self setAudioDeviceInput:newAudioDeviceInput];
		}
	}
	
	[[self session] commitConfiguration];
}

- (NSURL*) moviesDirectory
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSMoviesDirectory, NSUserDomainMask, YES);
    NSURL* movies_dir = [NSURL fileURLWithPath:[paths objectAtIndex: 0]];

    NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setFormatterBehavior:NSDateFormatterBehavior10_4];
    [dateFormatter setDateFormat:@"yyyy-MM-dd"];
    NSString* dateString = [dateFormatter stringFromDate:[NSDate date]];

    NSURL* subdir = [[movies_dir URLByAppendingPathComponent:@"Derby"] URLByAppendingPathComponent: dateString];
    BOOL isdir = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath: [subdir path] isDirectory:&isdir] || !isdir) {
        NSError* error;
        // TODO: Real attributes
        // TODO: name exists but as a file
        if (![[NSFileManager defaultManager] createDirectoryAtURL: subdir
                                      withIntermediateDirectories: YES
                                                       attributes: nil
                                                            error: &error]) {
            [[NSApplication sharedApplication] presentError:error];
        }
    }
    return subdir;
}

- (void) setMovieFileName:(NSString *)root
{
    NSURL* directory = [self moviesDirectory];
    
    int i = 0;
    while (true) {
        NSString* name;
        if (i == 0) {
            name = [NSString stringWithFormat:@"%@.mov", root];
        } else {
            name = [NSString stringWithFormat:@"%@-%d.mov", root, i];
        }
        NSURL* filepath = [directory URLByAppendingPathComponent:name];
        if (![[NSFileManager defaultManager] fileExistsAtPath: [filepath path] isDirectory:NULL]) {
            [self setMoviePath:filepath];
            break;
        }
    }
}

- (void) startRecording
{
    [[self movieFileOutput] startRecordingToOutputFileURL:[self moviePath] recordingDelegate: self];
}

- (void) cancelRecording
{
    [[self movieFileOutput] stopRecording];
    
    [[NSFileManager defaultManager] removeItemAtPath: [[self moviePath] path] error:NULL];
}

// replayRecording is what gets invoked by the Replay button
- (void) replayRecording
{
    [self doPlaybackOf: [self moviePath] skipback: 3 duration: 3 showings: 2 rate: 0.5];
}

- (void) captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)
         outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error
{
    if (error != nil && [error code] != noErr) {
        [[NSApplication sharedApplication] presentError:error];
    }
}

- (BOOL)captureOutputShouldProvideSampleAccurateRecordingStart:(AVCaptureOutput *)captureOutput
{
    // Sucks up a lot of CPU to say "YES", but there's maybe a half-second delay to say "NO".
    return NO;
}

- (void) setPortMessage:(NSString *)msg
{
    [[self portView] setStringValue:msg];
}
- (void) setStatus:(NSString *)msg
{
    [[self statusView] setStringValue:msg];
}

- (void) doPlaybackOf: (NSURL*) url skipback: (int) num_secs duration: (int) duration showings: (int) showings rate: (float) rate
{
    [[self movieFileOutput] stopRecording];

    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
    NSString *tracksKey = @"tracks";
    [asset loadValuesAsynchronouslyForKeys:@[tracksKey] completionHandler:
     ^{
         dispatch_async(dispatch_get_main_queue(),
                        ^{
                            NSError *error;
                            AVKeyValueStatus status = [asset statusOfValueForKey:tracksKey error: &error];
                            if (status == AVKeyValueStatusLoaded) {
                                NSMutableArray *items = [NSMutableArray arrayWithCapacity: showings];
                                for (int k = 0; k < showings; ++k) {
                                    AVPlayerItem* playerItem = [AVPlayerItem playerItemWithAsset:asset];
                                
                                    if (k == showings - 1) {
                                        [[NSNotificationCenter defaultCenter]
                                         addObserver:self
                                         selector:@selector(playerItemDidReachEnd:)
                                         name:AVPlayerItemDidPlayToEndTimeNotification
                                         object: playerItem];
                                    }
                                    
                                    CMTime duration = [asset duration];
                                    CMTime skip_back = CMTimeMake(num_secs, 1);
                                    if (CMTimeCompare(duration, skip_back) > 0) {
                                        [playerItem seekToTime:CMTimeSubtract(duration, skip_back)];
                                    } else {
                                        [playerItem seekToTime: kCMTimeZero];
                                    }
                                    
                                    [items addObject: playerItem];
                                }
                                
                                AVQueuePlayer* player = [[AVQueuePlayer alloc] initWithItems:items];
                                [[self playerView] setPlayer: player];

                                [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
                                [NSCursor hide];
                                // This unhides the playerview, which covers all the other views in the window.
                                [[self playerView] setHidden: NO];
                                //[player play];
                                // Amazing!  [player play] is effectively the same as [player setRate: 1.0].  Calling setRate instead gives the desired behavior.
                                [player setRate: rate];
                            }
                        });
     }];
}

- (void) playerItemDidReachEnd: (id) sender
{
    [[self playerView] setHidden:YES];
    [NSCursor unhide];
    [NSCursor setHiddenUntilMouseMoves:YES];
    [[NSApplication sharedApplication] hide: self];
}

@synthesize url = _url;

- (NSString*) url
{
    return _url;
}

- (void) setUrl:(NSString *)url
{
    _url = url;
    NSLog(@"Set URL to %@", url);
}

@end
