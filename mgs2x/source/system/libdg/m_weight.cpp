/*
	m_weight.c
	マルチウェイトエンベロープ補助ルーチン

	1999/09/07 K.Takabe
	$Id: m_weight.cpp,v 1.1.1.3 2002/11/19 11:42:12 Yoshizawa1 Exp $

*/

#ifdef KP_XBOX //BP


#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"

//-----------------------------------------------------------------------------

// マルチウェイト計算用マトリクスの最大数
#define DG_SKELTON_MATRIX_MAX    (64)

//-----------------------------------------------------------------------------

// マルチウェイト計算用マトリクス
static MATRIX DG_SkeletonMatrix[DG_SKELTON_MATRIX_MAX];

//-----------------------------------------------------------------------------
		
// マルチウェイト計算マイクロプログラム読み込み
void DG_StartMultiWeightSupport(void)
{
	// XBOXでは空関数
}


//-----------------------------------------------------------------------------
		
// マルチウェイト計算マトリクスの読み込み
void DG_StoreSkeletonMatrix(MATRIX *mats, int num)
{
	ASSERT(num < DG_SKELTON_MATRIX_MAX);
	memcpy(DG_SkeletonMatrix, mats, num * sizeof(MATRIX));
}

#if 0
// 頂点に対してマルチウェイト計算を行う
void DG_TransMultiWeightVertex(VECTOR *verts, VECTOR *weight, char *mat_id, int n_verts)
{
}

// 法線に対してマルチウェイト計算を行う
void DG_TransMultiWeightNormal(VECTOR *norms, VECTOR *weight, char *mati_d, int n_verts)
{
}
#endif

/* 頂点・法線に対してマルチウェイト計算を行う */
void DG_TransMultiWeightVertexNormal(
		VECTOR *res_verts, VECTOR *res_norms,                       // 結果
		VECTOR *verts, VECTOR *norms, VECTOR *weight, char *mat_id, // データ
		int n_verts)                                                // 入力頂点数
{
	struct _work {
		VECTOR    trans_verts[256];
		VECTOR    trans_norms[256];
		VECTOR    weight[256];
		int       index[256];
	} *work;
	int         i, count;
	int         *index;
	VECTOR      *tv, *tn, *w;	

	work = (struct _work *)SCRPAD_ADDR;

	// 256頂点ごとに処理する
	while (n_verts > 0) {
		count = n_verts;
		if (count > 256) count = 256;
		n_verts -= 256;

		// ウェイト値をメモリからワークへコピー
		memcpy(work->weight, weight, count * sizeof(VECTOR));
		weight += count;

		// マトリクスインデックスをメモリからワークへコピー
		memcpy(work->index, mat_id, count * sizeof(int));
		mat_id += count * 4;

		/* Ｖｕ０マイクロプログラムを用いてマルチウェイト計算 */
		index = work->index;
		w = work->weight;
		tv = work->trans_verts; // 頂点変換結果
		tn = work->trans_norms; // 法線変換結果
		
		for (i = count; i > 0; i --) {
			printf("verts: %f, %f, %f\n", verts->x, verts->y, verts->z, verts->w);
			printf("norms: %f, %f, %f\n", norms->x, norms->y, norms->z, norms->w);
			printf("w    : %08x\n", w);
			printf("index: %d\n", index);
			verts ++;
			norms ++;
			w ++;
			index ++;
			tv ++;
			tn ++;			
		}

		// 頂点の計算結果をメモリへ書き戻す
		memcpy(res_verts, work->trans_verts, sizeof(VECTOR));
		res_verts += count;

		// 法線の計算結果をメモリへ書き戻す
		memcpy(res_norms, work->trans_norms, sizeof(VECTOR));
		res_norms += count;
	}
}

//-----------------------------------------------------------------------------

#endif
