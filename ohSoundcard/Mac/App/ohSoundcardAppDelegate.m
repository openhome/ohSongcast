
#import "ohSoundcardAppDelegate.h"


@implementation ohSoundcardAppDelegate

@synthesize menu;
@synthesize menuItemStatus;
@synthesize menuItemOnOff;
@synthesize menuItemPrefs;


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // create the status bar item
    NSStatusBar* bar = [NSStatusBar systemStatusBar];
    statusItem = [[bar statusItemWithLength:NSSquareStatusItemLength] retain];
    [statusItem setHighlightMode:YES];
    [statusItem setMenu:menu];

    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

    // initialise menu items that do not depend on state
    [menuItemPrefs setTitle:[NSString stringWithFormat:NSLocalizedString(@"MenuPreferences", @""), appName]];

    // create and start the model
    model = [[Model alloc] init];
    [model start];

    // initialise the state dependent parts of the menu
    [self setMenuStatus:[model enabled]];
}


- (void)applicationWillTerminate:(NSNotification *)notification
{
    // stop and clean up the model
    [model stop];
    [model release];
    
    [statusItem release];
}


- (IBAction)menuItemOnOffClicked:(id)aSender
{
    // toggle the state of the soundcard
    [model setEnabled:![model enabled]];

    // update the ui
    [self setMenuStatus:[model enabled]];
}


- (IBAction)menuItemPrefsClicked:(id)aSender
{
    NSString* prefPaneName = NSLocalizedStringFromTable(@"PreferencePaneName", @"NonLocalizable", @"");
    [[NSWorkspace sharedWorkspace] openFile:prefPaneName];
}


- (void)setMenuStatus:(bool)aOn
{
    // get the bundle name from the info.plist
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

    // get strings and image for the new status
    NSString* imageFile;
    NSString* menuItemStatusText;
    NSString* menuItemOnOffText;
    
    if (aOn)
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



@end








