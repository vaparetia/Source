//----------------------------------------------------------------------------
// CMTXFont.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/Shaders/CDefaultShader.h"
#include "Engine/StlExtras/vector_s.h"

//------------------------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class CBaseTexture;
class CInputStream;

//-----------------------------------------------------------------------------

struct RENDERER_API SMTXFontControl
{
   SMTXFontControl(uint32 const characterSpacing = 100, uint32 const lineSpacing = 100);

   int      mCurrentFontIndex;
   real32   mOriginalCharacterSpacing;
   real32   mCurrentCharacterSpacing;
   real32   mCurrentCharacterKerningOffset;
   real32   mCurrentCharacterXScale;
   real32   mCurrentLineSpacing;          // Currently doesn't vary
   uint16   mLastControlCode;
};

//-----------------------------------------------------------------------------
// Name: class CMTXFont
// Desc: Texture-based font class for doing text in a 3D scene.
//-----------------------------------------------------------------------------
class RENDERER_API CMTXFont
{
public:
   // Font rendering flags
   enum ERenderFlags
   {
      kRF_CenteredX           = 1 << 0,
      kRF_CenteredY           = 1 << 1,
      kRF_JustifyRight        = 1 << 2,
      kRF_JustifyBottom       = 1 << 3,
      kRF_CreditsText         = 1 << 4,
      kRF_FixedWidth          = 1 << 5
   };
   static uint8 const skImageTag;
   static uint8 const skImageIndexOffset;

   // Constructor / destructor
   CMTXFont( CInputStream &stream );
   CMTXFont( CInputStream &stream, TResource<CBaseTexture> & texture );
   virtual ~CMTXFont();

   // 2D and 3D text drawing functions
   bool RenderText(real32 x, 
                   real32 y, 
                   CColor const color, 
                   const char* strText, uint32 dwFlags=0L) const;   
   
   static void BeginRender3DText(const char* strText, CVector2 & start, CVector2 & size, CMTXFont const** pFonts, uint32 characterSpacing = 100, uint32 lineSpacing = 100, uint32 dwFlags=0L);
   static void EndRender3DText(const char* strText, CVector2 const & start, CVector2 const & size, CMTXFont const** pFonts, uint32 dwFlags=0L, uint32 characterSpacing = 100, uint32 lineSpacing = 100);
   
   // Function to get extent of text
   static bool GetTextExtent(const char* strText, CVector2 &size, CMTXFont const** pFonts, SMTXFontControl const &fontControl);

   real32 GetCharacterWidth(uint32 characterIdx) const
   {
      real32 tx1 = m_fTexCoords[characterIdx * 4 + 0];
      real32 tx2 = m_fTexCoords[characterIdx * 4 + 2];

      return (tx2-tx1)*m_dwTexWidth - 2*m_dwOverlap;
   }

   real32 GetCharacterDeltaX(uint32 characterIdx, real32 widthMultiplier) const
   {
      real32 tx1 = m_fTexCoords[characterIdx * 4 + 0];
      real32 tx2 = m_fTexCoords[characterIdx * 4 + 2];
      real32 w = (tx2-tx1) * m_dwTexWidth  / 10.0f;

      return w * widthMultiplier - (2 * m_dwOverlap) / 10.0f;
   }
   // Turns UTF-8 string into a uint16 character or image index. Also has special codes for font changes.
   // Returns 'true' if last character in string.
   bool GetCharacterIndex(char const * const pString, uint16 &index, int &length) const;

   int const GetFontHeight() const { return m_dwFontHeight; }
   real32 GetRowHeight() const { return mRowHeight; };

   static bool CheckForControlCode(uint16 const character, SMTXFontControl &fontControl);

   static void FFontFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);
public:
   uint32                  m_dwFontHeight;
   uint32                  m_dwOverlap;                  // Character pixel spacing per side
   real32                  mRowHeight;
   bpe::vector_s<real32>   m_fTexCoords;
   bpe::vector_s<int16>    mCharacterYAdjust;
   bpe::vector_s<uint16>   mUTF8OneByteIndices;
   bpe::vector_s<uint16>   mUTF8TwoByteIndices;
   bpe::vector_s<uint16>   mUTF8TwoByteValue;
   bpe::vector_s<uint16>   mUTF8ThreeByteIndices;
   bpe::vector_s<uint32>   mUTF8ThreeByteValue;
   bpe::vector_s<uint16>   mUTF8FourByteIndices;
   bpe::vector_s<uint32>   mUTF8FourByteValue;
   bpe::vector_s<uint16>   mImageIndices;

   TResource<CBaseTexture> mTexture;

   uint32                  m_dwTexWidth;                 // Texture dimensions
   uint32                  m_dwTexHeight;

   boost::shared_ptr<CDefaultShader> mShaderInstance;
};

//-----------------------------------------------------------------------------


