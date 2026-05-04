/*
	xvanime.c
	頂点アニメーション補助ルーチン

	2002/03/25 K.Takabe
	$Id: wvanime.c,v 1.8 2002/12/31 08:00:11 takaki Exp $

*/
/*

	void DG_MakeAnimVertsBuffer( DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag )
	DG_VERTS_ANIME	*v_anime ;		アニメーション用ワーク
	DG_OBJ			*obj ;			バッファを作成するオブジェクト
	int				flag ;			バッファ作成のターゲット指定

		頂点アニメーション用頂点バッファ作成


	void DG_FreeAnimVertsBuffer( DG_VERTS_ANIME *v_anime )
	DG_VERTS_ANIME	*v_anime ;		アニメーション用ワーク

		頂点アニメのワーク開放


	void DG_RegistCommonVertex( FVECTOR *verts, int n_verts )
	FVECTOR		*verts ;		転送するベクトルの先頭ポインタ
	int			n_verts ;		転送ベクトル数

		FVECTOR頂点をSVECTORに変換し、スクラッチパッドへ転送する


	void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index )
	DG_VERTS_ANIME	*v_anime ;		アニメーション用ワーク
	short			*index ;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける


	void DG_RegistCommonNormal( FVECTOR *verts, int n_verts )
	FVECTOR		*verts ;		転送するベクトルの先頭ポインタ
	int			n_verts ;		転送ベクトル数

		FVECTOR法線をSVECTORに変換し、スクラッチパッドへ転送する


	void DG_RefineStripNormal( DG_VERTS_ANIME *v_anime, short *index )
	DG_VERTS_ANIME	*v_anime ;		アニメーション用ワーク
	short			*index ;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上の法線データをモデルの法線データに振り分ける


	void DG_RegistCommonUV( FVECTOR *verts, int n_verts )
	FVECTOR		*verts ;		転送するベクトルの先頭ポインタ
	int			n_verts ;		転送ベクトル数

		FVECTORＵＶ値をintに変換し、スクラッチパッドへ転送する


	void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index )
	DG_VERTS_ANIME	*v_anime ;		アニメーション用ワーク
	short			*index ;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける


	void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index )
	DG_VERTS_ANIME	*v_anime ;		アニメーション用ワーク
	short			*index ;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
		（頂点・法線のインデックスデータを使用する場合）


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

/* ---------------------------------------------------------------- */

	/*
		頂点アニメーション用頂点バッファ作成
	*/
void DG_MakeAnimVertsBuffer( DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag )
{
	int v_size, u_size, size ;
	DG_MDL	*mdl ;
	DG_MDLPACK	*mdl_pack ;
	u_long128	*verts0, *verts1 ;

	GV_ZeroMemory( v_anime, sizeof(DG_VERTS_ANIME) );
	v_anime->flag = flag ;
	v_anime->obj = obj ;
	v_anime->buffer_clock = 0 ;

	mdl = obj->model ;
	mdl_pack = mdl->packs ;
	size = v_size = u_size = 0 ;
#ifndef NEW_KMX_FORMAT
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		size += mdl_pack->n_verts ;
		mdl_pack++ ;
	}
#else
	size = mdl->n_verts ;
#endif

	/* XBOXでは頂点バッファとして確保するため頂点、法線、ＵＶ全てをセットして確保する */
	v_anime->mem[0] = verts0 = DG_AllocLocalVideoMemory( mdl->stride * size );
	if ( flag & DG_VANIME_SINGLE ){
		v_anime->mem[1] = verts1 = verts0 ;
	} else {
		v_anime->mem[1] = verts1 = DG_AllocLocalVideoMemory( mdl->stride * size );
	}
	if ( verts0 == NULL || verts1 == NULL ){
	    if ( verts0 ){
			DG_FreeLocalVideoMemory( verts0 ) ;
			v_anime->mem[0] = NULL ;
	    }
	    return ;
	}

	/* 頂点メモリの割り当て */
	v_anime->verts_top[0] = verts0 ;
	v_anime->verts_top[1] = verts1 ;
	obj->vbuff = v_anime->verts_top[v_anime->buffer_clock] ;
	/* メモリ内容の初期化 */
	memcpy( verts0, mdl->vbuff, mdl->stride * size );
	memcpy( verts1, mdl->vbuff, mdl->stride * size );

	/* 法線メモリの割り当て（とりあえずプリシェードオブジェクトも考慮する） */
	if ( flag & DG_VANIME_VERTS ){
	}
	
	/* 法線メモリの割り当て（とりあえずプリシェードオブジェクトも考慮する） */
	if ( flag & DG_VANIME_NORMS ){
	}

	/* ＵＶメモリの割り当て */
	if ( flag & DG_VANIME_UVS ){
	}
}

	/*
		頂点アニメーション用バッファ再割当
	*/
void DG_RestartAnimVerts( DG_VERTS_ANIME *v_anime )
{
	DG_OBJ  *obj ;
	int flag;

	obj = v_anime->obj ;
	flag = v_anime->flag ;

	obj->vbuff = v_anime->verts_top[v_anime->buffer_clock] ;

	/* 頂点メモリの割り当て */
	if ( flag & DG_VANIME_VERTS ){
		//obj->verts = v_anime->verts_top[v_anime->buffer_clock] ;
	}
	
	/* 法線メモリの割り当て */
	if ( flag & DG_VANIME_NORMS ){
		//obj->verts = v_anime->verts_top[v_anime->buffer_clock] ;
	}

	/* ＵＶメモリの割り当て */
	if ( flag & DG_VANIME_UVS ){
		//obj->uvs[0] = v_anime->uvs_top[v_anime->buffer_clock] ;
	}
}

	/*
		頂点アニメの一時停止
	*/
void DG_StopAnimVerts( DG_VERTS_ANIME *v_anime )
{
	DG_OBJ	*obj ;
	DG_MDL	*mdl ;

	obj = v_anime->obj ;
	mdl = obj->model ;
	obj->vbuff = mdl->vbuff ;

#ifdef _WINDOWS
	DG_AssignDGObjVertexBuffer(obj) ;
	if( obj->d3d_cvbuff ){ DG_AssignDGObjCVertexBuffer(obj) ; }
#endif
}

	/*
		頂点アニメのワーク開放
	*/
void DG_FreeAnimVertsBuffer( DG_VERTS_ANIME *v_anime )
{
	DG_OBJ	*obj ;
	DG_MDL	*mdl ;

	obj = v_anime->obj ;
	mdl = obj->model ;
	obj->vbuff = mdl->vbuff ;

	DG_DelayedFreeLocalVideoMemory( v_anime->mem[0] ) ;
	if ( !( v_anime->flag & DG_VANIME_SINGLE ) ) DG_DelayedFreeLocalVideoMemory( v_anime->mem[1] ) ;
}


/* ---------------------------------------------------------------- */

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex( FVECTOR *verts, int n_verts )
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;
	
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (short)verts->vx;
		svec->vy = (short)verts->vy;
		svec->vz = (short)verts->vz;
		svec->pad = (short)( verts->vw * 4096.0f );
	}
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDL		*mdl ;
	SVECTOR		*svec ;
	int			i ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;
	DWORD		stride ;

#ifndef NEW_KMX_FORMAT
	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (SVECTOR*)SCRPAD_ADDR ;

	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->vbuff = verts ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		index_top = index ;
		for ( j = n ; j > 0 ; j-- ){
			svec = buffer + *index ;
			verts->vx = svec->vx ;
			verts->vy = svec->vy ;
			verts->vz = svec->vz ;
			verts->wt = svec->pad ;
			index++ ;
			verts = (void*)( (char*)verts + mdl->stride );
		}
		/* CV2ファイルのインデックスはkmsのアライメントにそろっているためXBOXでは補正が必要 */
		index = index_top + ( ( n + 1 ) & ~1 ) ;
		mdl_pack++ ;
	}
#else
	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	svec = (SVECTOR*)SCRPAD_ADDR ;
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0; i--, svec++ ){
		verts->vx = svec->vx ;
		verts->vy = svec->vy ;
		verts->vz = svec->vz ;
		verts->wt = svec->pad ;
		verts = (void*)( (char*)verts + stride );
	}
#endif

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}

	/*
		FVECTORをSVECTORに変換し、モデルの頂点データに振り分ける
	*/
void DG_RegistCommonVertexRefineStripVertex( DG_VERTS_ANIME *v_anime, FVECTOR *src )
{
	int 			i;
	DG_MDL			*mdl ;
	DG_VERTEX_KMSM	*verts ;
	DWORD			stride ;
	
	mdl    = v_anime->obj->model ;
	verts  = v_anime->verts_top[ v_anime->buffer_clock ];
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0; i--, src++ ){
		verts->vx = (short)src->vx ;
		verts->vy = (short)src->vy ;
		verts->vz = (short)src->vz ;
		verts->wt = (short)(src->vw * 4096.0f) ;
		verts = (void*)( (char*)verts + stride );
	}
}


	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex2( FVECTOR *verts, int n_verts )
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (short)verts->vx;
		svec->vy = (short)verts->vy;
		svec->vz = (short)verts->vz;
		svec->pad = 4096 ;
	}
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex2( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDL		*mdl ;
	SVECTOR		*svec ;
	int			i ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;
	DWORD		stride ;

#ifndef NEW_KMX_FORMAT
	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (SVECTOR*)SCRPAD_ADDR ;

	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->vbuff = verts ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		index_top = index ;
		for ( j = n ; j > 0 ; j-- ){
			svec = buffer + *index ;
			verts->vx = svec->vx ;
			verts->vy = svec->vy ;
			verts->vz = svec->vz ;
			//verts->wt = svec->pad ;
			index++ ;
			verts = (void*)( (char*)verts + mdl->stride );
		}
		/* CV2ファイルのインデックスはkmsのアライメントにそろっているためXBOXでは補正が必要 */
		index = index_top + ( ( n + 1 ) & ~1 ) ;
		mdl_pack++ ;
	}
#else
	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	svec = (SVECTOR*)SCRPAD_ADDR ;
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, svec++ ){
		verts->vx = svec->vx ;
		verts->vy = svec->vy ;
		verts->vz = svec->vz ;
		//verts->wt = svec->vw ;
		verts = (void*)( (char*)verts + stride );
	}
#endif

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}

	/*
		FVECTORをSVECTORに変換し、頂点データをモデルの頂点データに振り分ける
	*/
extern void DG_RegistCommonVertex2RefineStripVertex2(DG_VERTS_ANIME *v_anime, FVECTOR *src )
{
	DG_MDL			*mdl ;
	int				i ;
	DG_VERTEX_KMSM	*verts ;
	DWORD			stride ;

	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, src++ ){
		verts->vx = (short)src->vx ;
		verts->vy = (short)src->vy ;
		verts->vz = (short)src->vz ;
		//verts->wt = 4096 ;
		verts = (void*)( (char*)verts + stride );
	}

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}

/* ---------------------------------------------------------------- */

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonNormal( FVECTOR *verts, int n_verts )
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)( verts->vx * 4096.0f );
		svec->vy = (int)( verts->vy * 4096.0f );
		svec->vz = (int)( verts->vz * 4096.0f );
	}
}

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonNormal2( FVECTOR *verts, int n_verts )
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;
	float	lenlen, div ;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		lenlen = verts->vx * verts->vx + verts->vy * verts->vy + verts->vz * verts->vz ;
		div = DG_RSQRT( lenlen ) * 4096.0f ;
		svec->vx = (int)( verts->vx * div );
		svec->vy = (int)( verts->vy * div );
		svec->vz = (int)( verts->vz * div );
	}
}

	/*
		スクラッチパッド上の法線データをモデルの法線データに振り分ける
	*/
void DG_RefineStripNormal( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDL		*mdl ;
	SVECTOR		*svec ;
	int			i ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;
	DWORD		stride ;

#ifndef NEW_KMX_FORMAT
	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (SVECTOR*)SCRPAD_ADDR ;

	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->vbuff = verts ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		index_top = index ;
		for ( j = n ; j > 0 ; j-- ){
			svec = buffer + *index ;
			verts->nx = svec->vx ;
			verts->ny = svec->vy ;
			verts->nz = svec->vz ;
			index++ ;
			verts = (void*)( (char*)verts + mdl->stride );
		}
		/* CV2ファイルのインデックスはkmsのアライメントにそろっているためXBOXでは補正が必要 */
		index = index_top + ( ( n + 1 ) & ~1 ) ;
		mdl_pack++ ;
	}
#else
	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	svec = (SVECTOR*)SCRPAD_ADDR ;
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, svec++ ){
		verts->nx = svec->vx ;
		verts->ny = svec->vy ;
		verts->nz = svec->vz ;
		verts = (void*)( (char*)verts + stride );
	}
#endif

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}

	/*
		FVECTORをSVECTORに変換し、モデルの法線データに振り分ける
	*/
void DG_RegistCommonNormal2RefineStripNormal( DG_VERTS_ANIME *v_anime, FVECTOR *src )
{
	DG_MDL			*mdl ;
	int				i ;
	DG_VERTEX_KMSM	*verts ;
	float			lenlen, div ;
	DWORD			stride ;

	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, src++ ){
		lenlen = (src->vx * src->vx) + (src->vy * src->vy) + (src->vz * src->vz) ;
#ifndef _WINDOWS
		div = DG_RSQRT( lenlen ) * 4096.0f ;
#else
		div = DG_RSQRT( lenlen ) * 32767.0f ;
#endif
		verts->nx = (short)(src->vx * div) ;
		verts->ny = (short)(src->vy * div) ;
		verts->nz = (short)(src->vz * div) ;
		verts = (void*)( (char*)verts + stride );
	}

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}
/* ---------------------------------------------------------------- */

	/*
		FVECTORをintに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonUV( FVECTOR *verts, int n_verts )
{
	short	*uvs = (short*)SCRPAD_ADDR ;
	int i;

	for (i = n_verts; i > 0; i--, verts++) {
		*uvs++ = (int)( verts->vx * 4096.0f );
		*uvs++ = (int)( verts->vy * 4096.0f );
	}

}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
	*/
void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDL		*mdl ;
	short		*uvs ;
	int			i ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;
	DWORD		stride ;

#ifndef NEW_KMX_FORMAT
	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (short*)SCRPAD_ADDR ;

	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->vbuff = verts ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		index_top = index ;
		for ( j = n ; j > 0 ; j-- ){
			uvs = buffer + *index * 2 ;
			verts->u0 = uvs[0] ;
			verts->v0 = uvs[1] ;
			index++ ;
			verts = (void*)( (char*)verts + mdl->stride );
		}
		/* CV2ファイルのインデックスはkmsのアライメントにそろっているためXBOXでは補正が必要 */
		index = index_top + ( ( n + 3 ) & ~3 ) ;
		mdl_pack++ ;
	}
#else
	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	uvs = (short*)SCRPAD_ADDR ;
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, uvs+=2 ){
		verts->u0 = uvs[0] ;
		verts->v0 = uvs[1] ;
		verts = (void*)( (char*)verts + stride );
	}
#endif

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}

	/*
		FVECTORをintに変換しモデルのＵＶデータに振り分ける
	*/
void DG_RegistCommonUVRefineStripUV( DG_VERTS_ANIME *v_anime, FVECTOR *src )
{
	int 			i ;
	DG_MDL			*mdl ;
	DG_VERTEX_KMSM	*verts ;
	DWORD			stride ;

	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, src++ ){
		verts->u0 = (short)(verts->vx * 4096.0f) ;
		verts->v0 = (short)(verts->vy * 4096.0f) ;
		verts = (void*)( (char*)verts + stride );
	}

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}
	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
		（頂点・法線のインデックスを使用する場合）
	*/
void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDL		*mdl ;
	short		*uvs ;
	int			i ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;
	DWORD		stride ;

#ifndef NEW_KMX_FORMAT
	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (short*)SCRPAD_ADDR ;

	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->vbuff = verts ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		index_top = index ;
		for ( j = n ; j > 0 ; j-- ){
			uvs = buffer + *index * 2 ;
			verts->u0 = uvs[0] ;
			verts->v0 = uvs[1] ;
			index++ ;
			verts = (void*)( (char*)verts + mdl->stride );
		}
		/* CV2ファイルのインデックスはkmsのアライメントにそろっているためXBOXでは補正が必要 */
		index = index_top + ( ( n + 1 ) & ~1 ) ;
		mdl_pack++ ;
	}
#else
	mdl = v_anime->obj->model ;
	verts = v_anime->verts_top[ v_anime->buffer_clock ];
	uvs = (short*)SCRPAD_ADDR ;
	stride = mdl->stride ;
	for ( i = mdl->n_verts ; i > 0 ; i--, uvs+=2 ){
		verts->u0 = uvs[0] ;
		verts->v0 = uvs[1] ;
		verts = (void*)( (char*)verts + stride );
	}
#endif

	DG_AssignDGObjVertexBuffer(v_anime->obj) ;
}

