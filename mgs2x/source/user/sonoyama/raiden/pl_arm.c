//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_arm.c
   新・主観腕
   
   2000/09/12 M.Sonoyama
   $Id: pl_arm.c,v 1.1.1.3 2002/11/19 11:50:57 Yoshizawa1 Exp $
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
#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_arm.h"
#include	"pl_arm_mot.h"


#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)

#define	ARM_CAMERA	(1832632) /* GV_StrCode( "武器カメラ" ) */

enum {
	CS_NONE = 0 ,
	CS_M9,
	CS_USP,
	CS_SOCOM,
	CS_FAMAS,
	CS_SPP,
	CS_STG,
	CS_RGB,
	CS_GRENADE,
	CS_MIC,
	CS_SPRAY,
	CS_DEMOMIC,
	CS_NKT,
} ;

enum {
	WS_M9 = 0,
	WS_USP,
	WS_SOCOM,
	WS_M4,
	WS_RGB,
	WS_SPRAY,
	WS_MIC,
} ;

enum {
	RS_M9 = 0,
	RS_USP,
	RS_GRENADE,
	RS_SOCOM,
	RS_RGB6,
	RS_M4,
} ;

/* 主観時武器カメラ設定 */
static	FVECTOR		WeaponCamShiftSnake[] = {
    { 0.0F, 500.0F, 240.0F, 0.0F },	/* 素手 */
    { 3.0F, 550.0F, 301.0F, 0.0F }, /* ベレッタ */
    { -65.0F, 547.0F, 297.0F, 0.0F }, /* ＵＳＰ */
    { -65.0F, 531.0F, 352.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 15.0F, 521.0F, 151.0F, 0.0F }, 	/* ＦＡＭＡＳ */
	{ -64.0F, 534.0F, 352.0F, 0.0f },	/* Ｓｐｐ１Ｍ */ 
	{ 27.0F, 632.0F, 427.0F, 42.0f },	/* スティンガー */ 
	{ 47.0F, 530.0F, 156.0F, 0.0F },		/* ＲＧＢ－６ */
	{ -45.0F, 559.0F, 14.0F, 0.0F },		/* グレネード */
	{ -76.0F, 302.0F, 66.0F, 0.0F },	/* マイク */
	{ -83.0F, 159.0F, 53.0F, 0.0F },	/* スプレー */
	{ -148.0F, 211.0F, -803.0F, 0.0F },	/* デモマイク */
    { 196.0F, 508.0F, 131.0F, 63.0F },	/* ニキータアタック */
} ;

static	FVECTOR		WeaponCamShiftRaiden[] = {
    { 0.0F, 451.0F, 208.0F, 0.0F },	/* 素手 */
    { 20.0F, 533.0F, 336.0F, 0.0F }, /* ベレッタ */
    { -76.0F, 514.0F, 312.0F, 0.0F }, /* ＵＳＰ */
    { -76.0F, 514.0F, 312.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 64.0F, 418.0F, 257.0F, 0.0F },	/* Ｍ４＆ＡＫ */
	{ -63.0F, 534.0F, 352.0F },		  	/* Ｓｐｐ１Ｍ */ 
	//{ 28.0F, 639.0F, 366.0F, 40.0F },	/* スティンガー */ 
	{ 17.0F, 606.0F, 434.0F, 40.0F },	/* スティンガー */ 
	{ 72.0F, 428.0F, 240.0F, 0.0F },	/* ＲＧＢ－６ */
	{ -45.0F, 508.0F, 14.0F, 0.0F },	/* グレネード */
	{ -33.0F, 203.0F, -24.0F, 0.0F },	/* マイク */
	{ -64.0F, 108.0F, -8.0F, 0.0F },	/* スプレー */
	{ -148.0F, 211.0F, -803.0F, 0.0F },	/* デモマイク */
    { 91.0F, 371.0F, 176.0F, 52.0F },	/* ニキータアタック */
} ;

/* 壁際 */
static	FVECTOR		WeaponCamShiftSnakeW[] = {
	{ 53.0F, 604.0F, -46.0F, 0.0F },	/* M9 */
	{ 60.0F, 642.0F, -53.0F, 0.0F },	/* USP */
	{ 0.0F, 0.0F, 0.0F },				/* SOCOM */
	{ -8.0F, 404.0F, 140.0F, 0.0F },	/* M4 */
	{ 47.0F, 492.0F,  96.0F, 0.0F },	/* RGB */
	{ -99.0F, 192.0F, -30.0F, -23.0F },	/* スプレー */
	{ -105.0F, 292.0F, 386.0F,0.0F },	/* マイク */
} ;

static	FVECTOR		WeaponCamShiftRaidenW[] = {
	{ 22.0F, 536.0F, -69.0F, 0.0F },	/* M9 */
	{ 60.0F, 642.0F, -53.0F, 0.0F },	/* USP */
	{ -29.0F, 509.0F, 20.0F, 0.0F },	/* SOCOM */
	{ -8.0F, 404.0F, 140.0F, 0.0F },	/* M4 */
	{ 47.0F, 366.0F, 132.0F, 0.0F },	/* RGB */
	//{ 61.0F, 199.0F, -19.0F, 0.0F },	/* スプレー */
	{ -64.0F, 108.0F, 26.0F, 0.0F },	/* スプレー */
	//{ 89.0F, 264.0F, -85.0F, 0.0F },	/* マイク */
	{ -33.0F, 203.0F, 283.0F, 0.0F },	/* マイク */
} ;

/* リロード */
static	FVECTOR		WeaponCamShiftSnakeR[] = {
//	{ -17.0F, 461.0F, 301.0F, 0.0F },
//	{ -77.0F, 464.0F, 304.0F, 0.0F }
    { 3.0F, 550.0F, 301.0F, 0.0F }, /* ベレッタ */
    { -65.0F, 547.0F, 297.0F, 0.0F }, /* ＵＳＰ */
//    { 30.0F, 381.0F, 301.0F, 32.0F }	/* グレ */
    { 30.0F, 395.0F, 300.0F, 80.0F },	/* グレ */
    { 0.0F, 0.0F, 0.0F },				/* SOCOM（なし） */
	{ -35.0F, 401.0F, 190.0F, 190.0F },	/* ＲＧＢ６ */
    { 15.0F, 521.0F, 94.0F, 0.0F }, 	/* ＦＡＭＡＳ */
} ;

static	FVECTOR		WeaponCamShiftRaidenR[] = {
    { -22.0F, 517.0F, 336.0F, 0.0F }, /* ベレッタ */
    { -65.0F, 547.0F, 297.0F, 0.0F }, /* ＵＳＰ */
	//    { 46.0F, 409.0F, 326.0F, 32.0F }	/* グレ */
    { 30.0F, 395.0F, 300.0F, 80.0F },	/* グレ */
	{ -56.0F, 523.0F, 315.0F, 0.0F },	/* SOCOM */
	{ -6.0F, 331.0F, 201.0F, 212.0F },	/* ＲＧＢ６ */
	{ 29.0F, 385.0F, 219.0F, 0.0F },	/* M4 */
} ;

/* 匍匐用の設定 */
static	FVECTOR		WeaponCamShiftSnakeG[] = {
    { 0.0F, 0.0F, 0.0F, 0.0F },	/* 素手 */
    { 52.0F, 115.0F, 558.0F, 0.0F }, /* ベレッタ */
    { -17.0F, 94.0F, 552.0F, 0.0F }, /* ＵＳＰ */
    { -14.0F, 101.0F, 552.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 14.0F, 140.0F, 472.0F, 0.0F }, /* ＦＡＭＡＳ */
	{ -13.0F, 103.0F, 552.0F, 0.0F },	/* Ｓｐｐ１Ｍ */ 
	{ 72.0F, 584.0F, 451.0F, 0.0F },	/* スティンガー */ 
	{ 47.0F, 87.0F, 549.0F, 0.0F },		/* ＲＧＢ－６ */
	{ 39.0F, 134.0F, 356.0F, 0.0F },	/* グレネード */
	{ 13.0F, 251.0F, 577.0F, 0.0F },	/* マイク */
	{ -70.0F, 119.0F, 539.0F, -87.0F }	/* スプレー */
} ;

static	FVECTOR		WeaponCamShiftRaidenG[] = {
    { 0.0F, 0.0F, 0.0F, 0.0F },	/* 素手 */
    { 85.0F, 122.0F, 549.0F, 0.0F }, /* Ｍ９２ */
    { -27.0F, 100.0F, 518.0F, 0.0F }, /* ＵＳＰ */
    { -27.0F, 100.0F, 518.0F, 0.0F }, /* ＳＯＣＯＭ */
    { 14.0F, 140.0F, 472.0F, 0.0F }, /* ＦＡＭＡＳ */
	{ -14.0F, 101.0F, 595.0F, 0.0F }, /* Ｓｐｐ１Ｍ */ 
	{ 72.0F, 584.0F, 451.0F, 0.0F },	/* スティンガー */ 
	{ 22.0F, 100.0F, 514.0F, 0.0F },		/* ＲＧＢ－６ */
	{ 25.0F, 113.0F, 284.0F, 0.0F },	/* グレネード */
	{ 31.0F, 168.0F, 433.0F, 0.0F },	/* マイク */
	{ -42.0F, 31.0F, 521.0F, -16.0F },	/* スプレー */
} ;

/* 壁際 */
static	FVECTOR		WeaponCamShiftSnakeWG[] = {
	{ -1.0F, 10.0F, 347.0F, 0.0F },
	{ -5.0F, 0.0F, 347.0F, 0.0F },
	{ 0.0F, 0.0F, 0.0F },				
	{ 19.0F, 34.0F, 440.0F, 0.0F },		/* M4 */
	{ 47.0F, 102.0F, 382.0F, 0.0F },		/* RGB */
	{ -70.0F, 119.0F, 579.0F, -87.0F },	/* スプレー */
	{ 31.0F, 250.0F, 836.0F, 0.0F },	/* マイク */
} ;

static	FVECTOR		WeaponCamShiftRaidenWG[] = {
	//{ -41.0F, 57.0F, 480.0F, 0.0F },	/* M9 */
	{ -72.0F, 306.0F, 484.0F, 0.0F },	/* M9 */
	{ -5.0F, 0.0F, 347.0F, 0.0F },		/* USP */
	{ -41.0F, 57.0F, 480.0F, 0.0F },	/* SOCOM */
	{ 19.0F, 34.0F, 440.0F, 0.0F },		/* M4 */
	{ 72.0F, 81.0F, 450.0F, 0.0F },		/* RGB */
	//{ 42.0F, -29.0F, 383.0F, -16.0F },	/* SPRAY */
	{ -42.0F, 31.0F, 579.0F, -16.0F },	/* SPRAY */
	//{ 2.0F, 123.0F, 366.0F, 0.0F },	/* マイク */
	{ 31.0F, 168.0F, 792.0F, 0.0F },	/* マイク */
} ;

/* リロード */
static	FVECTOR		WeaponCamShiftSnakeRG[] = {
    { 52.0F, 140.0F, 558.0F, 0.0F }, /* ベレッタ */
    { -17.0F, 94.0F, 552.0F, 0.0F }, /* ＵＳＰ */
    { 66.0F, 168.0F, 509.0F, 0.0F },	/* グレ */
	{ 0.0F, 0.0F, 0.0F, 0.0F },		/* SOCOM（なし） */
	{  47.0F, 102.0F, 537.0F, 0.0F },	/* ＲＧＢ６ */
	{ -16.7F, 79.7F, 507.8F, 0.0F },	/* M4 */
} ;

static	FVECTOR		WeaponCamShiftRaidenRG[] = {
    { 20.0F, 66.0F, 549.0F, 0.00F }, /* ベレッタ */
    { -17.0F, 94.0F, 552.0F, 0.0F }, /* ＵＳＰ */
    { 58.0F, 108.0F, 326.0F, 54.0F }, /* グレ */
	{ -14.0F, 66.0F, 517.0F, 0.0F },	/* SOCOM */
	{ -19.0F, 65.0F, 467.0F, 0.0F },	/* ＲＧＢ６ */
	{ -16.7F, 79.7F, 527.8F, 0.0F },	/* M4 */
} ;

/* 武器番号とカメラシフト位置セットの対応テーブル */
static	int	ShiftSet[] = {
	CS_NONE, CS_M9, CS_USP, CS_SOCOM, CS_NONE, 
	CS_RGB, CS_NKT, CS_STG, CS_NONE, CS_NONE, 
	CS_GRENADE,	CS_GRENADE, CS_MIC, CS_NONE, CS_SPRAY, 
	CS_FAMAS, CS_GRENADE, CS_GRENADE, CS_FAMAS, CS_NONE,
	CS_DEMOMIC, CS_NONE
} ;

static	int	ShiftSetW[] = {
	WS_M9, WS_M9, WS_USP, WS_SOCOM, WS_M9,
	WS_RGB, WS_M9, WS_M9, WS_M9, WS_M9,
	WS_M9, WS_M9, WS_MIC, WS_M9, WS_SPRAY,
	WS_M4, WS_M9, WS_M9, WS_M4, WS_M9,
	WS_M9, WS_M9
} ;

static	int	ShiftSetR[] = {
	RS_M9, RS_M9, RS_USP, RS_SOCOM, RS_M9,
	RS_RGB6, RS_M9, RS_M9, RS_M9, RS_M9,
	RS_GRENADE, RS_GRENADE, RS_M9, RS_M9, RS_M9, 
	RS_M4, RS_GRENADE, RS_GRENADE, RS_M4, RS_M9,
	RS_M9, RS_M9
} ;

/* 回転調整値 */
static	SVECTOR		WeaponCamRotateShift[] = {
	{ 0, 0, 0 },				/* 調整値なし */
	{ -69, 122, 0 },			/* マイク立ち */
	{ 37, 210, 0 },				/* マイク匍匐 */
	{ -93, 56, 0 },				/* デモマイク立ち */
	{ 678, 230, 0 },			/* ライデンＭ９ホフク壁 */
} ;

enum {
	RTS_NONE			=	0,
	RTS_MIC_STAND,
	RTS_MIC_GROUND,
	RTS_DEMOMIC_STAND,
	RTS_RAI_M9_GROUND_WALL,
} ;

static	int	RotateShiftSet[] = {
	RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE, RTS_MIC_STAND, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE,
	RTS_DEMOMIC_STAND, RTS_NONE
} ;

static	int	RotateShiftSetG[] = {
	RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE, RTS_MIC_GROUND, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE, RTS_NONE,
	RTS_NONE, RTS_NONE
} ;

/* ブレード特殊 */
static	FVECTOR		BladeShiftSet[] = {
	{ 0.0F, 500.0F, 0.0F, 0.0F },		/* 斬り */
	{ 29.0F, 353.0F, 94.0F, 0.0F },		/* 持ち替え */
	{ 336.0F, 494.0F, 15.0F, 0.0F },	/* ガード */
	{ 0.0F, 500.0F, 0.0F, 0.0F },		/* 回転きり */
    { 0.0F, 500.0F, 240.0F, 0.0F },		/* コンボ */
	//{ 689.0F, 57.0F, -233.0F },		/* コンボ */
	{ 81.0F, 298.0F, 94.0F },			/* 突き */
} ;


FVECTOR			ArmCamShift = { 0.0F, 0.0F, 0.0F } ;
SVECTOR			ArmCamRotateShift ;
float			ArmCamRotX = 0.0F ;

static	ARM_MOTION_SET	AMS ;

enum {
	AM_FLAG_NONE = 		0x0000,
	AM_FLAG_FIRST = 	0x0001,
	AM_FLAG_DEBUG =		0x8000,
} ;

typedef	struct	_Work_pl_arm { //BP_GENERAL - added name so you can debug on PC
    GV_ACT_EX		actor ;
    OBJECT			body ;

    FVECTOR			step ;
    FVECTOR			targ ;
	FVECTOR			delay ;

	TARGET			attack ;
	POWER_TARGET	power ;
	CONTROL			*pctrl ;
	OBJECT			*pbody ;
	GV_PAD			*pad ;

    int				cur_weapon ;
    int				cur_motion ;

    u_char			*motion ;
	u_char			*interp ;
	short			*start_time ;
    int				*trigger ;

    SVECTOR			rot ;
	int				chanl ;
	float			height_adjust ;
	int				count ;
	int				flag ;

    GM_CameraSet	*camera ;	/* 武器カメラ、起動はraiden.c */
#ifdef DEBUG_MODE
	GM_CameraSet	*debug ;
#endif
} Work ;

/* 腕攻撃制御用 */
typedef	struct	{
	FVECTOR		force ;
	long64		weapon_type ;
	u_char		from_unit ;
	u_char		to_unit ;
	u_char		faint ;
	u_char		damage ;
	float		add_len ;
	int			attack ;
} ARM_ATTACK_WORK ;

static	ARM_ATTACK_WORK		ArmAttackWork ;

void	PL_SetArmAttack( long64 weapon_type, FVECTOR *force, int from, int to, 
						 int faint, int damage, float add_len )
{	
	DG_COPY_VEC( &ArmAttackWork.force, force ) ;
	ArmAttackWork.weapon_type = weapon_type ;
	ArmAttackWork.from_unit = from ;
	ArmAttackWork.to_unit = to ;
	ArmAttackWork.faint = faint ;
	ArmAttackWork.damage = damage ;
	ArmAttackWork.add_len = add_len ;
	ArmAttackWork.attack = 1 ;
}

/*-----------------------------------------------------------------*/

/* 本体モーションを使用 */
static	void	ActObjectEX( OBJECT *body )
{
	DG_EVMOBJ	*evmobj ;
	DG_MDL		*mdl ;
	DG_OBJ		*obj ;
	FMATRIX		w[ 55 ], this, parent ;
	FMATRIX		world ;
	FVECTOR		diff ;
	int			i ;

	DG_GetPos( &world ) ;
	evmobj = body->evmobj ;
	body->evmobj = NULL ;
	GM_ActObject( body ) ;
	body->evmobj = evmobj ;

	{
		SVECTOR		rot ;

		rot.vx = GM_CameraDir.vx ;
		rot.vy = 0 ;
		rot.vz = 0 ;
		//GM_RotToQuatXAfterY( &rot, &GM_PlayerBody->m_ctrl->adjust[ HUMAN21_KOSHI ] ) ;
		GM_RotToQuatXAfterY( &rot, &GM_PlayerBody->m_ctrl->adjust[ HUMAN21_ONAKA ] ) ;
		GM_PlayerBody->m_ctrl->adjust_flag 
			|= /*( 1 << HUMAN21_KOSHI ) |*/ ( 1 << HUMAN21_ONAKA ) ;
	}
	_sceVu0SubVector( &diff, ( FVECTOR * )world.m[ 3 ], 
					 ( FVECTOR * )GM_PlayerBody->objs->world.m[ 3 ] ) ;
#if 0
	for ( i = 0; i < 21; i ++ ) {
		obj = &GM_PlayerBody->objs->objs[ i ] ;
		mdl = obj->model ;
		DG_COPY_MAT( &this, &obj->world ) ;
		if ( mdl->parent < 0 ) {
			DG_COPY_MAT( &parent, &GM_PlayerBody->objs->world ) ;
		} else {
			DG_COPY_MAT( &parent, &GM_PlayerBody->objs->objs[ mdl->parent ].world ) ;
		}
		GV_VecToMat( &DG_ZeroVector, &this ) ;
		GV_VecToMat( &DG_ZeroVector, &parent ) ;
		_sceVu0InversMatrix( &parent, &parent ) ;
		_sceVu0MulMatrix( &w[ i ], &this, &parent ) ;
		NewPointView( ( FVECTOR * )GM_PlayerBody->objs->objs[ i ].world.m[ 3 ], 32, 232, 32 ) ;
	}
	for ( i = 21; i < 55; i ++ ) {
		obj = &body->objs->objs[ i ] ;
		mdl = obj->model ;
		DG_COPY_MAT( &this, &obj->world ) ;
		DG_COPY_MAT( &parent, &body->objs->objs[ mdl->parent ].world ) ;
		GV_VecToMat( &DG_ZeroVector, &this ) ;
		GV_VecToMat( &DG_ZeroVector, &parent ) ;
		_sceVu0InversMatrix( &parent, &parent ) ;
		_sceVu0MulMatrix( &w[ i ], &this, &parent ) ;
	}
	for ( i = 0; i < 55; i ++ ) {
		obj = &body->objs->objs[ i ] ;
		mdl = obj->model ;

		if ( mdl->parent < 0 ) {
			DG_COPY_MAT( &parent, &body->objs->world ) ;
		} else {
			DG_COPY_MAT( &parent, &body->objs->objs[ mdl->parent ].world ) ;
		}
		_sceVu0MulMatrix( &obj->world, &w[ i ], &parent ) ;
		_sceVu0ApplyMatrix( ( FVECTOR * )obj->world.m[ 3 ], &parent, &obj->trans ) ;
		//NewPointView( ( FVECTOR * )obj->world.m[ 3 ], 32, 32, 232 ) ;
		//HZX_ViewMatrix( &obj->world, 100.0F ) ;
	}
#endif
#if 1
	for ( i = 21; i < 55; i ++ ) {
		obj = &body->objs->objs[ i ] ;
		mdl = obj->model ;
		DG_COPY_MAT( &this, &obj->world ) ;
		DG_COPY_MAT( &parent, &body->objs->objs[ mdl->parent ].world ) ;
		GV_VecToMat( &DG_ZeroVector, &this ) ;
		GV_VecToMat( &DG_ZeroVector, &parent ) ;
		_sceVu0InversMatrix( &parent, &parent ) ;
		_sceVu0MulMatrix( &w[ i ], &this, &parent ) ;
	}
	DG_COPY_MAT( &body->objs->world, &GM_PlayerBody->objs->world ) ;
	_sceVu0AddVector( ( FVECTOR * )body->objs->world.m[ 3 ], 
					  ( FVECTOR * )body->objs->world.m[ 3 ], &diff ) ;

	for ( i = 0; i < 55; i ++ ) {
		obj = &body->objs->objs[ i ] ;
		mdl = obj->model ;
		if ( i < 21 ) {
			DG_COPY_MAT( &obj->world, &GM_PlayerBody->objs->objs[ i ].world ) ;
		} else {
			DG_COPY_MAT( &parent, &body->objs->objs[ mdl->parent ].world ) ;
			_sceVu0MulMatrix( &obj->world, &w[ i ], &parent ) ;
			_sceVu0ApplyMatrix( ( FVECTOR * )obj->world.m[ 3 ], &parent, &obj->trans ) ;
		}
		_sceVu0AddVector( ( FVECTOR * )obj->world.m[ 3 ], 
						  ( FVECTOR * )obj->world.m[ 3 ], &diff ) ;
		//NewPointView( ( FVECTOR * )obj->world.m[ 3 ], 32, 232, 32 ) ;
	}
#if 0
	{
		FMATRIX		*dst ;
		FVECTOR		trans ;
		EVM_SKEL	*skel ;

		evmobj->group_id = body->objs->group_id ;
		evmobj->use_buffer = 1 - evmobj->use_buffer ;
		DG_COPY_VEC( &evmobj->world, &body->objs->world ) ;
		dst = evmobj->matrix[ evmobj->use_buffer ] ;
		skel = evmobj->def->skeleton ;
		for ( i = 0; i < evmobj->n_skeleton; i ++, dst ++, skel ++ ) {
			DG_COPY_MAT( dst, &body->objs->objs[ i ].world ) ;
			trans.vx = -skel->rt_tx ;
			trans.vy = -skel->rt_ty ;
			trans.vz = -skel->rt_tz ;
			trans.vw = 1.0F ;
			_sceVu0ApplyMatrix( ( FVECTOR * )dst->m[ 3 ], dst, &trans ) ;
			//NewPointView( ( FVECTOR * )dst->m[ 3 ], 32, 32, 232 ) ;
		}							
	}
#endif
#endif
}

/*-----------------------------------------------------------------*/

/* 武器ごとにモーションセット */
static	void	SetMotionSet( Work *work, int wp )
{
	AMS.change = ChangeSets[ wp ] ;
	AMS.shared = SharedSet ;
	work->cur_weapon = wp ;
	work->cur_motion = -1 ;
}

/* どのシフト設定にするか*/
static	FVECTOR	*SetWeaponCamShift( int trig )
{
	FVECTOR		*shift ;

	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
		if ( GM_CheckPlayerStatus( PLAYER_GROUND ) ) {
			if ( trig & ARM_IS_RELOAD ) {
				shift = WeaponCamShiftSnakeRG ;
			} else if ( trig & ARM_IS_WALL ) {
				shift = WeaponCamShiftSnakeWG ;
			} else {
				shift = WeaponCamShiftSnakeG ;
			}
		} else {
			if ( trig & ARM_IS_RELOAD ) {
				shift = WeaponCamShiftSnakeR ;
			} else if ( trig & ARM_IS_WALL ) {
				shift = WeaponCamShiftSnakeW ;
			} else {
				shift = WeaponCamShiftSnake ;
			}
		}
	} else {
		if ( GM_CheckPlayerStatus( PLAYER_GROUND ) ) {
			if ( trig & ARM_IS_RELOAD ) {
				shift = WeaponCamShiftRaidenRG ;
			} else if ( trig & ARM_IS_WALL ) {
				shift = WeaponCamShiftRaidenWG ;
			} else {
				shift = WeaponCamShiftRaidenG ;
			}
		} else {
			if ( trig & ARM_IS_RELOAD ) {
				shift = WeaponCamShiftRaidenR ;
			} else if ( trig & ARM_IS_WALL ) {
				shift = WeaponCamShiftRaidenW ;
			} else {
				shift = WeaponCamShiftRaiden ;
			}
		} 
	}
	return shift ;
}

/*-----------------------------------------------------------------*/

/* モーション変更部 */
static	void	ArmMotionAct( Work *work )
{
	int			wp, mot, stime, trig ;
	int			interp ;

	if ( work->flag & AM_FLAG_DEBUG ) return ;
	trig = *( work->trigger ) ;

    /* 終了検知 */
	if ( MT_CHECK_LAST1( work->body.m_ctrl, 0 ) ) {
		trig |= ARM_MOTION_IS_END ;
    } else {
		trig &= ~ARM_MOTION_IS_END ;    
    }
	/* モーション変更 */
	wp = PL_GetPlayerWeapon() ;
	if ( wp != work->cur_weapon ) {
		SetMotionSet( work, wp ) ;
	}
	mot = *( work->motion ) ;
	stime = *( work->start_time ) ;
	interp = *( work->interp ) ;
	if ( mot == PA_NOACT ) mot = AMstand ;
	if ( work->cur_motion != mot ||
		( trig & ARM_MOTION_SET_OVER ) ) {
		trig &= ~ARM_MOTION_SET_OVER ;
//		interp = 6 ;
		if ( !( GM_WeaponTypes[ wp ] & WP_TYPE_GRENADE ) ) {
			if ( mot == AMwall || mot == AMcrouch_wall ||
				work->cur_motion == AMwall ||
				work->cur_motion == AMcrouch_wall ) {
				interp = 10 ;
			}
		}
		if ( mot >= AM_MAX_CHANGE_MOTIONS ) {
			GM_ConfigObjectAction( &work->body, 0, AMS.shared[ mot - AM_MAX_CHANGE_MOTIONS ],
								   stime, 0xfffff, interp * NTSC_TIME_BASE ) ;
		} else {
			GM_ConfigObjectAction( &work->body, 0, AMS.change[ mot ],
								   stime, 0xfffff, interp * NTSC_TIME_BASE ) ;
		}
		work->cur_motion = mot ;
	}
	*work->trigger = trig ;
}

/* 制御部 */
static	void	ArmAction( Work *work )
{
	int			wp, wpr, trig ;
	FVECTOR		*WeaponCamShift ;	
	FVECTOR		shift ;

	if ( work->flag & AM_FLAG_DEBUG ) return ;

	trig = *( work->trigger ) ;

    /* 可視制御 */
    if ( trig & ARM_INVISIBLE ) {
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
    } else {
		DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
    }
	wp = PL_GetPlayerWeapon() ;
	/* カメラ位置パラメータの設定 */
	if ( work->camera->on ) {
		/* ブレード特殊 */
		if ( trig & ARM_BLADE ) {
			wp = ( trig & ARM_BLADE_MODE ) >> 12 ;
			WeaponCamShift = BladeShiftSet ;
			wpr = RotateShiftSet[ WP_Blade ] ;
		} else {
			/* コンボ特殊 */
			if ( trig & ARM_IS_COMBO ) 	wp = WP_None ;
			else if ( trig & ARM_IS_RELOAD ) wp = ShiftSetR[ wp ] ;
			else if ( trig & ARM_IS_WALL ) wp = ShiftSetW[ wp ] ;
			else 				   	  wp = ShiftSet[ wp ] ;
			WeaponCamShift = SetWeaponCamShift( trig ) ;
			if ( GM_CheckPlayerStatus( PLAYER_GROUND ) ) {
				wpr = RotateShiftSetG[ PL_GetPlayerWeapon() ] ;
			} else {
				wpr = RotateShiftSet[ PL_GetPlayerWeapon() ] ;
			}
		}
#ifdef DEBUG_MODE
//		if ( ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE ) &&
		if ( ( PlayerDebugMenuStatus & PDMS_STATUSVIEW ) &&
			( GM_PlayerDebugMode == GM_PDM_WEAPON_CAMERA ||
			 GM_PlayerDebugMode == GM_PDM_DEBUG_ARM ) ) {
			/* 更新しない */
		} else {
			//ArmCamShift = WeaponCamShift[ wp ] ;
			//ArmCamRotX = WeaponCamShift[ wp ].vw ;

            DG_COPY_VEC( &shift, &WeaponCamShift[ wp ] ) ;
			if ( trig & ARM_NEAR_WALL2 ) {
				shift.vz += 200.0F ;
			} else if ( trig & ARM_NEAR_WALL ) {
				shift.vz += 100.0F ;
			}
            if ( work->flag & AM_FLAG_FIRST ) {
				DG_COPY_VEC( &ArmCamShift, &shift ) ;
				ArmCamRotX = WeaponCamShift[ wp ].vw ;	
				ArmCamRotateShift = WeaponCamRotateShift[ wpr ] ;
				work->flag &= ~AM_FLAG_FIRST ;
			} else {
				GV_NearExp4VF( &ArmCamShift, &shift, 3 ) ;
				ArmCamRotX = GV_NearExp4F( ArmCamRotX, WeaponCamShift[ wp ].vw ) ;
				GV_NearExp4PV( &ArmCamRotateShift, 
							   &WeaponCamRotateShift[ wpr ], 3 ) ;
			}
		}
#else
		//ArmCamShift = WeaponCamShift[ wp ] ;
		//ArmCamRotX = WeaponCamShift[ wp ].vw ;
	    DG_COPY_VEC( &shift, &WeaponCamShift[ wp ] ) ;
		if ( trig & ARM_NEAR_WALL2 ) {
			shift.vz += 200.0F ;
		} else if ( trig & ARM_NEAR_WALL ) {
			shift.vz += 100.0F ;
		}
		if ( work->flag & AM_FLAG_FIRST ) {
			DG_COPY_VEC( &ArmCamShift, &shift ) ;
			ArmCamRotX = WeaponCamShift[ wp ].vw ;
			ArmCamRotateShift = WeaponCamRotateShift[ wpr ] ;
			work->flag &= ~AM_FLAG_FIRST ;
		} else {
			GV_NearExp4VF( &ArmCamShift, &shift, 3 ) ;
			ArmCamRotX = GV_NearExp4F( ArmCamRotX, WeaponCamShift[ wp ].vw ) ;
			GV_NearExp4PV( &ArmCamRotateShift, 
						   &WeaponCamRotateShift[ wpr ], 3 ) ;
		}
#endif
	} else {
		work->flag |= AM_FLAG_FIRST ;
	}
	trig &= ~( ARM_IS_WALL | ARM_IS_RELOAD | ARM_IS_COMBO | 
			   ARM_BLADE | ARM_BLADE_MODE | ARM_NEAR_WALL | ARM_NEAR_WALL2 ) ;
	*work->trigger = trig ;
}

/* 配置 */
static	void	SetPos( Work *work )
{
	GV_PAD		*pad ;
	int			rx ;
	float		height_adjust ;
	SVECTOR		rot ;
	FVECTOR		delay, delay_aim ;
	//FVECTOR		adj ;
	FVECTOR		pos, targ, shift ;
	FMATRIX		mat ;

	GV_NearExp4VF( &work->targ, &GM_CameraTarget, 3 ) ;
	GV_NearExp4PV( &work->rot, &GM_CameraDir, 3 ) ;

#ifdef DEBUG_MODE
	if ( work->camera->on || work->debug->on ) {
#else
	if ( work->camera->on ) {
#endif
		DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
		pad = work->pad ;
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
//		_sceVu0RotMatrixX( &adj, &DG_UnitMatrix,
//						   ArmCamRotX * 3.141592653589793F / 180.0F ) ;
//		_sceVu0MulMatrix( &mat, &mat, &adj ) ;
		DG_SetPos( &mat ) ;
		DG_RotVector( &ArmCamShift, &shift, 1 ) ;
		DG_COPY_VEC( &targ, &GM_CameraTarget ) ;
		_sceVu0SubVector( &pos, &work->targ, &shift ) ;
		height_adjust = work->pbody->objs->world.m[ 3 ][ 1 ] - pos.vy ;
		work->height_adjust = height_adjust 
			= GV_NearExp8F( work->height_adjust, height_adjust ) ;
		_sceVu0AddVector( &pos, &pos, &work->delay ) ;
		rot = work->rot ;
		if ( -- work->count > 0 ) {
			pos.vy -= work->count * 2.0F ;
			rx = rot.vx + ( int )work->count * 2 ;
			rot.vx = GV_NearExp4P( rot.vx, rx ) ;
		}
#if 0
		{
			SVECTOR			shiftrot ;
			FVECTOR			v1, v2, v3 ;

			shiftrot.vx = ArmCamRotateShift.vx + rot.vx ;
			shiftrot.vy = ArmCamRotateShift.vy + rot.vy ;
			shiftrot.vz = 0 ;
			
			DG_COPY_VEC( &v1, &ArmCamShift ) ;
			DG_SetPos2( &DG_ZeroVector, &rot ) ;
			DG_RotVector( &v1, &v2, 1 ) ;
			DG_SetPos2( &DG_ZeroVector, &shiftrot ) ;
			DG_RotVector( &v1, &v3, 1 ) ;
			_sceVu0SubVector( &v3, &v3, &v2 ) ;
			_sceVu0SubVector( &pos, &pos, &v3 ) ;
			rot.vx = shiftrot.vx ;
			rot.vy = shiftrot.vy ;
		}
#else
		{
			FVECTOR		quat ;
			
			if ( ArmCamRotateShift.vy != 0 ) {
				GM_RotToQuat( &ArmCamRotateShift, &quat ) ;
			} else {
				GM_RotToQuatXAfterY( &ArmCamRotateShift, &quat ) ;
			}
			DG_COPY_VEC( &work->body.m_ctrl->adjust[ HUMAN21_MIGI_TE ], &quat ) ;
			work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_MIGI_TE ) ;
		}
#endif

		DG_SetPos2( &pos, &rot ) ;
		DG_COPY_VEC( &work->camera->position, &targ ) ;
		work->camera->rotate.vx = GM_CameraDir.vx + ( short )ArmCamRotX ;
		if ( work->camera->rotate.vx > 1023 ) work->camera->rotate.vx = 1023 ;
		else if ( work->camera->rotate.vx < -1023 ) work->camera->rotate.vx = -1023 ;
		work->camera->rotate.vy = GM_CameraDir.vy ;
		if ( PL_GetPlayerWeapon() == WP_Usp && PL_GetUSPLightOn() ) {
			work->pbody->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
		} else {
			work->pbody->objs->flag |= DG_FLAG_SHADOWMAKE ;
		}
	} else {
		DG_SetPos( &work->pbody->objs->world ) ;
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		work->count = 16 ;
		work->height_adjust = 0.0F ;
		work->pbody->objs->flag |= DG_FLAG_SHADOWMAKE ;
	}
	/* モデル移動 */
	if ( *( work->trigger ) & ARM_ACTOBJECT_EX ) {
		ActObjectEX( &work->body ) ;
	} else {
		GM_ActObject( &work->body ) ;
	}

	if ( work->body.objs->flag & ( DG_FLAG_INVISIBLE0 << work->chanl ) ) {
		work->body.evmobj->flag |= ( DG_EVMOBJ_INVISIBLE0 << work->chanl ) ;
	} else {
		work->body.evmobj->flag &= ~( DG_EVMOBJ_INVISIBLE0 << work->chanl ) ;
	}
	
	*( work->trigger ) = *( work->trigger ) & ~ARM_ACTOBJECT_EX ;

	/* wp_mng.c で消す */
	//DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
}

/* 腕攻撃 */
static	void	ArmAttackHit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	work = ( Work * )ptr ;
	if ( !( def->class & TARGET_HAZARD ) ) {
		if ( HZX_OnlineHazardCheck( work->pctrl->hzx_id, &off->center, &def->center,
								    HZX_CHK_ALL, 0, 0 ) ) {
			if ( def->weapon_type == off->weapon_type ) {
				def->damaged &= ~TARGET_POWER ;
			}
			def->weapon_type &= ~off->weapon_type ;
			return ;
		}
	}
	GM_VctrlResetSkip( &GM_PlayerWork->vctrl ) ;
}

static	void	ArmAttack( Work *work )
{
	FVECTOR			from, to, diff ;
	TARGET			*t ;
	POWER_TARGET	*p ;
	ARM_ATTACK_WORK	*aaw ;

	aaw = &ArmAttackWork ;
	if ( aaw->attack == 0 ) return ;
	aaw->attack = 0 ;
	t = &work->attack ;
	p = &work->power ;
	GM_SetTargetWeaponType( t, aaw->weapon_type ) ;
	GM_SetPowerTarget( t, p, POWER_CONST, 255, aaw->faint, aaw->damage, &aaw->force ) ;

	//PL_ArmObjPos( aaw->from_unit, &from ) ;
	//PL_ArmObjPos( aaw->to_unit, &to ) ;
	GV_MatToVec( &work->body.objs->objs[ aaw->from_unit ].world, &from ) ;
	GV_MatToVec( &work->body.objs->objs[ aaw->to_unit ].world, &to ) ;
	_sceVu0SubVector( &diff, &to, &from ) ;
	GV_LenVec3F( &diff, &diff, 0.0F, GV_VecLen3F( &diff ) + aaw->add_len ) ;
	_sceVu0AddVector( &to, &from, &diff ) ;
	GM_MoveOnlineTargetMap( t, &from, &to, work->pctrl->map ) ;
	GM_PutTarget( t ) ;
#ifdef DEBUG_MODE	
	if ( PlayerDebugMenuStatus & PDMS_TARGETVIEW ) {
		ViewFromTo( &from, &to, 23, 232, 32 ) ;
	}
#endif
}

#ifdef DEBUG_MODE
/* 武器カメラ位置調整 */
static	void	AdjustWeaponCamera( Work *work ) 
{
	int		status ;

//	if ( ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE ) &&
	if ( ( PlayerDebugMenuStatus & PDMS_STATUSVIEW ) &&
		 ( GM_PlayerDebugMode == GM_PDM_WEAPON_CAMERA ||
		   GM_PlayerDebugMode == GM_PDM_DEBUG_ARM ) ) {
		status = GV_PadData[ 1 ].status ;
		if ( GM_PlayerDebugMode == GM_PDM_DEBUG_ARM && !work->debug->on ) {
			DG_COPY_VEC( &ArmCamShift, &WeaponCamShiftSnake[ 2 ] ) ; /* スネークのＵＳＰ */
			ArmCamRotX = 0.0F ;
			ArmCamRotateShift = DG_ZeroSVector ;
			return ;
		}
		if ( !( status & PAD_L2 ) ) {
			if ( status & PAD_U ) ArmCamShift.vy += 1.0F ;
			if ( status & PAD_D ) ArmCamShift.vy -= 1.0F ;
			if ( status & PAD_L ) ArmCamShift.vx += 1.0F ;
			if ( status & PAD_R ) ArmCamShift.vx -= 1.0F ;
			if ( status & PAD_X ) ArmCamShift.vz += 1.0F ;
			if ( status & PAD_B ) ArmCamShift.vz -= 1.0F ;
			if ( status & PAD_Y ) ArmCamRotX += 1.0F ;
			if ( status & PAD_A ) ArmCamRotX -= 1.0F ;	
		} else {
			if ( status & PAD_U ) ArmCamRotateShift.vx -= 1 ;
			if ( status & PAD_D ) ArmCamRotateShift.vx += 1 ;
			if ( status & PAD_L ) ArmCamRotateShift.vy += 1 ;
			if ( status & PAD_R ) ArmCamRotateShift.vy -= 1 ;
		}
	}
}

/* デバッグ用モーション鑑賞 */
static	void	DebugArm( Work *work )
{
	static int	Motion = 0 ;
	static int	P = 0 ;
	GV_PAD		*pad ;

	work->flag &= ~AM_FLAG_DEBUG ;
//	if ( ( GM_Debug2PMode != GM_DEBUG_MODE_PLAYER_STATE ) ||
	if ( !( PlayerDebugMenuStatus & PDMS_STATUSVIEW ) ||
		 ( GM_PlayerDebugMode != GM_PDM_DEBUG_ARM ) ) {
		if ( work->debug->on ) {
			work->debug->on = 0 ;
			GM_ChangeCamera( work->chanl ) ;		
			DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
			DG_VisibleObjs( work->pbody->objs ) ;
			GM_PlayerWork->weapon_body = work->pbody ;
			GM_PlayerWork->weapon_unit = HUMAN21_MIGI_TE ;
		}
		return ;
	}
#if 0
	if ( !GM_CheckPlayerStatus( PLAYER_WATCH ) ) {
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		if ( work->debug->on ) {
			work->debug->on = 0 ;
			GM_ChangeCamera( work->chanl ) ;
		}
		return ;
	}
#endif
	if ( work->debug->on == 0 ) {
		work->debug->on = 1 ;
		GM_ChangeCamera( 0 ) ;
		GM_PlayerWork->weapon_body = &work->body ;
		GM_PlayerWork->weapon_unit = HUMAN21_MIGI_TE ;
	}

	pad = GV_PadData + 1 ;
	DG_InvisibleObjs( work->pbody->objs ) ;
	DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
	if ( pad->analog_input & GV_PAD_ANALOG_R_USE ) {
		if ( P != 1 && ( pad->right_dy > 196 ) ) {
			Motion ++ ; P = 1 ;
		} else if ( P != 2 && ( pad->right_dy < 64 ) ) {
			Motion -- ; P = 2 ;
		}
		if ( Motion < 0 ) Motion = MAX_PLARM_MOTIONS - 1 ;
		else if ( Motion >= MAX_PLARM_MOTIONS ) Motion = 0 ;
	} else {
		P = 0 ;
	}
	if ( Motion != work->cur_motion ) {
		GM_ConfigObjectAction( &( work->body ), 0, Motion,
							   0, 0xfffff, 6 * TIME_BASE ) ;
		work->cur_motion = Motion ;
	}
	DEBUG_Locate( 120, 8, 0 ) ;
	DEBUG_Color( 255, 255, 255, 128 ) ;
	DEBUG_Printf( "ARM_MOTION %d\n", work->cur_motion ) ;
	DEBUG_Printf( "Shift %.2f %.2f %.2f %.2f\n", 
				  ArmCamShift.vx, ArmCamShift.vy, ArmCamShift.vz, ArmCamRotX ) ;
	work->flag |= AM_FLAG_DEBUG ;
}
#endif

/*-----------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
#ifdef DEBUG_MODE
	DebugArm( work ) ;
#endif
	ArmAction( work ) ;
#ifdef DEBUG_MODE
	AdjustWeaponCamera( work ) ;
#endif
	SetPos( work ) ;	
	ArmAttack( work ) ;
	ArmMotionAct( work ) ;
#ifdef DEBUG_MODE
	GM_CopyCamera( work->debug, work->camera ) ;
#endif
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &( work->body ) ) ;
    GM_PlayerArmBody = NULL ;
#ifdef DEBUG_MODE
	GM_DeleteCamera( work->debug ) ;
#endif
}

/*-----------------------------------------------------------------*/

static	void	InitTarget( Work *work )
{
	TARGET			*t ;
	
	t = &work->attack ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_CHILD, 0, 
				  ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, ArmAttackHit, ( void * )work ) ;
	ArmAttackWork.attack = 0 ;
}

static	int	GetResources( Work *work, int evm, int kms, int mar )
{
	OBJECT		*body ;

	body = &work->body ;
	GM_InitObject( body, kms, OBJECT_FLAG ) ;
	GM_ConfigObjectEvm( body, evm, DG_EVMOBJ_IRREACTION ) ;
	GM_ConfigObjectMotion( body, 1, mar, 0 ) ;
	GM_ConfigObjectStep( body, &work->step ) ;
	DG_SetLightMatrix( body->objs, work->pbody->objs->light ) ;
	body->evmobj->light = work->pbody->objs->light ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;
	DG_InvisibleObjs( body->objs ) ;
	body->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
    work->cur_motion = *( work->motion ) = 0 ;
	GM_PlayerArmBody = body ;
	SetMotionSet( work, WP_None ) ;
	InitTarget( work ) ;
#ifdef DEBUG_MODE
	{
		GM_CameraSet	*cam ;
		int				name ;

		name = 1941897 ; //GV_StrCode( "主観腕デバッグカメラ" ) ;
		work->debug = cam = NewProgramCamera( name, work->chanl, GM_CAMERA_PROG1, 0 ) ;
		GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE, CAM_FLAG_PAD_ADJUST ) ;
		GM_SetCameraRotate( cam, &work->pctrl->rot ) ;
		GM_SetCameraTrack( cam, 1000 ) ;
		GM_SetCameraAngle( cam, 2.0F ) ;
		GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_SUBJECT,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	}
#endif
    return 0 ;
}

/*---------------------------------------------------------------------------*/

/* 新・主観腕 */
void *NewPlayerArm( CONTROL *ctrl, OBJECT *body, 
				    int evm, int kms, int mar, 
				    u_char *motion, u_char *interp, short *start_time, 
				    int *trigger, int chanl ) 
{
	Work			*work ;

	if ( GM_PlayerWork == NULL ) return NULL ;
	work = ( Work *)GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
								    sizeof( Work ), PLAYER_ARM_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->pctrl = ctrl ;
	work->pbody = body ;
	work->motion = motion ;
	work->interp = interp ;
	work->start_time = start_time ;
	work->trigger = trigger ;
	work->chanl = chanl ;
    work->camera = GM_FindCamera( ARM_CAMERA ) ;
    work->targ = GM_PlayerControl->mov ;
	work->rot = GM_PlayerControl->rot ;
    work->cur_weapon = PL_GetPlayerWeapon() ;
	work->pad = GV_PadData + chanl ;
	work->flag = AM_FLAG_FIRST ;
	if ( GetResources( work, evm, kms, mar ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
