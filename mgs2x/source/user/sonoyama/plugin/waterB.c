/*
   waterB.c
   水中モードプラグインのバックアップ

   2001/04/23 M.Sonoyama
   $Id: waterB.c,v 1.1.1.3 2002/11/19 11:50:51 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef _XBOX 
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
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

enum rai_swim {
	WMtread_idle = 0,
	WMtread,
	WMtread2dive,
	WMdive,
	WMdive2tread,
	WMbreast,
	WMbreast2stand,
	WMstand,
	WMseabed_idle,
	WMseabed_walk,
	WMseabed2dive,
	WMstand2dive,
	WMdive_arm_r,
	WMdive_arm_l,
	WMdive_end,
	WMwall_idle,
	WMwall_touch,
	WMwall_turn_r,
	WMwall_turn_l,
	WMwall_turn_b,
	WMdive_max,
	WMdive_end_st,
	WMdam_stand,
	WMdam_lie,
	WMout_stand,
	WMout_lie,
	WMdozaemon,
	WMnoair_stand,
	MAX_WATER_MOTIONS
} ;

/* フラグ */
enum {
	WATER_FLAG_NORMAL		= 	0x0000,
	WATER_FLAG_AUTO_ROTATE	=	0x0001,		//角度自動調節機能付き
	WATER_FLAG_NO_ZONE_TRACE =	0x0002,		//ゾーンを使用した追従をしない
} ;

#define	A_PATTERN
//#define	B_PATTERN

#define	PAD_SWIM		(PAD_A)
#define	PAD_DIVE		(PAD_Y)
#define	PAD_MODE		(PAD_B)

#define	PAD_UPPER		(PAD_A)
#define	PAD_LOWER		(PAD_B)
#define	PAD_PRESS_UPPER		(PAD_PRESS_A)
#define	PAD_PRESS_LOWER		(PAD_PRESS_B)

#define	PAD_HEAD_UP			(PAD_U)
#define	PAD_HEAD_DOWN		(PAD_D)
#define	PAD_PRESS_HEAD_UP	(PAD_PRESS_U)
#define	PAD_PRESS_HEAD_DOWN	(PAD_PRESS_D)

#define	PAD_FAST		(PAD_A)

//#define	PAD_SWIM		(PAD_B)

#define	CHANGE_MRATE	(0.80F)

#define	CAMERA_TURNX		(64)
#define	CAMERA_TURNX_ABS	(64)
#define	CAMERA_TRACK_NEAR		(1250.0F)
#define	CAMERA_TRACK_NORMAL		(1500.0F)
#define	CAMERA_TRACK_FAR		(3000.0F)
#define	WATER_TURNX_LIMIT	(960-CAMERA_TURNX_ABS)

#define	HEAD	(1000.0F)
#define	FOOT	(1200.0F)

#define	FALL_SPEED			(8.0F)
#define	FALL_SPEED_BOTTOM	(16.0F)
#define	FLOAT_SPEED			(0.0F)

#define	FLOAT_HEIGHT		(444.637696F)	/* 立ち泳ぎ静止モーションの
											   頭の位置と腰との高さの差 */

#define	FLOAT_NEAR_CHK_HEIGHT	(1650.0F)

#define	CAM_INTERP_SPEED		(24.0F)
#define	CAM_INTERP_SPEED_H		(4.0F)

#define	MOT_INTERP_DEF			(24)

extern	float	GM_WaterLevel ;
extern	void	PL_AddDeadAction( PL_DeadMode *this, PL_ACTION action, PL_DEADMODE_COND_FUNC cond ) ;
extern	void	PL_RemoveDeadAction( PL_DeadMode *this ) ;

static	PL_PluginSet	PluginSet ;
static	PL_PollingSet	PollingSet ;
static	PL_DeadMode		DeadModeSet ;

static	int				Motion = 0 ;
static	float			HorizonLevel ;
static	float			HorizonLevel2 ;
static	int				WaterFlag = 0 ;
static	int				WaterStance ;

static	GM_CameraSet	*WaterCamera ;

static	int				ModeFlag ;

static	int				TurnXLimitU = WATER_TURNX_LIMIT ;
static	int				TurnXLimitD = -WATER_TURNX_LIMIT ;

static	int				WaterZoneGroupID = 0 ;
static	int				SeekCount = 0 ;

static	int				CamMode = 0 ;
static	int				CamClear = 0 ;

static	FVECTOR			CamTarget ;
static	SVECTOR			CamRot, NCamRot, CamRotAim ;
static	int				CamNoRzCount = 0 ;
static	float			CamTrack, CamTrackAim ;
static	float			CamAngle, CamAngleAim ;

static	FVECTOR			TrapCheckPos, TrapCheckAim, TrapCheckRootDiff ;

static	int				Elude2WaterMotion = -1 ;
static	int				Dead2WaterMotion = -1 ;

#ifdef DEBUG_MODE
static	RADAR_CTRL		Rctrl ;
#endif

/*---------------------------------------------------------------*/

enum {
	WS_SURFACE = 0,
	WS_STAND,
	WS_SWIM,
} ;

enum {
	MF_NONE	=			0x0000,
	MF_STAND =			0x0001,
	MF_RTURN =			0x0002,
	MF_USE_NCAMROT =	0x0004,
	MF_NO_CAMERA_TURN =	0x0008,
	MF_NO_CAMERA_RZ =	0x0010,
	MF_TURN_NOW =		0x0020,
	MF_CLEAR =			0x00ff,
	MF_SEEKDOWN_NEW_PRESS =		0x0100,
} ;

/*---------------------------------------------------------------*/

/*---------------------------------------------------------------

  プロトタイプ宣言 

---------------------------------------------------------------*/

static	void	AdjustObject( PlayerWork *work, FVECTOR *shift ) ;

static	void	TreadStill( PlayerWork *work, int time ) ;
static	void	TreadMove( PlayerWork *work, int time ) ;
static	void	Tread2Dive( PlayerWork *work, int time ) ;
static	void	Dive2Tread( PlayerWork *work, int time ) ;
static	void	Dive2Stand( PlayerWork *work, int time ) ;
static	void	WaterStand( PlayerWork *work, int time ) ;
static	void	Stand2Dive( PlayerWork *work, int time ) ;
static	void	SeabedStill( PlayerWork *work, int time ) ;
static	void	SeabedWalk( PlayerWork *work, int time ) ;
static	void	Seabed2Dive( PlayerWork *work, int time ) ;
static	void	Dive( PlayerWork *work, int time ) ;
static	void	WaterRise( PlayerWork *work, int time ) ;
//static	void	Submerge( PlayerWork *work, int time ) ;
static	void	Dive2WallTouch( PlayerWork *work, int time ) ;
static	void	WallTouch( PlayerWork *work, int time ) ;
static	void	TouchTurnR( PlayerWork *work, int time ) ;
static	void	TouchTurnL( PlayerWork *work, int time ) ;
static	void	TouchTurnB( PlayerWork *work, int time ) ;

static	void	WaterDamageCallback( PlayerWork *work, TARGET *off, TARGET *def ) ;
static	void	WaterCheckDamage( PlayerWork *work, long64 weapon_type, int dead ) ;

static	void	WaterDamageStand( PlayerWork *work, int time ) ;
static	void	WaterDamageLie( PlayerWork *work, int time ) ;
static	void	WaterDeadStand( PlayerWork *work, int time ) ;
static	void	WaterDeadLie( PlayerWork *work, int time ) ;
static	void	WaterDozaemon( PlayerWork *work, int time ) ;
static	void	WaterNoO2Dead( PlayerWork *work, int time ) ;

static	void	IntoWater( PlayerWork *work, int time ) ;
static	void	LeaveWater( PlayerWork *work, int time ) ;

static	void	EludeFallDeadWater( PlayerWork *work, int time ) ;

/* 息継ぎポイント吸い込み処理 */
static	void  		GuideToBreathPoint( FVECTOR *step, FVECTOR *mov, float guide_len, float step_len ) ;

/* 溺れゲームオーバー関連 */
static	int		WaterDeadModeConditionFunc( PlayerWork *work ) ;
static	void	WaterDeadModeNoO2( PlayerWork *work, int time ) ;

/*---------------------------------------------------------------

  インライン関数／デバッグ関数

---------------------------------------------------------------*/

/* デバッグ水面表示 */
#ifdef DEBUG_MODE
#if 0
static	void	ViewHorizon( void )
{
	static FVECTOR		v[ 2 ] ;

	v[ 0 ].vx = -200000.0F ;
	v[ 0 ].vy = -400000.0F ;
	v[ 0 ].vz = -400000.0F ;
	v[ 1 ].vx = 200000.0F ;
	v[ 1 ].vy = HorizonLevel ;
	v[ 1 ].vz = 200000.0F ;
	NewBoundingBoxView( &v[ 0 ], &v[ 1 ], 32, 232, 32 ) ;
}
#else
#define	ViewHorizon()
#endif
#else
#define	ViewHorizon()
#endif

/* カメラのＯＮ／ＯＦＦ */
static	inline	void	WaterCameraOn( void )
{
//	GM_CameraSet		*now ;

	if ( WaterCamera->on == 1 ) return ;

//	now = GM_GetCurrentCamera( 0 ) ;
//	DG_COPY_VEC( &WaterCamera->position, &now->position ) ;
//	DG_COPY_VEC( &WaterCamera->target, &now->target ) ;

	WaterCamera->on = 1 ;
	GM_ChangeCamera( 0 ) ;
	CamClear = 1 ;
	CamMode = 0 ;
	DG_COPY_VEC( &CamTarget, &GM_PlayerPosition ) ;
	CamRot.vy = GM_PlayerControl->rot.vy ;
    CamTrack = CamTrackAim = CAMERA_TRACK_NORMAL ;
    CamRot.vx = CamRotAim.vx = CAMERA_TURNX ;
}

static	inline	void	WaterCameraOff( void )
{
	if ( WaterCamera->on == 0 ) return ;
	WaterCamera->on = 0 ;
	GM_ChangeCamera( 0 ) ;
	if ( !Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		DG_VisibleObjsChanl( GM_PlayerBody->objs, GM_PlayerWork->chanl ) ;
	}
}

/* 当たりチェック高さ */
static	inline	void	SetHzxHeight( PlayerWork *work, float height )
{
	work->hzx_height = work->control.hzx_height = height ;
}

/* 水面高さ */
static	inline	float	HL( void )
{
	return HorizonLevel2 ;
}

/* カメラ補完 */
static	inline	void	CamPosInterp( FVECTOR *f, FVECTOR *t )
{
	float		vf ;
	GV_PAD		*pad ;

	pad = GM_PlayerWork->pad ;
	if ( pad->status & PAD_L1 ) vf = CAM_INTERP_SPEED_H ;
	else							  vf = CAM_INTERP_SPEED ;
	GV_NearExpNVF( f, t, vf, 3 ) ;
}

/* 大移動量チェック */
static	void	LargeMoveAdjust( PlayerWork *work )
{
	float		mot_len, obj_move ;
	FVECTOR		mov, nmov, diff ;
	int			c ;

	mot_len = GV_VecLen3F( &work->body.m_ctrl->step ) ;
	GV_MatToVec( &work->body.objs->world, &mov ) ;
	obj_move = GV_VecLen3F2( &work->pre_body_mov, &mov ) ;
	if ( mot_len + FALL_SPEED_BOTTOM < obj_move ) {
		DG_COPY_VEC( &nmov, &work->pre_body_mov ) ;
		if ( GV_VecLen3F( &work->control.step ) < 4.0F ) {
			if ( obj_move > 300.0F ) GV_NearExp2VF( &nmov, &mov, 3 ) ;
			else					 GV_NearExp4VF( &nmov, &mov, 3 ) ;
			c = 1 ;
		} else {
			if ( obj_move > 300.0F ) GV_NearExp4VF( &nmov, &mov, 3 ) ;
			else				     GV_NearExp8VF( &nmov, &mov, 3 ) ;
			c = 2 ;
		}
//if ( obj_move > 200.0F ) printf( "<%d>largemov %d : %f %f\n", GV_Time, c, mot_len, obj_move ) ;
		_sceVu0SubVector( &diff, &nmov, &mov ) ;
		AdjustObject( work, &diff ) ;
	}
}

/* 天井床幅チェック */
static	int		CeilToFloor( PlayerWork *work )
{
	float		levels[ 2 ] ;
	int			flag ;

	flag = HZX_LevelHazardCheck( work->control.hzx_id, &work->control.mov, 
								 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
	if ( flag == 0 ) return 0 ;
	HZX_GetLevelHeight( levels ) ;
	if ( !( flag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( !( flag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;	

	if ( levels[ 1 ] - levels[ 0 ] < 2000.0F ) {
//printf( "ceil to floor %f\n", levels[ 1 ] - levels[ 0 ] ) ;
		return 1 ;
	}
	return 0 ;
}

/*---------------------------------------------------------------

  操作関数

---------------------------------------------------------------*/

/* 追従カメラ位置計算 */

static	float	RAIL_HEIGHT = 1000.0F ;

static	int		NavigateCamera( GM_CameraSet *cam,
							    FVECTOR *cpos, FVECTOR *npos,
							    FVECTOR *ctrg, FVECTOR *ntrg )
{
	HZX_GROUP_ID	hzx_id ;
	int			czNo, nzNo ;
	HZX_ZON		*cz, *nz ;
	FVECTOR		czc, nzc ;
	int			again = 0 ;

	if ( WaterFlag & WATER_FLAG_NO_ZONE_TRACE ) return -1 ;
	hzx_id = WaterZoneGroupID ;
	if ( hzx_id == 0 ) {
		hzx_id = WaterZoneGroupID = GM_PlayerWork->control.hzx_id ;
	}
	nzNo = HZX_GetLowerZone( hzx_id, npos ) ;
	if ( nzNo < 0 ) {
		printf( "[%d] next zone is ???\n", GV_Time ) ;
		return -1 ;
	}
	nz = HZX_GetZone( hzx_id, nzNo ) ;
	GV_SetVec3( &nzc, nz->x, nz->y + RAIL_HEIGHT, nz->z ) ;
check_again :
	if ( HZX_OnlineHazardCheck( hzx_id, cpos, &nzc, HZX_CHK_ALL, 
							    HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
		if ( again == 0 ) {
			czNo = HZX_GetLowerZone( hzx_id, cpos ) ;
			if ( czNo < 0 ) {
				printf( "[%d] now zone is ???\n", GV_Time ) ;
				return -1 ;
			}
			cz = HZX_GetZone( hzx_id, czNo ) ;
			GV_SetVec3( &czc, cz->x, cz->y + RAIL_HEIGHT, cz->z ) ;
		}
		if ( again == 0 && 
			 HZX_OnlineHazardCheck( hzx_id, &czc, &nzc, HZX_CHK_ALL, 
								    HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
			/* 現在のゾーンと目標ゾーンの間に壁がある */
			czNo = HZX_AddressNo( GV_GetNo( hzx_id ), czNo, czNo ) ;
			nzNo = HZX_AddressNo( GV_GetNo( hzx_id ), nzNo, nzNo ) ;
			nzNo = HZX_Navigate( czNo, nzNo, cpos ) ;
			nz = HZX_GetZone( hzx_id, nzNo & 255 ) ;
			GV_SetVec3( &nzc, nz->x, nz->y + RAIL_HEIGHT, nz->z ) ;
			again = 1 ;
//printf( "[%d] navigate aim %x\n", GV_Time, nzNo & 255 ) ;
			goto check_again ;
		} else {
			if ( CamClear == 1 ) {
				DG_COPY_VEC( &cam->position, &czc ) ;
			} else {
				CamPosInterp( cpos, &czc ) ;
				DG_COPY_VEC( &cam->position, cpos ) ;
			}
			DG_COPY_VEC( &cam->target, ntrg ) ;
//printf( "[%d] to now zone %x\n", GV_Time, czNo & 255 ) ;
		}
	} else {
		if ( CamClear == 1 ) {
			DG_COPY_VEC( &cam->position, &nzc ) ;
		} else {
			CamPosInterp( cpos, &nzc ) ;
			DG_COPY_VEC( &cam->position, cpos ) ;
		}
		DG_COPY_VEC( &cam->target, ntrg ) ;
//printf( "[%d] to next zone %x\n", GV_Time, nzNo & 255 ) ;
	}
	return 0 ;
}

#define	TRACE_CUSHION	(150.0F)
#define	TRACE_CUSHION_Y	(150.0F)
//#define	TYPE_A
#define	TYPE_B

#define	BOUND_ADJ		(250.0F)
#define	BOUND_LEN		(6000.0F) 

static	void	UpdateCamera( PlayerWork *work )
{
    static FVECTOR	Cushion = { TRACE_CUSHION, TRACE_CUSHION_Y,	TRACE_CUSHION } ;
	GM_CameraSet	*cam ;
	FVECTOR			cpos, ctrg, npos, ntrg ;
	SVECTOR			nrot ;
	FVECTOR			vec ;
	FVECTOR			bound1, bound2 ;
	HZX_GROUP_ID	hzx_id ;
	int				ntrack ;
	int				ry_range ;

	cam = WaterCamera ;
	if ( cam->on == 0 ) {
		return ;
	}
	if ( !( ModeFlag & MF_STAND ) ) {
//		GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;
        DG_COPY_VEC( &work->camera, &work->control.mov ) ;
	}

	hzx_id = work->control.hzx_id ;

	/* 現在のカメラデータ */
	DG_COPY_VEC( &cpos, &cam->position ) ;
	DG_COPY_VEC( &ctrg, &cam->target ) ;

	/* カメラＺ回転 */
	if ( !( ModeFlag & MF_NO_CAMERA_RZ ) && CamNoRzCount == 0 ) {
		int			d1, d2, diff ;

		d1 = GV_VecDir2FromTo( &cpos, &ctrg ) ;
		d2 = work->control.rot.vy ;
		diff = GV_DiffDirS( d1, d2 ) ;
		if ( diff < -32 || diff > 32 ) {
			cam->rotate.vz = GV_NearExp16P( cam->rotate.vz, - diff / 5 ) ;
		} else {
			cam->rotate.vz = GV_NearExp16P( cam->rotate.vz, 0 ) ;
		}
	} else {
		cam->rotate.vz = GV_NearExp16P( cam->rotate.vz, 0 ) ;
	}
	if ( CamNoRzCount > 0 ) -- CamNoRzCount ;

	/* 次のカメラデータ */
	ry_range = ( work->pad->status & PAD_UDLR ) ? 256 : 32 ;
	if ( ModeFlag & MF_USE_NCAMROT ) {
		CamRot.vy = GV_NearRangeP( GV_NearPhase( CamRot.vy, NCamRot.vy ),
								  NCamRot.vy, ry_range ) ;
	} else {
		CamRot.vy = GV_NearRangeP( GV_NearPhase( CamRot.vy, work->control.turn.vy ),
								  work->control.turn.vy, ry_range ) ;
	}

	CamRot.vx = GV_NearExp8P( CamRot.vx, work->control.rot.vx + CamRotAim.vx ) ;
	if ( CamRot.vx > 2048 ) CamRot.vx -= 4096 ;
	else if ( CamRot.vx < -2048 ) CamRot.vx += 4096 ;
	if ( CamRot.vx < -960 ) CamRot.vx = -960 ;
	else if ( CamRot.vx > 960 ) CamRot.vx = 960 ;

	GV_SetVec3( &nrot, CamRot.vx, CamRot.vy, 0 ) ;
	CamRotAim.vx = CAMERA_TURNX ;

	DG_COPY_VEC( &ntrg, &ctrg ) ;
	GV_NearExp8VF( &CamTarget, &work->camera, 3 ) ;
	GM_CameraCushionTrace( &ntrg, &CamTarget, &Cushion ) ;

	CamTrack = GV_NearExp16F( CamTrack, CamTrackAim ) ;
	CamTrackAim = CAMERA_TRACK_NORMAL ;
	ntrack = CamTrack ;
//	ntrack = CAMERA_TRACK ;
	GM_CameraMakeCamera( &npos, &ntrg, &nrot, &ntrack ) ;

	CamAngle = GV_NearExp16F( CamAngle, CamAngleAim ) ;
	
	CamAngleAim = 1.60F ;
	cam->angle = CamAngle ;

	GV_SetVec3( &bound1, -1000000.0F, -1000000.0F, -1000000.0F ) ;
	GV_SetVec3( &bound2, 1000000.0F, 1000000.0F, 1000000.0F ) ;

	if ( HZX_OnlineHazardCheck( hzx_id, &ntrg, &npos,
							    HZX_CHK_ALL,
							    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_RECOIL_TYPE ) ) {
		HZX_GetOnlineVector( &vec ) ;
		GV_LenVec3F( &vec, &vec, 0.0F, GV_VecLen3F( &vec ) - 10.0F ) ;
		_sceVu0AddVector( &npos, &ntrg, &vec ) ;
		/* Ｘ方向バウンドチェック */
		GV_SetVec3( &vec, -BOUND_LEN, 0.0F, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vx = vec.vx + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, BOUND_LEN, 0.0F, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vx = vec.vx - BOUND_ADJ ;
		}
		/* Ｚ方向バウンドチェック */
		GV_SetVec3( &vec, 0.0F, 0.0F, -BOUND_LEN ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vz = vec.vz + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, 0.0F, 0.0F, BOUND_LEN ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vz = vec.vz - BOUND_ADJ ;
		}
		/* Ｙ方向バウンドチェック */
		GV_SetVec3( &vec, 0.0F, -BOUND_LEN, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vy = vec.vy + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, 0.0F, BOUND_LEN, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vy = vec.vy - BOUND_ADJ ;
		}
	}
#if 1
	/* 水面からでないように */
	if ( bound2.vy > HorizonLevel && 
		 bound1.vy < HorizonLevel ) {
		FVECTOR		npostmp, b2 ;

		DG_COPY_VEC( &npostmp, &npos ) ;
		DG_COPY_VEC( &b2, &bound2 ) ;
		b2.vy = HorizonLevel ;
		GM_CameraBoundTrace( &npostmp, &bound1, &b2 ) ;
		if ( !HZX_OnlineHazardCheck( hzx_id, &npostmp, &ntrg, 
								    HZX_CHK_ALL,
								    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
								    HZX_FLOOR_RECOIL_TYPE ) ) {		
			bound2.vy = HorizonLevel ;
		}
	}
#endif
	GM_CameraBoundTrace( &npos, &bound1, &bound2 ) ;
	if ( CamClear == 1 ) DG_COPY_VEC( &cpos, &npos ) ;

	if ( HZX_OnlineHazardCheck( hzx_id, &npos, &cpos, 
							    HZX_CHK_ALL,
							    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI,
							    HZX_FLOOR_RECOIL_TYPE ) ) {
		/* 現在のカメラと次のカメラの間に壁がある。 */
		if ( NavigateCamera( cam, &cpos, &npos,
							 &ctrg, &ntrg ) < 0 ) {
			/* ナビゲートがうまくいかない */
			printf( "[%d]navigate failed\n", GV_Time ) ;
			if ( CamClear == 1 ) {
				DG_COPY_VEC( &cam->position, &npos ) ;
			} else {
				CamPosInterp( &cpos, &npos ) ;
				DG_COPY_VEC( &cam->position, &cpos ) ;
			}
			DG_COPY_VEC( &cam->target, &ntrg ) ;
		}
	} else {
		/* 壁はない */
	    if ( CamClear == 1 ) {
			DG_COPY_VEC( &cam->position, &npos ) ;
		} else {
			CamPosInterp( &cpos, &npos ) ;
			DG_COPY_VEC( &cam->position, &cpos ) ;
		}
		DG_COPY_VEC( &cam->target, &ntrg ) ;
//        printf( "[%d]normal\n", GV_Time ) ;
	}
	CamClear = 0 ;
#ifdef DEBUG_MODE
#if 0
	if ( HZX_OnlineHazardCheck( hzx_id, &cam->target, &cam->position,
							    HZX_CHK_ALL,
							    HZX_SEG_RECOIL_TYPE | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_RECOIL_TYPE ) ) {
		FVECTOR			hit ;

		printf( "[%d] hazard\n", GV_Time ) ;
		DumpVec( &cam->position ) ;
		DumpVec( &cam->target ) ;
		HZX_GetOnlinePoint( &hit ) ;
		DumpVec( &hit ) ;
	}
#endif
	GM_RadarSetSight( &Rctrl, 0, 768, 1000.0F, RADAR_COLOR_RED ) ;
#endif
}

/* 右スティックカメラターン */
static	void	SeekCameraTurn( PlayerWork *work )
{
	static int	Rstate = 0, Press = 0 ;
	int			state, press ;
	float		rx, ry ;

	if ( !( work->pad->analog_input & GV_PAD_ANALOG_R_USE ) ||
		 ( ModeFlag & MF_NO_CAMERA_TURN ) ||
		 Status( PLAYER_WATCH | PLAYER_DAMAGED | PLAYER_DEAD ) ||
		 CeilToFloor( work ) ) {
		Rstate = 0 ;
		return ;
	}
	if ( GV_Time - Press < 16 ) {
		return ;
	}
	rx = ( float )( work->pad->right_dx - 128 ) ;
	ry = ( float )( work->pad->right_dy - 128 ) ;

	state = 0 ;
	if ( DG_FABS( rx ) > ANALOG_MARGIN_F ) {
		if ( rx < 0.0F ) state |= PAD_L ;
		else state |= PAD_R ;
	}
	if ( DG_FABS( ry ) > ANALOG_MARGIN_F ) {
		if ( ry < 0.0F ) state |= PAD_U ;
		else state |= PAD_D ;
	}
	press = state & ~Rstate ;
	if ( press & PAD_D ) {
//		work->control.turn.vy += 2048 ;
		ModeFlag |= MF_RTURN ;
        if ( work->action != TouchTurnR ) SetMode( work, TouchTurnB ) ;
	} else if ( press & PAD_L ) {
//		work->control.turn.vy += 1024 ;
		ModeFlag |= MF_RTURN ;
		if ( work->action != TouchTurnL ) SetMode( work, TouchTurnL ) ;
	} else if ( press & PAD_R ) {
//		work->control.turn.vy -= 1024 ;
		ModeFlag |= MF_RTURN ;
        if ( work->action != TouchTurnR ) SetMode( work, TouchTurnR ) ;
    }
	Rstate = state ;
	Press = GV_Time ;
}

static	FVECTOR		StepAdjust = { 0.0F, 0.0F, 0.0F, 1.0F } ;
static	FVECTOR		StepAdjust2 = { 0.0F, 0.0F, 0.0F, 1.0F } ;

static	FVECTOR		SinkStepAdjust = { 0.0F, 0.0F, 0.0F, 1.0F } ;
static	FVECTOR		SinkStepAdjust2 = { 0.0F, 0.0F, 0.0F, 1.0F } ;

/* イース風キャラずらし */
static	int		AdjustLikeYs( PlayerWork *work, FVECTOR *hit, HZX_HZD *hzd, FVECTOR *react,
							  FVECTOR *nfrom )
{
	FVECTOR		chk_mov, pole, step, rn ;
//	FVECTOR		nto ;
	FVECTOR		chk[ 4 ] ;
	SVECTOR		rot ;
	float		z, ip ;
	int			i ;

	PL_ObjPos( work, HUMAN21_KOSHI, &chk_mov ) ;
	HZX_GetNormal( &pole, hzd ) ;
	GV_VecToRot( &pole, &rot ) ;

	/* あたった面の法線と反発ベクトルが垂直に近いときは
	   リターン */
	_sceVu0Normalize( &pole, &pole ) ;
	_sceVu0Normalize( &rn, react ) ;
	ip = _sceVu0InnerProduct( &rn, &pole ) ;
	if ( DG_FABS( ip ) < 0.10F ) {
		printf( "adjust cancel %f\n", ip ) ;
		return 0 ;
	}

#if 0
	if ( _sceVu0InnerProduct( react, &pole ) > 0.0F ) z = -500.0F ;
	else											  z = 500.0F ;
#else
	z = 750.0F ;
#endif
	GV_SetVec3( &chk[ 0 ], -1250.0F, 0.0F, z ) ;
	GV_SetVec3( &chk[ 1 ], 1250.0F, 0.0F, z ) ;
	GV_SetVec3( &chk[ 2 ], 0.0F, 1250.0F, z ) ;
	GV_SetVec3( &chk[ 3 ], 0.0F, -1250.0F, z ) ;
#if 0
	DG_SetPos2( hit, &rot ) ;
#else
	DG_SetPos2( hit, &work->control.rot ) ;
#endif
	DG_PutVector( chk, chk, 4 ) ;
	for ( i = 0; i < 4; i ++ ) {
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &chk_mov, &chk[ i ],
								    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
								    HZX_FLOOR_NO_PLAYER ) ) {
//			ViewFromTo( &chk_mov, &chk[ i ], 32, 232, 32 ) ;
		} else {
//			ViewFromTo( &chk_mov, &chk[ i ], 32, 32, 232 ) ;
			_sceVu0SubVector( &step, &chk[ i ], hit ) ;
			ip = _sceVu0InnerProduct( &step, &pole ) ;
			_sceVu0ScaleVector( &pole, &pole, ip ) ;
			_sceVu0SubVector( &step, &step, &pole ) ;
#if 0
			{
				FVECTOR		to ;
				_sceVu0AddVector( &to, hit, &step ) ;
				ViewFromTo( hit, &to, 232, 32, 32 ) ;
			}
#endif
			GV_LenVec3F( &step, &step, 0.0F, 96.0F ) ;
#if 0		/* 必要ない？ */
            _sceVu0AddVector( &nto, nfrom, &step ) ;
            if ( HZX_OnlineHazardCheck( work->control.hzx_id, nfrom, &nto,
									    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
									    HZX_FLOOR_NO_PLAYER ) ) {
				return 0 ;
			}
#endif
 
            if ( !( work->pad->status & ( PAD_L | PAD_R ) ) ) {
				int			dir ;

				dir = GV_VecDir2( &step ) ;
//printf( "<ys adjust> %d %d\n", work->control.turn.vy, dir ) ;
				work->control.turn.vy = GV_NearSpeedP( work->control.turn.vy, dir, 32 ) ;
            }
//            if ( i == 0 ) work->control.turn.vz = 256 ;
//            else if ( i == 1 ) work->control.turn.vz = -256 ;

//			_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
			DG_COPY_VEC( &StepAdjust, &step ) ;
//printf( "<%d>ys adjust %f %f %f\n", GV_Time, step.vx, step.vy, step.vz ) ;
			return 1 ;
		}
	}
	return 0 ;
}

/* めりこみ回避処理 */
static	inline	int		Rot2048( short r )
{
	r &= 4095 ;
	if ( r > 2048 ) r -= 4096 ;
	return r ;
}

static	void	AdjustObject( PlayerWork *work, FVECTOR *shift )
{
	int			i ;
	DG_OBJS		*objs ;
//	float		len ;
	FVECTOR		s ;

	DG_COPY_VEC( &s, shift ) ;
//	len = GV_VecLen3F( &s ) ;
//	if ( len > 64.0F ) GV_LenVec3F( &s, &s, 0.0F, 64.0F ) ;
	objs = work->body.objs ;
	objs->world.m[ 3 ][ 0 ] += s.vx ;
	objs->world.m[ 3 ][ 1 ] += s.vy ;
	objs->world.m[ 3 ][ 2 ] += s.vz ;
	for ( i = 0; i < objs->def->n_models; i ++ ) {
		objs->objs[ i ].world.m[ 3 ][ 0 ] += s.vx ;
		objs->objs[ i ].world.m[ 3 ][ 1 ] += s.vy ;
		objs->objs[ i ].world.m[ 3 ][ 2 ] += s.vz ;
	}		
}

static	void	AvoidTurnforSwim( PlayerWork *work, HZX_HZD *hzd, FVECTOR *react, 
								  FVECTOR *nhead, int mode, int nostep )
{
	FVECTOR		pole, react2, vec, to ;
	SVECTOR		rot ;
	int			ip, c ;
	short		turn ;

	c = 0 ;
	HZX_GetNormal( &pole, hzd ) ;
	_sceVu0Normalize( &pole, &pole ) ;
	DG_COPY_VEC( &react2, react ) ;
	_sceVu0ScaleVector( &react2, &react2, -1.0F ) ;
	ip = _sceVu0InnerProduct( &react2, &pole ) ;
	_sceVu0ScaleVector( &pole, &pole, ip ) ;
	_sceVu0SubVector( &vec, &react2, &pole ) ;

	GV_VecToRot( &vec, &rot ) ;

	if ( hzd->type == HZX_TYPE_FLOOR ) {
		if ( DG_FABS( pole.vx ) > 0.01F && DG_FABS( pole.vz ) > 0.01F &&
			GV_DiffDirAbs( work->control.rot.vy, rot.vy ) > 1024 ) {
			_sceVu0ScaleVector( &react2, &react2, -1.0F ) ;
			_sceVu0Normalize( &pole, &pole ) ;
			ip = _sceVu0InnerProduct( &react2, &pole ) ;
			_sceVu0ScaleVector( &pole, &pole, ip ) ;
			_sceVu0SubVector( &vec, &react2, &pole ) ;
			GV_VecToRot( &vec, &rot ) ;
#if 0
			{
				FVECTOR	vto, vec2 ;

				GV_LenVec3F( &react2, &vec2, 0.0F, 1000.0F ) ;
				_sceVu0AddVector( &vto, nhead, &vec2 ) ;
				ViewFromTo( nhead, &vto, 32, 232, 32 ) ;

				GV_LenVec3F( &pole, &vec2, 0.0F, 1000.0F ) ;
				_sceVu0AddVector( &vto, nhead, &vec2 ) ;
				ViewFromTo( nhead, &vto, 232, 32, 32 ) ;

				GV_LenVec3F( &vec, &vec2, 0.0F, 1000.0F ) ;
				_sceVu0AddVector( &vto, nhead, &vec2 ) ;
				ViewFromTo( nhead, &vto, 32, 32, 232 ) ;
			}
			printf( "[%d] gyakuten\n", GV_Time ) ;
#endif
		}
	}

	GV_LenVec3F( &vec, &vec, 0.0F, GV_VecLen3F( &vec ) * 12.0F ) ;
	if ( GV_VecLen3F( &vec ) > 256.0F ) {
		GV_LenVec3F( &vec, &vec, 0.0F, 256.0F ) ;
	}
	_sceVu0AddVector( &to, nhead, &vec ) ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, nhead, &to, 
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		printf( "[%d] nogerahan\n", GV_Time ) ;
		HZX_GetOnlineVector( &vec ) ;
//		return ;
	}

	if ( !( ModeFlag & MF_RTURN ) &&	/* 右スティック旋回中は回転処理しない */
		 GV_DiffDirS( work->control.rot.vy, work->control.turn.vy ) *
		 GV_DiffDirS( work->control.rot.vy, rot.vy ) >= 0 ) {
		turn = GV_NearExp4P( work->control.rot.vy, rot.vy ) ;
		if ( GV_DiffDirAbs( turn, work->control.rot.vy ) > 
			 GV_DiffDirAbs( work->control.turn.vy, work->control.rot.vy ) ) {
//printf( "<avoid turn> %d %d\n", work->control.turn.vy, turn ) ;
//			work->control.turn.vy = turn ;
            work->control.turn.vy = GV_NearExp4P( work->control.turn.vy, turn ) ;
			c = 1 ;
		}
	} 
#if 0
	if ( GV_DiffDirS( work->control.rot.vx, work->control.turn.vx ) *
		 GV_DiffDirS( work->control.rot.vx, rot.vx ) >= 0 ) {
		turn = GV_NearExp4P( work->control.rot.vx, rot.vx ) ;
		if ( GV_DiffDirAbs( turn, work->control.rot.vx ) > 
			 GV_DiffDirAbs( work->control.turn.vx, work->control.rot.vx ) ) {
			work->control.turn.vx = turn ;
			work->control.turn.vx &= 4095 ;
			if ( work->control.turn.vx > 2048 ) work->control.turn.vx -= 4096 ;
			if ( work->control.turn.vx < -WATER_TURNX_LIMIT ) {
				work->control.turn.vx = -WATER_TURNX_LIMIT ;
			} else if ( work->control.turn.vx > WATER_TURNX_LIMIT ) {
				work->control.turn.vx = WATER_TURNX_LIMIT ;
			}
			c = 1 ;
		}
	}
#endif
	c = 1 ;
	if ( c && !nostep ) {
//		_sceVu0AddVector( &work->control.step, &work->control.step, &vec ) ;
//printf( "<%d>avoid turn %f %f %f\n", GV_Time, vec.vx, vec.vy, vec.vz ) ;
		DG_COPY_VEC( &StepAdjust, &vec ) ;
	}
}

static	int		AvoidDir, AvoidDirX, LastAvoidSinkTime ;

static	int		AvoidSinkforSwim( PlayerWork *work )
{
	FVECTOR		head, foot ;
	FVECTOR		nhead ;
//	FVECTOR		nfoot :
	FVECTOR		front_adj, back_adj ;
	FVECTOR		front_hit, back_hit ;
	HZX_HZD		front_hzd, back_hzd ;
	float		front, back ;
	int			res, c ;

	if ( LastAvoidSinkTime != GV_Time - 1 ) {
		DG_COPY_VEC( &StepAdjust, &DG_ZeroVector ) ;
		DG_COPY_VEC( &SinkStepAdjust, &DG_ZeroVector ) ;
	}
	DG_COPY_VEC( &StepAdjust2, &DG_ZeroVector ) ;
	DG_COPY_VEC( &SinkStepAdjust2, &DG_ZeroVector ) ;

//	PL_GetModelLength( work, &front, &back ) ;
//	GV_SetVec3( &head, 0.0F, 0.0F, front + 200.0F ) ;
//	GV_SetVec3( &foot, 0.0F, 0.0F, -back - 50.0F ) ;
	GV_SetVec3( &head, 0.0F, 0.0F, 1000.0F ) ;
	GV_SetVec3( &foot, 0.0F, 0.0F, -1000.0F ) ;

#if 0
	{
		FVECTOR		v ;
		SVECTOR		rot ;

		PL_ObjPos( work, HUMAN21_ATAMA, &v ) ;
		_sceVu0SubVector( &v, &v, &work->control.mov ) ;
		GV_VecToRot( &v, &rot ) ;
		DG_SetPos2( &work->control.mov, &rot ) ;
	}
#endif
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;

	DG_PutVector( &head, &head, 1 ) ;
	DG_PutVector( &foot, &foot, 1 ) ;

	res = 0 ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &head,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineHazard( &front_hzd, &front_hzd.attribute ) ;
		HZX_GetOnlinePoint( &front_hit ) ;
		_sceVu0SubVector( &front_adj, &front_hit, &head ) ;
		GV_LenVec3F( &front_adj, &front_adj, 0.0F, GV_VecLen3F( &front_adj ) + 10.0F ) ;
		res = 1 ; 
//		HZX_ViewHazard( &front_hzd ) ;
	} 
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &foot,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineHazard( &back_hzd, &back_hzd.attribute ) ;
		HZX_GetOnlinePoint( &back_hit ) ;
		_sceVu0SubVector( &back_adj, &back_hit, &foot ) ;
		GV_LenVec3F( &back_adj, &back_adj, 0.0F, GV_VecLen3F( &back_adj ) + 10.0F ) ;
		res |= 2 ;
	} 
	if ( ( res & 3 ) == 3 ) {
		printf( "no width\n" ) ;
//		return res ;
	}
	if ( res & 1 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
//		if ( res != 3 ) {

            DG_COPY_VEC( &SinkStepAdjust, &front_adj ) ;	
            GV_NearExp8VF( &SinkStepAdjust2, &SinkStepAdjust, 3 ) ;
            DG_COPY_VEC( &front_adj, &SinkStepAdjust2 ) ;	
            
//			_sceVu0AddVector( &work->control.step, &work->control.step, &front_adj ) ;
//			AdjustObject( work, &front_adj ) ;            

//printf( "<%d>avoid front\n", GV_Time ) ;
//		}
		_sceVu0AddVector( &nhead, &head, &front_adj ) ;
		c = AdjustLikeYs( work, &front_hit, &front_hzd, &front_adj, &nhead ) ;
		if ( !c ) AvoidTurnforSwim( work, &front_hzd, &front_adj, &nhead, 1, c ) ;
//c = 0 ;		
		res |= ( c ) ? 0x0100 : 0 ;
		if ( front_hzd.type == HZX_TYPE_SEGMENT && 
			!( front_hzd.attribute & HZX_SEG_NO_C4 ) ) res |= 0x1000 ;
		else if ( front_hzd.type == HZX_TYPE_FLOOR ) res |= 0x00010000 ;

		{
			FVECTOR		norm, dir ;
			float		ip ;

			_sceVu0ScaleVector( &front_adj, &front_adj, -1.0F ) ;
			HZX_GetNormal( &norm, &front_hzd ) ;
			_sceVu0Normalize( &norm, &norm ) ;
			ip = _sceVu0InnerProduct( &front_adj, &norm ) ;
			_sceVu0ScaleVector( &dir, &norm, ip ) ;
			AvoidDir = GV_VecDir2( &dir ) ;
			_sceVu0SubVector( &dir, &front_adj, &dir ) ;
			_sceVu0ScaleVector( &dir, &dir, -1.0F ) ;			
			AvoidDirX = GV_VecDir2X( &dir ) ;
		}
//		SeekCount = 0 ;
	} else if ( res & 2 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;

		DG_COPY_VEC( &SinkStepAdjust, &back_adj ) ;	
		GV_NearExp8VF( &SinkStepAdjust2, &SinkStepAdjust, 3 ) ;
		DG_COPY_VEC( &back_adj, &SinkStepAdjust2 ) ;	

//		_sceVu0AddVector( &work->control.step, &work->control.step, &back_adj ) ;
//		AdjustObject( work, &back_adj ) ;

//printf( "<%d>avoid back\n", GV_Time ) ;
//		_sceVu0AddVector( &nfoot, &foot, &back_adj ) ;
//		AvoidTurnforSwim( work, &back_hzd, &back_adj, &nfoot, 2, 1 ) ;
//		SeekCount = 0 ;
	}

	/* 前後とも壁なら無理矢理動かす */
	if ( ( res & 3 ) == 3 ) {
		FVECTOR		adj = { 24.0F, 0.0F, 0.0F } ;
//printf( "muriyarii\n" ) ;
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		DG_RotVector( &adj, &adj, 1 ) ;
		_sceVu0AddVector( &StepAdjust, &StepAdjust, &adj ) ;
	}

	GV_NearExp8VF( &StepAdjust2, &StepAdjust, 3 ) ;	
//	DG_COPY_VEC( &StepAdjust2, &StepAdjust ) ;
	_sceVu0SubVector( &StepAdjust, &StepAdjust, &StepAdjust2 ) ;

//printf( "<%d> adj %f %f %f, adj2 %f %f %f\n", GV_Time, 
//	     StepAdjust.vx, StepAdjust.vy, StepAdjust.vz,
//	     StepAdjust2.vx, StepAdjust2.vy, StepAdjust2.vz ) ;
	_sceVu0AddVector( &work->control.step, &work->control.step, &StepAdjust2 ) ;

	if ( res == 0 ) {
		GV_NearExp8VF( &SinkStepAdjust2, &SinkStepAdjust, 3 ) ;
	}

	_sceVu0AddVector( &work->control.step, &work->control.step, &SinkStepAdjust2 ) ;
	AdjustObject( work, &SinkStepAdjust2 ) ;
	_sceVu0SubVector( &SinkStepAdjust, &SinkStepAdjust, &SinkStepAdjust2 ) ;

//	_sceVu0AddVector( &work->control.step, &work->control.step, &StepAdjust ) ;
//	GV_NearExp8VF( &StepAdjust, &DG_ZeroVector, 3 ) ;
//if ( res != 0 ) printf( "<%d>:%d\n", GV_Time, res ) ;
	LastAvoidSinkTime = GV_Time ;

//DumpVec( &StepAdjust2 ) ;
//DumpVec( &SinkStepAdjust2 ) ;

	return res ;
}

/* ダイブ中高さ処理 */
static	void	DiveHeightAdjust( PlayerWork *work )
{
	DG_OBJ			*obj ;
	DG_MDL			*mdl ;
	FVECTOR			pos, vec ;
	float			max, min, y ;
	float			lheight, uheight ;
	int				i, n_models, flag ;
	float			levels[ 2 ], len ;

	PL_ObjPos( work, HUMAN21_KUBI, &pos ) ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &pos,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVector( &vec ) ;
		len = GV_VecLen3F( &vec ) ;
		if ( len > 8.0F ) {
			GV_LenVec3F( &vec, &vec, 0.0F, len - 8.0F ) ;
			_sceVu0AddVector( &pos, &work->control.mov, &vec ) ;
		}
		printf( "koshi kubi hazard! %f\n", len ) ;
	}
	flag = HZX_LevelHazardCheck( work->control.hzx_id, &pos, 
								 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
	HZX_GetLevelHeight( levels ) ;
	if ( !( flag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( !( flag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
	levels[ 0 ] = ( work->control.levels[ 0 ] > levels[ 0 ] ) ? 
		work->control.levels[ 0 ] : levels[ 0 ] ;
	levels[ 1 ] = ( work->control.levels[ 1 ] < levels[ 1 ] ) ? 
		work->control.levels[ 1 ] : levels[ 1 ] ;

	max = -1000000.0F ; min = 1000000.0F ;
	n_models = work->body.objs->def->n_models ;
	obj = work->body.objs->objs ;
	mdl = work->body.objs->def->models ;
	for ( i = 0; i <= HUMAN21_ATAMA; i ++, obj ++, mdl ++ ) { /* 上半身だけ */
		y = obj->world.m[ 3 ][ 1 ] ;
		if ( y > max ) max = y ;
		if ( y < min ) min = y ;
		{
			FVECTOR		emma = { 0.0F, 0.0F, -250.0F } ;
			FMATRIX		world ;

			DG_COPY_MAT( &world, &work->body.objs->objs[ HUMAN21_MUNE ].world ) ;
			GV_VecToMat( ( FVECTOR * )obj->world.m[ 3 ], &world ) ;
			DG_SetPos( &world ) ;
			DG_PutVector( &emma, &emma, 1 ) ;
			if ( emma.vy > max ) max = emma.vy ;
			if ( emma.vy < min ) min = emma.vy ;
		}
	}
	lheight = work->control.mov.vy - min + 100.0F ;
	uheight = max - work->control.mov.vy + 100.0F ;

	//printf( "[%d] : %f %f %f %f %f\n", GV_Time, work->control.mov.vy, 	
			 //	   levels[ 0 ], levels[ 1 ], lheight, uheight ) ;
	if ( lheight < work->control.height ) lheight = work->control.height ;
		
	/* 天井回避優先 */
	if ( levels[ 1 ] - uheight < work->control.mov.vy ) {
		work->control.step.vy -= GV_NearExp4F( 0.0F, work->control.mov.vy 
											  - ( levels[ 1 ] - uheight ) ) ;
	} else if ( levels[ 0 ] + lheight > work->control.mov.vy ) {
		work->control.step.vy += GV_NearExp4F( 0.0F, levels[ 0 ] + lheight 
											  - work->control.mov.vy ) ;
	}
}

#if 0
/* 急な床にはのぼらない */
static	int		ProcSteepSlope( PlayerWork *work )
{
	int			atr, rx ;
	FVECTOR		norm ;

	if ( !( work->control.grounded & 1 ) ) return 0 ;
	atr = work->control.level[ 0 ]->attribute ;
	if ( atr & HZX_FLOOR_FLAT ) return 0 ;
	HZX_GetNormal( &norm, work->control.level[ 0 ] ) ;
	rx = GV_VecDir2X( &norm ) ;	
	if ( rx < 0 ) rx = -rx ;
HZX_ViewHazard( work->control.level[ 0 ] ) ;
printf( "rx %d\n", rx ) ;
	if ( rx < 640 ) return 0 ;
	GV_LenVec3F( &norm, &norm, 0.0F, 16.0F ) ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
printf( "%f %f\n", norm.vx, norm.vz ) ;
	work->control.step.vx += norm.vx ;
	work->control.step.vz += norm.vz ;
	return 1 ;
}
#endif

/* 息継ぎ吸い込み */
static	void	BreathPointAdjust( PlayerWork *work )
{
	FVECTOR		step ;

	GuideToBreathPoint( &step, &work->control.mov, 
					    3000.0F, 6.0F ) ;
	work->control.step.vx += step.vx ;
	work->control.step.vz += step.vz ;
}


/*----------------------------------------------------------------*/

/* あわぶく */
static	void	Bubble( PlayerWork *work, int obj )
{
	extern void	*NewBubbleMany( FVECTOR *, int, int ) ;

	NewBubbleMany( ( FVECTOR * )work->body.objs->objs[ obj ].world.m[ 3 ], 60, 0 ) ;
}

/* 水中ステートをセット */
static	inline	void	SetWaterStatus( void )
{
	if ( ( GV_Time % 90 ) == 0 ) {
//		Bubble( GM_PlayerWork, HUMAN21_ATAMA ) ;
	}
	SetStatus( PLAYER_IN_THE_WATER | PLAYER_WEAPON_DISABLE | PLAYER_WEAPON_INVISIBLE ) ;
	SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_PEEP | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | 
			 FLAG_NO_IK ) ;
	GM_PlayerControl->turn.vz = 0 ;
	UpdateCamera( GM_PlayerWork ) ;
	if ( GM_PlayerControl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		PL_LevelCheck( GM_PlayerWork ) ;
	}

	if ( WaterCamera->on ) {
		GM_RadarSetFlag( &GM_PlayerWork->radar, RADAR_SIGHT ) ;
		SeekCameraTurn( GM_PlayerWork ) ;
	} else {
		GM_RadarResetFlag( &GM_PlayerWork->radar, RADAR_SIGHT ) ;
	}

	ModeFlag &= ~MF_CLEAR ;
	GM_PlayerControl->skip_flag &= ~CTRL_HZX_SEG_HORIZON_CHECK ;

	{
		GM_CameraSet		*now ;

		now = GM_GetCurrentCamera( 0 ) ;
		if ( now->position.vy > HorizonLevel ) {
			SetStatus( PLAYER_WATER_SURFACE ) ;
			PL_ObjPos( GM_PlayerWork, HUMAN21_ATAMA, &GM_PlayerFindPos ) ;
			PL_AddFindObj( HUMAN21_ATAMA ) ;
			/* トラップチェック位置頭 */
			PL_ObjPos( GM_PlayerWork, HUMAN21_ATAMA, &TrapCheckAim ) ;
			/* トラップチェック位置設定 */
			GV_NearExp4VF( &TrapCheckPos, &TrapCheckAim, 3 ) ;
			_sceVu0SubVector( &TrapCheckRootDiff, &TrapCheckPos, &GM_PlayerControl->mov ) ;
			GM_PlayerWork->control.root_offset = &TrapCheckRootDiff ;
		} else {
			_sceVu0AddVector( &TrapCheckPos, &GM_PlayerControl->mov, &GM_PlayerWork->root_diff ) ;
		}
	}

	/* Ｏ２管理 */
	if ( PL_ObjHeight( GM_PlayerWork, HUMAN21_ATAMA ) < HorizonLevel - 100.0F ) {
		SetStatus( PLAYER_NO_BREATH ) ;
	}
	/* メニュー設定 */
	PL_SetNoUseItemType( IT_TYPE_CBBOX | IT_TYPE_SUBJECT ) ;
}

/* 浮上チェック */
static	int		CheckWaterRise( PlayerWork *work )
{
	if ( ( work->pad->press & PAD_DIVE ) &&
		 ( work->action != WaterRise ) ) {
		if ( !CeilToFloor( work ) ) {
			SetMode( work, WaterRise ) ;
			return 1 ;
		}
	}
	return 0 ;
}

/* 腰位置キープでモーションセット */
static	void	SetActionL( PlayerWork *work, int action, int interp )
{
	/* 暫定 */
	if ( interp < 0 ) interp = MOT_INTERP_DEF ;
	SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	SetAction( work, action, interp ) ;
}

/* 水面から出ないように */
static	void	KeepHorizonLevel( PlayerWork *work )
{
	if ( work->control.mov.vy > HorizonLevel2 ) {
		work->control.mov.vy = GV_NearSpeedF( work->control.mov.vy, HorizonLevel2,
											  16.0F ) ;
	}
}

/* 水中シーク */
/* AvoidSinkより前に */
static	void	SeekTurnL( work )
PlayerWork		*work ;
{
    int		turn_to, padto, q, d, dq ;
	int		speed ;
	float	v ;

//	if ( !( work->pad->status & ( PAD_L | PAD_R ) ) ) return ;
	turn_to = work->control.turn.vy ;

	dq = 1024 ; q = 0 ;
	if ( WaterFlag & WATER_FLAG_AUTO_ROTATE ) {
		d = GV_DiffDirAbs( turn_to, 0 ) ;
		if ( d < dq ) {
			q = 0 ; dq = d ;
		}
		d = GV_DiffDirAbs( turn_to, 1024 ) ;
		if ( d < dq ) {
			q = 1024 ; dq = d ;
		}
		d = GV_DiffDirAbs( turn_to, 2048 ) ;
		if ( d < dq ) {
			q = 2048 ; dq = d ;
		}
		d = GV_DiffDirAbs( turn_to, 3072 ) ;
		if ( d < dq ) {
			q = 3072 ; dq = d ;
		}
	}

	padto = 0 ;
	if ( work->pad->status & work->pad->status & PAD_L ) {
		padto = 32 ;
	} else if ( work->pad->status & PAD_R ) {
		padto = -32 ;
	} 
#ifdef B_PATTERN
	if ( !( work->pad->status & ( PAD_R | PAD_L ) ) && 
		 ( work->pad->status & PAD_D ) ) {
		padto = 2048 ;
	}
#endif
	if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		v = ( float )( work->pad->left_dx - 128 ) ;
		if ( v < 0.0F ) v = -v ;
#ifdef B_PATTERN
		{
			float		v2 ;

			v2 = ( float )( work->pad->left_dy - 128 ) ;
			if ( v2 < 0.0F ) v2 = 0.0F ;
			if ( v2 > v ) v = v2 ;
		}
#endif
		v *= 2.0F ;
	} else {
		v = ( float )work->pad->pressure[ PAD_PRESS_L ] ;
		if ( v < ( float )work->pad->pressure[ PAD_PRESS_R ] ) {
			v = ( float )work->pad->pressure[ PAD_PRESS_R ] ;
		}
#ifdef B_PATTERN
		if ( v < ( float )work->pad->pressure[ PAD_PRESS_D ] ) {
			v = ( float )work->pad->pressure[ PAD_PRESS_D ] ;
		}
#endif
		v *= 2.4F ;
	}
	if ( v < 0.10F ) {
		if ( WaterFlag & WATER_FLAG_AUTO_ROTATE ) {
			if ( ++ SeekCount > 32 ) {
				v = 128.0F ; 
				padto = GV_DiffDirS( turn_to, q ) * 2 ;
			}
		}
	} else {
		SeekCount = 0 ;
	}
	if ( v > 256.0F ) v = 256.0F ;
	/* Ｘ傾きが大きいときは、
	   回転スピードを落とす */
	if ( work->control.rot.vx < -512 ||
		 work->control.rot.vx > 512 ) {
		v /= 3.0F ;
	}
	padto = ( int )( ( float )padto * v / 256.0F ) + turn_to ;
	speed = ( int )( 18.0F * v / 256.0F ) ;
	work->control.turn.vy = GV_NearSpeed( turn_to, padto, speed ) ;
}

/* Ｘ向き変更 */
#ifdef A_PATTERN
static	void	SeekTurnX( PlayerWork *work )
{
    int		turn_to, from, speed ;
	int		ul, dl ;
	float	v, base ;

    from = work->control.turn.vx ;
	speed = 0 ;
	ul = TurnXLimitU ;
	dl = TurnXLimitD ;
	TurnXLimitU = WATER_TURNX_LIMIT ;
	TurnXLimitD = -WATER_TURNX_LIMIT ;
	if ( work->pad->press & ( PAD_HEAD_DOWN | PAD_HEAD_UP ) ) {
		ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
	}
	if ( CeilToFloor( work ) ) goto seekturnx_nopad ;
	if ( work->pad->status & PAD_HEAD_DOWN ) {
		if ( ModeFlag & MF_SEEKDOWN_NEW_PRESS ) {
			goto seekturnx_nopad ;
		}
		turn_to = ul ;
		if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
			v = ( float )( work->pad->left_dy - 128 ) ;
			if ( v < 0.0F ) v = -v ;
			v *= 1.5F ;
		} else {
			v = ( float )work->pad->pressure[ PAD_PRESS_HEAD_DOWN ] ;
			v *= 2.4F ;
		}
		CamRotAim.vx = CAMERA_TURNX + 512 ;
	} else if ( work->pad->status & PAD_HEAD_UP ) {
		turn_to = dl ;
		if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
			v = ( float )( work->pad->left_dy - 128 ) ;
			if ( v < 0.0F ) v = -v ;
			v *= 1.5F ;			
		} else {
			v = ( float )work->pad->pressure[ PAD_PRESS_HEAD_UP ] ;
			v *= 2.4F ;
		}
		CamRotAim.vx = CAMERA_TURNX - 512 ;
	} else {
seekturnx_nopad :
		turn_to = 0 ;
		v = 196.0F ;
	}
	if ( ( work->pad->status & ( PAD_L | PAD_R ) ) && ( v < 128.0F ) ) {
		turn_to = 0 ;
		v = 196.0F ;
	}
	if ( v > 256.0F ) v = 256.0F ;
	turn_to = ( int )( ( float )turn_to * v / 256.0F ) ;
	from = GV_NearPhase( from, turn_to ) ;
	base = 32.0F ;
	if ( v >= 200.0F ) base = 128.0F ;
	speed = ( int )( base * v / 256.0F ) ;
	work->control.turn.vx = GV_NearSpeed( from, turn_to, speed ) ;
}
#endif
#ifdef B_PATTERN
static	void	SeekTurnX( PlayerWork *work )
{
    int		turn_to, from, speed ;
	int		ul, dl ;
	float	v ;

    from = work->control.turn.vx ;
	speed = 0 ;
	ul = TurnXLimitU ;
	dl = TurnXLimitD ;
	TurnXLimitU = WATER_TURNX_LIMIT ;
	TurnXLimitD = -WATER_TURNX_LIMIT ;
	if ( CeilToFloor( work ) ) goto seekturnx_nopad ;
	if ( work->pad->status & PAD_LOWER ) {
		turn_to = ul ;
		v = ( float )work->pad->pressure[ PAD_PRESS_LOWER ] ;
		v *= 2.4F ;
	} else if ( work->pad->status & PAD_UPPER ) {
		turn_to = dl ;
		v = ( float )work->pad->pressure[ PAD_PRESS_UPPER ] ;
		v *= 2.4F ;
	} else {
seekturnx_nopad :
		turn_to = 0 ;
		v = 196.0F ;
	}
	if ( v > 256.0F ) v = 256.0F ;
	from = GV_NearPhase( from, turn_to ) ;
	speed = ( int )( 16.0F * v / 256.0F ) ;
	work->control.turn.vx = GV_NearSpeed( from, turn_to, speed ) ;
}
#endif

/* 連打チェック */
/* data data2 を使用 */
static	int		QuickPadCheck( PlayerWork *work )
{
	int			res = 0 ;

	if ( work->pad->press & PAD_SWIM ) {
		if ( GV_Time - work->data < 16 ) {
			res = 1 ;
			work->data2 = 1 ;
		}
		work->data = GV_Time ;
	} else {
		if ( GV_Time - work->data < 16 && work->data2 == 1 ) res = 1 ;
	}
	if ( res == 0 ) work->data2 = 0 ;
	return res ;
}

/* 連打で2倍 */
static	void	MotionDouble( PlayerWork *work )
{
	/* いつも1.75倍 */
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.75F ) ;
#if 0
	if ( !( work->pad->status & PAD_SWIM ) ) {
		if ( ++ work->data >= 32 ) {
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
		}
	} else {
		if ( work->data > 0 ) {
			/* 連打で2倍 */
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.50F ) ;
			work->data2 = 12 ;
		}
		if ( work->data2 > 0 && -- work->data2 == 0 ) {
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
		}
		work->data = 0 ;
	}
#endif
}

/* 床チェック無し時の天井床チェック */
/* SetWaterStatus , step.vy = 0.0F の後にコールすること */
static	void	LevelAdjustforSwim( PlayerWork *work )
{
	if ( !( work->control.skip_flag & CTRL_SKIP_FLR_CHECK ) ) return ;
	if ( work->control.levels[ 1 ] - work->control.mov.vy < work->control.height ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy 
			= GV_NearExp4F( 0.0F, 
						   work->control.levels[ 1 ] - work->control.mov.vy
						   - work->control.height ) ;
	} else if ( work->control.mov.vy - work->control.levels[ 0 ] < work->control.height ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy 
			= GV_NearExp4F( 0.0F, 
						   work->control.height 
						   - ( work->control.mov.vy - work->control.levels[ 0 ] ) ) ;
	}
}

/* カメラＹ角度を腰のワールドで設定 */
static	void	SetNCamRot( PlayerWork *work )
{
	FVECTOR		v = { 0.0F, 0.0F, 10.0F } ;

	ModeFlag |= MF_USE_NCAMROT ;
	DG_SetPos( &work->body.objs->objs[ HUMAN21_KOSHI ].world ) ;
	DG_RotVector( &v, &v, 1 ) ;
	NCamRot.vy = GV_VecDir2( &v ) ;
}

/*---------------------------------------------------------------

  各モード関数 *******

---------------------------------------------------------------*/

#ifdef A_PATTERN

/* 立ち泳ぎ静止 */
static	void	TreadStill( PlayerWork *work, int time ) 
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SURFACE ;
		SetActionL( work, WMtread_idle, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
		WaterCameraOff() ;
	}
	SetWaterStatus() ;
	work->control.mov.vy = GV_NearSpeedF( work->control.mov.vy, HorizonLevel2,
										  16.0F ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;

	work->control.step.vy = 0.0F ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		if ( work->pad->press & ( PAD_MODE | PAD_SWIM | PAD_DIVE ) ) {
			SetMode( work, Tread2Dive ) ;
		} else if ( work->pad->status & PAD_UDLR ) {
			SetMode( work, TreadMove ) ;
		}
	}
    LargeMoveAdjust( work ) ;
}

/* 立ち泳ぎ移動 */
static	void	TreadMove( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SURFACE ;
		SetActionL( work, WMtread, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;		
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
		WaterCameraOff() ;
	}
	SetWaterStatus() ;
	work->control.mov.vy = GV_NearSpeedF( work->control.mov.vy, HorizonLevel2,
										  16.0F ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;
	work->control.step.vy = 0.0F ;
	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, TreadStill ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else if ( work->pad->press & ( PAD_MODE | PAD_SWIM | PAD_DIVE ) ) {
		SetMode( work, Tread2Dive ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else if ( !( work->pad->status & PAD_UDLR ) ) {
		SetMode( work, TreadStill ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else {
        if ( work->pad->dir >= 0 ) {
			work->control.turn.vy 
				= GV_NearSpeed( GV_NearPhase( work->control.turn.vy, work->pad->dir ),
							   work->pad->dir,   
							   48 ) ;
		}
		/* アナログ押し込みで速さを変える */
		{
			int			pressure ;
			float		speed ;

			pressure = PL_StrongestUDLRValue( work->pad ) ;
			speed = ( float )TIME_BASE ;
			if ( pressure > 160 && pressure <= 196 ) {
				speed *= 1.5F ;
			} else if ( pressure > 224 ) {
				speed *= 2.0F ;
			}
			MT_SetMotionSpeed( work->body.m_ctrl, speed ) ;
		}
	}
    LargeMoveAdjust( work ) ;
}

/* 立ち泳ぎからダイブ */
static	void	Tread2Dive( PlayerWork *work, int time ) 
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMtread2dive, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	work->control.step.vy = 0.0F ;
	SeekTurnX( work ) ;
	MotionDouble( work ) ;

	if ( PL_CheckMotionRate( work ) > CHANGE_MRATE && !CeilToFloor( work ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_MODE ) ) {
			if ( work->control.mov.vy > HL() ) {
				SetMode( work, Dive2Tread ) ;
			} else {
				SetMode( work, Dive2Stand ) ;
			}
			return ;
		}
	}
	SeekTurnL( work ) ;
	AvoidSinkforSwim( work ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, Dive ) ;
	}
    LargeMoveAdjust( work ) ;
}

/* ダイブから立ち泳ぎ */
static	void	Dive2Tread( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SURFACE ;
		SetActionL( work, WMdive2tread, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		WaterCameraOff() ;
	}
	SetWaterStatus() ;
	KeepHorizonLevel( work ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;

    LargeMoveAdjust( work ) ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	}

	if ( EndMotion( work ) || 
		( PL_MotionTime( work ) > 60 && ( work->pad->status & ( PAD_UDLR | PAD_DIVE ) ) ) ) {
		SetMode( work, TreadStill ) ;
	}
}

/* ダイブから水中立ち */
static	void	Dive2Stand( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetActionL( work, WMbreast2stand, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	KeepHorizonLevel( work ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;
	BreathPointAdjust( work ) ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		SeekTurnL( work ) ;
	}

    LargeMoveAdjust( work ) ;

	if ( PL_CheckMotionRate( work ) > CHANGE_MRATE ) {
		if ( !Status( PLAYER_WATCH ) && 
			( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) ) {
			SetMode( work, Stand2Dive ) ;
			return ;
		}
	}
	if ( EndMotion( work ) ) {
		SetMode( work, WaterStand ) ;
	}
	CheckWaterRise( work ) ;
}

/* 水中立ち */
static	void	WaterStand( PlayerWork *work, int time )
{
	float		upper, lower, mid ;

	SetWaterStatus() ;
//	UnsetFlag( FLAG_NO_IK ) ;
//	if ( time != 0 ) {
//		UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
//		if ( work->ftime == 1 ) {
//			work->body.m_ctrl->old_height = work->control.height ;
//			work->body.m_ctrl->root_old_height = work->control.height ;
//		}
//	}
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetActionL( work, WMstand, -1 ) ;
		UnsetFlag( FLAG_NO_WAIST_INTERP ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.grounded = 0 ;
		work->control.turn.vx = 0 ;
		WaterCameraOn() ;
	}

	BreathPointAdjust( work ) ;

    LargeMoveAdjust( work ) ;
#if 0
#if 1
	lower = work->control.levels[ 0 ] ;
	upper = work->control.levels[ 1 ] ;
	if ( upper > HorizonLevel ) upper = HorizonLevel ;
//	mid = ( lower + upper ) / 2.0F ;
	mid = ( upper - lower ) / 4.0F * 3.0F + lower ;
	if ( mid > work->control.mov.vy ) work->control.step.vy = -FALL_SPEED ;
	else 							  work->control.step.vy = FALL_SPEED ;
#else
	work->control.step.vy = -FALL_SPEED ;
#endif
#endif
#if 1
	if ( work->pad->status & PAD_HEAD_UP ) {
		work->control.step.vy = FALL_SPEED ;
		SetActionL( work, WMtread_idle, -1 ) ;
		CamRotAim.vx = CAMERA_TURNX - 512 ;
	} else if ( work->pad->status & PAD_HEAD_DOWN ) {
		work->control.step.vy = -FALL_SPEED * 2.0F ;
		SetActionL( work, WMtread_idle, -1 ) ;
		CamRotAim.vx = CAMERA_TURNX + 512 ;
	} else {
		work->control.step.vy = -FALL_SPEED ;
		SetActionL( work, WMstand, -1 ) ;
	}
#endif
	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Stand2Dive ) ;
		return ;
	}

	if ( work->control.mov.vy > HL() ) {
		SetMode( work, TreadStill ) ;
		return ;
	} else if ( work->control.grounded & 1 ) {
		if ( !( work->pad->status & PAD_U ) ) {
			SetMode( work, SeabedStill ) ;
		}
		return ;
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) ) {
		SetMode( work, Stand2Dive ) ;
		return ;
	} 

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
        SeekTurnL( work ) ;
	}
	CheckWaterRise( work ) ;
}

/* 水中立ちからダイブ */
static	void	Stand2Dive( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetActionL( work, WMstand2dive, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

	SeekTurnX( work ) ;

	if ( PL_CheckMotionRate( work ) > CHANGE_MRATE && !CeilToFloor( work ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_MODE ) ) {
			if ( work->control.mov.vy > HL() ) {
				SetMode( work, Dive2Tread ) ;
			} else {
				SetMode( work, Dive2Stand ) ;
			}
			return ;
		}
	}

	SeekTurnL( work ) ;
	MotionDouble( work ) ;
	AvoidSinkforSwim( work ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, Dive ) ;
	}
    LargeMoveAdjust( work ) ;
}

/* 水底立ち静止 */
static	void	SeabedStill( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetAction( work, WMseabed_idle, 12 ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.grounded = 0 ;
		work->control.turn.vx = 0 ;
		WaterCameraOn() ;
		PL_ObjPos( work, HUMAN21_ATAMA, &work->fv ) ;
	}
	SetWaterStatus() ;
	UnsetFlag( FLAG_NO_IK ) ;
	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	ModeFlag |= MF_STAND | MF_NO_CAMERA_RZ ;
	DG_COPY_VEC( &work->camera, &work->fv ) ;

	work->control.step.vy = -FALL_SPEED_BOTTOM ;

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Seabed2Dive ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		if ( work->control.mov.vy > HL() ) {
			SetMode( work, TreadStill ) ;
		} else if ( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) {
			SetMode( work, Seabed2Dive ) ;
		} else if ( work->pad->status & PAD_U ) {
			SetMode( work, SeabedWalk ) ;
		} else if ( work->pad->status & ( PAD_L | PAD_R ) ) {
			SeekTurnL( work ) ;
		}
	}
	CheckWaterRise( work ) ;
    LargeMoveAdjust( work ) ;
}

/* 水底歩き */
static	void	SeabedWalk( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetAction( work, WMseabed_walk, 12 ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.grounded = 0 ;
		work->control.turn.vx = 0 ;
		work->sv.vx = 0 ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	UnsetFlag( FLAG_NO_IK ) ;
	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_WAIST_INTERP ) ;
    ModeFlag |= MF_NO_CAMERA_RZ ;

    LargeMoveAdjust( work ) ;

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Seabed2Dive ) ;
		return ;
	}

	/* 2倍 */
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.0F ) ;
	work->control.step.vy = -FALL_SPEED_BOTTOM ;
	if ( time != 0 && !( work->control.grounded & 1 ) ) {
		SetMode( work, WaterStand ) ;
		return ;
	}
#if 1
    /* 前方壁チェック */
    if ( work->control.n_touches != 0 ) {
		int			dir ;

		dir = GV_VecDir2( &work->control.vecs[ 0 ] ) ;
		if ( GV_DiffDirAbs( dir, work->control.rot.vy ) < 128 ) {
			if ( ++ work->sv.vx > 32 ) {
				SetMode( work, WaterStand ) ;
				return ;
			}
		} else {
			work->sv.vx = 0 ;
		}
	}
#endif
	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, SeabedStill ) ;
	} else {
		if ( work->control.mov.vy > HL() ) {
			SetMode( work, TreadStill ) ;
		} else if ( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) {
			SetMode( work, Seabed2Dive ) ;	
		} else if ( !( work->pad->status & PAD_U ) ) {
			SetMode( work, SeabedStill ) ;
		} else {
            SeekTurnL( work ) ;
		}
	}
	CheckWaterRise( work ) ;
}

/* 水底からダイブ */
static	void	Seabed2Dive( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMseabed2dive, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

	SeekTurnX( work ) ;
	SeekTurnL( work ) ;
	MotionDouble( work ) ;
	AvoidSinkforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	if ( PL_CheckMotionRate( work ) > CHANGE_MRATE && !CeilToFloor( work ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_MODE ) ) {
			if ( work->control.mov.vy > HL() ) {
				SetMode( work, Dive2Tread ) ;
			} else {
				SetMode( work, Dive2Stand ) ;
			}
			return ;
		}
	}

	if ( EndMotion( work ) ) {
		SetMode( work, Dive ) ;
	}
	CheckWaterRise( work ) ;
}

/* ダイブ泳ぎ AAAAAAAA */
static	void	Dive( PlayerWork *work, int time )
{
	int			sink, mode ;
	int			mtime, ctf ;

	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		if ( work->data == 1 ) {
			SetActionL( work, WMdive_end, -1 ) ;
		} else {
			SetActionL( work, WMdive_end_st, 24 ) ;
		}
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->data = 0 ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->sv.vx = 0 ;
		work->sv.vy = 0 ;
		work->sv.vz = 0 ;
		work->idata = 0 ;
		work->idata2 = 0 ;
	}
	SetWaterStatus() ;
	work->control.step.vy = work->body.m_ctrl->step.vy ;
	ctf = CeilToFloor( work ) ;

	/* 高さ処理 */
#if 0
	if ( 1 ) {
		DG_OBJ			*obj ;
		DG_MDL			*mdl ;
		FVECTOR			pos, vec ;
		float			max, min, y ;
		float			lheight, uheight ;
		int				i, n_models, flag ;
		float			levels[ 2 ], len ;

		PL_ObjPos( work, HUMAN21_KUBI, &pos ) ;
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &pos,
								    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
								    HZX_FLOOR_NO_PLAYER ) ) {
			HZX_GetOnlineVector( &vec ) ;
			len = GV_VecLen3F( &vec ) ;
			if ( len > 8.0F ) {
				GV_LenVec3F( &vec, &vec, 0.0F, len - 8.0F ) ;
				_sceVu0AddVector( &pos, &work->control.mov, &vec ) ;
			}
			printf( "koshi kubi hazard! %f\n", len ) ;
		}

		flag = HZX_LevelHazardCheck( work->control.hzx_id, &pos, 
									 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
		HZX_GetLevelHeight( levels ) ;
		if ( !( flag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
		if ( !( flag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
		levels[ 0 ] = ( work->control.levels[ 0 ] > levels[ 0 ] ) ? 
			work->control.levels[ 0 ] : levels[ 0 ] ;
		levels[ 1 ] = ( work->control.levels[ 1 ] < levels[ 1 ] ) ? 
			work->control.levels[ 1 ] : levels[ 1 ] ;

		max = -1000000.0F ; min = 1000000.0F ;
		n_models = work->body.objs->def->n_models ;
		obj = work->body.objs->objs ;
		mdl = work->body.objs->def->models ;
		for ( i = 0; i <= HUMAN21_ATAMA; i ++, obj ++, mdl ++ ) { /* 上半身だけ */
			y = obj->world.m[ 3 ][ 1 ] ;
			if ( y > max ) max = y ;
			if ( y < min ) min = y ;
			{
				FVECTOR		emma = { 0.0F, 0.0F, -250.0F } ;
				FMATRIX		world ;

				DG_COPY_MAT( &world, &work->body.objs->objs[ HUMAN21_MUNE ].world ) ;
				GV_VecToMat( ( FVECTOR * )obj->world.m[ 3 ], &world ) ;
				DG_SetPos( &world ) ;
				DG_PutVector( &emma, &emma, 1 ) ;
				if ( emma.vy > max ) max = emma.vy ;
				if ( emma.vy < min ) min = emma.vy ;
			}
		}
		lheight = work->control.mov.vy - min + 100.0F ;
		uheight = max - work->control.mov.vy + 100.0F ;

//printf( "[%d] : %f %f %f %f %f\n", GV_Time, work->control.mov.vy, 	
//	   levels[ 0 ], levels[ 1 ], lheight, uheight ) ;
		if ( lheight < work->control.height ) lheight = work->control.height ;
		
		/* 天井回避優先 */
		if ( levels[ 1 ] - uheight < work->control.mov.vy ) {
			work->control.step.vy -= GV_NearExp4F( 0.0F, work->control.mov.vy 
				- ( levels[ 1 ] - uheight ) ) ;
		} else if ( levels[ 0 ] + lheight > work->control.mov.vy ) {
			work->control.step.vy += GV_NearExp4F( 0.0F, levels[ 0 ] + lheight 
				- work->control.mov.vy ) ;
		}

	}
#endif
	DiveHeightAdjust( work ) ;

	SetFlag( FLAG_NO_MOTION_STEP_Y ) ;

	SeekTurnL( work ) ;
	SeekTurnX( work ) ;

	if ( work->control.turn.vx < 0 && 
		 work->control.mov.vy > HL() &&
		 work->control.levels[ 1 ] > HorizonLevel ) {
		if ( !ctf ) {
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
			SetFlag( FLAG_NO_STEP ) ;
			SetMode( work, Dive2Tread ) ;
			return ;
		}
	}
#if 0
	if ( work->pad->status & ( PAD_UDLR ) ) {
//		if ( work->pad->press & PAD_U ) SetActionL( work, WMdive_arm_r, -1 ) ;
//		else if ( work->pad->press & PAD_D ) SetActionL( work, WMdive_arm_l, -1 ) ;
        if ( work->pad->press & ( PAD_U | PAD_D ) ) {
			work->motion1 = -1 ;
			SetActionL( work, WMdive_max, 12 ) ;						
		}
		if ( EndMotion( work ) ) {
			SetActionL( work, WMdive_end_st, -1 ) ;			
		}
	} else {
		SetActionL( work, WMdive_end_st, -1 ) ;
	}
	if ( work->motion1 == WMdive_end_st ) {
		if ( EndMotion( work ) && !ctf ) {
			SetFlag( FLAG_NO_STEP ) ;
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
			if ( work->control.mov.vy > HL() && 
				work->control.levels[ 1 ] > HorizonLevel ) {
				SetMode( work, TreadStill ) ;
			} else {
				SetMode( work, WaterStand ) ;
			}
			return ;
		} 
	}
#endif

	mode = work->sv.vx ;
	switch( mode ) {
	case 0 :
		if ( QuickPadCheck( work ) ) {
			work->sv.vx = 3 ;
			work->sv.vy = 0 ;
			SetActionL( work, WMdive_max, -1 ) ;
			break ;
		}
		if ( work->pad->press & PAD_SWIM ) {
			if ( work->sv.vy == 0 ) {
				SetActionL( work, WMdive_arm_r, 24 ) ;				
				Bubble( GM_PlayerWork, HUMAN21_MIGI_TE ) ;
				Bubble( GM_PlayerWork, HUMAN21_MIGI_KAKATO ) ;
				work->sv.vx = 1 ;
				work->sv.vy = 1 ;
			} else {
				SetActionL( work, WMdive_arm_l, 24 ) ;
				Bubble( GM_PlayerWork, HUMAN21_HIDARI_TE ) ;
				Bubble( GM_PlayerWork, HUMAN21_HIDARI_KAKATO ) ;
				work->sv.vx = 2 ;
				work->sv.vy = 0 ;
			}
		}

		if ( ctf && work->motion1 == WMdive_end ) {
			SetActionL( work, WMdive_end_st, 24 ) ;
		}

		if ( EndMotion( work ) ) {
			if ( work->motion1 == WMdive_end_st ) {
				if ( !( work->pad->status & PAD_SWIM ) && 
					 !ctf ) {
					SetActionL( work, WMdive_end, -1 ) ;
				}
			} else {
				if ( !ctf ) {
					SetFlag( FLAG_NO_STEP ) ;
					ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
					if ( work->control.mov.vy > HL() && 
						work->control.levels[ 1 ] > HorizonLevel ) {
						//SetMode( work, Dive2Tread ) ;
						SetMode( work, TreadStill ) ;
					} else {
						//SetMode( work, Dive2Stand ) ;
						SetMode( work, WaterStand ) ;
					}
				} else {
					SetActionL( work, WMdive_end_st, 24 ) ;
				}
			}
		}
		break ;
	case 1 :	/* 右かき */
//		work->control.turn.vz = 128 ;
		if ( QuickPadCheck( work ) ) {
			work->sv.vx = 3 ;
			work->sv.vy = 0 ;
			SetActionL( work, WMdive_max, -1 ) ;
			Bubble( GM_PlayerWork, HUMAN21_MIGI_KAKATO ) ;
			break ;
		}
		mtime = PL_MotionTime( work ) ;
		if ( mtime > 30 && ( work->pad->press & PAD_SWIM ) ) {
			work->sv.vz = 1 ;
		}
		if ( PL_MotionTime( work ) >= 60 ) {
			if ( work->sv.vz == 1 ) {
				SetActionL( work, WMdive_arm_l, 24 ) ;
				Bubble( GM_PlayerWork, HUMAN21_HIDARI_TE ) ;
				Bubble( GM_PlayerWork, HUMAN21_HIDARI_KAKATO ) ;
				work->sv.vx = 2 ;
				work->sv.vy = 0 ;
			} else if ( EndMotion( work ) ) {
				if ( work->pad->status & PAD_SWIM ) {
					SetActionL( work, WMdive_end_st, 24 ) ;
				} else {
					SetActionL( work, WMdive_end, -1 ) ;
				}
				work->sv.vx = 0 ;
				work->sv.vy = 1 ;
			}
			work->sv.vz = 0 ;			
		}
		break ;
	case 2 :	/* 左かき */
//		work->control.turn.vz = -128 ;
		if ( QuickPadCheck( work ) ) {
			work->sv.vx = 3 ;
			work->sv.vy = 0 ;
			SetActionL( work, WMdive_max, -1 ) ;
			Bubble( GM_PlayerWork, HUMAN21_MIGI_KAKATO ) ;
			break ;
		}
		mtime = PL_MotionTime( work ) ;
		if ( mtime > 30 && ( work->pad->press & PAD_SWIM ) ) {
			work->sv.vz = 1 ;
		}
		if ( PL_MotionTime( work ) >= 60 ) {
			if ( work->sv.vz == 1 ) {
				SetActionL( work, WMdive_arm_r, 24 ) ;
				Bubble( GM_PlayerWork, HUMAN21_MIGI_TE ) ;
				Bubble( GM_PlayerWork, HUMAN21_MIGI_KAKATO ) ;
				work->sv.vx = 1 ;
				work->sv.vy = 1 ;			
			} else if ( EndMotion( work ) ) {
				if ( work->pad->status & PAD_SWIM ) {
					SetActionL( work, WMdive_end_st, 24 ) ;
				} else {
					SetActionL( work, WMdive_end, -1 ) ;
				}
				work->sv.vx = 0 ;
				work->sv.vy = 0 ;
			}
			work->sv.vz = 0 ;			
		}
		break ;
	case 3 :	/* スーパー連打 */
		if ( !QuickPadCheck( work ) ) {
			work->sv.vx = 0 ;
			work->sv.vy = 0 ;
			work->sv.vz = 0 ;			
			if ( work->pad->status & PAD_SWIM ) {
				work->sv.vx = 4 ;
				SetActionL( work, WMdive_end, -1 ) ;
			} else {
				SetActionL( work, WMdive_end, -1 ) ;
			}
			break ;
		}
		CamTrackAim = CAMERA_TRACK_FAR ;
		CamAngleAim = 1.50F ;
		if ( EndMotion( work ) ) {
			Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		}
		break ;
	case 4 :
		if ( PL_CheckMotionTime( work, 16 ) ) {
			work->sv.vx = 0 ;
			SetActionL( work, WMdive_end_st, -1 ) ;
			break ;
		}
	}

	sink = AvoidSinkforSwim( work ) ;

	/* 体Ｚ回転 */
	work->control.turn.vz = WaterCamera->rotate.vz * 9 ;
	if ( work->control.turn.vz > 768 ) work->control.turn.vz = 768 ;
	else if ( work->control.turn.vz < -768 ) work->control.turn.vz = -768 ;

	if ( work->pad->status & PAD_HEAD_UP ) BreathPointAdjust( work ) ;

    LargeMoveAdjust( work ) ;

//printf( "<%d> : ", GV_Time ) ;
    if ( ( sink & 0x0001 ) && ( sink & 0x1000 ) && !( sink & 0x0100 ) &&
		!( work->pad->status & PAD_UDLR ) ) {
		work->idata += 3 ;
//printf( "a " ) ;
	} else {
		if ( work->idata > 0 ) -- work->idata ;
//printf( "b " ) ;
	}
//printf( "sink %x %d\n", sink, work->idata ) ;

	if ( work->idata > 64 && !ctf ) {
		SetFlag( FLAG_NO_STEP ) ;
		ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
		SetMode( work, Dive2WallTouch ) ;
		return ;
	}
	/* 床頭 */
	if ( ( sink & 1 ) && ( sink & 0x00010000 ) ) {
		if ( ( work->pad->status & PAD_HEAD_DOWN ) ) {
			printf( "yuka atama %d\n", AvoidDirX ) ;
//			work->control.turn.vx = GV_NearSpeedP( work->control.rot.vx, AvoidDirX, 256 ) ;
			work->idata2 = AvoidDirX ;
			ModeFlag |= MF_SEEKDOWN_NEW_PRESS ;
		}
	}
	if ( ModeFlag & MF_SEEKDOWN_NEW_PRESS ) {
		work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, work->idata2, 16 ) ;
		if ( PL_ObjHeight( work, HUMAN21_KUBI ) - work->control.levels[ 0 ] > 1000.0F ) {
			printf( "yuka atama kaijo\n" ) ;
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
		}
	}

	if ( ( work->pad->press & PAD_MODE ) || Status( PLAYER_WATCH ) ) {
		if ( !ctf ) {
			SetFlag( FLAG_NO_STEP ) ;
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
			if ( work->control.mov.vy > HL() && 
				work->control.levels[ 1 ] > HorizonLevel ) {
				SetMode( work, Dive2Tread ) ;
			} else {
				SetMode( work, Dive2Stand ) ;
			}
		} else if ( Status( PLAYER_WATCH ) ) {
			/* 天井低い主観 */
			work->sv.vx = 0 ;
			work->sv.vy = 0 ;
			work->sv.vz = 0 ;			
			SetActionL( work, WMdive_end_st, -1 ) ;
		}
	}

#if 1
	/* くねり実験 */
	{
		int				rx, dy ;
		static SVECTOR	rot = { 0, 0, 0 } ;

		rx = work->control.turn.vx ;
		if ( rx < 0 ) rx = -rx ;
		if ( rx < 512 ) {
			dy = GV_DiffDirS( work->control.rot.vy, work->control.turn.vy ) ;
			if ( dy < -32 || dy > 32 ) {
				rot.vy = GV_NearExp16P( rot.vy, -dy * 2 ) ;
			} else {
				rot.vy = GV_NearExp16P( rot.vy, 0 ) ;
			}
		} else {
			rot.vy = GV_NearExp16P( rot.vy, 0 ) ;
		}
		if ( rot.vy < 0 ) {
			work->turn_adjusts[ HUMAN21_MIGI_ASHI1 ].vy = rot.vy / 1 ;
			work->turn_adjusts[ HUMAN21_HIDARI_ASHI1 ].vy = rot.vy ;
			work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_MIGI_ASHI1 ) ;
			work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_HIDARI_ASHI1 ) ;
		} else if ( rot.vy > 0 ) {
			work->turn_adjusts[ HUMAN21_MIGI_ASHI1 ].vy = rot.vy ;
			work->turn_adjusts[ HUMAN21_HIDARI_ASHI1 ].vy = rot.vy / 1 ;
			work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_MIGI_ASHI1 ) ;
			work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_HIDARI_ASHI1 ) ;
		}
	}
#endif

	CheckWaterRise( work ) ;
}

/* 浮上 */
static	void	WaterRise( PlayerWork *work, int time ) 
{
	int			sink, mode ;
	int			mtime ;

	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMdive_max, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->sv.vx = 0 ;
	}
	SetWaterStatus() ;
	work->control.step.vy = work->body.m_ctrl->step.vy ;

	DiveHeightAdjust( work ) ;
	if ( !( work->pad->status & PAD_HEAD_DOWN ) ) BreathPointAdjust( work ) ;

	SetFlag( FLAG_NO_MOTION_STEP_Y ) ;

	SeekTurnL( work ) ;
//	SeekTurnX( work ) ;
	if ( work->pad->status & PAD_HEAD_DOWN ) {
		work->control.turn.vx = GV_NearExp8P( work->control.turn.vx, WATER_TURNX_LIMIT ) ;
		CamRotAim.vx = CAMERA_TURNX + 512 ;
	} else {
		work->control.turn.vx = GV_NearExp8P( work->control.turn.vx, -WATER_TURNX_LIMIT ) ;
		CamRotAim.vx = CAMERA_TURNX - 512 ;
	}

	if ( work->control.turn.vx < 0 && 
		 work->control.mov.vy > HL() &&
		 work->control.levels[ 1 ] > HorizonLevel ) {
		ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
		SetMode( work, Dive2Tread ) ;
		return ;
	}
	if ( !( work->pad->status & PAD_DIVE ) ) {
		ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
		SetMode( work, Dive2Stand ) ;
		return ;
	}
	CamTrackAim = CAMERA_TRACK_FAR ;
	if ( EndMotion( work ) ) {
		Bubble( work, HUMAN21_MIGI_KAKATO ) ;
	}

	/* 体Ｚ回転 */
	work->control.turn.vz = WaterCamera->rotate.vz * 6 ;

	sink = AvoidSinkforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	/* 壁あたりチェック */
    if ( ( sink & 1 ) && ( sink & 8 ) && !( sink & 4 ) ) {
		work->idata += 3 ;
	} else {
		if ( work->idata > 0 ) -- work->idata ;
	}
	/* 床あたりチェック */
	if ( ( sink & 1 ) && !( sink & 8 ) ) {
		work->sv.vx += 4 ;
	} else {
		if ( work->sv.vx > 0 ) -- work->sv.vx ;
	}
	if ( work->idata > 64 ) {
//		SetMode( work, Dive2WallTouch ) ;
//		return ;
	}
	if ( work->sv.vx > 64 ) {
//		SetMode( work, Dive2Stand ) ;
//		return ;
	}
}

#if 0
static	void	Submerge( PlayerWork *work, int time ) 
{

}
#endif

#endif

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

#ifdef B_PATTERN

/* 立ち泳ぎ静止 */
static	void	TreadStill( PlayerWork *work, int time ) 
{
//	SetWaterStatus() ;
//	UnsetFlag( FLAG_NO_IK ) ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMtread_idle, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.turn.vx = 0 ;
		WaterCameraOff() ;
	}
	SetWaterStatus() ;
	work->control.mov.vy = GV_NearSpeedF( work->control.mov.vy, HorizonLevel2,
										  16.0F ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;

	work->control.step.vy = 0.0F ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		if ( work->pad->press & PAD_LOWER ) {
			SetMode( work, Tread2Dive ) ;
		} else if ( work->pad->status & PAD_UDLR ) {
			SetMode( work, TreadMove ) ;
		}
	}
}

/* 立ち泳ぎ移動 */
static	void	TreadMove( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
//	UnsetFlag( FLAG_NO_IK ) ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMtread, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;		
		work->control.turn.vx = 0 ;
		WaterCameraOff() ;
	}
	SetWaterStatus() ;
	work->control.mov.vy = GV_NearSpeedF( work->control.mov.vy, HorizonLevel2,
										  16.0F ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;
	work->control.step.vy = 0.0F ;
	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, TreadStill ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else if ( work->pad->press & PAD_LOWER ) {
		SetMode( work, Tread2Dive ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else if ( !( work->pad->status & PAD_UDLR ) ) {
		SetMode( work, TreadStill ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else {
//		SeekTurnL( work ) ;
        if ( work->pad->dir >= 0 ) {
			work->control.turn.vy 
				= GV_NearSpeed( GV_NearPhase( work->control.turn.vy, work->pad->dir ),
							   work->pad->dir,   
							   48 ) ;
		}
		/* アナログ押し込みで速さを変える */
		{
			int			pressure ;
			float		speed ;

			pressure = PL_StrongestUDLRValue( work->pad ) ;
			speed = ( float )TIME_BASE ;
			if ( pressure > 160 && pressure <= 196 ) {
				speed *= 1.5F ;
			} else if ( pressure > 224 ) {
				speed *= 2.0F ;
			}
			MT_SetMotionSpeed( work->body.m_ctrl, speed ) ;
		}
	}
}

/* 立ち泳ぎからダイブ */
static	void	Tread2Dive( PlayerWork *work, int time ) 
{
//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMtread2dive, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	work->control.step.vy = 0.0F ;
	SeekTurnX( work ) ;
	MotionDouble( work ) ;
#ifdef TOGGLE
	if ( PL_CheckMotionRate( work ) > CHANGE_MRATE ) {
		if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_MODE ) ) {
			if ( work->control.mov.vy > HL() ) {
				SetMode( work, Dive2Tread ) ;
			} else {
				SetMode( work, Dive2Stand ) ;
			}
			return ;
		}
	}
	SeekTurnL( work ) ;
	AvoidSinkforSwim( work ) ;

#endif
	if ( EndMotion( work ) ) {
#ifdef TOGGLE
		SetMode( work, Dive ) ;
#else
		if ( ( work->pad->status & PAD_MODE ) || ( work->data2 > 0 ) ) {
			SetMode( work, Dive ) ;
		} else if ( work->control.mov.vy > HL() ) {
			SetMode( work, Dive2Tread ) ;
		} else {
			SetMode( work, Dive2Stand ) ;
		}
#endif
	}
}

/* ダイブから立ち泳ぎ */
static	void	Dive2Tread( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMdive2tread, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		WaterCameraOff() ;
	}
	SetWaterStatus() ;
	KeepHorizonLevel( work ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;
#if 0
	if ( work->pad->press & PAD_MODE ) {
		SetMode( work, Tread2Dive ) ;
		return ;
	}
#endif
	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	}

	if ( EndMotion( work ) ) {
		SetMode( work, TreadStill ) ;
	}
}

/* 水底立ち静止 */
static	void	SeabedStill( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
//	UnsetFlag( FLAG_NO_IK ) ;
//	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetAction( work, WMseabed_idle, 12 ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.grounded = 0 ;
		work->control.turn.vx = 0 ;
		WaterCameraOn() ;
		PL_ObjPos( work, HUMAN21_ATAMA, &work->fv ) ;
	}
	SetWaterStatus() ;
	UnsetFlag( FLAG_NO_IK ) ;
	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	ModeFlag |= MF_STAND ;
	DG_COPY_VEC( &work->camera, &work->fv ) ;

	work->control.step.vy = -FALL_SPEED_BOTTOM ;
	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		if ( work->control.mov.vy > HL() ) {
			SetMode( work, TreadStill ) ;
		} else if ( work->pad->press & ( PAD_UPPER | PAD_LOWER ) ) {
			SetMode( work, Seabed2Dive ) ;
		} else if ( work->pad->status & ( PAD_U | PAD_L | PAD_R ) ) {
			SetMode( work, SeabedWalk ) ;
		} else {
//			if ( work->pad->press & PAD_D ) SetMode( work, TouchTurnB ) ;
		}
	}
}

/* 水底歩き */
static	void	SeabedWalk( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
//	UnsetFlag( FLAG_NO_IK ) ;
//	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;

	ViewHorizon() ;
	if ( time == 0 ) {
		SetAction( work, WMseabed_walk, 12 ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.grounded = 0 ;
		work->control.turn.vx = 0 ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	UnsetFlag( FLAG_NO_IK ) ;
	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;

	/* 2倍 */
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.0F ) ;
	work->control.step.vy = -FALL_SPEED_BOTTOM ;

	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, SeabedStill ) ;
	} else {
		if ( work->control.mov.vy > HL() ) {
			SetMode( work, TreadStill ) ;
		} else if ( work->pad->press & ( PAD_UPPER | PAD_LOWER ) ) {
			SetMode( work, Seabed2Dive ) ;	
		} else if ( !( work->pad->status & ( PAD_U | PAD_L | PAD_R ) ) ) {
			SetMode( work, SeabedStill ) ;
		} else {
//			if ( work->pad->press & PAD_D ) SetMode( work, TouchTurnB ) ;
//			else SeekTurnL( work ) ;
            SeekTurnL( work ) ;
		}
	}
}

/* 水底からダイブ */
static	void	Seabed2Dive( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMseabed2dive, -1 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

	SeekTurnX( work ) ;
	SeekTurnL( work ) ;
	MotionDouble( work ) ;
	AvoidSinkforSwim( work ) ;
#if 0
	if ( work->ftime > 8 && work->pad->press & PAD_MODE ) {
		SetMode( work, Dive ) ;
		return ;
	}
#endif

	if ( EndMotion( work ) ) {
		SetMode( work, Dive ) ;
	}
}

/* ダイブから水底立ち */
static	void	Dive2Stand( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMbreast2stand, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	KeepHorizonLevel( work ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		SeekTurnL( work ) ;
	}

	if ( EndMotion( work ) ) {
		SetMode( work, SeabedStill ) ;
	}
}

/* ダイブ泳ぎ */
static	void	Dive( PlayerWork *work, int time )
{
	int			sink, mode, max ;
	int			mtime ;

//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		if ( work->data == 1 ) {
			SetActionL( work, WMdive_end, -1 ) ;
		} else {
			SetActionL( work, WMdive_end_st, 24 ) ;
		}
		work->data = 0 ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		WaterCameraOn() ;
		work->sv.vx = 0 ;
		work->sv.vy = 0 ;
		work->sv.vz = 0 ;
		work->idata = 0 ;
	}
	SetWaterStatus() ;
	work->control.step.vy = work->body.m_ctrl->step.vy ;

	/* 高さ処理 */
	if ( 1 ) {
		DG_OBJ			*obj ;
		DG_MDL			*mdl ;
		FVECTOR			pos, vec ;
		float			max, min, y ;
		float			lheight, uheight ;
		int				i, n_models, flag ;
		float			levels[ 2 ], len ;

		PL_ObjPos( work, HUMAN21_KUBI, &pos ) ;
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &pos,
								    HZX_CHK_ALL, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
			HZX_GetOnlineVector( &vec ) ;
			len = GV_VecLen3F( &vec ) ;
			if ( len > 8.0F ) {
				GV_LenVec3F( &vec, &vec, 0.0F, len - 8.0F ) ;
				_sceVu0AddVector( &pos, &work->control.mov, &vec ) ;
			}
			printf( "koshi kubi hazard! %f\n", len ) ;
		}

		flag = HZX_LevelHazardCheck( work->control.hzx_id, &pos, 
									 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
		HZX_GetLevelHeight( levels ) ;
		if ( !( flag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
		if ( !( flag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
		levels[ 0 ] = ( work->control.levels[ 0 ] > levels[ 0 ] ) ? 
			work->control.levels[ 0 ] : levels[ 0 ] ;
		levels[ 1 ] = ( work->control.levels[ 1 ] < levels[ 1 ] ) ? 
			work->control.levels[ 1 ] : levels[ 1 ] ;

		max = -1000000.0F ; min = 1000000.0F ;
		n_models = work->body.objs->def->n_models ;
		obj = work->body.objs->objs ;
		mdl = work->body.objs->def->models ;
		for ( i = 0; i <= HUMAN21_ATAMA; i ++, obj ++, mdl ++ ) { /* 上半身だけ */
			y = obj->world.m[ 3 ][ 1 ] ;
			if ( y > max ) max = y ;
			if ( y < min ) min = y ;
		}
		lheight = work->control.mov.vy - min + 100.0F ;
		uheight = max - work->control.mov.vy + 100.0F ;
//printf( "[%d] : %f %f %f %f %f\n", GV_Time, work->control.mov.vy, 	
//	   levels[ 0 ], levels[ 1 ], lheight, uheight ) ;
		if ( lheight < work->control.height ) lheight = work->control.height ;
		if ( levels[ 0 ] + lheight > work->control.mov.vy ) {
			work->control.step.vy += GV_NearExp4F( 0.0F, levels[ 0 ] + lheight 
				- work->control.mov.vy ) ;
		}
		if ( levels[ 1 ] - uheight < work->control.mov.vy ) {
			work->control.step.vy -= GV_NearExp4F( 0.0F, work->control.mov.vy 
				- ( levels[ 1 ] - uheight ) ) ;
		}
	}

	SetFlag( FLAG_NO_MOTION_STEP_Y ) ;

	SeekTurnL( work ) ;
	SeekTurnX( work ) ;

	if ( work->control.turn.vx < 0 && 
		 work->control.mov.vy > HL() &&
		 work->control.levels[ 1 ] > HorizonLevel ) {
		SetMode( work, Dive2Tread ) ;
		return ;
	}

	mode = PL_MoveLevel( work ) ;
	switch( mode ) {
	case 0 :
		SetActionL( work, WMdive_end_st, 12 ) ;
		break ;
	case 1 :
		if ( work->sv.vy == 0 ) {
			SetActionL( work, WMdive_arm_r, 24 ) ;		
			if ( EndMotion( work ) ) {		
				Bubble( GM_PlayerWork, HUMAN21_MIGI_TE ) ;
				Bubble( GM_PlayerWork, HUMAN21_MIGI_KAKATO ) ;
				work->sv.vy = 1 ;
			}
		} else {
			SetActionL( work, WMdive_arm_l, 24 ) ;
			if ( EndMotion( work ) ) {		
				Bubble( GM_PlayerWork, HUMAN21_HIDARI_TE ) ;
				Bubble( GM_PlayerWork, HUMAN21_HIDARI_KAKATO ) ;
				work->sv.vy = 0 ;
			}
		}
		break ;
	case 2 :
		SetActionL( work, WMdive_max, -1 ) ;
		CamTrackAim = CAMERA_TRACK_FAR ;
		if ( EndMotion( work ) ) {
			Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		}
		break ;
	}

	/* 体Ｚ回転 */
	work->control.turn.vz = WaterCamera->rotate.vz * 6 ;

	sink = AvoidSinkforSwim( work ) ;

    if ( ( sink & 1 ) && ( sink & 8 ) && !( sink & 4 ) ) {
		work->idata += 3 ;
	} else {
		if ( work->idata > 0 ) -- work->idata ;
	}
	if ( ( sink & 1 ) && !( sink & 8 ) ) {
		work->sv.vx += 8 ;
	} else {
		if ( work->sv.vx > 0 ) -- work->sv.vx ;
	}

	if ( work->idata > 64 || work->sv.vx > 64 ) {
//		SetMode( work, Dive2Stand ) ;
//		return ;
	}
	if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		PL_SubjectTurn( work ) ;
	}
}

#endif

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

#ifdef A_PATTERN
/* ダイブから壁手つきへ */
static	void	Dive2WallTouch( PlayerWork *work, int time )
{
//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetActionL( work, WMwall_touch, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->control.turn.vy = AvoidDir ;
		work->control.grounded = 0 ;
	}
	SetWaterStatus() ;
	KeepHorizonLevel( work ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		PL_SubjectTurn( work ) ;
	}

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Stand2Dive ) ;
		return ;
	}

	if ( work->control.grounded & 1 ) {
		SetMode( work, SeabedStill ) ;
#if 1
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( work->pad->status & ( PAD_R | PAD_L | PAD_D ) ) ) {
		if ( work->pad->status & PAD_R ) {
			SetMode( work, TouchTurnR ) ;
		} else if ( work->pad->status & PAD_L ) {
			SetMode( work, TouchTurnL ) ;
		} else if ( work->pad->status & PAD_D ) {
			SetMode( work, TouchTurnB ) ;
		}
#endif
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( PL_CheckMotionRate( work ) > CHANGE_MRATE ) &&
			    ( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) ) {
		SetMode( work, Stand2Dive ) ;
	}
	if ( EndMotion( work ) ) {
		SetMode( work, WallTouch ) ;
	}
}

/* 壁手つき */
static	void	WallTouch( PlayerWork *work, int time )
{
	SetWaterStatus() ;
	UnsetFlag( FLAG_NO_IK ) ;
	if ( time != 0 ) {
		WaterStance = WS_STAND ;
		UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
		if ( work->ftime == 1 ) {
			work->body.m_ctrl->old_height = work->control.height ;
			work->body.m_ctrl->root_old_height = work->control.height ;
		}
	}
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMwall_idle, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.grounded = 0 ;
		work->control.turn.vx = 0 ;
		WaterCameraOn() ;
	}

    LargeMoveAdjust( work ) ;

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Stand2Dive ) ;
		return ;
	}
#if 0	
	work->control.step.vy = -FALL_SPEED ;
#endif
	if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		PL_SubjectTurn( work ) ;
#if 1
		work->control.step.vy = -FALL_SPEED ;
#endif
	} else {
#if 1
		if ( work->pad->status & PAD_U ) work->control.step.vy = FALL_SPEED ;
		else work->control.step.vy = -FALL_SPEED ;
		if ( work->pad->status & ( PAD_L | PAD_R ) ) {
			SetMode( work, WaterStand ) ;
			return ;
		}
#endif
	}

	if ( work->control.grounded & 1 ) {
		SetMode( work, SeabedStill ) ;
#if 0
	} else if ( !Status( PLAYER_WATCH ) &&
			    ( work->pad->status & ( PAD_R | PAD_L | PAD_D ) ) ) {
		if ( work->pad->status & PAD_R ) {
			SetMode( work, TouchTurnR ) ;
		} else if ( work->pad->status & PAD_L ) {
			SetMode( work, TouchTurnL ) ;
		} else if ( work->pad->status & PAD_D ) {
			SetMode( work, TouchTurnB ) ;
		}
#endif
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) ) {
		SetMode( work, Stand2Dive ) ;
	}
	CheckWaterRise( work ) ;
}
#endif

/* ターン */
static	void	TouchTurn( PlayerWork *work, int time, int action, float rate )
{
	ModeFlag |= MF_NO_CAMERA_TURN ;
//	SetWaterStatus() ;
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, action, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->control.turn.vx = 0 ;
		PL_LeaveSubject( work ) ;
	}
	CamTrackAim = CAMERA_TRACK_FAR ;
	SetWaterStatus() ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

//	SeekTurnX( work ) ;

//	SeekTurnL( work ) ;
//	MotionDouble( work ) ;
	AvoidSinkforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	if ( EndMotion( work ) ||
		( PL_CheckMotionRate( work ) > rate && ( work->pad->status & PAD_SWIM ) ) ) {
		SetMode( work, Dive ) ;
		if ( action == WMwall_turn_b ) {
			/* 180度のときは、カメラＺ回転なし */
			CamNoRzCount = 32 ;
		}
	}
}

static	void	TouchTurnR( PlayerWork *work, int time )
{
	TouchTurn( work, time, WMwall_turn_r, 0.70F ) ;
	SetNCamRot( work ) ;
}

static	void	TouchTurnL( PlayerWork *work, int time )
{
	TouchTurn( work, time, WMwall_turn_l, 0.70F ) ;
	SetNCamRot( work ) ;
}

static	void	TouchTurnB( PlayerWork *work, int time )
{
	ModeFlag |= MF_NO_CAMERA_RZ ;
	TouchTurn( work, time, WMwall_turn_b, 0.95F ) ;
	SetNCamRot( work ) ;
	CamAngleAim = 1.50F ;
}

/*----------------------------------------------------------------*/

/* 立ちダメージ */
static	void	WaterDamageStand( PlayerWork *work, int time ) 
{
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMdam_stand, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		if ( WaterStance == WS_SURFACE ) work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		else							 work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
		PL_SetInvincible( work, 0 ) ;
	}

	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE ) ;

	work->control.step.vy = -FALL_SPEED ;

	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;

	if ( work->control.mov.vy > HorizonLevel2 ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = HorizonLevel2 - work->control.mov.vy ;
	}

	LargeMoveAdjust( work ) ;

	if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_DAMAGED ) ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		if ( CeilToFloor( work ) ) {
			SetMode( work, Dive ) ;
		} else if ( WaterStance == WS_SURFACE ) {
			SetMode( work, TreadStill ) ;
		} else {
			SetMode( work, WaterStand ) ;
		}
	}
}

/* 立ちアウト */
static	void	WaterDeadStand( PlayerWork *work, int time ) 
{
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMout_stand, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		if ( WaterStance == WS_SURFACE ) work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		else							 work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
		PL_LeaveSubject( work ) ;
		PL_SetInvincible( work, 0 ) ;
	}

	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;

	work->control.step.vy = -FALL_SPEED ;

	if ( work->control.mov.vy > HorizonLevel2 ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = HorizonLevel2 - work->control.mov.vy ;
	}

	LargeMoveAdjust( work ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, WaterDozaemon ) ;
	}
}

/* ダイブダメージ */
static	void	WaterDamageLie( PlayerWork *work, int time ) 
{
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMdam_lie, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
		PL_SetInvincible( work, 0 ) ;
	}

	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE ) ;

	DiveHeightAdjust( work ) ;
	AvoidSinkforSwim( work ) ;

	if ( work->control.mov.vy > HorizonLevel2 ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = HorizonLevel2 - work->control.mov.vy ;
	}

	LargeMoveAdjust( work ) ;

	if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_DAMAGED ) ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;		
		SetMode( work, Dive ) ;
	}
}

/* ダイブアウト */
static	void	WaterDeadLie( PlayerWork *work, int time ) 
{
	ViewHorizon() ;
	if ( time == 0 ) {
		SetActionL( work, WMout_lie, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveSubject( work ) ;
	}

	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	DiveHeightAdjust( work ) ;
	AvoidSinkforSwim( work ) ;

	if ( work->control.mov.vy > HorizonLevel2 ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = HorizonLevel2 - work->control.mov.vy ;
	}

	LargeMoveAdjust( work ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, WaterDozaemon ) ;
	}
}

/* ドザエモン */
static	void	WaterDozaemon( PlayerWork *work, int time ) 
{
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	PL_SetDeadFlag( work ) ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMdozaemon, -1 ) ;
		PL_SetInvincible( work, 0 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		work->control.turn.vx = 0 ;
	}
	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	DiveHeightAdjust( work ) ;
	AvoidSinkforSwim( work ) ;

	SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
	if ( work->control.mov.vy > HorizonLevel2 ) {
		work->control.step.vy = HorizonLevel2 - work->control.mov.vy ;
		work->data = 1 ;
	} else {
		if ( work->data == 1 ) work->control.step.vy = 0.0F ;
		else			       work->control.step.vy = FALL_SPEED / 2.0F ;
	}

	LargeMoveAdjust( work ) ;

	if ( work->data2 == 0 && EndMotion( work ) ) {
		PL_MotionSleep( work, 0 ) ;
		GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
		GM_GameOverProcEnd( &work->actor ) ;
		work->data = 2 ;
	}
}

static	void	WaterNoO2Dead( PlayerWork *work, int time ) 
{

}

/*----------------------------------------------------------------*/

/* 特殊モードからの戻り */
static	void	ReturnSeabedStill( PlayerWork *work, int time )
{
	UnsetFlag( FLAG_FORCE_END ) ;
	UnsetStatus( PLAYER_FORCE ) ;
	PL_ChangeMotionArc( work, Motion ) ;
	PL_SetAction( work, WMseabed_idle, 12 ) ;
	SetMode( work, SeabedStill ) ;
	work->control.step.vy = 0.0F ;
}

/* 水中モード入る */
static	void	IntoWater( PlayerWork *work, int time ) 
{
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH |
			 FLAG_NO_IK | FLAG_NO_GRAVITY ) ;
	if ( time == 0 ) {
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}
	if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_FORCE ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		SetStatus( PLAYER_IN_THE_WATER | PLAYER_WATER_SURFACE ) ;
		UpdateCamera( work ) ;
		/* トラップチェック位置初期化 */
		_sceVu0AddVector( &TrapCheckPos, &work->root_diff, &work->control.mov ) ;
		SetMode( work, TreadStill ) ;
		ModeFlag = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_SetMenuStatus( MENU_WEAPON_OFF ) ;
		GM_ResetMenuStatus( MENU_ITEM_OFF ) ;
		PL_DamageFunc = WaterCheckDamage ;
		work->dmg_callback = WaterDamageCallback ;

		PL_AddDeadAction( &DeadModeSet, WaterDeadModeNoO2, WaterDeadModeConditionFunc ) ;

		/* 戻りモード設定 */
		PL_ReturnMode = ReturnSeabedStill ;
		PL_ReturnModeMotionArc = Motion ;
#ifdef DEBUG_MODE
		GM_InitRadarControl( &Rctrl, &WaterCamera->position,
							 RADAR_VISIBLE, -1 ) ;
#endif
	}
	work->control.step.vy = 0.0F ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
}

/* 水中モード出る */
static	void	LeaveWater( PlayerWork *work, int time )
{
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH |
			 FLAG_NO_IK | FLAG_NO_GRAVITY ) ;
	UnsetStatus( PLAYER_IN_THE_WATER ) ;
	if ( time == 0 ) {
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}
	if ( EndMotion( work ) ) {
		SetFlag( FLAG_FORCE_END ) ;
		UnsetStatus( PLAYER_FORCE ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		SetMode( work, PL_StillMode[ 0 ] ) ;
		ModeFlag = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		if ( PL_DamageFunc == WaterCheckDamage ) PL_DamageFunc = NULL ;
		if ( work->dmg_callback == WaterDamageCallback ) work->dmg_callback = NULL ;
		PL_RemoveDeadAction( &DeadModeSet ) ;
		/* 戻りモード解除 */
		if ( PL_ReturnMode == ReturnSeabedStill ) PL_ReturnMode = NULL ;
		if ( PL_ReturnModeMotionArc == Motion ) PL_ReturnModeMotionArc = -1 ;
#ifdef DEBUG_MODE
		GM_FreeRadarControl( &Rctrl ) ;
#endif
	}
	work->control.step.vy = 0.0F ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
}

/*---------------------------------------------------------------

  初期化

---------------------------------------------------------------*/

/* カメラ */
static	void	InitWaterCamera( void )
{
	GM_CameraSet	*cam ;
	int				name ;

	WaterCamera = NULL ;
	name = GV_StrCode( "水中カメラ" ) ;
	name += 0 ; /* チャンネル */
	cam = WaterCamera = NewProgramCamera( name, 0, GM_CAMERA_PROG2, 254 ) ;
#ifdef TYPE_A
	GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET,
					  CAM_FLAG_PAD_ADJUST | CAM_FLAG_NO_CUSHION ) ;
#endif
#ifdef TYPE_B
//	GM_SetCameraType( cam, GM_CAM_TYPE_TARGET_AND_ROTATE,
//					  CAM_FLAG_TRACE | CAM_FLAG_PAD_ADJUST ) ;
	GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET,
					  CAM_FLAG_FIX | CAM_FLAG_PAD_ADJUST ) ;
#endif
	GM_SetCameraTrack( cam, 4000.0F ) ;
	GM_SetCameraAngle( cam, 1.75F ) ;
	CamAngle = CamAngleAim = 1.75F ;
	GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4,
						    GM_CAM_INTERP_EXP4, 0, 0 ) ;
	DG_COPY_VEC( &cam->position, &GM_PlayerPosition ) ;
	DG_COPY_VEC( &cam->target, &GM_PlayerPosition ) ;
	cam->on = 0 ;
	ASSERT( WaterCamera != NULL ) ;
}

static	int		SetWaterAct( PlayerWork *work, GV_MSG *msg, int len )
{
	int			which, motion, dir ;
	FVECTOR		start_pos ;

	if ( msg != NULL ) {
		which = msg->message[ 1 ] ;
		motion = msg->message[ 2 ] ;
		dir = msg->message[ 3 ] ;
		DG_COPY_VEC( &start_pos, &work->control.mov ) ;
		if ( len > 4 ) start_pos.vx = ( float )msg->message[ 4 ] ;
		if ( len > 5 ) start_pos.vz = ( float )msg->message[ 5 ] ;
		if ( len > 6 ) start_pos.vy = ( float )msg->message[ 6 ] ;
		if ( len > 4 ) {
			/* 開始位置移動 */
			GM_ResetControlPosition( &work->control, &start_pos ) ;
		}
	} else {
		/* 強制モーションから来たっぽい */
		which = GM_STRCODE_ENTER ;
		motion = 0 ;
		dir = -1 ;
		DG_COPY_VEC( &start_pos, &work->control.mov ) ;
		UnsetStatus( PLAYER_FORCE | PLAYER_PAD_OFF ) ;
		UnsetFlag( FLAG_FORCE | FLAG_FORCE_END ) ;
	}

	if ( which == GM_STRCODE_ENTER ) {
		/* 入水 */
		/* 以下のときは無効 */
		if ( Status( PLAYER_DAMAGED | PLAYER_DOWNED | PLAYER_INTRUDE |
					 PLAYER_FORCE | PLAYER_CB_BOX | PLAYER_LOCKER |
					 PLAYER_IN_THE_WATER | PLAYER_DEAD ) ||
			 ( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
			 ( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return -1 ;
		/* エルードからの場合特殊 */
		if ( Status( PLAYER_BEYOND ) ) {
			UnsetStatus( PLAYER_BEYOND | PLAYER_PAD_OFF ) ;			
			UnsetFlag( FLAG_FORCE ) ;
		}
		if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return -1 ;
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveCaution( work ) ;
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
		SetStatus( PLAYER_FORCE | PLAYER_WEAPON_INVISIBLE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		if ( msg != NULL ) {
			PL_ChangeMotionArc( work, PL_Force->marfile ) ;
			SetAction( work, motion, 6 ) ;
			SetMode( work, IntoWater ) ;
		} else {
			IntoWater( work, 0 ) ;
		}
	} else {
		/* 出水 */
		work->control.turn.vx = work->control.rot.vx = 0 ;
		if ( motion < 0 ) {
			work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
										  CTRL_SKIP_TRAP ) ;
			work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
			UnsetStatus( PLAYER_IN_THE_WATER ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
			ModeFlag = 0 ;
			PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
			GM_ResetMenuStatus( MENU_MENU_OFF ) ;
			SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
			if ( PL_DamageFunc == WaterCheckDamage ) PL_DamageFunc = NULL ;
			if ( work->dmg_callback == WaterDamageCallback ) work->dmg_callback = NULL ;
			PL_RemoveDeadAction( &DeadModeSet ) ;
			/* 戻りモード解除 */
			if ( PL_ReturnMode == ReturnSeabedStill ) PL_ReturnMode = NULL ;
			if ( PL_ReturnModeMotionArc == Motion ) PL_ReturnModeMotionArc = -1 ;
			WaterCameraOff() ;
#ifdef DEBUG_MODE
			GM_FreeRadarControl( &Rctrl ) ;
#endif
			return 1 ;
		}
		if ( Status( PLAYER_FORCE ) || Flag( FLAG_FORCE ) ) return -1 ;
		PL_SetInvincible( work, 0 ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
		PL_LeaveSubject( work ) ;
		SetStatus( PLAYER_FORCE | PLAYER_WEAPON_INVISIBLE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		WaterCameraOff() ;
		PL_ChangeMotionArc( work, PL_Force->marfile ) ;
		SetAction( work, motion, 6 ) ;
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		work->control.turn.vx = 0 ;
		SetMode( work, LeaveWater ) ;
		PL_Force->flag &= ~FA_USE_DEFAULT ;
		PL_Force->flag &= ~FA_NO_RECHECK_TRP ;
		PL_Force->e_turn = -1 ;
	}
	return 1 ;
#if 0
	UnsetFlag( FLAG_FORCE_END ) ;
	UnsetStatus( PLAYER_FORCE ) ;
	PL_ChangeMotionArc( work, Motion ) ;
	work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
								  CTRL_SKIP_TRAP ) ;
	SetStatus( PLAYER_IN_THE_WATER ) ;
	UpdateCamera( work ) ;
	WaterCameraOn() ;
	SetMode( work, TreadStill ) ;
	ModeFlag = 0 ;
#ifdef DEBUG_MODE
	GM_InitRadarControl( &Rctrl, &WaterCamera->position,
						 RADAR_VISIBLE, -1 ) ;
#endif
	return 1 ;
#endif
}


/*---------------------------------------------------------------

  ダメージチェック

---------------------------------------------------------------*/

static	void	WaterDamageCallback( PlayerWork *work, TARGET *off, TARGET *def )
{
	extern void *NewBloodWater( FVECTOR *pos, FVECTOR *vec, float pow, int mode ) ;
	FVECTOR		pos, force ;

	UnsetFlag( FLAG_BLOOD_SPLASH ) ;
	PL_ObjPos( work, HUMAN21_KUBI, &pos ) ;
	_sceVu0SubVector( &force, &pos, &off->center ) ;
	/* 水中血 */
	NewBloodWater( &pos, &force, 16, 0 ) ;
}

static	void	WaterCheckDamage( PlayerWork *work, long64 weapon_type, int dead )
{
	int			seNo ;
	PL_ACTION	next ;

	weapon_type &= ~( WP_THROWG | WP_BOXKICK ) ;
	if ( weapon_type == I64(0) ) {
		GM_ClearTargetDamage( &( work->def ) ) ;
		return ;
	}
	if ( dead ) {
		NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;		
		seNo = SD_V_POUT0001 ;
		if ( WaterStance == WS_SWIM ) next = WaterDeadLie ;
		else						  next = WaterDeadStand ;
	} else if ( weapon_type & ( WP_BLOW | WP_SOFTBLOW ) ) {
		/* 吹っ飛び */
		NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;		
		seNo = SD_V_PDMG01 ;
		if ( WaterStance == WS_SWIM ) next = WaterDamageLie ;
		else						  next = WaterDamageStand ;
	} else {
		/* のけぞり */
		NewPadVibration( PL_DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib1L, 2 | VAR_FLAG_FORCE ) ;
		seNo = SD_V_PDMG02 ;
		if ( WaterStance == WS_SWIM ) next = WaterDamageLie ;
		else						  next = WaterDamageStand ;
	}
	PL_SetMode( work, next ) ;
	GM_SeSetMode( seNo, &work->control.mov, GM_SEMODE_BOMB ) ;
	PL_SetInvincible( work, 0 ) ;
	GM_VctrlStopVibration( &work->vctrl ) ;
	GM_ClearTargetDamage( &( work->def ) ) ;
	GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;	
}

/*---------------------------------------------------------------

  溺れゲームオーバー登録

---------------------------------------------------------------*/

/* 判定 */
static	int		WaterDeadModeConditionFunc( PlayerWork *work )
{
	return ( GM_O2 <= 0 ) ? 1 : 0 ;
}

/* 溺れ死に */
static	void	WaterDeadModeNoO2( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetAction( work, WMnoair_stand, 6 ) ;
		SetFlag( FLAG_NO_WAIST_INTERP ) ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;		
		GM_GameOverProcStart( &work->actor ) ;
		PL_LeaveSubject( work ) ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
		SetStatus( PLAYER_DEAD ) ;
		PL_SetDeadFlag( work ) ;
		work->power.vital = 0 ;
		work->control.turn.vx = 0 ;
	}

	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	PL_SetFlag( FLAG_NO_GRAVITY ) ;
	work->control.step.vy = 0.0F ;

	/* 念のため */
	SetStatus( PLAYER_DEAD ) ;
	PL_SetDeadFlag( work ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, WaterDozaemon ) ;
//		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
//		GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
//		GM_GameOverProcEnd( &work->actor ) ;
	}	
}

/*---------------------------------------------------------------

  ポーリング関数

---------------------------------------------------------------*/

static	int	PollingFuncWater( PlayerWork *work )
{
	/* 水位を常に更新 */
	HorizonLevel = GM_WaterLevel ;
	HorizonLevel2 = HorizonLevel - FLOAT_HEIGHT ;

	/* 水面フラグ、無呼吸フラグ毎フレームリセット */
	UnsetStatus( PLAYER_WATER_SURFACE | PLAYER_NO_BREATH ) ;

	if ( !Status( PLAYER_IN_THE_WATER ) && WaterCamera->on == 1 ) {
		WaterCameraOff() ;
		work->control.skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
		work->control.turn.vx = 0 ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	}

	/* エルード落下チェック */
	if ( work->act_name == ELUDE_FALL ) {
		if ( work->control.mov.vy < GM_WaterLevel ) {
			if ( Status( PLAYER_DEAD ) && Dead2WaterMotion != -1 ) {
				if ( work->action != EludeFallDeadWater ) {
					PL_SetMode( work, EludeFallDeadWater ) ;
					return 1 ;
				}
			} else if ( Elude2WaterMotion != -1 ) {
				int		buf[ 4 ] ;

				buf[ 0 ] = PL_MSG_WATER ;
				buf[ 1 ] = GM_STRCODE_ENTER ;
				buf[ 2 ] = Elude2WaterMotion ;
				buf[ 3 ] = -1 ;
				PL_SendMessage( GM_PlayerControl->name, buf, 4 ) ;
				//UnsetStatus( PLAYER_BEYOND | PLAYER_PAD_OFF ) ;
				PL_UnsetInvincible( work ) ;
				work->control.skip_flag &= ~( CTRL_SKIP_HZX | CTRL_SKIP_TRAP ) ;
				return 1 ;
			}
		}
	}
	return 0 ;
}

/*---------------------------------------------------------------

  起動

---------------------------------------------------------------*/

/* プラグイン登録 */
int		NewPluginWaterMode( void )
{
    PL_AddPlugin( &PluginSet, PL_MSG_WATER, SetWaterAct, NULL ) ;
	PL_AddPollingFunc( &PollingSet, PollingFuncWater ) ;
	InitWaterCamera() ;
	if ( GCL_GetOption( 'z' ) != NULL ) {
		WaterZoneGroupID = GV_GetBit( GCL_GetNextInt() ) ;
	} else {
		WaterZoneGroupID = 0 ;
	}
	RAIL_HEIGHT = ( float )GCL_GetOptionValue( 'h', 1000 ) ;
	LastAvoidSinkTime = 0 ;
    return 0 ;
}

/* 設定 */
int		NewWaterModeSetting( void )
{
#if 0
	/* ここでは設定しない */
	/* command 水位設定 でやる */
	if ( GCL_GetOption( 'h' ) != NULL ) {
		HorizonLevel = ( float )GCL_GetNextInt() ;
		HorizonLevel2 = HorizonLevel - FLOAT_HEIGHT ;
		GM_WaterLevel = HorizonLevel ;
	}
#else
	HorizonLevel = GM_WaterLevel ;
	HorizonLevel2 = HorizonLevel - FLOAT_HEIGHT ;
#endif

	if ( GCL_GetOption( 'm' ) != NULL ) {
		Motion = GCL_GetOptionValue( 'm', 0 ) ;
	}

	WaterFlag = GCL_GetOptionValue( 'f', 0 ) ;

	/* エルード落下から水中に入るモーション */
	Elude2WaterMotion = GCL_GetOptionValue( 'e', -1 ) ;
	Dead2WaterMotion = GCL_GetOptionValue( 'd', -1 ) ;

	return 0 ;
}

#if 0
/* カメラのバウンド設定 */
int		NewSetWaterCameraBound( void )
{
	int			trap ;

	return 0 ;

	if ( WaterCamera == NULL ) return 0 ;
	trap = GCL_GetOptionValue( 't', 0 ) ;
	if ( trap != 0 ) {
		int				map, no, bit ;
		HZX_GROUP_ID	hzx_id ;
		HZX_GRP			*grp ;
		HZX_BLOCK		*blk ;
		HZX_TRP			*trp ;
		HZX_HDL			*hdl ;
		int				n_blocks, n_traps ;
#ifdef HZX_DTRP
		HZX_D_TRP		*dtrp ;
#endif
		hzx_id = GM_GetHzxGroupID( GM_CurrentStageMap ) ;
		if ( hzx_id == 0 ) return 0 ;
		hdl = HZX_GetCurrentHzx() ;
		while( hzx_id != 0 ) {
			no = GV_GetNo( hzx_id ) ;
			bit = GV_GetBit( no ) ;
			hzx_id &= ~bit ;
			grp = hdl->def->groups + no ;
			blk = grp->blocks ;
			n_blocks = grp->n_blocks ;
			while( -- n_blocks >= 0 ) {
				n_traps = blk->n_traps ;
				trp = blk->traps ;
				while( -- n_traps >= 0 ) {
					if ( trap == trp->name_id ) {
						WaterCamera->bound1.vx = ( float )( blk->tx + trp->b1.vx ) ;
						WaterCamera->bound1.vy = ( float )( blk->ty + trp->b1.vy ) ;
						WaterCamera->bound1.vz = ( float )( blk->tz + trp->b1.vz ) ;
						WaterCamera->bound2.vx = ( float )( blk->tx + trp->b2.vx ) ;
						WaterCamera->bound2.vy = ( float )( blk->ty + trp->b2.vy ) ;
						WaterCamera->bound2.vz = ( float )( blk->tz + trp->b2.vz ) ;
						goto bound_set_end ;
					}
					trp ++ ;
				}
				blk ++ ;
			}
#ifdef HZX_DTRP 
			dtrp = grp->dynamics->traps ;
			while( dtrp != NULL ) {
				if ( trap == dtrp->name_id ) {
					DG_COPY_VEC( &WaterCamera->bound1, &dtrp->b1 ) ;
					DG_COPY_VEC( &WaterCamera->bound2, &dtrp->b2 ) ;
					goto bound_set_end ;
				}
				dtrp = dtrp->next ;
			}
#endif
		}
	}
bound_set_end :
	if ( GCL_GetOption( 'I' ) != NULL ) {
		PL_GetNextFV( &WaterCamera->bound1 ) ;
	}
	if ( GCL_GetOption( 'A' ) != NULL ) {
		PL_GetNextFV( &WaterCamera->bound2 ) ;
	}

	{
		FVECTOR		v ;

		GV_SetVec3( &v, 500.0F, 500.0F, 500.0F ) ;
		_sceVu0AddVector( &WaterCamera->limit1, &WaterCamera->bound1, &v ) ;
		GV_SetVec3( &v, -500.0F, -500.0F, -500.0F ) ;
		_sceVu0AddVector( &WaterCamera->limit2, &WaterCamera->bound2, &v ) ;
	}

	WaterCamera->flag |= CAM_FLAG_BOUND | CAM_FLAG_LIMIT | CAM_FLAG_BOUNDCHANGE_QUICK ;
	return 0 ;
}
#endif

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* 息継ぎポイント管理 */

typedef	struct _BreathPoint	{
	FVECTOR					pos ;
	struct _BreathPoint 	*next ;
} BreathPoint ;
 
typedef	struct	{
	GV_ACT			actor ;
	BreathPoint		list ;
	int				n_points ;
} BPMNGWORK ;

static	BPMNGWORK	*BPMngWork = NULL ;

static	void	BPMngAct( BPMNGWORK *work )
{
}

static	void	BPMngDie( BPMNGWORK *work )
{
	BreathPoint		*list, *this ;

	list = work->list.next ;
	while( list != NULL ) {
		this = list ;
		list = list->next ;
		GV_DelayedFree( this ) ;
	}
	BPMngWork = NULL ;
}

void		*NewBreathPointManager( int name, int where )
{
	BPMNGWORK		*work ;

	work = ( BPMNGWORK * )GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT,
										  sizeof( BPMNGWORK ), 0 ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, BPMngAct, BPMngDie ) ;
	GV_SleepActor( &work->actor, GV_CLASS_WAITING ) ;
	work->list.next = NULL ;
	work->n_points = 0 ;
	BPMngWork = work ;
	return work ;
}

/* 息継ぎポイント登録 */
int			NewAddBreathPoint( void )
{
	BreathPoint		*bp ;

	ASSERT( BPMngWork != NULL ) ;
	bp = ( BreathPoint * )GV_Malloc( sizeof( BreathPoint ) ) ;
	if ( bp == NULL ) return 0 ;
	PL_GetOptionFV( 'p', &bp->pos ) ;

	bp->next = BPMngWork->list.next ;
	BPMngWork->list.next = bp ;

	return 1 ;
}

/* 息継ぎポイントへの吸い込み処理 */
static	void  		GuideToBreathPoint( FVECTOR *step, FVECTOR *mov, float guide_len, float step_len )
{
	BreathPoint 	*bp, *this ;
	FVECTOR			diff ;
	float			len ;

	if ( BPMngWork == NULL ) return ;
	bp = BPMngWork->list.next ;
	while( bp != NULL ) {
		this = bp ;
		bp = bp->next ;
		_sceVu0SubVector( &diff, &this->pos, mov ) ;
		len = GV_VecLen3F( &diff ) ;
		if ( len > guide_len ) continue ;
		diff.vy = 0.0F ;
		GV_LenVec3F( &diff, step, 0.0F, step_len ) ;
DumpVec( step ) ;
		return ;
	}
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* エルード死亡落下水中ドボン */
static	void	EludeFallDeadWater( PlayerWork *work, int time )
{
	BEYOND		*b ;

	b = PL_Beyond ;
	if ( time == 0 ) {
		SetAction( work, Dead2WaterMotion, 6 ) ;
		SetFlag( FLAG_NO_WAIST_INTERP ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;		
		GM_GameOverProcStart( &work->actor ) ;
		PL_LeaveSubject( work ) ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
		SetStatus( PLAYER_DEAD ) ;
		PL_SetDeadFlag( work ) ;
		work->power.vital = 0 ;
	}

	PL_LevelCheck( work ) ;
	PL_SetFlag( FLAG_NO_GRAVITY ) ;
	work->control.step.vy = 0.0F ;

	/* 念のため */
	SetStatus( PLAYER_DEAD ) ;
	PL_SetDeadFlag( work ) ;

	if ( EndMotion( work ) ) {
		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
		GM_GameOverProcEnd( &work->actor ) ;
	}
}

