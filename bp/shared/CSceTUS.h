#pragma once

#include "ITUSInterface.h"

class CSceTUS : public ITUSInterface
{
public:
   CSceTUS();
   virtual ~CSceTUS();

   virtual STransfarringError DeleteSlotData( int const slotId );
   virtual STransfarringError SetSlotData( int const slotId, void const *pBuffer, size_t const size, STUSSlotInfo const &pSlotInfo );
   virtual STransfarringError GetSlotData( int const slotId, void *pBufferOut, size_t const sizeOut, STUSSlotInfo *pSlotInfoOut );
   virtual STransfarringError GetSlotInfos( STUSSlotInfo *pSlotInfosOut, uint8 *pValidSlots );

   virtual STransfarringError IncrementSlotVariable( int const slotId, unsigned long long incrementBy, unsigned long long *pOutOldValue, unsigned long long *pOutNewValue );

private:
   bool EnsureValidTitleContext( STransfarringError * const pError );
   int AllocRequestID();

   int mTitleContext;
   SceNpId mTitleContextCachedID;
   int mLastRequestID;
};

