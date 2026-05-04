#include "stdafx.h"
#include "RVLCShader.h"
#include "Engine\GameObjectSystem\CGameObjectComponentProperties.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"

//----------------------------------------------------------------------------

CShader::CShader(TShaderProperties const & properties)
: mProperties( properties )
{
}

//----------------------------------------------------------------------------

CShader::~CShader()
{
   RenderBackend()->RemoveMaterialPropertiesMapping( *this );
}

//----------------------------------------------------------------------------

void CShader::SetupStateForRenderEntity( CShaderInstance const &material, CDrawableRenderEntity const *pEnt )
{
   (material);
   (pEnt);
}

//----------------------------------------------------------------------------

void CShader::BuildConstantParameterBuffer()
{
}

//----------------------------------------------------------------------------

void CShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   (updateData);
}

//----------------------------------------------------------------------------

void CShader::DrawRenderObjects(CDrawableRenderEntity * const *pFirstObject, int const numObjects )
{
   CShaderVertexDataBinding binding;

   for ( int i = 0; i < numObjects; ++i )
   {
      CDrawableRenderEntity *pEnt = pFirstObject[ i ];

      RenderBackend()->SetModelMatrix( pEnt->GetTransform() );

      pEnt->RenderGeometry( binding );
   }

   RenderBackend()->SetModelMatrix( CMatrix34::Identity() );
}

//----------------------------------------------------------------------------

void CShader::BeginSinglePassRender()
{
}

//----------------------------------------------------------------------------

void CShader::EndSinglePassRender()
{
}

//----------------------------------------------------------------------------

uint64 CShader::GetDrawOrder(CDrawableRenderEntity const & object) const
{
   (object);
   return 0;
}