/*
	vanime.c
	頂点アニメーション補助ルーチン

	1999/09/03 K.Takabe
	2001/06/11 F.Miyauchi
	$Id: vanime.cpp,v 1.1.1.3 2002/11/19 11:42:32 Yoshizawa1 Exp $

*/

// Xbox版方針
// MakeAnimVertsBufferと同時に頂点バッファを作成(2個～3個？ロックされないことが条件)
// FreeAnimVertsBufferで頂点バッファも開放
// 

/*

	void DG_MakeAnimVertsBuffer(DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag)
	DG_VERTS_ANIME	*v_anime;		アニメーション用ワーク
	DG_OBJ			*obj;			バッファを作成するオブジェクト
	int				flag;			バッファ作成のターゲット指定

		頂点アニメーション用頂点バッファ作成


	void DG_FreeAnimVertsBuffer(DG_VERTS_ANIME *v_anime)
	DG_VERTS_ANIME	*v_anime;		アニメーション用ワーク

		頂点アニメのワーク開放


	void DG_RegistCommonVertex(FVECTOR *verts, int n_verts)
	FVECTOR		*verts;		転送するベクトルの先頭ポインタ
	int			n_verts;		転送ベクトル数

		FVECTOR頂点をSVECTORに変換し、スクラッチパッドへ転送する


	void DG_RefineStripVertex(DG_VERTS_ANIME *v_anime, short *index)
	DG_VERTS_ANIME	*v_anime;		アニメーション用ワーク
	short			*index;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける


	void DG_RegistCommonNormal(FVECTOR *verts, int n_verts)
	FVECTOR		*verts;		転送するベクトルの先頭ポインタ
	int			n_verts;		転送ベクトル数

		FVECTOR法線をSVECTORに変換し、スクラッチパッドへ転送する


	void DG_RefineStripNormal(DG_VERTS_ANIME *v_anime, short *index)
	DG_VERTS_ANIME	*v_anime;		アニメーション用ワーク
	short			*index;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上の法線データをモデルの法線データに振り分ける


	void DG_RegistCommonUV(FVECTOR *verts, int n_verts)
	FVECTOR		*verts;		転送するベクトルの先頭ポインタ
	int			n_verts;		転送ベクトル数

		FVECTORＵＶ値をintに変換し、スクラッチパッドへ転送する


	void DG_RefineStripUV(DG_VERTS_ANIME *v_anime, short *index)
	DG_VERTS_ANIME	*v_anime;		アニメーション用ワーク
	short			*index;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける


	void DG_RefineStripUV2(DG_VERTS_ANIME *v_anime, short *index)
	DG_VERTS_ANIME	*v_anime;		アニメーション用ワーク
	short			*index;		共有頂点からストリップへの変換インデックス

		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
		（頂点・法線のインデックスデータを使用する場合）


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"

/* ---------------------------------------------------------------- */

	/*
		頂点アニメーション用頂点バッファ作成
	*/
void DG_MakeAnimVertsBuffer(DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag)
{
	int v_size, u_size, i, j, size;
	DG_MDL *mdl;
	DG_MDLPACK *mdl_pack;
	u_long128 *verts0, *verts1;

	GV_ZeroMemory(v_anime, sizeof(DG_VERTS_ANIME));
	v_anime->flag = flag;
	v_anime->obj = obj;
	v_anime->buffer_clock = 0;

	mdl = obj->model;
	mdl_pack = mdl->packs;
	size = v_size = u_size = 0;
	for (i = mdl->n_packs; i > 0; i--) {
		v_size += (mdl_pack->n_verts + 1) / 2;	/* 頂点・法線バッファサイズ計算 */
		u_size += (mdl_pack->n_verts + 3) / 4;	/* ＵＶバッファサイズ計算 */
		mdl_pack++;
	}

	if (flag & DG_VANIME_VERTS) size += sizeof(u_long128) * v_size;
	if (flag & DG_VANIME_NORMS) size += sizeof(u_long128) * v_size;
	if (flag & DG_VANIME_UVS) size += sizeof(u_long128) * u_size;

	v_anime->mem[0] = verts0 = (u_long128 *)GV_Malloc(size);
	if (flag & DG_VANIME_SINGLE){
		v_anime->mem[1] = verts1 = verts0;
	} else {
		v_anime->mem[1] = verts1 = (u_long128 *)GV_Malloc(size);
	}
	ASSERT(verts1 != NULL);

	/* 頂点メモリの割り当て */
	if (flag & DG_VANIME_VERTS) {
		v_anime->verts_top[0] = verts0;
		v_anime->verts_top[1] = verts1;
		mdl_pack = mdl->packs;
		obj->verts = (SVECTOR *)v_anime->verts_top[v_anime->buffer_clock];
		for (i = mdl->n_packs; i > 0; i--) {
			{/* メモリ内容の初期化 */
				u_long128	*src, *dst0, *dst1;
				src = (u_long128 *)mdl_pack->verts;
				dst0 = (u_long128 *)verts0;
				dst1 = (u_long128 *)verts1;
				for (j = (mdl_pack->n_verts + 1) / 2; j > 0; j--){
					*dst0++ = *src;
					*dst1++ = *src++;
				}
			}
			verts0 += (mdl_pack->n_verts + 1) / 2;
			verts1 += (mdl_pack->n_verts + 1) / 2;
			mdl_pack++;
		}
	}
	
	/* 法線メモリの割り当て（とりあえずプリシェードオブジェクトも考慮する） */
	if (flag & DG_VANIME_NORMS) {
		u_long128	*norms;
		v_anime->norms_top[0] = verts0;
		v_anime->norms_top[1] = verts1;
		mdl_pack = mdl->packs;
		norms = (u_long128 *)obj->norms;
		obj->norms = (SVECTOR *)v_anime->norms_top[v_anime->buffer_clock];
		for (i = mdl->n_packs; i > 0; i--) {
			{/* メモリ内容の初期化 */
				u_long128 *src, *dst0, *dst1;
				src = (u_long128 *)norms;
				dst0 = (u_long128 *)verts0;
				dst1 = (u_long128 *)verts1;
				for (j = (mdl_pack->n_verts + 1) / 2; j > 0; j--){
					*dst0++ = *src;
					*dst1++ = *src++;
				}
			}
			norms += (mdl_pack->n_verts + 1) / 2;
			verts0 += (mdl_pack->n_verts + 1) / 2;
			verts1 += (mdl_pack->n_verts + 1) / 2;
			mdl_pack++;
		}
	}

	/* ＵＶメモリの割り当て */
	if (flag & DG_VANIME_UVS) {
		v_anime->uvs_top[0] = verts0;
		v_anime->uvs_top[1] = verts1;
		mdl_pack = mdl->packs;
		obj->uvs[0] = (TVECTOR_S *)v_anime->uvs_top[v_anime->buffer_clock];
		for (i = mdl->n_packs; i > 0; i--) {
			{/* メモリ内容の初期化 */
				u_long128	*src, *dst0, *dst1;
				src = (u_long128 *)mdl_pack->uvs[0];
				dst0 = (u_long128 *)verts0;
				dst1 = (u_long128 *)verts1;
				for (j = (mdl_pack->n_verts + 3) / 4; j > 0; j--) {
					*dst0++ = *src;
					*dst1++ = *src++;
				}
			}
			verts0 += (mdl_pack->n_verts + 3) / 4;
			verts1 += (mdl_pack->n_verts + 3) / 4;
			mdl_pack++;
		}
	}


}

	/*
		頂点アニメーション用バッファ再割当
	*/
void DG_RestartAnimVerts(DG_VERTS_ANIME *v_anime)
{
	DG_OBJ *obj;
	u_long128 *verts0, *verts1;
	int flag;

	obj = v_anime->obj;
	flag = v_anime->flag;

	verts0 = (u_long128 *)v_anime->mem[0];
	if (flag & DG_VANIME_SINGLE){
		verts1 = verts0;
	} else {
		verts1 = (u_long128 *)v_anime->mem[1];
	}

	/* 頂点メモリの割り当て */
	if (flag & DG_VANIME_VERTS){
		obj->verts = (SVECTOR *)v_anime->verts_top[v_anime->buffer_clock];
	}
	
	/* 法線メモリの割り当て */
	if (flag & DG_VANIME_NORMS){
		obj->verts = (SVECTOR *)v_anime->verts_top[v_anime->buffer_clock];
	}

	/* ＵＶメモリの割り当て */
	if (flag & DG_VANIME_UVS){
		obj->uvs[0] = (TVECTOR_S *)v_anime->uvs_top[v_anime->buffer_clock];
	}
}

	/*
		頂点アニメの一時停止
	*/
void DG_StopAnimVerts(DG_VERTS_ANIME *v_anime)
{
	DG_OBJ	*obj;
	DG_MDL	*mdl;

	obj = v_anime->obj;
	mdl = obj->model;
	obj->verts = (SVECTOR *)mdl->packs->verts;
	obj->norms = (SVECTOR *)mdl->packs->norms;
	obj->uvs[0] = (TVECTOR_S*)mdl->packs->uvs[0];
	if (obj->flag & DG_FLAG_PAINT) obj->norms = obj->rgbs;
}

	/*
		頂点アニメのワーク開放
	*/
void DG_FreeAnimVertsBuffer(DG_VERTS_ANIME *v_anime)
{
	DG_OBJ	*obj;
	DG_MDL	*mdl;

	obj = v_anime->obj;
	mdl = obj->model;
	obj->verts = (SVECTOR*)mdl->packs->verts;
	obj->norms = (SVECTOR*)mdl->packs->norms;
	obj->uvs[0] = (TVECTOR_S*)mdl->packs->uvs[0];
	if (obj->flag & DG_FLAG_PAINT) obj->norms = obj->rgbs;
	GV_DelayedFree(v_anime->mem[0]);
	if (!(v_anime->flag & DG_VANIME_SINGLE)) GV_DelayedFree(v_anime->mem[1]);
}


/* ---------------------------------------------------------------- */

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex(FVECTOR *verts, int n_verts)
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;
	
	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)verts->vx;
		svec->vy = (int)verts->vy;
		svec->vz = (int)verts->vz;
		svec->pad = ((int)verts->vw) << 12;
	}
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex(DG_VERTS_ANIME *v_anime, short *index)
{
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	SVECTOR		*buffer, *svec;
	int			i, j, n;

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
}

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonVertex2(FVECTOR *verts, int n_verts)
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = (int)verts->vx;
		svec->vy = (int)verts->vy;
		svec->vz = (int)verts->vz;
	}
}

	/*
		スクラッチパッド上の頂点データをモデルの頂点データに振り分ける
	*/
void DG_RefineStripVertex2(DG_VERTS_ANIME *v_anime, short *index)
{
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	u_long64		*svec, mask1, mask2, *buffer;
	int			i, j, n;

	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (u_long64*)SCRPAD_ADDR;

	mask1 = 0xffff000000000000;
	mask2 = 0x0000ffffffffffff;

	svec = (u_long64 *)v_anime->verts_top[v_anime->buffer_clock];
	v_anime->obj->verts = (SVECTOR *)svec;
	for (i = mdl->n_packs; i > 0; i--){
		n = mdl_pack->n_verts;
		for (j = (n + 1) / 2; j > 0; j--){
			svec[0] = (svec[0] & mask1) | (buffer[index[0]] & mask2);
			svec[1] = (svec[1] & mask1) | (buffer[index[1]] & mask2);
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)svec & 63) == 0) asm("pref 0,64(%0)"::"r"(svec));
			svec += 2;
			index += 2;
		}
		mdl_pack++;
	}
}

/* ---------------------------------------------------------------- */

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonNormal(FVECTOR *verts, int n_verts)
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = ((int)verts->vx) << 12;
		svec->vy = ((int)verts->vy) << 12;
		svec->vz = ((int)verts->vz) << 12;
	}
}

	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する(法線正規化付き)
	*/
void DG_RegistCommonNormal2(FVECTOR *verts, int n_verts)
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	VECTOR vecTmp;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		D3DXVec3Normalize(&vecTmp, verts);
		svec->vx = ((int)vecTmp.vx) << 12;
		svec->vy = ((int)vecTmp.vy) << 12;
		svec->vz = ((int)vecTmp.vz) << 12;
	}
}

	/*
		スクラッチパッド上の法線データをモデルの法線データに振り分ける
	*/
void DG_RefineStripNormal(DG_VERTS_ANIME *v_anime, short *index)
{
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	u_long64		*norm, mask1, mask2, *buffer;
	int			i, j, n;

	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (u_long64 *)SCRPAD_ADDR;

	mask1 = 0xffff000000000000;
	mask2 = 0x0000ffffffffffff;

	norm = (u_long64 *)v_anime->norms_top[v_anime->buffer_clock];
	v_anime->obj->norms = (SVECTOR *)norm;
	for (i = mdl->n_packs; i > 0; i--){
		n = mdl_pack->n_verts;
		for (j = (n + 1) / 2; j > 0; j--){
			norm[0] = (norm[0] & mask1) | (buffer[index[0]] & mask2);
			norm[1] = (norm[1] & mask1) | (buffer[index[1]] & mask2);
			//if (((int)index & 63) == 0) asm("pref 0,64(%0)"::"r"(index));
			//if (((int)norm & 63) == 0) asm("pref 0,64(%0)"::"r"(norm));
			norm += 2;
			index += 2;
		}
		mdl_pack++;
	}
}

/* ---------------------------------------------------------------- */

	/*
		FVECTORをintに変換し、スクラッチパッドへ転送する
	*/
void DG_RegistCommonUV(FVECTOR *verts, int n_verts)
{
	SVECTOR	*svec = (SVECTOR *)SCRPAD_ADDR;
	int i;

	for (i = n_verts; i > 0; i--, svec++, verts++) {
		svec->vx = ((int)verts->vx) << 12;
		svec->vy = ((int)verts->vy) << 12;
	}
}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
	*/
void DG_RefineStripUV(DG_VERTS_ANIME *v_anime, short *index)
{
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	u_int		*uv, *buffer;
	int			i, j, n;

	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (u_int*)SCRPAD_ADDR;

	uv = (u_int *)v_anime->uvs_top[v_anime->buffer_clock];
	v_anime->obj->uvs[0] = (TVECTOR_S *)uv;
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
}

	/*
		スクラッチパッド上のＵＶデータをモデルのＵＶデータに振り分ける
		（頂点・法線のインデックスを使用する場合）
	*/
void DG_RefineStripUV2(DG_VERTS_ANIME *v_anime, short *index)
{
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	u_int		*uv, *buffer;
	int			i, j, n;

	mdl = v_anime->obj->model;
	mdl_pack = mdl->packs;
	buffer = (u_int *)SCRPAD_ADDR;

	uv = (u_int *)v_anime->uvs_top[v_anime->buffer_clock];
	v_anime->obj->uvs[0] = (TVECTOR_S *)uv;
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
}
