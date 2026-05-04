/*
	envmap.c
	実験環境マッピング

	1999/10/15 K.Takabe
	$Id: envmap.c,v 1.1.1.3 2002/11/19 11:51:09 Yoshizawa1 Exp $

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



/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	FMATRIX		lights[2] ;
	int			model_id ;
	DG_OBJS		*objs ;
	DG_TEX		*tex ;
	CV2_DEF		*cv2_def ;
	DG_TEX_TRANS	tex_trans ;
	DG_VERTS_ANIME	v_anime[0] ;
} Work ;


/* ---------------------------------------------------------------- */
/*----------------------------------------------------------------*/

static inline void _SetMatrix( FMATRIX *m )
{
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
}

static inline void _SetMatrix2( FMATRIX *m )
{
	asm ("
	lqc2		vf16,0x00(%0)
	lqc2		vf17,0x10(%0)
	lqc2		vf18,0x20(%0)
	lqc2		vf19,0x30(%0)
	": : "r"(m) );
}

static inline void _RotTrans( FVECTOR *res, FVECTOR *v )
{
	asm ("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}

static inline void _RotVector( FVECTOR *res, FVECTOR *v )
{
	asm ("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddz.xyzw		vf8, vf6,vf8
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}

static inline void _CalcEnvmap( FVECTOR *res, FVECTOR *v )
{
	asm ("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC,vf4,vf8
	vmadday.xyzw	ACC,vf5,vf8
	vmaddz.xyzw		vf8,vf6,vf8
	vmula.xy		ACC,vf1,vf8
	vmaddw.xy		vf8,vf2,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}

static inline void _CalcEnvmap2( FVECTOR *res, FVECTOR *v )
{
	asm ("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC,vf4,vf8
	vmadday.xyzw	ACC,vf5,vf8
	vmaddz.xyzw		vf11,vf6,vf8
	vmulax.xyzw		ACC,vf16,vf8
	vmadday.xyzw	ACC,vf17,vf8
	vmaddz.xyzw		vf12,vf18,vf8
	vsub.w			vf10,vf0,vf8
	vmula.xy		ACC,vf1,vf11
	vmaddw.xy		vf11,vf2,vf0
	vmula.xy		ACC,vf1,vf12
	vmaddw.xy		vf12,vf2,vf0
	vmulaw.xy		ACC,vf11,vf08
	vmaddw.xy		vf8,vf12,vf10
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}

/* ---------------------------------------------------------------- */
	/*
		エンベロープ用マトリクスの生成
	*/
#if 0
static void GetEnvelopeWorldMatrix( FMATRIX *offset_mat, FMATRIX *base, FMATRIX *target, float *trans )
{
	FVECTOR		trans_vec ;

	trans_vec.vx = trans[0] ;
	trans_vec.vy = trans[1] ;
	trans_vec.vz = trans[2] ;
	trans_vec.vw = 1.0f ;
	*offset_mat = *target ;
	_sceVu0ApplyMatrix( offset_mat->m[3], target, &trans_vec );
}
#endif

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
#if 0
	/* 遅いバージョン */
	int		i, j ;
	FVECTOR	*nvec, *uvs ;
	CV2_MDL	*cv2_mdl ;
	DG_OBJ	*obj ;
	DG_VERTS_ANIME	*v_anime ;
	FMATRIX	mat ;
	float	u_scale, v_scale, u_offset, v_offset ;

	u_scale = work->tex->u_scale ;
	v_scale = work->tex->v_scale ;
	u_offset = work->tex->u_offset ;
	v_offset = work->tex->v_offset ;

	/* ＵＶ値の計算 */
	cv2_mdl = work->cv2_def->models ;
	obj = work->objs->objs ;
	v_anime = work->v_anime ;
	for ( i = work->objs->n_models ; i > 0 ; i-- ){
		/* マトリクスを求める */
		sceVu0MulMatrix( &mat, &( DG_Chanl( work->objs->chanl )->eye_inv ), &obj->world );
		_SetMatrix( &mat );
		/* 計算 */
		nvec = cv2_mdl->norms ;
		uvs = (void*)SCRPAD_ADDR ;
		for ( j = cv2_mdl->n_norms ; j > 0 ; j-- ){
			_RotVector( uvs, nvec );
			uvs->vx = ( ( uvs->vx * 0.4999f ) + 0.5f ) * u_scale + u_offset ;
			uvs->vy = ( ( -uvs->vy * 0.4999f ) + 0.5f ) * v_scale + v_offset ;
			nvec++ ;
			uvs++ ;
		}
		/* DG_VERTS_ANIMEのバッファ切り替え */
		DG_SwitchVAnimeBuffer( v_anime );
		/* 型を変換してスクラッチパッドへ */
		DG_RegistCommonUV( (void*)SCRPAD_ADDR, cv2_mdl->n_norms );
		/* インデックスを参照して頂点を分配 */
		DG_RefineStripUV2( v_anime, cv2_mdl->norms_index );
		cv2_mdl++ ;
		v_anime++ ;
	}
#else
	/* ちょっと最適化バージョン */
	int		i, j ;
	FVECTOR	*nvec, *uvs ;
	CV2_MDL	*cv2_mdl ;
	DG_OBJ	*obj ;
	DG_VERTS_ANIME	*v_anime ;
	FMATRIX	mat, envelope_mat ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	{/* 定数をＶＵ０レジスタへ */
		FVECTOR	vec1, vec2 ;
		vec1.vx = -0.4999f * work->tex->u_scale ;
		vec1.vy = -0.4999f * work->tex->v_scale ;
		vec2.vx = 0.5f * work->tex->u_scale + work->tex->u_offset ;
		vec2.vy = 0.5f * work->tex->v_scale + work->tex->v_offset ;
		asm("
			lqc2		vf1,0(%0)
			lqc2		vf2,0(%1)
		"::"r"(&vec1),"r"(&vec2));
	}

	/* ＵＶ値の計算 */
	cv2_mdl = work->cv2_def->models ;
	obj = work->objs->objs ;
	v_anime = work->v_anime ;
	for ( i = work->objs->n_models ; i > 0 ; i-- ){
		/* マトリクスを求める */
		_sceVu0MulMatrix( &mat, &( DG_Chanl( work->objs->chanl )->eye_inv ), &obj->world );
#if 0
		GetEnvelopeWorldMatrix( &envelope_mat, &obj->world, &work->objs->objs[ obj->model->parent ], &obj->model->tx );
		_sceVu0MulMatrix( &envelope_mat, &( DG_Chanl( work->objs->chanl )->eye_inv ), &envelope_mat );
#else
		if ( !( obj->model->type & DG_TYPE_EXTEND ) ){
			if ( obj->model->parent != -1 ){
				_sceVu0MulMatrix( &envelope_mat, &( DG_Chanl( work->objs->chanl )->eye_inv ),
								&work->objs->objs[ obj->model->parent ].world );
			}
		} else {
			DG_OBJ	*tmp_obj ;
			tmp_obj = &work->objs->objs[ obj->model->parent ] ;
			if ( tmp_obj->model->parent != -1 ){
				_sceVu0MulMatrix( &envelope_mat, &( DG_Chanl( work->objs->chanl )->eye_inv ),
								&work->objs->objs[ tmp_obj->model->parent ].world );
			}
		}
#endif
		_SetMatrix( &mat );
		_SetMatrix2( &envelope_mat );
		/* 計算 */
		nvec = cv2_mdl->norms ;
		uvs = (void*)SCRPAD_ADDR ;
		for ( j = cv2_mdl->n_norms ; j > 0 ; j-- ){
			if ( ( (int)nvec & 0x3f ) == 0 ) asm("pref 0,64(%0)"::"r"(nvec));
			//_CalcEnvmap( uvs++, nvec );
			_CalcEnvmap2( uvs++, nvec );
			nvec++ ;
		}
		/* DG_VERTS_ANIMEのバッファ切り替え */
		DG_SwitchVAnimeBuffer( v_anime );
		/* 型を変換してスクラッチパッドへ */
		DG_RegistCommonUV( (void*)SCRPAD_ADDR, cv2_mdl->n_norms );
		/* インデックスを参照して頂点を分配 */
		DG_RefineStripUV2( v_anime, cv2_mdl->norms_index );
		cv2_mdl++ ;
		v_anime++ ;
		obj++ ;
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int		i ;
	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	for ( i = 0 ; i < work->objs->n_models ; i++ ){
		DG_FreeAnimVertsBuffer( &work->v_anime[i] );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, DG_OBJS *objs, int model_id, int tex_id )
{
	DG_TEX			*tex ;
	CV2_DEF		*cv2_def ;
	int			i, j ;

	work->model_id = model_id ;
	work->objs = objs ;

	/* 共有頂点データの取得 */
	work->cv2_def = cv2_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );
	if ( cv2_def->id == 6754556/* "Common Vertex Data" */ ) return (0);

	/* 環境マッピングテクスチャ取得 */
	//work->tex = tex = DG_GetTexture( 37293/* envmap */ );
	//work->tex = tex = DG_GetTexture( 13797/* envmap3 */ );
	work->tex = tex = DG_GetTexture2( objs->tri_id, tex_id );
	work->tex_trans = tex->tex_trans ;
printf("envmap tex: %d %08x\n", tex_id, tex );
printf("objs:%d %d %d\n", objs->n_models, objs->def->n_models, objs->def->n_x_models );

	{/*  */
		DG_OBJ			*obj ;
		DG_OBJ_PACKET	*packet ;
		DG_MDL			*mdl ;
		DG_VERTS_ANIME	*v_anime ;

		obj = objs->objs ;
		v_anime = work->v_anime ;
		for ( i = objs->n_models ; i > 0 ; obj++, v_anime++, i-- ){
			/* 頂点アニメワーク確保 */
			DG_MakeAnimVertsBuffer( v_anime , obj, DG_VANIME_UVS );
			/* パケットの修正 */
			mdl = obj->model ;
			packet = obj->packets ;
			for ( j = mdl->n_packs ; j > 0 ; packet++, j-- ){
				packet->tex_ptr[0] = &work->tex_trans ;
			}
		}
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewEnvmap( DG_OBJS *objs, int model_id, int tex_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) + sizeof(DG_VERTS_ANIME) * objs->n_models ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, objs, model_id, tex_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
