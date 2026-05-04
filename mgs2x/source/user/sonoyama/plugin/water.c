//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   water.c
   水中モードプラグイン（正式版）

   2000/08/04 M.Sonoyama
   $Id: water.c,v 1.1.1.3 2002/11/19 11:50:51 Yoshizawa1 Exp $
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
#include	"rand.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"
#include	"../raiden/motion.h"
#include	"ee_swim.h"

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
	WMbataashi_idle,
	WMbataashi_min,
	WMbataashi_short,
	WMstop2bataashi,
	WMstop2seabed,
	WMwall_turn_r_l,
	WMwall_turn_l_l,
	WMwall_turn_b_l,
	WMdive_armshift,
	WMnoair_lie,
	WMbataashi_idle_bend,
	MAX_WATER_MOTIONS
} ;

/* フラグ */
enum {
	WATER_FLAG_NORMAL		= 	0x0000,
	WATER_FLAG_AUTO_ROTATE	=	0x0001,		//角度自動調節機能付き
	WATER_FLAG_NO_ZONE_TRACE =	0x0002,		//ゾーンを使用した追従をしない
    WATER_FLAG_NO_FLOOR =		0x0004,		//底無し(w11)
} ;

#define	STRCODE_SEABED		(1766464)	/* GV_StrCode( "水底から" ) */
#define	STRCODE_DIVE		(12252221)	/* GV_StrCode( "水中から" ) */
#define	STRCODE_TREAD		(1766478)	/* GV_StrCode( "水面から" ) */
#define	STRCODE_STAIR_ENTER	(840190)	/* GV_StrCode( "階段入る" ) */
#define	STRCODE_STAIR_LEAVE	(498174)	/* GV_StrCode( "階段出る" ) */

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

#define	CAMERA_TURNX		(96)
#define	CAMERA_TURNX_ABS	(96)
#define	CAMERA_TRACK_NEAR		(2250.0F)
#define	CAMERA_TRACK_NORMAL		(2500.0F)
#define	CAMERA_TRACK_FAR		(4000.0F)
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

/* グローバル */
int			PL_EE_Exist ;
int			PL_EE_Mode ;
int			PL_EE_Flag ;
int			PL_EE_Motion ;
int			PL_EE_TurnEnd ;
int			PL_EE_Damage ;
int			PL_EE_O2Damage ;

CONTROL		*PL_WaterCamControl ;

extern 	void 	*NewBloodWater( FVECTOR *pos, FVECTOR *vec, float pow, int mode ) ;

extern void* NewDrowingBubble( int name, FVECTOR* pvecPos,
							   FVECTOR*	pvecInitDir, float fSizeMin,
							   float fSizeMax, int nLifeMin,
							   int nLifeMax, int nRGBA ) ;

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
int						DiveSpeed ;

GM_CameraSet			*PL_WaterCamera ;
#define	WaterCamera		PL_WaterCamera 

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

static	int				Procs[ 4 ] ;
static	FVECTOR			BubblePos ;

static	void			*ScrDropAct ;

//#define	C_DEBUG_MODE

#ifdef C_DEBUG_MODE
static	RADAR_CTRL		Rctrl ;
#endif

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
	MF_WATER_SURFACE =	0x0040,
	MF_CLEAR =			0x00ff,
	MF_SEEKDOWN_NEW_PRESS =		0x0100,
	MF_NARROW_AREA =			0x0200,
	MF_DIVE_SINK =				0x0400,
	MF_AVOIDSINK_USE_BODY =		0x0800,
	MF_CAMERA_NEAR =			0x1000,
} ;

/*---------------------------------------------------------------*/

/*---------------------------------------------------------------

  プロトタイプ宣言 

---------------------------------------------------------------*/

#include	"water.x"

/*---------------------------------------------------------------*/

static	PL_ACTION	DiveMode[] = {
	DiveStill, DiveSlowMove, DiveLittle,
	DiveMiddle, DiveMax
} ;

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
    DG_COPY_VEC( &WaterCamera->target, &CamTarget ) ;
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
#if 0
	if ( pad->status & PAD_L1 ) vf = CAM_INTERP_SPEED_H ;
	else							  vf = CAM_INTERP_SPEED ;
#else
	vf = CAM_INTERP_SPEED ;
#endif
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
								 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
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
	float		rail_height = RAIL_HEIGHT ;
	float		nheight, cheight ;

	if ( WaterFlag & WATER_FLAG_NO_ZONE_TRACE ) return -1 ;
	hzx_id = WaterZoneGroupID ;
	if ( hzx_id == 0 ) {
		hzx_id = WaterZoneGroupID = GM_PlayerWork->control.hzx_id ;
	}
	nzNo = HZX_GetLowerZone( hzx_id, npos ) ;
	if ( nzNo < 0 ) {
		FVECTOR		buf ;
		/* 無い場合は上を見てみる */
		DG_COPY_VEC( &buf, npos ) ;
		buf.vy += 10000.0F ;
		nzNo = HZX_GetLowerZone( hzx_id, &buf ) ;
	}

	if ( nzNo < 0 ) {
		printf( "[%d] next zone is ??? ", GV_Time ) ;
		DumpVec( npos ) ;
		return -1 ;
	}
	nz = HZX_GetZone( hzx_id, nzNo ) ;

	//HZX_ViewZone1( nz, 32, 232, 32 ) ;

	if ( nz->flag & 0x20 ) {
		//printf( "hikui zone\n" ) ;
		rail_height = 750.0F ;
	} else if ( WaterFlag & WATER_FLAG_NO_FLOOR ) {
		/* 追従ゾーン高さをプレイヤーと同じにする */
		/* w11ab しかないので、同じフラグにしてしまう。 */
		rail_height = GM_PlayerControl->mov.vy - nz->y ;
	}
	nheight = nz->y + rail_height ;
	GV_SetVec3( &nzc, nz->x, nheight, nz->z ) ;
check_again :
	if ( HZX_OnlineHazardCheck( hzx_id, cpos, &nzc, HZX_CHK_ALL, 
							    /*HZX_SEG_RECOIL_TYPE*/HZX_SEG_NO_HARITSUKI, 
							    /*HZX_FLOOR_RECOIL_TYPE*/ 0 ) ) {
		if ( again == 0 ) {
			czNo = HZX_GetLowerZone( hzx_id, cpos ) ;
			if ( czNo < 0 ) {
				FVECTOR		buf ;
				/* 無い場合は上を見てみる */
				DG_COPY_VEC( &buf, cpos ) ;
				buf.vy += 10000.0F ;
				czNo = HZX_GetLowerZone( hzx_id, &buf ) ;
			}
			if ( czNo < 0 ) {
				printf( "[%d] now zone is ??? ", GV_Time ) ;
				DumpVec( cpos ) ;
				return -1 ;
			}
			cz = HZX_GetZone( hzx_id, czNo ) ;

			//HZX_ViewZone1( nz, 32, 32, 232 ) ;

			if ( WaterFlag & WATER_FLAG_NO_FLOOR ) {
				rail_height = GM_PlayerControl->mov.vy - cz->z ;
			}
			cheight = cz->y + rail_height ;
			GV_SetVec3( &czc, cz->x, cheight, cz->z ) ;
		} else {
			printf( "[%d] ???\n", GV_Time ) ;
			//ViewFromTo( cpos, &nzc, 32, 232, 232 ) ;
			czNo = -1 ;
		}
		if ( again == 0 && 
			 HZX_OnlineHazardCheck( hzx_id, &czc, &nzc, HZX_CHK_ALL, 
								    /*HZX_SEG_RECOIL_TYPE*/HZX_SEG_NO_HARITSUKI, 
								    /*HZX_FLOOR_RECOIL_TYPE*/0 ) ) {
			/* 現在のゾーンと目標ゾーンの間に壁がある */
			czNo = HZX_AddressNo( GV_GetNo( hzx_id ), czNo, czNo ) ;
			nzNo = HZX_AddressNo( GV_GetNo( hzx_id ), nzNo, nzNo ) ;
			nzNo = HZX_Navigate( czNo, nzNo, cpos ) ;
			nz = HZX_GetZone( hzx_id, nzNo & 255 ) ;

			//HZX_ViewZone1( nz, 232, 32, 32 ) ;
			if ( nz->flag & 0x20 ) {
				nheight = nz->y + 750.0F ;
			} else {
				nheight = nz->y + 1000.0F ;
			}
			GV_SetVec3( &nzc, nz->x, nheight, nz->z ) ;
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
			//printf( "%f %f %f : ", cz->x, cz->y, cz->z ) ;
			//DumpVec( &czc ) ;
			//HZX_ViewZone( hzx_id, czNo ) ;
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
		//printf( "%f %f %f : ", nz->x, nz->y, nz->z ) ;
		//DumpVec( &nzc ) ;
	}
	return 0 ;
}

static	void		WaterCameraOutofRaiden( PlayerWork *work ) 
{
	GM_CameraSet	*cam ;
	FVECTOR			pos, res, from ;
	FVECTOR			shift = { 500.0F, 0.0F, 0.0F } ;
	FVECTOR			to1, to2, v, d1, d2 ;
	float			len1, len2, ip ;

	cam = PL_WaterCamera ;
	if ( cam == NULL || cam->on == 0 ) return ;

	DG_COPY_VEC( &pos, &cam->position ) ;
	if ( !CalcObjsCollision( &res, &pos, work->body.objs, 1.50F ) ) return ;

	GV_MatToVec( &work->body.objs->world, &from ) ;
	DG_SetPos( &work->body.objs->world ) ;

	len1 = len2 = 550.0F ;

	DG_PutVector( &shift, &to1, 1 ) ;
	shift.vx *= -1.0F ;
	DG_PutVector( &shift, &to2, 1 ) ;

	_sceVu0SubVector( &d1, &pos, ( FVECTOR * )work->body.objs->world.m[ 3 ] ) ;
	_sceVu0SubVector( &d2, &to1, ( FVECTOR * )work->body.objs->world.m[ 3 ] ) ;
	ip = _sceVu0InnerProduct( &d1, &d2 ) ;
	if ( ip < 0.0F ) {
		DG_COPY_VEC( &v, &to1 ) ;
		DG_COPY_VEC( &to1, &to2 ) ;
		DG_COPY_VEC( &to2, &v ) ;		
	}

	if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &from, &to1,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlinePoint( &to1 ) ;
		HZX_GetOnlineVector( &v ) ;
		len1 = GV_VecLen3F( &v ) ;
	} else {
		//DG_COPY_VEC( &cam->position, &to1 ) ;
		GV_NearExp4VF( &cam->position, &to1, 3 ) ;
		return ;
	}

	if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &from, &to2,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlinePoint( &to2 ) ;
		HZX_GetOnlineVector( &v ) ;
		len2 = GV_VecLen3F( &v ) ;
	} else {
		//DG_COPY_VEC( &cam->position, &to2 ) ;
		GV_NearExp4VF( &cam->position, &to2, 3 ) ;
		return ;
	}
	/* 両方壁のときは長いほう */
	if ( len1 > len2 ) {
		//DG_COPY_VEC( &cam->position, &to1 ) ;
		GV_NearExp2VF( &cam->position, &to1, 3 ) ;
	} else {
		//DG_COPY_VEC( &cam->position, &to2 ) ;
		GV_NearExp2VF( &cam->position, &to2, 3 ) ;
	}
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
	SVECTOR			nrot, aimrot ;
	FVECTOR			vec ;
	FVECTOR			bound1, bound2 ;
	HZX_GROUP_ID	hzx_id ;
	int				ntrack ;
	int				ry_range ;
	CONTROL			*ctrl ;

	cam = WaterCamera ;
	if ( cam->on == 0 ) {
		return ;
	}
	if ( !( ModeFlag & MF_STAND ) ) {
		//GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;
        //DG_COPY_VEC( &work->camera, &work->control.mov ) ;
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_KOSHI ].world, &work->camera ) ;
	}

	hzx_id = work->control.hzx_id ;

	ctrl = ( PL_WaterCamControl != NULL ) ? PL_WaterCamControl : &work->control ;
	if ( ctrl == PL_WaterCamControl ) DG_COPY_VEC( &work->camera, &PL_WaterCamControl->mov ) ;

	/* 現在のカメラデータ */
	DG_COPY_VEC( &cpos, &cam->position ) ;
	DG_COPY_VEC( &ctrg, &cam->target ) ;

	/* カメラＺ回転 */
	if ( !( ModeFlag & MF_NO_CAMERA_RZ ) && CamNoRzCount == 0 ) {
		int			d1, d2, diff ;

		d1 = GV_VecDir2FromTo( &cpos, &ctrg ) ;
		d2 = ctrl->rot.vy ;
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
		CamRot.vy = GV_NearRangeP( GV_NearPhase( CamRot.vy, ctrl->turn.vy ),
								  ctrl->turn.vy, ry_range ) ;
	}

	//if ( ctrl == PL_WaterCamControl ) CamRotAim.vx -= 512 ;
#if 1
	if ( !( ModeFlag & MF_NO_CAMERA_TURN ) ) {
		if ( cam->track < 1250 ) ModeFlag |= MF_CAMERA_NEAR ;
	} else {
		ModeFlag &= ~MF_CAMERA_NEAR ;
	}

	if ( ( ModeFlag & MF_CAMERA_NEAR ) && 
		 cam->track > 1850 ) {
		ModeFlag &= ~MF_CAMERA_NEAR ;
	}
	if ( ModeFlag & MF_CAMERA_NEAR ) {
		CamRotAim.vx += 320 ;
		if ( cam->track > 1250 ) {
			CamRotAim.vx -= 320 * ( int )( ( float )( cam->track - 1250 ) / 600.0F ) ;
		}
		//printf( "tikai!!%d : \n", CamRotAim.vx ) ;
		if ( PL_WaterCamControl == NULL && 
			 GM_GetNextCamera( 0 ) == WaterCamera ) {
			FVECTOR		shift = { 0.0F, 350.0F, 0.0F } ;
			FVECTOR		to ;
			float		len ;

			if ( cam->track > 1250 ) {
				shift.vy = 350.0F - ( ( float )( cam->track - 1250 ) / 600.0F ) / 350.0F ;
			}

			DG_SetPos( &work->body.objs->objs[ HUMAN21_KUBI ].world ) ;
			//DG_PutVector( &shift, &work->camera, 1 ) ;
			DG_PutVector( &shift, &to, 1 ) ;
			if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->camera, &to,
									    HZX_CHK_ALL, 0, 0 ) ) {
				HZX_GetOnlinePoint( &to ) ;
				len = GV_VecLen3F2( &work->camera, &to ) ;
				if ( len > 10.0F ) {
					HZX_GetOnlineVector( &to ) ;
					GV_LenVec3F( &to, &to, 0.0F, len - 10.0F ) ;
					_sceVu0AddVector( &to, &work->camera, &to ) ;
				}
			}
			DG_COPY_VEC( &work->camera, &to ) ;
			ModeFlag |= MF_CAMERA_NEAR ;
		}
	}
#endif

	aimrot.vx = ( ctrl->rot.vx + CamRotAim.vx ) & 4095 ;
	if ( aimrot.vx > 2048 ) aimrot.vx -= 4096 ;
	else if ( aimrot.vx < -2048 ) aimrot.vx += 4096 ;

	if ( aimrot.vx > 900 ) aimrot.vx = 900 ;
	if ( aimrot.vx < -900 ) aimrot.vx = -900 ;

	CamRot.vx = GV_NearExp8P( CamRot.vx, aimrot.vx ) ;

	CamRot.vx &= 4095 ;
	if ( CamRot.vx > 2048 ) CamRot.vx -= 4096 ;
	else if ( CamRot.vx < -2048 ) CamRot.vx += 4096 ;
	if ( CamRot.vx < -900 ) CamRot.vx = -900 ;
	else if ( CamRot.vx > 900 ) CamRot.vx = 900 ;

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
	//printf( "[%d] %f %f %f\n", GV_Time, npos.vx, npos.vy, npos.vz ) ;
	//printf( "\t %f %f %f\n", ntrg.vx, ntrg.vy, ntrg.vz ) ;
	//printf( "\t %d %d %d\n", nrot.vx, nrot.vy, nrot.vz ) ;

	if ( HZX_OnlineHazardCheck( hzx_id, &ntrg, &npos,
							    HZX_CHK_ALL,
							    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI, 
							    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {
		HZX_GetOnlineVector( &vec ) ;
		GV_LenVec3F( &vec, &vec, 0.0F, GV_VecLen3F( &vec ) - 10.0F ) ;
		_sceVu0AddVector( &npos, &ntrg, &vec ) ;
		//printf( "[%d/h] %f %f %f\n", GV_Time, npos.vx, npos.vy, npos.vz ) ;

		/* Ｙ方向バウンドチェック */
		GV_SetVec3( &vec, 0.0F, -BOUND_LEN, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vy = vec.vy + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, 0.0F, BOUND_LEN, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vy = vec.vy - BOUND_ADJ ;
		}
		/* Ｘ方向バウンドチェック */
		GV_SetVec3( &vec, -BOUND_LEN, 0.0F, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vx = vec.vx + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, BOUND_LEN, 0.0F, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vx = vec.vx - BOUND_ADJ ;
		}
		/* Ｚ方向バウンドチェック */
		GV_SetVec3( &vec, 0.0F, 0.0F, -BOUND_LEN ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vz = vec.vz + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, 0.0F, 0.0F, BOUND_LEN ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vz = vec.vz - BOUND_ADJ ;
		}
		/* Ｙ方向バウンドチェック */
		GV_SetVec3( &vec, 0.0F, -BOUND_LEN, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound1.vy = vec.vy + BOUND_ADJ ;
		}
		GV_SetVec3( &vec, 0.0F, BOUND_LEN, 0.0F ) ;
		_sceVu0AddVector( &vec, &npos, &vec ) ;
		if ( HZX_OnlineHazardCheck( hzx_id, &npos, &vec,
								    HZX_CHK_ALL,
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			HZX_GetOnlinePoint( &vec ) ;
			bound2.vy = vec.vy - BOUND_ADJ ;
		}
	}

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
								    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
								    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {		
			bound2.vy = HorizonLevel ;
			//printf( "[%d] bound set %f\n", GV_Time, HorizonLevel ) ;
		} 
	}

	GM_CameraBoundTrace( &npos, &bound1, &bound2 ) ;

	if ( CamClear == 1 ) DG_COPY_VEC( &cpos, &npos ) ;

	//printf( "[%d/2] %f %f %f\n", GV_Time, npos.vx, npos.vy, npos.vz ) ;

	if ( HZX_OnlineHazardCheck( hzx_id, &npos, &cpos, 
							    HZX_CHK_ALL,
							    /*HZX_SEG_RECOIL_TYPE |*/ HZX_SEG_NO_HARITSUKI,
							    /*HZX_FLOOR_RECOIL_TYPE |*/ 0 ) ) {
		/* 現在のカメラと次のカメラの間に壁がある。 */
		if ( NavigateCamera( cam, &cpos, &npos,
							 &ctrg, &ntrg ) < 0 ) {
			/* ナビゲートがうまくいかない */
			printf( "[%d]navigate failed ：%f %f %f\n", GV_Time,
				    npos.vx, npos.vy, npos.vz ) ;
			printf( "%f %f %f\n", ntrg.vx, ntrg.vy, ntrg.vz ) ;
			printf( "%f %f %f\n", work->camera.vx, work->camera.vy, work->camera.vz ) ;

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
        //printf( "[%d]normal\n", GV_Time ) ;
	}

	/* ライデン中でないかチェック */
	WaterCameraOutofRaiden( work ) ;

	if ( npos.vy <= GM_WaterLevel && 
		 cam->position.vy < GM_WaterLevel + 100.0F ) {
		cam->flag |= CAM_FLAG_BOUND | CAM_FLAG_BOUNDCHANGE_QUICK ;
		GV_SetVec3( &cam->bound1, -1000000.0F, -1000000.0F, -1000000.0F ) ;
		GV_SetVec3( &cam->bound2, 1000000.0F, GM_WaterLevel, 1000000.0F ) ;
		//printf( "[%d] cambound %f\n", GV_Time, GM_WaterLevel ) ;
	} else {
		cam->flag &= ~( CAM_FLAG_BOUND | CAM_FLAG_BOUNDCHANGE_QUICK ) ;
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
	//GM_RadarSetSight( &Rctrl, 0, 768, 1000.0F, RADAR_COLOR_RED ) ;
#endif
}

#if 0
#define	UpdateCameraAfter( _w )		UpdateCamera( _w )
#else
#define	UpdateCameraAfter( _w )		
#endif

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
        if ( work->action != TouchTurnR ) {
			SetMode( work, TouchTurnB ) ;
			SetEEMode( EE_MODE_TURN ) ;
		}
	} else if ( press & PAD_L ) {
//		work->control.turn.vy += 1024 ;
		ModeFlag |= MF_RTURN ;
		if ( work->action != TouchTurnL ) {
			SetMode( work, TouchTurnL ) ;
			SetEEMode( EE_MODE_TURN ) ;
		}
	} else if ( press & PAD_R ) {
//		work->control.turn.vy -= 1024 ;
		ModeFlag |= MF_RTURN ;
        if ( work->action != TouchTurnR ) {
			SetMode( work, TouchTurnR ) ;
			SetEEMode( EE_MODE_TURN ) ;
		}
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
								    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
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
									    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
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
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK  ) ) {
		//printf( "[%d] nogerahan\n", GV_Time ) ;
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
	if ( ModeFlag & MF_AVOIDSINK_USE_BODY ) {
		DG_SetPos( &work->body.objs->objs[ 0 ].world ) ;
	} else {
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	}
	ModeFlag &= ~MF_AVOIDSINK_USE_BODY ;

	DG_PutVector( &head, &head, 1 ) ;
	DG_PutVector( &foot, &foot, 1 ) ;

	res = 0 ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &head,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		HZX_GetOnlineHazard( &front_hzd, &front_hzd.attribute ) ;
		HZX_GetOnlinePoint( &front_hit ) ;
		_sceVu0SubVector( &front_adj, &front_hit, &head ) ;
		GV_LenVec3F( &front_adj, &front_adj, 0.0F, GV_VecLen3F( &front_adj ) + 10.0F ) ;
		res = 1 ; 
//		HZX_ViewHazard( &front_hzd ) ;
	} 
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &foot,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
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
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		HZX_GetOnlineVector( &vec ) ;
		len = GV_VecLen3F( &vec ) ;
		if ( len > 8.0F ) {
			GV_LenVec3F( &vec, &vec, 0.0F, len - 8.0F ) ;
			_sceVu0AddVector( &pos, &work->control.mov, &vec ) ;
		}
		printf( "koshi kubi hazard! %f\n", len ) ;
	}
	flag = HZX_LevelHazardCheck( work->control.hzx_id, &pos, 
								 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
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
			 //levels[ 0 ], levels[ 1 ], lheight, uheight ) ;
	if ( lheight < work->control.height ) lheight = work->control.height ;
		
	/* 天井回避優先 */
	if ( levels[ 1 ] - uheight < work->control.mov.vy ) {
		work->control.step.vy -= GV_NearExp2F( 0.0F, work->control.mov.vy 
											  - ( levels[ 1 ] - uheight ) ) ;
	} else if ( levels[ 0 ] + lheight > work->control.mov.vy ) {
		work->control.step.vy += GV_NearExp2F( 0.0F, levels[ 0 ] + lheight 
											  - work->control.mov.vy ) ;
	}
}

/* 息継ぎ吸い込み */
static	void	BreathPointAdjust( PlayerWork *work )
{
	FVECTOR		step ;

	PL_GuideToBreathPoint( &step, &work->control.mov, 
						   3000.0F, 24.0F, NULL ) ;
	work->control.step.vx += step.vx ;
	work->control.step.vy += step.vy ;
	work->control.step.vz += step.vz ;
}

/* ToDiveフェイズの連打チェック */
static	void	ToDiveQuickPadCheck( PlayerWork *work )
{
	if ( work->pad->press & PAD_SWIM ) {
		if ( GV_Time - work->counter < 32 ) work->sv.vx ++ ;
		work->counter = GV_Time ;
	}
	if ( work->sv.vx < DIVE_SPEED_ZERO ) work->sv.vx = DIVE_SPEED_ZERO ;
	else if ( work->sv.vx > DIVE_SPEED_MAX ) work->sv.vx = DIVE_SPEED_MAX ;
}

/*----------------------------------------------------------------*/

/* あわぶく */
static	void	Bubble( PlayerWork *work, int obj )
{
	extern void	*NewBubbleMany( FVECTOR *, int, int ) ;
	FVECTOR		*pos ;
	
	if ( Status( PLAYER_WATCH ) ) return ;

	if ( WaterCamera->on == 0 &&
		( obj == HUMAN21_KUBI || obj == HUMAN21_ATAMA ) ) {
		obj = HUMAN21_MIGI_ASHI1 + irnd() % 7 ;
	}

	if ( ( WaterFlag & WATER_FLAG_NO_FLOOR ) && obj == HUMAN21_ATAMA ) return ;

	if ( obj == HUMAN21_ATAMA ) {
		pos = &BubblePos ;
	} 
   else 
   {
      ASSERT( obj < work->body.objs->n_models );   //BP_MATH - check valid
		pos = ( FVECTOR * )work->body.objs->objs[ obj ].world.m[ 3 ] ;
	}
	NewBubbleMany( pos, 60, 0 ) ;
}

/* 水中ステートをセット */
static	inline	void	SetWaterStatus( void )
{
	{
		FVECTOR		shift = { 0.0F, 0.0F, 100.0F } ;

		DG_SetPos( &GM_PlayerWork->body.objs->objs[ HUMAN21_ATAMA ].world ) ;
		DG_PutVector( &shift, &BubblePos, 1 ) ;		
	}

	if ( ( GV_Time % DIRECT_TICK( 90 ) ) == 0 ) {
		Bubble( GM_PlayerWork, irnd() % 21 ) ;
	}
	if ( ( GV_Time % DIRECT_TICK( 110 ) ) == 0 ) {
		Bubble( GM_PlayerWork, irnd() % 21 ) ;
	}
	if ( ( GV_Time % DIRECT_TICK( 120 ) ) == 0 ) {
		Bubble( GM_PlayerWork, HUMAN21_ATAMA ) ;
	}

	SetStatus( PLAYER_IN_THE_WATER | PLAYER_WEAPON_DISABLE | PLAYER_WEAPON_INVISIBLE ) ;
	SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_PEEP | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | 
			 FLAG_NO_IK | FLAG_NO_GRAVITY ) ;
	GM_PlayerControl->turn.vz = 0 ;
	UpdateCamera( GM_PlayerWork ) ;
	if ( GM_PlayerControl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		PL_LevelCheck( GM_PlayerWork ) ;
	}

	if ( WaterCamera->on ) {
		GM_RadarSetFlag( &GM_PlayerWork->radar, RADAR_SIGHT ) ;
		SeekCameraTurn( GM_PlayerWork ) ;
	} else {
		if ( Status( PLAYER_WATCH ) ) {
			GM_RadarSetFlag( &GM_PlayerWork->radar, RADAR_SIGHT ) ;
		} else {
			GM_RadarResetFlag( &GM_PlayerWork->radar, RADAR_SIGHT ) ;
		}
	}

	{
		GM_CameraSet		*now ;

		now = GM_GetCurrentCamera( 0 ) ;
		if ( ( ModeFlag & MF_WATER_SURFACE ) || ( now->position.vy > HorizonLevel ) ) {
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
			PL_ObjPos( GM_PlayerWork, HUMAN21_KOSHI, &GM_PlayerFindPos ) ;
			_sceVu0AddVector( &TrapCheckPos, &GM_PlayerControl->mov, &GM_PlayerWork->root_diff ) ;
		}
	}

	ModeFlag &= ~MF_CLEAR ;
	GM_PlayerControl->skip_flag &= ~CTRL_HZX_SEG_HORIZON_CHECK ;

	/* Ｏ２管理 */
	if ( PL_ObjHeight( GM_PlayerWork, HUMAN21_ATAMA ) < HorizonLevel - 100.0F ) {
		SetStatus( PLAYER_NO_BREATH ) ;
	}
#if 0
	/* Ｏ２が０の時は、レーション自動使用不可 */
	if ( GM_O2 <= 0 ) SetFlag( FLAG_CANNOT_USE_RATION ) ;
#endif
	/* メニュー設定 */
	PL_SetNoUseItemType( IT_TYPE_CBBOX | IT_TYPE_SUBJECT ) ;
	PL_SetNoUseItem( IT_Tabacco ) ;

	/* 出血 */
#if 1
#if 0
	if ( Status( PLAYER_BLOOD_DROP ) ||
		 GM_Vitality < GM_VitalityMax ) {
#else
	if ( Status( PLAYER_BLOOD_DROP ) ) {
#endif
		int			unit, mode ;
		FVECTOR		pos, force ;
#if 0
		if ( Status( PLAYER_BLOOD_DROP ) ) mode = 4 ;
		else							   mode = 3 ;
#else
		mode = 4 ;
#endif	
		if ( GV_Time % 4 == 0 ) {
			unit = 13 + irnd() % 4 ;
			PL_ObjPos( GM_PlayerWork, unit, &pos ) ;
			force.vx = frnd() ;
			force.vy = frnd() ;
			force.vz = frnd() ;
			force.vw = 0.0F ;
			NewBloodWater( &pos, &force, 50.0F, mode ) ;
		}
	}
#endif 
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
	if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		v = ( float )( work->pad->left_dx - 128 ) ;
		if ( v < 0.0F ) v = -v ;
		v *= 2.0F ;
	} else {
		v = ( float )work->pad->pressure[ PAD_PRESS_L ] ;
		if ( v < ( float )work->pad->pressure[ PAD_PRESS_R ] ) {
			v = ( float )work->pad->pressure[ PAD_PRESS_R ] ;
		}
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
	speed = ( int )( 24.0F * v / 256.0F ) ;
	work->control.turn.vy = GV_NearSpeed( turn_to, padto, speed ) ;
}

/* Ｘ向き変更 */
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
		CamRotAim.vx = CAMERA_TURNX + 32 ;
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
		CamRotAim.vx = CAMERA_TURNX - 256 ;
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
	if ( v >= 200.0F ) base = 160.0F ;

	if ( GV_DiffDirAbs( from, 0 ) > 320 ) base *= 2.50F ;

	speed = ( int )( base * v / 256.0F ) ;
	//if ( GM_Configuration & GM_CONFIG_WATERUD_REVERSE ) turn_to = -turn_to ;
	if ( GM_Configuration & GM_CONFIG_SHUKAN_REVERSE ) turn_to = -turn_to ;
	work->control.turn.vx = GV_NearSpeed( from, turn_to, speed ) ;
}

/* 連打で2倍 */
static	void	MotionDouble( PlayerWork *work )
{
	/* いつも1.75倍 */
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.75F ) ;
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
		if ( work->pad->press & PAD_SWIM ) {
			SetMode( work, Tread2Dive ) ;
			SetEEMode( EE_MODE_S2L ) ;
		} else if ( work->pad->status & PAD_UDLR ) {
			SetMode( work, TreadMove ) ;
			SetEEMode( EE_MODE_STAND ) ;
		}
	}
    LargeMoveAdjust( work ) ;
	UpdateCameraAfter( GM_PlayerWork ) ;
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
		SetEEMode( EE_MODE_STAND ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else if ( work->pad->press & PAD_SWIM ) {
		SetMode( work, Tread2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	} else if ( !( work->pad->status & PAD_UDLR ) ) {
		SetMode( work, TreadStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
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
	UpdateCameraAfter( GM_PlayerWork ) ;
}

/* 立ち泳ぎからダイブ */
static	void	Tread2Dive( PlayerWork *work, int time ) 
{
	int			mtime ;

	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMtread2dive, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->sv.vx = 0 ;
		work->counter = 0 ;
		MotionDouble( work ) ;
	}

	if ( time == TIME_BASE * 16 ) NewPadVibration2( GV_StrCode( "rai_dive_01" ), 0 ) ;

	SetWaterStatus() ;

	ModeFlag |= MF_NO_CAMERA_TURN ;
	work->control.step.vy = 0.0F ;
	//SeekTurnX( work ) ;
	GM_SetCameraQuick( 0 ) ;

	SeekTurnL( work ) ;
	AvoidSinkforSwim( work ) ;

	ToDiveQuickPadCheck( work ) ;

	switch( work->data ) {
	case 0 :
		work->control.turn.vx = 900 ;
		if ( PL_CheckMotionTime( work, 42 ) ) {
			GM_SeSetMode( SD_P_INWTERM1, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
		if ( PL_MotionTime( work ) > 84 ) {	
			SetEEFlag( PL_EE_FLAG_STAND2LIE_NO_INTERP ) ;
		}
		if ( EndMotion( work ) ) {
			SetActionL( work, WMdive_armshift, -1 ) ;
			SetEEMode( EE_MODE_LIE ) ;
			work->fdata = GM_WORLD_LIMIT_BOTTOM ;
			if ( HZX_LevelHazardCheck( work->control.hzx_id, &work->control.mov,
									   HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) & 1 ) {
				float		levels[ 2 ] ;
				HZX_GetLevelHeight( levels ) ;
				work->fdata = levels[ 0 ] ;
			}
			work->data ++ ;
		}
		break ;
	case 1 :
		SetEEFlag( PL_EE_FLAG_STAND2LIE_NO_INTERP ) ;
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = work->body.m_ctrl->step.vy ;
		if ( work->control.mov.vy < work->fdata + 1200.0F ) {
			work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, 0, 48 ) ;
		} else {
			work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, 0, 12 ) ;
		}
		if ( EndMotion( work ) ) {
			SetActionL( work, WMdive_max, -1 ) ;
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 0.50F ) ;
			work->data ++ ;
		}
		break ;
	case 2 :
		mtime = PL_MotionTime( work ) ;
		if ( mtime < 24 ) {	
			SetEEFlag( PL_EE_FLAG_STAND2LIE_NO_INTERP ) ;
		}
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = work->body.m_ctrl->step.vy ;
		if ( work->control.mov.vy < work->fdata + 1200.0F ) {
			work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, 0, 48 ) ;
		} else {
			work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, 0, 12 ) ;
		}
		if ( EndMotion( work ) || work->control.mov.vy < work->fdata + 850.0F ) {
			if ( work->sv.vx >= DIVE_SPEED_MIDDLE ) work->sv.vx = DIVE_SPEED_MAX ;
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
			SetMode( work, DiveMode[ work->sv.vx ] ) ;
			DiveInit( work ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}
	}
#if 0
	if ( PL_CheckMotionTime( work, 42 ) ) {
		GM_SeSetMode( SD_P_INWTERM1, &work->control.mov, GM_SEMODE_BOMB ) ;
	}
	if ( EndMotion( work ) ) {
        if ( work->sv.vx >= DIVE_SPEED_MAX ) work->sv.vx = DIVE_SPEED_MIDDLE ;
		SetMode( work, DiveMode[ work->sv.vx ] ) ;
		DiveInit( work ) ;
		SetEEMode( EE_MODE_LIE ) ;
	}
#endif
    LargeMoveAdjust( work ) ;
	UpdateCameraAfter( GM_PlayerWork ) ;
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
		PL_IntoSubject( work ) ;
		//work->camdir.vx = work->control.rot.vx ;
		work->camdir.vx = -800 ;
		SetEEMode( EE_MODE_STAND ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.75F ) ;
	}
	SetWaterStatus() ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	ModeFlag |= MF_NO_CAMERA_TURN ;

	KeepHorizonLevel( work ) ;
	GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;

    LargeMoveAdjust( work ) ;
	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( work->data == 0 ) {
		if ( PL_CheckMotionRate( work ) > 0.90F ) {
			work->data = 1 ;
			PL_LeaveSubject( work ) ;
			/* 瞬時カメラにする */
			PL_SetInvincible( work, 0 ) ;
			GM_ChangeCurrentCameraInterpFunc2( 0, GM_CameraInterpQuick, 0 ) ;
			GM_SetCameraAngle( work->subject_camera, 2.00F ) ;
		} else {
			if ( PL_CheckMotionRate( work ) > 0.60F ) {
				//work->camdir.vx += 24 ;
			}	
			if ( work->camdir.vx > 0 ) work->camdir.vx = 0 ;

			{ 
				FVECTOR		shift = { 0.0F, -128.0F, -200.0F } ;
				SVECTOR		rot ;

				rot.vx = rot.vz = 0 ; rot.vy = work->control.rot.vy ;
				DG_SetPos2( &DG_ZeroVector, &rot ) ;
				shift.vy += work->ftime ;
				DG_RotVector( &shift, &shift, 1 ) ;
				_sceVu0AddVector( &work->camera, &work->camera, &shift ) ;
			}
			GM_SetCameraAngle( work->subject_camera, 1.50F ) ;
		}
	} else {
		ModeFlag |= MF_WATER_SURFACE ;
	}
	/* カメラが水中から出たらＳＥ */
	if ( work->data2 == 0 && 
		 GM_GetCurrentCamera( 0 )->position.vy > GM_WaterLevel ) {
		work->data2 = 1 ;
		//GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_P_OUTWTRS1 ) ;
		/* 呼吸 */
		/* 潜水ゴーグル時はなし */
		if ( !GM_CheckPlayerStatusEX( I64(0), PLAYER2_DIVE_GOGGLES ) ) {
			if ( GM_O2 < GM_O2Max / 3 ) {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_V_RAIBRE01 ) ;
			} else {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_V_RAIBRE02 ) ;
			}
		} else {
			/* 日本版から、潜水ゴーグル時はマスク吸いを呼ぶ */
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_MASK02 ) ;
		}	

		{
			extern void *NewVibrateCamera( int, int, int, int ) ;
			NewVibrateCamera( DIRECT_TICK( 24 ), 16, 16, 1 ) ;
		}
		/* 水滴 */
		{
			extern void 	*NewScrDrop( int life ) ;
			
			//if ( ScrDropAct != NULL ) GV_DestroyOtherActor( ScrDropAct ) ;
			ScrDropAct = NewScrDrop( 0 ) ;
		}
		NewPadVibration2( GV_StrCode( "rai_surfacing_01" ), 0 ) ;
	}
#if 0
	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	}
#endif
#if 0
	if ( EndMotion( work ) || 
		( PL_MotionTime( work ) > 60 && ( work->pad->status & ( PAD_UDLR | PAD_SWIM ) ) ) ) {
#else
	if ( EndMotion( work ) ) {
#endif
		PL_SetInvincible( work, 4 ) ;
		PL_LeaveSubject( work ) ;
		SetMode( work, TreadStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
	}
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
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}

	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	SetWaterStatus() ;
	UnsetFlag( FLAG_NO_IK ) ;
	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	ModeFlag |= MF_STAND | MF_NO_CAMERA_RZ ;
	DG_COPY_VEC( &work->camera, &work->fv ) ;

	work->control.step.vy = -FALL_SPEED_BOTTOM ;

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) || ( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
		SetMode( work, Seabed2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		if ( work->control.mov.vy > HL() ) {
			SetMode( work, TreadStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else if ( work->pad->press & PAD_SWIM ) {
			SetMode( work, Seabed2Dive ) ;
			SetEEMode( EE_MODE_S2L ) ;
		} else if ( ( work->control.grounded & 1 ) && 
				    ( work->pad->status & PAD_U ) ) {
			SetMode( work, SeabedWalk ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else if ( work->pad->status & ( PAD_L | PAD_R ) ) {
			SeekTurnL( work ) ;
		}
	}
    LargeMoveAdjust( work ) ;
	UpdateCameraAfter( GM_PlayerWork ) ;
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

	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	SetWaterStatus() ;

	SetStatus( PLAYER_WALK ) ;
	UnsetFlag( FLAG_NO_IK ) ;
	UnsetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_WAIST_INTERP ) ;
    ModeFlag |= MF_NO_CAMERA_RZ ;

    LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) || ( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
		SetMode( work, Seabed2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
		return ;
	}

	/* 2倍 */
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.0F ) ;
	work->control.step.vy = -FALL_SPEED_BOTTOM ;

	if ( time != 0 && !( work->control.grounded & 1 ) ) {
		work->data2 ++ ;
	} else {
		if ( work->data2 > 0 ) -- work->data2 ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, SeabedStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
	} else {
		if ( work->control.mov.vy > HL() ) {
			SetMode( work, TreadStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else if ( work->pad->press & PAD_SWIM ) {
			SetMode( work, Seabed2Dive ) ;	
			SetEEMode( EE_MODE_S2L ) ;
		} else if ( !( work->pad->status & PAD_U ) ) {
			SetMode( work, SeabedStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else if ( work->data2 > 8 ) {
			SetMode( work, SeabedStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else {
            SeekTurnL( work ) ;
		}
	}
#if 0
	/* めりこみ防止処理（ちょっと重い） */
	{
		float	front, back ;	
		FVECTOR	shift = { 0.0F, 0.0F, 0.0F } ;
		FVECTOR	mov ;

		PL_GetModelLength2( work, &front, &back, 0x1 << HUMAN21_ATAMA ) ; /* 頭だけ */
		front += 250.0F ;
		DG_COPY_VEC( &mov, &work->control.mov ) ;
		mov.vy = PL_ObjHeight( work, HUMAN21_ATAMA ) ;
		if ( PL_CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector,
									   &work->control.rot, front, 
									   HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, 0 ) ) {
			shift.vz = GV_VecLen3F2( &mov, &ResultPoint ) - front ;
			DG_SetPos2( &work->control.mov, &work->control.rot ) ;
			DG_RotVector( &shift, &shift, 1 ) ;
			PL_AdjustXZ( work, &shift ) ;
		}
	}
#endif
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
		work->sv.vx = 0 ;
		work->counter = 0 ;
	}
	SetWaterStatus() ;
	ModeFlag |= MF_NO_CAMERA_TURN ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

	SeekTurnX( work ) ;
	SeekTurnL( work ) ;
	MotionDouble( work ) ;
	AvoidSinkforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

	ToDiveQuickPadCheck( work ) ;

	if ( EndMotion( work ) ) {
		if ( work->sv.vx >= DIVE_SPEED_MAX ) work->sv.vx = DIVE_SPEED_MIDDLE ;
		SetMode( work, DiveMode[ work->sv.vx ] ) ;
		SetEEMode( EE_MODE_LIE ) ;
		DiveInit( work ) ;
	}
}

/*----------------------------------------------------------------

  泳ぎ姿勢

----------------------------------------------------------------*/

/* ダイブ共通処理 */
static	int		DiveCommon( PlayerWork *work )
{
	int			sink, ctf ;

	ViewHorizon() ;
	SetWaterStatus() ;
	if ( !Flag( FLAG_NO_MOTION_STEP_Y ) ) {
		work->control.step.vy = work->body.m_ctrl->step.vy ;
	} 
	ModeFlag &= ~( MF_NARROW_AREA | MF_DIVE_SINK ) ;
	ctf = CeilToFloor( work ) ;
	if ( ctf ) ModeFlag |= MF_NARROW_AREA ;

	/* エマ死に時は、はりつきなし属性壁も見る */
	if ( !ctf && work->life.value > 0 && GM_IsGameOver() ) {
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
	}

	/* 高さ処理 */
	DiveHeightAdjust( work ) ;
	SetFlag( FLAG_NO_MOTION_STEP_Y ) ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {
		SeekTurnL( work ) ;
		SeekTurnX( work ) ;
	}

	if ( work->control.turn.vx < 0 && 
		 work->control.mov.vy > HL() &&
		 work->control.levels[ 1 ] > HorizonLevel ) {
		if ( !ctf ) {
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
			SetFlag( FLAG_NO_STEP ) ;
			SetMode( work, Dive2Tread ) ;
			SetEEMode( EE_MODE_L2S ) ;
			return 1 ;
		}
	}	

	sink = AvoidSinkforSwim( work ) ;
	if ( sink != 0 ) ModeFlag |= MF_DIVE_SINK ;

	/* 体Ｚ回転 */
	if ( DiveSpeed >= DIVE_SPEED_MIDDLE && work->ftime > DIRECT_TICK( 48 ) ) {
		work->control.turn.vz = WaterCamera->rotate.vz * 9 ;
		if ( work->control.turn.vz > 768 ) work->control.turn.vz = 768 ;
		else if ( work->control.turn.vz < -768 ) work->control.turn.vz = -768 ;
	} else {
		work->control.turn.vz = 0 ;
	}

	if ( work->pad->status & PAD_HEAD_UP ) BreathPointAdjust( work ) ;

    LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

    if ( ( sink & 0x0001 ) && ( sink & 0x1000 ) && !( sink & 0x0100 ) &&
		!( work->pad->status & PAD_UDLR ) ) {
		work->idata += 3 ;
	} else {
		if ( work->idata > 0 ) -- work->idata ;
	}
	//printf( "%d %x\n", work->idata, sink ) ;
#if 1	/* 壁手つき */
	if ( work->idata > 64 && !ctf ) {
		if ( DiveSpeed != DIVE_SPEED_MAX ||
			 ( PL_MotionTime( work ) >= 22 && 
			   PL_MotionTime( work ) < 34 ) ) {
			SetFlag( FLAG_NO_STEP ) ;
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
			SetMode( work, Dive2WallTouch ) ;
			SetEEMode( EE_MODE_L2S ) ;
			return 1 ;
		}
	}
#endif
	/* 床頭 */
	if ( ( sink & 1 ) && ( sink & 0x00010000 ) ) {
		if ( ( work->pad->status & PAD_HEAD_DOWN ) ) {
			work->idata2 = AvoidDirX ;
			ModeFlag |= MF_SEEKDOWN_NEW_PRESS ;
		}
	}
	if ( ModeFlag & MF_SEEKDOWN_NEW_PRESS ) {
		work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, work->idata2, 16 ) ;
		if ( PL_ObjHeight( work, HUMAN21_KUBI ) - work->control.levels[ 0 ] > 1000.0F ) {
			ModeFlag &= ~MF_SEEKDOWN_NEW_PRESS ;
		}
	}
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
	return 0 ;
}

/* ダイブ開始 */
static	void	DiveInit( PlayerWork *work )
{
	work->sv.vx = 0 ;		/* 速度モード */
	work->sv.vy = 0 ;
	work->sv.vz = 0 ;
	work->idata = 0 ;		/* 壁頭 */
	work->idata2 = 0 ;		/* 床頭 */
	work->counter = 0 ;		/* 速度連打カウンター */
}

/* ダイブ静止 */
static	void	DiveStill( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMbataashi_idle, 24 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		DiveSpeed = DIVE_SPEED_ZERO ;
	}

	if ( work->data == 0 && EndMotion( work ) ) {
		work->data = 1 ;
		if ( !CeilToFloor( work ) ) {
			SetActionL( work, WMbataashi_idle_bend, 24 ) ;
		}
	}

	SetFlag( FLAG_NO_MOTION_STEP ) ;
	DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;

	if ( DiveCommon( work ) ) return ;

#if 1
	/* 急停止 */
	if ( work->pad->press & PAD_B ) {
		if ( !( ModeFlag & MF_NARROW_AREA ) && 
			 !Status( PLAYER_WATCH ) &&
			 work->control.mov.vy - work->control.levels[ 0 ] < 1000.0F &&
			!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
			SetMode( work, DiveBreakStand ) ;
			SetEEMode( EE_MODE_L2S ) ;
			SetFlag( FLAG_NO_STEP ) ;
			return ;
		} 
	}
#else

#endif

	if ( !Status( PLAYER_WATCH ) && ( work->pad->status & PAD_SWIM ) ) {
		SetMode( work, DiveLittle ) ;
		SetEEMode( EE_MODE_LIE ) ;
	}
}

/* ダイブ進みつづけ */
static	void	DiveSlowMove( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMbataashi_min, 24 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		DiveSpeed = DIVE_SPEED_SLOW ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}

	if ( DiveCommon( work ) ) return ;

	if ( work->pad->pressure[ PAD_PRESS_A ] > 96 ) {
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.50F ) ;
	} else {
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.00F ) ;
	}
#if 1
	/* 急停止 */
	if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_B ) ) {
		if ( !( ModeFlag & MF_NARROW_AREA ) && 
			 !Status( PLAYER_WATCH ) &&
			 work->control.mov.vy - work->control.levels[ 0 ] < 1000.0F &&
			!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
			SetMode( work, DiveBreakStand ) ;
			SetEEMode( EE_MODE_L2S ) ;
			SetFlag( FLAG_NO_STEP ) ;
		} else {
			SetMode( work, DiveStill ) ;		
			SetEEMode( EE_MODE_LIE ) ;
		}
		return ;
	}
#else
	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, DiveStill ) ;		
		SetEEMode( EE_MODE_LIE ) ;
		return ;
	}
#endif
	if ( !( work->pad->status & PAD_SWIM ) ) {
		SetMode( work, DiveLittle ) ;
		SetEEMode( EE_MODE_LIE ) ;
	}
}

/* ダイブちょっと進み */
static	void	DiveLittle( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMbataashi_short, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->counter = GV_Time ;
		Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		DiveSpeed = DIVE_SPEED_LITTLE ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}

	if ( DiveCommon( work ) ) return ;

#if 1
	/* 急停止 */
	if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_B ) ) {
		if ( !( ModeFlag & MF_NARROW_AREA ) && 
			 !Status( PLAYER_WATCH ) &&
			 work->control.mov.vy - work->control.levels[ 0 ] < 1000.0F &&
			!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
			SetMode( work, DiveBreakStand ) ;
			SetEEMode( EE_MODE_L2S ) ;
			SetFlag( FLAG_NO_STEP ) ;
		} else {
			SetMode( work, DiveStill ) ;		
			SetEEMode( EE_MODE_LIE ) ;
		}
		return ;
	}
#else
	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, DiveStill ) ;		
		SetEEMode( EE_MODE_LIE ) ;
		return ;
	}
#endif
	if ( work->pad->press & PAD_SWIM ) {
		if ( GV_Time - work->counter < 32 ) work->sv.vx ++ ;
		else								work->sv.vx = 0 ;
		work->counter = GV_Time ;
//printf( "little : rendaaa %d\n", work->sv.vx ) ;
		if ( work->sv.vx >= 1 ) {
			work->sv.vx = 0 ;
			work->counter = 0 ;
			SetMode( work, DiveMiddle ) ;
			SetEEMode( EE_MODE_LIE ) ;
			return ;
		}
		//SetMode( work, DiveLittle ) ;
		//return ;
	}
	if ( EndMotion( work ) ) {
		if ( work->pad->status & PAD_A ) {
			SetMode( work, DiveSlowMove ) ;
			SetEEMode( EE_MODE_LIE ) ;
		} else {
			SetMode( work, DiveStill ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}
	}
}

/* ダイブ中速（クロール） */
static	void	DiveMiddle( PlayerWork *work, int time )
{
	int			mtime ;

	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMdive_arm_r, 24 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->counter = GV_Time ;
		Bubble( work, HUMAN21_MIGI_TE ) ;
		Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		DiveSpeed = DIVE_SPEED_MIDDLE ;

		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	}

	if ( DiveCommon( work ) ) return ;

	mtime = PL_MotionTime( work ) ;
	if ( mtime > 30 && ( work->pad->press & PAD_SWIM ) ) {
		work->data2 = 1 ;
	}

	switch( work->data ) {
	case 0 :	/* 右 */
		if ( mtime >= 60 && work->data2 == 1 ) {
			SetActionL( work, WMdive_arm_l, 24 ) ;
			work->data = 1 ;
			work->data2 = 0 ;
			Bubble( work, HUMAN21_HIDARI_TE ) ;
			Bubble( work, HUMAN21_HIDARI_KAKATO ) ;
		}
		break ;
	case 1 :	/* 左 */
		if ( mtime >= 60 && work->data2 == 1 ) {
			SetActionL( work, WMdive_arm_r, 24 ) ;
			work->data = 0 ;
			work->data2 = 0 ;
			Bubble( work, HUMAN21_MIGI_TE ) ;
			Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		}	   
	}

	/* 急停止 */
	if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_B ) ) {
		if ( !( ModeFlag & MF_NARROW_AREA ) && 
			 work->control.mov.vy - work->control.levels[ 0 ] < 1000.0F &&
			!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
			SetMode( work, DiveBreakStand ) ;
			SetEEMode( EE_MODE_L2S ) ;
			SetFlag( FLAG_NO_STEP ) ;
		} else {
			SetMode( work, DiveBreak ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}
		return ;
	}

	if ( work->pad->press & PAD_SWIM ) {
		if ( GV_Time - work->counter < 32 ) work->sv.vx ++ ;
		else								work->sv.vx = 0 ;
		work->counter = GV_Time ;
		if ( work->sv.vx >= 2 && 
			 ( PL_MotionTime( work ) < 10 || PL_MotionTime( work ) > 54 ) ) {
			work->sv.vx = 0 ;
			work->counter = 0 ;
			SetMode( work, DiveMax ) ;
			SetEEMode( EE_MODE_LIE ) ;
			work->data = 1 ;
			return ;
		}
	}

	if ( EndMotion( work ) ) {
		work->sv.vx = 0 ;
		work->counter = 0 ;
		if ( work->pad->status & PAD_A ) {
			SetMode( work, DiveSlowMove ) ;
			SetEEMode( EE_MODE_LIE ) ;
		} else {
			SetMode( work, DiveLittle ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}		
	}
}

/* ダイブ高速（ドルフィン） */
static	void	DiveMax( PlayerWork *work, int time )
{
	int			mtime ;

	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		if ( work->data == 0 ) {
			SetActionL( work, WMdive_max, -1 ) ;
		} else {
			SetActionL( work, WMdive_armshift, -1 ) ;
		}
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		work->sv.vx = 24 ;
		work->counter = GV_Time ;
		Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		DiveSpeed = DIVE_SPEED_MAX ;
	}

	if ( work->data != 0 && EndMotion( work ) ) {
		SetActionL( work, WMdive_max, -1 ) ;		
		work->data = 0 ;
	}

	if ( DiveCommon( work ) ) return ;

	/* 急停止 */
	if ( Status( PLAYER_WATCH ) || ( work->pad->press & PAD_B ) ) {
		if ( !( ModeFlag & MF_NARROW_AREA ) && 
			 work->control.mov.vy - work->control.levels[ 0 ] < 1000.0F &&
			!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
			SetMode( work, DiveBreakStand ) ;
			SetEEMode( EE_MODE_L2S ) ;
			SetFlag( FLAG_NO_STEP ) ;
		} else {
			SetMode( work, DiveBreak ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}
		return ;
	}

	if ( EndMotion( work ) ) {
		Bubble( work, HUMAN21_MIGI_KAKATO ) ;
	}

	if ( work->pad->press & PAD_SWIM ) {
		if ( GV_Time - work->counter < 32 ) work->sv.vx = 48 ;
		else								work->sv.vx -- ;
		work->counter = GV_Time ;
	} else {
		work->sv.vx -- ;
	}

	mtime = PL_MotionTime( work ) ;

	if ( work->data == 0 && 
		 mtime >= 22 && mtime < 34 && 
		 work->sv.vx <= 0 ) {
		work->sv.vx = 0 ;
		work->counter = 0 ;
		if ( work->pad->status & PAD_A ) {
			SetMode( work, DiveSlowMove ) ;
			SetEEMode( EE_MODE_LIE ) ;
		} else {
			SetMode( work, DiveMiddle ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}		
	}
}

/* ダイブ急停止立ちへ */
static	void	DiveBreakStand( PlayerWork *work, int time )
{
	ViewHorizon() ;
	if ( time == 0 ) {
		WaterStance = WS_STAND ;
		SetActionL( work, WMstop2seabed, -1 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
	}
	SetWaterStatus() ;
	ModeFlag |= MF_NO_CAMERA_TURN ;
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

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( PL_CheckMotionRate( work ) > CHANGE_MRATE ) {
		if ( !Status( PLAYER_WATCH ) && 
			( work->pad->press & ( PAD_MODE | PAD_SWIM ) ) ) {
			SetMode( work, Seabed2Dive ) ;
			SetEEMode( EE_MODE_S2L ) ;
			return ;
		}
	}
	if ( EndMotion( work ) ) {
		SetMode( work, SeabedStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
	}
}

/* ダイブ急停止 */
static	void	DiveBreak( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		WaterStance = WS_SWIM ;
		SetActionL( work, WMstop2bataashi, 24 ) ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		WaterCameraOn() ;
		Bubble( work, HUMAN21_MIGI_KAKATO ) ;
		DiveSpeed = DIVE_SPEED_ZERO ;
	}

	if ( DiveCommon( work ) ) return ;

	if ( EndMotion( work ) ) {
		SetMode( work, DiveStill ) ;
		SetEEMode( EE_MODE_LIE ) ;
	}
}

/*----------------------------------------------------------------*/

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
	ModeFlag |= MF_NO_CAMERA_TURN ;
	KeepHorizonLevel( work ) ;
	work->control.turn.vx = GV_NearSpeed( work->control.turn.vx, 0, 16 ) ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		PL_SubjectTurn( work ) ;
	}

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Seabed2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
		return ;
	}

	if ( ( work->control.grounded & 1 ) &&
		!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
		SetMode( work, SeabedStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
#if 1
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( work->pad->status & ( PAD_R | PAD_L | PAD_D ) ) ) {
		if ( work->pad->status & PAD_R ) {
			SetMode( work, TouchTurnR ) ;
			SetEEMode( EE_MODE_TURN ) ;
		} else if ( work->pad->status & PAD_L ) {
			SetMode( work, TouchTurnL ) ;
			SetEEMode( EE_MODE_TURN ) ;
		} else if ( work->pad->status & PAD_D ) {
			SetMode( work, TouchTurnB ) ;
			SetEEMode( EE_MODE_TURN ) ;
		}
#endif
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( PL_CheckMotionRate( work ) > CHANGE_MRATE ) &&
			    ( work->pad->press & PAD_SWIM ) ) {
		SetMode( work, Seabed2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
	}
	if ( EndMotion( work ) ) {
		SetMode( work, WallTouch ) ;
		SetEEMode( EE_MODE_STAND ) ;
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

	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

	/* 天井低いゾーンに入っていたら
	   ダイブにする */
	if ( CeilToFloor( work ) ) {
		SetMode( work, Seabed2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		PL_SubjectTurn( work ) ;
	} 

	if ( ( work->control.grounded & 1 ) &&
		!( WaterFlag & WATER_FLAG_NO_FLOOR ) ) {
		SetMode( work, SeabedStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
	} else if ( !Status( PLAYER_WATCH ) &&
			    ( work->pad->status & ( PAD_R | PAD_L | PAD_D ) ) ) {
		if ( work->pad->status & PAD_R ) {
			SetMode( work, TouchTurnR ) ;
			SetEEMode( EE_MODE_TURN ) ;
		} else if ( work->pad->status & PAD_L ) {
			SetMode( work, TouchTurnL ) ;
			SetEEMode( EE_MODE_TURN ) ;
		} else if ( work->pad->status & PAD_D ) {
			SetMode( work, TouchTurnB ) ;
			SetEEMode( EE_MODE_TURN ) ;
		}
	} else if ( !Status( PLAYER_WATCH ) && 
			    ( work->pad->press & PAD_SWIM ) ) {
		SetMode( work, Seabed2Dive ) ;
		SetEEMode( EE_MODE_S2L ) ;
	}
}

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
	if ( time > 0 ) PL_EE_TurnEnd = 1 ;

	CamTrackAim = CAMERA_TRACK_FAR ;
	SetWaterStatus() ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	work->control.step.vy = 0.0F ;

	LevelAdjustforSwim( work ) ;

//	SeekTurnX( work ) ;

//	SeekTurnL( work ) ;
//	MotionDouble( work ) ;

	ModeFlag |= MF_AVOIDSINK_USE_BODY ;
	AvoidSinkforSwim( work ) ;

    LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( EndMotion( work ) ||
		( PL_CheckMotionRate( work ) > rate && ( work->pad->status & PAD_SWIM ) ) ) {
		if ( DiveSpeed >= DIVE_SPEED_MAX ) DiveSpeed = DIVE_SPEED_MIDDLE ;
		SetMode( work, DiveMode[ DiveSpeed ] ) ;
		SetEEMode( EE_MODE_LIE ) ;
		PL_EE_TurnEnd = 1 ;
		DiveInit( work ) ;
		if ( action == WMwall_turn_b ) {
			/* 180度のときは、カメラＺ回転なし */
			CamNoRzCount = 32 ;
		}
	}
}

static	void	TouchTurnR( PlayerWork *work, int time )
{
#if 1
	if ( WaterStance == WS_SWIM && 
		 DiveSpeed >= DIVE_SPEED_MIDDLE ) {
		TouchTurn( work, time, WMwall_turn_r_l, 0.95F ) ;
	} else {
		if ( DiveSpeed >= DIVE_SPEED_MIDDLE ) DiveSpeed = DIVE_SPEED_SLOW ;
		TouchTurn( work, time, WMwall_turn_r, 0.70F ) ;
	}
#else
	TouchTurn( work, time, WMwall_turn_r, 0.70F ) ;
#endif
	SetNCamRot( work ) ;
}

static	void	TouchTurnL( PlayerWork *work, int time )
{
#if 1
	if ( WaterStance == WS_SWIM && 
		 DiveSpeed >= DIVE_SPEED_MIDDLE ) {
		TouchTurn( work, time, WMwall_turn_l_l, 0.95F ) ;
	} else {
		if ( DiveSpeed >= DIVE_SPEED_MIDDLE ) DiveSpeed = DIVE_SPEED_SLOW ;
		TouchTurn( work, time, WMwall_turn_l, 0.70F ) ;
	}
#else
	TouchTurn( work, time, WMwall_turn_l, 0.70F ) ;
#endif
	SetNCamRot( work ) ;
}

static	void	TouchTurnB( PlayerWork *work, int time )
{
	ModeFlag |= MF_NO_CAMERA_RZ ;
	if ( WaterStance == WS_SWIM && 
		 DiveSpeed >= DIVE_SPEED_MIDDLE ) {
		TouchTurn( work, time, WMwall_turn_b_l, 0.95F ) ;
	} else {
		if ( DiveSpeed >= DIVE_SPEED_MIDDLE ) DiveSpeed = DIVE_SPEED_SLOW ;
		TouchTurn( work, time, WMwall_turn_b, 0.95F ) ;
	}
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

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_DAMAGED ) ;
		if ( Status( PLAYER_DEAD ) ) {
			SetMode( work, WaterDeadStand ) ;
			SetEEMode( EE_MODE_RAI_DEAD_STAND ) ;
			return ;
		}
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		if ( CeilToFloor( work ) ) {
			SetMode( work, DiveStill ) ;
			SetEEMode( EE_MODE_LIE ) ;
			DiveInit( work ) ;
		} else if ( WaterStance == WS_SURFACE ) {
			SetMode( work, TreadStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else {
			SetMode( work, SeabedStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
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

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, WaterDozaemon ) ;
		SetEEMode( EE_MODE_RAI_DEAD_IDLE ) ;
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
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
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

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_DAMAGED ) ;
		if ( Status( PLAYER_DEAD ) ) {
			SetMode( work, WaterDeadLie ) ;
			SetEEMode( EE_MODE_RAI_DEAD_LIE ) ;
			return ;
		}
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;		
		SetMode( work, DiveStill ) ;
		SetEEMode( EE_MODE_LIE ) ;
		DiveInit( work ) ;
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
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
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

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, WaterDozaemon ) ;
		SetEEMode( EE_MODE_RAI_DEAD_IDLE ) ;
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

	UpdateCameraAfter( GM_PlayerWork ) ;

	if ( work->data2 == 0 && EndMotion( work ) ) {
		PL_MotionSleep( work, 0 ) ;
		GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
		GM_GameOverProcEnd( &work->actor ) ;
		work->data = 2 ;
	}
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
	SetEEMode( EE_MODE_STAND ) ;
	work->control.step.vy = 0.0F ;
	/* 戻りモードの設定し直し */
	PL_ReturnMode = ReturnSeabedStill ;
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
		SetEEMode( EE_MODE_STAND ) ;
		ModeFlag = 0 ;
		DiveSpeed = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_SetMenuStatus( MENU_WEAPON_OFF ) ;
		GM_ResetMenuStatus( MENU_ITEM_OFF ) ;
		PL_DamageFunc = WaterCheckDamage ;
		work->dmg_callback = WaterDamageCallback ;

		PL_AddDeadAction( &DeadModeSet, WaterDeadModeNoO2, WaterDeadModeConditionFunc ) ;

		/* 戻りモード設定 */
		SetFlag( FLAG_RETURNMODE_SET ) ;
		PL_ReturnMode = ReturnSeabedStill ;
		PL_ReturnModeMotionArc = Motion ;
		if ( Procs[ 1 ] > 0 ) GM_ExecProc( Procs[ 1 ], NULL ) ;
#ifdef C_DEBUG_MODE
		GM_InitRadarControl( &Rctrl, &WaterCamera->position,
							 RADAR_VISIBLE, -1 ) ;
#endif
	}
	work->control.step.vy = 0.0F ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
}

/* 階段入る */
static	void	IntoWaterStair( PlayerWork *work, int time )
{
	FVECTOR		mov ;

	if ( time == 0 ) {
		if ( Procs[ 0 ] > 0 ) GM_ExecProc( Procs[ 0 ], NULL ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
		work->control.skip_flag |= CTRL_RESET_HZX_BASE ;
		DG_COPY_VEC( &work->fv, &work->control.mov ) ;
		work->fv.vy = 0.0F ;
	}
	SetStatus( PLAYER_FORCE | PLAYER_WEAPON_INVISIBLE | PLAYER_MENU_DISABLE ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	
	DG_COPY_VEC( &mov, &work->control.mov ) ;
	mov.vy = 0.0F ;
	if ( GV_VecLen3F2( &mov, &work->fv ) > ( float )work->data ) {
		UnsetStatus( PLAYER_FORCE ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		SetStatus( PLAYER_IN_THE_WATER | PLAYER_WATER_SURFACE ) ;
		UpdateCamera( work ) ;
		/* トラップチェック位置初期化 */
		_sceVu0AddVector( &TrapCheckPos, &work->root_diff, &work->control.mov ) ;
		SetMode( work, TreadStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
		ModeFlag = 0 ;
		DiveSpeed = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_SetMenuStatus( MENU_WEAPON_OFF ) ;
		GM_ResetMenuStatus( MENU_ITEM_OFF ) ;
		PL_DamageFunc = WaterCheckDamage ;
		work->dmg_callback = WaterDamageCallback ;
		PL_AddDeadAction( &DeadModeSet, WaterDeadModeNoO2, WaterDeadModeConditionFunc ) ;
		/* 戻りモード設定 */
		SetFlag( FLAG_RETURNMODE_SET ) ;
		PL_ReturnMode = ReturnSeabedStill ;
		PL_ReturnModeMotionArc = Motion ;
		if ( Procs[ 1 ] > 0 ) GM_ExecProc( Procs[ 1 ], NULL ) ;
#ifdef C_DEBUG_MODE
		GM_InitRadarControl( &Rctrl, &WaterCamera->position,
							 RADAR_VISIBLE, -1 ) ;
#endif		
	}
}

/* 階段出る */
static	void	LeaveWaterStair( PlayerWork *work, int time )
{
	FVECTOR		mov ;

	if ( time == 0 ) {
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		SetEEMode( EE_MODE_STAND ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		work->control.turn.vx = 0 ;
		if ( Procs[ 2 ] > 0 ) GM_ExecProc( Procs[ 2 ], NULL ) ;

		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
		work->control.skip_flag |= CTRL_RESET_HZX_BASE ;
		DG_COPY_VEC( &work->fv, &work->control.mov ) ;
		work->fv.vy = 0.0F ;
		UnsetStatus( PLAYER_IN_THE_WATER ) ;
	}

	SetStatus( PLAYER_FORCE | PLAYER_WEAPON_INVISIBLE | PLAYER_MENU_DISABLE ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	DG_COPY_VEC( &mov, &work->control.mov ) ;
	mov.vy = 0.0F ;
	if ( GV_VecLen3F2( &mov, &work->fv ) > ( float )work->data ) {
		PL_ReturnMotionArc( work ) ;
		PL_Force->flag |= FA_USE_DEFAULT ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		/* 戻りモード解除 */
		/* SetModeより前に */
		if ( PL_ReturnMode == ReturnSeabedStill ) PL_ReturnMode = NULL ;
		if ( PL_ReturnModeMotionArc == Motion ) PL_ReturnModeMotionArc = -1 ;
		UnsetFlag( FLAG_RETURNMODE_SET ) ;
		SetMode( work, PL_StillMode[ 0 ] ) ;
		ModeFlag = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		GM_ControlResetHeightMotion( &work->control ) ;
		if ( PL_DamageFunc == WaterCheckDamage ) PL_DamageFunc = NULL ;
		if ( work->dmg_callback == WaterDamageCallback ) work->dmg_callback = NULL ;
		PL_RemoveDeadAction( &DeadModeSet ) ;
		if ( Procs[ 3 ] > 0 ) GM_ExecProc( Procs[ 3 ], NULL ) ;
#ifdef C_DEBUG_MODE
		GM_FreeRadarControl( &Rctrl ) ;
#endif
	}
}

/* 水中モード出る */
static	void	LeaveWater( PlayerWork *work, int time )
{
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH |
			 FLAG_NO_IK | FLAG_NO_GRAVITY | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	UnsetStatus( PLAYER_IN_THE_WATER ) ;
	if ( time == 0 ) {
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}
	if ( EndMotion( work ) ) {
		PL_ReturnMotionArc( work ) ;
		//SetFlag( FLAG_FORCE_END ) ;
		//UnsetStatus( PLAYER_FORCE ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		/* 戻りモード解除 */
		/* SetModeより前に */
		if ( PL_ReturnMode == ReturnSeabedStill ) PL_ReturnMode = NULL ;
		if ( PL_ReturnModeMotionArc == Motion ) PL_ReturnModeMotionArc = -1 ;
		UnsetFlag( FLAG_RETURNMODE_SET ) ;
		SetMode( work, PL_StillMode[ 0 ] ) ;
		ModeFlag = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
		GM_ControlResetHeightMotion( &work->control ) ;
		if ( PL_DamageFunc == WaterCheckDamage ) PL_DamageFunc = NULL ;
		if ( work->dmg_callback == WaterDamageCallback ) work->dmg_callback = NULL ;
		PL_RemoveDeadAction( &DeadModeSet ) ;
		if ( Procs[ 3 ] > 0 ) GM_ExecProc( Procs[ 3 ], NULL ) ;
#ifdef C_DEBUG_MODE
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
		UnsetStatus( PLAYER_FORCE ) ;
		UnsetFlag( FLAG_FORCE | FLAG_FORCE_END ) ;
	}

	PL_WaterCamControl = NULL ;

	if ( which == GM_STRCODE_ENTER ) {
		/* 入水 */
		/* 以下のときは無効 */
#if 0
		if ( Status( PLAYER_DAMAGED | PLAYER_DOWNED | PLAYER_INTRUDE |
					 PLAYER_FORCE | PLAYER_CB_BOX | PLAYER_LOCKER |
					 PLAYER_IN_THE_WATER | PLAYER_DEAD ) ||
			 ( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
			 ( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return -1 ;
#endif
		if ( Status( PLAYER_IN_THE_WATER | PLAYER_DEAD | PLAYER_FORCE |
					 PLAYER_INTRUDE ) ) return -1 ;
		/* エルードからの場合特殊 */
		if ( Status( PLAYER_BEYOND ) ) {
			UnsetStatus( PLAYER_BEYOND ) ;			
			UnsetFlag( FLAG_FORCE | FLAG_MSG_SHADOW_OFF ) ;
		}
		if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return -1 ;
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveCaution( work ) ;
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		PL_UnequipSpecials() ;
		work->trigger = TRIG_FALL ;	/* グレネード落とし */

		GM_ClearTargetDamage( &( work->def ) ) ;
		work->touch.damaged = 0 ;
		UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;		
		UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;

		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
		SetStatus( PLAYER_FORCE | PLAYER_WEAPON_INVISIBLE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		SetMode2( work, NULL ) ;
		if ( msg != NULL ) {
			PL_ChangeMotionArc( work, PL_Force->marfile ) ;
			SetAction( work, motion, 6 ) ;
			SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_WAIST_INTERP ) ;
			SetMode( work, IntoWater ) ;
		} else {
			IntoWater( work, 0 ) ;
		}
		if ( Procs[ 0 ] > 0 ) GM_ExecProc( Procs[ 0 ], NULL ) ;
	} else if ( which == STRCODE_STAIR_ENTER ) {
		/* 階段入る */
#if 0
		if ( Status( PLAYER_DAMAGED | PLAYER_DOWNED | PLAYER_INTRUDE |
					 PLAYER_FORCE | PLAYER_CB_BOX | PLAYER_LOCKER |
					 PLAYER_IN_THE_WATER | PLAYER_DEAD ) ||
			 ( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
			 ( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return -1 ;
#endif
		if ( Status( PLAYER_IN_THE_WATER | PLAYER_DEAD | PLAYER_FORCE |
					 PLAYER_INTRUDE ) ) return -1 ;
		if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return -1 ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveCaution( work ) ;
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		PL_UnequipSpecials() ;
		work->trigger = TRIG_FALL ;	/* グレネード落とし */

		GM_ClearTargetDamage( &( work->def ) ) ;
		work->touch.damaged = 0 ;
		UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;		
		UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;

		SetMode2( work, NULL ) ;

		SetAction( work, none_stair_run, 0 ) ;
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		SetMode( work, IntoWaterStair ) ;
		work->data = motion ;

		SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
		SetStatus( PLAYER_FORCE | PLAYER_MENU_DISABLE ) ;
	} else if ( which == STRCODE_SEABED || 	
			    which == STRCODE_DIVE ) {
		/* 水底から ＯＲ 水中から */
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveCaution( work ) ;
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;

		UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;

		SetMode2( work, NULL ) ;

		PL_ChangeMotionArc( work, Motion ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		SetStatus( PLAYER_IN_THE_WATER ) ;
		DG_COPY_VEC( &WaterCamera->position, &work->control.mov ) ;
		DG_COPY_VEC( &WaterCamera->target, &work->control.mov ) ;

		/* トラップチェック位置初期化 */
		_sceVu0AddVector( &TrapCheckPos, &work->root_diff, &work->control.mov ) ;
		if ( which == STRCODE_SEABED ) {
			SetMode( work, SeabedStill ) ;
			SetEEMode( EE_MODE_STAND ) ;
		} else {
			SetMode( work, DiveStill ) ;
			SetEEMode( EE_MODE_LIE ) ;
		}
		ModeFlag = 0 ;
		DiveSpeed = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_SetMenuStatus( MENU_WEAPON_OFF ) ;
		GM_ResetMenuStatus( MENU_ITEM_OFF ) ;
		PL_DamageFunc = WaterCheckDamage ;
		work->dmg_callback = WaterDamageCallback ;

		PL_AddDeadAction( &DeadModeSet, WaterDeadModeNoO2, WaterDeadModeConditionFunc ) ;

		/* 戻りモード設定 */
		SetFlag( FLAG_RETURNMODE_SET ) ;
		PL_ReturnMode = ReturnSeabedStill ;
		PL_ReturnModeMotionArc = Motion ;
#ifdef C_DEBUG_MODE
		GM_InitRadarControl( &Rctrl, &WaterCamera->position,
							 RADAR_VISIBLE, -1 ) ;
#endif
	} else if ( which == STRCODE_TREAD ) {
		/* 水面から */
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveCaution( work ) ;
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		SetMode2( work, NULL ) ;

		UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;

		PL_ChangeMotionArc( work, Motion ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_TRAP ) ;
		SetStatus( PLAYER_IN_THE_WATER | PLAYER_WATER_SURFACE ) ;
		DG_COPY_VEC( &WaterCamera->position, &work->control.mov ) ;
		DG_COPY_VEC( &WaterCamera->target, &work->control.mov ) ;

		/* トラップチェック位置初期化 */
		_sceVu0AddVector( &TrapCheckPos, &work->root_diff, &work->control.mov ) ;
		SetMode( work, TreadStill ) ;
		SetEEMode( EE_MODE_STAND ) ;
		ModeFlag = 0 ;
		DiveSpeed = 0 ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		GM_SetMenuStatus( MENU_WEAPON_OFF ) ;
		GM_ResetMenuStatus( MENU_ITEM_OFF ) ;
		PL_DamageFunc = WaterCheckDamage ;
		work->dmg_callback = WaterDamageCallback ;

		PL_AddDeadAction( &DeadModeSet, WaterDeadModeNoO2, WaterDeadModeConditionFunc ) ;

		/* 戻りモード設定 */
		SetFlag( FLAG_RETURNMODE_SET ) ;
		PL_ReturnMode = ReturnSeabedStill ;
		PL_ReturnModeMotionArc = Motion ;
#ifdef C_DEBUG_MODE
		GM_InitRadarControl( &Rctrl, &WaterCamera->position,
							 RADAR_VISIBLE, -1 ) ;
#endif
	} else if ( which == STRCODE_STAIR_LEAVE ) {
		/* 階段出る */
		work->control.turn.vx = work->control.rot.vx = 0 ;
		if ( Status( PLAYER_FORCE ) || Flag( FLAG_FORCE ) ) return -1 ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveSubject( work ) ;
		SetMode2( work, NULL ) ;

		SetStatus( PLAYER_FORCE | PLAYER_WEAPON_INVISIBLE | PLAYER_MENU_DISABLE ) ;
		SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_FORCE ) ;
		WaterCameraOff() ;

		PL_ChangeMotionArc( work, work->org_motion ) ;
		SetAction( work, none_stair_run, 0 ) ;
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		SetMode( work, LeaveWaterStair ) ;
		work->data = motion ;

	} else {
		/* 出水 */
		work->control.turn.vx = work->control.rot.vx = 0 ;
		if ( motion < 0 ) {
			work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
										  CTRL_SKIP_TRAP ) ;
			work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
			UnsetStatus( PLAYER_IN_THE_WATER ) ;
			UnsetFlag( FLAG_RETURNMODE_SET ) ;
			SetMode2( work, NULL ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
			ModeFlag = 0 ;
			PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
			GM_ResetMenuStatus( MENU_MENU_OFF ) ;
			SetHzxHeight( work, NEAR_HEIGHT_STAND ) ;
			GM_ControlResetHeightMotion( &work->control ) ;
			if ( PL_DamageFunc == WaterCheckDamage ) PL_DamageFunc = NULL ;
			if ( work->dmg_callback == WaterDamageCallback ) work->dmg_callback = NULL ;
			PL_RemoveDeadAction( &DeadModeSet ) ;
			if ( PL_ReturnModeMotionArc == Motion ) PL_ReturnModeMotionArc = -1 ;
			WaterCameraOff() ;
#ifdef C_DEBUG_MODE
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
		SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_WAIST_INTERP ) ;
		if ( dir >= 0 ) work->control.turn.vy = work->control.rot.vy = dir ;
		work->control.turn.vx = 0 ;
		if ( Procs[ 2 ] > 0 ) GM_ExecProc( Procs[ 2 ], NULL ) ;
		SetMode2( work, NULL ) ;
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
#ifdef C_DEBUG_MODE
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
	FVECTOR		pos, force ;

	UnsetFlag( FLAG_BLOOD_SPLASH ) ;
	PL_ObjPos( work, HUMAN21_KUBI, &pos ) ;
	_sceVu0SubVector( &force, &pos, &off->center ) ;
	/* 水中血 */
	NewBloodWater( &pos, &force, 16.0F, 0 ) ;

	/* Ｏ２減らし */
#if 0		/* Ｏ２は減らさない */
	if ( Status( PLAYER_NO_BREATH ) ) {
		int		o2 ;

		o2 = GM_O2 ;
		o2 -= GM_O2Max / 8 ;	
		if ( o2 < 0 ) o2 = 0 ;
		GM_O2 = o2 ;
	}
#endif
	/* エマダメージ */
	if ( PL_EE_Exist ) PL_EE_Damage += off->power->damage ;
	//PL_EE_O2Damage = off->power->damage ;	/* Ｏ２は減らさない */
}

static	void	WaterCheckDamage( PlayerWork *work, long64 weapon_type, int dead )
{
	int			seNo, nextEE ;
	PL_ACTION	next ;

	weapon_type &= ~( WP_THROWG | WP_BOXKICK ) ;
	if ( weapon_type == I64(0) ) {
		GM_ClearTargetDamage( &( work->def ) ) ;
		return ;
	}
	if ( dead ) {
		NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;		
		seNo = SD_V_RAIOUW01 ;
		//if ( WaterStance == WS_SWIM ) next = WaterDeadLie ;
		//else						  next = WaterDeadStand ;
		if ( WaterStance == WS_SWIM ) {
			next = WaterDamageLie ;
			nextEE = EE_MODE_RAI_DAM_LIE ;
		} else {
			next = WaterDamageStand ;
			nextEE = EE_MODE_RAI_DAM_STAND ;
		}
	} else if ( weapon_type & ( WP_BLOW | WP_SOFTBLOW ) ) {
		/* 吹っ飛び */
		NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;		
		seNo = SD_V_RAIDMW01 ;
		if ( WaterStance == WS_SWIM ) {
			next = WaterDamageLie ;
			nextEE = EE_MODE_RAI_DAM_LIE ;
		} else {
			next = WaterDamageStand ;
			nextEE = EE_MODE_RAI_DAM_STAND ;
		}
	} else {
		/* のけぞり */
		NewPadVibration( PL_DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib1L, 2 | VAR_FLAG_FORCE ) ;
		seNo = SD_V_RAIDMW01 ;
		if ( WaterStance == WS_SWIM ) {
			next = WaterDamageLie ;
			nextEE = EE_MODE_RAI_DAM_LIE ;			
		} else {
			next = WaterDamageStand ;
			nextEE = EE_MODE_RAI_DAM_STAND ;			
		}
	}
	PL_ChangeMotionArc( work, Motion ) ;
	work->dmg_callback = WaterDamageCallback ;	/* 設定しなおし */
	PL_SetMode( work, next ) ;
	SetEEMode( nextEE ) ;
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
	extern	void   PL_WtDoorHoldEnd( int to, int name ) ;

	ViewHorizon() ;
	if ( time == 0 ) {
		PL_WtDoorHoldEnd( work->send_to, work->control.name ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		if ( WaterStance == WS_STAND ) {
			SetEEMode( EE_MODE_RAI_NOAIR_STAND ) ;
			SetActionL( work, WMnoair_stand, -1 ) ;
		} else {
			SetEEMode( EE_MODE_RAI_NOAIR_LIE ) ;
			SetActionL( work, WMnoair_lie, -1 ) ;
		}
		WaterStance = WS_STAND ;
		SetHzxHeight( work, HZX_HEIGHT_NO_USE ) ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		if ( WaterStance == WS_STAND ) {
			work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
			work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		} else {
			work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
			work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		}

		work->control.turn.vx = 0 ;

		PL_LeaveSubject( work ) ;
		PL_SetInvincible( work, 0 ) ;

		GM_GameOverProcStart( &work->actor ) ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
		SetStatus( PLAYER_DEAD ) ;
		PL_SetDeadFlag( work ) ;
		work->power.vital = 0 ;

		GM_SeSetMode( SD_V_RAIOUW01, &work->control.mov, GM_SEMODE_BOMB ) ;

		{
			FVECTOR		force = { 0.0F, 0.0F, 15.0F } ;

			DG_SetPos( &work->body.objs->objs[ HUMAN21_ATAMA ].world ) ;
			DG_RotVector( &force, &force, 1 ) ;
			NewDrowingBubble( GV_StrCode("溺れた時の泡"), &BubblePos, &force, 
							  10.f, 50.f, 120, 160, 0x50505530 );
		}
	}

	SetWaterStatus() ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	work->control.step.vy = 0.0F ;

	if ( work->control.mov.vy > HorizonLevel2 ) {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		work->control.step.vy = HorizonLevel2 - work->control.mov.vy ;
	}

	if ( WaterStance == WS_STAND ) {
		GM_ConfigControlNearCheck2( &work->control, FLOAT_NEAR_CHK_HEIGHT ) ;
	}

	if ( WaterStance != WS_STAND ) {
		DiveHeightAdjust( work ) ;
		AvoidSinkforSwim( work ) ;
	}

	LargeMoveAdjust( work ) ;

	UpdateCameraAfter( GM_PlayerWork ) ;

	/* 念のため */
	SetStatus( PLAYER_DEAD ) ;
	PL_SetDeadFlag( work ) ;

	if ( EndMotion( work ) ) {
		SetEEMode( EE_MODE_RAI_DEAD_IDLE ) ;
		SetMode( work, WaterDozaemon ) ;
	}
#if 0
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
#endif
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
					NewPadVibration2( GV_StrCode( "rai_dive_03" ), 0 ) ;
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
				//UnsetStatus( PLAYER_BEYOND ) ;
				PL_UnsetInvincible( work ) ;
				work->control.skip_flag &= ~( CTRL_SKIP_HZX | CTRL_SKIP_TRAP ) ;
				NewPadVibration2( GV_StrCode( "rai_dive_03" ), 0 ) ;
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
	BPMngWork = NULL ;

	Procs[ 0 ] = 0 ;
	Procs[ 1 ] = 0 ;
	Procs[ 2 ] = 0 ;
	Procs[ 3 ] = 0 ;

	PL_EE_Exist = 0 ;

	ScrDropAct = NULL ;

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

/* プロック設定 */
int		PL_COM_SetProcWaterInOut( void )
{
	if ( GCL_GetOption( 'i' ) != NULL ) {
		Procs[ 0 ] = GCL_GetNextInt() ;
		Procs[ 1 ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'o' ) != NULL ) {
		Procs[ 2 ] = GCL_GetNextInt() ;
		Procs[ 3 ] = GCL_GetNextInt() ;
	}
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

	ASSERT( BPMngWork == NULL ) ;
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
	GV_ZeroMemory( bp, sizeof( BreathPoint ) ) ;
	PL_GetOptionFV( 'p', &bp->pos ) ;

	bp->next = BPMngWork->list.next ;
	BPMngWork->list.next = bp ;

	return 1 ;
}

/* 息継ぎポイントへの吸い込み処理 */
int  				PL_GuideToBreathPoint( FVECTOR *step, FVECTOR *mov, float guide_len, 
									    float step_len, FVECTOR *bppos )
{
	BreathPoint 	*bp, *this ;
	FVECTOR			diff ;
	float			len ;

	DG_COPY_VEC( step, &DG_ZeroVector ) ;
	if ( BPMngWork == NULL ) return 0 ;
	bp = BPMngWork->list.next ;
	while( bp != NULL ) {
		this = bp ;
		bp = bp->next ;
		_sceVu0SubVector( &diff, &this->pos, mov ) ;
		len = GV_VecLen3F( &diff ) ;
		if ( len > guide_len ) continue ;
		if ( this->pos.vy - mov->vy > 1000.0F ) {
			diff.vy = 0.0F ;
		} 
		GV_LenVec3F( &diff, step, 0.0F, step_len ) ;
		//DumpVec( step ) ;
		if ( bppos != NULL ) DG_COPY_VEC( bppos, &this->pos ) ;
		return 1 ;
	}
	return 0 ;
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

