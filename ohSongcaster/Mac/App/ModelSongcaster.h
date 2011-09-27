
#import <Cocoa/Cocoa.h>
#import "ReceiverList.h"


@interface ModelSongcaster : NSObject<IReceiverListObserver>
{
    void* iSongcaster;
    ReceiverList* iReceivers;
    NSArray* iSelectedUdns;

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
- (void) playReceiver:(Receiver*)aReceiver andReconnect:(bool)aReconnect;
- (void) playReceiversAndReconnect:(bool)aReconnect;

@end


