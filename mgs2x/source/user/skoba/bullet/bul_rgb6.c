//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_rgb6.c
   ＲＧＢ６弾
   
   2000/02/29 M.Sonoyama ４００年に一度の閏日
   $Id: bul_rgb6.c,v 1.1.1.3 2002/11/19 11:50:01 Yoshizawa1 Exp $
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


/*------------------------------------------------------------*/

extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;

#define	BODY_NAME	MDL_RGB_BUL
#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)

#define	SHOT_SPEED	(250.0F) 

//static FVECTOR	Shift = { 29.5F, -110.5F, 20.5F } ;
static FVECTOR	Shift = { 0.0F, 0.0F, 0.0F } ;

typedef	struct _Work {
    GV_ACT		actor ;
    CONTROL		control ;
    OBJECT		body ;
    FMATRIX		lights[ 2 ] ;
    FVECTOR		shift ;
    TARGET		off ;
	GM_BOMB		list ;
    int			flag ;
    int			count ;
	int			nNo ;
} Work ;

/*--------------------------------------------------------------------*/

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    FVECTOR	mov ;

	{
		FVECTOR		dc ;

		GM_TargetGetCenter( &dc, def ) ;
		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &off->hit, &dc, 
								    HZX_CHK_ALL,
								    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
								    HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
			GM_TargetHitCancel( off, def ) ;
			return ;
		}
	}

    work = ( Work * )ptr ;
    work->flag |= 1 ;
    DG_COPY_VEC( &work->control.mov, &off->hit ) ;
    _sceVu0SubVector( &mov, &off->hit, &work->shift ) ;
    GV_VecToMat( &mov, &work->body.objs->world ) ;
}

/*--------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    FVECTOR	mov, from, diff ;

	if ( work->list.flag & GM_BMB_FLAG_DESTROY ) {
		GV_DestroyActor( work ) ;
		return ;
	}

	GM_SetCurrentMap( GM_StageMapAll ) ;

	/* 時間ぎれ */
	if ( -- work->count <= 0 ) work->flag |= 1 ;

    if ( work->flag & 1 ) {
		if ( work->control.mov.vy < GM_WaterLevel && 
			PL_BlastWaterFunc != NULL ) {
			( *PL_BlastWaterFunc )( &work->control.mov, BOTH_SIDE, 
								   1000, 2000, DMG_BLAST, FNT_BLAST, WP_Rgb6, 0 ) ;		
		} else {
			/* グレネードより狭い */
			NewBlast( &work->control.mov, BOTH_SIDE, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Rgb6 ) ;
		}
		GV_DestroyActor( work ) ;
		return ;
    }

	if ( work->list.flag & GM_BMB_FLAG_NOBLAST ) {
		if ( PL_FallBulletFunc != NULL ) {
			PL_FallBulletFunc( &work->body.objs->world, &Shift, &work->control.step, 
							   BODY_NAME, 50.0F, 50.0F, FALLBUL_TYPE_RANDOM | FALLBUL_TYPE_YDOWN ) ;
			GV_DestroyActor( work ) ;
			return ;
		}
	}

    DG_COPY_VEC( &from, &work->control.mov ) ;
    GM_ActControl( &work->control ) ;
    if ( work->body.map_name != GM_CurrentMap ) {
		work->body.map_name = GM_CurrentMap ;
		GM_GroupObjs( work->body.objs, GM_CurrentMap ) ;
    }

    DG_GetLightMatrix( &work->control.mov, work->lights ) ;
    _sceVu0SubVector( &mov, &work->control.mov, &work->shift ) ;
    GV_VecToMat( &mov, &work->body.objs->world ) ;

	_sceVu0SubVector( &diff, &work->control.mov, &from ) ;
	if ( !( work->flag & 2 ) &&
		PL_CheckBulletSplash( &mov, &diff, 75.0F, 125.0F ) ) {
		work->flag |= 2 ;
	}

#if 0
    if ( work->control.n_touches > 0 ||
		 PL_GetPlayerWeapon() != WP_Rgb6 ) {
		work->flag |= 1 ;
    }
#endif
	/* バーストは廃止 */
    if ( work->control.n_touches > 0 ) {
		work->flag |= 1 ;
    }

    GM_MoveOnlineTargetMap( &work->off, &from, &work->control.mov, GM_StageMapAll ) ;
    GM_PutTarget( &work->off ) ;
    work->control.step.vy -= 4.0F ;
	if ( work->control.mov.vy < GM_WaterLevel ) {
		work->control.step.vx = GV_NearExp2F( work->control.step.vx, 0.0F ) ;
		work->control.step.vz = GV_NearExp2F( work->control.step.vz, 0.0F ) ;
		work->control.step.vy = -8.0F ;
	}
	DG_COPY_VEC( &GM_RGB6Position[ work->nNo ], &work->control.mov ) ;
}

static	void	Die( work ) 
Work		*work ;
{
    GM_FreeControl( &work->control ) ;
    GM_FreeObject( &work->body ) ;
	GM_RemoveBombList( &work->list ) ;

	GM_N_RGB6S -- ;
	if ( GM_N_RGB6S == 0 ) GM_WeaponAlive &= ~WP_ALIVE_RGB6 ;
}

/*--------------------------------------------------------------------*/

static	int	InitControl( work, world )
Work		*work ;
FMATRIX		*world ;
{
    CONTROL	*ctrl ;
    FVECTOR	step, mov ;

    GV_MatToVec( world, &mov ) ;
    ctrl = &( work->control ) ;
    if ( GM_InitControl( ctrl, WP_Rgb6, GM_CurrentMap ) < 0 ) return -1 ;
    GM_ConfigControlPosition( ctrl, &mov, &DG_ZeroSVector ) ;
	ctrl->map = GM_StageMapAll ;
	ctrl->hzx_id = HZX_AllMapID ;
    //GM_ConfigControlMapCheck( ctrl ) ;
    GM_ConfigControlHazard( ctrl, 200, 250, 250 ) ;
    ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
    ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
    ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
    
    //step.vx = step.vy = 0.0F ;
    //step.vz = SHOT_SPEED ;
    step.vx = step.vz = 0.0F ;
    step.vy = -SHOT_SPEED ;
    DG_SetPos( world ) ;
    DG_RotVector( &step, &ctrl->step, 1 ) ;
    return 0 ;
}

static	int	InitObject( work, world, mov )
Work		*work ;
FMATRIX		*world ;
FVECTOR		*mov ;
{
    OBJECT	*body ;

    body = &work->body ;
    GM_InitObject( body, BODY_NAME, BODY_FLAG ) ;
    if ( body->objs == NULL ) return -1 ;
    body->map_name = GM_CurrentStageMap ;
    GM_GroupObjs( body->objs, GM_CurrentStageMap ) ;
    DG_COPY_MAT( &body->objs->world, world ) ;
    GV_VecToMat( mov, &body->objs->world ) ;
    GM_ConfigObjectLight( body, work->lights ) ;
    return 0 ;
}

static	void	SetTarget( work, name )
Work		*work ;
int		name ;
{
    TARGET	*t ;

    t = &work->off ;
    GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN |
				 TARGET_POWER | TARGET_THROUGH, GM_CurrentStageMap, ENEMY_SIDE,
				 &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_WEAPONCORE ) ;
    GM_SetTargetName( t, WP_Rgb6 ) ;
    GM_SetTargetCallBack( t, Hit, work ) ;
}

static	int	GetResources( work, world, ctrl, side, chanl )
Work		*work ;
FMATRIX		*world ;
CONTROL		*ctrl ;
int		side, chanl ;
{
    FVECTOR	from, to, hit ;

    /* まずオンラインチェックで
       キャラと発射点の間に壁があるかチェック */
    GV_MatToVec( world, &to ) ;
    DG_COPY_VEC( &from, &ctrl->mov ) ; from.vy = to.vy ;
    if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &from, &to,
							   HZX_CHK_ALL, HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
							   HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
		HZX_GetOnlinePoint( &hit ) ;
		NewBlast( &hit, BOTH_SIDE, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Rgb6 ) ;

		/* 発砲数 */
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
		GM_DecrementWeapon( WP_Rgb6, 1 ) ;
		NewPadVibration2( GV_StrCode( "rai_rgb_fire" ), 0 ) ;
		PL_DecrementMagazine() ;
		GM_SetWeaponFire( WP_Rgb6 ) ;

		return -1 ;
    }

    DG_SetPos( world ) ;
    DG_RotVector( &Shift, &work->shift, 1 ) ;
    _sceVu0SubVector( &to, &to, &work->shift ) ;

    if ( InitControl( work, world ) < 0 ) return -1 ;
    if ( InitObject( work, world, &to ) < 0 ) return -1 ;
    SetTarget( work, ctrl->name ) ;

	/* ボムリストに乗っける */
	GM_InitBombList( &work->list, WP_Rgb6, &work->control.mov, &work->control.step ) ;
	GM_AddBombList( &work->list ) ;

	GM_WeaponAlive |= WP_ALIVE_RGB6 ;
	work->nNo = GM_N_RGB6S ;
	GM_N_RGB6S++ ;

	DG_COPY_VEC( &GM_RGB6Position[ work->nNo ], &work->control.mov ) ;

	work->count = 5 * ( 300 / TIME_BASE ) ;

    return 0 ;
}

/* 起動 */
void	*NewBulletRGB6( world, ctrl, side, chanl )
FMATRIX		*world ;
CONTROL		*ctrl ;
int		side, chanl ;
{
    Work	*work ;

	if ( GM_N_RGB6S >= GM_CURRENT_RGB6_MAX ) return NULL ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, ctrl, side, chanl ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		/* 発砲数 */
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
		GM_DecrementWeapon( WP_Rgb6, 1 ) ;
		NewPadVibration2( GV_StrCode( "rai_rgb_fire" ), 0 ) ;
		PL_DecrementMagazine() ;
		GM_SetWeaponFire( WP_Rgb6 ) ;
    }
    return work ;
}
