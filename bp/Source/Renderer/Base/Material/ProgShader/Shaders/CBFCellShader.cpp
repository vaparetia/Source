//------------------------------------------------------------------------------------------
// CBFCellShader.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CBFCellShader.h"

//------------------------------------------------------------------------------------------

#include "Engine/Evaluators/IEvaluator.h"

#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderBFCell.h"

//------------------------------------------------------------------------------------------

using namespace std;

CBFCellShaderCRCs CBFCellShader::sCRC;

//------------------------------------------------------------------------------------------

CBFCellShaderCRCs::CBFCellShaderCRCs()
:  mColor(GetParameterCRC("g_CellColor"))
,  mColorTexture(GetParameterCRC("g_ColorTexture"))
,  mLightOffset(GetParameterCRC("g_LightOffset"))
,  mLayer1UvOffset(GetParameterCRC("g_Layer1UvOffset"))
,  mLayer1UvScale(GetParameterCRC("g_Layer1UvScale"))

,  mLayer2Color(GetParameterCRC("g_Layer2Color"))
,  mLayer2Texture(GetParameterCRC("g_Layer2Texture"))
,  mLayer2UvOffset(GetParameterCRC("g_Layer2UvOffset"))
,  mLayer2UvScale(GetParameterCRC("g_Layer2UvScale"))

,  mLayer3Texture(GetParameterCRC("g_Layer3Texture"))
,  mLayer3UvOffset(GetParameterCRC("g_Layer3UvOffset"))
,  mLayer3UvScale(GetParameterCRC("g_Layer3UvScale"))

,  mLayer4Texture(GetParameterCRC("g_Layer4Texture"))
,  mLayer4UvOffset(GetParameterCRC("g_Layer4UvOffset"))
,  mLayer4UvScale(GetParameterCRC("g_Layer4UvScale"))

,  mModulateTexture(GetParameterCRC("g_ModulateTexture"))

,  mExtraColor1(GetParameterCRC("g_ExtraColor1"))
,  mExtraColor2(GetParameterCRC("g_ExtraColor2"))
,  mExtraColor3(GetParameterCRC("g_ExtraColor3"))
{
}

//------------------------------------------------------------------------------------------

CBFCellShader::CBFCellShader(TShaderProperties const &pProperties)
: inherited(pProperties)
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderBFCell::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CBFCellShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   CCPLdrShaderBFCell const * const pProperties = static_cast<CCPLdrShaderBFCell const * const>( GetProperties() );
   switch( pProperties->mShaderType )
   {
   case CCPLdrShaderBFCell::kST_CellCore:
      return false;

   case CCPLdrShaderBFCell::kST_CellSurface:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS1:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS2:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS3:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS4:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS5:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS6:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS7:
      return true;

   case CCPLdrShaderBFCell::kST_CellMesh:
      return materialFlags.GetModulateColor().IsTransparent();
   case CCPLdrShaderBFCell::kST_CellMeshExpS1:
   case CCPLdrShaderBFCell::kST_CellMeshExpS3:
   case CCPLdrShaderBFCell::kST_CellMeshExpS7:
      return false;

   case CCPLdrShaderBFCell::kST_Background:
      return false;

   case CCPLdrShaderBFCell::kST_Shadow:
      return true;

   case CCPLdrShaderBFCell::kST_Wire:
      return false;

   case CCPLdrShaderBFCell::kST_PreMul:
   case CCPLdrShaderBFCell::kST_RimShader:
      return pProperties->mWriteDepth ? false : true;

   case CCPLdrShaderBFCell::kST_PreMulLayered:
      return true;

   default:
      return false;
   }
}

//------------------------------------------------------------------------------------------

void CBFCellShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderBFCell const * const pProperties = static_cast<CCPLdrShaderBFCell const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CBFCellShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   // cache some ptrs to make code more readable

   CCPLdrShaderBFCell const * const pProperties = static_cast<CCPLdrShaderBFCell const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // setup vertex format
   CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
   SetShaderVertexDataBinding(binding);

   SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();

   EBlendMode blendMode = GetBlendMode(materialFlags);
   switch( pProperties->mShaderType )
   {
   case CCPLdrShaderBFCell::kST_CellSurface:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurface.fx");
         SetShader(skShaderFileId, 
                CStringExtras::StringizeInt_s("BLENDMODE=%d", blendMode));
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS1:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S1.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS2:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S2.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS3:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S3.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS4:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S4.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS5:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S5.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS6:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S6.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellSurfaceExpS7:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellSurfaceEx_S7.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_CellCore:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellCore.fx");
         SetShader(skShaderFileId, CStringExtras::StringizeInt_s("BLENDMODE=%d;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", blendMode, outputControl.GetShaderColorSpace(), outputControl.GetFrameBufferFormat()));
      }
      break;
   
   case CCPLdrShaderBFCell::kST_CellMesh:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellMesh.fx");
         SetShader(skShaderFileId, CStringExtras::StringizeInt_s("TYPE=0;BLENDMODE=%d;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", blendMode, outputControl.GetShaderColorSpace(), outputControl.GetFrameBufferFormat()));
      }
      break;

   case CCPLdrShaderBFCell::kST_CellMeshExpS1:
      {
         SetShaderVertexDataBinding(CShaderVertexDataBinding(kVDU_Position, kVDU_Normal));
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellMesh.fx");
         SetShader(skShaderFileId, CStringExtras::StringizeInt_s("TYPE=1;BLENDMODE=0;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", outputControl.GetShaderColorSpace(), outputControl.GetFrameBufferFormat()));
      }
      break;

   case CCPLdrShaderBFCell::kST_CellMeshExpS2:
      {
         SetShaderVertexDataBinding(CShaderVertexDataBinding(kVDU_Position, kVDU_Normal));
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellMesh.fx");
         SetShader(skShaderFileId, CStringExtras::StringizeInt_s("TYPE=2;BLENDMODE=0;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", outputControl.GetShaderColorSpace(), outputControl.GetFrameBufferFormat()));
      }
      break;

   case CCPLdrShaderBFCell::kST_CellMeshExpS3:
      {
         SetShaderVertexDataBinding(CShaderVertexDataBinding(kVDU_Position, kVDU_Normal));
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellMesh.fx");
         SetShader(skShaderFileId, CStringExtras::StringizeInt_s("TYPE=3;BLENDMODE=0;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", outputControl.GetShaderColorSpace(), outputControl.GetFrameBufferFormat()));
      }
      break;

   case CCPLdrShaderBFCell::kST_CellMeshExpS7:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFCellMesh_S7.fx");
         SetShader(skShaderFileId, 
         CStringExtras::StringizeInt_s("DIFF=%d;INC=%d;ENV=%d;ENVMASK=%d", pProperties->mColorTexture_Resource ? 1 : 0,
                                                                           pProperties->mLayer2Texture_Resource ? 1 : 0,
                                                                           pProperties->mLayer3Texture_Resource ? 1 : 0,
                                                                           pProperties->mLayer4Texture_Resource ? 1 : 0));
      }
      break;

   case CCPLdrShaderBFCell::kST_Background:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFBackground.fx");
         SetShader(skShaderFileId, pProperties->mMirror ? "MIRROR=1" : "MIRROR=0");
      }
      break;

   case CCPLdrShaderBFCell::kST_Shadow:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFShadow.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_Wire:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFWire.fx");
         SetShader(skShaderFileId, "");
      }
      break;

   case CCPLdrShaderBFCell::kST_PreMul:
      {
         bool const forceWriteDepthOff = (materialFlags.GetModulateColor().GetA() < 1.0f);
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFPreMul.fx");
         SetShader(skShaderFileId, (pProperties->mWriteDepth && !forceWriteDepthOff) ? "WRITE_DEPTH=1" : "WRITE_DEPTH=0");
      }
      break;

   case CCPLdrShaderBFCell::kST_RimShader:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/RimShader.fx");
         SetShader(skShaderFileId, 
         CStringExtras::StringizeInt_s("WRITE_DEPTH=%d;CLAMP_U=%d;CLAMP_V=%d", pProperties->mWriteDepth,
                                                                               pProperties->mClampU,
                                                                               pProperties->mClampV));
      }
      break;

   case CCPLdrShaderBFCell::kST_PreMulLayered:
      {
         static const CShaderFileId skShaderFileId("$/enginesupport/shaders/BFPreMulLayered.fx");      
         SetShader(skShaderFileId, 
         CStringExtras::StringizeInt_s("LAYER1=%d;LAYER2=%d;MODULATETEX=%d", pProperties->mColorTexture_Resource ? 1 : 0,
                                                                             pProperties->mLayer2Texture_Resource ? 1 : 0,
                                                                             pProperties->mModulateTexture_Resource ? 1 : 0));
      }
      break;
   }
   // set shader
}

//----------------------------------------------------------------------------

void CBFCellShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderBFCell const * const pProperties = static_cast<CCPLdrShaderBFCell const * const>( GetProperties() );

   parameters.AddParameter(sCRC.mLightOffset, CVector3(0, pProperties->mLightOffset, 0));

   parameters.AddTexture(sCRC.mColorTexture, pProperties->mColorTexture_Resource ? pProperties->mColorTexture_Resource->GetPtr() : &RenderBackend()->GetBlackMap());
   parameters.AddParameter(sCRC.mLayer1UvScale, pProperties->mLayer1UvScale);

   parameters.AddParameter(sCRC.mLayer2UvScale, pProperties->mLayer2UvScale);
   parameters.AddTexture(sCRC.mLayer2Texture, pProperties->mLayer2Texture_Resource ? pProperties->mLayer2Texture_Resource->GetPtr() : &RenderBackend()->GetBlackMap());

   parameters.AddParameter(sCRC.mLayer3UvScale, pProperties->mLayer3UvScale);
   parameters.AddTexture(sCRC.mLayer3Texture, pProperties->mLayer3Texture_Resource ? pProperties->mLayer3Texture_Resource->GetPtr() : &RenderBackend()->GetBlackMap());

   parameters.AddParameter(sCRC.mLayer4UvScale, pProperties->mLayer4UvScale);
   parameters.AddTexture(sCRC.mLayer4Texture, pProperties->mLayer4Texture_Resource ? pProperties->mLayer4Texture_Resource->GetPtr() : &RenderBackend()->GetBlackMap());

   parameters.AddTexture(sCRC.mModulateTexture, pProperties->mModulateTexture_Resource ? pProperties->mModulateTexture_Resource->GetPtr() : &RenderBackend()->GetWhiteMap());
}

//----------------------------------------------------------------------------

void CBFCellShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( parameterType == kPT_Unshared )
   {
      CCPLdrShaderBFCell const * const pProperties = static_cast<CCPLdrShaderBFCell const * const>( GetProperties() );
      CMaterialFlags const & materialFlags = object->GetMaterialFlags();

      CEvaluatorUpdateData updateData = object->GetUpdateData(this);

      // Color
      {
         CColorf color = pProperties->mColor ? pProperties->mColor->GetValue(updateData) : CColorf::White();

         switch( pProperties->mShaderType )
         {
         case CCPLdrShaderBFCell::kST_Background:
         case CCPLdrShaderBFCell::kST_Shadow:
         case CCPLdrShaderBFCell::kST_Wire:
         case CCPLdrShaderBFCell::kST_PreMulLayered:
            color *= materialFlags.GetModulateColor();
            break;

         case CCPLdrShaderBFCell::kST_PreMul:
         case CCPLdrShaderBFCell::kST_RimShader:
            {
               // alpha channel is ignored, this is additive color so we multiple alpha directly into rgb
               color *= materialFlags.GetModulateColor();
               color *= materialFlags.GetModulateColor().GetA();
            }
            break;
         }

         parameters.AddParameterGamma(sCRC.mColor, color, colorSpace);
      }

      // Layer2 Color
      {
         CColorf const color = pProperties->mLayer2Color ? pProperties->mLayer2Color->GetValue(updateData) : CColorf::White();
         parameters.AddParameterGamma(sCRC.mLayer2Color, color, colorSpace);
      }

      // Layer1 UVOffset
      {
         CVector3 const uvOffset = pProperties->mLayer1UvOffset ? pProperties->mLayer1UvOffset->GetValue(updateData) : CVector3::Zero();
         parameters.AddParameter(sCRC.mLayer1UvOffset, uvOffset);
      }


      // Layer2 UVOffset
      {
         CVector3 const uvOffset = pProperties->mLayer2UvOffset ? pProperties->mLayer2UvOffset->GetValue(updateData) : CVector3::Zero();
         parameters.AddParameter(sCRC.mLayer2UvOffset, uvOffset);
      }

      // Layer3 UVOffset
      {
         CVector3 const uvOffset = pProperties->mLayer3UvOffset ? pProperties->mLayer3UvOffset->GetValue(updateData) : CVector3::Zero();
         parameters.AddParameter(sCRC.mLayer3UvOffset, uvOffset);
      }

      // Layer4 UVOffset
      {
         CVector3 const uvOffset = pProperties->mLayer4UvOffset ? pProperties->mLayer4UvOffset->GetValue(updateData) : CVector3::Zero();
         parameters.AddParameter(sCRC.mLayer4UvOffset, uvOffset);
      }

      // Extra Color 1
      {
         CColorf const color = pProperties->mExtraColor1 ? pProperties->mExtraColor1->GetValue(updateData) : CColorf::White();
         parameters.AddParameterGamma(sCRC.mExtraColor1, color, colorSpace);
      }

      // Extra Color 2
      {
         CColorf const color = pProperties->mExtraColor2 ? pProperties->mExtraColor2->GetValue(updateData) : CColorf::White();
         parameters.AddParameterGamma(sCRC.mExtraColor2, color, colorSpace);
      }

      // Extra Color 3
      {
         CColorf const color = pProperties->mExtraColor3 ? pProperties->mExtraColor3->GetValue(updateData) : CColorf::White();
         parameters.AddParameterGamma(sCRC.mExtraColor3, color, colorSpace);
      }
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CBFCellShader::RenderGeometry(CDrawableRenderEntity& object) 
{
   CCPLdrShaderBFCell const * const pProperties = static_cast<CCPLdrShaderBFCell const * const>(GetProperties());

   switch( pProperties->mShaderType )
   {
   case CCPLdrShaderBFCell::kST_CellSurface:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS1:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS2:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS3:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS4:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS5:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS6:
   case CCPLdrShaderBFCell::kST_CellSurfaceExpS7:
   case CCPLdrShaderBFCell::kST_RimShader:
      {
         CColorf const modulateColor = object.GetMaterialFlags().GetModulateColor();
         
         if( close_enough(modulateColor.GetA(), 0.0f, CColorf::kAlphaTolerance) )
            return;
      }
      break;

   default:
      break;
   }

   inherited::RenderGeometry(object);

}

