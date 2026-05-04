/*
	patch.c
	チャンネル処理ユニット／パッチ曲面オブジェクト処理ルーチン

	2000/07/11 K.Takabe
	$Id: patch.cpp,v 1.1.1.3 2002/11/19 11:42:17 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／パッチ曲面オブジェクト処理ルーチン

	このパッチ曲面オブジェクトとはグループ化された四角形パッチ曲面を
	管理するオブジェクトです。曲面生成には頂点及び各頂点での傾きベクトル
	が必要になります。


	void		DG_PatchChanl(cp, which)
	DG_CHANL	*cp;		チャンネル構造体
	int		which;		ダブルバッファ選択

	キューされたパッチ曲面オブジェクトのパケット作成などを行なう


	DG_PATCH*	DG_MakePatch(int flag, int n_comdl, int chanl)
	int			flag;			処理フラグ
	int			n_patch;		パッチ曲面数
	int			chanl;			処理チャンネル

	パッチ曲面オブジェクトを作成する（作成後はDG_QueuePatchObjs()で登録すること）
	実際にはこの関数を使用せずに下のDG_MakePatchMesh()関数を使う方いい。


	void	DG_FreePatch(DG_PATCH *patch)
	DG_PATCH	*patch;		開放オブジェクト

	メモリを破棄する（これを呼ぶ前にDG_DequeuePatchObjs()でシステムから削除しておくこと）

	------------------------------------------------

	DG_PATCH*	DG_MakePatchMesh(int flag, int n_vert_s, int n_verts_t)
	int		flag;			初期化フラグ
	int		n_vert_s;		Ｓ軸方向頂点数（２以上）
	int		n_verts_t;		Ｔ軸方向頂点数（２以上）

	メッシュの頂点数で確保＆自動初期化


	void DG_SetupPatchMeshBounding(DG_PATCH *patch)
	DG_PATCH	*patch;		対象オブジェクト

	バウンディング及び中心座標自動設定（とても遅い）


	void DG_ConfigPatchLOD(DG_PATCH *patch, int level)
	DG_PATCH	*patch;		対象オブジェクト
	int			level;			ＬＯＤ基準距離（2^nで指定する）

	ＬＯＤパラメータ設定

	------------------------------------------------

	static	inline	void	DG_SwitchBuffPatch(DG_PATCH *patch)
	DG_PATCH	*patch;		対象オブジェクト

	頂点バッファの切り替え


*/
#define BUMP_TEST

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

#include "shader.h"

/* ---------------------------------------------------------------- */
/*
  プラグイン固有設定
*/

/* メインチャンネルプラグイン用設定 */
/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(3300559)	/* "patch" */
/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE)
/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL + 0x200)
/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(32)

// LODレベル補正のためのエッジ情報
enum {
#if 1
	EDGE_U = 0x0001, // 上
	EDGE_D = 0x0002, // 下
	EDGE_L = 0x0004, // 左
	EDGE_R = 0x0008, // 右
#else
	// エッジ接続補正OFFのテスト
	EDGE_U = 0,
	EDGE_D = 0,
	EDGE_L = 0,
	EDGE_R = 0,
#endif
} EDGE_FLAG;

/* ---------------------------------------------------------------- */
/*
  プラグイン固有変数
*/

/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0;
static DG_PLUGIN	PatchPlugin;

// 頂点バッファ情報

// 確保する頂点バッファ数(パッチ1つにつき11つ)
#define PATCH_VBUF_NUM   (6)
// 確保するインデックスバッファ数(パーツ1つにつき1つ)
#define PATCH_IBUF_NUM   (256 * PATCH_VBUF_NUM)
#define PATCH_VERTEX_MAX (65536) // 一つのメッシュに許される最大頂点数
#define PATCH_INDEX_MAX  (600)   // 一つのパーツに許される最大インデックス数

static LPDIRECT3DVERTEXBUFFER8 DG_PatchVbuf[PATCH_VBUF_NUM];
static LPDIRECT3DINDEXBUFFER8 DG_PatchIbuf[PATCH_IBUF_NUM];
static int DG_PatchVbufID = 0;
static int DG_PatchIbufID = 0;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor(DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status);

/* ---------------------------------------------------------------- */

/*
 * patch 用頂点シェーダ
 */

enum {
	PATCH_SHADER_DEFAULT, // 透視変換のみ
	PATCH_SHADER_LIGHT,   // 光源計算あり
#ifdef BUMP_TEST	
	PATCH_SHADER_LIGHT_BUMP,   // 光源計算＋バンプ
#endif	
	PATCH_SHADER_NUM,
};

static DWORD DG_PatchVertexShaderHandle[PATCH_SHADER_NUM];
static char *DG_PatchVertexShaderName[PATCH_SHADER_NUM] = {
	"patch",
	"patchl",
#ifdef BUMP_TEST	
	"patchbl",
#endif
};
static DWORD DG_PatchPixelShaderHandle[PATCH_SHADER_NUM];
static char *DG_PatchPixelShaderName[PATCH_SHADER_NUM] = {
	NULL,
	NULL,
#ifdef BUMP_TEST	
	"patchbl",
#endif	
};

void DG_InitPatchVertexShader(void)
{
	DWORD dwPatchDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0, D3DVSDT_FLOAT3),   // position
		D3DVSD_REG(2, D3DVSDT_FLOAT3),   // normal
		D3DVSD_REG(3, D3DVSDT_D3DCOLOR), // diffuse
		D3DVSD_REG(7, D3DVSDT_FLOAT2),   // uv0
		D3DVSD_END()
	};
	int i;

	for (i = 0; i < PATCH_SHADER_NUM; i++) {
		DG_CreateVertexShader(GV_StrCode(DG_PatchVertexShaderName[i]),
							  dwPatchDecl,
							  &DG_PatchVertexShaderHandle[i]);
		if( DG_PatchPixelShaderName[i] != NULL ){
			DG_CreatePixelShader(GV_StrCode(DG_PatchPixelShaderName[i]),
								 &DG_PatchPixelShaderHandle[i]);
		} else {
			DG_PatchPixelShaderHandle[i] = NULL;
		}
	}
}


/*
  プラグイン起動
*/
void DG_AddPluginPatch(void)
{
	if (PluginStartFlag != 0) return;

	/* プラグイン作成 */
	DG_MakePlugin(&PatchPlugin, PLUGIN_ID, PLUGIN_FLAG,
				   PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE);
	DG_AddPlugin(&PatchPlugin);

	PluginStartFlag = 1;

	//Xbox : 頂点バッファの初期化
	int i;
	for (i = 0; i < PATCH_VBUF_NUM; i++) {
		DG_CreateVertexBuffer(
				PATCH_VERTEX_MAX * sizeof(DG_PATCHVERTEX),
				D3DUSAGE_DYNAMIC,
				D3DFVF_PATCHVERTEX,
				D3DPOOL_DEFAULT,
				&DG_PatchVbuf[i]);
	}

	// インデックスバッファの初期化
	for (i = 0; i < PATCH_IBUF_NUM; i++) {
		DG_CreateIndexBuffer(
				PATCH_INDEX_MAX * sizeof(short),
				D3DUSAGE_DYNAMIC,
				D3DFMT_INDEX16,
				D3DPOOL_DEFAULT,
				&DG_PatchIbuf[i]);
	}
}

/*
		プラグイン終了
	*/
void DG_DeletePluginPatch(void)
{
	while (PluginStartFlag != 0){
		DG_DeletePlugin(&PatchPlugin);
		DG_FreePlugin(&PatchPlugin);
	}
	PluginStartFlag = 0;
}


//Xbox : 頂点バッファの解放
void DG_ReleasePatchVertexBuffer(void)
{
	int i;
	for (i = 0; i < PATCH_VBUF_NUM; i++) {
		if (DG_PatchVbuf[i]) {
			DG_PatchVbuf[i]->Release();
			DG_PatchVbuf[i] = NULL;
		}
	}

	for (i = 0; i < PATCH_IBUF_NUM; i++) {
		if (DG_PatchIbuf[i]) {
			DG_PatchIbuf[i]->Release();
			DG_PatchIbuf[i] = NULL;
		}
	}
}


/* ---------------------------------------------------------------- */
/*
  オブジェクト登録
*/
void DG_QueuePatchObjs(DG_PATCH *patch)
{
	if (PluginStartFlag == 0){
		return;
	}
	DG_QueueUserObject(PatchPlugin.obj_buffer, patch);
}

/*
		オブジェクト削除
	*/
void DG_DequeuePatchObjs(DG_PATCH *patch)
{
	if (PluginStartFlag == 0){
		return;
	}
	DG_DequeueUserObject(PatchPlugin.obj_buffer, patch);
}

/* ---------------------------------------------------------------- */

#define ALIGNSIZE1(_n)   (_n)
#define ALIGNSIZE16(_n)  (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n)  (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)  (sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v) (sizeof(_v)/(sizeof(int) * 4))

typedef struct _scrpad_work {
	MATRIX		eye_pers;
	MATRIX		view;
	MATRIX		eye;
	MATRIX		world;
	MATRIX		screen;
	MATRIX		light[2];
	VECTOR		scale;
	VECTOR		tmp_vec;
	//u_long128		dma_buffer[64];
	//MATRIX_PACKET	matrix_packet;
	//PATCH_PACKET	patch_packet[2];
	int			buffer_switch;
	int			last_type;
	//u_long128	local_work[0];
	u_long64		local_work[0];
} ScrpadWork;

/*----------------------------------------------------------------*/
/* Ｖｕ０レジスタ使用スクラッチパッド間のみ使用可能バージョン */
#define Vu0CopyMatrix(_a, _b) ((*(_a)) = (*(_b)))

/*----------------------------------------------------------------*/
static inline void SetLightMatrix(MATRIX *res_light, MATRIX *light, MATRIX *world)
{
	D3DXMatrixMultiply(res_light, world, light);
}

/*----------------------------------------------------------------*/

static int BoundCheck(MATRIX *mat, VECTOR *bound_max, VECTOR *bound_min, VECTOR *scale)
{
	//static const float range = 4.885197850513e-4; //1.0/2047.0
	ScrpadWork     *scrpad;
	int    i, flag, and_flag, or_flag;
	float  w;
	VECTOR vec;

	static DG_POINTVERTEX point[8];
	DG_POINTVERTEX *p = point;

	scrpad = (ScrpadWork *)SCRPAD_ADDR;	

	and_flag = CLIP_FLAG;
	or_flag = 0;

	for (i = 8; i > 0; i --) {
		flag = 0;
		vec.vx = (i & 0x01) ? bound_max->vx : bound_min->vx;
		vec.vy = (i & 0x02) ? bound_max->vy : bound_min->vy;
		vec.vz = (i & 0x04) ? bound_max->vz : bound_min->vz;
		vec.vw = 1.0f;
		// 透視変換
		D3DXVec4Transform(&vec, &vec, mat);

		// clipw命令のエミュレート…
		w = DG_FABS(vec.vw);
		if (vec.vx >   w) flag |= CLIP_X0_FLAG;
		else if (vec.vx < - w) flag |= CLIP_X1_FLAG;
		if (vec.vy >   w) flag |= CLIP_Y0_FLAG;
		else if (vec.vy < - w) flag |= CLIP_Y1_FLAG;
		if (vec.vz >   w) flag |= CLIP_Z0_FLAG;
		else if (vec.vz < - w) flag |= CLIP_Z1_FLAG;

		and_flag &= flag; // 全部が画面外
		or_flag |= flag;  // 一部が画面外

		// プリミティブ座標チェック
		//vec.vx *= scale->vx * range;
		//vec.vy *= scale->vy * range;
	}

#if 0
	{
		// バウンディングチェックのテスト
		DWORD color;

		if (and_flag) color = 0x80808000;
		else if (or_flag) color = 0x80800000;
		else color = 0x00808080;
		
		p->v.x = bound_min->x;
		p->v.y = bound_min->y;
		p->v.z = bound_min->z;
		p->rgba = color;
		p++;
		p->v.x = bound_max->x;
		p->v.y = bound_min->y;
		p->v.z = bound_min->z;
		p->rgba = color;
		p++;
		p->v.x = bound_max->x;
		p->v.y = bound_min->y;
		p->v.z = bound_max->z;
		p->rgba = color;
		p++;
		p->v.x = bound_min->x;
		p->v.y = bound_min->y;
		p->v.z = bound_max->z;
		p->rgba = color;
		p++;
		p->v.x = bound_min->x;
		p->v.y = bound_min->y;
		p->v.z = bound_min->z;
		p->rgba = color;
		p++;
		
		DG_SetRenderState(D3DRS_LIGHTING, FALSE);
		DG_SetTexture(0, NULL);
		DG_SetVertexShader(D3DFVF_POINTVERTEX);		
		DG_SetAlphaMode(0);
		DG_DrawPrimitiveUP(D3DPT_LINESTRIP, 4, point, sizeof(DG_POINTVERTEX));
	}
#endif
	
	// Zの中心位置を求める
	D3DXVec3Add(&vec, bound_max, bound_min);
	vec.vx *= 0.5f;
	vec.vy *= 0.5f;
	vec.vz *= 0.5f;
	vec.vw = 1.0f;

	D3DXVec3Transform(&vec, &vec, mat);
	scrpad->scale.vw = vec.vw;
	
	if (and_flag & CLIP_FLAG) return 2; // 完全に画面外
	return 0; // 画面内
}

/*----------------------------------------------------------------*/

#include "patch_index.h" // パッチ用インデックスバッファ

// t^3, t^2, t, 1.0f
static const VECTOR t_table[] = {
	{1.0           , 1.0       , 1.0   , 1.0},		// t=16/16
	{0.823974609375, 0.87890625, 0.9375, 1.0},		// t=15/16
	{0.669921875   , 0.765625  , 0.875 , 1.0},		// t=14/16
	{0.536376953125, 0.66015625, 0.8125, 1.0},		// t=13/16
	{0.421875      , 0.5625    , 0.75  , 1.0},		// t=12/16
	{0.324951171875, 0.47265625, 0.6875, 1.0},		// t=11/16
	{0.244140625   , 0.390625  , 0.625 , 1.0},		// t=10/16
	{0.177978515625, 0.31640625, 0.5625, 1.0},		// t= 9/16
	{0.125         , 0.25      , 0.5   , 1.0},		// t= 8/16
	{0.083740234375, 0.19140625, 0.4375, 1.0},		// t= 7/16
	{0.052734375   , 0.140625  , 0.375 , 1.0},		// t= 6/16
	{0.030517578125, 0.09765625, 0.3125, 1.0},		// t= 5/16
	{0.015625      , 0.0625    , 0.25  , 1.0},		// t= 4/16
	{0.006591796875, 0.03515625, 0.1875, 1.0},		// t= 3/16
	{0.001953125   , 0.015625  , 0.125 , 1.0},		// t= 2/16
	{0.000244140625, 0.00390625, 0.0625, 1.0},		// t= 1/16
	{0.0           , 0.0       , 0.0   , 1.0},		// t= 0/16
};

// 3t^2, 2t, 1.0f, 0.0f
static const VECTOR dt_table[] = {
	{3.0       , 2.0  , 1.0, 0.0},		// t=16/16
	{2.63671875, 1.875, 1.0, 0.0},		// t=15/16
	{2.296875  , 1.75 , 1.0, 0.0},		// t=14/16
	{1.98046875, 1.625, 1.0, 0.0},		// t=13/16
	{1.6875    , 1.5  , 1.0, 0.0},		// t=12/16
	{1.41796875, 1.375, 1.0, 0.0},		// t=11/16
	{1.171875  , 1.25 , 1.0, 0.0},		// t=10/16
	{0.94921875, 1.125, 1.0, 0.0},		// t= 9/16
	{0.75      , 1.0  , 1.0, 0.0},		// t= 8/16
	{0.57421875, 0.875, 1.0, 0.0},		// t= 7/16
	{0.421875  , 0.75 , 1.0, 0.0},		// t= 6/16
	{0.29296875, 0.625, 1.0, 0.0},		// t= 5/16
	{0.1875    , 0.5  , 1.0, 0.0},		// t= 4/16
	{0.10546875, 0.375, 1.0, 0.0},		// t= 3/16
	{0.046875  , 0.25 , 1.0, 0.0},		// t= 2/16
	{0.01171875, 0.125, 1.0, 0.0},		// t= 1/16
	{0.0       , 0.0  , 1.0, 0.0},		// t= 0/16
};

static const MATRIX mat_hermite = {
	2.0f,-2.0f, 1.0f, 1.0f,
	-3.0f, 3.0f,-2.0f,-1.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 0.0f,
};

// エルミート補間行列を生成する
static void MakeHermiteMatrix(MATRIX *res, VECTOR *q0, VECTOR *q1, VECTOR *d0, VECTOR *d1)
{
	MATRIX	data_mat;
	*(VECTOR *)data_mat.m[0] = *q0;
	*(VECTOR *)data_mat.m[1] = *q1;
	*(VECTOR *)data_mat.m[2] = *d0;
	*(VECTOR *)data_mat.m[3] = *d1;
	D3DXMatrixMultiply(res, &mat_hermite, &data_mat);
}

static void LerpVec(VECTOR *res, VECTOR *v1, VECTOR *v2, float t)
{
	res->vx = (v2->vx - v1->vx) * t + v1->vx;
	res->vy = (v2->vy - v1->vy) * t + v1->vy;
	res->vz = (v2->vz - v1->vz) * t + v1->vz;
}

// エルミート補間済みメッシュを作成する
static int CreateHermiteMesh(DG_PATCHVERTEX *pv, DG_PATCH *patch)
{
	static DWORD color = 0x80808080;

	int i, j, k;
	int n_rect = 0;
	int n_point = 0;
	DG_PATCH_VERT *verts, *v0, *v1, *v2, *v3;
	DG_PATCH_PARTS *parts;

	verts = patch->verts[patch->buffer_clock];
	parts = patch->parts;

	for (i = patch->n_patch; i > 0; i--, parts++) {
		int index = 0; // インデックス番号
		LPDIRECT3DINDEXBUFFER8 ibuf;
		unsigned short *pi;

		if (parts->flag & 2) {  // クリップアウト
			parts->ibuf = NULL;
			continue;
		}
		
		ibuf = DG_PatchIbuf[DG_PatchIbufID];
		DG_PatchIbufID = (DG_PatchIbufID + 1) % PATCH_IBUF_NUM;
		parts->ibuf = ibuf;
		ibuf->Lock(0, sizeof(short) * PATCH_INDEX_MAX, (unsigned char **)&pi, 0);

		parts->n_verts = 0;
		parts->n_prims = 0;

		// MGS2と同等の補間アルゴリズムを実装する
		// 1.  v0->v2(t0軸), v1->v3(t1軸)の補間を行う
		// 2.  S軸方向の補間を行う
		//
		// v0_________v1 →S軸
		//  |         |
		//  |  patch  |
		//  |         |
		//  |_________|
		// v2         v3
		//  ↓
		// T軸
		//
		static MATRIX t0_interp_mat, t1_interp_mat;
		static MATRIX s_interp_mat;
		static VECTOR vecT0, vecT1;
		static VECTOR vecDt0, vecDt1, vecDs0, vecDs1;
		static VECTOR vecUV0, vecUV1, vecUV;

		v0 = &verts[parts->v_index[0]];
		v1 = &verts[parts->v_index[1]];
		v2 = &verts[parts->v_index[2]];
		v3 = &verts[parts->v_index[3]];		
		MakeHermiteMatrix(&t0_interp_mat, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt);
		MakeHermiteMatrix(&t1_interp_mat, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt);

		// t軸方向ループ
		int	div, n_verts, n_skip;
		div = 1 << (4 - parts->lod_level);
		n_verts = div * 2 + 2;
		n_skip = 16 / div;

		const VECTOR *pt = &t_table[16];
		const VECTOR *pdt = &dt_table[16];		
		for (j = 16; j >= 0; j -= n_skip, pt -= n_skip, pdt -= n_skip) {
			// 補間座標
			D3DXVec4Transform(&vecT0, pt, &t0_interp_mat);
			D3DXVec4Transform(&vecT1, pt, &t1_interp_mat);

			// 補間傾き
			D3DXVec4Transform(&vecDt0, pdt, &t0_interp_mat);
			D3DXVec4Transform(&vecDt1, pdt, &t1_interp_mat);
			
			// uvは線形補間
			LerpVec(&vecUV0, &v0->uv, &v2->uv, pt->vz);
			LerpVec(&vecUV1, &v1->uv, &v3->uv, pt->vz);
			
			// Sの傾きは線形補間
			LerpVec(&vecDs0, &v0->pos_ds, &v2->pos_ds, pt->vz);
			LerpVec(&vecDs1, &v1->pos_ds, &v3->pos_ds, pt->vz);

			// s方向の補間行列を求める
			MakeHermiteMatrix(&s_interp_mat, &vecT0, &vecT1, &vecDs0, &vecDs1);
			
			// s軸方向ループ
			const VECTOR *ps = &t_table[16];
			const VECTOR *pds = &dt_table[16];
			for (k = 16; k >= 0; k -= n_skip, ps -= n_skip, pds -= n_skip) {
				VECTOR vecDs, vecDt, vecS;
				// LOD確認用
				static const DWORD color_table[] = {
					0x80808080,
					0x80808000,
					0x80008080,
					0x80800080,
					0x80000080,
				};
				
				D3DXVec4Transform(&vecS, ps, &s_interp_mat);
				D3DXVec4Transform(&vecDs, pds, &s_interp_mat);

				// Tの傾きは線形補間
				LerpVec(&vecDt, &vecDt0, &vecDt1, ps->vz);

				//vecDs と vecDt の外積→法線(正規化は頂点シェーダに任せる)
				D3DXVec3Cross(&vecDt, &vecDt, &vecDs);

				// 頂点バッファに格納
				pv->v.x = vecS.vx;
				pv->v.y = vecS.vy;
				pv->v.z = vecS.vz;
				pv->n.x = -vecDt.vx;
				pv->n.y = -vecDt.vy;
				pv->n.z = -vecDt.vz;
				pv->rgba = 0x80808080;//color_table[parts->lod_level];
				LerpVec(&vecUV, &vecUV0, &vecUV1, ps->vz); // uvは線形補間
				pv->tu0 = vecUV.vx;
				pv->tv0 = vecUV.vy;
				pv++;
				n_point++;
			}
		}

		// インデックスバッファへの書き込み
		// 単純なメッシュなので、あらかじめ最適な配置を計算しておく(patch_vertex.h)
		// ※今は最適でも何でもない…
		memcpy(pi, patch_index_addr[parts->lod_level],
			   patch_index_size[parts->lod_level]);
		parts->n_verts = patch_index_size[parts->lod_level] / sizeof(short);
		parts->n_prims = parts->n_verts - 2;

		//TODO:要最適化
		if (parts->edge_flag & EDGE_U) { // 上側エッジ
			int i;
			unsigned short *p = patch_index_addr[parts->lod_level];
			unsigned short *index = pi;
			int ofs = 3;
			// 奇数番目のインデックスを2つ前のインデックスに揃える
			// ex)
			//  0-1-2   0->3->1->4->2->5 を 0->3->0->4->2->5 に
			//  |/|/|
			//  3-4-5
			//
			index += ofs; // インデックスバッファ
			p += ofs;     // インデックス情報マトリクス
			for (i = 0; i < div / 2; i++, index += 4, p += 4) {
				index[0] = p[-2];
			}
		}
		if (parts->edge_flag & EDGE_D) { // 下側エッジ
			int i;
			unsigned short *p = patch_index_addr[parts->lod_level];
			unsigned short *index = pi;
			int ofs = ((div + 1) * 2 + 2) * (div - 1) + 4;
			// 奇数番目のインデックスを一つ前のインデックスに揃える
			// EDGE_U とほとんど同じ。開始アドレスだけ違う
			index += ofs;
			p += ofs;
			for (i = 0; i < div / 2; i++, index += 4, p += 4) {
				index[0] = p[-2];				
			}
		}
		if (parts->edge_flag & EDGE_L) { // 左側エッジ
			int i;
			unsigned short *p = patch_index_addr[parts->lod_level];
			unsigned short *index = pi;
			int ofs = (div + 1) * 2 + 2; // 1行飛ばすのに必要なオフセット
			// 奇数行目のインデックスを一つ前のインデックスに揃える
			// ex)
			//  0-1-2   0->3->1->4->2->5->5->3 を 0->0->1->4->2->5->5->0 に
			//  |/|/|
			//  3-4-5

			index += ofs;
			p += 0;       // p は先頭を指したまま
			for (i = 0; i < div / 2; i++, index += ofs * 2, p += ofs * 2) {
				index[0] = *p;
				index[1] = *p;				
			}
		}
		if (parts->edge_flag & EDGE_R) { // 右側エッジ
			int i;
			unsigned short *p = patch_index_addr[parts->lod_level];
			unsigned short *index = pi;
			int ofs = (div + 1) * 2 + 2; // 1行飛ばすのに必要なオフセット

			index += (div + 1) * 2;
			p += (div + 1) * 2;
			for (i = 0; i < div / 2; i++, index += ofs * 2, p += ofs * 2) {
				index[0] = *p + (div + 1);
				index[1] = *p + (div + 1);
				index[ofs - 1] = *p + (div + 1);
			}
		}
		ibuf->Unlock();
	}
	return n_point;
}

static void DrawMesh(DG_PATCH *patch)
{
	int i;
	int base_index = 0;
	DG_PATCH_PARTS *parts;

	parts = patch->parts;

	for (i = patch->n_patch; i > 0; i--, parts++) {
		if (parts->ibuf == NULL) continue;
		DG_SetIndices(parts->ibuf, base_index);
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
								0,
								parts->n_verts,
								0,
								parts->n_prims);
		base_index += patch_index_max[parts->lod_level] + 1;
	}
}

/*----------------------------------------------------------------*/

static void ChainObj(DG_PATCH *patch)
{

	LPDIRECT3DVERTEXBUFFER8 vbuf;
	ScrpadWork     *scrpad;	
	DG_PATCHVERTEX *pv;
	DG_PATCH_VERT  *verts;
	DG_PATCH_PARTS *parts;
	int i, size;

	scrpad = (ScrpadWork *)SCRPAD_ADDR;
	
	// 空き頂点バッファを検索
	do {
		vbuf = DG_PatchVbuf[DG_PatchVbufID];
#ifdef KP_XBOX
		if (vbuf->IsBusy()) {
			printf("Patch: Busy %d\n", DG_PatchVbufID);
			DG_PatchVbufID = (DG_PatchVbufID + 1) % PATCH_VBUF_NUM;
			continue;
		}
#endif
		DG_PatchVbufID = (DG_PatchVbufID + 1) % PATCH_VBUF_NUM;
		break;
	} while (1);

	// 光源計算
	SetLightMatrix(&scrpad->light[0], &scrpad->world, &patch->light[0]);
	
	D3DXMatrixMultiply(&scrpad->screen, &patch->world, &scrpad->eye_pers);
	DG_SetTransform(D3DTS_WORLD, &patch->world);
	
	if (!(patch->flag & DG_PATCH_NOCHECK)) {
		/* パッチ曲面のバウンディングチェック及びＬＯＤレベル計算 */
		parts = patch->parts;
		for (i = patch->n_patch; i > 0; i--, parts++) {
			float	f;
			int		z, l;
			parts->flag &= ~0x000f;
			
			if (BoundCheck(&scrpad->screen, &parts->max, &parts->min, &scrpad->scale) == 2) {
				parts->flag |= 2;
			} else {
				parts->flag |= 0;
			}
			/* カメラからの距離を求める（拡大率も考慮する） */
			f = -scrpad->scale.vw * scrpad->scale.vz + patch->lod_z_bias;

			f = DG_MAX(f, 0.0f);
			f = DG_MIN(f, 999999.0f);
			z = DG_FTOI(f);
			/* 上位連続ビット数を数える(PS2では'plzcw'命令) */
#if 0
			if (z != 0) {
				int n = 0;
				unsigned int tmp = 0x80000000;
				while ((tmp & z) == 0) {
					tmp = (tmp >> 1) | 0x8000000;
					n++;
				}
				l = n + 0;  // この数値を大きくすると、デフォルトの分割が細かくなる。
				l = 31 - l;
			} else {
				l = 0;
			}
#else
			{
				int n = 0;
				int tmp = z ;
				if ( !( tmp & 0x80000000 ) ) tmp = ~tmp ;
				while ( tmp & 0x40000000 ){
					tmp <<= 1 ;
					n++;
				}
				l = n + 0;  // この数値を大きくすると、デフォルトの分割が細かくなる。
				l = 31 - l;
			}
#endif

			/* ＬＯＤレベルの決定処理 */
			l -= patch->lod_level_bias;
			if (l > 4) l = 4;
			if (l < 0) l = 0;
			parts->lod_level = l;
		}

#if 0
		// lod test
		parts = patch->parts;
		for (i = patch->n_patch; i > 0; parts++, i--) {
			if ((i / 16) % 2) parts->lod_level = 3;
			else parts->lod_level = 4;
		}
#endif
		
		/* パッチ曲面のエッジ補正チェック
		 * 隣のパッチより自分のLODが小さければフラグを立てる
		 */
		parts = patch->parts;
		for (i = patch->n_patch; i > 0; parts++, i--) {
			int		index;
			parts->edge_flag = 0;
			index = parts->parts_index[0];
			if (index != -1) {
				if (parts->lod_level < patch->parts[index].lod_level) {
					//parts->edge_flag |= 0x0001;
					parts->edge_flag |= EDGE_L;
				}
			}
			index = parts->parts_index[1];
			if (index != -1) {
				if (parts->lod_level < patch->parts[index].lod_level) {
					//parts->edge_flag |= 0x0002;
					parts->edge_flag |= EDGE_R;
				}
			}
			index = parts->parts_index[2];
			if (index != -1) {
				if (parts->lod_level < patch->parts[index].lod_level) {
					//parts->edge_flag |= 0x0004;
					parts->edge_flag |= EDGE_U;
				}
			}
			index = parts->parts_index[3];
			if (index != -1) {
				if (parts->lod_level < patch->parts[index].lod_level) {
					//parts->edge_flag |= 0x0008;
					parts->edge_flag |= EDGE_D;
				}
			}
		}
	}

	size = patch->n_verts * sizeof(DG_PATCHVERTEX);
	// 頂点データの書き込み
	vbuf->Lock(0, size, (unsigned char **)&pv, 0);
	
	verts = patch->verts[0];
	int n_point = CreateHermiteMesh(pv, patch);
	vbuf->Unlock();

	// 描画関連の初期化
	DG_SetRenderState(D3DRS_LIGHTING, FALSE);
	DG_SetTexture(0, patch->tex->tex_trans.ptex);
	DG_SetAlphaMode(patch->tex->tex_trans.alpha.data);

	// パッチに有効なのは、「1平行光源 + アンビエント」
	// patch->light[1].m[0][0]～m[0][2];  // ローカルライトカラー
	// patch->light[1].m[3][0]～m[3][2];  // ローカルアンビエント
	// scrpad->light[0].m[0][0]～m[2][0]; // ローカルライト方向	
	
	// 強制黒フォグ
	if (patch->flag & DG_PATCH_FOGBLACK) {
		DG_SetRenderState(D3DRS_FOGCOLOR, 0x00000000);
	}

	VECTOR vecTmp;
	DG_SetVertexShaderConstant(CV_ZERO, &DG_ZeroVector, 1);
	vecTmp(patch->light[1].m[0][0] / 256.0f,
		   patch->light[1].m[0][1] / 256.0f,
		   patch->light[1].m[0][2] / 256.0f, 0.0f);
	DG_SetVertexShaderConstant(CV_LIGHTCOL, &vecTmp, 1);

	vecTmp(patch->light[1].m[3][0] / 256.0f,
		   patch->light[1].m[3][1] / 256.0f,
		   patch->light[1].m[3][2] / 256.0f, 0.0f);
	DG_SetVertexShaderConstant(CV_AMBIENT, &vecTmp, 1);	

	vecTmp(scrpad->light[0].m[0][0],
		   scrpad->light[0].m[1][0],
		   scrpad->light[0].m[2][0], 1.0f);
	D3DXVec3Normalize(&vecTmp, &vecTmp);
	DG_SetVertexShaderConstant(CV_LIGHTVEC, &vecTmp, 1);

	DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);	
	DG_SetVertexShaderConstantMatrix(CV_WORLDEYEPERS, &scrpad->screen, 1);
	DG_SetVertexShaderConstantMatrix(CV_WORLD, &patch->world, 1);

#ifndef BUMP_TEST	
	DG_SetVertexShader(DG_PatchVertexShaderHandle[1]); // 光源計算/フォグあり
#else
	DG_SetRenderState(D3DRS_FOGENABLE, FALSE);
	DG_SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	DG_SetVertexShader(DG_PatchVertexShaderHandle[2]);
	DG_SetPixelShader(DG_PatchPixelShaderHandle[2]);
	DG_SetTexture( 1, patch->pTexBump );
	DG_SetTexture( 3, patch->pTexLight );

	// 定数
	vecTmp(0.f, 0.5f, 2.f, 1.f);
	DG_SetVertexShaderConstant( CV_CONST4, &vecTmp, 1);

	float fTmp = 1.f;
	DG_SetTextureStageState(3, D3DTSS_BUMPENVMAT00, *(DWORD*)&fTmp);
	DG_SetTextureStageState(3, D3DTSS_BUMPENVMAT11, *(DWORD*)&fTmp);
	DG_SetTextureStageState(3, D3DTSS_BUMPENVMAT01, 0);
	DG_SetTextureStageState(3, D3DTSS_BUMPENVMAT10, 0);

	// タンジェントスペース
	DG_SetVertexShaderConstant( CV_CONST5, &patch->vecS, 1);
	DG_SetVertexShaderConstant( CV_CONST6, &patch->vecT, 1);

	// バンプ用光源方向
	DG_SetVertexShaderConstant( CV_ADDLIGHTVEC, &patch->vecBumpLight, 1);	// Diffuse
#if 0	
	vecTmp = patch->vecBumpLight + DG_Chanl(0)->eye.vecZ;
	if(vecTmp.x != 0.f || vecTmp.y != 0.f || vecTmp.z != 0.f){
		D3DXVec3Normalize(&vecTmp, &vecTmp);
	}
	vecTmp.w = 0.f;
	DG_SetVertexShaderConstant( CV_ADDLIGHTVEC + 1, &vecTmp, 1);			// Specular
#else
	DG_SetVertexShaderConstant( CV_ADDLIGHTVEC + 1, &DG_Chanl(0)->eye.vecT, 1);			// カメラ位置
#endif	
	// バンプ用光源色
	DG_SetVertexShaderConstant( CV_ADDLIGHTCOL, &patch->vecBumpCol, 1);
#endif

	DG_SetStreamSource(0, vbuf, sizeof(DG_PATCHVERTEX));
	
	// メッシュを描画
	DrawMesh(patch);

	// 強制黒フォグを元に戻す
	if (patch->flag & DG_PATCH_FOGBLACK) {
		DG_SetDxFogColor();
	}

	//制御点を描画
	//DG_SetTexture(0, NULL);
	//DG_DrawPrimitive(D3DPT_POINTLIST, 0, n_point);
#ifndef BUMP_TEST
#else
	DG_SetPixelShader(NULL);
#endif

#if 0
	// チェック用
	if (0) {
		static float z = 100.0f;
		static float prev_z = 100.0f;
		VECTOR tmp;
		if (z != 0) tmp(0.0f, 0.0f, 1.0f / z, 0.0f);
		if (prev_z != z) {
			printf("z = %f\n", z);
			prev_z = z;
		}
		if (GV_PadData[0].status & PAD_U) z -= 100.0f;
		else if (GV_PadData[0].status & PAD_D) z += 100.0f;
		DG_SetVertexShaderConstant(CV_CONST2, &tmp, 1);
	}

	if (1) {
		extern void DEBUG_Printf( char *fmt, ... );
		static float param1 = DG_FogParam.x;
		static float param2 = DG_FogParam.y;
		static float Near = 20 * 65536.0f;
		static float Far = 21 * 65536.0f;
		
		if (GV_PadData[0].status & PAD_R1) {
			Near += 500.0f;
		} else if (GV_PadData[0].status & PAD_L1) {
			Near -= 500.0f;			
		}
		if (GV_PadData[0].status & PAD_R2) {
			Far += 500.0f;
		} else if (GV_PadData[0].status & PAD_L2) {
			Far -= 500.0f;			
		}
		DG_SetFogParam(Near, Far);
		if (param1 != DG_FogParam.x || param2 != DG_FogParam.y) {
			param1 = DG_FogParam.x;
			param2 = DG_FogParam.y;
		}
		DEBUG_Printf("param    = %f, %f, %f, %f\n",
					 DG_FogParam.x, DG_FogParam.y, param1, param2);
		DEBUG_Printf("NEAR,FAR = %f, %f\n", Near, Far);
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
	}
#endif
}

/*----------------------------------------------------------------*/
/*
  プラグイン実行アクター
*/
static void PluginActor(DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status)
{
	//DG_OBJ_QUEUE	*queue;
	DG_PATCH		*patch, **que;
	//int		i, j, gid, c_gid, size, n, time, old_tex_code, invisible_flag;
	int		i, invisible_flag;
	ScrpadWork		*scrpad;
	//DG_TEXTURE_LIST	*tex_list;
	int		last_semi_trans = 0;

	scrpad = (ScrpadWork *)SCRPAD_ADDR;
	if (obj_buff->n_queue == 0) return;

	invisible_flag = DG_PATCH_INVISIBLE0 << cp->chanl_num;
	
	scrpad->buffer_switch = 0;
	scrpad->eye_pers = cp->eye_pers;
	scrpad->view = cp->eye_inv;
#if 0
	//???
	D3DXMatrixIdentity(&scrpad->eye);
	scrpad->eye.m[0][0] = -cp->eye.m[0][0];
	scrpad->eye.m[0][1] = -cp->eye.m[0][2];
	scrpad->eye.m[0][2] = cp->eye.m[0][1];
	scrpad->eye.m[1][0] = -cp->eye.m[1][0];
	scrpad->eye.m[1][1] = -cp->eye.m[1][2];
	scrpad->eye.m[1][2] = cp->eye.m[1][1];
	scrpad->eye.m[2][0] = -cp->eye.m[2][0];
	scrpad->eye.m[2][1] = -cp->eye.m[2][2];
	scrpad->eye.m[2][2] = cp->eye.m[2][1];
	D3DXMatrixMultiply(&scrpad->view, &scrpad->view, &scrpad->eye);
#endif
	D3DXMatrixMultiply(&scrpad->eye_pers, &cp->eye_inv, &cp->pers);
	DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	DG_SetTransform(D3DTS_PROJECTION, &cp->pers);
	
	scrpad->scale.vx = cp->width / 2.0f;
	scrpad->scale.vy = cp->height / 2.0f;
	// original
	//scrpad->scale.vz = 1.0f / cp->pers.m[0][0];/* ＝投影距離 (screen) */
	scrpad->scale.vz = -1.0f / cp->pers.m[0][0]; /* ＝投影距離 (screen) */
	scrpad->last_type = 0;

#ifndef BUMP_TEST
	DG_BeginScene();
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

	/* 全オブジェクトの検索 */
	que = (DG_PATCH **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; i--, que++){
		patch = *que;

		if (patch->flag & invisible_flag) continue;
		//if (patch->tri_id != tex_list->code) continue;
#if 0
		/* 強制半透明描画チェック */
		if (patch->flag & DG_PATCH_SEMITRANS) {
			if (last_semi_trans == 0){
				//size = DG_WriteRasterMaskPacks(scrpad->dma_buffer, 1);
				//DG_SendCacheFIFO(scrpad->local_work, scrpad->dma_buffer, size);
			}
			last_semi_trans = 1;
		} else {
			if (last_semi_trans != 0){
				//size = DG_WriteRasterMaskPacks(scrpad->dma_buffer, 0);
				//DG_SendCacheFIFO(scrpad->local_work, scrpad->dma_buffer, size);
			}
			last_semi_trans = 0;
		}
#endif
		ChainObj(patch);
	}
	// テクスチャモードを元に戻す
	DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DG_SetRenderState(D3DRS_FOGENABLE, FALSE);
	DG_EndScene();
#else	// BUMP_TEST
	DG_BeginScene();
	DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	DG_SetTextureStageState(3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	
//	DG_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
//	DG_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
	
	//float	start, end ;
	//start = DG_GetHSyncTimer();
	/* 全オブジェクトの検索 */
	que = (DG_PATCH **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; i--, que++){
		patch = *que;

		if (patch->flag & invisible_flag) continue;
		ChainObj(patch);
	}
	//end = DG_GetHSyncTimer();
	//printf("patch chain ... %4.2f HSync\n", end - start );
	// テクスチャモードを元に戻す
	DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DG_SetRenderState(D3DRS_FOGENABLE, FALSE);	
	DG_EndScene();
#endif	// BUMP_TEST
}


/*----------------------------------------------------------------*/
/*
  パッチ曲面オブジェクトの作成
*/
DG_PATCH*	DG_MakePatch(int flag, int n_patch, int n_verts)
{
	DG_PATCH    *patch;
	int         size;

	flag |= DG_PATCH_INVISIBLE2|DG_PATCH_INVISIBLE3;
	size = sizeof(DG_PATCH) + sizeof(DG_PATCH_PARTS) * n_patch
			+ sizeof(DG_PATCH_VERT) * n_verts * 2;
	patch = (DG_PATCH *)GV_Malloc(size);
	if (patch == NULL){
		return NULL;
	}
	GV_ZeroMemory(patch, size);

	patch->flag = flag;
	patch->n_verts = n_verts;
	patch->n_patch = n_patch;

	patch->tri_id = 0;
	patch->lod_z_bias = (float)(1 << 12);
	patch->lod_level_bias = 12;
	D3DXMatrixIdentity(&patch->world);
	patch->light = &DG_LightMatrix;
	patch->envmap_correct = DG_UnitMatrix;

	patch->verts[0] = (DG_PATCH_VERT *)((int)patch + sizeof(DG_PATCH));
	patch->verts[1] = (DG_PATCH_VERT *)((int)patch + sizeof(DG_PATCH) + sizeof(DG_PATCH_VERT) * n_verts * 1);
	patch->parts = (DG_PATCH_PARTS *)((int)patch + sizeof(DG_PATCH) + sizeof(DG_PATCH_VERT) * n_verts * 2);

	/* パケットの初期化 */
#ifdef BUMP_TEST
	patch->pTexBump = patch->pTexLight = NULL;
#endif
	
	return (patch);
}

/*
		メモリ開放
	*/
void	DG_FreePatch(DG_PATCH *patch)
{
	GV_DelayedFree(patch);
}

/*----------------------------------------------------------------*/
/*
  メッシュの頂点数で確保＆自動初期化
*/
DG_PATCH* DG_MakePatchMesh(int flag, int n_verts_s, int n_verts_t)
{
	DG_PATCH	*patch;
	DG_PATCH_PARTS	*parts;
	int			n_patch, n_verts;
	int			n_patch_s, n_patch_t;
	int			i, j;

	if (n_verts_s < 2) n_verts_s = 2;
	if (n_verts_t < 2) n_verts_t = 2;
	n_verts = n_verts_s * n_verts_t;
	n_patch = (n_verts_s - 1) * (n_verts_t - 1);
	patch = DG_MakePatch(flag, n_patch, n_verts);
	if (patch == NULL) return (NULL);

	n_patch_s = n_verts_s - 1;
	n_patch_t = n_verts_t - 1;
	for (i = 0; i < n_patch_t; i++){
		for (j = 0; j < n_patch_s; j++){
			parts = &patch->parts[ n_patch_s * i + j ];
			/* 頂点インデックス設定 */
			parts->v_index[ 0 ] = n_verts_s * (i + 0) + (j + 0);
			parts->v_index[ 1 ] = n_verts_s * (i + 0) + (j + 1);
			parts->v_index[ 2 ] = n_verts_s * (i + 1) + (j + 0);
			parts->v_index[ 3 ] = n_verts_s * (i + 1) + (j + 1);
			/* 隣接パッチインデックス設定 */
			if (j > 0){
				parts->parts_index[0] = n_patch_s * (i + 0) + (j - 1);
			} else {
				parts->parts_index[0] = -1;
			}
			if (j < (n_patch_s - 1)){
				parts->parts_index[1] = n_patch_s * (i + 0) + (j + 1);
			} else {
				parts->parts_index[1] = -1;
			}
			if (i > 0){
				parts->parts_index[2] = n_patch_s * (i - 1) + (j + 0);
			} else {
				parts->parts_index[2] = -1;
			}
			if (i < (n_patch_t - 1)){
				parts->parts_index[3] = n_patch_s * (i + 1) + (j + 0);
			} else {
				parts->parts_index[3] = -1;
			}
			/* その他設定 */
			parts->flag = 0;
		}
	}

	return (patch);

}

// バウンディング及び中心座標自動設定(とても遅い)
void DG_SetupPatchMeshBounding(DG_PATCH *patch)
{
	DG_PATCH_PARTS	*parts;
	DG_PATCH_VERT	*verts;
	VECTOR			max, min, center, g_max, g_min;
	static const VECTOR def_max = { FLT_MAX, FLT_MAX, FLT_MAX, 1.0 };
	static const VECTOR def_min = { -FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0 };
	//static const VECTOR const_data = { 0.25, 0, 0, 0 };
	int			i, j;

	g_max = def_min;
	g_min = def_max;

	parts = patch->parts;
	verts = patch->verts[patch->buffer_clock];
	for (i = patch->n_patch; i > 0; parts++, i--) {
		max = def_min;
		min = def_max;
		center = DG_ZeroVector;

		for (j = 0; j < 4; j++) {
		   	DG_PATCH_VERT *v;
			v = &verts[parts->v_index[j]];
			//printf("index[%d] = %d : %f, %f, %f, %f\n", j, parts->v_index[j],
			//v->pos.vx, v->pos.vy, v->pos.vz);
			max.vx = DG_MAX(max.vx, v->pos.vx);
			max.vy = DG_MAX(max.vy, v->pos.vy);
			max.vz = DG_MAX(max.vz, v->pos.vz);			
			min.vx = DG_MIN(min.vx, v->pos.vx);
			min.vy = DG_MIN(min.vy, v->pos.vy);
			min.vz = DG_MIN(min.vz, v->pos.vz);			
			center += v->pos;
		}

		// 中心位置を求める
		center.vx *= 0.25f;
		center.vy *= 0.25f;
		center.vz *= 0.25f;

		//printf("max   : %f, %f, %f\n", max.vx, max.vy, max.vz);
		//printf("min   : %f, %f, %f\n", min.vx, min.vy, min.vz);
		//printf("center: %f, %f, %f\n", center.vx, center.vy, center.vz);		
		max = (max - center) + max;
		min = (min - center) + min;

		g_max.vx = DG_MAX(g_max.vx, max.vx);
		g_max.vy = DG_MAX(g_max.vy, max.vy);
		g_max.vz = DG_MAX(g_max.vz, max.vz);			
		g_min.vx = DG_MIN(g_min.vx, min.vx);
		g_min.vy = DG_MIN(g_min.vy, min.vy);
		g_min.vz = DG_MIN(g_min.vz, min.vz);			

		parts->max = max;
		parts->min = min;
		//printf("max: %f, %f, %f\n", max.vx, max.vy, max.vz);
		//printf("min: %f, %f, %f\n", min.vx, min.vy, min.vz);
	}

	// 全体バウンディング
	patch->max = g_max;
	patch->min = g_min;
}

/*
		ＬＯＤパラメータ設定
	*/
void DG_ConfigPatchLOD(DG_PATCH *patch, int level)
{
	patch->lod_z_bias = (float)(1 << level);
	patch->lod_level_bias = level;
}


//--------------------------------------------------------------------------
// 以降未使用/チェック用
//--------------------------------------------------------------------------

#if 0
// 制御点を結ぶメッシュを作成する
static void CreateControlPointMesh(unsigned char *pv, DG_PATCH *patch)
{
	static const DWORD color = 0x80808080;
	int i;
	DG_PATCH_VERT *verts;
	DG_PATCH_PARTS *parts;

	verts = patch->verts[patch->buffer_clock];
	parts = patch->parts;
	
	for (i = patch->n_patch; i > 0; i--) {
		memcpy(pv, &verts[parts->v_index[0]], sizeof(VECTOR));
		memcpy(&((DG_PATCHVERTEX *)pv)->rgba, &color, sizeof(D3DCOLOR));
		pv += sizeof(DG_PATCHVERTEX);
		memcpy(pv, &verts[parts->v_index[1]], sizeof(VECTOR));
		memcpy(&((DG_PATCHVERTEX *)pv)->rgba, &color, sizeof(D3DCOLOR));		
		pv += sizeof(DG_PATCHVERTEX);		
		memcpy(pv, &verts[parts->v_index[3]], sizeof(VECTOR));
		memcpy(&((DG_PATCHVERTEX *)pv)->rgba, &color, sizeof(D3DCOLOR));		
		pv += sizeof(DG_PATCHVERTEX);		
		memcpy(pv, &verts[parts->v_index[2]], sizeof(VECTOR));
		memcpy(&((DG_PATCHVERTEX *)pv)->rgba, &color, sizeof(D3DCOLOR));
		pv += sizeof(DG_PATCHVERTEX);
		memcpy(pv, &verts[parts->v_index[0]], sizeof(VECTOR));
		memcpy(&((DG_PATCHVERTEX *)pv)->rgba, &color, sizeof(D3DCOLOR));
		pv += sizeof(DG_PATCHVERTEX);
		parts++;
	}
}

	// コントロールポイントを描画
static void DrawControlPointMesh(int n_patch)
{
	int i;
	for (i = 0; i < n_patch; i++) {
		DG_DrawPrimitive(D3DPT_LINESTRIP, 5 * i, 4);
	}
}
#endif
