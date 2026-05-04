//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   uniform.c
   ゴル兵制服

   2000/06/02 M.Sonoyama
   $Id: uniform.c,v 1.1.1.3 2002/11/19 11:50:17 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
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

#define	BODY_NAME		GV_StrCode( "rai_gbs_mt" ) 
#define	BODY_HIGH_NAME	GV_StrCode( "rai_gbs_sh_mt" ) 

//#define	HAND_NAME		GV_StrCode( "snh_def_dummy" ) 	/* 今はスネークで代用 */
//#define	HAND_NAME_EVM	GV_StrCode( "snh_def_mh_mt" ) 	/* 今はスネークで代用 */

#define	HAND_NAME		GV_StrCode( "rah_def_dummy" ) 	
#define	HAND_NAME_EVM	GV_StrCode( "rah_gbs_mh" ) 	

/*------------------------------------------------------------------*/

/* ゴルキャップ主観 */
extern	void *NewGoruRaiSub2D( int mode , int camera_chanl ) ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT_EX	actor ;
	OBJECT		body ;
	OBJECT		body_high ;
	OBJECT		hand ;
	DG_OBJS		*golhead ;
	DG_OBJS		*shadow_obj ;

    CONTROL		*ctrl ;

	OBJECT		*body_org ;
	OBJECT		*hand_org ;
	DG_OBJS		*shadow_obj_org ;

	int			flag_body ;
	int			flag_hand ;
	int			flag_hand_evm ;

	int			cap_subject_on ;

	void		*work2 ;
} Work ;

typedef	struct	{
	GV_ACT		actor ;
	OBJECT		*body_org ;
	OBJECT		*hand_org ;
	int			*flag_body ;
	int			*flag_hand ;
	int			*flag_hand_evm ;
} Work2 ;

/*------------------------------------------------------------------*/

static	void	CopyMatrix_DG_OBJS( DG_OBJS * new, DG_OBJS * org )
{
	int			i ;

	new->group_id = org->group_id ;
	DG_COPY_MAT( &new->world, &org->world ) ;
	new->root = &org->world ;
	for ( i = 0; i < new->n_models; i ++ ) {
		DG_COPY_MAT( &new->objs[ i ].world, &org->objs[ i ].world ) ;
	}
}

static	void	CopyObjectFlag( DG_OBJS * new, DG_OBJS * org )
{
	int			i ;

	for ( i = 0; i < org->n_models; i ++ ) {
		new->objs[ i ].flag = org->objs[ i ].flag ;
	}
}

static	void	CopyEvmMatrix( DG_EVMOBJ * new, DG_EVMOBJ * org )
{
	int			i ;
	FMATRIX		*nmat, *omat ;

	new->group_id = org->group_id ;
	DG_COPY_MAT( &new->world, &org->world ) ;
	omat = org->matrix[ org->use_buffer ] ;
	nmat = new->matrix[ new->use_buffer ] ;
	for ( i = 0; i < new->n_skeleton; i ++, nmat ++, omat ++ ) {
		DG_COPY_MAT( nmat, omat ) ;
	}
}

/*------------------------------------------------------------------*/

static	void	Act( work )
Work			*work ;
{
	int			flag ;
	OBJECT		*body ;

	work->flag_body = flag = work->body_org->objs->flag & ( DG_FLAG_INVISIBLE ) ;

	work->body_high.objs->flag &= ~( DG_FLAG_INVISIBLE | DG_FLAG_SHADOWMAKE ) ;
	work->body_high.objs->flag |= flag & ~DG_FLAG_SHADOWMAKE ;
	work->body.objs->flag &= ~( DG_FLAG_INVISIBLE | DG_FLAG_SHADOWMAKE ) ;
	work->body.objs->flag |= flag ;

	CopyMatrix_DG_OBJS( work->body_high.objs, work->body_org->objs ) ;
	CopyMatrix_DG_OBJS( work->body.objs, work->body_org->objs ) ;
	CopyObjectFlag( work->body_high.objs, work->body_org->objs ) ;
	CopyObjectFlag( work->body.objs, work->body_org->objs ) ;

	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_LOD_HIGH ) ) {
		DG_InvisibleObjs( work->body.objs ) ;
		body = &work->body_high ;
	} else {
		DG_InvisibleObjs( work->body_high.objs ) ;
		body = &work->body ;
	}
	PL_ActionBlurObjs = body->objs ;
	PL_ActionBlurParent = work ;

	DG_InvisibleObjs( work->body_org->objs ) ;
	//work->body_org->objs->flag &= ~DG_FLAG_SHADOWMAKE ;

	work->flag_hand = flag = work->hand_org->objs->flag & DG_FLAG_INVISIBLE ;
	work->hand.objs->flag &= ~DG_FLAG_INVISIBLE ;
	work->hand.objs->flag |= flag ;
	CopyMatrix_DG_OBJS( work->hand.objs, work->hand_org->objs ) ;
	DG_InvisibleObjs( work->hand_org->objs ) ;
	/* EVMの更新 */
	DG_SetPos( &work->hand.objs->world ) ;
	DG_GetPos( &work->hand.evmobj->world ) ;
   DG_Arm_SwitchEvmBuffer(work->hand.evmobj);
	CopyEvmMatrix( work->hand.evmobj, work->hand_org->evmobj ) ;
	work->flag_hand_evm = flag = work->hand_org->evmobj->flag & DG_EVMOBJ_INVISIBLE ;
	work->hand.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	work->hand.evmobj->flag |= flag ;
	work->hand_org->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;

	/* ゴルヘッド */
	if ( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) {
		flag = body->objs->flag & ( DG_FLAG_INVISIBLE0|DG_FLAG_INVISIBLE1 ) ;
		work->golhead->flag &= ~DG_FLAG_INVISIBLE ;
		//work->golhead->flag |= flag | DG_FLAG_SHADOWMAKE ;
		work->golhead->flag |= flag ;
#if 0
		work->body_org->objs->objs[ HUMAN21_KUBI ].flag |= DG_FLAG_INVISIBLE ;
		work->body_org->objs->objs[ HUMAN21_ATAMA ].flag |= DG_FLAG_INVISIBLE ;
		body->objs->objs[ HUMAN21_KUBI ].flag |= DG_FLAG_INVISIBLE ;
		body->objs->objs[ HUMAN21_ATAMA ].flag |= DG_FLAG_INVISIBLE ;
#endif
		PL_VisibleHead( 0, work->body_org->objs, -1 ) ;
		PL_VisibleHead( 0, body->objs, -1 ) ;

		GM_GroupObjs( work->golhead, body->map_name ) ;
		DG_COPY_MAT( &work->golhead->world, 
					&work->body_org->objs->objs[ HUMAN21_MUNE ].world ) ;
		DG_COPY_MAT( &work->golhead->objs[ 0 ].world, 
					&work->body_org->objs->objs[ HUMAN21_MUNE ].world ) ;
		DG_COPY_MAT( &work->golhead->objs[ 1 ].world, 
					&work->body_org->objs->objs[ HUMAN21_KUBI ].world ) ;
		DG_COPY_MAT( &work->golhead->objs[ 2 ].world, 
					&work->body_org->objs->objs[ HUMAN21_ATAMA ].world ) ;
		PL_GolCapObjs = work->golhead ;

		/* 映り込み対策 */
		if ( GM_CheckPlayerStatus( PLAYER_GROUND ) && 
			 GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			work->golhead->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
		} else {
			work->golhead->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE0 ;
		}

		/* 主観制御 */
		if ( !( body->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
			/* プレイヤー表示中 */
			NewGoruRaiSub2D( 1, 0 ) ;
			work->cap_subject_on = 0 ;
		} else if ( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			/* 主観でない */
			NewGoruRaiSub2D( 1, 0 ) ;
			work->cap_subject_on = 0 ;
		} else {
			GM_CameraSet	*next, *cam ;
			
			cam = GM_GetCurrentCamera( 0 ) ;
			next = GM_GetNextCamera( 0 ) ;
			if ( work->cap_subject_on == 0 ) {
				if ( GM_Camera->chanl[ 0 ].time > 0 &&
					 GV_VecLen3F2( &cam->position, &next->position ) > 240.0F ) {
					/* まだカメラが位置についてない */
					NewGoruRaiSub2D( 1, 0 ) ;
				} else {
					NewGoruRaiSub2D( 0, 0 ) ;
					work->cap_subject_on = 1 ;
				}
			} else {
				NewGoruRaiSub2D( 0, 0 ) ;
			}
		}
	} else {
		PL_VisibleHead( 1, work->body_org->objs, -1 ) ;
		PL_VisibleHead( 1, body->objs, -1 ) ;
#if 0
		work->body_org->objs->objs[ HUMAN21_KUBI ].flag &= ~DG_FLAG_INVISIBLE ;
		work->body_org->objs->objs[ HUMAN21_ATAMA ].flag &= ~DG_FLAG_INVISIBLE ;
		body->objs->objs[ HUMAN21_KUBI ].flag &= ~DG_FLAG_INVISIBLE ;
		body->objs->objs[ HUMAN21_ATAMA ].flag &= ~DG_FLAG_INVISIBLE ;
#endif
		//work->golhead->flag &= ~DG_FLAG_SHADOWMAKE ;
		DG_InvisibleObjs( work->golhead ) ;
		PL_GolCapObjs = NULL ;
		NewGoruRaiSub2D( 1, 0 ) ;
	}
}

static	void	Die( work )
Work			*work ;
{
	GM_FreeObject( &work->body ) ;
	GM_FreeObject( &work->body_high ) ;
	GM_FreeObject( &work->hand ) ;
	if ( work->golhead != NULL ) {
		DG_DequeueObjs( work->golhead ) ;
		DG_FreeObjs( work->golhead ) ;
	}
	if ( GM_PlayerBody != NULL ) {
		work->body_org->objs->flag |= DG_FLAG_SHADOWMAKE ;
		PL_VisibleHead( 1, work->body_org->objs, -1 ) ;
#if 0
		work->body_org->objs->objs[ HUMAN21_KUBI ].flag &= ~DG_FLAG_INVISIBLE ;
		work->body_org->objs->objs[ HUMAN21_ATAMA ].flag &= ~DG_FLAG_INVISIBLE ;
#endif
	}

//	GV_DestroyActorQuick( work->work2 ) ;
	GV_DestroyOtherActor( work->work2 ) ;

	NewGoruRaiSub2D( -1, 0 ) ;

	PL_GolUniformObjs = NULL ;
	PL_GolCapObjs = NULL ;

	/* 本体用影モデルを戻す */
	if ( work->shadow_obj != NULL ) DG_FreeObjs( work->shadow_obj ) ;
	DG_SetLowObjs( work->body_org->objs, work->shadow_obj_org ) ;
}

static	void	Act2( work )
Work2			*work ;
{
	if ( GM_PlayerBody == NULL ) return ;
	work->body_org->objs->flag &= ~( DG_FLAG_INVISIBLE ) ;
	work->body_org->objs->flag |= *( work->flag_body ) ;

	work->hand_org->objs->flag &= ~DG_FLAG_INVISIBLE ;
	work->hand_org->objs->flag |= *( work->flag_hand ) ;

	work->hand_org->evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	work->hand_org->evmobj->flag |= *( work->flag_hand_evm ) ;

	if ( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) {
		PL_VisibleHead( 0, work->body_org->objs, -1 ) ;
#if 0
		work->body_org->objs->objs[ HUMAN21_KUBI ].flag |= DG_FLAG_INVISIBLE ;
		work->body_org->objs->objs[ HUMAN21_ATAMA ].flag |= DG_FLAG_INVISIBLE ;
#endif
	}
}

/*------------------------------------------------------------------*/

static	int	GetResources( work ) 
Work		*work ;
{
	OBJECT		*org ;
	DG_DEF		*def ;
	DG_OBJS		*objs ;

	org = work->body_org = GM_PlayerBody ;
	GM_InitObject( &work->body, BODY_NAME, org->objs->flag & ~( DG_FLAG_INVISIBLE | DG_FLAG_SHADOWMAKE ) ) ;
	if ( work->body.objs == NULL ) return -1 ;
	GM_ConfigObjectLight( &work->body, org->objs->light ) ;

	GM_InitObject( &work->body_high, BODY_HIGH_NAME, 
				  ( org->objs->flag & ~( DG_FLAG_INVISIBLE | DG_FLAG_SHADOWMAKE ) ) ) ;
	if ( work->body_high.objs == NULL ) return -1 ;
	GM_ConfigObjectLight( &work->body_high, org->objs->light ) ;

	work->flag_body = org->objs->flag & ( DG_FLAG_INVISIBLE ) ;

	org = work->hand_org = GM_PlayerArmBody ;
	GM_InitObject( &work->hand, HAND_NAME, org->objs->flag & ~( DG_FLAG_INVISIBLE | DG_FLAG_SHADOWMAKE ) ) ;
	if ( work->hand.objs == NULL ) return -1 ;
	GM_ConfigObjectEvm( &work->hand, HAND_NAME_EVM, 0 ) ;
	GM_ConfigObjectLight( &work->hand, org->objs->light ) ;
	work->flag_hand = org->objs->flag & DG_FLAG_INVISIBLE ;
	work->flag_hand_evm = org->evmobj->flag & DG_EVMOBJ_INVISIBLE ;	

	/* ゴルヘッド */
	{
		DG_DEF		*def ;
		DG_OBJS		*objs ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "rai_gbs_gbshead" ), 'k' ) ) ;
		if ( def != NULL ) {
			objs = work->golhead = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
											   DG_FLAG_IRREACTION, 0 ) ;
			if ( objs == NULL ) return -1 ;
			DG_QueueObjs( objs ) ;
			DG_SetLightMatrix( objs, work->body.objs->light ) ;
			if ( !( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ) {
				DG_InvisibleObjs( objs ) ;
			}
		}
	}
	NewGoruRaiSub2D( 0, 0 ) ;
	NewGoruRaiSub2D( 1, 0 ) ;


	/* 本体用影モデルをすげかえ */
	work->shadow_obj_org = work->body_org->objs->low ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( BODY_NAME, 'k' ) ) ;
	objs = work->shadow_obj = DG_MakeObjs( def, DG_FLAG_FINISHCALC, 0 ) ;
	if ( objs == NULL ) return -1 ;
	work->body_org->objs->low = NULL ;
	DG_SetLowObjs( work->body_org->objs, objs ) ;
	if ( work->body_org->objs->low == NULL ) {
		/* 接続失敗 */
		printf( "golshadow failed! %d %d\n", work->body_org->objs->n_models,
			     objs->n_models ) ;
		DG_FreeObjs( objs ) ;
		work->shadow_obj = NULL ;
		DG_SetLowObjs( work->body_org->objs, work->shadow_obj_org ) ;
	}

	PL_GolUniformObjs = work->body.objs ;
	PL_GolCapObjs = NULL ;

	return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewUniform( ctrl, body, unit, trigger )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
{
    Work	*work ;
	Work2	*work2 ;

	if ( GM_PlayerBody == NULL ) return NULL ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_UNIFORM_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
	} else return NULL ;

    work2 = ( Work2 * )GV_CreateActor( GV_ACTOR_PREV2, GV_CLASS_OBJECT, 
									  sizeof( Work2 ), PLAYER_UNIFORM_ACTOR2_PRIO ) ;
    if ( work2 != NULL ) {
		GV_SetActor( &( work2->actor ), Act2, NULL ) ;
		work2->body_org = work->body_org ;
		work2->hand_org = work->hand_org ;
		work2->flag_body = &work->flag_body ;
		work2->flag_hand = &work->flag_hand ;
		work2->flag_hand_evm = &work->flag_hand_evm ;
    } else return NULL ;
	work->work2 = work2 ;
    return work ;
}

int		PL_COM_GolCapOn( void )
{
	GM_PlayerStateFlag |= PL_GBSCAP_EXIST ;
	return 0 ;
}

int		PL_COM_GolCapOff( void )
{
	GM_PlayerStateFlag &= ~PL_GBSCAP_EXIST ;
	return 0 ;
}
