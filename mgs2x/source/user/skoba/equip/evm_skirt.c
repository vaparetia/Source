//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	evm_skirt.c
	マルチウェイトエンベロープモデル --> スカート

	2001/05/31 S.Kobayashi
	$Id: evm_skirt.c,v 1.1.1.3 2002/11/19 11:50:16 Yoshizawa1 Exp $

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

#define MODEL_FLAG	( DG_EVMOBJ_IRREACTION )
#define HOS_FEMALEA_SK_MH (8080099)
#define HOS_FEMALEB_SK_MH (9652153)//8080163)

typedef	struct	{
	GV_ACT_EX	actor ;
	DG_EVMOBJ	*evmobj ;
	OBJECT      *pObj;
	int         name;  // いまんとこ意味無し
	int			power ;
} Work ;

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
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

static void EvmActMotion( MOTION_CONTROL *m_ctrl, DG_EVMOBJ *evmobj )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;
	FVECTOR     *joints_tmp;
	int         point[ 3 ] = { 0 , 13 , 17 };

	if ( evmobj->root != NULL ){
		scrpad->root_mat = *( evmobj->root );
	} else {
		scrpad->root_mat = evmobj->world ;
	}
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	joints_tmp = scrpad->joints;
	for ( i = 0 ; i < 3 ; i++ ){
		*joints_tmp = m_ctrl->abs_rots[ point[ i ] ];
		joints_tmp++;
	}
	{/* オブジェクトにマトリクスを設定する */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		EVM_SKEL		*skel ;
		int		i ;

		skel = evmobj->def->skeleton ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = 0 ; i < evmobj->n_skeleton ; i++ ){
				FMATRIX	*parent ;
				if ( i < m_ctrl->n_joints ){
					MT_QuatToMat( mats, joints );
					_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
					scrpad->vec.vx = skel->rt_tx + 0.0f ;
					scrpad->vec.vy = skel->rt_ty + 0.0f ;
					scrpad->vec.vz = skel->rt_tz + 0.0f ;
					parent = &scrpad->mats[ skel->parent ] ;
					_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
					scrpad->vec.vx = -skel->rt_tx ;
					scrpad->vec.vy = -skel->rt_ty ;
					scrpad->vec.vz = -skel->rt_tz ;
					_sceVu0ApplyMatrix( &mats->m[3][0], mats, &scrpad->vec );
					*skel_mats = *mats ;
				} else {
					*mats = *skel_mats = scrpad->mats[ skel->parent ] ;
				}
				skel++ ;
				mats++ ;
				skel_mats++ ;
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
	/* マトリクスバッファ切り替え */
   DG_Arm_SwitchEvmBuffer(evmobj);
	/* モーションの割り当て */
	EvmActMotion( pWork->pObj->m_ctrl, evmobj );
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


static void *NewEvm_Skirt( OBJECT *pObj , int strcode , int name )
{
	Work		*pWork ;

	pWork = ( Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
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

void *NewEvm_SkirtA( OBJECT *pObj )
{
	return ( NewEvm_Skirt( pObj , HOS_FEMALEA_SK_MH , 0 ) );
}

void *NewEvm_SkirtB( OBJECT *pObj )
{
	return ( NewEvm_Skirt( pObj , HOS_FEMALEB_SK_MH , 0 ) );
}
