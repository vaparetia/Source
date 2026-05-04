#include "CStreamDriver_RawExtract.h"
#include <string.h>

//----------------------------------------------------------------------------
CStreamDriver_RawExtract::CStreamDriver_RawExtract(const char* pcFileExt) : m_pFile(NULL)
{
   strcpy(m_fileExt, pcFileExt);
}

CStreamDriver_RawExtract::~CStreamDriver_RawExtract()
{
   if (m_pFile)
      fclose(m_pFile);
}

//----------------------------------------------------------------------------
void CStreamDriver_RawExtract::ProcessPacket(const STREAM_TAG& packet, const void * const pBody)
{
   if (!m_pFile)
   {
      sprintf(m_fileName, "%s/%s/%s.%s", gOutputFolder, gCurrStreamName, gCurrStreamName, m_fileExt);
      fopen_s(&m_pFile, m_fileName, "wb");
   }

   fwrite(pBody, packet._size - sizeof(packet), 1, m_pFile);
}

//----------------------------------------------------------------------------
void CStreamDriver_RawExtract::EndStream()
{
   if (m_pFile)
   {
      printf("Extracting %s\n", m_fileName);

      fclose(m_pFile);
      m_pFile = NULL;
   }
}