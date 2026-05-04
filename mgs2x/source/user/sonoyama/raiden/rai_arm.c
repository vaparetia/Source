/*
   rai_arm.c
   主観腕
   
   1999/07/29 M.Sonoyama
   $Id: rai_arm.c,v 1.1.1.3 2002/11/19 11:51:00 Yoshizawa1 Exp $
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
#include	"rai_arm.h"
#include	"arm_motion.h"

#define	HAND_NEEDLESS

#define	CAMERA_TURN

#define	MODEL_NAME	(12660725) /* GV_StrCode( "rai_arm" ) */
#define	MOTION_NAME	MODEL_NAME
#define	OBJECT_FLAG		(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG2	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE)
#define	ONE_CLOCK	(300 / 60)

#define	RIGHT_NAME	(2800220) /* GV_StrCode( "rh_rdef" ) */
#define	LEFT_NAME	(2603612) /* GV_StrCode( "rh_ldef" ) */

#define	ARM_CAMERA	(1832632) /* GV_StrCode( "武器カメラ" ) */

#define	ARM_VISIBLE_MODE_A
//#define	ARM_VISIBLE_MODE_B

FVECTOR			ArmBodyPosition = { 0.0F, 0.0F, 0.0F } ;
FVECTOR			ArmCamShift = { 0.0F, 0.0F, 0.0F } ;
float			ArmCamRotX = 0.0F ;

/* 主観時武器カメラ設定 */
static	FVECTOR		WeaponCamShiftSnake[] = {
    { 0.0F, 500.0F, 240.0F, 0.0F },	/* 素手 */
    { -15.0F, 553.0F, 262.0F, 1.55F }, /* ベレッタ */
    { -65.0F, 542.0F, 352.0F, 0.0F }, /* ＵＳＰ */	/* 旧主観腕 */
//    { -65.0F, 545.0F, 352.0F, 0.0F }, /* ＵＳＰ */
    { -65.0F, 531.0F, 352.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 93.0F, 551.0F, -401.0F, 15.5F }, /* ＦＡＭＡＳ */
	{ -64.0F, 534.0F, 352.0F },		  	/* Ｓｐｐ１Ｍ */ 
	{ 27.0F, 634.0F, 427.0F },		  	/* スティンガー */ 
	{ 104.0F, 615.0F, 249.0F, 6.950F }	/* ＲＧＢ－６ */
} ;

static	FVECTOR		WeaponCamShiftRaiden[] = {
    { 0.0F, 500.0F, 0.0F, 0.0F },	/* 素手 */
    { 37.0F, 535.0F, 282.0F, 0.0F }, /* ベレッタ */
    { -63.0F, 532.0F, 352.0F, 0.0F }, /* ＵＳＰ */
    { -63.0F, 535.0F, 352.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 20.0F, 511.0F, 109.0F, 0.0F },	/* ＦＡＭＡＳ */
	{ -63.0F, 534.0F, 352.0F },		  	/* Ｓｐｐ１Ｍ */ 
	{ 72.0F, 584.0F, 451.0F },		  	/* スティンガー */ 
	{ 104.0F, 615.0F, 249.0F, 6.950F }	/* ＲＧＢ－６ */
} ;

/* 匍匐用の設定 */
static	FVECTOR		WeaponCamShiftSnakeG[] = {
    { 0.0F, 0.0F, 0.0F, 0.0F },	/* 素手 */
    { 35.0F, 150.0F, 430.0F, 2.75F }, /* ベレッタ */
    { -15.0F, 110.0F, 552.0F, 0.0F }, /* ＵＳＰ */	/* 旧主観腕 */
//    { -14.0F, 111.0F, 552.0F, 0.0F }, /* ＵＳＰ */
    { -14.0F, 101.0F, 552.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 100.0F, 225.0F, 473.0F, 2.95F }, /* ＦＡＭＡＳ */
	{ -13.0F, 103.0F, 552.0F },		  	/* Ｓｐｐ１Ｍ */ 
	{ 72.0F, 584.0F, 451.0F },		  	/* スティンガー */ 
	{ 0.0F, 0.0F, 0.0F }				/* ＲＧＢ－６ */
} ;

static	FVECTOR		WeaponCamShiftRaidenG[] = {
    { 0.0F, 0.0F, 0.0F, 0.0F },	/* 素手 */
    { 54.0F, 117.0F, 595.0F, 0.0F }, /* ベレッタ */
    { -13.0F, 101.0F, 595.0F, 0.0F }, /* ＵＳＰ */
    { -14.0F, 104.0F, 595.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 102.0F, 150.0F, 538.0F, 0.0F },	/* ＦＡＭＡＳ */
	{ -14.0F, 101.0F, 595.0F, 0.0F },		  	/* Ｓｐｐ１Ｍ */ 
	{ 72.0F, 584.0F, 451.0F },		  	/* スティンガー */ 
	{ 0.0F, 0.0F, 0.0F }				/* ＲＧＢ－６ */
} ;

/* 武器番号とカメラシフト位置セットの対応テーブル */
static	int	ShiftSet[] = {
	0, 1, 2, 3, 4,	
	0, 5, 7, 0, 6,
	0, 0, 0, 0, 0,	
	0, 0, 0, 1, 1,
	1, 0, 0, 0, 0
} ;

#define	MAX_SET_WEAPON	(6)

static	FVECTOR		*WeaponCamShift ;

static	ARM_MOTION_SET	AMS ;

typedef	struct	_Work {
    GV_ACT		actor ;
    OBJECT		body ;

    OBJECT		right_hand ;
    OBJECT		left_hand ;

    FVECTOR		step ;
    FVECTOR		targ ;
	FVECTOR		delay ;

    int			cur_weapon ;
    int			cur_motion ;
    int			cur_rhand ;
    int			cur_lhand ;

    int			*motion ;
    int			*rhand ;
    int			*lhand ;
    int			*trigger ;

    SVECTOR			rot ;
	int				chanl ;
	float			height_adjust ;
	int				count ;
    GM_CameraSet	*camera ;	/* 武器カメラ、起動はraiden.c */
} Work ;

/*-----------------------------------------------------------------*/

/* 武器毎にモーションをセット */
static	void	SetMotionSet( wp )
int		wp ;
{
    ASSERT( wp >= 0 && wp < MAX_WEAPONS ) ;
    AMS.change = ChangeSets[ wp ] ;
	//    MS.attack = AttackSets[ wp ] ;
    AMS.shared = SharedSet ;
}

/*-----------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    int			motion, mt ;
    int			trig, wp, rx ;
	FMATRIX		mat, adj ;
	FVECTOR		shift, pos, targ ;
	SVECTOR		rot ;
	float		height_adjust ;
	FVECTOR		delay, delay_aim ;
	GV_PAD		*pad ;

    trig = *( work->trigger ) ;

    /* 可視制御 */
    if ( trig & ARM_INVISIBLE ) {
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
    } else {
		DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
    }


    /* 終了検知 */
    if ( GM_CheckObject_IsEnd( &( work->body ), 0 ) ) {
		trig |= ARM_MOTION_IS_END ;
    } else {
		trig &= ~ARM_MOTION_IS_END ;    
    }
    /* モーション変更 */
    wp = PL_GetPlayerWeapon() ;
    if ( wp != work->cur_weapon ) {
		/* 武器変更 */
		SetMotionSet( wp ) ;
		work->cur_weapon = wp ;
		/* 必ずモーション変更する */
		work->cur_motion = -1 ;
    }
    motion = *( work->motion ) ;
    if ( work->cur_motion != motion ) {
		int	interp ;

		interp = 6 * ONE_CLOCK ;
		if ( motion >= AM_MAX_CHANGE_MOTIONS ) {
			mt = motion - AM_MAX_CHANGE_MOTIONS ;
			GM_ConfigObjectAction( &( work->body ), 0, AMS.shared[ mt ],
								  0, 0xfffff, interp ) ;
		} else {
			GM_ConfigObjectAction( &( work->body ), 0, AMS.change[ motion ],
								  0, 0xfffff, interp ) ;
		}
		work->cur_motion = motion ;
    }
	
    /* カメラ更新 */
	GV_NearExp4VF( &work->targ, &GM_CameraTarget, 3 ) ;
	GV_NearExp4PV( &work->rot, &GM_CameraDir, 3 ) ;
	if ( work->camera->on == 1 ) {
#ifdef DEBUG_MODE
		if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
			GM_PlayerDebugMode == GM_PDM_WEAPON_CAMERA ) {
			int	status ;
			
			status = GV_PadData[ 1 ].status ;
			if ( status & PAD_U ) ArmCamShift.vy += 1.0F ;
			if ( status & PAD_D ) ArmCamShift.vy -= 1.0F ;
			if ( status & PAD_L ) ArmCamShift.vx += 1.0F ;
			if ( status & PAD_R ) ArmCamShift.vx -= 1.0F ;
			if ( status & PAD_X ) ArmCamShift.vz += 1.0F ;
			if ( status & PAD_B ) ArmCamShift.vz -= 1.0F ;
			if ( status & PAD_Y ) ArmCamRotX += 0.05F ;
			if ( status & PAD_A ) ArmCamRotX -= 0.05F ;
		} else {
			/* スネーク用とライデン用が用意してある */
			if ( GM_PlayerStatus & PLAYER_GROUND ) {
				WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
					WeaponCamShiftSnakeG : WeaponCamShiftRaidenG ;
			} else {
				WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
					WeaponCamShiftSnake : WeaponCamShiftRaiden ;
			}
#if 1
			if ( trig & ARM_IS_COMBO ) {
				wp = WP_None ;	
				trig &= ~ARM_IS_COMBO ;
			}
			wp = ShiftSet[ wp ]; 
#else
			if ( wp >= MAX_SET_WEAPON ) {
				if ( wp == WP_ColdSpray ) wp = WP_m92 ;
				else if ( wp == WP_Spp1M ) wp = 5 ;
				else if ( wp == WP_Stinger ) wp = 6 ;
				else						wp = WP_Usp ;
			}
#endif			
			ArmCamShift = WeaponCamShift[ wp ] ;
			ArmCamRotX = WeaponCamShift[ wp ].vw ;
		}
#else
		/* スネーク用とライデン用が用意してある */
		if ( GM_PlayerStatus & PLAYER_GROUND ) {
			WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
				WeaponCamShiftSnakeG : WeaponCamShiftRaidenG ;
		} else {
			WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
				WeaponCamShiftSnake : WeaponCamShiftRaiden ;
		}
#if 1
		if ( trig & ARM_IS_COMBO ) {
			wp = WP_None ;	
			trig &= ~ARM_IS_COMBO ;
		}
		wp = ShiftSet[ wp ] ;
#else
		if ( wp >= MAX_SET_WEAPON ) {
			if ( wp == WP_ColdSpray ) wp = WP_m92 ;
			else					  wp = WP_Usp ;
		}
#endif
		ArmCamShift = WeaponCamShift[ wp ] ;
		ArmCamRotX = WeaponCamShift[ wp ].vw ;
#endif

		DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
		pad = GV_PadData ;
		DG_COPY_VEC( &delay_aim, &DG_ZeroVector ) ;
		if ( pad->status & PAD_UDLR ) {
			DG_COPY_VEC( &delay, &DG_ZeroVector ) ;
			if ( pad->status & PAD_R ) {
				delay.vx = -16.0F ;
			} else if ( pad->status & PAD_L ) {
				delay.vx = 16.0F ;
			}
			if ( pad->status & PAD_U ) {
				delay.vy = 16.0F ;
			} else if ( pad->status & PAD_D ) {
				delay.vy = -16.0F ;
			}			
			DG_RotVector( &delay, &delay_aim, 1 ) ;
		} 

		GV_NearExp16VF( &work->delay, &delay_aim, 3 ) ;
		DG_GetPos( &mat ) ;
		_sceVu0RotMatrixX( &adj, &DG_UnitMatrix,
						   ArmCamRotX * 3.141592653589793F / 180.0F ) ;
		_sceVu0MulMatrix( &mat, &mat, &adj ) ;
		DG_SetPos( &mat ) ;
		DG_RotVector( &ArmCamShift, &shift, 1 ) ;
		DG_COPY_VEC( &targ, &GM_CameraTarget ) ;
		_sceVu0SubVector( &pos, &work->targ, &shift ) ;
		height_adjust = GM_PlayerBody->objs->world.m[ 3 ][ 1 ] - pos.vy ;
		work->height_adjust = height_adjust 
			= GV_NearExp8F( work->height_adjust, height_adjust ) ;
		_sceVu0AddVector( &pos, &pos, &work->delay ) ;
//		pos.vy += height_adjust ;
//		targ.vy += height_adjust ;
		rot = work->rot ;
//		if ( work->camera->on == 1 ) {
			if ( -- work->count > 0 ) {
				pos.vy -= work->count * 2.0F ;
				rx = rot.vx + ( int )work->count * 2 ;
				rot.vx = GV_NearExp4P( rot.vx, rx ) ;
			}
//		} else {
//			DG_InvisibleObjs( work->body.objs ) ;
//			work->count = 16 ;
//		}
		DG_SetPos2( &pos, &rot ) ;

//		_sceVu0AddVector( &work->camera->position, &targ, &work->delay ) ;
		DG_COPY_VEC( &work->camera->position, &targ ) ;
		work->camera->rotate.vx = GM_CameraDir.vx ;
		work->camera->rotate.vy = GM_CameraDir.vy ;

		if ( PL_GetPlayerWeapon() == WP_Usp && PL_GetUSPLightOn() ) {
			GM_PlayerBody->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
		} else {
			GM_PlayerBody->objs->flag |= DG_FLAG_SHADOWMAKE ;
		}
	} else {
		DG_SetPos( &GM_PlayerBody->objs->world ) ;
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		work->count = 16 ;
		work->height_adjust = 0.0F ;
		GM_PlayerBody->objs->flag |= DG_FLAG_SHADOWMAKE ;
	}

	/* モデル移動 */
    GM_ActObject( &( work->body ) ) ;

    *( work->trigger ) = trig ;

	/*-------------------------------------------------------------------*/
	/*-------------------------------------------------------------------*/
	/*-------------------------------------------------------------------*/
#if 0
    trig = *( work->trigger ) ;
#ifdef CAMERA_TURN
    {
		SVECTOR		rot ;

		rot = GM_PlayerControl->rot ;
		rot.vx = GM_CameraDir.vx ;
		rot.vy = GM_CameraDir.vy ;
		if ( work->cur_motion == AMstand ||
			work->cur_motion == AMcrouch ) rot.vx = 0 ;

		GV_NearExp4PV( &work->rot, &rot, 3 ) ;
		GV_NearExp4VF( &work->pos, &ArmBodyPosition, 3 ) ;
		DG_SetPos2( &work->pos, &work->rot ) ;

		work->camera->rotate.vx = GM_CameraDir.vx ;
		work->camera->rotate.vy = GV_NearExp4P( work->camera->rotate.vy, work->rot.vy ) ;
    }
#else
    DG_SetPos( work->body.objs->root ) ;
#endif
    GM_ActObject( &( work->body ) ) ;

#ifdef ARM_VISIBLE_MODE_A
    /* 可視制御 */
    if ( trig & ARM_INVISIBLE ) {
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
#ifndef HAND_NEEDLESS
		DG_InvisibleObjs( work->right_hand.objs ) ;
		DG_InvisibleObjs( work->left_hand.objs ) ;
#endif
    } else {
		DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
#ifndef HAND_NEEDLESS
		DG_VisibleObjs( work->right_hand.objs ) ;
		DG_VisibleObjs( work->left_hand.objs ) ;
#endif
    }
#endif

    /* 終了検知 */
    if ( GM_CheckObject_IsEnd( &( work->body ), 0 ) ) {
		trig |= ARM_MOTION_IS_END ;
    } else {
		trig &= ~ARM_MOTION_IS_END ;    
    }

    /* モーション変更 */
    wp = PL_GetPlayerWeapon() ;
    if ( wp != work->cur_weapon ) {
		/* 武器変更 */
		SetMotionSet( wp ) ;
		work->cur_weapon = wp ;
		/* 必ずモーション変更する */
		work->cur_motion = -1 ;
    }
    motion = *( work->motion ) ;
    if ( work->cur_motion != motion ) {
		int	interp ;

		interp = 6 * ONE_CLOCK ;
		if ( motion >= AM_MAX_CHANGE_MOTIONS ) {
			mt = motion - AM_MAX_CHANGE_MOTIONS ;
			GM_ConfigObjectAction( &( work->body ), 0, AMS.shared[ mt ],
								  0, 0xfffff, interp ) ;
		} else {
			GM_ConfigObjectAction( &( work->body ), 0, AMS.change[ motion ],
								  0, 0xfffff, interp ) ;
		}
		work->cur_motion = motion ;
    }
    *( work->trigger ) = trig ;

    /* カメラ更新 */
#ifdef DEBUG_MODE
    if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
		GM_PlayerDebugMode == GM_PDM_WEAPON_CAMERA ) {
		int	status ;

		status = GV_PadData[ 1 ].status ;
		if ( status & PAD_U ) ArmCamShift.vy += 1.0F ;
		if ( status & PAD_D ) ArmCamShift.vy -= 1.0F ;
		if ( status & PAD_L ) ArmCamShift.vx += 1.0F ;
		if ( status & PAD_R ) ArmCamShift.vx -= 1.0F ;
		if ( status & PAD_X ) ArmCamShift.vz += 1.0F ;
		if ( status & PAD_B ) ArmCamShift.vz -= 1.0F ;
		if ( status & PAD_Y ) ArmCamRotX += 0.05F ;
		if ( status & PAD_A ) ArmCamRotX -= 0.05F ;
    } else {
		/* スネーク用とライデン用が用意してある */
		if ( GM_PlayerStatus & PLAYER_GROUND ) {
			WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
				WeaponCamShiftSnakeG : WeaponCamShiftRaidenG ;
		} else {
			WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
				WeaponCamShiftSnake : WeaponCamShiftRaiden ;
		}

		if ( wp >= MAX_SET_WEAPON ) wp = WP_Usp ;

		ArmCamShift = WeaponCamShift[ wp ] ;
		ArmCamRotX = WeaponCamShift[ wp ].vw ;
    }
#else
    /* スネーク用とライデン用が用意してある */
    if ( GM_PlayerStatus & PLAYER_GROUND ) {
		WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
			WeaponCamShiftSnakeG : WeaponCamShiftRaidenG ;
    } else {
		WeaponCamShift = ( GM_PlayerStatus & PLAYER_SNAKE ) ?
			WeaponCamShiftSnake : WeaponCamShiftRaiden ;
    }

    if ( wp >= MAX_SET_WEAPON ) wp = WP_Usp ;

    ArmCamShift = WeaponCamShift[ wp ] ;
    ArmCamRotX = WeaponCamShift[ wp ].vw ;
#endif
    if ( work->camera->on == 1 ) {
		FMATRIX		mat, adj ;

		_sceVu0RotMatrixX( &adj, &DG_UnitMatrix, 
						  ArmCamRotX * 3.141592653589793F / 180.0F ) ;
		_sceVu0MulMatrix( &mat, &( work->body.objs->world ),
						 &adj ) ;
		if ( GM_Camera->chanl[ work->camera->chanl ].flag & GM_FLAG_CAMERA_INTERP ) {
			mat.m[ 3 ][ 0 ] = work->pos.vx ;
			mat.m[ 3 ][ 1 ] = work->pos.vy ;
			mat.m[ 3 ][ 2 ] = work->pos.vz ;
		} else {
			mat.m[ 3 ][ 0 ] = ArmBodyPosition.vx ;
			mat.m[ 3 ][ 1 ] = ArmBodyPosition.vy ;
			mat.m[ 3 ][ 2 ] = ArmBodyPosition.vz ;
		}
		DG_SetPos( &mat ) ;
		DG_PutVector( &ArmCamShift, &( work->camera->position ), 1 ) ;
		//	GM_SetCameraQuick( 0 ) ;
#ifdef ARM_VISIBLE_MODE_B
		DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
#ifndef HAND_NEEDLESS
		DG_VisibleObjs( work->right_hand.objs ) ;
		DG_VisibleObjs( work->left_hand.objs ) ;
#endif
#endif
    } else {
#ifdef ARM_VISIBLE_MODE_B
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
#ifndef HAND_NEEDLESS
		DG_InvisibleObjs( work->right_hand.objs ) ;
		DG_InvisibleObjs( work->left_hand.objs ) ;
#endif
#endif
    }
#endif
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &( work->body ) ) ;
#ifndef HAND_NEEDLESS
    GM_FreeObject( &( work->right_hand ) ) ;
    GM_FreeObject( &( work->left_hand ) ) ;
#endif
    GM_PlayerArmBody = NULL ;
}

/*-----------------------------------------------------------------*/

/* オブジェクトを初期化 */
static	void	InitObject( work, root )
Work		*work ;
OBJECT		*root ;
{
    OBJECT	*body ;

    body = &( work->body ) ;
    GM_InitObject( body, MODEL_NAME, OBJECT_FLAG ) ;
	//    GM_ConfigObjectRoot( body, root, 0 ) ;
    GM_ConfigObjectMotion( body, 1, MOTION_NAME, MT_FLAG_HUMAN1 ) ;
    GM_ConfigObjectStep( body, &( work->step ) ) ;
    DG_SetLightMatrix( body->objs, GM_PlayerBody->objs->light ) ;
    work->cur_weapon = PL_GetPlayerWeapon() ;
    SetMotionSet( work->cur_weapon ) ;
    GM_ConfigObjectAction( &( work->body ), 0, AMS.change[ AMstand ], 0, 0xfffff, 0 ) ;
    work->cur_motion = AMstand ;
    *( work->motion ) = AMstand ;
	DG_InvisibleObjs( body->objs ) ;
}

#ifndef HAND_NEEDLESS
static	void	InitRightHand( work )
Work		*work ;
{
    OBJECT	*body ;

    body = &( work->right_hand ) ;
    GM_InitObject( body, RIGHT_NAME, OBJECT_FLAG2 ) ;
    GM_ConfigObjectRoot( body, &( work->body ), ARM_RIGHT_HAND_UNIT ) ;
}

static	void	InitLeftHand( work )
Work		*work ;
{
    OBJECT	*body ;

    body = &( work->left_hand ) ;
    GM_InitObject( body, LEFT_NAME, OBJECT_FLAG2 ) ;
    GM_ConfigObjectRoot( body, &( work->body ), ARM_LEFT_HAND_UNIT ) ;
}
#endif

static	int	GetResources( work, root ) 
Work		*work ;
OBJECT		*root ;
{
    InitObject( work, root ) ;
#ifndef HAND_NEEDLESS
    InitRightHand( work ) ;
    InitLeftHand( work ) ;
#endif
    work->camera = GM_FindCamera( ARM_CAMERA ) ;
    work->targ = GM_PlayerControl->mov ;
	work->rot = GM_PlayerControl->rot ;
    return 0 ;
}

/*---------------------------------------------------------------------------*/

/* 主観腕 */
void *NewRaidenArm( root, motion, rhand, lhand, trigger )
OBJECT		*root ;
int			*motion, *rhand, *lhand, *trigger ;
{
    Work		*work ;

    work = (Work *)GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
								  sizeof( Work ), PLAYER_ARM_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		work->motion = motion ;
		work->rhand = rhand ;
		work->lhand = lhand ;
		work->trigger = trigger ;
		work->chanl = GM_CurrentCameraChanl ;
		if ( GetResources( work, root ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return (void *)( &( work->body ) ) ;
}
