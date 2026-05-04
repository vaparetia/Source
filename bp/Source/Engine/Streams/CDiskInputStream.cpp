//------------------------------------------------------------------------------------------
// CDiskInputStream.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Engine/Streams/CDiskInputStream.h"

#if BPE_TARGET==BPE_TARGET_RVL

//----------------------------------------------------------------------------

CDiskInputStream::CDiskInputStream(std::string const &fileName, uint32 const bufferSize)
:  CInputStream(bufferSize)
,  mFileLength(0)
{
   if ( RVLFileIO::OpenForRead( &mFileInfo, fileName.c_str(), NULL ) )
   {
      mFileLength = RVLFileIO::GetFileSize( &mFileInfo );
   }
   else
   {
      BPE_VERIFY(false, kException_CouldntOpenFile, "CDiskInputStream couldn`t open file.");
   }

}

//----------------------------------------------------------------------------

CDiskInputStream::~CDiskInputStream()
{
   RVLFileIO::Close( &mFileInfo );
}

//----------------------------------------------------------------------------

const uint32 CDiskInputStream::Read(void *pBuffer, uint32 const bufferSize)
{
   size_t numberOfBytesRead = 0;
   RVLFileIO::ReadSync( &mFileInfo, pBuffer, bufferSize, &numberOfBytesRead );
   return numberOfBytesRead;
}

#elif BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>

//----------------------------------------------------------------------------

CDiskInputStream::CDiskInputStream(std::string const &fileName, uint32 const bufferSize)
:  CInputStream(bufferSize)
,  mFileHandle(CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
,  mFileLength(0)
{
   BPE_VERIFY(mFileHandle != INVALID_HANDLE_VALUE, kException_CouldntOpenFile, "CDiskInputStream couldn`t open file.");
   mFileLength = GetFileSize(mFileHandle, NULL);
}

//----------------------------------------------------------------------------

CDiskInputStream::~CDiskInputStream()
{
   BOOL closed = CloseHandle(mFileHandle);
   BPE_VERIFY(closed != 0, kException_CouldntCloseFile, "CDiskInputStream couldn`t close file");
}

//----------------------------------------------------------------------------

const uint32 CDiskInputStream::Read(void *pBuffer, uint32 const bufferSize)
{
   DWORD numberOfBytesRead = 0;
   ReadFile(mFileHandle, pBuffer, bufferSize, &numberOfBytesRead, NULL);
   return numberOfBytesRead;
}

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
// FILE version
//----------------------------------------------------------------------------

CDiskInputStream::CDiskInputStream(std::string const &fileName, uint32 const bufferSize)
:  CInputStream(bufferSize)
,  mFileHandle( fopen( fileName.c_str(), "rb" ) )
{
   BPE_VERIFY(mFileHandle != NULL, kException_CouldntOpenFile, "CDiskInputStream couldn`t open file.");
   fseek((FILE*) mFileHandle, 0, SEEK_END);
   mFileLength = ftell((FILE*) mFileHandle);
   fseek((FILE*) mFileHandle, 0, SEEK_SET);
}

//----------------------------------------------------------------------------

CDiskInputStream::~CDiskInputStream()
{
   bool const closedOK = (fclose((FILE*)mFileHandle) == 0);
   BPE_VERIFY(closedOK, kException_CouldntCloseFile, "CDiskInputStream couldn`t close file");
}

//----------------------------------------------------------------------------

const uint32 CDiskInputStream::Read(void *pBuffer, const uint32 bufferSize)
{
   int const numberOfBytesRead = fread(pBuffer, 1, bufferSize, (FILE*) mFileHandle);
   return numberOfBytesRead;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------

std::string CDiskInputStream::ReadAllAsString()
{
   std::vector<char> s;
   s.resize(GetFileLength() + 1, 0);
   Read(&s[0], GetFileLength());
   std::string fullString(&s[0]);

   return fullString;
}

//----------------------------------------------------------------------------
