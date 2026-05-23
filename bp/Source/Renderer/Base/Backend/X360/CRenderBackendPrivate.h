//------------------------------------------------------------------------------------------
// CRenderBackendPrivate.h
// Bluepoint
// Copyright 2011
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

BPE_CTASSERT(BPE_TARGET == BPE_TARGET_X360);

#include "Renderer/Base/Backend/CRenderBackend.h"

//------------------------------------------------------------------------------------------

inline CD3DCachedDevice * const GetD3DDevice()
{
   return RenderBackend()->GetCachedDeviceWrapper();
}

inline IDirect3DDevice9 * const D3DDeviceUncached()
{
   return RenderBackend()->GetDirect3DDevice().GetPtr();
}

//------------------------------------------------------------------------------------------


