//----------------------------------------------------------------------------
// CRenderBackendPrivate.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"

//----------------------------------------------------------------------------

BPE_CTASSERT(BPE_TARGET == BPE_TARGET_PS3);

//----------------------------------------------------------------------------

static inline void _psglSetFragmentProgramEmbeddedConstant( const uint32 dstVidOffset, const float *srcHostAdr, const uint32 sizeInWords )
{
   cell::Gcm::cellGcmSetTransferLocation(CELL_GCM_LOCATION_LOCAL);

   void *pointer=NULL;
   cell::Gcm::cellGcmSetInlineTransferPointer(dstVidOffset, sizeInWords, &pointer);
   float *fp = (float*)pointer;
   float *src = (float*)srcHostAdr;
   for (uint32_t j=0; j<sizeInWords;j++)
   {
      *fp = cellGcmSwap16Float32(*src);
      fp++;src++;
   }
}
