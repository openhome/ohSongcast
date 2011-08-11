
#import "ReceiverList.h"
#import "Receiver.h"
#include "../../Soundcard.h"


// Definition for a class used to pass arguments across
// the soundcard-main thread boundary
@interface CallbackArg : NSObject
{
    void* ptr;
}

@property (readonly) void* ptr;

- (id) initWithPtr:(void*)aPtr;

@end



// Implementation of receiver list class
@implementation ReceiverList


- (id) initWithReceivers:(NSArray*)aReceivers
{
    self = [super init];
    
    iList = [[NSMutableArray alloc] initWithArray:aReceivers];
    iObserver = nil;
    
    return self;
}


- (NSArray*) receivers
{
    return iList;
}


- (void) addObserver:(id<IReceiverListObserver>)aObserver
{
    iObserver = aObserver;
}


- (Receiver*) receiverWithUdn:(NSString*)aUdn
{
    for (Receiver* receiver in iList)
    {
        if ([[receiver udn] compare:aUdn] == NSOrderedSame)
        {
            return receiver;
        }
    }
    
    return nil;
}


- (void) receiverAdded:(CallbackArg*)aArg
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn([aArg ptr])];
    
    Receiver* receiver = [self receiverWithUdn:udn];
    
    // update existing receivers and add new ones
    if (receiver)
    {
        [receiver updateWithPtr:[aArg ptr]];
    }
    else
    {
        receiver = [[[Receiver alloc] initWithPtr:[aArg ptr]] autorelease];
        [iList addObject:receiver];
    }
    
    // notify of changes
    [iObserver receiverListChanged];
    
    // the ref count of the passed in arg can now be decremented - this is to match
    // the add ref call in the C-style callback method that is called which dispatches
    // this method call to the main thread (ReceiverListCallback, below)
    ReceiverRemoveRef([aArg ptr]);
}


- (void) receiverRemoved:(CallbackArg*)aArg
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn([aArg ptr])];
    
    Receiver* receiver = [self receiverWithUdn:udn];
    
    if (receiver)
    {
        // receiver is in the list - clear the pointer
        [receiver updateWithPtr:nil];
        
        // notify of changes
        [iObserver receiverListChanged];
    }
    
    // the ref count of the passed in arg can now be decremented - this is to match
    // the add ref call in the C-style callback method that is called which dispatches
    // this method call to the main thread (ReceiverListCallback, below)
    ReceiverRemoveRef([aArg ptr]);
}


- (void) receiverChanged:(CallbackArg*)aArg
{
    [self receiverAdded:aArg];
}


@end



// Implementation of the callback arg class
@implementation CallbackArg

@synthesize ptr;

- (id) initWithPtr:(void*)aPtr
{
    self = [super init];
    ptr = aPtr;
    return self;
}

@end



// Callback from the ohSoundcard code for a receiver event
void ReceiverListCallback(void* aPtr, ECallbackType aType, THandle aReceiver)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    ReceiverList* receiverList = (ReceiverList*)aPtr;

    // The receiver is passed asynchronously to the main thread - add a ref that gets decremented
    // in the main thread functions
    ReceiverAddRef(aReceiver);

    switch (aType)
    {
        case eAdded:
            [receiverList performSelectorOnMainThread:@selector(receiverAdded:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver] autorelease] waitUntilDone:FALSE];
            break;
        case eRemoved:
            [receiverList performSelectorOnMainThread:@selector(receiverRemoved:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver] autorelease] waitUntilDone:FALSE];
            break;
        case eChanged:
            [receiverList performSelectorOnMainThread:@selector(receiverChanged:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver] autorelease] waitUntilDone:FALSE];
            break;
    }
    
    [pool drain];
}


