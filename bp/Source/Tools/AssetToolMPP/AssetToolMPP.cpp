//----------------------------------------------------------------------------
// AssetToolMPP.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AssetToolMPP.h"

//----------------------------------------------------------------------------

// Only included here to avoid: 
// error C2872: 'IServiceProvider' : ambiguous symbol
#include "servprov.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CDiskOutputStream.h"
#include "Tools/CookerHelper/CEditorMesh.h"
#include "Tools/Helper/Helper.h"
#include "Tools/Open3d/STLSupport.h"

#include "Boost/shared_ptr.hpp"
#include "IL/il.h"

//----------------------------------------------------------------------------

using namespace Open3d;
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

#pragma region CollisionCooker

struct SCollisionShader
{
   SCollisionShader(std::string const & name, uint32 const dataSize, boost::shared_ptr<char> const & data)
      : mName(name)
      , mDataSize(dataSize)
      , mData(data)
   {
   }

   std::string             mName;
   uint32                  mDataSize;
   boost::shared_ptr<char> mData;
};

bool AssetToolMPP::Cookers::CookCOLL( String^ inputPath, String^ outputPath, AssetSystem::Manager^ assetManager )
{
   // get property source
   FileBasedPropertyObjectEnumerator ^ propertySource = gcnew FileBasedPropertyObjectEnumerator();
   propertySource->AddFiles(FileBasedPropertyObjectEnumerator::FileSearchParams::DefaultFileSearchParams(assetManager));

   XmlDocument^ doc = gcnew XmlDocument();
   doc->Load( inputPath );

   Scene^ scene = Scene::Attach( doc );

   CookProperties::CookInfo^ cookInfo = gcnew CookProperties::CookInfo();
   std::vector<SCollisionShader> collisionMaterials;

   std::vector<CEditorMesh::Triangle> triangles;
   bpe::vector_s<uint32> triangleMaterialIndices;
   CMergeContainer<CVector3> vertices;

   List<DagNode^>^ meshNodes = scene->GetMeshes();
   for( int meshIdx = 0; meshIdx < meshNodes->Count; ++meshIdx )
   {
      DagNode^ node = meshNodes[meshIdx];
      Transform^ transformNode = Transform::Attach( node->Node["Transform"] );

      TVector3Vector points;
      DataArray^ pointsData = node->GetDataArray( "Points" );
      pointsData->GetData( points );

      // transform points into world space
      CMatrix34 const transform = transformNode->GetTransform();
      for( int i = 0; i < points.size(); ++i )
      {
         points[i] = transform * points[i];
      }

      List<Submesh^>^ submeshes = node->GetSubmeshes();
      for( int submeshIdx = 0; submeshIdx < submeshes->Count; ++submeshIdx )
      {
         Submesh^ mesh = submeshes[submeshIdx];

         // find material index for current material
         int materialIndex = -1;
         {
            std::string const materialName = Helper::StringHelper::ConvertString( mesh->Material );

            // see if the material we're looking for has already been exported
            for( int i = 0; i < collisionMaterials.size(); ++i )
            {
               if( collisionMaterials[i].mName == materialName )
               {
                  materialIndex = i;
                  break;
               }
            }

            // nope, so export it now
            if( materialIndex == -1 )
            {
               Material^ material = scene->GetMaterial( mesh->Material );

               // if we don't have a material, just use the default material
               if( material == nullptr )
               {
                  material = scene->CreateMaterial(mesh->Material);
                  material->PropertyData = String::Format("<UsingPropertyContainer containerVersion=\"2\" id=\"Shader\" name=\"{0}\"/>", mesh->Material);
               }

               // write property data for material
               System::IO::MemoryStream ^ output = gcnew System::IO::MemoryStream();
               {
                  XmlDocument^ materialDoc = gcnew XmlDocument();
                  materialDoc->LoadXml( material->PropertyData );
                  MergePropertiesV2::Merge_Inplace( materialDoc, propertySource );

                  // remove any components that aren't of type CollisionShader
                  XmlNodeList^ nonCollisionShaderComponentGroups = materialDoc->SelectNodes("./PropertyContainer/ComponentGroup[@id != \"CollisionShader\"]");
                  for( int i = 0; i < nonCollisionShaderComponentGroups->Count; ++i )
                  {
                     XmlNode^ nonCollisionShaderNode = nonCollisionShaderComponentGroups[i];
                     nonCollisionShaderNode->ParentNode->RemoveChild(nonCollisionShaderNode);
                  }

                  CookProperties::WriteCookedPropertiesForPropertyContainer(
                     assetManager, 
                     (XmlElement ^)materialDoc->SelectSingleNode("PropertyContainer"), 
                     cookInfo, gcnew CookProperties::CookOutput(), output);

                  // This script object has no children, write zero
                  for (int loop = 0; loop < sizeof(int); loop++) 
                     output->WriteByte(0); 
               }

               // get size of property data
               int const shaderDataSize = (int) output->Length;

               // get buffer for property data
               array<byte>^ outputBuffer = output->GetBuffer();
               output->Close();

               // copy into native array
               boost::shared_ptr<char> shaderData( new char [shaderDataSize] );
               System::Runtime::InteropServices::Marshal::Copy( outputBuffer, 0, (IntPtr)shaderData.get(), shaderDataSize );

               materialIndex = collisionMaterials.size();
               collisionMaterials.push_back( SCollisionShader(materialName, shaderDataSize, shaderData) );
            }
         }

         // get point indices
         TIntVector pointIndices;
         IndexArray^ pointIndexData = mesh->GetIndexArray("Points");
         pointIndexData->GetIndices( pointIndices );

         // create merged ueber structure for them
         int const triCount = pointIndices.size() / 3;

         for( int triIdx = 0; triIdx < triCount; ++triIdx )
         {
            int indices[3];
            for( int index = 0; index < 3; ++index )
            {
               int const offset = triIdx * 3 + index;
               indices[index] = vertices.GetIndexForElement(points[ pointIndices[ offset ] ] );
            }

            triangles.push_back( CEditorMesh::Triangle( indices[0], indices[1], indices[2] ) );
            triangleMaterialIndices.push_back(materialIndex);
         }
      }
   }

   CDiskOutputStream outStream( Helper::StringHelper::ConvertString( outputPath ) );

   // header
   outStream.WriteUint32('COLL');
   // version
   outStream.WriteUint32( 1 );

   // write materials
   outStream.WriteInt32( collisionMaterials.size() );
   for( int i = 0; i < collisionMaterials.size(); ++i )
   {
      int const dataSize = collisionMaterials[i].mDataSize;
      outStream.Put(collisionMaterials[i].mData.get(), dataSize);
   }

   // write vertices
   {
      bpe::vector_s<CVector3> points(vertices.GetArray());
      outStream.Put( points );
   }

   // write indices
   {
      bpe::vector_s<uint32> indices;
      for( int i = 0; i < triangles.size(); ++i )
      {
         indices.push_back( triangles[i].mIndex1 );
         indices.push_back( triangles[i].mIndex2 );
         indices.push_back( triangles[i].mIndex3 );
      }
      outStream.Put( indices );
   }

   // write triangle material index list
   {
      triangleMaterialIndices.PutTo(outStream);
   }

   return true;
}

#pragma endregion
#pragma region FontCooker
public ref struct SCookFontInfo
{
   SCookFontInfo(String ^fontName, int const height, bool const bBold, bool const bItalic, int const borderSize, String ^symbols)
      : mFontName(fontName)
      , mHeight(height)
      , mbBold(bBold)
      , mbItalic(bItalic)
      , mBorderSize(borderSize)
      , mSymbols(symbols)
   {
      if (mSymbols->Length == 0)
      {
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
   // load up font
   XmlDocument^ doc = gcnew XmlDocument();
   doc->Load(inputPath);

   XmlElement^ fontNav = (XmlElement ^) doc->SelectSingleNode("/Font");
   if (fontNav == nullptr)
   {
      printf("Invalid xml. Try:\n<Font name=\"OCR A Extended\" size=\"12\" bold=\"false\" italic=\"false\"/>\n");
      return false;
   }

   String ^gcTrueTypeName = fontNav->GetAttribute("name", String::Empty);
   if (gcTrueTypeName->Length == 0)
   {
      printf("No font name specified.\n");
      return false;
   }

   String ^imagesPath = fontNav->GetAttribute("images", String::Empty);
   XmlNodeList^ images = doc->SelectNodes("//Image");

   bool bold = Tools::Common::Misc::ParseBool(fontNav->GetAttribute("bold", String::Empty));
   bool italic = Tools::Common::Misc::ParseBool(fontNav->GetAttribute("italic", String::Empty));
   int size = 12;

   String ^gcSize = fontNav->GetAttribute("size", String::Empty);
   if (gcSize != String::Empty)
   {
      size = System::Int32::Parse(gcSize);
   }

   int border = 0;

   String ^gcBorder = fontNav->GetAttribute("border", String::Empty);
   if (gcBorder != String::Empty)
   {
      border = System::Int32::Parse(gcBorder);
   }
   List<SCookFontInfo^> ^fonts = gcnew List<SCookFontInfo^>();
   fonts->Add(gcnew SCookFontInfo(gcTrueTypeName, size, bold, italic, border, ""));

   std::string const trueTypeName = Helper::StringHelper::ConvertString(gcTrueTypeName);
   std::string const destBMPFilename(Helper::StringHelper::ConvertString(System::IO::Path::ChangeExtension(outputPath, ".tga")));
   String ^gcDestBMPFilenameAsset = assetManager->GetRepositoryRelativePath(System::IO::Path::ChangeExtension(outputPath, ".tga"));
   std::string const destBMPFilenameAsset(Helper::StringHelper::ConvertString(assetManager->GetCookedAssetPath(gcDestBMPFilenameAsset)));
   std::string const destFONTFilename(Helper::StringHelper::ConvertString(outputPath));
   std::string const imagesFilename(Helper::StringHelper::ConvertString(assetManager->GetSystemPath(imagesPath)));

   // Build dependencies
   List<AssetSystem::BuildAsset^> ^buildAssets = gcnew List<AssetSystem::BuildAsset^>();

   // Add sub fonts
   XmlNodeList^ subFonts = doc->SelectNodes("//Subfont");

   for( int i = 0; i < subFonts->Count; ++i )
   {
      XmlElement^ subFont = (XmlElement^)subFonts[i];

      String ^gcSubFontTrueTypeName = subFont->GetAttribute("name", String::Empty);
      if (gcSubFontTrueTypeName->Length == 0)
      {
         printf("No font name specified for subfont.\n");
         continue;
      }

      int size = 12;
      String ^gcSize = subFont->GetAttribute("size", String::Empty);
      if (gcSize != String::Empty)
      {
         size = System::Int32::Parse(gcSize);
      }

      bool bold = Tools::Common::Misc::ParseBool(subFont->GetAttribute("bold", String::Empty));
      bool italic = Tools::Common::Misc::ParseBool(subFont->GetAttribute("italic", String::Empty));

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

      String ^symbols = System::IO::File::OpenText( symbolsFilename )->ReadToEnd();
      fonts->Add(gcnew SCookFontInfo(gcSubFontTrueTypeName, size, bold, italic, 0, symbols ));
   }

   make_font(destFONTFilename, 
             destBMPFilename, 
             destBMPFilenameAsset, 
             imagesFilename,
             fonts,
             images, 
             assetManager);

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
      DWORD dwBold   = font->mbBold   ? FW_BOLD : FW_NORMAL;
      DWORD dwItalic = font->mbItalic ? TRUE    : FALSE;

      // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
      // anti aliased font, but this is not guaranteed.
      INT nHeight = -MulDiv( dwFontHeight, (INT)GetDeviceCaps(hDC, LOGPIXELSY), 72 );
      std::string const fontName(Helper::StringHelper::ConvertString(font->mFontName));
      HFONT hFont = CreateFont( nHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, fontName.c_str() );

      if( NULL == hFont )
      {
         printf("Error! CreateFont function could not create font %s.\n", fontName.c_str());
         return E_FAIL;
      }

      HGDIOBJ hbmOld = SelectObject( hDC, hbmBitmap );
      HGDIOBJ hFontOld = SelectObject( hDC, hFont );

      // Set text properties
      SetTextColor( hDC, RGB(255,255,255) );
      SetBkColor(   hDC, 0x00000000 );
      SetTextAlign( hDC, TA_TOP );

      // Loop through all printable character and output them to the bitmap..
      // Meanwhile, keep track of the corresponding tex coords for each character.
      SIZE size;

      // Calculate the spacing between characters based on line height
      GetTextExtentPoint32( hDC, TEXT(" "), 1, &size );

      if (fontLoop == 0)
      {
         dwPadding = ((DWORD) ceil(size.cy * 0.05f));
         x = dwOverlap + dwPadding;
         y = dwOverlap + dwPadding;
         masterFontRowHeight = size.cy;
      }

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

         // Check to see if this character exists in the font
         {
            WORD glyphIndices[4]; // Use array in case of multiple glyphs for string
            GetGlyphIndicesW(hDC, charStringUnicode.c_str(), charStringUnicode.size(), glyphIndices, GGI_MARK_NONEXISTING_GLYPHS);
            WORD glyphIndex = glyphIndices[0];        
            if (glyphIndex == 0xFFFF || ((glyphIndex == 0) && (charStringAnsi != " ")))
            {
               // Character doesn't exist
               continue;
            }
         }

         // Add character so it doesn't get added to bitmap again
         addedCharacters.insert(utf32Char);

         GetTextExtentPoint32W( hDC, charStringUnicode.c_str(), charStringUnicode.size(), &size );

         maxRowHeight = bpe::max_val(int(size.cy + dwOverlap + dwPadding), maxRowHeight);

         if( (DWORD)(x + size.cx + dwOverlap + dwPadding) > dwTexWidth )
         {
            x  = dwOverlap + dwPadding;
            y += maxRowHeight;
            
            maxRowHeight = 0;
         }

         // render character bit map
         ExtTextOutW( hDC, x+0, y+0, ETO_OPAQUE, NULL, charStringUnicode.c_str(), charStringUnicode.size(), NULL );

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
   }

   // Fix the alpha for font
   for( int ty=0; ty < dwTexHeight; ty++ )
   {
      for( int tx=0; tx < dwTexWidth; tx++ )
      {
         uint32 & color = pBitmapBits[dwTexWidth*ty + tx];

         // do a little bit of gamma correction to make the anti aliasing look better
         uint32 intensity = color & 0xff;
         intensity = (intensity * intensity) >> 8;

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

#pragma endregion

//----------------------------------------------------------------------------
