
#import <Cocoa/Cocoa.h>


// Declaration of protocol for receiver list changes
@protocol IReceiverListObserver

- (void) receiverListChanged;

@end



// Declaration of the receiver list class
@interface ReceiverList : NSObject
{
    NSMutableArray* iList;
    id<IReceiverListObserver> iObserver;
}

- (id) initWithReceivers:(NSArray*)aReceivers;
- (NSArray*) receivers;
- (void) addObserver:(id<IReceiverListObserver>)aObserver;

@end





