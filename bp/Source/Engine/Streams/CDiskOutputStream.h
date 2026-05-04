//------------------------------------------------------------------------------------------
// CDiskOutputStream.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/COutputStream.h"

//----------------------------------------------------------------------------

class CDiskOutputStream : public COutputStream
{
public:
   enum EException
   {
      kException_CouldntOpenFile,
      kException_CouldntCloseFile
   };

   ENGINE_API explicit CDiskOutputStream(std::string const & fileName, uint32 const bufferSize = 4096);
   ENGINE_API ~CDiskOutputStream();

protected:
   void Write(void const * pBuffer, uint32 const bufferSize);

private:
   BPE_HANDLE mFileHandle;
};

//----------------------------------------------------------------------------
