#pragma once

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/RVL/RVLCShader.h"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/Graphics/CColor.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Material/CMaterialFlags.h"

class RENDERER_API CDefaultShader : public CShader
{
   typedef CShader inherited;
public:
   CDefaultShader();

   virtual void Bind( CMaterialFlags const * const pMaterialFlags );

public:
   boost::optional< TResource<CBaseTexture> >         mTexture;
   boost::optional<CColor>                            mConstColor;
   bool                                               mHasColorStream;
   bool                                               mUseKonstColor;
   EBlendMode                                         mBlendMode;
   bool                                               mZEnable;
};