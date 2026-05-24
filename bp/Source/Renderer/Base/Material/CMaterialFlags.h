//----------------------------------------------------------------------------
// CMaterialFlags.h
// Stub — minimal interface required by ProgShader renderer.
//----------------------------------------------------------------------------

#pragma once

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Engine/Graphics/CColorf.h"

//----------------------------------------------------------------------------

class RENDERER_API CMaterialFlags
{
public:
   CMaterialFlags()
      :  mpLightState(NULL)
      ,  mLightSets(0xFFFFFFFF)
   {
   }

   static CMaterialFlags Normal() { return CMaterialFlags(); }

   CLightState const * GetLightState() const      { return mpLightState; }
   void                SetLightState(CLightState const * p) { mpLightState = p; }

   CColorf GetModulateColor() const  { return CColorf::White(); }
   CColorf GetAdditiveColor() const  { return CColorf::Zero(); }

   uint32 mLightSets;

private:
   CLightState const * mpLightState;
};

//----------------------------------------------------------------------------
