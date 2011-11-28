
#import "ReceiverList.h"
#import "Receiver.h"
#include "../../Songcast.h"


// Implementation of receiver list class
@implementation ReceiverList


- (id) initWithReceivers:(NSArray*)aReceivers
{
    self = [super init];

    iLock = [[NSObject alloc] init];
    iList = [[NSMutableArray alloc] initWithArray:aReceivers];
    iObserver = nil;
    
    return self;
}


- (NSArray*) receivers
{
    // lock the list and return a copy containing the same objects
    @synchronized(iLock)
    {
        return [NSArray arrayWithArray:iList];
    }
}


- (void) setObserver:(NSObject<IReceiverListObserver>*)aObserver
{
    @synchronized(iLock)
    {
        if (iObserver) {
            [iObserver release];
        }

        iObserver = aObserver;

        if (iObserver) {
            [iObserver retain];
        }
    }
}


- (void) removeUnavailableUnselected:(NSArray*)aSelected
{
    // lock the list and rebuild it containing only receivers that are
    // selected and/or available on the network i.e. remove all
    // receivers that are both unavailable and unselected
    @synchronized(iLock)
    {
        // build the new list
        NSMutableArray* list = [[NSMutableArray alloc] initWithCapacity:0];
        
        for (Receiver* receiver in iList)
        {
            if ([aSelected containsObject:[receiver udn]] || [receiver status] != eReceiverStateOffline)
            {
                [list addObject:receiver];
            }
        }
        
        // replace old list
        [iList release];
        iList = list;
    }
}


- (void) receiverChangedCallback:(THandle)aPtr type:(ECallbackType)aType
{
    // This is called from the ohSongcast receiver manager thread
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn(aPtr)];

    // lock access to the receiver list
    @synchronized(iLock)
    {
        // get the receiver that has changed
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
        switch (aType)
        {
            case eAdded:
                if (receiver)
                {
                    // receiver already in the list - update with the new ptr
                    [receiver updateWithPtr:aPtr];
                }
                else
                {
                    // receiver not in list - create a new one
                    receiver = [[[Receiver alloc] initWithPtr:aPtr] autorelease];
                    [iList addObject:receiver];
                }
                
                // send notification in the main thread
                [iObserver performSelectorOnMainThread:@selector(receiverAdded:) withObject:receiver waitUntilDone:FALSE];
                break;
                
            case eRemoved:
                if (receiver)
                {
                    // clear the ptr for this receiver and send notification in the main thread
                    [receiver updateWithPtr:nil];
                    [iObserver performSelectorOnMainThread:@selector(receiverRemoved:) withObject:receiver waitUntilDone:FALSE];
                }
                break;
                
            case eChanged:
                if (receiver)
                {
                    // update the existing receiver and send notification in the main thread
                    [receiver updateWithPtr:aPtr];
                    [iObserver performSelectorOnMainThread:@selector(receiverChanged:) withObject:receiver waitUntilDone:FALSE];
                }
                break;
        }
    }
}


@end



// Callback from the ohSongcast code for a receiver event
void ReceiverListCallback(void* aPtr, ECallbackType aType, THandle aReceiver)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    ReceiverList* receiverList = (ReceiverList*)aPtr;
    
    [receiverList receiverChangedCallback:aReceiver type:aType];

    [pool drain];
}


