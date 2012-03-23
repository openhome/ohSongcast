#include <OpenHome/Fifo.h>

using namespace OpenHome;

// FifoLiteBase

FifoLiteBase::FifoLiteBase(TUint aSlots) : iSlots(aSlots), iSlotsUsed(0),
    iReadIndex(0), iWriteIndex(0)
{
}

void* FifoLiteBase::operator new(size_t aBytes)
{
	return (ExAllocatePoolWithTag(NonPagedPool, aBytes, '2ten'));
}

void  FifoLiteBase::operator delete(void* aPtr)
{
	ExFreePoolWithTag(aPtr, '2ten');
}

TUint FifoLiteBase::Slots() const
{
    return iSlots;
}

TUint FifoLiteBase::SlotsFree() const
{
    return iSlots - iSlotsUsed;
}

TUint FifoLiteBase::SlotsUsed() const
{
    return iSlotsUsed;
}

TUint FifoLiteBase::Write()
{
    ASSERT(iSlots > iSlotsUsed);
    TUint index = iWriteIndex++;
    if(iWriteIndex == iSlots) {
        iWriteIndex = 0;
    }
    iSlotsUsed++;
    return (index);
}

TUint FifoLiteBase::WriteBack()
{
    ASSERT(iSlots > iSlotsUsed);
    if(iReadIndex == 0) {
        iReadIndex = iSlots - 1;;
    }
    else {
        iReadIndex--;
    }
    iSlotsUsed++;
    return (iReadIndex);
}

TUint FifoLiteBase::Read()
{
    ASSERT(iSlotsUsed > 0);
    TUint index = iReadIndex++;
    if(iReadIndex == iSlots) {
        iReadIndex = 0;
    }
    iSlotsUsed--;
    return index;
}
