#ifndef HEADER_TYPES
#define HEADER_TYPES

#include <OpenHome/OsTypes.h>

#define DllExport
#define DllExportClass
#define STDCALL __stdcall

namespace OpenHome {

typedef char TChar;
typedef bool TBool;
typedef void TAny;

typedef signed char TInt8;
typedef short TInt16;
typedef int TInt32;
typedef long long TInt64;

typedef unsigned char TByte;
typedef unsigned char TUint8;
typedef unsigned short TUint16;
typedef unsigned TUint32;
typedef unsigned long long TUint64;

typedef unsigned TKey;
typedef unsigned TUint;
typedef int TInt;
typedef int TStatus;

typedef void* THandle;

typedef unsigned TIpAddress;

} // namespace OpenHome

#endif //HEADER_TYPES
