
#import "SongcasterAppDelegate.h"
#import "CrashLogging.h"


@implementation SongcasterAppDelegate

@synthesize menu;
@synthesize menuItemStatus;
@synthesize menuItemOnOff;
@synthesize menuItemReconnect;
@synthesize menuItemPrefs;


- (void) sessionDidResignActive:(NSNotification*)aNotification
{
    // set the system state
    iSessionResigned = true;

    // user session has been resigned - stop the songcaster - this will do nothing
    // if the model has not been started i.e. this notification has occurred on
    // startup of the application
    [model stop];
}


- (void) sessionDidBecomeActive:(NSNotification*)aNotification
{
    // set the system state
    iSessionResigned = false;

    // user session has just become active again - restart the songcaster
    [model start];
}


- (void) networkReachabilityChanged:(SCNetworkReachabilityFlags)aFlags
{
    // ignore any changes when sleeping or when this user session is resigned
    if (iSleeping || iSessionResigned) {
        return;
    }

    if (aFlags == kSCNetworkReachabilityFlagsReachable)
    {
        [model start];
    }
    else
    {
        [model stop];
    }
}


void NetworkReachabilityChanged(SCNetworkReachabilityRef aReachability,
                                SCNetworkReachabilityFlags aFlags,
                                void* aInfo)
{
    SongcasterAppDelegate* app = (SongcasterAppDelegate*)aInfo;
    [app networkReachabilityChanged:aFlags];
}


- (void) willSleep:(NSNotification*)aNotification
{
    // set the system state
    iSleeping = true;

    // stop the songcaster
    [model stop];
}


- (void) didWake:(NSNotification*)aNotification
{
    // set the system state - do not restart the songcaster here since the network
    // may be unavailable
    iSleeping = false;
}


- (void) awakeFromNib
{
    NSString* productId = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
    NSString* crashLogUrl = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcasterCrashLogUrl"];

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
    model = [[Model alloc] init];
    [model setObserver:self];

    // initialise system state
    iSessionResigned = false;
    iSleeping = false;

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

    SCNetworkReachabilityContext context;
    context.version = 0;
    context.retain = NULL;
    context.release = NULL;
    context.copyDescription = NULL;
    context.info = self;

    iReachability = SCNetworkReachabilityCreateWithName(NULL, "www.google.com");
    SCNetworkReachabilitySetCallback(iReachability, NetworkReachabilityChanged, &context);
    SCNetworkReachabilityScheduleWithRunLoop(iReachability, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // start the songcaster if the user session is active and the system is not asleep
    if (!iSessionResigned && !iSleeping) {
        [model start];
    }
}


- (void)applicationWillTerminate:(NSNotification *)notification
{
    // stop and clean up the model
    [model stop];
    [model release];

    if (statusItem)
    {
        [statusItem release];
        statusItem = nil;
    }
}


- (IBAction)menuItemOnOffClicked:(id)aSender
{
    // toggle the state of the songcaster - allow the eventing to come back
    // up through enabledChanged to update the UI
    [model setEnabled:![model enabled]];
}


- (IBAction)menuItemReconnectClicked:(id)aSender
{
    [model reconnectReceivers];
}


- (IBAction)menuItemPrefsClicked:(id)aSender
{
    NSString* prefPaneName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcasterPreferencePane"];
    [[NSWorkspace sharedWorkspace] openFile:prefPaneName];
}


- (void) enabledChanged
{
    // do nothing if the status item is not visible
    if (![model iconVisible])
    {
        return;
    }

    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
    
    // get strings and image for the new status
    NSString* imageFile;
    NSString* menuItemStatusText;
    NSString* menuItemOnOffText;
    
    if ([model enabled])
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
    [statusItem setImage:image];
    [image release];
    
    [menuItemStatus setTitle:[NSString stringWithFormat:menuItemStatusText, appName]];
    [menuItemOnOff setTitle:[NSString stringWithFormat:menuItemOnOffText, appName]];
}


- (void) iconVisibleChanged
{
    bool visible = [model iconVisible];
    
    if (visible && statusItem == nil)
    {
        // create the status bar item
        statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
        [statusItem setHighlightMode:YES];
        [statusItem setMenu:menu];
        
        // make sure icon/menus are correctly configured
        [self enabledChanged];
    }
    else if (!visible && statusItem)
    {
        // remove the status item
        [[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
        [statusItem release];
        statusItem = nil;
    }
}


@end








