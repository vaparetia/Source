//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_gre.c
   投げられたグレネード系
   
   2000/02/08 M.Sonoyama
   $Id: bul_gre.c,v 1.1.1.3 2002/11/19 11:50:00 Yoshizawa1 Exp $
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

#include "BP_Debug.h"//BP_INPUT


/*------------------------------------------------------------*/

extern	void	*NewBlast3( FVECTOR *, int, int, int, int, int, int, int ) ;
extern	void	*NewBlastChaff( FMATRIX * ) ;
extern	void	*NewBlastStun( FMATRIX * ) ;

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)
#define	ROTATE_STEP	(96)

#define	MAX_BOUND_COUNT	(4)

static FVECTOR	ShiftGrenade = { 29.5F, -110.5F, 20.5F } ;
//static FVECTOR	ShiftMagazine = { 17.5F, -115.9F, -1.2F } ;
static FVECTOR	ShiftMagazine = { 0.0F, 0.0F, 0.0F } ;
static FVECTOR	ShiftChaff = { 29.5F, -108.5F, 5.0F } ;
#ifdef DEBUG_MODE
//#define	NO_BLAST
#endif

/*------------------------------------------------------------*/

typedef struct _Work {
    GV_ACT				actor ;
    CONTROL_NOEVT		control ;
    OBJECT				body ;
    FMATRIX				world ;
    TARGET				target ;
	TARGET				defense ;

    int					side ;
    int					weapon ;
    int					map ;
    int					count ;

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

    int					noise_sw ;
    int					mode ;
    short				dir ;
    short				data ;

	GM_BOMB				list ;
	HZX_D_CALLBACK		dhcb ;
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
	GRN_FLAG_SPLASH =		0x0040,
	GRN_FLAG_NOISED =		0x0080,
} ;

/*------------------------------------------------------------*/

/* グローバルリストに登録 */
static	void	AddList( Work *work, int weapon )
{
	GM_BOMB	*this ;

	this = &work->list ;
	this->weapon = weapon ;
	this->mov = &work->control.mov ;
	this->step = &work->control.step ;

	GM_AddBombList( this ) ;
	GM_N_Grenades ++ ;
}

/* グローバルリストから削除 */
static	void	RemoveList( Work *work ) 
{
	GM_RemoveBombList( &work->list ) ;
}


/*------------------------------------------------------------*/

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    //FVECTOR	step, v1, diff ;
    //float	ip, len ;
	//long64	wp ;

    work = ( Work * )ptr ;

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

    /* 連続で同じターゲットには当たらない */
#if 0
    if ( def == work->ride_on ) 
#else
	if ( work->flag & GRN_FLAG_HIT_TARGET ) 
#endif
	{
		def->damaged &= ~( off->class & TARGET_CHECK_CLASS ) ;
		def->weapon_type &= ~off->weapon_type ;
		return ;
	}

	if ( work->noise_sw == 0 ) work->noise_sw = 1 ;
	//DG_COPY_VEC( &work->pole, &off->center ) ;
	DG_COPY_VEC( &work->pole, &off->size ) ;
	work->flag |= GRN_FLAG_BOUND ;
	work->ride_on = def ;

#if 1 //korekore	
	if ( GM_WaterLevel < off->hit.vy ) {
		GM_SeSetMode( SD_A_MAGAZI03, &off->hit, GM_SEMODE_BOMB ) ;
	}
#endif
#if 1
	work->flag |= GRN_FLAG_HIT_TARGET ;
#endif
#if 0
	if ( work->ride_on != NULL ) {
		/* 法線情報とスロープステップを再計算 */
		step = DG_ZeroVector ; step.vy = -2.0F ;
		ip = _sceVu0InnerProduct( &step, &off->center ) ;
		if ( ip <= 0 ) return ;
		_sceVu0ScaleVector( &v1, &off->center, ip ) ;
		_sceVu0SubVector( &work->slope, &step, &v1 ) ;
		return ;
	}

	GV_VecToMat( &off->hit, &work->body.objs->world ) ;
	_sceVu0SubVector( &diff, &off->hit, &off->offset ) ;
	len = GV_VecLen3F( &diff ) ;
	if ( len == 0.0F ) {
		printf( "zero len\n" ) ;
		return ;
	}

	GV_LenVec3F( &diff, &diff, len, len - 1.0F ) ;
	_sceVu0AddVector( &work->control.mov, &off->offset, &diff ) ;
	DG_COPY_VEC( &work->pole, &off->center ) ;
	work->flag = 1 ;
	if ( work->pole.vy != 0.0F ) {
		if ( ++ work->bound_count >= MAX_BOUND_COUNT ) {
			step = DG_ZeroVector ; step.vy = -2.0F ;
			//	    _sceVu0CopyMatrix( &work->body.objs->world, &work->world ) ;
			GV_VecToMat( &off->hit, &work->body.objs->world ) ;
			work->flag = 0 ;
			ip = _sceVu0InnerProduct( &step, &off->center ) ;
			work->bound_count = 0 ;
			if ( ip <= 0 ) {
				work->control.step = step ;
				return ;
			} 
			work->ride_on = def ;
			_sceVu0ScaleVector( &v1, &off->center, ip ) ;
			_sceVu0SubVector( &work->control.step, &step, &v1 ) ;
			DG_COPY_VEC( &work->slope, &work->control.step ) ;
		}
	}
#endif
}

/*------------------------------------------------------------*/

static	void	Hitted( off, def, ptr )
TARGET			*off, *def ;
void			*ptr ;
{
    Work			*work ;
	FVECTOR			force ;
	POWER_TARGET	dummy ;
	long64			wp ;

	work = ( Work * )ptr ;

	wp = off->weapon_type ;
	if ( wp & WP_BLAST ) {
		/* 爆破ユウバク */
		if ( work->count > 8  ) work->count = 8 ;
		return ;
	}
	if ( !( wp & ( WP_M92 | WP_BULLET | WP_BLAST ) ) ) {
		def->damaged &= ~( off->class & TARGET_CHECK_CLASS ) ;
		def->weapon_type &= ~off->weapon_type ;
		return ;
	}

	dummy.force = DG_ZeroVector ;
	def->power = &dummy ;

	GM_MakeTargetForce( off, def ) ;
	DG_COPY_VEC( &force, &dummy.force ) ;
	if ( force.vy < 0.0F ) force.vy = 0 - force.vy ;
	else if ( DG_FABS( force.vy ) < 0.10F ) force.vy = 32.0F ;
	if ( off->power != NULL && 
		 ( off->power->type & POWER_EXPLODE ) ) {
		_sceVu0ScaleVector( &force, &force, 8.0F ) ;
	}
	if ( force.vx < -128.0F ) force.vx = -128.0F ;
	if ( force.vz < -128.0F ) force.vz = -128.0F ;
	if ( force.vx > 128.0F ) force.vx = 128.0F ;
	if ( force.vz > 128.0F ) force.vz = 128.0F ;
	if ( force.vy > 128.0F ) force.vy = 128.0F ;

	_sceVu0AddVector( &work->control.step, &work->control.step, &force ) ;
	DG_COPY_VEC( &work->step, &work->control.step ) ;
	def->power = NULL ;
	/* 通常状態へ */
	work->mode = GRN_MODE_NORMAL ;
	work->bound_count = 0 ;
	work->flag &= ~( GRN_FLAG_ON_SLOPE | GRN_FLAG_STOP | GRN_FLAG_BOUND | GRN_FLAG_NOISED ) ;
	work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	if ( work->weapon == WP_Magazine ) work->control.height = 40.0F ;
	GM_GroupObjs( work->body.objs, GM_StageMapAll ) ;

	/* 誘爆 */
	work->count = 0 ;
}

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
			GM_GroupObjs( work->body.objs, GM_StageMapAll ) ;
		} else {
			work->flag &= ~GRN_FLAG_ON_MOVEFLOOR ;
		}
	}
}

/* 位置・回転更新 */
static	int		UpdatePos( work )
Work		*work ;
{
	FVECTOR	v1, mov, shift ;

	if ( !( work->flag & GRN_FLAG_STOP ) || ( work->flag & GRN_FLAG_ON_MOVEFLOOR ) ) {
		/* PAL用速度調整 */
		PL_PalAdjV( &work->control.step ) ;

		/* 床抜け防止用の丸め処理 */
		if ( work->control.mov.vy > 0.0f )
		  work->control.mov.vy = (float)((int)(work->control.mov.vy + 0.999f)) ;
		else
		  work->control.mov.vy = (float)((int)work->control.mov.vy) ;

		GM_ActControl( ( CONTROL * )&work->control ) ;	
		PL_PalAdjVR( &work->control.step ) ;
	}
	/* 移動床処理 */
	ExecMoveFloorFunc( work ) ;

	/* ワールドリミットで消す */
	if ( GM_CtrlWorldLimit( ( CONTROL * )&work->control ) ) return -1 ;
	/* bomblost_g内で消す */
	if ( !( work->flag & GRN_FLAG_STOP ) || ( work->flag & GRN_FLAG_ON_MOVEFLOOR ) ) {
		if ( HZX_CheckInsideTrap( HZX_CurrentGroupID, &work->control.mov, 
								  GM_TRP_BOMBLOST_G ) ) return -1 ;
	}
	/* 外部からの命令で消す */
	if ( work->list.flag & GM_BMB_FLAG_DESTROY ) return -1 ;

	GV_MatToVec( &work->body.objs->world, &v1 ) ;
	DG_SetPos2( &v1, &work->rot ) ;
	DG_GetPos( &work->body.objs->world ) ;
	DG_RotVector( &work->shift, &shift, 1 ) ;
	_sceVu0SubVector( &mov, &work->control.mov, &shift ) ;
	GV_VecToMat( &mov, &work->body.objs->world ) ;

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
//	if ( DG_FABS( pole.vx ) > 0.10F || DG_FABS( pole.vz ) > 0.10F ) {
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
		//printf( "len %f\n", len ) ;
		//DumpVec( &work->control.step ) ;
	} else {
		/* 平坦床 */
		work->flag &= ~GRN_FLAG_ON_SLOPE ;
		work->control.step.vy = 0.0F ;
		DG_COPY_VEC( &work->slope, &DG_ZeroVector ) ;
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
		if ( work->control.mov.vy > GM_WaterLevel ) {
			if ( work->weapon != WP_Magazine ) {
				GM_SeSetMode( SD_W_BOUND02, &work->control.mov, GM_SEMODE_NORMAL ) ;
			} else {
				GM_SeSetMode( SD_W_MAGAZI01, &work->control.mov, GM_SEMODE_NORMAL ) ;
			}
		}
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
				/* ２回目のときノイズ発生 */
				/* ノイズは、表示中マップに発生 */
				if ( !( work->flag & GRN_FLAG_NOISED ) && work->bound_count == 2 ) {
					int			map ;

					/* ノイズマップは単一でなければならない */
					map = GM_GetMapIDfromPos( GM_StageMapAll, &work->control.mov ) ;
					if ( map != 0 && work->control.mov.vy > GM_WaterLevel ) {
						if ( work->weapon != WP_Magazine ) {
							GM_SetNoise( NOISE_S, &work->control.mov, map ) ;
						} else {
							GM_SetNoiseStatus( NOISE_S, &work->control.mov, map,
											   NOISE_STATUS_MAG ) ;
						}
					}
					work->flag |= GRN_FLAG_NOISED ;
				}
				if ( work->bound_count >= MAX_BOUND_COUNT || 
					 work->control.mov.vy < GM_WaterLevel ) {
					/* 床上状態へ */
					work->mode = GRN_MODE_ON_FLOOR ;
					work->data = 0 ;
					if ( work->weapon == WP_Magazine ) {
						work->dir = ( short )( BP_PS2_rand() * 4095 ) ;
					}
					/* ニアチェック、床チェック開始 */
					work->control.skip_flag 
						&= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ) ;
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

/* ゲームレベル別処理 */
static	void	StunBlast( Work *work )
{
	int			r1, r2 ;

	switch ( GM_GameLevel ) {
	case GM_LEVEL_HARD :
	case GM_LEVEL_EXTREME :
		r1 = 6000 ; r2 = 12000 ;
		break ;
	case GM_LEVEL_NORMAL :
		r1 = 8000 ; r2 = 16000 ;
		break ;
	default :
		r1 = 16000 ; r2 = 32000 ;
	}	
	NewBlast3( (FVECTOR *)work->body.objs->world.m[3], work->side, r1, 
			  r1, 0, 0, WP_StunGrenade, BLAST_TYPE_NO_FIRE | BLAST_TYPE_ONETARG ) ;
	NewBlast3( (FVECTOR *)work->body.objs->world.m[3], work->side, r2, 
			  r2, 0, 0, WP_StunFar, BLAST_TYPE_NO_FIRE | BLAST_TYPE_ONETARG ) ;
}

/*------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	FVECTOR	step, from, v1, diff ;
	float	len ;
	int		flag ;

	/* 全モード共通 */
	GM_SetCurrentMap( GM_StageMapAll ) ; /* 要チェック */
#if 0
	/* 敵兵気づき用に１秒おきにノイズ発生 */
	if ( work->noise_sw > 0 ) {
		++ work->noise_sw ;
		if ( ( work->noise_sw % 60 ) == 0  ) {
			GM_SetNoise( NOISE_S, &work->control.mov, work->control.map ) ;
		}
	}
#endif
	/* 爆発 */
#if 1
	if ( ( !PL_AttackDisable() && !GM_CheckGameStatus( STATE_PAD_DEMO ) &&
		  !( work->list.flag & GM_BMB_FLAG_NOBLAST ) ) && 
		 ( -- work->count <= 0 || ( work->list.flag & GM_BMB_FLAG_BLAST ) ) ) {
		switch( work->weapon ) {
		case WP_Magazine :
			if ( work->flag & GRN_FLAG_STOP ) {
				if ( UpdatePos( work ) < 0 ) {
					GV_DestroyActor( work ) ;
					return ;
				}
				goto set_target ;
			} else {
				goto action ;
			}
		case WP_ChaffGrenade :
			if ( work->body.objs->world.m[ 3 ][ 1 ] < GM_WaterLevel && 
				PL_BlastWaterFunc != NULL ) {
				( *PL_BlastWaterFunc )( &work->control.mov, NO_SIDE, 
									   1000, 2000, 0, 0, WP_ChaffGrenade, 
									    BLAST_TYPE_NO_NOISE | BLAST_TYPE_NO_SE ) ;
				GM_SeSetMode( SD_W_EXPLOS03, &work->control.mov, GM_SEMODE_NORMAL ) ;
			} else {
				NewBlastChaff( &work->body.objs->world ) ;
			}
			break ;
		case WP_StunGrenade :
			if ( work->body.objs->world.m[ 3 ][ 1 ] < GM_WaterLevel && 
				PL_BlastWaterFunc != NULL ) {
				( *PL_BlastWaterFunc )( &work->control.mov, NO_SIDE, 
									   1000, 2000, 0, 0, WP_StunGrenade, BLAST_TYPE_NO_SE ) ;
				GM_SeSetMode( SD_W_EXPLOS03, &work->control.mov, GM_SEMODE_NORMAL ) ;
			} else {
				NewBlastStun( &work->body.objs->world ) ;
				StunBlast( work ) ;
			}
			break ;
		default :
#ifndef NO_BLAST
			flag = ( work->side == PLAYER_SIDE ) ? BLAST_TYPE_NO_NOISE : 0 ;
			if ( work->body.objs->world.m[ 3 ][ 1 ] < GM_WaterLevel && 
				PL_BlastWaterFunc != NULL ) {
				( *PL_BlastWaterFunc )( &work->control.mov, work->side, 
									   1000, 2000, DMG_BLAST, FNT_BLAST, work->weapon, flag ) ;
			} else {
				NewBlast3( &work->control.mov, work->side, gBP_Game_GrenadeBlast_InnerRange /*1500*/,             //BP_INPUT - replaced with tweakable grenade blast ranges
						  gBP_Game_GrenadeBlast_OuterRange /*3000*/, DMG_BLAST, FNT_BLAST, work->weapon, flag ) ;
			}
#endif
		}
		GV_DestroyActor( work ) ;
		return ;
	}   
#endif
action :
	if ( work->time < 0 ) ++ work->time ; 

	switch( work->mode ) {
	case GRN_MODE_NORMAL :		/* 通常状態 */	
		//printf( "normal %f\n", work->control.step.vy ) ;
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

		/* 水面水飛沫 */
		_sceVu0SubVector( &diff, &work->control.mov, &from ) ;
		if ( !( work->flag & GRN_FLAG_SPLASH ) &&
			PL_CheckBulletSplash( &from, &diff, 75.0F, 125.0F ) ) {
			work->flag |= GRN_FLAG_SPLASH ;
		}

		if ( work->control.mov.vy > GM_WaterLevel ) {
			work->control.step.vy -= 9.0F ;
		} else {
			/* 水中では一定速度 */
			work->control.step.vx = 0.0F ;
			work->control.step.vz = 0.0F ;
			work->control.step.vy = -8.0F ;
		}

		if ( work->time >= 0 ) {
			GM_MoveOnlineTargetMap( &work->target, &from, &work->control.mov, GM_StageMapAll ) ;
			GM_PutTarget( &work->target ) ;
		}	
		break ;
	case GRN_MODE_ON_FLOOR :	/* 床上 */
		//printf( "on floor\n" ) ;
		from = work->control.mov ;
		if ( work->weapon == WP_Magazine ) work->control.height = 20.0F ;
        if ( UpdatePos( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return ;
		}
		if ( !( work->flag & GRN_FLAG_STOP ) &&
			 ( work->control.grounded & 1 ) ) UpdateFloor( work ) ;
		_sceVu0SubVector( &v1, &work->control.mov, &from ) ;
		if ( work->flag & GRN_FLAG_ON_SLOPE ) {
			//	    _sceVu0AddVector( &work->control.step, &work->control.step, &work->slope ) ;
		} else {
			work->control.step.vx /= 1.50F ;
			work->control.step.vz /= 1.50F ;
		}
		if ( work->weapon == WP_Magazine ) {
			work->rot.vx = GV_NearExp4P( work->rot.vx & 4095, 0 ) ;
			work->rot.vy = GV_NearExp4P( work->rot.vy & 4095, work->dir ) ;
			work->rot.vz = GV_NearExp4P( work->rot.vz & 4095, 1024 ) ;
		} else {
			if ( DG_FABS( v1.vx ) > 0.10F || DG_FABS( v1.vz ) > 0.10F ) {
				work->dir = GV_VecDir2( &v1 ) + 1024 ;
			}
			work->rot.vx = GV_NearExp4P( work->rot.vx & 4095, 0 ) ;
			work->rot.vy = GV_NearExp4P( work->rot.vy & 4095, work->dir ) ;
			v1.vx = work->control.step.vx ;
			v1.vy = 0.0F ;
			v1.vz = work->control.step.vz ;
			len = GV_VecLen3F( &v1 ) ;
			work->rot.vz += len * 4096.0F / 240.0F ;
		}

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
			if ( work->weapon == WP_Magazine ) work->control.height = 40.0F ;
		}
		work->control.step.vy -= work->control.height - 8.0F ;

        if ( work->control.level[ 0 ]->attribute & HZX_FLOOR_DYNAMIC ) {
			/* 動的床上のときはＳＴＯＰ状態にしない */
			//printf( "on dynamic %f\n", work->control.levels[ 0 ] ) ;
			work->data = 16 ;
			return ;
		}
		if ( work->data > 0 ) {
			/* 通常状態に戻る可能性がある */
			return ;
		}

		if ( DG_FABS( work->control.step.vx ) < 1.0F &&
			 DG_FABS( work->control.step.vz ) < 1.0F ) {
			DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;
			work->flag |= GRN_FLAG_STOP | GRN_FLAG_ON_MOVEFLOOR ; /* 移動床上フラグも立てておく */
		} else if ( work->control.n_touches ) {
			DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;
			work->flag |= GRN_FLAG_STOP | GRN_FLAG_ON_MOVEFLOOR ; /* 移動床上フラグも立てておく */
		}
		
		if ( work->flag & GRN_FLAG_STOP ) {
			HZX_ZONE_ADD	zone[ 2 ] ;
			int				hzx_id, map ;

			/* マップの確定 */
			HZX_GetInterruptZone( &work->control.mov, 0, 100, &zone[ 0 ] ) ;
			hzx_id = GV_GetBit( HZX_ZoneMapNo( zone[ 0 ] ) ) ;
			hzx_id |= GV_GetBit( HZX_ZoneMapNo( zone[ 1 ] ) ) ;
			map = GM_GetMapIDfromHzxGroupID( hzx_id ) ;
			GM_GroupObjs( work->body.objs, map ) ;
		}

#if 0							/* 床上の時はターゲットチェックしない */
		if ( work->time >= 0 ) {
			GM_MoveOnlineTarget( &work->target, &from, &work->control.mov ) ;
			GM_PutTarget( &work->target ) ;
		}	
#endif
		break ;
	case GRN_MODE_ON_TARGET :	/* ターゲット上 */
	  ;
	}
set_target :
	GM_MoveTargetMap( &work->defense, &work->control.mov, GM_StageMapAll ) ;
	//NewTargetView2( &work->defense, 232, 32, 232 ) ;

	/* 外部からの命令で非表示 */
	/* 戻せない */
	if ( work->list.flag & GM_BMB_FLAG_INVISIBLE ) {
		DG_InvisibleObjs( work->body.objs ) ;
	}
}

static	void	Die( work )
Work		*work ;
{
	GM_FreeControl( ( CONTROL * )&work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_FreeTarget( &work->defense ) ;
	RemoveList( work ) ;
	GM_N_Grenades -- ;
}

/*------------------------------------------------------------*/

static	int	InitControl( work, weapon, world, force )
Work		*work ;
int		weapon ;
FMATRIX		*world ;
FVECTOR		*force ;
{
	CONTROL	*ctrl ;
	FVECTOR	mov, shift ;

	ctrl = ( CONTROL * )&( work->control ) ;
	/* 武器番号を名前にする */
	if ( GM_InitControl( ctrl, weapon, GM_StageMapAll ) < 0 ) return -1 ;
	GV_MatToVec( world, &mov ) ;
	DG_SetPos( world ) ;
	DG_RotVector( &work->shift, &shift, 1 ) ;
	DG_RotVector( force, &ctrl->step, 1 ) ;
	_sceVu0AddVector( &ctrl->mov, &mov, &shift ) ;

	/* mov と ctrl->mov の間に壁がある */
	if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &ctrl->mov, &mov,
							   HZX_CHK_ALL, 
							   HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_MISSILE,
							   HZX_FLOOR_RECOIL_TYPE | HZX_FLOOR_NO_MISSILE ) ) {
		//printf( "bul_gre.c : mov mov online\n" ) ;
		DG_COPY_VEC( &ctrl->mov, &mov ) ;
	}
	/* 全マップ対応 */
//	GM_ConfigControlMapID( ctrl ) ;
//	GM_ConfigControlMapCheck( ctrl ) ;
	GM_ConfigControlHazard( ctrl, 40, 80, 80 ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE | HZX_FLOOR_PITFALL ;
	/* onlineチェックのみ行う */
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	return 0 ;
}

/* ターゲット初期化 */
static	void	InitTarget( work )
Work		*work ;
{
	TARGET		*t ;
	FVECTOR		size ;

	t = &work->target ;
	/* TARGET_POWERだけでいいかなあ */
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | 
				 TARGET_GET_NORMAL | TARGET_POWER, 
				 GM_StageMapAll, ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_THROWG ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
	GM_SetTargetName( t, WP_ThrowG ) ;
	work->time = -4 ;			/* ４フレーム後からターゲットチェック */

	/* 防御用 */
	t = &work->defense ;
	size.vx = size.vy = size.vz = 40.0F ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_NO_CLAYMORE, GM_StageMapAll, BOTH_SIDE,
				  &size, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, Hitted, work ) ;
	GM_SetTargetName( t, WP_ThrowG ) ;
	GM_PutTarget( t ) ;
}

static	int	GetResources( work, world, force, weapon, body ) 
Work		*work ;
FMATRIX		*world ;
FVECTOR		*force ;
int		weapon, body ;
{
	switch( weapon ) {
	case WP_Magazine :
		DG_COPY_VEC( &work->shift, &ShiftMagazine ) ;
		break ;
	case WP_Grenade :
		DG_COPY_VEC( &work->shift, &ShiftGrenade ) ;
		break ;
	default :	
		DG_COPY_VEC( &work->shift, &ShiftChaff ) ;	
	}

	if ( InitControl( work, weapon, world, force ) < 0 ) return -1 ;
	GM_InitObject( &( work->body ), body, BODY_FLAG ) ;
	if ( work->body.objs == NULL ) return -1 ;
	GM_ConfigControlObject( ( CONTROL * )&work->control, &work->body ) ;
	work->body.map_name = GM_StageMapAll ;
	DG_COPY_MAT( &work->body.objs->world, world ) ;
	GM_GroupObjs( work->body.objs, GM_StageMapAll ) ;
	InitTarget( work ) ;
	work->ride_on = NULL ;
	work->mode = GRN_MODE_NORMAL ;

	AddList( work, weapon ) ;

	return 0 ;
}

/* グレネード起動 */
void	*NewBulletGrenade( world, force, side, weapon, body, count )
FMATRIX		*world ;
FVECTOR		*force ;
int		side ;
int		weapon, body, count ;
{
	Work			*work ;
	GM_BOMB			*this, *last ;

	if ( GM_N_Grenades >= GM_CURRENT_GRENADE_MAX ) {
		/* 最初に登録した奴（リストの最後の奴）を消す */
		this = GM_BombList.next ;
		last = NULL ;
		while( this != NULL ) {
			if ( GM_WeaponTypes[ this->weapon ] & WP_TYPE_GRENADE ) {
				last = this ;
			}
			this = this->next ;
		}
		if ( last != NULL ) last->flag |= GM_BMB_FLAG_DESTROY ;
	}

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, force, weapon, body ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->map = GM_StageMapAll ;
		work->world = *world ;
		work->side = side ;
		work->weapon = weapon ;
		work->count = count ;
		if ( work->count == 0 ) work->count = 1 ;
		work->rotate = ROTATE_STEP ;

		GM_SetWeaponFire( weapon ) ;
	}
	return work ;
}

/*----------------------------------------------------------------*/

int		NewSetGrenadeFlag( void )
{
	PL_Bul_Grenade_Flag = GCL_GetOptionValue( 'f', 0 ) ;
	return 0 ;
}
