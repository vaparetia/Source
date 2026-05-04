//----------------------------------------------------------------------------
// TextToolHelper.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "TextToolHelper.h"

//----------------------------------------------------------------------------

#include "string"
#include "memory"

#include "font.h"
#include "../../BP/Shared/BP_Font.h"

//----------------------------------------------------------------------------

using namespace System::Drawing::Imaging;

std::auto_ptr<char> gResidentFont;
std::auto_ptr<char> gBPFont;

//----------------------------------------------------------------------------

static std::string ConvertString( System::String^ value )
{
   System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(value);

   char const* str = (char const*)(void*) ptr;

   std::string ret( str );

   System::Runtime::InteropServices::Marshal::FreeHGlobal( ptr );

   return ret;
}


//----------------------------------------------------------------------------

TextToolHelper::TextRender::TextRender()
{
}

//----------------------------------------------------------------------------

char* LoadBinaryFile(String^ path)
{
   char* result = NULL;

   std::string pathStr = ConvertString(path);

   FILE* handle = fopen(pathStr.c_str(), "rb");
   if( handle )
   {
      fseek(handle, 0, SEEK_END);
      int fileSize = ftell(handle);
      fseek(handle, 0, SEEK_SET);

      result = new char [fileSize];

      if( fread(result, fileSize, 1, handle) != 1 )
      {
         delete [] result;
         result = NULL;
      }

      fclose(handle);
   }

   return result;
}

//----------------------------------------------------------------------------

void TextToolHelper::TextRender::LoadResidentFont( String^ fontPath )
{
   char* fontData = LoadBinaryFile(fontPath);
   if( fontData )
   {
      gResidentFont = std::auto_ptr<char>(fontData);
      font_resident_load_set(fontData);
   }
}

//----------------------------------------------------------------------------

std::auto_ptr<char> fontData0, fontData1, fontData2, fontData3, fontData4, fontData5, fontData6, fontData7;

std::auto_ptr<char>* kFontDataPtrs[] =
{
   &fontData0,
   &fontData1,
   &fontData2,
   &fontData3,
   &fontData4,
   &fontData5,
   &fontData6,
   &fontData7
};

void TextToolHelper::TextRender::SetFontData( int type, array<Byte>^ fontData )
{
   std::auto_ptr<char>* pFontData = kFontDataPtrs[type];
   int const fontSize = fontData->Length;

   *pFontData = std::auto_ptr<char>(new char[fontSize]);
   char* pFontDataPtr = pFontData->get();

   System::Runtime::InteropServices::Marshal::Copy(fontData, 0, (IntPtr)pFontDataPtr, fontSize);

   font_set_top_addr(type, pFontDataPtr);
}

//----------------------------------------------------------------------------

Bitmap^ TextToolHelper::TextRender::RenderText(array<Byte>^ text, int width, int height, bool displayRubi)
{
   int const bufferSize = width * height * 2;
   std::auto_ptr<unsigned char> imageBuffer(new unsigned char[bufferSize]);

   unsigned char* vramtop = imageBuffer.get();
   memset(vramtop, 0, bufferSize);

   FONT_VRAMINFO vinfo;
   font_set_vraminfo( &vinfo, vramtop, width, height, 0, displayRubi ? RUBI_SIZE_H : 0, FONT_NO_KINSOKU );

   FONT_DRAWINFO drawinfo;
   font_open_drawinfo( &drawinfo, &vinfo );

   std::string textStr;
   for( int i = 0; i < text->Length; ++i )
   {
      textStr += (char)text[i];
   }

   font_draw_string( &drawinfo, (char*)textStr.c_str() );

   Bitmap^ bitmap = gcnew Bitmap(width, height, Imaging::PixelFormat::Format32bppArgb);
   
   unsigned int const palette[16] =
   {
      0x00000000,
      0xFF555555,
      0xFFAAAAAA,
      0xFFFFFFFF,

      0x00000000,
      0xFF555555,
      0xFFAAAAAA,
      0xFFFFFFFF,

      0x00000000,
      0xFF555555,
      0xFFAAAAAA,
      0xFFFFFFFF,

      0x00000000,
      0xFF555555,
      0xFFAAAAAA,
      0xFFFFFFFF
   };

   Rectangle rect = Rectangle(0, 0, width, height);

   BitmapData^ bitmapData = bitmap->LockBits(rect, ImageLockMode::WriteOnly, bitmap->PixelFormat);
   
   unsigned int* ptr = (unsigned int*)bitmapData->Scan0.ToInt32();
   int const stride = bitmapData->Stride / 4;

   for( int y = 0; y < height; ++y )
   {
      for( int x = 0; x < width; ++x )
      {
         unsigned char input = vramtop[(width*y + x) / 2];
         
         if( x & 1 )
            input = (input & 0xf0) >> 4;
         else
            input = input & 0x0f;

         ptr[y * stride + x] = palette[input];
      }
   }

   bitmap->UnlockBits(bitmapData);
   return bitmap;
}

//----------------------------------------------------------------------------

void TextToolHelper::TextRender::BP_LoadFont( String^ fontPath )
{
   char* fontData = LoadBinaryFile(fontPath);
   if( fontData )
   {
      gBPFont = std::auto_ptr<char>(fontData);
      BP_font_resident_load_set(fontData);
   }
} 

//----------------------------------------------------------------------------

Bitmap^ TextToolHelper::TextRender::BP_RenderText(array<Byte>^ text, int width, int height, int platform)
{
   return BP_RenderText(text, width, height, platform, TextToolHelper::TextPaneMode::Default);
}

//----------------------------------------------------------------------------

Bitmap^ TextToolHelper::TextRender::BP_RenderText(array<Byte>^ text, int width, int height, int platform, TextToolHelper::TextPaneMode mode)
{
   Bitmap^ bitmap = gcnew Bitmap(width, height, Imaging::PixelFormat::Format32bppArgb);

   Rectangle rect = Rectangle(0, 0, width, height);

   BitmapData^ bitmapData = bitmap->LockBits(rect, ImageLockMode::WriteOnly, bitmap->PixelFormat);

   unsigned int* ptr = (unsigned int*)bitmapData->Scan0.ToInt32();

   BP_font_set_platform(platform);
   BP_FONT_VRAMINFO vinfo;

   int c_skip = 0;
   int l_skip = 0;
   int flag = 0;
   
   switch(mode)
   {
   case TextToolHelper::TextPaneMode::MGS2_Codec:
      break;
   
   case TextToolHelper::TextPaneMode::MGS3_Codec:
      l_skip = BP_RUBI_SIZE_H / 3;
      flag = FONT_NO_KINSOKU;
      break;
   
   case TextToolHelper::TextPaneMode::MGS3_Codec_Japan:
      l_skip = BP_RUBI_SIZE_H;
      flag = 0;
      break;
   }

   BP_font_set_vraminfo_direct( &vinfo, ptr, width, bitmapData->Stride, height, c_skip, l_skip, flag );

   BP_FONT_DRAWINFO drawinfo;
   BP_font_open_drawinfo( &drawinfo, &vinfo );

   switch(mode)
   {
   case TextToolHelper::TextPaneMode::MGS3_Codec:
   case TextToolHelper::TextPaneMode::MGS3_Codec_Japan:
      BP_font_set_locate(&drawinfo, 0, BP_RUBI_SIZE_H);
      break;

   default:
      break;
   }

   BP_font_set_color(&drawinfo, 255, 255, 255, 255);

   std::string textStr;
   for( int i = 0; i < text->Length; ++i )
   {
      textStr += (char)text[i];
   }

   BP_font_draw_string( &drawinfo, (char*)textStr.c_str() );

   bitmap->UnlockBits(bitmapData);
   return bitmap;
}

//----------------------------------------------------------------------------

void TextToolHelper::TextRender::BP_GetPaneSize(TextPaneMode mode, int % width, int % height)
{
   switch(mode)
   {
   case TextToolHelper::TextPaneMode::MGS2_Codec:
      //TODO
      width = BP_FONT_BUFFER_WIDTH( 23, 0 );
      height = BP_FONT_BUFFER_HEIGHT( 4, BP_RUBI_SIZE_H );
      break;

   case TextToolHelper::TextPaneMode::MGS3_Codec:
   case TextToolHelper::TextPaneMode::MGS3_Codec_Japan:
      width = BP_FONT_BUFFER_WIDTH( 23, 0 );
      height = BP_FONT_BUFFER_HEIGHT( 4, BP_RUBI_SIZE_H );
      break;

   case TextToolHelper::TextPaneMode::Default:
   default:
      width = -1;
      height = -1;
      break;
   }
}
