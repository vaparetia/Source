// img2tga.cpp
// converts icon (.img) files to .tga files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "Engine/Mechanics/zlib/zlib.h"
#include "FreeImage.h"

#pragma warning(disable:4996)

typedef unsigned char u_char;
typedef unsigned short u_short;

namespace
{
#define TELOP_CODE_END		0
#define TELOP_CODE_BLANK	1
#define TELOP_CODE_BMP		2
#define TELOP_CODE_STOP		3
#define TELOP_CODE_FADE		4
#define TELOP_CODE_SPEED	5

#define ZLIB_WORK_SIZE			(45656)
#define TELOP_DECODE_BUF_WIDTH	(512)
#define TELOP_HEIGHT 448
#define TELOP_DECODE_BUF_HEIGHT	(512)

   enum {
      TELOP_TITLE,
      TELOP_WAIT,
      TELOP_DECODE,
      TELOP_BLANK,
      TELOP_STOP,
      TELOP_FADE,
      TELOP_END
   };


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

}

static z_stream z;

static int gMgsVersion = 2;

struct SRGBA
{
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
};

typedef struct {

   void *telop_vram;
   unsigned char *telop_data;
   int display_y;
   int telop_y;
   int telop_fade;
   int telop_fade_step;
   int telop_end_flag;

   int decode_y;
   int telop_state;
   int blank_count;

   int speed;

   std::vector< SRGBA >  tgaBuf;
   std::string bmpString;
} Work;

static void zlib_init( Work * work )
{
   if( inflateInit( &z ) != Z_OK )
   {
      printf("zlib_init: error!\n");
      throw false;
   }
}

static int zlib_decode( void *decode, int decode_size, Work * work )
{
   z.next_out = (Bytef*)decode;
   z.avail_out = decode_size;

   z.next_in = work->telop_data;
   z.avail_in = 0x7FFFFFFF;

   for( ;; )
   {
      int status;
      status = inflate( &z, Z_NO_FLUSH );
      if( status == Z_STREAM_END || z.avail_out == 0 ){
         work->telop_data = z.next_in;
         return ( status == Z_STREAM_END ? 0 : 1 );
         break;
      }
      if( status != Z_OK )
      {
         printf( "status %d %s\n", status, z.msg );
         throw false;
      }
   }
}

static void zlib_finish( void )
{
   inflateEnd( &z );
}

static void process_bmp( Work * work, const int lineCount, System::Xml::XmlDocument^ xmlDoc )
{
   int dec_size = ( lineCount * TELOP_DECODE_BUF_WIDTH );
   std::vector< SRGBA > tifBuf;
   tifBuf.reserve( dec_size );
   work->tgaBuf.reserve( work->tgaBuf.size() + dec_size );
   const u_char * target = (u_char*)work->telop_vram + TELOP_DECODE_BUF_WIDTH * sizeof( int );
   for( int i=0; i < dec_size; ++i )
   {
      SRGBA rgba;
      rgba.r = *target++;
      rgba.g = *target++;
      rgba.b = *target++;
      rgba.a = *target++;
      work->tgaBuf.push_back( rgba );

      //For the temporary tif, use a black-on-white with no grayscale.
      if( gMgsVersion == 2 )
      {
         //MGS2 bitmaps are white with a white-on-black grayscale alpha channel.
         int level;
         if( rgba.a > 0 )
         {
            level = (int)140 - rgba.a;
            if( level < 0 )
            {
               level = 0;
            }
         }
         else
         {
            level = 255;
         }

         rgba.r = rgba.g = rgba.b = (unsigned char)level;
         rgba.a = 255;
      }
      else
      {
         //MGS3 bitmaps are white-on-black grayscale with a full white alpha channel.
         int level = 255 - rgba.g;
         rgba.r = rgba.g = rgba.b = level;
         rgba.a = 255;
      }
      tifBuf.push_back( rgba );
   }

   //Extract the text from the bitmap for this line.
   {
      //Save a temporary TIF for OCR.
      FIBITMAP * dib = FreeImage_Allocate( TELOP_DECODE_BUF_WIDTH, lineCount, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK );
      int bytespp = FreeImage_GetLine( dib ) / FreeImage_GetWidth( dib );
      const unsigned char * pRGBA = reinterpret_cast< const unsigned char*>( &tifBuf[0] );

      for( int y = FreeImage_GetHeight(dib)-1; y >= 0; --y )
      {
         BYTE * bits = FreeImage_GetScanLine( dib, y );
         for( int x=0; x < (int)FreeImage_GetWidth( dib ); ++x, pRGBA += sizeof(unsigned int) )
         {
            const unsigned char r = *(pRGBA+0);
            const unsigned char g = *(pRGBA+1);
            const unsigned char b = *(pRGBA+2);
            const unsigned char a = *(pRGBA+3);
            bits[FI_RGBA_RED] = r;
            bits[FI_RGBA_GREEN] = g;
            bits[FI_RGBA_BLUE] = b;
            bits[FI_RGBA_ALPHA] = a;

            bits += bytespp;
         }
      }

      const char * const tifFilename = "_row2tga_temp.tif";
      FreeImage_Save(FIF_TIFF, dib, tifFilename);
      FreeImage_Unload( dib );

      //Run the Tesseract tool on the tif to get the text back out.
      char tesseractCmdLine[256];
      if( const char * const bpe_root = getenv( "BPE_ROOT" ) )
      {
         sprintf( tesseractCmdLine, "%s\\bin\\tesseract.exe %s _row2tga_temp", bpe_root, tifFilename );
         int ret = system( tesseractCmdLine );
         if( ret != 0 )
         {
            printf("Error: OCR failed!\n");
            throw false;
         }

         FILE * ocrFile = fopen( "_row2tga_temp.txt", "rt" );
         if( !ocrFile )
         {
            printf("Error: could not open _row2tga_temp.txt for reading!\n");
            throw false;
         }
         fseek( ocrFile, 0, SEEK_END );
         int ocrSize = ftell( ocrFile );
         fseek( ocrFile, 0, SEEK_SET );
         char * ocrBuf = new char[ocrSize+1];
         memset( ocrBuf, 0, ocrSize+1 );
         fread( ocrBuf, ocrSize, 1, ocrFile );
         fclose( ocrFile );
         //printf("Read OCR string: %s\n", ocrBuf );
         work->bmpString = ocrBuf;
         delete ocrBuf;
      }
      else
      {
         printf("Error: BPE_ROOT not set!\n");
         throw false;
      }
   }
}

#if 0
static void WriteTitleName( Work* work, System::Xml::XmlDocument^ xmlDoc )
{
   short width;
   short height;
   int dec_y;
   int result;

   if( *work->telop_data != TELOP_CODE_BMP )
   {
      printf("Error: Expected to start with TELOP_CODE_BMP!\n");
      throw false;
   }
   memcpy( &width, work->telop_data + 1, sizeof( short ) );
   memcpy( &height, work->telop_data + 3, sizeof( short ) );
   printf( "title width = %d heigt = %d\n", width, height );
   work->telop_data += 5;
   zlib_init( work );

   work->telop_vram = new unsigned char[height * TELOP_DECODE_BUF_WIDTH * sizeof(int)];
   dec_y = 0;

   // decode
   do {
      u_char* target;
      target = (u_char*)work->telop_vram
         + ( dec_y * TELOP_DECODE_BUF_WIDTH ) * sizeof( int );
      result = zlib_decode( target, TELOP_DECODE_BUF_WIDTH * sizeof( int ), work );
      ++dec_y;
   } while (result != 0);

   zlib_finish();

   process_bmp( work, dec_y, xmlDoc );
   delete work->telop_vram;
   work->telop_vram = NULL;

   System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "titlename" ) );
   System::Int32^ int32 = gcnew System::Int32( dec_y );
   codeElement->SetAttribute( "lines", int32->ToString() );
   codeElement->InnerText = gcnew System::String( work->bmpString.c_str() );
}
#endif

static void decode_telop( Work *work, System::Xml::XmlDocument^ xmlDoc )
{
   int dec_y, disp_y;

   if( work->telop_state == TELOP_TITLE )
   {
      work->telop_state = TELOP_WAIT;
   }

   dec_y = work->decode_y;
   disp_y = work->display_y;

   for( ;; )
   {
RETRY:
      switch( work->telop_state ){
        case TELOP_WAIT:
           switch( *work->telop_data ){
        case TELOP_CODE_END:
           {
              work->telop_state = TELOP_END;
              System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "end" ) );
           }
           break;
        case TELOP_CODE_BLANK:
           {
              //printf( "--BLANK %d\n", work->telop_y / 16 );
              memcpy( &work->blank_count, work->telop_data + 1, sizeof( int ) );
              work->telop_data += 1 + sizeof( int );
              work->telop_state = TELOP_BLANK;
              //printf( "blank = %d\n", work->blank_count );
              System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "blank" ) );
              System::Int32^ int32 = gcnew System::Int32( work->blank_count );
              codeElement->SetAttribute( "count", int32->ToString() );
           }
           break;
        case TELOP_CODE_STOP:
           {
              memcpy( &work->blank_count, work->telop_data + 1, sizeof( int ) );
              work->telop_data += 1 + sizeof( int );
              work->telop_state = TELOP_STOP;
              //printf( "stop = %d\n", work->blank_count );
              System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "stop" ) );
              System::Int32^ int32 = gcnew System::Int32( work->blank_count );
              codeElement->SetAttribute( "count", int32->ToString() );
           }
           break;
        case TELOP_CODE_FADE:
           {
              memcpy( &work->telop_fade_step, work->telop_data + 1, sizeof( int ) );
              work->telop_data += 1 + sizeof( int );
              work->telop_state = TELOP_FADE;
              //printf( "stop = %d\n", work->telop_fade_step );
              System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "fade" ) );
              System::Int32^ int32 = gcnew System::Int32( work->telop_fade_step );
              codeElement->SetAttribute( "step", int32->ToString() );
           }
           break;
        case TELOP_CODE_SPEED:
           {
              memcpy( &work->speed, work->telop_data + 1, sizeof( int ) );
              work->telop_data += 1 + sizeof( int );
              //printf( "speed = %d\n", work->speed );
              System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "speed" ) );
              System::Int32^ int32 = gcnew System::Int32( work->speed );
              codeElement->SetAttribute( "value", int32->ToString() );
           }
           break;
        case TELOP_CODE_BMP:
           //printf( "--TELOP %d\n", work->telop_y / 16 );
           work->telop_state = TELOP_DECODE;
           {
              short width;
              short height;
              memcpy( &width, work->telop_data + 1, sizeof( short ) );
              memcpy( &height, work->telop_data + 3, sizeof( short ) );
              //printf( "width = %d heigt = %d\n", width, height );
              work->telop_data += 5;

              dec_y = 0;
              if( work->telop_vram != NULL )
              {
                 printf("Error: last TELOP_DECODE did not finish!\n");
                 throw false;
              }
              work->telop_vram = new unsigned char[TELOP_DECODE_BUF_WIDTH * height * sizeof(int)];
           }
           zlib_init( work );
           break;
           }
           goto RETRY;

        case TELOP_DECODE:
           {
              void *target;
              int result;
              if( work->telop_vram == NULL )
              {
                 printf("Error: TELOP_DECODE should have been preceded by a TELOP_CODE_BMP!\n");
                 throw false;
              }
              target = (u_char*)work->telop_vram
                 + ( dec_y * TELOP_DECODE_BUF_WIDTH ) * sizeof( int );
              result = zlib_decode( target, TELOP_DECODE_BUF_WIDTH * sizeof( int ), work );
              if( result == 0 )
              {
                 zlib_finish();
                 process_bmp( work, dec_y, xmlDoc );

                 delete work->telop_vram;
                 work->telop_vram = NULL;
                 work->telop_state = TELOP_WAIT;

                 System::Xml::XmlElement^ codeElement = (System::Xml::XmlElement^)xmlDoc->DocumentElement->AppendChild( xmlDoc->CreateElement( "bmp" ) );
                 System::Int32^ int32 = gcnew System::Int32( dec_y );
                 codeElement->SetAttribute( "lines", int32->ToString() );
                 codeElement->InnerText = gcnew System::String( work->bmpString.c_str() );

                 goto RETRY;
              }
           }
           break;
        case TELOP_BLANK:
           {
#if 0
              void *target;
              target = (u_char*)work->telop_vram
                 + ( dec_y * TELOP_DECODE_BUF_WIDTH ) * sizeof( int );
              memset( target, 0, TELOP_DECODE_BUF_WIDTH * sizeof( int ) );
#endif
              work->telop_state = TELOP_WAIT;
              goto RETRY;
           }
           break;
        case TELOP_STOP:
           {
              work->telop_state = TELOP_WAIT;
              goto RETRY;
           }
           break;
        case TELOP_FADE:
           {
              work->telop_state = TELOP_WAIT;
              goto RETRY;
           }
           break;
        case TELOP_END:
           return;
      }
      work->telop_y += 16;

//      dec_y = ( dec_y + 1 ) % TELOP_DECODE_BUF_HEIGHT;
      dec_y = dec_y+1;
      disp_y = ( disp_y + 1 ) % TELOP_DECODE_BUF_HEIGHT;
   }
}


void convert( const char * const inFileName )
{
   //Load entire .row file
   int flen;
   char outFileName[FILENAME_MAX];
   FILE * fp;

   printf("Converting %s...\n", inFileName );
   fp = fopen( inFileName, "rb" );
   if( !fp )
   {
      printf("Failed to open input file %s!\n", inFileName);
      throw false;
   }
   fseek( fp, 0, SEEK_END );
   flen = ftell( fp );
   unsigned char * inbuf = new unsigned char[flen];
   fseek( fp, 0, SEEK_SET );
   fread( inbuf, 1, flen, fp );
   fclose( fp );

   Work work = { 0 };
   work.telop_data = inbuf;
   work.tgaBuf.reserve( TELOP_DECODE_BUF_WIDTH * TELOP_DECODE_BUF_HEIGHT );

   System::Xml::XmlDocument^ xmlDoc = gcnew System::Xml::XmlDocument();

   xmlDoc->AppendChild( xmlDoc->CreateElement( "telop" ) );

//   WriteTitleName( &work, xmlDoc );

   work.telop_state = TELOP_TITLE;

   decode_telop( &work, xmlDoc );

   System::String^ docName = gcnew System::String( inFileName );
   xmlDoc->Save( docName + ".telop" );
#if 1
   //Write output file.
   //Leave .row extension so source file is clear. (we will not use this tga anyway)
   strcpy( outFileName, inFileName );
   strcat( outFileName, ".tga" );

   fp = fopen( outFileName, "wb" );
   if( !fp )
   {
      printf("Failed to open output file %s!\n", outFileName );
      throw false;
   }
   {
      TGA_HEADER tga_header;
      int backwards = 0;

      memset(&tga_header, 0, sizeof(tga_header));
      tga_header.imagetype = 2;
      tga_header.width = TELOP_DECODE_BUF_WIDTH;
      tga_header.height = work.tgaBuf.size() / ( TELOP_DECODE_BUF_WIDTH );
      tga_header.bits = 32;
      tga_header.descriptor = backwards ? 0x00 : 0x20;

      fwrite(&tga_header, sizeof(tga_header), 1, fp);
      fwrite( &work.tgaBuf[0], work.tgaBuf.size() * sizeof( SRGBA ), 1, fp );
   }
   fclose(fp);
#endif

   delete inbuf;
}

void usage()
{
   fprintf(stderr, "row2tga <mgs2 .row file>\n");
}

int main(int argc, char *argv[])
{
   int len;
   if (argc != 2)
   {
      usage();
      return -1;
   }

   len = strlen(argv[1]);
   if (len <= 4 || (_stricmp(argv[1] + len - 4, ".row")&&_stricmp(argv[1] + len - 4, ".raw")))
   {
      usage();
      return -1;
   }

   if( strstr( argv[1], ".row" ) )
   {
      gMgsVersion = 2;
   }
   else
   {
      gMgsVersion = 3;
   }

   try
   {
      convert( argv[1] );
   }
   catch(...)
   {
      printf("Conversion failed: %s\n", argv[1] );
      return -1;
   }

   return 0;
}
