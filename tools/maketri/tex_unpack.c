/*

 tex_unpack.c

 To use, change to the directory containing a tri file, then run 'maketri -x -i <blah.tri>'
 Alternatively, change to the directory and just run 'maketri -x' and it will find all tri files and extract the textures
 Where possible it will look up the original texture name in the idlist.txt

 2010 Bluepoint

*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <ctype.h>

#include "loadinc.h"

#include "tex_unpack.h"

#define DG_TEXFLAG_INJECTED_TEX 0x00000008

static unsigned long32 VramImage[1024*1024];

/* ページ内における8ビットCLUTオフセット（ブロック単位）[[8-bit CLUT in the page offset (block unit)]] */
static short	clut_offset_table0[8] = {0,4,16,20,8,12,24,28};
/* ページ内における8ビットCLUTオフセット番号と [[8-bit CLUT in the page number and offset]] */
static short	clut_offset_table1[128] = {
   0x00,0x01,0x10,0x11,0x20,0x21,0x30,0x31,
   0x02,0x03,0x12,0x13,0x22,0x23,0x32,0x33,
   0x04,0x05,0x14,0x15,0x24,0x25,0x34,0x35,
   0x06,0x07,0x16,0x17,0x26,0x27,0x36,0x37,
   0x08,0x09,0x18,0x19,0x28,0x29,0x38,0x39,
   0x0a,0x0b,0x1a,0x1b,0x2a,0x2b,0x3a,0x3b,
   0x0c,0x0d,0x1c,0x1d,0x2c,0x2d,0x3c,0x3d,
   0x0e,0x0f,0x1e,0x1f,0x2e,0x2f,0x3e,0x3f,
   0x40,0x41,0x50,0x51,0x60,0x61,0x70,0x71,
   0x42,0x43,0x52,0x53,0x62,0x63,0x72,0x73,
   0x44,0x45,0x54,0x55,0x64,0x65,0x74,0x75,
   0x46,0x47,0x56,0x57,0x66,0x67,0x76,0x77,
   0x48,0x49,0x58,0x59,0x68,0x69,0x78,0x79,
   0x4a,0x4b,0x5a,0x5b,0x6a,0x6b,0x7a,0x7b,
   0x4c,0x4d,0x5c,0x5d,0x6c,0x6d,0x7c,0x7d,
   0x4e,0x4f,0x5e,0x5f,0x6e,0x6f,0x7e,0x7f
};

// static int tex_mode[4] = { 0x14, 0x13, 0x02, 0x00 }; /* モード値－＞ＰＳＭ値変換テーブル [[Mode value -> PSM value conversion table]] */
static int psm_to_mode(int psm)
{
   switch (psm)
   {
   case 20:
      return 0;
   case 19:
      return 1;
   case 2:
      return 2;
   case 0:
      return 3;
   }
   assert(1==2);
   return 3;
}

/* ページのサイズテーブル [[Page size table]] */
static long32		block_size[4][2] = {{32,16},{16,16},{16,8},{8,8}};

#pragma pack(push)
#pragma pack(1)
typedef struct
{
   char  identsize;          // size of ID field that follows 18 byte header (0 usually)
   char  colourmaptype;      // type of colour map 0=none, 1=has palette
   char  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

   short colourmapstart;     // first colour map entry in palette
   short colourmaplength;    // number of colours in palette
   char  colourmapbits;      // number of bits per palette entry 15,16,24,32

   short xstart;             // image x origin
   short ystart;             // image y origin
   short width;              // image width in pixels
   short height;             // image height in pixels
   char  bits;               // image bits per pixel 8,16,24,32
   char  descriptor;         // image descriptor bits (vh flip bits)

   // pixel data follows header
}
TGA_HEADER;
#pragma pack(pop)

typedef enum
{
   kRemapAlpha_Do,
   kRemapAlpha_Dont
}
ERemapAlpha;

void SaveBitmapToFile( void *pBitmapBits, short width, short height, short wBitsPerPixel, unsigned int *clut, int u, int v, int uw, int vh, char *lpszFileName, ERemapAlpha remapAlpha )
{
   FILE *fp;
   TGA_HEADER tga_header;
   int i;
   int backwards = 0;
   int x, y;
   unsigned int *trueClut = (unsigned int *) malloc(1024);

   assert(lpszFileName);
   assert(sizeof(TGA_HEADER) == 18);

   memset(&tga_header, 0, sizeof(tga_header));
   tga_header.imagetype = 2;
   tga_header.width = uw;
   tga_header.height = vh;
   tga_header.bits = 32;
   tga_header.descriptor = backwards ? 0x00 : 0x20;

   fp = fopen(lpszFileName, "wb");

   if (fp == NULL)
   {
      fprintf(stderr, "Failed writing to file");
      return;
   }

   fwrite(&tga_header, sizeof(tga_header), 1, fp);

   // convert the palette
   if (wBitsPerPixel == 4 || wBitsPerPixel == 8)
   {
      unsigned char alpha, red, green, blue;
      unsigned char *trueClutBytes = (unsigned char *) trueClut;
      for (i = 0; i < 256; ++i)
      {
         alpha = (clut[i]>>24);
         if (remapAlpha == kRemapAlpha_Do)
         {
            // deduced by looking at the palettes
            if (alpha >= 0x80) alpha = 255; else alpha = 2*alpha;
         }
         blue = ((clut[i]>>16)&0xff);
         green = ((clut[i]>>8)&0xff);
         red = (clut[i]&0xff);
         trueClutBytes[4*i] = blue;
         trueClutBytes[4*i + 1] = green;
         trueClutBytes[4*i + 2] = red;
         trueClutBytes[4*i + 3] = alpha;
      }
   }

   for (y = 0; y < vh; ++y)
   {
      for (x = 0; x < uw; ++x)
      {
         int elemIndex = (v+y)*width + u + x;
         switch (wBitsPerPixel)
         {
         case 4:
            {
               // need to write out palette indices as chars
               unsigned char *data = (unsigned char *) pBitmapBits;
               unsigned char elem = data[elemIndex/2];
               unsigned char dat0 = elem & 0x0f;
               unsigned char dat1 = elem >> 4;
               unsigned int col = trueClut[dat0];
               fwrite(&col, 4, 1, fp);
               // increment again so we don't write out the pair of pixels twice
               ++x;
               // don't write an extra pixel if uw is odd
               if (x < uw)
               {
                  col = trueClut[dat1];
                  fwrite(&col, 4, 1, fp);
               }
            }
            break;
         case 8:
            {
               unsigned char *data = (unsigned char *) pBitmapBits;
               unsigned char elem = data[elemIndex];
               unsigned int col = trueClut[elem];
               fwrite(&col, 4, 1, fp);
            }
            break;
         case 16:
            {
               unsigned short *data = (unsigned short *) pBitmapBits;
               unsigned short elem = data[elemIndex];
               // convert 16 to 32
               // fix for whatever alpha format at some point - here I'm just assuming ABBBBBGGGGGRRRRR
               unsigned char red = elem&0x1;
               unsigned char green = (elem&0x3e0)>>5;
               unsigned char blue = (elem&0x7c)>>10;
               unsigned char alpha = (elem & 0x8000) ? 0xff : 0;
               unsigned int col = blue | (green<<8) | (red<<16) | (alpha<<24);
               fwrite(&col, 4, 1, fp);
            }
            break;
         case 32:
            {
               unsigned int *data = (unsigned int *) pBitmapBits;
               unsigned int elem = data[elemIndex];
               fwrite(&elem, 4, 1, fp);
            }
            break;
         }
      }
   }
   fclose(fp);
   free(trueClut);
}

static void ConvertImageDataToBlockSet( unsigned long32 *src, unsigned long32 *dst )
{
   int		i, j, k ;
   int		block_offset, height_offset, image_offset ;

   for ( j = 0 ; j < 32 ; j++ ){
      /* ブロック内Ｙ座標によるオフセットを計算 [Y coordinate offset is calculated by the block]] */
      height_offset = ( j & 7 ) * 8 ;
      for ( i = 0 ; i < 64 ; i+=8 ){
         /* データ格納先オフセットを計算 [[Calculate the offset where the data storage]] */
         image_offset = i + j * 64 ;
         /* ブロックによるオフセットを計算 [[Calculate the offset of the block]] */
         block_offset = GetImageBlockNumber32bit( i, j, 64 ) * 64 ;
         /* ８ピクセル分書き込み [[8 pixels of writing]] */
         for ( k = 0 ; k < 8 ; k++ ){
            // just swapped the sides here
            dst[ block_offset + height_offset + k ] = src[ image_offset + k ] ;
         }
      }
   }
}

// this code was ripped from ConvertBlockImageClut32
void ConvertBlockImageToClut32( void *src, void *dst, int offset, int num )
{
   static int	trans_offset_data[16] = {
      64*0+ 0,64*1+ 0,64*0+16,64*1+16,
      64*0+32,64*1+32,64*0+48,64*1+48,
      64*2+ 0,64*3+ 0,64*2+16,64*3+16,
      64*2+32,64*3+32,64*2+48,64*3+48,
   };
   unsigned int	*data, *clut ;
   int				i, j ;

   clut = dst ;
   data = src ;
   for ( j = offset ; j < num ; j++ ){
      for ( i = 0 ; i < 16 ; i++ ){
         // just swapped the sides here
          *clut++ = data[ trans_offset_data[ j ] + i ];
      }
   }
}

void ConvertBlockImage32to4( void *dst, void *src )
{
   int		x, y ;
   unsigned int	*data ;
   unsigned char	*index ;
   int		palette ;
   int		offset, shift ;

   data = src ;
   index = dst ;
   /* ３２×１６の４ビットブロックイメージを３２ビットブロックイメージに変換 */
   for ( y = 0 ; y < 16 ; y++ ){
      for ( x = 0 ; x < 32 ; x++ ){
         /* インデックスの取り出し [[retrieving index]] */
         /* 格納オフセット位置＆シフト量を算出 [[Calculates the storage offset and the amount of shift]] */
         offset = ( y / 4 ) * 16 ;
         offset += x & 7 ;
         shift = x & 0x18 ;
         if ( y & 1 ){
            offset += 8 ;
         }
         if ( y & 2 ){
            offset ^= 0x04 ;
            shift += 4 ;
         }
         if ( y & 4 ){
            offset ^= 0x04 ;
         }
         /* データの書き込み [[Write data]] */
         palette = (data[offset]>>shift)&0x0f;
         if ( ( x & 1 ) == 0 ){
            *index = palette;
         } else {
            *index |= palette<<4;
            index++ ;
         }
      }
   }
}

void ConvertBlockImage32to8( void *dst, void *src )
{
   int		x, y ;
   unsigned int	*data ;
   unsigned char	*index ;
   int		offset, shift ;

   data = src ;
   index = dst ;
   /* １６×１６の８ビットブロックイメージを３２ビットブロックイメージに変換 [[Convert 16x16 8-bit block image to 32-bit block image]] */
   for ( y = 0 ; y < 16 ; y++ ){
      for ( x = 0 ; x < 16 ; x++ ){
         /* 格納オフセット位置＆シフト量を算出 [[Calculates the storage offset and the amount of shift]] */
         offset = ( y / 4 ) * 16 ;
         offset += x & 7 ;
         shift = ( x & 0x08 ) * 2 ;
         if ( y & 1 ){
            offset += 8 ;
         }
         if ( y & 2 ){
            offset ^= 0x04 ;
            shift += 8 ;
         }
         if ( y & 4 ){
            offset ^= 0x04 ;
         }
         /* データの書き込み [[Write data]] */
         *index = (data[ offset ] >> shift) & 0xff;
         index++ ;
      }
   }
}

void ConvertBlockImage32to16( void *dst, void *src )
{
   int		x, y ;
   unsigned int	*data ;
   unsigned short	*color ;
   int		offset, shift ;

   data = src ;
   color = dst ;
   /* １６×８の１６ビットブロックイメージを３２ビットブロックイメージに変換 [[Convert 16x8 16-bit block image to 32-bit block image]] */
   for ( y = 0 ; y < 8 ; y++ ){
      for ( x = 0 ; x < 16 ; x++ ){
         /* 格納オフセット位置＆シフト量を算出 [[Calculates the storage offset and the amount of shift]] */
         offset = y * 8 ;
         offset += x & 7 ;
         shift = ( x & 0x08 ) * 2 ;
         /* データの書き込み [[Write data]] */
         *color = (data[ offset ] >> shift) & 0xffff;
         color++ ;
      }
   }
}

void ConvertBlockImage32to32_forUnpack( void *dst, void *src )
{
   int		x, y ;
   unsigned int	*data ;
   unsigned int	*color ;
   int		offset ;

   data = src ;
   color = dst ;
   /* ８×８の３２ビットブロックイメージを３２ビットブロックイメージに変換（何もしない） [[8x8 32-bit block images into 32-bit block images (nothing)]] */
   for ( y = 0 ; y < 8 ; y++ ){
      for ( x = 0 ; x < 8 ; x++ ){
         /* 格納オフセット位置＆シフト量を算出 [[Store offset and calculate the position shift]] */
         offset = y * 8 ;
         offset += x ;
         /* データの書き込み [[Write data]] */
         *color = data[offset];
         color++ ;
      }
   }
}

#if 0
/* ベースとなるテクスチャ設定値を生成 [[Underlying texture generation settings]] */
tex = 0 ;/* ベースアドレス [[Base address]] */
tex |= ( info->width / 64 ) << 14 ;	/* TBW */
tex |= tex_mode[ info->mode & 7 ] << 20 ;	/* PSM */
tex |= (unsigned long64)info->w_exp << 26 ;	/* TW */
tex |= (unsigned long64)info->h_exp << 30 ;	/* TH */
tex |= (unsigned long64)1 << 34 ;	/* TCC(RGBA) */
tex |= (unsigned long64)0 << 35 ;	/* TFX(MODULATE) */
tex |= (unsigned long64)0 << 37 ;	/* CBP */
tex |= (unsigned long64)0 << 51 ;	/* CPSM(PSMCT32) */
tex |= (unsigned long64)0 << 55 ;	/* CSM(CSM1) */
tex |= (unsigned long64)0 << 56 ;	/* CSA */
tex |= (unsigned long64)0 << 61 ;	/* CLD(???) */
#endif

typedef struct
{
   unsigned int base_address        : 14;
   unsigned int block_width         : 6;
   unsigned int ps_mode             : 6;
   unsigned int w_exp               : 4;
   unsigned int h_exp_lo            : 2;

   unsigned int h_exp_hi            : 2;
   unsigned int tcc_rgba            : 1;
   unsigned int tfx_modulate        : 2;
   unsigned int clut_base_p         : 14;
   unsigned int cpsm_psmct32        : 4;
   unsigned int csm_csm1            : 1;
   unsigned int clut_offset         : 5;
   unsigned int cld                 : 3;
}
tex0_bits;

#if 0
tex2 = 0;	/* TEX2レジスタ [[Register]] */
tex2 |= tex_mode[ 1 ] << 20 ;		/* PSM TEX2は 8bit mode 固定 [[Fixed]] */
tex2 |= (unsigned long64)0 << 37 ;	/* CBP */
tex2 |= (unsigned long64)0 << 51 ;	/* CPSM(PSMCT32) */
tex2 |= (unsigned long64)0 << 55 ;	/* CSM(CSM1) */
tex2 |= (unsigned long64)0 << 56 ;	/* CSA */
tex2 |= (unsigned long64)4 << 61 ;	/* CLD(???) */
#endif

typedef struct
{
   unsigned int _register           : 20;
   unsigned int tex_mode            : 12;
   unsigned int tex_mode_hi         : 5;
   unsigned int clut_base_p         : 14;
   unsigned int cpsm_psmct32        : 4;
   unsigned int csm_csm1            : 1;
   unsigned int clut_offset         : 5;
   unsigned int cld                 : 3;
}
tex2_bits;

typedef struct
{
   char caName[252];
   int id;
}
SDictionaryEntry;

typedef struct 
{
   SDictionaryEntry *el;
   int size;
}
SDictionary;

char *strip_extension(char *caName)
{
   // remove file extension
   {
      int i = strlen(caName);
      while (--i > 0)
      {
         if (caName[i] == '.')
         {
            caName[i] = 0;
            break;
         }
      }
   }
   return caName;
}

SDictionary try_read_index_file(void)
{
   int dictionaryCapacity = 0;
   FILE	*fp;
   SDictionary dictionary;
   dictionary.el = NULL;
   dictionary.size = 0;

   // attempt to read index file
   fp = fopen("idlist.txt", "rt");
   if (fp != NULL)
   {
      char caLine[512];
      char caName[256];
      int id;
      while (fgets(caLine, sizeof(caLine), fp) != NULL)
      {
         if (sscanf(caLine, "%260s => %*s : %d", caName, &id) == 2)
         {
            if (dictionary.size == dictionaryCapacity)
            {
               if (dictionaryCapacity == 0)
               {
                  dictionaryCapacity = 256;
                  dictionary.el = (SDictionaryEntry *) malloc(dictionaryCapacity*sizeof(SDictionaryEntry));
               }
               else
               {
                  SDictionaryEntry *newDic;
                  newDic = (SDictionaryEntry *) malloc(2*dictionaryCapacity*sizeof(SDictionaryEntry));
                  memcpy(newDic, dictionary.el, dictionaryCapacity*sizeof(SDictionaryEntry));
                  dictionary.el = newDic;
                  dictionaryCapacity *= 2;
               }
            }
            strip_extension(caName);
            strcpy(dictionary.el[dictionary.size].caName, caName);
            dictionary.el[dictionary.size].id = id;
            ++dictionary.size;
         }

      }
      fclose(fp);
   }
   return dictionary;
}

char *try_get_filename_for_id(SDictionary *dic, int id)
{
   int i;
   for (i = 0; i < dic->size; ++i)
   {
      SDictionaryEntry *elen = &dic->el[i];
      if (elen->id == id)
      {
         return dic->el[i].caName;
      }
   }
   return NULL;
}

void try_free_dictionary(SDictionary *dic)
{
   if (dic->el != NULL)
   {
      free(dic->el);
      dic->el = NULL;
      dic->size = 0;
   }
}

const char * const kColorString[] = { "Cs", "Cd", "0", "Reserved" };
const char * const kAlphaString[] = { "As", "Ad", "FIX", "Reserved" };

void gather_register_usage_from_reg(unsigned int texId, DG_GSREG *gsreg)
{
   switch(gsreg->reg)
   {
   case SCE_GS_ALPHA_1:
      {
         __int64 alphaData = gsreg->data & 0xFF;
         int a = alphaData & 3;
         int b = (alphaData >> 2) & 3;
         int c = (alphaData >> 4) & 3;
         int d = (alphaData >> 6) & 3;
         int fix = (gsreg->data >> 32) & 0xFF;

         printf(";SCE_GS_ALPHA_1(%d, %d, %d, %d);FIX=%d", a, b, c, d, fix);
         printf(";Cv = ((%s - %s)*%s)>>7 + %s", kColorString[a], kColorString[b], kAlphaString[c], kColorString[d]);
      }
      break;
   
   case SCE_GS_CLAMP_1:
      break;

   case SCE_GS_TEX0_1:
      {
         int const psm = (gsreg->data >> 20) & 63;
         int const tcc = (gsreg->data >> 34) & 1;
         int const tfx = (gsreg->data >> 35) & 3;
         int const csm = (gsreg->data >> 55) & 1;
         int const cpsm = (gsreg->data >> 51) & 15;
         int const cld = (gsreg->data >> 61) & 7;

         if( tcc == 0 )
            printf(";TCC:RGB");
         else if( tcc == 1 )
            printf(";TCC:RGBA");

         switch(tfx)
         {
         case 0: printf(";TFX:MODULATE"); break;
         case 1: printf(";TFX:DECAL"); break;
         case 2: printf(";TFX:HIGHLIGHT"); break;
         case 3: printf(";TFX:HIGHLIGHT2"); break;
         }

      }
      break;

   case SCE_GS_TEX1_1:
      break;

   case SCE_GS_TEX2_1:
      break;

   default:
      printf(";Reg: %llx Data: %llx", gsreg->reg, gsreg->data);
      break;

   }
}

int dg_tex_cmp( void const *a, void const *b )
{
   return memcmp( a, b, sizeof( DG_TEX ) );
}

void GatherTextureStatsFromTRIFile(char *filename, EGame game, EGetStats getStats)
{
   TRI_FILEHEADER	header;
   DG_TEX *tex;
   unsigned int i, j;
   char folderName[260];
   char triName[260];
   FILE *fp = fopen(filename, "rb");

   if (fp == NULL)
   {
      fprintf(stderr, "TRI file '%s' could not be opened!\n", filename);
   }
   else
   {
      strcpy(folderName, filename);
      strip_extension(folderName);

      {
         char* lastSlash = strrchr(folderName, '\\');
         if( lastSlash == NULL )
            strcpy(triName, folderName);
         else
            strcpy(triName, lastSlash + 1);
      }
      

      fread(&header, sizeof(TRI_FILEHEADER), 1, fp);

      tex = (DG_TEX *) malloc(sizeof(DG_TEX)*header.n_textures);
      if (tex == NULL)
      {
         fprintf(stderr, "Could not allocate %d texture headers!\n", header.n_textures);
      }
      else
      {
         char *c = (char *) tex;

         fread(tex, sizeof(DG_TEX), header.n_textures, fp);

         if ( getStats == kGetStats_UniqueTex )
         {
            for (i = 0; i < header.n_textures; ++i)
            {
               tex[i].pad[0] = 0;
            }

            qsort( tex, header.n_textures, sizeof( DG_TEX ), dg_tex_cmp );
         }

         for (i = 0; i < header.n_textures; ++i)
         {
            // DG_GSREG consists of 2 64 bit words, reg and data
            // reg is an index into the PS2 general purpose register table
            // Then the data is the value that should be poked into that register
            // 
            
            DG_TEX_TRANS *trans = &tex[i].tex_trans;
            DG_GSREG *regs = &trans->clamp;
            unsigned int numRegs = (game == kGame_MGS3) ? 5 : 4;
            
            printf("\"%8.8x\";%08x.tga", tex[i].tex_id, tex[i].tex_id);

            for (j = 0; j < numRegs; ++j)
            {
               gather_register_usage_from_reg(tex[i].tex_id, &regs[j]);
            }

            printf("\n");
            if ( getStats == kGetStats_UniqueTex )
            {
               printf( "  uv off: %g %g uv scale: %g %g\n", 
                  tex[i].u_offset, tex[i].v_offset,
                  tex[i].u_scale, tex[i].v_scale );
            }
         }

         if ( getStats == kGetStats_UniqueTex )
         {
            printf( "Header hex data:\n" );
            for ( i = 0; i < sizeof( DG_TEX ) * header.n_textures; ++i )
            {
               printf( "%2.2x", c[i] );
            }
            printf( "\n" );
         }

         free(tex);
      }
      fclose(fp);
   }
}

static void debug_print( char const *debugText, int start, int size )
{
#if 0
   printf( "%s - %d bytes at %d (%x - %x)\n",
      debugText, 
      size, 
      start,
      start,
      start+ size );
#endif
}

// reversed version of WriteTextureRowImageFile
void ReadTextureRowImageFile(char *filename, EGame game, ETextureName useWhat, EExtractWhat extractWhat)
{
   TRI_FILEHEADER	header;
   FILE	*fp;
   DG_TEX *tex;
   int header_size;
   unsigned int i, num;
   unsigned long32 buffer[ 64 * 32 ];
   static int x = 0;
   char folderName[260];
   char exportedTexName[260];
   SDictionary dictionary = try_read_index_file();

   fp = fopen(filename, "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "TRI file '%s' could not be opened!\n", filename);
   }
   else
   {
      if (extractWhat == kExtractTextures)
      {
         strcpy(folderName, filename);
         strip_extension(folderName);
         _mkdir(folderName);
      }

      fread(&header, sizeof(TRI_FILEHEADER), 1, fp);

      tex = (DG_TEX *) malloc(sizeof(DG_TEX)*header.n_textures);
      if (tex == NULL)
      {
         fprintf(stderr, "Could not allocate %d texture headers!\n", header.n_textures);
      }
      else
      {
         fread(tex, sizeof(DG_TEX), header.n_textures, fp);

         header_size = sizeof(TRI_FILEHEADER);
         header_size += sizeof( DG_TEX ) * header.n_textures;
         header_size = ( header_size + 127 ) & 0xffff80;

         fseek(fp, header_size, SEEK_SET);

         for ( i = 0 ; i < header.tex_size ; i += 32 )
         {
            debug_print( "fread 1", ftell( fp ), sizeof( long32 ) * 64 * 32 );

            fread( buffer, sizeof( long32 ), 64 * 32, fp );
            ConvertImageDataToBlockSet( buffer, &VramImage[ 64 * i ] );
         }

         if ( header.clut_size != 0 ){
            for ( i = 0 ; i < header.clut_size ; i += 32 )
            {
               debug_print( "fread 2", ftell( fp ), sizeof( long32 ) * 64 * 32 );

               fread( buffer, sizeof( long32 ), 64 * 32, fp );
               ConvertImageDataToBlockSet( buffer, &VramImage[ 64 * ( header.clut_offset + i ) ] );
            }
         }

         num = 0;
         while (num < header.n_textures)
         {
            DG_TEX *texx = &tex[num];

            // skip the textures that were added but don't have pixel data in the TRI
            if (texx->flag & DG_TEXFLAG_INJECTED_TEX)
            {
               num++;
               continue;
            }

            {
               tex0_bits *t0 = (tex0_bits *)(&texx->tex_trans.tex0.data);
               tex0_bits *t0_for_sheet = t0;
               int mode = psm_to_mode(t0->ps_mode);
               int infoWidth = (1<<t0->w_exp);
               int h_exp = (t0->h_exp_hi<<2) | t0->h_exp_lo;
               int infoHeight = (1<<h_exp);
               /* ブロックのサイズを算出 [[Calculate the size of the block]] */
               int dx = block_size[mode][0];
               int dy = block_size[mode][1];
               /* テクスチャのブロック数を算出 [[Calculate the number of blocks of texture]] */
               int width = infoWidth/dx;
               int height = infoHeight/dy;
               int		i, j, k, l, block_num, offset ;
               unsigned int *image_data = malloc(sizeof(long)*infoWidth*infoHeight);
               unsigned int clut[256];
               unsigned long32	block_buffer[ 64 ], *d_addr, *s_addr ;
               int bitsPerPixel = 32;
               assert(sizeof(tex0_bits) == sizeof(u_long64));

               /* テクスチャイメージデータの書き出し [[Export texture image data]] */
               for ( j = 0 ; j < height ; j++ ){
                  for ( i = 0 ; i < width ; i++ ){
                     switch ( mode ){
                     case 0:
                        {
                           // texture block with 4-bit CLUT

                           /* そのブロックの該当ブロック番号を取得 [[Get the block number corresponding to the block]] */
                           block_num = GetImageBlockNumber4bit( i * dx, j * dy, infoWidth ) ;
                           /* ブロック内のデータを３２カラー形式に変換 [[32 color format data in the block]] */
                           ConvertBlockImage32to4( block_buffer, &VramImage[ ( t0->base_address + block_num ) * 64 ] );

                           // pull one block of data from the buffer
                           /* １ブロック分のデータをバッファに転送 [[Transfer one block of data to the buffer]] */
                           offset = infoWidth / 8 ;
                           d_addr = block_buffer ;
                           s_addr = (unsigned long32*)image_data + i * dx / 8 + j * dy * offset ;
                           for ( l = 0 ; l < 16 ; l++ ){
                              for ( k = 0 ; k < 4 ; k++ ){
                                 s_addr[k] = *d_addr++ ;
                              }
                              s_addr += offset ;
                           }
                           break;
                        }
                     case 1:
                        {
                           // texture block with 8-bit CLUT

                           /* そのブロックの該当ブロック番号を取得 */
                           block_num = GetImageBlockNumber8bit( i * dx, j * dy, infoWidth ) ;
                           /* ブロック内のデータを３２カラー形式に変換 */
                           ConvertBlockImage32to8( block_buffer, &VramImage[ ( t0->base_address + block_num ) * 64 ] );

                           offset = infoWidth / 4 ;
                           d_addr = block_buffer ;
                           s_addr = (unsigned long32*)image_data + i * dx / 4 + j * dy * offset ;
                           for ( l = 0 ; l < 16 ; l++ ){
                              for ( k = 0 ; k < 4 ; k++ ){
                                 s_addr[k] = *d_addr++ ;
                              }
                              s_addr += offset ;
                           }
                           break;
                        }
                     case 2:
                        {
                           // texture block, 16-bit color

                           /* そのブロックの該当ブロック番号を取得 */
                           block_num = GetImageBlockNumber16bit( i * dx, j * dy, infoWidth ) ;
                           /* ブロック内のデータを３２カラー形式に変換 */
                           ConvertBlockImage32to16( block_buffer, &VramImage[ ( t0->base_address + block_num ) * 64 ] );

                           offset = infoWidth / 2 ;
                           d_addr = block_buffer ;
                           s_addr = (unsigned long32*)image_data + i * dx / 2 + j * dy * offset ;
                           for ( l = 0 ; l < 8 ; l++ ){
                              for ( k = 0 ; k < 8 ; k++ ){
                                 s_addr[k] = *d_addr++ ;
                              }
                              s_addr += offset ;
                           }
                           break;
                        }
                     case 3:
                        {
                           // texture block, 32-bit color

                           /* そのブロックの該当ブロック番号を取得 */
                           block_num = GetImageBlockNumber32bit( i * dx, j * dy, infoWidth ) ;
                           /* ブロック内のデータを３２カラー形式に変換 */
                           ConvertBlockImage32to32_forUnpack( block_buffer, &VramImage[ ( t0->base_address + block_num ) * 64 ] );

                           offset = infoWidth ;
                           d_addr = block_buffer ;
                           s_addr = (unsigned long32*)image_data + i * dx + j * dy * offset ;
                           for ( l = 0 ; l < 8 ; l++ ){
                              for ( k = 0 ; k < 8 ; k++ ){
                                 s_addr[k] = *d_addr++ ;
                              }
                              s_addr += offset ;
                           }
                           break;
                        }
                     }
                  }
               }

               // write out the whole texture sheet with the first palette as a test/reference
               if (extractWhat == kExtractSheets)
               {
                  sprintf(exportedTexName, "%s_%d.tga", filename, num);
                  switch (mode)
                  {
                  case 0:
                     // 4-bit CLUT
                     ConvertBlockImageToClut32(&VramImage[64*(header.clut_offset + t0->clut_base_p)], clut, t0->clut_offset, t0->clut_offset + 1);
                     bitsPerPixel = 4;
                     break;
                  case 1:
                     // 8-bit CLUT
                     ConvertBlockImageToClut32(&VramImage[64*(header.clut_offset + t0->clut_base_p)], clut, 0, 16);
                     bitsPerPixel = 8;
                     break;
                  case 2:
                     bitsPerPixel = 16;
                     break;
                  case 3:
                     bitsPerPixel = 32;
                     break;
                  }
                  SaveBitmapToFile(image_data, infoWidth, infoHeight, bitsPerPixel, clut, 0, 0, infoWidth, infoHeight, exportedTexName, kRemapAlpha_Do);
               }

               do 
               {
                  if (extractWhat == kExtractTextures)
                  {
                     // see mgs3/source/system/libdg/texture.c for both paths
                     int u, v, uw, vh;
                     assert(game == kGame_MGS2 || game == kGame_MGS3);
                     if (game == kGame_MGS2)
                     {
                        uw = (int)(1.1f + texx->u_scale*infoWidth);
                        vh = (int)(1.1f + texx->v_scale*infoHeight);
                        u = (int)(texx->u_offset*infoWidth - 0.4f);
                        v = (int)(texx->v_offset*infoHeight - 0.4f);
                     }
                     else
                     {
                        uw = (int)(0.5f + texx->u_scale*infoWidth);
                        vh = (int)(0.5f + texx->v_scale*infoHeight);
                        u = (int)(16.0f*texx->u_offset*infoWidth + 0.5f);
                        v = (int)(16.0f*texx->v_offset*infoHeight + 0.5f);
                        /* 0.5ドット内側に修正しているかをチェック */
                        /* [[ 0.5 check that the correct side of the dots ]] (No, I don't know what that means either) */
                        if ((u & 0x0f) || (v & 0x0f))
                        {
                           ++uw;
                           ++vh;
                        }
                        u >>= 4;
                        v >>= 4;
                     }

                     switch (mode)
                     {
                     case 0:
                        // 4-bit CLUT
                        ConvertBlockImageToClut32(&VramImage[64*(header.clut_offset + t0->clut_base_p)], clut, t0->clut_offset, t0->clut_offset + 1);
                        bitsPerPixel = 4;
                        break;
                     case 1:
                        // 8-bit CLUT
                        ConvertBlockImageToClut32(&VramImage[64*(header.clut_offset + t0->clut_base_p)], clut, 0, 16);
                        bitsPerPixel = 8;
                        break;
                     case 2:
                        bitsPerPixel = 16;
                        break;
                     case 3:
                        bitsPerPixel = 32;
                        break;
                     }

                     // now write out textures
                     sprintf(exportedTexName, "%s/%08x.tga", folderName, texx->tex_id);
                     if (useWhat == kUseName)
                     {
                        char *originalName = try_get_filename_for_id(&dictionary, texx->tex_id);
                        if (originalName != NULL)
                        {
                           sprintf(exportedTexName, "%s/%s.tga", folderName, originalName);
                        }
                     }
                     // use the UVs to extract the texture, if necessary
                     printf("[%d/%d] Tex %s xywh %d %d %d %d clut base %d clut off %d\n", num, header.n_textures, exportedTexName, u, v, uw, vh, t0->clut_base_p, t0->clut_offset);
                     SaveBitmapToFile(image_data, infoWidth, infoHeight, bitsPerPixel, clut, u, v, uw, vh, exportedTexName, kRemapAlpha_Dont);
                  }

                  num++;
                  if (num >= header.n_textures)
                  {
                     break;
                  }
                  texx = &tex[num];
                  t0 = (tex0_bits *)(&texx->tex_trans.tex0.data);
                  mode = psm_to_mode(t0->ps_mode);
               }
               while (t0->base_address == t0_for_sheet->base_address);

               free(image_data);
            }
         }
      }

      free(tex);
      fclose(fp);
   }

   try_free_dictionary(&dictionary);
}

void ReadTextureRowImageFiles(EGame game, ETextureName useWhat, EExtractWhat extractWhat)
{
   struct _finddata32_t finddata;
   int handle = _findfirst32("*.tri", &finddata);
   if (handle != -1)
   {
      do
      {
         ReadTextureRowImageFile(finddata.name, game, useWhat, extractWhat);
      }
      while (_findnext32(handle, &finddata) != -1);
      _findclose(handle);
   }
}

void GatherTextureStatsFromTRIFiles(EGame game, EGetStats getStats)
{
   struct _finddata32_t finddata;
   int handle = _findfirst32("*.tri", &finddata);
   if (handle != -1)
   {
      do
      {
         GatherTextureStatsFromTRIFile(finddata.name, game, getStats );
      }
      while (_findnext32(handle, &finddata) != -1);
      _findclose(handle);
   }
}

void BP_RebuildTextureRowImageFile(char *filename, EGame game)
{
   TRI_FILEHEADER	header;
   FILE	*fp;
   int header_size;
   char folderName[260];
   char rebuiltTriFilename[260];
   char * headerBuf;
   char * texBuf;
   char * clutBuf;
   int tex_size;
   int clut_size;
   int srcsize, dstsize;
   unsigned int i;

   if( strstr( filename, "_bp/" ) || strstr( filename, "_bp\\" ) )
   {
      //skip already rebuilt files.
      printf("Skipping already rebuilt %s\n", filename );
      return;
   }

   printf("Rebuilding %s\n", filename );
   strcpy( folderName, filename );
   {
      char * rebuiltFilenameAt = strrchr( folderName, '/' );
      if( !rebuiltFilenameAt )
      {
         rebuiltFilenameAt = strrchr( folderName, '\\' );
      }

      if( !rebuiltFilenameAt )
      {
         rebuiltFilenameAt = folderName;
      }
      else
      {
         ++rebuiltFilenameAt;
      }
      *rebuiltFilenameAt = 0;
      strcat( folderName, "_bp/" );
      _mkdir( folderName );
      strcpy( rebuiltTriFilename, folderName );
      strcat( rebuiltTriFilename, filename + (rebuiltFilenameAt-folderName) );
   }

   fp = fopen(filename, "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "TRI file '%s' could not be opened for reading!\n", filename);
      return;
   }

   //Pass the header through unaltered, then write out the appropriate amount of zero memory
   //for tex and clut data.
   fread(&header, sizeof(TRI_FILEHEADER), 1, fp);

//   tex = (DG_TEX *) malloc(sizeof(DG_TEX)*header.n_textures);

//   fread(tex, sizeof(DG_TEX), header.n_textures, fp);

   header_size = sizeof(TRI_FILEHEADER);
   header_size += sizeof( DG_TEX ) * header.n_textures;
   header_size = ( header_size + 127 ) & 0xffff80;

   headerBuf = (char*)malloc( header_size );
   memcpy( headerBuf, &header, sizeof( header ) );
   fread( headerBuf+sizeof(header), 1, header_size-sizeof(header), fp );

   tex_size = 0;
   for ( i = 0 ; i < header.tex_size ; i += 32 )
   {
      tex_size += sizeof( long32 ) * 64 * 32;
   }

   clut_size = 0;
   for ( i = 0 ; i < header.clut_size ; i += 32 )
   {
      clut_size += sizeof( long32 ) * 64 * 32;
   }

   texBuf = (char*)malloc( tex_size );
   memset( texBuf, 0, tex_size );

   clutBuf = (char*)malloc( clut_size );
   memset( clutBuf, 0, clut_size );

   fseek( fp, 0, SEEK_END );
   srcsize = ftell( fp );

   fclose(fp);
   fp = fopen(rebuiltTriFilename, "wb");
   if (fp == NULL)
   {
      fprintf(stderr, "TRI file '%s' could not be opened for writing!\n", rebuiltTriFilename);

      free( headerBuf );
      free( texBuf );
      free( clutBuf );

      return;
   }

   fwrite( headerBuf, header_size, 1, fp );
   fwrite( texBuf, tex_size, 1, fp );
   fwrite( clutBuf, clut_size, 1, fp );
   
//   printf( "Rewrite Sizes: %d %d %d - hdr %d %d\n", header_size, tex_size, clut_size, header.tex_size, header.clut_size );
   
   dstsize = ftell( fp );
   fclose( fp );

   free( headerBuf );
   free( texBuf );
   free( clutBuf );
   if( srcsize != dstsize )
   {
      fprintf(stderr, "Error: rebuilt TRI file '%s' size does not match src!\n", rebuiltTriFilename);
      exit(1);
   }
}

static unsigned _decode_hex( char const *str )
{
   unsigned sofar = 0;
   char const *cnt;
   for ( cnt = str; *cnt; ++cnt )
   {
      char c = tolower( *cnt );
      unsigned val = 0;

      if ( c >= '0' && c <= '9' )
      {
         val = c - '0';
      }
      else if ( c >= 'a' && c <= 'f' )
      {
         val = 10 + ( c - 'a' );
      }

      sofar = ( sofar << 4 ) | val;
   }

   return sofar;
}

static unsigned int _get_strcode_of_pathed_name(char *str)
{
   unsigned char c;
   unsigned char *p;
   unsigned int id = 0;

   if ( !strncmp( str, "id:", 3 ) )
   {
      // We have a hex value
      return _decode_hex( str + 3 );
   }

   // find the name only
   p = ( unsigned char * )(str + strlen(str) - 1);
   while (p != str)
   {
      if (*p == '\\' || *p == '/')
      {
         ++p;
         break;
      }
      --p;
   }

   while( ( c = *( p++ ) ) != '.' )
   {
      id = ( id << 5 ) | ( id >> 19 );
      id += tolower(c);
      id &= 0x00ffffff;
   }
   if( id == 0 ) id = 1;

   return id;
}

static int round_up_32( int i )
{
   return ( i + 31 ) & (~31);
}

static void BP_TexAndClutSizesFromHeader( TRI_FILEHEADER const *pHeader, int *tex_size, int *clut_size )
{
   *tex_size = (round_up_32( pHeader->tex_size ) / 32)*(sizeof( long32 ) * 64 * 32);
   *clut_size = (round_up_32( pHeader->clut_size ) / 32)*(sizeof( long32 ) * 64 * 32);
}

static void BP_RewriteTRIFile( 
   char const *filename, 
   TRI_FILEHEADER *pHeader,
   DG_TEX const *tex, 
   int const texCount,
   void const *texBuf,
   void const *clutBuf )
{
   FILE *fp = fopen(filename, "wb");
   if (fp != NULL)
   {
      TRI_FILEHEADER header = *pHeader;
      int header_size;
      int header_padded_size;
      int tex_size;
      int clut_size;
      char padding[128] = { 0 };

      BP_TexAndClutSizesFromHeader( pHeader, &tex_size, &clut_size );

      header.n_textures = texCount;

      header_size = sizeof(TRI_FILEHEADER) + sizeof(DG_TEX) * header.n_textures;
      header_padded_size = ( header_size + 127 ) & 0x7fffff80;

      header.texel_addr = header_padded_size;
      header.clut_addr = header_padded_size + tex_size;

      fwrite(&header, sizeof(TRI_FILEHEADER), 1, fp);
      fwrite(tex, sizeof(DG_TEX), header.n_textures, fp);
      fwrite(padding, header_padded_size - header_size, 1, fp);
      fwrite(texBuf, tex_size, 1, fp);
      fwrite(clutBuf, clut_size, 1, fp);
      
//      printf( "Fix Sizes: %d %d %d - hdr %d %d\n", header_padded_size, tex_size, clut_size, pHeader->tex_size, pHeader->clut_size );
      
      fclose(fp);
   }
   else
   {
      fprintf(stderr, "TRI file '%s' could not be opened for writing!\n", filename);
      fprintf(stderr, " It probably needs to be checked out first.\n");
   }

}

void BP_FixDupesAndRemoveFilesInTRI( char const *filename, char *textureFilenames[], unsigned int numTextures )
{
   DG_TEX *pSrcTex;
   DG_TEX *pDstTex;
   TRI_FILEHEADER	header;
   int newTextureCount = 0;
   unsigned i;
   FILE *fp;
   unsigned *strcodes;

   if( strstr( filename, "_bp/" ) || strstr( filename, "_bp\\" ) )
   {
      // don't add
      printf("Don't add to the rebuilt files - add to the source and then rebuild %s\n", filename );
      return;
   }

   fp = fopen(filename, "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "TRI file '%s' could not be opened for reading!\n", filename);
      return;
   }

   strcodes = malloc( numTextures * sizeof( unsigned ) );

   for ( i = 0; i < numTextures; ++i )
   {
      strcodes[i] = _get_strcode_of_pathed_name( textureFilenames[i] );
   }

   fread(&header, sizeof(TRI_FILEHEADER), 1, fp);

   pSrcTex = (DG_TEX *) malloc(sizeof(DG_TEX)*(header.n_textures) );
   pDstTex = (DG_TEX *) malloc(sizeof(DG_TEX)*(header.n_textures) );

   // read it this way for debugging porpoises
   for (i = 0; i < header.n_textures; ++i)
   {
      fread(pSrcTex + i, sizeof(DG_TEX), 1, fp);
   }

   // Search for dupes
   for ( i = 0; i < header.n_textures; ++i )
   {
      int isValid = 1;
      if ( pSrcTex[i].flag == DG_TEXFLAG_INJECTED_TEX )
      {
         unsigned j;

         // See if this is one of the textures we passed in
         for ( j = 0; j < numTextures; ++j )
         {
            if ( pSrcTex[i].tex_id == strcodes[j] )
            {
               printf( "%8.8x\n", strcodes[j] );
               isValid = 0;
               printf( "Deleting: %8.8x\n", pSrcTex[i].tex_id );
            }
         }

         if ( isValid )
         {
            // If we haven't failed, find dupes
            for ( j = i + 1; j < header.n_textures; ++j )
            {
               if ( pSrcTex[j].flag == DG_TEXFLAG_INJECTED_TEX && pSrcTex[j].tex_id == pSrcTex[i].tex_id )
               {
                  printf( "Found dupe: %8.8x\n", pSrcTex[i].tex_id );
                  isValid = 0;
                  break;
               }
            }
         }
      }

      if ( isValid )
      {
         // Only write this if it's not duplicated later

         pDstTex[newTextureCount++] = pSrcTex[i];
      }
   }

//   if ( newTextureCount != header.n_textures )
   {
      int tex_size;
      int clut_size;
      char *texBuf;
      char *clutBuf;

      // We had duped textures - write it out
      if ( newTextureCount != header.n_textures )
      {
         printf( "Fixing: %s\n", filename );
      }
      else
      {
         printf( "Refreshing: %s\n", filename );
      }

      BP_TexAndClutSizesFromHeader( &header, &tex_size, &clut_size );

      texBuf = (char *) malloc( tex_size );
      memset( texBuf, 0, tex_size ); // need to call this, as we could be working from an invalid tri
      fseek(fp, header.texel_addr, SEEK_SET);
      fread( texBuf, tex_size, 1, fp );

      clutBuf = (char *) malloc( clut_size );
      memset( clutBuf, 0, clut_size ); // need to call this, as we could be working from an invalid tri
      fseek(fp, header.clut_addr, SEEK_SET);
      fread( clutBuf, clut_size, 1, fp );

      fclose(fp);

      BP_RewriteTRIFile( filename, &header, pDstTex, newTextureCount, texBuf, clutBuf );

      free( texBuf );
      free( clutBuf );
   }
#if 0
   else
   {
      fclose( fp );
   }
#endif

   free( pDstTex );
   free( pSrcTex );
}

void BP_AddTextureToTextureRowImageFile( char *filename, char *textureFilenames[], unsigned int numTextures, EGame game )
{
   FILE *fp;
   TRI_FILEHEADER	header;
   DG_TEX *tex;
   DG_TEX *tex_head;
   char *texBuf;
   char *clutBuf;
   int tex_size;
   int clut_size;
   unsigned int i;
   int badTextureCount = 0;

   if( strstr( filename, "_bp/" ) || strstr( filename, "_bp\\" ) )
   {
      // don't add
      fprintf(stderr, "Don't add to the rebuilt files - add to the source and then rebuild %s\n", filename );
      return;
   }

   fp = fopen(filename, "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "TRI file '%s' could not be opened for reading!\n", filename);
      return;
   }

   fread(&header, sizeof(TRI_FILEHEADER), 1, fp);

   tex_head = tex = (DG_TEX *) malloc(sizeof(DG_TEX)*(header.n_textures + numTextures));

   // read it this way for debugging porpoises
   for (i = 0; i < header.n_textures; ++i)
   {
      fread(tex, sizeof(DG_TEX), 1, fp);

      tex++;
   }
   // add the new textures
   for (i = 0; i < numTextures; ++i)
   {
      unsigned j = 0;
      unsigned strcode = _get_strcode_of_pathed_name(textureFilenames[i]);
      for ( j = 0; j < header.n_textures; ++j )
      {
         if ( tex_head[j].tex_id == strcode )
         {
            fprintf( stderr, "%s (%8.8x) already in tri file\n", textureFilenames[i], strcode );
            ++badTextureCount;
            break;
         }
      }
      
      memset(tex, 0, sizeof(DG_TEX));
      tex->flag = DG_TEXFLAG_INJECTED_TEX;
      tex->tex_id = strcode;
      tex++;
   }

   if ( badTextureCount > 0 )
   {
      fprintf( stderr, "\n" );
      fprintf( stderr, "Error - %d bad textures - TRI file not touched\n", badTextureCount );
      return;
   }

   // reset tex
   tex -= (header.n_textures + numTextures);

   // a bit of an odd calculation
   // header.tex_size should always be a multiple of 32 anyway
   // ditto for clut_size
   BP_TexAndClutSizesFromHeader( &header, &tex_size, &clut_size );

   texBuf = (char *) malloc( tex_size );
   memset( texBuf, 0, tex_size ); // need to call this, as we could be working from an invalid tri
   fseek(fp, header.texel_addr, SEEK_SET);
   fread( texBuf, tex_size, 1, fp );

   clutBuf = (char *) malloc( clut_size );
   memset( clutBuf, 0, clut_size ); // need to call this, as we could be working from an invalid tri
   fseek(fp, header.clut_addr, SEEK_SET);
   fread( clutBuf, clut_size, 1, fp );

   fclose(fp);

   BP_RewriteTRIFile( filename, &header, tex_head, header.n_textures + numTextures, texBuf, clutBuf );

   free(tex);
   free(texBuf);
   free(clutBuf);
}