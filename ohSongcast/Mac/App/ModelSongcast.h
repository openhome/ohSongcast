
#import <Cocoa/Cocoa.h>
#import "ReceiverList.h"
#import "SubnetList.h"


@interface ModelSongcast : NSObject<IReceiverListObserver, ISubnetListObserver>
{
    void* iSongcast;
    ReceiverList* iReceivers;
    NSArray* iSelectedUdns;
    SubnetList* iSubnets;

    id iReceiversChangedObject;
    SEL iReceiversChangedSelector;

    id iSubnetsChangedObject;
    SEL iSubnetsChangedSelector;

    id iConfigurationChangedObject;
    SEL iConfigurationChangedSelector;
}

- (id) initWithReceivers:(NSArray*)aReceivers andSelectedUdns:(NSArray*)aSelectedUdns multicastEnabled:(bool)aMulticastEnabled multicastChannel:(uint32_t)aMulticastChannel latencyMs:(uint32_t)aLatencyMs;
- (void) dispose;

- (void) setReceiversChangedObserver:(id)aObserver selector:(SEL)aSelector;
- (void) setSubnetsChangedObserver:(id)aObserver selector:(SEL)aSelector;
- (void) setConfigurationChangedObserver:(id)aObserver selector:(SEL)aSelector;

- (bool) enabled;
- (void) setEnabled:(bool)aValue;
- (void) setMulticastEnabled:(bool)aValue;
- (void) setMulticastChannel:(uint32_t)aValue;
- (void) setLatencyMs:(uint32_t)aValue;

- (NSArray*) receivers;
- (void) setSelectedUdns:(NSArray*)aSelectedUdns;
- (void) refreshReceivers;

- (void) stopReceiver:(Receiver*)aReceiver;
- (void) stopReceivers;
- (void) playReceiver:(Receiver*)aReceiver;
- (void) playReceivers;

- (NSArray*) subnets;
- (uint32_t) subnet;
- (void) setSubnet:(uint32_t)aAddress;

@end


