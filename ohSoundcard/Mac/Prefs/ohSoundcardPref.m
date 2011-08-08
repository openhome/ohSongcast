
#import "ohSoundcardPref.h"



@implementation ohSoundcardPref

@synthesize icon;
@synthesize textSenderName;
@synthesize buttonOnOff;
@synthesize textDescription;
@synthesize buttonShowInStatusBar;
@synthesize buttonHelp;


- (void) mainViewDidLoad
{
    // get the bundle name from the info.plist
    NSString* appName = [[[self bundle] infoDictionary] objectForKey:@"CFBundleName"];

    // initialise the text for the UI elements
    [buttonOnOff setTitle:[NSString stringWithFormat:[buttonOnOff title], appName]];
    [buttonOnOff setAlternateTitle:[NSString stringWithFormat:[buttonOnOff alternateTitle], appName]];
    [textDescription setStringValue:[NSString stringWithFormat:[textDescription stringValue], appName]];
    [buttonShowInStatusBar setTitle:[NSString stringWithFormat:[buttonShowInStatusBar title], appName]];    
    
    // create the preferences object
    iPreferences = [[Preferences alloc] initWithBundle:[self bundle]];    
    [iPreferences synchronize];

    // register for notifications from app
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];

    // initialise UI from preferences
    [self updateButtonOnOff];
    [buttonShowInStatusBar setState:([iPreferences iconVisible] ? NSOnState : NSOffState)];
}


- (void) updateButtonOnOff
{
    bool enabled = [iPreferences enabled];

    [buttonOnOff setState:(enabled ? NSOnState : NSOffState)];
}


- (IBAction) buttonOnOffClicked:(id)aSender
{
    [iPreferences setEnabled:([buttonOnOff state] == NSOnState)];
}


- (IBAction) buttonShowInStatusBarClicked:(id)aSender
{
    [iPreferences setIconVisible:([buttonShowInStatusBar state] == NSOnState)];
}


- (IBAction) buttonHelpClicked:(id)aSender
{
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    [iPreferences synchronize];
    [self updateButtonOnOff];
}


@end





