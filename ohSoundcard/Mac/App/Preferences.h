
#import <Cocoa/Cocoa.h>


@interface PrefReceiver : NSObject
{
    NSString* udn;
    NSString* room;
    NSString* group;
    NSString* name;
    bool available;
}

@property (assign) NSString* udn;
@property (assign) NSString* room;
@property (assign) NSString* group;
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



