/*
	chain.c
	チャンネル処理ユニット／オブジェクトＤＭＡ接続ルーチン

	1999/07/07 K.Takabe
	$Id: chain.cpp,v 1.1.1.3 2002/11/19 11:42:02 Yoshizawa1 Exp $

*/
/*

	void		DG_ChainChanl( cp, which)
	DG_CHANL	*cp;		チャンネル構造体
	int		which;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する


	void		DG_SortChainChanl( cp, which)
	DG_CHANL	*cp;		チャンネル構造体
	int		which;		ダブルバッファ選択

	キューされた各オブジェクトをソートしてＤＭＡパケットに接続する
	同時にソート済みのプリミティブオブジェクトも表示する

*/


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
#include "private.h"
#include "shader.h"

//-----------------------------------------------------------------------------

#if 1
DWORD DG_PixelShaderHandle[DG_PSHADER_NUM];
static char *DG_PixelShaderName[DG_PSHADER_NUM] = {
	"",
	"plight",  // ポイントライト処理シェーダ
	//"normal",  // 通常のシェーダ
	"test",	   // テスト用
};

// 常駐ピクセルシェーダ初期化/ピクセルシェーダは使うかどうか謎
void DG_InitPixelShader(void)
{
	int i;
	// 0番目はデフォルトシェーダ
	DG_PixelShaderHandle[DG_PSHADER_NONE] = 0;

	// シェーダのロード
	for (i = 1; i < DG_PSHADER_NUM; i++) {
		DG_CreatePixelShader(GV_StrCode(DG_PixelShaderName[i]),
							 &DG_PixelShaderHandle[i]);
	}
}
#else
void DG_InitPixelShader(void)
{
}
#endif

// 頂点シェーダ場合分け
// 分岐の要素は以下の通り。
//
// ・平行光源計算
// ・ウェイト計算
// ・追加光源計算
// ・点光源計算
// ・点光源処理でピクセルシェーダを使うかどうか
//
// まじめに全部作ると32通りになってしまうので、
//
// ・平行光源処理+点光源処理は同じシェーダで行う。
//   点光源無効OBJの場合は点光源色を(0,0,0)にしておく
//
// ・点光源計算をした場合、ピクセルシェーダに渡す値も一緒に書き込む
//
// という制限をつける。
//
enum {
	DG_OBJ_LIGHT   = (0x01 << 0), // 平行光源 + 環境光
	DG_OBJ_WEIGHT  = (0x01 << 1), // ウェイト計算
	DG_OBJ_ALIGHT  = (0x01 << 2), // 追加平行光源 + 環境光
	DG_OBJ_PLIGHT  = (0x01 << 3), // 点光源
};

extern "C" {
DWORD DG_ObjVertexShaderHandle[DG_VSHADER_OBJ_NUM];
}
static char *DG_ObjVertexShaderName[] = {
	"obj",   // 普通の変換のみ
	"objl",  // 平行光源
	"objw",  // ウェイト計算
	"objwl", // ウェイト計算
	
	"obja",  // 追加光源
	"objal", // 追加光源 + 平行光源
	"objaw", // 追加光源 + ウェイト計算
	"objawl",// 追加光源 + ウェイト計算 + 平行光源

	"objp",  // 通常変換 + ポイントライト
};
void DG_InitObjVertexShader(void)
{
	// TODO:バンド幅節約のためには、不要なメンバが少ない方が良い。
	DWORD dwObjDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),   // position
		D3DVSD_REG(1,  D3DVSDT_FLOAT1),   // blend
		D3DVSD_REG(2,  D3DVSDT_FLOAT3),   // normal
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR), // diffuse
		D3DVSD_REG(7,  D3DVSDT_FLOAT2),   // uv0
		D3DVSD_REG(8,  D3DVSDT_FLOAT2),   // uv1
		D3DVSD_REG(9,  D3DVSDT_FLOAT2),   // uv2
		D3DVSD_END()
	};
	int i;

	for (i = 0; i < DG_VSHADER_OBJ_NUM; i++) {
		DG_CreateVertexShader(GV_StrCode(DG_ObjVertexShaderName[i]),
							  dwObjDecl,
							  &DG_ObjVertexShaderHandle[i]);
	}
}

//-----------------------------------------------------------------------------  

#define _CopyVector(a, b) { *(VECTOR *)(a) = *(VECTOR *)(b); }
#define _CopyMatrix(a, b) { *(MATRIX *)(a) = *(MATRIX *)(b); }
#define Vu0CopyMatrix(_a, _b) _CopyMatrix(_a, _b)

#define ALIGNSIZE1(_n)   (_n)
#define ALIGNSIZE16(_n)  (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n)  (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	 (sizeof(_v) / sizeof(int))
#define SIZEOF_QWORD(_v) (sizeof(_v) / 16)

//-----------------------------------------------------------------------------

#if 0
typedef unsigned int DG_DMATAG;
typedef unsigned int DG_GIFTAG;
typedef unsigned int DG_VIFCODE;
#endif

/* ＶＵ１ワークパラメータ */
typedef ALIGN16_DECL(struct) _vu1_work {
	FVECTOR	clamp_min;		/* クランプ処理用最小値（現在未使用） */
	FVECTOR	clamp_max;		/* クランプ処理用最大値（現在未使用） */
	float	fog_param1, fog_param2, fog_param3, fog_param4;	/* フォグパラメータ */
	float	color_clip, poly_alpha, specular_mul, specular_clip;	/* 各種定数 */
	FVECTOR	scale;			/* プリミティブ座標算出用スケール値（xyz=pvec*scale+offset） */
	FVECTOR	offset;		/* プリミティブ座標算出用オフセット */
	float	param1, param2, param3, param4;					/* 各種定数２ */
	FVECTOR	aspect;
	FMATRIX	pers;			/* 透視変換マトリクス */
} Vu1Work ;

/* ＶＵ１メモリ初期化パケット */
typedef ALIGN16_DECL(struct) _vu1_init_packet {
	DG_DMATAG	init_vif;
	u_int		init_vifcode[12];
	DG_DMATAG	init_vu1_prog;
	DG_DMATAG	init_param_trans;
	Vu1Work		init_param;
} VU1_INIT_PACKET ;

//-----------------------------------------------------------------------------

/*
  各種ローカル構造体定義
*/

/* １パケット分のＤＭＡ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _pack_dma_packet {
	DG_DMATAG		verts_trans;		/* 頂点転送ＤＭＡタグ */
	DG_DMATAG		norms_trans;		/* 法線転送ＤＭＡタグ */
	DG_DMATAG		uvs0_trans;		/* ＵＶ０転送ＤＭＡタグ */

	DG_DMATAG		trans_tag;			/* パラメータ転送ＤＭＡタグ */
	struct _pack_dma_datas{
		int			n_verts;
		int			flag;
		int			fog;
		int			bound;
	} datas;
	DG_VIFCODE		prog_exec;
} PACK_DMA_PACKET ;

/* オブジェクトマトリクススタック構造体 */
typedef struct _matrix_stack {
	DG_DMATAG	next_tag;
	struct _stack_datas {
		FMATRIX		screen;
		FMATRIX		local_light;
		FMATRIX		local_color;
		FMATRIX		connection;
		FMATRIX		option;
		int			n_verts;
		int			flag;
		int			fog;
		int			bound;
		DG_GIFTAG	giftag_normal;	/* 通常描画用描画ＧＩＦタグ */
		DG_GIFTAG	giftag_clip;	/* クリップポリゴン用描画ＧＩＦタグ */
		IVECTOR		gif_regslist;	/* マルチテクスチャ用レジスタリスト */
	} datas;
} MATRIX_STACK;

/* テクスチャ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _tex_dma_packet {
	DG_DMATAG		tex0_trans;		/* テクスチャ０転送ＤＭＡパケット */
} TEX_DMA_PACKET ;

/* ＤＭＡ転送データパケット */
typedef ALIGN16_DECL(struct) _packet_buffer {
	/* マトリクス転送 */
	DG_DMATAG			mat_trans_tag;
	/* テクスチャ情報転送 */
	TEX_DMA_PACKET		tex_packet;
	/* 頂点情報転送 */
	PACK_DMA_PACKET		pack_packet;
} PACKET_BUFFER ;


//-----------------------------------------------------------------------------

/*
  スクラッチパッドワーク定義
*/

/* ソート用ワーク */
typedef struct {
	void		*tag;
	u_int		z;
	void		*obj;
} SORT_TAG;

typedef struct {
	SORT_TAG	list[768];
	void		*ot3[64];
	void		*ot2[64];
	void		*ot1[64];
} SORT_WORK;

/* 通常モデル描画用ワーク */
typedef struct {
	DG_OBJS		*objs;
	int			use_tri;
} OBJ_LIST;

/* オブジェクトマトリクスストア用ワーク */
typedef struct {
	FMATRIX			world;
	FMATRIX			screen;
	FMATRIX			inv_mat;
} OBJ_MATRIX;

typedef struct {
	u_long64          *current_matrix_addr;
	int             pad[3];	
	OBJ_MATRIX      buffer[2][8];
	u_long64          local_work[0];
} MATRIXSTORE_WORK;

/* ベーススクラッチパッドワーク定義 */
typedef struct {
	/* ＤＭＡ生成用バッファ */
	MATRIX_STACK    matrix_stack;       /* マトリクススタック */
	PACKET_BUFFER   pre_packet[2];      /* ＤＭＡパケット */
	u_long64          dma_buffer[128];    /* 汎用ＤＭＡパケット生成バッファ */

	/* オブジェクト状態管理用 */
	FMATRIX     screen;
	FMATRIX     local_light;
	FMATRIX		local_color;
	FMATRIX		connection;
	int			fog;
	float		fog_param1, fog_param2;
	short		vu_prog_list[16];
	FMATRIX		eye_pers;
	FMATRIX		eye_inv;

	/* 各種ワーク */
	int			max_num;
	void		*now;
	int			invisible_flag;
	int			buffer_switch;

	u_long64      *matrix_store_addr;  // StoreMatrixObjs()関数用ローカル変数
	void        *matrix_addr;        // ChainObj() 関数用ローカル変数
	DG_OBJS		*current_objs;       // 
	u_long64	local_work[1024 * 4];    // ローカルワーク
} ScrpadWork;

// 汎用メモリワーク
// (主にスクラッチパッド上で構成し終わったデータなどを退避)
extern "C" {
u_long64 DG_AS_GetLocalMemoryAddress()[1024 * 4];
}

//-----------------------------------------------------------------------------  

static void PrintMatrix(char *str, MATRIX *mat)
{
	printf(str);
	printf("%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n",
		   mat->m[0][0], mat->m[0][1], mat->m[0][2], mat->m[0][3],
		   mat->m[1][0], mat->m[1][1], mat->m[1][2], mat->m[1][3],
		   mat->m[2][0], mat->m[2][1], mat->m[2][2], mat->m[2][3],
		   mat->m[3][0], mat->m[3][1], mat->m[3][2], mat->m[3][3]);
}


// オブジェクトを描画する
static void ChainObj(DG_OBJ *obj)
{
	static const float afConst[][4] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
	};

	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_PACKET *pkt;
	DG_VBUF_INFO  *info;
	DG_INDEX_INFO *index;
	MATRIX        matTmp;
	u_long64        alpha;
	int           i, j;
	int           vs_flag; // 頂点シェーダ分岐用フラグ
	
	pkt = obj->packets;
	if (pkt == NULL) return;
	index = pkt->index;
	
	DG_SetTexture(0, pkt->ptex); 	// TODO: マルチテクスチャ管理
	//DG_SetTexture(0, NULL); 	// TODO: マルチテクスチャ管理

	//info = pkt->vbuf_info;
	info = pkt->index->vbuf_info;
	DG_SetStreamSource(0, info->vbuf, info->stride);
	DG_SetIndices(info->ibuf, 0);

	vs_flag = 0;

	// TODO:ライティング判定はOBJS毎で良いため、無駄。
	if (!(obj->flag & DG_FLAG_PAINT)) { // 光源計算あり
		vs_flag |= DG_OBJ_LIGHT;
		//if (obj->flag & DG_FLAG_POINTLIGHT	&& DG_PointLightFlag) {  // 点光源計算有り
		if (0) {  // 点光源計算有り
			DG_SetVertexShaderConstant(CV_PLIGHTPOS, &DG_PointLightPos[0], DG_POINTLIGHT_MAX);
			DG_SetVertexShaderConstant(CV_PLIGHTCOL, &DG_PointLightCol[0], DG_POINTLIGHT_MAX);
			DG_SetVertexShaderConstant(CV_PLIGHTPARAM, &DG_PointLightParam[0], DG_POINTLIGHT_MAX);
		} else {
			for (i = 0; i < DG_POINTLIGHT_MAX; i++) {
				DG_SetVertexShaderConstant(CV_PLIGHTCOL + i, &DG_ZeroVector, 1);
			}
		}
	}
	if (obj->parent != -1) {
		vs_flag |= DG_OBJ_WEIGHT;        // ウェイトあり
		D3DXMatrixTranspose(&matTmp, &obj->inv_mat);
		DG_SetVertexShaderConstant(CV_WORLD1, &matTmp, 4);
	}
	if (DG_AdditionalLightFlag) {        // 追加光源計算有り
		vs_flag |= DG_OBJ_ALIGHT;
		D3DXMatrixTranspose(&matTmp, &DG_AdditionalLightMatrix);
		DG_SetVertexShaderConstant(CV_ADDLIGHTVEC, &matTmp, 4);
		DG_SetVertexShaderConstant(CV_ADDLIGHTCOL, &DG_AdditionalColorMatrix, 4);
	}
	//if (obj->flag & DG_FLAG_POINTLIGHT2 && DG_PointLightFlag) {  // 点光源計算有り
	if (0) {  // 点光源計算有り
		vs_flag = DG_OBJ_PLIGHT; // '=' なので注意。		
		DG_SetVertexShaderConstant(CV_PLIGHTPOS, &DG_PointLightPos[0], DG_POINTLIGHT_MAX);
		DG_SetVertexShaderConstant(CV_PLIGHTCOL, &DG_PointLightCol[0], DG_POINTLIGHT_MAX);
		DG_SetVertexShaderConstant(CV_PLIGHTPARAM, &DG_PointLightParam[0], DG_POINTLIGHT_MAX);
		DG_SetPixelShader(DG_PixelShaderHandle[DG_PSHADER_POINTLIGHT]);
	} else {
		DG_SetPixelShader(0);
	}
	DG_SetVertexShader(DG_ObjVertexShaderHandle[vs_flag]);

	// 共通定数
	DG_SetVertexShaderConstant(CV_ZERO, afConst[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, afConst[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, afConst[2], 1);
	D3DXMatrixTranspose(&matTmp, &obj->world);
	DG_SetVertexShaderConstant(CV_WORLD0, &matTmp, 4);
	D3DXMatrixTranspose(&matTmp, &scrpad->eye_pers);
	DG_SetVertexShaderConstant(CV_EYEPERS, &matTmp, 4);
	DG_SetAlphaMode(0);	
	// 不透明パケットを先に描く
	for (j = obj->n_packs; j > 0; j --, pkt++, index++) {
		alpha = pkt->tex_ptr[0]->alpha.data;
		if (alpha == 0) {
			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, index->min, index->num,
									index->start, index->prims);
		}
	}

	// 半透明パケット
	pkt = obj->packets;
	index = pkt->index;
	for (j = obj->n_packs; j > 0; j --, pkt++, index++) {
		alpha = pkt->tex_ptr[0]->alpha.data;
		if (alpha != 0) {
			DG_SetAlphaMode(alpha);
			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, index->min, index->num,
									index->start, index->prims);
		}
	}
}


//-----------------------------------------------------------------------------

// オブジェクトのソートリストを作成する
// work->ot1 に接続する
static void MakeSortList(DG_OBJS *objs)
{
	ScrpadWork  *work = (ScrpadWork *)SCRPAD_ADDR;
	SORT_WORK   *sort_work;
	SORT_TAG    *list;
	DG_OBJ      *obj;
	int         i;
	u_int       z;
	void        **ot;

	sort_work = (SORT_WORK *)DG_AS_GetLocalMemoryAddress();

	list = (SORT_TAG *)work->now;
	obj = objs->objs;
	for (i = 0; i < objs->n_models; i ++, obj ++) {
		if (obj->bound_mode & 2) continue;
		if (!(obj->mdl_type & DG_TYPE_TRANS)) continue;
		if (obj->flag & work->invisible_flag) continue;
		/* ソート用データを作成 */
		z = (((u_int)obj->sort_z + 0x7fffff) >> 6);
		list->obj = obj;
		list->z = z;
		z &= 63;
		ot = sort_work->ot1 + z;
		list->tag = *ot;
		*ot = list;
		list ++;
		work->max_num ++;
	}
	work->now = list;
}

// ソートを行う
static void SortList(void)
{
	ScrpadWork  *work = (ScrpadWork *)SCRPAD_ADDR;
	SORT_WORK   *sort_work;
	int         i, z;
	SORT_TAG    *list;
	void        **ot2, **ot3, *next_addr;

	sort_work = (SORT_WORK *)DG_AS_GetLocalMemoryAddress();

	/* 大きい方からソートしていく */
	for (i = 63; i >= 0; i --) {
		next_addr = sort_work->ot1[i];
		while (next_addr != NULL){
			list = (SORT_TAG *)next_addr;
			next_addr = list->tag;
			z = list->z; z = (z >> 6) & 63;
			ot2 = sort_work->ot2 + z;
			list->tag = *ot2;
			*ot2 = list;
		}
	}

	/* 小さい方からソートしていく */
	for (i = 0; i < 64  ; i++){
		next_addr = sort_work->ot2[i];
		while (next_addr != NULL){
			list = (SORT_TAG *)next_addr;
			next_addr = list->tag;
			z = list->z; z = (z >> 12) & 63;
			ot3 = sort_work->ot3 + z;
			list->tag = *ot3;
			*ot3 = list;
		}
	}
}

extern "C" {
	extern void	*DG_Prim2OT[64];
}
// DG_Prim2OT, sort_work->ot3 を辿って描画する
static void SortChainObjs(DG_CHANL *cp, int which)
{
	ScrpadWork  *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	SORT_WORK   *sort_work;
	DG_OBJ      *obj;
	int         k, obj_flag;
	void        *next_addr, *next_addr_prim;
	SORT_TAG    *list;
	DG_PRIM2_PACKET	*prim_packet;

	sort_work = (SORT_WORK *)DG_AS_GetLocalMemoryAddress();

	for (k = 63; k >= 0 ; k --) {
		next_addr = sort_work->ot3[k];
		next_addr_prim = DG_Prim2OT[k];
		DG_Prim2OT[k] = NULL;
		while (next_addr != NULL || next_addr_prim != NULL) {
			list = (SORT_TAG *)next_addr;
			prim_packet = (DG_PRIM2_PACKET *)next_addr_prim;

			// 処理を行うオブジェクトを決定(マージソート)
			if (next_addr_prim == NULL || next_addr == NULL) {
				if (next_addr == NULL) { // obj 無し
					obj_flag = 0;
				} else {
					obj_flag = 1;
				}
			} else {
				// objのzの方が大きかったらobjを先に描画
				if (list->z > (u_int)prim_packet->sort_z) {
					obj_flag = 1;
				} else {
					obj_flag = 0;
				}
			}

			if (obj_flag) {
				// ソートしたオブジェクトを接続
				next_addr = list->tag;
				obj = (DG_OBJ *)list->obj;
				scrpad->fog = obj->fog;
				scrpad->matrix_addr = obj->matrix_addr;
				// オブジェクト描画
				ChainObj(obj);
			} else {
				// ソートしたプリミティブを接続
				next_addr_prim = prim_packet->next_addr;
				// PRIM2描画
				DG_WritePrimPacks(prim_packet, cp, which);
			}
		}
	}
}

//-----------------------------------------------------------------------------

// SortChanl用のモードを設定する
static void DG_SortChanlRenderState(DG_CHANL *cp)
{
	// カメラ/透視変換マトリクスの設定
	DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	DG_SetTransform(D3DTS_PROJECTION, &cp->pers);
}

// キューされた各オブジェクトをソートしてＤＭＡパケットに接続する(半透明&プリミティブ)
void DG_SortChainChanl(DG_CHANL *cp, int which)
{
	ScrpadWork      *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJS         **oque, *objs;
	DG_OBJ_QUEUE    *que;
	DG_OBJ_BUFFER   *obj_buff;
	//DG_TEXTURE_LIST *tex_list;
	int             i, gid, c_gid;
	SORT_WORK       *sort_work;

	que = cp->obj_queue;

	if (que == NULL) {
		printf("que == NULL\n");
		return;
	}
	obj_buff = &que->objs_buffer;
	c_gid = cp->group_id;

	/* ワーク初期化 */
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress();
	memset(sort_work->ot3, 0, sizeof(int)* 64 * 3);
	scrpad->max_num = 0;
	scrpad->now = sort_work->list;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num;
	scrpad->buffer_switch = 0;
	scrpad->fog_param1 = DG_FogParam1;
	scrpad->fog_param2 = DG_FogParam2;

	/* リストの作成 */
	oque = (DG_OBJS **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; -- i, oque ++) {
		objs = *oque;
		if (objs->flag & DG_FLAG_MULTITEX) continue;
		if (objs->flag & scrpad->invisible_flag) continue;
		if (objs->flag & (DG_FLAG_PLUGINDRAW | DG_FLAG_TEST)) continue;
		if (!(objs->group_id & cp->group_id)) continue;
		if (objs->flag & (DG_FLAG_SHADOWVOL | DG_FLAG_SHADOWWRITE)) continue;
		if ((gid = objs->group_id) != 0 && !(gid & c_gid)) continue;
		// sort_work->ot1 に接続する
		MakeSortList(*oque);
	}
	SortList(); // sort_work->ot1 をソートして sort_work->ot3 に入れる

	DG_SortChanlRenderState(cp);

	DG_BeginScene();
	SortChainObjs(cp, which); // 描画
	DG_EndScene();
}

//-----------------------------------------------------------------------------

// 不透明モデル描画
static void ChainObjs(DG_OBJS *objs)
{
	ScrpadWork    *scrpad = (ScrpadWork*)SCRPAD_ADDR;
	DG_OBJ        *obj;
	int         i;

	if (!(objs->flag & DG_FLAG_PAINT)) {
		MATRIX matTmp;
		D3DXMatrixTranspose(&matTmp, &objs->light[0]);
		DG_SetVertexShaderConstant(CV_LIGHTVEC, &matTmp, 4);
		DG_SetVertexShaderConstant(CV_LIGHTCOL, &objs->light[1], 4);
	}

	scrpad->current_objs = objs;
	obj = objs->objs;
	for (i = objs->n_models; i > 0; i --, obj ++) {
		if (obj->bound_mode & 2) continue;                // クリップアウトチェック
		if (obj->mdl_type & DG_TYPE_TRANS) continue;      // 半透明なら描かない
		if (obj->flag & scrpad->invisible_flag) continue; // 不可視なら描かない
		scrpad->fog = obj->fog;
		scrpad->matrix_addr = obj->matrix_addr;
		// OBJ描画
		ChainObj(obj);
	}
}

//-----------------------------------------------------------------------------

// chanl 毎のレンダリングステートを設定する
static void DG_ChanlRenderState(DG_CHANL *cp)
{
	// カメラ/透視変換マトリクスの設定
	DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	DG_SetTransform(D3DTS_PROJECTION, &cp->pers);

	// アルファブレンディングOFF
	DG_AlphaBlendDisable();
}

// キューされた各オブジェクトをＤＭＡパケットに接続する(不透明オブジェクト)
void DG_ChainChanl(DG_CHANL *cp, int which)
{
	ScrpadWork      *scrpad = (ScrpadWork*)SCRPAD_ADDR;
	DG_OBJS         **oque, *objs;
	DG_OBJ_QUEUE    *que;
	DG_OBJ_BUFFER   *obj_buff;
	int             i, j, old_tex_code, max_objs, max_mem_objs;
	//int             i, j, old_tex_code, max_objs, max_mem_objs;
	DG_TEXTURE_LIST	*tex_list;
	int             c_gid;
	//int             gid;
	OBJ_LIST        *obj_list, *mem_obj_list;

	que = cp->obj_queue;

	if (que == NULL) return;

	obj_buff = &que->objs_buffer;

	/* スクラッチパッド内の必要なワークを初期化する */
	c_gid = cp->group_id;
	scrpad->eye_pers = cp->eye_pers;
	scrpad->eye_inv = cp->eye_inv;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num;
	scrpad->buffer_switch = 0;
	scrpad->fog_param1 = DG_FogParam1;
	scrpad->fog_param2 = DG_FogParam2;

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST *)scrpad->local_work;
	max_objs = 0;
	oque = (DG_OBJS **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; -- i, oque ++) {
		objs = *oque;
		// TODO:仮でチェックを外しておく
		//if (objs->flag & DG_FLAG_MULTITEX) continue;
		if (objs->flag & scrpad->invisible_flag) continue;
		if (objs->flag & (DG_FLAG_PLUGINDRAW | DG_FLAG_TEST)) continue;
		//if (!(objs->group_id & cp->group_id)) continue;
		if (objs->flag & (DG_FLAG_SHADOWVOL | DG_FLAG_SHADOWWRITE)) continue;
		//if ((gid = objs->group_id) != 0 && !(gid & c_gid)) continue;
		if (objs->bound_mode == 2) continue;
		obj_list[max_objs].objs = objs;
		obj_list[max_objs].use_tri = objs->tri_id;
		max_objs ++;
	}

	if (max_objs == 0) return;

	// オブジェクトリストをテクスチャ毎にソートしてメモリに書き出す
	mem_obj_list = (OBJ_LIST *)DG_AS_GetLocalMemoryAddress();
	max_mem_objs = 0;

	// オブジェクトをTRI毎にソートする
	for (tex_list = DG_TextureList, j = DG_MaxTextures; j > 0; tex_list++, j--) {
		//if (tex_list->header->compress_flag & TRI_FLAG_TRANS) continue;
		for (i = 0; i < max_objs; i++) {
			if (tex_list->code != obj_list[i].use_tri) continue;
			mem_obj_list->objs = obj_list[i].objs;
			/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list->use_tri = (int)tex_list;
			mem_obj_list ++;
			max_mem_objs ++;
		}
	}
	
	// メモリに書き出したソート完了済みオブジェクトを描画する
	DG_ChanlRenderState(cp); // chanl 毎の RenderState を設定する
	DG_BeginScene();
	mem_obj_list = (OBJ_LIST *)DG_AS_GetLocalMemoryAddress();
	old_tex_code = -1;
	for (i = max_mem_objs; i > 0; i--) {
		// Xbox追加:ユーザ定義シェーダ使用コールバック
		if (mem_obj_list->objs->exec_func != NULL) {
			mem_obj_list->objs->exec_func(cp, which,
										  mem_obj_list->objs,
										  mem_obj_list->objs->extend_data);
		} else {
			ChainObjs(mem_obj_list->objs);			
		}
		mem_obj_list++;
	}
	DG_SetPixelShader(0);
	DG_EndScene();

#ifdef __THIS_IS_OLD_VERSION__
#if 0
	// TODO: オブジェクトのテクスチャ毎ソート
	for (tex_list = DG_TextureList, j = DG_MaxTextures; j > 0; tex_list ++, j --) {
		//if (tex_list->header->compress_flag & TRI_FLAG_TRANS) continue;
		for (i = 0; i < max_objs; i ++) {
			// 仮
			//if (tex_list->code != obj_list[i].use_tri) continue;
			mem_obj_list->objs = obj_list[i].objs;
			/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list->use_tri = (int)tex_list;
			mem_obj_list ++;
			max_mem_objs ++;
		}
	}

	/* メモリに書き出したソート完了済みオブジェクトをテクスチャ転送を挟みながら描画する */
	mem_obj_list = (OBJ_LIST *)DG_AS_GetLocalMemoryAddress();

	old_tex_code = - 1;
	for (i = max_mem_objs; i > 0; i --){
		/* テクスチャが前回と違っていたら転送する */
		if (mem_obj_list->use_tri != old_tex_code) {
			tex_list = (DG_TEXTURE_LIST *)mem_obj_list->use_tri;
			tex_list->flag = 1;
			//size = DG_WriteTextureChangePacks2(scrpad->dma_buffer, tex_list, which);
			//DG_SendCacheFIFO(scrpad->local_work, scrpad->dma_buffer, size);
			old_tex_code = mem_obj_list->use_tri;
			tex_list ++;
		}
		ChainObjs(mem_obj_list->objs);
		mem_obj_list ++;
	}
#else
	// 仮/そのまま書き出しておく。
	// テクスチャまわりの仕様決定後に書き直す
	for (i = 0; i < max_objs; i ++) {
		mem_obj_list->objs = obj_list[i].objs;
		mem_obj_list ++;
		max_mem_objs ++;
	}
	/* メモリに書き出したソート完了済みオブジェクトをテクスチャ転送を挟みながら描画する */

	// chanl 毎の RenderState を設定する	
	DG_ChanlRenderState(cp);
	DG_BeginScene();
	mem_obj_list = (OBJ_LIST *)DG_AS_GetLocalMemoryAddress();
	old_tex_code = - 1;
	for (i = max_mem_objs; i > 0; i --) {
		ChainObjs(mem_obj_list->objs); // 描画
		mem_obj_list ++;
	}
	DG_SetPixelShader(0);
	DG_EndScene();
#endif
#endif
}

//-----------------------------------------------------------------------------
