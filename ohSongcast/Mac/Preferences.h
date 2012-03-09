
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

@property (copy) NSString* udn;
@property (copy) NSString* room;
@property (copy) NSString* group;
@property (copy) NSString* name;
@property (assign) EReceiverState status;

- (id) initWithDict:(NSDictionary*)aDict;
- (NSDictionary*) convertToDict;

@end



@interface PrefSubnet : NSObject
{
    uint32_t address;
    NSString* name;
}

@property (assign) uint32_t address;
@property (copy) NSString* name;

- (id) initWithDict:(NSDictionary*)aDict;
- (NSDictionary*) convertToDict;

@end



@interface PrefMulticastChannel : NSObject
{
}

+ (uint32_t) generate;

@end



@interface Preferences : NSObject
{
    CFStringRef appId;
}

- (id) initWithBundle:(NSBundle*)aBundle;

- (bool) iconVisible;
- (void) setIconVisible:(bool)aVisible;
- (void) addObserverIconVisible:(id)aObserver selector:(SEL)aSelector;

- (bool) enabled;
- (void) setEnabled:(bool)aEnabled;
- (void) addObserverEnabled:(id)aObserver selector:(SEL)aSelector;

- (bool) hasRunWizard;
- (void) setHasRunWizard:(bool)aHasRunWizard;

- (bool) multicastEnabled;
- (void) setMulticastEnabled:(bool)aEnabled;
- (void) addObserverMulticastEnabled:(id)aObserver selector:(SEL)aSelector;

- (uint32_t) multicastChannel;
- (void) setMulticastChannel:(uint32_t)aChannel;
- (void) addObserverMulticastChannel:(id)aObserver selector:(SEL)aSelector;

- (uint32_t) latencyMs;
- (void) setLatencyMs:(uint32_t)aLatencyMs;
- (void) addObserverLatencyMs:(id)aObserver selector:(SEL)aSelector;

- (bool) autoUpdatesEnabled;
- (void) setAutoUpdatesEnabled:(bool)aEnabled;
- (void) addObserverAutoUpdatesEnabled:(id)aObserver selector:(SEL)aSelector;

- (bool) betaUpdatesEnabled;
- (void) setBetaUpdatesEnabled:(bool)aEnabled;
- (void) addObserverBetaUpdatesEnabled:(id)aObserver selector:(SEL)aSelector;

- (NSArray*) receiverList;
- (void) setReceiverList:(NSArray*)aReceiverList;
- (void) addObserverReceiverList:(id)aObserver selector:(SEL)aSelector;

- (NSArray*) selectedUdnList;
- (void) setSelectedUdnList:(NSArray*)aSelectedUdnList;
- (void) addObserverSelectedUdnList:(id)aObserver selector:(SEL)aSelector;

- (NSArray*) subnetList;
- (void) setSubnetList:(NSArray*)aSubnetList;
- (void) addObserverSubnetList:(id)aObserver selector:(SEL)aSelector;

- (PrefSubnet*) selectedSubnet;
- (void) setSelectedSubnet:(PrefSubnet*)aSubnet;
- (void) addObserverSelectedSubnet:(id)aObserver selector:(SEL)aSelector;

- (void) refreshReceiverList;
- (void) addObserverRefreshReceiverList:(id)aObserver selector:(SEL)aSelector;

- (void) reconnectReceivers;
- (void) addObserverReconnectReceivers:(id)aObserver selector:(SEL)aSelector;

- (void) checkForUpdates;
- (void) addObserverCheckForUpdates:(id)aObserver selector:(SEL)aSelector;

- (void) synchronize;

@end



