//----------------------------------------------------------------------------
// BP_Font.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/System/COsContext.h"

#ifdef MGS_VERSION
#include "BP_BuildDefines.h"
#include "BP_EndianSupport.h"
#else
//Tool mode.  No endian swapping needed.
#define BP_LE_SwapSInt(x) (x)
#define BP_LE_SwapUInt_Inp(...) (void*)0
#endif

#include "BP_Font.h"

#include "FontRaw/MGS_FONT.raw.h"

//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "Renderer/Base/Backend/CTexture.h"

#if defined( MGS_VERSION )
#include "BP_Renderer.h"
#endif

//----------------------------------------------------------------------------

#define BP_OUTPUT_CHECKER_BOARD() 0

//----------------------------------------------------------------------------

#ifndef BPE_IS_ENDIAN_LITTLE
   #define BPE_IS_ENDIAN_LITTLE() 1
#endif

#ifndef ASSERT
   #define ASSERT(x)
#endif

#define FALSE	0
#define TRUE	1

//----------------------------------------------------------------------------

#define SP_SPACE	   0x3000 //Ideographic Space
#define SP_JTEN		0x3001 //Ideographic Comma
#define SP_JMARU	   0x3002 //Ideographic Full Stop

// These two codes fall into the Unicode PUA (Private Use Area) 0xE000 -> 0xF8FF
#define BP_TOP_KINSOKU_CHARACTER       0xE000
#define BP_BACK_KINSOKU_CHARACTER      0xE001

#define TOP_KINSOKU_MASK	0x40000000
#define BACK_KINSOKU_MASK	0x20000000

#define FONT_HIRA_NUM	0x51
#define FONT_KATA_NUM	0x54
#define FONT_KIGOU_NUM	0x1D

#define FONT_HIRA_TOP	0
#define FONT_KATA_TOP	(FONT_HIRA_NUM)
#define FONT_KIGOU_TOP	(FONT_HIRA_NUM+FONT_KATA_NUM)
#define FONT_ZEN_TOP	(FONT_HIRA_NUM+FONT_KATA_NUM+FONT_KIGOU_NUM)

#define CODE( a )	(a)

#define IS_BLOCK_END_CODE( _code ) ( ((_code) == SP_JTEN) || ((_code) == SP_JMARU) || ((_code) == CODE('}')) || ((_code) == CODE( ',' )) )

//----------------------------------------------------------------------------

static int font_ref_num = 0;
static int font_line_len = 10;

//----------------------------------------------------------------------------

int const kPlatformCharacterBegin = 0xE080;
int const kPlatformCharacterCount = 12;

// CROSS       :  0xE080
// CIRCLE      :  0xE081
// SQUARE      :  0xE082
// TRIANGLE    :  0xE083
// L1          :  0xE084
// L2          :  0xE085
// L3          :  0xE086
// R1          :  0xE087
// R2          :  0xE088
// R3          :  0xE089
// OK          :  0xE08A
// CANCEL      :  0xE08B

int const kCharacterReplacements[][kPlatformCharacterCount] =
{
   // PS3
   {
      0xE100,  // 0xE080 -> Cross
      0xE101,  // 0xE081 -> Circle
      0xE102,  // 0xE082 -> Square
      0xE103,  // 0xE083 -> Triangle
      0xE104,  // 0xE084 -> L1
      0xE105,  // 0xE085 -> L2
      0xE106,  // 0xE086 -> L3
      0xE107,  // 0xE087 -> R1
      0xE108,  // 0xE088 -> R2
      0xE109,  // 0xE089 -> R3
      0xE100,  // 0xE08A -> OK (Cross)
      0xE101,  // 0xE08B -> CANCEL (Circle)
   },

   // PS3 (Japan)
   {
      0xE100,  // 0xE080 -> Cross
      0xE101,  // 0xE081 -> Circle
      0xE102,  // 0xE082 -> Square
      0xE103,  // 0xE083 -> Triangle
      0xE104,  // 0xE084 -> L1
      0xE105,  // 0xE085 -> L2
      0xE106,  // 0xE086 -> L3
      0xE107,  // 0xE087 -> R1
      0xE108,  // 0xE088 -> R2
      0xE109,  // 0xE089 -> R3
      0xE101,  // 0xE08A -> OK (Circle)
      0xE100,  // 0xE08B -> CANCEL (Cross)
   },

   // X360
   {
      0xE200,  // 0xE080 -> A
      0xE201,  // 0xE081 -> B
      0xE202,  // 0xE082 -> X
      0xE203,  // 0xE083 -> Y
      0xE204,  // 0xE084 -> LB
      0xE205,  // 0xE085 -> LT
      0xE206,  // 0xE086 -> LS
      0xE207,  // 0xE087 -> RB
      0xE208,  // 0xE088 -> RT
      0xE209,  // 0xE089 -> RS
      0xE200,  // 0xE08A -> OK (A)
      0xE201,  // 0xE08B -> CANCEL (B)
   },

   // VITA (for now it's a copy of PS3)
   {
      0xE100,  // 0xE080 -> Cross
      0xE101,  // 0xE081 -> Circle
      0xE102,  // 0xE082 -> Square
      0xE103,  // 0xE083 -> Triangle
      0xE104,  // 0xE084 -> L1
      0xE105,  // 0xE085 -> L2
      0xE106,  // 0xE086 -> L3
      0xE107,  // 0xE087 -> R1
      0xE108,  // 0xE088 -> R2
      0xE109,  // 0xE089 -> R3
      0xE100,  // 0xE08A -> OK (Cross)
      0xE101,  // 0xE08B -> CANCEL (Circle)
   },
};

int gFontPlatform = BP_FONT_PLATFORM_X360;

void BP_font_set_platform(int platform)
{
   gFontPlatform = platform;
}

int const BP_ReplaceCharacter(int character)
{
   int replaceCharacterIndex = character - kPlatformCharacterBegin;
   if( replaceCharacterIndex >= 0 && replaceCharacterIndex < kPlatformCharacterCount )
   {
      return kCharacterReplacements[gFontPlatform][replaceCharacterIndex];
   }

   return character;
}

//----------------------------------------------------------------------------

struct SBPFontCharacter
{
   unsigned char width;
   unsigned char height;
   char offsetX;
   char offsetY;
   unsigned char advance;
   unsigned char data[0];
};

struct SBPCharacterTableEntry
{
   unsigned int unicode;
   unsigned int fileOffset;
};

SBPFont* gBP_Font = NULL;

static SBPFontCharacter* BP_GetFontCharacter(int code)
{
   char* pFontBegin = (char*)gBP_Font;

   int const characterCount = gBP_Font->characterCount;
   SBPCharacterTableEntry* pTable = (SBPCharacterTableEntry*)(pFontBegin + gBP_Font->characterTableOffset);

   for( int i = 0; i < characterCount; ++i )
   {
      if( pTable->unicode == code )
         return (SBPFontCharacter*)(pFontBegin + pTable->fileOffset);

      ++pTable;
   }

   return NULL;
}

static inline int BP_AdjustForFontWidth(int value)
{
   return value * gBP_Font->fontSizeX / 24;
}

static inline int BP_AdjustForFontHeight(int value)
{
   return value * gBP_Font->fontSizeY / 24;
}

//----------------------------------------------------------------------------

static inline unsigned int BP_ScaleColor(unsigned int color, unsigned char alpha)
{
   unsigned int rgb = color & 0x00FFFFFF;
   unsigned int a = ((color / 255) * alpha) & 0xFF000000;
   return (rgb | a);
}

static inline int BP_GetCode( int code )
{
   return code & ~(TOP_KINSOKU_MASK|BACK_KINSOKU_MASK);
}

unsigned char const *BP_font_decode_utf8_character_internal( int *code, unsigned char const *m )
{
   // Decode UTF-8 character

   unsigned char c0 = *m;
   
   if( c0 < 128 )
   {
      *code = c0;
      m += 1;
   }
   else if ( c0 > 191 && c0 < 224 )
   {
      unsigned char c1 = *(m + 1);
      *code = (c0 & 31) << 6 | c1 & 63;
      m += 2;
   }
   else
   {
      unsigned char c1 = *(m + 1);
      unsigned char c2 = *(m + 2);
      *code = (c0 & 15) << 12 | (c1 & 63) << 6 | (c2 & 63);

      m += 3;
   }

	return m;
}

unsigned char const *BP_font_decode_utf8_character( int *code, unsigned char const *m )
{
   int extraFlags = 0;
   int peekCode;
   unsigned char const * peekM = m;
   
   do 
   {
      peekM = BP_font_decode_utf8_character_internal(&peekCode, peekM);
      switch(peekCode)
      {
      case BP_TOP_KINSOKU_CHARACTER:
         extraFlags |= TOP_KINSOKU_MASK;
         break;
      case BP_BACK_KINSOKU_CHARACTER:
         extraFlags |= BACK_KINSOKU_MASK;
         break;
      }
   } 
   while (peekCode == BP_TOP_KINSOKU_CHARACTER || peekCode == BP_BACK_KINSOKU_CHARACTER);

   // Allow dynamic character replacement
   *code = BP_ReplaceCharacter(peekCode) | extraFlags;

   return peekM;
}

//----------------------------------------------------------------------------

static const int kOutlineOffsets[] = 
{
   -2, -2,
   -1, -2,
   0, -2,
   1, -2,
   2, -2,

   -2, -1,
   -1, -1,
   0, -1,
   1, -1,
   2, -1,

   -2, 0,
   -1, 0,
   1, 0,
   2, 0,

   -2, 1,
   -1, 1,
   0, 1,
   1, 1,
   2, 1,

   -2, 2,
   -1, 2,
   0, 2,
   1, 2,
   2, 2,

   0, 0
};

int BP_GetOutlineOffsets(int renderPass, int* x, int* y)
{
   *x = kOutlineOffsets[renderPass*2 + 0];
   *y = kOutlineOffsets[renderPass*2 + 1];

   return (renderPass == (BP_FONT_OUTLINED_DRAW_COUNT - 1)) ? 1 : 0;
}

//----------------------------------------------------------------------------

void BP_font_init_resource(int reverseOkCancel)
{
   const unsigned char * const pData = binary_MGS_FONT_RAW_data;
   const int size = binary_MGS_FONT_RAW_size;

   // Decompress
   // HACK! last 4 bytes of file are decompressed size with .gz
   // decompressed size in Intel format
   uint32 decompressedSize = BP_LE_SwapSInt( *((uint32*) (pData + (size - 4))) );

   uint8 * pDestData = (uint8*) malloc(decompressedSize);

   int const ret = uncompress_gzip(pDestData, &decompressedSize, (uint8*) pData, size);
   BPE_VERIFY(ret == 0, false, "zlib decompression error!");

   BP_font_resident_load_set((char*)pDestData);

#if BPE_TARGET == BPE_TARGET_PS3
   if( reverseOkCancel )
      gFontPlatform = BP_FONT_PLATFORM_PS3_JP;
   else
      gFontPlatform = BP_FONT_PLATFORM_PS3;

#elif BPE_TARGET == BPE_TARGET_X360
   gFontPlatform = BP_FONT_PLATFORM_X360;
#elif BPE_TARGET == BPE_TARGET_VITA
   gFontPlatform = BP_FONT_PLATFORM_VITA;
#endif

}

void BP_font_resident_load_set( char *fonttop )
{
   BPE_ASSERT( gBP_Font == NULL, "Already initialized" );
   gBP_Font = (SBPFont*)fonttop;

   BP_LE_SwapUInt_Inp( &gBP_Font->id );
   BP_LE_SwapUInt_Inp( &gBP_Font->fontSizeX );
   BP_LE_SwapUInt_Inp( &gBP_Font->fontSizeY );
   BP_LE_SwapUInt_Inp( &gBP_Font->maxCharacterDescent );
   BP_LE_SwapUInt_Inp( &gBP_Font->characterTableOffset );
   BP_LE_SwapUInt_Inp( &gBP_Font->characterCount );

   char* pFontBegin = (char*)gBP_Font;

   int const characterCount = gBP_Font->characterCount;
   SBPCharacterTableEntry* pTable = (SBPCharacterTableEntry*)(pFontBegin + gBP_Font->characterTableOffset);

   for( int i = 0; i < characterCount; ++i )
   {
      BP_LE_SwapUInt_Inp( &pTable->unicode );
      BP_LE_SwapUInt_Inp( &pTable->fileOffset );
      ++pTable;
   }
}

int BP_font_size_W()
{
   return gBP_Font ? gBP_Font->fontSizeX : 24;
}

int BP_font_size_H()
{
   return gBP_Font ? gBP_Font->fontSizeY : 24;
}

int BP_font_max_offset()
{
   return gBP_Font ? gBP_Font->maxCharacterDescent : 0;
}

static inline int BP_GetCharacterWidth( int code )
{
   SBPFontCharacter* pFontChar = BP_GetFontCharacter(code);
   return pFontChar ? pFontChar->width : gBP_Font->fontSizeX;
}

static inline int BP_GetCharacterAdvance( int code )
{
   SBPFontCharacter* pFontChar = BP_GetFontCharacter(code);
   return pFontChar ? pFontChar->advance : gBP_Font->fontSizeX;
}

static void BP_DrawTdot(BP_FONT_VRAMINFO *out, int xx, int yy, SBPFontCharacter* pFontChar)
{
   int const advance = pFontChar->advance;

   int const dotWidth = BP_AdjustForFontWidth(2);
   int const dotHeight = BP_AdjustForFontHeight(2);

   int const halfDotWidth = dotWidth / 2;
   int const halfDotHeight = dotHeight / 2;

   int const dotOffsetX = (advance + 1) / 2 - halfDotWidth;
   int const dotOffsetY = -(out->l_skip / 2 + halfDotHeight);

   int x = xx + dotOffsetX;
   int y = yy + dotOffsetY;

   if( y < 0 )
      return;

   int const stride = (out->pitch / 4);

   unsigned int* pCurrentOutput = (unsigned int*)out->vramtop + y * stride + x;

   unsigned int const baseColor = out->colorRGBA;

   for( int y = 0; y < dotHeight; ++y )
   {
      for( int x = 0; x < dotWidth; ++x )
      {
         *(pCurrentOutput + x) = baseColor;
      }

      pCurrentOutput += stride;
   }
}

static int BP_DrawUnknownCharacter(BP_FONT_VRAMINFO *out, int x, int y)
{
   int const width = gBP_Font->fontSizeX;
   int const height = gBP_Font->fontSizeY;

   int const stride = (out->pitch / 4);

   unsigned int* pCurrentOutput = (unsigned int*)out->vramtop + y * stride + x;

   for( int y = 0; y < width; ++y )
   {
      for( int x = 0; x < height; ++x )
      {
         *(pCurrentOutput + x) = BP_MakeFontColor(255, 0, 0, 255);
      }

      pCurrentOutput += stride;
   }

   return width;
}

static int BP_DrawCharacter( BP_FONT_VRAMINFO *out, int xx, int yy, int code, int drawTdot )
{
   if( code > 0 )
   {
      SBPFontCharacter* pFontChar = BP_GetFontCharacter(code);

      if( !pFontChar )
         return BP_DrawUnknownCharacter(out, xx, yy);

      if( drawTdot )
         BP_DrawTdot(out, xx, yy, pFontChar);

      int const characterWidth = pFontChar->width;
      int const characterHeight = pFontChar->height;

      int x = xx + pFontChar->offsetX;
      int y = yy + pFontChar->offsetY;
      y = (y > 0) ? y : 0;

      int const stride = (out->pitch / 4);

      unsigned int* pCurrentOutput = (unsigned int*)out->vramtop + y * stride + x;
      
      unsigned char* charData = pFontChar->data;

      unsigned int const baseColor = out->colorRGBA;

      if (code != ' ')
      {
         unsigned int const borderColor = BP_ScaleColor(baseColor, 0);
         if (x > 0)
         {
            // fill left border
            for (int by = 0; by <= characterHeight; ++by)
            {
               if ((*(pCurrentOutput + by*stride - 1) >> 24) < 10)
               {
                  *(pCurrentOutput + by*stride - 1) = borderColor;
               }
            }
         }
         // fill right border
         for (int by = 0; by <= characterHeight; ++by)
         {
            *(pCurrentOutput + by*stride + characterWidth) = borderColor;
         }
         if (y > 0)
         {
            // fill top border
            for (int bx = 0; bx <= characterWidth; ++bx)
            {
               if ((*(pCurrentOutput + bx - stride) >> 24) < 10)
               {
                  *(pCurrentOutput + bx - stride) = borderColor;
               }
            }
         }
         // fill bottom border
         for (int bx = 0; bx <= characterWidth; ++bx)
         {
            *(pCurrentOutput + bx + characterHeight*stride) = borderColor;
         }
         if (x > 0 && y > 0)
         {
            // fill top left pixel
            *(pCurrentOutput - 1 - stride) = borderColor;
         }
      }

      for( int y = 0; y < characterHeight; ++y )
      {
         for( int x = 0; x < characterWidth; ++x )
         {
#if BP_OUTPUT_CHECKER_BOARD()
            unsigned char value = (x + y) % 2 ? 255 : 0;
#else
            unsigned char value = *charData++;
#endif
            *(pCurrentOutput + x) = BP_ScaleColor(baseColor, value);
         }

         pCurrentOutput += stride;
      }

      return pFontChar->advance;
   }
   else if( code == 0 )
   {
      int const advance = font_line_len;

      int const lineWidth = font_line_len - 2;
      int const lineHeight = BP_AdjustForFontHeight(2);
      int const halfLineWidth = lineWidth / 2;
      int const halfLineHeight = lineHeight /2;

      int const lineOffsetX = (advance + 1) / 2 - halfLineWidth;
      int const lineOffsetY = (BP_FONT_SIZE_H / 2 + halfLineHeight);

      int x = xx + lineOffsetX;
      int y = yy + lineOffsetY;

      if( y >= 0 )
      {
         int const stride = (out->pitch / 4);

         unsigned int* pCurrentOutput = (unsigned int*)out->vramtop + y * stride + x;

         unsigned int const baseColor = out->colorRGBA;

         for( int y = 0; y < lineHeight; ++y )
         {
            for( int x = 0; x < lineWidth; ++x )
            {
               *(pCurrentOutput + x) = baseColor;
            }

            pCurrentOutput += stride;
         }
      }

      return font_line_len;
   }
   
   return 0;
}

static void set_rubi_left_pos( BP_FONT_DRAWINFO *dr, int xmax, int x, int y )
{
	dr->rubi_left_x = x;
	dr->rubi_left_y = y;
	dr->rubi_orikaeshi_xmax = xmax;
}

static void set_rubi_orikaeshi( BP_FONT_DRAWINFO *dr, int xmax )
{
	dr->rubi_orikaeshi_xmax = xmax;
}

static inline int BP_Clamp(int value, int min, int max)
{
   if( value < min )
      return min;
   else if(value > max)
      return max;
   else 
      return value;
}

static unsigned char BP_GetRubiCharacterValue(SBPFontCharacter* pFontChar, int x, int y)
{
   unsigned int value = 0;

   value += pFontChar->data[BP_Clamp(y+0, 0, pFontChar->height-1) * pFontChar->width + BP_Clamp(x+0, 0, pFontChar->width-1)];
   value += pFontChar->data[BP_Clamp(y+0, 0, pFontChar->height-1) * pFontChar->width + BP_Clamp(x+1, 0, pFontChar->width-1)];
   value += pFontChar->data[BP_Clamp(y+1, 0, pFontChar->height-1) * pFontChar->width + BP_Clamp(x+0, 0, pFontChar->width-1)];
   value += pFontChar->data[BP_Clamp(y+1, 0, pFontChar->height-1) * pFontChar->width + BP_Clamp(x+1, 0, pFontChar->width-1)];
   
   value >>= 2;

   return (unsigned char)value;
}

static int BP_DrawRubiCharacter( BP_FONT_VRAMINFO *out, int xx, int yy, int code )
{
   SBPFontCharacter* pFontChar = BP_GetFontCharacter(code);

   int const offsetX = pFontChar ? pFontChar->offsetX : 0;
   int const offsetY = pFontChar ? pFontChar->offsetY : 0;

   int const width = pFontChar ? pFontChar->width : gBP_Font->fontSizeX;
   int const height = pFontChar ? pFontChar->height : gBP_Font->fontSizeY;

   int const advance = pFontChar ? pFontChar->advance : gBP_Font->fontSizeX;

   int const halfCharacterWidth = (width + 1) / 2;
   int const halfCharacterHeight = (height + 1) / 2;

   int x = xx + (offsetX + 1) / 2;
   int y = yy + (offsetY + 1) / 2;

   int const stride = (out->pitch / 4);

   unsigned int* pCurrentOutput = (unsigned int*)out->vramtop + y * stride + x;

   unsigned int const baseColor = out->colorRGBA;

   for( int y = 0; y < halfCharacterHeight; ++y )
   {
      for( int x = 0; x < halfCharacterWidth; ++x )
      {
         unsigned int color;

         if( pFontChar )
         {
            unsigned char value = BP_GetRubiCharacterValue(pFontChar, x*2, y*2);
            color = BP_ScaleColor(baseColor, value);
         }
         else
         {
            color = BP_MakeFontColor(255, 0, 0, 255);
         }

         *(pCurrentOutput + x) = color;
      }

      pCurrentOutput += stride;
   }

   return (advance + 1) / 2;
}

static int get_rubi_width( int *count, unsigned char const *string )
{
	unsigned char const *m;
	int n, w;

	w = 0;
	n = 0;
	m = string;

	for( ;; )
   {
		int code;
		m = (unsigned char const *)BP_font_decode_utf8_character( &code, m );
		code = BP_GetCode( code );

		if( code == CODE( '}' ) ){
			break;
		}

		w += ( BP_GetCharacterAdvance( code ) + 1 ) / 2;
		n++;
	}

	*count = n;

	return w;
}

static int draw_rubi_string( BP_FONT_DRAWINFO *out, int x, int y, unsigned char const *string )
{
	int width;
	int count;
	int basewidth, basey;
	int dx, top;
	unsigned char const *m;

	width = get_rubi_width( &count, string );

	if( y > out->rubi_left_y )
   {
		if( out->rubi_orikaeshi_xmax - out->rubi_left_x <= x / 2 ){
			basey = y;
			basewidth = x;
			out->rubi_left_x = 0;
		} else {
			basewidth = out->rubi_orikaeshi_xmax - out->rubi_left_x;
			basey = out->rubi_left_y;
		}
	} 
   else 
   {
		basewidth = x - out->rubi_left_x;
		basey = y;
	}

	dx = 1;
	count = count - 1;
	
   if( count > 0 && width - basewidth < 0 )
   {
		dx = ( basewidth - width ) / count;
		if( dx > 4 ) dx = 4;
		if( dx < 1 ) dx = 1;
	}

	width = width + dx * count;

	top = out->rubi_left_x + ( basewidth - width ) / 2;

   if( top + width > out->rubi_orikaeshi_xmax )
   {
		top = out->rubi_orikaeshi_xmax - width;
	}

	if( top < 0 ) top = 0;

	x = top;

	for( m = string; ; )
   {
		int code;
      m = (unsigned char*)BP_font_decode_utf8_character( &code, m );
		code = BP_GetCode( code );

		if( code == CODE( '}' ) )
			break;

		x += BP_DrawRubiCharacter( out->outinfo, x, basey - BP_FONT_SIZE_H / 2, code ) + dx;
	}

	return 0;
}

static void BP_font_set_vraminfo_common(BP_FONT_VRAMINFO *vinfo, int c_skip, int l_skip, int flag )
{
   vinfo->c_skip = c_skip;
   vinfo->l_skip = l_skip;
   vinfo->flag = flag;

   if( l_skip >= BP_RUBI_SIZE_H )
   {
      vinfo->rubi_display_flag = 1;
   }
   else 
   {
      vinfo->rubi_display_flag = 0;
   }

   vinfo->y_step = BP_FONT_SIZE_H + l_skip;

   vinfo->colorRGBA = BP_MakeFontColor(255, 255, 255, 128);
   vinfo->max_width = 0;

   vinfo->max_lines = 32767;
}

static void BP_font_set_vraminfo_target(BP_FONT_VRAMINFO *vinfo, void* vramtop, int width, int height, int pitch)
{
   vinfo->vramtop = (unsigned char*)vramtop;

   vinfo->height = height;
   vinfo->width = width;
   vinfo->pitch = pitch;

   if( !( vinfo->flag & FONT_NO_KINSOKU ) )
   {
      vinfo->width = width - ( BP_FONT_SIZE_W + vinfo->c_skip );
   }
}

void BP_font_set_vraminfo_direct( BP_FONT_VRAMINFO *vinfo, void *vramtop, int width, int pitch, int height, int c_skip, int l_skip, int flag )
{
   BP_font_set_vraminfo_common(vinfo, c_skip, l_skip, flag);
   BP_font_set_vraminfo_target(vinfo, vramtop, width, height, pitch);

   vinfo->bp_texture = 0;
   vinfo->pBufferedTexture = NULL;
}

void BP_font_set_vraminfo_texture(BP_FONT_VRAMINFO *vinfo, unsigned int bp_texture, int c_skip, int l_skip, int flag)
{
   BP_font_set_vraminfo_common(vinfo, c_skip, l_skip, flag);
   BP_font_set_vraminfo_target(vinfo, NULL, 0, 0, 0);

   vinfo->bp_texture = bp_texture;
   vinfo->pBufferedTexture = NULL;
}

#if defined( MGS_VERSION )
void BP_font_set_vraminfo_texture_buffered(BP_FONT_VRAMINFO *vinfo, SBP_BufferedTexture* pBT, int c_skip, int l_skip, int flag)
{
   BPE_VERIFY(pBT != NULL, false, "Must have pBT!");
   BPE_VERIFY(pBT->aTextureHandle[0] != NULL, false, "Must have texture!");
   BPE_VERIFY(pBT->aTextureHandle[1] != NULL, false, "Must have texture!");
   BPE_VERIFY(pBT->aTextureHandle[2] != NULL, false, "Must have texture!");

   BP_font_set_vraminfo_common(vinfo, c_skip, l_skip, flag);
   BP_font_set_vraminfo_target(vinfo, NULL, 0, 0, 0);

   vinfo->pBufferedTexture = pBT;
}

void BP_font_begin_render_texture(BP_FONT_VRAMINFO *vinfo, int doClear)
{
   if( vinfo->pBufferedTexture )
   {
      BP_font_begin_render_texture_buffered_internal(vinfo, doClear);
   }
   else
   {
      BP_font_begin_render_texture_internal(vinfo, doClear);
   }
}

void BP_font_begin_render_texture_internal(BP_FONT_VRAMINFO *vinfo, int doClear)
{
   BPE_VERIFY(vinfo->pBufferedTexture == NULL, false, "BP_font_begin_render_texture must not have pBufferedTexture");

   CBaseTexture* pTexture = (CBaseTexture*)vinfo->bp_texture;
   
   BPE_VERIFY(pTexture != NULL, false, "Must have texture!");

   void* memory;
   int pitch;
   
   pTexture->Lock(&memory, &pitch);

   int const width = pTexture->GetWidth();
   int const height = pTexture->GetHeight();

   if( doClear )
      memset(memory, 0x00, pitch * height);

   BP_font_set_vraminfo_target(vinfo, memory, width, height, pitch);
}

void BP_font_begin_render_texture_buffered_internal(BP_FONT_VRAMINFO *vinfo, int doClear)
{
   BPE_VERIFY(vinfo->pBufferedTexture != NULL, false, "BP_font_begin_render_texture_double_buffered must have valid pBufferedTexture");

   SBP_BufferedTexture* pBT = vinfo->pBufferedTexture;
   // Increment to next texture
   bool gotNewTexture = false;
   if( pBT->lastLockedFrame != BP_GetFrameCount() )
   {
      pBT->lastLockedFrame = BP_GetFrameCount();
      pBT->prevIndex = pBT->currIndex;
      pBT->currIndex = (pBT->currIndex+1)%BP_BUFFEREDTEXTURE_MAX_TEXTURE;
      gotNewTexture = true;
   }

   CBaseTexture* pCurrTexture = (CBaseTexture*)pBT->aTextureHandle[pBT->currIndex];
   CBaseTexture* pPrevTexture = (CBaseTexture*)pBT->aTextureHandle[pBT->prevIndex];

   void* memory_Curr;
   void* memory_Prev;
   int pitch;
   
   pCurrTexture->Lock(&memory_Curr, &pitch);
   pPrevTexture->Lock(&memory_Prev, &pitch);

   int const width = pCurrTexture->GetWidth();
   int const height = pCurrTexture->GetHeight();
   BPE_ASSERT( pPrevTexture->GetWidth() == width && pPrevTexture->GetHeight() == height, "Both textures must have same width/height" );

   if( doClear )
      memset(memory_Curr, 0x00, pitch * height);
   else if( gotNewTexture )
      memcpy(memory_Curr, memory_Prev, pitch * height);

   BP_font_set_vraminfo_target(vinfo, memory_Curr, width, height, pitch);
}

void BP_font_end_render_texture(BP_FONT_VRAMINFO *vinfo)
{
   if( vinfo->pBufferedTexture )
   {
      BP_font_end_render_texture_buffered_internal(vinfo);
   }
   else
   {
      BP_font_end_render_texture_internal(vinfo);
   }
}

void BP_font_end_render_texture_internal(BP_FONT_VRAMINFO *vinfo)
{
   CBaseTexture* pTexture = (CBaseTexture*)vinfo->bp_texture;
   pTexture->Unlock();

   BP_font_set_vraminfo_target(vinfo, NULL, 0, 0, 0);
}

void BP_font_end_render_texture_buffered_internal(BP_FONT_VRAMINFO *vinfo)
{
   SBP_BufferedTexture* pBT = vinfo->pBufferedTexture;

   CBaseTexture* pCurrTexture = (CBaseTexture*)pBT->aTextureHandle[pBT->currIndex];
   CBaseTexture* pPrevTexture = (CBaseTexture*)pBT->aTextureHandle[pBT->prevIndex];
   pCurrTexture->Unlock();
   pPrevTexture->Unlock();

   BP_font_set_vraminfo_target(vinfo, NULL, 0, 0, 0);
}

void BP_font_clear_texture(BP_FONT_VRAMINFO *vinfo)
{
   bool const wasLocked = (vinfo->vramtop != 0);
   
   if( wasLocked )
   {
      BP_font_end_render_texture(vinfo);
      BP_font_begin_render_texture(vinfo, 1);
   }
   else
   {
      BP_font_begin_render_texture(vinfo, 1);
      BP_font_end_render_texture(vinfo);
   }
}

void BP_font_clear_texture_range(BP_FONT_VRAMINFO *vinfo, int y0, int y1)
{
   bool const wasLocked = (vinfo->vramtop != 0);

   if( !wasLocked )
      BP_font_begin_render_texture(vinfo, 0);

   BPE_VERIFY(y0 >= 0 && y0 < vinfo->height && y1 >= 0 && y1 < vinfo->height, false, "BP_font_clear_texture_range out of range!");

   int const lineCount = y1 - y0;
   
   unsigned char* memory = vinfo->vramtop + vinfo->pitch * y0;
   memset(memory, 0, lineCount * vinfo->pitch);

   if( !wasLocked )
      BP_font_end_render_texture(vinfo);
}
#endif

void BP_font_set_clut4( unsigned int *clut_buf, int no, int fg, int bg )
{
	unsigned int *top;
	typedef struct {
#if BPE_IS_ENDIAN_LITTLE()
		unsigned char r, g, b, a;
#else
      unsigned char a, b, g, r;
#endif
	} RGBA;
	RGBA *fc;
	RGBA *bc;

   int r, g, b;

	fc = (RGBA*)&fg;
	bc = (RGBA*)&bg;

	ASSERT( no < 4 );
	top = clut_buf + no * 4;

	top[ 0 ] = bg;

	r = ( fc->r * 1 + bc->r * 2 ) / 3;
	g = ( fc->g * 1 + bc->g * 2 ) / 3;
	b = ( fc->b * 1 + bc->b * 2 ) / 3;
	top[ 1 ] = FONT_RGB( r, g, b );

	r = ( fc->r * 5 + bc->r * 3 ) / 8;
	g = ( fc->g * 5 + bc->g * 3 ) / 8;
	b = ( fc->b * 5 + bc->b * 3 ) / 8;
	top[ 2 ] = FONT_RGB( r, g, b );

	top[ 3 ] = fg;
}

static int draw_font( BP_FONT_DRAWINFO *dr, int x, int y, int code )
{
   int dx = BP_DrawCharacter(dr->outinfo, x, y, code, dr->put_Tdot_flag);

	return dx + dr->outinfo->c_skip;
}

static int draw_rubi( BP_FONT_DRAWINFO *dr, int x, int y, unsigned char const *str )
{
	if( dr->outinfo->rubi_display_flag == 0 )
		return 0;
	
   return draw_rubi_string( dr, x, y, str );
}

int BP_font_draw_string( BP_FONT_DRAWINFO *work, char const *string )
{
	BP_FONT_VRAMINFO *outinfo = work->outinfo;

   int x = work->xtop;
   int y = work->ytop + outinfo->l_skip;
	int dx = 0;
	int result = 0;
	int orikaeshi_flag = 0;
   int no_orikaeshi = 0;
   int num_lines = 0;

   unsigned char const *currentPtr = ( unsigned char const * )string;

	outinfo->max_width = 0;

	for( ;; )
   {
      // End of string?
      if( *currentPtr == '\0' )
			break;

      // Examine current character
      int mdata;
		unsigned char const * next = BP_font_decode_utf8_character( &mdata, currentPtr );
		
      int code = BP_GetCode( mdata );

      // Skip invalid characters
		if( mdata < CODE( 0x20 ) )
      {
         currentPtr = next;
		}
      else if(( code == CODE( '\\' ) ) && (( outinfo->flag & FONT_NO_META_CHARA ) == 0 ))
      {
			currentPtr = next;
			next = BP_font_decode_utf8_character( &mdata, currentPtr );
			code = BP_GetCode( mdata );
		}
      else if(( outinfo->flag & FONT_NO_META_CHARA ) == 0 )
      {
         // Embedded command handling
			if( code == CODE( '#' ) )
         {
            int d;
				currentPtr = (unsigned char*)BP_font_decode_utf8_character( &d, next );
				d = BP_GetCode( d );

            switch( d )
            {
               //#R (assumes "{" comes next) LEGACY?
            case CODE( 'R' ):
               currentPtr = (unsigned char*)BP_font_decode_utf8_character( &d, currentPtr );
               ASSERT( BP_GetCode( d ) == CODE( '{' ) );

               // NOTE: fallthrough intentional

               //#{ (begins block)
            case CODE( '{' ):
               work->block_flag = TRUE;
               
               if( outinfo->rubi_display_flag )
               {
                  work->rubi_flag = TRUE;
                  set_rubi_left_pos( work, outinfo->width, x, y );
               }
               break;
               
               //#T -> TDOT?
            case CODE( 'T' ):
               currentPtr = (unsigned char*)BP_font_decode_utf8_character( &d, currentPtr );
               ASSERT( BP_GetCode( d ) == CODE( '{' ) );
               work->block_flag = TRUE;
               work->put_Tdot_flag = TRUE;
               break;
            
               //#- -> 1.5 times font size line (NOTE: Some symbols used to trigger this as well 0x8312 and 0x8313), but we're remapping all of them to the '-' code.
            case 0x30FC: //Katakana-Hiragana Prolonged Sound Mark
            case 0xFF0D: //Fullwidth Hyphen-Minus
            case CODE( '-' ):
               font_line_len = BP_FONT_SIZE_W * 3 / 2;
               code = 0;
               goto DRAW;

               //#_ -> full width line
            case CODE( '_' ):
               font_line_len = BP_FONT_SIZE_W;
               code = 0;
               goto DRAW;

               //#N -> Newline
            case CODE( 'N' ):
               mdata = code = CODE( '\n' );
               break;

#ifdef _KPXBOX
            case CODE( 'L' ):		// LockOn
               {
                  static int lock_codes[] = {
                     CODE( 'Y' ), CODE( 'A' ), CODE( CODE_BLACK ), CODE( 'L' )
                  };
                  mdata = code = lock_codes[ GM_GET_CONFIG_CONTROLS() ];
               }
               goto DRAW;
               break;
            case CODE( 0x8303 ):	// Action button
               {
                  static int action_codes[] = {
                     CODE( 'Y' ), CODE( 'A' ), CODE( 'Y' ), CODE( 'Y' )
                  };
                  mdata = code = action_codes[ GM_GET_CONFIG_CONTROLS() ];
               }
               goto DRAW;
               break;
#endif

#ifdef PSX2
            case CODE( 0x8302 ):	// Circle
            case CODE( 0x8303 ):	// Triangle
            case CODE( 0x8304 ):	// Square
            case CODE( 0x8305 ):	// Cross
               mdata = code = d;
               break;
#endif

               //#1 -> replaces 1 with a number set by code (font_ref_num)
            case CODE( '1' ):
               if( font_ref_num < 10 ){
                  mdata = code = CODE( '0' + font_ref_num );
               } else {
                  mdata = code = CODE( '*' );
               }
               goto DRAW;

               //#W -> finishes rendering of text
            case CODE( 'W' ):
               goto OVER;

               //#C(?) -> Callback with value specified in parenthesis
            case CODE( 'C' ):
               currentPtr = (unsigned char*)BP_font_decode_utf8_character( &d, currentPtr );
               if( BP_GetCode( d ) == CODE( '(' ) ){
                  int value = 0;
                  for( ;; ){
                     currentPtr = (unsigned char*)BP_font_decode_utf8_character( &d, currentPtr );
                     code = BP_GetCode( d );
                     if( BP_GetCode( d ) == CODE( ')' ) ){
                        break;
                     }
                     if( code >= CODE( '0' ) && code <= CODE( '9' ) ){
                        value = value * 10 + ( code - CODE( '0' ) );
                     } else {
                        ASSERT( FALSE );
                     }
                  }
                  d = value;
               } 
               else 
               {
                  d = BP_GetCode( d ) - CODE( '0' );
                  ASSERT( d >= 0 && d <= 9 );
               }

               if( work->callback != NULL )
               {
                  char *res;
                  work->now_x = x;
                  work->now_y = y;
                  res = ( *work->callback )( work, (char*)currentPtr, d );
                  if( res != NULL ){
                     currentPtr = (unsigned char*)res;
                  }
                  x = work->now_x;
                  y = work->now_y;
               }
               continue;

            default:
               break;
            }
			}
         else if( work->block_flag )
         {
				if( IS_BLOCK_END_CODE( code ) )
            {
               // Are we currently inside a ruby block? (i.e. "#{????, ???}
					if( work->rubi_flag && code != CODE( '}' ) )
               {
						draw_rubi( work, x, y, next );
					}
					work->rubi_flag = FALSE;

					do 
               {
						currentPtr = (unsigned char*)BP_font_decode_utf8_character( &code, currentPtr );
						code = BP_GetCode( code );
					}
               while( code != CODE( '}' ) );
				
               next = BP_font_decode_utf8_character( &code, currentPtr );
					
               if( code == CODE( '#' ) )
               {
						currentPtr = next;
					}

					work->put_Tdot_flag = FALSE;
					work->block_flag = FALSE;

					dx = 0;
					goto NEXT;
				}
			}
         else if( code == CODE( '|' ) )
         {
				currentPtr = next;
				mdata = code = CODE( '\n' );
			}
		}

      if( mdata < CODE( 0x20 ) )
      {
         switch( mdata )
         {
         case CODE( '\n' ):
            if( work->cr_flag == FALSE )
            {
               goto DO_CR;
            } 
            else 
            {
               work->cr_flag = FALSE;
               orikaeshi_flag = 0;
            }
            break;
         }
         continue;
      }

		work->prev_char = (char*)currentPtr;
		currentPtr = (unsigned char*)BP_font_decode_utf8_character( &mdata, currentPtr );
		code = BP_GetCode( mdata );
DRAW:
		dx = draw_font( work, x, y, code );

NEXT:
		{
			unsigned char const *nn;
			int next_width;
			int next_mdata, next_code;
			int buf_width;
			nn = BP_font_decode_utf8_character( &next_mdata, currentPtr );
			next_code = BP_GetCode( next_mdata );
			next_width = BP_GetCharacterWidth( next_code );

			if( outinfo->flag & FONT_NO_KINSOKU ){
				buf_width = outinfo->width;
			} else {
				buf_width = outinfo->width - BP_FONT_SIZE_W;
			}

			if(( code == CODE( ' ' )) && ( outinfo->flag & FONT_WORD_WRAP )) {
				unsigned char const *nn2;
				int next_width2;
				int next_mdata2, next_code2;
				int word_width;
				nn2 = currentPtr;
				word_width = 0;
				do {
					if( *nn2 == '\0' ) {
						break;
					}
					nn2 = BP_font_decode_utf8_character( &next_mdata2, (unsigned char*)nn2 );
					next_code2 = BP_GetCode( next_mdata2 );
					next_width2 = BP_GetCharacterWidth( next_code2 );
					word_width += next_width2 + outinfo->c_skip;
					if(( x + dx + word_width - 1 ) >= buf_width ) {
						goto DO_CR;
					}
				} while( next_code2 != CODE( ' ' ));
			}

			if( next_width > 0 && ( x + dx + next_width + outinfo->c_skip - 1 ) >= buf_width )
         {
				no_orikaeshi = 0;

				if( ! ( outinfo->flag & FONT_NO_KINSOKU ) )
            {
					if( mdata & BACK_KINSOKU_MASK )
               {
						draw_font( work, x, y, SP_SPACE );
						currentPtr = (unsigned char*)work->prev_char;
						dx = 0;
					}
               else if( next_mdata & TOP_KINSOKU_MASK )
               {
						if( work->block_flag && IS_BLOCK_END_CODE( next_code ) ){
							goto NO_KINSOKU;
						} else {
							int nn_mdata;
							BP_font_decode_utf8_character( &nn_mdata, (unsigned char*)nn );
							if( code == SP_JMARU || !( nn_mdata & TOP_KINSOKU_MASK ) ){
								dx += draw_font( work, x + dx, y, next_code );
								currentPtr = (unsigned char*)nn;
								no_orikaeshi = 1;
							}
						}
					}
				}

				if( no_orikaeshi == 0 ){
					orikaeshi_flag = 1;
				}
				if( work->rubi_flag ){
					set_rubi_orikaeshi( work, x + dx );
				}
				if( outinfo->flag & FONT_NO_NEW_LINE_SPACE ) {
					int tmp;
					if( next_code == CODE( ' ' )) {
						currentPtr = (unsigned char*)BP_font_decode_utf8_character( &tmp, currentPtr );
					}
				}
DO_CR:
				x += dx;
				if( outinfo->max_width < x ){
					outinfo->max_width = x;
				}
				x = work->xtop;
				y = y + outinfo->y_step;
				work->yc ++;
				work->cr_flag = TRUE;
            ++num_lines;

				if ( (outinfo->max_lines > 0 && num_lines >= outinfo->max_lines)
               || (y + BP_FONT_SIZE_H + BP_DOWN_MARGINE) > outinfo->height )
            {
					if( *currentPtr == '\0' || ( *currentPtr == '\n' && *(currentPtr+1) == '\0' ) ){
						goto OK;
					} else {
						goto OVER;
					}
				}
			}
         else 
         {
NO_KINSOKU:
				x += dx;
				work->cr_flag = FALSE;
				if( orikaeshi_flag ){
					result |= FONT_RESULT_ORIKAESHI;
				}
			}
		}
	}

	if( outinfo->max_width < x )
   {
		outinfo->max_width = x;
	}
OK:
	work->prev_char = NULL;
	return result;

OVER:
	work->prev_char = (char*)currentPtr;
	return result | FONT_RESULT_OVER;
}

void BP_font_open_drawinfo( BP_FONT_DRAWINFO *draw, BP_FONT_VRAMINFO *vram )
{
	memset( draw, 0, sizeof( BP_FONT_DRAWINFO ) );

	draw->outinfo = vram;
}

void BP_font_set_locate( BP_FONT_DRAWINFO *draw, int x, int y )
{
	draw->xtop = x;
	draw->ytop = y;
}

void BP_font_set_color( BP_FONT_DRAWINFO *draw, int r, int g, int b, int a)
{
   unsigned int color = BP_MakeFontColor(r, g, b, a);

	draw->colorRGBA = color;
	draw->outinfo->colorRGBA = color;
}

void BP_font_set_refnum( int refnum )
{
	font_ref_num = refnum;
}

