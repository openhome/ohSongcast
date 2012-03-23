#ifndef HEADER_STANDARD
#define HEADER_STANDARD

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Exception.h>

#include <wdm.h>

namespace OpenHome {

#define ASSERTS() ASSERT(false);

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
