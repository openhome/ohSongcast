
#import "CrashLogging.h"
#import <AppKit/AppKit.h>


// Implementation of the crash monitor

@implementation CrashMonitor


- (id) init
{
    self = [super init];

    iDumpers = [[NSMutableArray alloc] initWithCapacity:0];

    return self;
}


- (void) addDumper:(NSObject<ICrashLogDumper>*)aDumper
{
    [iDumpers addObject:aDumper];
}


- (void) start
{
    // get the standard path "~/Library"
    NSArray* libraryFolderUrls = [[NSFileManager defaultManager] URLsForDirectory:NSLibraryDirectory inDomains:NSUserDomainMask];

    if ([libraryFolderUrls count] == 0)
    {
        // the "~/Library" folder cannot be found
        return;
    }

    // create the folder URL where the crash logs are stored and get all crash files
    NSURL* crashLogFolderUrl = [[libraryFolderUrls objectAtIndex:0] URLByAppendingPathComponent:@"Logs/CrashReporter"];
    NSArray* crashLogFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[crashLogFolderUrl path] error:nil];

    if (crashLogFiles == nil || [crashLogFiles count] == 0)
    {
        // no crash log files exist
        return;
    }

    // find the most recent crash file - relevant crash log files are appName_YYYY-MM-DD-HHMMSS_macName.crash
    NSString* appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleExecutable"];

    NSString* mostRecentDate = nil;
    NSString* mostRecentFile = nil;

    for (NSString* f in crashLogFiles)
    {
        if ([f hasPrefix:appName] && [f hasSuffix:@".crash"])
        {
            NSString* dateTimeStr = [f substringWithRange:NSMakeRange([appName length]+1, 17)];

            if (mostRecentDate == nil || [dateTimeStr compare:mostRecentDate] == NSOrderedDescending)
            {
                mostRecentDate = dateTimeStr;
                mostRecentFile = f;
            }
        }
    }

    if (mostRecentDate == nil)
    {
        // no crash log files exist for this application
        return;
    }

    // get the last crash date from the preferences
    CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("LastCrashDate"), kCFPreferencesCurrentApplication);
    NSString* lastCrashDate = nil;
    if (pref)
    {
        if (CFGetTypeID(pref) == CFStringGetTypeID())
        {
            lastCrashDate = (NSString*)((CFStringRef)pref);
        }
        CFRelease(pref);
    }

    if (lastCrashDate != nil && [mostRecentDate compare:lastCrashDate] != NSOrderedDescending)
    {
        // there are no new crash files since last crash
        return;
    }

    // get contents of crash log file
    NSString* crashLogFile = [[crashLogFolderUrl URLByAppendingPathComponent:mostRecentFile] path];
    NSString* crashLogContents = [NSString stringWithContentsOfFile:crashLogFile encoding:NSUTF8StringEncoding error:nil];

    // create the crash log
    CrashLog* log = [[CrashLog alloc] initWithLog:crashLogContents];

    // dump to all dumpers
    for (NSObject<ICrashLogDumper>* d in iDumpers)
    {
        [d dump:log];
    }

    // free up the log
    [log release];

    // set the last crash date in the preferences
    CFPreferencesSetAppValue(CFSTR("LastCrashDate"), mostRecentDate, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}


@end



// Implementation of the crash log

@implementation CrashLog


- (id) initWithLog:(NSString*)aLog
{
    self = [super init];

    iLog = [aLog retain];

    return self;
}


- (void) dealloc
{
    [iLog release];
    [super dealloc];
}


- (NSString*) string
{
    return iLog;
}


@end



// Implementation of the crash log dumper to show a dialog and send the report

@implementation CrashLogDumperReport


- (id) initWithProductId:(NSString*)aProductId uri:(NSString*)aUri
{
    self = [super init];

    iProductId = [aProductId retain];
    iUri = [aUri retain];

    return self;
}


- (void) dealloc
{
    [iProductId release];
    [iUri release];
    [super dealloc];
}


- (void) dump:(CrashLog*)aCrashLog
{
    NSString* bundleName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
    NSString* manufacturerName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcasterManufacturerName"];

    NSAlert* alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:[NSString stringWithFormat:NSLocalizedString(@"CrashLogReport", @""), manufacturerName]];
    [alert addButtonWithTitle:NSLocalizedString(@"CrashLogIgnore", @"")];
    [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"CrashLogTitle", @""), bundleName]];
    [alert setInformativeText:[NSString stringWithFormat:NSLocalizedString(@"CrashLogText", @""), manufacturerName]];
    [alert setAlertStyle:NSCriticalAlertStyle];

    if ([alert runModal] == NSAlertFirstButtonReturn)
    {
        [CrashReporterHttp send:[aCrashLog string] productId:iProductId name:@"" email:@"" phone:@"" comment:@"" url:iUri];
    }

    [alert release];
}


@end



// Implementation of the http crash reporter

@implementation CrashReporterHttp


+ (NSString*) appendValue:(NSString*)aValue forKey:(NSString*)aKey toPostData:(NSString*)aPostData withHeader:(NSString*)aHeader
{
    NSString* postData = [aPostData stringByAppendingString:aHeader];
    postData = [postData stringByAppendingString:@"\nContent-Disposition: form-data; name=\""];
    postData = [postData stringByAppendingString:aKey];
    postData = [postData stringByAppendingString:@"\"\n\n"];
    postData = [postData stringByAppendingString:aValue];
    postData = [postData stringByAppendingString:@"\n"];
    return postData;
}


+ (bool) send:(NSString*)aReport productId:(NSString*)aProductId name:(NSString*)aName email:(NSString*)aEmail phone:(NSString*)aPhone comment:(NSString*)aComment url:(NSString*)aUrl
{
    // create the post data to send
    CFUUIDRef uuid = CFUUIDCreate(nil);
    NSString* boundary = (NSString*)CFUUIDCreateString(nil, uuid);
    CFRelease(uuid);
    [boundary autorelease];

    // create the post data for the report
    NSString* header = [[NSString stringWithString:@"--"] stringByAppendingString:boundary];
    NSString* footer = [header stringByAppendingString:@"--"];

    NSString* postData = [NSString string];

    // product id
    postData = [CrashReporterHttp appendValue:aProductId forKey:@"prodId" toPostData:postData withHeader:header];

    // name
    if ([aName length] != 0) {
        postData = [CrashReporterHttp appendValue:aName forKey:@"name" toPostData:postData withHeader:header];
    }

    // email
    if ([aEmail length] != 0) {
        postData = [CrashReporterHttp appendValue:aEmail forKey:@"email" toPostData:postData withHeader:header];
    }

    // phone
    if ([aPhone length] != 0) {
        postData = [CrashReporterHttp appendValue:aPhone forKey:@"phone" toPostData:postData withHeader:header];
    }

    // comment
    if ([aComment length] != 0) {
        postData = [CrashReporterHttp appendValue:aComment forKey:@"comment" toPostData:postData withHeader:header];
    }

    // report
    postData = [CrashReporterHttp appendValue:aReport forKey:@"data" toPostData:postData withHeader:header];

    // footer
    postData = [postData stringByAppendingString:@"\n"];
    postData = [postData stringByAppendingString:footer];


    // create the http URL
    CFURLRef url = CFURLCreateWithString(NULL, (CFStringRef)aUrl, NULL);
    if (url == NULL) {
        return false;
    }

    // create the http request
    CFHTTPMessageRef httpReq = CFHTTPMessageCreateRequest(NULL, CFSTR("POST"), url, kCFHTTPVersion1_1);
    CFRelease(url);
    if (httpReq == NULL) {
        return false;
    }

    // add headers
    NSString* contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@", boundary];
    CFHTTPMessageSetHeaderFieldValue(httpReq, CFSTR("Content-Type"), (CFStringRef)contentType);

    // set the body
    CFDataRef body = CFStringCreateExternalRepresentation(NULL, (CFStringRef)postData, kCFStringEncodingUTF8, 0);
    if (body == NULL) {
        CFRelease(httpReq);
        return false;
    }
    CFHTTPMessageSetBody(httpReq, body);
    CFRelease(body);


    // create the read stream to send the request
    CFReadStreamRef httpStream = CFReadStreamCreateForHTTPRequest(NULL, httpReq);
    CFRelease(httpReq);
    if (httpStream == NULL) {
        return false;
    }
 
    // open the stream - this sends the request
    if (CFReadStreamOpen(httpStream) == FALSE) {
        CFRelease(httpStream);
        return false;
    }

    // wait for response
    if (CFReadStreamRead(httpStream, 0, 0) < 0) {
        CFReadStreamClose(httpStream);
        CFRelease(httpStream);
        return false;
    }

    // get the response status code
    CFHTTPMessageRef httpResp = (CFHTTPMessageRef)CFReadStreamCopyProperty(httpStream, kCFStreamPropertyHTTPResponseHeader);
    UInt32 statusCode = CFHTTPMessageGetResponseStatusCode(httpResp);
    CFRelease(httpResp);

    // close the stream
    CFReadStreamClose(httpStream);
    CFRelease(httpStream);

    // return success or failure
    return (statusCode == 200);
}


@end




