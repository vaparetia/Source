//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xvanime.c
	頂点アニメーション補助ルーチン

	2002/03/25 K.Takabe
	$Id: xvanime.c,v 1.1.1.3 2002/11/19 11:42:38 Yoshizawa1 Exp $

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

#ifdef KP_XBOX //BP

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
	int v_size, u_size, i, j, size ;
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
	u_long128	*verts0, *verts1 ;
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
		svec->vx = (int)verts->vx;
		svec->vy = (int)verts->vy;
		svec->vz = (int)verts->vz;
		svec->pad = (int)( verts->vw * 4096.0f );
	}
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	SVECTOR		*buffer, *svec ;
	int			i, j, n ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;

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
			verts->wt = svec->pad * 32767.0f / 4096.0f ;
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
	for ( i = 0 ; i < mdl->n_verts ; i++, svec++ ){
		verts->vx = svec->vx ;
		verts->vy = svec->vy ;
		verts->vz = svec->vz ;
		verts->wt = svec->pad * 32767 / 4096 ;
		verts = (void*)( (char*)verts + mdl->stride );
	}
#endif
}

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex2( FVECTOR *verts, int n_verts )
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)verts->vx;
		svec->vy = (int)verts->vy;
		svec->vz = (int)verts->vz;
		svec->pad = 4096 ;
	}
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex2( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	SVECTOR		*buffer, *svec ;
	int			i, j, n ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;

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
	v_anime->obj->vbuff = verts ;
	svec = (SVECTOR*)SCRPAD_ADDR ;
	for ( i = 0 ; i < mdl->n_verts ; i++, svec++ ){
		verts->vx = svec->vx ;
		verts->vy = svec->vy ;
		verts->vz = svec->vz ;
		//verts->wt = svec->vw * 32767 / 4096 ;
		verts = (void*)( (char*)verts + mdl->stride );
	}
#endif
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
		svec->vx = (int)( verts->vx * 32767.0f );
		svec->vy = (int)( verts->vy * 32767.0f );
		svec->vz = (int)( verts->vz * 32767.0f );
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
		div = DG_RSQRT( lenlen ) * 32767.0f ;
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
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	SVECTOR		*buffer, *svec ;
	int			i, j, n ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;

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
	v_anime->obj->vbuff = verts ;
	svec = (SVECTOR*)SCRPAD_ADDR ;
	for ( i = 0 ; i < mdl->n_verts ; i++, svec++ ){
		verts->nx = svec->vx ;
		verts->ny = svec->vy ;
		verts->nz = svec->vz ;
		verts = (void*)( (char*)verts + mdl->stride );
	}
#endif
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
		*uvs++ = (int)( verts->vx * 32767.0f );
		*uvs++ = (int)( verts->vy * 32767.0f );
	}

}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
	*/
void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	short		*buffer, *uvs ;
	int			i, j, n ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;

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
	v_anime->obj->vbuff = verts ;
	uvs = (SVECTOR*)SCRPAD_ADDR ;
	for ( i = 0 ; i < mdl->n_verts ; i++, uvs+=2 ){
		verts->u0 = uvs[0] ;
		verts->v0 = uvs[1] ;
		verts = (void*)( (char*)verts + mdl->stride );
	}
#endif
}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
		（頂点・法線のインデックスを使用する場合）
	*/
void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	short		*buffer, *uvs ;
	int			i, j, n ;
	DG_VERTEX_KMSM	*verts ;
	short		*index_top = index ;

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
	uvs = (SVECTOR*)SCRPAD_ADDR ;
	for ( i = 0 ; i < mdl->n_verts ; i++, uvs+=2 ){
		verts->u0 = uvs[0] ;
		verts->v0 = uvs[1] ;
		verts = (void*)( (char*)verts + mdl->stride );
	}
#endif
}

#endif
