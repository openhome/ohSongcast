
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
    IBOutlet NSImageView* icon;
    IBOutlet NSButton* buttonOnOff;
    IBOutlet NSButton* buttonAutoplay;
    IBOutlet NSTextField* textDescription;
    IBOutlet NSButton* buttonShowInStatusBar;
    IBOutlet NSButton* buttonHelp;
    IBOutlet NSTableView* tableViewReceiverList;

    Preferences* iPreferences;
    NSArray* iReceiverList;
}

@property (assign) NSImageView* icon;
@property (assign) NSButton* buttonOnOff;
@property (assign) NSButton* buttonAutoplay;
@property (assign) NSTextField* textDescription;
@property (assign) NSButton* buttonShowInStatusBar;
@property (assign) NSButton* buttonHelp;

- (void) mainViewDidLoad;
- (void) updateButtonOnOff;
- (IBAction) buttonOnOffClicked:(id)aSender;
- (IBAction) buttonAutoplayClicked:(id)aSender;
- (IBAction) buttonRefreshClicked:(id)aSender;
- (IBAction) buttonReconnectClicked:(id)aSender;
- (IBAction) buttonShowInStatusBarClicked:(id)aSender;
- (IBAction) buttonHelpClicked:(id)aSender;
- (void) preferenceEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceReceiverListChanged:(NSNotification*)aNotification;

@end


// Declaration of custom table cell
@interface CellReceiver : NSTextFieldCell
{
}

@end



