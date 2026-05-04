//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	imagetrg.c
	心象ターゲット
	
	2000/07/12 Y.Korekado
	$Id: imagetrg.c,v 1.1.1.3 2002/11/19 11:44:03 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
void	*NewImageTarget( body, sw )

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

void	*NewTargetView( TARGET			*targ, u_char r, u_char g, u_char b );

/*----------------------------------------------------------------*/
#define TARGET_CLASS	(TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			*body ;

    TARGET			attack ;	/* 攻撃ターゲット */
    POWER_TARGET	atk_power ;	/* 攻撃用 */
    TARGET			target ;	/* 防御ターゲット */
    POWER_TARGET	trg_power ;	/* 防御属性 */

    TARGET			*calltrg ;	/* 攻撃目標 */
	int				life ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40
/*----------------------------------------------------------------*/
static	void BreakAttack( work )
Work	*work ;
{
	static FVECTOR size = { 500.0F,500.0F,500.0F } ;
	TARGET	*off ;
	POWER_TARGET	*power ;

	off = &( work->attack ) ;
	power = &( work->atk_power ) ;

	GM_SetTarget( off, TARGET_OFFENSE | TARGET_CHECK_ONE, 0, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_SOCOM ) ; 

	GM_MoveTarget3( off, &work->body->objs->objs[0].world ) ;

	GM_SetPowerTarget( off, power, POWER_ONCE, 1, 0, 0, &DG_ZeroVector ) ;
	GM_PutTarget( off ) ;

//printf("attack meca break type[%x] %x %x \n",off->weapon_type, WP_MECABREAK, I64(1)<<WP_Gus );
}

static	int TargetCheck( work )
Work	*work ;
{
	TARGET *trg ;

	trg = &(work->target) ;
	if ( TARGET_POWER & trg->damaged ) {
		if ( trg->weapon_type & (WP_BULLET) ) {
			CLEAR_FLAG( trg->damaged ) ;
			CLEAR_FLAG( trg->weapon_type ) ;
			return 1 ;
		}
		CLEAR_FLAG( trg->damaged ) ;
		CLEAR_FLAG( trg->weapon_type ) ;
	}
	return 0 ;
}



static	void	Act( work )
Work		*work ;
{
	if( work->life < 0 ) {
		GV_DestroyActor( work ) ;
		return ;
	}

	if ( TargetCheck( work ) ) {
		GV_DestroyActor( work ) ;
		BreakAttack( work ) ;
		return ;
	}
	GM_MoveTarget3( &( work->target ), &work->body->objs->objs[0].world ) ;
printf("image target def->weapon_type\n");
	
	work->life -- ;
}

static	void	Die( work )
Work		*work ;
{
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
printf("image target def->weapon_type[%x] off[%x]\n",def->weapon_type,off->weapon_type);
printf("image target [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}

static	int	GetResources( Work	*work, TARGET *calltrg, OBJECT *body, int life )
{
	FVECTOR		size, offset ;
	TARGET		*trg ;

	work->body = body ;
	work->calltrg = calltrg ;
	work->life = life ;
	
	trg = &work->target ;

	size.vx = 200.0F ; size.vy = 200.0F ; size.vz = 200.0F ;
	offset.vx = 0.0F ; offset.vy = 1000.0F ; offset.vz = 0.0F ; 

	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  &size, &offset ) ;
    GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, &work->trg_power, POWER_DECREASE, 1, 1, 1, &DG_ZeroVector ) ;
    GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargCallBack, work ) ;

NewTargetView( trg, 34, 184, 200 ) ;
	return 0 ;
}

void	*NewImageTarget( TARGET *trg, OBJECT *body, int life )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, trg, body, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
