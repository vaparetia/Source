//----------------------------------------------------------------------------
// TextureCooker.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "stdafx.h"

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Math/CVector3.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Renderer/Base/Backend/CTexture.h"

#include "IL/il.h"
#include "IL/ilu.h"

//----------------------------------------------------------------------------

namespace TextureCooker
{
   enum EGenerateMipMaps
   {
      kGM_No,
      kGM_Yes,
      // TODO: Add kGM_Yes_NoGamma for linear textures

      kGM_Count
   };

   enum EColorConversion
   {
      kCC_NormalMap_DXT5,
      kCC_YCoCg_DXT5,
      kCC_None
   };

   class CTexMipMap 
   {
   public:
      CTexMipMap(ILuint imageHandle, uint32 mipMapLevel = 0)
      {
         mImageHandle = imageHandle; // No image? Hard to tell from the IL documentation.
         mMipMapLevel = mipMapLevel;
      }

      ILuint   mImageHandle;
      uint32   mMipMapLevel;
   };

   // List of mip maps for 1 face of a texture, usually only mip map level 0 is explicitly specified.
   // Mip maps are generally created in the platform specific cooker, but can be 
   // explicitly specified adding additional mip map levels.
   class CTextureFace
   {      
   public:
      CTextureFace(CTexMipMap const &mipMap)
      {
         mMipLevels.push_back(mipMap);
      }

      std::vector<CTexMipMap>    mMipLevels;
   };

   // 2D textures only have 1 face, but cube maps have 6.
   // 3D textures also only have 1 face as the depth is part of the source image handle.
   class CTextureToCook
   {
   public:
      CTextureToCook()
         : mMinRGBA(0xDEADBEEF)
         , mMaxRGBA(0xBEEFDEAD)
         , mNormalImageWidth( 0 )
         , mNormalImageHeight( 0 )
      {
         // Initialize to sensible defaults.
         mFormat = CBaseTexture::kFormat_A8R8G8B8;
         mDiscFormat = CBaseTexture::kDiscFormat_PTC;
         mGenerateMipMaps = kGM_Yes;
         mbCalculatedMinMaxRGBA = false;

         mFilterHint = CBaseTexture::kFH_Default;
         mAlphaRefValue = -1;

         mAdditionalFlags = 0;

         mMaxLODOffset = 0;
      }

      // Helper function for common case of adding top level image.
      int AddImage(ILuint imageHandle) 
      {
         mTextureFaces.push_back(CTexMipMap(imageHandle));
         // Return index of added tex face.
         return mTextureFaces.size() - 1;
      }

      // Helper function for common case of accessing top level image.
      ILint GetTopLevelImageHandle() const
      {
         // Main image handle is top level mip of first image.
         return mTextureFaces[0].mMipLevels[0].mImageHandle;
      }

      void SetNormalImageInputPath( char const *str, int const originalWidth, int const originalHeight ) { mNormalImageInputPath = str; mNormalImageWidth = originalWidth; mNormalImageHeight = originalHeight; }
      void ClearNormalImageInputPath() { mNormalImageInputPath.clear(); }

      void CalcMinMaxRGBA();
      bool HasAlpha() const;

      std::vector<CTextureFace>  mTextureFaces; // 1 for 2D, 6 for cube map.
      CBaseTexture::EFormat      mFormat;

      CBaseTexture::EDiscFormat  mDiscFormat;

      EGenerateMipMaps           mGenerateMipMaps;
      uint32                     mMinRGBA;
      uint32                     mMaxRGBA;
      uint32                     mMinRGBANonZero;
      bool                       mbCalculatedMinMaxRGBA;
      uint32                     mAdditionalFlags;

      CBaseTexture::EFilterHint  mFilterHint;
      int                        mAlphaRefValue;

      int                        mMaxLODOffset;

      std::string                mNormalImageInputPath;
      int                        mNormalImageWidth;
      int                        mNormalImageHeight;
   };

   struct SProcessImageOptions
   {
      SProcessImageOptions()
      {
         mColorConversion = kCC_None;
         mGenerateMipMaps = kGM_Yes;
         mTexFormat = CBaseTexture::kFormat_A8R8G8B8;
         mDiscFormat = CBaseTexture::kDiscFormat_PTC;
         mSkipMipLevels = 0;

         mFilterHint = CBaseTexture::kFH_Default;
         mAlphaRefValue = -1;

         mAdditionalFlags = 0;

         mMaxLODOffset = 0;
      }

      EColorConversion           mColorConversion;
      EGenerateMipMaps           mGenerateMipMaps;
      CBaseTexture::EFormat      mTexFormat;
      CBaseTexture::EDiscFormat  mDiscFormat;
      int                        mSkipMipLevels;

      uint32                     mAdditionalFlags;

      CBaseTexture::EFilterHint  mFilterHint;
      int                        mAlphaRefValue;

      int                        mMaxLODOffset;
   };

   bool WriteTextureRVL(std::string const & outputPath, CTextureToCook const & tex);
   bool WriteTextureWin32(std::string const & outputPath, CTextureToCook const & tex);
   bool WriteTexturePS3(std::string const & outputPath, CTextureToCook const & tex);
   bool WriteTextureX360(std::string const & outputPath, CTextureToCook const & tex, bool debugInfo);
   bool WriteTextureVTA(std::string const & outputPath, CTextureToCook const & tex);
}

