#pragma once

#include <stdio.h>
#include "CStreamDriver.h"

class CStreamDriver_RawExtract : public CStreamDriver
{
public:

   CStreamDriver_RawExtract(const char* pcFileExt);
   virtual ~CStreamDriver_RawExtract();

   virtual void ProcessPacket(const STREAM_TAG& packet, const void * const pBody);
   virtual void EndStream();

private:
   FILE* m_pFile;
   char m_fileName[255];
   char m_fileExt[255];
};