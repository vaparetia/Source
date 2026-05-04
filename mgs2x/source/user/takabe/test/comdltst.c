/*
	comdltst.c
	複製オブジェクト表示実験プログラム

	1999/07/07 K.Takabe
	$Id: comdltst.c,v 1.1.1.3 2002/11/19 11:51:29 Yoshizawa1 Exp $

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

#include	"libdg.h"

#if 0
/* 半透明（オーバーレイ）にしたい場合 */
#define	COMDL_FLAG	(DG_COMDL_SEMITRANS)
/* フォグをかけたくない場合 */
#define	COMDL_FLAG	(DG_COMDL_NOFOG)
/* 一時的に表示を消したい場合 */
#define	COMDL_FLAG	(DG_COMDL_INVISIBLE)
#endif

#define	COMDL_FLAG	(DG_COMDL_DEFAULT)
#define N_OBJS	(1000)

typedef	struct	{
	GV_ACT		actor ;
	DG_COMDL	*comdl ;
	FVECTOR		pos[N_OBJS] ;
	SVECTOR		rot[N_OBJS] ;
} Work ;

static void Act( Work *work )
{
	int		i ;

	/* アニメーション実験（激重） */
	for ( i = 0 ; i < N_OBJS ; i++ ){
		work->pos[i].vy += work->pos[i].vw ;
		if ( work->pos[i].vy < 0.0f ){
			work->pos[i].vy = 0.0f ;
			work->pos[i].vw = -work->pos[i].vw ;
		}
		work->pos[i].vw -= 2.0f ;
		work->rot[i].vx += work->rot[i].pad ;
		work->rot[i].vy += work->rot[i].pad ;
		work->rot[i].vz += work->rot[i].pad ;
		DG_SetPos2( &work->pos[i], &work->rot[i] );
		DG_GetPos( &work->comdl->pos[i].world );
		//printf("%f %f %f %f\n", work->pos[i].vx, work->pos[i].vy, work->pos[i].vz, work->pos[i].vw );
		//printf("%d %d %d %d\n", work->rot[i].vx, work->rot[i].vy, work->rot[i].vz, work->rot[i].pad );
	}

}

static void Die( Work *work )
{
	DG_DequeueComdlObjs( work->comdl );
	DG_FreeComdl( work->comdl );
}

static int GetResources( Work *work )
{
	DG_COMDL	*comdl ;
	DG_DEF		*def ;
	int			i ;

	def = GV_GetCache( GV_CacheID( GV_StrCode("usp_emb"), 'k' ) );

	/* 通常モデルファイル(*.kms)中の任意のDG_OBJPACKを表示モデルとして初期化を行なう */
	work->comdl = comdl = DG_MakeComdl( def->models[0].packs, COMDL_FLAG, N_OBJS, 0 );
	DG_QueueComdlObjs( comdl );

	for ( i = 0 ; i < N_OBJS ; i++ ){
		comdl->pos[i].world = DG_UnitMatrix ;
		comdl->pos[i].world.m[3][0] = 0.0f ;
		comdl->pos[i].world.m[3][1] = (float)i * 500.0f + 2000 ;
		comdl->pos[i].world.m[3][2] = 13000.0f ;
		comdl->pos[i].world.m[0][0] = 10.0f ;
		comdl->pos[i].world.m[1][1] = 10.0f ;
		comdl->pos[i].world.m[2][2] = 10.0f ;
		/* 整数型なので注意！ */
		comdl->pos[i].color.vx = 128 ;
		comdl->pos[i].color.vy = 128 ;
		comdl->pos[i].color.vz = 128 ;
		comdl->pos[i].color.vw = 128 ;

		/* アニメーション実験 */
		work->pos[i].vx = (float)( ( ( (rand()>>16) * 4000 ) >> 15 ) - 2000 ) ;
		work->pos[i].vy = (float)( ( ( (rand()>>16) * 400 ) >> 15 ) );
		work->pos[i].vz = (float)( ( ( (rand()>>16) * 4000 ) >> 15 ) - 2000 ) ;
		work->pos[i].vw = (float)( ( ( (rand()>>16) * 40 ) >> 15 ) - 20 ) ;
		work->rot[i].vx = ( ( (rand()>>16) * 4096 ) >> 15 ) ;
		work->rot[i].vy = ( ( (rand()>>16) * 4096 ) >> 15 ) ;
		work->rot[i].vz = ( ( (rand()>>16) * 4096 ) >> 15 ) ;
		work->rot[i].pad = ( ( (rand()>>16) * 80 ) >> 15 ) - 40 ;
	}

	return (0);
}


void *NewComdlObjectTest( void )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
