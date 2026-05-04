/*
	m_weight.c
	マルチウェイト実験プログラム

	1999/09/01 K.Takabe
	$Id: m_weight.c,v 1.1.1.3 2002/11/19 11:51:29 Yoshizawa1 Exp $

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
#include	"libdg.cnf"
#include	"libmt.h"
#include	"def_dma.h"
#include	"gameheader.h"

typedef	struct	{
	GV_ACT		actor ;
	DG_DEF		*def ;
	DG_MDL		*mdl ;
	DG_OBJS		*objs ;
	DG_VERTS_ANIME	v_anime ;
	int			n_verts ;
	int			n_index ;
	FVECTOR		*org_buffer ;
	FVECTOR		*buffer ;
	FVECTOR		*normal ;
	short		*v_index ;
	FVECTOR		*weight ;
	char		*mat_index ;
	int			count ;
	FMATRIX		skeleton[64] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		rots[64] ;
} Work ;


/* 同じ頂点を検索 */
static int SearchVertex( DG_MDL *mdl, int n_packs, int n_verts, SVECTOR *org )
{
	int		k, l, total_num ; 
	DG_OBJPACK	*pack ;
	SVECTOR		*vec ;

	total_num = 0 ;
	pack = mdl->packs ;
	for ( k = 0 ; k <= n_packs ; k++ ){
		vec = (SVECTOR*)pack->verts ;
		if ( k == n_packs ){
			l = n_verts ;
		} else {
			l = pack->n_verts ;
		}
		for (  ; l > 0 ; l-- ){
			if ( *(u_long*)vec == *(u_long*)org ){
				/* 同一頂点発見 */
				return ( total_num );
			}
			vec++ ;
			total_num++ ;
		}
		total_num += pack->n_verts & 0x01 ;
		pack++ ;
	}
	return ( -1 );
}

/* 共有頂点情報を作成 */
static void GetVertexWork( DG_OBJ *obj, FVECTOR **org_buffer, short **v_index_buffer, int *n_org_verts, int *n_list )
{
	DG_OBJPACK	*pack ;
	SVECTOR		*vec ;
	int			i, j, v_size, wk_size, index, total ;
	short		*v_index ;
	FVECTOR		*fverts ;
	DG_MDL		*mdl ;

	mdl = obj->model ;

	/* ワークの最大を求める */
	wk_size = 0 ;
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		wk_size += ( pack->n_verts + 1 ) & 0xfe ;
		pack++ ;
	}
	*n_list = wk_size ;
	v_index = *v_index_buffer = GV_Malloc( sizeof(short) * wk_size );
printf("============== %d\n", wk_size );

	/* 共有頂点にした場合の頂点数を求める（遅い！！） */
	v_size = 0 ;
	pack = mdl->packs ;
	v_index = *v_index_buffer ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (SVECTOR*)pack->verts ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = SearchVertex( mdl, i, j, vec );
			*v_index = index ;
			if ( index == -1 ) v_size++ ;
			vec++ ;
			v_index++ ;
		}
		if ( pack->n_verts & 1 ){
			*v_index = 0 ;
			v_index++ ;
		}
		pack++ ;
	}
	*n_org_verts = v_size ;
	fverts = *org_buffer = (void*)( ( (int)GV_Malloc( sizeof(FVECTOR) * v_size + 64 ) + 63 ) & ~63 );
	//fverts = *org_buffer = (int)GV_Malloc( sizeof(FVECTOR) * v_size + 64 ) ;
printf("============== %d\n", v_size );

	total = 0 ;
	pack = mdl->packs ;
	v_index = *v_index_buffer ;
	for ( i = 0 ; i < mdl->n_packs ; i++ ){
		vec = (SVECTOR*)pack->verts ;
		for ( j = 0 ; j < pack->n_verts ; j++ ){
			index = *v_index ;
			if ( index == -1 ){
				fverts[ total ].vx = vec->vx ;
				fverts[ total ].vy = vec->vy ;
				fverts[ total ].vz = vec->vz ;
				fverts[ total ].vw = (float)vec->pad / 4096.0f ;
				*v_index = total ;
				total++ ;
			} else {
				*v_index = (*v_index_buffer)[index] ;
			}
			vec++ ;
			v_index++ ;
		}
		if ( pack->n_verts & 1 ){
			v_index++ ;
		}
		pack++ ;
	}

}


static void Act( Work *work )
{
	register int		a, b ;
#if 0
	if ( GV_PadData[0].status & PAD_A ){
		FVECTOR	*dst, *src ;
		int		i ;
		float	scale ;

		work->count = ( work->count + 1 ) & 255  ;
		scale = sinf( (float)work->count / 128.0f * (float)M_PI )/2.0f + 0.5f ;
		src = work->buffer ;
		dst = work->org_buffer ;
		for ( i = work->n_verts ; i > 0 ; i-- ){
			dst->vx = src->vx + src->vx * src->vy / 100.f * scale ;
			dst->vy = src->vy + src->vy * src->vy / 100.f * scale ;
			dst->vz = src->vz + src->vz * src->vy / 100.f * scale ;
			src++ ;
			dst++ ;
		}
	}
#endif
	/* マルチウェイト計算実験 */
	DG_StartMultiWeightSupport();

	work->pos.vy = 0 ;
	work->rot.vy = 0 ;
	DG_SetPos2( &work->pos, &work->rot );
	DG_GetPos( &work->skeleton[0] );
	work->pos.vy = 500 ;
	work->pos.vz = 300 ;
	work->pos.vx = GM_PlayerPosition.vx - work->objs->world.m[3][0] ;
	work->pos.vy = GM_PlayerPosition.vy - work->objs->world.m[3][1] ;
	work->pos.vz = GM_PlayerPosition.vz - work->objs->world.m[3][2] ;
	work->rot.vy = 512 ;
	DG_SetPos2( &work->pos, &work->rot );
	DG_GetPos( &work->skeleton[1] );

	DG_StoreSkeletonMatrix( work->skeleton, 2 );

	DG_TransMultiWeightVertexNormal( work->org_buffer, work->normal, work->buffer, work->normal, work->weight, work->mat_index, work->n_verts );

	//VSync();
	GV_SET_PRFC_CLOCK();
	FlushCache( 0 );
	DG_RegistCommonVertex( work->org_buffer, work->n_verts );
	a = GV_GET_PRFC_CLOCK();
	GV_SET_PRFC_CLOCK();
	DG_RefineStripVertex( &work->v_anime, work->v_index );
	b = GV_GET_PRFC_CLOCK();
	if ( GV_PadData[0].press & PAD_B ) printf("0:%d %d\n",a, b);
}

static void Die( Work *work )
{
	DG_FreeAnimVertsBuffer( &work->v_anime );
	DG_DequeueObjs( work->objs );
	DG_FreeObjs( work->objs );
}

static int GetResources( Work *work )
{
	int		i ;
	FVECTOR	*src, *dst ;
	DG_DEF	*def ;
	float	max_size ;

	/* オブジェクトの初期化 */
	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode( "clos_16" ), 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 );
	DG_QueueObjs( work->objs );
	DG_SetPos2( &DG_ZeroVector,&DG_ZeroSVector);
	DG_PutObjs( work->objs );
	max_size = sqrtf( work->objs->def->ux * work->objs->def->ux + work->objs->def->uz * work->objs->def->uz );
	work->objs->def->ux = 5000 ;
	work->objs->def->uy = 5000 ;
	work->objs->def->uz = 5000 ;
	work->objs->def->lx = -5000 ;
	work->objs->def->ly = -5000 ;
	work->objs->def->lz = -5000 ;
	work->objs->def->models[0].ux = 5000 ;
	work->objs->def->models[0].uy = 5000 ;
	work->objs->def->models[0].uz = 5000 ;
	work->objs->def->models[0].lx = -5000 ;
	work->objs->def->models[0].ly = -5000 ;
	work->objs->def->models[0].lz = -5000 ;

	/* 頂点アニメーション用バッファ確保 */
	DG_MakeAnimVertsBuffer( &work->v_anime, &work->objs->objs[0], DG_VANIME_VERTS );

	/* 共有頂点データの作成 */
	GetVertexWork( 
				  &work->objs->objs[0],
				  &work->org_buffer, &work->v_index,
				  &work->n_verts, &work->n_index );

	/* 共有頂点データをコピーしておく */
	work->buffer = GV_Malloc( sizeof(FVECTOR) * work->n_verts );
	dst = work->buffer ;
	src = work->org_buffer ;
	for ( i = work->n_verts ; i > 0 ; i-- ){
		*dst++ = *src++ ;
	}

	work->normal = GV_Malloc( sizeof(FVECTOR) * work->n_verts );

	work->weight = GV_Malloc( sizeof(FVECTOR) * work->n_verts );
	work->mat_index = GV_Malloc( sizeof(int) * work->n_verts );

	for ( i = 0 ; i < work->n_verts ; i++ ){
		float		r ;
#define FABS(_f) (((_f)<0.0f)?(-_f):(_f))
		r = sqrtf( work->buffer[i].vx*work->buffer[i].vx + work->buffer[i].vz*work->buffer[i].vz );
		work->weight[i].vx = FABS(r) ;
		work->weight[i].vy = max_size -  FABS(r) ;
		work->weight[i].vz = 0.0f ;
		work->weight[i].vw = 0.0f ;
		MT_QuatNormalize( &work->weight[i], &work->weight[i] );
		work->mat_index[ i * 4 + 0 ] = 0 ;
		work->mat_index[ i * 4 + 1 ] = 1 ;
		work->mat_index[ i * 4 + 2 ] = 2 ;
		work->mat_index[ i * 4 + 3 ] = 3 ;
	}

	work->objs->world.m[3][0] = 0 ;
	work->objs->world.m[3][1] = 500 ;
	work->objs->world.m[3][2] = 16000 ;


	return (0);
}


void *NewModelTestProgram( void )
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
