//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ee_swim.c
   エマ泳ぎ

   2001/02/23 M.Sonoyama
   $Id: ee_swim.c,v 1.4 2002/11/23 12:24:50 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
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
#include	"libutl.h"
#include	"ee_swim.h"

/*----------------------------------------------------------------*/

extern	void	Leg_IKcalc( CONTROL *, OBJECT * ) ;
extern 	void	*NewBubbleMany( FVECTOR *, int, int ) ;

/*----------------------------------------------------------------*/

#ifdef DEBUG_MODE
//#define O2_NODAMAGE 
#endif

#define		NTSC_TIME_BASE	(5)

#define		OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE|DG_FLAG_IRREACTION)

#define		S_SPHERE			(500)
#define		R_SPHERE			(474)
#define		NEAR_HEIGHT_STAND	(748.0F)
#define		NEAR_HEIGHT_SQUAT	(248.0F)
#define		NEAR_HEIGHT_GROUND	(248.0F)

#define		EndMotion( _w )		MT_CHECK_LAST1( (_w)->body.m_ctrl, 0 )

/* 髪の毛 */
#define		HAIR_NORMAL			GV_StrCode( "エマ髪の毛" )
#define		HAIR_WET			GV_StrCode( "エマぬれ髪の毛" )

enum {
	HAIR_MODE_NORMAL = 0,
	HAIR_MODE_WET,
	HAIR_MODE_WATER,
} ;

typedef	struct {
	int			motion ;
	int			end_mode ;
	int			start_proc ;
	int			end_proc ;
	int			flag ;
	int			padding[ 3 ] ;
} FORCE_ACT_CONTROL ;

typedef	struct	{
	FMATRIX		prev[ 21 ] ;
	SVECTOR		buffer[ 32 ] ;
	SVECTOR		adjust[ 6 ] ;
	SVECTOR		head_adjust ;
	int			head_ik_flag ;
	int			head_ik_mode ;
} IK_CONTROL ;

typedef	struct _Work {
	GV_ACT_EX			actor ;
	CONTROL				control ;
	OBJECT				body ;
	RADAR_CTRL			radar ;
	IK_CONTROL			ik_ctrl ;
	GM_GageSet			life ;
	GM_GageSet			o2 ;
	FMATRIX				lights[ 2 ] ;

	FVECTOR				shift ;

	FVECTOR				rai_shift ;

	SVECTOR				rot_adjusts[ 24 ] ;
	SVECTOR				turn_adjusts[ 24 ] ;
	SVECTOR				adjusts_min[ 24 ] ;
	SVECTOR				adjusts_max[ 24 ] ;

	FORCE_ACT_CONTROL	fa_ctrl ;

	int					shiftNo ;
	int					flag ;
	int					mode ;
	int					motion ;

	int					time ;
	int					ftime ;
	int					ftime_count ;
	void				( *action )( struct _Work *, int ) ;

	int					data ;
	int					data2 ;
	float				fdata ;
	float				fdata2 ;

	SVECTOR				interp_adj ;
	SVECTOR				interp_adj_to ;
	int					interp ;
	int					o2_gage_invisible_count ;

	int					adjust_interp_n[ 21 ] ;
	int					adjust_interp_n_r[ 21 ] ;
	int					prev_mode ;

	int					current_mar ;
	int					original_mar ;
	int					forceact_mar ;
	float				o2value ;

	int					procs[ 4 ] ;
	int					hair_mode ;
	int					heart_beat_decay ;
	int					last_heart_beat_time ;
	int					heart_beat_decay_changed ;
} Work ;

static	Work			*PL_EE_Work = NULL ;
static	int				PL_EE_LifeAdjust ;

static	int				LegIKAdj = 0 ;
static	int				LegIKAdjAim = 0 ;

static	int				PeterTrap = 3851832 ;	/* pt001 */

enum {
	EE_PROC_DEAD 		= 0,

	EE_PROC_MAX			= 4,
} ;

typedef	void	( *ACTION )( Work *, int ) ;

enum { 
	EE_FLAG_NOTHING 			=	0x0000,
	EE_FLAG_NO_ACTCONTROL		=	0x0001,
	EE_FLAG_NO_ACTOBJECT 		=	0x0002,
	EE_FLAG_NO_ACTMOTION		=	0x0004,
	EE_FLAG_NO_WAIST_INTERP 	=	0x0008,
	EE_FLAG_NO_MODE_CHANGE 		=	0x0010,
	EE_FLAG_STAND 				=	0x0020,
	EE_FLAG_NO_MOTION_STEP_Y	=	0x0040,
	EE_FLAG_NO_MOVEIK			=	0x0080,
	EE_FLAG_NO_HEADIK			=	0x0100,
	EE_FLAG_RETURN_MOTION_ARC	=	0x0200,
	EE_FLAG_RAI_SHIFT_STEP		=	0x0400,

	EE_FLAG_RESET_ALL			=	0x0fff,

	EE_FLAG_WTDOOR				= 	0x1000,
	EE_FLAG_CAMERA_IN_WATER		=	0x2000,
} ;

enum {
	EE_MESG_ON					= 0,
	EE_MESG_OFF,
	EE_MESG_POSITION,
	EE_MESG_MOTION,
	EE_MESG_WTDOOR,
	EE_MESG_WATERINOUT_STAIR,
	EE_MAX_MESGS
} ;

/*----------------------------------------------------------------*/

extern void* NewDrowingBubble( int name, FVECTOR* pvecPos,
							   FVECTOR*	pvecInitDir, float fSizeMin,
							   float fSizeMax, int nLifeMin,
							   int nLifeMax, int nRGBA ) ;

static	FVECTOR			Shifts[] = {
	{ 17.802F, 128.536F, -183.119 },	/* 立ち */
	{ 10.668F, 219.982F, -95.884 },		/* 泳ぎ */
	{ -70.826F,-216.954F,-172.628},		/* どざえもん */
	{ 10.668F, 183.0F, 128.0F },		/* 泳ぎ入り時調整 */
} ;

/* プロトタイプ */
static	void	StandStill( Work *work, int time ) ;
static	void	Stand2Lie( Work *work, int time ) ;
static	void	LieStill( Work *work, int time ) ;
static	void	Lie2Stand( Work *work, int time ) ;
static	void	Turn( Work *work, int time ) ;
static	void	DamStand( Work *work, int time ) ;
static	void	DamLie( Work *work, int time ) ;
static	void	DeadStand( Work *work, int time ) ;
static	void	DeadLie( Work *work, int time ) ;
static	void	DeadIdle( Work *work, int time ) ;
static	void	NoAirStand( Work *work, int time ) ;
static	void	NoAirLie( Work *work, int time ) ;

static	void	EEDeadStand( Work *work, int time ) ;
static	void	EEDeadLie( Work *work, int time ) ;
static	void	EEDeadIdle( Work *work, int time ) ;

static	void	ForceAct( Work *work, int time ) ;

static	ACTION	ModeAction[] = {
	NULL,
	NULL,
	StandStill,
	LieStill,
	Stand2Lie,
	Lie2Stand,
	Turn,
	DamStand,
	DamLie,
	DeadStand,
	DeadLie,
	DeadIdle,
	NoAirStand,
	NoAirLie,
	NULL
} ;

/*
   メモ

   首位置差	-13.0F, 95.0F, -200.0F
*/

/*
   ライデン首から見たエマ首の座標位置は

   ema_rai_non_swim_bataasi_idle.mtn
   (X:1.0668 Y:21.9982 Z:-9.5884) 

   ema_rai_non_swim_seabed_idle.mtn 
   (X:1.7802 Y:12.8536 Z:-18.3119)

   ema_rai_non_swim_down_idle.mtn 
   (X:-7.0826 Y:-21.6954 Z:-17.2628)   
   です。

   ema_rai_non_swim_noair_down_lie.mtn 〈横ダウン〉
   （Ｘ：0.1993  Ｙ：22.7035  Ｚ：1.2876）
   ema_rai_non_swim_noair_down_stand.mtn 〈立ちダウン〉
   （Ｘ：4.6259  Ｙ：-0.6539  Ｚ：-24.1405）
*/

/*----------------------------------------------------------------*/

/* アジャスト関連 */

/* ＳＶＥＣＴＯＲが０ベクトルかどうかチェック */
static	inline	int		CheckZeroSVector( v )
SVECTOR			*v ;
{
#if 1//BP_MATH - replaced with version that works for both little and big endian
   return( (v->vx == 0) && (v->vy == 0) && (v->vz == 0) );
#else
	return ( ( ( *( long64 * )v & I64(0xffffffffffff) ) ) ? 0 : 1 ) ;
#endif
}

static	void	InitAdjustMinMax( Work *work ) 
{
	int			i ;

	for ( i = 0; i < 21; i ++ ) {
		work->adjusts_min[ i ].vx = -2048 ;
		work->adjusts_min[ i ].vy = -2048 ;
		work->adjusts_min[ i ].vz = -2048 ;
		work->adjusts_max[ i ].vx = 2048 ;
		work->adjusts_max[ i ].vy = 2048 ;
		work->adjusts_max[ i ].vz = 2048 ;
	}
}

static	void	SetAdjustMinMax( Work *work, int unit, 	
								 int minx, int miny, int minz,
								 int maxx, int maxy, int maxz ) 
{
	GV_SetVec3( &work->adjusts_min[ unit ], minx, miny, minz ) ;
	GV_SetVec3( &work->adjusts_max[ unit ], maxx, maxy, maxz ) ;
}

static	void	CheckAdjustMinMax( Work *work, int i, SVECTOR *rot ) 
{
	rot->vx &= 4095 ; if ( rot->vx > 2048 ) rot->vx -= 4096 ;
	rot->vy &= 4095 ; if ( rot->vy > 2048 ) rot->vy -= 4096 ;
	rot->vz &= 4095 ; if ( rot->vz > 2048 ) rot->vz -= 4096 ;
	if ( rot->vx < work->adjusts_min[ i ].vx ) rot->vx = work->adjusts_min[ i ].vx ;
	if ( rot->vy < work->adjusts_min[ i ].vy ) rot->vy = work->adjusts_min[ i ].vy ;
	if ( rot->vz < work->adjusts_min[ i ].vz ) rot->vz = work->adjusts_min[ i ].vz ;
	if ( rot->vx > work->adjusts_max[ i ].vx ) rot->vx = work->adjusts_max[ i ].vx ;
	if ( rot->vy > work->adjusts_max[ i ].vy ) rot->vy = work->adjusts_max[ i ].vy ;
	if ( rot->vz > work->adjusts_max[ i ].vz ) rot->vz = work->adjusts_max[ i ].vz ;
}

/* アジャスト値のリセット */
static	void	ResetRotAdjust( work )
Work			*work ;
{
	MOTION_CONTROL	*m_ctrl ;
	SVECTOR			*adj, *tadj ;
	int				i, flag, flag2 ;

	adj = work->rot_adjusts ;
	tadj = work->turn_adjusts ;
	/* 角度 */
	m_ctrl = work->body.m_ctrl ;
	flag = m_ctrl->adjust_flag ;
	flag2 = 0 ;
	GV_ZeroMemory( tadj, sizeof( SVECTOR ) * 21 ) ;
	for ( i = 0; i < 21; i ++ ) {
		if ( ( flag & 1 ) && !CheckZeroSVector( &adj[ i ] ) ) {
			flag2 |= ( 1 << i ) ;
		}
		flag >>= 1 ;
	}
	m_ctrl->adjust_flag = flag2 ;
}

/* アジャスト値計算 */
static	void	ExprRotAdjust( work )
Work			*work ;
{
	FVECTOR			quat ;
	MOTION_CONTROL	*m_ctrl ;
	SVECTOR			*adj, *tadj ;
	int				i, flag ;

	adj = work->rot_adjusts ;
	tadj = work->turn_adjusts ;
	/* 角度 */
	m_ctrl = work->body.m_ctrl ;
	flag = m_ctrl->adjust_flag ;

	for ( i = 0; i < 21; i ++ ) {
		if ( flag & 1 ) {
			if ( work->adjust_interp_n[ i ] < work->adjust_interp_n_r[ i ] ) {
				work->adjust_interp_n_r[ i ] 
					= GV_NearExp2( work->adjust_interp_n_r[ i ], work->adjust_interp_n[ i ] ) ;
			} else {
				work->adjust_interp_n_r[ i ] = work->adjust_interp_n[ i ] ;
			}

			GV_NearExp16PV( &adj[ i ], &tadj[ i ], 3 ) ;
			//GV_NearExpNPV( &adj[ i ], &tadj[ i ], 3, work->adjust_interp_n_r[ i ] ) ;

			CheckAdjustMinMax( work, i, &adj[ i ] ) ;
			//printf( "[%d] %d : ", GV_Time, i ) ;
			//DumpSVec( &adj[ i ] ) ;
			if ( adj[ i ].vy != 0 ) {
				GM_RotToQuat( &adj[ i ], &quat ) ;
			} else {
				GM_RotToQuatXAfterY( &adj[ i ], &quat ) ;
			}
			DG_COPY_VEC( &m_ctrl->adjust[ i ], &quat ) ;
		}
		flag >>= 1 ;
	}	
}

/* 角度アジャスト値セット */
static	void	SetRotAdjust( work, rot, n ) 
Work			*work ;
SVECTOR			*rot ;
int				n ;
{
	work->turn_adjusts[ n ] = *rot ;
	work->body.m_ctrl->adjust_flag |= 1LL /* Added LL since adjust_flag is 64-bit */ << n ;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	void	SetAction( Work *work, int motion, int interp )
{
	int			rc ;

	if ( work->motion == motion ) return ;
	GM_ConfigObjectAction( &work->body, 0, motion, 0, 0xfffff, interp * NTSC_TIME_BASE ) ;
	if ( interp == 0 ) work->body.m_ctrl->interp_flag = 0 ;
	work->motion = motion ;
	if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
	}
	work->flag |= EE_FLAG_NO_WAIST_INTERP ;
}

static	void	SetMode( Work *work, ACTION action )
{
	work->time = 0 ;
	work->ftime_count = 0 ;
	work->action = action ;
	work->data = work->data2 = 0 ;
}

/*----------------------------------------------------------------*/

#if 0
static	int		CheckRaidenRot( Work *work, short *adjx )
{
	FMATRIX		m1, m2, m3 ;
	FVECTOR		d1, d2, v ;
	SVECTOR		r1, r2, rr1, rr2 ;

	DG_COPY_MAT( &m1, &GM_PlayerBody->objs->objs[ HUMAN21_KOSHI ].world ) ;
	DG_COPY_MAT( &m2, &GM_PlayerBody->objs->objs[ HUMAN21_MUNE ].world ) ;
	DG_COPY_MAT( &m3, &GM_PlayerBody->objs->objs[ HUMAN21_MIGI_ASHI2 ].world ) ;
	_sceVu0InversMatrix( &m1, &m1 ) ;
	_sceVu0MulMatrix( &m2, &m1, &m2 ) ;
	_sceVu0MulMatrix( &m3, &m1, &m3 ) ;
	GV_MatToVec( &m2, &d1 ) ;
	GV_MatToVec( &m3, &d2 ) ;
	if ( DG_FABS( d1.vz ) < 0.10F || DG_FABS( d2.vz ) < 0.10F ) {		
		/* さぼって調整しない */
		return 0 ;
	}

	v.vx = d1.vy ; v.vz = d1.vz ; r1.vx = -GV_VecDir2( &v ) ;
	v.vx = d2.vy ; v.vz = d2.vz ; r2.vx = -GV_VecDir2( &v ) ;

	rr1.vx = GV_DiffDirS( r1.vx, r2.vx ) ;
	if ( rr1.vx < 0 ) rr1.vx += 4096 ;
//	printf( "<%d>\n", GV_Time ) ;
//	printf( "\t<rai>%d : %d %d - ", rr1.vx, r1.vx, r2.vx ) ;	

	DG_COPY_MAT( &m1, &work->body.objs->objs[ HUMAN21_KOSHI ].world ) ;
//	DG_COPY_MAT( &m2, &work->body.objs->objs[ HUMAN21_MUNE ].world ) ;
	DG_COPY_MAT( &m2, &GM_PlayerBody->objs->objs[ HUMAN21_MUNE ].world ) ;
	DG_COPY_MAT( &m3, &work->body.objs->objs[ HUMAN21_MIGI_ASHI2 ].world ) ;
	_sceVu0InversMatrix( &m1, &m1 ) ;
	_sceVu0MulMatrix( &m2, &m1, &m2 ) ;
	_sceVu0MulMatrix( &m3, &m1, &m3 ) ;
	GV_MatToVec( &m2, &d1 ) ;
	GV_MatToVec( &m3, &d2 ) ;
	if ( DG_FABS( d1.vz ) < 0.10F || DG_FABS( d2.vz ) < 0.10F ) {		
		/* さぼって調整しない */
		return 0 ;
	}

	v.vx = d1.vy ; v.vz = d1.vz ; r1.vx = -GV_VecDir2( &v ) ;
	v.vx = d2.vy ; v.vz = d2.vz ; r2.vx = -GV_VecDir2( &v ) ;

	rr2.vx = GV_DiffDirS( r1.vx, r2.vx ) ;
	if ( rr2.vx < 0 ) rr2.vx += 4096 ;
//	printf( "<ee>%d : %d %d\n", rr2.vx, r1.vx, r2.vx ) ;	
//	DumpVec( &d1 ) ;
//	DumpVec( &d2 ) ;

	



	if ( rr1.vx - 320 > rr2.vx + *adjx ) {
//printf( "<%d> %d %d %d \n", GV_Time, rr1.vx, rr2.vx, *adjx ) ;
		*adjx = rr1.vx - 320 - rr2.vx ;
//printf( "-> %d\n", *adjx ) ;
		return 1 ;
	}
	return 0 ;
}
#endif

static	void	AdjustObject( Work *work, FVECTOR *shift )
{
	int			i ;
	DG_OBJS		*objs ;
	FVECTOR		s ;

	DG_COPY_VEC( &s, shift ) ;
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

static	void	MatSlerp( FMATRIX *m3, FMATRIX *m1, FMATRIX *m2, float t )
{
	FVECTOR		quat, pre_quat ;

	MT_MatToQuat( &pre_quat, m1 ) ;
	MT_MatToQuat( &quat, m2 ) ;
	MT_QuatSlerp( &quat, &pre_quat, &quat, t ) ;
	MT_QuatNormalize( &quat, &quat ) ;
	MT_QuatToMat( m3, &quat ) ;
}

#if 0
static	void	SetPosTrans( Work *work, int mode )
{
	FVECTOR		shift, pre_mov, mov, diff ;
	FVECTOR		nv = { 0.0F, 0.0F, 10.0F } ;
	SVECTOR		rot, adjr = { -256, 0, 0 } ;
	FMATRIX		world, pre_world ;

	DG_COPY_MAT( &world, &GM_PlayerBody->objs->objs[ HUMAN21_KUBI ].world ) ;
	GV_MatToVec( &GM_PlayerBody->objs->objs[ HUMAN21_KUBI ].world, &mov ) ;

	DG_COPY_MAT( &pre_world, &work->body.objs->world ) ;
	GV_VecToMat( &DG_ZeroVector, &pre_world ) ;

	GV_VecToMat( &DG_ZeroVector, &world ) ;
	DG_SetPos( &world ) ;
	DG_RotatePos( &adjr ) ;
	DG_GetPos( &world ) ;

	MatSlerp( &world, &pre_world, &world, 0.05F ) ;

	GV_VecToMat( &mov, &world ) ;

	DG_SetPos( &world ) ;
	DG_COPY_VEC( &shift, &Shifts[ mode ] ) ;

	DG_MovePos( &shift ) ;
	DG_GetPos( &world ) ;
	GV_MatToVec( &world, &mov ) ;

	GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &pre_mov ) ;
	mov.vx = pre_mov.vx * 0.50F + mov.vx * 0.50F ;
	mov.vy = pre_mov.vy * 0.50F + mov.vy * 0.50F ;
	mov.vz = pre_mov.vz * 0.50F + mov.vz * 0.50F ;
	GV_VecToMat( &mov, &world ) ;
	DG_SetPos( &world ) ;

	GM_ActObject2( &work->body ) ;
	
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &diff ) ;
	_sceVu0SubVector( &diff, &mov, &diff ) ;
	AdjustObject( work, &diff ) ;

	GV_MatToVec( &work->body.objs->world, &work->control.mov ) ;

	DG_RotVector( &nv, &nv, 1 ) ;
	GV_VecToRot( &nv, &rot ) ;
#if 0
	memmove( &work->rot_buf[ 1 ], &work->rot_buf[ 0 ], sizeof( SVECTOR ) * 31 ) ;
	work->rot_buf[ 0 ] = rot ;

	{
		SVECTOR		adj = { 0, 0, 0 }, adj2, adj3 ;
		int			diff, speed, lev, c ;

		adj2 = work->turn_adjusts[ 23 ] ;
		adj2.vx = adj2.vx * 4 / 3 ;

		if ( adj2.vx > 512 ) adj2.vx = 512 ;
		else if ( adj2.vx < -200 ) adj2.vx = -200 ;	/* きめうち */
		lev = ( adj2.vx < 0 ) ? -adj2.vx : adj2.vx ;
		if ( lev < 128 ) GV_NearSpeedPV( &work->turn_adjusts_buf[ 0 ], &adj2, 3, 4 ) ;
		else if ( adj2.vx < 0 ) GV_NearExpNPV( &work->turn_adjusts_buf[ 0 ], &adj2, 3, 16 ) ;
		else 					GV_NearExpNPV( &work->turn_adjusts_buf[ 0 ], &adj2, 3, 24 ) ;
		SetRotAdjust( work, &work->turn_adjusts_buf[ 0 ], HUMAN21_MIGI_ASHI2 ) ;
		SetRotAdjust( work, &work->turn_adjusts_buf[ 0 ], HUMAN21_HIDARI_ASHI2 ) ;
		adj3 = adj2 ;

		adj2 = work->turn_adjusts[ 22 ] ;
		adj2.vx = adj2.vx * 5 / 3 ;
		if ( adj2.vx < -200 ) adj2.vx = -200 ;
		else if ( adj2.vx > 200 ) adj2.vx = 200 ;
		lev = ( adj2.vx < 0 ) ? -adj2.vx : adj2.vx ;
		if ( lev < 128 ) GV_NearSpeedPV( &work->turn_adjusts_buf[ 1 ], &adj2, 3, 4 ) ;
		else if ( adj2.vx < 0 ) GV_NearExpNPV( &work->turn_adjusts_buf[ 1 ], &adj2, 3, 24 ) ;
		else 					GV_NearExpNPV( &work->turn_adjusts_buf[ 1 ], &adj2, 3, 32 ) ;
		SetRotAdjust( work, &work->turn_adjusts_buf[ 1 ], HUMAN21_MIGI_KAKATO ) ;
		SetRotAdjust( work, &work->turn_adjusts_buf[ 1 ], HUMAN21_HIDARI_KAKATO ) ;

		work->turn_adjusts[ 22 ] = adj3 ;

		adj.vx = ( work->rot_buf[ 4 ].vx - work->rot_buf[ 3 ].vx ) ;
		if ( adj.vx > 0 ) adj.vx *= 16 ;
		else			  adj.vx *= 48 ;
		if ( adj.vx > 1024 ) adj.vx = 1024 ;
		else if ( adj.vx < -1024 ) adj.vx = -1024 ;

		work->turn_adjusts[ 23 ] = adj ;

		c = CheckRaidenRot( work, &adj.vx ) ;

		lev = ( adj.vx < 0 ) ? -adj.vx : adj.vx ;
		if ( lev < 128 )	GV_NearSpeedPV( &work->turn_adjusts_buf[ 2 ], &adj, 3, 4 ) ;
		else if ( adj.vx < 0 ) GV_NearExpNPV( &work->turn_adjusts_buf[ 2 ], &adj, 3, 8 ) ;
		else 					GV_NearExpNPV( &work->turn_adjusts_buf[ 2 ], &adj, 3, 16 ) ;

		if ( c && work->turn_adjusts_buf[ 2 ].vx < adj.vx ) {
//			work->turn_adjusts_buf[ 2 ].vx = adj.vx ;
            GV_NearExpNPV( &work->turn_adjusts_buf[ 2 ], &adj, 3, 4 ) ;
		}

		adj2 = work->turn_adjusts_buf[ 2 ] ;
		SetRotAdjust( work, &adj2, HUMAN21_MIGI_ASHI1 ) ;
		SetRotAdjust( work, &adj2, HUMAN21_HIDARI_ASHI1 ) ;
	}
#endif
	rot.vz = GM_PlayerControl->rot.vz ;
	work->control.rot = work->control.turn = rot ;
}
#endif

static	inline	void	ExecProc( int proc )
{
	if ( proc != 0 ) GM_ExecProc( proc, NULL ) ;
}

/*----------------------------------------------------------------*/

static	void	DiveHeightAdjust( Work *work )
{
	DG_OBJ			*obj ;
	DG_MDL			*mdl ;
	FVECTOR			pos, vec ;
	float			max, min, y ;
	float			lheight, uheight ;
	int				i, n_models, flag ;
	float			levels[ 2 ], len ;

	GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &pos ) ;
	if ( HZX_OnlineHazardCheck( HZX_AllMapID, &work->control.mov, &pos,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVector( &vec ) ;
		len = GV_VecLen3F( &vec ) ;
		if ( len > 8.0F ) {
			GV_LenVec3F( &vec, &vec, 0.0F, len - 8.0F ) ;
			_sceVu0AddVector( &pos, &work->control.mov, &vec ) ;
		}
		printf( "ee koshi kubi hazard! %f\n", len ) ;
	}
	flag = HZX_LevelHazardCheck( HZX_AllMapID, &pos, 
								 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
	HZX_GetLevelHeight( levels ) ;
	if ( !( flag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( !( flag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
#if 0
	levels[ 0 ] = ( work->control.levels[ 0 ] > levels[ 0 ] ) ? 
		work->control.levels[ 0 ] : levels[ 0 ] ;
	levels[ 1 ] = ( work->control.levels[ 1 ] < levels[ 1 ] ) ? 
		work->control.levels[ 1 ] : levels[ 1 ] ;
#endif
	if ( levels[ 1 ] > GM_WaterLevel ) levels[ 1 ] = GM_WaterLevel ;

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

	//if ( lheight < work->control.height ) lheight = work->control.height ;

	/* 天井回避優先 */
	if ( levels[ 1 ] - uheight < work->control.mov.vy ) {
		work->control.step.vy = -GV_NearExp2F( 0.0F, work->control.mov.vy 
											  - ( levels[ 1 ] - uheight ) ) ;
		work->flag |= EE_FLAG_NO_MOTION_STEP_Y ;
		//printf( "0 : %f\n", work->control.step.vy ) ;
	} else if ( levels[ 0 ] + lheight > work->control.mov.vy ) {
		work->control.step.vy = GV_NearExp2F( 0.0F, levels[ 0 ] + lheight 
											  - work->control.mov.vy ) ;
		work->flag |= EE_FLAG_NO_MOTION_STEP_Y ;
		//printf( "1 : %f\n", work->control.step.vy ) ;
	}
}

/* めりこみ回避 */
static	void	AvoidSink( Work *work )
{
	FVECTOR		front, back, adj, pos, root, v ;
	FVECTOR		unitVec = { 0.0F, 0.0F, 100.0F } ;
	float		len, flen, blen, intp ;
	int			i, c ;
	
	DG_SetPos( &work->body.objs->world ) ;
	DG_RotVector( &unitVec, &unitVec, 1 ) ;
	DG_COPY_VEC( &root, &work->control.mov ) ;
	flen = blen = 0.0F ;
	for ( i = 0; i < 21; i ++ ) {
		GV_MatToVec( &work->body.objs->objs[ i ].world, &pos ) ;
		_sceVu0SubVector( &v, &pos, &root ) ;
		intp = _sceVu0InnerProduct( &unitVec, &v ) ;
		len = GV_VecLen3F( &v ) ;
		if ( intp >= 0 ) {
			if ( flen < len ) {
				flen = len ;
				DG_COPY_VEC( &front, &pos ) ;
			}
		} else {
			if ( blen < len ) {
				blen = len ;
				DG_COPY_VEC( &back, &pos ) ;
			}
		}
	}
	c = 0 ;
	if ( HZX_OnlineHazardCheck( HZX_AllMapID, &root, &front, HZX_CHK_ALL,
							    HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
							    HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlinePoint( &adj ) ;
		_sceVu0SubVector( &adj, &adj, &front ) ;
		c = 1 ;
	} else if ( HZX_OnlineHazardCheck( HZX_AllMapID, &root, &back, HZX_CHK_ALL,
									   HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
									   HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlinePoint( &adj ) ;
		_sceVu0SubVector( &adj, &adj, &back ) ;
		c = 1 ;
	}
	if ( c ) {
		adj.vy = 0.0F ;
		_sceVu0AddVector( &work->control.step, &work->control.step, &adj ) ;
	}
}

/* プレイヤーを押してみる */
static	void	PushPlayer( Work *work )
{
	FVECTOR		diff ;
	float		len ;

	_sceVu0SubVector( &diff, &GM_PlayerControl->mov, &work->control.mov ) ;
	diff.vy = 0.0F ;
	len = GV_VecLen3F( &diff ) ;
	if ( len < 1000.0F ) {
		if ( len > 32.0F ) {
			GV_LenVec3F( &diff, &diff, 0.0F, 32.0F ) ;
		} else {
			GV_LenVec3F( &diff, &diff, 0.0F, len ) ;
		}
		GM_PlayerControl->step.vx += diff.vx ;
		GM_PlayerControl->step.vz += diff.vz ;
		/* エマも動かす */
		work->control.step.vx -= diff.vx ;
		work->control.step.vz -= diff.vz ;
	}
}

/*----------------------------------------------------------------*/

static	void	GetRootWorld( FMATRIX *world, int flag )
{
	FMATRIX		tmpMat ;
	FVECTOR		tmpVec, def ;
	SVECTOR		tmpRot, defRot ;
	FVECTOR		unitVec = { 0.0F, 0.0F, 1000.0F } ;
	float		intp ;

	GV_SetVec3( &defRot, 0, GM_PlayerControl->rot.vy, 0 ) ;

	if ( flag == 0 ) {
		//DG_COPY_MAT( world, &GM_PlayerBody->objs->world ) ;
		DG_SetPos2( ( FVECTOR * )GM_PlayerBody->objs->objs[ 0 ].world .m[ 3 ], 
				   &GM_PlayerControl->rot ) ;
		DG_GetPos( world ) ;
	} else if ( flag == 1 ) {
		DG_COPY_MAT( world, &GM_PlayerBody->objs->objs[ 0 ].world ) ;
	} else if ( flag == 2 ) {
		/* 腰のＹ回転無視 */
		DG_COPY_MAT( &tmpMat, &GM_PlayerBody->objs->objs[ 0 ].world ) ;
		DG_SetPos( &tmpMat ) ;
		DG_RotVector( &unitVec, &tmpVec, 1 ) ;

		DG_SetPos2( &GM_PlayerControl->mov, &defRot ) ;
		DG_RotVector( &unitVec, &def, 1 ) ;
		intp = _sceVu0InnerProduct( &tmpVec, &def ) ;
		
		GV_VecToRot( &tmpVec, &tmpRot ) ;
		if ( intp <= 0 ) tmpRot.vx = -tmpRot.vx + 2048 ;
		tmpRot.vy = GM_PlayerControl->rot.vy ;
		DG_SetPos2( ( FVECTOR * )tmpMat.m[ 3 ], &tmpRot ) ;
		DG_GetPos( world ) ;
	} else if ( flag == 3 ) {
		/* 回転は胸、位置は腰 */
		DG_COPY_MAT( &tmpMat, &GM_PlayerBody->objs->objs[ HUMAN21_MUNE ].world ) ;
		GV_VecToMat( ( FVECTOR * )GM_PlayerBody->objs->world.m[ 3 ], &tmpMat ) ;
		DG_COPY_MAT( world, &tmpMat ) ;
	} else if ( flag == 4 ) {
		/* 腰の回転 - X1024 */
		DG_COPY_MAT( &tmpMat, &GM_PlayerBody->objs->objs[ 0 ].world ) ;
		DG_SetPos( &tmpMat ) ;
		DG_RotVector( &unitVec, &tmpVec, 1 ) ;

		DG_SetPos2( &GM_PlayerControl->mov, &defRot ) ;
		DG_RotVector( &unitVec, &def, 1 ) ;
		intp = _sceVu0InnerProduct( &tmpVec, &def ) ;

		GV_VecToRot( &tmpVec, &tmpRot ) ;
		if ( intp <= 0 ) tmpRot.vx = -tmpRot.vx + 2048 ;

		tmpRot.vx -= 1024 ;
		DG_SetPos2( ( FVECTOR * )tmpMat.m[ 3 ], &tmpRot ) ;
		DG_GetPos( world ) ;
	} else if ( flag == 5 ) {
		/* 回転は胸 - X1024、位置は腰 */
		DG_COPY_MAT( &tmpMat, &GM_PlayerBody->objs->objs[ HUMAN21_MUNE ].world ) ;
		GV_VecToMat( ( FVECTOR * )GM_PlayerBody->objs->world.m[ 3 ], &tmpMat ) ;
		DG_SetPos( &tmpMat ) ;
		DG_RotVector( &unitVec, &tmpVec, 1 ) ;

		DG_SetPos2( &GM_PlayerControl->mov, &defRot ) ;
		DG_RotVector( &unitVec, &def, 1 ) ;
		intp = _sceVu0InnerProduct( &tmpVec, &def ) ;

		GV_VecToRot( &tmpVec, &tmpRot ) ;
		if ( intp <= 0 ) tmpRot.vx = -tmpRot.vx + 2048 ;

		tmpRot.vx -= 1024 ; tmpRot.vy = GM_PlayerControl->rot.vy ;

		DG_SetPos2( ( FVECTOR * )tmpMat.m[ 3 ], &tmpRot ) ;
		DG_GetPos( world ) ;
	} else if ( flag == 6 ) {
		FMATRIX		tmpMat2 ;
		/* 回転は胸 - X1024、位置は腰 （２）*/
		GV_SetVec3( &tmpRot, -1024, 0, 0 ) ;
		DG_SetPos2( &DG_ZeroVector, &tmpRot ) ;
		DG_GetPos( &tmpMat2 ) ;
		DG_COPY_MAT( &tmpMat, &GM_PlayerBody->objs->objs[ HUMAN21_MUNE ].world ) ;
		GV_VecToMat( &DG_ZeroVector, &tmpMat ) ;
		_sceVu0MulMatrix( &tmpMat, &tmpMat, &tmpMat2 ) ;
		GV_VecToMat( ( FVECTOR * )GM_PlayerBody->objs->world.m[ 3 ], &tmpMat ) ;
		DG_SetPos( &tmpMat ) ;		
		DG_GetPos( world ) ;
	} else if ( flag == 7 ) {
		FMATRIX		tmpMat2 ;
		/* 腰の回転 + X64 */
		GV_SetVec3( &tmpRot, 64, 0, 0 ) ;
		DG_SetPos2( &DG_ZeroVector, &tmpRot ) ;
		DG_GetPos( &tmpMat2 ) ;
		DG_SetPos2( ( FVECTOR * )GM_PlayerBody->objs->objs[ 0 ].world .m[ 3 ], 
				   &GM_PlayerControl->rot ) ;
		DG_GetPos( &tmpMat ) ;
		GV_VecToMat( &DG_ZeroVector, &tmpMat ) ;
		_sceVu0MulMatrix( &tmpMat, &tmpMat, &tmpMat2 ) ;
		GV_VecToMat( ( FVECTOR * )GM_PlayerBody->objs->objs[ 0 ].world.m[ 3 ], &tmpMat ) ;
		DG_SetPos( &tmpMat ) ;		
		DG_GetPos( world ) ;
	}
}

/* ＩＫ */
static	void	SetAdjustInterpN( Work *work, SVECTOR *rot, int unit, int v1, int v2 )
{
	int			n ;

	if ( rot->vx > 1600 ) rot->vx = 1600 ;
	else if ( rot->vx < -1600 ) rot->vx = -1600 ;

	if ( rot->vx > v1 || rot->vx < -v1 ) n = 16 ;
	else if ( rot->vx > v2 || rot->vx < -v2 ) n = 32 ;
	else n = 48 ;
	work->adjust_interp_n[ unit ] = n ;
}

static	void	UpdateIKControl( Work *work )
{
	IK_CONTROL	*ik_ctrl ;
	int			i ;

	ik_ctrl = &work->ik_ctrl ;

	for ( i = 0;i < 21; i ++ ) {
		DG_COPY_MAT( &ik_ctrl->prev[ i ], &work->body.objs->objs[ i ].world ) ;
	}
}

static	void	LegIK( Work *work )
{
	IK_CONTROL	*ik_ctrl ;
	FMATRIX		m1, m2, m3 ;
	FVECTOR		unitVec = { 0.0F, 0.0F, 10.0F } ;
	FVECTOR		v1, v2, defv, diff ;
	SVECTOR		r1, r2, r3, defr, resr, resr2, resr3 ;
	float		len ;

	defr.vx = defr.vz = 0 ;
	if ( work->flag & EE_FLAG_STAND ) {
		defr.vy = GM_PlayerControl->rot.vy ;
	} else {
		GV_MatToVec( &work->body.objs->objs[ 0 ].world, &v1 ) ;
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &v2 ) ;
		defr.vy = GV_VecDir2FromTo( &v1, &v2 ) ;
	}
	DG_SetPos2( &DG_ZeroVector, &defr ) ;
	DG_RotVector( &unitVec, &defv, 1 ) ;

	ik_ctrl = &work->ik_ctrl ;

	DG_COPY_MAT( &m1, &ik_ctrl->prev[ 0 ] ) ;
	DG_COPY_MAT( &m2, &work->body.objs->objs[ 0 ].world ) ;

	GV_VecToMat( &DG_ZeroVector, &m1 ) ;
	GV_VecToMat( &DG_ZeroVector, &m2 ) ;
	MatSlerp( &m3, &m1, &m2, 1.0F ) ;

	DG_SetPos( &m1 ) ;
	DG_RotVector( &unitVec, &v1, 1 ) ;
	GV_VecToRot( &v1, &r1 ) ;

	if ( _sceVu0InnerProduct( &defv, &v1 ) <= 0 ) {
		r1.vx = -r1.vx + 2048 ;
		r1.vy += 2048 ;
		r1.vx &= 4095 ;
		r1.vy &= 4095 ;
	}

	DG_SetPos( &m3 ) ;
	DG_RotVector( &unitVec, &v2, 1 ) ;
	GV_VecToRot( &v2, &r2 ) ;

	if ( GV_DiffDirAbs( r1.vy, r2.vy ) > 1024 ) {
		r2.vx = -r2.vx + 2048 ;
		r2.vy += 2048 ;
		r2.vx &= 4095 ;
		r2.vy &= 4095 ;
	}

	r3.vx = -GV_DiffDirS( r1.vx, r2.vx ) ;
	r3.vy = 0 ;
	r3.vz = 0 ;
#if 0
	printf( "-----\n" ) ;
	DumpSVec( &defr ) ;
	DumpSVec( &r1 ) ;
	DumpSVec( &r2 ) ;
	DumpSVec( &r3 ) ;
#endif

	DG_COPY_MAT( &m1, &ik_ctrl->prev[ 0 ] ) ;
	DG_COPY_MAT( &m2, &work->body.objs->objs[ 0 ].world ) ;
	_sceVu0SubVector( &diff, ( FVECTOR * )m2.m[ 3 ], ( FVECTOR * )m1.m[ 3 ] ) ;

	if ( !( work->flag & EE_FLAG_NO_MOVEIK ) ) {
		if ( work->flag & EE_FLAG_STAND ) {	
			/* ＸＺ */	
			diff.vy = 0.0F ;
			len = GV_VecLen3F( &diff ) ;
			r3.vx += ( int )( len * 0.90F ) ;
			LegIKAdj = 0 ;
			LegIKAdjAim = 0 ;
		} else {
			/* Ｙ */
			r3.vx -= ( int )( diff.vy * 1.05F ) ;

			{
				int			dy ;

				dy = GV_DiffDirS( GM_PlayerControl->rot.vy, GM_PlayerControl->turn.vy ) ;
				r3.vy = -dy * 2 ;

				if ( ( GV_DiffDirAbs( r1.vx, 1024 ) < 256 || GV_DiffDirAbs( r1.vx, -1024 ) < 256 ) &&
					 ( r3.vx < 32 && r3.vx > -32 ) ) {
					len = GV_VecLen3F( &diff ) ;
					if ( len > 48.0F ) {
						if ( r3.vx > 0 ) LegIKAdjAim = 16 ;
						else			 LegIKAdjAim = -4 ;
					} else if ( len > 32.0F ) {
						if ( r3.vx > 0 ) LegIKAdjAim = 20 ;
						else			 LegIKAdjAim = -4 ;
					} else {
						LegIKAdj = 0 ;
						LegIKAdjAim = 0 ;
					}
					LegIKAdj = GV_NearExp8P( LegIKAdj, LegIKAdjAim ) ;
					r3.vx += LegIKAdj ;
					//printf( "%f %d\n", len, r3.vx ) ;
				} else {
					LegIKAdj = 0 ;
					LegIKAdjAim = 0 ;
				}
			}
		}
	}

	{
		int		i ;

		for ( i = 31; i >= 1; i -- ) {
			ik_ctrl->buffer[ i ] = ik_ctrl->buffer[ i - 1 ] ;
		}
		ik_ctrl->buffer[ 0 ] = r3 ;
	}
#if 1
	resr = r3 ; resr.vx *= 16 ;
	SetRotAdjust( work, &resr, HUMAN21_MIGI_ASHI1 ) ;
	SetAdjustInterpN( work, &resr, HUMAN21_MIGI_ASHI1, 768, 512 ) ;
	ik_ctrl->adjust[ 0 ] = resr ;
	
	resr = ik_ctrl->buffer[ 8 ] ;
	resr.vx *= 16 ;
	resr.vy = 0 ;
	
	resr2 = resr ;
	resr2.vx = ( int )( ( float )resr2.vx * 1.15F ) ;
	if ( resr2.vx < -512 ) resr2.vx = -512 ;
	SetRotAdjust( work, &resr2, HUMAN21_MIGI_ASHI2 ) ;
	SetAdjustInterpN( work, &resr2, HUMAN21_MIGI_ASHI2, 1024, 320 ) ;
	ik_ctrl->adjust[ 1 ] = resr2 ;

	resr = ik_ctrl->buffer[ 16 ] ;
	resr.vx *= 16 ;
	resr.vy = 0 ;
	
	resr3 = resr ;
	resr3.vx = resr.vx * 1.15F * 2.0F ;
	//if ( resr3.vx < -128 ) resr3.vx = -128 ;
	SetRotAdjust( work, &resr3, HUMAN21_MIGI_KAKATO ) ;
	SetAdjustInterpN( work, &resr3, HUMAN21_MIGI_KAKATO, 1024, 320 ) ;
	ik_ctrl->adjust[ 2 ] = resr3 ;
	
	resr = r3 ; resr.vx *= 14 ;
	SetRotAdjust( work, &resr, HUMAN21_HIDARI_ASHI1 ) ;
	SetAdjustInterpN( work, &resr, HUMAN21_HIDARI_ASHI1, 768, 512 ) ;
	ik_ctrl->adjust[ 3 ] = resr ;

	resr = ik_ctrl->buffer[ 12 ] ;
	resr.vx *= 14 ;
	resr.vy = 0 ;	
	
	resr2 = resr ;
	resr2.vx = ( int )( ( float )resr2.vx * 1.50F ) ;
	if ( resr2.vx < -512 ) resr2.vx = -512 ;
	SetRotAdjust( work, &resr2, HUMAN21_HIDARI_ASHI2 ) ;
	SetAdjustInterpN( work, &resr2, HUMAN21_HIDARI_ASHI2, 1024, 320 ) ;
	ik_ctrl->adjust[ 4 ] = resr2 ;
	
	resr = ik_ctrl->buffer[ 20 ] ;
	resr.vx *= 14 ;
	resr.vy = 0 ;	

	resr3 = resr ;
	resr3.vx = resr.vx * 1.50F * 2.0F ;
	//if ( resr3.vx < -128 ) resr3.vx = -128 ;
	SetRotAdjust( work, &resr3, HUMAN21_HIDARI_KAKATO ) ;
	SetAdjustInterpN( work, &resr3, HUMAN21_HIDARI_KAKATO, 1024, 320 ) ;
	ik_ctrl->adjust[ 5 ] = resr3 ;
#endif
}

/* ライデンとの交差チェック */
static	void	CrossCheck( Work *work )
{
	FVECTOR		pos, pos2, res ;
	SVECTOR		rot ;

	/* 右足 */
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_ASHI2 ].world, &pos ) ;
	if ( CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_ASHI1, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_ASHI2, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_ASHI1, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_ASHI2, 1.0F ) ) {
		rot = work->rot_adjusts[ HUMAN21_MIGI_ASHI1 ] ;
		rot.vx += 320 ;
		SetRotAdjust( work, &rot, HUMAN21_MIGI_ASHI1 ) ;
        work->adjust_interp_n[ HUMAN21_MIGI_ASHI1 ] 
			= work->adjust_interp_n_r[ HUMAN21_MIGI_ASHI1 ] = 8 ;
	}
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_KAKATO ].world, &pos ) ;
	if ( CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_ASHI2, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_KAKATO, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_ASHI2, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_KAKATO, 1.0F ) ) {
		rot = work->rot_adjusts[ HUMAN21_MIGI_ASHI2 ] ;
		rot.vx += 320 ;
		SetRotAdjust( work, &rot, HUMAN21_MIGI_ASHI2 ) ;
        work->adjust_interp_n[ HUMAN21_MIGI_ASHI2 ] 
			= work->adjust_interp_n_r[ HUMAN21_MIGI_ASHI2 ] = 8 ;
	} else {
		/* ここだけ壁チェック */
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_ASHI2 ].world, &pos ) ;		
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_KAKATO ].world, &pos2 ) ;		
		if ( HZX_OnlineHazardCheck( HZX_AllMapID, &pos, &pos2, HZX_CHK_ALL,
								    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
			//printf( "right : ashi - kakato hazard\n" ) ;
			rot = work->rot_adjusts[ HUMAN21_MIGI_ASHI2 ] ;
			rot.vx -= 640 ;
			SetRotAdjust( work, &rot, HUMAN21_MIGI_ASHI2 ) ;
			work->adjust_interp_n[ HUMAN21_MIGI_ASHI2 ] 
				= work->adjust_interp_n_r[ HUMAN21_MIGI_ASHI2 ] = 8 ;			
		}
	}


	/* 左足 */
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_ASHI2 ].world, &pos ) ;
	if ( CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_ASHI1, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_ASHI2, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_ASHI1, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_ASHI2, 1.0F ) ) {
		rot = work->rot_adjusts[ HUMAN21_HIDARI_ASHI1 ] ;
		rot.vx += 320 ;
		SetRotAdjust( work, &rot, HUMAN21_HIDARI_ASHI1 ) ;
        work->adjust_interp_n[ HUMAN21_HIDARI_ASHI1 ] 
			= work->adjust_interp_n_r[ HUMAN21_HIDARI_ASHI1 ] = 8 ;
	}
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_KAKATO ].world, &pos ) ;
	if ( CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_ASHI2, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_MIGI_KAKATO, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_ASHI2, 1.0F ) ||
		 CalcObjCollision( &res, &pos, GM_PlayerBody->objs, HUMAN21_HIDARI_KAKATO, 1.0F ) ) {
		rot = work->rot_adjusts[ HUMAN21_HIDARI_ASHI2 ] ;
		rot.vx += 320 ;
		SetRotAdjust( work, &rot, HUMAN21_HIDARI_ASHI2 ) ;
        work->adjust_interp_n[ HUMAN21_HIDARI_ASHI2 ] 
			= work->adjust_interp_n_r[ HUMAN21_HIDARI_ASHI2 ] = 8 ;
	} else {
		/* ここだけ壁チェック */
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_ASHI2 ].world, &pos ) ;		
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_KAKATO ].world, &pos2 ) ;		
		if ( HZX_OnlineHazardCheck( HZX_AllMapID, &pos, &pos2, HZX_CHK_ALL,
								    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
			//printf( "left : ashi - kakato hazard\n" ) ;
			rot = work->rot_adjusts[ HUMAN21_HIDARI_ASHI2 ] ;
			rot.vx -= 640 ;
			SetRotAdjust( work, &rot, HUMAN21_HIDARI_ASHI2 ) ;
			work->adjust_interp_n[ HUMAN21_HIDARI_ASHI2 ] 
				= work->adjust_interp_n_r[ HUMAN21_HIDARI_ASHI2 ] = 8 ;			
		}
	}
}

/* 腕ＩＫ */
static	void	ArmIK( Work *work )
{
	FVECTOR		p1, p2 ;
	SVECTOR		rot ;
	float		len ;

	GV_MatToVec( &work->ik_ctrl.prev[ 0 ], &p1 ) ;
	GV_MatToVec( &work->body.objs->objs[ 0 ].world, &p2 ) ;	
	len = GV_VecLen3F2( &p1, &p2 ) ;
	if ( ( work->flag & EE_FLAG_STAND ) && len > 16.0F ) {
		rot.vx = -128 ;
		rot.vy = 160 ;
		rot.vz = 0 ;
		SetRotAdjust( work, &rot, HUMAN21_MIGI_KATA ) ;
		work->adjust_interp_n[ HUMAN21_MIGI_KATA ] = 8 ;
		rot.vy *= -1 ;
		SetRotAdjust( work, &rot, HUMAN21_HIDARI_KATA ) ;
		work->adjust_interp_n[ HUMAN21_HIDARI_KATA ] = 8 ;
	}
}

/* 頭ＩＫ */
static	void	HeadIK( Work *work )
{
	SVECTOR		rot, rtmp ;
	FVECTOR		bppos, step ;
	int			v, c ;
	int			mode ;

	if ( work->flag & EE_FLAG_NO_HEADIK ) return ;

	c = PL_GuideToBreathPoint( &step, &work->control.mov, 4000.0F, 1000.0F, &bppos ) ;
	if ( c ) work->ik_ctrl.head_ik_flag = 0 ;

	if ( work->ik_ctrl.head_ik_flag > 0 ) {
		if ( ( work->flag & EE_FLAG_STAND ) && ( work->ik_ctrl.head_ik_mode != 1 ) ) {
			work->ik_ctrl.head_ik_flag = 0 ;			
			return ;
		}
		if ( !( work->flag & EE_FLAG_STAND ) && ( work->ik_ctrl.head_ik_mode != 2 ) ) {
			work->ik_ctrl.head_ik_flag = 0 ;			
			return ;
		}
		-- work->ik_ctrl.head_ik_flag ;
		SetRotAdjust( work, &work->ik_ctrl.head_adjust, HUMAN21_ATAMA ) ;
		work->adjust_interp_n[ HUMAN21_ATAMA ] = 24 ;
		if ( work->ik_ctrl.head_ik_flag == 0 ) {
			work->ik_ctrl.head_ik_flag = -180 ;
			//work->ik_ctrl.head_ik_flag = -10 ;
		}
		return ;
	} else if ( work->ik_ctrl.head_ik_flag < 0 ) {
		++ work->ik_ctrl.head_ik_flag ;
		return ;
	}

	GV_SetVec3( &rot, 0, 0, 0 ) ;

	mode = 0 ;
#if 1
	v = irnd() % 4 ;
	if ( v == 0 ) mode = 1 ;
	else if ( v == 1  ) mode = 2 ;
	else if ( v == 2  ) mode = 3 ;
#else
	if ( GV_PadData->press & PAD_X ) mode = 1 ;
	else if ( GV_PadData->press & PAD_Y ) mode = 2 ;
	else if ( GV_PadData->press & PAD_L1 ) mode = 3 ;
#endif

	if ( work->flag & EE_FLAG_STAND ) {
		work->ik_ctrl.head_ik_mode = 1 ;
		if ( c && !GM_CheckPlayerStatus( PLAYER_WATER_SURFACE ) ) {
			_sceVu0SubVector( &step, &bppos, 
							 ( FVECTOR * )&work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ] ) ;
			GV_VecToRot( &step, &rtmp ) ;
			if ( GV_DiffDirAbs( work->control.rot.vy, rtmp.vy ) > 768 ) goto to_bp_skip_1 ;
			rtmp.vy = GV_DiffDirS( work->control.rot.vy, rtmp.vy ) ;
			work->ik_ctrl.head_ik_flag = 0 ;
			work->ik_ctrl.head_adjust = rtmp ;
			SetRotAdjust( work, &work->ik_ctrl.head_adjust, HUMAN21_ATAMA ) ;
			work->adjust_interp_n[ HUMAN21_ATAMA ] = 24 ;
			return ;
		}
to_bp_skip_1 :		
	    if ( mode == 1 ) {
			/* 頭を起こす */
			GV_SetVec3( &rtmp, -320, 0, 0 ) ;
			rot.vx += rtmp.vx ;
			rot.vy += rtmp.vy ;
			rot.vz += rtmp.vz ;
			work->ik_ctrl.head_ik_flag = 240 ;
		} else if ( mode == 2 ) {
			/* 右 */
			GV_SetVec3( &rtmp, -256, 320, 512 ) ;			
			rot.vx += rtmp.vx ;
			rot.vy += rtmp.vy ;
			rot.vz += rtmp.vz ;
			work->ik_ctrl.head_ik_flag = 240 ;
		} else if ( mode == 3 ) {
			/* 左 */
			GV_SetVec3( &rtmp, -320, -800, 0 ) ;
			rot.vx += rtmp.vx ;
			rot.vy += rtmp.vy ;
			rot.vz += rtmp.vz ;
			work->ik_ctrl.head_ik_flag = 240 ;
		}
	} else {
		work->ik_ctrl.head_ik_mode = 2 ;
		if ( c ) {
			_sceVu0SubVector( &step, &bppos, 
							 ( FVECTOR * )&work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ] ) ;
			GV_VecToRot( &step, &rtmp ) ;
			if ( GV_DiffDirAbs( work->control.rot.vy, rtmp.vy ) > 768 ) goto to_bp_skip_2 ;
			rtmp.vy = GV_DiffDirS( work->control.rot.vy, rtmp.vy ) ;
			work->ik_ctrl.head_ik_flag = 0 ;
			work->ik_ctrl.head_adjust = rtmp ;
			SetRotAdjust( work, &work->ik_ctrl.head_adjust, HUMAN21_ATAMA ) ;
			work->adjust_interp_n[ HUMAN21_ATAMA ] = 24 ;
			return ;
		}
to_bp_skip_2 :
  	    if ( mode == 1 ) {
			GV_SetVec3( &rtmp, -512, 0, 0 ) ;
			rot.vx += rtmp.vx ;
			rot.vy += rtmp.vy ;
			rot.vz += rtmp.vz ;
			work->ik_ctrl.head_ik_flag = 240 ;
		} else if ( mode == 2 ) {
			GV_SetVec3( &rtmp, -256, -512, -512 ) ;			
			rot.vx += rtmp.vx ;
			rot.vy += rtmp.vy ;
			rot.vz += rtmp.vz ;
			work->ik_ctrl.head_ik_flag = 240 ;
		} else if ( mode == 3 ) {
			GV_SetVec3( &rtmp, -256, 512, 512 ) ;
			rot.vx += rtmp.vx ;
			rot.vy += rtmp.vy ;
			rot.vz += rtmp.vz ;
			work->ik_ctrl.head_ik_flag = 240 ;
		}
	}
	work->ik_ctrl.head_adjust = rot ;
}

/* 位置セット */
static	void	SetPosNormal( Work *work, int mode, int flag, float t, float t2 )
{
	FMATRIX		pWorld, preWorld, nextWorld, mat ;
	FVECTOR		preMov, nextMov, diff ;
	FVECTOR		unitVec = { 0.0F, 0.0F, 10.0F } ;
	FVECTOR		tmpVec, tmpVec2 ;
	SVECTOR		defRot ;
	float		intp ;

	GetRootWorld( &pWorld, flag ) ;

	DG_COPY_MAT( &preWorld, &work->body.objs->world ) ;
	GV_VecToMat( &DG_ZeroVector, &pWorld ) ;
	GV_VecToMat( &DG_ZeroVector, &preWorld ) ;
	MatSlerp( &nextWorld, &preWorld, &pWorld, t ) ;	

	GetRootWorld( &pWorld, flag ) ;
	//HZX_ViewMatrix( &pWorld, 750.0F ) ;
	GV_VecToMat( ( FVECTOR * )GM_PlayerBody->objs->objs[ HUMAN21_KUBI ].world.m[ 3 ], &pWorld ) ;
	DG_SetPos( &pWorld ) ;
	GV_NearExpNVF( &work->shift, &Shifts[ mode ], 24.0F, 3 ) ;
	DG_COPY_VEC( &work->shift, &Shifts[ mode ] ) ;
	DG_MovePos( &work->shift ) ;
	DG_GetPos( &mat ) ;
	GV_MatToVec( &mat, &nextMov ) ;
	DG_COPY_VEC( &preMov, ( FVECTOR * )work->body.objs->objs[ HUMAN21_KUBI ].world.m[ 3 ] ) ;
	nextMov.vx = preMov.vx * ( 1 - t2 ) + nextMov.vx * t2 ;
	nextMov.vy = preMov.vy * ( 1 - t2 ) + nextMov.vy * t2 ;
	nextMov.vz = preMov.vz * ( 1 - t2 ) + nextMov.vz * t2 ;
	//printf( "step %f\n", GV_VecLen3F2( &preMov, &nextMov ) ) ;
	
	GV_VecToMat( &nextMov, &nextWorld ) ;
	DG_SetPos( &nextWorld ) ;

	if ( work->interp > 0 ) {
		GV_NearTimePV( &work->interp_adj, &DG_ZeroSVector, work->interp / TIME_BASE, 3 ) ;
		SetRotAdjust( work, &work->interp_adj, HUMAN21_KOSHI ) ;
		work->rot_adjusts[ HUMAN21_KOSHI ] = work->interp_adj ;
		//DumpSVec( &work->rot_adjusts[ HUMAN21_KOSHI ] ) ;
		if ( work->interp > 0 ) {
			work->interp -= TIME_BASE ;
			if ( work->interp < 0 ) work->interp = 0 ;
		}
	}

	ExprRotAdjust( work ) ;
	GM_ActMotion( &work->body ) ;
	GM_ActObject2( &work->body ) ;
	ResetRotAdjust( work ) ;

	DG_COPY_VEC( &diff, ( FVECTOR * )work->body.objs->objs[ HUMAN21_KUBI ].world.m[ 3 ] ) ;
	_sceVu0SubVector( &diff, &nextMov, &diff ) ;
	AdjustObject( work, &diff ) ;

	//HZX_ViewMatrix( &work->body.objs->world, 750.0F ) ;
	
	GV_SetVec3( &defRot, 0, GM_PlayerControl->rot.vy, 0 ) ;
	DG_SetPos2( &GM_PlayerControl->mov, &defRot ) ;
	DG_RotVector( &unitVec, &tmpVec2, 1 ) ;

	DG_SetPos( &nextWorld ) ;
	DG_RotVector( &unitVec, &tmpVec, 1 ) ;
	intp = _sceVu0InnerProduct( &tmpVec, &tmpVec2 ) ;

	GV_VecToRot( &tmpVec, &work->control.rot ) ;
	if ( intp <= 0 ) {
		work->control.rot.vx = -work->control.rot.vx + 2048 ;
		work->control.rot.vy += 2048 ;
	}

	work->control.turn = work->control.rot ;
	DG_COPY_VEC( &work->control.mov, ( FVECTOR * )work->body.objs->world.m[ 3 ] ) ;

	LegIK( work ) ;
	CrossCheck( work ) ;
}

/* マトリクスリセット */
static	void	ResetWorld( Work *work, int interp )
{
	SVECTOR		adj ;
	FVECTOR		quat, *old_abs_rots ;
	int			i ;

	if ( PL_EE_TurnEnd == 1 ) {
		printf( "turn end reset\n" ) ;
		work->control.rot = work->control.turn = GM_PlayerControl->rot ;
		PL_EE_TurnEnd = 0 ;
	}

	adj.vx = GV_DiffDirS( work->control.rot.vx, GM_PlayerControl->rot.vx ) ;
	//adj.vy = GV_DiffDirS( work->control.rot.vy, GM_PlayerControl->rot.vy ) ;
	adj.vy = 0 ;
	adj.vz = 0 ;

	DG_SetPos( &work->body.objs->world ) ;
	DG_RotatePos( &adj ) ;
	DG_GetPos( &work->body.objs->world ) ;

	adj.vx *= -1 ;
	adj.vy *= -1 ;

	GM_RotToQuatXAfterY( &adj, &quat ) ;
	old_abs_rots = work->body.m_ctrl->old_abs_rots ;
	for ( i = 0; i < 21; i ++, old_abs_rots ++ ) {
		MT_QuatMul( old_abs_rots, &quat, old_abs_rots ) ;
	}
	work->interp_adj = adj ;
	work->interp = interp * NTSC_TIME_BASE ;
}

#if 0
static	void	ResetWorld( Work *work, short from, short to, float *interp )
{
	SVECTOR		adj ;

	if ( *interp < ( float )TIME_BASE ) return ;

	from = GV_NearPhase( from, to ) ;
	adj.vx = from + ( float )( to - from ) / ( *interp / ( float )TIME_BASE ) ;
	adj.vy = adj.vz = 0 ;
	printf( "%d adj\n", adj.vx ) ;
	SetRotAdjust( work, &adj, HUMAN21_KOSHI ) ;
	work->rot_adjusts[ HUMAN21_KOSHI ] = adj ;
	*interp = *interp - ( float )TIME_BASE ;
}
#endif
 
/* CONTROL->rotを頭の方向に */
static	void	HeadRot( Work *work )
{
	FVECTOR		p1, p2 ;
	int			dir ;

	GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &p1 ) ;
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_KOSHI ].world, &p2 ) ;
	dir = GV_VecDir2FromTo( &p2, &p1 ) ;
	work->control.rot.vy = work->control.turn.vy = dir ;
}

/* 位置調整 */
static	void	AdjustMovMode( Work *work, float x, float y, float z, int mode ) 
{
	FVECTOR		pos ;

	GV_SetVec3( &pos, x, y, z ) ;
	DG_SetPos( &GM_PlayerBody->objs->world ) ;
	DG_PutVector( &pos, &pos, 1 ) ;
	if ( mode == 0 ) GV_NearExp2VF( &work->control.mov, &pos, 3 ) ;
	else {
		DG_COPY_VEC( &work->control.mov, &pos ) ;
		_sceVu0SubVector( &work->rai_shift, &pos, ( FVECTOR * )GM_PlayerBody->objs->world.m[ 3 ] ) ;
	}
}

#define	AdjustMov( _w, _x, _y, _z ) 	AdjustMovMode( _w, _x, _y, _z, 0 )

static	void	Bubble( Work *work, int time )
{
#if 0//BP_MATH - bubbles keep pointer to this FVECTOR so we need to make it global
	FVECTOR		shift = { 0.0F, 0.0F, 100.0F } ;
#else
   static FVECTOR	shift;
   BP_Vec4_Set( &shift, 0.0f, 0.0f, 100.0f, 1.0f );
#endif

	if ( time < 0 || ( GV_Time % time == 0 ) ) {
		DG_SetPos( &work->body.objs->objs[ HUMAN21_ATAMA ].world ) ;
		DG_PutVector( &shift, &shift, 1 ) ;
		NewBubbleMany( &shift, 60, 0 ) ;
	}
}

static	void	BodyBubble( Work *work, int obj )
{
	if ( !( work->body.objs->flag & DG_FLAG_INVISIBLE0 ) ) 
   {
      ASSERT( obj < work->body.objs->n_models );   //BP_MATH - check valid
		NewBubbleMany( ( FVECTOR * )work->body.objs->objs[ obj ].world.m[ 3 ], 60, 0 ) ;
	}
}

static	void	DeadBubble( Work *work )
{
	FVECTOR		shift = { 0.0F, 0.0F, 100.0F } ;
	FVECTOR		force = { 0.0F, 0.0F, 15.0F } ;

	DG_SetPos( &work->body.objs->objs[ HUMAN21_ATAMA ].world ) ;
	DG_RotVector( &force, &force, 1 ) ;
	DG_PutVector( &shift, &shift, 1 ) ;
	NewDrowingBubble( GV_StrCode("溺れた時の泡"), &shift, &force, 
					  10.f, 50.f, 120, 160, 0x50505530 );
}

/* 天井床幅チェック */
static	int		CeilToFloor( Work *work )
{
	float		levels[ 2 ] ;
	int			flag ;

	flag = HZX_LevelHazardCheck( HZX_CurrentGroupID, &work->control.mov, 
								 HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( flag == 0 ) return 0 ;
	HZX_GetLevelHeight( levels ) ;
	if ( !( flag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( !( flag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;	
	if ( levels[ 1 ] - levels[ 0 ] < 2000.0F ) {
		return 1 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------*/

/* モード関数 */

/* 立ちくっつき */
static	void	StandStill( Work *work, int time )
{
	float		t ;

	if ( time == 0 ) {
		if ( work->flag & EE_FLAG_WTDOOR ) {
			SetAction( work, EMstill, 0 ) ;
		} else {
			SetAction( work, EMstill, 24 ) ;
			ResetWorld( work, 24 ) ;
		}
	}
	work->flag |= EE_FLAG_NO_ACTCONTROL | EE_FLAG_NO_ACTOBJECT |
		          EE_FLAG_NO_ACTMOTION ;
	work->flag |= EE_FLAG_STAND ;
	if ( work->time < 32 * TIME_BASE || 
		 !GM_CheckPlayerStatus( PLAYER_WATER_SURFACE ) ) {
		t = 0.10F ;
	} else {
		t = 0.50F ;
	}
	if ( ( work->flag & EE_FLAG_WTDOOR ) ) {
		SetPosNormal( work, 0, 0, 1.0F, 1.0F ) ;
	} else if ( !GM_CheckPlayerStatus( PLAYER_IN_THE_WATER ) ) {
		SetPosNormal( work, 0, 0, 1.0F, 1.0F ) ;
	} else {
		SetPosNormal( work, 0, 0, 0.20F, t ) ;
	}
}

/* 立ちから横 */
static	void	Stand2Lie( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, EMstill, 24 ) ;
	}

	if ( PL_EE_Flag & PL_EE_FLAG_STAND2LIE_NO_INTERP ) {
		work->data = 1 ;
	} else {
		work->data = 0 ;
	}

	work->flag |= EE_FLAG_NO_ACTCONTROL | EE_FLAG_NO_ACTOBJECT |
		          EE_FLAG_NO_ACTMOTION ;
	work->flag |= EE_FLAG_NO_HEADIK ;
	if ( work->data == 0 ) {
		SetPosNormal( work, 0, 3, 0.10F, 0.50F ) ;
	} else {
		SetPosNormal( work, 0, 3, 0.90F, 1.0F ) ;
	}
}

/* 横くっつき */
static	void	LieStill( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, EMdive_still, 24 ) ;
		ResetWorld( work, 24 ) ;
	}
	work->flag |= EE_FLAG_NO_ACTCONTROL | EE_FLAG_NO_ACTOBJECT |
		          EE_FLAG_NO_ACTMOTION ;

	if ( PL_EE_Flag & PL_EE_FLAG_STAND2LIE_NO_INTERP ) {
		SetPosNormal( work, 3, 0, 0.90F, 0.90F ) ;
	} else if ( DiveSpeed >= DIVE_SPEED_MIDDLE ) {
		SetPosNormal( work, 1, 7, 0.20F, 0.50F ) ;
	} else {
		SetPosNormal( work, 1, 0, 0.20F, 0.50F ) ;
	}
}

/* 横から立ち */
static	void	Lie2Stand( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, EMdive_still, 24 ) ;
	}

	work->flag |= EE_FLAG_NO_ACTCONTROL | EE_FLAG_NO_ACTOBJECT |
		          EE_FLAG_NO_ACTMOTION ;
	work->flag |= EE_FLAG_NO_HEADIK ;
	SetPosNormal( work, 1, 5, 0.02F, 0.50F ) ;
}

/* ターン */
static	void	Turn( Work *work, int time )
{
	if ( time == 0 ) {
		if ( work->motion == EMstill ) {
			work->control.rot.vx += 1024 - 256 ;	/* 適当に調整 */
			work->data = 1 ;
		}
		SetAction( work, EMdive_still, 24 ) ;
		ResetWorld( work, 24 ) ;
	}
	work->flag |= EE_FLAG_NO_ACTCONTROL | EE_FLAG_NO_ACTOBJECT |
		          EE_FLAG_NO_ACTMOTION | EE_FLAG_NO_MOVEIK ;
	work->flag |= EE_FLAG_NO_HEADIK ;
	if ( work->data == 1 && time < TIME_BASE * 32 ) {
		SetPosNormal( work, 1, 6, 0.50F, 0.10F ) ;
	} else {
		SetPosNormal( work, 1, 6, 0.50F, 0.50F ) ;
	}
}

/* 立ちダメージ */
static	void	DamStand( Work *work, int time )
{
	work->control.rot = work->control.turn = GM_PlayerControl->rot ;
	if ( time == 0 ) {
		if ( GV_Time & 1 ) {
			GM_SeSetMode( SD_V_EMADMW01, &work->control.mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_V_EMADMW02, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
		SetAction( work, EMrai_dam_stand, 2 ) ;
		ResetWorld( work, 2 ) ;
		/* 位置調整 */
		AdjustMovMode( work, 0.0F, 202.140F, -217.604F, 1 ) ;
	}

	work->flag |= EE_FLAG_STAND ;
	work->flag |= EE_FLAG_NO_HEADIK | EE_FLAG_RAI_SHIFT_STEP ;
	if ( GM_IsGameOver() && !GM_CheckPlayerStatus( PLAYER_DEAD ) ) {
		work->flag |= EE_FLAG_NO_MODE_CHANGE ;
		if ( EndMotion( work ) ) {
			SetMode( work, EEDeadStand ) ;
		}
	}
}

/* 横ダメージ */
static	void	DamLie( Work *work, int time )
{
	work->control.rot = work->control.turn = GM_PlayerControl->rot ;
	if ( time == 0 ) {
		if ( GV_Time & 1 ) {
			GM_SeSetMode( SD_V_EMADMW01, &work->control.mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_V_EMADMW02, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
		SetAction( work, EMrai_dam_lie, 2 ) ;
		ResetWorld( work, 2 ) ;
		/* 位置調整 */
		AdjustMovMode( work, 0.0F, 229.960F, 73.909F, 1 ) ;
	}

	work->flag |= EE_FLAG_NO_HEADIK | EE_FLAG_RAI_SHIFT_STEP ;

	if ( GM_IsGameOver() && !GM_CheckPlayerStatus( PLAYER_DEAD ) ) {
		work->flag |= EE_FLAG_NO_MODE_CHANGE ;
		if ( EndMotion( work ) ) {
			SetMode( work, EEDeadLie ) ;
		}
	}
}

/* ライデン死立ち */
static	void	DeadStand( Work *work, int time )
{
	work->control.rot = work->control.turn = GM_PlayerControl->rot ;
	if ( time == 0 ) {
		SetAction( work, EMrai_out_stand, 2 ) ;
		ResetWorld( work, 2 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		/* 位置調整 */
		AdjustMovMode( work, 83.710F, 60.493F, -266.806F, 1 ) ;
	}
	work->flag |= EE_FLAG_STAND ;
	work->flag |= EE_FLAG_NO_HEADIK | EE_FLAG_RAI_SHIFT_STEP ;
}

/* ライデン死横 */
static	void	DeadLie( Work *work, int time )
{
	work->control.rot = work->control.turn = GM_PlayerControl->rot ;
	if ( time == 0 ) {
		SetAction( work, EMrai_out_lie, 2 ) ;
		ResetWorld( work, 2 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		/* 位置調整 */
		AdjustMovMode( work, -11.355F, 229.960F, 73.909F, 1 ) ;
	}
	work->flag |= EE_FLAG_NO_HEADIK | EE_FLAG_RAI_SHIFT_STEP ;
}

/* ライデンＯ２切れ立ち */
static	void	NoAirStand( Work *work, int time )
{
	work->control.rot = work->control.turn = GM_PlayerControl->rot ;
	if ( time == 0 ) {
		SetAction( work, EMrai_noair_stand, 2 ) ;
		ResetWorld( work, 2 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		/* 位置調整 */
		AdjustMovMode( work, 83.710F, 60.493F, -266.806F, 1 ) ;
	}
	work->flag |= EE_FLAG_STAND ;
	work->flag |= EE_FLAG_NO_HEADIK | EE_FLAG_RAI_SHIFT_STEP ;
}

/* ライデンＯ２切れ横 */
static	void	NoAirLie( Work *work, int time )
{
	work->control.rot = work->control.turn = GM_PlayerControl->rot ;
	if ( time == 0 ) {
		SetAction( work, EMrai_noair_lie, 2 ) ;
		ResetWorld( work, 2 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		/* 位置調整 */
		AdjustMovMode( work, -11.355F, 229.960F, 73.909F, 1 ) ;
	}
	work->flag |= EE_FLAG_NO_HEADIK | EE_FLAG_RAI_SHIFT_STEP ;
}

/* ライデン死静止 */
static	void	DeadIdle( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, EMrai_out_idle, 4 ) ;
		ResetWorld( work, 4 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	}
	work->flag |= EE_FLAG_NO_ACTCONTROL | EE_FLAG_NO_ACTOBJECT |
		          EE_FLAG_NO_ACTMOTION ;
	SetPosNormal( work, 2, 0, 0.50F, 0.10F ) ;

	work->flag |= EE_FLAG_NO_HEADIK ;

}

/* エマ死に立ち */
static	void	EEDeadStand( Work *work, int time )
{
	if ( time == 0 ) {
#if 0
		if ( work->o2.value <= 0 ) {
			GM_SeSetMode( SD_V_EMAGIV01, &work->control.mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_V_EMAOUW01, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
#else
		GM_SeSetMode( SD_V_EMAOUW01, &work->control.mov, GM_SEMODE_BOMB ) ;
#endif
		SetAction( work, EMout_stand, 24 ) ;
		ResetWorld( work, 24 ) ;
		HeadRot( work ) ;
		work->control.turn.vx = 0 ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;

		DeadBubble( work ) ;
	}

	if ( CeilToFloor( work ) ) {
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
	} else {
		work->control.seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
	}

	work->flag |= EE_FLAG_STAND ;
	work->flag |= EE_FLAG_NO_MODE_CHANGE ;
	work->flag |= EE_FLAG_NO_HEADIK ;

	if ( work->control.mov.vy > GM_WaterLevel - 150.0F ) {
		FVECTOR			diff ;

		work->control.mov.vy = GM_WaterLevel - 150.0F ;
		diff.vx = diff.vz = 0.0F ;
		diff.vy = ( GM_WaterLevel - 150.0F ) - work->control.mov.vy ;
		AdjustObject( work, &diff ) ;
	}
	
	if ( time > 0 ) {
		DiveHeightAdjust( work ) ;
		AvoidSink( work ) ;
	}
	PushPlayer( work ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, EEDeadIdle ) ;
		work->data = 1 ;
	}
	PL_WaterCamControl = &work->control ;
}	

/* エマ死に横 */
static	void	EEDeadLie( Work *work, int time )
{
	if ( time == 0 ) {
#if 0
		if ( work->o2.value <= 0 ) {
			GM_SeSetMode( SD_V_EMAGIV01, &work->control.mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_V_EMAOUW01, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
#else
		GM_SeSetMode( SD_V_EMAOUW01, &work->control.mov, GM_SEMODE_BOMB ) ;
#endif
		SetAction( work, EMout_lie, 24 ) ;
		ResetWorld( work, 24 ) ;
		HeadRot( work ) ;
		work->control.turn.vx = 0 ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		DeadBubble( work ) ;
	}

	work->flag |= EE_FLAG_NO_MODE_CHANGE ;	
	work->flag |= EE_FLAG_NO_HEADIK ;

	if ( work->control.mov.vy > GM_WaterLevel - 150.0F ) {
		FVECTOR			diff ;

		work->control.mov.vy = GM_WaterLevel - 150.0F ;
		diff.vx = diff.vz = 0.0F ;
		diff.vy = ( GM_WaterLevel - 150.0F ) - work->control.mov.vy ;
		AdjustObject( work, &diff ) ;
	}

	if ( time > 0 ) {
		DiveHeightAdjust( work ) ;
		AvoidSink( work ) ;
	}
	PushPlayer( work ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, EEDeadIdle ) ;
		work->data = 1 ;
	}
	PL_WaterCamControl = &work->control ;
}

/* エマ死に静止 */
static	void	EEDeadIdle( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, EMout_idle, 2 ) ;
		if ( work->data == 0 ) HeadRot( work ) ;
		work->data = 0 ;
		work->control.turn.vx = 0 ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.seg_flag |= HZX_SEG_NO_HARITSUKI ;
		if ( work->procs[ EE_PROC_DEAD ] > 0 ) {
			GM_ExecProc( work->procs[ EE_PROC_DEAD ], NULL ) ;
		}
	}
	work->flag |= EE_FLAG_NO_MODE_CHANGE | EE_FLAG_NO_MOTION_STEP_Y ;
	work->flag |= EE_FLAG_NO_HEADIK ;

	if ( work->control.mov.vy > GM_WaterLevel - 150.0F ) {
		FVECTOR			diff ;

		work->control.mov.vy = GM_WaterLevel - 150.0F ;
		diff.vx = diff.vz = 0.0F ;
		diff.vy = ( GM_WaterLevel - 150.0F ) - work->control.mov.vy ;
		AdjustObject( work, &diff ) ;
	} else {
		work->control.step.vy = 4.0F ;
	}

	if ( time > 0 ) {
		DiveHeightAdjust( work ) ;
		AvoidSink( work ) ;
	}
	PushPlayer( work ) ;

	if ( work->data == 0 && EndMotion( work ) ) {
		work->body.m_ctrl->mt3_ctrl[ 0 ].flag |= MT3_SLEEP ;
		GM_GameOverProcEnd( work ) ;
		work->data = 1 ;
	}
	PL_WaterCamControl = &work->control ;
}

/* モーションファイルチェンジ */
static	void	ChangeMotionArc( Work *work, int mar )
{
	if ( mar == work->current_mar ) return ;
	work->body.m_ctrl->motion_arc = MT_GetMotionArchives( mar ) ;
	MT_FreeSequence( work->body.m_ctrl->sar_ctrl ) ;
	work->body.m_ctrl->sar_ctrl = MT_InitSequence( work->body.m_ctrl->n_layer, mar, 0 ) ;
	ASSERT( work->body.m_ctrl->sar_ctrl == NULL ||
		    work->body.m_ctrl->sar_ctrl->header != NULL ) ;	
	work->motion = -1 ;
	work->current_mar = mar ;	
}

/* 強制モーション */
static	void	ForceAct( Work *work, int time )
{
	FORCE_ACT_CONTROL	*fac ;

	fac = &work->fa_ctrl ;
	if ( time == 0 ) {
#if 0
		ChangeMotionArc( work, work->forceact_mar ) ;
		SetAction( work, fac->motion, 0 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ;
		ExecProc( fac->start_proc ) ;
		work->flag |= EE_FLAG_NO_WAIST_INTERP | EE_FLAG_NO_MOTION_STEP_Y ;
#endif
	}

	work->flag |= EE_FLAG_NO_HEADIK ;

	if ( EndMotion( work ) ) {
		ExecProc( fac->end_proc ) ;
		if ( fac->end_mode == 0 ) {
			SetMode( work, StandStill ) ;
			work->mode = EE_MODE_STAND ;
			PL_EE_Mode = EE_MODE_STAND ;
			//work->flag |= EE_FLAG_WTDOOR | EE_FLAG_RETURN_MOTION_ARC ;
			work->flag |= EE_FLAG_RETURN_MOTION_ARC ;
		} else {
			DG_InvisibleObjs( work->body.objs ) ;
			GV_WaitMessage( &work->actor, work->control.name ) ;
			work->mode = EE_MODE_OFF ;
		}
		work->control.skip_flag &= ~( CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ) ;
		PL_EE_Mode = work->mode ;
	}
}

/*----------------------------------------------------------------*/

static	void	CheckMessage( Work *work )
{
	int					n_msg, code ;
	GV_MSG				*msg ;
	FVECTOR				mov ;
	int					len, *mesg_buf ;
	FORCE_ACT_CONTROL	*fac ;

	n_msg = GV_ReceiveMessage( work->control.name, &msg ) ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
		case EE_MESG_ON :
			DG_VisibleObjs( work->body.objs ) ;
			if ( work->mode == EE_MODE_OFF ) {
				PL_EE_LifeAdjust = 0 ;
				PL_EE_Damage = 0 ;
				PL_EE_O2Damage = 0 ;
			}
			break ;
		case EE_MESG_OFF :
			DG_InvisibleObjs( work->body.objs ) ;
			GM_InvisibleGage( &work->life ) ;
			GM_InvisibleGage( &work->o2 ) ;
			GV_WaitMessage( &work->actor, work->control.name ) ;
			work->mode = EE_MODE_OFF ;
			work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
			PL_EE_Exist = 0 ;

			PL_EE_LifeAdjust = 0 ;
			PL_EE_Damage = 0 ;
			PL_EE_O2Damage = 0 ;

			break ;
		case EE_MESG_POSITION :
			if ( GM_IsGameOver() ) break ;

			if ( msg->message[ 1 ] >= 0 ) {
				work->control.rot.vy = work->control.turn.vy = msg->message[ 1 ] ;
			}
			len = msg->message_len - 2 ;
			if ( len <= 0 ) break ;
			mesg_buf = &msg->message[ 2 ] ;
			DG_COPY_VEC( &mov, &work->control.mov ) ;
			while( len > 0 ) {
				if ( mesg_buf[ 0 ] == 0 ) {
					mov.vx = mesg_buf[ 1 ] ;
				} else if ( mesg_buf[ 0 ] == 1 ) {
					mov.vy = mesg_buf[ 1 ] ;
				} else if ( mesg_buf[ 0 ] == 2 ) {
					mov.vz = mesg_buf[ 1 ] ;
				}
				len -= 2 ; mesg_buf += 2 ;
			}
			GM_ResetControlPosition( &work->control, &mov ) ;
			work->flag |= EE_FLAG_NO_WAIST_INTERP | EE_FLAG_NO_MOTION_STEP_Y ;

			PL_EE_LifeAdjust = 0 ;
			PL_EE_Damage = 0 ;
			PL_EE_O2Damage = 0 ;

			break ;
		case EE_MESG_MOTION :
			if ( GM_IsGameOver() ) break ;
			fac = &work->fa_ctrl ;
			fac->motion = msg->message[ 1 ] ;
			fac->end_mode = msg->message[ 2 ] ;
			fac->start_proc = msg->message[ 3 ] ;
			fac->end_proc = msg->message[ 4 ] ;
			fac->flag = msg->message[ 5 ] ;

			if ( fac->end_mode == 0 ) {
				/* 終了後立ちのとき */
				NewPadVibration2( GV_StrCode( "ema_rai_onbu_start" ), 0 ) ;
			} else {
				/* 終了後ＯＦＦのとき */
				NewPadVibration2( GV_StrCode( "ema_rai_onbu_end" ), 0 ) ;
			}
			/* ここでモーションセット */
			ChangeMotionArc( work, work->forceact_mar ) ;
			SetAction( work, fac->motion, 0 ) ;
			work->control.skip_flag |= CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ;
			work->control.turn.vx = work->control.rot.vx = 0 ;
			ExecProc( fac->start_proc ) ;
			work->flag |= EE_FLAG_NO_WAIST_INTERP | EE_FLAG_NO_MOTION_STEP_Y ;
			SetMode( work, ForceAct ) ;

			work->mode = EE_MODE_FORCEACT ;
			PL_EE_Mode = EE_MODE_FORCEACT ;
			DG_VisibleObjs( work->body.objs ) ;	/* on にする */

			PL_EE_LifeAdjust = 0 ;
			PL_EE_Damage = 0 ;
			PL_EE_O2Damage = 0 ;

			break ;
		case EE_MESG_WTDOOR :
		case EE_MESG_WATERINOUT_STAIR :
			if ( GM_IsGameOver() ) break ;
			/* ITEM_DISABLEかどうかで、プレイヤーの状態を判断 */
			if ( !GM_CheckPlayerStatus( PLAYER_ITEM_DISABLE ) ) break ;
			work->flag |= EE_FLAG_WTDOOR ;
			if ( work->action == StandStill ) break ;
			SetMode( work, StandStill ) ;
			work->mode = EE_MODE_STAND ;
			PL_EE_Mode = EE_MODE_STAND ;

			PL_EE_LifeAdjust = 0 ;
			PL_EE_Damage = 0 ;
			PL_EE_O2Damage = 0 ;

			break ;
		default :
		  ;
		}
		msg ++ ;
	}
}

/*----------------------------------------------------------------*/

static	void	Action( Work *work )
{
	int			time ;
	void		( *action )( Work *, int ) ;

	work->flag &= ~EE_FLAG_RESET_ALL ;

	time = work->time ;
	work->time += TIME_BASE ;
	work->ftime = work->ftime_count ;
	work->ftime_count ++ ;
	action = work->action ;
	if ( action != NULL ) ( *action )( work, time ) ;

	if ( PL_EE_Mode != EE_MODE_FORCEACT &&
		 PL_EE_Mode != work->mode &&
		 !( work->flag & EE_FLAG_NO_MODE_CHANGE ) &&
		 ModeAction[ PL_EE_Mode ] != NULL ) {
		work->prev_mode = work->mode ;
		work->mode = PL_EE_Mode ;
		work->flag &= ~EE_FLAG_WTDOOR ;	/* 水密ドアフラグリセット */
		SetMode( work, ModeAction[ work->mode ] ) ;
	}

	PL_EE_Flag = PL_EE_FLAG_NOTHING ;

	/* 腕ＩＫ */
	ArmIK( work ) ;
	/* 頭ＩＫ */
	HeadIK( work ) ;

	/* 泡 */
	{
		if ( !GM_CheckPlayerStatus( PLAYER_WATCH ) && 
			 work->o2.value > 0 ) {	
			Bubble( work, 90 ) ;
		}
		if ( ( GV_Time % DIRECT_TICK( 90 ) ) == 0 ) {
			BodyBubble( work, irnd() % 21 ) ;
		}
		if ( ( GV_Time % DIRECT_TICK( 110 ) ) == 0 ) {
			BodyBubble( work, irnd() % 21 ) ;
		}
	}

	/* 表示／非表示 */
	if ( work->mode != EE_MODE_OFF && GM_PlayerBody != NULL ) {
		int		vflag ;

		vflag = ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE ) ;
		work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->body.objs->flag |= vflag ;
	}
	/* カメラが水中から出たらＳＥ */
	if ( GM_GetCurrentCamera( 0 )->position.vy > GM_WaterLevel ) {
		if ( work->flag & EE_FLAG_CAMERA_IN_WATER ) {
			if ( work->o2.value < work->o2.max / 3 ) {
				if ( work->o2.value > 0 ) {
					GM_SeSetMode( SD_V_EMABTH02, 
								  &GM_GetCurrentCamera( 0 )->position, GM_SEMODE_BOMB ) ;
				}
			} else {
				GM_SeSetMode( SD_V_EMABTH01, 
							  &GM_GetCurrentCamera( 0 )->position, GM_SEMODE_BOMB ) ;
			}
		}
		work->flag &= ~EE_FLAG_CAMERA_IN_WATER ;
	} else {
		work->flag |= EE_FLAG_CAMERA_IN_WATER ;
	}

	/* 振動 */
	if ( ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_WATERMINE_NEAR ) ||
		   GM_CheckInsideTrap( GM_PlayerControl, &PeterTrap, 1, 0 ) ) &&
		work->heart_beat_decay > DIRECT_TICK( 30 ) ) {
		/* 機雷かピーターに近い */
		work->heart_beat_decay = DIRECT_TICK( 30 ) ;
		work->heart_beat_decay_changed = DIRECT_TICK( 90 ) ;
	}
	if ( work->mode != EE_MODE_OFF && 
		 work->mode != EE_MODE_FORCEACT &&
		 !GM_CheckPlayerStatus( PLAYER_FORCE ) &&
		 PL_GetPlayerItem() != IT_VibSensor &&
		 ( work->o2.value > 0 || work->life.value > 0 ) ) {
		static	u_char	DamageVib1L[] = { 132, 6, 0, 0 } ;
		extern 	void	NewPadVibration( u_char *, int ) ;   

		if ( work->heart_beat_decay_changed > 0 ) {
			-- work->heart_beat_decay_changed ;
		} else {
			work->heart_beat_decay = GV_NearExp16P( work->heart_beat_decay, DIRECT_TICK( 40 ) ) ;
		}
		if ( GV_Time - work->last_heart_beat_time > work->heart_beat_decay ) {
			NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;
			work->last_heart_beat_time = GV_Time ;
		}
	}
}

/* ライフ更新 */
static	void	UpdateLife( Work *work )
{
	if ( work->mode == EE_MODE_OFF ) {
		return ;
	}

	//if ( GV_PadData->press & PAD_L1 ) PL_EE_LifeAdjust = 255 ;

	if ( !GM_IsGameOver() ) {
		int			plife ;

		plife = work->life.value ;
		work->life.value -= PL_EE_Damage ;
		work->life.value -= PL_EE_LifeAdjust ;
#ifndef O2_NODAMAGE 
		if ( PL_EE_O2Damage > 0 ) work->o2.value -= PL_EE_O2Damage ;
#endif
		if ( work->life.value < work->life.min ) {
			work->life.value = work->life.min ;
		}
		if ( work->o2.value < work->o2.min ) {
			work->o2.value = work->o2.min ;
		}
		if ( plife > work->life.value || 
			 work->o2.value < work->o2.max / 3 ) {
			work->heart_beat_decay_changed = DIRECT_TICK( 120 ) ;
			work->heart_beat_decay = DIRECT_TICK( 20 ) ;
		}
		if ( work->life.value <= work->life.min ) {
			/* エマ死亡 */
			GM_GameOverProcStart( work ) ;
			if ( PL_EE_Damage == 0 ) {
				/* Ｏ２切れ時のみここを通るはず */
				if ( work->flag & EE_FLAG_STAND ) SetMode( work, EEDeadStand ) ;
				else							  SetMode( work, EEDeadLie ) ;
			}
		}
		//printf( "%d %d %d\n", work->life.value, work->life.prev, work->life.dec ) ;
	}
	PL_EE_LifeAdjust = 0 ;
	PL_EE_Damage = 0 ;
	PL_EE_O2Damage = 0 ;
}

/* ゲージ */
static	void	GageAct( Work *work )
{
	if ( work->mode == EE_MODE_OFF ||
		 GM_IsGameOver() ) {
		GM_InvisibleGage( &work->life ) ;
		GM_InvisibleGage( &work->o2 ) ;
	} else {
		GM_GageSet		*gs ;
		float			value ;

		GM_VisibleGage( &work->life ) ;
		gs = &work->o2 ;
		if ( GM_CheckPlayerStatus( PLAYER_NO_BREATH ) ||
			 GM_WaterLevel > GM_PlayerBody->objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ) {
			if ( !GM_CheckPlayerStatus( PLAYER_INVINCIBLE_ALL ) ) {
				GM_VisibleGage( gs ) ;
				gs->flag &= ~GM_GAGE_WARNING ;
				work->o2_gage_invisible_count = 0 ;
				value = work->o2value ;
				
				value -= 1.0F * ( float )TIME_BASE / 5.0F ;
				if ( work->life.value < work->life.max ) value -= 1.0F * ( float )TIME_BASE / 5.0F ;
				if ( value < ( float )gs->min ) value = ( float )gs->min ;
				gs->value = ( int )value ;
				work->o2value = value ;
				if ( gs->value == gs->min ) {
					/* Ｏ２無し、ライフを減らす */
					if ( GV_Time % DIRECT_TICK( 15 ) == 0 ) PL_EE_LifeAdjust += 1 ;
					if ( ( ( GV_Time - DIRECT_TICK( 45 ) ) % DIRECT_TICK( 90 ) ) == 0 ) {
						/* プレイヤーのタイミングとちょっとずらす */
						GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
					}
				} else if ( gs->value < gs->max / 3 ) {
					/* １／３以下 */
					if ( ( ( GV_Time - DIRECT_TICK( 45 ) ) % DIRECT_TICK( 90 ) ) == 0 ) {
						/* プレイヤーのタイミングとちょっとずらす */
						GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
						gs->flag |= GM_GAGE_WARNING ;
					}
				}
			}
		} else {
			value = work->o2value ;
			if ( value < ( float )gs->max ) {
				value += 24.0F * ( ( float )TIME_BASE / 5.0F ) ; 
			} else {
				value = ( float )gs->max ;
				if ( work->o2_gage_invisible_count < ( 300 / TIME_BASE ) * 4 ) {
					if ( ++ work->o2_gage_invisible_count >= ( 300 / TIME_BASE ) * 4 ) {
						GM_InvisibleGage( gs ) ;
					}
				}
			}
			gs->value = ( int )value ;
			work->o2value = value ;
			gs->flag &= ~GM_GAGE_WARNING ;
		}
#ifdef O2_NODAMAGE 
		work->o2value = ( float )gs->max ;
#endif
		//printf( "[%d] %d %f\n", GV_Time, gs->value, work->o2value ) ;
	}
}

static	void		WaterCameraOutofEE( Work *work ) 
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
							    HZX_CHK_ALL, HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
		HZX_GetOnlinePoint( &to1 ) ;
		HZX_GetOnlineVector( &v ) ;
		len1 = GV_VecLen3F( &v ) ;
		if ( len1 > 10.0F ) {
			GV_LenVec3F( &v, &v, 0.0F, len1 - 10.0F ) ;
			_sceVu0AddVector( &to1, &from, &v ) ;
		}
	} else {
		//DG_COPY_VEC( &cam->position, &to1 ) ;
		GV_NearExp8VF( &cam->position, &to1, 3 ) ;
		return ;
	}

	if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &from, &to2,
							    HZX_CHK_ALL, HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
		HZX_GetOnlinePoint( &to2 ) ;
		HZX_GetOnlineVector( &v ) ;
		len2 = GV_VecLen3F( &v ) ;
		if ( len2 > 10.0F ) {
			GV_LenVec3F( &v, &v, 0.0F, len2 - 10.0F ) ;
			_sceVu0AddVector( &to2, &from, &v ) ;
		}
	} else {
		//DG_COPY_VEC( &cam->position, &to2 ) ;
		GV_NearExp8VF( &cam->position, &to2, 3 ) ;
		return ;
	}
	/* 両方壁のときは長いほう */
	if ( len1 > len2 ) {
		//DG_COPY_VEC( &cam->position, &to1 ) ;
		GV_NearExp4VF( &cam->position, &to1, 3 ) ;
	} else {
		//DG_COPY_VEC( &cam->position, &to2 ) ;
		GV_NearExp4VF( &cam->position, &to2, 3 ) ;
	}
}

static	void		LocalSendMessage( int address, int len, int v1, int v2 )
{
	GV_MSG			msg ;
	int				buf[ 2 ] ;

	msg.address = address ;
	msg.message = buf ;
	msg.message_len = len ;
	buf[ 0 ] = v1 ;
	buf[ 1 ] = v2 ;
	GV_SendMessage( &msg ) ;
}

static	void		HairModeAct( Work *work ) 
{
	int				water ;

	if ( work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] > GM_WaterLevel ) {
		water = 0 ;
	} else {
		water = 1 ;
	}
	if ( water == 1 ) {
		/* 水中 */
		work->hair_mode = HAIR_MODE_WATER ;
		if ( work->hair_mode != HAIR_MODE_WATER ) {
			LocalSendMessage( HAIR_NORMAL, 2, 0, 0 ) ;	
			LocalSendMessage( HAIR_WET, 2, 0, 1 ) ;	
			LocalSendMessage( HAIR_WET, 2, 1, 22 ) ;
		}
	} else if ( work->hair_mode == HAIR_MODE_WATER ) {
		/* ぬれ陸上 */
		work->hair_mode = HAIR_MODE_WET ;		
		LocalSendMessage( HAIR_NORMAL, 2, 0, 0 ) ;	
		LocalSendMessage( HAIR_WET, 2, 0, 1 ) ;	
		LocalSendMessage( HAIR_WET, 2, 1, 17 ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	float		pheight ;

	PL_EE_Exist = 1 ;

	PL_WaterCamControl = NULL ;

	/* 常にプレイヤーと同マップ */
	GM_SetCurrentMap( GM_PlayerMap ) ;
	work->control.map = GM_PlayerMap ;

	CheckMessage( work ) ;

	UpdateLife( work ) ;

	/* ゲージ管理 */
	GageAct( work ) ;

	UpdateIKControl( work ) ;

	pheight = work->body.height ;

	if ( !( work->flag & EE_FLAG_NO_ACTMOTION ) ) {
		GM_ActMotion( &work->body ) ;
		ResetRotAdjust( work ) ;
	}

	{
		work->rai_shift.vx += work->body.m_ctrl->step.vx - GM_PlayerBody->m_ctrl->step.vx ;
		work->rai_shift.vz += work->body.m_ctrl->step.vz - GM_PlayerBody->m_ctrl->step.vz ;
		work->rai_shift.vy += work->body.m_ctrl->step.vw - GM_PlayerBody->m_ctrl->step.vw ;
	}

	
	if ( work->flag & EE_FLAG_RETURN_MOTION_ARC ) {
		ChangeMotionArc( work, work->original_mar ) ;
		work->flag &= ~EE_FLAG_RETURN_MOTION_ARC ;
	}

	if ( !( work->flag & EE_FLAG_NO_ACTCONTROL ) ) {
		/* 腰高さ補完なし */
		if ( work->flag & EE_FLAG_NO_WAIST_INTERP ) {
			work->body.height 
				= work->body.m_ctrl->height 
					= work->body.m_ctrl->old_height 
						= work->body.m_ctrl->mt3_ctrl[ 0 ].move->step.vw ;
			work->body.m_ctrl->root_height 
				= work->body.m_ctrl->root_old_height 
					= work->body.m_ctrl->mt3_ctrl[ 0 ].root->step.vw ;
			pheight = work->body.height ;
			work->flag &= ~EE_FLAG_NO_WAIST_INTERP ;
		}
		work->control.height = work->body.height ;
		if ( !( work->flag & EE_FLAG_NO_MOTION_STEP_Y ) ) {
			work->control.step.vy = work->body.height - pheight ;
		}
		if ( work->flag & EE_FLAG_RAI_SHIFT_STEP ) {
			FVECTOR		pos ;

			_sceVu0AddVector( &pos, ( FVECTOR * )&GM_PlayerBody->objs->world.m[ 3 ], &work->rai_shift ) ;
			_sceVu0SubVector( &work->control.step, &pos, &work->control.mov ) ;
			work->flag &= ~EE_FLAG_RAI_SHIFT_STEP ;
		}
		GM_ActControl( &work->control ) ;
		//printf( "%f %f\n", work->control.step.vy, work->control.mov.vy ) ;
	}
	if ( !( work->flag & EE_FLAG_NO_ACTOBJECT ) ) {
		GM_ActObject2( &work->body ) ;
	}
	
	DG_GetLightMatrix( &work->control.mov, work->lights ) ;

	Action( work ) ;

	if ( !( work->flag & EE_FLAG_NO_ACTMOTION ) ) {
		ExprRotAdjust( work ) ;
	}
	/* 髪の毛モード制御 */
	HairModeAct( work ) ;

	/* エマからカメラを出す処理をいれてみる */
	WaterCameraOutofEE( work ) ;
}

/*----------------------------------------------------------------*/

static	void	Die( Work *work )
{
	GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_RemoveGageSet( &work->life ) ;
	GM_RemoveGageSet( &work->o2 ) ;
	PL_EE_Exist = 0 ;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	void	InitGage( Work *work )
{
	GM_GageSet	*gs ;
	int			cur, max ;

	if ( GCL_GetOption( 'l' ) != NULL ) {
		cur = GCL_GetNextInt() ;
		max = GCL_GetNextInt() ;
	} else {
		cur = 10 ;
		max = 100 ;
	}
	gs = &work->life ;
	GM_InitGageSet( gs, "EMMA", 16, max, GM_DEFAULT_GAGE_HEIGHT,
				    cur, max, 0, 30, GM_GAGE_LEVEL_NPC_LIFE ) ;
	GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_NPC_LIFE ) ;
    //GM_SetGageColor( gs, 0, 0, 0, 40, 128, 118, 110, 190, 118, 255, 0, 0 ) ;
	GM_AppendGageSet( gs ) ;
	GM_InvisibleGage( gs ) ;

	max = GCL_GetOptionValue( 'o', 3600 ) ;
	gs = &work->o2 ;
	GM_InitGageSet( gs, "EMMA O2", 16, max / 15 , GM_DEFAULT_GAGE_HEIGHT,
				    max, max, 0, 30, GM_GAGE_LEVEL_NPC_O2 ) ;
	//GM_SetGageColor( gs, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
	GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_PLAYER_O2 ) ;
	GM_AppendGageSet( gs ) ;
	GM_InvisibleGage( gs ) ;

	work->o2value = ( float )work->o2.value ;
}

static	int		GetResources( Work *work, int name, int where )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			mode ;

	ctrl = &work->control ;
	body = &work->body ;

	GM_InitControl( ctrl, name, where ) ;
//	GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlObject( ctrl, body ) ;
//	GM_ConfigControlMapCheck( ctrl ) ;
//	GM_ConfigControlHazard( ctrl, 1000, R_SPHERE, S_SPHERE ) ;
//	GM_ConfigControlHzxHeight( ctrl, NEAR_HEIGHT_STAND, StartPos.vy ) ;
	GM_ConfigControlHzxCheckFlag( ctrl, HZX_SEG_NO_PLAYER | HZX_SEG_NO_HARITSUKI, 
								  HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;	
	//ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK | CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
	ctrl->attribute |= CTRL_ATR_NPC ;
	ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;

	work->original_mar = GCL_GetOptionValue( 'T', 0 ) ;
	work->forceact_mar = GCL_GetOptionValue( 'f', work->original_mar ) ;
	work->current_mar = work->original_mar ;

	GM_InitObject( body, GCL_GetOptionValue( 'm', 0 ), OBJECT_FLAG ) ;
	GM_ConfigObjectLight( body, work->lights ) ;
	GM_ConfigObjectMotion( body, 1, work->original_mar, MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &ctrl->step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;

//	GV_SetVec3( &ctrl->mov, 9500.0F, -4000.0F, -15000.0F ) ;
	GM_ConfigControlMapID( ctrl ) ;

	work->motion = 0 ;
	work->action = NULL ;

	SetMode( work, StandStill ) ;

	mode = GCL_GetOptionValue( 's', 1 ) ;
	if ( mode == 0 ) {
		GV_WaitMessage( &work->actor, work->control.name ) ;
		DG_InvisibleObjs( body->objs ) ;
		work->mode = EE_MODE_OFF ;
	} else {
		work->mode = EE_MODE_STAND ;
	}
	work->prev_mode = work->mode ;
	PL_EE_Mode = work->mode ;
	PL_EE_Flag = PL_EE_FLAG_NOTHING ;
	PL_EE_TurnEnd = 0 ;
	PL_EE_Damage = 0 ;
	PL_EE_O2Damage = 0 ;
	PL_EE_LifeAdjust = 0 ;
	
	DG_COPY_VEC( &work->shift, &Shifts[ 0 ] ) ;

	InitGage( work ) ;

	{
		int		i ;

		for ( i = 0; i < 21; i ++ ) {
			work->adjust_interp_n[ i ] = 4 ; 
			work->adjust_interp_n_r[ i ] = 4 ; 
		}
		InitAdjustMinMax( work ) ;

		SetAdjustMinMax( work, HUMAN21_MIGI_ASHI1, -1280, -2048, -2048, 512, 2048, 2048 ) ;
		SetAdjustMinMax( work, HUMAN21_HIDARI_ASHI1, -1280, -2048, -2048, 512, 2048, 2048 ) ;
		SetAdjustMinMax( work, HUMAN21_MIGI_ASHI2, -512, -2048, -2048, 512, 2048, 2048 ) ;
		SetAdjustMinMax( work, HUMAN21_HIDARI_ASHI2, -512, -2048, -2048, 512, 2048, 2048 ) ;
		SetAdjustMinMax( work, HUMAN21_MIGI_KAKATO, -128, -2048, -2048, 640, 2048, 2048 ) ;
		SetAdjustMinMax( work, HUMAN21_HIDARI_KAKATO, -128, -2048, -2048, 640, 2048, 2048 ) ;

		SetAdjustMinMax( work, HUMAN21_ATAMA, -512, -768, -2048, 0, 768, 2048 ) ;
	}


	{
		int		i ;

		GCL_GetOption( 'p' ) ;
		i = 0 ;
		while( GCL_NextStr() != NULL ) {
			if ( i == EE_PROC_MAX ) break ;
			work->procs[ i ] = GCL_GetNextInt() ;
			i ++ ;
		}
	}

	{
		extern	void	*NewEmmaEquip( int name, OBJECT *body, int flag ) ;
		int				card ;
		/* 髪の毛 ＆ ＩＤカード */
		card = GCL_GetOptionValue( 'c', 1 ) ;
		if ( card ) GV_SetActorChild( work, NewEmmaEquip( name, body, 0x000d ) ) ;	
		else		GV_SetActorChild( work, NewEmmaEquip( name, body, 0x000c ) ) ;
	}
	work->hair_mode = HAIR_MODE_NORMAL ;

	work->heart_beat_decay = DIRECT_TICK( 40 ) ;
	work->heart_beat_decay_changed = 0 ;
	work->last_heart_beat_time = 0 ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewEESwim( int name, int where )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	PL_EE_Work = work ;
	return work ;
}

int		EE_COM_SetLife( void )
{
	GCL_GetOption( 'v' ) ;
	PL_EE_Work->life.value = GCL_GetNextInt() ;
	PL_EE_Work->life.dec = PL_EE_Work->life.value ;
	PL_EE_Work->life.prev = PL_EE_Work->life.value ;
	return 0 ;
}

int		EE_COM_GetLife( void )
{
	return PL_EE_Work->life.value ;
}

int		EE_COM_GetO2( void )
{
	return PL_EE_Work->o2.value ;
}
