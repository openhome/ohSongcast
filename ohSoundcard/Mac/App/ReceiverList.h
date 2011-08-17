
#import <Cocoa/Cocoa.h>
#import "Receiver.h"


// Declaration of protocol for receiver list changes
@protocol IReceiverListObserver

- (void) receiverAdded:(Receiver*)aReceiver;
- (void) receiverRemoved:(Receiver*)aReceiver;
- (void) receiverChanged:(Receiver*)aReceiver;

@end



// Declaration of the receiver list class
@interface ReceiverList : NSObject
{
    NSObject* iLock;
    NSMutableArray* iList;
    NSObject<IReceiverListObserver>* iObserver;
}

- (id) initWithReceivers:(NSArray*)aReceivers;
- (NSArray*) receivers;
- (void) addObserver:(NSObject<IReceiverListObserver>*)aObserver;
- (void) removeNonSelected:(NSArray*)aSelected;

@end





