/*
	light.c
	光源データ管理ルーチン

	1999/07/07 K.Takabe
	$Id: light.cpp,v 1.1.1.3 2002/11/19 11:42:11 Yoshizawa1 Exp $

*/

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <windows.h>
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "private.h"

//-----------------------------------------------------------------------------

// NOTE: ライトの方向について
//  DirectX に計算を任せているので、
//  MGS2のカメラ行列/透視変換行列をそのまま使用する場合、
//  Lightの方向に注意。

//-----------------------------------------------------------------------------

// 絶対値マクロ(PS2互換用)
#define FABS(_x) ((_x) > 0.0f ? (_x) : - (_x))

// 色設定の基準値
//#define RGB_ONE   (128.0f)
#define RGB_ONE   (255.0f)


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

//-----------------------------------------------------------------------------

// ローカルライトマトリクス（デフォルト）
MATRIX DG_LightMatrix = {
	0.347f, 0.0f, 0.0f, 0.0f,  /* Ｘ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
	-0.625f, 0.0f, 0.0f, 0.0f, /* Ｙ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
	0.694f, 0.0f, 0.0f, 0.0f,  /* Ｚ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
	1.0f, 0.0f, 0.0f, 0.0f     /* 光源強度 */
};

// ローカルカラーマトリクス（デフォルト）
MATRIX DG_ColorMatrix = {
////	64.0f/256.0f, 64.0f/256.0f, 96.0f/256.0f, 0.0f,  /* 平行光源カラー */
	256.0f/256.0f, 256.0f/256.0f, 256.0f/256.0f, 0.0f,  /* 平行光源カラー */
	0.0f, 0.0f, 0.0f, 0.0f,     /* 点光源１カラー */
	0.0f, 0.0f, 0.0f, 0.0f,	    /* 点光源２カラー */
////	24.0f/256.0f, 32.0f/256.0f, 46.0f/256.0f, 0.0f,  /* 環境光カラー */
	256.0f/256.0f, 256.0f/256.0f, 256.0f/256.0f, 0.0f,  /* 環境光カラー */
};

// Xbox追加/リアルタイム点光源

// 点光源位置
VECTOR DG_PointLightPos[DG_POINTLIGHT_MAX] = {
	{0.0f, 0.0f, 0.0f, 0.0f},
};
// 点光源色(w要素は光源到達距離)
VECTOR DG_PointLightCol[DG_POINTLIGHT_MAX] = {
	{0.8f, 0.8f, 0.8f, 1000.0f},
};
// 点光源の減衰パラメータ
// 光源→頂点間の距離を d とすると、
// 減衰パラメータ A = 1 / (param.x + d * param.y + d * d * param.z)
// また、w要素はピクセルシェーダで変化する光源到達距離
VECTOR DG_PointLightParam[DG_POINTLIGHT_MAX] = {
	{0.0f, 1.0f, 0.0f, 2000.0f},
};

int DG_PointLightFlag = FALSE;

void DG_PointLightEnable(void)
{
	DG_PointLightFlag = TRUE;
}
void DG_PointLightDisable(void)
{
	DG_PointLightFlag = FALSE;
}
void DG_SetPointLightStatus(int flag)
{
	DG_PointLightFlag = flag;
}

void DG_SetPointLightCol(int n, VECTOR *v)
{
	ASSERT(n < DG_POINTLIGHT_MAX);
	DG_PointLightCol[n] = *v;
}
void DG_SetPointLightPos(int n, VECTOR *v)
{
	ASSERT(n < DG_POINTLIGHT_MAX);
	DG_PointLightCol[n] = *v;
}
void DG_ClearPointLight(int n)
{
	ASSERT(n < DG_POINTLIGHT_MAX);
	DG_PointLightCol[n] = DG_ZeroVector;
}

// Xbox追加
//
// リアルタイム光源
// PSHADEで作成したオブジェクトにも有効。
//
int DG_AdditionalLightFlag = FALSE;

MATRIX DG_AdditionalLightMatrix = {
	0.0f, 0.0f, 0.0f, 0.0f,  /* Ｘ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
	0.0f, 0.0f, 0.0f, 0.0f,  /* Ｙ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
	0.0f, 0.0f, 0.0f, 0.0f,  /* Ｚ成分×４（平行光源、点光源１、点光源２、スペキュラー） */
	0.0f, 0.0f, 0.0f, 0.0f,  /* 光源強度 */
};

MATRIX DG_AdditionalLightPosMatrix = {
	0.0f, 0.0f, 0.0f, 0.0f,  /* 光源位置1 */
	0.0f, 0.0f, 0.0f, 0.0f,  /* 光源位置2 */
	0.0f, 0.0f, 0.0f, 0.0f,  /* 光源位置3 */
	0.0f, 0.0f, 0.0f, 0.0f,  /* 未使用 */
};

MATRIX DG_AdditionalColorMatrix = {
	64.0f/256.0f, 64.0f/256.0f, 96.0f/256.0f, 0.0f,  /* 平行光源カラー */
	0.0f, 0.0f, 0.0f, 0.0f,     /* 点光源１カラー */
	0.0f, 0.0f, 0.0f, 0.0f,	    /* 点光源２カラー */
	0.0f, 0.0f, 0.0f, 0.0f,     /* 環境光カラー */
};

// 追加光源方向 + Power
void DG_SetAdditionalLightVec(int n, VECTOR *v)
{
	ASSERT(n < 3);
	DG_AdditionalLightMatrix.m[0][n] = v->x;
	DG_AdditionalLightMatrix.m[1][n] = v->y;
	DG_AdditionalLightMatrix.m[2][n] = v->z;
	DG_AdditionalLightMatrix.m[3][n] = v->w;	
}

// 追加光源強度の設定
void DG_SetAdditionalLightPow(int n, float power)
{
	ASSERT(n < 3);
	DG_AdditionalLightMatrix.m[3][n] = power;
}

// 追加光源色
void DG_SetAdditionalLightCol(int n, VECTOR *v)
{
	ASSERT(n < 3);
	*(VECTOR *)DG_AdditionalColorMatrix.m[n] = *v;
}

// 追加光源方向
void DG_SetAdditionalAmbient(VECTOR *v)
{
	*(VECTOR *)DG_AdditionalColorMatrix.m[3] = *v;
}

void DG_SetAdditionalLightStatus(int flag)
{
	DG_AdditionalLightFlag = flag;
}

void DG_AdditionalLightEnable(void)
{
	DG_AdditionalLightFlag = TRUE;
}

void DG_AdditionalLightDisable(void)
{
	DG_AdditionalLightFlag = FALSE;
}


// XBOX
// ライト
D3DLIGHT8 D3DLight[4];

//-----------------------------------------------------------------------------

// 固定光源バッファ（通常はロードデータを参照）
typedef struct {
	int     map_id;
	int	    flag;
	int     pad;
	LIT_DEF *FixedLights;
} FIXLIGHT;

#define MAX_FIX_LIGHTS 16
static FIXLIGHT fix_lights[MAX_FIX_LIGHTS]; // 固定光源バッファ実体

/* 固定光源バッファ中の管理フラグ */
enum {
	FIXLIGHT_CHANGE = 0x00000001,   /* 光源状態変化 */
};

// 一時光源バッファ
DG_TLIGHT	DG_TLights[2];
int			DG_LightClock;
static VECTOR max_vec = { 99999999.0f,  99999999.0f,  99999999.0f, 0.0f};
static VECTOR min_vec = {-99999999.0f, -99999999.0f, -99999999.0f, 0.0f};


//-----------------------------------------------------------------------------

static inline void BoundResize(VECTOR *max, VECTOR *min, VECTOR *vec2)
{
	max->vx = DG_MAX(max->vx, vec2->vx);
	max->vy = DG_MAX(max->vy, vec2->vy);
	max->vz = DG_MAX(max->vz, vec2->vz);

	min->vx = DG_MIN(min->vx, vec2->vx);
	min->vy = DG_MIN(min->vy, vec2->vy);
	min->vz = DG_MIN(min->vz, vec2->vz);
}

//-----------------------------------------------------------------------------

// 光源システムを初期化する
void DG_InitLightSystem(void)
{
	int i, j;

    DG_LightClock = 0;
    DG_ResetFixedLight();
    for (i = 0; i < 2; i++) {
        DG_TLights[i].TmpLightDef.n_lit_group = 3;
        for (j = 0; j < 3; j++){
            DG_TLights[i].TmpLightGrp[j].n_lights = 0;
            DG_TLights[i].TmpLightGrp[j].bound_max = min_vec;
            DG_TLights[i].TmpLightGrp[j].bound_min = max_vec;
        }
    }
    for (i = 0; i < 2; i++){
        DG_TLights[i].TmpLightGrp[0].lit = DG_TLights[0].points;
        DG_TLights[i].TmpLightGrp[0].type = LIT_TYPE_POINT;
        DG_TLights[i].TmpLightGrp[1].lit = DG_TLights[0].spots;
        DG_TLights[i].TmpLightGrp[1].type = LIT_TYPE_SPOT;
        DG_TLights[i].TmpLightGrp[2].lit = DG_TLights[0].blacks;
        DG_TLights[i].TmpLightGrp[2].type = LIT_TYPE_BLACKPOINT;
    }

	// XBOX
	// DirectXのライトを初期化
	GV_ZeroMemory(D3DLight, sizeof(D3DLight));
	for (i = 0; i < 4; i ++) {
		D3DLight[i].Type = D3DLIGHT_DIRECTIONAL; // _POINT, _SPOT
		D3DLight[i].Direction.x = - DG_LightMatrix.m[0][0];
		D3DLight[i].Direction.y = - DG_LightMatrix.m[1][0];
		D3DLight[i].Direction.z = DG_LightMatrix.m[2][0];
		// ライトパラメータを設定する
		if (FAILED(g_pd3dDevice->SetLight(i, &D3DLight[i]))) {
			ASSERT(0);
		}
		// ライトを有効にする
		if (FAILED(g_pd3dDevice->LightEnable(i, TRUE))) {
			ASSERT(0);
		}
	}

	// Diffuse, Ambient 光を反射するように設定する。
	D3DMATERIAL8 m;
	GV_ZeroMemory(&m, sizeof(D3DMATERIAL8));
	m.Diffuse.r = 1.0f;
	m.Diffuse.g = 1.0f;
	m.Diffuse.b = 1.0f;
	m.Diffuse.a = 1.0f;

	m.Ambient.r = 1.0f;
	m.Ambient.g = 1.0f;
	m.Ambient.b = 1.0f;
	m.Ambient.a = 1.0f;
	if (FAILED(g_pd3dDevice->SetMaterial(&m))) {
		ASSERT(0);
	}

	DG_SetAmbient(0, 0, 0);
}

// ライトを全て無効にする
void DG_DisableDxLight(void)
{
	if (FAILED(g_pd3dDevice->LightEnable(0, FALSE))) ASSERT(0);
	if (FAILED(g_pd3dDevice->LightEnable(1, FALSE))) ASSERT(0);
	if (FAILED(g_pd3dDevice->LightEnable(2, FALSE))) ASSERT(0);
	if (FAILED(g_pd3dDevice->LightEnable(3, FALSE))) ASSERT(0);	
}

// ライトを全て有効にする
void DG_EnableDxLight(void)
{
	if (FAILED(g_pd3dDevice->LightEnable(0, TRUE))) ASSERT(0);
	if (FAILED(g_pd3dDevice->LightEnable(1, TRUE))) ASSERT(0);
	if (FAILED(g_pd3dDevice->LightEnable(2, TRUE))) ASSERT(0);
	if (FAILED(g_pd3dDevice->LightEnable(3, TRUE))) ASSERT(0);	
}


// アンビエント光を設定する
// ※0番ライトでAmbient光を一緒に設定していることに注意。
//   うまくいかなかったら SetRenderState();
void DG_SetAmbient(int r, int g, int b)
{
	//printf("DG_SetAmbient %d %d %d\n", r, g, b);
    ASSERT(r < 256);
    ASSERT(g < 256);
    ASSERT(b < 256);
#if 0
    DG_ColorMatrix.m[3][0] = (float)r / RGB_ONE;
    DG_ColorMatrix.m[3][1] = (float)g / RGB_ONE;
    DG_ColorMatrix.m[3][2] = (float)b / RGB_ONE;
#else
    DG_ColorMatrix.m[3][0] = (float)r ;
    DG_ColorMatrix.m[3][1] = (float)g ;
    DG_ColorMatrix.m[3][2] = (float)b ;
#endif

	// 0番ライトでAmbient色を一緒に設定する。
	D3DLIGHT8 *light = &D3DLight[0];
	light->Ambient.r = (float)r / RGB_ONE;
	light->Ambient.g = (float)g / RGB_ONE;
	light->Ambient.b = (float)b / RGB_ONE;
	light->Ambient.a = 0.0f;
}

// 主光源の方向を設定する
void DG_SetMainLightDir(int x, int y, int z)
{
    VECTOR vec;
	
	vec.vx = (float)x;
    vec.vy = (float)y;
    vec.vz = (float)z;
	//printf("DG_SetMainLightDir (%f, %f, %f)\n", vec.x, vec.y, vec.z);
	
    //_sceVu0Normalize(&vec, &vec);
	D3DXVec3Normalize(&vec, &vec);
    DG_LightMatrix.m[0][0] = vec.vx;
    DG_LightMatrix.m[1][0] = vec.vy;
    DG_LightMatrix.m[2][0] = vec.vz;
    DG_LightMatrix.m[3][0] = 1.0f; // ????

	// XBOX
	D3DLIGHT8 *light = &D3DLight[0];
	light->Direction.x = - vec.vx;
	light->Direction.y = - vec.vy;
	light->Direction.z = vec.vz;	
	if (FAILED(g_pd3dDevice->SetLight(0, light))) {
		ASSERT(0);
	}
}

// 主光源の色を設定する
void DG_SetMainLightCol(int r, int g, int b)
{
	//printf("DG_SetMainLightCol (%d, %d, %d)\n", r, g, b);
#if 0
    DG_ColorMatrix.m[0][0] = (float)r / RGB_ONE;
    DG_ColorMatrix.m[0][1] = (float)g / RGB_ONE;
    DG_ColorMatrix.m[0][2] = (float)b / RGB_ONE;
#else
    DG_ColorMatrix.m[0][0] = (float)r ;
    DG_ColorMatrix.m[0][1] = (float)g ;
    DG_ColorMatrix.m[0][2] = (float)b ;
#endif

	// XBOX
	D3DLIGHT8 *light = &D3DLight[0];
	// Diffuse が1.0になるのは色が 128.0 の時(PSX2側の仕様)
	light->Diffuse.r = (float)r / RGB_ONE;
	light->Diffuse.g = (float)g / RGB_ONE;
	light->Diffuse.b = (float)b / RGB_ONE;
	light->Diffuse.a = 0.0f;

	if (FAILED(g_pd3dDevice->SetLight(0, light))) {
		ASSERT(0);
	}
}

// light matrix, color matrix から DirectX のメインライト用 パラメータを生成する
void DG_SetDxMainLight(MATRIX *light, MATRIX *color)
{
	const float rate = 1.0f / RGB_ONE; // 色成分の倍率
	
	D3DLight[0].Direction.x = - light->m[0][0];
	D3DLight[0].Direction.y = light->m[1][0];
	D3DLight[0].Direction.z = light->m[2][0];

	D3DLight[0].Diffuse.r = color->m[0][0] * rate ;
	D3DLight[0].Diffuse.g = color->m[0][1] * rate ;
	D3DLight[0].Diffuse.b = color->m[0][2] * rate ;

	if (FAILED(g_pd3dDevice->SetLight(0, &D3DLight[0]))) {
		ASSERT(0);
	}

	// アンビエント
	D3DLight[0].Ambient.r = color->m[3][0] * rate ;
	D3DLight[0].Ambient.g = color->m[3][1] * rate ;
	D3DLight[0].Ambient.b = color->m[3][2] * rate ;
	D3DLight[0].Ambient.a = 0.0f;
	//g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(r,g,b));
}

void DG_SetDxLight(MATRIX *light, MATRIX *color)
{
	int i;
	float len;
	
	// 3方向の平行光源
	for (i = 2; i >= 0; i --) {
		D3DLight[i].Direction.x = - light->m[0][i];
		D3DLight[i].Direction.y = - light->m[1][i];
		D3DLight[i].Direction.z = light->m[2][i];

		len = D3DXVec3Length((D3DXVECTOR3 *)&D3DLight[i].Direction);
		if (len < 0.0001f) {
			// 方向の大きさが0だとエラーになるので、
			// 強度0の光源を置く。方向は適当。
			D3DLight[i].Direction.x = 1.0f;
			D3DLight[i].Diffuse.r
					= D3DLight[i].Diffuse.g
					= D3DLight[i].Diffuse.b
					= 0.0f;
		} else {
			float tmp = light->m[3][i]; // 標準の強度は1.0f
			D3DLight[i].Diffuse.r = color->m[i][0] * tmp;
			D3DLight[i].Diffuse.g = color->m[i][1] * tmp;
			D3DLight[i].Diffuse.b = color->m[i][2] * tmp;
		}
		if (FAILED(g_pd3dDevice->SetLight(i, &D3DLight[i]))) {
			ASSERT(0);
		}
	}

	// アンビエント
	D3DLight[0].Ambient.r = color->m[3][0];
	D3DLight[0].Ambient.g = color->m[3][1];
	D3DLight[0].Ambient.b = color->m[3][2];
	D3DLight[0].Ambient.a = 0.0f;
	//g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(r,g,b));
}


//-----------------------------------------------------------------------------

// 固定光源データを初期化する
void DG_ResetFixedLight(void)
{
	int i;
	FIXLIGHT *p;

	p = fix_lights;

	for (i = MAX_FIX_LIGHTS; i > 0; i --, p ++) {
		p->map_id = 0;
		p->flag = 0;
		p->FixedLights = NULL;
	}
}

// 光源データをセットする(map.cから呼ばれる)
void DG_SetFixedLightMap(LIT_DEF *light, int map_id)
{
	void DG_LightFlagClear(LIT_DEF *lit_def);
	int i;
	FIXLIGHT *p;
	/* 空き探索:同じものが設定されていたらスキップ */
	/* MAX_FIX_LIGHTS - 1 以上は検索されない */
	p = fix_lights;

	for (i = MAX_FIX_LIGHTS - 1; i > 0; i --, p ++) {
		if (p->FixedLights == light) {
			p->map_id |= map_id;
			return;
		}
		if (p->FixedLights == NULL) {
			p->map_id = map_id;
			p->flag = 0;
			p->FixedLights = light;
			DG_LightFlagClear(light);
#if 1
			DG_ColorMatrix.m[3][0] = (float)light->ambient.r / RGB_ONE;
			DG_ColorMatrix.m[3][1] = (float)light->ambient.g / RGB_ONE;
			DG_ColorMatrix.m[3][2] = (float)light->ambient.b / RGB_ONE;
			DG_LightMatrix.m[0][0] = light->dir.vx;
			DG_LightMatrix.m[1][0] = light->dir.vy;
			DG_LightMatrix.m[2][0] = light->dir.vz;
			DG_LightMatrix.m[3][0] = light->dir.vw;
			DG_ColorMatrix.m[0][0] = (float)light->color.r / RGB_ONE;
			DG_ColorMatrix.m[0][1] = (float)light->color.g / RGB_ONE;
			DG_ColorMatrix.m[0][2] = (float)light->color.b / RGB_ONE;
			DG_SetDxMainLight(&DG_LightMatrix, &DG_ColorMatrix);
#else
			DG_SetAmbient((int)light->ambient.r,
						  (int)light->ambient.g,
						  (int)light->ambient.b);						  
			DG_LightMatrix.m[0][0] = light->dir.vx;
			DG_LightMatrix.m[1][0] = light->dir.vy;
			DG_LightMatrix.m[2][0] = light->dir.vz;
			DG_LightMatrix.m[3][0] = light->dir.vw;
			DG_SetMainLightCol(light->color.r, light->color.g, light->color.b);
#endif
			return;
		}
	}
}

#if 0
// 固定光源データを登録する
void DG_SetFixedLight(LIT_DEF *light)
{
	//DG_SetFixedLightMap(light, 0x7fffffff);
	DG_SetFixedLightMap(light, 0x00000000);
}
#endif

	
// 現在のマップＩＤに対応する固定光源データを取得する
// map_id は GM_GetDGGroupID(map)などとして求めること
LIT_DEF* DG_GetFixedLight(int map_id)
{
	int i;
	FIXLIGHT *p;

	p = fix_lights;
	for (i = MAX_FIX_LIGHTS - 1; i > 0; i --, p ++) {
		if (p->map_id & map_id) {
			return p->FixedLights;
		}
	}
	return NULL;
}


// ライトデータの無効フラグをクリアする
void DG_LightFlagClear(LIT_DEF *lit_def)
{
	LIT_GRP *lit_grp;
	int     i, j;

	if (lit_def == NULL) return;

	//lit_grp = lit_def->grp;
	// VC++ で lit_def の定義がうまくいかなかった…
	lit_grp = (LIT_GRP *)(lit_def + 1);
	for (i = lit_def->n_lit_group; i > 0; i --, lit_grp ++) {
		/* きちんとダイナミックフラグを見るように改良したほうがいいかも・・・ */
		if (lit_grp->type & LIT_TYPE_POINT) {
			/* 点光源について処理 */
			LIT_POINT *light;
			light = (LIT_POINT *)lit_grp->lit;
			for (j = lit_grp->n_lights; j > 0; j --, light ++) {
				light->flag &= ~LIT_FLAG_DISABLE;
			}
		} else if (lit_grp->type & LIT_TYPE_POINT) {
			/* スポット光源について処理 */
			LIT_SPOT *light;
			light = (LIT_SPOT *)lit_grp->lit;
			for (j = lit_grp->n_lights; j > 0; j --, light ++) {
				light->flag &= ~LIT_FLAG_DISABLE;
			}
		} else if (lit_grp->type & LIT_TYPE_LINE) {
			/* 線光源について処理 */
			LIT_LINE *light;
			light = (LIT_LINE *)lit_grp->lit;
			for (j = lit_grp->n_lights; j > 0; j--, light++) {
				light->flag &= ~LIT_FLAG_DISABLE;
			}
		} else if (lit_grp->type & LIT_TYPE_BLACKPOINT) {
			/* 黒点光源について処理 */
			LIT_BLACKPOINT *light;
			light = (LIT_BLACKPOINT *)lit_grp->lit;
			for (j = lit_grp->n_lights; j > 0; j --, light ++) {
				light->flag &= ~LIT_FLAG_DISABLE;
			}
		}
	}
}

//-----------------------------------------------------------------------------

// 一時光源バッファをクリアする(フレーム毎に実行)
void DG_ClearTmpLight(void)
{
	DG_TLIGHT *tlp;

	if (GV_PauseLevel != 0) return;
	DG_LightClock = 1 - DG_LightClock;
	tlp = DG_TLights + DG_LightClock;
	tlp->TmpLightGrp[0].n_lights = 0;
	tlp->TmpLightGrp[1].n_lights = 0;
	tlp->TmpLightGrp[2].n_lights = 0;
}

// 一時点光源（白色固定）をセットする
int DG_SetTmpLight(FVECTOR *point, float r_range, float e_range)
{
	DG_TLIGHT	*tlp;
	LIT_GRP		*grp;
	LIT_POINT	*lit;
	FVECTOR		max, min;
	int		nl;

	tlp = DG_TLights + DG_LightClock;
	grp = &tlp->TmpLightGrp[0];
	nl = grp->n_lights;
	if (nl >= MAX_TMPLIGHTS) return -1;
	grp->n_lights = nl + 1;
	lit = ((LIT_POINT*)grp->lit) + nl;
	lit->point = *point;
	lit->point.vw = 1.5f;
	lit->r_range = r_range;
	lit->e_range = e_range;
	lit->flag = LIT_FLAG_CHARAONLY;
	*(int*)&lit->color = 0x00ffffff;
	/* バウンディング設定 */
	max.vx = point->vx + e_range;
	max.vy = point->vy + e_range;
	max.vz = point->vz + e_range;
	min.vx = point->vx - e_range;
	min.vy = point->vy - e_range;
	min.vz = point->vz - e_range;
	BoundResize(&grp->bound_max, &grp->bound_min, &max);
	BoundResize(&grp->bound_max, &grp->bound_min, &min);
	return 0;
}

// 一時点光源をセットする
int DG_SetTmpLight2(FVECTOR *point, float r_range, float e_range, int color, int flag)
{
	DG_TLIGHT		*tlp;
	LIT_GRP		*grp;
	LIT_POINT	*lit;
	FVECTOR		max, min;
	int		nl;

	tlp = DG_TLights + DG_LightClock;
	grp = &tlp->TmpLightGrp[0];
	nl = grp->n_lights;
	if (nl >= MAX_TMPLIGHTS) return -1;
	grp->n_lights = nl + 1;
	lit = ((LIT_POINT*)grp->lit) + nl;
	lit->point = *point;
	lit->point.vw = 1.5f;
	lit->r_range = r_range;
	lit->e_range = e_range;
	lit->flag = flag;
	*(int*)&lit->color = color;
	/* バウンディング設定 */
	max.vx = point->vx + e_range;
	max.vy = point->vy + e_range;
	max.vz = point->vz + e_range;
	min.vx = point->vx - e_range;
	min.vy = point->vy - e_range;
	min.vz = point->vz - e_range;
	BoundResize(&grp->bound_max, &grp->bound_min, &max);
	BoundResize(&grp->bound_max, &grp->bound_min, &min);
	return 0;
}

// 一時スポット光源をセットする
int DG_SetTmpSpotLight(FVECTOR *point, FVECTOR *dir, float range,
					   float umbra, float penumbra, int color, int flag)
{
	DG_TLIGHT		*tlp;
	LIT_GRP		*grp;
	LIT_SPOT	*lit;
	//FVECTOR		max, min;
	int		nl;
	float	r;

	tlp = DG_TLights + DG_LightClock;
	grp = &tlp->TmpLightGrp[1];
	nl = grp->n_lights;
	if (nl >= MAX_TMPLIGHTS) return -1;
	grp->n_lights = nl + 1;
	lit = ((LIT_SPOT*)grp->lit) + nl;
	lit->point = *point;
	lit->point.vw = 1.5f;
	//_sceVu0Normalize(&lit->dir, dir);
	D3DXVec3Normalize(&lit->dir, dir);	
	lit->dir.vw = range;
	lit->umbra = umbra;
	lit->penumbra = penumbra;
	lit->flag = flag;
	*(int*)&lit->color = color;
	/* バウンディング対応バージョン */
	r = bp_sqrtf(1.0f - penumbra * penumbra);  //BP_MATH - emulate PS2 sqrtf
	{
		FVECTOR	end_pos;
		end_pos.vx = (float)lit->dir.x * range * 2;
		end_pos.vy = (float)lit->dir.y * range * 2;
		end_pos.vz = (float)lit->dir.z * range * 2;
		lit->bound_max.vx = DG_MAX(lit->point.vx, end_pos.vx);
		lit->bound_max.vy = DG_MAX(lit->point.vy, end_pos.vy);
		lit->bound_max.vz = DG_MAX(lit->point.vz, end_pos.vz);
		lit->bound_min.vx = DG_MIN(lit->point.vx, end_pos.vx);
		lit->bound_min.vy = DG_MIN(lit->point.vy, end_pos.vy);
		lit->bound_min.vz = DG_MIN(lit->point.vz, end_pos.vz);
	}
	BoundResize(&grp->bound_max, &grp->bound_min, &lit->bound_max);
	BoundResize(&grp->bound_max, &grp->bound_min, &lit->bound_min);
	return 0;
}


//-----------------------------------------------------------------------------

// 光源ベクトルを算出する(内部使用)
static inline float GetLightVector(VECTOR *vec, float r_range, VECTOR *light)
{
	float sq, sq2, pw, rr, rrrr;

	rr = vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz;
	rrrr = r_range * r_range;
	
	if (rr >= rrrr) return 0.0f;
	sq = bp_sqrtf(rr);   //BP_MATH - emulate PS2 sqrtf
	if (sq < 1.0f) sq = 1.0f;
	sq2 = r_range - sq;
	if (sq2 < 0.0F) return 0.0f;

	pw = sq2 / r_range;

	// lightベクトルを正規化
	rrrr = 1.0f / sq;
	light->vx = vec->vx * rrrr;
	light->vy = vec->vy * rrrr;
	light->vz = vec->vz * rrrr;
	return pw;
}

// 登録されている光源データから一番影響の強い２つを平行光源データとして取得する。
// 残りの１つはステージ固有の平行光源が必ず最後に入る
static int GetLightMatrix(VECTOR *pos, MATRIX *light, int fix_flag)
{
	VECTOR  fvec = {0.0f, 0.0f, 0.0f, 0.0f};
	int     offset;
	int     max_lit;
	int     i, j;
	float   r, d, p;

	FIXLIGHT *flp;
	LIT_DEF  *def;
	LIT_GRP  *grp;
	LIT_BLACKPOINT *black = NULL;
	int      n_group, map;

	max_lit = 0;
	light[0] = DG_LightMatrix;
	light[1] = DG_ColorMatrix;

	flp = fix_lights;
	if (fix_flag != 0) {
		// 一時光源は含めない
#if 1
		def = NULL;
		n_group = - 1;
#else
		def = flp->FixedLights;
		grp = def->grp;
		n_group = def->n_lit_group;
		flp ++;
#endif
	} else {
		// 一時光源を含める。
		// 一回目はTmpLight, 二回目以降はFixLightについて計算する。
		def = &DG_TLights[1 - DG_LightClock].TmpLightDef;
		grp = (LIT_GRP *)&def[1];
		n_group = def->n_lit_group;
	}

	for (; ;) {
		// バウンディングチェック
		if (def != NULL
			&& grp->bound_max.vx > pos->vx && grp->bound_min.vx < pos->vx
			&& grp->bound_max.vy > pos->vy && grp->bound_min.vy < pos->vy
			&& grp->bound_max.vz > pos->vz && grp->bound_min.vz < pos->vz) {

			// 光源の種類毎に処理
			if (grp->type & LIT_TYPE_POINT) { // 点光源処理
				LIT_POINT *lights;
				lights = (LIT_POINT *)grp->lit;

				for (j = grp->n_lights; j > 0; -- j, lights ++) {
					if (!(lights->flag & LIT_FLAG_CHARAONLY)) continue;
					if (lights->flag & LIT_FLAG_DISABLE) continue;

					r = lights->e_range;
					fvec.vx = d = pos->vx - lights->point.vx;
					if (FABS(d) > r) continue;
					fvec.vy = d = pos->vy - lights->point.vy;
					if (FABS(d) > r) continue;
					fvec.vz = d = pos->vz - lights->point.vz;
					if (FABS(d) > r) continue;
					
					p = GetLightVector(&fvec, lights->r_range * 2, &fvec);
					/* 光源ベクトルに距離影響度×１．５を乗算する */
					p = p * 1.5f * lights->point.vw;
					//_sceVu0ScaleVector(&fvec, &fvec, p * 1.5f);
					//D3DXVec3Scale(&fvec, &fvec, p * 1.5f);
					if (p <= 0.0f) continue;
					/* 光の強い順に並ぶようにする */
					for (offset = 0; offset < max_lit; offset ++) {
						if (light[0].m[3][offset] < p) break;
					}
					if (offset == 2) continue;
					/* 弱い光をスライドさせる */
					if (max_lit != 2) {
						i = 2;
					} else {
						i = 1;
					}
					for (; i > offset; i --) {
						light[0].m[0][i] = light[0].m[0][i-1];
						light[0].m[1][i] = light[0].m[1][i-1];
						light[0].m[2][i] = light[0].m[2][i-1];
						light[0].m[3][i] = light[0].m[3][i-1];
						// *(u_long128*)&light[1].m[i][0] = *(u_long128*)&light[1].m[i-1][0];
						light[1].m[i][0] = light[1].m[i-1][0];
						light[1].m[i][1] = light[1].m[i-1][1];
						light[1].m[i][2] = light[1].m[i-1][2];
						light[1].m[i][3] = light[1].m[i-1][3];						
					}
					if (max_lit < 2) {
						max_lit ++;
					}
					light[0].m[0][offset] = fvec.vx;
					light[0].m[1][offset] = fvec.vy;
					light[0].m[2][offset] = fvec.vz;
					light[0].m[3][offset] = p;
					light[1].m[offset][0] = lights->color.r / RGB_ONE;
					light[1].m[offset][1] = lights->color.g / RGB_ONE;
					light[1].m[offset][2] = lights->color.b / RGB_ONE;
				}
			} else if (grp->type & LIT_TYPE_SPOT) { // スポット光源処理
				LIT_SPOT *lights;
				float    cos;
				lights = (LIT_SPOT *)grp->lit;
				for (j = grp->n_lights; j > 0; j--, lights++) {
					if (!(lights->flag & LIT_FLAG_CHARAONLY)) continue;
					if (lights->flag & LIT_FLAG_DISABLE) continue;
					r = lights->dir.vw * 2;
#if 0
					// バウンディングボックス非対応バージョン
					fvec.vx = d = pos->vx - lights->point.vx;
					if (FABS(d) > r) continue;
					fvec.vy = d = pos->vy - lights->point.vy;
					if (FABS(d) > r) continue;
					fvec.vz = d = pos->vz - lights->point.vz;
					if (FABS(d) > r) continue;
#else
					// バウンディングボックス対応バージョン
					if (pos->vx > lights->bound_max.vx
						|| pos->vx < lights->bound_min.vx) continue;
					if (pos->vy > lights->bound_max.vy
						|| pos->vy < lights->bound_min.vy) continue;
					if (pos->vz > lights->bound_max.vz
						|| pos->vz < lights->bound_min.vz) continue;
					fvec.vx = pos->vx - lights->point.vx;
					fvec.vy = pos->vy - lights->point.vy;
					fvec.vz = pos->vz - lights->point.vz;
#endif
					p = GetLightVector(&fvec, r, &fvec);
					if (p <= 0.0f) continue;
					//cos = _sceVu0InnerProduct(&lights->dir, &fvec);
					cos = D3DXVec3Dot(&lights->dir, &fvec);
					if (cos >= lights->umbra) {
						cos = 1.0f;
					} else if (cos >= lights->penumbra) {
						cos = (cos - lights->penumbra) / (lights->umbra - lights->penumbra);
					} else {
						continue;
					}
					// 光源ベクトルに距離影響度×１．５を乗算する
					p = p * cos * 1.5f * lights->point.vw;
					//_sceVu0ScaleVector(&fvec, &fvec, p);
					//D3DXVec3Scale(&fvec, &fvec, p);
					// 光の強い順に並ぶようにする
					for (offset = 0; offset < max_lit; offset ++) {
						if (light[0].m[3][offset] < p) break;
					}
					if (offset == 2) continue;
					// 弱い光をスライドさせる
					if (max_lit != 2) {
						i = 2;
					} else {
						i = 1;
					}
					for (; i > offset; i --) {
						light[0].m[0][i] = light[0].m[0][i-1];
						light[0].m[1][i] = light[0].m[1][i-1];
						light[0].m[2][i] = light[0].m[2][i-1];
						light[0].m[3][i] = light[0].m[3][i-1];
						// *(u_long128*)&light[1].m[i][0] = *(u_long128*)&light[1].m[i-1][0];
						light[1].m[i][0] = light[1].m[i-1][0];
						light[1].m[i][1] = light[1].m[i-1][1];
						light[1].m[i][2] = light[1].m[i-1][2];
						light[1].m[i][3] = light[1].m[i-1][3];						
					}
					if (max_lit < 2) max_lit ++;
					light[0].m[0][offset] = fvec.vx;
					light[0].m[1][offset] = fvec.vy;
					light[0].m[2][offset] = fvec.vz;
					light[0].m[3][offset] = p;
					light[1].m[offset][0] = lights->color.r / RGB_ONE;
					light[1].m[offset][1] = lights->color.g / RGB_ONE;
					light[1].m[offset][2] = lights->color.b / RGB_ONE;
				}
			} else if (grp->type & LIT_TYPE_LINE) { // 線光源処理
				LIT_LINE	*lights;
				VECTOR		point;
				lights = (LIT_LINE *)grp->lit;
				for (j = grp->n_lights; j > 0; j--, lights++){
					if (!(lights->flag & LIT_FLAG_CHARAONLY)) continue;
					if (lights->flag & LIT_FLAG_DISABLE) continue;
					if (pos->vx > lights->bound_max.vx
						|| pos->vx < lights->bound_min.vx) continue;
					if (pos->vy > lights->bound_max.vy
						|| pos->vy < lights->bound_min.vy) continue;
					if (pos->vz > lights->bound_max.vz
						|| pos->vz < lights->bound_min.vz) continue;

					// 終点算出
					point.vx = lights->point.vx + lights->dir.vx * lights->dir.vw;
					point.vy = lights->point.vy + lights->dir.vy * lights->dir.vw;	
					point.vz = lights->point.vz + lights->dir.vz * lights->dir.vw;
					{
						float n1, n2, n3;
						// 各点での距離算出
						//n1 = _sceVu0InnerProduct(&lights->dir, &lights->point);
						//n2 = _sceVu0InnerProduct(&lights->dir, &point);
						//n3 = _sceVu0InnerProduct(&lights->dir, pos);
						n1 = D3DXVec3Dot(&lights->dir, &lights->point);
						n2 = D3DXVec3Dot(&lights->dir, &point);
						n3 = D3DXVec3Dot(&lights->dir, pos);
						if (n3 <= n1){
							// 始点位置における点光源計算を行なう
							point = lights->point;
						} else if (n3 >= n2){
							// 終点位置における点光源計算を行なう
							//point = point;
						} else {
							// 最近点を算出し、その位置において点光源計算を行なう
							n3 = n3 - n1;
							point.vx = lights->point.vx + lights->dir.vx * n3;
							point.vy = lights->point.vy + lights->dir.vy * n3;
							point.vz = lights->point.vz + lights->dir.vz * n3;
						}
					}
					fvec.vx = pos->vx - point.vx;
					fvec.vy = pos->vy - point.vy;
					fvec.vz = pos->vz - point.vz;

					p = GetLightVector(&fvec, lights->r_range * 2, &fvec);
					if (p <= 0.0f) continue;
					// 光源ベクトルに距離影響度×１．５を乗算する
					p = p * 1.5f * lights->point.vw;
					//_sceVu0ScaleVector(&fvec, &fvec, p * 1.5f);
					//D3DXVec3Scale(&fvec, &fvec, p * 1.5f);
					// 光の強い順に並ぶようにする
					for (offset = 0; offset < max_lit; offset ++) {
						if (light[0].m[3][offset] < p) break;
					}
					if (offset == 2) continue;
					// 弱い光をスライドさせる
					if (max_lit != 2) {
						i = 2;
					} else {
						i = 1;
					}
					for (; i > offset; i --) {
						light[0].m[0][i] = light[0].m[0][i-1];
						light[0].m[1][i] = light[0].m[1][i-1];
						light[0].m[2][i] = light[0].m[2][i-1];
						light[0].m[3][i] = light[0].m[3][i-1];
						// *(u_long128*)&light[1].m[i][0] = *(u_long128*)&light[1].m[i-1][0];
						light[1].m[i][0] = light[1].m[i-1][0];
						light[1].m[i][1] = light[1].m[i-1][1];
						light[1].m[i][2] = light[1].m[i-1][2];
						light[1].m[i][3] = light[1].m[i-1][3];						
					}
					if (max_lit < 2) max_lit ++;
					light[0].m[0][offset] = fvec.vx;
					light[0].m[1][offset] = fvec.vy;
					light[0].m[2][offset] = fvec.vz;
					light[0].m[3][offset] = p;
					light[1].m[offset][0] = lights->color.r / RGB_ONE;
					light[1].m[offset][1] = lights->color.g / RGB_ONE;
					light[1].m[offset][2] = lights->color.b / RGB_ONE;
				}
			} else if (grp->type & LIT_TYPE_BLACKPOINT) { // 黒光源処理
				LIT_BLACKPOINT	*lights;
				lights = (LIT_BLACKPOINT *)grp->lit;
				for (j = grp->n_lights; (j > 0) && (black == NULL); j--, lights++) {
					if (!(lights->flag & LIT_FLAG_CHARAONLY)) continue;
					if (lights->flag & LIT_FLAG_DISABLE) continue;
					if (pos->vx > lights->bound_max.vx
						|| pos->vx < lights->bound_min.vx) continue;
					if (pos->vy > lights->bound_max.vy
						|| pos->vy < lights->bound_min.vy) continue;
					if (pos->vz > lights->bound_max.vz
						|| pos->vz < lights->bound_min.vz) continue;
					black = lights;
				}
			}
		}

		grp ++;
		if (-- n_group <= 0) {
#if 0
			// TODO:GM_CurrentMap対応
			extern int GM_CurrentMap;
			do { //map_id support
				if ((def = flp->FixedLights) == NULL) break;
				grp = (LIT_GRP *)&def[1];
				n_group = def->n_lit_group;
				map = flp->map_id;
				flp ++;
			} while ((GM_CurrentMap != 0 && (map & GM_CurrentMap) == 0) || n_group == 0);
#else
			do {
				def = flp->FixedLights;
				if (def == NULL) break;
				grp = (LIT_GRP *)&def[1];
				n_group = def->n_lit_group;
				map = flp->map_id;
				flp ++;
			} while (n_group == 0);
#endif
			if (def == NULL) break;
		}
	}
	
	// 黒光源による減衰処理
	if (black != NULL){
		fvec.vx = pos->vx - black->point.vx;
		fvec.vy = pos->vy - black->point.vy;
		fvec.vz = pos->vz - black->point.vz;
		p = GetLightVector(&fvec, black->r_range * 2, &fvec);
		// original
		//p = DG_MAX(p, 0.0f); p = 1.0f - p;
		if (p < 0.0f) {
			p = 1.0f;
		} else {
			p = 1.0f - p;
		}
		//_sceVu0ScaleVector(&light[1].m[0][0], &light[1].m[0][0], p);
		//_sceVu0ScaleVector(&light[1].m[1][0], &light[1].m[1][0], p);
		//_sceVu0ScaleVector(&light[1].m[2][0], &light[1].m[2][0], p);
		//_sceVu0ScaleVector(&light[1].m[3][0], &light[1].m[3][0], p);
#if 0
		D3DXVec3Scale((FVECTOR *)&light[1].m[0][0],
					  (FVECTOR *)&light[1].m[0][0], p);
		D3DXVec3Scale((FVECTOR *)&light[1].m[1][0],
					  (FVECTOR *)&light[1].m[1][0], p);
		D3DXVec3Scale((FVECTOR *)&light[1].m[2][0],
					  (FVECTOR *)&light[1].m[2][0], p);
		D3DXVec3Scale((FVECTOR *)&light[1].m[3][0],
					  (FVECTOR *)&light[1].m[3][0], p);
#endif
	}
	return 2;
}

// 登録されている光源データから一番影響の強い３つを平行光源データとして取得する
int DG_GetLightMatrix(VECTOR *pos, MATRIX *light)
{
	return GetLightMatrix(pos, light, 0);
}

// 登録されている光源データから一番影響の強い３つを平行光源データとして取得する
// (一時光源含まず)
int DG_GetLightMatrixFix(VECTOR *pos, MATRIX *light)
{
	return GetLightMatrix(pos, light, 1);
}

// オブジェクトに光源マトリクスを設定する
void DG_SetLightMatrix(DG_OBJS *objs, MATRIX *light)
{
	int     i; 
	DG_OBJ  *obj;

	objs->light = light;
	obj = objs->objs;
	for (i = objs->n_models; i > 0; i --) {
		obj->light = light;
		obj ++;
	}
}

//-----------------------------------------------------------------------------
