//----------------------------------------------------------------------------
// BP_UIAdjust.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_UIAdjust.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

void BP_UI_AdjustSprite(int const originalViewWidth, int const originalViewHeight, 
                        int const newViewWidth, int const newViewHeight,
                        unsigned int bp_tex,
                        int* spriteX, int* spriteY, int* spriteWidth, int *spriteHeight,
                        int* texWidth, int* texHeight)
{
   CBaseTexture* pTexture = (CBaseTexture*)bp_tex;

   CBaseTexture::ESizeHint const sizeHint = pTexture->GetSizeHint();
   CBaseTexture::EAdjustHint const adjustHint = pTexture->GetHAdjustHint();

   int const bpTexWidth = pTexture->GetWidth();
   int const bpTexHeight = pTexture->GetHeight();

   switch(sizeHint)
   {
   case CBaseTexture::kSH_None:
      {
         // Simple rescale for new viewport settings
         real32 const xScale = (real32)newViewWidth / originalViewWidth;
         real32 const yScale = (real32)newViewHeight / originalViewHeight;

         *spriteX *= xScale;
         *spriteWidth *= xScale;

         *spriteY *= yScale;
         *spriteHeight *= yScale;
      }
      break;

   case CBaseTexture::kSH_AspectCorrect:
      {
         real32 xScale = (real32)newViewWidth / originalViewWidth;
         
         xScale *= (4.0f / 3.0f) / (16.0f / 9.0f);

         real32 yScale = (real32)newViewHeight / originalViewHeight;

         switch(adjustHint)
         {
         case CBaseTexture::kAH_Left:
            *spriteX *= xScale;
            break;
         
         case CBaseTexture::kAH_Center:
            *spriteX = (*spriteX - originalViewWidth / 2) * xScale + newViewWidth / 2;
            break;
         
         case CBaseTexture::kAH_Right:
            *spriteX = (*spriteX - originalViewWidth) * xScale + newViewWidth;
            break;
         }
         *spriteWidth *= xScale;

         *spriteY *= yScale;
         *spriteHeight *= yScale;
      }
      break;

   case CBaseTexture::kSH_PixelPerfect:
      {
         // Simple rescale for new viewport settings
         real32 const xScale = (real32)newViewWidth / originalViewWidth;
         real32 const yScale = (real32)newViewHeight / originalViewHeight;

         *spriteX *= xScale;
         *spriteWidth = *texWidth = bpTexWidth;

         *spriteY *= yScale;
         *spriteHeight = *texHeight = bpTexHeight;
      }
      break;
   }
}

//----------------------------------------------------------------------------

int BP_UI_IsPixelPerfect(unsigned int bp_tex)
{
   CBaseTexture* pTexture = (CBaseTexture*)bp_tex;

   CBaseTexture::ESizeHint const sizeHint = pTexture->GetSizeHint();
   
   return (sizeHint == CBaseTexture::kSH_PixelPerfect) ? 1 : 0;
}

//----------------------------------------------------------------------------

void BP_UI_GetTextureSize(unsigned int bp_tex, int * width, int * height)
{
   CBaseTexture* pTexture = (CBaseTexture*)bp_tex;
   *width = pTexture->GetWidth();
   *height = pTexture->GetHeight();
}

//----------------------------------------------------------------------------

float BP_AdjustWidescreenCenterX(float x, float width)
{
   float halfWidth = width * 0.5f;
   float val = x - halfWidth;
   val *= 3.0f / 4.0f;
   val += halfWidth;

   return val;
}

//----------------------------------------------------------------------------

void BP_FitToVirtualPS2Screen( int* outVWidth, int* outVHeight, int realPelWidth,
                               int realPelHeight )
{
   // determine the desired scale.
   CRenderBackend* renderBackend = RenderBackend();
   int bbWidth = renderBackend->GetBackBufferWidth();
   int bbHeight = renderBackend->GetBackBufferHeight();

   // fit to the height.
   float ratioY = ( float )bbHeight / ( float )realPelHeight;
   if ( ratioY * realPelWidth <= ( float )bbWidth )
   {
      *outVHeight = 424;
      *outVWidth = ( int )( 512.0f * ( ratioY * realPelWidth ) / ( float )bbWidth + 0.5f );
   }

   // fit to the width.
   float ratioX = ( float )bbWidth / ( float )realPelWidth;
   if ( ratioX * realPelHeight <= ( float )bbHeight )
   {
      *outVHeight = ( int )( 424.0f * ( ratioX * realPelHeight ) / ( float )bbHeight + 0.5f );
      *outVHeight = 512;
   }
/*
   // determine the desired area of the framebuffer that we want to fill.
   while ( 2 * realPelWidth <= bbWidth && 2 * realPelHeight <= bbHeight )
   {
      realPelWidth *= 2;
      realPelHeight *= 2;
   }

   // now determine the ratio of the framebuffer we want to fill.
   *outVWidth = 512 * realPelWidth / bbWidth;
   *outVHeight = 424 * realPelHeight / bbHeight;
*/
}
