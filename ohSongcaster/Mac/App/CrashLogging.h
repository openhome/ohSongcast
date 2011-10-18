
#import <Foundation/Foundation.h>


// Declaration of the crash log

@interface CrashLog : NSObject
{
    NSString* iLog;
}

- (id) initWithLog:(NSString*)aLog;
- (NSString*) string;

@end


// Declaration of interface for dumping crash logs

@protocol ICrashLogDumper

- (void) dump:(CrashLog*)aCrashLog;

@end


// Declaration of the crash monitor that detects crashes

@interface CrashMonitor : NSObject
{
    NSMutableArray* iDumpers;
}

- (void) addDumper:(NSObject<ICrashLogDumper>*)aDumper;
- (void) start;

@end


// Declaration of crash log dumper to display a dialog and send the crash report

@interface CrashLogDumperReport : NSObject<ICrashLogDumper>
{
    NSString* iProductId;
    NSString* iUri;
}

- (id) initWithProductId:(NSString*)aProductId uri:(NSString*)aUri;
- (void) dump:(CrashLog*)aCrashLog;

@end


// Declaration of the http crash reporter

@interface CrashReporterHttp : NSObject

+ (bool) send:(NSString*)aReport productId:(NSString*)aProductId name:(NSString*)aName email:(NSString*)aEmail phone:(NSString*)aPhone comment:(NSString*)aComment url:(NSString*)aUrl;

@end



