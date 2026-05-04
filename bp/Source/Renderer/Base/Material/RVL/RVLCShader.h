//----------------------------------------------------------------------------
// RVLCShader.h
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

#include "Renderer/Base/BPERendererAPI.h"

class CShaderInstance;
class CMaterialFlags;
class CDrawableRenderEntity;
class CGameObjectComponentProperties;
class CEvaluatorUpdateData;

class RENDERER_API CShader
{
public:
   typedef boost::shared_ptr<CGameObjectComponentProperties const>   TShaderProperties;

   // Creates a specific 'material' instance from properties.
   static boost::shared_ptr<CShader> const   CreateMaterial(TShaderProperties const & properties);

   CGameObjectComponentProperties const *GetProperties() const { return mProperties.get(); }
protected:
   explicit CShader(TShaderProperties const & properties);

public:
   virtual ~CShader();

   virtual void BuildConstantParameterBuffer();
   virtual void Bind( CMaterialFlags const * const pMaterialFlags ) = 0;
   virtual void SetupStateForRenderEntity( CShaderInstance const &material, CDrawableRenderEntity const *pEnt );
   virtual uint64                GetDrawOrder(CDrawableRenderEntity const & object) const;
   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

   virtual void DrawRenderObjects( CDrawableRenderEntity * const *pFirstObject, int const numObjects );

   virtual void BeginSinglePassRender();
   virtual void EndSinglePassRender();

protected:
   TShaderProperties mProperties;
};