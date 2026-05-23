#include "StdAfx.h"
#include "RVLCShader.h"
#include "Engine\GameObjectSystem\CGameObjectComponentProperties.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
//----------------------------------------------------------------------------

#include "Renderer/Base/ShaderObjects/RVLShader/CCPLdrRVLShaderOneTex.h"
#include "Renderer/Base/Material/RVL/Shaders/CRVLShaderOneTex.h"

//----------------------------------------------------------------------------

static inline boost::shared_ptr<CShader> const material_factory(CShader::TShaderProperties const & properties)
{
   switch ( properties->GetComponentType() )
   {
   case CCPLdrRVLShaderOneTex::kComponentPropertiesType:
      return boost::shared_ptr<CShader>( new CRVLShaderOneTex( properties ) );

   default:
      BPE_VERIFY(false, false, "unsupported shader property data");
      break;
   }

   return boost::shared_ptr<CShader>();
}

//----------------------------------------------------------------------------

boost::shared_ptr<CShader> const CShader::CreateMaterial(TShaderProperties const & properties)
{
   boost::shared_ptr<CShader> const ptr = material_factory( properties );

   RenderBackend()->AddMaterialPropertiesMapping( ptr );

   return ptr;
}

//----------------------------------------------------------------------------

