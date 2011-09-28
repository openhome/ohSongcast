
#import <Cocoa/Cocoa.h>
#import <SystemConfiguration/SCNetworkReachability.h>
#import "Model.h"


@interface SongcasterAppDelegate : NSObject <NSApplicationDelegate, IModelObserver>
{
    IBOutlet NSMenu* menu;
    IBOutlet NSMenuItem* menuItemStatus;
    IBOutlet NSMenuItem* menuItemOnOff;
    IBOutlet NSMenuItem* menuItemReconnect;
    IBOutlet NSMenuItem* menuItemPrefs;

    NSStatusItem* statusItem;
    Model* model;
    bool iSessionResigned;
    bool iSleeping;
    SCNetworkReachabilityRef iReachability;
}

@property (assign) IBOutlet NSMenu* menu;
@property (assign) IBOutlet NSMenuItem* menuItemStatus;
@property (assign) IBOutlet NSMenuItem* menuItemOnOff;
@property (assign) IBOutlet NSMenuItem* menuItemReconnect;
@property (assign) IBOutlet NSMenuItem* menuItemPrefs;

- (IBAction)menuItemOnOffClicked:(id)aSender;
- (IBAction)menuItemReconnectClicked:(id)aSender;
- (IBAction)menuItemPrefsClicked:(id)aSender;

@end
