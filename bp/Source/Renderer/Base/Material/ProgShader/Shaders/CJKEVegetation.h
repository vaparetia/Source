//------------------------------------------------------------------------------------------
// CJKEVegetation.h
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/Shaders/NJKEShaderCommon.h"
#include "Renderer/Base/Material/ProgShader/Shaders/PSShaderHelpers.h"

//------------------------------------------------------------------------------------------
class CJKEVegetationCRCs : public CShaderCRCs
{
public:
   CJKEVegetationCRCs();

   uint32   mDiffuseTexture;
   uint32   mNormalTexture;
   uint32   mIncandescenceTexture;

   uint32   mAmbientColor;

   uint32   mMovementDirection;
   uint32   mMovementParameters;

   uint32   mShakeSource;
   uint32   mShakeParams;

   uint32   mProjectionZRows;

   NJKEShaderCommon::SFogParameters mFog;
};

//------------------------------------------------------------------------------------------

class RENDERER_API CJKEVegetation : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKEVegetation(TShaderProperties const &pProperties);

   virtual void   Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void   BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void   BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool   HasAlpha(CMaterialFlags const &materialFlags) const;
   virtual uint64 GetDrawOrder(CDrawableRenderEntity const & object) const;

   static void SetGlobalShakeParameters( CVector3 const &shakeSource, real32 const shakeStrength, real32 const radialStrength, real32 const falloff );

protected:
   static CJKEVegetationCRCs     sCRC;

   PSShaderHelpers::ELightmapType   mLightmapType;
   CVector3                         mAmbientValue;
};

