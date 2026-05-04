//----------------------------------------------------------------------------
// BP_MD5.cpp
//
// MD5 hash generation
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "MGS_SysCommon.h"
#include "BP_MD5.h"

#define ARCH_IS_BIG_ENDIAN BPE_IS_ENDIAN_BIG()

extern "C" 
{
#include "md5.h"
}

extern "C" void BP_CreateMD5Hash( unsigned char * const hashDst, const void * data, unsigned int const dataSize )
{
   md5_state_s state;
   md5_init(&state);

   md5_append(&state, (const md5_byte_t*)data, dataSize);

   md5_finish(&state, (md5_byte_t*)hashDst);
}

extern "C" int BP_CompareMD5Hashes( unsigned char * const pA, unsigned char * const pB )
{
   return !memcmp( pA, pB, 16 );
}

extern "C" int BP_IsMD5HashZero( const unsigned char * const hashPtr )
{
   unsigned char zeroMem[16];
   memset( zeroMem, 0, sizeof(zeroMem) );

   return !memcmp( hashPtr, zeroMem, 16 );
}