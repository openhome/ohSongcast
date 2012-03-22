#include <OpenHome/Buffer.h>
#include <OpenHome/Standard.h>
#include <OpenHome/Arch.h>

#include <string.h>

using namespace OpenHome;

const TUint kMinBwdMaxBytes = 4;

#define OhNetStrlen(s) (TUint)strlen(s)

// Brx

TBool Brx::Equals(const Brx& aBrx) const
{
    if(Bytes() == aBrx.Bytes()) {
        const TByte* dest = Ptr();
        ASSERT(dest != NULL);
        const TByte* src = aBrx.Ptr();
        ASSERT(src != NULL);
        return(memcmp(dest, src, Bytes()) == 0);
    }
    return false;
}

const TByte& Brx::At(TUint aByteIndex) const
{
    ASSERT(aByteIndex < Bytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    return((TByte&)(ptr[aByteIndex]));
}

Brn Brx::Split(TUint aByteIndex) const
{
    ASSERT(aByteIndex <= Bytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    return Brn(ptr + aByteIndex, Bytes() - aByteIndex);
}

Brn Brx::Split(TUint aByteIndex, TUint aBytes) const
{
    ASSERT(aByteIndex + aBytes <= Bytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    return Brn(ptr + aByteIndex, aBytes);
}

TBool Brx::BeginsWith(const Brx& aBrx) const
{
    if(Bytes() >= aBrx.Bytes()) {
        const TByte* ptr1 = Ptr();
        ASSERT(ptr1 != NULL);
        const TByte* ptr2 = aBrx.Ptr();
        ASSERT(ptr2 != NULL);
        return(memcmp(ptr1, ptr2, aBrx.Bytes()) == 0);
    }
    return false;
}

// Brn

Brn::Brn(const TChar* aPtr) : Brx(OhNetStrlen(aPtr)), iPtr((TByte*)aPtr)
{
}

void Brn::Set(const TChar* aStr)
{
    iPtr = (const TByte*)aStr;
    iBytes = OhNetStrlen(aStr);
}

const TByte* Brn::Ptr() const
{
    return iPtr;
}

// Bwx

Bwx::Bwx(TUint aBytes, TUint aMaxBytes) : Brx(aBytes), iMaxBytes(aMaxBytes)
{
    //Ensure we didn't just construct an invalid Buffer
    ASSERT(Bytes() <= MaxBytes());
}

void Bwx::Clear()
{
	iBytes = 0;
}

void Bwx::Replace(const Brx& aBuf)
{
    if (aBuf.Bytes() > MaxBytes()) {
		ASSERTS();
    }
    else
    {
        const TByte* dest = Ptr();
        ASSERT(dest != NULL);
        const TByte* src = aBuf.Ptr();
        ASSERT(src != NULL);
        (void)memmove(const_cast<TByte*>(dest), src, aBuf.Bytes());
        iBytes = aBuf.Bytes();
    }
}

void Bwx::ReplaceThrow(const Brx& aBuf)
{
    if (aBuf.Bytes() > MaxBytes()) {
        THROW(BufferOverflow);
    }
    else
    {
        const TByte* dest = Ptr();
        ASSERT(dest != NULL);
        const TByte* src = aBuf.Ptr();
        ASSERT(src != NULL);
        (void)memmove(const_cast<TByte*>(dest), src, aBuf.Bytes());
        iBytes = aBuf.Bytes();
    }
}

void Bwx::Replace(const TByte* aPtr, TUint aBytes)
{
    ASSERT(aBytes <= MaxBytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    memcpy(const_cast<TByte*>(ptr), aPtr, aBytes);
    iBytes = aBytes;
}

void Bwx::Replace(const TChar* aStr)
{
    TUint bytes = OhNetStrlen(aStr);
    ASSERT(bytes <= MaxBytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    memcpy(const_cast<TByte*>(ptr), aStr, bytes);
    iBytes = bytes;
}

void Bwx::Append(const TChar* aStr)
{
    Append((TByte*)aStr, OhNetStrlen(aStr));
}

void Bwx::Append(const Brx& aB)
{
    const TByte* ptr = aB.Ptr();
    ASSERT(ptr != NULL);
    Append(ptr, aB.Bytes());
}

void Bwx::Append(const TByte* aPtr, TUint aBytes)
{
    ASSERT(Bytes() + aBytes <= MaxBytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    memcpy(const_cast<TByte*>(ptr+Bytes()), aPtr, aBytes);
    iBytes = Bytes() + aBytes;
}

const TChar* Bwx::PtrZ() const
{
    ASSERT(Bytes() + 1 <= MaxBytes());
    //Nul terminate without updating number of bytes in buffer
    TByte* ptr = (TByte*)Ptr();
    ASSERT(ptr != NULL);
    *(ptr + Bytes()) = '\0';
    return (const TChar*)ptr;
}

void Bwx::Fill(TByte aByte)
{
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    memset(const_cast<TByte*>(ptr), aByte, Bytes());
}

void Bwx::SetBytes(TUint aBytes)
{
    ASSERT(aBytes <= MaxBytes());
    iBytes = aBytes;
}

TByte& Bwx::At(TUint aByteIndex)
{
    ASSERT(aByteIndex < Bytes());
    const TByte* ptr = Ptr();
    ASSERT(ptr != NULL);
    return(const_cast<TByte&>(ptr[aByteIndex]));
}

// Bwn

Bwn::Bwn() : Bwx(0,0), iPtr(0)
{
}

Bwn::Bwn(const Bwx& aBwx) : Bwx(aBwx.Bytes(), aBwx.MaxBytes()), iPtr(aBwx.Ptr())
{
    ASSERT(iPtr != NULL);
}

Bwn::Bwn(const Bwn& aBwn) : Bwx(aBwn.Bytes(), aBwn.MaxBytes()), iPtr(aBwn.Ptr())
{
    ASSERT(iPtr != NULL);
}

Bwn::Bwn(const TByte* aPtr, TUint aMaxBytes) : Bwx(0, aMaxBytes), iPtr(aPtr)
{
}

Bwn::Bwn(const TByte* aPtr, TUint aBytes, TUint aMaxBytes) : Bwx(aBytes, aMaxBytes), iPtr(aPtr)
{
}

Bwn::Bwn(const TChar* aPtr, TUint aMaxBytes) : Bwx(0, aMaxBytes), iPtr((TByte*)aPtr)
{
}

Bwn::Bwn(const TChar* aPtr, TUint aBytes, TUint aMaxBytes) : Bwx(aBytes, aMaxBytes), iPtr((TByte*)aPtr)
{
}

void Bwn::Set(const Bwx& aBwx)
{
    iPtr = aBwx.Ptr();
    ASSERT(iPtr != NULL);
    iBytes = aBwx.Bytes();
    iMaxBytes = aBwx.MaxBytes();
}

void Bwn::Set(const TByte* aPtr, TUint aMaxBytes)
{
    iPtr = aPtr;
    iBytes = 0;
    iMaxBytes = aMaxBytes;
}

void Bwn::Set(const TByte* aPtr, TUint aBytes, TUint aMaxBytes)
{
    ASSERT(aBytes <= aMaxBytes);
    iPtr = aPtr;
    iBytes = aBytes;
    iMaxBytes = aMaxBytes;
}

const TByte* Bwn::Ptr() const
{
    return iPtr;
}

// BufferCmp

TBool BufferCmp::operator()(const Brx& aStr1, const Brx& aStr2) const
{
    const TInt bytes1 = aStr1.Bytes();
    const TInt bytes2 = aStr2.Bytes();
    const TInt bytes = (bytes1<bytes2? bytes1 : bytes2);
    const TByte* ptr1 = aStr1.Ptr();
    const TByte* ptr2 = aStr2.Ptr();
    for (TInt i=0; i<bytes; i++) {
        if (*ptr1 != *ptr2) {
            return (*ptr1 < *ptr2);
        }
        ptr1++;
        ptr2++;
    }
    return (bytes1 < bytes2);
}

