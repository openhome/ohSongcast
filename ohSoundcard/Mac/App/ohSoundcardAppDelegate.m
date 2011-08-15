
#import "ohSoundcardAppDelegate.h"


@implementation ohSoundcardAppDelegate

@synthesize menu;
@synthesize menuItemStatus;
@synthesize menuItemOnOff;
@synthesize menuItemReconnect;
@synthesize menuItemPrefs;


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

    // initialise menu items that do not depend on state
    [menuItemReconnect setTitle:NSLocalizedString(@"MenuReconnect", @"")];
    [menuItemPrefs setTitle:[NSString stringWithFormat:NSLocalizedString(@"MenuPreferences", @""), appName]];

    // create and start the model
    model = [[Model alloc] init];
    [model start];
    [model setObserver:self];

    // initialise the state dependent parts of the UI
    [self iconVisibleChanged];
    [self enabledChanged];
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
    // toggle the state of the soundcard - allow the eventing to come back
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








