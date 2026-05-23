//------------------------------------------------------------------------------------------
// CTexture.h
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"

#include "Engine/Mechanics/IObject.h"

#if BPE_TARGET == BPE_TARGET_X360
#include <xtl.h>
#endif


//------------------------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class CColor;
class CVector2;
class CVector4;
class CBaseTexture;

//------------------------------------------------------------------------------------------

struct SCreateTextureParams
{
   SCreateTextureParams()
   {
#if BPE_TARGET == BPE_TARGET_PS3
      mCreateZCull = true;
      mZCullOffset = 0;
      mIsTiled = true;
#elif BPE_TARGET == BPE_TARGET_X360
      mBaseAddress = 0;
      mHierarchicalZBase = 0;
      mRenderTargetWidth = -1;
      mRenderTargetHeight = -1;
      mCreateRenderTargetTexture = 1;
      mCreateDepthTargetTexture = 1;
#elif BPE_TARGET == BPE_TARGET_VITA
      mBaseAddress = 0;
      mMSAATrick = 0;
      mNumScenes = 1;
      mXTiles = 0;
      mYTiles = 0;
      mStride = 0;
      mLoadStore = 3;
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
      mPVRTCFormat = 0;
#endif
      mClearMemory = 0;
   }

#if BPE_TARGET == BPE_TARGET_PS3
   bool  mCreateZCull;
   int   mZCullOffset;     // ZCull memory offset to use. (Must be 4096 byte aligned)
   bool  mIsTiled;
#elif BPE_TARGET == BPE_TARGET_X360
   unsigned int mBaseAddress;
   unsigned int mHierarchicalZBase;
   int mRenderTargetWidth;
   int mRenderTargetHeight;
   int mCreateRenderTargetTexture;
   int mCreateDepthTargetTexture;
#elif BPE_TARGET == BPE_TARGET_VITA
   unsigned int mBaseAddress;
   int mMSAATrick;
   int mNumScenes;
   int mXTiles;
   int mYTiles;
   int mStride;
   int mLoadStore;
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
   int mPVRTCFormat;   // 0 = PVRTC 4bpp, 1 = PVRTC 2bpp
#endif
   int mClearMemory;
};

//------------------------------------------------------------------------------------------

class RENDERER_API CBaseTexture : public IObject
{
   friend class CBaseRenderBackend;
   friend class CRenderBackend;
public:
   enum EGeneratedTextureType
   {
      kGTT_White,
      kGTT_Grey,
      kGTT_Black,
      kGTT_FlatNormal,
      kGTT_SpecularPower,
   };

   enum EAntiAliasType
   {
      kAA_None,
      kAA_MSAA2x,
      kAA_MSAA4x,

      kAA_Count,
      kAA_Invalid = -1,
      kAA_LastValid = kAA_Count - 1
   };

   enum EType
   {
      kType_Normal,
      kType_Cube
   };

   enum EUsage
   {
      kUsage_Texture,
      kUsage_TextureLinear,
      kUsage_RenderTarget,
      kUsage_DepthBuffer,

      kUsage_Invalid = -1
   };

   enum EFormat
   {
      kFormat_A8R8G8B8,
      kFormat_X8R8G8B8,

      kFormat_A16B16G16R16F,
      kFormat_R32F,
      
      kFormat_D24X8,
      
      kFormat_DXT1,
      kFormat_DXT3,
      kFormat_DXT5,
      kFormat_A32B32G32R32F,

      kFormat_Luminance8,

      kFormat_D24FS8,

      kFormat_YVU420P2_CSC1,

      // If adding anything, make sure to change
      // TextureFormatAsString
#if BPE_TARGET == BPE_TARGET_VITA
      kFormat_A8B8G8R8,
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
      kFormat_PVRTC4,   // 4bpp PVRTC - native PowerVR tile compression
      kFormat_PVRTC2,   // 2bpp PVRTC - higher compression, lower quality
#endif
      kFormat_Count,

      kFormat_Invalid = -1
   };

   enum EDiscFormat
   {
      kDiscFormat_Raw,
      kDiscFormat_PTC,

      kDiscFormat_Count,

      kDiscFormat_Invalid = -1
   };

   // 8 bits of additional cooker flags
   enum EAdditionalFlags
   {
      kAF_SizeHintBeginBit             = 10, // reserving 4 bits for future expansion of options.
      kAF_HorizJustificationBeginBit   = 14, // reserving 4 bits for future expansion of options.

      kAF_UsePS2MipmapSelection  =  (1 << 0),
      kAF_DisableAlphaFailHint   =  (1 << 1),
      kAF_AlphaRefOverride       =  (1 << 2),
      kAF_ForceClampU            =  (1 << 3),
      kAF_ForceClampV            =  (1 << 4),
      kAF_ForceBorderU           =  (1 << 5),
      kAF_ForceBorderV           =  (1 << 6),
      kAF_SystemMemory           =  (1 << 7),
      kAF_ForceDisableClutLerp   =  (1 << 8),
      kAF_ForceDisableDepthWrite =  (1 << 9),
      
      kAF_SizeHint               =  (1 << kAF_SizeHintBeginBit), 
      kAF_HorizJustificationHint =  (1 << kAF_HorizJustificationBeginBit),

      kAF_RTAlias                =  (1 << 18),
      //kAF_???                  =  (1 << 19),
   };

   // These flags are meant to be hints for rendering to determine how the texture should be displayed on screen.
   // Can expand this structure up to 16 options
   enum ESizeHint
   {
      kSH_None,                  // Default
      kSH_PixelPerfect,          // Render pixels 1 to 1 on screen
      kSH_AspectCorrect,         // Render texture adjusting for the aspect ratio so what used to be a square in 4:3 remains a square when rendered at 16:9.
   };

   // This determines how the primitive will be aligned on screen when any form of aspect correction occurs.
   // Can expand this structure up to 16 options
   enum EAdjustHint
   {
      kAH_None     = 0,
      kAH_Center   = 1,
      kAH_Left     = 2,
      kAH_Right    = 3
   };

   enum EFilterHint
   {
      kFH_Default    = -1,
      kFH_Point      = 0,
      kFH_Bilinear   = 1,
      kFH_Trilinear  = 2,
      kFH_Aniso      = 3
   };

   enum EForceLoadStore
   {
      kLS_Load       = 1,
      kLS_Store      = 2
   };

public:
   virtual ~CBaseTexture() {}

   static void DeleteTexture(CBaseTexture* pTexture);

   static CBaseTexture * CreateSolid(CColor const & color);

   static CBaseTexture * Create(void *pMemory, uint32 const size);

   static CBaseTexture * Create(EGeneratedTextureType const type);

   static CBaseTexture * CreateCheckerboard( int const width, int const height, const CColor & color0, const CColor & color1 );

   static CBaseTexture * Create(int const width, 
                                int const height,
                                int const levels,
                                EFormat const format,
                                EUsage const usage,
                                EAntiAliasType const aaFlag,
                                ERenderMemory const memory = kRM_Video,
                                SCreateTextureParams const * pCreateTextureParams = NULL);

   static CBaseTexture * TryCreate(int const width, 
                                   int const height,
                                   int const levels,
                                   EFormat const format,
                                   EUsage const usage,
                                   EAntiAliasType const aaFlag,
                                   ERenderMemory const memory = kRM_Video,
                                   SCreateTextureParams const * pCreateTextureParams = NULL);

   static CBaseTexture * CreateTexture( int const width, int const height, int const levels, EFormat const format, SCreateTextureParams const * pCreateTextureParams = NULL );

#if BPE_TARGET == BPE_TARGET_X360
   static CBaseTexture * Create(int const width, 
      int const height,
      int const levels,
      EFormat const format,
      EUsage const usage,
      EAntiAliasType const aaFlag,
      ERenderMemory const memory,
      D3DFORMAT const d3dFormat,
      D3DMULTISAMPLE_TYPE multisampleType,
      bool fillTexture,
      SCreateTextureParams const * pCreateTextureParams);
#endif

   bool const HasAlpha() const { return mHasAlpha; }

   // Factories
   static void FTextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);   
   static void FJPGTextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);   
   static void FPNGTextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);
   static void FTGATextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   static CBaseTexture * CreateFromPNG(void * pMemory, int const size);
   static CBaseTexture * CreateFromJPEG(void * pMemory, int const size);
   static CBaseTexture * CreateFromTGA(void * pMemory, int const size);

   bool LoadFromJPEG(void * pMemory, int size);
   bool SaveToJPEG(void ** pMemory, int * psize);

   bool SaveToPNG(void ** pMemory, int * psize);

   EFormat const GetFormat() const { return mFormat; }
   int const GetWidth() const;
   int const GetHeight() const;
   int const GetDepth() const;
   int GetMipCount() const;

   int const GetAntiAliasWidth(EAntiAliasType const type) const;
   int const GetAntiAliasHeight(EAntiAliasType const type) const;
   
   EAntiAliasType const GetAntiAliasType() const;

   ESizeHint GetSizeHint() const { return (ESizeHint)( (mAdditionalFlags >> kAF_SizeHintBeginBit) & 0xf); }
   EAdjustHint GetHAdjustHint() const { return (EAdjustHint)((mAdditionalFlags >> kAF_HorizJustificationBeginBit) & 0xf); }

   // This function directly sets the texture, it doens't change any of the filter settings currently associated with that stage.
   // This is useful if just want to override the texture used by the shader but maintain all the sampler settings from the shader itself.
   virtual void SetTexture(int const textureStage) const = 0;

   virtual void Lock(void** pMemory, int * pPitch) = 0;
   virtual void Unlock() = 0;
   virtual unsigned int GetEDRAMEnd() const { return 0; }
   virtual unsigned int GetEDRAMEndHierarchicalZ() const { return 0; }

   static CVector4 const GetSpecularPowerMapSample(CVector2 const & texCoord, CVector2 const & texelSize, void* data);
   static char const *TextureFormatAsString( EFormat const format );

   void CloneTexture(CBaseTexture* pSource);

   void SetCurrentTextureParams(int const textureStage) const;
   static int GetCurrentTextureWidth(int const textureStage);
   static int GetCurrentTextureHeight(int const textureStage);

protected:
   CBaseTexture(EFormat format, EType const type, EUsage usage);

   void CloneBaseTexture(CBaseTexture* pSource);

public:
   //IMPORTANT: Look at CloneTexture when adding/removing/modifying variables
   int      mIsResidentTexture;

#if BPE_TARGET == BPE_TARGET_WIN32 || defined(_DEBUG)
   std::string mDebugName;
#endif

   EFormat  mFormat;
   EType    mType;
   EUsage   mUsage;
   uint32   mAdditionalFlags;    // See EAdditionalFlags
   uint32   mMinRGBA;            // Contains minimum of each color component used by this texture
   uint32   mMaxRGBA;            // Contains maximum of each color component used by this texture
   int8     mFilterHint;
   uint8    mAlphaRefValue;
   int8     mMaxLODOffset;
   bool     mHasAlpha : 1;
   uint32   mCustomData;
   uint32   mMinRGBANonZero;
};

