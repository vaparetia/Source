#include "CStreamDriver_IPU.h"
#include <string.h>

//----------------------------------------------------------------------------
CStreamDriver_IPU::CStreamDriver_IPU() : m_pFile(NULL), m_fileOutCount(0)
{
}

CStreamDriver_IPU::~CStreamDriver_IPU()
{
   if (m_pFile)
      fclose(m_pFile);
}

//----------------------------------------------------------------------------
void CStreamDriver_IPU::ProcessPacket(const STREAM_TAG& packet, const void * const pBody)
{
   // Check for a header packet for an ipu stream. There could be multiple ipu movies so we need to write out 
   // different files using the header as an indicator for a new file.
   if (packet._size == 32 && packet._option == 0)
   {
      if (m_pFile)
      {
         fclose(m_pFile);
         m_pFile = NULL;
      }

      sprintf(m_fileName, "%s/%s/%s_%d.ipu", gOutputFolder, gCurrStreamName, gCurrStreamName, m_fileOutCount);
      fopen_s(&m_pFile, m_fileName, "wb");
      m_fileOutCount++;
   }   

   fwrite(pBody, packet._size - sizeof(packet), 1, m_pFile);
}

//----------------------------------------------------------------------------
void CStreamDriver_IPU::EndStream()
{
   if (m_pFile)
   {
      printf("Extracting %s\n", m_fileName);
      
      fclose(m_pFile);
      m_pFile = NULL;
   }
}