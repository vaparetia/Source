#pragma once

#include "RVLFileIO.h"
#include <revolution/dvd.h>

namespace RVLFileIO_Dvd
{
   struct TFileInfo;

   typedef void TCallback( s32 result, void * cookie, TFileInfo *pInfo );   

   struct TFileInfo
   {
      TFileInfo()
         : mReadOffset( 0 )
      {
      }

      DVDFileInfo mDvdInfo; // KEEP THIS AS THE TOP MEMBER
      uint32 mReadOffset;
      TCallback *mpCallback;
   };

   void Init();
   void Shutdown();
   void IdleTick();

   bool DoesExist( char const *fileName );
   bool OpenForRead( TFileInfo *pInfo, char const *fileName, uint32 *pError );
   void Close( TFileInfo *pInfo );
   uint32 GetFileSize( TFileInfo *pInfo );
   void ReadSync( TFileInfo *pInfo, void *pBuffer, uint32 readBytes, uint32 *pActuallyRead );
   void ReadAsync( TFileInfo *pInfo, void *pBuffer, uint32 readBytes );
   void ReadAsyncCallback( TFileInfo *pInfo, void *pBuffer, uint32 readBytes, TCallback *pCallback, void * cookie );
   bool IsAsyncReadComplete( TFileInfo *pInfo );
   void Cancel( TFileInfo *pInfo );
}