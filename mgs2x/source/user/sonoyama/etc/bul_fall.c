//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_fall.c
   途中で落ちる弾
   
   2001/05/16 M.Sonoyama
   $Id: bul_fall.c,v 1.1.1.3 2002/11/19 11:50:41 Yoshizawa1 Exp $
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

/*------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)
#define	ROTATE_STEP	(96)

#define	MAX_BOUND_COUNT	(4)

/*------------------------------------------------------------*/

typedef struct _Work {
    GV_ACT				actor ;
    CONTROL_NOEVT		control ;
    OBJECT				body ;
    FMATRIX				world ;
	FMATRIX				lights[ 2 ] ;

    int					map ;

    FVECTOR				shift ;
    FVECTOR				slope ;
    FVECTOR				step ;
    FVECTOR				pole ;

    short				rotate ;
    short				time ;
    short				bound_count ;	
    short				flag ;
    TARGET				*ride_on ;		/* ターゲット上で止まった時の移動用 */
    SVECTOR				rot ;

	float				front ;
	float				back ;

    int					noise_sw ;
    int					mode ;
    short				dir ;
    short				data ;

	SVECTOR				last_rot ;
	int					type ;
	int					die_count ;
} Work ;

/*------------------------------------------------------------*/

enum {
    GRN_MODE_NORMAL = 0,
    GRN_MODE_ON_FLOOR,
    GRN_MODE_ON_TARGET,
} ;

enum {
    GRN_FLAG_NORMAL = 0x0000,
    GRN_FLAG_BOUND  = 0x0001,
    GRN_FLAG_ON_SLOPE = 0x0002,
    GRN_FLAG_HIT_TARGET = 0x0004,
	GRN_FLAG_STOP =		0x0008,
	GRN_FLAG_HITTED =	0x0010,
	GRN_FLAG_ON_MOVEFLOOR = 0x0020,
	GRN_FLAG_NOTIPCHECK		= 0x0100,
} ;

/*------------------------------------------------------------*/

/* 移動床処理 */
static	void	ExecMoveFloorFunc( Work *work )
{
	FVECTOR		npos ;

	if ( ( work->flag & GRN_FLAG_ON_MOVEFLOOR ) && 
		 ( work->flag & GRN_FLAG_STOP ) ) {
		if ( PL_ExecMoveFloorFunc( &npos, work->control.hzx_id, &work->control.mov, 
								   NULL, 0 ) ) {
			DG_COPY_VEC( &work->control.mov, &npos ) ;
		} else {
			work->flag &= ~GRN_FLAG_ON_MOVEFLOOR ;
		}
	}
}

/* 位置・回転更新 */
static	int		UpdatePos( work )
Work		*work ;
{
	HZX_HZD		hzdf, hzdb ;
	FVECTOR		v1, mov, shift, adj ;
	int			hit_flag = 0 ;

	if ( !( work->flag & GRN_FLAG_STOP ) || ( work->flag & GRN_FLAG_ON_MOVEFLOOR ) ) {
		/* PAL用速度調整 */
		PL_PalAdjV( &work->control.step ) ;
		/* 先端処理 */
		if ( !( work->flag & GRN_FLAG_NOTIPCHECK ) &&
			work->front > 0.10F && work->back > 0.10F ) {
			FVECTOR		tip, hit, front_adj, back_adj ;
			int			online_type ;

			GV_SetVec3( &tip, 0.0F, -work->front, 0.0F ) ;
			DG_SetPos2( &work->control.mov, &work->rot ) ;
			DG_PutVector( &tip, &tip, 1 ) ;
			if ( HZX_OnlineHazardCheck( HZX_AllMapID, &work->control.mov, &tip,
									    HZX_CHK_ALL, 
									    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
									    HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
				HZX_GetOnlinePoint( &hit ) ;
				HZX_GetOnlineHazard( &hzdf, NULL ) ;
				_sceVu0SubVector( &front_adj, &hit, &tip ) ;
				online_type = HZX_GetOnlineHazardType() ;
				if ( online_type == HZX_ONLINE_TYPE_SEGMENT ) hit_flag |= 1 ;
				else										  hit_flag |= 2 ;
			}
			GV_SetVec3( &tip, 0.0F, work->back, 0.0F ) ;
			DG_SetPos2( &work->control.mov, &work->rot ) ;
			DG_PutVector( &tip, &tip, 1 ) ;
			if ( HZX_OnlineHazardCheck( HZX_AllMapID, &work->control.mov, &tip,
									    HZX_CHK_ALL, 
									    HZX_SEG_NO_MISSILE,
									    HZX_FLOOR_NO_MISSILE ) ) {
				HZX_GetOnlinePoint( &hit ) ;
				HZX_GetOnlineHazard( &hzdb, NULL ) ;
				_sceVu0SubVector( &back_adj, &hit, &tip ) ;
				online_type = HZX_GetOnlineHazardType() ;
				if ( online_type == HZX_ONLINE_TYPE_SEGMENT ) hit_flag |= 4 ;
				else										  hit_flag |= 8 ;
			}
			if ( hit_flag != 0 ) {
				if ( hit_flag & 3 ) {
					if ( hit_flag & 1 ) {
						work->control.step.vx = 0.0F ;
						work->control.step.vz = 0.0F ;
						front_adj.vy = 0.0F ;
					} else {
						work->control.step.vy *= -0.25F ;
						front_adj.vx = front_adj.vz = 0.0F ;
						//if ( front_adj.vy < 9.0F ) front_adj.vy = 9.0F ;
					}
					DG_COPY_VEC( &adj, &front_adj ) ;
					_sceVu0AddVector( &work->control.step, &work->control.step, &front_adj ) ;
				} else if ( hit_flag & 12 ) {
					if ( hit_flag & 4 ) {
						work->control.step.vx = 0.0F ;
						work->control.step.vz = 0.0F ;
						back_adj.vy = 0.0F ;
					} else {
						work->control.step.vy *= -0.25F ;
						back_adj.vx = back_adj.vz = 0.0F ;
						//if ( back_adj.vy < 9.0F ) back_adj.vy = 9.0F ;
					}
					DG_COPY_VEC( &adj, &back_adj ) ;
					_sceVu0AddVector( &work->control.step, &work->control.step, &back_adj ) ;
				}
				if ( hit_flag & ( 2 | 8 ) ) {
					if ( work->bound_count >= MAX_BOUND_COUNT ) work->control.step.vy = 0.0F ;
				}
			}
		}

		//printf( "hit_flag %x : ", hit_flag ) ;
		//DumpVec( &work->control.step ) ;
		GM_ActControl( ( CONTROL * )&work->control ) ;	

		if ( hit_flag != 0 ) {
			work->control.step.vx /= 1.50F ;
			work->control.step.vz /= 1.50F ;
		} 
		if ( ( hit_flag & ( 2 | 8 ) ) ) {
			work->rotate *= -1 ;
			work->control.step.vy -= adj.vy ;
			if ( work->control.step.vy > 0.0F &&	
				 work->control.step.vy < 9.0F ) {
				if ( ++ work->bound_count >= MAX_BOUND_COUNT ) {
					work->flag |= GRN_FLAG_NOTIPCHECK ;
				}
			}
		}
		//DumpVec( &work->control.step ) ;
		PL_PalAdjVR( &work->control.step ) ;
	}
	/* 移動床処理 */
	ExecMoveFloorFunc( work ) ;

	/* ワールドリミットで消す */
	if ( GM_CtrlWorldLimit( ( CONTROL * )&work->control ) ) return -1 ;
	/* bomblost内で消す */
	if ( !( work->flag & GRN_FLAG_STOP ) || ( work->flag & GRN_FLAG_ON_MOVEFLOOR ) ) {
		if ( HZX_CheckInsideTrap( HZX_CurrentGroupID, &work->control.mov, 
								  GM_TRP_BOMBLOST ) ) return -1 ;
	}

	GV_MatToVec( &work->body.objs->world, &v1 ) ;
	DG_SetPos2( &v1, &work->rot ) ;
	DG_GetPos( &work->body.objs->world ) ;
	DG_RotVector( &work->shift, &shift, 1 ) ;
	_sceVu0SubVector( &mov, &work->control.mov, &shift ) ;
	GV_VecToMat( &mov, &work->body.objs->world ) ;
	if ( !( work->flag & GRN_FLAG_STOP ) ) {
		DG_GetLightMatrix( &mov, work->lights ) ;
	}
	return 0 ;
}

/* 床更新 */
static	void	UpdateFloor( work )
Work		*work ;
{
	HZX_FLR	*flr ;
	FVECTOR	pole, step ;
	float	len ;
	int		rx ;

	flr = ( HZX_FLR * )work->control.level[ 0 ] ;
	pole.vx = flr->p1.h ;
	pole.vy = flr->p3.h ;
	pole.vz = flr->p2.h ;
	DG_COPY_VEC( &work->pole, &pole ) ;
	
	rx = GV_VecDir2X( &pole ) & 4095 ;
	if ( GV_DiffDirAbs( rx, 1024 ) > 128 &&
		 GV_DiffDirAbs( rx, -1024 ) > 128 ) {
		/* 傾斜床 */
		work->flag |= GRN_FLAG_ON_SLOPE ;
		DG_COPY_VEC( &step, &work->control.step ) ;
		step.vy = 0.0F ;
		len = GV_VecLen3F( &step ) ;
		if ( _sceVu0InnerProduct( &step, &pole ) < 0 ) len = 0.0F ;
		pole.vy = 0.0F ;
		GV_LenVec3F( &pole, &step, 0.0F, len + 2.0F ) ;
		work->control.step.vx = step.vx ;
		work->control.step.vz = step.vz ;
		work->control.step.vy = 0.0F ;
		DG_COPY_VEC( &work->slope, &work->control.step ) ;

		{
			FVECTOR	head, vec ;
			SVECTOR	rot ;
			float	hh, diff ;
			int		turn ;	

			rot.vx = 0 ; rot.vy = work->rot.vy ; rot.vz = 0 ;
			DG_SetPos2( &( work->control.mov ), &rot ) ;
			head.vx = head.vy = 0.0F ;
			head.vz = 250.0F ; 
			DG_PutVector( &head, &head, 1 ) ;
			HZX_SlopeFloorLevel( &hh, &head, work->control.level[ 0 ] ) ;
			diff = hh - work->control.levels[ 0 ] ;
			vec.vx = diff ; 
			vec.vz = 250.0F ;
			turn = - GV_VecDir2( &vec ) ;
			if ( turn < -2048 ) turn += 4096 ;
			work->last_rot.vx = turn - 1024 ;
		}
	} else {
		/* 平坦床 */
		work->flag &= ~GRN_FLAG_ON_SLOPE ;
		work->control.step.vy = 0.0F ;
		DG_COPY_VEC( &work->slope, &DG_ZeroVector ) ;
		work->last_rot.vx = -1024 ;
	}
}

/* バウンドチェック */
static	void	CheckBoundHazard( work )
Work		*work ;
{
	int		type ;
	HZX_SEG	*seg ;
	HZX_FLR	*flr ;
	FVECTOR	pole, ptp ;

	if ( work->control.n_touches != 0 ) {
		type = HZX_GetOnlineHazardType() ;
		HZX_GetOnlinePoint( &ptp ) ;
		if ( type == 1 ) {
			/* 壁 */
			seg = ( HZX_SEG * )work->control.level[ 0 ] ;
			pole.vx = seg->p2.z - seg->p1.z ;
			pole.vy = 0.0F ;
			pole.vz = seg->p1.x - seg->p2.x ;
			DG_COPY_VEC( &work->pole, &pole ) ;
			work->flag |= GRN_FLAG_BOUND ;
		} else {
			/* 床 */
			work->flag |= GRN_FLAG_BOUND ;
			if ( ptp.vy < work->control.mov.vy ) { /* 床バウンド */
				++ work->bound_count ;
				if ( work->bound_count >= MAX_BOUND_COUNT ) {
					/* 床上状態へ */
					work->mode = GRN_MODE_ON_FLOOR ;
					work->data = 0 ;
					/* ニアチェック、床チェック開始 */
					if ( work->type & FALLBUL_TYPE_NONEARCHECK ) {
						work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
					} else {
						work->control.skip_flag 
							&= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ) ;
					}
					UpdateFloor( work ) ;
					return ;
				}
			}
			flr = ( HZX_FLR * )work->control.level[ 0 ] ;
			pole.vx = flr->p1.h ;
			pole.vy = flr->p3.h ;
			pole.vz = flr->p2.h ;
			DG_COPY_VEC( &work->pole, &pole ) ;
		}	
	}
}

/*------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	FVECTOR	step, from, v1 ;
	float	len ;	

	/* 全モード共通 */
	GM_SetCurrentMap( GM_StageMapAll ) ; /* 要チェック */

	if ( work->time < 0 ) ++ work->time ; 

	switch( work->mode ) {
	case GRN_MODE_NORMAL :		/* 通常状態 */	
		/* バウンド処理 */
		if ( work->flag & GRN_FLAG_BOUND ) {
			DG_ReflectVector( &work->pole, &work->step, &step ) ;
			step.vy /= 2.0F ;
			step.vx /= 3.0F ; step.vz /= 3.0F ;
			work->control.step = step ;
			work->rotate *= -1 ;
			work->flag &= ~GRN_FLAG_BOUND ;
		}
		work->step = work->control.step ;
		from = work->control.mov ;
		work->rot.vx += work->rotate ;
		work->rot.vy += work->rotate ;
        if ( UpdatePos( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return ;
		}
		CheckBoundHazard( work ) ;

		work->control.step.vy -= 9.0F ;
		break ;
	case GRN_MODE_ON_FLOOR :	/* 床上 */
		from = work->control.mov ;
		/* 床上終了フラグ */
		if ( ( work->type & FALLBUL_TYPE_ENDDESTROY ) &&
			 ( ++ work->die_count >= 300 / TIME_BASE * 2 ) ) {	/* ２秒 */
			GV_DestroyActor( work ) ;
			return ;
		}
        if ( UpdatePos( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return ;
		}
		if ( !( work->flag & GRN_FLAG_STOP ) &&
			 ( work->control.grounded & 1 ) ) UpdateFloor( work ) ;
		_sceVu0SubVector( &v1, &work->control.mov, &from ) ;
		if ( work->flag & GRN_FLAG_ON_SLOPE ) {
			//_sceVu0AddVector( &work->control.step, &work->control.step, &work->slope ) ;
		} else {
			work->control.step.vx /= 1.50F ;
			work->control.step.vz /= 1.50F ;
		}
		if ( DG_FABS( v1.vx ) > 0.10F || DG_FABS( v1.vz ) > 0.10F ) {
			work->dir = GV_VecDir2( &v1 ) + 1024 ;
		}
		if ( !( work->type & FALLBUL_TYPE_RANDOM ) ) {
			work->rot.vx = GV_NearExp4P( work->rot.vx & 4095, work->last_rot.vx ) ;
			//work->rot.vy = GV_NearExp4P( work->rot.vy & 4095, work->last_rot.vy ) ;
		}
		v1.vx = work->control.step.vx ;
		v1.vy = 0.0F ;
		v1.vz = work->control.step.vz ;
		len = GV_VecLen3F( &v1 ) ;
		//work->rot.vz += len * 4096.0F / 240.0F ;

		if ( work->flag & GRN_FLAG_STOP ) break ;

		if ( work->control.grounded & 1 ) {
			work->control.step.vy = 0.0F ;
			work->data = 0 ;
		} else if ( ++ work->data >= 16 ) {
			/* 通常状態へ */
			work->mode = GRN_MODE_NORMAL ;
			work->bound_count = 0 ;
			work->flag &= ~GRN_FLAG_ON_SLOPE ;
			work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
		}
		if ( work->control.height < 8.0F ) {
			work->control.step.vy -= 4.0F ;
		} else {
			work->control.step.vy -= work->control.height - 8.0F ;
		}
		if ( DG_FABS( work->control.step.vx ) < 1.0F &&
			 DG_FABS( work->control.step.vz ) < 1.0F ) {
			DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;
			work->flag |= GRN_FLAG_STOP | GRN_FLAG_ON_MOVEFLOOR ; /* 移動床上フラグも立てておく */
		} else if ( work->control.n_touches ) {
			DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;
			work->flag |= GRN_FLAG_STOP | GRN_FLAG_ON_MOVEFLOOR ; /* 移動床上フラグも立てておく */
		}
		break ;
	case GRN_MODE_ON_TARGET :	/* ターゲット上 */
	  ;
	}
}

static	void	Die( work )
Work		*work ;
{
	GM_FreeControl( ( CONTROL * )&work->control ) ;
	GM_FreeObject( &work->body ) ;
}

/*------------------------------------------------------------*/

static	int	InitControl( Work *work, FMATRIX *world, FVECTOR *force, int model, int height )
{
	CONTROL	*ctrl ;
	FVECTOR	mov, shift ;

	ctrl = ( CONTROL * )&( work->control ) ;
	/* モデル名を名前にする */
	if ( model == MDL_CHAFF_PIN ||
		 model == MDL_CHAFF_LEVER ||
		 model == MDL_STUN_PIN ||
		 model == MDL_STUN_LEVER ||
		 model == MDL_GRENADE_PIN ||
		 model == MDL_GRENADE_LEVER ||
		 model == MDL_CHAFF_PIN_RAI ||
		 model == MDL_CHAFF_LEVER_RAI ||
		 model == MDL_STUN_PIN_RAI ||
		 model == MDL_STUN_LEVER_RAI ||
		 model == MDL_GRENADE_PIN_RAI ||
		 model == MDL_GRENADE_LEVER_RAI ) {
		if ( GM_InitControl( ctrl, 0, GM_StageMapAll ) < 0 ) return -1 ;
	} else {
		if ( GM_InitControl( ctrl, model, GM_StageMapAll ) < 0 ) return -1 ;
	}
	GV_MatToVec( world, &mov ) ;
	DG_SetPos( world ) ;
	DG_RotVector( &work->shift, &shift, 1 ) ;
	DG_COPY_VEC( &ctrl->step, force ) ;
	ctrl->step.vx /= 4.0F ;
	ctrl->step.vz /= 4.0F ;

	_sceVu0AddVector( &ctrl->mov, &mov, &shift ) ;

	GM_ConfigControlHazard( ctrl, height, 80, 80 ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	/* onlineチェックのみ行う */
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	return 0 ;
}

static	int	GetResources( Work *work, FMATRIX *world, FVECTOR *force, int model, 
						  int type, int height ) 
{
	if ( InitControl( work, world, force, model, height ) < 0 ) return -1 ;
	GM_InitObject( &( work->body ), model, BODY_FLAG ) ;
	if ( work->body.objs == NULL ) return -1 ;
	GM_ConfigControlObject( ( CONTROL * )&work->control, &work->body ) ;
	if ( type & FALLBUL_TYPE_CHANL0ONLY ) {
		DG_InvisibleObjs( work->body.objs ) ;
		DG_VisibleObjsChanl( work->body.objs, 0 ) ;
	}
	GM_ConfigObjectLight( &work->body, work->lights ) ;
	DG_GetLightMatrix( &work->control.mov, work->lights ) ;
	work->body.map_name = GM_StageMapAll ;
	DG_COPY_MAT( &work->body.objs->world, world ) ;
	GM_GroupObjs( work->body.objs, GM_StageMapAll ) ;
	work->ride_on = NULL ;
	work->mode = GRN_MODE_NORMAL ;

	work->rotate = 12 ;
	{
		FVECTOR		dir ;
		if ( !( type & FALLBUL_TYPE_YDOWN ) ) {
			GV_SetVec3( &dir, 0.0F, 0.0F, 100.0F ) ;
		} else {
			GV_SetVec3( &dir, 0.0F, -100.0F, 0.0F ) ;
		}
		DG_SetPos( world ) ;
		DG_RotVector( &dir, &dir, 1 ) ;
		GV_VecToRot( &dir, &work->rot ) ;
		if ( !( type & FALLBUL_TYPE_ENDXROTZERO ) ) {
			work->rot.vx -= 1024 ;	
			work->last_rot.vx = -1024 ;
		}
		work->last_rot.vy = work->rot.vy ;
	}

	return 0 ;
}

/* 起動 */
void	*NewFallingBulletEX( FMATRIX *world, FVECTOR *shift, FVECTOR *force, int model, float front, 
						   float back, int type, int height )
{
	Work			*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( shift != NULL ) DG_COPY_VEC( &work->shift, shift ) ;
		if ( GetResources( work, world, force, model, type, height ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->map = GM_StageMapAll ;
		work->world = *world ;
		work->front = front ;
		work->back = back ;
		work->type = type ;
	}
	return work ;
}

void	*NewFallingBullet( FMATRIX *world, FVECTOR *shift, FVECTOR *force, int model, float front, 
						   float back, int type )
{
	return NewFallingBulletEX( world, shift, force, model, front, back, type, 40 ) ;
}

/*----------------------------------------------------------------*/

int		PL_COM_FallBulletSystemOn( void )
{
	PL_FallBulletFunc = NewFallingBullet ;
	return 0 ;
}
