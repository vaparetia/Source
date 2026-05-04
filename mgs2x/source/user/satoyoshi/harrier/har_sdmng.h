/*
  ハリアーサウンド用ヘッダ（Ｘのみ）

  2002/05/15 M.Kobayashi
  $Id: har_sdmng.h,v 1.1.1.3 2002/11/19 11:48:24 Yoshizawa1 Exp $
 */

#ifndef __HAR_SDMNG_H__
#define __HAR_SDMNG_H__
#ifdef KP_XBOX
#include	"sd_ee.h"

#define SD_3D_SNG_TRACK void //BP

extern SD_3D_SNG_TRACK* HAR_GetSngTrackTable( void );
extern void HAR_ReleaseSngTrackTable( void );
#define TR_HARRIER_TARBIN	0x10
#define TR_HARRIER_HOBARING	0x11
#define TR_HARRIER_DOPPLER	0x12
#define TR_KASACKA_WING		0x13
#define TR_KASACKA_TARBIN	0x14
#endif
#endif //__HAR_SDMNG_H__
