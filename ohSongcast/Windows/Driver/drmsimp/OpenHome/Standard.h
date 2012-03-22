#ifndef HEADER_STANDARD
#define HEADER_STANDARD

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Exception.h>

namespace OpenHome {

#define ASSERT(x) if (!(x)) ASSERTS()
#define ASSERTS() for(;;);

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
