
#import <Cocoa/Cocoa.h>


@interface Model : NSObject
{
    bool iEnabled;
}

- (void)start;
- (void)stop;
- (bool)enabled;
- (void)setEnabled:(bool)aValue;

@end
