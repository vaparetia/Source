/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_DXT_SPU_H__
#define __EDGE_DXT_SPU_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

void edgeDxtCompress1(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount);
void edgeDxtCompress1a(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount);
void edgeDxtCompress3(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount);
void edgeDxtCompress5(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount);

void edgeDxtDecompress1(const void* inputBlocks, void* outputPixels, uint32_t rowStride, uint32_t blockCount);
void edgeDxtDecompress3(const void* inputBlocks, void* outputPixels, uint32_t rowStride, uint32_t blockCount);
void edgeDxtDecompress5(const void* inputBlocks, void* outputPixels, uint32_t rowStride, uint32_t blockCount);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EDGE_DXT_SPU_H__

