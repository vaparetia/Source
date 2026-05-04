//------------------------------------------------------------------------------------------
// CRenderBackendPrivate.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

BPE_CTASSERT(BPE_TARGET == BPE_TARGET_WIN32);

#include "Renderer/Base/Backend/CRenderBackend.h"

//------------------------------------------------------------------------------------------

inline CD3DCachedDevice * const D3DDevice()
{
   return RenderBackend()->GetCachedDeviceWrapper();
}

inline IDirect3DDevice9 * const D3DDeviceUncached()
{
   return RenderBackend()->GetDirect3DDevice().GetPtr();
}

//------------------------------------------------------------------------------------------


