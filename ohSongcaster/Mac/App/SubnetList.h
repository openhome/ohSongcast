
#import <Cocoa/Cocoa.h>


// Declaration of the subnet list class
@interface Subnet : NSObject
{
    void* iPtr;
}

- (id) initWithPtr:(void*)aPtr;
- (void) dispose;
- (uint32_t) address;

@end


// Declaration of protocol for subnet list changes
@protocol ISubnetListObserver

- (void) subnetAdded:(Subnet*)aSubnet;
- (void) subnetRemoved:(Subnet*)aSubnet;
- (void) subnetChanged:(Subnet*)aSubnet;

@end


// Declaration of the subnet list class
@interface SubnetList : NSObject
{
    NSObject* iLock;
    NSMutableArray* iList;
    NSObject<ISubnetListObserver>* iObserver;
}

- (NSArray*) subnets;
- (void) setObserver:(NSObject<ISubnetListObserver>*)aObserver;

@end





