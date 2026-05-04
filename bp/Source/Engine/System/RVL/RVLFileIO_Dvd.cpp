#include "stdafx.h"
#include "RVLFileIO_Dvd.h"

namespace RVLFileIO_Dvd
{
   inline uint32 get_real_read_bytes( TFileInfo *pInfo, uint32 const readBytes )
   {
      uint32 const length = pInfo->mDvdInfo.length;
      uint32 const offset = pInfo->mReadOffset;
      uint32 realReadBytes;

      if ( readBytes + offset > length )
      {
         realReadBytes = length - offset;
      }
      else
      {
         realReadBytes = readBytes;
      }

      return realReadBytes;
   }
}

void RVLFileIO_Dvd::Init()
{
   DVDInit();
}

void RVLFileIO_Dvd::Shutdown()
{
}

bool RVLFileIO_Dvd::OpenForRead( TFileInfo *pInfo, char const *fileName, uint32 *pError )
{
   s32 entryNum = DVDConvertPathToEntrynum( fileName );
   uint32 result = 1; // fail

   pInfo->mReadOffset = 0;

   if ( entryNum != -1 )
   {
      if ( DVDFastOpen( entryNum, &pInfo->mDvdInfo ) )
      {
         result = 0;
      }
   }

   if ( pError )
   {
      *pError = result;
   }

   return result > 0;
}

void RVLFileIO_Dvd::Close( TFileInfo *pInfo )
{
   DVDClose( &pInfo->mDvdInfo );  
}

uint32 RVLFileIO_Dvd::GetFileSize( TFileInfo *pInfo )
{
   return pInfo->mDvdInfo.length;
}

void RVLFileIO_Dvd::ReadSync( TFileInfo *pInfo, void *pBuffer, uint32 readBytes, uint32 *pActuallyRead )
{
   uint32 const realReadBytes = get_real_read_bytes( pInfo, readBytes );
   s32 result = DVDRead( &pInfo->mDvdInfo, pBuffer, realReadBytes, pInfo->mReadOffset );

   BPE_VERIFY( result >= 0, false, "Error reading from file" );

   pInfo->mReadOffset += uint32( result );
}

void RVLFileIO_Dvd::ReadAsync( TFileInfo *pInfo, void *pBuffer, uint32 readBytes )
{
   ReadAsyncCallback( pInfo, pBuffer, readBytes, NULL, 0 );
}

namespace RVLFileIO_Dvd
{
   void DvdCallback( s32 result, DVDFileInfo *pDVDFileInfo )
   {
      TFileInfo *pFileInfo = reinterpret_cast< TFileInfo * >( pDVDFileInfo );

      if ( pFileInfo->mpCallback )
      {
         (*pFileInfo->mpCallback)( result, DVDGetUserData( &pDVDFileInfo->cb ), pFileInfo );
      }
   }
}

void RVLFileIO_Dvd::ReadAsyncCallback( TFileInfo *pInfo, void *pBuffer, uint32 readBytes, TCallback *pCallback, void *cookie )
{
   uint32 const realReadBytes = get_real_read_bytes( pInfo, readBytes );
   BPE_VERIFY( realReadBytes == readBytes, false, "Read less bytes than expected in RVLFileIO_Mcs::ReadAsync" );
   BPE_VERIFY( DVDGetFileInfoStatus( &pInfo->mDvdInfo ) == DVD_STATE_END, false, "Using a file info that's already being used" );

   DVDSetUserData( &pInfo->mDvdInfo.cb, cookie );
   pInfo->mpCallback = pCallback;
   DVDReadAsync( &pInfo->mDvdInfo, pBuffer, realReadBytes, pInfo->mReadOffset, &DvdCallback );
}

bool RVLFileIO_Dvd::IsAsyncReadComplete( TFileInfo *pInfo )
{
   switch ( DVDGetFileInfoStatus( &pInfo->mDvdInfo ) )
   {
   case DVD_STATE_END:
   case DVD_STATE_CANCELED:
      return true;
   default:
      return false;
   }
}

void RVLFileIO_Dvd::IdleTick()
{
}

void RVLFileIO_Dvd::Cancel( TFileInfo *pInfo )
{
   pInfo->mpCallback = NULL;
   DVDCancel( &pInfo->mDvdInfo.cb );
}

bool RVLFileIO_Dvd::DoesExist( char const *fileName )
{
   return ( DVDConvertPathToEntrynum( fileName ) != -1 );
}