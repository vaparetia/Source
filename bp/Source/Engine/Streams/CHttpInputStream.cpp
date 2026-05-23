//----------------------------------------------------------------------------
// CHttpInputStream.cpp
// Bluepoint
// Copyright 2007
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <stdlib.h>
#include "Engine/Streams/CHttpInputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"
#include "Engine/Streams/CMemoryInputStream.h"
#if BPE_TARGET == BPE_TARGET_WIN32
#include "Engine/Mechanics/Http/Win32GenericHTTPClient.h"
#endif

//----------------------------------------------------------------------------
int CHttpInputStream::sActiveStreamCount = 0;
//----------------------------------------------------------------------------

CHttpInputStream::CHttpInputStream(char const * const pURL)
: mStatus(kHS_Downloading)
, mURL(pURL)
{
   mInternalStatus = kIS_Waiting;

   mpData.reset(new CGrowableMemoryOutStream());
}

//----------------------------------------------------------------------------

CHttpInputStream::~CHttpInputStream()
{
   Close();
}

//----------------------------------------------------------------------------
void CHttpInputStream::Initialize()
{
#if BPE_TARGET == BPE_TARGET_WIN32
   mpHTTPReader.reset(new GenericHTTPClient());
   if (!mpHTTPReader->Open(mURL.c_str()))
   {
      Close();
      mStatus = kHS_Error;
      return;
   }
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   mClient = 0;
   mTrans = 0;
   int ret = cellHttpCreateClient(&mClient);
   if (ret)
   {
      mStatus = kHS_Error;
      Close();
      return;
   }

   CellHttpUri uri;
   size_t uriPoolSize = 0;
   ret = cellHttpUtilParseUri(NULL, mURL.c_str(), NULL, 0, &uriPoolSize);
   if (ret)
   {
      mStatus = kHS_Error;
      Close();
      return;
   }

   boost::scoped_ptr<uint8> uriPool((uint8*)malloc(uriPoolSize));
   ret = cellHttpUtilParseUri(&uri, mURL.c_str(), uriPool.get(), uriPoolSize, NULL);
   if (ret)
   {
      mStatus = kHS_Error;
      Close();
      return;
   }

#if 0
   bpe_debugger_printf("  scheme:   %s\n", uri.scheme);
   bpe_debugger_printf("  hostname: %s\n", uri.hostname);
   bpe_debugger_printf("  port:     %d\n", uri.port);
   bpe_debugger_printf("  path:     %s\n", uri.path);
   bpe_debugger_printf("  username: %s\n", uri.username);
   bpe_debugger_printf("  password: %s\n", uri.password);
#endif

   ret = cellHttpCreateTransaction(&mTrans, mClient, CELL_HTTP_METHOD_GET, &uri);
   if (ret)
   {
      mStatus = kHS_Error;
      Close();
      return;
   }
   ret = cellHttpSendRequest(mTrans, NULL, 0, NULL);
   if (ret)
   {
      mStatus = kHS_Error;
      Close();
      return;
   }

   {
      int code = 0;
      ret = cellHttpResponseGetStatusCode(mTrans, &code);
      if (ret)
      {
         mStatus = kHS_Error;
         Close();
         return;
      }
   }
#endif
}
//----------------------------------------------------------------------------

void CHttpInputStream::Update()
{
   if ((mInternalStatus == kIS_Waiting) && (sActiveStreamCount == 0))
   {
      // Not thread safe!
      sActiveStreamCount++;
      mInternalStatus = kIS_Running;
      Initialize();
   }

   if (mInternalStatus != kIS_Running)
   {
      return;
   }

#if BPE_TARGET == BPE_TARGET_WIN32
   if (mStatus == kHS_Downloading)
   {
      uint32 const kBufferSize = 16384;
      uint8 buffer[kBufferSize];
      uint32 bytesRead = 0;

      bytesRead = mpHTTPReader->ResponseOfBytes(buffer, kBufferSize);
      if (bytesRead == 0)
      {
         DWORD const error = mpHTTPReader->GetLastError();
         if (error == ERROR_MORE_DATA)
         {
            // Still downloading
         }
         else if (error == ERROR_SUCCESS)
         {
            // Completed
            mStatus = kHS_DownloadComplete;
            Close();
            // Construct output stream
            mpData->Flush();
            mpInputStream.reset(new CMemoryInputStream(mpData->Data(), mpData->GetDataSize(), CMemoryInputStream::kOwner_App));
         }
         else
         {
            // Something's gone wrong
            Close();
            mStatus = kHS_Error;
         }
      }
      else
      {
         // Add to output buffer
         mpData->Put(buffer, bytesRead);
      }
   }
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   if (mStatus == kHS_Downloading)
   {
      uint32 const kBufferSize = 16384;
      uint8 buffer[kBufferSize];
      size_t bytesRead = 0;

      int ret = cellHttpRecvResponse(mTrans, buffer, kBufferSize, &bytesRead);
      if (ret)
      {
         Close();
         mStatus = kHS_Error;
      }
      else
      {
         // Add to output buffer
         mpData->Put(buffer, bytesRead);
         if (bytesRead == 0)
         {
            // We're done
            Close();
            mStatus = kHS_DownloadComplete;
            // Construct output stream
            mpData->Flush();
            mpInputStream.reset(new CMemoryInputStream(mpData->Data(), mpData->GetDataSize(), CMemoryInputStream::kOwner_App));
         }
      }
   }
#endif
}

//----------------------------------------------------------------------------

CHttpInputStream::EHTTPStatus CHttpInputStream::GetStatus() const
{
   return mStatus;
}

//----------------------------------------------------------------------------

CInputStream * CHttpInputStream::GetInputStream()
{
   BPE_VERIFY(mStatus == kHS_DownloadComplete, false, "Can't get stream until download is complete!");
   return mpInputStream.get();
}

//----------------------------------------------------------------------------

void CHttpInputStream::Close()
{
#if BPE_TARGET == BPE_TARGET_WIN32
   if (mpHTTPReader)
   {
      mpHTTPReader->Close();
      mpHTTPReader.reset();
   }

#endif

#if BPE_TARGET == BPE_TARGET_PS3
   if (mTrans)
   {
      int ret = cellHttpDestroyTransaction(mTrans);
      mTrans = 0;
   }
   if (mClient)
   {
      int ret = cellHttpDestroyClient(mClient);
      mClient = 0;
   }
#endif

   if (mInternalStatus == kIS_Running)
   {
      // Allow something else to run
      mInternalStatus = kIS_Finished;
      sActiveStreamCount--;
   }
}

//----------------------------------------------------------------------------

std::string const & CHttpInputStream::GetURL() const
{
   return mURL;
}

//----------------------------------------------------------------------------

