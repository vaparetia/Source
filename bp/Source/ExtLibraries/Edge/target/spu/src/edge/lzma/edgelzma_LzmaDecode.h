/* 
  LzmaDecode.h
  LZMA Decoder interface

  LZMA SDK 4.40 Copyright (c) 1999-2006 Igor Pavlov (2006-05-01)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and 
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to 
  statically or dynamically link your code (or bind by name) to the 
  interfaces of this file without subjecting your linked code to the 
  terms of the CPL or GNU LGPL. Any modifications or additions 
  to this file, however, are subject to the LGPL or CPL terms.
*/

#ifndef __LZMADECODE_H
#define __LZMADECODE_H

#include "edgelzma_LzmaTypes.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

#define CProb UInt16

#define LZMA_RESULT_OK 0			// this must be 0
#define LZMA_RESULT_DATA_ERROR 1	// this must be 1

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

typedef struct
{
  int lc;
  int lp;
  int pb;
}CLzmaProperties;

int LzmaDecodeProperties(CLzmaProperties *propsRes, const unsigned char *propsData, int size);

#define LzmaGetNumProbs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

typedef struct
{
  CLzmaProperties Properties;
  UInt16 *Probs;
} CLzmaDecoderState;

int LzmaDecode_spu_asm(const CLzmaDecoderState *const vs,
    const unsigned char *const inStream, SizeT const inSize, SizeT *const inSizeProcessed,
    unsigned char *const outStream, SizeT const outSize, SizeT *const outSizeProcessed);
int LzmaDecode(const CLzmaDecoderState *const vs,
    const unsigned char *const inStream, SizeT const inSize, SizeT *const inSizeProcessed,
    unsigned char *const outStream, SizeT const outSize, SizeT *const outSizeProcessed);

#ifdef __cplusplus
} //extern "C" 
#endif // __cplusplus

#endif
