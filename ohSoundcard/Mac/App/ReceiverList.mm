
#import "ReceiverList.h"
#import "Receiver.h"
#include "../../Soundcard.h"


// Definition for a class used to pass arguments across
// the soundcard-main thread boundary
@interface CallbackArg : NSObject
{
    void* ptr;
    ECallbackType callbackType;
}

@property (readonly) void* ptr;
@property (readonly) ECallbackType callbackType;

- (id) initWithPtr:(void*)aPtr callbackType:(ECallbackType)aCallbackType;

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


- (void) receiverCallback:(CallbackArg*)aArg
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn([aArg ptr])];    
    Receiver* receiver = nil;
    for (Receiver* r in iList)
    {
        if ([[r udn] compare:udn] == NSOrderedSame)
        {
            receiver = r;
            break;
        }
    }
    
    // handle different callback types
    switch ([aArg callbackType])
    {
        case eAdded:
            if (receiver)
            {
                // receiver already in the list - update with the new ptr
                [receiver updateWithPtr:[aArg ptr]];
            }
            else
            {
                // receiver not in list - create a new one
                receiver = [[[Receiver alloc] initWithPtr:[aArg ptr]] autorelease];
                [iList addObject:receiver];
            }

            // send notification
            [iObserver receiverAdded:receiver];
            break;

        case eRemoved:
            if (receiver)
            {
                // clear the ptr for this receiver and send notification
                [receiver updateWithPtr:nil];
                [iObserver receiverRemoved:receiver];
            }
            break;

        case eChanged:
            if (receiver)
            {
                // update the existing receiver and send notification
                [receiver updateWithPtr:[aArg ptr]];
                [iObserver receiverChanged:receiver];
            }
            break;
    }
        
    // the ref count of the passed in arg ptr can now be decremented - this is to match
    // the add ref call in the C-style callback method that is called which dispatches
    // this method call to the main thread (ReceiverListCallback, below)
    ReceiverRemoveRef([aArg ptr]);
}


@end



// Implementation of the callback arg class
@implementation CallbackArg

@synthesize ptr;
@synthesize callbackType;

- (id) initWithPtr:(void*)aPtr callbackType:(ECallbackType)aCallbackType
{
    self = [super init];
    ptr = aPtr;
    callbackType = aCallbackType;
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

    // Post to the main thread
    [receiverList performSelectorOnMainThread:@selector(receiverCallback:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver callbackType:aType] autorelease] waitUntilDone:FALSE];
    
    [pool drain];
}


