#pragma once

#include "RVLFileIO.h"

namespace RVLFileIO_Mcs
{
   struct TFileInfo
   {
      u32 opaque[3]; // Matches McsFileInfo
   };

   typedef void TCallback( s32 result, void * cookie, TFileInfo *pInfo );   

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