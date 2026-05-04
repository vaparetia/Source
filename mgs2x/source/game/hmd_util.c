//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   hmd_util.c
   HMDカメラシステムユーティリティ
   
   2002/05/13 T.Morita
   $Id: hmd_util.c,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"usbhmd.h"
#include	"util.h"
#include	"camera.h"


/* HMD オフセット回転 限界値 */
#define  YAW_LIMIT 24000


static short GM_HMD_Pitch ;
static short GM_HMD_Yaw   ;
/*---------------------------------------------------------------------------*/

void GM_ResetHMD()
{
    GM_HMD_Pitch = 0 ;
    GM_HMD_Yaw   = 0 ;
}


/* ヘッドマウントディスプレイ用のマトリック生成関数 */
void GM_SetHMDCamera( DG_CHANL *chanl, FVECTOR *pos, FVECTOR *trg, float angle )
{
#ifdef PSX2
    FMATRIX	mat, inv ;
    FMATRIX m ;
    SVECTOR rot;
    USBHMD_DATA hmd;

    usbhmd_read( &hmd );
    //printf( "%08d y%08d\n", hmd.pitch, -hmd.yaw ) ;
    rot.vx = hmd.pitch ;
    rot.vy = (-hmd.yaw<-YAW_LIMIT ?-YAW_LIMIT :
	      -hmd.yaw> YAW_LIMIT ? YAW_LIMIT :
	                           -hmd.yaw   ) ;
    rot.vx = 1.5f * rot.vx * 2048.0F / 0x8000;
    rot.vy = 1.7f * rot.vy * 2048.0F / 0x8000;
    rot.vz = 0.0F;
    DG_SetPos2( &DG_ZeroVector, &rot );
    DG_GetPos( &m );
	
    DG_MakeCameraMatrix( &mat, pos, trg );
    _sceVu0MulMatrix( &mat, &mat, &m );

    pos->vw = pos->vw + hmd.roll * M_PI / 0x8000;
    //pos->vw = 0.0F;
	
    /* カメラ逆行列を計算する  */
    _sceVu0InversMatrix( &inv, &mat );
	
    /* Ｚ回転 */
    _sceVu0RotMatrixZ( &inv, &inv, pos->vw ) ;
    _sceVu0InversMatrix( &mat, &inv ) ;
    /* カメラ設定 */
    BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
    DG_SetCamera( chanl, &mat, angle );
#endif
}

