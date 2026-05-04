//----------------------------------------------------------------------------

#include "CStreamDriver_MTA.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//----------------------------------------------------------------------------

typedef struct {
   union
   {
      struct
      {
         unsigned	noTrack_le		: 8;
         unsigned	noPacket_le	:24;
      }
      bits;
      unsigned bits_uint;
   };
   short		rateL,rateR;
   short		pcmL,oDeltaL;
   short		pcmR,oDeltaR;
} INFO_DECODE;

namespace
{
   const int skMaxTrackBuffers = 8;
   INFO_DECODE infoDecode;
}

//----------------------------------------------------------------------------

#define	MTA_FLOOR	7

enum {

   PR_DECODE				= 33
   , PR_DECODE_LOW			= 93

   , MTA_NQUEUE_COMMAND	= (1<<6)
   , MTA_NAUTOMATION		= 24

   , MTA_SPU_CORE			= 0
   , MTA_LPCM				= 256 * 2
   , MTA_NBANK_DECODEBUFF	= 4
   , MTA_SZMTABUFF			= (128-MTA_NBANK_DECODEBUFF)*0x400
   , MTA_SZMTABUFF_S		= (  8-MTA_NBANK_DECODEBUFF)*0x400
   , MTA_JOBK_MAX			= 236

   , MTA_VOL_INIT			= 127
   , MTA_NBIT_VOL			= 7

   , MTA_FADE_TIME_PAUSE	= 5
   , MTA_TIMEOUT			= 20
};

//----------------------------------------------------------------------------

#define	NTABLERATE	32
typedef struct {
   short	delta;
   short	iRate;
} TABLE_RATE_INDEX;
static const TABLE_RATE_INDEX tblRateLIndex4[] =
{
   {    1,  0 * 16 },{    5,  0 * 16 },{    9,  0 * 16 },{   13,  0 * 16 },{   16,  2 * 16 },{   20,  4 * 16 },{   24,  6 * 16 },{   28,  8 * 16 },{ -    1,  0 * 16 },{ -    5,  0 * 16 },{ -    9,  0 * 16 },{ -   13,  0 * 16 },{ -   16,  2 * 16 },{ -   20,  4 * 16 },{ -   24,  6 * 16 },{ -   28,  8 * 16 },	//  0
   {    2,  0 * 16 },{    6,  0 * 16 },{   11,  0 * 16 },{   15,  0 * 16 },{   20,  3 * 16 },{   24,  5 * 16 },{   29,  7 * 16 },{   33,  9 * 16 },{ -    2,  0 * 16 },{ -    6,  0 * 16 },{ -   11,  0 * 16 },{ -   15,  0 * 16 },{ -   20,  3 * 16 },{ -   24,  5 * 16 },{ -   29,  7 * 16 },{ -   33,  9 * 16 },	//  1
   {    2,  1 * 16 },{    7,  1 * 16 },{   13,  1 * 16 },{   18,  1 * 16 },{   23,  4 * 16 },{   28,  6 * 16 },{   34,  8 * 16 },{   39, 10 * 16 },{ -    2,  1 * 16 },{ -    7,  1 * 16 },{ -   13,  1 * 16 },{ -   18,  1 * 16 },{ -   23,  4 * 16 },{ -   28,  6 * 16 },{ -   34,  8 * 16 },{ -   39, 10 * 16 },	//  2
   {    3,  2 * 16 },{    9,  2 * 16 },{   15,  2 * 16 },{   21,  2 * 16 },{   28,  5 * 16 },{   34,  7 * 16 },{   40,  9 * 16 },{   46, 11 * 16 },{ -    3,  2 * 16 },{ -    9,  2 * 16 },{ -   15,  2 * 16 },{ -   21,  2 * 16 },{ -   28,  5 * 16 },{ -   34,  7 * 16 },{ -   40,  9 * 16 },{ -   46, 11 * 16 },	//  3
   {    3,  3 * 16 },{   11,  3 * 16 },{   18,  3 * 16 },{   26,  3 * 16 },{   33,  6 * 16 },{   41,  8 * 16 },{   48, 10 * 16 },{   56, 12 * 16 },{ -    3,  3 * 16 },{ -   11,  3 * 16 },{ -   18,  3 * 16 },{ -   26,  3 * 16 },{ -   33,  6 * 16 },{ -   41,  8 * 16 },{ -   48, 10 * 16 },{ -   56, 12 * 16 },	//  4
   {    4,  4 * 16 },{   13,  4 * 16 },{   22,  4 * 16 },{   31,  4 * 16 },{   40,  7 * 16 },{   49,  9 * 16 },{   58, 11 * 16 },{   67, 13 * 16 },{ -    4,  4 * 16 },{ -   13,  4 * 16 },{ -   22,  4 * 16 },{ -   31,  4 * 16 },{ -   40,  7 * 16 },{ -   49,  9 * 16 },{ -   58, 11 * 16 },{ -   67, 13 * 16 },	//  5
   {    5,  5 * 16 },{   16,  5 * 16 },{   26,  5 * 16 },{   37,  5 * 16 },{   48,  8 * 16 },{   59, 10 * 16 },{   69, 12 * 16 },{   80, 14 * 16 },{ -    5,  5 * 16 },{ -   16,  5 * 16 },{ -   26,  5 * 16 },{ -   37,  5 * 16 },{ -   48,  8 * 16 },{ -   59, 10 * 16 },{ -   69, 12 * 16 },{ -   80, 14 * 16 },	//  6
   {    6,  6 * 16 },{   19,  6 * 16 },{   31,  6 * 16 },{   44,  6 * 16 },{   57,  9 * 16 },{   70, 11 * 16 },{   82, 13 * 16 },{   95, 15 * 16 },{ -    6,  6 * 16 },{ -   19,  6 * 16 },{ -   31,  6 * 16 },{ -   44,  6 * 16 },{ -   57,  9 * 16 },{ -   70, 11 * 16 },{ -   82, 13 * 16 },{ -   95, 15 * 16 },	//  7
   {    7,  7 * 16 },{   22,  7 * 16 },{   38,  7 * 16 },{   53,  7 * 16 },{   68, 10 * 16 },{   83, 12 * 16 },{   99, 14 * 16 },{  114, 16 * 16 },{ -    7,  7 * 16 },{ -   22,  7 * 16 },{ -   38,  7 * 16 },{ -   53,  7 * 16 },{ -   68, 10 * 16 },{ -   83, 12 * 16 },{ -   99, 14 * 16 },{ -  114, 16 * 16 },	//  8
   {    9,  8 * 16 },{   27,  8 * 16 },{   45,  8 * 16 },{   63,  8 * 16 },{   81, 11 * 16 },{   99, 13 * 16 },{  117, 15 * 16 },{  135, 17 * 16 },{ -    9,  8 * 16 },{ -   27,  8 * 16 },{ -   45,  8 * 16 },{ -   63,  8 * 16 },{ -   81, 11 * 16 },{ -   99, 13 * 16 },{ -  117, 15 * 16 },{ -  135, 17 * 16 },	//  9
   {   10,  9 * 16 },{   32,  9 * 16 },{   53,  9 * 16 },{   75,  9 * 16 },{   96, 12 * 16 },{  118, 14 * 16 },{  139, 16 * 16 },{  161, 18 * 16 },{ -   10,  9 * 16 },{ -   32,  9 * 16 },{ -   53,  9 * 16 },{ -   75,  9 * 16 },{ -   96, 12 * 16 },{ -  118, 14 * 16 },{ -  139, 16 * 16 },{ -  161, 18 * 16 },	// 10
   {   12, 10 * 16 },{   38, 10 * 16 },{   64, 10 * 16 },{   90, 10 * 16 },{  115, 13 * 16 },{  141, 15 * 16 },{  167, 17 * 16 },{  193, 19 * 16 },{ -   12, 10 * 16 },{ -   38, 10 * 16 },{ -   64, 10 * 16 },{ -   90, 10 * 16 },{ -  115, 13 * 16 },{ -  141, 15 * 16 },{ -  167, 17 * 16 },{ -  193, 19 * 16 },	// 11
   {   15, 11 * 16 },{   45, 11 * 16 },{   76, 11 * 16 },{  106, 11 * 16 },{  137, 14 * 16 },{  167, 16 * 16 },{  198, 18 * 16 },{  228, 20 * 16 },{ -   15, 11 * 16 },{ -   45, 11 * 16 },{ -   76, 11 * 16 },{ -  106, 11 * 16 },{ -  137, 14 * 16 },{ -  167, 16 * 16 },{ -  198, 18 * 16 },{ -  228, 20 * 16 },	// 12
   {   18, 12 * 16 },{   54, 12 * 16 },{   91, 12 * 16 },{  127, 12 * 16 },{  164, 15 * 16 },{  200, 17 * 16 },{  237, 19 * 16 },{  273, 21 * 16 },{ -   18, 12 * 16 },{ -   54, 12 * 16 },{ -   91, 12 * 16 },{ -  127, 12 * 16 },{ -  164, 15 * 16 },{ -  200, 17 * 16 },{ -  237, 19 * 16 },{ -  273, 21 * 16 },	// 13
   {   21, 13 * 16 },{   65, 13 * 16 },{  108, 13 * 16 },{  152, 13 * 16 },{  195, 16 * 16 },{  239, 18 * 16 },{  282, 20 * 16 },{  326, 22 * 16 },{ -   21, 13 * 16 },{ -   65, 13 * 16 },{ -  108, 13 * 16 },{ -  152, 13 * 16 },{ -  195, 16 * 16 },{ -  239, 18 * 16 },{ -  282, 20 * 16 },{ -  326, 22 * 16 },	// 14
   {   25, 14 * 16 },{   77, 14 * 16 },{  129, 14 * 16 },{  181, 14 * 16 },{  232, 17 * 16 },{  284, 19 * 16 },{  336, 21 * 16 },{  388, 23 * 16 },{ -   25, 14 * 16 },{ -   77, 14 * 16 },{ -  129, 14 * 16 },{ -  181, 14 * 16 },{ -  232, 17 * 16 },{ -  284, 19 * 16 },{ -  336, 21 * 16 },{ -  388, 23 * 16 },	// 15
   {   30, 15 * 16 },{   92, 15 * 16 },{  153, 15 * 16 },{  215, 15 * 16 },{  276, 18 * 16 },{  338, 20 * 16 },{  399, 22 * 16 },{  461, 24 * 16 },{ -   30, 15 * 16 },{ -   92, 15 * 16 },{ -  153, 15 * 16 },{ -  215, 15 * 16 },{ -  276, 18 * 16 },{ -  338, 20 * 16 },{ -  399, 22 * 16 },{ -  461, 24 * 16 },	// 16
   {   36, 16 * 16 },{  109, 16 * 16 },{  183, 16 * 16 },{  256, 16 * 16 },{  329, 19 * 16 },{  402, 21 * 16 },{  476, 23 * 16 },{  549, 25 * 16 },{ -   36, 16 * 16 },{ -  109, 16 * 16 },{ -  183, 16 * 16 },{ -  256, 16 * 16 },{ -  329, 19 * 16 },{ -  402, 21 * 16 },{ -  476, 23 * 16 },{ -  549, 25 * 16 },	// 17
   {   43, 17 * 16 },{  130, 17 * 16 },{  218, 17 * 16 },{  305, 17 * 16 },{  392, 20 * 16 },{  479, 22 * 16 },{  567, 24 * 16 },{  654, 26 * 16 },{ -   43, 17 * 16 },{ -  130, 17 * 16 },{ -  218, 17 * 16 },{ -  305, 17 * 16 },{ -  392, 20 * 16 },{ -  479, 22 * 16 },{ -  567, 24 * 16 },{ -  654, 26 * 16 },	// 18
   {   52, 18 * 16 },{  156, 18 * 16 },{  260, 18 * 16 },{  364, 18 * 16 },{  468, 21 * 16 },{  572, 23 * 16 },{  676, 25 * 16 },{  780, 27 * 16 },{ -   52, 18 * 16 },{ -  156, 18 * 16 },{ -  260, 18 * 16 },{ -  364, 18 * 16 },{ -  468, 21 * 16 },{ -  572, 23 * 16 },{ -  676, 25 * 16 },{ -  780, 27 * 16 },	// 19
   {   62, 19 * 16 },{  186, 19 * 16 },{  310, 19 * 16 },{  434, 19 * 16 },{  558, 22 * 16 },{  682, 24 * 16 },{  806, 26 * 16 },{  930, 28 * 16 },{ -   62, 19 * 16 },{ -  186, 19 * 16 },{ -  310, 19 * 16 },{ -  434, 19 * 16 },{ -  558, 22 * 16 },{ -  682, 24 * 16 },{ -  806, 26 * 16 },{ -  930, 28 * 16 },	// 20
   {   73, 20 * 16 },{  221, 20 * 16 },{  368, 20 * 16 },{  516, 20 * 16 },{  663, 23 * 16 },{  811, 25 * 16 },{  958, 27 * 16 },{ 1106, 29 * 16 },{ -   73, 20 * 16 },{ -  221, 20 * 16 },{ -  368, 20 * 16 },{ -  516, 20 * 16 },{ -  663, 23 * 16 },{ -  811, 25 * 16 },{ -  958, 27 * 16 },{ - 1106, 29 * 16 },	// 21
   {   87, 21 * 16 },{  263, 21 * 16 },{  439, 21 * 16 },{  615, 21 * 16 },{  790, 24 * 16 },{  966, 26 * 16 },{ 1142, 28 * 16 },{ 1318, 30 * 16 },{ -   87, 21 * 16 },{ -  263, 21 * 16 },{ -  439, 21 * 16 },{ -  615, 21 * 16 },{ -  790, 24 * 16 },{ -  966, 26 * 16 },{ - 1142, 28 * 16 },{ - 1318, 30 * 16 },	// 22
   {  104, 22 * 16 },{  314, 22 * 16 },{  523, 22 * 16 },{  733, 22 * 16 },{  942, 25 * 16 },{ 1152, 27 * 16 },{ 1361, 29 * 16 },{ 1571, 31 * 16 },{ -  104, 22 * 16 },{ -  314, 22 * 16 },{ -  523, 22 * 16 },{ -  733, 22 * 16 },{ -  942, 25 * 16 },{ - 1152, 27 * 16 },{ - 1361, 29 * 16 },{ - 1571, 31 * 16 },	// 23
   {  124, 23 * 16 },{  374, 23 * 16 },{  623, 23 * 16 },{  873, 23 * 16 },{ 1122, 26 * 16 },{ 1372, 28 * 16 },{ 1621, 30 * 16 },{ 1871, 31 * 16 },{ -  124, 23 * 16 },{ -  374, 23 * 16 },{ -  623, 23 * 16 },{ -  873, 23 * 16 },{ - 1122, 26 * 16 },{ - 1372, 28 * 16 },{ - 1621, 30 * 16 },{ - 1871, 31 * 16 },	// 24
   {  148, 24 * 16 },{  445, 24 * 16 },{  743, 24 * 16 },{ 1040, 24 * 16 },{ 1337, 27 * 16 },{ 1634, 29 * 16 },{ 1932, 31 * 16 },{ 2229, 31 * 16 },{ -  148, 24 * 16 },{ -  445, 24 * 16 },{ -  743, 24 * 16 },{ - 1040, 24 * 16 },{ - 1337, 27 * 16 },{ - 1634, 29 * 16 },{ - 1932, 31 * 16 },{ - 2229, 31 * 16 },	// 25
   {  177, 25 * 16 },{  531, 25 * 16 },{  885, 25 * 16 },{ 1239, 25 * 16 },{ 1593, 28 * 16 },{ 1947, 30 * 16 },{ 2301, 31 * 16 },{ 2655, 31 * 16 },{ -  177, 25 * 16 },{ -  531, 25 * 16 },{ -  885, 25 * 16 },{ - 1239, 25 * 16 },{ - 1593, 28 * 16 },{ - 1947, 30 * 16 },{ - 2301, 31 * 16 },{ - 2655, 31 * 16 },	// 26
   {  210, 26 * 16 },{  632, 26 * 16 },{ 1053, 26 * 16 },{ 1475, 26 * 16 },{ 1896, 29 * 16 },{ 2318, 31 * 16 },{ 2739, 31 * 16 },{ 3161, 31 * 16 },{ -  210, 26 * 16 },{ -  632, 26 * 16 },{ - 1053, 26 * 16 },{ - 1475, 26 * 16 },{ - 1896, 29 * 16 },{ - 2318, 31 * 16 },{ - 2739, 31 * 16 },{ - 3161, 31 * 16 },	// 27
   {  251, 27 * 16 },{  753, 27 * 16 },{ 1255, 27 * 16 },{ 1757, 27 * 16 },{ 2260, 30 * 16 },{ 2762, 31 * 16 },{ 3264, 31 * 16 },{ 3766, 31 * 16 },{ -  251, 27 * 16 },{ -  753, 27 * 16 },{ - 1255, 27 * 16 },{ - 1757, 27 * 16 },{ - 2260, 30 * 16 },{ - 2762, 31 * 16 },{ - 3264, 31 * 16 },{ - 3766, 31 * 16 },	// 28
   {  299, 28 * 16 },{  897, 28 * 16 },{ 1495, 28 * 16 },{ 2093, 28 * 16 },{ 2692, 31 * 16 },{ 3290, 31 * 16 },{ 3888, 31 * 16 },{ 4486, 31 * 16 },{ -  299, 28 * 16 },{ -  897, 28 * 16 },{ - 1495, 28 * 16 },{ - 2093, 28 * 16 },{ - 2692, 31 * 16 },{ - 3290, 31 * 16 },{ - 3888, 31 * 16 },{ - 4486, 31 * 16 },	// 29
   {  356, 29 * 16 },{ 1068, 29 * 16 },{ 1781, 29 * 16 },{ 2493, 29 * 16 },{ 3206, 31 * 16 },{ 3918, 31 * 16 },{ 4631, 31 * 16 },{ 5343, 31 * 16 },{ -  356, 29 * 16 },{ - 1068, 29 * 16 },{ - 1781, 29 * 16 },{ - 2493, 29 * 16 },{ - 3206, 31 * 16 },{ - 3918, 31 * 16 },{ - 4631, 31 * 16 },{ - 5343, 31 * 16 },	// 30
   {  424, 30 * 16 },{ 1273, 30 * 16 },{ 2121, 30 * 16 },{ 2970, 30 * 16 },{ 3819, 31 * 16 },{ 4668, 31 * 16 },{ 5516, 31 * 16 },{ 6365, 31 * 16 },{ -  424, 30 * 16 },{ - 1273, 30 * 16 },{ - 2121, 30 * 16 },{ - 2970, 30 * 16 },{ - 3819, 31 * 16 },{ - 4668, 31 * 16 },{ - 5516, 31 * 16 },{ - 6365, 31 * 16 },	// 31
};

static void decode4bitAdpcmStereo( unsigned char **pw, short * const outPacketBuf )
{
   int volM;

   volM = 0x3fff;
   if ( volM != 0 )
   {
#define	decode4bitAdpcm( no, pcm, adp, iPcm, iRate, volM )						\
      {																				\
      int idx;																	\
      idx = iRate + ((adp >> (no * 4)) & 15);										\
      iPcm += tblRateLIndex4[idx].delta;											\
      *((short *)pcm + no) += (((iPcm * volM) >> (MTA_NBIT_VOL+MTA_FLOOR))-1);	\
      iRate = tblRateLIndex4[idx].iRate;											\
   }
#define	decode4bitAdpcm256( pw, pcm, pcmX, rateX, volM )						\
      {																				\
      int i;																		\
      int iPcm;																	\
      int iRate;																	\
      iPcm  = (pcmX);																\
      iRate = (rateX) << 4;														\
      for ( i = MTA_LPCM/2 ; i != 0 ; i -= 8 )									\
      {																			\
      register unsigned int adp;														\
      adp = (*(unsigned int *)(*pw));													\
      decode4bitAdpcm( 0, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 1, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 2, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 3, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 4, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 5, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 6, pcm, adp, iPcm, iRate, volM );						\
      decode4bitAdpcm( 7, pcm, adp, iPcm, iRate, volM );						\
      pcm += 4;																\
      *pw += 4;																\
   }																			\
   }
      INFO_DECODE id;
      unsigned int *pcm;

      id = *(INFO_DECODE *)*pw;

      *pw += sizeof( INFO_DECODE );
      pcm  = (unsigned int *)outPacketBuf;
      decode4bitAdpcm256( pw, pcm, id.pcmL, id.rateL, volM );		// left
      decode4bitAdpcm256( pw, pcm, id.pcmR, id.rateR, volM );		// right
   }
   else
   {
      *pw += sizeof( INFO_DECODE ) + 128 * 2;
   }
}

//----------------------------------------------------------------------------

CStreamDriver_MTA::CStreamDriver_MTA()
: mbInitialized( false )
{
}

void CStreamDriver_MTA::ProcessPacket( const STREAM_TAG & packet, const void * const pBody )
{
   const int bodySize = packet._size - sizeof( packet );
   if( !mbInitialized )
   {
      //Header packet.
      mFrequency = 48000;
      mSize = ( *( int * )( (char *)pBody + 4 ) ) + 8 - 0x800;
      mChannel = ( *( int * )( (char *)pBody + 64 + 4 + 4 + 4 ) );
      mDuration = (int)(( mSize / 272 * 256 ) * 300.0F / ( mFrequency * mChannel / 2 ));
      mPacketSize = ( *( int * )( (char *)pBody + 64 + 4 + 4 + 0x18 ) );

      if( bodySize != sizeof( mHeader ) )
      {
         printf("Error: unexpected MTA header size!\n");
         throw false;
      }
      memcpy( &mHeader, pBody, sizeof(mHeader) );
      const int packetCount = mSize / mPacketSize;
      if( packetCount * mPacketSize != mSize )
      {
         printf("Error: MTA stream size not a multiple of packet size???\n" );
         throw false;
      }
      const int trackBufferSize = packetCount * 0x400;

      mTrackBuffers.resize( skMaxTrackBuffers, TTrackBuffer() );
      for( unsigned int i=0; i < mTrackBuffers.size(); ++i )
      {
         mTrackBuffers[i].reserve( trackBufferSize / sizeof(short) );
      }

      //Write out an xml document with mta header information.
      {
         XmlDocument^ doc = gcnew XmlDocument();
         doc->AppendChild( doc->CreateXmlDeclaration( "1.0", nullptr, nullptr ) );
         XmlNode^ documentRoot = doc->AppendChild(doc->CreateElement("MTA"));
         ((XmlElement^)documentRoot)->SetAttribute("id", mHeader.id.ToString());
         ((XmlElement^)documentRoot)->SetAttribute("size", mHeader.size.ToString());

         //head
         {
            XmlElement^ element = doc->CreateElement("head");

            XmlElement^ subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("id")));
            subElement->InnerText = mHeader.head.id.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("size")));
            subElement->InnerText = mHeader.head.size.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("ver")));
            subElement->InnerText = mHeader.head.ver.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("ch")));
            subElement->InnerText = mHeader.head.ch.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("vol")));
            subElement->InnerText = mHeader.head.vol.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("pan")));
            subElement->InnerText = mHeader.head.pan.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("lpTop")));
            subElement->InnerText = mHeader.head.lpTop.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("lpEnd")));
            subElement->InnerText = mHeader.head.lpEnd.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("szPacket")));
            subElement->InnerText = mHeader.head.szPacket.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("lpTopPacket")));
            subElement->InnerText = mHeader.head.lpTopPacket.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("lpEndPacket")));
            subElement->InnerText = mHeader.head.lpEndPacket.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("option")));
            subElement->InnerText = mHeader.head.option.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("fLoop")));
            subElement->InnerText = mHeader.head.head_bits.fLoop.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("fFadeIn")));
            subElement->InnerText = mHeader.head.head_bits.fFadeIn.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("fPrecAtEnable")));
            subElement->InnerText = mHeader.head.head_bits.fPrecAtEnable.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("sysAT")));
            subElement->InnerText = mHeader.head.sysAT.ToString("X8");

            documentRoot->AppendChild(element);
         }
         //ch
         for( int ch=0; ch < 16; ++ch )
         {
            XmlElement^ element = doc->CreateElement("ch");
            element->SetAttribute("index", ch.ToString());

            XmlElement^ subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("id")));
            subElement->InnerText = mHeader.ch[ch].id.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("size")));
            subElement->InnerText = mHeader.ch[ch].size.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("mode")));
            subElement->InnerText = mHeader.ch[ch].mode.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("vol")));
            subElement->InnerText = mHeader.ch[ch].vol.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("volInit")));
            subElement->InnerText = mHeader.ch[ch].volInit.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("pan")));
            subElement->InnerText = mHeader.ch[ch].pan.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("fLoop")));
            subElement->InnerText = mHeader.ch[ch].ch_bits.fLoop.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("fPlii")));
            subElement->InnerText = mHeader.ch[ch].ch_bits.fPlii.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("tblAt")));
            array<Byte>^ tblAtData = gcnew array<Byte>( 64 );
            System::Runtime::InteropServices::Marshal::Copy( (IntPtr)(mHeader.ch[ch].tblAt), tblAtData, 0, 64 );
            subElement->InnerText = System::Convert::ToBase64String( tblAtData, System::Base64FormattingOptions::None );

            documentRoot->AppendChild(element);
         }
         //data
         {
            XmlElement^ element = doc->CreateElement("data");

            XmlElement^ subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("id")));
            subElement->InnerText = mHeader.data.id.ToString();

            subElement = (XmlElement^)(element->AppendChild(doc->CreateElement("size")));
            subElement->InnerText = mHeader.data.size.ToString();

            documentRoot->AppendChild(element);
         }

         char filename[FILENAME_MAX];
         sprintf( filename, "%s/%s/%s.mta.xml", gOutputFolder, gCurrStreamName, gCurrStreamName );
         doc->Save(gcnew System::String(filename));
      }

      mbInitialized = true;
   }
   else
   {
      //Stream packet.

      if( packet._option == 0 )
      {
         //Skip over weird no-packet packets
         return;
      }

      memcpy( &infoDecode, pBody, sizeof( infoDecode ) );
      const int mtaPacketCount = packet._option;
      if( mPacketSize * mtaPacketCount != bodySize )
      {
         printf("Error: MTA stream packet size too small!  Expected at least %d, got %d\n", mPacketSize * mtaPacketCount, bodySize );
         throw false;
      }

      unsigned char * pw = (unsigned char*)pBody;

      for( int i=0; i < mtaPacketCount; ++i )
      {
         for( int tr = 0; tr < skMaxTrackBuffers; ++tr )
         {
            short onePacketBuff[0x400/sizeof(short)] = { 0 };

            bool bTrackUsed = true;
            switch( mHeader.ch[tr*2].mode )
            {
            case 0x000:
               {
                  decode4bitAdpcmStereo( &pw, onePacketBuff );
               }
               break;
            case -1:
               //Nothing in this channel.
               bTrackUsed = false;
               break;
            default:
               {
                  printf("TODO: mode %d!\n", mHeader.ch[i].mode );
                  throw false;
               }
            }

            if( bTrackUsed )
            {
               TTrackBuffer & trackBuffer = mTrackBuffers[tr];
               for( int j=0; j < sizeof( onePacketBuff ) / sizeof(*onePacketBuff); ++j )
               {
                  trackBuffer.push_back( onePacketBuff[j] );
               }
            }
         }
      }

      unsigned int diff = pw - (unsigned char*)pBody;
      if( diff != bodySize )
      {
         printf("Error: packet size mismatch!\n");
         throw false;
      }
   }
}

void CStreamDriver_MTA::EndStream()
{
   if( mbInitialized )
   {
      for( unsigned int tr=0; tr < mTrackBuffers.size(); ++tr )
      {
         const TTrackBuffer & trackBuffer = mTrackBuffers[tr];

         if( !trackBuffer.size() )
         {
            //No data for this track.
            continue;
         }

         char fullpath[FILENAME_MAX];

         //Re-interleave and save out a wav file for this track.
         std::vector< short > wavBuffer( trackBuffer.size(), 0 );

         short * pDst = &wavBuffer[0];
         const short * pSrcL = &trackBuffer[0];

#if 0
         //No reason to save out the mta buffers directly; they're straight decoded stereo lpcm16 data,
         //just interleaved at 256 samples rather than 1.
         sprintf( fullpath, "%s/%s/%s.%d.mta", gOutputFolder, gCurrStreamName, gCurrStreamName, tr );
         FILE * fp = fopen( fullpath, "wb" );
         if( !fp )
         {
            printf("Error opening output file: %s\n", fullpath );
            throw false;
         }

         const short * const mtaBuffer = &trackBuffer[0];
         const int mtaBufferSize = trackBuffer.size() * sizeof(trackBuffer[0]);
         fwrite( mtaBuffer, mtaBufferSize, 1, fp );
         fclose( fp );
#endif

         const short * pSrcR = &trackBuffer[MTA_LPCM/2];
         for( unsigned int i=0; i < wavBuffer.size(); )
         {
            *pDst++ = *pSrcL++;
            *pDst++ = *pSrcR++;

            i += 2;
            if( ( i & (MTA_LPCM-1) ) == 0 )
            {
               //Next 1024byte / 512sample packet.  (we're already halfway there from incrementing during loop)
               pSrcL += MTA_LPCM/2;
               pSrcR += MTA_LPCM/2;
            }
         }

         sprintf( fullpath, "%s/%s/%s.%d.wav", gOutputFolder, gCurrStreamName, gCurrStreamName, tr );
         unsigned char * const pWavBuffer = (unsigned char * const)(&wavBuffer[0]);
         const unsigned int wavBufferSize = wavBuffer.size() * sizeof(wavBuffer[0] );
         SaveLPCM16WavFile( fullpath, 2, mFrequency, wavBufferSize, pWavBuffer );
      }
      mTrackBuffers.clear();
   }
   mbInitialized = false;
}

