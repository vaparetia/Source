//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    maoparticle.c
    汎用パーティクル
    2001/07/23 Masafumi Okuta
    $Id: maoparticle.c,v 1.1.1.3 2002/11/19 11:47:52 Yoshizawa1 Exp $
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

#ifdef DEBUG_MODE
//#define CALC_ACT_TIME
#endif

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))

#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI8(_f)	( DG_FTOI( ( (float)(_f)*256.0f) ) )

// RGBA設定
#define COL_R		(244)
#define COL_G		(255)
#define COL_B		(175)
#define ALPHA		(20)

// プリミティブ,頂点数
#define N_PRIMS		(32)
#define N_VERTS		(16)

#define RAISE		(500)

static inline int MPART_DemoRand( int nSeed, int a ){  
    return (GM_IRnd( &nSeed ) % a); 
} 
static inline int MPART_GetDemoRandom( int nSeed, int min, int max ){ 
    return (GM_IRnd( &nSeed ) % ((max - min) + 1) + min); 
}



enum{ // 拡張フラグ
EXFLAG_NORM_ALP = 0x00000001, // 通常半透明
EXFLAG_ADD_ALP  = 0x00000002, // 加算半透明
EXFLAG_SUB_ALP  = 0x00000004, // 減算半透明
// チェック用
EXFLAG_CHK_ALPHA  = 0x0000000f, // 半透明モードチェック
EXFLAG_BOOLD  	= 0x00000010, // 血チェック
};

extern float 	_TS_Sin( int );
extern float	OK_slow_param;

typedef struct _MAO_PARTICLE {
    // 基本システム
    GV_ACT_EX		actor;
    int			map;				// 所属するマップ
    HZX_GROUP_ID	hzx_id;				// HZXのグループID 
    
    // 拡張管理
    int			nCntr;

    FVECTOR*		vecPos;				// パーティクル位置
    FVECTOR*		vecDir;				// 方向ベクトル
    int*		nLife;				// 生存時間
    float*		fSpeed;				// 速度
    float*		fSpeedAdd;			// 1フレームの速度変化量
    float*		fSize;				// 大きさ
    float*		fSizeAdd;			// 1フレームのサイズ変化量 
    u_short*		nR;				// R値 12:4の固定小数
    u_short*		nG;				// G値 12:4の固定小数
    u_short*		nB;				// B値 12:4の固定小数
    u_short*		nA;				// A値 12:4の固定小数
    short*		nDecR;				// R減衰
    short*		nDecG;				// G減衰
    short*		nDecB;				// B減衰
    short*		nDecA;				// A減衰
    int*		nRadAng;			// 回転角度

    // 引数
    int			nRandSeed;			// ランダムシード
    FMATRIX		matOrg;				// 発生座標系
    int			nPrimNum;			// 発生数
    int			nVertNum;			// 頂点数
    int			nLifeMin;			// 生存時間最小
    int			nLifeMax;			// 生存時間最大
    int			nAng;				// 角度範囲
    float		fInitSpd;			// 初期速度
    float		fAimSpd;			// 目標速度
    float		fGravity;			// 重力
    float		fInitSize;			// 初期サイズ
    float		fAimSize;			// 目標サイズ
    int			texcode;			// テクスチャコード
    int			nInitRGBA;			// 初期RGBA
    int			nAimRGBA;			// 目標RGBA
    float		fRandRate;			// ランダム幅(0.f～1.f)
    int			nFlag;				// 拡張フラグ

    // 描画用
    DG_PRIM2*		prim;		
}Work;

#ifdef CALC_ACT_TIME
// 処理時間ダンプ
static void DbgDumpExecTime( int nTime)
{
    DEBUG_Locate( 240, 10, 0 );
    DEBUG_Printf("WATERDUST-EXECTIME = %.3f\n", (float)nTime / 60.f / 262.f );
}
#endif
// 動作関数
static void Act( Work* work )
{
    int 	i,j;	
    FVECTOR*	pvecPos;
    FVECTOR*	pvecDir;
    int*	pnLife;
    float*	pfSpeed;
    float*	pfSpeedAdd;
    float*	pfSize;
    float*	pfSizeAdd;
    u_short*	pnR;
    u_short*	pnG;
    u_short*	pnB;
    u_short*	pnA;
    short*	pnDecR;
    short*	pnDecG;
    short*	pnDecB;
    short*	pnDecA;
    int*	pnRadAng;

    FVECTOR*		pvecPrim;
    DG_PRIM2_UVRGBWH*	uvrgbwh;

#ifdef CALC_ACT_TIME
    int time;
GV_SET_PRFC_CLOCK();
#endif
    GM_GroupPrim2( work->prim, GM_CurrentStageMap );

    if ( OK_slow_param < 0.01f){ // スロー過ぎるのでうごかなくする
	return;
    }

    pvecPos 	= work->vecPos;
    pvecDir 	= work->vecDir;
    pnLife  	= work->nLife;
    pfSpeed 	= work->fSpeed;
    pfSpeedAdd 	= work->fSpeedAdd;
    pfSize  	= work->fSize;
    pfSizeAdd  	= work->fSizeAdd;
    pnR 	= work->nR;
    pnG 	= work->nG;
    pnB 	= work->nB;
    pnA 	= work->nA;
    pnDecR 	= work->nDecR;
    pnDecG 	= work->nDecG;
    pnDecB 	= work->nDecB;
    pnDecA 	= work->nDecA;
    pnRadAng	= work->nRadAng;

    pvecPrim = work->prim->pos[work->prim->buffer_clock];
    uvrgbwh  = work->prim->uvrgb[work->prim->buffer_clock];
    for ( i = 0; i < work->nPrimNum; i++){
	for ( j = 0; j < work->nVertNum; j++){
	    FVECTOR vec;

	    _sceVu0ScaleVector( &vec, pvecDir, ((*pfSpeed) * OK_slow_param) );
	    _sceVu0AddVector( pvecPos, pvecPos, &vec);
	    pvecDir->vy -= (work->fGravity * OK_slow_param);
	    _sceVu0Normalize( pvecDir, pvecDir);

	    _sceVu0CopyVector( pvecPrim, pvecPos);

	    // 速度
	    (*pfSpeed) += ((*pfSpeedAdd) * OK_slow_param);
	    if ( (*pfSpeed) < 0.f ) {
		(*pfSpeed) = 0;
	    }
	    // サイズ
	    uvrgbwh->w  = (int)((*pfSize) * _TS_Sin( (*pnRadAng)) );
	    uvrgbwh->h  = (int)((*pfSize) * _TS_Sin( (*pnRadAng) + 1024) );
	    (*pfSize)  += ((*pfSizeAdd) * OK_slow_param);
	    if ( (*pfSize) < 0.f ) {
		(*pfSize) = 0.f;
	    }

	    // RGBA
	    uvrgbwh->r  = (u_char)((*pnR) >> 8);
	    uvrgbwh->g  = (u_char)((*pnG) >> 8);
	    uvrgbwh->b  = (u_char)((*pnB) >> 8);
	    uvrgbwh->a  = (u_char)((*pnA) >> 8);

	    if ( (*pnR) + (*pnDecR) >= 0xffff )    (*pnR) = 0xffff;
	    else if ( (*pnR) + (*pnDecR) <= 0x00 ) (*pnR) = 0x00;
	    else			       	   (*pnR) += (u_short)((float)(*pnDecR) * OK_slow_param);
	    if ( (*pnG) + (*pnDecG) >= 0xffff )    (*pnG) = 0xffff;
	    else if ( (*pnG) + (*pnDecG) <= 0x00 ) (*pnG) = 0x00;
	    else			       	   (*pnG) += (u_short)((float)(*pnDecG) * OK_slow_param);
	    if ( (*pnB) + (*pnDecB) >= 0xffff )    (*pnB) = 0xffff;
	    else if ( (*pnB) + (*pnDecB) <= 0x00 ) (*pnB) = 0x00;
	    else			       	   (*pnB) += (u_short)((float)(*pnDecB) * OK_slow_param);
	    if ( (*pnA) + (*pnDecA) >= 0xffff )    (*pnA) = 0xffff;
	    else if ( (*pnA) + (*pnDecA) <= 0x00 ) (*pnA) = 0x00;
	    else			       	   (*pnA) += (u_short)((float)(*pnDecA) * OK_slow_param);

	    pvecPos++;
	    pvecDir++;
	    pnLife++;
	    pfSpeed++;
	    pfSpeedAdd++;
	    pfSize++;
	    pfSizeAdd++;
	    pnR++;
	    pnG++;
	    pnB++;
	    pnA++;
	    pnDecR++;
	    pnDecG++;
	    pnDecB++;
	    pnDecA++;
	    pnRadAng++;

	    pvecPrim++;
	    uvrgbwh++;
	}
    }	

    work->nCntr++;
    if ( (int)((float)work->nCntr * OK_slow_param) > work->nLifeMax){
	GV_DestroyActor( work);
    }

#ifdef CALC_ACT_TIME
    time = GV_GET_PRFC_CLOCK();
    DbgDumpExecTime( time);
#endif
}
// 破棄関数
static void Die( Work* work )
{
    work->prim  = MAO_FreePrim2( work->prim );
    if ( work->vecPos != NULL) 		GV_Free( work->vecPos);
    if ( work->vecDir != NULL) 		GV_Free( work->vecDir);
    if ( work->nLife  != NULL) 		GV_Free( work->nLife);
    if ( work->fSpeed != NULL) 		GV_Free( work->fSpeed);
    if ( work->fSpeedAdd != NULL) 	GV_Free( work->fSpeedAdd);
    if ( work->fSize  != NULL) 		GV_Free( work->fSize);
    if ( work->fSizeAdd  != NULL) 	GV_Free( work->fSizeAdd);
    if ( work->nR  != NULL) 		GV_Free( work->nR);
    if ( work->nG  != NULL) 		GV_Free( work->nG);
    if ( work->nB  != NULL) 		GV_Free( work->nB);
    if ( work->nA  != NULL) 		GV_Free( work->nA);
    if ( work->nDecR  != NULL) 		GV_Free( work->nDecR);
    if ( work->nDecG  != NULL) 		GV_Free( work->nDecG);
    if ( work->nDecB  != NULL) 		GV_Free( work->nDecB);
    if ( work->nDecA  != NULL) 		GV_Free( work->nDecA);
    if ( work->nRadAng  != NULL) 	GV_Free( work->nRadAng);
}

// パケット初期化
static int InitPacket( DG_PRIM2* 	prim, 		// プリミティブワーク
		       DG_TEX* 		tex, 		// テクスチャタグ
		       long64		tagAlpha,	// ALPHAタグ
		       int		nRGBA,		// RGBA値
		       FVECTOR* 	pvecPos, 	// 位置データ列へのポインタ
		       float*		pfSize,		// サイズデータ列へのポインタ
		       int*		pnRadAng,	// 回転データ列へのポインタ
		       int		nRaise,		// 優先値
		       int		nRandSeed,	// ランダムシード
		       int		nPrimNum,	// プリミティブ数
		       int		nVertNum)	// 頂点数
{	
    u_short		r,g,b,a;
    int			i, j;
    FVECTOR*		pvecVert1;
    FVECTOR*		pvecVert2;
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    // RGBA値
    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    pvecVert1 = prim->pos[0];
    pvecVert2 = prim->pos[1];
    uvrgbwh1  = prim->uvrgb[0];
    uvrgbwh2  = prim->uvrgb[1];
    for ( i = 0; i < nPrimNum; i++){
	for ( j = 0; j < nVertNum; j++){
	    _sceVu0CopyVector( pvecVert1, pvecPos);
	    _sceVu0CopyVector( pvecVert2, pvecPos);

	    uvrgbwh2->w  = uvrgbwh1->w  = (int)((*pfSize) * _TS_Sin( (*pnRadAng)) );
	    uvrgbwh2->h  = uvrgbwh1->h  = (int)((*pfSize) * _TS_Sin( (*pnRadAng) + 1024) );

	    uvrgbwh2->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );/* 左上 */
	    uvrgbwh2->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );/* 左上 */
	    uvrgbwh2->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );/* 右下 */
	    uvrgbwh2->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );/* 右下 */
	    uvrgbwh2->q0 = uvrgbwh1->q0 = 4096;
	    uvrgbwh2->q1 = uvrgbwh1->q1 = 4096;
	    uvrgbwh2->f0 = uvrgbwh1->f0 = 0x0fff;
	    uvrgbwh2->f1 = uvrgbwh1->f1 = 0x0fff;

	    uvrgbwh2->r  = uvrgbwh1->r  = r;
	    uvrgbwh2->g  = uvrgbwh1->g  = g;
	    uvrgbwh2->b  = uvrgbwh1->b  = b;
	    uvrgbwh2->a  = uvrgbwh1->a  = a;

	    pvecVert1++;
	    pvecVert2++;
	    uvrgbwh1++;
	    uvrgbwh2++;
	    pvecPos++;
	    pnRadAng++;
	}
    }

    return 1;
}

// リソース初期化
static int GetResources( Work*		work, 		// ワーク
			 int		nRandSeed,	// ランダムシード
			 FMATRIX*	pmatOrg,	// 発生座標系
			 int		nPrimNum,	// 発生数
			 int		nVertNum,	// 頂点数
			 int		nLifeMin,	// 生存時間最小
			 int		nLifeMax,	// 生存時間最大
			 int		nAng,		// 角度範囲
			 float		fInitSpd,	// 初期速度
			 float		fAimSpd,	// 目標速度
			 float		fGravity,	// 重力
			 float		fInitSize,	// 初期サイズ
			 float		fAimSize,	// 目標サイズ
			 int		texcode,	// テクスチャコード
			 int		nInitRGBA,	// 初期RGBA
			 int		nAimRGBA,	// 目標RGBA
			 float		fRandRate,	// ランダム幅
			 int		nFlag)		// 拡張フラグ
{
    int 	i,j;
    long64	tagAlpha;
    DG_TEX*	tex;
    FVECTOR*	pvecPos;
    FVECTOR*	pvecDir;
    float*	pfSpeed;
    float*	pfSpeedAdd;
    float*	pfSize;
    float*	pfSizeAdd;
    int*	pnLife;
    u_short*	pnR;
    u_short*	pnG;
    u_short*	pnB;
    u_short*	pnA;
    short*	pnDecR;
    short*	pnDecG;
    short*	pnDecB;
    short*	pnDecA;
    int*	pnRadAng;

    // 引数取得
    work->nRandSeed	= nRandSeed;
    _sceVu0CopyMatrix( &work->matOrg, pmatOrg);
    work->nPrimNum	= (( nPrimNum > N_PRIMS) ? N_PRIMS : nPrimNum );
    work->nVertNum	= (( nVertNum > N_VERTS) ? N_VERTS : nVertNum );
    work->nLifeMin 	= COUNT_VMODE( nLifeMin);
    work->nLifeMax 	= COUNT_VMODE( nLifeMax);
    work->nAng	   	= nAng;
    work->fInitSpd 	= STEP_VMODEF( fInitSpd);
    work->fAimSpd  	= STEP_VMODEF( fAimSpd);
    work->fGravity 	= fGravity;
    work->fInitSize	= fInitSize;
    work->fAimSize	= fAimSize;
    work->texcode	= texcode;
    work->nInitRGBA	= nInitRGBA;
    work->nAimRGBA	= nAimRGBA;
    work->fRandRate	= fRandRate;
    work->nFlag    	= nFlag;
// #ifdef JAPANESE_BP_IGNORE()
#if 1 
    if ( work->nFlag & EXFLAG_BOOLD){
	work->nInitRGBA	= (nInitRGBA & 0xffffff00) + ((nInitRGBA & 0x000000ff) / 2);
	work->nAimRGBA	= (nAimRGBA & 0xffffff00) + ((nAimRGBA & 0x000000ff) / 2);
    }
#endif
    {
	// 拡張フラグを解析	
	if ( work->nFlag & EXFLAG_NORM_ALP)	  tagAlpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ); // 半透明
	else if ( work->nFlag & EXFLAG_ADD_ALP)   tagAlpha = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ); // 加算半透明
	else if ( work->nFlag & EXFLAG_SUB_ALP)   tagAlpha = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ); // 減算半透明
	else					  tagAlpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ); // 半透明
    }

    work->nCntr = 0;


    // 位置ワーク
    if( ( work->vecPos = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecPos, sizeof(FVECTOR) * work->nPrimNum * work->nVertNum );

    // 方向ワーク
    if( ( work->vecDir = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecDir, sizeof(FVECTOR) * work->nPrimNum * work->nVertNum );

    // 速度ワーク
    if( ( work->fSpeed = (float *)GV_Malloc(sizeof(float) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fSpeed, sizeof(float) * work->nPrimNum * work->nVertNum );

    // 速度変化量ワーク
    if( ( work->fSpeedAdd = (float *)GV_Malloc(sizeof(float) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fSpeedAdd, sizeof(float) * work->nPrimNum * work->nVertNum );

    // サイズワーク
    if( ( work->fSize = (float *)GV_Malloc(sizeof(float) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fSize, sizeof(float) * work->nPrimNum * work->nVertNum );

    // サイズ変化量ワーク
    if( ( work->fSizeAdd = (float *)GV_Malloc(sizeof(float) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fSizeAdd, sizeof(float) * work->nPrimNum * work->nVertNum );

    // 生存時間ワーク
    if( ( work->nLife = (int *)GV_Malloc(sizeof(int) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nLife, sizeof(int) * work->nPrimNum * work->nVertNum );

    // カラーワーク
    if( ( work->nR = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nR, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    if( ( work->nG = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nG, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    if( ( work->nB = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nB, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    if( ( work->nA = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nA, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    // 色減衰ワーク
    if( ( work->nDecR = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nDecR, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    if( ( work->nDecG = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nDecG, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    if( ( work->nDecB = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nDecB, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    if( ( work->nDecA = (u_short *)GV_Malloc(sizeof(u_short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nDecA, sizeof(u_short) * work->nPrimNum * work->nVertNum );

    // 回転角度ワーク
    if( ( work->nRadAng = (int *)GV_Malloc(sizeof(int) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nRadAng, sizeof(int) * work->nPrimNum * work->nVertNum );

    pvecPos 	= work->vecPos;
    pvecDir 	= work->vecDir;
    pnLife  	= work->nLife;
    pfSpeed 	= work->fSpeed;
    pfSpeedAdd 	= work->fSpeedAdd;
    pfSize  	= work->fSize;
    pfSizeAdd  	= work->fSizeAdd;
    pnR  	= work->nR;
    pnG  	= work->nG;
    pnB  	= work->nB;
    pnA  	= work->nA;
    pnDecR  	= work->nDecR;
    pnDecG  	= work->nDecG;
    pnDecB  	= work->nDecB;
    pnDecA  	= work->nDecA;
    pnRadAng	= work->nRadAng;
    

    for ( i = 0; i < work->nPrimNum; i++){
	FMATRIX mat;
	SVECTOR vecRot;
	for ( j = 0; j < work->nVertNum; j++){
	    _sceVu0CopyVector( pvecPos, (FVECTOR*)&work->matOrg.m[3][0]);
	    DG_SetPos( &work->matOrg );
#if 0
	    vecRot.vx = 0;
	    vecRot.vy = (int)( GM_FRnd( &work->nRandSeed) * (float)work->nAng);
	    vecRot.vz = GM_IRnd( &work->nRandSeed ) % 4096;
#else
	    vecRot.vx = (int)( GM_FRnd( &work->nRandSeed) * (float)work->nAng);
	    vecRot.vy = (int)( GM_FRnd( &work->nRandSeed) * (float)work->nAng);
	    vecRot.vz = 0;
#endif
	    DG_RotatePos( &vecRot);
	    DG_GetPos( &mat);
	    _sceVu0CopyVector( pvecDir, (FVECTOR*)&mat.m[2][0]);

	    if ( work->nLifeMax - work->nLifeMin != 0){
		(*pnLife)  = work->nLifeMin + 
		    (int)((float)MPART_DemoRand( work->nRandSeed, work->nLifeMax - work->nLifeMin) * work->fRandRate);
	    }else{
		(*pnLife)  = work->nLifeMin;
	    }

	    (*pfSpeed)    = work->fInitSpd + GM_Rnd( &work->nRandSeed) * 
		( work->fAimSpd - work->fInitSpd ) * work->fRandRate; 
	    (*pfSpeedAdd) = (work->fAimSpd - (*pfSpeed)) / (float)(*pnLife);
	    (*pfSize)  	  = work->fInitSize;
	    // + GM_Rnd( &work->nRandSeed) * ( work->fAimSize - work->fInitSize ) * work->fRandRate; 
	    (*pfSizeAdd) = (work->fAimSize - (*pfSize)) / (float)(*pnLife);
	    (*pnR)  	  = ((work->nInitRGBA & 0xff000000) >> 24) * 256;
	    (*pnG)  	  = ((work->nInitRGBA & 0x00ff0000) >> 16) * 256;
	    (*pnB)  	  = ((work->nInitRGBA & 0x0000ff00) >> 8)  * 256;
	    (*pnA)  	  =  (work->nInitRGBA & 0x000000ff)        * 256;
	    (*pnDecR) 	  = (((work->nAimRGBA  & 0xff000000) >> 24) - 
			     ((work->nInitRGBA & 0xff000000) >> 24)) * 256 / (*pnLife);
	    (*pnDecG)	  = (((work->nAimRGBA  & 0x00ff0000) >> 16) - 
			     ((work->nInitRGBA & 0x00ff0000) >> 16)) * 256 / (*pnLife);
	    (*pnDecB)	  = (((work->nAimRGBA  & 0x0000ff00) >> 8)  - 
			     ((work->nInitRGBA & 0x0000ff00) >> 8)) * 256  / (*pnLife);
	    (*pnDecA) 	  = ((work->nAimRGBA   & 0x000000ff)         - 
			     (work->nInitRGBA  & 0x000000ff)) * 256  / (*pnLife);
	    (*pnRadAng)	  = GM_IRnd( &work->nRandSeed ) & 4095;

	    pvecPos++;
	    pvecDir++;
	    pnLife++;
	    pfSpeed++;
	    pfSpeedAdd++;
	    pfSize++;
	    pfSizeAdd++;
	    pnR++;
	    pnG++;
	    pnB++;
	    pnA++;
	    pnDecR++;
	    pnDecG++;
	    pnDecB++;
	    pnDecA++;
	    pnRadAng++;
	}
    }

    // 描画用ワーク初期化
    // プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->nPrimNum, work->nVertNum);
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
    tex = DG_GetTexture( work->texcode );
    // パケットの初期化
    InitPacket( work->prim, tex, tagAlpha, work->nInitRGBA, work->vecPos, work->fSize, 
		work->nRadAng, RAISE, work->nRandSeed, work->nPrimNum, work->nVertNum);

    return 0;
}

// 汎用パーティクル
void* NewMAOParticle( int	nRandSeed,	// ランダムシード
		      FMATRIX*	pmatOrg,	// 発生座標系
		      int	nPrimNum,	// 発生数
		      int	nVertNum,	// 頂点数
		      int	nLifeMin,	// 生存時間最小
		      int	nLifeMax,	// 生存時間最大
		      int	nAng,		// 角度範囲
		      float	fInitSpd,	// 初期速度
		      float	fAimSpd,	// 目標速度
		      float	fGravity,	// 重力
		      float	fInitSize,	// 初期サイズ
		      float	fAimSize,	// 目標サイズ
		      int	texcode,	// テクスチャコード
		      int	nInitRGBA,	// 初期RGBA
		      int	nAimRGBA,	// 目標RGBA
		      float	fRandRate,	// ランダム幅
		      int	nFlag)		// 拡張フラグ
{
    Work*	work;

    OPERATOR();

    // 血チェック
    if ( (nFlag & EXFLAG_BOOLD) && ( GM_Configuration & GM_CONFIG_BLOOD_OFF )){
	return NULL;
    }

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, nRandSeed, pmatOrg, nPrimNum, nVertNum,nLifeMin, nLifeMax, 
			   nAng, fInitSpd, fAimSpd, fGravity, fInitSize, fAimSize, texcode, 
			   nInitRGBA, nAimRGBA, fRandRate, nFlag ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}

