//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_spray.c
   スプレー
   
   2000/05/10 M.Sonoyama
   $Id: bul_spray.c,v 1.1.1.3 2002/11/19 11:50:01 Yoshizawa1 Exp $
   */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

typedef	struct	{
	GV_ACT_EX		actor ;
	FVECTOR			from ;
	TARGET			offense[ 2 ] ;
	POWER_TARGET	pt_offense ;
	int				map ;
	int				hit_time ;
} Work ;

/*------------------------------------------------------------*/

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work			*work ;
	GM_CameraSet	*cam ;
	FVECTOR			dc, dsize, from, to ;
	int				i ;

	work = ( Work * )ptr ;
#if 0
	if ( GV_Time == work->hit_time ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
#endif
	GM_TargetGetCenter( &dc, def ) ;
	DG_COPY_VEC( &dsize, &def->size ) ;
	if ( def->class & TARGET_ROTATE ) {
		DG_SetPos( &def->world ) ;
		DG_RotVector( &dsize, &dsize, 1 ) ;
	}

	cam = GM_GetCurrentCamera( 0 ) ;
	DG_COPY_VEC( &from, &cam->position ) ;
	
	/* 中心同士チェック */
	if ( !HZX_OnlineHazardCheck( HZX_AllMapID, &from, &dc, HZX_CHK_ALL,
								HZX_SEG_NO_BULLET | HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE, 
								HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_SPRAY | HZX_FLOOR_RECOIL_TYPE ) ) {
		work->hit_time = GV_Time ;
		return ;
	}
	/* 頂点チェック */
	for ( i = 0; i < 8; i ++ ) {
		to.vx = ( i & 1 ) ? dsize.vx : -dsize.vx ;
		to.vy = ( i & 2 ) ? dsize.vy : -dsize.vy ;
		to.vz = ( i & 4 ) ? dsize.vz : -dsize.vz ;
		_sceVu0AddVector( &to, &dc, &to ) ;
		if ( !HZX_OnlineHazardCheck( HZX_AllMapID, &from, &to, HZX_CHK_ALL,
								   HZX_SEG_NO_BULLET | HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE, 
								   HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_SPRAY | HZX_FLOOR_RECOIL_TYPE ) ) {
			/* その頂点から防御側中心が見えるか？ */
			if ( !HZX_OnlineHazardCheck( HZX_CurrentGroupID, &to, &dc, HZX_CHK_ALL,
										 HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES ) ) {
				work->hit_time = GV_Time ;
				return ;				
			}
		}
	}
	GM_TargetHitCancel( off, def ) ;
}

/*------------------------------------------------------------*/

static	void	Act( Work *work ) 
{
	GV_DestroyActor( work ) ;
}

static	void	Die( Work *work )
{
}

/*------------------------------------------------------------*/

static	int	GetResources( Work *work, FMATRIX *world, int map, int side, int weapon )
{
	TARGET			*t ;
	FVECTOR			size, ofs ;
	int damege;
	/* 近い奴 */
	t = &work->offense[ 0 ] ;
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_SPRAY_CHECK_WIDE ) ) {
		GV_SetVec3( &size, 450.0F, 625.0F, 350.0F ) ;
	} else {
		GV_SetVec3( &size, 250.0F, 625.0F, 250.0F ) ;
	}
	GV_SetVec3( &ofs, 0.0F, -375.0F, 0.0F ) ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ROTATE, map, side, &size, &ofs ) ;
	GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
	GM_SetPowerTarget( t, &work->pt_offense, POWER_CONST, 255, 0, 0, &DG_ZeroVector ) ;
	GM_MoveTarget2Map( t, world, map ) ;
	GM_PutTarget( t ) ;

	//NewTargetView2( t, 32, 232, 32 ) ;

	/* 遠い奴 */
	damege = 10 + ( GV_PadData[ 0 ].pressure[ PAD_PRESS_Y ] / 127 ); // 10段階
    damege = damege > 12 ? 12 : damege;

	t = &work->offense[ 1 ] ;
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_SPRAY_CHECK_WIDE ) ) {
		GV_SetVec3( &size, 600.0F, 1125.0F, 450.0F ) ;
	} else {
		GV_SetVec3( &size, 300.0F, 1125.0F, 300.0F ) ;
	}
	GV_SetVec3( &ofs, 0.0F, -2125.0F, 0.0F ) ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ROTATE, map, side, &size, &ofs ) ;
	GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
	GM_SetPowerTarget( t, &work->pt_offense, POWER_CONST, 255, 0, damege, &DG_ZeroVector ) ;
	GM_MoveTarget2Map( t, world, map ) ;
	GM_PutTarget( t ) ;

	//NewTargetView2( t, 32, 32, 232 ) ;

	/* 発射点を少し手前にしておく */
	GV_SetVec3( &work->from, 0.0F, 500.0F, 0.0F ) ;
	DG_SetPos( world ) ;
	DG_PutVector( &work->from, &work->from, 1 ) ;

	work->map = map ;

	return 0 ;
}

/*------------------------------------------------------------*/

void		*NewBulletSpray( FMATRIX *world, int map, int side, int weapon )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, world, map, side, weapon ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GM_SetWeaponFire( WP_ColdSpray ) ;
	}
	return work ;
}

/*----------------------------------------------------------------*/

/* スプレー当たり判定拡大 */
int		PL_SprayCheckWide( void )
{
	GM_SetPlayerStatusEX( I64(0), PLAYER2_SPRAY_CHECK_WIDE ) ;
	return 0 ;
}

/* スプレー当たり判定通常 */
int		PL_SprayCheckNormal( void )
{
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_SPRAY_CHECK_WIDE ) ;
	return 0 ;
}
