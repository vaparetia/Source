#include "StdAfx.h"
#include "PSCShader.h"
#include "Renderer/Base/ShaderObjects/CShaderObjectFactory.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

boost::shared_ptr<CShader> const CShader::CreateMaterial(boost::shared_ptr<CGameObjectComponentProperties const> const & properties)
{
   boost::shared_ptr<CShader> material;

#if 0
   material.reset(new CGOWShader(properties));

   material->BuildConstantParameterBuffer();
   RenderBackend()->AddMaterialPropertiesMapping(material);
#endif

   return material;
}
