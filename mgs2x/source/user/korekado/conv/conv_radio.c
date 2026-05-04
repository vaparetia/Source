//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	radio.c
	無線機
	
	1999/11/05 Y.Korekado
	$Id: radio.c,v 1.1.1.3 2002/11/19 11:44:04 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewRadio( body, sw )

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"define.h"

#include "BP_TrophyLogicMGS2.h"

#define TARGET_ON (1)

extern	void *NewOpticalCamouflage( DG_OBJS *objs, int flag ) ;

/*----------------------------------------------------------------*/
//#define SCALE_DEF	(1)

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
//#define TARGET_CLASS	(TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
#define TARGET_CLASS	(TARGET_ROTATE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
#define FLAG_CLEAR	(0)

//#define RADIO_OFFSET_VY	(-50.0f)
#define RADIO_OFFSET_VY	(110.0f)
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		radio ;

	OBJECT		*body ;
	int			*flag ;

    TARGET			attack ;	/* 攻撃ターゲット */
    POWER_TARGET	atk_power ;	/* 攻撃用 */
    TARGET			target ;	/* 防御ターゲット */
    POWER_TARGET	trg_power ;	/* 防御属性 */

    TARGET			*parent ;	/* 親ターゲット */

	int			think1 ;
	u_int			count ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */
//static FVECTOR	LH_Shift = { -49.0, -100.0, -13.0 } ;
//static SVECTOR	LH_Rot = { 1021, 4042, 2845 } ;
static FVECTOR	LH_Shift = { -5.0, -77.5, -67.5 } ;
static SVECTOR	LH_Rot = { 993, 193, -1038 } ;

static FVECTOR	RH_Shift = { -5.0, -77.5, -67.5 } ;
static SVECTOR	RH_Rot = { 993, -193, 1010 } ;

static FVECTOR	W_Shift = { -120.0, 25.0, -97.5 } ;
static SVECTOR	W_Rot = { 4028, 3777, 2048 } ;

/*----------------------------------------------------------------*/
static	void BreakRadioAttack( work )
Work	*work ;
{
//	static FVECTOR force = { 0.0F,0.0F,0.0F } ;
	static FVECTOR size = { 500.0F,500.0F,500.0F } ;
	TARGET	*off ;
	POWER_TARGET	*power ;

	off = &( work->attack ) ;
	power = &( work->atk_power ) ;

	GM_SetTarget( off, TARGET_OFFENSE | TARGET_CHECK_ONE, 0, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_MECABREAK ) ; 

	GM_MoveTarget3( off, &work->radio.objs->world ) ;

	GM_SetPowerTarget( off, power, POWER_ONCE, 1, 0, 0, &DG_ZeroVector ) ;
	GM_PutTarget( off ) ;

   bp_trophy_destroyed_radio();

//printf("attack meca break type[%x] %x %x \n",off->weapon_type, WP_MECABREAK, I64(1)<<WP_Gus );
}

static	void TargetCheck( work )
Work	*work ;
{
	TARGET *trg ;

	trg = &(work->target) ;
	if ( TARGET_POWER & trg->damaged ) {
		if ( trg->weapon_type & (WP_BULLET|WP_M92) ) {
			extern void *NewRadio_break( FMATRIX *mat, FVECTOR *force ) ;

printf("RADIO BREAK!!!!!!!!!!!!!!!!!!!!!!!!!! \n" ) ;
			
			SET_FLAG( *work->flag, SW_FLAG_BREAK ) ;
			BreakRadioAttack( work ) ;
			/* 無線機破壊エフェクト */
			NewRadio_break( &work->radio.objs->world, &trg->power->force ) ;
		}
		CLEAR_FLAG( trg->damaged ) ;
		CLEAR_FLAG( trg->weapon_type ) ;
	}
}


static	int SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ||
		!(*work->flag & SW_FLAG_VISIBLE) ) {
		SET_FLAG( work->radio.objs->flag, DG_FLAG_INVISIBLE ) ;
		SET_FLAG( work->target.class,TARGET_SKIP ) ;
		return 0 ;
	}

	UNSET_FLAG( work->radio.objs->flag, DG_FLAG_INVISIBLE ) ;
	/* SWITCH1 ON:手に持つ OFF:腰に付ける */
	if ( *work->flag & SW_FLAG_SWITCH1 ) {
		DG_SetPos( &(BODYWORLD(work->body, HUMAN21_HIDARI_TE)) ) ;
		DG_MovePos( &LH_Shift ) ;
		DG_RotatePosZYX( &LH_Rot ) ;
		work->target.offset.vy = 50.0F ;
	} else if ( *work->flag & SW_FLAG_SWITCH2 ) {	/* 右手 */
		DG_SetPos( &(BODYWORLD(work->body, HUMAN21_MIGI_TE)) ) ;
		DG_MovePos( &RH_Shift ) ;
		DG_RotatePosZYX( &RH_Rot ) ;
		work->target.offset.vy = 50.0F ;
	} else {
		DG_SetPos( &(BODYWORLD(work->body, HUMAN21_KOSHI)) ) ;
		DG_MovePos( &W_Shift ) ;
		DG_RotatePosZYX( &W_Rot ) ;
		work->target.offset.vy = RADIO_OFFSET_VY ;
	}

	/* 故障 */
	if ( *work->flag & SW_FLAG_BREAK ) {
		SET_FLAG( work->target.class,TARGET_SKIP ) ;
	} else {
		UNSET_FLAG( work->target.class, TARGET_SKIP ) ;
	}

	return 1 ;
}

static	void	Act( work )
Work		*work ;
{
	FVECTOR	pos ;
	if ( *work->flag < 0 ) {
		GV_DestroyActor( work ) ;
		return ;
	}
//	Think( work ) ;
	
	KR_GroupObject( &work->radio, work->body->map_name ) ;
	TargetCheck( work );
	if ( SetInvisible( work ) ) {
		DG_PutObjs( work->radio.objs ) ;
#ifdef SCALE_DEF
work->radio.objs->world.m[3][0] /= 1.4F ;
work->radio.objs->world.m[3][1] /= 1.4F ;
work->radio.objs->world.m[3][2] /= 1.4F ;
work->radio.objs->world.m[3][3] /= 1.4F ;
#endif
//		KR_FMatToFvec( &work->radio.objs->world, &pos ) ;
//	    GM_MoveTarget( &( work->target ), &pos ) ;
	    GM_MoveTarget2( &( work->target ), &work->radio.objs->world ) ;
	}

#ifndef TARGET_ON
SET_FLAG( work->target.class, TARGET_SKIP ) ;/* デバッグ */
#endif
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &work->radio ) ;

	if ( !(GV_IsFollowDestroy( work )) ) {
		if ( work->parent != NULL ) GM_TargetDisconnectChild( work->parent, &work->target ) ;
	}
    GM_FreeTarget( &work->target ) ;
}

/*----------------------------------------------------------------*/
static	void	TargCallBack( off, def, work )
TARGET		*off, *def ;
Work	*work ;
{
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
//printf("oya call def->weapon_type[%x] off[%x]\n",def->weapon_type,off->weapon_type);
//printf("Oya [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}


static	int	GetResources( work, body, data, flag, parent )
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
int		data ;
int		*flag ;
TARGET	*parent ;
{
	FVECTOR		size, offset ;
	TARGET		*trg ;

	work->body = body ;
	work->flag = flag ;

	GM_InitObject( &work->radio, data, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

#ifdef SCALE_DEF
work->radio.objs->world.m[3][0] /= 1.4F ;
work->radio.objs->world.m[3][1] /= 1.4 ;
work->radio.objs->world.m[3][2] /= 1.4 ;
work->radio.objs->world.m[3][3] /= 1.4 ;
#endif

	if ( *work->flag ) {
		DG_SetPos( &work->body->objs->objs[HUMAN21_HIDARI_TE].world ) ;
		DG_PutObjs( work->radio.objs ) ;
	} else {
		DG_SetPos( &work->body->objs->objs[HUMAN21_MUNE].world ) ;
		DG_PutObjs( work->radio.objs ) ;
	}
	DG_SetLightMatrix( work->radio.objs, body->objs->light );

	trg = &work->target ;
#if 1
	size.vx = 50.0F ; size.vy = 100.0F ; size.vz = 50.0F ;
#else
	size.vx = 35.0F ; size.vy = 50.0F ; size.vz = 35.0F ;
#endif
	offset.vx = 0.0F ; offset.vy = RADIO_OFFSET_VY ; offset.vz = 0.0F ; 

	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  &size, &offset ) ;
    GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, &work->trg_power, POWER_DECREASE, 1, 1, 1, &DG_ZeroVector ) ;
    GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargCallBack, work ) ;

	if ( body->objs->flag & DG_FLAG_OPTCMF ) {
		GV_SetActorChild( work, NewOpticalCamouflage( work->radio.objs, 0 ) ) ;
	}

#ifdef TARGET_PARTS_IS_LINK_LIST
	work->parent = parent ;
	if ( work->parent != NULL ) GM_TargetConnectChild( parent, trg, 1, 0 ) ;
#endif

#ifdef TARGET_ON
//NewTargetView( trg, 34, 184, 200 ) ;
#endif
	work->think1 = 0 ;
	work->count = 0 ;

	return 0 ;
}

void	*NewRadio( body, data, flag, parent )
OBJECT	*body ;	/* ボディーオブジェクト */
int		data ;
int		*flag ;
TARGET	*parent ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, body, data, flag, parent ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
