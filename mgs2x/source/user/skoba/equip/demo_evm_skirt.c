//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	evm_skirt.c
	マルチウェイトエンベロープモデル --> スカート

	2001/05/31 S.Kobayashi
	$Id: demo_evm_skirt.c,v 1.1.1.3 2002/11/19 11:50:15 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <stdarg.h>

#include	"libdg.h"
#include	"gameheader.h"
#include    "../test/etc.h"

#include "bp_matrix.h"

#define MODEL_FLAG	( DG_EVMOBJ_IRREACTION )
#define HOS_FEMALEA_SK_MH (8080099)
#define HOS_FEMALEB_SK_MH (9652153)
#define HOS_FEMALEC_SK_MH (239441)
#define HOS_FEMALED_SK_MH (272209)

typedef	struct	{
	GV_ACT_EX	actor ;
	DG_EVMOBJ	*evmobj ;
	OBJECT      *pObj;
	int         name;  // いまんとこ意味無し
	int			power ;
} Work ;

typedef struct {
	FVECTOR	root ;
	FMATRIX	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[ 64 ] ;
} ScrPadWork ;

// extern 
extern void DG_DequeueEvmObj( DG_EVMOBJ * );
extern int DG_QueueEvmObj( DG_EVMOBJ * );

static	void	Clairvoyance( Work *work )
{
#ifdef DEBUG_MODE
#if 1
	int value ;

	if( GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ] == 255 ){
		work->power ++ ;
	} else {
		work->power -= 8 ;
	}
	
	if ( work->power > 60 ) {
		if ( work->power > 210 ) work->power = 210 ;
		value = 128 - ( (work->power-60)/4 ) ;
	} else {
		if ( work->power < 0 ) work->power = 0 ;
		value = 128 ;
	}

	if ( value  < 100 ) {
		work->evmobj->flag |= DG_EVMOBJ_SEMITRANS ;
	} else {
		work->evmobj->flag &= ~DG_EVMOBJ_SEMITRANS ;
	}
#endif
#endif
}

static void EvmActMotion( OBJECT *pObj , DG_EVMOBJ *evmobj )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;
	FMATRIX     *joints_tmp;
	int         point[ 3 ] = { 0 , 13 , 17 };

	if ( evmobj->root != NULL ){
		scrpad->root_mat = *( evmobj->root );
	} else {
		scrpad->root_mat = evmobj->world ;
	}
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;
	joints_tmp = scrpad->joints;
	for ( i = 0 ; i < 3 ; i++ ){
		*joints_tmp = pObj->objs->objs[ point[ i ] ].world;//evmobj->matrix[ 1 - evmobj->use_buffer ][ point[ i ] ];
		joints_tmp++;
	}
	{/* オブジェクトにマトリクスを設定する */
		FMATRIX			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		EVM_SKEL		*skel ;
		int		i ;

		skel = evmobj->def->skeleton ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = 0 ; i < 3 ; i++ ){
				*skel_mats = *joints;
				scrpad->vec.vx = -skel->rt_tx ;
				scrpad->vec.vy = -skel->rt_ty ;
				scrpad->vec.vz = -skel->rt_tz ;
				_sceVu0ApplyMatrix( &skel_mats->m[3][0], joints, &scrpad->vec );

				skel_mats++;
				skel++ ;
				mats++ ;
				joints++ ;
			}
		}
	}

}
static void Act( Work *pWork )
{
	DG_EVMOBJ	*evmobj ;

	evmobj = pWork->evmobj ;
	evmobj->light = pWork->pObj->objs->light ;
	evmobj->world = pWork->pObj->objs->world ;

	pWork->evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	if ( pWork->pObj->evmobj ) {
	    if ( pWork->pObj->evmobj->flag & DG_EVMOBJ_INVISIBLE &&
		 pWork->pObj->objs->flag & DG_FLAG_INVISIBLE )
		pWork->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	} else {
	    if ( pWork->pObj->objs->flag & DG_FLAG_INVISIBLE )
		pWork->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	}

	/* マトリクスバッファ切り替え */
   DG_Arm_SwitchEvmBuffer(evmobj);
	/* モーションの割り当て */
	EvmActMotion( pWork->pObj , evmobj );
	Clairvoyance( pWork ) ;
	return ;
}

static void Die( Work *work )
{
	DG_DequeueEvmObj( work->evmobj );
	DG_FreeEvmObj( work->evmobj );
}

static int GetResources( Work *work , int strcode )
{
	DG_EVMOBJ	*evmobj ;
	EVM_DEF		*def ;

	if ( work->pObj == NULL ){
		SK_Err("obj return\0");
		return ( -1 );
	}
	
	def = GV_GetCache( GV_CacheID( strcode , 'e' ) );

	work->evmobj = evmobj = DG_MakeEvmObj( def, MODEL_FLAG, 0 );
	if ( evmobj == NULL ){
		SK_Err("evmobj return\0");
		return ( -1 );
	}
	DG_QueueEvmObj( evmobj );

	work->power = 0 ;
	return (0);
}


void *SK_NewEvm_Skirt( OBJECT *pObj , int strcode , int name )
{
	Work		*pWork ;

	pWork = GV_NewActorPrio( GV_ACTOR_USER, sizeof( Work ), 32 ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->pObj = pObj;
		pWork->name = name;
		if ( GetResources( pWork , strcode ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

// シナリオ
void *NewScnEvm_SkirtA( OBJECT *pObj , int name ) // ネーちゃん
{
	return ( SK_NewEvm_Skirt( pObj , HOS_FEMALEA_SK_MH , name ) );
}

void *NewScnEvm_SkirtB( OBJECT *pObj , int name ) // おばちゃん
{
	return ( SK_NewEvm_Skirt( pObj , HOS_FEMALEB_SK_MH , name ) );
}

void *NewScnEvm_SkirtC( OBJECT *pObj , int name ) // 細いひと cti_female_skirt1_mh
{
	return ( SK_NewEvm_Skirt( pObj , HOS_FEMALEC_SK_MH , name ) );
}

void *NewScnEvm_SkirtD( OBJECT *pObj , int name ) // 太いかた cti_female_skirt2_mh
{
	return ( SK_NewEvm_Skirt( pObj , HOS_FEMALED_SK_MH , name ) );
}
