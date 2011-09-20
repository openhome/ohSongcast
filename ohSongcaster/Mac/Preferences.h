
#import <Cocoa/Cocoa.h>


// Enum for the receiver state
typedef enum
{
    eReceiverStateOffline,
    eReceiverStateDisconnected,
    eReceiverStateConnecting,
    eReceiverStateConnected
    
} EReceiverState;



@interface PrefReceiver : NSObject
{
    NSString* udn;
    NSString* room;
    NSString* group;
    NSString* name;
    EReceiverState status;
}

@property (assign) NSString* udn;
@property (assign) NSString* room;
@property (assign) NSString* group;
@property (assign) NSString* name;
@property (assign) EReceiverState status;

- (id) initWithDict:(NSDictionary*)aDict;
- (NSDictionary*) convertToDict;

@end



@interface Preferences : NSObject
{
    CFStringRef appId;
}

- (id) initWithBundle:(NSBundle*)aBundle;

- (bool) iconVisible;
- (void) setIconVisible:(bool)aVisible;
- (void) addObserverIconVisible:(id)aObserver selector:(SEL)aSelector;

- (bool) autoplayReceivers;
- (void) setAutoplayReceivers:(bool)aAutoplayReceivers;
- (void) addObserverAutoplayReceivers:(id)aObserver selector:(SEL)aSelector;

- (bool) enabled;
- (void) setEnabled:(bool)aEnabled;
- (void) addObserverEnabled:(id)aObserver selector:(SEL)aSelector;

- (NSArray*) receiverList;
- (void) setReceiverList:(NSArray*)aReceiverList;
- (void) addObserverReceiverList:(id)aObserver selector:(SEL)aSelector;

- (NSArray*) selectedUdnList;
- (void) setSelectedUdnList:(NSArray*)aSelectedUdnList;
- (void) addObserverSelectedUdnList:(id)aObserver selector:(SEL)aSelector;

- (void) refreshReceiverList;
- (void) addObserverRefreshReceiverList:(id)aObserver selector:(SEL)aSelector;

- (void) reconnectReceivers;
- (void) addObserverReconnectReceivers:(id)aObserver selector:(SEL)aSelector;

- (void) synchronize;

@end



