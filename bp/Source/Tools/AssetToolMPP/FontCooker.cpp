//----------------------------------------------------------------------------
// FontCooker.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AssetToolMPP.h"

//----------------------------------------------------------------------------

// Only included here to avoid: 
// error C2872: 'IServiceProvider' : ambiguous symbol
#include "servprov.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CDiskOutputStream.h"
#include "Tools/Helper/Helper.h"

#include "IL/il.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

//----------------------------------------------------------------------------

using namespace System::Collections::Generic;
using namespace System::Xml::XPath;
using namespace System::Xml;
using namespace System;
using namespace Tools::Property::Build;
using namespace Tools::Property::Cook;
using namespace Tools::Property::Misc;
using namespace Tools::Property::Source;
using namespace Tools;

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

public ref struct SCookFontInfo
{
   SCookFontInfo(bool const bFreeType, String ^fontName, int const height, bool const bBold, bool const bItalic, int const borderSize, String ^symbols)
      : mFreeType(bFreeType)
      , mFontName(fontName)
      , mHeight(height)
      , mbBold(bBold)
      , mbItalic(bItalic)
      , mBorderSize(borderSize)
      , mSymbols(symbols)
   {
      if (mSymbols->Length == 0)
      {
         // Fill in with 32->256 characters
         for (int loop = 32; loop < 256; loop++)
         {
            mSymbols += Convert::ToChar( loop ).ToString();
         }
         // Manually add the (tm) symbol as it's not part of the 0-FF unicode set
         mSymbols += "™";
      }

      // Convert to UTF32
      Text::UTF32Encoding^ u32noED = gcnew Text::UTF32Encoding( false, false, false );
      array<Byte>^myBytes = gcnew array<Byte>(u32noED->GetByteCount( mSymbols ));
      u32noED->GetBytes( mSymbols, 0, mSymbols->Length, myBytes, 0 );

      mSymbolsUTF32 = gcnew List<int>();
      IO::BinaryReader ^ stream = gcnew IO::BinaryReader(gcnew IO::MemoryStream(myBytes));
      while (stream->BaseStream->Position < stream->BaseStream->Length)
      {
         mSymbolsUTF32->Add(stream->ReadInt32());
      }
   }

   bool                    mFreeType;
   String ^                mFontName;
   int                     mHeight;
   bool                    mbBold;
   bool                    mbItalic;
   int                     mBorderSize;
   String ^                mSymbols;     
   List<int> ^             mSymbolsUTF32;    // UTF-32, to make it easy to check which characters have already been embedded in bitmap 
private:
   SCookFontInfo() {};
};

HRESULT make_font(std::string const &destFONTFilename, 
                  std::string const &destBMPFilename, 
                  std::string const &destBMPFilenameAsset, 
                  std::string const &imagesFilename,
                  List<SCookFontInfo^> ^fonts,
                  XmlNodeList^ images,
                  AssetSystem::Manager^ manager);

bool AssetToolMPP::Cookers::CookFONT( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager )
{
   // Load up font
   XmlDocument^ doc = gcnew XmlDocument();
   doc->Load(inputPath);

   // Get font element
   XmlElement^ fontNav = (XmlElement ^) doc->SelectSingleNode("/Font");
   if (fontNav == nullptr)
   {
      printf("Invalid xml. Try:\n<Font name=\"OCR A Extended\" size=\"12\" bold=\"false\" italic=\"false\"/>\n");
      return false;
   }

   // Build dependencies
   List<AssetSystem::BuildAsset^> ^buildAssets = gcnew List<AssetSystem::BuildAsset^>();

   // Get the font name
   String ^gcTrueTypeName = fontNav->GetAttribute("name", String::Empty);

   // Get the font asset path
   String ^gcFontAssetPath = fontNav->GetAttribute("asset", String::Empty);
   if (gcFontAssetPath->Length > 0)
   {
      // Add as a dependency
      buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("FreeType", assetManager->GetRepositoryRelativePath(gcFontAssetPath)));

      // Convert to a system path
      gcFontAssetPath = assetManager->GetSystemPath(gcFontAssetPath);
   }
   else if (gcTrueTypeName->Length == 0)
   {
      printf("No font name or asset path specified.\n");
      return false;
   }

   // use FreeType if the description includes an asset path
   bool bUseFreeType = gcFontAssetPath->Length != 0;

   // Get images path
   String ^imagesPath = fontNav->GetAttribute("images", String::Empty);
   XmlNodeList^ images = doc->SelectNodes("//Image");

   // Get bold and italic flags
   bool bold = Tools::Common::Misc::ParseBool(fontNav->GetAttribute("bold", String::Empty));
   bool italic = Tools::Common::Misc::ParseBool(fontNav->GetAttribute("italic", String::Empty));

   // Get font size in points
   int size = 12;
   String ^gcSize = fontNav->GetAttribute("size", String::Empty);
   if (gcSize != String::Empty)
   {
      size = System::Int32::Parse(gcSize);
   }

   int border = 0;

   // Get font border
   String ^gcBorder = fontNav->GetAttribute("border", String::Empty);
   if (gcBorder != String::Empty)
   {
      border = System::Int32::Parse(gcBorder);
   }

   // Cook info
   List<SCookFontInfo^> ^fonts = gcnew List<SCookFontInfo^>();

   // Get symbols asset
   String^ symbols = "";
   String ^gcSubFontSymbolsAssetName = fontNav->GetAttribute("useSymbols", String::Empty);
   if (gcSubFontSymbolsAssetName->Length != 0)
   {
      String ^symbolsFilename(assetManager->GetSystemPath(gcSubFontSymbolsAssetName));
      if (System::IO::File::Exists(symbolsFilename))
      {
         // Add build dependency on symbols
         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("symbols", assetManager->GetRepositoryRelativePath(gcSubFontSymbolsAssetName)));

         // Read symbols data
         symbols = System::IO::File::OpenText( symbolsFilename )->ReadToEnd();
      }
   }

   if (gcFontAssetPath->Length > 0)
   {
      // Add font using FreeType rendering
      fonts->Add(gcnew SCookFontInfo(true, gcFontAssetPath, size, bold, italic, border, symbols));
   }
   else
   {
      // Add font using system rendering
      fonts->Add(gcnew SCookFontInfo(false, gcTrueTypeName, size, bold, italic, border, symbols));
   }

   // Generate output names
   std::string const destBMPFilename(Helper::StringHelper::ConvertString(System::IO::Path::ChangeExtension(outputPath, ".tga")));
   String ^gcDestBMPFilenameAsset = assetManager->GetRepositoryRelativePath(System::IO::Path::ChangeExtension(outputPath, ".tga"));
   std::string const destBMPFilenameAsset(Helper::StringHelper::ConvertString(assetManager->GetCookedAssetPath(gcDestBMPFilenameAsset)));
   std::string const destFONTFilename(Helper::StringHelper::ConvertString(outputPath));
   std::string const imagesFilename(Helper::StringHelper::ConvertString(assetManager->GetSystemPath(imagesPath)));

   // Add sub fonts
   XmlNodeList^ subFonts = doc->SelectNodes("//Subfont");

   for( int i = 0; i < subFonts->Count; ++i )
   {
      XmlElement^ subFont = (XmlElement^)subFonts[i];

      // Get subfont name
      String ^gcSubFontTrueTypeName = subFont->GetAttribute("name", String::Empty);

      // Get subfont asset path
      String ^gcSubFontAssetPath = subFont->GetAttribute("asset", String::Empty);
      if (gcSubFontAssetPath->Length > 0)
      {
         // Add as a dependency
         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("FreeType", assetManager->GetRepositoryRelativePath(gcSubFontAssetPath)));

         // Convert to a system path
         gcSubFontAssetPath = assetManager->GetSystemPath(gcSubFontAssetPath);
      }
      else if (gcSubFontTrueTypeName->Length == 0)
      {
         printf("No font name or asset path specified for subfont.\n");
         continue;
      }

      // Get subfont size
      int size = 12;
      String ^gcSize = subFont->GetAttribute("size", String::Empty);
      if (gcSize != String::Empty)
      {
         size = System::Int32::Parse(gcSize);
      }

      // Set subfont bold and italic flags
      bool bold = Tools::Common::Misc::ParseBool(subFont->GetAttribute("bold", String::Empty));
      bool italic = Tools::Common::Misc::ParseBool(subFont->GetAttribute("italic", String::Empty));

      // Get symbols asset
      String ^gcSubFontSymbolsAssetName = subFont->GetAttribute("useSymbols", String::Empty);
      if (gcSubFontSymbolsAssetName->Length == 0)
      {
         printf("No symbols specified for sub font.\n");
         continue;
      }
      String ^symbolsFilename(assetManager->GetSystemPath(gcSubFontSymbolsAssetName));
      if (!System::IO::File::Exists(symbolsFilename))
      {
         printf("Subfont: Missing symbols file %s\n", Helper::StringHelper::ConvertString(symbolsFilename).c_str());
         continue;
      }

      // Add build dependency on symbols
      buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("symbols", assetManager->GetRepositoryRelativePath(gcSubFontSymbolsAssetName)));

      // Read symbols data
      symbols = System::IO::File::OpenText( symbolsFilename )->ReadToEnd();

      if (gcSubFontAssetPath->Length > 0)
      {
         // Add font using FreeType rendering
         fonts->Add(gcnew SCookFontInfo(true, gcSubFontAssetPath, size, bold, italic, 0, symbols ));
      }
      else
      {
         // Add font using system rendering
         fonts->Add(gcnew SCookFontInfo(false, gcSubFontTrueTypeName, size, bold, italic, 0, symbols ));
      }
   }

   // Render the font
   make_font(destFONTFilename, 
             destBMPFilename, 
             destBMPFilenameAsset, 
             imagesFilename,
             fonts,
             images, 
             assetManager);

   // Add dependency on output bitmap
   {
      List<AssetSystem::PackageAsset^> ^packageAssets = gcnew List<AssetSystem::PackageAsset^>();
      AssetSystem::Helper::MarkSourceAssetAsGenerated( gcDestBMPFilenameAsset, assetManager );
      packageAssets->Add(gcnew AssetSystem::PackageAsset(gcDestBMPFilenameAsset, AssetSystem::PackageAsset::EPackageType::kPackageAndLoad, AssetSystem::PackageAsset::EAssetSource::kGenerated ));   
      AssetSystem::PackageAssets::WriteAssets(inputPath, packageAssets, assetManager);
   }

   // Add build dependency on source image   
   if( imagesPath != "" )
   {
      buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("images", assetManager->GetRepositoryRelativePath(imagesPath)));
   }
   
   // Write the assets
   AssetSystem::BuildAssets::WriteAssets(inputPath, buildAssets, assetManager, System::Nullable<Tools::AssetSystem::PlatformType::EPlatform>());

   return true;
}

//----------------------------------------------------------------------------

#include <tchar.h>
#include <WinGDI.h>

HRESULT make_font(std::string const &destFONTFilename, 
                  std::string const &destBMPFilename, 
                  std::string const &destBMPFilenameAsset, 
                  std::string const &imagesFilename,
                  List<SCookFontInfo^> ^fonts,
                  XmlNodeList^ images,
                  AssetSystem::Manager^ manager)
{
   if (fonts->Count == 0)
   {  
      // No fonts specified
      return E_FAIL;
   }

   FT_Error error;
   FT_Library library;

   error = FT_Init_FreeType(&library);
   if (error)
   {
      // an error occurred during library initialization
      return E_FAIL;
   }

   FileBasedPropertyObjectEnumerator ^ dataSource = gcnew FileBasedPropertyObjectEnumerator();
   dataSource->AddFiles(FileBasedPropertyObjectEnumerator::FileSearchParams::DefaultFileSearchParams(manager));

   bpe::vector_s<real32>   fTexCoords;
   bpe::vector_s<int16>       characterYAdjust;
   bpe::vector_s<uint16>      utf8OneByteIndices;
   utf8OneByteIndices.resize(128, 1); // Use direct lookup for speed for 1 byte characters, use index 1 to prevent string termination issues
   std::map<uint16, uint16>   utf8TwoByteMap;   // use character code for key, allows sorting
   std::map<uint32, uint16>   utf8ThreeByteMap;
   std::map<uint32, uint16>   utf8FourByteMap;

   bpe::vector_s<uint16>     imageIndices;
   imageIndices.resize(128, 1); // Use direct lookup for speed for images, use index 1 to prevent string termination issues

   // UTF-8 encoding
   Text::Encoding^ utf8Encoder = Text::Encoding::UTF8;

   int const kMaxCharacterCount = 256;
   int const kMaxImageCount = 128;
   int const kCharacterArraySize = kMaxCharacterCount + kMaxImageCount;

   fTexCoords.reserve(1024);
   characterYAdjust.reserve(256);
   
   DWORD dwTexWidth = 256;                 // Texture dimensions
   DWORD dwTexHeight =  256;

   int const borderSize = fonts[0]->mBorderSize;   // Currently use same border size for all fonts

   int maxRowHeight = 0;

   // convert true type font to font bitmap
   // Large fonts need larger textures
   int const mainFontHeight = fonts[0]->mHeight;   // Use height from first font
   if ( mainFontHeight >= 60 )
      dwTexWidth = dwTexHeight = 2048;
   else if ( mainFontHeight >= 30)
      dwTexWidth = dwTexHeight = 1024;
   else if ( mainFontHeight >= 15 )
      dwTexWidth = dwTexHeight = 512;
   else
      dwTexWidth  = dwTexHeight = 256;

   // TODO[GAR]: This needs to check to make sure that it can fit all of the symbols into this one file. There is no error checking for this!!!
   dwTexHeight *= 2;

   // Prepare to create a bitmap
   BITMAPINFO bmi;
   ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
   bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth       =  (int)dwTexWidth;
   bmi.bmiHeader.biHeight      =  (int)dwTexHeight;
   bmi.bmiHeader.biPlanes      = 1;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biBitCount    = 32;

   // Create a DC and a bitmap for the font
   HDC     hDC       = CreateCompatibleDC( NULL );
   DWORD*  pBitmapBits = NULL;
   HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0 );

   SetMapMode( hDC, MM_TEXT );
   DWORD x = 0;
   DWORD y = 0;

   DWORD dwPadding = 0;
   DWORD const dwOverlap = 1 + borderSize;                      // Character pixel spacing per side
   int masterFontRowHeight = 0;   // Used to determine the spacing between rows, specified by the size of ' ' in the first font

   // Use std::set to check for UTF32 characters that have already been added.
   std::set<int>  addedCharacters;

   uint16 texCoordIndex = 0;
   // Add dummy entries for first 32 ASCII chars so character indices match ASCII
   for (int loop = 0; loop < 32; loop++)
   {
      characterYAdjust.push_back(0);
      fTexCoords.push_back(0.0f);
      fTexCoords.push_back(0.0f);
      fTexCoords.push_back(0.0f);
      fTexCoords.push_back(0.0f);
      utf8OneByteIndices[loop] = texCoordIndex;
      texCoordIndex++;
   }

   for (int fontLoop = 0; fontLoop < fonts->Count; fontLoop++)
   {
      SCookFontInfo ^font = fonts[fontLoop];

      DWORD dwFontHeight = font->mHeight;

      SIZE size;

      FT_Face face;

      if (font->mFreeType)
      {
         INT myDPI = (INT)GetDeviceCaps(hDC, LOGPIXELSY);
         myDPI = 96;
         INT nHeight = MulDiv( dwFontHeight, myDPI, 72 );
//         INT nHeight = dwFontHeight;

         std::string const fontName(Helper::StringHelper::ConvertString(font->mFontName));
         error = FT_New_Face(library, fontName.c_str(), 0, &face);
         if (error == FT_Err_Unknown_File_Format)
         {
            printf("Error! FT_New_Face reports unknown format for %s.\n", fontName.c_str());
            return E_FAIL;
         }
         else if (error)
         {
            printf("Error! FT_New_Face function could not create font %s.\n", fontName.c_str());
            return E_FAIL;
         }

         error = FT_Set_Pixel_Sizes(face, 0, nHeight);

         // Calculate the spacing between characters based on line height
         error = FT_Load_Char(face, ' ', FT_LOAD_DEFAULT | FT_LOAD_VERTICAL_LAYOUT);

         size.cx = 0;
         size.cy = face->glyph->advance.y / 64;
      }

      if (fontLoop == 0)
      {
         dwPadding = ((DWORD) ceil(size.cy * 0.05f));
         x = dwOverlap + dwPadding;
         y = dwOverlap + dwPadding;
         masterFontRowHeight = size.cy;
      }

      // Loop through all printable character and output them to the bitmap..
      // Meanwhile, keep track of the corresponding tex coords for each character.
      for (int charLoop = 0; charLoop < font->mSymbolsUTF32->Count; charLoop++ )
      {
         int const utf32Char = font->mSymbolsUTF32[charLoop];
         if (utf32Char < 32) continue; // Skip characters < 32, already been added
         if (std::find(addedCharacters.begin(), addedCharacters.end(), utf32Char) != addedCharacters.end())
         {
            // Already added this character
            continue;
         }

         String ^unicodeString = Char::ConvertFromUtf32(utf32Char);
         std::wstring const charStringUnicode(Helper::StringHelper::ConvertStringUnicode(unicodeString));
         std::string const charStringAnsi(Helper::StringHelper::ConvertString(unicodeString));

         if (font->mFreeType)
         {
            // Get glyph index from character
            FT_UInt glyphIndex = FT_Get_Char_Index(face, utf32Char);
            if ((glyphIndex == 0) && (charStringAnsi != " "))
            {
               // Character doesn't have a glyph
               continue;
            }

            // Load glyph data
            error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
            if (error)
            {
               // Something went wrong...
               continue;
            }
            if (font->mbBold)
            {
               FT_Outline_Embolden(&face->glyph->outline, 64);
            }

            // Render to a bitmap
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

            // Get character extents
            // HACK: use advance values
            size.cx = face->glyph->linearHoriAdvance / 65536;  //face->glyph->bitmap.width;
            size.cy = face->size->metrics.height / 64;  //face->glyph->bitmap.rows;

         }

         // Add character so it doesn't get added to bitmap again
         addedCharacters.insert(utf32Char);

         maxRowHeight = bpe::max_val(int(size.cy + dwOverlap + dwPadding), maxRowHeight);

         if( (DWORD)(x + size.cx + dwOverlap + dwPadding) > dwTexWidth )
         {
            x  = dwOverlap + dwPadding;
            y += maxRowHeight;
            
            maxRowHeight = 0;
         }

         if (font->mFreeType)
         {
            // Get base image position
            int by = dwTexHeight - 1 - y - face->size->metrics.ascender / 64 + face->glyph->bitmap_top;
            int bx = x + face->glyph->bitmap_left;

            // Draw glyph bitmap into the font image
            for (int gy = 0; gy < face->glyph->bitmap.rows; ++gy)
            {
               for (int gx = 0; gx < face->glyph->bitmap.width; ++gx)
               {
                  int value = face->glyph->bitmap.buffer[gy*face->glyph->bitmap.pitch+gx];
                  pBitmapBits[(by - gy) * dwTexWidth + (bx + gx)] = value;
               }
            }
         }

         characterYAdjust.push_back(0);   // No offset

         fTexCoords.push_back(real32(x + 0       - dwOverlap  ) / dwTexWidth);
         fTexCoords.push_back(real32(y + 0       - borderSize ) / dwTexHeight);
         fTexCoords.push_back(real32(x + size.cx + dwOverlap  ) / dwTexWidth);
         fTexCoords.push_back(real32(y + size.cy + borderSize ) / dwTexHeight);

         x += size.cx + (2 * (dwOverlap + dwPadding));

         // Add to array indices
         int const utf8ByteCount = utf8Encoder->GetByteCount(unicodeString);
         array<Byte>^ utf8Bytes = utf8Encoder->GetBytes( unicodeString );
         switch (utf8ByteCount)
         {
            case 1:
               utf8OneByteIndices[(uint8) utf8Bytes[0] ] = texCoordIndex;
               break;
            case 2:
               utf8TwoByteMap.insert(std::pair<uint16, uint16>((uint16) utf8Bytes[1] + (utf8Bytes[0] << 8), texCoordIndex));
               break;
            case 3:
               utf8ThreeByteMap.insert(std::pair<uint32, uint16>((uint32) utf8Bytes[2] + (utf8Bytes[1] << 8) + (utf8Bytes[0] << 16), texCoordIndex ));
               break;
            case 4:
               utf8FourByteMap.insert(std::pair<uint32, uint16>((uint32) utf8Bytes[3] + (utf8Bytes[2] << 8) + (utf8Bytes[1] << 16) + (utf8Bytes[0] << 24), texCoordIndex ));
               break;
         }

         texCoordIndex++;
      }
      
      /*
      SelectObject( hDC, hbmOld );
      SelectObject( hDC, hFontOld );
      DeleteObject( hbmBitmap );
      DeleteObject( hFont );
      DeleteDC( hDC );
      */

      if (font->mFreeType)
      {
         FT_Done_Face(face);
      }
   }

   // Fix the alpha for font
   for( int ty=0; ty < dwTexHeight; ty++ )
   {
      for( int tx=0; tx < dwTexWidth; tx++ )
      {
         uint32 & color = pBitmapBits[dwTexWidth*ty + tx];

         uint32 intensity = color & 0xff;
         // rrrr -> rgba
         color = intensity | (intensity << 8) | (intensity << 16) | (intensity << 24);
      }
   }

   if( borderSize > 0 )
   {
      uint32* pCopy = new uint32[dwTexHeight * dwTexWidth];
      memcpy( pCopy, pBitmapBits, sizeof(uint32) * dwTexHeight * dwTexWidth);

      for( int ty = 0; ty < dwTexHeight; ty++ )
      {
         for( int tx = 0; tx < dwTexWidth; tx++ )
         {
            int const offset = dwTexWidth*ty + tx;

            uint32 color = pCopy[offset];

            uint32 accumulatedAlpha = 0;
            uint32 validPixels = 0;

            for( int x = -borderSize; x <= borderSize; ++x )
            {
               for( int y = -borderSize; y <= borderSize; ++y )
               {
                  int const actX = tx + x;
                  int const actY = ty + y;

                  if( (actX < 0) || (actX >= dwTexWidth) ||
                      (actY < 0) || (actY >= dwTexHeight) )
                      continue;

                  uint32 const sample = pCopy[dwTexWidth*actY + actX] >> 24;

                  if( sample > 0 )
                  {
                     accumulatedAlpha = bpe::max_val(accumulatedAlpha, sample);
                     ++validPixels;
                  }
               }
            }

            if( validPixels > 0 )
            {
               color &= 0x00ffffff;

               uint32 const newAlpha = accumulatedAlpha;
               color |= (newAlpha << 24);
            }

            pBitmapBits[offset] = color;
         }
      }

      delete [] pCopy;
   }

   FT_Done_FreeType(library);

   ilInit();

   // insert images into font
   if( !imagesFilename.empty() )
   {
      ILuint imagesHandle;
      ilGenImages(1, &imagesHandle);
      ilBindImage(imagesHandle);

      if( ilLoadImage(const_cast<char*>( imagesFilename.c_str())) == IL_FALSE )
      {
         printf( "ERROR: Font cooker couldn't load image %s (ErrorCode: 0x%x).\n", imagesFilename.c_str(), ilGetError() );
         return false;
      }

      int const imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
      int const imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);

      uint32* pSourceImageRef = (uint32*)ilGetData();

      // Copy image data so we can guarantee RGBA format
      uint32* pSourceImage = new uint32[imageWidth * imageHeight];
      ilCopyPixels(0, 0, 0, imageWidth, imageHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, pSourceImage);

      for( int i = 0; i < images->Count; ++i )
      {
         XmlElement^ image = (XmlElement^)images[i];

         uint16 const id = System::UInt16::Parse(image->GetAttribute("id"));
         if (id > 127)
         {
            printf("Font Cooker: Invalid image id: %d. Max 127.\n", id);
            continue;
         }

         int const charX = System::Int32::Parse(image->GetAttribute("x"));
         int const charY = System::Int32::Parse(image->GetAttribute("y"));

         int yAdjust = 0;

         String^ yAdjustStr = image->GetAttribute("yadjust");
         if( yAdjustStr != String::Empty ) 
         {
            yAdjust = System::Int32::Parse(yAdjustStr);
         }

         int const charWidth = System::Int32::Parse(image->GetAttribute("width"));
         int const charHeight = System::Int32::Parse(image->GetAttribute("height"));

         // fill fit character on this "line" of the texture page?
         int const xAfterCharacter = x + charWidth + dwOverlap + dwPadding;
         if( xAfterCharacter > dwTexWidth )
         {
            // nope, go to next line
            x  = dwOverlap + dwPadding;
            y += maxRowHeight;
            
            maxRowHeight = 0;
         }
         else
         {
            maxRowHeight = bpe::max_val(charHeight + 1, maxRowHeight);
         }

         // copy from "images" charX/charY -> charWidth/charHeight to pDibBlahblah with x/y -> charWidth/charHeight
         int const yOffset = bpe::max_val( 0, (mainFontHeight - charHeight) / 2 );
         
         characterYAdjust.push_back(yAdjust);

         for( int tx = 0; tx < charWidth; ++tx )
         {
            for( int ty = 0; ty < charHeight; ++ty )
            {
               int const sourceX = tx + charX;
               int const sourceY = ty + charY;

               int const destX = x + tx;
               int const destY = y + ty + yOffset;

               uint32 sourcePixel = pSourceImage[(imageHeight - sourceY - 1) * imageWidth + sourceX];
               uint32 *pDestPixel = &pBitmapBits[(dwTexHeight - destY - 1) * dwTexWidth + destX];
               *pDestPixel = sourcePixel;
            }
         }

         fTexCoords.push_back(real32(x + 0         ) / dwTexWidth);
         fTexCoords.push_back(real32(y + 0         ) / dwTexHeight);
         fTexCoords.push_back(real32(x + charWidth ) / dwTexWidth);
         fTexCoords.push_back(real32(y + bpe::max_val(charHeight, mainFontHeight)) / dwTexHeight);

         // Use special array for images
         imageIndices[id] = texCoordIndex;
         texCoordIndex++;

         x += charWidth + (2 * (dwOverlap + dwPadding));
      }

      ilDeleteImages(1, &imagesHandle);
      delete [] pSourceImage;
   }

   // Fix the alpha for images
   for( int ty=0; ty < dwTexHeight; ty++ )
   {
      for( int tx=0; tx < dwTexWidth; tx++ )
      {
         uint32 const offset = dwTexWidth*ty + tx;

         uint32 const alpha = pBitmapBits[offset] >> 24;
         if( alpha == 0 )
         {
            uint32 const newAlpha = pBitmapBits[offset] & 0xff;
            pBitmapBits[offset] |= (newAlpha << 24);
         }
      }
   }

   // resize image to only contain what was ended up being used
   {
      int actualImageHeight = dwTexHeight;

      for( bool lineEmpty = true; (actualImageHeight > 0) && lineEmpty; )
      {
         for( int tx = 0; tx < dwTexWidth; ++tx )
         {
            if( pBitmapBits[(dwTexHeight - actualImageHeight) * dwTexWidth + tx] != 0 )
            {
               lineEmpty = false;
               break;
            }
         }

         if( lineEmpty )
         {
            actualImageHeight--;
         }
      }
         
      actualImageHeight = (actualImageHeight + 3) & ~3;

      int const lineDifference = dwTexHeight - actualImageHeight;

      real32 const scaleFactor = static_cast<real32>(dwTexHeight) / actualImageHeight;
      dwTexHeight = actualImageHeight;
      pBitmapBits += lineDifference * dwTexWidth;

      for( int i= 0; i < (fTexCoords.size() / 4); ++i )
      {
         fTexCoords[i*4 + 1] *= scaleFactor;
         fTexCoords[i*4 + 3] *= scaleFactor;
      }
   }

   // save font image
   {
      ILuint fontImageHandle;
      ilGenImages(1, &fontImageHandle);

      ilBindImage(fontImageHandle);

      ilTexImage(dwTexWidth,
                 dwTexHeight,
                 1,
                 4,
                 IL_RGBA,
                 IL_UNSIGNED_BYTE,
                 pBitmapBits);

      ilEnable(IL_FILE_OVERWRITE);
      ilSaveImage((char *) destBMPFilename.c_str());

      ilDeleteImages(1, &fontImageHandle);
   }

   ilShutDown();

   // save meta data for font image
   {
      System::String^ texturePath = gcnew System::String(destBMPFilename.c_str());
      System::String^ metaDataPath = manager->GetMetaDataSystemPath(texturePath);
      
      if( !System::IO::File::Exists(metaDataPath) )
      {
         XmlDocument^ metaData = AssetSystem::MetaData::CreateMetaData(texturePath, manager, dataSource);

         // Switch texture type to s3tc
         Helpers::SetValueAtLabel(metaData->CreateNavigator(), "texture_format", "kDXT5");

         // Disable mipmaps
         Helpers::SetValueAtLabel(metaData->CreateNavigator(), "gen_mips", "kForceOff");

         AssetSystem::MetaData::SaveMetaData(texturePath, manager, metaData);
      }
   }

   // Write data
   CDiskOutputStream outStream(destFONTFilename.c_str());
   
   // header
   outStream.WriteUint32('FONT');
   // version 4
   outStream.WriteUint32(5);
   
   outStream.WriteInt32(mainFontHeight);

   outStream.WriteInt32(dwOverlap);

   outStream.WriteReal32((real32) masterFontRowHeight);
   
   // Write UVS
   outStream.Put(fTexCoords);

   // Write character y adjust values
   outStream.Put(characterYAdjust);

   // UTF-8 Lookup tables
   outStream.Put(utf8OneByteIndices);
   {
      // Make sure character values are sorted for fast runtime lookups
      bpe::vector_s<uint16>     utf8TwoByteIndices;
      bpe::vector_s<uint16>     utf8TwoByteValue;
      utf8TwoByteIndices.reserve(utf8TwoByteMap.size());
      utf8TwoByteValue.reserve(utf8TwoByteMap.size());
      for (std::map<uint16, uint16>::const_iterator it = utf8TwoByteMap.begin(); it != utf8TwoByteMap.end(); ++it)
      {
         utf8TwoByteValue.push_back(it->first);
         utf8TwoByteIndices.push_back(it->second);
      }

      bpe::vector_s<uint16>     utf8ThreeByteIndices;
      bpe::vector_s<uint32>     utf8ThreeByteValue;
      utf8ThreeByteIndices.reserve(utf8ThreeByteMap.size());
      utf8ThreeByteValue.reserve(utf8ThreeByteMap.size());
      for (std::map<uint32, uint16>::const_iterator it = utf8ThreeByteMap.begin(); it != utf8ThreeByteMap.end(); ++it)
      {
         utf8ThreeByteValue.push_back(it->first);
         utf8ThreeByteIndices.push_back(it->second);
      }

      bpe::vector_s<uint16>     utf8FourByteIndices;
      bpe::vector_s<uint32>     utf8FourByteValue;
      utf8FourByteIndices.reserve(utf8FourByteMap  .size());
      utf8FourByteValue.reserve(utf8FourByteMap.size());
      for (std::map<uint32, uint16>::const_iterator it = utf8FourByteMap.begin(); it != utf8FourByteMap.end(); ++it)
      {
         utf8FourByteValue.push_back(it->first);
         utf8FourByteIndices.push_back(it->second);
      }

      // Write sorted vectors
      outStream.Put(utf8TwoByteIndices);
      outStream.Put(utf8TwoByteValue);
      outStream.Put(utf8ThreeByteIndices);
      outStream.Put(utf8ThreeByteValue);
      outStream.Put(utf8FourByteIndices);
      outStream.Put(utf8FourByteValue);

   }
   // Image lookup table
   outStream.Put(imageIndices);

   // Write texture asset filename
   outStream.Put(destBMPFilenameAsset);

   return S_OK;
}

//----------------------------------------------------------------------------
