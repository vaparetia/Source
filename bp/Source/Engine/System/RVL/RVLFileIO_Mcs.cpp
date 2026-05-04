#include "stdafx.h"
#include <nw4r/mcs/fileIO.h>
#include "RVLFileIO_Mcs.h"
#include <revolution/vi.h>
#include <boost/scoped_ptr.hpp>

//----------------------------------------------------------------------------

BPE_CTASSERT( sizeof( RVLFileIO_Mcs::TFileInfo ) == sizeof( nw4r::mcs::FileInfo ) );

//----------------------------------------------------------------------------


namespace
{
   char const skFilePrepend[] = "%BPE_REPOSITORY%";
   int const skFilePrependLen = sizeof( skFilePrepend );
   int const skMaxPath = 255 + skFilePrependLen;
   bool sIsInitialized = false;
   boost::scoped_ptr<char> sFileWorkBuffer;
   boost::scoped_ptr<char> sDeviceObjectData;

   class DeviceEnumerate : public nw4r::mcs::IDeviceEnumerate
   {
   public:
      DeviceEnumerate() : mbFind(false)   {}
      virtual bool                    Find(const nw4r::mcs::DeviceInfo& deviceInfo);

      bool                            IsFind() const          { return mbFind; }
      const nw4r::mcs::DeviceInfo&    GetDeviceInfo() const   { return mDeviceInfo; }

   private:
      bool                            mbFind;
      nw4r::mcs::DeviceInfo           mDeviceInfo;
   };

   bool
      DeviceEnumerate::Find(const nw4r::mcs::DeviceInfo& deviceInfo)
   {
      mbFind = true;
      mDeviceInfo = deviceInfo;
      return false;               // Do not continue search
   }

   inline nw4r::mcs::FileInfo *mcs_cast( RVLFileIO_Mcs::TFileInfo *fi )
   {
      return reinterpret_cast< nw4r::mcs::FileInfo * >( fi );
   }
}

//----------------------------------------------------------------------------

void RVLFileIO_Mcs::Init()
{
   if ( !sIsInitialized )
   {
      nw4r::mcs::Mcs_Init();
      nw4r::mcs::FileIO_Init();

      // Enumerate the communication devices
      DeviceEnumerate deviceEnumerate;
      nw4r::mcs::Mcs_EnumDevices(&deviceEnumerate);

      if (deviceEnumerate.IsFind())
      {
         const u32 deviceObjBufSize = nw4r::mcs::Mcs_GetDeviceObjectMemSize(deviceEnumerate.GetDeviceInfo());
         sDeviceObjectData.reset( new char[ deviceObjBufSize ] );
         nw4r::mcs::Mcs_CreateDeviceObject(deviceEnumerate.GetDeviceInfo(), sDeviceObjectData.get(), deviceObjBufSize);

         sFileWorkBuffer.reset( (char *) bpe_rvl_alloc_aligned_mem2( 32, nw4r::mcs::FILEIO_WORKBUFFER_SIZE ) );
         nw4r::mcs::FileIO_RegisterBuffer(sFileWorkBuffer.get());

         u32 errorCode = nw4r::mcs::Mcs_Open();
         if ( errorCode == nw4r::mcs::MCS_ERROR_SUCCESS )
         {
            if (! nw4r::mcs::Mcs_IsServerConnect())
            {
               OSReport("Waiting MCS server connect...\n");
               do
               {
                  VIWaitForRetrace();
                  nw4r::mcs::Mcs_Polling();
               }while (! nw4r::mcs::Mcs_IsServerConnect());
            }

            OSReport("MCS Server connected!\n");
         }
      }

      sIsInitialized = true;
   }
}

void RVLFileIO_Mcs::Shutdown()
{
   if ( sIsInitialized )
   {
      nw4r::mcs::FileIO_UnregisterBuffer();
      nw4r::mcs::Mcs_DestroyDeviceObject();
      sFileWorkBuffer.reset();
      sDeviceObjectData.reset();

      nw4r::mcs::Mcs_Close();
      sIsInitialized = false;
   }
}

bool RVLFileIO_Mcs::OpenForRead( TFileInfo *pInfo, char const *fileName, uint32 *pError )
{
   char newFilename[ skMaxPath ];

   strcpy( newFilename, skFilePrepend );
   strcat( newFilename, fileName );

   u32 result = nw4r::mcs::FileIO_Open( mcs_cast( pInfo ), newFilename, nw4r::mcs::FILEIO_FLAG_READ | nw4r::mcs::FILEIO_FLAG_INCENVVAR | nw4r::mcs::FILEIO_FLAG_SHAREREAD );

   if ( pError != NULL )
   {
      *pError = result;
   }

   return ( result == nw4r::mcs::FILEIO_ERROR_SUCCESS );
}

void RVLFileIO_Mcs::Close( TFileInfo *pInfo )
{
   nw4r::mcs::FileIO_Close( mcs_cast( pInfo ) );
}

uint32 RVLFileIO_Mcs::GetFileSize( TFileInfo *pInfo )
{
   return nw4r::mcs::FileIO_GetOpenFileSize( mcs_cast( pInfo ) );
}

void RVLFileIO_Mcs::ReadSync( TFileInfo *pInfo, void *pBuffer, uint32 readBytes, uint32 *pActuallyRead )
{
   u32 result = nw4r::mcs::FileIO_Read( mcs_cast( pInfo ), pBuffer, readBytes, pActuallyRead);

   BPE_VERIFY( result == nw4r::mcs::FILEIO_ERROR_SUCCESS, false, "Got a failure reading from a file" );
}

void RVLFileIO_Mcs::ReadAsync( TFileInfo *pInfo, void *pBuffer, uint32 readBytes )
{
   uint32 actuallyRead = 0;

   ReadSync( pInfo, pBuffer, readBytes, &actuallyRead );

   BPE_VERIFY( actuallyRead == readBytes, false, "Read less bytes than expected in RVLFileIO_Mcs::ReadAsync" );
}

void RVLFileIO_Mcs::ReadAsyncCallback( TFileInfo *pInfo, void *pBuffer, uint32 readBytes, TCallback *pCallback, void * cookie )
{
   uint32 actuallyRead = 0;

   ReadSync( pInfo, pBuffer, readBytes, &actuallyRead );

   BPE_VERIFY( actuallyRead == readBytes, false, "Read less bytes than expected in RVLFileIO_Mcs::ReadAsync" );

   if ( pCallback != NULL )
   {
      (*pCallback)( 0, cookie, pInfo );
   }
}

bool RVLFileIO_Mcs::IsAsyncReadComplete( TFileInfo *pInfo )
{
   return pInfo != NULL;
}

void RVLFileIO_Mcs::IdleTick()
{
   nw4r::mcs::Mcs_Polling();
}

void RVLFileIO_Mcs::Cancel( TFileInfo *pInfo )
{
   (pInfo);
}

bool RVLFileIO_Mcs::DoesExist( char const *fileName )
{
   TFileInfo fileInfo;

   if ( OpenForRead( &fileInfo, fileName, NULL ) )
   {
      Close( &fileInfo );
      return true;
   }

   return false;
}