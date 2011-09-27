
#import "SongcasterAppDelegate.h"


@implementation SongcasterAppDelegate

@synthesize menu;
@synthesize menuItemStatus;
@synthesize menuItemOnOff;
@synthesize menuItemReconnect;
@synthesize menuItemPrefs;


- (void) sessionDidResignActive:(NSNotification*)aNotification
{
    // set the start resigned flag to true - this event is called before the
    // applicationDidFinishLaunching event
    iStartResigned = true;

    // user session has been resigned - stop the songcaster - this will do nothing
    // if the model has not been started i.e. this notification has occurred on
    // startup of the application
    [model stop];
}


- (void) sessionDidBecomeActive:(NSNotification*)aNotification
{
    // user session has just become active again - restart the songcaster
    [model start];
}


- (void) awakeFromNib
{
    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

    // initialise menu items that do not depend on state
    [menuItemReconnect setTitle:NSLocalizedString(@"MenuReconnect", @"")];
    [menuItemPrefs setTitle:[NSString stringWithFormat:NSLocalizedString(@"MenuPreferences", @""), appName]];

    // create and initialise the model
    model = [[Model alloc] init];
    [model setObserver:self];

    // initialise flag for when the app starts when the user session is resigned
    iStartResigned = false;

    // setup system event notifications
    NSNotificationCenter* center = [[NSWorkspace sharedWorkspace] notificationCenter];
//    [center addObserver:self selector:@selector(willSleep:)
//                             name:NSWorkspaceWillSleepNotification object:NULL];
//    [center addObserver:self selector:@selector(didWake:)
//                             name:NSWorkspaceDidWakeNotification object:NULL];
    [center addObserver:self selector:@selector(sessionDidResignActive:)
                             name:NSWorkspaceSessionDidResignActiveNotification object:NULL];
    [center addObserver:self selector:@selector(sessionDidBecomeActive:)
                             name:NSWorkspaceSessionDidBecomeActiveNotification object:NULL];
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // start the model if this instance of the application started while the user
    // session is active
    if (!iStartResigned) {
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
    NSString* prefPaneName = NSLocalizedStringFromTable(@"PreferencePaneName", @"NonLocalizable", @"");
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








