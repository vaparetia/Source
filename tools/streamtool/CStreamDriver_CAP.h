#pragma once

#include <stdio.h>
#include "CStreamDriver.h"

class CStreamDriver_CAP : public CStreamDriver
{
public:
   enum EMode
   {
      kMode_Caption,
      kMode_RadioCaption,
      kMode_StrippedDownJapaneseCaption
   };

public:
   CStreamDriver_CAP(EMode const mode);
   virtual ~CStreamDriver_CAP();

   virtual void ProcessPacket(const STREAM_TAG& packet, const void * const pBody);
   virtual void EndStream();

private:

   void ProcessPacket_Caption( const STREAM_TAG & packet, const void * const pBody );
   void ProcessPacket_RadioCaption( const STREAM_TAG & packet, const void * const pBody );

   void BeginStream();

private:
   EMode mMode;
};

//----------------------------------------------------------------------------
