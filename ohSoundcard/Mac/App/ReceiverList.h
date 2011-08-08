
#import <Cocoa/Cocoa.h>
#import "Preferences.h"


// Declaration of a class to hold receiver data
@interface Receiver : NSObject
{
    NSString* udn;
    NSString* room;
    NSString* group;
    NSString* name;
    void* iPtr;
}

@property (assign) NSString* udn;
@property (assign) NSString* room;
@property (assign) NSString* group;
@property (assign) NSString* name;

- (id) initWithPtr:(void*)aPtr;
- (id) initWithPref:(PrefReceiver*)aPref;
- (void) updateWithPtr:(void*)aPtr;
- (PrefReceiver*) convertToPref;

@end



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





