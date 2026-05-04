//----------------------------------------------------------------------------
// fontgen.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"

//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "../../BP/Shared/BP_Font.h"

#include "Tools/Helper/Helper.h"

#include "vector"
#include "algorithm"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;
using namespace System::IO;

//----------------------------------------------------------------------------

#pragma warning(disable:4996)

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

void WriteTGA(char* path, unsigned char* bitmapData, int const width, int const height, int const bitsPerPixel, int const pitch)
{
   FILE * fp = fopen( path, "wb" );
   if( !fp )
   {
      printf("Failed to open output file %s!\n", path );
      throw false;
   }

   TGA_HEADER tga_header;
   int backwards = 0;

   memset(&tga_header, 0, sizeof(tga_header));
   tga_header.imagetype = 3;
   tga_header.width = width;
   tga_header.height = height;
   tga_header.bits = bitsPerPixel;
   tga_header.descriptor = backwards ? 0x00 : 0x20;

   fwrite(&tga_header, sizeof(tga_header), 1, fp);
   for( int y = 0; y < height; ++y )
   {
      unsigned char* scanLine = bitmapData + pitch * y;
      int const scanLineLength = (width * bitsPerPixel) / 8;

      fwrite(scanLine, scanLineLength, 1, fp );
   }
   
   fclose(fp);
}

unsigned char * LoadTGA(const char * path, int * pWidth, int *pHeight)
{
   FILE* fp = fopen(path, "rb");
   
   if( fp )
   {
      fseek(fp, 0, SEEK_END);
      int const fileSize = ftell(fp);
      fseek(fp, 0, SEEK_SET);

      void* tgaData = malloc(fileSize);
      fread(tgaData, fileSize, 1, fp);

      TGA_HEADER* header = (TGA_HEADER*)tgaData;

      fclose(fp);

      header->descriptor &= ~(1 << 3);

      if( header->identsize != 0 || header->colourmaptype != 0 || header->imagetype != 3 || header->bits != 8 || header->descriptor != 0 )
      {
         printf("Can't load image (%s), expecting 8 bit gray scale tga\n", path);
         return NULL;
      }

      *pWidth = header->width;
      *pHeight = header->height;

      unsigned char* outputPtr = (unsigned char*)(&header[1]);

      // upside down?
      int const isUpsideDown = (header->descriptor & 0x20) == 0;
      if( isUpsideDown )
      {
         unsigned char* inputPtr = outputPtr;
         outputPtr = (unsigned char*) malloc(header->width * header->height);

         for( int y = 0; y < header->height; ++y )
         {
            int const sourceY = (header->height - 1) - y;

            memcpy(outputPtr + y*header->width, inputPtr + sourceY*header->width, header->width);
         }
      }

      // image data starts immediately following the header.
      return outputPtr;
   }

   return NULL;
}

struct SIcon
{
   SIcon(uint16 unicode, const char* pPath)
      :  mUnicode(unicode)
      ,  mPath(pPath)
   {
   }

   uint16         mUnicode;
   const char*    mPath;

   int            mWidth;
   int            mHeight;
   unsigned char* mpData;
};

SIcon gIcons[] =
{
   SIcon(0xE100, "Icons/button_ps3_x.tga"),
   SIcon(0xE101, "Icons/button_ps3_circle.tga"),
   SIcon(0xE102, "Icons/button_ps3_square.tga"),
   SIcon(0xE103, "Icons/button_ps3_triangle.tga"),
   SIcon(0xE104, "Icons/button_L1.tga"),
   SIcon(0xE105, "Icons/button_L2.tga"),
   SIcon(0xE106, "Icons/button_L3.tga"),
   SIcon(0xE107, "Icons/button_R1.tga"),
   SIcon(0xE108, "Icons/button_R2.tga"),
   SIcon(0xE109, "Icons/button_R3.tga"),

   SIcon(0xE200, "Icons/button_360_a.tga"),
   SIcon(0xE201, "Icons/button_360_b.tga"),
   SIcon(0xE202, "Icons/button_360_x.tga"),
   SIcon(0xE203, "Icons/button_360_y.tga"),
   SIcon(0xE204, "Icons/button_LB.tga"),
   SIcon(0xE205, "Icons/button_LT.tga"),
   SIcon(0xE206, "Icons/button_LS.tga"),
   SIcon(0xE207, "Icons/button_RB.tga"),
   SIcon(0xE208, "Icons/button_RT.tga"),
   SIcon(0xE209, "Icons/button_RS.tga"),
};

int const kIconCount = sizeof(gIcons) / sizeof(gIcons[0]);

void LoadIcons(String^ fontFolderPath)
{
   for( int i = 0; i < kIconCount; ++i )
   {
      SIcon & icon = gIcons[i];

      String^ iconPath = Path::Combine(fontFolderPath, gcnew String(icon.mPath));
      std::string iconPathStr = Helper::StringHelper::ConvertString(iconPath);

      icon.mpData = LoadTGA(iconPathStr.c_str(), &icon.mWidth, &icon.mHeight);
   }
}

void Downsample( unsigned char *dst, unsigned char *bitmap, unsigned int width, unsigned int height, unsigned int yOffset, unsigned int targetWidth, unsigned int targetHeight )
{
   // simple box filter - samples outside the original image are set to 0
   for ( unsigned int y = 0; y < targetHeight; ++y )
   {
      // iterate over target pixels and evaluate.
      for ( unsigned int x = 0; x < targetWidth; ++x )
      {
         int px = 2*x;
         int py = 2*y - (yOffset & 1);

         int s00 = 0, s01 = 0, s10 = 0, s11 = 0;
         if (py >= 0)
         {
            s00 = bitmap[ py * width + px ];
            if (px+1 < width)
            {
               s01 = bitmap[ py * width + px + 1 ];
            }
         }
         if (py + 1 < height)
         {
            // sample the source image.
            s10 = bitmap[ (py + 1) * width + px ];
            if (px+1 < width)
            {
               s11 = bitmap[ (py + 1) * width + px + 1 ];
            }
         }

         // perform filtering.
         float final = 0.25f*(s00 + s01 + s10 + s11);

         // store the final value.
         *dst++ = ( unsigned char )( int )( final + 0.5f );
      }
   }
}

int main(array<System::String ^> ^args)
{
   FT_Library library;
   FT_Error error;

   if( FT_Init_FreeType(&library) )
      return -1;

   String^ fontFolderPath = Environment::ExpandEnvironmentVariables("%BPE_REPOSITORY%\\EngineSupport\\Fonts");

   // for VITA, font size is 30 not 40 - since the screen is 3/4 the size
   int fontSize = 30;
   
   if( args->Length > 0 )
      fontSize = Int32::Parse(args[0]);

   int const originalFontSize = fontSize;

   fontSize *= 2;

   LoadIcons(fontFolderPath);

   // Load europe font.
   FT_Face europeFace;
   {
      String^ fontPath = Path::Combine(fontFolderPath, "ALPSN__.TTF");

      Console::WriteLine("Loading Font: {0}...", fontPath);

      std::string fontPathStr = Helper::StringHelper::ConvertString(fontPath);

      if( FT_New_Face(library, fontPathStr.c_str(), 0, &europeFace) )
      {
         Console::WriteLine("Error: Can't load {0}.", fontPath);
         return -1;
      }

      error = FT_Set_Pixel_Sizes(europeFace, 0, fontSize);
   }

   // Load japan font.
   FT_Face japanFace;
   {
      String^ fontPath = Path::Combine(fontFolderPath, "DF-HeiSeiGothic-W5.ttc");

      Console::WriteLine("Loading Font: {0}...", fontPath);

      std::string fontPathStr = Helper::StringHelper::ConvertString(fontPath);

      if( FT_New_Face(library, fontPathStr.c_str(), 0, &japanFace) )
      {
         Console::WriteLine("Error: Can't load {0}.", fontPath);
         return -1;
      }

      error = FT_Set_Pixel_Sizes(japanFace, 0, fontSize);
   }

   // Load japan fallback font.
   FT_Face japanFallbackFace;
   {
      String^ fontPath = Path::Combine(fontFolderPath, "msgothic.ttc");

      Console::WriteLine("Loading Font: {0}...", fontPath);

      std::string fontPathStr = Helper::StringHelper::ConvertString(fontPath);

      if( FT_New_Face(library, fontPathStr.c_str(), 0, &japanFallbackFace) )
      {
         Console::WriteLine("Error: Can't load {0}.", fontPath);
         return -1;
      }

      error = FT_Set_Pixel_Sizes(japanFallbackFace, 0, fontSize);
   }

   // Get all unicodes to render out
   std::vector<int> characters;
   {
      // Load char map.
      XmlDocument^ charMapDoc = gcnew XmlDocument();
      {
         String^ path = Environment::ExpandEnvironmentVariables("%BPE_REPOSITORY%\\CharacterMap.xml");
         Console::WriteLine("Loading Character Map: {0}...", path);
         charMapDoc->Load(path);
      }

      // Get all characters and convert to unicode
      for each(XmlAttribute^ charAttr in charMapDoc->SelectNodes("//@char") )
      {
         String^ characterString = charAttr->InnerText;
         
         if( characterString->Length == 1 )
         {
            int unicode = Char::ConvertToUtf32(characterString, 0);
            if( unicode >= 32 )
            {
               characters.push_back(unicode);
            }
         }
      }

      // Add all icons to character list.
      for( int i = 0; i < kIconCount; ++i )
         characters.push_back(gIcons[i].mUnicode);

      std::sort(characters.begin(), characters.end());
   }

   // Write out font
   {
      CLabeledOffsetStream output;
      output.SetEndian(COutputStream::kOE_LittleEndian);

      output.AddLabel("Start");

      // Write header
      {
         output.AddLabel("Header");
         output.WriteUint32('FONT');

         output.WriteUint32(originalFontSize);
         output.WriteUint32(originalFontSize);
         output.WriteUint32('DUMY'); // dummy placeholder value for max character descent calculated during writing of characters.
         output.AddOffset32("CharacterTable", "Start");
         output.WriteUint32(characters.size());
      }

      // Write character table
      {
         output.AddLabel("CharacterTable");
         for( int i = 0; i < characters.size(); ++i )
         {
            int const unicode = characters[i];
            
            output.WriteUint32(unicode);
            output.AddOffset32(CStringExtras::Stringize("Char_%d", unicode), "Start");
         }
      }

      int maxCharacterDescent = 0;

      // Write characters
      {
         for(unsigned int characterIdx = 0; characterIdx < characters.size(); ++characterIdx )
         {
            int const unicode = characters[characterIdx];

            SIcon const * pIcon = NULL;
         
            for( int i = 0; i < kIconCount; ++i )
            {
               SIcon const & icon = gIcons[i];
               if( icon.mUnicode == unicode )
               {
                  pIcon = &icon;
                  break;
               }
            }

            unsigned char* pBitmap = NULL;
            unsigned char *pAllocatedBitmap = NULL;
            int outputWidth = 0;
            int outputHeight = 0;
            int outputOffsetX = 0;
            int outputOffsetY = 0;
            int outputAdvance = -1;

            if( pIcon != NULL )
            {
               pBitmap = pIcon->mpData;

               outputWidth = pIcon->mWidth;
               outputHeight = pIcon->mHeight;
             
               int const xPadding = bpe::max_val(0, originalFontSize - outputWidth);
               int const yPadding = bpe::max_val(0, originalFontSize - outputHeight);

               outputOffsetX = xPadding / 2;
               outputOffsetY = yPadding / 2;

               outputAdvance = pIcon->mWidth;
            }
            else
            {
               // Write out character data.

               // Hiragana begins at 0x3040
               // Katakana begins at 0x30a0
               // Kanji is above that "somewhere"
               FT_Face currentFont;

               bool useJapanFont = false;

               if( unicode > 0x2020 )
                  useJapanFont = true;
               
               switch(unicode)
               {
               case 0x00D7:   // Multiplication Symbol
               case 0x2015:   // Horizontal Bar
               case 0x03B5:   // Greek Small Letter Epsilon
                  useJapanFont = true;
                  break;

               case 0x2122:   // TM Symbol
                  useJapanFont = false;
                  break;
               }

               if( useJapanFont )
                  currentFont = japanFace;
               else
                  currentFont = europeFace;

               // Render glyph
               FT_GlyphSlot glyph;
               
               while(true)
               {
                  glyph = currentFont->glyph;

                  FT_UInt glyphIndex = FT_Get_Char_Index(currentFont, unicode);

                  if( glyphIndex == 0 )
                  {
                     if( currentFont == japanFace )
                     {
                        Console::WriteLine("WARNING: Font does not contain character for unicode {0:x}, using fallback japanese font.", unicode);
                        currentFont = japanFallbackFace;
                        continue;
                     }

                     Console::WriteLine("ERROR: Font does not contain character for unicode {0:x}", unicode);
                     //return -1;
                  }

                  if( FT_Load_Glyph(currentFont, glyphIndex, FT_LOAD_DEFAULT) )
                  {
                     Console::WriteLine("ERROR: FT_Load_Glyph for unicode {0:x}", unicode);
                     //return -1;
                  }

                  FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);

#if 0
                  char tgaPath[256];
                  sprintf(tgaPath, "Char.%d.%4.4x.tga", unicode, unicode);

                  WriteTGA(tgaPath, currentFont->glyph->bitmap.buffer, currentFont->glyph->bitmap.width, currentFont->glyph->bitmap.rows, 8, currentFont->glyph->bitmap.pitch);
#endif
                  break;
               }

               BPE_VERIFY(glyph->bitmap.width == glyph->bitmap.pitch, false, "Bad code assumption");

               int const baseLine = currentFont->size->metrics.ascender / 64;

               int const bitmapWidth = glyph->bitmap.width;
               int const bitmapHeight = glyph->bitmap.rows;
               int const offsetX = glyph->bitmap_left;
               int const offsetY = baseLine - glyph->bitmap_top;
               int const advance = glyph->metrics.horiAdvance / 64;

               // if offsetY is odd, add a line of padding above the character in the source
               // to get the baseline consistent
               // if bitmapWidth is odd, round up
               // if bitmapHeight is odd, round up

               unsigned int targetWidth = (bitmapWidth + 1) / 2;
               unsigned int targetHeight = (bitmapHeight + (offsetY & 1) + 1) / 2;

               // downsample the image.
               pAllocatedBitmap = (unsigned char *) malloc(targetHeight*targetWidth);
               Downsample( pAllocatedBitmap, currentFont->glyph->bitmap.buffer, bitmapWidth, bitmapHeight, offsetY, targetWidth, targetHeight );

               pBitmap = pAllocatedBitmap;

               outputWidth = targetWidth;
               outputHeight =  targetHeight;
               // round up the offsets - tends to make the character positions better (judgment call)
               outputOffsetX = offsetX / 2;
               outputOffsetY = offsetY / 2;
               outputAdvance = (advance + 1) / 2;

               if( !pIcon && !useJapanFont )
               {
                  // Adjust european characters up by a few pixels to align them better with the japanese font.
                  outputOffsetY -= 3;
               }
            }

            if( outputAdvance > originalFontSize )
            {
               // Only complain about non icon characters being too big!
               if( pIcon == NULL )
                  printf("Character too wide (%d vs %d): %d\n", outputAdvance, originalFontSize, unicode);
            }

            // Update max character descent
            {
               // Distance in pixels that character extends below the pixel bounds as seen from the top left corner.
               int const currentCharacterDescent = (outputOffsetY + outputHeight) - originalFontSize;
               maxCharacterDescent = bpe::max_val(maxCharacterDescent, currentCharacterDescent);
            }

            // Write out data for character
            {
               output.AddLabel(CStringExtras::Stringize("Char_%d", unicode));

               output.WriteUint8(outputWidth);
               output.WriteUint8(outputHeight);
               output.WriteInt8(outputOffsetX);
               output.WriteInt8(outputOffsetY);
               output.WriteUint8(outputAdvance);

               int const bitmapSize = outputWidth * outputHeight;

               if( bitmapSize > 0 )
               {
                  output.Put(pBitmap, bitmapSize);
               }
            }
            if (pAllocatedBitmap != NULL)
            {
               free(pAllocatedBitmap);
               pAllocatedBitmap = NULL;
            }
         }
      }

      // Store binary data
      {
         output.PatchOffsets();

         String^ outputPath = Environment::ExpandEnvironmentVariables("%BPE_REPOSITORY%\\EngineSupport\\Fonts\\MGS_Font.raw");
         std::string outputPathStr = Helper::StringHelper::ConvertString(outputPath);
         CDiskOutputStream diskOut(outputPathStr);
         
         SBPFont* pFont = (SBPFont*)output.GetData();
         pFont->maxCharacterDescent = maxCharacterDescent;

         diskOut.Put(pFont, output.GetDataSize());
      }
   }

   FT_Done_Face(europeFace);
   FT_Done_Face(japanFace);

   FT_Done_FreeType(library);

    return 0;
}
