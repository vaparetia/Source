#pragma once

#include <stdio.h>
#include "CStreamDriver.h"

class CStreamDriver_M2V : public CStreamDriver
{
public:

   CStreamDriver_M2V();
   virtual ~CStreamDriver_M2V();

   virtual void ProcessPacket(const STREAM_TAG& packet, const void * const pBody);
   virtual void EndStream();

private:
   FILE* m_pFile;
   char m_fileName[255];
};

//----------------------------------------------------------------------------
