
#import <Cocoa/Cocoa.h>
#import "Preferences.h"


@interface Model : NSObject
{
    void* iSoundcard;
    Preferences* iPreferences;
    NSMutableArray* iReceiverList;
}

- (void) start;
- (void) stop;
- (bool) enabled;
- (void) setEnabled:(bool)aValue;

@end
