//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_com.c 
   フォーチュン コマンド群

   2000/03/08 T.Morita
   $Id: fort_com.c,v 1.1.1.3 2002/11/19 11:46:17 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

#include "../include/fort.h"

static Work *FRT_Work = NULL ;


/*
  Initialize Function
*/
void FRT_InitCommandWork( void *ptr )
{
    FRT_Work = (Work *)ptr ;
}

void FRT_FreeCommandWork()
{
    FRT_Work = NULL ;
}


FVECTOR  *FRT_MiscFortuneJointPos( int id )
{
    return BODYPOS( &FRT_Work->body, id );
}


FVECTOR  *FRT_MiscBulletPos()
{
    return &FRT_Work->weap_lnr ;
}



/*

  プレイヤーの位置を正確に知るための関数
  一度計算したら,その結果を使い回す。

*/
FVECTOR  *FRT_MiscPlayerPosNoHide( int joint )
{
    if ( GM_CheckPlayerStatus( PLAYER_SQUAT ) )
    {
	/*覗き込みでプレーヤーの頭が出る*/
	/*主観で構えて近くを狙う        */
	if ( (FRT_PLY_CAMERAPOS.vy - GM_PlayerControl->mov.vy > 550.0f) ||         
	     GM_CheckPlayerStatus( PLAYER_HOLD ) )
	    return &FRT_PLY_CAMERAPOS ;
	else
	    return BODYPOS( GM_PlayerBody, HUMAN21_KOSHI ) ;
    }
    if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
	return &FRT_PLY_CAMERAPOS ;

    if ( joint == FRT_GET_PLY_CONTROL )
	return &GM_PlayerControl->mov ;

    return BODYPOS( GM_PlayerBody, joint ) ;
}

FVECTOR  *FRT_MiscPlayerPos( int joint )
{
    return FRT_MiscPlayerPosNoHide( joint ) ;
}


int NewGetFortunePos( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( FRT_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->control.mov.vx ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->control.mov.vy ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->control.mov.vz ) ;

    return 1 ;
}



int NewGetFortuneBulletPos( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    FVECTOR v ;

    if ( FRT_Work == NULL )
	return 0 ;

    _sceVu0SubVector( &v, &FRT_Work->ply_shoot, &FRT_Work->control.mov ) ;
    _sceVu0Normalize( &v, &v ) ;
    _sceVu0ScaleVector( &v, &v, 300.0f ) ;
    FRT_Work->ply_shoot.vx += v.vz ;
    FRT_Work->ply_shoot.vz -= v.vx ;
    _sceVu0CopyVector( &v, &FRT_Work->ply_shoot ) ;

    if ( v.vz > -3200.0f )/* カメラメリコミ対処 */
	v.vz = -3200.0f ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)v.vx ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)v.vy ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)v.vz ) ;

    return 1 ;
}


int NewGetFortuneBombPos( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( FRT_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->ply_throw.vx ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->ply_throw.vy ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->ply_throw.vz ) ;

    return 1 ;
}


int NewGetFortuneFlag( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( FRT_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->flag ) ;

    return 1 ;
}


int NewGetFortuneActFlag( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( FRT_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->act_flg ) ;

    return 1 ;
}


int NewGetFortuneLife( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( FRT_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FRT_Work->vitality ) ;

    return 1 ;
}
