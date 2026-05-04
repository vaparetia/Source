#pragma once
#include "TransfarringDefines.h"

struct STUSSlotInfo
{
   unsigned char mAccessoryData[ 384 ];
};

class ITUSInterface
{
public:
   virtual ~ITUSInterface() { }

   virtual STransfarringError DeleteSlotData( int const slotIndex ) = 0;
   virtual STransfarringError SetSlotData( int const slotIndex, void const *pBuffer, size_t const size, STUSSlotInfo const &pSlotInfo ) = 0;
   virtual STransfarringError GetSlotData( int const slotIndex, void *pBufferOut, size_t const sizeOut, STUSSlotInfo *pSlotInfoOut ) = 0;
   virtual STransfarringError GetSlotInfos( STUSSlotInfo *pSlotInfosOut, uint8 *pValidSlots ) = 0;

   virtual STransfarringError IncrementSlotVariable( int const slotIndex, unsigned long long incrementBy, unsigned long long *pOutOldValue, unsigned long long *pOutNewValue ) = 0;
};

extern ITUSInterface *gpTUS;