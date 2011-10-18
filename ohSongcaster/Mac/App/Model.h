
#import <Cocoa/Cocoa.h>
#import "Preferences.h"
#import "ModelSongcaster.h"


@protocol IModelObserver

- (void) enabledChanged;
- (void) iconVisibleChanged;

@end


@interface Model : NSObject
{
    Preferences* iPreferences;
    id<IModelObserver> iObserver;
    ModelSongcaster* iModelSongcaster;
}

- (void) start;
- (void) stop;
- (void) setObserver:(id<IModelObserver>)aObserver;

- (bool) iconVisible;
- (bool) enabled;
- (void) setEnabled:(bool)aValue;
- (bool) hasRunWizard;
- (void) reconnectReceivers;

- (void) preferenceEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceIconVisibleChanged:(NSNotification*)aNotification;
- (void) preferenceSelectedUdnListChanged:(NSNotification*)aNotification;
- (void) preferenceRefreshReceiverList:(NSNotification*)aNotification;
- (void) preferenceReconnectReceivers:(NSNotification*)aNotification;

@end
