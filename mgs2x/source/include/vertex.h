/*
	libdg使用頂点タイプ定義(Xbox新設)

	2001/05/15 F.Miyauchi
	$Id: vertex.h,v 1.1.1.3 2002/11/19 11:42:00 Yoshizawa1 Exp $
*/

// libdg.h で include される

#ifndef __VERTEX_H__
#define __VERTEX_H__

//-----------------------------------------------------------------------------
// 頂点バッファ設定用
//-----------------------------------------------------------------------------

// 頂点バッファのため、適当に頂点フォーマット/フラグを定義してみる。暫定。

// 頂点座標のみ
typedef struct tagDG_POINTVERTEX {
	D3DXVECTOR3  v;
	D3DCOLOR     rgba;
} DG_POINTVERTEX;
#define D3DFVF_POINTVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// 色付き頂点
typedef struct tagDG_COLORVERTEX {
	D3DXVECTOR3  v;
	D3DCOLOR     rgba;
} DG_COLORVERTEX;
#define D3DFVF_COLORVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// 色付き頂点(法線付き)
typedef struct tagDG_COLORNVERTEX {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;		
	D3DCOLOR     rgba;
} DG_COLORNVERTEX;
#define D3DFVF_COLORNVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)

// 頂点/法線/色/テクスチャ座標
typedef struct tagDG_TEXTUREVERTEX {
	D3DXVECTOR3  v;
	D3DCOLOR     rgba;
	float        tu0, tv0;
} DG_TEXTUREVERTEX;
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// 頂点/法線/色/テクスチャ座標
typedef struct tagDG_NORMALVERTEX {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
} DG_NORMALVERTEX;
#define D3DFVF_NORMALVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// PRIM2用頂点
typedef DG_NORMALVERTEX    DG_PRIM2VERTEX;
#define D3DFVF_PRIM2VERTEX D3DFVF_NORMALVERTEX

typedef struct tagDG_NORMALVERTEX2 {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
	float        tu1, tv1;
} DG_NORMALVERTEX2;
#define D3DFVF_NORMALVERTEX2 (D3DFVF_NORMALVERTEX|D3DFVF_TEX2)

typedef struct tagDG_NORMALVERTEX3 {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
	float        tu1, tv1;
	float        tu2, tv2;	
} DG_NORMALVERTEX3;
#define D3DFVF_NORMALVERTEX3 (D3DFVF_NORMALVERTEX|D3DFVF_TEX1|D3DFVF_TEX3)


// 頂点/ブレンド係数/法線/色/テクスチャ座標
typedef struct tagDG_BLENDVERTEX {
	D3DXVECTOR3  v;
	float        blend;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
} DG_BLENDVERTEX;
#define D3DFVF_BLENDVERTEX (D3DFVF_XYZB1|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct tagDG_BLENDVERTEX2 {
	D3DXVECTOR3  v;
	float        blend;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
	float        tu1, tv1;
} DG_BLENDVERTEX2;
#define D3DFVF_BLENDVERTEX2 (D3DFVF_BLENDVERTEX|D3DFVF_TEX2)

typedef struct tagDG_BLENDVERTEX3 {
	D3DXVECTOR3  v;
	float        blend;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
	float        tu1, tv1;
	float        tu2, tv2;
} DG_BLENDVERTEX3;
#define D3DFVF_BLENDVERTEX3 (D3DFVF_BLENDVERTEX|D3DFVF_TEX2|D3DFVF_TEX3)


//座標変換/ライティング済み2D頂点
typedef struct tagDG_2DCOLORVERTEX {
	D3DXVECTOR4 v;
	D3DCOLOR    rgba;
} DG_2DCOLORVERTEX;
#define D3DFVF_2DCOLORVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

typedef struct tagDG_2DVERTEX {
	D3DXVECTOR4 v;
	D3DCOLOR    rgba;
	float       tu0, tv0;
} DG_2DVERTEX;
#define D3DFVF_2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct tagDG_2DVERTEX2 {
	D3DXVECTOR4 v;
	D3DCOLOR    rgba;
	float       tu0, tv0;
	float       tu1, tv1;	
} DG_2DVERTEX2;
#define D3DFVF_2DVERTEX2 (D3DFVF_2DVERTEX|D3DFVF_TEX2)

typedef struct tagDG_2DVERTEX3 {
	D3DXVECTOR4 v;
	D3DCOLOR    rgba;
	float       tu0, tv0;
	float       tu1, tv1;
	float       tu2, tv2;
} DG_2DVERTEX3;
#define D3DFVF_2DVERTEX3 (D3DFVF_2DVERTEX|D3DFVF_TEX2|D3DFVF_TEX3)

// EVMモデル用頂点
#if 0
typedef struct tagDG_EVMVERTEX {
	D3DXVECTOR3  v;
	float        blend1;  // 頂点シェーダのv1.x
	float        blend2;  // v1.y
	float        blend3;  // v1.z (v1.w = 1 - (v1.x + v1.y + v1.z)
	DWORD        indices; // 行列インデックス(v2.xyzw)
	D3DXVECTOR3  n;       // v3
	D3DCOLOR     rgba;
	float        tu0, tv0;
	float        tu1, tv1;
	float        tu2, tv2;
} DG_EVMVERTEX;
#define D3DFVF_EVMVERTEX (D3DFVF_XYZB4|D3DFVF_LASTBETA_UBYTE4|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEX2|D3DFVF_TEX3)
#else
// EVMモデル用頂点
typedef struct tagDG_EVMVERTEX {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
	float        tu1, tv1;
	float        tu2, tv2;
	float        tu3, tv3;
	float        blend[4];
	float        index[4];
} DG_EVMVERTEX;
// 固定機能の使用を止めたのでFVF不要
#define D3DFVF_EVMVERTEX (0)
#endif

// 曲面パッチ用頂点フォーマット
// ※現状通常の頂点と同じ
typedef struct tagDG_PATCHVERTEX {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0;
} DG_PATCHVERTEX;
#define D3DFVF_PATCHVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// ボリュームテクスチャ
typedef struct tagDG_VOLUMEVERTEX {
	D3DXVECTOR3  v;
	D3DXVECTOR3  n;
	D3DCOLOR     rgba;
	float        tu0, tv0, tw0;
} DG_VOLUMEVERTEX;
#define D3DFVF_VOLUMEVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE3(0))


// 頂点バッファタイプ
enum {
	DG_VTYPE_POINT,
	DG_VTYPE_COLOR,	
	DG_VTYPE_NORMAL,
	DG_VTYPE_NORMAL2,
	DG_VTYPE_NORMAL3,
	DG_VTYPE_BLEND,
	DG_VTYPE_BLEND2,
	DG_VTYPE_BLEND3,
	DG_VTYPE_2DCOLOR,
	DG_VTYPE_2DTEX,
	DG_VTYPE_2DTEX2,
	DG_VTYPE_2DTEX3,
	DG_VTYPE_EVM,
};

#endif /* __VERTEX_H__ */
