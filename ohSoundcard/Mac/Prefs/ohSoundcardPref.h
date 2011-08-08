
#import <PreferencePanes/PreferencePanes.h>
#import "Preferences.h"


@interface ohSoundcardPref : NSPreferencePane 
{
    IBOutlet NSImageView* icon;
    IBOutlet NSTextField* textSenderName;
    IBOutlet NSButton* buttonOnOff;
    IBOutlet NSTextField* textDescription;
    IBOutlet NSButton* buttonShowInStatusBar;
    IBOutlet NSButton* buttonHelp;

    Preferences* iPreferences;
}

@property (assign) NSImageView* icon;
@property (assign) NSTextField* textSenderName;
@property (assign) NSButton* buttonOnOff;
@property (assign) NSTextField* textDescription;
@property (assign) NSButton* buttonShowInStatusBar;
@property (assign) NSButton* buttonHelp;

- (void) mainViewDidLoad;
- (void) updateButtonOnOff;
- (IBAction) buttonOnOffClicked:(id)aSender;
- (IBAction) buttonShowInStatusBarClicked:(id)aSender;
- (IBAction) buttonHelpClicked:(id)aSender;
- (void) preferenceEnabledChanged:(NSNotification*)aNotification;

@end
