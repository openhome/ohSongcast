
#import <Cocoa/Cocoa.h>
#import "Preferences.h"


// Declaration of the subnet list class
@interface Subnet : NSObject
{
    void* iPtr;
}

- (id) initWithPtr:(void*)aPtr;
- (void) dispose;
- (PrefSubnet*) convertToPref;
- (uint32_t) address;
- (NSString*) name;

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





