/*
	camofmap.c
	実験スクロールアニメーションマッピング

	1999/11/10 K.Takabe
	$Id: camofmap.c,v 1.1.1.3 2002/11/19 11:51:08 Yoshizawa1 Exp $

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
	DG_VERTS_ANIME	v_anime[22] ;
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
	vmaddaz.xyzw	ACC,vf6,vf8
	vmaddw.xyzw		vf11,vf7,vf0
	vmulax.xyzw		ACC,vf16,vf8
	vmadday.xyzw	ACC,vf17,vf8
	vmaddaz.xyzw	ACC,vf18,vf8
	vmaddw.xyzw		vf12,vf19,vf0
	vsub.w			vf10,vf0,vf8
	vmula.xy		ACC,vf1,vf11
	vmaddw.xy		vf11,vf2,vf0
	vmula.xy		ACC,vf1,vf12
	vmaddw.xy		vf12,vf2,vf0
	vmulaw.xy		ACC,vf11,vf8
	vmaddw.xy		vf8,vf12,vf10
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}

static inline void _CalcEnvmap3( FVECTOR *res, FVECTOR *v )
{
	asm ("
	lqc2			vf8,0x00(%1)
	vsub.w			vf9,vf0,vf8
	vmulax.xyzw		ACC,vf16,vf8
	vmadday.xyzw	ACC,vf17,vf8
	vmaddaz.xyzw	ACC,vf18,vf8
	vmaddw.xyz		vf9,vf19,vf0
	vmulaw.xyz		ACC,vf8,vf8
	vmaddw.xyz		vf8,vf9,vf9
	vmulax.xyzw		ACC,vf4,vf8
	vmadday.xyzw	ACC,vf5,vf8
	vmaddaz.xyzw	ACC,vf6,vf8
	vmaddw.xyzw		vf9,vf7,vf0
	vdiv			Q,vf0w,vf9w
	vwaitq
	vmulq			vf9,vf9,Q
	vmula.xy		ACC,vf1,vf9
	vmaddw.xy		vf8,vf2,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}

/* ---------------------------------------------------------------- */
#if 0
	/*
		エンベロープ用マトリクスの生成
	*/
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

	/*
		エンベロープ用補正マトリクスの生成
	*/
static void GetEnvelopeMatrix( FMATRIX *offset_mat, FMATRIX *base, FMATRIX *target, float *trans )
{
	FMATRIX	mat1, mat2 ;
	FVECTOR	tmp_vec ;
	tmp_vec.vx = trans[0] ;
	tmp_vec.vy = trans[1] ;
	tmp_vec.vz = trans[2] ;
	tmp_vec.vw = 1.0f ;
	mat1 = *base ;
	mat2 = *target ;
	mat1.m[3][0] = 0.0F ; mat1.m[3][1] = 0.0F ; mat1.m[3][2] = 0.0F ; 
	mat2.m[3][0] = 0.0F ; mat2.m[3][1] = 0.0F ; mat2.m[3][2] = 0.0F ; 
	_sceVu0InversMatrix( &mat2, &mat2 );
	_sceVu0MulMatrix( offset_mat, &mat2, &mat1 );
	_sceVu0ApplyMatrix( &tmp_vec, target, &tmp_vec );
	_sceVu0SubVector( offset_mat->m[3], &tmp_vec, base->m[3] );
	_sceVu0ApplyMatrix( offset_mat->m[3], &mat2, offset_mat->m[3] );
	/* (base)*(offset_mat)=(target) */
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	/* ちょっと最適化バージョン */
	int		i, j ;
	FVECTOR	*uvs, *vvec ;
	CV2_MDL	*cv2_mdl ;
	DG_OBJ	*obj ;
	DG_VERTS_ANIME	*v_anime ;
	FMATRIX	mat, envelope_mat ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	{/* 定数をＶＵ０レジスタへ */
		FVECTOR	vec1, vec2 ;
		vec1.vx = 0.5f * 0.50f ;
		vec1.vy = 0.5f * ( (float)DRAW_HEIGHT/512.0f ) ;
		vec2.vx = 0.5f ;
		vec2.vy = 0.5f * ( (float)DRAW_HEIGHT/512.0f ) ;
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
		_sceVu0MulMatrix( &mat, &( DG_Chanl( work->objs->chanl )->eye_pers ), &obj->world );
		GetEnvelopeMatrix( &envelope_mat, &obj->world, &work->objs->objs[ obj->model->parent ].world, &obj->model->tx );
		_SetMatrix( &mat );
		_SetMatrix2( &envelope_mat );
		/* 計算 */
		vvec = cv2_mdl->verts ;
		uvs = (void*)SCRPAD_ADDR ;
		for ( j = cv2_mdl->n_verts ; j > 0 ; j-- ){
			if ( ( (int)vvec & 0x3f ) == 0 ) asm("pref 0,64(%0)"::"r"(vvec));
			_CalcEnvmap3( uvs++, vvec++ );
		}
		/* DG_VERTS_ANIMEのバッファ切り替え */
		DG_SwitchVAnimeBuffer( v_anime );
		/* 型を変換してスクラッチパッドへ */
		DG_RegistCommonUV( (void*)SCRPAD_ADDR, cv2_mdl->n_verts );
		/* インデックスを参照して頂点を分配 */
		DG_RefineStripUV2( v_anime, cv2_mdl->verts_index );
		cv2_mdl++ ;
		v_anime++ ;
		obj++ ;
	}
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
static int GetResources( Work *work, DG_OBJS *objs, int model_id, int param )
{
	DG_TEX			*tex ;
	CV2_DEF		*cv2_def ;
	int			i, j ;

	work->model_id = model_id ;
	work->objs = objs ;

	/* 共有頂点データの取得 */
	work->cv2_def = cv2_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );
	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return (0);

	/* 環境マッピングテクスチャ取得 */
	//work->tex = tex = DG_GetTexture( 37293/* envmap */ );
	//work->tex = tex = DG_GetTexture( 13797/* envmap3 */ );
	work->tex = tex = DG_GetTexture( 0 );
	work->tex_trans = tex->tex_trans ;

	work->tex_trans.alpha.data = param ;
	work->tex_trans.tex2.data = 0 ;
	work->tex_trans.tex0.data = SCE_GS_SET_TEX0(
												BUFFER_PAGE(0) / 64 ,
												BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),9,9,0,0,0,0,0,0,0);
	work->tex_trans.clamp.data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1) ;


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
void *NewCamouflageMapping( DG_OBJS *objs, int model_id, int param )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, objs, model_id, param ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
