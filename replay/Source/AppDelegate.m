//
//  AppDelegate.m
//
//  Created by Jeff Piazza on 4/21/14.
//  Copyright (c) 2014 ___FULLUSERNAME___. All rights reserved.
//
// TODO: Show connection status (green/red)
// TODO: Instead of polling, listen on port for commands!!
// TODO: Configuration:
//    Slow-motion replay -- disable sound?
//    Selectable number of replays
//    Selectable playback length.
// TODO: Preview on/off
//

#import "AppDelegate.h"

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

// Playback-related stuff

- (void) refreshDevices;
- (BOOL) selectedVideoDeviceProvidesAudio;
@end

@implementation AppDelegate
- (id)init
{
	self = [super init];
	if (self) {
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
    
    //TODO
    [self showUrlSheet: [self window]];
}

- (void)showUrlSheet: (NSWindow *)window
{
    if (!_urlSheet)
        [NSBundle loadNibNamed:@"UrlDialog" owner: self];
    
    [NSApp beginSheet: _urlSheet
       modalForWindow: window
        modalDelegate: self
       didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
          contextInfo: nil];
}

- (IBAction) tryNewUrl: (id) sender
{
    NSLog(@"tryNewUrl");
    NSURL *baseURL = [NSURL URLWithString:[_urlField stringValue]];

    [NSApp endSheet:_urlSheet];
    
    // TODO: fix up baseURL, adding http:// on the front and /action.php?query=replay-poll on the end.  Maybe remove /index.php from the end, if present.
    NSURLRequest *request = [NSURLRequest requestWithURL:baseURL];

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
    NSString *strData = [[NSString alloc]initWithData:responseData encoding:NSUTF8StringEncoding];
    NSLog(@"%@",strData);
    
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
            // self tryNewUrl:
    });
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
	return [_videoDeviceInput device];
}

- (void)setSelectedVideoDevice:(AVCaptureDevice *)selectedVideoDevice
{
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
    NSURL* subdir = [movies_dir URLByAppendingPathComponent:@"Derby"];
    BOOL isdir = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath: [subdir path] isDirectory:&isdir] || !isdir) {
        NSError* error;
        // TODO: Real attributes
        // TODO: name exists but as a file
        if (![[NSFileManager defaultManager] createDirectoryAtURL: subdir
                                      withIntermediateDirectories: NO
                                                       attributes: nil
                                                            error: &error]) {
            [[NSApplication sharedApplication] presentError:error];
        }
    }
    return subdir;
}

- (NSURL*) movieFile
{
    return [[self moviesDirectory] URLByAppendingPathComponent:@"SampleRecording.mov"];
}

- (void) startRecording
{
    [[self movieFileOutput] startRecordingToOutputFileURL:[self movieFile] recordingDelegate: self];
}

- (void) stopRecording
{
    [[self movieFileOutput] stopRecording];
}

- (void) captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error
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

- (void) playerItemDidReachEnd: (id) sender
{
    [[self playerView] setHidden:YES];
    [[NSApplication sharedApplication] hide: self];
}

//- (void) doPlaybackOnce: (void (^)(void)) completion

- (void) doPlayback
{
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    
    [[self playerView] setHidden: NO];
    
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:[self movieFile] options:nil];
    NSString *tracksKey = @"tracks";
    [asset loadValuesAsynchronouslyForKeys:@[tracksKey] completionHandler:
     ^{
         dispatch_async(dispatch_get_main_queue(),
                        ^{
                            NSError *error;
                            AVKeyValueStatus status = [asset statusOfValueForKey:tracksKey error: &error];
                            if (status == AVKeyValueStatusLoaded) {
                                // TODO: Rewind to T-3.0 seconds
                                AVPlayerItem* playerItem = [AVPlayerItem playerItemWithAsset:asset];
                                // TODO: The observer should be a separate object that keeps a count of number of replays still to perform
                                [[NSNotificationCenter defaultCenter]
                                 addObserver:self
                                 selector:@selector(playerItemDidReachEnd:)
                                 name:AVPlayerItemDidPlayToEndTimeNotification
                                 object: playerItem];
                                
                                // Render the playerView hidden when playback stops.
                                AVPlayer* player = [AVPlayer playerWithPlayerItem:playerItem];
                                CMTime duration = [asset duration];
                                CMTime skip_back = CMTimeMake(3, 1);
                                if (CMTimeCompare(duration, skip_back) > 0) {
                                    [player seekToTime:CMTimeSubtract(duration, skip_back)];
                                }
                                // http://stackoverflow.com/questions/6630356/avplayer-rate-property-does-not-work
                                // Maybe slo-mo requires no sound track?
                                [player setRate: 0.5];
                                if ([playerItem canPlaySlowForward]) {
                                    NSLog(@"Player's rate is %1.2f", [player rate]);
                                } else {
                                    NSLog(@"Can't play slow forward!");
                                }
                                [[self playerView] setPlayer: player];
                                [player play];
                            }
                        });
     }];
}

- (void) replayRecording
{
    // [[self window] toggleFullScreen:nil];
    if (![self isPlaying]) {  // We weren't playing before, but we're about to start
        [self setIsPlaying:YES];
        [self performSelector: @selector(doPlayback) withObject: nil afterDelay: 5.0];
    } else {
        [[self playerView] setHidden: YES];
        [self setIsPlaying:NO];
    }
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
