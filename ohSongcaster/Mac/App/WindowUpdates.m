
#import "WindowUpdates.h"


@implementation WindowUpdates

@synthesize progressChecking;
@synthesize progressDownloading;
@synthesize textAvailable;
@synthesize viewChecking;
@synthesize viewAvailable;
@synthesize viewUnavailable;
@synthesize viewDownloading;


- (void) awakeFromNib
{
    [[self window] orderOut:self];
}


- (void) setAutoUpdate:(AutoUpdate*)aAutoUpdate
{
    iAutoUpdate = [aAutoUpdate retain];
}


- (void) hideAll
{
    [viewChecking setHidden:YES];
    [viewAvailable setHidden:YES];
    [viewUnavailable setHidden:YES];
    [viewDownloading setHidden:YES];

    [progressChecking stopAnimation:self];
    [progressDownloading stopAnimation:self];
}


- (void) checkForUpdatesFinished
{
    // back in the main thread - show the next state of the window
    [self hideAll];
    if (iUpdateFound == NULL) {
        [viewUnavailable setHidden:NO];
    }
    else {
        [textAvailable setStringValue:[NSString stringWithFormat:NSLocalizedString(@"UpdateAvailableText", @""), [iUpdateFound appName], [iUpdateFound version]]];
        [viewAvailable setHidden:NO];
    }
}


- (void) performCheckForUpdates
{
    // running in a background thread - must create an autorelease pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    // check for updates
    iUpdateFound = [[iAutoUpdate checkForUpdates] retain];
    // post back to main thread
    [self performSelectorOnMainThread:@selector(checkForUpdatesFinished) withObject:nil waitUntilDone:FALSE];

    // clean up autoreleased objects
    [pool drain];
}


- (void) downloadFinished
{
    // back in the main thread - download has finished
    [[self window] orderOut:self];
}


- (void) performDownload
{
    // running in a background thread - must create an autorelease pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    // download the update and run the installer
    [iAutoUpdate installUpdate:iUpdateFound];
    // post back to main thread
    [self performSelectorOnMainThread:@selector(downloadFinished) withObject:nil waitUntilDone:FALSE];

    // clean up autoreleased objects
    [pool drain];
}


- (void) startChecking
{
    if (iAutoUpdate == NULL) {
        return;
    }

    // show the update window in checking mode
    [self hideAll];
    [viewChecking setHidden:NO];
    [progressChecking startAnimation:self];
    [[self window] center];
    [[self window] makeKeyAndOrderFront:self];

    // start checking on a background thread
    if (iUpdateFound != NULL) {
        [iUpdateFound release];
        iUpdateFound = NULL;
    }
    [self performSelectorInBackground:@selector(performCheckForUpdates) withObject:nil];
}


- (IBAction)buttonDetailsClicked:(id)aSender
{
    if (iUpdateFound != NULL)
    {
        NSURL* url = [NSURL URLWithString:[iUpdateFound historyUri]];
        [[NSWorkspace sharedWorkspace] openURL:url];
    }
}


- (IBAction)buttonInstallClicked:(id)aSender
{
    // show the downloading view
    [self hideAll];
    [viewDownloading setHidden:NO];
    [progressDownloading startAnimation:self];

    // start downloading on a background thread
    [self performSelectorInBackground:@selector(performDownload) withObject:nil];
}


- (IBAction)buttonNotNowClicked:(id)aSender
{
    // don't install - just close the window
    [[self window] orderOut:self];
}


@end








