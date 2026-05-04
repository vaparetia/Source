//----------------------------------------------------------------------------
// BP_MD5.h
//
// MD5 hash generation
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// hashDst should be 16 bytes long
void BP_CreateMD5Hash( unsigned char * const hashDst, const void * data, unsigned int const dataSize );
int BP_CompareMD5Hashes( unsigned char * const pA, unsigned char * const pB );
int BP_IsMD5HashZero( const unsigned char * const hashPtr );

#ifdef __cplusplus
};
#endif