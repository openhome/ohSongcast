
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

@property (copy) NSString* udn;
@property (copy) NSString* title;
@property (assign) bool selected;
@property (assign) EReceiverState status;

- (id) initWithPref:(PrefReceiver*)aPref uniqueInRoom:(bool)aUnique;

@end


// Main class for the preference pane
@interface SongcastPrefPane : NSPreferencePane 
{
    IBOutlet NSButton* buttonOnOff;
    IBOutlet NSTextField* textAbout;
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
    IBOutlet NSPopUpButton* buttonNetworkAdapter;
    IBOutlet NSButton* buttonAutoUpdates;
    IBOutlet NSButton* buttonBeta;

    Preferences* iPreferences;
    NSArray* iReceiverList;
    NSArray* iSubnetList;
}

@property (assign) NSButton* buttonOnOff;
@property (assign) NSTextField* textAbout;
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
@property (assign) NSPopUpButton* buttonNetworkAdapter;
@property (assign) NSButton* buttonAutoUpdates;
@property (assign) NSButton* buttonBeta;

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
- (IBAction) buttonNetworkAdapterClicked:(id)aSender;
- (IBAction) buttonAutoUpdatesClicked:(id)aSender;
- (IBAction) buttonBetaClicked:(id)aSender;
- (IBAction) buttonCheckForUpdatesClicked:(id)aSender;
- (void) preferenceEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceReceiverListChanged:(NSNotification*)aNotification;
- (void) preferenceSubnetListChanged:(NSNotification*)aNotification;

@end


// Declaration of custom table cell
@interface CellReceiver : NSTextFieldCell
{
}

@end



