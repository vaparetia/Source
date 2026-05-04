/*
	put_mdl.c
	オブジェクト表示実験プログラム

	2001/03/28 K.Takabe
	$Id: put_mdl.c,v 1.1.1.3 2002/11/19 11:51:30 Yoshizawa1 Exp $

*/
/*

chara モデル表示パフォーマンステスト[NewModelPerformanceTest] $s:name \
	-n $w:Ｘ配置数 $w:Ｚ配置数 \
	-d $w:Ｘ配置間隔 $w:Ｚ配置間隔 \
	-m $s:モデル名 $w:モデルタイプ（０：ＫＭＳ、１：ＥＶＭ） \
	-f $w:フラグ

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdarg.h>

#include	"libdg.h"
#include	"gameheader.h"

#define MODEL_FLAG	(0)




#define MAX_OBJS_X	(3)
#define MAX_OBJS_Z	(3)
#define MAX_OBJS	(MAX_OBJS_X*MAX_OBJS_Z)
typedef struct _benchobj{
	DG_OBJS		*objs ;
	DG_EVMOBJ	*evmobj ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	int			mode ;
} BENCHOBJ ;

typedef	struct	{
	GV_ACT		actor ;
	int			n_x ;
	int			n_z ;
	int			dx ;
	int			dz ;
	int			type ;
	int			model_id ;
	int			flag ;
	FVECTOR		rots[ 55 ];
	BENCHOBJ	objlist[0] ;
} Work ;


extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
} ScrPadWork ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{


}

static void Die( Work *work )
{
	int		i, j ;
	BENCHOBJ	*obj_work ;

	for ( i = 0 ; i < work->n_z ; i++ ){
		for ( j = 0 ; j < work->n_x ; j++ ){
			obj_work = &work->objlist[ j + i * work->n_x ] ;
			if ( work->type == 0 ){
				if ( work->flag & 0x02 ){
					DG_FreePreshade( obj_work->objs );
				}
				DG_DequeueObjs( obj_work->objs );
				DG_FreeObjs( obj_work->objs );
			} else {
				DG_DequeueEvmObj( obj_work->evmobj );
				DG_FreeEvmObj( obj_work->evmobj );
			}
		}
	}
}

static int GetResources( Work *work, int name, int where )
{
	BENCHOBJ	*obj_work ;
	LIT_DEF		*lit_def ;
	int			i, j, k ;

	lit_def = GV_GetCache( GV_CacheID( GV_StrCode( "debug" ), 'l' ) );

	if ( GCL_GetOption( 'm' ) != NULL ){
		work->model_id = GCL_GetNextInt();
		work->type = GCL_GetNextInt();
	}

	if ( GCL_GetOption( 'f' ) != NULL ){
		work->flag = GCL_GetNextInt();
	}

	if ( GCL_GetOption( 'd' ) != NULL ){
		work->dx = GCL_GetNextInt();
		work->dz = GCL_GetNextInt();
	} else {
		work->dx = 1500 ;
		work->dz = 1500 ; 
	}

	for ( i = 0 ; i < work->n_z ; i++ ){
		for ( j = 0 ; j < work->n_x ; j++ ){
			int		num ;
			obj_work = &work->objlist[ j + i * work->n_x ] ;

			num = i + j * work->n_x ;
			work->objlist[num].pos.vx = i * work->dx - work->n_x * work->dx / 2 ;
			work->objlist[num].pos.vz = j * work->dz - work->n_z * work->dz / 2 ;
			work->objlist[num].pos.vy = 0 ;
			DG_SetPos2( &work->objlist[num].pos, &work->objlist[num].rot );

			if ( work->type == 0 ){
				DG_DEF	*def ;
				DG_OBJS	*objs ;
				def = GV_GetCache( GV_CacheID( work->model_id, 'k' ) );
				objs = obj_work->objs = DG_MakeObjs( def, MODEL_FLAG, 0 );
				GM_GroupObject( objs, where );
				objs->rots = work->rots ;
				DG_GetPos( &obj_work->objs->world );
				if ( work->flag & 0x0001 ){
					/* 強制的にシングルテクスチャモデルに */
					objs->flag &= ~DG_FLAG_MULTITEX ;
				}
				if ( work->flag & 0x0002 ){
					/* 強制的にプリシェードに */
					objs->flag &= ~DG_FLAG_MULTITEX ;
					objs->flag |= DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT ;
					for ( k = 0 ; k < objs->n_models ; k++ ){
						objs->objs[ k ].flag |= DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT ;
					}
					DG_MakePreshade( objs, lit_def );
				}
				DG_QueueObjs( objs );
			} else {
				EVM_DEF		*def ;
				DG_EVMOBJ	*evmobj ;
				def = GV_GetCache( GV_CacheID( work->model_id, 'e' ) );
				evmobj = obj_work->evmobj = DG_MakeEvmObj( def, MODEL_FLAG, 0 );
				GM_GroupObject( evmobj, where );
				DG_QueueEvmObj( evmobj );
				DG_GetPos( &evmobj->world );
				for ( k = 0 ; k < evmobj->n_skeleton ; k++ ){
					DG_GetPos( &evmobj->matrix[0][k] );
					DG_GetPos( &evmobj->matrix[1][k] );
				}
			}
		}
	}


	return (0);
}


void *NewModelPerformanceTest( int name, int where )
{
	Work		*work ;
	int			x, z ;

	OPERATOR() ;

	if ( GCL_GetOption( 'n' ) != NULL ){
		x = GCL_GetNextInt();
		z = GCL_GetNextInt();
	} else {
		x = 3 ;
		z = 3 ;
	}

	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) + sizeof(BENCHOBJ) * x * z ) ;
	work->n_x = x ;
	work->n_z = z ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
