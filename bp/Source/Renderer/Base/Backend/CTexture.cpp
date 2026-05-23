//----------------------------------------------------------------------------
// CTexture.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CTexture.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/Basics/CEventLog.h"
#include "Engine/Graphics/CColorf.h"
#include "Engine/Math/CMatrix4.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Resource/CResId.h"

//----------------------------------------------------------------------------

#define SUPPORT_PNG
#define SUPPORT_JPG
#define SUPPORT_TGA

//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

void * gpBigTextureScratchBuffer0 = NULL;
int gBigTextureScratchBuffer0Length = 1280*720;
int gWritePos = 0;

namespace
{
   struct STextureParams
   {
      int mWidth;
      int mHeight;
   };
#define MAX_TEXTURE_STAGES 16
   STextureParams gCurrentTextureParams[MAX_TEXTURE_STAGES];
}

//----------------------------------------------------------------------------

CBaseTexture::CBaseTexture(EFormat format, EType const type, EUsage usage)
:  mIsResidentTexture(0)
,  mFormat(format)
,  mType(type)
,  mUsage(usage)
,  mAdditionalFlags(0)
,  mMinRGBA(0x00000000)
,  mMaxRGBA(0xFFFFFFFF)
,  mFilterHint(kFH_Default)
,  mAlphaRefValue(0)
,  mMaxLODOffset(0)
,  mHasAlpha(false)
,  mCustomData(0)
{
}


//----------------------------------------------------------------------------

void CBaseTexture::DeleteTexture(CBaseTexture* pTexture)
{
   gpRenderBackend->DeleteRenderResourceFrameDelayed((IObject*)pTexture);
}

//----------------------------------------------------------------------------

#ifdef SUPPORT_PNG
#include "ExtLibraries/libPNG/png.h"
#endif

void CBaseTexture::FTextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   CResId const &resourceId = buildData.mResourceId;

   if( buildData.mpMemory && buildData.mSize )
   {
      // Check to see if it's a jpeg file
#ifdef SUPPORT_JPG
      {
         uint32 header;
         {
            CMemoryInputStream stream(buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App);
            header = stream.ReadUint32();
         }
         if (header == 0xFFD8FFE0)
         {
            // Yep, it's a jpeg, use jpeg factory
            return FJPGTextureFactory(buildData, returnResource);
         }
      }
#endif

#ifdef SUPPORT_PNG
      {
         // Check if it's a PNG file
         int const kMaxBytesToCheck = 8;
         if (png_sig_cmp((png_bytep)buildData.mpMemory, (png_size_t)0, kMaxBytesToCheck) == 0)
         {
            // It's a PNG file
            FPNGTextureFactory(buildData, returnResource);
            return;
         }
      }
#endif

#ifdef SUPPORT_TGA
      {
         unsigned char tga_header[18] = { 0 };
         tga_header[2] = 2;//2  -  Uncompressed, RGB images
         //The rest of the header bytes are zero in our data.
         if( !memcmp( tga_header, buildData.mpMemory, 12 ) )
         {
            FTGATextureFactory( buildData, returnResource );
            return;
         }
      }
#endif

      // Nope, must be a regular 'TXTR', use texture factory.
      CBaseTexture* pTexture = Create(buildData.mpMemory, buildData.mSize);
#if BPE_TARGET == BPE_TARGET_WIN32 || defined(_DEBUG)
      if( pTexture )
      {
         pTexture->mDebugName = resourceId;
      }
#endif
      returnResource.mpResource = pTexture;
      return;
   }
   else
   {
      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return;
   }
}

//----------------------------------------------------------------------------

CVector4 const CBaseTexture::GetSpecularPowerMapSample(CVector2 const & texCoord, CVector2 const & /*texelSize*/, void* /*data*/)
{
   real32 const value = texCoord[kAX];
   real32 const power = 1.0f + (texCoord[kAY] * texCoord[kAY]) * 127.0f;

   real32 const out = bpe::min_val(1.0f, powf(value, power) );

   return CVector4(out, out, out, out);
}


//----------------------------------------------------------------------------

#ifdef SUPPORT_PNG
// callback data for libPNG
struct SPNGCallbackData
{
   SPNGCallbackData()
      :  mpTexture(NULL)
      ,  mpImageStart(NULL)
      ,  mPitch(0)
   {
   }

   CBaseTexture * mpTexture;
   uint8 *        mpImageStart;
   int            mPitch;
};

//----------------------------------------------------------------------------

static void bpe_png_info_callback(png_structp png_ptr, png_infop info)
{
    SPNGCallbackData * pThis = reinterpret_cast<SPNGCallbackData*>(png_get_progressive_ptr(png_ptr));

    uint16 width = (uint16) png_get_image_width(png_ptr, info);
    // Round up to 4 (16 byte aligned)
    width = (width + 3) & (~3);

    uint16 height = (uint16) png_get_image_height(png_ptr, info);
    // Round up to 4 (16 byte aligned)
    height = (height + 3) & (~3);

    pThis->mpTexture = CBaseTexture::CreateTexture(width, height, 1, CBaseTexture::kFormat_A8R8G8B8);
    pThis->mpTexture->Lock((void**)&pThis->mpImageStart, &pThis->mPitch);

    int const channels = png_get_channels(png_ptr, info);
    int const bit_depth = png_get_bit_depth(png_ptr, info);
    int const color_type = png_get_color_type(png_ptr, info);

    pThis->mpTexture->mHasAlpha = (channels == 4);

    // Set options so we're ARGB, 8 bit
    if (color_type == PNG_COLOR_TYPE_PALETTE)
       png_set_palette_to_rgb(png_ptr);

    if (bit_depth == 16)
       png_set_strip_16(png_ptr);

    if (bit_depth < 8)
       png_set_packing(png_ptr);

#if BPE_TARGET == BPE_TARGET_WIN32
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
       png_set_bgr(png_ptr);
#endif

#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
   png_set_swap_alpha(png_ptr);
#endif

    if (color_type == PNG_COLOR_TYPE_RGB)
       png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    // Argh! They don't tell you that you have to call this in the documentation! 
    // Otherwise zlib goes nuts because you haven't allocated a row buffer.
    png_read_update_info(png_ptr, info);
}

//----------------------------------------------------------------------------

static void bpe_png_row_callback(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
   SPNGCallbackData * pThis = reinterpret_cast<SPNGCallbackData*>(png_get_progressive_ptr(png_ptr));

   // Ptr to current row
   png_bytep pOldRow = (png_bytep)(pThis->mpImageStart + (row_num * pThis->mPitch));
   png_progressive_combine_row(png_ptr, pOldRow, new_row);
}

//----------------------------------------------------------------------------

static void bpe_png_end_callback(png_structp png_ptr, png_infop info)
{
   SPNGCallbackData * pThis = reinterpret_cast<SPNGCallbackData*>(png_get_progressive_ptr(png_ptr));
   pThis->mpTexture->Unlock();
}

//----------------------------------------------------------------------------

static void bpe_png_user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
   bpe_debugger_printf("FPNGTextureFactory Error! %s\n", error_msg);
   BPE_VERIFYA(false, "FPNGTextureFactory Error!");
}  

//----------------------------------------------------------------------------

static void bpe_png_user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
   bpe_debugger_printf("FPNGTextureFactory warning: %s\n", warning_msg);
}

//----------------------------------------------------------------------------

void CBaseTexture::FPNGTextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   returnResource.mpResource = CreateFromPNG(buildData.mpMemory, buildData.mSize);
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::CreateFromPNG(void * pMemory, int const size)
{
   if ( (pMemory == NULL) || (size == 0) )
   {
      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return NULL;
   }

   png_structp png_ptr;
   png_infop info_ptr;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, bpe_png_user_error_fn, bpe_png_user_warning_fn);//(png_voidp)user_error_ptr, user_error_fn, user_warning_fn);
   if (!png_ptr)
   {
      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return NULL;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
   {
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL,(png_infopp)NULL);

      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return NULL;
   }
   png_set_interlace_handling(png_ptr);

   SPNGCallbackData userCallbackData;

   // Setup our callbacks
   png_set_progressive_read_fn(png_ptr, (void *) &userCallbackData, bpe_png_info_callback, bpe_png_row_callback, bpe_png_end_callback);

   // Process data, all copying is done in the callbacks
   png_process_data(png_ptr, info_ptr, (png_bytep) pMemory, size);

   // Clean up libPNG
   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

   return userCallbackData.mpTexture;
}

static void user_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
   if( length + gWritePos > gBigTextureScratchBuffer0Length )
   {
      png_error(png_ptr, "Write Error");
   }

   char* pBufferWrite = (char*)gpBigTextureScratchBuffer0;
   memcpy( pBufferWrite + gWritePos, data, length );
   gWritePos += length;
}

static void user_flush_fn(png_structp png_ptr)
{
   // we are writing to a piece of memory, no flush neccessary
}

bool CBaseTexture::SaveToPNG(void ** pMemory, int * psize)
{
   if( gpBigTextureScratchBuffer0 == NULL )
   {
      gpBigTextureScratchBuffer0 = malloc(gBigTextureScratchBuffer0Length);
   }

   if ( (pMemory == NULL) || (psize == NULL) )
   {
      BPE_VERIFYA( false, "Invalid Data to SaveToPNG" );
      return false;
   }

   FILE *fp = NULL;
   png_structp png_ptr;
   png_infop info_ptr;

   // PNG_TEST_PS3 - comment this in to test on PS3
   /*
   fp = fopen("/app_home/test.png", "wb");
   if( !fp )
   {
      return false;
   }
   */

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, bpe_png_user_error_fn, bpe_png_user_warning_fn);
   if( !png_ptr )
   {
      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return NULL;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if( !info_ptr )
   {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return NULL;
   }

   // set error handling
   if( setjmp(png_ptr->jmpbuf) )
   {
      png_write_destroy(png_ptr);
      if (fp != NULL)
      {
         fclose(fp);
      }
      free(png_ptr);
      free(info_ptr);
      return false;
   }

   // stick texture data into rgb buffer
   int imageWidth = GetWidth();
   int imageHeight = GetHeight();
   int row_stride = imageWidth*3;

   uint8* pOutputMemory = NULL;
   int texturePitch = 0;
   uint8* pRGBBuffer = (uint8*)malloc(imageWidth*imageHeight*3);
   Lock((void**)&pOutputMemory, &texturePitch);
   for( int yy=0;yy<imageHeight;++yy )
   {
      for( int xx=0;xx<imageWidth;++xx )
      {
         CColor pixelColor = CColor::FromARGB(*(uint32*)(&pOutputMemory[yy*texturePitch+xx*4]));
         pRGBBuffer[yy*row_stride+xx*3+0] = pixelColor.GetR();
         pRGBBuffer[yy*row_stride+xx*3+1] = pixelColor.GetG();
         pRGBBuffer[yy*row_stride+xx*3+2] = pixelColor.GetB();
      }
   }
   Unlock();

   // set up the output control if you are using standard C streams
   png_init_io(png_ptr, fp);

   // PNG_TEST_PS3 - comment png_set_write_fn out to test on PS3
   png_set_write_fn(png_ptr, NULL, user_write_fn, user_flush_fn);
   // NOTE: this is not thread safe as we are using global variables in callback routines
   gWritePos = 0;

   // set the file information here
   png_set_IHDR(png_ptr, info_ptr, imageWidth, imageHeight, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

   // optional significant bit chunk
   info_ptr->valid |= PNG_INFO_sBIT;
   // we are dealing with a color image
   info_ptr->sig_bit.red = 0xFF;
   info_ptr->sig_bit.green = 0xFF;
   info_ptr->sig_bit.blue = 0xFF;
  
   // write the file information
   png_write_info(png_ptr, info_ptr);

   // the easiest way to write the image
   png_bytep* row_pointers = new png_bytep[imageHeight];
   for( int yy=0; yy<imageHeight; ++yy )
   {
      row_pointers[yy] = &pRGBBuffer[yy*row_stride];
   }
   png_write_image(png_ptr, row_pointers);

   // write the rest of the file
   png_write_end(png_ptr, info_ptr);

   // return results
   *pMemory = gpBigTextureScratchBuffer0;
   *psize = gWritePos;

   // clean up
   png_destroy_write_struct(&png_ptr, &info_ptr);
   free(pRGBBuffer);
   delete [] row_pointers;

   if (fp != NULL)
   {
      fclose(fp);
   }
   return true;
}
#endif

//----------------------------------------------------------------------------

#ifdef SUPPORT_TGA

void CBaseTexture::FTGATextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   returnResource.mpResource = CreateFromTGA(buildData.mpMemory, buildData.mSize);
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::CreateFromTGA(void * pMemory, int const size)
{
   const uint8 * const tga_header = (const uint8*)pMemory;
   BPE_VERIFY( tga_header[2] == 2, false, "TODO: Unsupported format" );

   bool backwards = false;

   unsigned short width = (unsigned short)tga_header[12] | (unsigned short)( tga_header[13] << 8 );
   unsigned short height = (unsigned short)tga_header[14] | (unsigned short)( tga_header[15] << 8 );
   const uint32 bpp = tga_header[16];
   BPE_VERIFY( bpp == 32, false, "TODO: Unsupported format" );
   const int screenOriginBit = tga_header[17] & 0x20;

   uint32*        pBaseDestImageData;
   int            pitch;
   CBaseTexture * const pTexture = CBaseTexture::CreateTexture(width, height, 1, CBaseTexture::kFormat_A8R8G8B8);
   pTexture->Lock((void**)&pBaseDestImageData, &pitch);

   pTexture->mHasAlpha = true;

   const uint8 * const pBaseSrcImageData = tga_header + 18;
   for( int y = 0; y < height; ++y)
   {
      //N.B. if screen origin bit, lines are in reverse order.
      const uint8 * pSrc = pBaseSrcImageData + (width*sizeof(uint32)) * ( screenOriginBit ? y : ( height - 1 - y ) );
      for( int x = 0; x < width; ++x, pSrc+=4 )
      {
         const uint8 b = *(pSrc+0);
         const uint8 g = *(pSrc+1);
         const uint8 r = *(pSrc+2);
         const uint8 a = *(pSrc+3);
         const uint32 c = (uint32(a)<<24) | (uint32(r)<<16) | (uint32(g)<<8) | (uint32(b));
         pBaseDestImageData[(y * pitch / 4)  + x] = c;
      }
   }

   pTexture->Unlock();
   return pTexture;
}

#endif //SUPPORT_TGA

//----------------------------------------------------------------------------
// Windows.h fudge for libJPEG

#ifdef SUPPORT_JPG
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
#define XMD_H
#undef FAR
#endif
extern "C"
{
#include "ExtLibraries/libJPEG/jpeglib.h"
};

#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
#undef XMD_H
#endif

#include <setjmp.h>

//----------------------------------------------------------------------------

/*
* ERROR HANDLING:
*
* The JPEG library's standard error handler (jerror.c) is divided into
* several "methods" which you can override individually.  This lets you
* adjust the behavior without duplicating a lot of code, which you might
* have to update with each future release.
*
* Our example here shows how to override the "error_exit" method so that
* control is returned to the library's caller when a fatal error occurs,
* rather than calling exit() as the standard error_exit method does.
*
* We use C's setjmp/longjmp facility to return control.  This means that the
* routine which calls the JPEG library must first execute a setjmp() call to
* establish the return point.  We want the replacement error_exit to do a
* longjmp().  But we need to make the setjmp buffer accessible to the
* error_exit routine.  To do this, we make a private extension of the
* standard JPEG error handler object.  (If we were using C++, we'd say we
* were making a subclass of the regular error handler.)
*
* Here's the extended error handler struct:
*/

struct my_error_mgr {
   struct jpeg_error_mgr pub;	/* "public" fields */

   jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
* Here's the routine that will replace the standard error_exit method:
*/

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
   /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
   my_error_ptr myerr = (my_error_ptr) cinfo->err;

   /* Always display the message. */
   /* We could postpone this until after returning, if we chose. */
   (*cinfo->err->output_message) (cinfo);

   /* Return control to the setjmp point */
   longjmp(myerr->setjmp_buffer, 1);
}


//----------------------------------------------------------------------------

void CBaseTexture::FJPGTextureFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{  
   returnResource.mpResource = CreateFromJPEG(buildData.mpMemory, buildData.mSize);
}

void LoadJPEGHeader_Internal(void * pMemory, int const size, jpeg_decompress_struct* pcinfo, jpeg_source_mgr* pmemory_stream, my_error_mgr* pjerr)
{
   memset(pcinfo, 0, sizeof(jpeg_decompress_struct));

   // Might want to improve error handling!
   /* We set up the normal JPEG error routines, then override error_exit. */
   pcinfo->err = jpeg_std_error(&pjerr->pub);
   pjerr->pub.error_exit = my_error_exit;
   
   /* Now we can initialize the JPEG decompression object. */
   jpeg_create_decompress(pcinfo);

   /* Step 2: specify data source (eg, a file) */
   jpeg_init_memory_stream_source(pmemory_stream, pMemory, size); 
   pcinfo->src = pmemory_stream;

   jpeg_read_header(pcinfo, TRUE);
}

bool LoadJPEGData_Internal(jpeg_decompress_struct* pcinfo, CBaseTexture* pTexture)
{   
   jpeg_start_decompress(pcinfo);

   if( pcinfo->output_width != pTexture->GetWidth() || pcinfo->output_height != pTexture->GetHeight() )
   {
      BPE_VERIFYA( false, "Invalid Data LoadJPEGData_Internal" );
      return false;
   }

   uint32* pOutputMemory = NULL;
   int texturePitch = 0;
   pTexture->Lock((void**)&pOutputMemory, &texturePitch);

   int row_stride = pcinfo->output_width * pcinfo->output_components;
   /* Make a one-row-high sample array that will go away when done with image */
   JSAMPARRAY buffer = (*pcinfo->mem->alloc_sarray)((j_common_ptr) pcinfo, JPOOL_IMAGE, row_stride, 1);

   while(pcinfo->output_scanline < pcinfo->output_height) 
   {
      (void) jpeg_read_scanlines(pcinfo, buffer, 1);
      if(pcinfo->output_components == 3)   // RGB
      {
         for (int loop = 0; loop < pcinfo->output_width; loop++)
         {
            uint8 const red = buffer[0][loop * 3];
            uint8 const green = buffer[0][(loop * 3) + 1];
            uint8 const blue = buffer[0][(loop * 3) + 2];
            uint32 const color = CColor(red, green, blue).GetARGB();

            pOutputMemory[loop] = color;
         }

         pOutputMemory += (texturePitch / 4);
      }
      else
      {
         for (int loop = 0; loop < pcinfo->output_width; loop++)
         {
            // Not RGB, just stuff a green texture in there
            uint32 const color = CColor(0, 0xFF, 0).GetARGB();
            pOutputMemory[loop] = color;
         }

         pOutputMemory += (texturePitch / 4);
      }
   }

   jpeg_finish_decompress(pcinfo);
   jpeg_destroy_decompress(pcinfo);

   pTexture->Unlock();

   return true;
}

CBaseTexture * CBaseTexture::CreateFromJPEG(void * pMemory, int const size)
{
   if ( (pMemory == NULL) || (size == 0) )
   {
      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return NULL;
   }
   
   jpeg_source_mgr memory_stream;
   jpeg_decompress_struct cinfo;
   my_error_mgr jerr;
   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer)) 
   {
      /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
      jpeg_destroy_decompress(&cinfo);
      BPE_VERIFYA( false, "Invalid jpeg data to generate texture" );
      return NULL;
   }

   LoadJPEGHeader_Internal(pMemory, size, &cinfo, &memory_stream, &jerr);
   CBaseTexture* pTexture = CBaseTexture::CreateTexture(cinfo.output_width, cinfo.output_height, 1, kFormat_A8R8G8B8 );
   if( !LoadJPEGData_Internal(&cinfo, pTexture) )
   {
      return NULL;
   }
   return pTexture;
}

bool CBaseTexture::LoadFromJPEG(void * pMemory, int size)
{
   if ( (pMemory == NULL) || (size == 0) )
   {
      BPE_VERIFYA( false, "Invalid Data to generate texture" );
      return false;
   }
   
   jpeg_source_mgr memory_stream;
   jpeg_decompress_struct cinfo;
   my_error_mgr jerr;
   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer)) 
   {
      /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
      jpeg_destroy_decompress(&cinfo);
      BPE_VERIFYA( false, "Invalid jpeg data to generate texture" );
      return false;
   }

   LoadJPEGHeader_Internal(pMemory, size, &cinfo, &memory_stream, &jerr);
   if( !LoadJPEGData_Internal(&cinfo, this) )
   {
      return false;
   }
   return true;
}

static void init_destination__DUMMY(j_compress_ptr cinfo)
{
   // This does nothing, we need it so we don't crash
   // when the JPEG library calls into function pointers
}

static boolean empty_output_buffer__DUMMY(j_compress_ptr cinfo)
{
   // This does nothing, we need it so we don't crash
   // when the JPEG library calls into function pointers
   return TRUE;
}

static void term_destination__DUMMY(j_compress_ptr cinfo)
{
   // This does nothing, we need it so we don't crash
   // when the JPEG library calls into function pointers
}

bool CBaseTexture::SaveToJPEG(void ** pMemory, int * psize)
{
   if( gpBigTextureScratchBuffer0 == NULL )
   {
      gpBigTextureScratchBuffer0 = malloc(gBigTextureScratchBuffer0Length);
   }

   if ( (pMemory == NULL) || (psize == 0) )
   {
      BPE_VERIFYA( false, "Invalid Data to SaveToJPEG" );
      return false;
   }

   jpeg_compress_struct cinfo;
   memset(&cinfo, 0, sizeof(jpeg_compress_struct));

   my_error_mgr jerr;

   // Might want to improve error handling!
   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;

   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer)) 
   {
      /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
      jpeg_destroy_compress(&cinfo);
      BPE_VERIFYA( false, "Invalid jpeg data to generate texture" );
      return false;
   }
   
   /* Now we can initialize the JPEG decompression object. */
   jpeg_create_compress(&cinfo);

   jpeg_destination_mgr memory_stream;
   memory_stream.free_in_buffer = gBigTextureScratchBuffer0Length;
   memory_stream.next_output_byte = (JOCTET*)gpBigTextureScratchBuffer0;
   memory_stream.init_destination = init_destination__DUMMY;
   memory_stream.empty_output_buffer = empty_output_buffer__DUMMY;
   memory_stream.term_destination = term_destination__DUMMY;
   
   cinfo.dest = &memory_stream;
   cinfo.image_width = GetWidth();
   cinfo.image_height = GetHeight();
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;
   jpeg_set_defaults(&cinfo);

   jpeg_start_compress(&cinfo, TRUE);
   
   int row_stride = cinfo.image_width * cinfo.input_components;

   // stick texture data into rgb buffer
   uint8* pOutputMemory = NULL;
   int texturePitch = 0;
   uint8* pRGBBuffer = (uint8*)malloc(cinfo.image_width*cinfo.image_height*3);
   Lock((void**)&pOutputMemory, &texturePitch);
   for( int yy=0;yy<cinfo.image_height;++yy )
   {
      for( int xx=0;xx<cinfo.image_width;++xx )
      {
         CColor pixelColor = CColor::FromARGB(*(uint32*)(&pOutputMemory[yy*texturePitch+xx*4]));
         pRGBBuffer[yy*row_stride+xx*3+0] = pixelColor.GetR();
         pRGBBuffer[yy*row_stride+xx*3+1] = pixelColor.GetG();
         pRGBBuffer[yy*row_stride+xx*3+2] = pixelColor.GetB();
      }
   }
   Unlock();

   JSAMPLE* row_pointer;
   while(cinfo.next_scanline < cinfo.image_height)
   {
     row_pointer = (JSAMPLE*)&pRGBBuffer[cinfo.next_scanline*row_stride];
     jpeg_write_scanlines(&cinfo, &row_pointer, 1);
   }

   (void) jpeg_finish_compress(&cinfo);

   // return results
   *pMemory = gpBigTextureScratchBuffer0;
   *psize = gBigTextureScratchBuffer0Length - cinfo.dest->free_in_buffer;

   // clean up
   jpeg_destroy_compress(&cinfo);
   free(pRGBBuffer);
   
   return true;
}
#endif

//----------------------------------------------------------------------------

char const *CBaseTexture::TextureFormatAsString( EFormat const format )
{
   switch ( format )
   {
   case kFormat_A8R8G8B8:
      return "ARGB8";

   case kFormat_A16B16G16R16F:
      return "ABGR16";

   case kFormat_R32F:
      return "R32F";

   case kFormat_D24X8:
      return "F24X8";

   case kFormat_DXT1:
      return "DXT1";

   case kFormat_DXT3:
      return "DXT3";

   case kFormat_DXT5:
      return "DXT5";

   case kFormat_A32B32G32R32F:
      return "ABGR32F";

   case kFormat_Luminance8:
      return "Lum8";

   case kFormat_D24FS8:
      return "FloatingDepth24S8";

   case kFormat_YVU420P2_CSC1:
      return "YVU420P2_CSC1";

#if BPE_TARGET == BPE_TARGET_VITA
   case kFormat_A8B8G8R8:
      return "ABGR8";
#endif

   case kFormat_Invalid:
      return "Invalid";

   default:
      BPE_ASSERTA( "Invalid texture format" );
      return "?????";
   }
}

CBaseTexture * CBaseTexture::CreateTexture( int const width, int const height, int const levels, EFormat const format, SCreateTextureParams const * pCreateTextureParams )
{
   return Create( width, height, levels, format, kUsage_TextureLinear, kAA_None, kRM_Video, pCreateTextureParams );
}

//----------------------------------------------------------------------------

void CBaseTexture::CloneBaseTexture(CBaseTexture* pSource)
{
   BPE_VERIFY(mUsage == kUsage_Texture || mUsage == kUsage_TextureLinear, false, "Only cloning of textures is supported.");

   mIsResidentTexture = pSource->mIsResidentTexture;
#if BPE_TARGET == BPE_TARGET_WIN32 || defined(_DEBUG)
   mDebugName = pSource->mDebugName;
#endif
   mFormat = pSource->mFormat;
   mType = pSource->mType;
   mUsage = pSource->mUsage;
   mAdditionalFlags = pSource->mAdditionalFlags;
   mMinRGBA = pSource->mMinRGBA;
   mMaxRGBA = pSource->mMaxRGBA;
   mFilterHint = pSource->mFilterHint;
   mAlphaRefValue = pSource->mAlphaRefValue;
   mMaxLODOffset = pSource->mMaxLODOffset;
   mHasAlpha = pSource->mHasAlpha;
   mCustomData = pSource->mCustomData;
}

//----------------------------------------------------------------------------

void CBaseTexture::SetCurrentTextureParams(int const textureStage) const
{
   gCurrentTextureParams[textureStage].mWidth = GetWidth();
   gCurrentTextureParams[textureStage].mHeight = GetHeight();
}

int CBaseTexture::GetCurrentTextureWidth(int const textureStage)
{
   return gCurrentTextureParams[textureStage].mWidth;
}

int CBaseTexture::GetCurrentTextureHeight(int const textureStage)
{
   return gCurrentTextureParams[textureStage].mHeight;
}
