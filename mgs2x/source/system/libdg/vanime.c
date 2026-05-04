//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vanime.c
	頂点アニメーション補助ルーチン

	1999/09/03 K.Takabe
	$Id: vanime.c,v 1.1.1.3 2002/11/19 11:42:32 Yoshizawa1 Exp $

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
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#define VANIM_ENABLE          /*return;*/

// making this structure available for some functions that manipulate UVs.
typedef struct _tvector_s{
	short u, v;
} TVECTOR_S ;

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
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		v_size += ( mdl_pack->n_verts + 1 ) / 2 ;	/* 頂点・法線バッファサイズ計算 */
		u_size += ( mdl_pack->n_verts + 3 ) / 4 ;	/* ＵＶバッファサイズ計算 */
		mdl_pack++ ;
	}

	if ( flag & DG_VANIME_VERTS ) size += sizeof(u_long128) * v_size ;
	if ( flag & DG_VANIME_NORMS ) size += sizeof(u_long128) * v_size ;
	if ( flag & DG_VANIME_UVS ) size += sizeof(u_long128) * u_size ;

	v_anime->mem[0] = verts0 = GV_Malloc( size );
	if ( flag & DG_VANIME_SINGLE ){
		v_anime->mem[1] = verts1 = verts0 ;
	} else {
		v_anime->mem[1] = verts1 = GV_Malloc( size );
	}
	if ( verts0 == NULL || verts1 == NULL )
	{
	    if ( verts0 )
	    {
		GV_Free( verts0 ) ;
		v_anime->mem[0] = NULL ;
	    }
	    return ;
	}

	/* 頂点メモリの割り当て */
	if ( flag & DG_VANIME_VERTS ){
		v_anime->verts_top[0] = verts0 ;
		v_anime->verts_top[1] = verts1 ;
		mdl_pack = mdl->packs ;
		obj->verts = v_anime->verts_top[v_anime->buffer_clock] ;
		for ( i = mdl->n_packs ; i > 0 ; i-- ){
			{/* メモリ内容の初期化 */
				u_long128	*src, *dst0, *dst1 ;
				src = (u_long128*)mdl_pack->verts ;
				dst0 = (u_long128*)verts0 ;
				dst1 = (u_long128*)verts1 ;
				for ( j = ( mdl_pack->n_verts + 1 ) / 2 ; j > 0 ; j-- ){
					*dst0++ = *src ;
					*dst1++ = *src++ ;
				}
			}
			verts0 += ( mdl_pack->n_verts + 1 ) / 2 ;
			verts1 += ( mdl_pack->n_verts + 1 ) / 2 ;
			mdl_pack++ ;
		}
	}
	
	/* 法線メモリの割り当て（とりあえずプリシェードオブジェクトも考慮する） */
	if ( flag & DG_VANIME_NORMS ){
		u_long128	*norms ;
		v_anime->norms_top[0] = verts0 ;
		v_anime->norms_top[1] = verts1 ;
		mdl_pack = mdl->packs ;
		norms = (u_long128*)obj->norms ;
		obj->norms = v_anime->norms_top[v_anime->buffer_clock] ;
		for ( i = mdl->n_packs ; i > 0 ; i-- ){
			{/* メモリ内容の初期化 */
				u_long128	*src, *dst0, *dst1 ;
				src = (u_long128*)norms ;
				dst0 = (u_long128*)verts0 ;
				dst1 = (u_long128*)verts1 ;
				for ( j = ( mdl_pack->n_verts + 1 ) / 2 ; j > 0 ; j-- ){
					*dst0++ = *src ;
					*dst1++ = *src++ ;
				}
			}
			norms += ( mdl_pack->n_verts + 1 ) / 2 ;
			verts0 += ( mdl_pack->n_verts + 1 ) / 2 ;
			verts1 += ( mdl_pack->n_verts + 1 ) / 2 ;
			mdl_pack++ ;
		}
	}

	/* ＵＶメモリの割り当て */
	if ( flag & DG_VANIME_UVS ){
		v_anime->uvs_top[0] = verts0 ;
		v_anime->uvs_top[1] = verts1 ;
		mdl_pack = mdl->packs ;
		obj->uvs[0] = v_anime->uvs_top[v_anime->buffer_clock] ;
		for ( i = mdl->n_packs ; i > 0 ; i-- ){
			{/* メモリ内容の初期化 */
				u_long128	*src, *dst0, *dst1 ;
				src = (u_long128*)mdl_pack->uvs[0] ;
				dst0 = (u_long128*)verts0 ;
				dst1 = (u_long128*)verts1 ;
				for ( j = ( mdl_pack->n_verts + 3 ) / 4 ; j > 0 ; j-- ){
					*dst0++ = *src ;
					*dst1++ = *src++ ;
				}
			}
			verts0 += ( mdl_pack->n_verts + 3 ) / 4 ;
			verts1 += ( mdl_pack->n_verts + 3 ) / 4 ;
			mdl_pack++ ;
		}
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

	verts0 = v_anime->mem[0] ;
	if ( flag & DG_VANIME_SINGLE ){
		verts1 = verts0 ;
	} else {
		verts1 = v_anime->mem[1] ;
	}

	/* 頂点メモリの割り当て */
	if ( flag & DG_VANIME_VERTS ){
		obj->verts = v_anime->verts_top[v_anime->buffer_clock] ;
	}
	
	/* 法線メモリの割り当て */
	if ( flag & DG_VANIME_NORMS ){
		obj->verts = v_anime->verts_top[v_anime->buffer_clock] ;
	}

	/* ＵＶメモリの割り当て */
	if ( flag & DG_VANIME_UVS ){
		obj->uvs[0] = v_anime->uvs_top[v_anime->buffer_clock] ;
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
	obj->verts = (SVECTOR*)mdl->packs->verts ;
	obj->norms = (SVECTOR*)mdl->packs->norms ;
	obj->uvs[0] = mdl->packs->uvs[0] ;
	if ( obj->flag & DG_FLAG_PAINT ) obj->norms = ( SVECTOR* )obj->rgbs ;
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
	obj->verts = (SVECTOR*)mdl->packs->verts ;
	obj->norms = (SVECTOR*)mdl->packs->norms ;
	obj->uvs[0] = mdl->packs->uvs[0] ;
	if ( obj->flag & DG_FLAG_PAINT ) obj->norms = ( SVECTOR* )obj->rgbs ;
	GV_DelayedFree( v_anime->mem[0] );
	if ( !( v_anime->flag & DG_VANIME_SINGLE ) ) GV_DelayedFree( v_anime->mem[1] );
}


/* ---------------------------------------------------------------- */

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex( FVECTOR *verts, int n_verts )
{
#if 1 //BP_ASM
   SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;
   VANIM_ENABLE;
	
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)verts->vx;
		svec->vy = (int)verts->vy;
		svec->vz = (int)verts->vz;
		svec->pad = (int)(verts->vw * 4096.0f);
	}
#else //BP_ASM
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
	asm("
		move			$8,%0
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf4,0($8)		# 読み込み
		lqc2			vf5,16($8)		# 読み込み
		lqc2			vf6,32($8)		# 読み込み
		lqc2			vf7,48($8)		# 読み込み
		pref			0,64($8)		# プリフェッチ
		vftoi0.xyz		vf4,vf4			# 整数化
		addi			$9,$9,-1		# カウンタデクリメント
		vftoi0.xyz		vf5,vf5			# 整数化
		addi			$8,$8,64		# アドレスインクリメント
		vftoi0.xyz		vf6,vf6			# 整数化
		addi			$10,$10,32		# アドレスインクリメント
		vftoi0.xyz		vf7,vf7			# 整数化
		vftoi12.w		vf4,vf4			# 整数化
		vftoi12.w		vf5,vf5			# 整数化
		vftoi12.w		vf6,vf6			# 整数化
		vftoi12.w		vf7,vf7			# 整数化
		qmfc2			$11,vf4			# ＶＵからＣＯＲＥへ
		qmfc2			$12,vf5			# ＶＵからＣＯＲＥへ
		qmfc2			$13,vf6			# ＶＵからＣＯＲＥへ
		qmfc2			$14,vf7			# ＶＵからＣＯＲＥへ
		ppach			$11,$0,$11		# IVECTORからSVECTORへ変換
		ppach			$12,$0,$12		# IVECTORからSVECTORへ変換
		ppach			$13,$0,$13		# IVECTORからSVECTORへ変換
		ppach			$14,$0,$14		# IVECTORからSVECTORへ変換
		sd				$11,0-32($10)	# メモリへ
		sd				$12,8-32($10)	# メモリへ
		sd				$13,16-32($10)	# メモリへ
		sd				$14,24-32($10)	# メモリへ
		bgtz			$9,0b
		nop
	"::"r"(verts),"r"((n_verts+3)/4),"r"(svec):"$8","$9","$10","$11","$12","$13","$14" );
#endif
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index )
{
#if 1 //BP_ASM
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	SVECTOR		*buffer, *svec;
	int			i, j, n;

   VANIM_ENABLE;
	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (SVECTOR *)SCRPAD_ADDR;

	svec = (SVECTOR *)v_anime->verts_top[v_anime->buffer_clock];
	v_anime->obj->verts = svec;
	for (i = mdl->n_packs; i > 0; i--){
		n = mdl_pack->n_verts;
		for (j = (n + 1) / 2; j > 0; j--){
			svec[0] = buffer[index[0]];
			svec[1] = buffer[index[1]];
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)svec & 63) == 0) asm("pref 0,64(%0)"::"r"(svec));
			svec += 2;
			index += 2;
		}
		mdl_pack++;
	}
#else
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	SVECTOR		*buffer, *svec ;
	int			i, j, n ;

	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (SVECTOR*)SCRPAD_ADDR ;

	//v_anime->buffer_clock = 1 - v_anime->buffer_clock ;
	svec = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->verts = svec ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		for ( j = ( n + 1 ) / 2 ; j > 0 ; j-- ){
			svec[0] = buffer[ index[0] ] ;
			svec[1] = buffer[ index[1] ] ;
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)svec & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(svec));
			svec += 2 ;
			index += 2 ;
		}
		mdl_pack++ ;
	}
#endif
}

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex2( FVECTOR *verts, int n_verts )
{
#if 1 //BP_ASM
   SVECTOR *svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

   VANIM_ENABLE;
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)verts->vx;
		svec->vy = (int)verts->vy;
		svec->vz = (int)verts->vz;
		svec->pad = 4096 ;
	}
#else
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
	asm("
		move			$8,%0
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf4,0($8)		# 読み込み
		lqc2			vf5,16($8)		# 読み込み
		lqc2			vf6,32($8)		# 読み込み
		lqc2			vf7,48($8)		# 読み込み
		pref			0,64($8)		# プリフェッチ
		vftoi0.xyz		vf4,vf4			# 整数化
		addi			$9,$9,-1		# カウンタデクリメント
		vftoi0.xyz		vf5,vf5			# 整数化
		vftoi0.xyz		vf6,vf6			# 整数化
		vftoi0.xyz		vf7,vf7			# 整数化
		addi			$8,$8,64		# アドレスインクリメント
		addi			$10,$10,32		# アドレスインクリメント
		qmfc2			$11,vf4			# ＶＵからＣＯＲＥへ
		qmfc2			$12,vf5			# ＶＵからＣＯＲＥへ
		qmfc2			$13,vf6			# ＶＵからＣＯＲＥへ
		qmfc2			$14,vf7			# ＶＵからＣＯＲＥへ
		ppach			$11,$0,$11		# IVECTORからSVECTORへ変換
		ppach			$12,$0,$12		# IVECTORからSVECTORへ変換
		ppach			$13,$0,$13		# IVECTORからSVECTORへ変換
		ppach			$14,$0,$14		# IVECTORからSVECTORへ変換
		sd				$11,0-32($10)	# メモリへ
		sd				$12,8-32($10)	# メモリへ
		sd				$13,16-32($10)	# メモリへ
		sd				$14,24-32($10)	# メモリへ
		bgtz			$9,0b
		nop
	"::"r"(verts),"r"((n_verts+3)/4),"r"(svec):"$8","$9","$10","$11","$12","$13","$14" );
#endif
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex2( DG_VERTS_ANIME *v_anime, short *index )
{
#if 1
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
//	u_long64		*svec, mask1, mask2, *buffer;
   SVECTOR     *svec, *buffer;
	int			i, j, n;

   VANIM_ENABLE;
	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (SVECTOR*)SCRPAD_ADDR;

//	mask1 = 0xffff000000000000;
//	mask2 = 0x0000ffffffffffff;

	svec = (SVECTOR *)v_anime->verts_top[v_anime->buffer_clock];
	v_anime->obj->verts = (SVECTOR *)svec;
	for (i = mdl->n_packs; i > 0; i--){
		n = mdl_pack->n_verts;
		for (j = (n + 1) / 2; j > 0; j--){
         svec[0].vx = buffer[index[0]].vx;
         svec[0].vy = buffer[index[0]].vy;
         svec[0].vz = buffer[index[0]].vz;

         svec[1].vx = buffer[index[1]].vx;
         svec[1].vy = buffer[index[1]].vy;
         svec[1].vz = buffer[index[1]].vz;

//       svec[0] = (svec[0] & mask1) | (buffer[index[0]] & mask2);
//			svec[1] = (svec[1] & mask1) | (buffer[index[1]] & mask2);
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)svec & 63) == 0) asm("pref 0,64(%0)"::"r"(svec));
			svec += 2;
			index += 2;
		}
		mdl_pack++;
	}

#else
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	u_long64		*svec, mask1, mask2, *buffer ;
	int			i, j, n ;

	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (u_long64*)SCRPAD_ADDR ;

	mask1 = 0xffff000000000000 ;
	mask2 = 0x0000ffffffffffff ;
	//v_anime->buffer_clock = 1 - v_anime->buffer_clock ;
	svec = v_anime->verts_top[ v_anime->buffer_clock ];
	v_anime->obj->verts = (SVECTOR*)svec ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		for ( j = ( n + 1 ) / 2 ; j > 0 ; j-- ){
			svec[0] = ( svec[0] & mask1 ) | ( buffer[ index[0] ] & mask2 ) ;
			svec[1] = ( svec[1] & mask1 ) | ( buffer[ index[1] ] & mask2 ) ;
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)svec & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(svec));
			svec += 2 ;
			index += 2 ;
		}
		mdl_pack++ ;
	}
#endif
}

/* ---------------------------------------------------------------- */

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonNormal( FVECTOR *verts, int n_verts )
{
#if 1 //BP_ASM
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

   VANIM_ENABLE;
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)(verts->vx * 4096.0f);
		svec->vy = (int)(verts->vy * 4096.0f);
		svec->vz = (int)(verts->vz * 4096.0f);
	}
#else
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
	asm("
		move			$8,%0
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf4,0($8)		# 読み込み
		lqc2			vf5,16($8)		# 読み込み
		lqc2			vf6,32($8)		# 読み込み
		lqc2			vf7,48($8)		# 読み込み
		pref			0,64($8)		# プリフェッチ
		vftoi12.xyz		vf4,vf4			# 整数化
		addi			$9,$9,-1		# カウンタデクリメント
		vftoi12.xyz		vf5,vf5			# 整数化
		addi			$8,$8,64		# アドレスインクリメント
		vftoi12.xyz		vf6,vf6			# 整数化
		addi			$10,$10,32		# アドレスインクリメント
		vftoi12.xyz		vf7,vf7			# 整数化
		qmfc2			$11,vf4			# ＶＵからＣＯＲＥへ
		qmfc2			$12,vf5			# ＶＵからＣＯＲＥへ
		qmfc2			$13,vf6			# ＶＵからＣＯＲＥへ
		qmfc2			$14,vf7			# ＶＵからＣＯＲＥへ
		ppach			$11,$0,$11		# IVECTORからSVECTORへ変換
		ppach			$12,$0,$12		# IVECTORからSVECTORへ変換
		ppach			$13,$0,$13		# IVECTORからSVECTORへ変換
		ppach			$14,$0,$14		# IVECTORからSVECTORへ変換
		sd				$11,0-32($10)	# メモリへ
		sd				$12,8-32($10)	# メモリへ
		sd				$13,16-32($10)	# メモリへ
		sd				$14,24-32($10)	# メモリへ
		bgtz			$9,0b
		nop
	"::"r"(verts),"r"((n_verts+3)/4),"r"(svec):"$8","$9","$10","$11","$12","$13","$14" );
#endif
}

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonNormal2( FVECTOR *verts, int n_verts )
{
#if 1 //BP_ASM
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	VECTOR vecTmp;
	int i;
   float lenlen, div;

   VANIM_ENABLE;
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		lenlen = verts->vx * verts->vx + verts->vy * verts->vy + verts->vz * verts->vz ;
		div = DG_RSQRT( lenlen ) * 4096.0f ;
		svec->vx = (int)( verts->vx * div );
		svec->vy = (int)( verts->vy * div );
		svec->vz = (int)( verts->vz * div );
	}
#else
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
	asm("
		move			$8,%0
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf4,0($8)		# 読み込み
		lqc2			vf5,16($8)		# 読み込み
		lqc2			vf6,32($8)		# 読み込み
		lqc2			vf7,48($8)		# 読み込み
		pref			0,64($8)		# プリフェッチ
		vmul.xyz		vf8,vf4,vf4		# ２乗
		vmul.xyz		vf9,vf5,vf5		# ２乗
		vmul.xyz		vf10,vf6,vf6	# ２乗
		vmul.xyz		vf11,vf7,vf7	# ２乗
		vmulax.w		ACC,vf0,vf8		# 
		vmadday.w		ACC,vf0,vf8		# 
		vmaddz.w		vf8,vf0,vf8		# 
		vmulax.w		ACC,vf0,vf9		# 
		vmadday.w		ACC,vf0,vf9		# 
		vmaddz.w		vf9,vf0,vf9		# 
		vrsqrt			Q,vf0w,vf8w		# 
		vmulax.w		ACC,vf0,vf10	# 
		vmadday.w		ACC,vf0,vf10	# 
		vmaddz.w		vf10,vf0,vf10	# 
		vmulax.w		ACC,vf0,vf11	# 
		vmadday.w		ACC,vf0,vf11	# 
		vmaddz.w		vf11,vf0,vf11	# 
		vwaitq
		vmulq.xyz		vf4,vf4,Q
		vrsqrt			Q,vf0w,vf9w		# 
		vftoi12.xyz		vf4,vf4			# 整数化
		addi			$9,$9,-1		# カウンタデクリメント
		vwaitq
		vmulq.xyz		vf5,vf5,Q
		vrsqrt			Q,vf0w,vf10w	# 
		vftoi12.xyz		vf5,vf5			# 整数化
		addi			$8,$8,64		# アドレスインクリメント
		vwaitq
		vmulq.xyz		vf6,vf6,Q
		vrsqrt			Q,vf0w,vf11w	# 
		vftoi12.xyz		vf6,vf6			# 整数化
		addi			$10,$10,32		# アドレスインクリメント
		vwaitq
		vmulq.xyz		vf7,vf7,Q
		vftoi12.xyz		vf7,vf7			# 整数化
		qmfc2			$11,vf4			# ＶＵからＣＯＲＥへ
		qmfc2			$12,vf5			# ＶＵからＣＯＲＥへ
		qmfc2			$13,vf6			# ＶＵからＣＯＲＥへ
		qmfc2			$14,vf7			# ＶＵからＣＯＲＥへ
		ppach			$11,$0,$11		# IVECTORからSVECTORへ変換
		ppach			$12,$0,$12		# IVECTORからSVECTORへ変換
		ppach			$13,$0,$13		# IVECTORからSVECTORへ変換
		ppach			$14,$0,$14		# IVECTORからSVECTORへ変換
		sd				$11,0-32($10)	# メモリへ
		sd				$12,8-32($10)	# メモリへ
		sd				$13,16-32($10)	# メモリへ
		sd				$14,24-32($10)	# メモリへ
		bgtz			$9,0b
		nop
	"::"r"(verts),"r"((n_verts+3)/4),"r"(svec):"$8","$9","$10","$11","$12","$13","$14" );
#endif
}

	/*
		スクラッチパッド上の法線データをモデルの法線データに振り分ける
	*/
void DG_RefineStripNormal( DG_VERTS_ANIME *v_anime, short *index )
{
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	SVECTOR     *norm, *buffer ;
	int			i, j, n ;

   VANIM_ENABLE;
	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (SVECTOR*)SCRPAD_ADDR ;

	//v_anime->buffer_clock = 1 - v_anime->buffer_clock ;
	norm = v_anime->norms_top[ v_anime->buffer_clock ];
	v_anime->obj->norms = (SVECTOR*)norm ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		for ( j = ( n + 1 ) / 2 ; j > 0 ; j-- ){
			norm[0].vx = buffer[ index[0] ].vx;
         norm[0].vy = buffer[ index[0] ].vy;
         norm[0].vz = buffer[ index[0] ].vz;

			norm[1].vx = buffer[ index[1] ].vx;
         norm[1].vy = buffer[ index[1] ].vy;
         norm[1].vz = buffer[ index[1] ].vz;
#if 0 //BP_ASM
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)norm & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(norm));
#endif
			norm += 2 ;
			index += 2 ;
		}
		mdl_pack++ ;
	}
}

/* ---------------------------------------------------------------- */

	/*
		FVECTORをintに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonUV( FVECTOR *verts, int n_verts )
{
#if 1 //BP_ASM
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

   VANIM_ENABLE;
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)(verts->vx * 4096.0f);
		svec->vy = (int)(verts->vy * 4096.0f);
	}
#else
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
	asm("
		move			$8,%0
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf4,0($8)		# 読み込み
		lqc2			vf5,16($8)		# 読み込み
		lqc2			vf6,32($8)		# 読み込み
		lqc2			vf7,48($8)		# 読み込み
		pref			0,64($8)		# プリフェッチ
		vftoi12.xy		vf4,vf4			# 整数化
		addi			$9,$9,-1		# カウンタデクリメント
		vftoi12.xy		vf5,vf5			# 整数化
		addi			$8,$8,64		# アドレスインクリメント
		vftoi12.xy		vf6,vf6			# 整数化
		addi			$10,$10,16		# アドレスインクリメント
		vftoi12.xy		vf7,vf7			# 整数化
		qmfc2			$11,vf4			# ＶＵからＣＯＲＥへ
		qmfc2			$12,vf5			# ＶＵからＣＯＲＥへ
		qmfc2			$13,vf6			# ＶＵからＣＯＲＥへ
		qmfc2			$14,vf7			# ＶＵからＣＯＲＥへ
		ppach			$11,$0,$11		# IVECTORからSVECTORへ変換
		ppach			$12,$0,$12		# IVECTORからSVECTORへ変換
		ppach			$13,$0,$13		# IVECTORからSVECTORへ変換
		ppach			$14,$0,$14		# IVECTORからSVECTORへ変換
		sw				$11,0-16($10)	# メモリへ
		sw				$12,4-16($10)	# メモリへ
		sw				$13,8-16($10)	# メモリへ
		sw				$14,12-16($10)	# メモリへ
		bgtz			$9,0b
		nop
	"::"r"(verts),"r"((n_verts+3)/4),"r"(svec):"$8","$9","$10","$11","$12","$13","$14" );
#endif
}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
	*/
void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index )
{
#if 1 //BP_ASM
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	u_int		*uv, *buffer;
	int			i, j, n;

   VANIM_ENABLE;
	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (u_int*)SCRPAD_ADDR;

	uv = (u_int *)v_anime->uvs_top[v_anime->buffer_clock];
	v_anime->obj->uvs[0] = (short*)uv;
	for (i = mdl->n_packs; i > 0; i--){
		n = mdl_pack->n_verts;
		for (j = (n + 3) / 4; j > 0; j--){
			uv[0] = buffer[index[0]];
			uv[1] = buffer[index[1]];
			uv[2] = buffer[index[2]];
			uv[3] = buffer[index[3]];
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)uv & 63) == 0) asm("pref 0,64(%0)"::"r"(uv));
			uv += 4;
			index += 4;
		}
		mdl_pack++;
	}
#else
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	u_int		*uv, *buffer ;
	int			i, j, n ;

	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (u_int*)SCRPAD_ADDR ;

	//v_anime->buffer_clock = 1 - v_anime->buffer_clock ;
	uv = v_anime->uvs_top[ v_anime->buffer_clock ];
	v_anime->obj->uvs[0] = (short*)uv ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		for ( j = ( n + 3 ) / 4 ; j > 0 ; j-- ){
			uv[0] = buffer[ index[0] ] ;
			uv[1] = buffer[ index[1] ] ;
			uv[2] = buffer[ index[2] ] ;
			uv[3] = buffer[ index[3] ] ;
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)uv & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(uv));
			uv += 4 ;
			index += 4 ;
		}
		mdl_pack++ ;
	}
#endif
}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
		（頂点・法線のインデックスを使用する場合）
	*/
void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index )
{
#if 1 //BP_ASM
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	u_int		*uv, *buffer;
	int			i, j, n;

   VANIM_ENABLE;
	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (u_int *)SCRPAD_ADDR;

	uv = (u_int *)v_anime->uvs_top[v_anime->buffer_clock];
	v_anime->obj->uvs[0] = (short*)uv;
	for (i = mdl->n_packs; i > 0; i--) {
		n = mdl_pack->n_verts;
		for (j = (n + 1) / 2; j > 1; j -= 2) {
			uv[0] = buffer[index[0]];
			uv[1] = buffer[index[1]];
			uv[2] = buffer[index[2]];
			uv[3] = buffer[index[3]];
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)uv & 63) == 0) asm("pref 0,64(%0)"::"r"(uv));
			uv += 4;
			index += 4;
		}
		if (j > 0){
			uv[0] = buffer[index[0]];
			uv[1] = buffer[index[1]];
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)uv & 63) == 0) asm("pref 0,64(%0)"::"r"(uv));
			uv += 4;
			index += 2;
		}
		mdl_pack++;
	}
#else
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	u_int		*uv, *buffer ;
	int			i, j, n ;

	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (u_int*)SCRPAD_ADDR ;

	//v_anime->buffer_clock = 1 - v_anime->buffer_clock ;
	uv = v_anime->uvs_top[ v_anime->buffer_clock ];
	v_anime->obj->uvs[0] = (short*)uv ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		for ( j = ( n + 1 ) / 2 ; j > 1 ; j -= 2 ){
			uv[0] = buffer[ index[0] ] ;
			uv[1] = buffer[ index[1] ] ;
			uv[2] = buffer[ index[2] ] ;
			uv[3] = buffer[ index[3] ] ;
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)uv & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(uv));
			uv += 4 ;
			index += 4 ;
		}
		if ( j > 0 ){
			uv[0] = buffer[ index[0] ] ;
			uv[1] = buffer[ index[1] ] ;
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)uv & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(uv));
			uv += 4 ;
			index += 2 ;
		}
		mdl_pack++ ;
	}
#endif
}

