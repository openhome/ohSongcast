
#import "SongcastAppDelegate.h"
#import "CrashLogging.h"


@implementation SongcastAppDelegate

@synthesize menu;
@synthesize menuItemStatus;
@synthesize menuItemOnOff;
@synthesize menuItemReconnect;
@synthesize menuItemPrefs;


- (void) sessionDidResignActive:(NSNotification*)aNotification
{
    // set the system state
    iSessionResigned = true;

    // user session has been resigned - stop songcast - this will do nothing
    // if the model has not been started i.e. this notification has occurred on
    // startup of the application
    [iModel stop];
}


- (void) sessionDidBecomeActive:(NSNotification*)aNotification
{
    // set the system state
    iSessionResigned = false;

    // user session has just become active again - restart songcast
    [iModel start];
}


- (void) willSleep:(NSNotification*)aNotification
{
    // going into hibernation - switch off songcast
    [iModel setEnabled:false];
}


- (void) didWake:(NSNotification*)aNotification
{
}


- (void) awakeFromNib
{
    NSString* productId = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastProductId"];
    NSString* crashLogUrl = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastCrashLogUrl"];

    // check for any new crash logs
    if ([crashLogUrl length] != 0)
    {
        CrashLogDumperReport* crashDumper = [[CrashLogDumperReport alloc] initWithProductId:productId uri:crashLogUrl];

        CrashMonitor* crashMonitor = [[CrashMonitor alloc] init];
        [crashMonitor addDumper:crashDumper];
        [crashMonitor start];

        [crashMonitor release];
        [crashDumper release];
    }

    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

    // initialise menu items that do not depend on state
    [menuItemReconnect setTitle:NSLocalizedString(@"MenuReconnect", @"")];
    [menuItemPrefs setTitle:[NSString stringWithFormat:NSLocalizedString(@"MenuPreferences", @""), appName]];

    // create and initialise the model
    iModel = [[Model alloc] init];
    [iModel setObserver:self];

    // initialise system state
    iSessionResigned = false;

    // setup system event notifications
    NSNotificationCenter* center = [[NSWorkspace sharedWorkspace] notificationCenter];
    [center addObserver:self selector:@selector(willSleep:)
                             name:NSWorkspaceWillSleepNotification object:NULL];
    [center addObserver:self selector:@selector(didWake:)
                             name:NSWorkspaceDidWakeNotification object:NULL];
    [center addObserver:self selector:@selector(sessionDidResignActive:)
                             name:NSWorkspaceSessionDidResignActiveNotification object:NULL];
    [center addObserver:self selector:@selector(sessionDidBecomeActive:)
                             name:NSWorkspaceSessionDidBecomeActiveNotification object:NULL];

    // create the update window
    iWindowUpdates = [[WindowUpdates alloc] init];
    [iWindowUpdates setAutoUpdate:[iModel autoUpdate]];
    if ([NSBundle loadNibNamed:@"WindowUpdates.nib" owner:iWindowUpdates] == NO) {
        [iWindowUpdates release];
        iWindowUpdates = nil;
    }

    // do the automatic update check
    if ([iModel autoUpdatesEnabled])
    {
        [iWindowUpdates startAutomaticCheck];
    }
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // fire open the preferences if the wizard has not been run
    if (![iModel hasRunWizard]) {
        [self menuItemPrefsClicked:self];
    }

    // start songcast if the user session is active
    if (!iSessionResigned) {
        [iModel start];
    }
}


- (void)applicationWillTerminate:(NSNotification *)notification
{
    // stop and clean up the model
    [iModel stop];
    [iModel release];

    if (iStatusItem)
    {
        [iStatusItem release];
        iStatusItem = nil;
    }
}


- (IBAction)menuItemOnOffClicked:(id)aSender
{
    // toggle the state of songcast - allow the eventing to come back
    // up through enabledChanged to update the UI
    [iModel setEnabled:![iModel enabled]];
}


- (IBAction)menuItemReconnectClicked:(id)aSender
{
    [iModel reconnectReceivers];
}


- (IBAction)menuItemPrefsClicked:(id)aSender
{
    NSString* prefPaneName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastPreferencePane"];
    [[NSWorkspace sharedWorkspace] openFile:prefPaneName];
}


- (void) enabledChanged
{
    // do nothing if the status item is not visible
    if (![iModel iconVisible])
    {
        return;
    }

    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
    
    // get strings and image for the new status
    NSString* imageFile;
    NSString* menuItemStatusText;
    NSString* menuItemOnOffText;
    
    if ([iModel enabled])
    {
        imageFile = [[NSBundle mainBundle] pathForResource:@"MenuIconOn" ofType:@"png"];
        menuItemStatusText = NSLocalizedString(@"MenuStatusOn", @"");
        menuItemOnOffText = NSLocalizedString(@"MenuTurnOff", @"");
    }
    else
    {
        imageFile = [[NSBundle mainBundle] pathForResource:@"MenuIconOff" ofType:@"png"];
        menuItemStatusText = NSLocalizedString(@"MenuStatusOff", @"");
        menuItemOnOffText = NSLocalizedString(@"MenuTurnOn", @"");        
    }
    
    // set the images and text into the menu
    NSImage* image = [[NSImage alloc] initWithContentsOfFile:imageFile];
    [iStatusItem setImage:image];
    [image release];
    
    [menuItemStatus setTitle:[NSString stringWithFormat:menuItemStatusText, appName]];
    [menuItemOnOff setTitle:[NSString stringWithFormat:menuItemOnOffText, appName]];
}


- (void) iconVisibleChanged
{
    bool visible = [iModel iconVisible];
    
    if (visible && iStatusItem == nil)
    {
        // create the status bar item
        iStatusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
        [iStatusItem setHighlightMode:YES];
        [iStatusItem setMenu:menu];
        
        // make sure icon/menus are correctly configured
        [self enabledChanged];
    }
    else if (!visible && iStatusItem)
    {
        // remove the status item
        [[NSStatusBar systemStatusBar] removeStatusItem:iStatusItem];
        [iStatusItem release];
        iStatusItem = nil;
    }
}


- (void) checkForUpdates
{
    // start checking for updates
    [iWindowUpdates startManualCheck];
}


@end



