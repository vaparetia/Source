#pragma once

#include <stdio.h>
#include <vector>
#include "CStreamDriver.h"

struct _dm_work;
struct _demo_packet;

class CStreamDriver_Demo : public CStreamDriver
{
public:
   CStreamDriver_Demo();
   virtual ~CStreamDriver_Demo();

   virtual void ProcessPacket(const STREAM_TAG& packet, const void * const pBody);
   virtual void EndStream();

private:
   void BeginStream();

   void DM_Packet_System( _dm_work *work, _demo_packet *demo_packet );
   void DM_Packet_Effect( _dm_work *work, _demo_packet *demo_packet );

   void DM_ExecDemoStream( _dm_work *work, void *packet_stream_ptr, int exec_flag );

private:
   bool  mbInitialized;
   std::vector< int >   mEffectIds;
   STREAM_TAG           mCurrStreamTag;
};

//----------------------------------------------------------------------------
