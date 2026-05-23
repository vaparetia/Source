#pragma once

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/RVL/RVLCShader.h"

class RENDERER_API CRVLShaderOneTex : public CShader
{
   typedef CShader inherited;
public:
   CRVLShaderOneTex( TShaderProperties const &properties );

   virtual void Bind( CMaterialFlags const * const pMaterialFlags );
};