#include "stdafx.h"

#if MGS_VERSION == 3
#include "common.h"
#include "../../mode/eq_menu/primctrl.h"
#endif

#include "BP_Renderer.h"
#include "BP_Font.h"

#include "transfarring_menu_data.h"
#include "transfarring_menu_rendering.h"
#include "Transfarring_UI_Strings.h"

#define TEX_BASE0		(BUFFER_PAGE(2))					// VRAM texture area. 3rd page.
#define CLUT_BASE		(BUFFER_PAGE(3) - 64 * 32)			// VRAM clut area.

#define TEXT_WIDTH_ADJUSTMENT_DELTA (-0.16f)

#define TEXT_SHADOW_OFFSET_X (2)
#define TEXT_SHADOW_OFFSET_Y (2)
#define TEXT_SHADOW_COLOR (0x00202020)

#define BP_FONT_LINE_SPACING (2)

static int skPrimBufferSize = 24000;

#if MGS_VERSION == 2
static const unsigned int skTriCode = 0x009deed4; /*GV_StrCode("save_load")*/

// Map MGS3 DMA function names to MGS2 names
#define DG_DmapackSetViewPort(addr, x, y, w, h) DG_SetDmapackWindow(addr, 0, 0, DRAW_WIDTH, DRAW_HEIGHT)
#define DG_DmapackSetViewMapping(addr, x0, y0, x1, y1) DG_SetDmapackViewMapping(addr, x0, y0, x1, y1)
#define DG_DmapackSetAlpha(addr, alpha) DG_SetDmapackAlpha(addr, alpha)
#define DG_DmapackSetBox(addr, x0, y0, x1, y1, rgba) DG_SetDmapackBox_F(addr, x0, y0, x0 + x1, y0 + y1, rgba)
#define DG_DmapackSetLine(addr, x0, y0, rgba0, x1, y1, rgba1) DG_SetDmapackLine_F(addr, x0, y0, rgba0, x1, y1, rgba1)
#define DG_DmapackSetTexture(addr, tex) DG_SetDmapackTex(addr, tex)
#define DG_DmapackSetCoordinateScale(addr, uScale, vScale) (addr)
#define DG_DmapackSetTextureDynamic(addr, tex, bp_texture) DG_SetDmapackTextureDynamic01(addr, tex, bp_texture)

#define TWEAK_COLOR(color) ((((color) >> 1) & 0x007F7F7F) | ((color) & 0xFF000000))

typedef struct {
   DG_DMAPACK		*dmapack ;
   int				tri_id ;
   DG_TEXTURE_LIST	*tex_list ;
   int				current_offset ;
   int				end_offset ;
   u_long128		*buffer[2] ;
   u_long128		*current_buffer ;
} PRIM_CONTROL ;

/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

static void PRIMCTRL_MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id )
{
   DG_DMAPACK	*dmapack ;
   u_long128	*buffer ;

   dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, prio );
   DG_QueueDmapack( dmapack );

   buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
   prim_ctrl->buffer[0] = &buffer[0] ;
   prim_ctrl->buffer[1] = &buffer[buffer_size] ;

   //prim_ctrl->tri_id = tri_id ;
   //prim_ctrl->tex_list = DG_GetTextureList( tri_id );

   dmapack->autopacket = buffer ;

   prim_ctrl->current_offset = 0 ;
   prim_ctrl->end_offset = buffer_size ;
   prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
}

static void PRIMCTRL_FreePrimControl( PRIM_CONTROL *prim_ctrl )
{
   DG_DequeueDmapack( prim_ctrl->dmapack );
   DG_FreeDmapack( prim_ctrl->dmapack );
   GV_DelayedFree( prim_ctrl->buffer[0] );
}


static void PRIMCTRL_ClosePrimControl( PRIM_CONTROL *prim_ctrl )
{
   DG_SetDmapackEnd( prim_ctrl->current_buffer );
}

static void PRIMCTRL_ResetPrimControl( PRIM_CONTROL *prim_ctrl )
{
   //int		size ;

   if ( prim_ctrl->current_offset >= ( prim_ctrl->end_offset - 1 ) ){
      printf("%s: prim buffer over error!!!\n", __FILE__);
   }
   prim_ctrl->current_offset = 0 ;
   prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;

   PRIMCTRL_ClosePrimControl( prim_ctrl );
}
#endif // MGS_VERSION == 2
#if MGS_VERSION == 3
#define BASE_COLOR_SPRITES 0x80404040
#define BASE_COLOR_LINES 0x80808080
static float const skVertexScale = 1.0f;
static float const skTexScale = 1.0f/16.0f;
static const unsigned int skTriCode = 0x00fdea15; // memcard_pal_0_slot_memcard-normal
#endif

static int const skRowHeight = 40;
static int const skCellWidth = 30;

static int const skCharWidths[] =
{
   14, // " "
   30, // !
   30, // "
   30, // #
   30, // $
   30, // %
   30, // &
   30, // '
   30, // (
   30, // )
   30, // *
   30, // +
   15, // ,
   30, // -
   15, // .
   30, // /
   30, // 0
   30, // 1
   30, // 2
   30, // 3
   30, // 4
   30, // 5
   30, // 6
   30, // 7
   30, // 8
   30, // 9
   15, // :
   15, // ;
   30, // <
   30, // =
   30, // >
   30, // ?
   30, // @
   30, // A
   30, // B
   30, // C
   28, // D
   30, // E
   28, // F
   30, // G
   30, // H
   15, // I
   30, // J
   30, // K
   30, // L
   30, // M
   30, // N
   30, // O
   28, // P
   30, // Q
   30, // R
   30, // S
   28, // T
   30, // U
   30, // V
   30, // W
   30, // X
   30, // Y
   30, // Z
   30, // [
   30, // '\'
   30, // ]
   30, // ^
   30, // _
   30, // `
   28, // a
   26, // b
   26, // c
   26, // d
   26, // e
   21, // f
   26, // g
   26, // h
   14, // i
   30, // j
   25, // k
   15, // l
   30, // m
   24, // n
   25, // o
   26, // p
   26, // q
   20, // r
   26, // s
   20, // t
   25, // u
   26, // v
   27, // w
   30, // x
   30, // y
   26, // z
   30, // {
   30, // |
   30, // }
   30, // ~
   30, // ¡
   30, // ¦
   30, // ©
   32, // ®
   30, // ´
   30, // ¿
   30, // À
   30, // Á
   30, // Â
   30, // Ã
   30, // Ä
   30, // Å
   30, // Æ
   30, // Ç
   30, // È
   30, // É
   30, // Ê
   30, // Ë
   30, // Ì
   30, // Í
   30, // Î
   30, // Ï
   30, // Ð
   30, // Ñ
   30, // Ò
   30, // Ó
   30, // Ô
   30, // Õ
   30, // Ö
   30, // Ù
   30, // Ú
   30, // Û
   30, // Ü
   30, // Ý
   30, // Þ
   30, // ß
   28, // à
   28, // á
   28, // â
   28, // ã
   28, // ä
   28, // å
   30, // æ
   30, // ç
   26, // è
   26, // é
   26, // ê
   26, // ë
   14, // ì
   14, // í
   14, // î
   14, // ï
   24, // ñ
   25, // ò
   25, // ó
   25, // ô
   25, // õ
   25, // ö
   25, // ù
   25, // ú
   25, // û
   25, // ü
   30, // ý
   30, // ÿ
   30, // ™
};
static int const skWidthTableSize = sizeof(skCharWidths) / sizeof(skCharWidths[0]);
static int const skAsciiCharShiftOffset = ' ';

#if MGS_VERSION == 3
static float const skUvScaleDivisor = 960.0f;
#endif
#if MGS_VERSION == 2
static float const skUvScaleDivisor = 1.0f;
#endif

static unsigned char skExtAsciiToTextureIndex[] = 
{
   0, // €
   0, // 
   0, // ‚
   0, // ƒ
   0, // „
   0, // …
   0, // †
   0, // ‡
   0, // ˆ
   0, // ‰
   0, // Š
   0, // ‹
   0, // Œ
   0, // 
   0, // Ž
   0, // 
   0, // 
   0, // ‘
   0, // ’
   0, // “
   0, // ”
   0, // •
   0, // –
   0, // —
   0, // 
   0, // ™
   0, // š
   0, // ›
   0, // œ
   0, // 
   0, // ž
   0, // Ÿ
   0, // 
   95, // ¡
   0, // ¢
   0, // £
   0, // ¤
   0, // ¥
   96, // ¦
   0, // §
   0, // ¨
   97, // ©
   0, // ª
   0, // «
   0, // ¬
   0, // 
   98, // ®
   0, // ¯
   0, // °
   0, // ±
   0, // ²
   0, // ³
   99, // ´
   0, // µ
   0, // ¶
   0, // ·
   0, // ¸
   0, // ¹
   0, // º
   0, // »
   0, // ¼
   0, // ½
   0, // ¾
   100, // ¿
   101, // À
   102, // Á
   103, // Â
   104, // Ã
   105, // Ä
   106, // Å
   107, // Æ
   108, // Ç
   109, // È
   110, // É
   111, // Ê
   112, // Ë
   113, // Ì
   114, // Í
   115, // Î
   116, // Ï
   117, // Ð
   118, // Ñ
   119, // Ò
   120, // Ó
   121, // Ô
   122, // Õ
   123, // Ö
   0, // ×
   0, // Ø
   124, // Ù
   125, // Ú
   126, // Û
   127, // Ü
   128, // Ý
   129, // Þ
   130, // ß
   131, // à
   132, // á
   133, // â
   134, // ã
   135, // ä
   136, // å
   137, // æ
   138, // ç
   139, // è
   140, // é
   141, // ê
   142, // ë
   143, // ì
   144, // í
   145, // î
   146, // ï
   0, // ð
   147, // ñ
   148, // ò
   149, // ó
   150, // ô
   151, // õ
   152, // ö
   0, // ÷
   0, // ø
   153, // ù
   154, // ú
   155, // û
   156, // ü
   157, // ý
   0, // þ
   158, // ÿ  
};

static int const skExtAsciiToTextureIndexTableSize = sizeof(skExtAsciiToTextureIndex) / sizeof(skExtAsciiToTextureIndex[0]);
static int const skExtAsciiCharShiftOffset =  (unsigned char)'€';

typedef struct STransfarringGlobalRenderingData
{
   PRIM_CONTROL mPrimControl;
   void *mpPrimBuffer;
   DG_TEX *mpCustomTextTexture;
}
SRData;

typedef struct  
{
   DG_TEX mDgTex;
   BP_FONT_VRAMINFO  mVinfo;
   int mBpTextVramIndex;
   unsigned int   mBpTextVram[2];
   
   int mBpTextChanged;
   
   float mAlignmentOffsetX;
}
SEpTextData;

static void StartScene(PRIM_CONTROL *prim_ctrl)
{
   PRIMCTRL_ResetPrimControl(prim_ctrl);

   prim_ctrl->current_buffer = DG_DmapackSetViewPort( prim_ctrl->current_buffer, 0, 0, DRAW_WIDTH, DRAW_HEIGHT );
   prim_ctrl->current_buffer = DG_DmapackSetViewMapping( prim_ctrl->current_buffer, 0.0f, 0.0f, 960.0f, 544.0f );

#if MGS_VERSION == 3
   prim_ctrl->current_buffer = DG_DmapackSetCoordinateScale( prim_ctrl->current_buffer, skVertexScale, skTexScale );
   prim_ctrl->current_buffer = DG_DmapackSetOriginMatrix( prim_ctrl->current_buffer, 0, 0, 0, 256 );
#endif
}

static void EndScene(PRIM_CONTROL *prim_ctrl)
{
   PRIMCTRL_ClosePrimControl(prim_ctrl);
#if MGS_VERSION == 3 && defined(BP_VITA)
   PRIMCTRL_FlipBuffer(prim_ctrl);
#endif
}

static void DrawFilledBox(PRIM_CONTROL *prim_ctrl, float x, float y, float w, float h, unsigned int color, EBoxFlags flags)
{
#if MGS_VERSION == 3
   prim_ctrl->current_buffer = DG_DmapackSetBaseColor(prim_ctrl->current_buffer, DG_MakeDmaPackColorFromInt(BASE_COLOR_LINES));
   
   if(flags & kFadeOutLeftToRight)
   {
      int const colorWithoutAlpha = color & 0x00FFFFFF;
      
      prim_ctrl->current_buffer = DG_DmapackSetQuad(prim_ctrl->current_buffer,
         x, y, DG_MakeDmaPackColorFromInt(color),
         x + w, y, DG_MakeDmaPackColorFromInt(colorWithoutAlpha),
         x + w, y + h, DG_MakeDmaPackColorFromInt(colorWithoutAlpha),
         x, y + h, DG_MakeDmaPackColorFromInt(color));
   }
   else
   {
      prim_ctrl->current_buffer = DG_DmapackSetBox(prim_ctrl->current_buffer, x, y, w, h, DG_MakeDmaPackColorFromInt(color));
   }
#else
   if(flags & kFadeOutLeftToRight)
   {
      //TODO: Implement
   }
   else
   {
      prim_ctrl->current_buffer = DG_DmapackSetBox(prim_ctrl->current_buffer, x, y, w, h, DG_MakeDmaPackColorFromInt(color));
   }
#endif
}

static void DrawOutlinedBox(PRIM_CONTROL *prim_ctrl, float x, float y, float w, float h, unsigned int color)
{
   DG_DMAPACK_COLOR lineColor = DG_MakeDmaPackColorFromInt(color);

#if MGS_VERSION == 3
   prim_ctrl->current_buffer = DG_DmapackSetBaseColor(prim_ctrl->current_buffer, DG_MakeDmaPackColorFromInt(BASE_COLOR_LINES));
#endif

   prim_ctrl->current_buffer = DG_DmapackSetLine(prim_ctrl->current_buffer, x,     y,     lineColor, x + w, y,     lineColor);
   prim_ctrl->current_buffer = DG_DmapackSetLine(prim_ctrl->current_buffer, x + w, y,     lineColor, x + w, y + h, lineColor);
   prim_ctrl->current_buffer = DG_DmapackSetLine(prim_ctrl->current_buffer, x + w, y + h, lineColor, x,     y + h, lineColor);
   prim_ctrl->current_buffer = DG_DmapackSetLine(prim_ctrl->current_buffer, x,     y + h, lineColor, x,     y,     lineColor);
}

static void DrawLine(PRIM_CONTROL *prim_ctrl, float x, float y, float w, float h, unsigned int color)
{
   DG_DMAPACK_COLOR lineColor = DG_MakeDmaPackColorFromInt(color);

#if MGS_VERSION == 3
   prim_ctrl->current_buffer = DG_DmapackSetBaseColor(prim_ctrl->current_buffer, DG_MakeDmaPackColorFromInt(BASE_COLOR_LINES));
#endif
   if (w > h)
   {
      prim_ctrl->current_buffer = DG_DmapackSetLine(prim_ctrl->current_buffer, x,     y,     lineColor, x + w, y,     lineColor);
   }
   else
   {
      prim_ctrl->current_buffer = DG_DmapackSetLine(prim_ctrl->current_buffer, x,     y + h, lineColor, x,     y,     lineColor);
   }
}

static int Utf8ToFontTextureIndex(int *textureIndex, char const * const text)
{
   int bytesRead = 0;
   char const byte_0 = *text;
   
   if(byte_0)
   {
      if(byte_0 >= 0)
      {
         *textureIndex = byte_0 - skAsciiCharShiftOffset;
         bytesRead = 1;
      }
      else if(byte_0 == (char)0xC2 || byte_0 == (char)0xC3)
      {
         char const byte_1 = *(text + 1);
         
         if(byte_1)
         {
            //byte_0   = ---- --xx
            //byte_1   = --yy yyyy
            //*outChar = xxyy yyyy

            int const extAsciiIndex = ((byte_0 & 0x1F) << 6 | (byte_1 & 0x3F)) - skExtAsciiCharShiftOffset;
            
            if(extAsciiIndex >= 0 && extAsciiIndex < skExtAsciiToTextureIndexTableSize && skExtAsciiToTextureIndex[extAsciiIndex])
            {
               *textureIndex = skExtAsciiToTextureIndex[extAsciiIndex];
               bytesRead = 2;
            }
         }
      }
      //Special case for ™
      else if(byte_0 == (char)0xE2 && *(text + 1) == (char)0x84 && *(text + 2) == (char)0xA2)
      {
         *textureIndex = skWidthTableSize - 1;
         bytesRead = 3;
      }

      if(!bytesRead)
      {
         //Unsupported Character
         *textureIndex = ' ';
         bytesRead = 1;
         XASSERT(0, "Utf8ToLatin1() - Unsupported Character");
      }
   }
   return bytesRead;
}

static void PrintText(PRIM_CONTROL *prim_ctrl, float x, float y, unsigned int color, float scale, char const *text, DG_TEX *texture)
{
   if(text && *text)
   {
      float const kXScale = skUvScaleDivisor / 960.0f;
      float const kYScale = skUvScaleDivisor / 200.0f;

      char const *currentChar = text;
      float currentX = x;
      float currentY = y;
      
#if MGS_VERSION == 3
      prim_ctrl->current_buffer = DG_DmapackSetBaseColor(prim_ctrl->current_buffer, DG_MakeDmaPackColorFromInt(BASE_COLOR_SPRITES));
#endif
#if MGS_VERSION == 2
      unsigned int tweakedColor = TWEAK_COLOR(color);
#endif
      prim_ctrl->current_buffer = DG_DmapackSetCoordinateScale(prim_ctrl->current_buffer, skVertexScale, 1.0f / skUvScaleDivisor);
      prim_ctrl->current_buffer = DG_DmapackSetTexture(prim_ctrl->current_buffer, texture);
      prim_ctrl->current_buffer = DG_DmapackSetAlpha(prim_ctrl->current_buffer, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));

      while(*currentChar)
      {
         int index, row, column;
   
         currentChar = currentChar + Utf8ToFontTextureIndex(&index, currentChar);

         index = (index < 0 || index >= skWidthTableSize) ? 0 : index;
         row = index / 32;
         column = index % 32;
         {
#if MGS_VERSION == 3
            float const u_0 = column * skCellWidth * kXScale + 0.5f;
            float const v_0 = row * skRowHeight * kYScale + 0.5f;
            float const u_1 = u_0 + skCharWidths[index] * kXScale + 0.5f;
            float const v_1 = v_0 + skRowHeight * kYScale + 0.5f;

            prim_ctrl->current_buffer = DG_DmapackSetSprt(prim_ctrl->current_buffer, currentX + 0.5f, currentY + 0.5f,
               skCharWidths[index] * scale + 0.5f, skRowHeight * scale + 0.5f, u_0, v_0, u_1, v_1, DG_MakeDmaPackColorFromInt(color));
#endif

#if MGS_VERSION == 2
            float const u_0 = column * skCellWidth * kXScale;
            float const v_0 = row * skRowHeight * kYScale;
            float const u_1 = u_0 + skCharWidths[index] * kXScale;
            float const v_1 = v_0 + skRowHeight * kYScale;

#if defined(BP_PS3)
            prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, currentX, currentY, u_0, v_0,
               currentX + skCharWidths[index] * scale, currentY + skRowHeight * scale, u_1, v_1, DG_MakeDmaPackColorFromInt(tweakedColor), SPR_FLAG_PIXELPERFECT);
#else
            prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, currentX, currentY, u_0, v_0,
               currentX + skCharWidths[index] * scale, currentY + skRowHeight * scale, u_1, v_1, DG_MakeDmaPackColorFromInt(tweakedColor), 0);
#endif
#endif
            currentX += (skCharWidths[index] + skCharWidths[index] * TEXT_WIDTH_ADJUSTMENT_DELTA) * scale;
         }
      }
      prim_ctrl->current_buffer = DG_DmapackSetCoordinateScale( prim_ctrl->current_buffer, skVertexScale, skTexScale );
   }
}

static void PrintBpText(PRIM_CONTROL *prim_ctrl, float x, float y, float w, float h, unsigned int color, float scale, ETextFlags flags,
                        SEpTextData *pEpTextData, char const *text)
{
   if(text && *text)
   {
      int const texWidth = w > BP_FONT_SIZE_W ? w : BP_FONT_SIZE_W;
      int const texHeight = h > BP_FONT_SIZE_H ? h : BP_FONT_SIZE_H;

#if BP_VITA
      // vita font is 3/4 PS3 font
      scale *= 4.0f/3.0f;
#endif

      if(pEpTextData->mBpTextChanged)
      {
         BP_FONT_DRAWINFO dr;

         pEpTextData->mBpTextChanged = 0;
         
         pEpTextData->mBpTextVramIndex = 1 - pEpTextData->mBpTextVramIndex;

         BP_font_set_vraminfo_texture(&pEpTextData->mVinfo, pEpTextData->mBpTextVram[pEpTextData->mBpTextVramIndex], 0, BP_FONT_LINE_SPACING, 0);

         BP_font_begin_render_texture(&pEpTextData->mVinfo, 1);
         BP_font_open_drawinfo(&dr, &pEpTextData->mVinfo);
         dr.xtop = 0;
         dr.ytop = 0;

         //Let the quad deal with alpha to avoid morphing distorting the font color. 
         BP_font_set_color(&dr, color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, 0x80);
         BP_font_draw_string(&dr, (char*)text);
         BP_font_end_render_texture(&pEpTextData->mVinfo);

         if(flags & kAlignCenter)
         {
            int const actualWidth = BP_font_get_draw_width(&dr);
            pEpTextData->mAlignmentOffsetX = -(actualWidth / 2.0f) * scale;
         }
         else if(flags & kAlignRight)
         {
            int const actualWidth = BP_font_get_draw_width(&dr);
            pEpTextData->mAlignmentOffsetX = -actualWidth * scale;
         }
         else
         {
            pEpTextData->mAlignmentOffsetX = 0.0f;
         }
      }
#if MGS_VERSION == 3
      prim_ctrl->current_buffer = DG_DmapackSetBaseColor(prim_ctrl->current_buffer, DG_MakeDmaPackColorFromInt(BASE_COLOR_LINES));
#endif
      prim_ctrl->current_buffer = DG_DmapackSetTextureDynamic(prim_ctrl->current_buffer, &pEpTextData->mDgTex, pEpTextData->mBpTextVram[pEpTextData->mBpTextVramIndex]);
      {
         int const shadowOffsetY = TEXT_SHADOW_OFFSET_Y * scale;
         float const shadowOffsetX = TEXT_SHADOW_OFFSET_X * scale;
         int const alpha = color & 0xff000000;
         DG_DMAPACK_COLOR const spriteColor = DG_MakeDmaPackColorFromInt(0x00808080 | alpha);
         float const spriteX = x + pEpTextData->mAlignmentOffsetX;
         float const spriteY = y;
         float const spriteWidth = w * scale;
         float const spriteHeight = h * scale;

#if MGS_VERSION == 2
         float const u0 = 0.5f / texWidth;
         float const v0 = 0.5f / texHeight;
#if defined(BP_PS3)
         if(flags & kDropShadow)
         {
            prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, spriteX + shadowOffsetX, spriteY + shadowOffsetY, u0, v0,
               spriteX + spriteWidth, spriteY + spriteHeight, 1.0f + u0, 1.0f + v0, DG_MakeDmaPackColorFromInt(TEXT_SHADOW_COLOR | alpha), SPR_FLAG_PIXELPERFECT);
         }
         prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, spriteX, spriteY, u0, v0,
            spriteX + spriteWidth, spriteY + spriteHeight, 1.0f + u0, 1.0f + v0, spriteColor, SPR_FLAG_PIXELPERFECT);
#else
         if(flags & kDropShadow)
         {
            prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, spriteX + shadowOffsetX, spriteY + shadowOffsetY, u0, v0,
               spriteX + spriteWidth, spriteY + spriteHeight, 1.0f + u0, 1.0f + v0, DG_MakeDmaPackColorFromInt(TEXT_SHADOW_COLOR | alpha), 0);
         }
         prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, spriteX, spriteY, u0, v0,
            spriteX + spriteWidth, spriteY + spriteHeight, 1.0f + u0, 1.0f + v0, spriteColor, 0);
#endif
#elif MGS_VERSION == 3
         int const u0 = 8;
         int const v0 = 8;

         if(flags & kDropShadow)
         {
            prim_ctrl->current_buffer = DG_DmapackSetSprt(prim_ctrl->current_buffer, spriteX + shadowOffsetX, spriteY + shadowOffsetY,
               spriteWidth, spriteHeight, u0, v0, u0 + texWidth * 16, v0 + texHeight * 16, DG_MakeDmaPackColorFromInt(TEXT_SHADOW_COLOR | alpha));
         }
         prim_ctrl->current_buffer = DG_DmapackSetSprt(prim_ctrl->current_buffer, spriteX, spriteY,
            spriteWidth, spriteHeight, u0, v0, u0 + texWidth * 16, v0 + texHeight * 16, spriteColor);
#endif
      }
   }
}


static void DrawSprite(PRIM_CONTROL *prim_ctrl, float x, float y, float w, float h, unsigned int color, DG_TEX *tex)
{
   prim_ctrl->current_buffer = DG_DmapackSetTexture(prim_ctrl->current_buffer, tex);
   prim_ctrl->current_buffer = DG_DmapackSetAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

#if MGS_VERSION == 2
#if defined(BP_PS3)
   prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, x, y, 0, 0, x + w, y + h, 1.0f, 1.0f, DG_MakeDmaPackColorFromInt(TWEAK_COLOR(color)), SPR_FLAG_PIXELPERFECT);
#else
   prim_ctrl->current_buffer = DG_SetDmapackSprt_F(prim_ctrl->current_buffer, x, y, 0, 0, x + w, y + h, 1.0f, 1.0f, DG_MakeDmaPackColorFromInt(TWEAK_COLOR(color)), 0);
#endif
#endif
#if MGS_VERSION == 3
   prim_ctrl->current_buffer = DG_DmapackSetBaseColor(prim_ctrl->current_buffer, DG_MakeDmaPackColorFromInt(BASE_COLOR_SPRITES));
   prim_ctrl->current_buffer = DG_DmapackSetSprt(prim_ctrl->current_buffer, x, y, w, h, 0, 0, 16, 16, DG_MakeDmaPackColorFromInt(color));
#endif
}

static void RenderRecursive(SRData *data, SMenuObject *obj, float px, float py, unsigned int palpha)
{
   PRIM_CONTROL *prim_ctrl = &data->mPrimControl;

   // draw object
   unsigned int alpha = ((obj->curStatus.col>>24)*palpha)>>7; // not 8 because 0x80 is max

   if (alpha > 0)
   {
      unsigned int col = (alpha<<24)|(obj->curStatus.col & 0xffffff);
      float x = px + obj->curStatus.x;
      float y = py + obj->curStatus.y;
      float w = obj->curStatus.w;
      float h = obj->curStatus.h;
      SMenuObject *child = obj->pFirstChild;
      switch (obj->type)
      {
      case kNull:
         break;
      case kFBox:
         DrawFilledBox(prim_ctrl, x, y, w, h, col, obj->boxFlags);
         break;
      case kOBox:
         DrawOutlinedBox(prim_ctrl, x, y, w, h, col);
         break;
      case kLine:
         DrawLine(prim_ctrl, x, y, w, h, col);
         break;
      case kText:
         PrintText(prim_ctrl, x, y, col, obj->scale, obj->textId == kTString_USE_OVERRIDE
            ?  obj->textOverride : GetTransfarringString(obj->textId), data->mpCustomTextTexture);
         break;
      case kSprt:
         DrawSprite(prim_ctrl, x, y, w, h, col, obj->tex[0]);
         break;
      case kAnim:
         DrawSprite(prim_ctrl, x, y, w, h, col, obj->tex[obj->currentFrame]);
         break;
      case kBpText:
         PrintBpText(prim_ctrl, x, y, w, h, col, obj->scale, obj->textFlags, (SEpTextData*)obj->pMiscData,
            obj->textId == kTString_USE_OVERRIDE ? obj->textOverride : GetTransfarringString(obj->textId));
         break;
      }

      while (child != NULL)
      {
         RenderRecursive(data, child, x, y, alpha);
         child = child->pSibling;
      }
   }
}

//----------------------------------------------------------------------------------------------------------
void Transfarring_RenderScene(SRData *data)
{
   PRIM_CONTROL *prim_ctrl = &data->mPrimControl;

   StartScene(prim_ctrl);
   RenderRecursive(data, &gTransfarringMenuObjects[0], 0, 0, 128);
   EndScene(prim_ctrl);

#if MGS_VERSION == 3
   if ((unsigned int)prim_ctrl->current_buffer > (unsigned int)prim_ctrl->end_buffer)
   {
      unsigned int overrun = (unsigned int)prim_ctrl->current_buffer - (unsigned int)prim_ctrl->end_buffer;
      char errormsg[128] = { 0 };
      sprintf(errormsg, "Buffer overrun by %d bytes in transfarring_menu.c!\n", overrun);
      printf(errormsg);
      XASSERT(0, errormsg);
   }
#endif
}

void Transfarring_InitMenuObjectRenderingData(SMenuObject *obj)
{
   if((obj->type == kAnim || obj->type == kSprt) && obj->textureName != NULL)
   {
      char textureName[256];
      
      //Use '!' to mark a texture for localization
      if(obj->textureName[0] == '!')
      {
         sprintf(textureName, "%s%s", &obj->textureName[1], GetLanguageDependentImageStringPostfix());
      }
      else
      {
         strcpy(textureName, obj->textureName);
      }

      if (obj->type == kAnim)
      {
         int texIndex = 0;
         char frameName[128];

         do
         {
            sprintf(frameName, "%s_%02d", textureName, texIndex);
            obj->tex[texIndex] = DG_GetTexture2(skTriCode, GV_StrCode(frameName));
            if(obj->tex[texIndex])
            {
               obj->tex[texIndex]->BP_flag |= DG_TEXFLAG_UV_CLAMP;
            }
         }
         while (obj->tex[texIndex] && ++texIndex < MAX_ANIM_FRAMES);

         obj->currentFrame = 0;
      }
      else
      {
         obj->tex[0] = DG_GetTexture2(skTriCode, GV_StrCode(textureName));
         if (obj->tex[0])
         {
            obj->tex[0]->BP_flag |= DG_TEXFLAG_UV_CLAMP;
         }
         else
         {
            printf("Transfarring menu: missing texture %s!\n", obj->textureName);            
         }
      }
   }
   else if(obj->type == kBpText)
   {
      SEpTextData *pEpTextData = (SEpTextData*)GV_Malloc(sizeof(SEpTextData));
      DG_TEX *tex = &pEpTextData->mDgTex;
      DG_TEX_TRANS *tex_trans = &tex->tex_trans;

      int texWidth = obj->refStatus.w > BP_FONT_SIZE_W ? obj->refStatus.w : BP_FONT_SIZE_W;
      int texHeight = obj->refStatus.h > BP_FONT_SIZE_H ? obj->refStatus.h : BP_FONT_SIZE_H;

      obj->pMiscData = pEpTextData;

      pEpTextData->mBpTextVramIndex = 0;
      pEpTextData->mBpTextVram[0] = BP_AllocDynamicTexture(texWidth, texHeight, 1);
      pEpTextData->mBpTextVram[1] = BP_AllocDynamicTexture(texWidth, texHeight, 1);
      pEpTextData->mBpTextChanged = 1;

      /* Initialize texture. */
      tex->u_offset = 0.0f;
      tex->v_offset = 0.0f;
      tex->u_scale = 1.0f;
      tex->v_scale = 1.0f;
      tex->BP_flag = DG_TEXFLAG_UV_CLAMP;
      tex_trans->giftag.tag  = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 7);
      tex_trans->giftag.regs = 0x0ffeeeee;
#if MGS_VERSION == 3
      tex_trans->tex1.data = SCE_GS_SET_TEX1(1, 0, 1, 1, 0, 0, 0);									/* Mag filter: LINEAR; Min filter: LINEAR */
      tex_trans->tex1.reg  = SCE_GS_TEX1_1;
#endif
      tex_trans->tex2.data = 0;
      tex_trans->tex2.reg  = SCE_GS_TEX2_1;
      tex_trans->tex0.data = SCE_GS_SET_TEX0(TEX_BASE0 / 64, texWidth / 64,
         SCE_GS_PSMT4, 10, 10, 1, 0, CLUT_BASE / 64, SCE_GS_PSMCT32, 0, 0, 1);						/* 4 bpp tex; Tex area: 1024 * 1024; Tex function: MODULATE; 32 bpp clut */
      tex_trans->tex0.reg  = SCE_GS_TEX0_1;
      tex_trans->alpha.data = SCE_GS_SET_ALPHA(0, 1, 0, 1, 128);										/* (Cs - Cd) * As >> 7 + Cd */
      tex_trans->alpha.reg  = SCE_GS_ALPHA_1;
      tex_trans->clamp.data = SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0);	/* S wrap: REGION CLAMP; T wrap: REGION CLAMP */
      tex_trans->clamp.reg  = SCE_GS_CLAMP_1;
   }
}

void Transfarring_DestroyMenuObjectRenderingData(SMenuObject *obj)
{
   if(obj->type == kBpText)
   {
      SEpTextData *pEpTextData = (SEpTextData*)obj->pMiscData;

      BP_FreeDynamicTexture(pEpTextData->mBpTextVram[0]);
      BP_FreeDynamicTexture(pEpTextData->mBpTextVram[1]);

      GV_Free(pEpTextData);
   }
}

SRData* Transfarring_InitRenderingData()
{
   SRData *data = (SRData*)GV_Malloc(sizeof(SRData));

#if MGS_VERSION == 2
   PRIMCTRL_MakePrimControl(&data->mPrimControl, skPrimBufferSize, 64, 0);
   PRIMCTRL_ResetPrimControl(&data->mPrimControl);
#endif

#if MGS_VERSION == 3
   //PRIMCTRL_MakePrimControl double buffers
   data->mpPrimBuffer = GV_Malloc(skPrimBufferSize * 2);
   PRIMCTRL_MakePrimControl(&data->mPrimControl, DG_SCENE_MENU, DG_DMAPACK_PHASE_AFTER, DMA_PRIO_TRANSFARRING, DG_DMAPACK_MENU, data->mpPrimBuffer, skPrimBufferSize, 0);
   PRIMCTRL_ResetPrimControl(&data->mPrimControl);
#endif
   data->mpCustomTextTexture = DG_GetTexture2(skTriCode, GV_StrCode("transfarring_font"));
   if(data->mpCustomTextTexture)
   {
      data->mpCustomTextTexture->BP_flag |= DG_TEXFLAG_UV_CLAMP;
   }
   return data;
}

void Transfarring_DestroyRenderingData(SRData *data)
{
#if MGS_VERSION == 2
   PRIMCTRL_FreePrimControl(&data->mPrimControl);
#endif

#if MGS_VERSION == 3
   GV_Free(data->mpPrimBuffer);
   PRIMCTRL_FreePrimControl(&data->mPrimControl);
#endif

   GV_Free(data);
}

void Transfarring_FlagBpTextChanged(SMenuObject *obj)
{
   if(obj && obj->type == kBpText && obj->pMiscData)
   {
      SEpTextData *bpTextData = (SEpTextData*)obj->pMiscData;
      bpTextData->mBpTextChanged = 1;
   }
}

float Transfarring_GetApproximatedTextWidth(float scale, char const * const text)
{
   char const *currentChar = text;
   float width = 0.0f;

   while(*currentChar)
   {
      int index;
      currentChar = currentChar + Utf8ToFontTextureIndex(&index, currentChar);
      index = (index < 0 || index >= skWidthTableSize) ? 0 : index;
      width += (skCharWidths[index] + skCharWidths[index] * TEXT_WIDTH_ADJUSTMENT_DELTA) * scale;
   }
   return width;
}
