#ifndef HEADER_ARCH
#define HEADER_ARCH

#include <OpenHome/OhNetTypes.h>

namespace OpenHome {

#define SwapEndian16(x) (((x)>>8) | ((x)<<8))

#define SwapEndian32(x) ((((x)&0xFFul)<<24) | (((x)&0xFF00ul)<<8) | (((x)&0xFF0000ul)>>8) | (((x)&0xFF000000ul)>>24))

class Arch
{
public:
    inline static TUint16 BigEndian2(TUint16 x) {return (TUint16)(SwapEndian16(x));}
    inline static TUint32 BigEndian4(TUint32 x) {return (TUint32)(SwapEndian32(x));}
    inline static TUint64 BigEndian8(TUint64 x) { return (TUint64(BigEndian4(TUint32(x))) << 32) | BigEndian4(TUint32(x>>32)); }
    inline static TUint16 LittleEndian2(TUint16 x) {return x;}
    inline static TUint32 LittleEndian4(TUint32 x) {return x;}
    inline static TUint64 LittleEndian8(TUint64 x) {return x;}
};

} // namespace OpenHome

#endif  // HEADER_ARCH

