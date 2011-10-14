
#import <PreferencePanes/PreferencePanes.h>
#import "Preferences.h"


// Receiver class for data displayed in the table view
@interface Receiver : NSObject<NSCopying>
{
    NSString* udn;
    NSString* title;
    bool selected;
    EReceiverState status;
}

@property (assign) NSString* udn;
@property (assign) NSString* title;
@property (assign) bool selected;
@property (assign) EReceiverState status;

- (id) initWithPref:(PrefReceiver*)aPref uniqueInRoom:(bool)aUnique;

@end


// Main class for the preference pane
@interface SongcasterPrefPane : NSPreferencePane 
{
    IBOutlet NSButton* buttonOnOff;
    IBOutlet NSTextField* textDescription;
    IBOutlet NSButton* buttonShowInStatusBar;
    IBOutlet NSTableView* tableViewReceiverList;
    IBOutlet NSBox* boxGettingStarted;
    IBOutlet NSBox* boxMain;
    IBOutlet NSTextField* textStep1Text;
    IBOutlet NSMatrix* buttonSongcastMode;
    IBOutlet NSTextField* textMulticastChannel;
    IBOutlet NSTextField* textLatencyMs;
    IBOutlet NSSlider* sliderLatencyMs;

    Preferences* iPreferences;
    NSArray* iReceiverList;
}

@property (assign) NSButton* buttonOnOff;
@property (assign) NSTextField* textDescription;
@property (assign) NSButton* buttonShowInStatusBar;
@property (assign) NSTableView* tableViewReceiverList;
@property (assign) NSBox* boxGettingStarted;
@property (assign) NSBox* boxMain;
@property (assign) NSTextField* textStep1Text;
@property (assign) NSMatrix* buttonSongcastMode;
@property (assign) NSTextField* textMulticastChannel;
@property (assign) NSTextField* textLatencyMs;
@property (assign) NSSlider* sliderLatencyMs;

- (void) mainViewDidLoad;
- (void) updateButtonOnOff;
- (IBAction) buttonOnOffClicked:(id)aSender;
- (IBAction) buttonRefreshClicked:(id)aSender;
- (IBAction) buttonReconnectClicked:(id)aSender;
- (IBAction) buttonShowInStatusBarClicked:(id)aSender;
- (IBAction) buttonHelpClicked:(id)aSender;
- (IBAction) buttonWizardClicked:(id)aSender;
- (IBAction) buttonSongcastModeClicked:(id)aSender;
- (IBAction) buttonMulticastChannelClicked:(id)aSender;
- (IBAction) sliderLatencyMsChanged:(id)aSender;
- (void) preferenceEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceReceiverListChanged:(NSNotification*)aNotification;

@end


// Declaration of custom table cell
@interface CellReceiver : NSTextFieldCell
{
}

@end



