#ifndef HEADER_STANDARD
#define HEADER_STANDARD

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Exception.h>

namespace OpenHome {

#define OHASSERT(x)  if(!(x)) OHASSERTS()	

#define OHASSERTS() *((TUint32*)0) = 0

class INonCopyable
{
protected:
    INonCopyable() {}
private:
    INonCopyable(const INonCopyable &);
    void operator=(const INonCopyable &);
};

} // namespace OpenHome

#endif // HEADER_STANDARD
