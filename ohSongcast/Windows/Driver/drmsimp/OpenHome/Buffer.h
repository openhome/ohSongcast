#ifndef HEADER_BUFFER
#define HEADER_BUFFER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Exception.h>
#include <OpenHome/Standard.h>

EXCEPTION(BufferFull);
EXCEPTION(BufferOverflow);

namespace OpenHome {

class Brn;

class DllExportClass Brx
{
public:
    inline TUint Bytes() const;
    inline const TByte& operator[](TUint aByteIndex) const;
    inline TBool operator==(const Brx& aBrx) const;
    inline TBool operator!=(const Brx& aBrx) const;
    DllExport TBool Equals(const Brx& aBrx) const;
    const TByte& At(TUint aByteIndex) const;
    virtual const TByte* Ptr() const=0;
    Brn Split(TUint aByteIndex) const;
    Brn Split(TUint aByteIndex, TUint aBytes) const;
    TBool BeginsWith(const Brx& aBrx) const;
    virtual ~Brx() {};
protected:
    explicit inline Brx(TUint aBytes);
    explicit inline Brx(const Brx& aBrx);
    TUint iBytes;
private:
    Brx& operator=(const Brx& aBrx);
};

class DllExportClass Brn : public Brx
{
public:
    inline explicit Brn();
    inline explicit Brn(const Brx& aBrx);
    inline Brn(const Brn& aBrn);
    inline explicit Brn(const TByte* aPtr, TUint aBytes);
    DllExport explicit Brn(const TChar* aPtr);
    inline void Set(const Brx& aBrx);
    inline void Set(const TByte* aPtr, TUint aBytes);
    void Set(const TChar* aPtr);
    virtual const TByte* Ptr() const;
protected:
    const TByte* iPtr;
private:
    Brn& operator=(const Brn& aBrn);
};

class DllExportClass Bwx : public Brx
{
public:
	void Clear();
    void Replace(const Brx& aBuf);
    void ReplaceThrow(const Brx& aBuf);
    void Replace(const TByte* aPtr, TUint aBytes);
    void Replace(const TChar* aStr);
    inline void Append(TChar aChar);
    inline void Append(TByte aByte);
    void Append(const Brx& aB);
    void Append(const TChar* aStr);
    void Append(const TByte* aPtr, TUint aBytes);
    const TChar* PtrZ() const;
    void Fill(TByte aFillByte);
    inline void FillZ();
    inline TUint MaxBytes() const;
    void SetBytes(TUint aBytes);
    inline TByte& operator[](TUint aByteIndex);
    TByte& At(TUint aByteIndex);
protected:
    explicit Bwx(TUint aBytes, TUint aMaxBytes);
    TUint iMaxBytes;
};

class DllExportClass Bwn : public Bwx
{
public:
    explicit Bwn();
    explicit Bwn(const Bwx& aBwx);
    Bwn(const Bwn& aBwn);
    explicit Bwn(const TByte* aPtr, TUint aMaxBytes);
    explicit Bwn(const TByte* aPtr, TUint aBytes, TUint aMaxBytes);
    explicit Bwn(const TChar* aPtr, TUint aMaxBytes);
    explicit Bwn(const TChar* aPtr, TUint aBytes, TUint aMaxBytes);
    void Set(const Bwx& aBwx);
    void Set(const TByte* aPtr, TUint aMaxBytes);
    void Set(const TByte* aPtr, TUint aBytes, TUint aMaxBytes);
    virtual const TByte* Ptr() const;
protected:
    const TByte* iPtr;
private:
    Bwn& operator=(const Bwn&);
};

template <TUint S> class DllExportClass Bws : public Bwx
{
public:
    inline Bws();
    inline Bws(TUint aBytes);
    explicit inline Bws(const TChar* aStr);
    explicit inline Bws(const TByte* aPtr, TUint aBytes);
    explicit inline Bws(const Brx& aBuf);
    explicit inline Bws(const Bws<S>&);
    inline const TByte* Ptr() const;
protected:
    TByte iBuf[S];
private:
    Bws<S>& operator=(const Bws<S>&);
};

/**
 * Custom comparison function for stl map keyed on Brn
 */
class BufferCmp
{
public:
    TBool operator()(const Brx& aStr1, const Brx& aStr2) const;
};

#include <OpenHome/Buffer.inl>

} // namespace OpenHome

#endif
