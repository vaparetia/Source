//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	imagetrg.c
	心象ターゲット
	
	2000/07/12 Y.Korekado
	$Id: imagetrg.c,v 1.1.1.3 2002/11/19 11:48:46 Yoshizawa1 Exp $
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

#include "libutl.h"

#include	"gameheader.h"
#include	"../../korekado/conv/korekado.x"
#include	"../../korekado/conv/define.h"
#include 	"../util/ts_util.h"
/*----------------------------------------------------------------*/
#define TARGET_CLASS	(TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

    TARGET			attack ;	/* 攻撃ターゲット */
    POWER_TARGET	atk_power ;	/* 攻撃用 */
    TARGET			target ;	/* 防御ターゲット */
    POWER_TARGET	trg_power ;	/* 防御属性 */

    TARGET			*calltrg ;	/* 攻撃目標 */

	FVECTOR			*size;
	FVECTOR			*pos;
	FMATRIX			world ;
//	OBJECT			*body;
	int 			mode;
	int				life ;
	int				wait ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40
/*----------------------------------------------------------------*/

static	int BreakAttack( work )
Work	*work ;
{
//	static FVECTOR size = { 500.0F,500.0F,500.0F } ;
	TARGET	*off ;
	POWER_TARGET	*pow ;

	off = &( work->attack ) ;
	pow = &( work->atk_power ) ;

	GM_SetTarget( off, TARGET_OFFENSE | TARGET_CHECK_ONE, 0, ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_HEADMARK ) ; 
//	GM_MoveTarget3( off, &work->calltrg->world ) ;
	GM_SetPowerTarget( off, pow, POWER_ONCE, 1, 0, 0, &DG_ZeroVector ) ;
	GM_PutTarget( off ) ;
	GM_TargetSetDirectAttack( off, work->calltrg );
//	printf("sleepy\n");
//printf("attack meca break type[%x] %x %x \n",off->weapon_type, WP_MECABREAK, 1L<<WP_Gus );
	return 0;
}

static	int TargetCheck( work )
Work	*work ;
{
	TARGET *trg ;

	trg = &(work->target);

	if ( TARGET_POWER & trg->damaged ) {
		if ( trg->weapon_type & (WP_BULLET|WP_M92) ) {
			CLEAR_FLAG( trg->damaged ) ;
			CLEAR_FLAG( trg->weapon_type ) ;
			if(!work->mode) return 0;
			return 1 ;
		}
		CLEAR_FLAG( trg->damaged ) ;
		CLEAR_FLAG( trg->weapon_type ) ;
	}

	return 0 ;
}

/*----------------------------------------------------------------*/
static	void	TargCallBack( off, def, work )
TARGET		*off, *def ;
Work	*work ;
{
#if 0
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;

printf("image target def->weapon_type[%x] off[%x]\n",def->weapon_type,off->weapon_type);
printf("image target [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
#else
	FVECTOR		pole,temp;
	FMATRIX		mat;
	
	if ( work->mode ) return;
	
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {
			if( off->weapon_type & WP_BULLET ) {
				//printf("TyoDa-nn\n");
				fpu_SubVectors( &pole,(FVECTOR*)DG_Chanls[0].eye.m[3],&def->hit );
				fpu_VectorNormal(&pole);
				fpu_SubVectors( &temp,&def->hit,&off->center );
				TS_MakeMatrix( &mat,&temp,&def->hit);

				GM_CallSpark( 0, &mat, &pole ) ;
			}
		}
	}
	
#endif
	
}

static void SetTarget( Work *work )
{
	TARGET		*trg = &work->target ;
	
	GM_SetTarget( trg, TARGET_CLASS, 0, ENEMY_SIDE, work->size, &DG_ZeroVector  );
    GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, &work->trg_power, POWER_DECREASE, 1, 1, 1, &DG_ZeroVector ) ;
    GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargCallBack, work ) ;
	
//	GV_SetActorChild(work,NewTargetView( trg, 34, 184, 200 ) );
}


static	void	Act( work )
Work		*work ;
{
//	static FVECTOR size = { 300.0f,300.0f,300.0f,0.0f };

	if( work->life-- < work->wait ){
		if( work->life < 0 ) {
			GV_DestroyActor( work ) ;
			//printf("time over\n");
			return ;
		}		
//		fpu_SubVectors(&force, (FVECTOR*)DG_Chanls[0].eye.m[3],work->pos);
		
		if ( TargetCheck( work ) ) {
			GV_DestroyActor( work ) ;
			BreakAttack( work ) ;
			//printf("target check\n");
			return ;
		}

//		TS_MakeMatrix( &work->world, &force, work->pos );
		work->pos->vw = 0.0f;
		*(FVECTOR*)work->world.m[3] = *work->pos;
		
		GM_MoveTarget3Map( &( work->target ), &work->world, GM_CurrentStageMap );
	    GM_SetTargetSize( &( work->target ), work->size );
	//	GM_SetTargetSize( &( work->target ), &size );
	}else if( work->life == work->wait ){
		SetTarget( work );
	}

}

static	void	Die( work )
Work		*work ;
{
    GM_FreeTarget( &work->target ) ;
}

static	int	GetResources( Work	*work, TARGET *calltrg,
						  FVECTOR *pos, FVECTOR *size, int wait_time, int trg_life, int mode )
{
	TARGET		*trg ;

//	work->body = body ;
	work->calltrg = calltrg ;
	work->life = wait_time + trg_life;
	work->wait = trg_life;
	
	trg = &work->target ;

	work->world = DG_UnitMatrix;
	work->pos = pos;
	work->size = size;
	work->mode = mode;
//	work->body = body;
	GV_SetActorSignalFunc( work,GV_DefaultSignalFunc);
					   
	return 0 ;
}

//
//
//

void	*NewImageTarget2( TARGET *trg, FVECTOR *pos, FVECTOR *size,
						  int wait_time, int trg_life, int mode )
{
	Work		*work ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
								   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, trg, pos, size, wait_time, trg_life, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		//printf("Make MarkTarget\n");
	}
	
	return work ;
}
