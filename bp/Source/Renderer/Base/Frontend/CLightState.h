//------------------------------------------------------------------------------------------
// CLightState.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Engine/Graphics/CSphericalHarmonics3.h"

//------------------------------------------------------------------------------------------

class CBaseTexture;
class CLight;

//------------------------------------------------------------------------------------------

/**
 * CLightState class.
 * 
 * Provides information on how to light a mesh.
 */
 
class RENDERER_API CLightState
{
public:
   CLightState();
   ~CLightState();

   // Performance optimization, faster than reset via copying default constructor.
   void           Reset();

   // Assumes lights are already sorted by priority
   void           AddLight( CLight const * const pLight );
   int            GetNumLights() const;                     // Static + dynamic total
   CLight const * GetLight(int const index) const;          // Static then dynamic

   typedef bpe::reserved_vector<CLight const *, 4> TDynamicLights;
   TDynamicLights                            mDynamicLights;
   bpe::reserved_vector<CLight const *, 4>   mStaticLights;
   CVector3                                  mAmbient;
   CVector3                                  mAmbientOcclusion;
   mutable boost::shared_ptr<CBaseTexture>   mDynamicLightsTextureDir;
   mutable boost::shared_ptr<CBaseTexture>   mDynamicLightsTextureColor;
};

//------------------------------------------------------------------------------------------

class RENDERER_API CLightSets
{
public:
   CLightSets();
   ~CLightSets();

   // Performance optimization, faster than reset via copying default constructor.
   void Reset();

   static int const skNumLightSets              = 6;
   static int const skNumLightSetCombinations   = 1 << skNumLightSets;  // 64 combinations

   bpe::reserved_vector<CLightState, skNumLightSetCombinations>  mLightSets;
};

//------------------------------------------------------------------------------------------



