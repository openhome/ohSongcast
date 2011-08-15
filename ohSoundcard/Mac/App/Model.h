
#import <Cocoa/Cocoa.h>
#import "Preferences.h"
#import "ReceiverList.h"


@protocol IModelObserver

- (void) enabledChanged;
- (void) iconVisibleChanged;

@end


@interface Model : NSObject<IReceiverListObserver>
{
    void* iSoundcard;
    Preferences* iPreferences;
    bool iEnabled;
    bool iAutoplay;
    ReceiverList* iReceiverList;
    NSArray* iSelectedUdnList;
    id<IModelObserver> iObserver;
}

- (void) start;
- (void) stop;
- (void) setObserver:(id<IModelObserver>)aObserver;

- (bool) iconVisible;
- (bool) enabled;
- (void) setEnabled:(bool)aValue;
- (void) reconnectReceivers;

- (void) preferenceEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceIconVisibleChanged:(NSNotification*)aNotification;
- (void) preferenceSelectedUdnListChanged:(NSNotification*)aNotification;
- (void) preferenceAutoplayReceiversChanged:(NSNotification*)aNotification;
- (void) preferenceRefreshReceiverList:(NSNotification*)aNotification;
- (void) preferenceReconnectReceivers:(NSNotification*)aNotification;

@end
