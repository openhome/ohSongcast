
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
    [[self window] center];
    [[self window] setLevel:NSFloatingWindowLevel];
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


- (void) checkForUpdatesFinished:(AutoUpdateInfo*)aUpdateInfo
{
    // back running in main thread - store the found update info
    [iUpdateCheck setUpdateInfo:aUpdateInfo];

    if (aUpdateInfo != NULL)
    {
        // update found - show the available view
        [self hideAll];
        [textAvailable setStringValue:[NSString stringWithFormat:@"A new version of %@ (%@) is available.", [aUpdateInfo appName], [aUpdateInfo version]]];
        [viewAvailable setHidden:NO];

        // if this is an automatic update check, the window is not visible yet
        if ([iUpdateCheck isManual] == false)
        {
            [[self window] center];
            [[self window] makeKeyAndOrderFront:self];
        }
    }
    else
    {
        // no update found - changing the view does not matter when doing an automatic check
        // because the window is not visible
        [self hideAll];
        [viewUnavailable setHidden:NO];
    }
}


- (void) performCheckForUpdates
{
    // running in a background thread - must create an autorelease pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    // check for updates
    AutoUpdateInfo* updateInfo = [iAutoUpdate checkForUpdates];
    // post back to main thread
    [self performSelectorOnMainThread:@selector(checkForUpdatesFinished:) withObject:updateInfo waitUntilDone:FALSE];

    // clean up autoreleased objects
    [pool drain];
}


- (void) downloadFinished
{
    // back in the main thread - download has finished
    [[self window] orderOut:self];
}


- (void) performDownload:(AutoUpdateInfo*)aUpdateInfo
{
    // running in a background thread - must create an autorelease pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    // download the update and run the installer
    [iAutoUpdate installUpdate:aUpdateInfo];
    // post back to main thread
    [self performSelectorOnMainThread:@selector(downloadFinished) withObject:nil waitUntilDone:FALSE];

    // clean up autoreleased objects
    [pool drain];
}


- (void) startAutomaticCheck
{
    if (iAutoUpdate == NULL) {
        return;
    }

    // start checking on a background thread - don't show any UI just yet
    if (iUpdateCheck != NULL) {
        [iUpdateCheck release];
    }
    iUpdateCheck = [[UpdateCheck alloc] init];
    [iUpdateCheck setIsManual:false];

    [self performSelectorInBackground:@selector(performCheckForUpdates) withObject:nil];
}


- (void) startManualCheck
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
    if (iUpdateCheck != NULL) {
        [iUpdateCheck release];
    }
    iUpdateCheck = [[UpdateCheck alloc] init];
    [iUpdateCheck setIsManual:true];

    [self performSelectorInBackground:@selector(performCheckForUpdates) withObject:nil];
}


- (IBAction)buttonDetailsClicked:(id)aSender
{
    if (iUpdateCheck != NULL && [iUpdateCheck updateInfo] != NULL)
    {
        NSURL* url = [NSURL URLWithString:[[iUpdateCheck updateInfo] historyUri]];
        [[NSWorkspace sharedWorkspace] openURL:url];
    }
}


- (IBAction)buttonInstallClicked:(id)aSender
{
    // show the downloading view
    [self hideAll];
    [viewDownloading setHidden:NO];
    [progressDownloading startAnimation:self];

    // start downloading on a background thread - pass in the update object so the other thread does
    // not have to directly access members
    [self performSelectorInBackground:@selector(performDownload:) withObject:[iUpdateCheck updateInfo]];
}


- (IBAction)buttonNotNowClicked:(id)aSender
{
    // don't install - just close the window
    [[self window] orderOut:self];
}


@end


@implementation UpdateCheck

@synthesize isManual;
@synthesize updateInfo;

- (void) dealloc
{
    [updateInfo release];
    [super dealloc];
}

@end






