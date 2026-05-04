//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "streamtool.h"

#include "CStreamDriver.h"

//----------------------------------------------------------------------------

typedef struct _FILE_MTA {
   int		id;
   int		size;
   int		pad[14];
   struct {
      int		id;
      int		size;
      int		ver;
      int		ch;
      int		vol;
      int		pan;
      int		lpTop;
      int		lpEnd;
      int		szPacket;
      int		lpTopPacket;
      int		lpEndPacket;
      int		option;
      union
      {
         struct
         {
            unsigned	fLoop			: 1;
            unsigned	fFadeIn			: 1;
            unsigned	fPrecAtEnable	: 1;
            unsigned	bpadA			:29;
         }
         head_bits;
         unsigned head_bits_uint;
      };
      int		sysAT;
      int		pad[32];
   } head;
   struct {
      int			id;
      int			size;
      int			mode;
      signed char	vol;
      signed char	volInit;
      short		pan;
      int			pad[7];
      union
      {
         struct
         {
            unsigned	fLoop	: 1;
            unsigned	fPlii	: 1;
            unsigned	bpadA	:30;
         }
         ch_bits;
         unsigned ch_bits_uint;
      };
      signed char tblAt[64];
   } ch[16];
   struct {
      int				id;
      int				size;
//      unsigned int   data[0];
   } data;
} FILE_MTA;

class CStreamDriver_MTA : public CStreamDriver
{
public:
   CStreamDriver_MTA();

   virtual void ProcessPacket( const STREAM_TAG & packet, const void * const pBody );
   virtual void EndStream();

private:
   bool mbInitialized;

   int mFrequency;
   int mSize;
   int mChannel;
   int mDuration;
   int mPacketSize;

   FILE_MTA mHeader;

   typedef std::vector< short > TTrackBuffer;
   std::vector< TTrackBuffer >  mTrackBuffers;
};

//----------------------------------------------------------------------------
