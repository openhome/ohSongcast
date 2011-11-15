
#import "AutoUpdate.h"
#import <AppKit/AppKit.h>


@implementation AutoUpdate


- (id) initWithFeedUri:(NSString*)aFeedUri appName:(NSString*)aAppName currentVersion:(NSString*)aCurrentVersion relativeDataPath:(NSString*)aRelativeDataPath
{
    self = [super init];

    iFeedUri = [aFeedUri retain];
    iAppName = [aAppName retain];
    iTarget = [[NSString stringWithUTF8String:"macosx"] retain];
    iCurrentVersion = [aCurrentVersion retain];
    iRelativeDataPath = [aRelativeDataPath retain];
    iCheckForBeta = false;

    return self;
}


- (void) setCheckForBeta:(bool)aCheckForBeta
{
    iCheckForBeta = aCheckForBeta;
}


- (CFDataRef) httpGet:(NSString*)aUri
{
    // create the URI to get
    CFURLRef url = CFURLCreateWithString(NULL, (CFStringRef)aUri, NULL);
    if (url == NULL) {
        return NULL;
    }

    // create the http request
    CFHTTPMessageRef httpReq = CFHTTPMessageCreateRequest(NULL, CFSTR("GET"), url, kCFHTTPVersion1_1);
    CFRelease(url);
    if (httpReq == NULL) {
        return NULL;
    }

    // create the read stream to send the request
    CFReadStreamRef httpStream = CFReadStreamCreateForHTTPRequest(NULL, httpReq);
    CFRelease(httpReq);
    if (httpStream == NULL) {
        return NULL;
    }

    // open the stream - this sends the request
    if (CFReadStreamOpen(httpStream) == FALSE) {
        CFRelease(httpStream);
        return NULL;
    }

    // create a response message
    CFHTTPMessageRef httpResp = CFHTTPMessageCreateEmpty(NULL, FALSE);
    if (httpResp == NULL) {
        CFReadStreamClose(httpStream);
        CFRelease(httpStream);
        return NULL;
    }

    // read the response
    while (1)
    {
        UInt8 buffer[1024];
        CFIndex bytesRead = CFReadStreamRead(httpStream, buffer, 1024);

        if (bytesRead > 0) {
            // has read some data
            if (CFHTTPMessageAppendBytes(httpResp, buffer, bytesRead) == FALSE) {
                CFReadStreamClose(httpStream);
                CFRelease(httpStream);
                CFRelease(httpResp);
                return NULL;
            }
        }
        else if (bytesRead == 0) {
            // EOS
            break;
        }
        else {
            // error
            CFReadStreamClose(httpStream);
            CFRelease(httpStream);
            CFRelease(httpResp);
            return NULL;
        }
    }

    // close the read stream
    CFReadStreamClose(httpStream);
    CFRelease(httpStream);

    // get the response status code
    CFIndex statusCode = CFHTTPMessageGetResponseStatusCode(httpResp);
    if (statusCode != 200) {
        CFRelease(httpResp);
        return NULL;
    }

    // get the response body
    CFDataRef body = CFHTTPMessageCopyBody(httpResp);
    CFRelease(httpResp);
    return body;
}


- (CFXMLTreeRef) findTreeNode:(CFXMLTreeRef)aParent name:(CFStringRef)aName attributeName:(CFStringRef)aAttributeName attributeValue:(CFStringRef)aAttributeValue
{
    // look for element nodes that are immediate children of aParent that are <aName aAttributeName="aAttributeValue">
    int childCount = CFTreeGetChildCount(aParent);

    for (int i=0 ; i<childCount ; i++)
    {
        CFXMLTreeRef child = CFTreeGetChildAtIndex(aParent, i);
        CFXMLNodeRef xmlNode = CFXMLTreeGetNode(child);
        CFXMLNodeTypeCode type = CFXMLNodeGetTypeCode(xmlNode);

        if (type == kCFXMLNodeTypeElement)
        {
            CFStringRef text = CFXMLNodeGetString(xmlNode);
            if (CFStringCompare(text, aName, 0) == kCFCompareEqualTo)
            {
                if (aAttributeName == NULL) {
                    return child;
                }

                CFXMLElementInfo* info = (CFXMLElementInfo*)CFXMLNodeGetInfoPtr(xmlNode);
                CFStringRef attrValue = (CFStringRef)CFDictionaryGetValue(info->attributes, aAttributeName);

                if (attrValue != NULL && CFStringCompare(attrValue, aAttributeValue, 0) == kCFCompareEqualTo)
                {
                    return child;
                }
            }
        }
    }

    return NULL;
}


- (CFXMLTreeRef) findTreeNode:(CFXMLTreeRef)aParent name:(CFStringRef)aName
{
    return [self findTreeNode:aParent name:aName attributeName:NULL attributeValue:NULL];
}


- (AutoUpdateInfo*) getAutoUpdateInfo:(CFXMLTreeRef)aNode type:(CFStringRef)aType target:(CFStringRef)aTarget
{
    // look for the type node
    CFXMLTreeRef typeNode = [self findTreeNode:aNode name:aType];
    if (typeNode == NULL) {
        return NULL;
    }

    // get the url node
    CFXMLTreeRef urlNode = [self findTreeNode:typeNode name:CFSTR("url") attributeName:CFSTR("target") attributeValue:aTarget];
    if (urlNode == NULL) {
        return NULL;
    }

    // get the version
    CFXMLNodeRef xmlNode = CFXMLTreeGetNode(typeNode);
    CFXMLElementInfo* nodeInfo = (CFXMLElementInfo*)CFXMLNodeGetInfoPtr(xmlNode);
    CFStringRef version = (CFStringRef)CFDictionaryGetValue(nodeInfo->attributes, CFSTR("version"));
    if (version == NULL) {
        return NULL;
    }

    // get the url text node
    CFStringRef url = NULL;
    int childCount = CFTreeGetChildCount(urlNode);

    for (int i=0 ; i<childCount ; i++)
    {
        CFXMLTreeRef child = CFTreeGetChildAtIndex(urlNode, i);
        CFXMLNodeRef xmlNode = CFXMLTreeGetNode(child);
        CFXMLNodeTypeCode type = CFXMLNodeGetTypeCode(xmlNode);

        if (type == kCFXMLNodeTypeText)
        {
            url = CFXMLNodeGetString(xmlNode);
            break;
        }
    }

    if (url == NULL) {
        return NULL;
    }

    // create the info object
    AutoUpdateInfo* info = [[[AutoUpdateInfo alloc] init] autorelease];
    //[info setVersion:(NSString*)version];
    //[info setUri:(NSString*)url];
    [info setVersion:[NSString stringWithString:(NSString*)version]];
    [info setUri:[NSString stringWithString:(NSString*)url]];
    return info;
}


- (AutoUpdateInfo*) checkForUpdates
{
    // get the XML description for the auto updates
    CFDataRef body = [self httpGet:iFeedUri];
    if (body == NULL) {
        return NULL;
    }

    // create the root XML tree node
    CFXMLTreeRef rootTreeNode = CFXMLTreeCreateFromData(NULL, body, NULL, kCFXMLParserSkipWhitespace, kCFXMLNodeCurrentVersion);
    if (rootTreeNode == NULL) {
        return NULL;
    }

    // look for the top level <autoupdate> node
    CFXMLTreeRef autoUpdateTreeNode = [self findTreeNode:rootTreeNode name:CFSTR("autoupdate")];
    if (autoUpdateTreeNode == NULL) {
        CFRelease(rootTreeNode);
        return NULL;
    }

    // look for the <updateinfo> node
    CFXMLTreeRef updateInfoTreeNode = [self findTreeNode:autoUpdateTreeNode name:CFSTR("updateinfo")];
    if (updateInfoTreeNode == NULL) {
        CFRelease(rootTreeNode);
        return NULL;
    }

    // look for the <application name="appName"> node
    CFXMLTreeRef appTreeNode = [self findTreeNode:updateInfoTreeNode name:CFSTR("application") attributeName:CFSTR("name") attributeValue:(CFStringRef)iAppName];
    if (appTreeNode == NULL) {
        CFRelease(rootTreeNode);
        return NULL;
    }

    // look for a stable release
    AutoUpdateInfo* info = [self getAutoUpdateInfo:appTreeNode type:CFSTR("stable") target:(CFStringRef)iTarget];

    if (iCheckForBeta)
    {
        // look for an available beta release
        AutoUpdateInfo* betaInfo = [self getAutoUpdateInfo:appTreeNode type:CFSTR("beta") target:(CFStringRef)iTarget];

        if (betaInfo != NULL && info == NULL) {
            // beta available but no stable - just use the beta
            info = betaInfo;
        }
        else if (betaInfo != NULL) {
            // beta and stable available - choose highest version
            if ([[info version] compare:[betaInfo version]] == NSOrderedAscending) {
                info = betaInfo;
            }
        }
    }

    if (info != NULL)
    {
        if ([iCurrentVersion compare:[info version]] != NSOrderedAscending) {
            // no new version available
            info = NULL;
        }
    }

    CFRelease(rootTreeNode);
    return info;
}


- (bool) installUpdate:(AutoUpdateInfo*)aInfo
{
    // get the standard library path for the current user i.e. "~/Library"
    NSArray* libraryFolderUrls = [[NSFileManager defaultManager] URLsForDirectory:NSLibraryDirectory inDomains:NSUserDomainMask];
    if ([libraryFolderUrls count] == 0) {
        return false;
    }

    // create necessary folders for the update
    NSURL* dataFolderUrl = [[libraryFolderUrls objectAtIndex:0] URLByAppendingPathComponent:iRelativeDataPath];
    NSURL* updatesFolderUrl = [dataFolderUrl URLByAppendingPathComponent:@"Updates"];
    NSURL* mountFolderUrl = [updatesFolderUrl URLByAppendingPathComponent:@"MountPoint"];
    // succeeds if the directory already exists
    if ([[NSFileManager defaultManager] createDirectoryAtPath:[mountFolderUrl path] withIntermediateDirectories:YES attributes:nil error:nil] == FALSE) {
        return false;
    }

    // download the installer
    CFDataRef installer = [self httpGet:[aInfo uri]];
    if (installer == NULL) {
        return false;
    }

    // write the dmg file
    NSURL* dmgFileUrl = [updatesFolderUrl URLByAppendingPathComponent:@"Update.dmg"];
    // succeeds if the file already exists
    if ([[NSFileManager defaultManager] createFileAtPath:[dmgFileUrl path] contents:(NSData*)installer attributes:nil] == FALSE) {
        CFRelease(installer);
        return false;
    }

    // don't need the data anymore
    CFRelease(installer);

    // mount the dmg file
    NSTask* mountTask = [NSTask launchedTaskWithLaunchPath:@"/usr/bin/hdiutil" arguments:[NSArray arrayWithObjects:@"attach", [dmgFileUrl path], @"-mountpoint", [mountFolderUrl path], nil]];
    if (mountTask == NULL) {
        return false;
    }
    [mountTask waitUntilExit];

    // copy the installer
    NSURL* srcFileUrl = [mountFolderUrl URLByAppendingPathComponent:@"Installer.pkg"];
    NSURL* dstFileUrl = [updatesFolderUrl URLByAppendingPathComponent:@"Installer.pkg"];
    // delete the dst file first since the copy will fail if it already exists
    [[NSFileManager defaultManager] removeItemAtURL:dstFileUrl error:nil];
    bool installerValid = [[NSFileManager defaultManager] copyItemAtURL:srcFileUrl toURL:dstFileUrl error:nil];

    // unmount the dmg file
    NSTask* unmountTask = [NSTask launchedTaskWithLaunchPath:@"/usr/bin/hdiutil" arguments:[NSArray arrayWithObjects:@"detach", [mountFolderUrl path], nil]];
    if (unmountTask != NULL) {
        [unmountTask waitUntilExit];
    }

    // run the installer
    return (installerValid ? [[NSWorkspace sharedWorkspace] openFile:[dstFileUrl path]] : false);
}


@end


@implementation AutoUpdateInfo

@synthesize version;
@synthesize uri;

@end


