/*  SCE CONFIDENTIAL
 *  PLAYSTATION(R)3 Programmer Tool Runtime Library 240.001
 *  Copyright (C) 2008 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#ifndef __GTFCONV_CONVERT_H__
#define __GTFCONV_CONVERT_H__

#include "multi_platform.h"
#include "utils.h"

namespace gtfconv{

bool convertLayOutBuffer(layout_t *playout, CellGcmTexture tex, CellUtilDDSHeader ddsh);
bool convertBufferByLayout(uint8_t *gtf_image, uint8_t *dds_image, layout_t *playout, uint32_t layout_num, CellGcmTexture tex, bool bDds2Gtf);

} // namespace gtfconv

#endif // __GTFCONV_CONVERT_H__
