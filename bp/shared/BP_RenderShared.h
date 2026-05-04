//----------------------------------------------------------------------------
// BP_RenderShared.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#include "bp_matrix.h"
#include "BP_RenderGS.h"

#if BP_ENABLE_TEXTURE_TOOL
#include "BP_TextureTool.h"
#endif

static inline CBaseTexture const * BP_DG_GetTexture(DG_TEX* pTex)
{
#if BP_ENABLE_TEXTURE_TOOL
   return BP_TextureTool_GetBPTexture(pTex);
#else
   return (CBaseTexture const *)pTex->BP_TextureHandle;
#endif
}

static inline CBaseTexture const * BP_DG_GetTexture(DG_TEX_BP* pTex)
{
#if BP_ENABLE_TEXTURE_TOOL
   // Texture tool "pTex->pDebugOrigSrc" may point to a deleted texture between level loads 
   // during multi-threaded rendering. To avoid crashing, BP_Renderer.cpp special case waits
   // when the texture tool is enabled. If it's not enabled, we just use the input DG_TEX_BP
   // texture handle which is guaranteed to be valid.
   return BP_TextureTool_GetEnable() ? BP_TextureTool_GetBPTexture(pTex->pDebugOrigSrc) : (CBaseTexture const *)pTex->BP_TextureHandle;
#else
   return (CBaseTexture const *)pTex->BP_TextureHandle;
#endif
}

//----------------------------------------------------------------------------
// TODO: this function will be going away very soon in favor of DG_TEX_BP* version
static inline void BP_DG_SetTexture(DG_TEX* pTex, int unit)
{
#if BP_ENABLE_TEXTURE_TOOL
   CBaseTexture const * pTexture = BP_TextureTool_GetBPTexture(pTex);
#else
   CBaseTexture const * pTexture = (CBaseTexture const *)pTex->BP_TextureHandle;
#endif

   if( !pTexture )
   {
#ifdef BP_RENDER_SINGLE_RENDER_TARGET
#if BP_VITA
      // we do not want all of the ifndef BP_RENDER_SINGLE_RENDER_TARGET code, but we do want this
      if( pTex->BP_flag & DG_TEXFLAG_RENDERTARGET )
      {
         ERenderTarget const rt = (ERenderTarget)pTex->tex_id;
         pTexture = BP_GetRenderTarget(rt);
      }
      else
      {
         BP_RENDER_TODO_BREAK;
         pTexture = &gpRenderBackend->GetWhiteMap();
      }
#else
      pTexture = &gpRenderBackend->GetBlackMap();
#endif
#else
      if( pTex->BP_flag & DG_TEXFLAG_RENDERTARGET )
      {
         ERenderTarget const rt = (ERenderTarget)pTex->tex_id;
         pTexture = BP_GetRenderTarget(rt);
      }
      else
      {
         BP_RENDER_TODO_BREAK;
         pTexture = &gpRenderBackend->GetWhiteMap();
      }
#endif
   }

#if BP_ENABLE_DEBUG_TEXTURE_GPU_MARKER && (BPE_TARGET == BPE_TARGET_WIN32 || defined(_DEBUG))
   BPE_GPU_EVENT(pTexture->mDebugName.c_str());
#endif

   gpRenderBackend->SetTexture(unit, pTexture);
}

//----------------------------------------------------------------------------

static inline void BP_DG_SetTexture(DG_TEX_BP* pTex, int unit)
{
#if BP_ENABLE_TEXTURE_TOOL
   // Texture tool "pTex->pDebugOrigSrc" may point to a deleted texture between level loads 
   // during multi-threaded rendering. To avoid crashing, BP_Renderer.cpp special case waits
   // when the texture tool is enabled. If it's not enabled, we just use the input DG_TEX_BP
   // texture handle which is guaranteed to be valid.
   CBaseTexture const * pTexture = BP_TextureTool_GetEnable() ? BP_TextureTool_GetBPTexture(pTex->pDebugOrigSrc) : (CBaseTexture const *)pTex->BP_TextureHandle;
#else
   CBaseTexture const * pTexture = (CBaseTexture const *)pTex->BP_TextureHandle;
#endif

   if( !pTexture )
   {
#ifdef BP_RENDER_SINGLE_RENDER_TARGET
#if BP_VITA
      // we do not want all of the ifndef BP_RENDER_SINGLE_RENDER_TARGET code, but we do want this
      if( pTex->BP_flag & DG_TEXFLAG_RENDERTARGET )
      {
         ERenderTarget const rt = (ERenderTarget)pTex->tex_id;
         pTexture = BP_GetRenderTarget(rt);
      }
      else
      {
         BP_RENDER_TODO_BREAK;
         pTexture = &gpRenderBackend->GetWhiteMap();
      }
#else
      pTexture = &gpRenderBackend->GetBlackMap();
#endif
#else
      if( pTex->BP_flag & DG_TEXFLAG_RENDERTARGET )
      {
         ERenderTarget const rt = (ERenderTarget)pTex->tex_id;
         pTexture = BP_GetRenderTarget(rt);
      }
      else
      {
         BP_RENDER_TODO_BREAK;
         pTexture = &gpRenderBackend->GetWhiteMap();
      }
#endif
   }

#if BP_ENABLE_DEBUG_TEXTURE_GPU_MARKER && (BPE_TARGET == BPE_TARGET_WIN32 || defined(_DEBUG))
   BPE_GPU_EVENT(pTexture->mDebugName.c_str());
#endif

   gpRenderBackend->SetTexture(unit, pTexture);
}

//----------------------------------------------------------------------------
// TODO: this function will be going away very soon in favor of DG_TEX_BP* version
static inline void BP_SetTextureNoClamp(DG_TEX* pTex, int unit)
{
   BP_DG_SetTexture(pTex, unit);
   gpRenderBackend->SetTextureFilter(unit, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
}

//----------------------------------------------------------------------------

static inline void BP_SetTextureNoClamp(DG_TEX_BP* pTex, int unit)
{
   BP_DG_SetTexture(pTex, unit);
   gpRenderBackend->SetTextureFilter(unit, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
}

//----------------------------------------------------------------------------
// TODO: this function will be going away very soon in favor of DG_TEX_BP* version
static inline void BP_SetTexture(DG_TEX* pTex, int unit)
{
   BP_SetTextureNoClamp(pTex, unit);
   BP_GS_SetClamp(unit, pTex->tex_trans.clamp.data);
}

//----------------------------------------------------------------------------

static inline void BP_SetTexture(DG_TEX_BP* pTex, int unit)
{
   BP_SetTextureNoClamp(pTex, unit);
   BP_GS_SetClamp(unit, pTex->tex_trans.clamp.data);
}

//----------------------------------------------------------------------------

static inline void BP_KPPersMatrix_to_VS_Matrix44(float const * input, float * output)
{
   // this is what the code below does
   // CMatrix4* pInputPersMatrix = (CMatrix4*)input;
   // CMatrix4 result = pInputPersMatrix->ScaleThenTranslate(CVector3(1, -1, 0.5f), CVector3(0, 0, 0.5f));
   // transpose
   // BP_Matrix44_to_VS_Matrix44((float const*)&result, output);

   output[0]  =   input[0];
   output[1]  =   input[4];
   output[2]  =   input[8];
   output[3]  =   input[12];

   output[4]  =  -input[1];
   output[5]  =  -input[5];
   output[6]  =  -input[9];
   output[7]  =  -input[13];

   output[8]  =  0.5f*(input[2]  + input[3]);
   output[9]  =  0.5f*(input[6]  + input[7]);
   output[10] =  0.5f*(input[10] + input[11]);
   output[11] =  0.5f*(input[14] + input[15]);

   output[12] =   input[3];
   output[13] =   input[7];
   output[14] =   input[11];
   output[15] =   input[15];
}

//----------------------------------------------------------------------------

static inline void BP_KPPersMatrix_to_Matrix44(float const * input, float * output)
{
   // this is what the code below does
   // CMatrix4* pInputPersMatrix = (CMatrix4*)input;
   // CMatrix4* result = (CMatrix4*)output;
   // *result = pInputPersMatrix->ScaleThenTranslate(CVector3(1, -1, 0.5f), CVector3(0, 0, 0.5f));

   output[0]  =   input[0];
   output[4]  =   input[4];
   output[8]  =   input[8];
   output[12] =   input[12];

   output[1]  =  -input[1];
   output[5]  =  -input[5];
   output[9]  =  -input[9];
   output[13] =  -input[13];

   output[2]  =  0.5f*(input[2]  + input[3]);
   output[6]  =  0.5f*(input[6]  + input[7]);
   output[10] =  0.5f*(input[10] + input[11]);
   output[14] =  0.5f*(input[14] + input[15]);

   output[3]  =   input[3];
   output[7]  =   input[7];
   output[11] =   input[11];
   output[15] =   input[15];
}

//----------------------------------------------------------------------------
static inline int BP_IsValidVitaShaderCombination(int const blendMode, int const alphaTestFunc )
{
   if ( blendMode > 2 )
   {
      return 0;
   }
   else
   {
      if (alphaTestFunc == 1 || alphaTestFunc == 4 || alphaTestFunc == 6 || alphaTestFunc == 7)
         return 1;
      else
         return 0;
   }
}


//----------------------------------------------------------------------------
