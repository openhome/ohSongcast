#ifndef HEADER_STANDARD
#define HEADER_STANDARD

#include <kern/assert.h>
#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Exception.h>


namespace OpenHome {

#define ASSERT(expr) assert(expr);
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
