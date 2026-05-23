//----------------------------------------------------------------------------
// CHttpInputStream.h
// Bluepoint
// Copyright 2007
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/CInputStream.h"
#include <boost/scoped_ptr.hpp>

//----------------------------------------------------------------------------
#if BPE_TARGET == BPE_TARGET_WIN32
class GenericHTTPClient;
#endif

#if BPE_TARGET == BPE_TARGET_PS3
#include <cell/http.h>
#endif

class CGrowableMemoryOutStream;
//----------------------------------------------------------------------------

class ENGINE_API CHttpInputStream
{
public:
   CHttpInputStream(char const * const pURL);
   ~CHttpInputStream();

   void                 Update();         // Pump this to update

   enum EHTTPStatus
   {
      kHS_Downloading,
      kHS_DownloadComplete,
      kHS_Error,

      kHS_Count
   };
   EHTTPStatus          GetStatus() const;
   CInputStream *       GetInputStream(); // Will assert if data isn't finished downloaded yet.
   void                 Close();          // Release resources
   std::string const &  GetURL() const;

protected:
   void           Initialize();

   EHTTPStatus                                     mStatus;
   boost::scoped_ptr<CInputStream>                 mpInputStream;
   boost::scoped_ptr<CGrowableMemoryOutStream>     mpData;
   std::string                                     mURL;

   // Looks like the http stream stuff screws if you try and download too many streams
   enum EInternalStatus
   {
      kIS_Waiting,
      kIS_Running,
      kIS_Finished,

      kIS_Count
   };
   EInternalStatus                                 mInternalStatus;
   static int sActiveStreamCount;

#if BPE_TARGET == BPE_TARGET_WIN32
   boost::scoped_ptr<GenericHTTPClient>            mpHTTPReader;
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   CellHttpClientId                                mClient;
   CellHttpTransId                                 mTrans;
#endif

};

//----------------------------------------------------------------------------
