
#import <Cocoa/Cocoa.h>
#import "Preferences.h"
#import "ReceiverList.h"


@interface Model : NSObject<IReceiverListObserver>
{
    void* iSoundcard;
    Preferences* iPreferences;
    ReceiverList* iReceiverList;
}

- (void) start;
- (void) stop;
- (bool) enabled;
- (void) setEnabled:(bool)aValue;

@end
