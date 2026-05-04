//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    electrofield.c
    オセロット電磁フィールド
    2001/08/06 Masafumi Okuta
    $Id: electrofield.c,v 1.1.1.3 2002/11/19 11:47:50 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"

// 便利マクロ
extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))

#define	SET_FLAG( a,b )		(a) |= (b)
#define	UNSET_FLAG( a,b )	(a) &= ~(b)
#define	CLEAR_FLAG( a ) 	(a) = 0
#define	BODYWORLD(a,b) 		(a)->objs->objs[(b)].world
#define FTOI12(_f)		( DG_FTOI( ( (float)(_f)*4096.0f) ) )

// 設定マクロ
#define N_UNIT		(8)		// バリアユニット数
#define N_PRIMS		(16)		// プリミティブ数
#define N_VERTS		(32)		// 頂点数
#define N_VERTS_CIR	(N_VERTS >> 1)	// 円分割数

#define RADIUS		(3000.f)	// バリアの半径
#define Z_DIST		(70.f)		// Y=X^2のXの√

#define CIR_ANG		(1800)		// 弧の角度
#define CIR_ANG_HALF	(CIR_ANG >> 1)	// 弧の角度の半分

#define	SCR_POS		(SCRPAD_ADDR)	// スクラッチパッド

#define TEX_NAME	(10972307)	// chi01_alp

// シード固定ランダム用関数
static inline int ELE_GetRandom( int* pnSeed, int min, int max){ return ( GM_IRnd( pnSeed) % ((max - min) + 1) + min ); }
static inline int ELE_Rand( int* pnSeed, int a){ return ( GM_IRnd( pnSeed) % a ); }
static inline float ELE_Rnd( int* pnSeed){ return ( GM_Rnd( pnSeed) ); }
static inline float ELE_FRnd( int* pnSeed){ return ( GM_FRnd( pnSeed) ); }

// 外部
extern float _TS_Sin( int s );

typedef struct _ELECTROFEILD { // オセロット電磁場フィールド
    // 基本システム
    GV_ACT_EX		actor;

    // 拡張管理
    int			nBaseAng;			// 基本方向
    int			nRandSeed;			// ランダムシード
    float 		fAlphaRate;			// α値
    FMATRIX		matOrg;				// 座標系

    FMATRIX		matPrim[N_UNIT];		// 座標系
    SVECTOR		vecRot[N_UNIT];			// 回転情報

    // テクスチャのUV情報保存用
    float		u_scale[N_UNIT];		
    float		v_scale[N_UNIT];		
    float		u_offset[N_UNIT];		
    float		v_offset[N_UNIT];		

    // 描画系
    DG_PRIM2*		prim[N_UNIT];			// エフェクト
} Work;

// プリミティブ更新
static int UpdatePacket( Work* 		work, 		// 電磁場ワーク
			 DG_PRIM2*	prim,		// プリミティブワーク
			 int		nBaseAng,	// 基本方向
			 u_int		nRGBA,		// カラー
			 float		u_scale,	// UV情報:テクスチャデータより
			 float		v_scale,	// UV情報:テクスチャデータより
			 float		u_offset,	// UV情報:テクスチャデータより
			 float		v_offset)	// UV情報:テクスチャデータより
{
    int 	i,j;
    u_short 	a = (u_short)(nRGBA & 0x000000ff);

    { // UVアニメーション:与えられた角度を基準にテクスチャデータを扇形に取得する
	DG_PRIM2_UVRGB*	uvrgb1;

	uvrgb1 = prim->uvrgb[ prim->buffer_clock];
	for ( i = 0; i < N_PRIMS; i++ ){
	    float fOutRate = 1.f - (float)i / (float)N_PRIMS;
	    float fInRate  = fOutRate - 1.f / (float)N_PRIMS;
	    for ( j = 0; j < N_VERTS_CIR; j++ ){
		int nAng = j * 4096 / N_VERTS_CIR + nBaseAng;
		u_short nAlpha = (u_short)((float)a * ( 1.f - (float)abs( j - (N_VERTS_CIR >> 1) ) / (float)(N_VERTS_CIR >> 1)) * work->fAlphaRate);
		float fU;
		float fV;

		nAng &= 4095;
		fU = _TS_Sin( nAng) 	   * 0.5f * fOutRate + 0.5f;
		fV = _TS_Sin( nAng + 1024) * 0.5f * fOutRate + 0.5f;
		uvrgb1->u = FTOI12( fU * u_scale + u_offset ); //
		uvrgb1->v = FTOI12( fV * v_scale + v_offset ); //
		uvrgb1->a = nAlpha;
		uvrgb1++;

		fU = _TS_Sin( nAng) 	   * 0.5f * fInRate + 0.5f;
		fV = _TS_Sin( nAng + 1024) * 0.5f * fInRate + 0.5f;
		uvrgb1->u = FTOI12( fU * u_scale + u_offset ); //
		uvrgb1->v = FTOI12( fV * v_scale + v_offset ); //
		uvrgb1->a = nAlpha;
		uvrgb1++;
	    }
	}
    }

    return 1;
}

// 動作関数
static void Act( Work* work )
{
    int i;

    for ( i = 0; i < N_UNIT; i++){
	// プリミティブ更新
	GM_GroupPrim2( work->prim[i], GM_CurrentStageMap );
	DG_SwitchBuffPrim2( work->prim[i]);

	// UVアニメ用の回転情報を更新
	if ( i % 2 == 0 ) 	work->vecRot[i].vz += (i+1) * 8 + ELE_GetRandom( &work->nRandSeed, -4,4);
	else			work->vecRot[i].vz += (i+1) * -8 + ELE_GetRandom( &work->nRandSeed, -4,4);
	work->vecRot[i].vz &= 4095;

	UpdatePacket( work, work->prim[i], work->vecRot[i].vz, 0xc0c0c030, work->u_scale[i], work->v_scale[i], 
		      work->u_offset[i], work->v_offset[i]);
    }

    // α減衰
    work->fAlphaRate *= 0.97f;
}

// 破棄関数
static void Die( Work* work )
{
    int i;
    for ( i = 0; i < N_UNIT; i++){
	work->prim[i] = MAO_FreePrim2( work->prim[i] );
    }
}

// プリミティブ初期化
static int InitPacket( Work* 		work, 		// ワーク
		       DG_PRIM2*	prim,		// プリミティブ
		       DG_TEX* 		tex,		// テクスチャデータ
		       long64		tagAlpha,	// ALPHAタグ
		       u_int		nRGBA,		// カラー
		       int		nRaise)		// 優先順位
{
    u_char 	r,g,b,a;
    int 	i,j;
    float	fRadOut;
    float	fRadIn;
    float	fZDistOut;
    float	fZDistIn;

    FVECTOR* pvecVert;
    FVECTOR* pvecVertCopy;
    FVECTOR* pvecVertTop1;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    // 最外周のドーナッツを作成
    pvecVert 	= SCR_POS;
    pvecVertTop1 = prim->pos[0];
    pvecVertCopy = prim->pos[0];
    fRadOut = RADIUS;
    fRadIn  = fRadOut - RADIUS / (float)N_PRIMS;
    fZDistOut = Z_DIST;
    fZDistIn  = fZDistOut - Z_DIST / (float)N_PRIMS;
    for ( j = 0; j < N_VERTS_CIR; j++ ){
	int nAng = (j * CIR_ANG / N_VERTS_CIR - CIR_ANG_HALF) + work->nBaseAng;
	float fX = _TS_Sin( nAng);
	float fY = _TS_Sin( nAng + 1024);

	pvecVert->vx = fX * fRadOut;
	pvecVert->vy = fY * fRadOut;
	pvecVert->vz = fZDistOut * fZDistOut;
	pvecVert->vw = 1.f;
	pvecVert++;

	pvecVert->vx = fX * fRadIn;
	pvecVert->vy = fY * fRadIn;
	pvecVert->vz = fZDistIn * fZDistIn;
	pvecVert->vw = 1.f;
	pvecVert++;
    }

    // スクラッチパッド->メインメモリ
    Mao_CopyScrToMem( pvecVertTop1, SCR_POS, sizeof(FVECTOR), N_VERTS);
    pvecVertTop1 += N_VERTS;

    // 頂点位置は相対で作成
    for ( i = 1; i < N_PRIMS; i++ ){
	fRadOut 	= fRadIn;
	fRadIn  	= fRadOut - RADIUS / (float)N_PRIMS;
	fZDistOut 	= fZDistIn;
	fZDistIn  	= fZDistOut - Z_DIST / (float)N_PRIMS;
	pvecVert 	= SCR_POS;
	for ( j = 0; j < N_VERTS_CIR; j++ ){
	    int nAng = (j * CIR_ANG / N_VERTS_CIR - CIR_ANG_HALF) + work->nBaseAng;
	    float fX = _TS_Sin( nAng);
	    float fY = _TS_Sin( nAng + 1024);
	    pvecVertCopy++;

	    _sceVu0CopyVector( pvecVert, pvecVertCopy);
	    pvecVert++;
	    pvecVertCopy++;

	    pvecVert->vx = fX * fRadIn;
	    pvecVert->vy = fY * fRadIn;
	    pvecVert->vz = fZDistIn * fZDistIn;
	    pvecVert->vw = 1.f;
	    pvecVert++;
	}

	Mao_CopyScrToMem( pvecVertTop1, SCR_POS, sizeof(FVECTOR), N_VERTS);
	pvecVertTop1 += N_VERTS;
    }

    memcpy( prim->pos[1], prim->pos[0], sizeof(FVECTOR) * N_PRIMS * N_VERTS);

    { // UVRGBA設定
	DG_PRIM2_UVRGB*	uvrgb1;
	DG_PRIM2_UVRGB*	uvrgb2;
	DG_PRIM2_UVRGB*	uvrgbTop;

	uvrgb1 = prim->uvrgb[0];
	uvrgb2 = prim->uvrgb[1];
	uvrgbTop = prim->uvrgb[0];
	for ( i = 0; i < N_PRIMS; i++ ){
	    float fOutRate = 1.f - (float)i / (float)N_PRIMS;
	    float fInRate  = fOutRate - 1.f / (float)N_PRIMS;
	    for ( j = 0; j < N_VERTS_CIR; j++ ){
		int nAng = j * 4096 / N_VERTS_CIR;
		u_short nAlpha = (u_short)((float)a * ( 1.f - (float)abs( j - (N_VERTS_CIR >> 1) ) / (float)(N_VERTS_CIR >> 1)));
		float fU = _TS_Sin( nAng) 	 * 0.5f * fOutRate + 0.5f;
		float fV = _TS_Sin( nAng + 1024) * 0.5f * fOutRate + 0.5f;
		uvrgb2->u = uvrgb1->u = FTOI12( fU * tex->u_scale + tex->u_offset ); //
		uvrgb2->v = uvrgb1->v = FTOI12( fV * tex->v_scale + tex->v_offset ); //
		uvrgb2->q = uvrgb1->q = 4096;
		uvrgb2->f = uvrgb1->f = 0x0fff;
		uvrgb2->r = uvrgb1->r = r;
		uvrgb2->g = uvrgb1->g = g;
		uvrgb2->b = uvrgb1->b = b;
		uvrgb2->a = uvrgb1->a = nAlpha;
		uvrgb1++;
		uvrgb2++;

		fU = _TS_Sin( nAng) 	   * 0.5f * fInRate + 0.5f;
		fV = _TS_Sin( nAng + 1024) * 0.5f * fInRate + 0.5f;
		uvrgb2->u = uvrgb1->u = FTOI12( fU * tex->u_scale + tex->u_offset ); //
		uvrgb2->v = uvrgb1->v = FTOI12( fV * tex->v_scale + tex->v_offset ); //
		uvrgb2->q = uvrgb1->q = 4096;
		uvrgb2->f = uvrgb1->f = 0x0fff;
		uvrgb2->r = uvrgb1->r = r;
		uvrgb2->g = uvrgb1->g = g;
		uvrgb2->b = uvrgb1->b = b;
		uvrgb2->a = uvrgb1->a = nAlpha;
		uvrgb1++;
		uvrgb2++;
	    }
	    uvrgbTop += N_VERTS;
	}
    }

    return 1;
}
// リソース初期化
static int GetResources( Work*		work,
			 FMATRIX*	pmatOrg)		// 発生座標系	
{
    int i;
    DG_TEX* tex;

    _sceVu0CopyMatrix( &work->matOrg, pmatOrg);

    work->fAlphaRate = 1.f;
    
    for ( i = 0; i < N_UNIT; i++){
	FMATRIX mat;

	_sceVu0CopyMatrix( &work->matPrim[i], &DG_UnitMatrix);
	work->vecRot[i] = DG_ZeroSVector;
	work->vecRot[i].vz = ELE_Rand( &work->nRandSeed, 4096);

	DG_SetPos2( &DG_ZeroVector, &DG_ZeroSVector); 
	DG_GetPos( &mat);
	MAO_AbsMat( &work->matPrim[i], &work->matOrg, &mat);
	_sceVu0CopyVector( (FVECTOR*)&work->matPrim[i].m[3][0], (FVECTOR*)&work->matOrg.m[3][0]);

	// プリミティブ本体の作成
	work->prim[i] = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_SHADE|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
	if ( work->prim[i] == NULL){
	    MAO_PRINTF("Cannot MakePrim!!\n");
	    return -1;
	}
	tex = DG_GetTexture( TEX_NAME );

	if ( i % 3 == 0) 	InitPacket( work, work->prim[i], tex, SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0x00 ), 0x80808020, -1000 * i);
	else if ( i % 3 == 0) 	InitPacket( work, work->prim[i], tex, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ), 0x10204020, -1000 * i);
	else		 	InitPacket( work, work->prim[i], tex, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ), 0x10204020, -1000 * i);

	work->prim[i]->root = &work->matPrim[i];
	work->u_scale[i]  = tex->u_scale;
	work->v_scale[i]  = tex->v_scale;
	work->u_offset[i] = tex->u_offset;
	work->v_offset[i] = tex->v_offset;
    }

    return 0;
}

// 電磁フィールドを生成
void* NewElectroField( FMATRIX*  pmatOrg,		// 発生座標系
		       int	 nBaseAng,		// 基本角度	
		       int	 nRandSeed)		// ランダムシード
{
    Work*	work;

    OPERATOR();
    
    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	work->nBaseAng = nBaseAng;
	work->nRandSeed = nRandSeed;

	if ( GetResources( work, pmatOrg) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
