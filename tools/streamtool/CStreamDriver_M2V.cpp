#include "CStreamDriver_M2V.h"

//----------------------------------------------------------------------------
CStreamDriver_M2V::CStreamDriver_M2V() : m_pFile(NULL)
{
}

CStreamDriver_M2V::~CStreamDriver_M2V()
{
   if (m_pFile)
      fclose(m_pFile);
}

//----------------------------------------------------------------------------
void CStreamDriver_M2V::ProcessPacket(const STREAM_TAG& packet, const void * const pBody)
{
   if (!m_pFile)
   {
      sprintf(m_fileName, "%s/%s/%s.m2v", gOutputFolder, gCurrStreamName, gCurrStreamName);
      fopen_s(&m_pFile, m_fileName, "wb");
      if( !m_pFile )
      {
         printf("Error: could not open %s for writing. (need to check out file?)\n", m_fileName );
         throw false;
      }
   }

   fwrite(pBody, packet._size - sizeof(packet), 1, m_pFile);
}

//----------------------------------------------------------------------------
void CStreamDriver_M2V::EndStream()
{
   if (m_pFile)
   {
      printf("Extracting %s\n", m_fileName);
      fclose(m_pFile);
      m_pFile = NULL;
   }
}