/*
  shdwchin.c
  チャンネル処理ユニット／影オブジェクトＤＭＡ接続ルーチン

  1999/07/07 K.Takabe
  $Id: shdwchin.cpp,v 1.1.1.3 2002/11/19 11:42:25 Yoshizawa1 Exp $
  
*/

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <windows.h>
#include <d3dx8.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "private.h"
#include "dgconf.h"
#include "shader.h"

// デバッグ用
//#define DG_SHADOW_DEBUG

//-----------------------------------------------------------------------------

#define _CopyVector(a, b) { *(VECTOR *)(a) = *(VECTOR *)(b); }
#define _CopyMatrix(a, b) { *(MATRIX *)(a) = *(MATRIX *)(b); }

#define ALIGNSIZE1(_n)   (_n)
#define ALIGNSIZE16(_n)  (((_n) + 15) & 0xfffffff0)
#define ALIGNSIZE64(_n)  (((_n) + 63) & 0xffffffc0)
#define ALIGNSIZE128(_n) (((_n) + 127) & 0xffffff80)
#define SIZEOF_WORD(_v)	 (sizeof(_v) / sizeof(int))
#define SIZEOF_QWORD(_v) (sizeof(_v) / 16)

//-----------------------------------------------------------------------------

typedef struct {
	DG_OBJS     *objs;
	int         flag;
	int         shadow_id;
	int         bound_mode;
	int         group_id;
} OBJ_LIST;

typedef struct {
    /* オブジェクト表示用 */
    int            buffer_switch;
    int            pad0[3];
    /* ＤＭＡ生成用バッファ */
    //MATRIX_STACK   matrix_stack;    /* マトリクススタック */
    //PACKET_BUFFER  dma_packet[2];   /* ＤＭＡパケット（ダブルバッファ） */
    //u_long128      dma_buffer[128]; /* 汎用ＤＭＡパケット生成バッファ */

    /* 各種処理用 */
    MATRIX         eye_pers;
    MATRIX         eye_pers2;
    MATRIX         light_view;
    MATRIX         light_view2;
    MATRIX         light_trans;
    MATRIX         light_trans_inv;	
    MATRIX         light_pers;
    MATRIX         light_pers2;
	MATRIX         light_pers3;
	
    VECTOR         light_vector;
    VECTOR         color_vector;
    MATRIX         raise_eye_pers;  /* 優先上げマトリクス（クリップオブジェクト用） */
    MATRIX         raise_eye_pers2; /* 優先上げマトリクス（非クリップオブジェクト用） */
    MATRIX         root;
    MATRIX         tmp_mat;
    VECTOR         tmp_vec;
    VECTOR         verts[16];
    int            bound_mode;
    float          fog_param1;
    float          fog_param2;
    float          far_range;      /* 減衰の最大距離 */
    int            invisible_flag;
    int            pad[2];

    /* ChainObj()関数固有ローカル変数 */
    void           *matrix_addr;

    /* オブジェクト検索用リスト */
    int            max_list;
    OBJ_LIST       list[128];

    /* 計算結果一時保存 */
	VECTOR         lit_pos;
	VECTOR         lit_trg;
    MATRIX         screen;           /* 表示オブジェクトの透視変換マトリクス */
    MATRIX         local_light;      /* 表示オブジェクトのローカルライトマトリクス */
    MATRIX         local_color;      /* 表示オブジェクトのローカルカラーマトリクス */
    MATRIX         env_mat;          /* 表示オブジェクトのエンベロープ計算マトリクス */
    int            fog;              /* 表示オブジェクトの固定フォグ値 */
    int            local_bound_mode; /* 表示オブジェクトのバウンディングチェック結果 */
} ScrpadWork;

//-----------------------------------------------------------------------------

int DG_ActiveShadowFlag = 0;

//-----------------------------------------------------------------------------


// 平行光源からのView行列をSCRPADに設定する
static void GetLightViewport(DG_OBJS *objs, int num)
{
	const float lit_dist = 6000.0f; // 平行光源の仮想位置算出用
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR;
	VECTOR lit_pos, lit_trg;
	
	/* ライトのマトリクスを生成 */
	scrpad->tmp_vec.vx = objs->light[0].m[0][num];
	scrpad->tmp_vec.vy = objs->light[0].m[1][num];
	scrpad->tmp_vec.vz = objs->light[0].m[2][num];

	// 光の方向ベクトル
	D3DXVec3Normalize(&scrpad->tmp_vec, &scrpad->tmp_vec);
	scrpad->light_vector = scrpad->tmp_vec;
#if 0
	scrpad->color_vector.vx = 128.0f;
	scrpad->color_vector.vy = 128.0f;
	scrpad->color_vector.vz = 128.0f;
	scrpad->color_vector.vw = f * 64.0f;
#endif

	scrpad->lit_pos.vx = objs->world.m[3][0] - scrpad->tmp_vec.vx * lit_dist;
	scrpad->lit_pos.vy = objs->world.m[3][1] - scrpad->tmp_vec.vy * lit_dist;
	scrpad->lit_pos.vz = objs->world.m[3][2] - scrpad->tmp_vec.vz * lit_dist;
	scrpad->lit_trg.vx = objs->world.m[3][0];
	scrpad->lit_trg.vy = objs->world.m[3][1];
	scrpad->lit_trg.vz = objs->world.m[3][2];

	// 光源方向を適当に線で描画する
#ifdef DG_SHADOW_DEBUG
	{
		DG_COLORVERTEX v[4];
		MATRIX mat;
		
		v[0].v = (D3DXVECTOR3)scrpad->lit_pos;
		v[1].v = (D3DXVECTOR3)scrpad->lit_trg;
		v[0].rgba = 0xffffff00;
		v[1].rgba = 0xffffff00;
		v[2].v = (D3DXVECTOR3)scrpad->lit_pos;
		v[3].v = (D3DXVECTOR3)scrpad->lit_trg;
		v[2].rgba = 0xffffff00;
		v[3].rgba = 0xffffff00;

		D3DXMatrixIdentity(&mat);
		DG_BeginScene();
		DG_SetTransform(D3DTS_WORLD, &mat);
		DG_SetVertexShader(D3DFVF_COLORVERTEX);
		DG_DrawPrimitiveUP(D3DPT_LINELIST, 2, v, sizeof(DG_COLORVERTEX));
		DG_EndScene();
	}
#endif
	
	DG_MakeCameraMatrix(&scrpad->light_trans, &scrpad->lit_pos, &scrpad->lit_trg);
	/* ライトのビューポートマトリクスを生成 */
	/* 平行投影 */
	D3DXMatrixIdentity(&scrpad->light_view);
	scrpad->light_view.m[0][0] = 1.0f / 1500.0f;
	scrpad->light_view.m[1][1] = 1.0f / 1500.0f;
	scrpad->light_view.m[2][2] = -1.0f / 65536.0f;
	scrpad->light_view.m[3][2] = 1.0f;
	scrpad->light_view2 = scrpad->light_view;
	scrpad->light_view2.m[0][0] = 1.0f / 1500.0f * 128.0f;
	scrpad->light_view2.m[1][1] = 1.0f / 1500.0f * 128.0f;
	scrpad->light_view2.m[3][0] = 2048.0f;
	scrpad->light_view2.m[3][1] = 2048.0f;
	scrpad->light_view2.m[2][2] = -65536.0f / 65536.0f;
	scrpad->light_view2.m[3][2] = 65536.0f;

	D3DXMatrixInverse(&scrpad->light_trans_inv, NULL, &scrpad->light_trans);
	D3DXMatrixMultiply(&scrpad->light_pers, &scrpad->light_trans_inv, &scrpad->light_view);
	D3DXMatrixMultiply(&scrpad->light_pers2, &scrpad->light_trans_inv, &scrpad->light_view2);

	{	// 光源バウンディングチェック用?
		MATRIX mat;
		lit_pos.vx = objs->world.m[3][0];
		lit_pos.vy = objs->world.m[3][1];
		lit_pos.vz = objs->world.m[3][2];
		lit_trg.vx = objs->world.m[3][0] + scrpad->tmp_vec.vx * lit_dist;
		lit_trg.vy = objs->world.m[3][1] + scrpad->tmp_vec.vy * lit_dist;
		lit_trg.vz = objs->world.m[3][2] + scrpad->tmp_vec.vz * lit_dist;
		DG_MakeCameraMatrix(&mat, &lit_pos, &lit_trg);
		D3DXMatrixInverse(&mat, NULL, &mat);
		D3DXMatrixMultiply(&scrpad->light_pers3, &mat, &scrpad->light_view);
	}
}

// obj->screen にライト視点の行列を設定する
static void ShadowScreenObjs(DG_OBJS *objs)
{
	ScrpadWork *scrpad = (ScrpadWork*)SCRPAD_ADDR;
	DG_OBJ     *obj;
	int        i;

	obj = objs->objs;
	for (i = objs->n_models; i > 0; i--) {
		D3DXMatrixMultiply(&obj->screen, &obj->world, &scrpad->light_pers);
		obj->bound_mode = 0;
		obj ++;
	}
}

// バウンディングチェック
// screen.c と同じものを持ってくる
static u_long64 MakeBoundVerts(MATRIX *eye_pers, float *org, VECTOR *verts)
{
	float   lx, ly, lz, ux, uy, uz;
	int     i, j;
	u_long64  flag = 0;
	float   w;
	
	// bound 座標が入ってくる
	lx = org[0];
	ly = org[1];
	lz = org[2];

	ux = org[3];
	uy = org[4];
	uz = org[5];

	for (i = 8; i > 0;) {
		for (j = 4; j > 0; j --, i --) {
			verts->vx = (i & 0x01) ? lx : ux;
			verts->vy = (i & 0x02) ? ly : uy;
			verts->vz = (i & 0x04) ? lz : uz;
			verts->vw = 1.0f;

			//printf("verts0 : %f, %f %f, %f\n",
			//verts->vx, verts->vy, verts->vz, verts->vw);
			// 透視変換
			D3DXVec4Transform(verts, verts, eye_pers);
			flag <<= 6; // 最初1回のシフトは無駄

			// clipw 命令のエミュレート …
			w = DG_FABS(verts->vw);
			if (verts->vx >   w) flag |= CLIP_X0_FLAG;
			if (verts->vx < - w) flag |= CLIP_X1_FLAG;
			if (verts->vy >   w) flag |= CLIP_Y0_FLAG;
			if (verts->vy < - w) flag |= CLIP_Y1_FLAG;
			if (verts->vz >   w) flag |= CLIP_Z0_FLAG;
			if (verts->vz < - w) flag |= CLIP_Z1_FLAG;

			verts->vx /= verts->w;
			verts->vy /= verts->w;
			verts ++;
		}
	}
	return flag;
}

// TODO: バウンディングチェックの実装/高速化
static int BoundCheck(MATRIX *eye_pers, float *bound, int arg_flag)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	VECTOR     *verts;
	u_long64     flag, and_flag;
	int        i, bound_flag;
	//float      z;
	
	bound_flag = 0;
	verts = scrpad->verts; // SCRPAD上の VECTOR[16] 先頭
	flag = MakeBoundVerts(eye_pers, bound, verts);

	// フラグの累積チェック
	and_flag = flag;
	for (i = 7; i > 0; i --) {
		flag >>= 6;
		and_flag &= flag;
	}

	// XYZクリップで完全に画面外
	if (and_flag & CLIP_FLAG) return 2;
	// じゃなかったら描画する
	return 0;
}

// 影描画実作業
// light_persを使って各頂点のUV値を求めて描画を行う。
//
// 取りあえず、起動時に頂点バッファをあらかじめ確保する
//  →マルチテクスチャで描くのが正解かも？？
//    モデルのUV[2]に指定してやるだけで影描画可能…ということでうまくいくかな？

static DG_COLORVERTEX DG_LineVerts[2];

// 影用の頂点バッファ
//static LPDIRECT3DVERTEXBUFFER8 DG_ShadowVertexBuf = NULL;

// 影用の頂点バッファを初期化
void DG_InitShadowVertexBuffer(void)
{
#if 0
	DG_CreateVertexBuffer(
			256 * sizeof(DG_TEXTUREVERTEX),
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
			D3DFVF_TEXTUREVERTEX,
			D3DPOOL_DEFAULT,
			&DG_ShadowVertexBuf);
#endif
}

// 影用の頂点バッファを開放
void DG_ReleaseShadowVertexBuffer(void)
{
#if 0
	if (DG_ShadowVertexBuf != NULL) {
		DG_ShadowVertexBuf->Release();
	}
#endif
}

static DWORD DG_ShadowVertexShaderHandle;
static char *DG_ShadowVertexShaderName = {
	"shadow",
};
void DG_InitShadowVertexShader(void)
{
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
	DG_CreateVertexShader(GV_StrCode(DG_ShadowVertexShaderName),
						  dwObjDecl,
						  &DG_ShadowVertexShaderHandle);
}


// 影テクスチャを利用してモデルを描画する
static void ChainObj(DG_OBJ *obj)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	int j;
	int bound;
	SVECTOR          *verts;
	SVECTOR          *norms;
	TVECTOR_S        *uvs0;
	DG_OBJ_PACKET    *pack;
	MATRIX           matTmp;
	DG_VBUF_INFO     *info;
	DG_INDEX_INFO    *index;
	
	// オブジェクトのマトリクスを設定
	DG_SetTransform(D3DTS_WORLD, &obj->world);

	bound = scrpad->local_bound_mode;
	verts = obj->verts;
	norms = obj->norms;
	uvs0 = obj->uvs[0];
	pack = obj->packets;
	info = pack->vbuf_info;
	
	// ターゲット位置を投影する
	//D3DXVec3Transform(&vecPos, &scrpad->lit_trg, &scrpad->light_pers);
	//printf("vecPos = %f, %f, %f\n", vecPos.vx, vecPos.vy, vecPos.vz);
	//DG_SetStreamSource(0, DG_ShadowVertexBuf, sizeof(DG_TEXTUREVERTEX));
	
#if 0
	DG_TEXTUREVERTEX *pShadow, *pShadowTop;
	int              n_prims;
	int              k;
	MATRIX           matInv;  // obj->world^(-1)
	VECTOR           vecPos, vecTrg;
	VECTOR           vecLight;
	VECTOR           vecTmp;
	DG_MDLPACK       *mdlpack;
	
	// オブジェクト座標系での光源ベクトル
	D3DXMatrixInverse(&matInv, NULL, &obj->world);
	D3DXVec3Transform(&vecLight, &scrpad->light_vector, &matInv);
	
	// オブジェクト座標系での光源位置
	D3DXVec3Transform(&vecPos, &scrpad->lit_pos, &matInv);

	// オブジェクト座標系での光源目標位置
	D3DXVec3Transform(&vecTrg, &scrpad->lit_trg, &matInv);
	
	// 各頂点を光源視点で変換してUV値を求める
	D3DXMatrixMultiply(&matTmp, &obj->world, &scrpad->light_pers);

	// 影/DrawPrimitiveUP使用版
	// モデルw00a + 影モデル w00a で 1割り込みを少し越える負荷。
	index = pack->index;
	DG_SetIndices(info->ibuf, 0);
	DG_SetVertexShader(D3DFVF_TEXTUREVERTEX);	

	
	static const float afConst[][4] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
	};
	DG_SetVertexShaderConstant(CV_ZERO, &afConst[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &afConst[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &afConst[2], 1);
	DG_SetVertexShaderConstant(CV_PLIGHTPOS, DG_PointLightPos, DG_POINTLIGHT_MAX);
	DG_SetVertexShaderConstant(CV_PLIGHTCOL, DG_PointLightCol, DG_POINTLIGHT_MAX);
	DG_SetVertexShaderConstant(CV_PLIGHTPARAM, DG_PointLightParam, DG_POINTLIGHT_MAX);
	
	// 頂点バッファ領域を確保(頂点キックで増える分を見積もる必要あり。適当。)	
	pShadowTop = (DG_TEXTUREVERTEX *)GV_Malloc(1024 * 2 * sizeof(DG_TEXTUREVERTEX));
	for (j = obj->n_packs; j > 0; j--, pack++, index++) {
		pShadow = pShadowTop;
		mdlpack = pack->mdlpack;
		verts = mdlpack->verts;
		norms = mdlpack->norms;
		n_prims = 0;
		for (k = pack->n_verts; k > 0; k--, verts++, norms++) {
			pShadow->v.x = (float)verts->vx;
			pShadow->v.y = (float)verts->vy;
			pShadow->v.z = (float)verts->vz;
			// vecTmp = verts * obj->world * light_pers
			D3DXVec3Transform(&vecTmp, (VECTOR *)&pShadow->v, &matTmp);

			pShadow->tu0 = (  vecTmp.vx / vecTmp.vw + 1.0f) * 0.5f;
			// 設定されたカメラ行列では'y'がさかさま
			pShadow->tv0 = (- vecTmp.vy / vecTmp.vw + 1.0f) * 0.5f;
			pShadow->rgba = 0x30808080; // コレのalphaで影の輝度を変える(0x00で影なし)

			// 描画キック判定
			if (!(norms->pad & 0x8000)) {
				pShadow++;
				n_prims++;
			} else if (k > pack->n_verts - 1 || (norms - 1)->pad & 0x8000) {
				pShadow++;
			} else {
				// 頂点を増やしてストリップを切る
				pShadow[1] = pShadow[0];
				pShadow[0] = pShadow[-1];
				pShadow[2] = pShadow[1];
				pShadow += 3;
				n_prims += 4;
			}
		}
		// n_prims の制限数オーバーに注意
		DG_DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, n_prims,
						   pShadowTop, sizeof(DG_TEXTUREVERTEX));
	}
	GV_Free(pShadowTop);
#else
	/* 頂点シェーダバージョン */
	{
		DG_OBJ_PACKET *pkt;
		DG_VBUF_INFO *info;

		pkt = obj->packets;
		index = pkt->index;
		//info = pkt->vbuf_info;
		info = index->vbuf_info;
		DG_SetStreamSource(0, info->vbuf, info->stride);
		DG_SetIndices(info->ibuf, 0);

		static float afConst[][4] = {
			{0.0f, 0.0f, 0.0f, 0.0f},			
			{1.0f, 1.0f, 1.0f, 1.0f},
			{0.5f, 0.5f, 0.5f, 0.5f},
		};
		DG_SetVertexShaderConstant(CV_ZERO, &afConst[0], 1);		
		DG_SetVertexShaderConstant(CV_ONE, &afConst[1], 1);
		DG_SetVertexShaderConstant(CV_HALF, &afConst[2], 1);		

		// TODO:影のアルファ値
		VECTOR shadow_alpha = {0.2f, 0.2f, 0.2f, 0.2f};
		DG_SetVertexShaderConstant(CV_SHADOWALPHA, &shadow_alpha, 1);
		
		// 各頂点を光源視点で変換してUV値を求める
		D3DXMatrixMultiply(&matTmp, &obj->world, &scrpad->light_pers);
		D3DXMatrixTranspose(&matTmp, &matTmp);
		DG_SetVertexShaderConstant(CV_MATSHADOW, &matTmp, 4);
		
		D3DXMatrixTranspose(&matTmp, &obj->world);
		DG_SetVertexShaderConstant(CV_WORLD0, &matTmp, 4);
		
		D3DXMatrixTranspose(&matTmp, &scrpad->eye_pers);
		DG_SetVertexShaderConstant(CV_EYEPERS, &matTmp, 4);
		
		DG_SetVertexShader(DG_ShadowVertexShaderHandle);
		for (j = obj->n_packs; j > 0; j --, pkt++, index++) {
			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, index->min, index->num,
									index->start, index->prims);
		}
	}
#endif
}

// 影テクスチャを生成する
static void MakeShadowChainObjs(DG_OBJS *objs)
{
	ScrpadWork    *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ        *obj;
	DG_OBJ_PACKET *pkt;
	int           i, j;
	DG_VBUF_INFO  *info;
	LPDIRECT3DSURFACE8 pBackSurface;
	LPDIRECT3DSURFACE8 pBackDepthStencil;
	LPDIRECT3DSURFACE8 pShadowSurface;

	// バックバッファの情報を保存
	g_pd3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
	g_pd3dDevice->GetDepthStencilSurface(&pBackDepthStencil);

	// レンダリングターゲットの変更
	DG_ShadowTexture->GetSurfaceLevel(0, &pShadowSurface);
	g_pd3dDevice->SetRenderTarget(pShadowSurface, NULL);
	pShadowSurface->Release(); // 使い終わったらReleaseする

	// Z値更新無し
	DG_SetRenderState(D3DRS_ZENABLE, FALSE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	DG_SetRenderState(D3DRS_LIGHTING, TRUE);

	DG_BeginScene();
	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0);
	//DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x0000ff00, 0.0f, 0);

	// カメラ/透視変換マトリクスの設定
	DG_SetTransform(D3DTS_VIEW, &scrpad->light_trans_inv);
	DG_SetTransform(D3DTS_PROJECTION, &scrpad->light_view);
	
	// 強制的に光をなくして黒で描画
	//DG_DisableDxLight();
	
	DG_INDEX_INFO *index;
	obj = objs->objs;
	pkt = obj->packets;
	//info = pkt->vbuf_info;
	index = pkt->index;
	info = pkt->index->vbuf_info;
	
	DG_SetStreamSource(0, info->vbuf, info->stride);
	DG_SetVertexShader(info->fvf);
	DG_SetIndices(info->ibuf, 0);
	DG_SetTexture(0, NULL);
	for (i = objs->n_models; i > 0; i--, obj++) {	
		DG_SetTransform(D3DTS_WORLD, &obj->world);
		if (obj->parent != - 1) {
			DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
			DG_SetTransform(D3DTS_WORLD1, &obj->inv_mat);
		} else {
			DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		}
		pkt = obj->packets;
		for (j = obj->n_packs; j > 0; j --, pkt++, index++) {
			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
									index->min,
									index->num,
									index->start,
									index->prims);
		}
	}

#if 0
	if (0) { // 影を少し縮小してエッジを滑らかに…なるのかな。。
		DG_2DVERTEX v[4];
		float x, y, w, h;
		x = 1.0f;  y = 1.0f;
		w = 254.0f; h = 254.0f;
		
		v[0].v.x = x; v[0].v.y = y; v[0].v.z = 1; v[0].v.w = 1.0f;
		v[0].rgba = 0xff808080;
		v[0].tu0 = 0.0f;
		v[0].tv0 = 0.0f;

		v[1].v.x = x + w; v[1].v.y = y; v[1].v.z = 1; v[1].v.w = 1.0f;
		v[1].rgba = 0xff808080;
		v[1].tu0 = 1.0f;
		v[1].tv0 = 0;

		v[2].v.x = x; v[2].v.y = y + h; v[2].v.z = 1; v[2].v.w = 1.0f;
		v[2].rgba = 0xff808080;
		v[2].tu0 = 0.0f;
		v[2].tv0 = 1.0f;

		v[3].v.x = x + w; v[3].v.y = y + h; v[3].v.z = 1; v[3].v.w = 1.0f;
		v[3].rgba = 0xff808080;
		v[3].tu0 = 1.0f;
		v[3].tv0 = 1.0f;
		DG_SetTexture(0, DG_ShadowTexture);
		DG_SetVertexShader(D3DFVF_2DVERTEX);
		DG_DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(DG_2DVERTEX));
	}
#endif

	DG_EndScene();

	// バックバッファに戻す
	DG_SetRenderTarget(pBackSurface, pBackDepthStencil);
	pBackSurface->Release();
	pBackDepthStencil->Release();
	
	// Z値更新有りに戻す
	DG_SetRenderState(D3DRS_ZENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);	
	DG_SetTexture(0, DG_ShadowTexture);

	// 光源を再び有効にする
	//DG_EnableDxLight();
	
	// 影テクスチャの描画(デバッグ用)
#ifdef DG_SHADOW_DEBUG
	DG_BeginScene();	
	{
		DG_2DVERTEX v[4];
		float x, y, w, h;
		x = 48.0f;  y = 48.0f;
		w = 128.0f; h = 128.0f;
		
		v[0].v.x = x; v[0].v.y = y; v[0].v.z = 1; v[0].v.w = 1.0f;
		v[0].rgba = 0xff808080;
		v[0].tu0 = 0.0f;
		v[0].tv0 = 0.0f;

		v[1].v.x = x + w; v[1].v.y = y; v[1].v.z = 1; v[1].v.w = 1.0f;
		v[1].rgba = 0xff808080;
		v[1].tu0 = 1.0f;
		v[1].tv0 = 0;

		v[2].v.x = x; v[2].v.y = y + h; v[2].v.z = 1; v[2].v.w = 1.0f;
		v[2].rgba = 0xff808080;
		v[2].tu0 = 0.0f;
		v[2].tv0 = 1.0f;

		v[3].v.x = x + w; v[3].v.y = y + h; v[3].v.z = 1; v[3].v.w = 1.0f;
		v[3].rgba = 0xff808080;
		v[3].tu0 = 1.0f;
		v[3].tv0 = 1.0f;

		DG_SetVertexShader(D3DFVF_2DVERTEX);
		DG_DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(DG_2DVERTEX));
	}
	DG_EndScene();
#endif	
}


// 影テクスチャを物体に合わせて描画する
static void WriteShadowChainObjs(DG_OBJS *objs)
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR;
	DG_OBJ		*obj;
	int			i;
	int         bound;
	MATRIX      matTmp;

	obj = objs->objs;
	for (i = objs->n_models; i > 0; i --, obj ++) {
		if (obj->bound_mode & 2) continue;
		if (obj->flag & scrpad->invisible_flag) continue;
		// TODO: w00aの床は半透明にはフラグが立ってしまっているのでとりあえず外す
		//if (obj->model->type
		//& (DG_TYPE_TRANS|DG_TYPE_OVERLAY0|DG_TYPE_OVERLAY1|DG_TYPE_OVERLAY2)) continue;

		// スポットライトから見たバウンディングをチェック
		//D3DXMatrixMultiply(&scrpad->local_light, &obj->world, &scrpad->light_pers);
		//bound = BoundCheck(&scrpad->light_pers, &objs->def->lx, 0);
		D3DXMatrixMultiply(&matTmp, &obj->world, &scrpad->light_pers);
		bound = BoundCheck(&matTmp, &objs->def->lx, 0);
		if (bound == 2) continue;
		scrpad->local_bound_mode = obj->bound_mode | bound;

		/* オブジェクト描画パラメータ構造体にデータをセット */
		//scrpad->objenv.screen = obj->screen;
#if 0
		{ // TODO: フォグ計算
			float f;
			int   ifog;
			f = scrpad->fog_param1 * obj->sort_z + scrpad->fog_param2;
			f = DG_MAX(f, 0.0f);
			f = DG_MIN(f, 255.0f);
			ifog = DG_FTOI(f);
			scrpad->fog = (ifog << 4) | 0x8000;
		}
		*(FVECTOR *)scrpad->local_color.m[0] = scrpad->color_vector;
		scrpad->local_color.m[1][3] = scrpad->far_range;
		{ // 光源ベクトルをモデル座標相対に変換
			MATRIX tmp_mat;
			D3DXMatrixInverse(&tmp_mat, NULL, &obj->world);
			scrpad->light_vector.vw = 0.0f;
			D3DXVec4Transform((VECTOR *)scrpad->local_color.m[2],
							  &scrpad->light_vector, &tmp_mat);
		}
#endif
		ChainObj(obj); // 影描画
	}
}


// スポットライトからのView行列をSCRPADに設定する
static void GetSpotViewport(DG_SPOT *spot)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	float angle, fNear;
	
	fNear = 100.0f;
	angle = 2.0f;
	angle = spot->angle;
	/* ライトのビューポートマトリクスを生成 */
	scrpad->color_vector.vx = 128.0f;
	scrpad->color_vector.vy = 128.0f;
	scrpad->color_vector.vz = 128.0f;
	scrpad->color_vector.vw = 128.0f;
	scrpad->light_vector = *(VECTOR *)spot->world.m[2];
	scrpad->far_range = spot->range * 2 - fNear;
	/* パース付き投影 */
	DG_ViewScreenMatrix(scrpad->light_view,
						angle,
						1.0f, 1.0f, 0.0f, 0.0f,
						-1.0f, 1.0f, fNear, spot->range * 2);
	DG_ViewScreenMatrix(scrpad->light_view2,
						angle * 128.0f,
						1.0f, 1.0f, 2048.0f, 2048.0f,
						(float)DRAW_Z_MIN, (float)DRAW_Z_MAX, fNear, spot->range * 2);
	D3DXMatrixInverse(&scrpad->tmp_mat, NULL, &spot->world);
	D3DXMatrixMultiply(&scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat);
	D3DXMatrixMultiply(&scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat);
}


//-----------------------------------------------------------------------------

// スポットライト処理チャンネル
void DG_SpotChainChanl(DG_CHANL *cp, int which)
{
}

//-----------------------------------------------------------------------------

// 平行投影マトリクスを生成する
static void OrthoScreenMatrix(MATRIX *res, float width, float height, float scale,
							  float zmin, float zmax, float nearz, float farz)
{
	float z_width, z_center;

	//_sceVu0UnitMatrix(res);
	D3DXMatrixIdentity(res);
	/* X */
	res->m[0][0] = 2.0f * scale / width;
	/* Y */
	res->m[1][1] = 2.0f * scale / height;
	/* Z */
	z_width = (farz - nearz) / (zmax - zmin);
	z_center = (- farz * zmin + nearz * zmax) / (zmax - zmin);
	res->m[2][2] = - 1.0f / z_width;
	res->m[3][2] = z_center / z_width;
	/* W */
	res->m[3][3] = 1.0f;
}

static void GetParallelViewport(DG_SPOT *spot)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	float      angle, fNear;

	fNear = 100.0f;
	angle = spot->angle;
	/* ライトのビューポートマトリクスを生成 */
	scrpad->color_vector.vx = 128.0f;
	scrpad->color_vector.vy = 128.0f;
	scrpad->color_vector.vz = 128.0f;
	scrpad->color_vector.vw = (float)(spot->color >> 24);
	scrpad->light_vector = *(VECTOR *)spot->world.m[2];
	scrpad->far_range = spot->range * 2 - fNear;

	if (angle == 0.0f) {
		/* 平行投影 */
		OrthoScreenMatrix(&scrpad->light_view, 3000.0f, 3000.0f, 1.0f,
						  -1.0f, 1.0f, fNear, spot->range * 2);
		OrthoScreenMatrix(&scrpad->light_view2, 3000.0f, 3000.0f, 128.0f,
						  (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, fNear, spot->range * 2);
		scrpad->light_view2.m[3][0] = 2048.0f;
		scrpad->light_view2.m[3][1] = 2048.0f;
	} else {
		/* パース付き投影 */
		DG_ViewScreenMatrix(scrpad->light_view,
							angle,
							1.0f, 1.0f, 0.0f, 0.0f,
							- 1.0f, 1.0f, fNear, spot->range * 2);
		DG_ViewScreenMatrix(scrpad->light_view2,
							angle * 128.0f,
							1.0f, 1.0f, 2048.0f, 2048.0f,
							(float)DRAW_Z_MIN, (float)DRAW_Z_MAX, fNear, spot->range * 2);
	}
	/* ライト用の透視変換マトリクスの作成 */
	D3DXMatrixInverse(&scrpad->tmp_mat, NULL, &spot->world);
	D3DXMatrixMultiply(&scrpad->light_pers, &scrpad->tmp_mat, &scrpad->light_view);
	D3DXMatrixMultiply(&scrpad->light_pers2, &scrpad->tmp_mat, &scrpad->light_view2);
}

// 影投影処理ステージ
void DG_ShadowChainChanl(DG_CHANL *cp, int which)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJS       **oque, **oque2, *objs, *objs2;
	//DG_SPOT       **spot_queue, *spot;
	DG_OBJ_QUEUE  *que;
	DG_OBJ_BUFFER *obj_buff;
	int i, j, c_gid;
	//int      i, j, k, size, shadow_id, enable, c_gid;
	//int gid;
	//OBJ_LIST *list;

	if (DG_ActiveShadowFlag != 1) return;

	que = cp->obj_queue;
	if (que == NULL) return;
	obj_buff = &que->objs_buffer;

	c_gid = cp->group_id;

	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num;
	scrpad->buffer_switch = 0;
	scrpad->eye_pers = cp->eye_pers;
//	scrpad->eye_pers2 = cp->eye_pers2;
	scrpad->raise_eye_pers = cp->raise_eye_pers;
//	scrpad->raise_eye_pers2 = cp->raise_eye_pers2;
	scrpad->fog_param1 = 0.0f;
	scrpad->fog_param2 = 255.0f;

	// TODO: とりあえず旧バージョン(shdwchin.c,v 1.1 1999/07/09) をベースに作る
	oque = (DG_OBJS **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; i --, oque ++) {
		objs = *oque;
		if (objs->flag & (DG_FLAG_INVISIBLE | DG_FLAG_TEST) ) continue;
		if (!(objs->flag & DG_FLAG_SHADOWMAKE)) continue;

		GetLightViewport(*oque, 0);  // 影投影マトリクス生成
		MakeShadowChainObjs(*oque);  // 影テクスチャを生成

		// カメラマトリクスを元に戻す(MakeShadowChain...で変更されるため)
		DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
		// 優先上げ透視変換マトリクスを設定する
		DG_SetTransform(D3DTS_PROJECTION, &cp->raise_pers);

		// 影テクスチャを設定
		DG_SetTexture(0, DG_ShadowTexture);
		// テクスチャモードを設定
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		DG_SetRenderState(D3DRS_LIGHTING, FALSE);		
		DG_SetSemitransShadow();
		//DG_SetSemitransAdd();
		DG_BeginScene();
		// 影の描画
		oque2 = (DG_OBJS **)obj_buff->queue;
		for (j = obj_buff->n_queue; j > 0; j --, oque2 ++) {
			objs2 = *oque2;
			if (objs2->flag & (DG_FLAG_INVISIBLE | DG_FLAG_TEST) ) continue;
			if (!(objs2->flag & DG_FLAG_SHADOWWRITE)) continue;

			scrpad->root = objs2->world;
			if (objs2->flag & DG_FLAG_ONEPIECE) {
				// 原点オフセット付加
				scrpad->root.m[3][0] += objs2->trans.vx;
				scrpad->root.m[3][1] += objs2->trans.vy;
				scrpad->root.m[3][2] += objs2->trans.vz;
			}
			// 光源視点による全体バウンディングチェック
			D3DXMatrixMultiply(&scrpad->tmp_mat, &scrpad->root, &scrpad->light_pers);
			scrpad->bound_mode = BoundCheck(&scrpad->tmp_mat, &objs2->def->lx, 0);
			if (scrpad->bound_mode == 2) continue;
			WriteShadowChainObjs(objs2); // 影オブジェクトの描画
		}
		DG_EndScene();
		DG_SetRenderState(D3DRS_LIGHTING, FALSE);
	}
}
