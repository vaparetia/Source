/*
   water_old.c
   水中モードプラグイン（テスト用バージョン）

   2000/05/15 M.Sonoyama
   $Id: water_old.c,v 1.1.1.3 2002/11/19 11:50:52 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"gameheader.h"
#include	"camera.h"
#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_inline.h"

enum {
	WMfloat = 0,	/* 水面浮き */
	WMfloat_move,	/* 水面浮き移動 */
	WMtoswim,		/* 浮きから泳ぎ */
	WMtofloat,		/* 泳ぎから浮き */
	WMstand,		/* 水中立ち */
	WMwalk,			/* 水中歩き */
	WMswim_p,		/* 泳ぎポーズ静止 */
	WMswim,			/* 泳ぎループ */
	MAX_WATER_MOTIONS
} ;

#define	PAD_DIVE		(PAD_B)
#define	PAD_SWIM		(PAD_A)
#define	PAD_STAND		(PAD_B)
#define	PAD_UPPER		(PAD_U)
#define	PAD_LOWER		(PAD_D)

static	PL_PluginSet	PluginSet ;
static	PL_PollingSet	PollingSet ;

static	int				Motion = 0 ;
static	float			HorizonLevel, Diff ;

static	GM_CameraSet	*WaterCamera ;

static	float			Levels[ 2 ] ;
static	int				LevelFlag, HazardFlag ;
static	SVECTOR			PreRot ;

/*---------------------------------------------------------------*/

static	void	Water_Floating( PlayerWork *, int ) ;
static	void	Water_FloatMove( PlayerWork *, int ) ;
static	void	Water_toSwim( PlayerWork *, int ) ;
static	void	Water_SwimStill( PlayerWork *, int ) ;
static	void	Water_Swim( PlayerWork *, int ) ;
static	void	Water_toFloat( PlayerWork *, int ) ;
static	void	Water_toStand( PlayerWork *, int ) ;
static	void	Water_Stand( PlayerWork *, int ) ;
static	void	Water_Walk( PlayerWork *, int ) ;

/*---------------------------------------------------------------*/

#define	WATER_TURNX_LIMIT	(640)
#define	SWIM_DEPTH			(160.0F)

#define	HEAD	(1000.0F)
#define	FOOT	(1200.0F)

#define	FALL_SPEED			(8.0F)
#define	FLOAT_SPEED			(0.0F)

/*---------------------------------------------------------------*/

/* 独自の床チェック */
static	void	CheckLevelHazard( work, mov )
PlayerWork		*work ;
FVECTOR			*mov ;
{
	int			flag ;

	flag = HZX_LevelHazardCheck( work->control.hzx_id,
								 mov,
								 work->control.hzx_check_type, 
								 work->control.flr_flag ) ;
	Levels[ 0 ] = -10000000.0F ;
	Levels[ 1 ] = 10000000.0F ;
	if ( flag )	HZX_GetLevelHeight( Levels ) ;
	LevelFlag = flag ;
}

/* 独自のめり込みチェック */
/* 
   mode == 0 : step 書き込みあり 
   mode == 1 : step 書き込み無し
*/
static	int	CheckSink( work, head, waist, foot, mode )
PlayerWork		*work ;
FVECTOR		*head, *waist, *foot ;
int			mode ;
{
	int				c ;
	HZX_GROUP_ID	hzx_id ;
	FVECTOR			hit, step ;
	FVECTOR			w, h, f ;

	DG_COPY_VEC( &w, waist ) ;
	DG_COPY_VEC( &h, head ) ;
	DG_COPY_VEC( &f, foot ) ;

	if ( work->control.interp != 0 ) return 0 ;

	c = 0 ;
	hzx_id = work->control.hzx_id ;
	if ( HZX_OnlineHazardCheck( hzx_id, &w, &h, HZX_CHK_ALL,
							    HZX_SEG_NO_PLAYER | HZX_SEG_RECOIL_TYPE, 
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_RECOIL_TYPE ) ) {
		HZX_GetOnlinePoint( &hit ) ;
		_sceVu0SubVector( &step, &hit, &h ) ;
		if ( mode == 0 ) {
			_sceVu0AddVector( &work->control.step, &step, &work->control.step ) ;
		}
		_sceVu0AddVector( &w, &w, &step ) ;
		_sceVu0AddVector( &h, &h, &step ) ;
		_sceVu0AddVector( &f, &f, &step ) ;
		c |= 4 ;
	}
	if ( HZX_OnlineHazardCheck( hzx_id, &w, &f, HZX_CHK_ALL,
							    HZX_SEG_NO_PLAYER | HZX_SEG_RECOIL_TYPE, 
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_RECOIL_TYPE ) ) {
		HZX_GetOnlinePoint( &hit ) ;
		_sceVu0SubVector( &step, &hit, &f ) ;
		if ( mode == 0 ) {
			_sceVu0AddVector( &work->control.step, &step, &work->control.step ) ;
		}
		_sceVu0AddVector( &w, &w, &step ) ;
		_sceVu0AddVector( &h, &h, &step ) ;
		_sceVu0AddVector( &f, &f, &step ) ;
		c |= 8 ;
		if ( c & 4 ) c |= 0x10 ;
		else if ( HZX_OnlineHazardCheck( hzx_id, &w, &h, HZX_CHK_ALL,
										HZX_SEG_NO_PLAYER | HZX_SEG_RECOIL_TYPE, 
										HZX_FLOOR_NO_PLAYER | HZX_FLOOR_RECOIL_TYPE ) ) {
			c |= 0x10 | 4 ;
		}
	}	

	DG_COPY_VEC( waist, &w ) ;
	DG_COPY_VEC( head, &h ) ;
	DG_COPY_VEC( foot, &f ) ;
	return c ;
}

static	int	CheckSink2( work, rot, mode )
PlayerWork			*work ;
SVECTOR				*rot ;
int					mode ;
{
	FVECTOR		w ;
	FVECTOR		h = { 0.0F, 0.0F, HEAD } ; 
	FVECTOR		f = { 0.0F, 0.0F, -FOOT } ; 

	DG_COPY_VEC( &w, &work->control.mov ) ;
	DG_SetPos2( &w, rot ) ;
	DG_PutVector( &h, &h, 1 ) ;
	DG_PutVector( &f, &f, 1 ) ;	
	return CheckSink( work, &h, &w, &f, mode ) ;
}

static	int	CheckSink3( work, rot, mode )
PlayerWork			*work ;
SVECTOR				*rot ;
int					mode ;
{
	FVECTOR		w, h, f ;

	DG_COPY_VEC( &h, &DG_ZeroVector ) ;
	DG_COPY_VEC( &f, &DG_ZeroVector ) ;
	work->g_rot = rot->vx ;
	PL_GetModelLength( work, ( float * )&h.vz, ( float * )&f.vz ) ;
	f.vz = - f.vz ;
	DG_COPY_VEC( &w, &work->control.mov ) ;
	DG_SetPos2( &w, rot ) ;
	DG_PutVector( &h, &h, 1 ) ;
	DG_PutVector( &f, &f, 1 ) ;	
	return CheckSink( work, &h, &w, &f, mode ) ;
}

/* 水中用当たりチェック */
static	int		CheckHazardforWater( work )
PlayerWork		*work ;
{
	FVECTOR		head_def = { 0.0F, 0.0F, HEAD } ; 
	FVECTOR		foot_def = { 0.0F, 0.0F, -FOOT } ; 
	FVECTOR		waist, head, foot ;
	float		svy1[ 2 ], svy2[ 2 ], svy3[ 2 ] ;
	int			c ;

	c = 0 ;
	DG_COPY_VEC( &waist, &work->control.mov ) ;
	DG_SetPos2( &waist, &work->control.rot ) ;
	DG_PutVector( &head_def, &head, 1 ) ;
	DG_PutVector( &foot_def, &foot, 1 ) ;

	/* めりこみチェック */
	c |= CheckSink( work, &head, &waist, &foot, 0 ) ;
	if ( c & 0x10 ) {
		DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;
		GM_ResetControlPosition( &work->control, &work->pre_mov ) ;
		work->control.turn = work->control.rot = PreRot ;
		work->g_rot = PreRot.vx ;
		DG_COPY_VEC( &waist, &work->pre_mov ) ;
		DG_SetPos2( &waist, &PreRot ) ;
		DG_PutVector( &head_def, &head, 1 ) ;
		DG_PutVector( &foot_def, &foot, 1 ) ;
	}
	/* 床・天井チェック */
	CheckLevelHazard( work, &waist ) ;
	svy1[ 0 ] = svy1[ 1 ] = 0.0F ;
	if ( LevelFlag & 1 ) {
		if ( Levels[ 0 ] > ( waist.vy - 350.0F ) ) {
			svy1[ 0 ] = Levels[ 0 ] + 350.0F - waist.vy ;
			c |= 1 ;
		}
	} 
	if ( LevelFlag & 2 ) {	
		if ( Levels[ 1 ] < ( waist.vy + 350.0F ) ) {
			svy1[ 1 ] = Levels[ 1 ] - 350.0F - waist.vy ;
			c |= 2 ;
		}
	}
	CheckLevelHazard( work, &head ) ;
	svy2[ 0 ] = svy2[ 1 ] = 0.0F ;
	if ( LevelFlag & 1 ) {
		if ( Levels[ 0 ] > ( head.vy - 350.0F ) ) {
			svy2[ 0 ] = Levels[ 0 ] + 350.0F - head.vy ;
			c |= 1 ;
		}
	} 
	if ( LevelFlag & 2 ) {	
		if ( Levels[ 1 ] < ( head.vy + 350.0F ) ) {
			svy2[ 1 ] = Levels[ 1 ] - 350.0F - head.vy ;
			c |= 2 ;
		}
	}

	CheckLevelHazard( work, &foot ) ;
	svy3[ 0 ] = svy3[ 1 ] = 0.0F ;
	if ( LevelFlag & 1 ) {
		if ( Levels[ 0 ] > ( foot.vy - 350.0F ) ) {
			svy3[ 0 ] = Levels[ 0 ] + 350.0F - foot.vy ;
			c |= 1 ;
		}
	} 
	if ( LevelFlag & 2 ) {	
		if ( Levels[ 1 ] < ( head.vy + 350.0F ) ) {
			svy3[ 1 ] = Levels[ 1 ] - 350.0F - head.vy ;
			c |= 2 ;
		}
	}
	if ( c & 3 ) {
		if ( ( c & 3 ) == 3 ) printf( "ouch\n" ) ;	
		else if ( c & 1 ) {
			svy1[ 0 ] = DG_MAX( svy1[ 0 ], svy2[ 0 ] ) ;
			svy1[ 0 ] = DG_MAX( svy1[ 0 ], svy3[ 0 ] ) ;
		} else if ( c & 2 ) {
			svy1[ 0 ] = DG_MIN( svy1[ 1 ], svy2[ 1 ] ) ;
			svy1[ 0 ] = DG_MIN( svy1[ 0 ], svy3[ 1 ] ) ;
		}
		if ( ( c & ( 4 | 8 ) ) == 0 ) work->control.step.vy = 0.0F ;
		work->control.step.vy += svy1[ 0 ] ;
	}
	HazardFlag = c ;
	return c ;
}

/* 潜り姿勢になれる方向を向く */
static	int		CheckSwimEnableDir( work )
PlayerWork		*work ;
{
    SVECTOR	def ;
    int		c ;

    /* ４方向バージョン */
	def.vx = 0 ;
    def.vy = work->control.rot.vy ; 
	def.vz = 0 ;
    c = CheckSink2( work, &def, 1 ) ;
    if ( !( c & 0x10 ) ) return ( def.vy & 4095 ) ;
    def.vy = ( def.vy / 1024 * 1024 ) & 4095 ;
    c = CheckSink2( work, &def, 1 ) ;
    if ( !( c & 0x10 ) ) {
		work->control.interp = 16 ;
		work->control.turn.vy = def.vy ;
		return ( def.vy & 4095 ) ;
	}
    def.vy = def.vy + 1024 ;
    c = CheckSink2( work, &def, 1 ) ;
    if ( !( c & 0x10 ) ) {
		work->control.interp = 16 ;
		work->control.turn.vy = def.vy ;
		return ( def.vy & 4095 ) ;
	}
	work->control.interp = 0 ;
    return -1 ;
}

/*---------------------------------------------------------------*/

/* カメラのＯＮ／ＯＦＦ */
static	inline	void	WaterCameraOn( void )
{
	WaterCamera->on = 1 ;
	GM_ChangeCamera( 0 ) ;
}

static	inline	void	WaterCameraOff( void )
{
	WaterCamera->on = 0 ;
	GM_ChangeCamera( 0 ) ;
	if ( !Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		DG_VisibleObjsChanl( GM_PlayerBody->objs, GM_PlayerWork->chanl ) ;
	}
}

/* カメラ角度更新 */
static	void	UpdateCamera( work )
PlayerWork		*work ;
{
	FVECTOR	vec = { 0.0F, 750.0F, -2000.0F } ;
	float	vx[] = { -450.0F, 0.0F, 450.0F } ;
	float	vy[] = { 350.0F, 750.0F, -150.0F } ;
	float	len, maxlen ;
	FVECTOR	from, to, from2 ;
	GM_CameraSet	*cam ;
	int				i, j, c ;

	cam = WaterCamera ;

	DG_COPY_VEC( &to, &work->control.mov ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutVector( &vec, &from, 1 ) ;

	if ( work->pad->status & PAD_R ) {
		vx[ 0 ] = 450.0F ; vx[ 2 ] = -450.0F ;
	}

	maxlen = 2000.0F ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &to, &from, HZX_CHK_ALL,
							    HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
		HZX_GetOnlinePoint( &from ) ;
		maxlen = GV_VecLen3F2( &from, &to ) ;
		c = 0 ;
		for ( i = 0; i < 3; i ++ ) {
			for ( j = 0; j < 3; j ++ ) {
				vec.vx = vx[ i ] ;
				vec.vy = vy[ j ] ;
				DG_PutVector( &vec, &from2, 1 ) ;				
				if ( !HZX_OnlineHazardCheck( work->control.hzx_id, &to, &from2, HZX_CHK_ALL,
										    HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
					DG_COPY_VEC( &from, &from2 ) ;
					c = 1 ;
					break ;
				} else {
					HZX_GetOnlinePoint( &from2 ) ;					
					len = GV_VecLen3F2( &from2, &to ) ;
					if ( maxlen < len ) {
						maxlen = len ;
						DG_COPY_VEC( &from, &from2 ) ;
					}
				}
			}
			if ( c ) break ;
		}
	} 

//	DG_COPY_VEC( &cam->position, &from ) ;
//	DG_COPY_VEC( &cam->target, &to ) ;
	GV_NearExp16VF( &cam->position, &from, 3 ) ;
	GV_NearExp16VF( &cam->target, &to, 3 ) ;

	if ( !Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		len = GV_VecLen3F2( &cam->position, &cam->target ) ;
		if ( cam->on == 1 && len < 650.0F ) {
			DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		} else {
			DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
		}
	}
}

/* 当たりチェック高さ */
static	inline	void	SetHzxHeight( PlayerWork *work, float height )
{
	work->hzx_height = work->control.hzx_height = height ;
}

/* 水中モード共通 */
static	void	WaterCommonProc( work )
PlayerWork		*work ;
{
	UpdateCamera( work ) ;
	SetFlag( FLAG_CANNOT_CHANGE ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
}

/* 水面に高さ調整 */
static	void	KeepHorizonLevel( work )
PlayerWork		*work ;
{
	float		v ;

	Diff = HorizonLevel - work->control.mov.vy ;
	v = GV_NearExp16F( Diff, 0.0F ) ;
	work->control.mov.vy += Diff - v ;
}

/* Ｘ回転を徐々に０ */
static	void	ResetTurnX( work )
PlayerWork		*work ;
{
	int			turn ;
//	work->control.turn.vx = GV_NearExp16P( work->control.turn.vx, 0 ) ;
	turn = work->control.turn.vx ;
	if ( turn > 0 ) {
		turn -= 4 ;
		if ( turn < 0 ) turn = 0 ;
	} else {
		turn += 4 ;
		if ( turn > 0 ) turn = 0 ;
	}
	work->control.turn.vx = turn ;
	work->g_rot = work->control.rot.vx ;
}

/* Ｙ移動量 */
static	void	SetStep( work )
PlayerWork			*work ;
{
	float		len ;
	FVECTOR		step ;
	
	DG_COPY_VEC( &step, &work->body.m_ctrl->step ) ;
	step.vy = 0.0F ;
	len = GV_VecLen3F( &step ) ;
	work->control.step.vy 
		+= -len * sinf( ( float )work->control.rot.vx * ( float )PI / 2048.0F ) ;
}

/* 泳ぎ中回転 */
static	void	WaterTurn( work, mode )
PlayerWork		*work ;
int				mode ;
{
	GV_PAD		*pad ;

	if ( HazardFlag & 0x10 ) return ;

	pad = work->pad ;
	if ( pad->status & PAD_L ) work->control.turn.vy += 8 ;
	else if ( pad->status & PAD_R ) work->control.turn.vy -= 8 ;
	if ( mode == 0 ) {
		if ( pad->status & PAD_LOWER ) work->control.turn.vx -= 8 ;
		else if ( pad->status & PAD_UPPER ) work->control.turn.vx += 8 ;
	}
	if ( work->control.turn.vx > WATER_TURNX_LIMIT ) 
		work->control.turn.vx = WATER_TURNX_LIMIT ;
	if ( work->control.turn.vx < -WATER_TURNX_LIMIT ) 
		work->control.turn.vx = -WATER_TURNX_LIMIT ;
	work->g_rot = work->control.rot.vx ;
}

/* 水中歩きシーク */
static	void	WaterSeek( work )
PlayerWork		*work ;
{
	GV_PAD		*pad ;
	int			turn_to, diff ;

	pad = work->pad ;
	if ( pad->dir < 0 ) return ;
    turn_to = work->control.turn.vy ;
    diff = GV_DiffDirS( turn_to, pad->dir ) ;
    if ( diff != 0 ) {
		turn_to = GV_NearPhase( turn_to, pad->dir ) ;
		work->control.turn.vy = GV_NearSpeed( turn_to, pad->dir, 16 ) ;
    }
}

/* 終わり */
static	void	EndWater( work )
PlayerWork		*work ;
{
	SetFlag( FLAG_FORCE_END ) ;
	WaterCameraOff() ;
	UnsetStatus( PLAYER_IN_THE_WATER ) ;
	PL_Force->e_turn = -1 ;
	SetHzxHeight( work, 750.0F ) ;
	work->control.hzx_base = work->control.mov.vy - work->body.height ;
	SetMode( work, PL_StillMode[ STAND ] ) ;	
printf( "[%d] out water\n", GV_Time ) ;
}

/*---------------------------------------------------------------*/

/* 浮き */
static	void	Water_Floating( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	if ( time == 0 ) {
		SetAction( work, WMfloat, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		Diff = HorizonLevel - work->control.mov.vy ;
	}
	ResetTurnX( work ) ;
	KeepHorizonLevel( work ) ;
	work->control.step.vy = 0 ;
	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
//		PL_SeekTurn( work ) ;
        if ( work->pad->dir >= 0 ) {
			SetMode( work, Water_FloatMove ) ;
		}
	}
	/* 潜る */
	if ( work->pad->status & PAD_DIVE ) {
		if ( CheckSwimEnableDir( work ) >= 0 ) {
			SetMode( work, Water_toSwim ) ;
		}
	}
}

/* 浮き移動 */
static	void	Water_FloatMove( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	if ( time == 0 ) {
		SetAction( work, WMfloat_move, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	}	
	ResetTurnX( work ) ;
	KeepHorizonLevel( work ) ;
	work->control.step.vy = 0 ;

	/* 床チェック */
	CheckLevelHazard( work, &work->control.mov ) ;
	if ( LevelFlag & 1 ) {
		float	diff ;
		diff = work->control.mov.vy - Levels[ 0 ] ;
		if ( diff < work->control.height ) {
			SetMode( work, Water_Walk ) ;
			return ;
		}
	}

	if ( Status( PLAYER_WATCH ) ) {	
		SetMode( work, Water_Floating ) ;
		return ;
	}
	if ( work->pad->dir == -1 ) {
		SetMode( work, Water_Floating ) ;
		return ;
	} else {
		work->control.turn.vy = work->pad->dir ;
	}
	/* 潜る */
	if ( work->pad->status & PAD_DIVE ) {
		if ( CheckSwimEnableDir( work ) >= 0 ) {
			SetMode( work, Water_toSwim ) ;
		}
	}
}

/* 潜り */
static	void	Water_toSwim( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	if ( time == 0 ) {
		SetAction( work, WMtoswim, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	}	
	SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_IK ) ;
	work->control.step.vy = 0.0F ;
	CheckHazardforWater( work ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, Water_SwimStill ) ;
	}
}

/* 泳ぎ静止 */
static	void	Water_SwimStill( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	if ( time == 0 ) {
		SetAction( work, WMswim_p, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		WaterCameraOn() ;
	}	
	ResetTurnX( work ) ;
	SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_IK ) ;
	GV_NearExp16VF( &work->control.step, &DG_ZeroVector, 3 ) ;
	work->control.step.vy = FLOAT_SPEED ;
	CheckHazardforWater( work ) ;
	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		WaterTurn( work, 0 ) ;
		if ( work->pad->press & PAD_SWIM ) {
			SetMode( work, Water_Swim ) ;
			return ;
		}
	}

	if ( work->control.mov.vy >= HorizonLevel - SWIM_DEPTH ) {	
		work->control.mov.vy = HorizonLevel - SWIM_DEPTH ;
		work->control.turn.vx = 0 ;
	}

#if 0
	if ( ++ work->data >= 180 ) {
		if ( work->control.mov.vy >= HorizonLevel - SWIM_DEPTH ) {
			SetMode( work, Water_toFloat ) ;
			WaterCameraOff() ;
			return ;
		} 
	}
#endif
	
	if ( work->pad->press & PAD_STAND ) {
		StopTurn( work ) ;
		if ( work->control.mov.vy >= HorizonLevel - SWIM_DEPTH ) {
			SetMode( work, Water_toFloat ) ;
			WaterCameraOff() ;
		} else {		
			SetMode( work, Water_toStand ) ;
		}
	}
}

/* 泳ぎ */
static	void	Water_Swim( work, time )
PlayerWork		*work ;
int				time ;
{
	int			c ;

	WaterCommonProc( work ) ;
	if ( time == 0 ) {
		SetAction( work, WMswim, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	}	
	SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_IK ) ;
	ResetTurnX( work ) ;
	WaterTurn( work, 0 ) ;

	work->control.step.vy = FLOAT_SPEED / 2.0F ;
	if ( !( c = CheckHazardforWater( work ) ) ) SetStep( work ) ;
	else {
		if ( ( c & 1 ) && ( work->pad->status & PAD_UPPER ) ) {
			work->control.turn.vx -= 32 ;
			if ( work->control.turn.vx < 0 ) work->control.turn.vx = 0 ;
		}
		else if ( ( c & 2 ) && ( work->pad->status & PAD_LOWER ) ) {
			work->control.turn.vx += 32 ;
			if ( work->control.turn.vx > 0 ) work->control.turn.vx = 0 ;
		}
	}

	if ( ( work->pad->press & PAD_SWIM ) &&
		 ( PL_MotionTime( work ) >= 72 ) ) {
		work->motion1 = -1 ;
		SetAction( work, WMswim, 6 ) ;
	}

	if ( work->control.mov.vy >= HorizonLevel ) {	
		work->control.mov.vy = HorizonLevel ;	
		if ( work->control.rot.vx < -480 ) {
			SetMode( work, Water_toFloat ) ;
			WaterCameraOff() ;
			return ;
		}
	} 
	if ( EndMotion( work ) ) {
		SetMode( work, Water_SwimStill ) ;
		return ;
	}
	if ( work->pad->press & PAD_STAND ) {
		SetMode( work, Water_toStand ) ;
		return ;
	}
}

/* 泳ぎから水中立ち */
static	void	Water_toStand( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	SetHzxHeight( work, 750.0F ) ;
	work->control.hzx_base = work->control.mov.vy - work->control.height ;
	if ( time == 0 ) {
		SetAction( work, WMtofloat, 6 ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
	}	
	SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_IK ) ;
	ResetTurnX( work ) ;
	work->control.step.vy = 0.0F ;
	CheckHazardforWater( work ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, Water_Stand ) ;
	}
}

/* 泳ぎから浮き */
static	void	Water_toFloat( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	if ( time == 0 ) {
		SetAction( work, WMtofloat, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		Diff = HorizonLevel - work->control.mov.vy ;
	}	
	SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_IK ) ;
	KeepHorizonLevel( work ) ;
	ResetTurnX( work ) ;
	work->control.step.vy = 0.0F ;
	if ( EndMotion( work ) ) {
		SetMode( work, Water_Floating ) ;
	}
}

/* 水中立ち */
static	void	Water_Stand( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	SetHzxHeight( work, 750.0F ) ;
	work->control.hzx_base = work->control.mov.vy - work->control.height ;
		
	if ( time == 0 ) {
		SetAction( work, WMstand, 6 ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
	}	
	ResetTurnX( work ) ;
	work->control.step.vy = -FALL_SPEED ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		if ( work->pad->press & PAD_DIVE ) {
			if ( CheckSwimEnableDir( work ) >= 0 ) {
				WaterCameraOn() ;
				SetMode( work, Water_toSwim ) ;
				return ;
			}
		}
		if ( work->pad->dir != -1 ) {
			SetMode( work, Water_Walk ) ;
		}
	}	
	if ( work->control.grounded & 1 ) WaterCameraOff() ;
	else if ( ++ work->data >= 30 ) WaterCameraOn() ;
}

/* 水中歩き */
static	void	Water_Walk( work, time )
PlayerWork		*work ;
int				time ;
{
	WaterCommonProc( work ) ;
	SetHzxHeight( work, 750.0F ) ;
	work->control.hzx_base = work->control.mov.vy - work->control.height ;

	if ( time == 0 ) {
		SetAction( work, WMwalk, 6 ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
	}	
	ResetTurnX( work ) ;
	work->control.step.vy = -FALL_SPEED ;
	if ( work->control.mov.vy > HorizonLevel ) {
		EndWater( work ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, Water_Stand ) ;
	} else {
		if ( work->pad->press & PAD_DIVE ) {
			if ( CheckSwimEnableDir( work ) >= 0 ) {
				WaterCameraOn() ;
				SetMode( work, Water_toSwim ) ;
				return ;
			}
		}
		if ( work->pad->dir == -1 ) {
			SetMode( work, Water_Stand ) ;
		} else {
			if ( WaterCamera->on ) {
				WaterSeek( work ) ;
			} else {
				work->control.turn.vy = work->pad->dir ;
			}
		}
	}	
	if ( work->control.grounded & 1 ) WaterCameraOff() ;
	else if ( ++ work->data >= 30 ) WaterCameraOn() ;
}

/*---------------------------------------------------------------*/

/* ポーリング関数 */
static	int		CheckIntoWater( work )
PlayerWork		*work ;
{
	PreRot = work->control.rot ;
	if ( Status( PLAYER_FORCE | PLAYER_DAMAGED | PLAYER_DEAD | 	
				 PLAYER_DOWNED ) ) return 0 ;
	if ( !Status( PLAYER_IN_THE_WATER ) && 
		 HorizonLevel >= work->control.mov.vy ) {
		UnsetStatus( PLAYER_BEYOND | PLAYER_PAD_OFF ) ;
		PL_UnsetInvincible( work ) ;
		if ( WeaponType( work ) & WP_TYPE_SUBJECT ) PL_NoWeapon( work ) ;
		if ( ItemType( work ) & IT_TYPE_ABNORMAL ) PL_NoEquip( work ) ;
		PL_LeaveCaution( work ) ;
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		SetMode2( work, NULL ) ;
		/* モーションファイルチェンジ */
		PL_ChangeMotionArc( work, Motion ) ;
		/* 腰でニアチェック */
		work->control.skip_flag &= ~( CTRL_SKIP_HZX | CTRL_SKIP_TRAP ) ;
		SetStatus( PLAYER_IN_THE_WATER ) ;
		SetMode( work, Water_Floating ) ;
printf( "[%d] into water\n", GV_Time ) ;
		return 1 ;
	}
	return 0 ;
}

/*---------------------------------------------------------------*/

/* カメラ */
static	void	InitWaterCamera( void )
{
	GM_CameraSet	*cam ;
	int				name ;

	WaterCamera = NULL ;
	name = GV_StrCode( "水中カメラ" ) ;
	name += 0 ; /* チャンネル */
	cam = WaterCamera = NewProgramCamera( name, 0, GM_CAMERA_PROG2, 254 ) ;
	GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET,
					  CAM_FLAG_PAD_ADJUST | CAM_FLAG_NO_CUSHION ) ;
	GM_SetCameraTrack( cam, 2000.0F ) ;
	GM_SetCameraAngle( cam, 2.0F ) ;
	GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8,
						    GM_CAM_INTERP_BEHIND, 0, 0 ) ;
	DG_COPY_VEC( &cam->position, &GM_PlayerPosition ) ;
	DG_COPY_VEC( &cam->target, &GM_PlayerPosition ) ;
	cam->on = 0 ;
	ASSERT( WaterCamera != NULL ) ;
}

/* プラグイン登録 */
int		NewPluginWaterModeTest( void )
{
//    PL_AddPlugin( &PluginSet, PL_MSG_WATER, NULL, NULL ) ;
    PL_AddPollingFunc( &PollingSet, CheckIntoWater ) ;
    InitWaterCamera() ;
    return 0 ;
}

/* 設定 */
int		NewWaterModeSettingTest( void )
{
	if ( GCL_GetOption( 'h' ) != NULL ) {
		HorizonLevel = ( float )GCL_GetNextInt() ;
	}
	Motion = GCL_GetOptionValue( 'm', 0 ) ;
	ASSERT( Motion != 0 ) ;
	return 0 ;
}

