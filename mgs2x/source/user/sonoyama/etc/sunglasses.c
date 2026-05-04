//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sunglasses.c
   サングラス管理

   2001/09/20	M.Sonoyama
   $Id: sunglasses.c,v 1.1.1.3 2002/11/19 11:50:45 Yoshizawa1 Exp $
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
#endif


#include	"gameheader.h"

#include "bp_matrix.h"

/* ライデンモデル */
static	int	RaidenKms[] = {
	2681521,			/* rai_def_mt */
	12663374,			/* rai_def */
	11804297,			/* rai_def_sh_mt */
	413122,				/* rai_naked_mt */
	3968315,			/* rai_naked_sh_mt */
	8870921,			/* rai_naked */
	413302,				/* rai_naked_sh */
} ;

static	int	RaidenEvm[] = {
	14591498,			/* rai_def_addhand_mh_mt */
	5512841,			/* rai_def_mh_mt */
	15029734,			/* rai_naked_p073_mh_mt */
	14454074,			/* rai_naked_mh_mt */
	2009334,			/* rai_gbs_addhand_mh_mt */
	2269234,			/* rai_gbs_gbshead_mh_mt */
	2314122,			/* rai_gbs_raihead_mh_mt */
	2802864,			/* rai_gbshead_addhand_mh_mt */
} ;

/* スネーク */
static	int	SnakeKms[] = {
	4274856,			/* sna_def */
	5614437,			/* sna_def_sh */
	4154915,			/* sna_dive_sh_mt */
} ; 

static	int	SnakeEvm[] = {
	4105761,			/* sna_def_addhand_mh_mt */
	5614245,			/* sna_def_mh */
	12894690,			/* sna_coat_mh_mt */
} ;

/* プリスキン */
static	int	PliskinKms[] = {
	1710768,			/* iro_def_mt */
	11769633,			/* iro_def_sh_mt */
} ;

static	int	PliskinEvm[] = {
	7775619,			/* iro_def_addhand_mh_mt */
	6275207,			/* iro_bandage_addhand_mh_mt */
	7248650,			/* iro_bandage_mh_mt */
	2309386,			/* iro_blood_mh_mt */
	5478177,			/* iro_def_mh_mt */
	599886,				/* iro_skull_mh_mt */
	9695135,			/* iro_takeoff_addhand_mh_mt */
	10335038,			/* iro_takeoff_mh_mt */
} ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

#define	VFLAG1	(DG_FLAG_INVISIBLE0|DG_FLAG_INVISIBLE1)
#define	VFLAG2	(DG_EVMOBJ_INVISIBLE0|DG_EVMOBJ_INVISIBLE1)

typedef	struct	{
	GV_ACT_EX		actor ;
	CONTROL			*pctrl ;
	OBJECT			*pbody ;
	int				unit ;
	DG_EVMOBJ		*evmobj ;
	DG_OBJS			*mirror_objs ;

	int				model ;
} Work ;

static	void	Act( Work *work )
{
	DG_EVMOBJ		*evm ;
	FMATRIX			*src ;
	FMATRIX			*dst ;
	FVECTOR			trans ;
	EVM_SKEL		*skel ;
	int				visible = 0 ;
	int				i ;

	//if ( GV_PauseLevel != 0 ) return ;

	/* 存在チェック */
	if ( work->pctrl != NULL ) {
		for ( i = 0; i < GM_N_WhereList; i ++ ) {
			if ( GM_WhereList[ i ] == work->pctrl ) break ;
		}
		if ( i == GM_N_WhereList ) {
			GV_DestroyActor( work ) ;
			return ;
		}
	}

	evm = work->evmobj ;
	evm->flag |= DG_EVMOBJ_INVISIBLE ;
	if ( work->pbody->objs != NULL ) {
		if ( !( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) /*&&
			 !( work->pbody->objs->objs[ work->unit ].flag & DG_FLAG_INVISIBLE0 )*/ ) {
			evm->flag &= ~DG_EVMOBJ_INVISIBLE0 ;
			visible = 1 ;
		}
		if ( !( work->pbody->objs->flag & DG_FLAG_INVISIBLE1 ) /*&&
			 !( work->pbody->objs->objs[ work->unit ].flag & DG_FLAG_INVISIBLE1 )*/ ) {
			evm->flag &= ~DG_EVMOBJ_INVISIBLE1 ;
			visible = 1 ;
		}		
	}

	if ( visible == 0 && work->pbody->evmobj != NULL ) {
		if ( !( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) ) {
			evm->flag &= ~DG_EVMOBJ_INVISIBLE0 ;
			visible = 1 ;
		}
		if ( !( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE1 ) ) {
			evm->flag &= ~DG_EVMOBJ_INVISIBLE1 ;
			visible = 1 ;
		}
	}

	if ( visible == 0 || work->pbody->objs == NULL ) return ;

   DG_Arm_SwitchEvmBuffer(evm);

	if ( work->pbody->evmobj != NULL ) {
		evm->group_id = work->pbody->evmobj->group_id ;
		src = work->pbody->evmobj->matrix[ work->pbody->evmobj->use_buffer ] ;
		skel = work->pbody->evmobj->def->skeleton ;
		src += work->unit ;
		skel += work->unit ;
		dst = evm->matrix[ evm->use_buffer ] ;
		DG_COPY_MAT( dst, src ) ;

		trans.vx = skel->rt_tx ;
		trans.vy = skel->rt_ty ;
		trans.vz = skel->rt_tz ;
		trans.vw = 1.0F ;
		_sceVu0ApplyMatrix( ( FVECTOR * )dst->m[ 3 ], dst, &trans ) ;

		skel = evm->def->skeleton ;
		trans.vx = -skel->rt_tx ;
		trans.vy = -skel->rt_ty ;
		trans.vz = -skel->rt_tz ;
		trans.vw = 1.0F ;
		_sceVu0ApplyMatrix( ( FVECTOR * )dst->m[ 3 ], dst, &trans ) ;		

	} else {
		evm->group_id = work->pbody->objs->group_id ;
		dst = evm->matrix[ evm->use_buffer ] ;
		skel = evm->def->skeleton ;
		DG_COPY_MAT( dst, &work->pbody->objs->objs[ work->unit ].world ) ;
		trans.vx = -skel->rt_tx ;
		trans.vy = -skel->rt_ty ;
		trans.vz = -skel->rt_tz ;
		trans.vw = 1.0F ;
		_sceVu0ApplyMatrix( ( FVECTOR * )dst->m[ 3 ], dst, &trans ) ;
	} 
}

static	void	Die( Work *work )
{
	if ( work->evmobj != NULL ) {
		DG_DequeueEvmObj( work->evmobj ) ;
		DG_FreeEvmObj( work->evmobj ) ;
	}
	if ( work->mirror_objs != NULL ) {
		DG_DequeueObjs( work->mirror_objs ) ;
		DG_FreeObjs( work->mirror_objs ) ;
		DG_DisconnectObjs( work->pbody->objs, work->mirror_objs ) ;
	}
}

static	int		GetResources( Work *work, CONTROL *pctrl, OBJECT *pbody, 
							  int model, int mirror, int unit ) 
{
	EVM_DEF		*evmdef ;
	DG_DEF		*def ;
	DG_OBJS		*objs ;

	work->pctrl = pctrl ;
	work->pbody = pbody ;
	work->unit = unit ;

	work->model = model ;

	evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( model, 'e' ) ) ;
	if ( evmdef == NULL ) return -1 ;
	work->evmobj = DG_MakeEvmObj( evmdef, DG_EVMOBJ_LATTERDRAW, 0 ) ;
	DG_QueueEvmObj( work->evmobj ) ;
	if ( pbody->objs != NULL ) {
		work->evmobj->light = pbody->objs->light ;
		work->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	} else if ( pbody->evmobj != NULL ) {
		work->evmobj->light = pbody->evmobj->light ;
		work->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	} else return -1 ;

	/* うつりこみ */
	if ( mirror == 0 || pbody->objs == NULL ) return 0 ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( mirror, 'k' ) ) ;
	if ( def == NULL ) return 0 ;
	work->mirror_objs = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
	if ( objs == NULL ) return 0 ;
	DG_QueueObjs( objs ) ;
	objs->light = pbody->objs->light ;
	DG_InvisibleObjs( objs ) ;
	DG_ConnectObjs( pbody->objs, objs ) ;
	objs->root = &pbody->objs->objs[ unit ].world ;

	return 0 ;
}

void			*NewEvmEquip( CONTROL *pctrl, OBJECT *pbody, int model, int mirror, int unit )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof( Work ),
									 PLAYER_SUNGLASSES_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, pctrl, pbody, model, mirror, unit ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;	
}

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX		actor ;
	int				time ;
} WorkD ;

static	int		IsRaiden( OBJECT *body )
{
	int			i ;
	DG_DEF		*def ;
	EVM_DEF		*evmdef ;

	if ( body->evmobj != NULL ) {
		for ( i = 0; i < sizeof( RaidenEvm ) / sizeof( int ); i ++ ) {
			evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( RaidenEvm[ i ], 'e' ) ) ;
			if ( evmdef == NULL ) continue ;
			printf( "body %x - evm %x\n", body->evmobj->def, evmdef ) ;
			if ( body->evmobj->def == evmdef ) {
				printf( "raiden evm found : %d\n", RaidenEvm[ i ] ) ;
				return 1 ;
			}
		}
	} else if ( body->objs != NULL ) {
		for ( i = 0; i < sizeof( RaidenKms ) / sizeof( int ); i ++ ) {
			def = ( DG_DEF * )GV_GetCache( GV_CacheID( RaidenKms[ i ], 'k' ) ) ;
			if ( def == NULL ) continue ;
			if ( body->objs->def == def ) {
				printf( "raiden kms found : %d\n", RaidenKms[ i ] ) ;
				return 1 ;
			}
		}
	}
	return 0 ;
}

static	int		IsSnake( OBJECT *body )
{
	int			i ;
	DG_DEF		*def ;
	EVM_DEF		*evmdef ;

	if ( body->evmobj != NULL ) {
		for ( i = 0; i < sizeof( SnakeEvm ) / sizeof( int ); i ++ ) {
			evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( SnakeEvm[ i ], 'e' ) ) ;
			if ( evmdef == NULL ) continue ;
			if ( body->evmobj->def == evmdef ) {
				printf( "snake evm found : %d\n", SnakeEvm[ i ] ) ;
				return 1 ;
			}
		}
	} else if ( body->objs != NULL ) {
		for ( i = 0; i < sizeof( SnakeKms ) / sizeof( int ); i ++ ) {
			def = ( DG_DEF * )GV_GetCache( GV_CacheID( SnakeKms[ i ], 'k' ) ) ;
			if ( def == NULL ) continue ;
			if ( body->objs->def == def ) {
				printf( "snake kms found : %d\n", SnakeKms[ i ] ) ;
				return 1 ;
			}
		}
	}
	return 0 ;
}

static	int		IsPliskin( OBJECT *body )
{
	int			i ;
	DG_DEF		*def ;
	EVM_DEF		*evmdef ;

	if ( body->evmobj != NULL ) {
		for ( i = 0; i < sizeof( PliskinEvm ) / sizeof( int ); i ++ ) {
			evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( PliskinEvm[ i ], 'e' ) ) ;
			if ( evmdef == NULL ) continue ;
			if ( body->evmobj->def == evmdef ) {
				printf( "pliskin evm found : %d\n", PliskinEvm[ i ] ) ;
				return 1 ;
			}
		}
	} else if ( body->objs != NULL ) {
		for ( i = 0; i < sizeof( PliskinKms ) / sizeof( int ); i ++ ) {
			def = ( DG_DEF * )GV_GetCache( GV_CacheID( PliskinKms[ i ], 'k' ) ) ;
			if ( def == NULL ) continue ;
			if ( body->objs->def == def ) {
				printf( "pliskin kms found : %d\n", PliskinKms[ i ] ) ;
				return 1 ;
			}
		}
	}
	return 0 ;
}

#define RAIDEN_NAME 10163495 //GV_StrCode( "ライデン" )
#define SNAKE_NAME  2677990  //GV_StrCode( "スネーク" )
static	void	MakeSunGlassesAct( WorkD *work )
{
	int			i ;	
	void		*child ;
	CONTROL		*ctrl ;

	for ( i = 0; i < GM_N_WhereList; i ++ ) {
		ctrl = GM_WhereList[ i ] ;
		if ( i == 0 ) {
			if ( ctrl != GM_PlayerControl &&
				 ctrl->name != SNAKE_NAME &&
				 ctrl->name != RAIDEN_NAME ) {
				continue ;
			}
		}
		if ( ctrl->object == NULL ) continue ;
		child = NULL ;
		if ( IsRaiden( ctrl->object ) ) {
			child = NewEvmEquip( ctrl, ctrl->object, 
								 MDL_RAIDEN_GLASSES, MDL_RAIDEN_GLASSES_MIRROR,
								 HUMAN21_ATAMA ) ;
		} else if ( IsSnake( ctrl->object ) ) {
			if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
				child = NewEvmEquip( ctrl, ctrl->object, 
									MDL_SNAKE_GLASSES_BLACK, MDL_SNAKE_GLASSES_BLACK_MIRROR,
									HUMAN21_ATAMA ) ;
			} else {
				child = NewEvmEquip( ctrl, ctrl->object, 
									MDL_SNAKE_GLASSES, MDL_SNAKE_GLASSES_MIRROR,
									HUMAN21_ATAMA ) ;
			}
		} else if ( IsPliskin( ctrl->object ) ) {
			child = NewEvmEquip( ctrl, ctrl->object, 
								 MDL_PLISKIN_GLASSES_BLACK, 0,
								 HUMAN21_ATAMA ) ;
		}
		if ( child != NULL ) {
			GV_SetActorChild( work, child ) ;
		}
	}
}

static	void	ActD( WorkD *work )
{
	if ( ++ work->time == 2 ) {
		MakeSunGlassesAct( work ) ;
		GV_SleepActor( work, GV_CLASS_WAITING ) ;
	}
}

/* サングラス管理起動 */
void	*GM_SunGlassesManager( void )
{
	WorkD		*work ;

	if ( !( GM_ClearFlag & GM_CLEARED_SUNGLASSES_PLAYING ) ) return NULL ;
	work = ( WorkD * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof( WorkD ),
									  PLAYER_SUNGLASSES_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, ActD, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	return work ;
}

