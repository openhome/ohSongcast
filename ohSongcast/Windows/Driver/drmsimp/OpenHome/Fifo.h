#ifndef HEADER_FIFO
#define HEADER_FIFO

#include <OpenHome/Standard.h>
#include <OpenHome/Exception.h>

EXCEPTION(FifoReadError);

namespace OpenHome {

// FifoLite provides lightweight thread unaware first in first out buffering
//
// Writes assert if the fifo is full (use SlotsFree() to check before writing)
// Reads from an empty fifo assert (use SlotsUsed() to check before reading)
 
class FifoLiteBase : public INonCopyable
{
public:
    TUint Slots() const;
    TUint SlotsFree() const;
    TUint SlotsUsed() const;
protected:
    FifoLiteBase(TUint aSlots);
    TUint Write();          // return index of entry to write
    TUint WriteBack();      // return index of entry to write at rhs of fifo
    TUint Read();           // return index of entry to read
private:
    TUint iSlots;
    TUint iSlotsUsed;
    TUint iReadIndex; 
    TUint iWriteIndex;
};

template <class T, TUint S> class FifoLite : public FifoLiteBase 
{
public:
    inline FifoLite() : FifoLiteBase(S) {}
    inline void Write(T aEntry) { iBuf[FifoLiteBase::Write()] = aEntry; }
    inline void WriteBack(T aEntry) { iBuf[FifoLiteBase::WriteBack()] = aEntry; }
    inline T Read() { return (iBuf[FifoLiteBase::Read()]); }
private:
    T iBuf[S];
};

} // namespace OpenHome

#endif //HEADER_FIFO
