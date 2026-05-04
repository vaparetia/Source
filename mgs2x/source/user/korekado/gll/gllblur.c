//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	actblur.c
	アクションブラーエフェクト
	をぱくって
	gllblur.c

	2002/08/22 Y.Korekado
	$Id: gllblur.c,v 1.2 2002/11/25 01:54:46 Yoshizawa1 Exp $
*/
/*

＜プログラム呼び出しインターフェイス＞
	void *NewActionBlurEffect( OBJECT *body, u_long64 mask, int *enable_flag )
	OBJECT	*body ;			ブラーをかけるオブジェクト
	u_long64	mask ;			関節単位の表示・非表示
	int		enable_flag ;	フラグ（0:非表示 1:親オブジェクトと同じ 2:強制表示）


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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

//#include "../other/vec_util.h"

#define MAX_N_BLUR		(1)
#define	VISBLE_COUNT	(3)
#define DEF_BLUR_NUM	(15)
/* ---------------------------------------------------------------- */
void *NewGllBlurEffect2( DG_OBJS *org_objs, FVECTOR *org_abs_rots, u_long64 mask, int *enable_flag, int count ) ;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	FVECTOR		*org_abs_rots ;
	DG_OBJS		*org_objs ;
	int			n_joints ;
 	u_long64		mask ;
	DG_OBJS		*objs[ MAX_N_BLUR ] ;
	FMATRIX		old_world ;
	int			*enable_flag ;
	int			count ;
	int			vis_count ;
	FVECTOR		old_abs_rots[0] ;
} Work ;


/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[128] ;
	FVECTOR	trans[128] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[128] ;
} ScrpadWork ;
/* ---------------------------------------------------------------- */

static void ActMotion2( DG_OBJS *objs )
{
	ScrpadWork	*scrpad  = SCRPAD_ADDR ;


	{/* オブジェクトにマトリクスを設定する */
		/* 関節移動量なし */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj = objs->objs ;
		int		i ;

		def = objs->def ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
				FMATRIX	*parent ;
				mdl = obj->model ;
				MT_QuatToMat( mats, joints );
				_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
				scrpad->vec = obj->trans ;
				parent = &scrpad->mats[ mdl->parent ] ;
				_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
				obj->world = *mats ;
				obj++ ;
				mats++ ;
				joints++ ;
			}
		}
	}


}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i, j, flag, vis_flag ;
	float	t ;
	ScrpadWork	*scrpad  = SCRPAD_ADDR ;
	FVECTOR		*org_abs_rots ;
	FMATRIX		*root ;

	/* 各種フラグの設定 */
	flag = work->org_objs->flag ;
	flag &= ~DG_FLAG_MULTITEX ;
	flag &= ~DG_FLAG_SHADOWMAKE ;
	flag |= DG_FLAG_SEMITRANS ;

#if 0
	if ( !( *work->enable_flag ) ){
		flag |= DG_FLAG_INVISIBLE ;
	}
#else
	vis_flag = *work->enable_flag ;
	switch ( vis_flag ){
	  case 0:
		if ( work->vis_count > work->count ) {
			work->vis_count -- ;
		} else {
			flag |= DG_FLAG_INVISIBLE ;
			work->vis_count -- ;
			if ( work->vis_count < 0 ) work->vis_count = 0 ;
			else vis_flag = 1 ;
		}
		break ;
	  case 1:
		if ( work->vis_count < (DEF_BLUR_NUM - work->count) ) {
			flag |= DG_FLAG_INVISIBLE ;
			work->vis_count ++ ;
		} else {
			work->vis_count = DEF_BLUR_NUM ;
		}
		break ;
	  case 2:
		flag &= ~DG_FLAG_INVISIBLE ;
		break ;
	}
#endif
	/* 光学迷彩時には表示を消す */
	if ( flag & DG_FLAG_OPTCMF ){
		flag &= ~DG_FLAG_OPTCMF ;
		flag |= DG_FLAG_INVISIBLE ;
	}

	if ( work->org_objs->root == NULL ){
		root = &work->org_objs->world ;
	} else {
		root = work->org_objs->root ;
	}

	/* 関節マトリクスの設定 */
	org_abs_rots = work->org_abs_rots ;
	for ( i = 0 ; i < MAX_N_BLUR ; i++ ){
		if ( work->objs[ i ] != NULL ){
			t = (float)( i + 1 ) / ( MAX_N_BLUR ) ;

			if ( vis_flag ){
				/* 表示中のみ処理する */
				scrpad->root_mat = *root ;
				_sceVu0InterVector( (FVECTOR*)scrpad->root_mat.m[3],
								(FVECTOR*)scrpad->root_mat.m[3], (FVECTOR*)work->old_world.m[3], t );
				work->objs[ i ]->world = scrpad->root_mat ;
				for ( j = 0 ; j < work->n_joints ; j++ ){
					MT_QuatSlerp( &scrpad->joints[ j ], &org_abs_rots[ j ], &work->old_abs_rots[ j ], t );
					MT_QuatNormalize( &scrpad->joints[ j ], &scrpad->joints[ j ] );
				}
				ActMotion2( work->objs[ i ] );
			}
			/* モデルのフラグ再設定 */
			work->objs[ i ]->flag = flag ;
		}
	}

	/* 現在の状態を保存 */
	if ( work->count ){
		for ( j = 0 ; j < work->n_joints ; j++ ){
			work->old_abs_rots[ j + work->n_joints ] = work->old_abs_rots[ j ] ;
		}
	}
	work->old_world = *root ;
	for ( j = 0 ; j < work->n_joints ; j++ ){
		work->old_abs_rots[ j ] = org_abs_rots[ j ] ;
		MT_QuatNormalize( &work->old_abs_rots[ j ], &work->old_abs_rots[ j ] );
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int		i ;

	for ( i = 0 ; i < MAX_N_BLUR ; i++ ){
printf("ddddddddddddddddddddddddddddddd work->count [%d]\n",work->count ) ;
		if ( !(work->count%VISBLE_COUNT) ) {
			DG_DequeueObjs( work->objs[ i ] );
		}
		DG_FreeObjs( work->objs[ i ] );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, DG_OBJS *org_objs, FVECTOR *org_abs_rots, u_long64 mask, int *enable_flag, int count )
{
	DG_OBJS		*objs ;
	int			i, j, flag ;

	//work->body = body ;
	work->org_objs = org_objs ;
	work->org_abs_rots = org_abs_rots ;
	work->mask = mask ;
	work->enable_flag = enable_flag ;
	work->vis_count = 0 ;

	flag = org_objs->flag ;
	flag &= ~DG_FLAG_INVISIBLE ;
	for ( i = 0 ; i < MAX_N_BLUR ; i++ ){
		work->objs[ i ] = objs = DG_MakeObjs( org_objs->def, flag, 0 );
		if ( objs != NULL ){
			objs->flag |= DG_FLAG_INVISIBLE ;
			if ( !(count%VISBLE_COUNT) ) {
				DG_QueueObjs( objs );
			}
			DG_SetLightMatrix( objs, org_objs->light );
			for ( j = 0, mask = work->mask ; j < objs->def->n_x_models ; j++, mask>>=1 ){
				if ( !( mask & 1 ) ){
					objs->objs[ j ].flag |= DG_FLAG_INVISIBLE ;
				}
			}
		}
	}

printf("ddddddddddddddddddddddddddddddd work->count [%d]\n",count ) ;

	work->old_world = org_objs->world ;
	for ( i = 0 ; i < work->n_joints ; i++ ){
		work->old_abs_rots[ i ] = org_abs_rots[ i ] ;
		MT_QuatNormalize( &work->old_abs_rots[ i ], &work->old_abs_rots[ i ] );
	}

	work->count = count ;
	if ( count ){
		for ( i = 0 ; i < work->n_joints ; i++ ){
			work->old_abs_rots[ i + work->n_joints ] = work->old_abs_rots[ i ] ;
		}
		NewGllBlurEffect2( work->objs[MAX_N_BLUR-1], work->old_abs_rots + work->n_joints,
							 work->mask, enable_flag, count-1 );
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewGllBlurEffect( OBJECT *body, u_long64 mask, int *enable_flag )
{
	return NewGllBlurEffect2( body->objs, body->m_ctrl->abs_rots, mask, enable_flag, DEF_BLUR_NUM );
}

void *NewGllBlurEffect2( DG_OBJS *org_objs, FVECTOR *org_abs_rots, u_long64 mask, int *enable_flag, int count )
{
	Work		*work ;
	int			n_joint ;

	OPERATOR() ;

	n_joint = org_objs->def->n_models ;

	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_OBJECT,
								  sizeof( Work ) + sizeof(FVECTOR) * n_joint * ( (count)?2:1 ), 200-count ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->n_joints = n_joint ;
		if ( GetResources( work, org_objs, org_abs_rots, mask, enable_flag, count ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
