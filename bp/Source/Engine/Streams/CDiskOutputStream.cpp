//------------------------------------------------------------------------------------------
// CDiskOutputStream.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Streams/CDiskOutputStream.h"

#if BPE_TARGET == BPE_TARGET_WIN32

#include <windows.h>

//----------------------------------------------------------------------------

CDiskOutputStream::CDiskOutputStream(std::string const &fileName, uint32 const bufferSize)
: COutputStream( bufferSize )
, mFileHandle(CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))
{
   if( mFileHandle == INVALID_HANDLE_VALUE )
   {
      printf("Cannot open %s for write.\n", fileName.c_str());
   }

   BPE_VERIFY(mFileHandle != INVALID_HANDLE_VALUE, kException_CouldntOpenFile, "CDiskOutputStream couldn`t open file.");
}

//----------------------------------------------------------------------------

CDiskOutputStream::~CDiskOutputStream()
{
   Flush();

   BOOL closed = CloseHandle(mFileHandle);
   BPE_VERIFY(closed != 0, kException_CouldntCloseFile, "CDiskOutputStream couldn`t close file");
}

//----------------------------------------------------------------------------

void CDiskOutputStream::Write(const void* pBuffer, const uint32 bufferSize )
{
   DWORD numberOfBytesWritten;
   WriteFile(mFileHandle, pBuffer, bufferSize, &numberOfBytesWritten, NULL);
   BPE_ASSERT(numberOfBytesWritten == bufferSize, "CDiskOutputStream didn't write correct number of bytes.");
}

//----------------------------------------------------------------------------

#else

//----------------------------------------------------------------------------

CDiskOutputStream::CDiskOutputStream(std::string const &fileName, uint32 const bufferSize)
: COutputStream(bufferSize)
, mFileHandle(fopen( fileName.c_str(), "wb" ))
{
   BPE_VERIFY(mFileHandle != NULL, kException_CouldntOpenFile, "CDiskOutputStream couldn`t open file.");
}

//----------------------------------------------------------------------------

CDiskOutputStream::~CDiskOutputStream()
{
   Flush();

   bool const closedOK = (fclose((FILE*)mFileHandle) == 0);
   BPE_VERIFY(closedOK, kException_CouldntCloseFile, "CDiskOutputStream couldn`t close file");
}

//----------------------------------------------------------------------------

void CDiskOutputStream::Write(const void* pBuffer, const uint32 bufferSize)
{
   int const numberOfBytesWritten = fwrite( pBuffer, 1, bufferSize, (FILE *) mFileHandle);
   BPE_ASSERT(numberOfBytesWritten == bufferSize, "CDiskOutputStream didn't write correct number of bytes.");
}

//----------------------------------------------------------------------------

#endif // Win32

