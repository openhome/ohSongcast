
#import <Cocoa/Cocoa.h>
#import "Preferences.h"
#import "ModelSongcaster.h"
#import "AutoUpdate.h"


@protocol IModelObserver

- (void) enabledChanged;
- (void) iconVisibleChanged;
- (void) checkForUpdates;

@end


@interface Model : NSObject
{
    Preferences* iPreferences;
    id<IModelObserver> iObserver;
    ModelSongcaster* iModelSongcaster;
    AutoUpdate* iAutoUpdate;
}

- (void) start;
- (void) stop;
- (void) setObserver:(id<IModelObserver>)aObserver;

- (bool) iconVisible;
- (bool) enabled;
- (void) setEnabled:(bool)aValue;
- (bool) hasRunWizard;
- (bool) autoUpdatesEnabled;
- (AutoUpdate*) autoUpdate;
- (void) reconnectReceivers;

- (void) preferenceEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceIconVisibleChanged:(NSNotification*)aNotification;
- (void) preferenceSelectedUdnListChanged:(NSNotification*)aNotification;
- (void) preferenceRefreshReceiverList:(NSNotification*)aNotification;
- (void) preferenceReconnectReceivers:(NSNotification*)aNotification;
- (void) preferenceMulticastEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceMulticastChannelChanged:(NSNotification*)aNotification;
- (void) preferenceLatencyMsChanged:(NSNotification*)aNotification;
- (void) preferenceSelectedSubnetChanged:(NSNotification*)aNotification;
- (void) preferenceAutoUpdatesEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceBetaUpdatesEnabledChanged:(NSNotification*)aNotification;
- (void) preferenceCheckForUpdates:(NSNotification*)aNotification;

@end
