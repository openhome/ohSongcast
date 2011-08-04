
#import <Cocoa/Cocoa.h>


@interface PrefReceiver : NSObject
{
    NSString* udn;
    NSString* name;
    bool available;
}

@property (assign) NSString* udn;
@property (assign) NSString* name;
@property (assign) bool available;

- (id) initWithDict:(NSDictionary*)aDict;
- (NSDictionary*) convertToDict;

@end



@interface Preferences : NSObject
{
    CFStringRef appId;
}

- (NSArray*) receiverList;
- (void) setReceiverList:(NSArray*)aReceiverList;
- (void) synchronize;

@end



