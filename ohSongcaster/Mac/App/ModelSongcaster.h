
#import <Cocoa/Cocoa.h>
#import "ReceiverList.h"
#import "SubnetList.h"


@interface ModelSongcaster : NSObject<IReceiverListObserver, ISubnetListObserver>
{
    void* iSongcaster;
    ReceiverList* iReceivers;
    NSArray* iSelectedUdns;
    SubnetList* iSubnets;

    id iReceiversChangedObject;
    SEL iReceiversChangedSelector;

    id iConfigurationChangedObject;
    SEL iConfigurationChangedSelector;
}

- (id) initWithReceivers:(NSArray*)aReceivers andSelectedUdns:(NSArray*)aSelectedUdns;
- (void) dispose;

- (void) setReceiversChangedObserver:(id)aObserver selector:(SEL)aSelector;
- (void) setConfigurationChangedObserver:(id)aObserver selector:(SEL)aSelector;

- (bool) enabled;
- (void) setEnabled:(bool)aValue;

- (NSArray*) receivers;
- (void) setSelectedUdns:(NSArray*)aSelectedUdns;
- (void) refreshReceivers;

- (void) stopReceiver:(Receiver*)aReceiver;
- (void) stopReceivers;
- (void) playReceiver:(Receiver*)aReceiver;
- (void) playReceivers;

@end


