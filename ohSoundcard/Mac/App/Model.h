
#import <Cocoa/Cocoa.h>
#import "Preferences.h"


@interface Model : NSObject
{
    bool iEnabled;
    void* iSoundcard;
    Preferences* iPreferences;
    NSMutableArray* iReceiverList;
}

- (void) start;
- (void) stop;
- (bool) enabled;
- (void) setEnabled:(bool)aValue;

@end
