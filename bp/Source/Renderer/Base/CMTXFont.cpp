//----------------------------------------------------------------------------
// CMTXFont.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <stdlib.h>
#include "CMTXFont.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"

#include "Engine/Math/CVector2.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Resource/CResourceFactory.h"

//-----------------------------------------------------------------------------

uint8 const CMTXFont::skImageTag = 0xFF;
uint8 const CMTXFont::skImageIndexOffset = 32;

//-----------------------------------------------------------------------------
// Custom vertex types for rendering text
//-----------------------------------------------------------------------------

#define MAX_NUM_VERTICES 1024*6
#define _T(x)  (x)

struct SFont3DVertex
{
   SFont3DVertex()
   : mP(CVector3::kConstructUninitialized)
   , mUV(CVector2::kConstructUninitialized)
   {
   }

   SFont3DVertex(CVector3 const &p, uint32 const color, CVector2 const &uv)
   : mP(p)
   , mColor(color)
   , mUV(uv)
   {
   }

   CVector3 mP;  
   uint32   mColor;
   CVector2 mUV;
};

static SFont3DVertex sFontVertices[MAX_NUM_VERTICES];

//-----------------------------------------------------------------------------
// Name: CMTXFont()
// Desc: Font class constructor
//-----------------------------------------------------------------------------
CMTXFont::CMTXFont(CInputStream &stream)
: m_dwFontHeight(stream.ReadUint32())   
, m_dwOverlap(stream.ReadUint32())   
, mRowHeight(stream.ReadReal32())
, m_fTexCoords(stream)
, mCharacterYAdjust(stream)
, mUTF8OneByteIndices(stream)
, mUTF8TwoByteIndices(stream)
, mUTF8TwoByteValue(stream)
, mUTF8ThreeByteIndices(stream)
, mUTF8ThreeByteValue(stream)
, mUTF8FourByteIndices(stream)
, mUTF8FourByteValue(stream)
, mImageIndices(stream)
, mTexture(RenderBackend()->ResourcePool()->GetResource(CResId(stream)))
, mShaderInstance(new CDefaultShader)
{
   mTexture.Lock();

   m_dwTexWidth = mTexture->GetWidth();
   m_dwTexHeight = mTexture->GetHeight();

   mShaderInstance->mBlendMode = CDefaultShader::kBM_Alpha;
   mShaderInstance->mZEnable = false;
   mShaderInstance->mpTexture = mTexture.GetPtr();
}

//-----------------------------------------------------------------------------
// Name: CMTXFont()
// Desc: Font class constructor
//-----------------------------------------------------------------------------
CMTXFont::CMTXFont(CInputStream &stream, TResource<CBaseTexture> & texture)
: m_dwFontHeight(stream.ReadUint32())   
, m_dwOverlap(stream.ReadUint32())   
, mRowHeight(stream.ReadReal32())
, m_fTexCoords(stream)
, mCharacterYAdjust(stream)
, mUTF8OneByteIndices(stream)
, mUTF8TwoByteIndices(stream)
, mUTF8TwoByteValue(stream)
, mUTF8ThreeByteIndices(stream)
, mUTF8ThreeByteValue(stream)
, mUTF8FourByteIndices(stream)
, mUTF8FourByteValue(stream)
, mImageIndices(stream)
, mTexture(texture)
, mShaderInstance(new CDefaultShader)
{
   mTexture.Lock();

   m_dwTexWidth = mTexture->GetWidth();
   m_dwTexHeight = mTexture->GetHeight();

   mShaderInstance->mBlendMode = CDefaultShader::kBM_Alpha;
   mShaderInstance->mZEnable = false;
   mShaderInstance->mpTexture = mTexture.GetPtr();
}

//-----------------------------------------------------------------------------
// Name: ~CMTXFont()
// Desc: Font class destructor
//-----------------------------------------------------------------------------

CMTXFont::~CMTXFont()
{
}

//-----------------------------------------------------------------------------
static int const skFontControlCodeStart = 0xA000;
static int const skCharacterWidthControlCodeStart = 0xA100;
static int const skCharacterScaleXControlCodeStart = 0xA200;
static int const skCharacterKerningOffsetCodeStart = 0xA300;

static bool bpe_font_process_string_control_code(uint8 const * pChars, uint16 &index, int &length)
{
   // find terminating character
   uint8 const *pTerminatingChar = pChars + 1;
   while (*pTerminatingChar != '#')
   {
      if (*pTerminatingChar == 0)
      {
         // End of string, with no terminating char, invalid
         return true;
      }
      pTerminatingChar++;
   }
   length = (int) ((pTerminatingChar - pChars) + 1); // Get control code length
   if (length == 2)
   {
      // 2 bytes '##', just a hash symbol
      index = '#';
      return false;
   }

   // Must be control code
   if ((length < 4) || (length > 6) )
   {
      // Incorrect length, need at least "#f0#" or max "#w255#"
      index = '?';
      return false;
   }

   uint16 controlCode = 0;
   uint8 const char1 = pChars[1];
   switch (char1)
   {
   case 'f':
      controlCode = skFontControlCodeStart;
      break;
   case 'w':
      controlCode = skCharacterWidthControlCodeStart;
      break;
   case 'k':
      controlCode = skCharacterKerningOffsetCodeStart;
      break;
   case 'x':
      controlCode = skCharacterScaleXControlCodeStart;
      break;
   default:
      // Don't know this control code, just use '?'
      index = '?';
      return false;
   }
   // Get integer value, 0-255
   char integerString[4];
   // Copy numbers
   strncpy(integerString, (char const *) pChars + 2, length - 3);
   // Terminate
   integerString[length - 3] = 0;
   int8 const controlCodeValue = atoi(integerString);

   uint16 const controlCode16Value = *(uint8*)&controlCodeValue;
   index = controlCode | controlCode16Value;
   return false;
}

//-----------------------------------------------------------------------------

bool CMTXFont::GetCharacterIndex(char const * const pString, uint16 &index, int &length) const
{
   uint8 const * pChars = (uint8 *) pString;
   uint8 const char0 = pChars[0];

   if (char0 == 0)
   {
      // Last character
      return true;
   }

   // Most common path
   if (char0 < 128)
   {
      // ASCII
      if (char0 != '#')
      {
         length = 1;
         index = mUTF8OneByteIndices[char0];
         return false;
      }
      else
      {
         // Check for control code
         return bpe_font_process_string_control_code(pChars, index, length);
      }
   }

   // Slow multi-character UTF-8 path

   // Check number of remaining bytes
   int remainingLength = 0;
   while (remainingLength < 4)
   {
      remainingLength++;
      if (pChars[remainingLength] == 0)
      {
         break;
      }
   }

   // See http://en.wikipedia.org/wiki/UTF-8 
   if (char0 < 0xE0)
   {
      if (char0 < 0xC2)
      {
         // Invalid code
         return true;
      }
      length = 2;
      if (remainingLength < length)
      {
         // Not enough chars
         return true;
      }
      uint16 characterCode = (char0 << 8) + pChars[1];
      // Find it in sorted vector
      std::vector<uint16>::const_iterator it = std::lower_bound(mUTF8TwoByteValue.begin(), mUTF8TwoByteValue.end(), characterCode);
      if ((it != mUTF8TwoByteValue.end()) && (*it == characterCode))
      {
         // Got a match
         index = mUTF8TwoByteIndices[it - mUTF8TwoByteValue.begin()];
      }
      else
      {
         // Unknown character
         index = '?';
      }
      return false;

   }
   else if (char0 < 0xF0)
   {
      length = 3;
      if (remainingLength < length)
      {
         // Not enough chars
         return true;
      }
      uint32 characterCode = (char0 << 16) + (pChars[1] << 8) + pChars[2];
      // Find it in sorted vector
      std::vector<uint32>::const_iterator it = std::lower_bound(mUTF8ThreeByteValue.begin(), mUTF8ThreeByteValue.end(), characterCode);
      if ((it != mUTF8ThreeByteValue.end()) && (*it == characterCode))
      {
         // Got a match
         index = mUTF8ThreeByteIndices[it - mUTF8ThreeByteValue.begin()];
      }
      else
      {
         // Unknown character
         index = '?';
      }
      return false;
   }
   else if (char0 < 0xF5)
   {
      length = 4;
      if (remainingLength < length)
      {
         // Not enough chars
         return true;
      }
      uint32 characterCode = (char0 << 24) + (pChars[1] << 16) + (pChars[2] << 8) + pChars[3];
      // Find it in sorted vector
      std::vector<uint32>::const_iterator it = std::lower_bound(mUTF8FourByteValue.begin(), mUTF8FourByteValue.end(), characterCode);
      if ((it != mUTF8FourByteValue.end()) && (*it == characterCode))
      {
         // Got a match
         index = mUTF8FourByteIndices[it - mUTF8FourByteValue.begin()];
      }
      else
      {
         // Unknown character
         index = '?';
      }
      return false;
   }
   else if (char0 < skImageTag)
   {
      // Restricted, treat as invalid
      return true;
   }
   else
   {
      // skImageTag, 0xFF - Internal image
      length = 2;
      if (remainingLength < length)
      {
         // Not enough chars
         return true;
      }
      uint8 const char1 = pChars[1];
      index = mImageIndices[char1 - skImageIndexOffset];
      return false;
   }

   return false;
}

//-----------------------------------------------------------------------------

SMTXFontControl::SMTXFontControl(uint32 const characterSpacing, uint32 const lineSpacing)
   : mCurrentFontIndex(0)
   , mOriginalCharacterSpacing(characterSpacing / 100.0f)
   , mCurrentCharacterSpacing(mOriginalCharacterSpacing)
   , mCurrentCharacterKerningOffset(0)
   , mCurrentLineSpacing(lineSpacing / 100.0f)
   , mCurrentCharacterXScale(1.0f)
   , mLastControlCode(0)
{  
}

//-----------------------------------------------------------------------------

static bool bpe_font_check_for_control_code(uint16 const character, SMTXFontControl &fontControl)
{
   int const controlCode = (character & 0xFF00);
   uint8 const value = (uint8) (character & 0xFF);
   switch (controlCode)
   {
   case skFontControlCodeStart:
      fontControl.mCurrentFontIndex = value;
      break;
   
   case skCharacterWidthControlCodeStart:
      fontControl.mCurrentCharacterSpacing = fontControl.mOriginalCharacterSpacing * (value / 100.0f);
      break;
   
   case skCharacterScaleXControlCodeStart:
      fontControl.mCurrentCharacterXScale = value / 100.0f;
      break;

   case skCharacterKerningOffsetCodeStart:
      fontControl.mCurrentCharacterKerningOffset = (real32)(*(int8*)&value) / 10.0f;
      break;

   default:
      BPE_ASSERT(controlCode < skFontControlCodeStart, "Invalid font control code.");
      return false;
   }
   fontControl.mLastControlCode = controlCode;
   return true;
}

//-----------------------------------------------------------------------------

bool CMTXFont::CheckForControlCode(uint16 const character, SMTXFontControl &fontControl)
{
   return bpe_font_check_for_control_code(character, fontControl);
}

//-----------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: Get the dimensions of a text string
//-----------------------------------------------------------------------------

bool CMTXFont::GetTextExtent( const char* strText, CVector2 &size, CMTXFont const** pFonts, SMTXFontControl const &fontControl )
{
   if(NULL==strText)
      return false;

   SMTXFontControl fontControlCopy(fontControl); 

   real32 fRowWidth  = 0.0f;
   real32 fWidth     = 0.0f;
   real32 fHeight    = 0.0f;

   bool bAddRowHeight = true;
   while( return_true())
   {
      uint16 charIndex;
      int charLength;
      if (pFonts[fontControlCopy.mCurrentFontIndex]->GetCharacterIndex(strText, charIndex, charLength))
      {
         // Last char in string
         break;
      }

      strText += charLength;
      if (bpe_font_check_for_control_code(charIndex, fontControlCopy))
      {
         // Control code
         continue;
      }

      if (charIndex == '\n')
      {
         fRowWidth = 0.0f;
         fHeight += pFonts[fontControlCopy.mCurrentFontIndex]->GetRowHeight() * fontControlCopy.mCurrentLineSpacing;
         continue;
      }

      if(charIndex < 32)
         continue;

      // Add first row height here as it might have been scaled by control code
      if (bAddRowHeight)
      {
         fHeight += pFonts[fontControlCopy.mCurrentFontIndex]->GetRowHeight() * fontControlCopy.mCurrentLineSpacing;
         bAddRowHeight = false;
      }

      fRowWidth += pFonts[fontControlCopy.mCurrentFontIndex]->GetCharacterWidth(charIndex) * fontControlCopy.mCurrentCharacterSpacing * fontControlCopy.mCurrentCharacterXScale;

      if(fRowWidth > fWidth)
         fWidth = fRowWidth;
   }

   size.mX = fWidth;
   size.mY = fHeight;

   return true;
}

//-----------------------------------------------------------------------------

static void setup_render_text_renderstate(CMTXFont const &font, uint32 const dwFlags)
{
   font.mShaderInstance->Bind(NULL);
   font.mShaderInstance->BeginSinglePassRender();
}

//----------------------------------------------------------------------------

static void reset_render_text_renderstate(CMTXFont const &font)
{
   font.mShaderInstance->EndSinglePassRender();
}

//-----------------------------------------------------------------------------
// Name: DrawText()
// Desc: Draws 2D text. Note that sx and sy are in pixels
//-----------------------------------------------------------------------------
bool CMTXFont::RenderText(real32 sx, real32 sy, CColor const color, const char* strText, uint32 dwFlags) const
{
   if(strlen(strText) == 0) return true;

   uint32 const dwColor = color.GetUint32_PlatformSpecific();

   mShaderInstance->mHasColorStream = true;

   RenderBackend()->SetScreenSpaceOrtho();

   setup_render_text_renderstate(*this, dwFlags);

   int const calculateLineWidth = dwFlags & (kRF_CenteredX|kRF_JustifyRight);

   // Center the text block in the viewport
   real32 lineWidth = 0.0f;
   if(calculateLineWidth)
   {
      const char* strTextTmp = strText;

      SMTXFontControl tempFontControl;
      while( return_true())
      {
         uint16 charIndex;
         int charLength;
         if (GetCharacterIndex(strTextTmp, charIndex, charLength))
         {
            // Last char in string
            break;
         }

         strTextTmp += charLength;
         if (bpe_font_check_for_control_code(charIndex, tempFontControl) && (tempFontControl.mLastControlCode == skFontControlCodeStart))
         {
            // Changed font, not supported
            break;
         }

         if (charIndex == '\n')
         {
            break;  // Isn't supported.  
         }

         lineWidth += GetCharacterWidth(charIndex);
      }

   }

   if( dwFlags & kRF_CenteredX)
   {
      sx = (RenderBackend()->GetViewWidth() - lineWidth) / 2.0f;
   }
   else if(dwFlags & kRF_JustifyRight )
   {
      sx -= lineWidth;
   }

   if(dwFlags & kRF_CenteredY)
   {
      real32 fLineHeight = mRowHeight;
      sy = (RenderBackend()->GetViewHeight()-fLineHeight)/2;
   }

   // Adjust for character spacing
   sx -= m_dwOverlap;
   real32 fStartX = sx;

   // Fill vertex buffer
   SFont3DVertex * pVertices = sFontVertices;

   // Fill vertex buffer
   uint32         numPrimitives = 0;

   real32 const kZ = 0.0f;

   // Deal with different rasterization rules under ogl/directx.
   // Hmm, still some problems, oh well, it's only debug text anyway.
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   real32 const kPOX = -0.5f;
   real32 const kPOY = -0.5f;
#else
   real32 const kPOX = 0.0f;
   real32 const kPOY = 0.0f;
#endif

   SMTXFontControl fontControl;
   
   CShaderVertexDataBinding vertexDataBinding;
   vertexDataBinding.Set(kVDU_Position, kVDS_Position);
   vertexDataBinding.Set(kVDU_Color0, kVDS_Color0);
   vertexDataBinding.Set(kVDU_TexCoord0, kVDS_TexCoord0);

   while( return_true() )
   {
      uint16 charIndex;
      int charLength;
      if (GetCharacterIndex(strText, charIndex, charLength))
      {
         // Last char in string
         break;
      }

      strText += charLength;
      if (bpe_font_check_for_control_code(charIndex, fontControl) && (fontControl.mLastControlCode == skFontControlCodeStart))
      {
         // Changed font, not supported
         break;
      }

      if(charIndex == '\n')
      {
         sx = fStartX;
         sy += mRowHeight;
      }

      if(charIndex < 32)
         continue;

      real32 tx1 = m_fTexCoords[charIndex * 4 + 0];
      real32 ty1 = m_fTexCoords[charIndex * 4 + 1];
      real32 tx2 = m_fTexCoords[charIndex * 4 + 2];
      real32 ty2 = m_fTexCoords[charIndex * 4 + 3];

      real32 w = (tx2-tx1) * m_dwTexWidth;
      real32 h = (ty2-ty1) * m_dwTexHeight;

      if(charIndex != _T(' '))
      {
         *pVertices++ = SFont3DVertex( CVector3(sx-0-kPOX,sy+h-kPOY,kZ), dwColor, CVector2(tx1, ty2) );
         *pVertices++ = SFont3DVertex( CVector3(sx-0-kPOX,sy+0-kPOY,kZ), dwColor, CVector2(tx1, ty1) );
         *pVertices++ = SFont3DVertex( CVector3(sx+w-kPOX,sy+h-kPOY,kZ), dwColor, CVector2(tx2, ty2) );
         *pVertices++ = SFont3DVertex( CVector3(sx+w-kPOX,sy+0-kPOY,kZ), dwColor, CVector2(tx2, ty1) );
         *pVertices++ = SFont3DVertex( CVector3(sx+w-kPOX,sy+h-kPOY,kZ), dwColor, CVector2(tx2, ty2) );
         *pVertices++ = SFont3DVertex( CVector3(sx-0-kPOX,sy+0-kPOY,kZ), dwColor, CVector2(tx1, ty1) );

         numPrimitives += 2;

         if(numPrimitives*3 > (MAX_NUM_VERTICES-6))
         {
            RenderBackend()->RenderPrimitivesUserVertexData(CMeshChunk::kPrimitive_TriangleList, vertexDataBinding, reinterpret_cast<real32*>(sFontVertices), numPrimitives * 3);

            pVertices = sFontVertices;
            numPrimitives = 0L;
         }
      }

      sx += w - (2 * m_dwOverlap);
   }

   // Unlock and render the vertex buffer

   mTexture->SetTexture( 0 );
   if(numPrimitives > 0)
      RenderBackend()->RenderPrimitivesUserVertexData(CMeshChunk::kPrimitive_TriangleList, vertexDataBinding, reinterpret_cast<real32*>(sFontVertices), numPrimitives * 3);

   reset_render_text_renderstate(*this);

   return true;
}




//-----------------------------------------------------------------------------
// Name: Render3DText()
// Desc: Renders 3D text
//-----------------------------------------------------------------------------

void CMTXFont::BeginRender3DText( const char* strText, CVector2 & start, CVector2 & size, CMTXFont const** pFonts, uint32 dwFlags, uint32 characterSpacing, uint32 lineSpacing)
{
   // Position for each text element
   start = CVector2(0.0f, 0.0f);

   size = CVector2::Zero();
   SMTXFontControl fontControl(characterSpacing, lineSpacing);
   GetTextExtent(strText, size, pFonts, fontControl);

   // Center the text block at the origin (not the viewport)
   if(dwFlags & kRF_CenteredX)
   {
      start.SetX( (size.mX / 10.0f) / 2.0f );
   }

   if(dwFlags & kRF_CenteredY)
   {
      start.SetY( -(size.mY / 10.0f) / 2.0f );
   }
   else if(dwFlags & kRF_JustifyBottom)
   {
      start.SetY( size.mY / 10.0f );
   }
   else
   {
      start.SetY(0.0f);
   }
}

//-----------------------------------------------------------------------------
// Assumes buffer has already been allocated
static void bpe_convert_to_indices(const char* strText, CMTXFont const** pFonts, uint16 * pDestBuffer)
{
   SMTXFontControl fontControl;
   while( return_true())
   {
      uint16 charIndex;
      int charLength;
      if (pFonts[fontControl.mCurrentFontIndex]->GetCharacterIndex(strText, charIndex, charLength))
      {
         // Last char in string
         break;
      }

      strText += charLength;
      bpe_font_check_for_control_code(charIndex, fontControl);

      if ((charIndex < 32) && (charIndex != '\n'))
         continue;

      *pDestBuffer = charIndex;
      pDestBuffer++;
   }
   *pDestBuffer = 0;
}

//-----------------------------------------------------------------------------

void CMTXFont::EndRender3DText(const char* strText, 
                               CVector2 const & start, 
                               CVector2 const & size, 
                               CMTXFont const** pFonts, 
                               uint32 dwFlags, 
                               uint32 characterSpacing, 
                               uint32 lineSpacing)
{
   SMTXFontControl fontControl(characterSpacing, lineSpacing);

   real32 x = start.GetX();
   real32 y = start.GetY();

   CMTXFont const * pCurrentFont = pFonts[fontControl.mCurrentFontIndex];
   pCurrentFont->mTexture->SetTexture(0);

   // Adjust for character spacing
   x += pCurrentFont->m_dwOverlap / 10.0f;
   real32 fStartX = x;

   // Fill vertex buffer
   SFont3DVertex fontVertices[MAX_NUM_VERTICES];

   uint32         dwNumTriangles = 0L;

   uint16 * pStrCpy = (uint16*) alloca(strlen(strText) * 2 + 2);
   bpe_convert_to_indices(strText, pFonts, pStrCpy);

   bool const bShouldCalculateLineOffset = ((dwFlags & kRF_CreditsText) != 0) || ((dwFlags & kRF_JustifyRight) != 0);
   bool bRecalculateLineOffset = bShouldCalculateLineOffset;
   SFont3DVertex * pVertices = fontVertices;
   
   int c;

   CShaderVertexDataBinding vertexDataBinding;
   vertexDataBinding.Set(kVDU_Position, kVDS_Position);
   vertexDataBinding.Set(kVDU_Color0, kVDS_Color0);
   vertexDataBinding.Set(kVDU_TexCoord0, kVDS_TexCoord0);

   while((c = *pStrCpy++) != 0)
   {
      // calculate line offset
      if( bRecalculateLineOffset )
      {
         bRecalculateLineOffset = false;

         real32 lineWidth = 0.0f;

         // calculate line width
         {
            SMTXFontControl tempFontControl(fontControl);

            uint16 const * pTemp = pStrCpy - 1;
            while(pTemp[0] != 0 && pTemp[0] != '\n')
            {
               int tempChar = *pTemp;
               ++pTemp;

               if (bpe_font_check_for_control_code(tempChar, tempFontControl))
               {
                  // Control code, skip
                  continue;
               }

               if( dwFlags & kRF_FixedWidth )
               {
                  real32 const deltaX = (pCurrentFont->mRowHeight / 10.0f) * fontControl.mCurrentCharacterSpacing;
                  lineWidth += deltaX;
               }
               else
               {
                  lineWidth += tempFontControl.mCurrentCharacterXScale * pFonts[tempFontControl.mCurrentFontIndex]->GetCharacterDeltaX(tempChar, tempFontControl.mCurrentCharacterSpacing);
               }
            }
         }

         // adjust line offset based on settings and width of line
         if ((dwFlags & kRF_CreditsText) != 0)
         {
            // Center line of text for credits
            real32 const kReferencePaneWidth = 1280.0f;
            x -= ((kReferencePaneWidth / 10.0f) - lineWidth);
            x *= 0.5f;
         }
         else
         {
            // Right justify
            x += lineWidth;
         }
      }

      // Check for font change
      {
         if (bpe_font_check_for_control_code(c, fontControl))
         {
            // Might have changed font
            CMTXFont const * newFont = pFonts[fontControl.mCurrentFontIndex];

            // only do the switch if the new font is actually different
            if( newFont != pCurrentFont )
            {
               // flush any triangles up until this point
               if( dwNumTriangles > 0 )
               {
                  // render the vertex buffer
                  RenderBackend()->RenderPrimitivesUserVertexData(CMeshChunk::kPrimitive_TriangleList, vertexDataBinding, reinterpret_cast<real32*>(fontVertices), dwNumTriangles * 3);
                  pVertices = fontVertices;
                  dwNumTriangles = 0L;
               }

               pCurrentFont = newFont;
               pCurrentFont->mTexture->SetTexture(0);
            }
            continue;
         }
      }

      // check if new line, means we must recalculate line x offset on the next character
      if(c == '\n')
      {
         bRecalculateLineOffset = bShouldCalculateLineOffset;
         x = fStartX;
         y -= pCurrentFont->mRowHeight/10.0f * fontControl.mCurrentLineSpacing;
      }

      if(c < 32)
         continue;

      real32 const tx1 = pCurrentFont->m_fTexCoords[c * 4 + 0];
      real32 const ty1 = pCurrentFont->m_fTexCoords[c * 4 + 1];
      real32 const tx2 = pCurrentFont->m_fTexCoords[c * 4 + 2];
      real32 const ty2 = pCurrentFont->m_fTexCoords[c * 4 + 3];

      real32 const originalCharacterWidth = (tx2-tx1) * pCurrentFont->m_dwTexWidth  / 10.0f;
      real32 const w = fontControl.mCurrentCharacterXScale * originalCharacterWidth;
      
      real32 const originalCharacterHeight = (ty2 - ty1) * pCurrentFont->m_dwTexHeight / 10.0f;
      real32 const h = originalCharacterHeight;

      real32 const overlapOffset = (2 * pCurrentFont->m_dwOverlap) / 10.0f;
      real32 const scaledOverlapOffset = fontControl.mCurrentCharacterXScale * overlapOffset;
      
      real32 deltaX = w * fontControl.mCurrentCharacterSpacing - scaledOverlapOffset;
      
      if( dwFlags & kRF_FixedWidth )
      {
         deltaX = (pCurrentFont->mRowHeight / 10.0f) * fontControl.mCurrentCharacterSpacing;
      }

      real32 const yAdjust = pCurrentFont->mCharacterYAdjust[c] / 10.0f;
      real32 const yOffset = (pCurrentFont->mRowHeight / 10.0f - h) * 0.5f - yAdjust;
      
      if(c != ' ')
      {
         *pVertices++ = SFont3DVertex( CVector3(x-0,0,y + yOffset + 0), 0xffffffff, CVector2(tx1, ty2) );
         *pVertices++ = SFont3DVertex( CVector3(x-0,0,y + yOffset + h), 0xffffffff, CVector2(tx1, ty1) );
         *pVertices++ = SFont3DVertex( CVector3(x-w,0,y + yOffset + 0), 0xffffffff, CVector2(tx2, ty2) );
         *pVertices++ = SFont3DVertex( CVector3(x-w,0,y + yOffset + h), 0xffffffff, CVector2(tx2, ty1) );
         *pVertices++ = SFont3DVertex( CVector3(x-w,0,y + yOffset + 0), 0xffffffff, CVector2(tx2, ty2) );
         *pVertices++ = SFont3DVertex( CVector3(x-0,0,y + yOffset + h), 0xffffffff, CVector2(tx1, ty1) );
         dwNumTriangles += 2;

         if( dwNumTriangles*3 > (MAX_NUM_VERTICES-6) )
         {
            // render the vertex buffer
            RenderBackend()->RenderPrimitivesUserVertexData(CMeshChunk::kPrimitive_TriangleList,vertexDataBinding, reinterpret_cast<real32*>(fontVertices), dwNumTriangles * 3);
            pVertices = fontVertices;
            dwNumTriangles = 0L;
         }
      }
      
      x -= deltaX;
   }

   if(dwNumTriangles > 0)
   {
      RenderBackend()->RenderPrimitivesUserVertexData(CMeshChunk::kPrimitive_TriangleList, vertexDataBinding, reinterpret_cast<real32*>(fontVertices), dwNumTriangles * 3);
   }
}

//-----------------------------------------------------------------------------

void CMTXFont::FFontFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   CMemoryInputStream stream(buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App);
   uint32 const magicNumber = stream.ReadUint32();
   BPE_VERIFY(magicNumber == 'FONT', false, "Invalid header for FONT resource.");

   uint32 const version = stream.ReadUint32();
   BPE_VERIFY(version == 5, false, "Invalid version number for FONT resource, recook fonts with latest tools.");

   returnResource.mpResource = new CMTXFont(stream);
}

//-----------------------------------------------------------------------------


