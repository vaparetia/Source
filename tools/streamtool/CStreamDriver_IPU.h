#pragma once

#include <stdio.h>
#include "CStreamDriver.h"

class CStreamDriver_IPU : public CStreamDriver
{
public:

   CStreamDriver_IPU();
   virtual ~CStreamDriver_IPU();

   virtual void ProcessPacket(const STREAM_TAG& packet, const void * const pBody);
   virtual void EndStream();

private:
   FILE* m_pFile;
   int m_fileOutCount;
   char m_fileName[255];
};

//----------------------------------------------------------------------------
