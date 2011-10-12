
#import "SubnetList.h"
#include "../../Songcaster.h"



// Implementation of subnet class
@implementation Subnet


- (id) initWithPtr:(void*)aPtr
{
    self = [super init];

    iPtr = aPtr;
    SubnetAddRef(iPtr);

    return self;
}


- (void) dispose
{
    SubnetRemoveRef(iPtr);
    iPtr = 0;
}


- (uint32_t) address
{
    return SubnetAddress(iPtr);
}


- (bool) matches:(void*)aPtr
{
    return (iPtr == aPtr);
}


@end



// Implementation of subnet list class
@implementation SubnetList


- (id) init
{
    self = [super init];

    iLock = [[NSObject alloc] init];
    iList = [[NSMutableArray alloc] initWithCapacity:0];
    iObserver = nil;
    
    return self;
}


- (NSArray*) subnets
{
    // lock the list and return a copy containing the same objects
    @synchronized(iLock)
    {
        return [NSArray arrayWithArray:iList];
    }
}


- (void) setObserver:(NSObject<ISubnetListObserver>*)aObserver
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


- (void) subnetChangedCallback:(THandle)aPtr type:(ECallbackType)aType
{
    // lock access to the subnet list
    @synchronized(iLock)
    {
        // get the subnet that has changed
        Subnet* subnet = nil;
        for (Subnet* s in iList)
        {
            if ([s matches:aPtr])
            {
                subnet = s;
                break;
            }
        }

        // handle different callback types
        switch (aType)
        {
            case eAdded:
                if (!subnet)
                {
                    subnet = [[[Subnet alloc] initWithPtr:aPtr] autorelease];
                    [iList addObject:subnet];
                }

                // send notification in the main thread
                [iObserver performSelectorOnMainThread:@selector(subnetAdded:) withObject:subnet waitUntilDone:FALSE];
                break;
                
            case eRemoved:
                if (subnet)
                {
                    [iList removeObject:subnet];
                    [subnet dispose];
                    [subnet autorelease];

                    // send notification in the main thread
                    [iObserver performSelectorOnMainThread:@selector(subnetRemoved:) withObject:subnet waitUntilDone:FALSE];
                }
                break;
                
            case eChanged:
                if (subnet)
                {
                    // send notification in the main thread
                    [iObserver performSelectorOnMainThread:@selector(subnetChanged:) withObject:subnet waitUntilDone:FALSE];
                }
                break;
        }
    }
}


@end


// Callback from the ohSongcaster code for a subnet event
void SubnetListCallback(void* aPtr, ECallbackType aType, THandle aSubnet)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    SubnetList* subnetList = (SubnetList*)aPtr;
    
    [subnetList subnetChangedCallback:aSubnet type:aType];

    [pool drain];
}


