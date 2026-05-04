//----------------------------------------------------------------------------
// BP_RendererFX.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

enum EMonoType
{
   kMT_Mini,
   kMT_Scene,
   kMT_MiniGlare
};

typedef struct _SBP_PFX_Blinds
{
   u_long64 alphaData; 
   unsigned int color;

   int startY;
   int darkenHeight;
   int blankHeight;
} SBP_PFX_Blinds;

typedef struct _SBP_PFX_Mono
{
   FVECTOR        colorWeighting;
   int            monoType;
   unsigned int   palette[256];
} SBP_PFX_Mono;

#if MGS_VERSION == 3
typedef struct _SBP_PFX_ScopeBlur
{
   int invisible;
   u_long64 alphaData;
   float alphaFocusValue;
} SBP_PFX_ScopeBlur;
#endif

//----------------------------------------------------------------------------

void BP_InitTweakDebugMenu();
void BP_InitFXShader();
void BP_FX_InitDebugMenu();

#ifdef __cplusplus
extern "C" {
#endif

#if MGS_VERSION==2
#  define BP_FX_NEEDS_NOISE_TEXTURE 1
#endif

#if BP_FX_NEEDS_NOISE_TEXTURE
unsigned int GetRandomNoiseTexture();
unsigned char* GetRandomNoiseValues512x512();
#endif

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus

struct SRect;
struct SRectf;

void BP_YUVConversion(CBaseTexture* pY, CBaseTexture* pU, CBaseTexture* pV, CBaseTexture* pResult);
void BP_ResolveColorAndDepth(CBaseTexture* pColor, CBaseTexture* pDepth0, CBaseTexture* pDepth1, CBaseTexture* pDepth2, CBaseTexture* pDepth3, int doZReload, CBaseTexture::EAntiAliasType antiAliasType, SRectf const & uvRect);
void BP_DrawFullscreenTexture(CBaseTexture const * pTexture, int isFilter, int isRGBA, real32 const z = 1.0f);
void BP_DrawFullscreenTextureModulate(CBaseTexture const * pTexture, CVector4 const & modulate, int isFilter, int isRGBA, real32 const z = 1.0f);
void BP_DrawFullscreenTextureModulateSRect(CBaseTexture const * pTexture, CVector4 const & modulate, int isFilter, int isRGBA, SRect const & rect, real32 const z = 1.0f);
void BP_DrawFullscreenTextureModulateWithXTextures(const CBaseTexture** apTexture, int numTextures, CVector4 const & modulate, int isFilter, real32 const z = 1.0f);

void BP_DrawRectTextureModulate(CBaseTexture* pTexture, real32 const x, real32 const y, real32 const width, real32 const height, CVector4 const & modulate, CVector4 const & uvRect, int isFilter, int isRGBA);

void BP_Downsample(CBaseTexture* pSource, CBaseTexture* pDest);
void BP_Downsample2x(CBaseTexture* pSource, CBaseTexture* pDest);

void BP_DepthDownsample(CBaseTexture* pSource, CBaseTexture* pDest);

// Separable blur: pSource -> pTemp -> pDest (pSource and pDest can be the same!)
void BP_Blur(CBaseTexture* pSource, CBaseTexture *pDest, CBaseTexture* pTemp, real32 const weight0, real32 const weight12, real32 const weight34, real32 const weight56, real32 const pass1Scale, real32 const pass2Scale, bool doFinalResolve);
void BP_BlurSRect(CBaseTexture* pSource, CBaseTexture *pDest, CBaseTexture* pTemp, real32 const weight0, real32 const weight12, real32 const weight34, real32 const weight56, real32 const pass1Scale, real32 const pass2Scale, bool doFinalResolve, SRect const & srcRect, SRect const & dstRect);

#if BPE_TARGET == BPE_TARGET_PS3
void BP_CopyMemory(CBaseTexture* pSource, CBaseTexture* pDest, int const bilinear);
#endif

// Can pass NULL for pDest to copy to back buffer.
void BP_CopyTexture(CBaseTexture* pSource, CBaseTexture* pDest);
void BP_CopyTextureRect(CBaseTexture* pSource, CBaseTexture* pDest, int const x0, int const y0, int const x1, int const y1);
void BP_CopyTextureSRect(CBaseTexture* pSource, CBaseTexture* pDest, SRect const & srcRect, SRect const & dstRect);

void BP_ZCullReload(int clearStencil = 0);

//----------------------------------------------------------------------------

void BP_RsxInitUserCallback();

void BP_InitMLAA();
void BP_DestroyMLAA();
void BP_ClearMLAABuffer();

void BP_BeginMLAA(CBaseTexture* pInput);
void BP_FinalizeMLAA(CBaseTexture* pOutput);

#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
#define BP_EXTERN_C extern "C"
#else
#define BP_EXTERN_C
#endif

BP_EXTERN_C void BP_PostFx_Mono(void* pData);
BP_EXTERN_C void BP_PostFx_Blinds(void* pData);

#if MGS_VERSION == 2
BP_EXTERN_C void BP_PostFx_MGS2_FarFocus(void* pData);
BP_EXTERN_C void BP_PostFX_MGS2_VRClear(void* pData);
BP_EXTERN_C void BP_PostFX_MGS2_CodexInOut(void* pData);
BP_EXTERN_C void BP_PostFx_MGS2_ReduceScreen(void* pData);

#elif MGS_VERSION == 3
BP_EXTERN_C void BP_PostFx_ScopeBlur(void* pData);
BP_EXTERN_C void BP_PostFx_MGS3_GlareNew(void* pData);
BP_EXTERN_C void BP_PostFX_MGS3_GhostSmoke(void* pData);
BP_EXTERN_C void BP_PostFx_MGS3_FarFocus(void* pData);
BP_EXTERN_C void BP_PostFx_MGS3_Contrast(void* pData);
BP_EXTERN_C void BP_PostFx_MGS3_LensflareNew(void* pData);
BP_EXTERN_C void BP_PostFx_MGS3_LensflareNewMakeAlpha(void* pData);
BP_EXTERN_C void BP_PostFx_ShockwaveFlutter(void* pData);
void BP_PostFx_InitShockwave();

typedef struct _SBP_LensflareNewMakeAlpha
{
   unsigned int   u0, v0, u1, v1;
} SBP_LensflareNewMakeAlpha;

typedef struct _SBP_Flare
{
   float xywh[4];
   float uv0[2];
   float uv1[2];
   unsigned char color[4];
} SBP_Flare;

typedef struct _SBP_LensflareNew
{
   unsigned int   u0, v0, u1, v1;
   float z;
   unsigned int tex;
   SBP_Flare flares[32];
} SBP_LensflareNew;

#endif

BP_EXTERN_C void BP_FX_Halfres_MSAA(void* pData);
#if BPE_TARGET == BPE_TARGET_VITA
BP_EXTERN_C void BP_PostFx_Offscreen(void* pData);
void BP_PostFx_ResetDmapackFX();
#endif
//----------------------------------------------------------------------------

