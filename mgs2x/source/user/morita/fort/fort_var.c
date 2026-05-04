//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"

#include "morita/brk_utl/brk_utl.x"


/* efct/efct_flow.c でも使われる weapon/fort_laser.c */
int     FRT_SmokeClouded = 0 ;
int     FRT_BulletEnable = 0 ;

/* 今,消える火が燃えているか */
int     FRT_GasFire = 0 ;

/* 今,爆発しているか？ */
int     FRT_GasExplode = 0 ;


/* 天井に撃った回数 */
int     FRT_LastCieling = 0 ;


/* weapon/lnr.c など,リニアガンの先のベクトル */
FVECTOR FRT_LNR_NozzleOfst = { -24.0f, -1129.501831f, 75.0f, 1.0f } ;

 /* リニアガンが撃った時に暗くするためにフォグ値を取っておくためのもの */
CVECTOR FRT_FogColor ;
float   FRT_FogNear  ;
float   FRT_FogFar   ;
