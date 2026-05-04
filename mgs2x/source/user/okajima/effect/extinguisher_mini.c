//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	extinguisher_mini.c
	消化器と噴出物
	2000/04/11 S.Okajima
	$Id: extinguisher_mini.c,v 1.1.1.3 2002/11/19 11:47:05 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define N_PRIMS		(8)
#define N_VERTS		(16)
//#define	MAX_ALPHA	(64.0f)
#define	MAX_ALPHA	(16.0f)
#define	DIM_ALPHA	(0.5f)

#define	EDGE_0		(68)
#define	EDGE_1		(27)

#define	REFLECT_DECAY		(0.6f)
#define	FRICTION_RATIO		(0.3f)
#define	TEX_MARGIN_RATIO	(0.7f)
#define	DECAY_SPEED			(0.99f)

#define	CONT_TIME			(60*8)
#define TARGET_SIZE (3000.0f)

extern void *NewFlour_Gas3( FVECTOR *center );
extern void *NewExtinguisherJet( FVECTOR *pos, FVECTOR *force );
extern void *NewFlour_Fall( int map, int name, FVECTOR *hit ,FVECTOR *force);
extern void	*NewTargetView( TARGET			*targ, u_char r, u_char g, u_char b );

typedef struct holowork_t
{
	GV_ACT_EX	actor ;

	int		name;
	int		where;

	FVECTOR	center;

	int		debug;

	int		mode;
	int		count;

	FVECTOR	pos;
	FVECTOR	bound_min;
	FVECTOR	bound_max;

	TARGET tgt;
	POWER_TARGET pow;

	TARGET target_at;
	POWER_TARGET power_at;

	int		se_count;

	int		irs_on;

	int		proc_id;

} Work ;

static void SetAttackTarget( Work *work )
{
	FVECTOR	size;
	FVECTOR	center;
	FVECTOR	bound_min;
	FVECTOR	bound_max;

	bound_min.vx = work->center.vx - TARGET_SIZE;
	bound_min.vy = work->center.vy - TARGET_SIZE;
	bound_min.vz = work->center.vz - TARGET_SIZE;
	bound_max.vx = work->center.vx + TARGET_SIZE;
	bound_max.vy = work->center.vy + TARGET_SIZE;
	bound_max.vz = work->center.vz + TARGET_SIZE;

	bound_min.vx = ( bound_min.vx > work->bound_min.vx )? bound_min.vx: work->bound_min.vx;
	bound_max.vx = ( bound_max.vx < work->bound_max.vx )? bound_max.vx: work->bound_max.vx;
	bound_min.vy = ( bound_min.vy > work->bound_min.vy )? bound_min.vy: work->bound_min.vy;
	bound_max.vy = ( bound_max.vy < work->bound_max.vy )? bound_max.vy: work->bound_max.vy;
	bound_min.vz = ( bound_min.vz > work->bound_min.vz )? bound_min.vz: work->bound_min.vz;
	bound_max.vz = ( bound_max.vz < work->bound_max.vz )? bound_max.vz: work->bound_max.vz;

//	AN_Test_Eye2( &bound_min, 2 );
//	AN_Test_Eye2( &bound_max, 2 );
//	NewBoundingBoxView( &bound_min, &bound_max, 64, 128, 255 ) ;

	size.vx   = (bound_max.vx - bound_min.vx)*0.5f;
	size.vy   = (bound_max.vy - bound_min.vy)*0.5f;
	size.vz   = (bound_max.vz - bound_min.vz)*0.5f;
	center.vx = bound_min.vx + size.vx;
	center.vy = bound_min.vy + size.vy;
	center.vz = bound_min.vz + size.vz;

	GM_SetTarget( &work->target_at,
	              TARGET_OFFENSE,
	              work->where,
	              BOTH_SIDE,
	              &size,
	              &DG_ZeroVector);
	GM_SetTargetWeaponType( &work->target_at, WP_NOBLOOD|WP_EXTINGUISHER );
	GM_MoveTargetMap( &work->target_at, &center, work->where );

	GM_SetPowerTarget( &work->target_at,
	                   &work->power_at,
	                   POWER_ONCE,
	                   255,
	                   0,
	                   0,
	                   &DG_ZeroVector);
	if( work->debug ) NewTargetView( &work->target_at, 0, 255, 0 ) ;
}

static void Die( Work *work )
{
	GM_FreeTarget( &work->tgt ) ;
	GM_ClearTargetDamage( &work->tgt ) ;
	GM_FreeTarget( &work->target_at ) ;
	GM_ClearTargetDamage( &work->target_at ) ;
}

static void Act( Work *work )
{
	switch( work->mode ){
	  case 0:
		break;
	  case 1:
		if( work->se_count++ > 12 ){
			work->se_count = 0;
			GM_SeSetMode( SD_A_SYOKAK03, &work->pos, GM_SEMODE_NORMAL );	/* 消火器連続噴出 */
		}
		if(work->count-- < 0){
			GV_DestroyActor( work ) ;
		}
		break;
	}
}

static void TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work        *work ;
	FVECTOR		fvtemp;

	work = (Work *)ptr ;
	if ( def->damaged & TARGET_POWER ){
		if( &work->tgt == def  &&  (def->weapon_type & (WP_BULLET|WP_M92)) ){
			extern void *NewExtinguisherGas( FMATRIX *world, FVECTOR *pole );
			if( work->irs_on ){
				extern int OK_IRS_OnFlag;
				OK_IRS_OnFlag = 1;
			}
			if( def->power != NULL ){
				DG_COPY_VEC( &fvtemp, &def->power->force );
			}else{
				_sceVu0AddVector( &fvtemp, &def->center, &def->offset ) ;
				_sceVu0SubVector( &fvtemp, &fvtemp, &def->hit ) ;
			}

			NewFlour_Gas3( &def->hit );
			NewExtinguisherJet( &def->hit, &fvtemp );
			NewFlour_Fall( work->where, work->name, &def->hit ,&def->power->force);

			def->class &= ~(TARGET_THROUGH|TARGET_POWER);

			if( work->mode==0 ){
				work->mode=1;
				GM_SeSetMode( SD_A_SYOKAK01, &def->hit, GM_SEMODE_NORMAL );	/* 消火器着弾 */
				if( work->proc_id > 0 ){
					GCL_ExecProc( work->proc_id, NULL );
				}
			}
		}
		GM_ClearTargetDamage( def ) ;
	}
}



#ifdef PSX2
#define PERROR(...) { printf( __VA_ARGS__ ) ; return -1 ; }
#else
#define PERROR(...) 	 { printf( __VA_ARGS__ ) ; return -1 ; } 
#endif



static int GetResources( Work *work )
{
	int	  buf[3] ;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;

	work->proc_id = -1;
	if ( GCL_GetOption( 'e' ) != NULL ){
		work->proc_id = GCL_GetNextInt();
	}

	work->irs_on = 0;
	if ( GCL_GetOption( 'i' ) != NULL ){
		work->irs_on = GCL_GetNextInt() ;
	}

	if( GCL_GetOption( 'd' ) != NULL ){
		work->debug = GCL_GetNextInt();
	}else{
		work->debug = 0;
	}

	if ( GCL_GetOption( 'p' ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->center ) ;
		DG_COPY_VEC( &work->pos, &work->center );
	}else{
		PERROR( "No position!!\n" ) ;
	}

/*
	svtemp0.vx = 0;
	svtemp0.vy = 0;
	svtemp0.vz = 0;
	DG_SetPos2( &work->pos, &svtemp0 );
*/

	fvtemp0.vx = 100.0f;
	fvtemp0.vy = 300.0f;
	fvtemp0.vz = 100.0f;
	GM_SetTarget( &work->tgt, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH|TARGET_ONLINE, work->where, ENEMY_SIDE, &fvtemp0, &DG_ZeroVector ) ;
	GM_SetPowerTarget( &work->tgt, &work->pow, POWER_CONST, 255, 0, 0, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( &work->tgt, TargetCallBack, work ) ;
	GM_PutTarget( &work->tgt ) ;
	GM_MoveTarget( &work->tgt, &work->pos ) ;
	if( work->debug ) NewTargetView( &work->tgt, 255, 0, 0 ) ;

	work->mode = 0;
	work->count = 0;

	if ( GCL_GetOption( 'b' ) != NULL ){
		fvtemp0.vx = (float)GCL_GetNextInt() ;
		fvtemp0.vy = (float)GCL_GetNextInt() ;
		fvtemp0.vz = (float)GCL_GetNextInt() ;
		fvtemp1.vx = (float)GCL_GetNextInt() ;
		fvtemp1.vy = (float)GCL_GetNextInt() ;
		fvtemp1.vz = (float)GCL_GetNextInt() ;

		work->bound_min.vx = (fvtemp0.vx < fvtemp1.vx)?fvtemp0.vx:fvtemp1.vx ;
		work->bound_min.vy = (fvtemp0.vy < fvtemp1.vy)?fvtemp0.vy:fvtemp1.vy ;
		work->bound_min.vz = (fvtemp0.vz < fvtemp1.vz)?fvtemp0.vz:fvtemp1.vz ;
		work->bound_max.vx = (fvtemp0.vx > fvtemp1.vx)?fvtemp0.vx:fvtemp1.vx ;
		work->bound_max.vy = (fvtemp0.vy > fvtemp1.vy)?fvtemp0.vy:fvtemp1.vy ;
		work->bound_max.vz = (fvtemp0.vz > fvtemp1.vz)?fvtemp0.vz:fvtemp1.vz ;
	}
	SetAttackTarget( work );

	work->se_count = 100;

	return 0 ;
}


/*------------------------------------------------------------------*/
void *NewExtinguisherMini( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;
		work->where = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


