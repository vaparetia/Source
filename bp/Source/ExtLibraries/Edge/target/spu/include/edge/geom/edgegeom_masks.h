/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef EDGEGEOM_MASKS_H
#define EDGEGEOM_MASKS_H

#include <spu_intrinsics.h>

/* All hard-coded shuffle masks and mask tables used by EDGE are
* defined here, to avoid duplicate definitions bloating the library's
* code footprint.
*/

#ifdef __cplusplus
extern "C" 
{
#endif

extern const vec_uchar16 s_000A000C000E000G;
extern const vec_uchar16 s_000C000000000000;
extern const vec_uchar16 s_00C000AB00BC0000;
extern const vec_uchar16 s_0A000B000C000D00;
extern const vec_uchar16 s_0A0B0c0d;
extern const vec_uchar16 s_0A0C0E0G;
extern const vec_uchar16 s_0B0D0F0H;
extern const vec_uchar16 s_0Dd0;
extern const vec_uchar16 s_A0B0C0D0;
extern const vec_uchar16 s_AAAa;
extern const vec_uchar16 s_Aa00;
extern const vec_uchar16 s_AaBb;
extern const vec_uchar16 s_AaCc;
extern const vec_uchar16 s_AACCEEGGIIKKMMOO;
extern const vec_uchar16 s_ABa0;
extern const vec_uchar16 s_ABab;
extern const vec_uchar16 s_ABCa;
extern const vec_uchar16 s_ABCD;
extern const vec_uchar16 s_ABCDEFGHd000MNOP;
extern const vec_uchar16 s_ABEFIJMNabefijmn;
extern const vec_uchar16 s_ACac;
extern const vec_uchar16 s_ACEG0000;
extern const vec_uchar16 s_AEIM000000000000;
extern const vec_uchar16 s_AIBJCKDLEMFNGOHP;
extern const vec_uchar16 s_BAba;
extern const vec_uchar16 s_BBBBFFFFJJJJNNNN;
extern const vec_uchar16 s_BCab;
extern const vec_uchar16 s_BCAD;
extern const vec_uchar16 s_bCaD;
extern const vec_uchar16 s_BCbc;
extern const vec_uchar16 s_BCDD;
extern const vec_uchar16 s_BbDd;
extern const vec_uchar16 s_Cabc;
extern const vec_uchar16 s_caBD;
extern const vec_uchar16 s_CcCc;
extern const vec_uchar16 s_Cc00;
extern const vec_uchar16 s_CD00BC00AB000000;
extern const vec_uchar16 s_CDa0;
extern const vec_uchar16 s_cdAB;
extern const vec_uchar16 s_dBC0;
extern const vec_uchar16 s_DdCc;
extern const vec_uchar16 s_Dd00;
extern const vec_uchar16 s_EAeaEAea;
extern const vec_uchar16 s_FBDbdf00;


extern const qword cullMaskTable00[16];

extern const qword cullMaskTable01[16];

extern const qword cullPreviousTable[16];

extern const qword uvMaskTable[12];

#ifdef __cplusplus
}
#endif

#endif // EDGEGEOM_MASKS_H
