
#import <Cocoa/Cocoa.h>
#import "Model.h"


@interface ohSoundcardAppDelegate : NSObject <NSApplicationDelegate>
{
    IBOutlet NSMenu* menu;
    IBOutlet NSMenuItem* menuItemStatus;
    IBOutlet NSMenuItem* menuItemOnOff;
    IBOutlet NSMenuItem* menuItemPrefs;

    NSStatusItem* statusItem;
    Model* model;
}

@property (assign) IBOutlet NSMenu* menu;
@property (assign) IBOutlet NSMenuItem* menuItemStatus;
@property (assign) IBOutlet NSMenuItem* menuItemOnOff;
@property (assign) IBOutlet NSMenuItem* menuItemPrefs;

- (IBAction)menuItemOnOffClicked:(id)aSender;
- (IBAction)menuItemPrefsClicked:(id)aSender;
- (void)setMenuStatus:(bool)aOn;

@end
