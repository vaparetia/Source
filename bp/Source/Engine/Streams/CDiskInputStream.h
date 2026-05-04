//------------------------------------------------------------------------------------------
// CDiskInputStream.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/CInputStream.h"

//----------------------------------------------------------------------------

#if BPE_TARGET==BPE_TARGET_RVL
#include "Engine/System/RVL/RVLFileIO.h"
#endif

class CDiskInputStream : public CInputStream
{
public:
   enum EException
   {
      kException_CouldntOpenFile,
      kException_CouldntCloseFile
   };

public:
   ENGINE_API explicit CDiskInputStream(std::string const &fileName, uint32 const bufferSize = 4096);
   ENGINE_API virtual ~CDiskInputStream();

   // Useful for loading text files.
   ENGINE_API std::string  ReadAllAsString();
   ENGINE_API uint32       GetFileLength()   { return mFileLength; };

protected:
   virtual uint32 const Read(void *pBuffer, uint32 const bufferSize);

private:
#if BPE_TARGET==BPE_TARGET_RVL
   RVLFileIO::TFileInfo mFileInfo;
#else
   BPE_HANDLE     mFileHandle;
#endif
   uint32         mFileLength;
};

//----------------------------------------------------------------------------
