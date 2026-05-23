//------------------------------------------------------------------------------------------
// DCCTexture.cpp
// Dreamcast texture stub
//
// All methods are no-ops until the KallistiOS / PVRTC pipeline is implemented.
// See docs_and_planning/04 ASSET EXTRACTION GUIDE.md for the target format.
//------------------------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "Renderer/Base/Backend/DC/DCCTexture.h"

//----------------------------------------------------------------------------

CTexture::CTexture(EFormat format, EType type, EUsage usage)
:  CBaseTexture(format, type, usage)
,  mpPVRData(NULL)
,  mMemorySize(0)
,  mWidth(0)
,  mHeight(0)
{
}

CTexture::~CTexture()
{
   FreeMemory();
}

void CTexture::FreeMemory()
{
   // TODO Phase 1: pvr_mem_free(mpPVRData)
   mpPVRData = NULL;
   mMemorySize = 0;
}

void CTexture::SetTexture(int const /*textureStage*/) const
{
   // TODO Phase 2: pvr_txr_load / set texture state in current polygon header
}

void CTexture::Lock(void ** pMemory, int * pPitch)
{
   // TODO Phase 2: map VRAM for CPU write
   if (pMemory) *pMemory = NULL;
   if (pPitch)  *pPitch = 0;
}

void CTexture::Unlock()
{
   // TODO Phase 2
}

//----------------------------------------------------------------------------
