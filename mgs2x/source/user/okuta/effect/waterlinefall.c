//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterlinefall.c
    送水管から流れ落ちる水
    2001/07/24 Masafumi Okuta
    $Id: waterlinefall.c,v 1.1.1.3 2002/11/19 11:47:54 Yoshizawa1 Exp $
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

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))


#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

// RGBA設定
#define COL_R		(244)
#define COL_G		(255)
#define COL_B		(175)
#define ALPHA		(32)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UV  	(SCRPAD_ADDR + 0x2000)

// プリミティブ,頂点数
#define N_PRIMS		(1)
#define N_VERTS_RING	(31)

#define BODY_VERTS	(64)

#define BODY_HEIGHT	(800.f)	
#define POLE_HEIGHT	(800.f)	

extern void TS_VecToRot( SVECTOR *rot, FVECTOR *vec );
extern float _TS_Sin( int s );

extern float 	GM_WaterLevel;
extern FVECTOR 	G_wind;
extern int 	G_wind_intense;
extern int 	G_wind_intense_max;
extern int  	DG_TickCount;

#if 0
#define BODY_R (0x30)
#define BODY_G (0x20)
#define BODY_B (0x10)
#define BODY_A (0x25)
#define POLE_R (0x20)
#define POLE_G (0x20)
#define POLE_B (0x30)
#define POLE_A (0x30)
#else
#define BODY_R (0x40)
#define BODY_G (0x40)
#define BODY_B (0x40)
#define BODY_A (0x30)
#define POLE_R (0x30)
#define POLE_G (0x30)
#define POLE_B (0x30)
#define POLE_A (0x30)
#endif

#define BODY_RAD (610.f)
#define POLE_RAD (600.f)

#define TBL_SE_MAX (4)
static int TBL_SE_INTV[TBL_SE_MAX] = { 8, 11, 16, 18};

enum{ // メッセージ
WATER_MESSAGE_ACTIVE,	// アクティブモード
WATER_MESSAGE_SLEEP,	// スリープモード
};

typedef struct _WATERFALL {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;
    int			map;				// マップID
    GV_MSG*		msg;				// 自分宛メッセージ	
    
    // 拡張管理
    int			nCntr;				// カウンタ	
    int			nSleep;				// スリープモード
    int			nPrimNum;			// プリミティブ数
    int			nVertNum;			// 頂点数
    int			nRingNum;			// 輪っか本体数
    FMATRIX		matOrg;				// 発生地点
    FVECTOR		vecInitOffset;			// 初期オフセット
    SVECTOR		vecRandAng;			// ランダム拡散角度
    float		fInitSpd;			// 初期速度
    float		fAimSpd;			// 目標速度
    float		fInitSize;			// 初期サイズ
    float		fAimSize;			// 目標サイズ
    float		fGravity;			// 重力
    int			nRGBA;				// 色
    float		fFallY;				// 高さ	
    int			nFlag;				// フラグ
    u_char		ucFallSe;			// 落下水音フラグ
    u_char		ucPaddd;			// パディング
    int			nFallSeTime;			// 最後に流水音をコールした時間
    int			nFallSeIntv;

    // 飛抹
    FVECTOR*		vecDir;
    float*		fSpeed;
    float*		fSize;
    short*		nDelay;

    // 水流本体
    FVECTOR*		vecBodyPos;
    FVECTOR*		vecBodyDir;
    float*		fBodySpeed;
    float*		fBodySize;
    short*		nBodyDelay;

    // 描画用
    DG_PRIM2*		prim;		
    DG_PRIM2*		prim_body;		
    DG_PRIM2*		prim_pole;		

    // 飛び出し口飛抹
    FMATRIX		matSpread;
    float		fSpreadMin;
    float		fSpreadMax;
} Work;

static void CheckMessage( Work* work )
{
    GV_MSG	*msg ;
    int n_msg, code ;

    n_msg = GV_ReceiveMessage( work->name, &work->msg ); // メッセージ取得
    msg = work->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case WATER_MESSAGE_ACTIVE:	// アクティブモード
	    work->nSleep = 0;
	    break;
	case WATER_MESSAGE_SLEEP:	// スリープモード
	    work->nSleep = 1;
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

static void ResetPos( Work* 		work, 
		      FVECTOR*  	pvecPos,
		      FVECTOR*  	pvecDir,
		      float*		pfSpeed,
		      float*		pfSize,
		      DG_PRIM2_UVRGBWH*	uvrgbwh)
{
    FMATRIX 	mat;
    FVECTOR	vecOffset;
    SVECTOR 	vecRot;
    float fChgSpdRate  = work->fAimSpd - work->fInitSpd;

    DG_SetPos( &work->matOrg );
    vecRot.vx = (int)( frnd() * (float)work->vecRandAng.vx);
    vecRot.vy = (int)( frnd() * (float)work->vecRandAng.vy);
    vecRot.vz = (int)( frnd() * (float)work->vecRandAng.vz);
    DG_RotatePos( &vecRot);
    vecOffset.vx = frnd() * work->vecInitOffset.vx;
    vecOffset.vy = frnd() * work->vecInitOffset.vy;
    vecOffset.vz = frnd() * work->vecInitOffset.vz;
    vecOffset.vw = 1.f;
    DG_MovePos( &vecOffset);
    DG_GetPos( &mat);
    _sceVu0CopyVector( pvecPos, (FVECTOR*)&mat.m[3][0]);
    _sceVu0CopyVector( pvecDir, (FVECTOR*)&mat.m[2][0]);

    (*pfSpeed) = work->fInitSpd + rnd() * fChgSpdRate;
    (*pfSize)  = work->fInitSize;
    uvrgbwh->w = uvrgbwh->h = (*pfSize);
}

static void ResetBodyPos( Work* 	work, 
			  FVECTOR*  	pvecPos,
			  FVECTOR*  	pvecDir,
			  float*	pfSpeed,
			  float*	pfSize)
{
    FMATRIX 	mat;

    float fChgSpdRate  = work->fAimSpd - work->fInitSpd;

    DG_SetPos( &work->matOrg );
    DG_GetPos( &mat);
    _sceVu0CopyVector( pvecPos, (FVECTOR*)&mat.m[3][0]);
    _sceVu0CopyVector( pvecDir, (FVECTOR*)&mat.m[2][0]);

    (*pfSpeed) = work->fInitSpd + 0.5f * fChgSpdRate;
    (*pfSize)  = 20.f;
}

static void ActRingBody( Work* 		work, 
			 int		nNum,
			 DG_PRIM2*	prim_add,
			 DG_PRIM2*	prim_sub,
			 FVECTOR*	pvecPos,
			 FVECTOR*	pvecDir,
			 float*		pfSpeed,
			 float*		pfSize,
			 short*		pnDelay,
			 float		fRad,
			 float		fHeight)
{
    int			i, j;

    SVECTOR		vecRot;
    FVECTOR*		pvecVert;
    FVECTOR*		pvecTmp;
    FVECTOR*		pvecMemAdd;
    FVECTOR*		pvecMemSub;
    DG_PRIM2_UVRGB*	uvrgbAdd;
    DG_PRIM2_UVRGB*	uvrgbSub;
    DG_PRIM2_UVRGB*	uvrgbAddPre;
    DG_PRIM2_UVRGB*	uvrgbSubPre;

    pvecMemAdd = prim_add->pos[prim_add->buffer_clock];
    pvecMemSub = prim_sub->pos[prim_sub->buffer_clock];
    uvrgbAdd   = prim_add->uvrgb[prim_add->buffer_clock];
    uvrgbSub   = prim_sub->uvrgb[prim_sub->buffer_clock];
    uvrgbAddPre   = prim_add->uvrgb[(prim_add->buffer_clock+1)%2];
    uvrgbSubPre   = prim_sub->uvrgb[(prim_add->buffer_clock+1)%2];

    prim_sub->flag |= DG_PRIM_AS_CUSTOMWORLD;
    prim_sub->as_world.m[3][0] = 5.f; // 疑似バンプ

    
    for ( i = 0; i < nNum; i++){
	FVECTOR vec;

	pvecTmp  = SCR_POS;
	pvecVert = SCR_POS;

	if ( (*pnDelay) > 0){
	    (*pnDelay)--;
	}else{
	    (*pfSize) = (*pfSize) * 0.98f + fRad * 0.02f;

	    for ( j = 0; j < N_VERTS_RING; j++){
		int nAng = (4096 * j / N_VERTS_RING);
		float fX, fY;
		fX = (*pfSize) * _TS_Sin( nAng );
		fY = (*pfSize) * _TS_Sin( nAng + 1024 );
		pvecVert->vx = fX;
		pvecVert->vy = fY;
		pvecVert->vz = -fHeight;
		pvecVert++;
		
		pvecVert->vx = fX;
		pvecVert->vy = fY;
		pvecVert->vz = fHeight;
		pvecVert++;
	    }
	    pvecVert->vx = 0.f;
	    pvecVert->vy = (*pfSize);
	    pvecVert->vz = -fHeight;
	    pvecVert++;

	    pvecVert->vx = 0.f;
	    pvecVert->vy = (*pfSize);
	    pvecVert->vz = fHeight;
	    pvecVert++;

	    // 移動ベクトル生成
	    _sceVu0ScaleVector( &vec, pvecDir, (*pfSpeed) );
	    // 位置更新
	    _sceVu0AddVector( pvecPos, pvecPos, &vec);
	    // 重力反映
	    pvecDir->vy -= work->fGravity;
	    _sceVu0Normalize( pvecDir, pvecDir);

	    // マトリクス生成
	    TS_VecToRot( &vecRot, pvecDir);

	    vecRot.vz = i * 4096 / nNum;
	    DG_SetPos2( pvecPos, &vecRot);
	    DG_PutVector( pvecTmp, pvecTmp, 64);

	    Mao_CopyScrToMem( pvecMemAdd, pvecTmp, sizeof(FVECTOR), 64 );
	    Mao_CopyScrToMem( pvecMemSub, pvecTmp, sizeof(FVECTOR), 64 );

	    if ( (pvecPos->vy - (2000.f + i * 5.f) ) < work->fFallY){
		for ( j = 0; j < BODY_VERTS; j++){
		    if ( uvrgbAddPre->a >= 2) uvrgbAdd->a = uvrgbAddPre->a - 2;
		    else		      uvrgbAdd->a = 0;
		    if ( uvrgbSubPre->a >= 2) uvrgbSub->a = uvrgbSubPre->a - 2;
		    else		      uvrgbSub->a = 0;
		    uvrgbAdd++;
		    uvrgbSub++;
		    uvrgbAddPre++;
		    uvrgbSubPre++;
		}
	    }else{
		for ( j = 0; j < BODY_VERTS; j++){
		    uvrgbAdd->a = BODY_A;
		    uvrgbSub->a = POLE_A;
		    uvrgbAdd++;
		    uvrgbSub++;
		    uvrgbAddPre++;
		    uvrgbSubPre++;
		}
	    }

	    if ( pvecPos->vy < work->fFallY ){ // 一定の高さまでいったらリセット
		ResetBodyPos( work, pvecPos, pvecDir, pfSpeed, pfSize);
	    }
	}

	pvecTmp    += 64;
	pvecMemAdd += 64;
	pvecMemSub += 64;

	pvecPos++;
	pvecDir++;
	pfSpeed++;
	pfSize++;
	pnDelay++;
    }
}

// 動作関数
static void Act( Work* work )
{
    int 		i,j;	
//    FVECTOR*		pvecPos;
    FVECTOR*		pvecDir;
    float*		pfSpeed;
    float*		pfSize;	
    short*		pnDelay;
    FVECTOR*		pvecPrim;
    FVECTOR*		pvecPrimPre;
    DG_PRIM2_UVRGBWH*	uvrgbwh;
    DG_PRIM2_UVRGBWH*	uvrgbwhPre;
    u_char		a = (work->nRGBA & 0x000000ff);

#if 0
GV_SET_PRFC_CLOCK();
#endif

    // メッセージチェック
    CheckMessage( work);

    // スリープモードチェック
    if ( work->nSleep ) return;

    GM_GroupPrim2( work->prim, work->map );
    GM_GroupPrim2( work->prim_body, work->map );
    GM_GroupPrim2( work->prim_pole, work->map );

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    DG_SwitchBuffPrim2( work->prim_body );
    DG_SwitchBuffPrim2( work->prim_pole );


    pvecDir 	= work->vecDir;
    pfSpeed	= work->fSpeed;
    pfSize  	= work->fSize;
    pnDelay 	= work->nDelay;

    pvecPrim    = work->prim->pos[work->prim->buffer_clock];
    pvecPrimPre = work->prim->pos[(work->prim->buffer_clock+1) % 2];
    uvrgbwh  	= work->prim->uvrgb[work->prim->buffer_clock];
    uvrgbwhPre  = work->prim->uvrgb[(work->prim->buffer_clock+1) % 2];
    for ( i = 0; i < work->nPrimNum; i++){
	for ( j = 0; j < work->nVertNum; j++){
	    FVECTOR vec;

	    if ( (*pnDelay) > 0 ) {
		(*pnDelay)--;
	    }else{
		_sceVu0ScaleVector( &vec, pvecDir, (*pfSpeed) );
		_sceVu0AddVector( pvecPrim, pvecPrimPre, &vec);
		pvecDir->vy -= work->fGravity;
		_sceVu0Normalize( pvecDir, pvecDir);

		(*pfSize) = (*pfSize) * 0.99f + work->fAimSize * 0.01f;

		uvrgbwh->w = (*pfSize);
		uvrgbwh->h = (*pfSize);

		if ( pvecPrim->vy - 3000.f < work->fFallY){
		    {
			FVECTOR vecWind;
			float fWind = ( 1.f - (pvecPrim->vy - work->fFallY) / 3000.f) * 0.015f;
			_sceVu0ScaleVector( &vecWind, &G_wind, fWind);
			_sceVu0AddVector( pvecPrim, pvecPrim, &vecWind);
		    }
		}

		if ( (pvecPrim->vy - 1500 + j * 5) < work->fFallY && uvrgbwhPre->a > 0){
		    uvrgbwh->a = uvrgbwhPre->a - 1;
		} else{
		    uvrgbwh->a = a;
		}

		if ( pvecPrim->vy < work->fFallY ){
		    ResetPos( work, pvecPrim, pvecDir, pfSpeed, pfSize, uvrgbwh);
		}
	    }

	    pvecDir++;
	    pfSpeed++;
	    pfSize++;
	    pnDelay++;

	    pvecPrim++;
	    pvecPrimPre++;
	    uvrgbwh++;
	    uvrgbwhPre++;
	}
    }

    ActRingBody(work, work->nRingNum, work->prim_body, work->prim_pole, work->vecBodyPos, work->vecBodyDir, 
		work->fBodySpeed, work->fBodySize, work->nBodyDelay, BODY_RAD, BODY_HEIGHT);

    // 水流の音
    if ( abs( DG_TickCount - work->nFallSeTime) >= work->nFallSeIntv){
	if ( work->ucFallSe ) GM_SeSetMode( SD_A_PIPEWTR1, (FVECTOR*)&work->matOrg.m[3][0], GM_SEMODE_NORMAL); 
	else 		      GM_SeSetMode( SD_A_PIPEWTR2, (FVECTOR*)&work->matOrg.m[3][0], GM_SEMODE_NORMAL); 
	work->ucFallSe = (~work->ucFallSe & 1);
	work->nFallSeTime = DG_TickCount;
	work->nFallSeIntv = COUNT_VMODE( TBL_SE_INTV[ MAO_Rand(TBL_SE_MAX)]);
    }

    work->nCntr++;
}

// 破棄関数
static void Die( Work* work )
{
    work->prim       = MAO_FreePrim2( work->prim );
    work->prim_body  = MAO_FreePrim2( work->prim_body );
    work->prim_pole  = MAO_FreePrim2( work->prim_pole );

    if ( work->vecDir != NULL) 		GV_Free( work->vecDir);
    if ( work->fSpeed != NULL) 		GV_Free( work->fSpeed);
    if ( work->fSize  != NULL) 		GV_Free( work->fSize);
    if ( work->nDelay != NULL) 		GV_Free( work->nDelay);

    if ( work->vecBodyPos != NULL) 	GV_Free( work->vecBodyPos);
    if ( work->vecBodyDir != NULL) 	GV_Free( work->vecBodyDir);
    if ( work->fBodySpeed != NULL) 	GV_Free( work->fBodySpeed);
    if ( work->fBodySize  != NULL) 	GV_Free( work->fBodySize);
    if ( work->nBodyDelay != NULL) 	GV_Free( work->nBodyDelay);

}

// パケット初期化
static int InitPacket( Work* 		work, 	// 
		       DG_PRIM2*	prim,	
		       DG_TEX* 		tex) 	// テクスチャタグ
{	
    u_short		r,g,b,a;
    int			i, j;
    float*		pfSize;
    FVECTOR*		pvecVert1;
    FVECTOR*		pvecVert2;
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;

    // 優先設定
    prim->raise = 3000;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );	// α設定

    // RGBA値
    r = ((work->nRGBA & 0xff000000) >> 24);
    g = ((work->nRGBA & 0x00ff0000) >> 16);
    b = ((work->nRGBA & 0x0000ff00) >> 8);
    a = (work->nRGBA & 0x000000ff);

    pfSize    = work->fSize;
    pvecVert1 = prim->pos[0];
    pvecVert2 = prim->pos[1];
    uvrgbwh1  = prim->uvrgb[0];
    uvrgbwh2  = prim->uvrgb[1];
    for ( i = 0; i < work->nPrimNum; i++){
	for ( j = 0; j < work->nVertNum; j++){
	    extern float _TS_Sin( int s );
	    _sceVu0CopyVector( pvecVert1, (FVECTOR*)&work->matOrg.m[3][0]);
	    _sceVu0CopyVector( pvecVert2, (FVECTOR*)&work->matOrg.m[3][0]);

	    uvrgbwh2->w  = uvrgbwh1->w  = (int)((*pfSize) );
	    uvrgbwh2->h  = uvrgbwh1->h  = (int)((*pfSize) );

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
	    pfSize++;
	}
    }

    return 1;
}

// パケット初期化
static int InitPacketBody( Work* 	work, 		// 
			   DG_PRIM2*	prim_add,	// 加算ワーク
			   DG_PRIM2*	prim_sub,	// 減算ワーク
			   DG_TEX* 	tex) 		// テクスチャタグ
{	
    int			i, j;
    float*		pfSize;
    FVECTOR*		pvecVert;
    FVECTOR*		pvecTmp;
    FVECTOR*		pvecMemAdd1;
    FVECTOR*		pvecMemAdd2;
    DG_PRIM2_UVRGB*	uvrgbAdd1;
    DG_PRIM2_UVRGB*	uvrgbAdd2;
    FVECTOR*		pvecMemSub1;
    FVECTOR*		pvecMemSub2;
    DG_PRIM2_UVRGB*	uvrgbSub1;
    DG_PRIM2_UVRGB*	uvrgbSub2;
    extern float _TS_Sin( int s );

    // 優先設定
    prim_add->raise = 0;
    prim_sub->raise = 1000;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim_add, tex );	// プリミティブにテクスチャを設定
    DG_ConfigPrim2Tex( prim_sub, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim_add, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );	// α設定
    DG_SetPrim2Alpha( prim_sub, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );	// α設定

    pfSize  = work->fBodySize;
    pvecMemAdd1 = prim_add->pos[0];
    pvecMemAdd2 = prim_add->pos[1];
    uvrgbAdd1   = prim_add->uvrgb[0];
    uvrgbAdd2   = prim_add->uvrgb[1];
    pvecMemSub1 = prim_sub->pos[0];
    pvecMemSub2 = prim_sub->pos[1];
    uvrgbSub1   = prim_sub->uvrgb[0];
    uvrgbSub2   = prim_sub->uvrgb[1];

    for ( i = 0; i < work->nRingNum; i++){
	pvecTmp  = SCR_POS;
	pvecVert = SCR_POS;
	for ( j = 0; j < N_VERTS_RING; j++){
	    int	  nAng = 4096 * j / N_VERTS_RING;
	    float fX = (*pfSize) * _TS_Sin( nAng );
	    float fY = (*pfSize) * _TS_Sin( nAng + 1024 );
	    float fU = (_TS_Sin( nAng ) + 1.f) * 0.5f;
	    pvecVert->vx = fX;
	    pvecVert->vy = fY;
	    pvecVert->vz = -POLE_HEIGHT;
	    pvecVert++;

	    uvrgbSub2->u = uvrgbSub1->u = uvrgbAdd2->u = uvrgbAdd1->u = FTOI12( fU * tex->u_scale + tex->u_offset );
	    uvrgbSub2->v = uvrgbSub1->v = uvrgbAdd2->v = uvrgbAdd1->v = FTOI12( 0.f * tex->v_scale + tex->v_offset );
	    uvrgbAdd1++;
	    uvrgbAdd2++;
	    uvrgbSub1++;
	    uvrgbSub2++;

	    pvecVert->vx = fX;
	    pvecVert->vy = fY;
	    pvecVert->vz = POLE_HEIGHT;
	    pvecVert++;

	    uvrgbSub2->u = uvrgbSub1->u = uvrgbAdd2->u = uvrgbAdd1->u = FTOI12( fU * tex->u_scale + tex->u_offset );
	    uvrgbSub2->v = uvrgbSub1->v = uvrgbAdd2->v = uvrgbAdd1->v = FTOI12( 1.f * tex->v_scale + tex->v_offset );
	    uvrgbAdd1++;
	    uvrgbAdd2++;
	    uvrgbSub1++;
	    uvrgbSub2++;
	}
	pvecVert->vx = 0.f;
	pvecVert->vy = (*pfSize);
	pvecVert->vz = -POLE_HEIGHT;
	pvecVert++;
	uvrgbSub2->u = uvrgbSub1->u = uvrgbAdd2->u = uvrgbAdd1->u = FTOI12( 0.f * tex->u_scale + tex->u_offset );
	uvrgbSub2->v = uvrgbSub1->v = uvrgbAdd2->v = uvrgbAdd1->v = FTOI12( 0.f * tex->v_scale + tex->v_offset );
	uvrgbAdd1++;
	uvrgbAdd2++;
	uvrgbSub1++;
	uvrgbSub2++;

	pvecVert->vx = 0.f;
	pvecVert->vy = (*pfSize);
	pvecVert->vz = POLE_HEIGHT;
	pvecVert++;
	uvrgbSub2->u = uvrgbSub1->u = uvrgbAdd2->u = uvrgbAdd1->u = FTOI12( 0.f * tex->u_scale + tex->u_offset );
	uvrgbSub2->v = uvrgbSub1->v = uvrgbAdd2->v = uvrgbAdd1->v = FTOI12( 1.f * tex->v_scale + tex->v_offset );
	uvrgbAdd1++;
	uvrgbAdd2++;
	uvrgbSub1++;
	uvrgbSub2++;

	DG_SetPos( &work->matOrg);
	DG_PutVector( pvecTmp, pvecTmp, 64);

	Mao_CopyScrToMem( pvecMemAdd1, pvecTmp, sizeof(FVECTOR), 64 );
	Mao_CopyScrToMem( pvecMemAdd2, pvecTmp, sizeof(FVECTOR), 64 );
	Mao_CopyScrToMem( pvecMemSub1, pvecTmp, sizeof(FVECTOR), 64 );
	Mao_CopyScrToMem( pvecMemSub2, pvecTmp, sizeof(FVECTOR), 64 );

	pvecTmp     += 64;
	pvecMemAdd1 += 64;
	pvecMemAdd2 += 64;
	pvecMemSub1 += 64;
	pvecMemSub2 += 64;
    }

    uvrgbAdd1  = prim_add->uvrgb[0];
    uvrgbAdd2  = prim_add->uvrgb[1];
    uvrgbSub1  = prim_sub->uvrgb[0];
    uvrgbSub2  = prim_sub->uvrgb[1];
    for ( i = 0; i < work->nRingNum; i++){
	for ( j = 0; j < 64; j++){
	    uvrgbAdd2->q = uvrgbAdd1->q = 4096;
	    uvrgbAdd2->f = uvrgbAdd1->f = 0x0fff;

	    uvrgbAdd2->r  = uvrgbAdd1->r  = BODY_R;
	    uvrgbAdd2->g  = uvrgbAdd1->g  = BODY_G;
	    uvrgbAdd2->b  = uvrgbAdd1->b  = BODY_B;
	    uvrgbAdd2->a  = uvrgbAdd1->a  = BODY_A;

	    uvrgbAdd1++;
	    uvrgbAdd2++;


	    uvrgbSub2->q = uvrgbSub1->q = 4096;
	    uvrgbSub2->f = uvrgbSub1->f = 0x0fff;

	    uvrgbSub2->r  = uvrgbSub1->r  = POLE_R;
	    uvrgbSub2->g  = uvrgbSub1->g  = POLE_G;
	    uvrgbSub2->b  = uvrgbSub1->b  = POLE_B;
	    uvrgbSub2->a  = uvrgbSub1->a  = POLE_A;

	    uvrgbSub1++;
	    uvrgbSub2++;
	}
    }
    return 1;
}


// リソース初期化
static int GetResources( Work*	work,	// ワーク
			 int 	name, 	// 名前ID
			 int 	map) 	// マップID		
{
    int		i,j;
    char* 	opt;
    FVECTOR 	vecPos;
    SVECTOR 	vecRot;
    DG_TEX*	tex;
    extern int ENE_GCL_GetFV( char *ptr, FVECTOR *fvec );
    extern int GCL_GetNextIV( int *vec );

    work->name   = name;
    work->map    = map;

    work->nCntr  = 0;
    work->nSleep = 0;

    // GCL引数取得
    work->nPrimNum 	= GCL_GetOptionValue( 'u', 16 ); // プリミティブ数
    work->nVertNum 	= GCL_GetOptionValue( 'v', 16 ); // 頂点数
    work->nRingNum 	= GCL_GetOptionValue( 'n', 12 ); // 輪っか本体数

    if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){	// 座標取得
	if ( ENE_GCL_GetFV( opt, &vecPos ) != 0 ){
	    vecPos.vx = 0.f;
	    vecPos.vy = 0.f;
	    vecPos.vz = 0.f;
	    vecPos.vw = 1.f;
	}
    }else{
	vecPos.vx = 0.f;
	vecPos.vy = 0.f;
	vecPos.vz = 0.f;
	vecPos.vw = 1.f;
    }

    work->fFallY   = (float)GCL_GetOptionValue( 'f', (int)(vecPos.vy - 2500));


    if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){	// 回転情報取得
	if ( GCL_NextStr() != NULL ){
	    int vec[3];
	    GCL_GetNextIV( vec );
	    vecRot.vx = vec[0];
	    vecRot.vy = vec[1];
	    vecRot.vz = vec[2];
	}else{
	    vecRot.vx = 0;
	    vecRot.vy = 0;
	    vecRot.vz = 0;
	}
    }else{
	vecRot.vx = 0;
	vecRot.vy = 0;
	vecRot.vz = 0;
    }

    if ( ( opt = GCL_GetOption( 'o' ) ) != NULL ){	// 初期オフセット取得
	if ( ENE_GCL_GetFV( opt, &work->vecInitOffset ) != 0 ){
	    work->vecInitOffset.vx = 0.f;
	    work->vecInitOffset.vy = 0.f;
	    work->vecInitOffset.vz = 0.f;
	    work->vecInitOffset.vw = 1.f;
	}
    }else{
	work->vecInitOffset.vx = 0.f;
	work->vecInitOffset.vy = 0.f;
	work->vecInitOffset.vz = 0.f;
	work->vecInitOffset.vw = 1.f;
    }

    if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){	// 拡散角度取得
	if ( GCL_NextStr() != NULL ){
	    int vec[3];
	    GCL_GetNextIV( vec );
	    work->vecRandAng.vx = vec[0];
	    work->vecRandAng.vy = vec[1];
	    work->vecRandAng.vz = vec[2];
	}else{
	    work->vecRandAng.vx = 0;
	    work->vecRandAng.vy = 0;
	    work->vecRandAng.vz = 0;
	}
    }else{
	work->vecRandAng.vx = 0;
	work->vecRandAng.vy = 0;
	work->vecRandAng.vz = 0;
    }

    work->fInitSpd  = (float)GCL_GetOptionValue( 'i', 15 ); 		// 初期速度
    work->fAimSpd   = (float)GCL_GetOptionValue( 's', 45 ); 	  	// 目標速度
    work->fInitSize = (float)GCL_GetOptionValue( 'b', 15 ); 		// 初期サイズ
    work->fAimSize  = (float)GCL_GetOptionValue( 'd', 50 ); 	  	// 目標サイズ
    work->nRGBA	    =        GCL_GetOptionValue( 'c', 0x20202020 );  	// 色
    work->fGravity  = (float)GCL_GetOptionValue( 'g', 5 ) * 0.01f; 	// 重力
    // マトリクス生成
    DG_SetPos2( &vecPos, &vecRot);
    DG_GetPos( &work->matOrg);

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

    // サイズワーク
    if( ( work->fSize = (float *)GV_Malloc(sizeof(float) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fSize, sizeof(float) * work->nPrimNum * work->nVertNum );

    // 遅延ワーク
    if( ( work->nDelay = (short *)GV_Malloc(sizeof(short) * work->nPrimNum * work->nVertNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nDelay, sizeof(short) * work->nPrimNum * work->nVertNum );

    {
	FVECTOR* 	pvecDir = work->vecDir;
	float*   	pfSpeed = work->fSpeed;
	float*   	pfSize  = work->fSize;
	short*		pnDelay = work->nDelay;
	FMATRIX 	mat;
	FVECTOR		vecOffset;
	SVECTOR 	vecRot;
	float 		fChgSpdRate  = work->fAimSpd - work->fInitSpd;
	float 		fChgSizeRate = work->fAimSize - work->fInitSize;

	for ( i = 0; i < work->nPrimNum; i++){
	    for ( j = 0; j < work->nVertNum; j++){
		DG_SetPos( &work->matOrg );
		vecRot.vx = (int)( frnd() * (float)work->vecRandAng.vx);
		vecRot.vy = (int)( frnd() * (float)work->vecRandAng.vy);
		vecRot.vz = (int)( frnd() * (float)work->vecRandAng.vz);
		DG_RotatePos( &vecRot);
		vecOffset.vx = frnd() * work->vecInitOffset.vx;
		vecOffset.vy = frnd() * work->vecInitOffset.vy;
		vecOffset.vz = frnd() * work->vecInitOffset.vz;
		vecOffset.vw = 1.f;
		DG_MovePos( &vecOffset);
		DG_GetPos( &mat);
		_sceVu0CopyVector( pvecDir, (FVECTOR*)&mat.m[2][0]);

		(*pfSpeed) = work->fInitSpd + rnd() * fChgSpdRate;
		(*pfSize)  = work->fInitSize + rnd() * fChgSizeRate;
		(*pnDelay) = j * 10;

		pvecDir++;
		pfSpeed++;
		pfSize++;
		pnDelay++;
	    }
	}
    }

    // 位置ワーク
    if( ( work->vecBodyPos = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nRingNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecBodyPos, sizeof(FVECTOR) * work->nRingNum );

    // 方向ワーク
    if( ( work->vecBodyDir = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nRingNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecBodyDir, sizeof(FVECTOR) * work->nRingNum );

    // 速度ワーク
    if( ( work->fBodySpeed = (float *)GV_Malloc(sizeof(float) * work->nRingNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fBodySpeed, sizeof(float) * work->nRingNum );

    // サイズワーク
    if( ( work->fBodySize = (float *)GV_Malloc(sizeof(float) * work->nRingNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fBodySize, sizeof(float) * work->nRingNum );

    // サイズワーク
    if( ( work->nBodyDelay = (short *)GV_Malloc(sizeof(short) * work->nRingNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! MAO-Particle Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nBodyDelay, sizeof(short) * work->nRingNum );


    { // 本体
	FVECTOR* 	pvecPos = work->vecBodyPos;
	FVECTOR* 	pvecDir = work->vecBodyDir;
	float*   	pfSpeed = work->fBodySpeed;
	float*   	pfSize  = work->fBodySize;
	short*		pnDelay = work->nBodyDelay;
	float 		fChgSpdRate  = work->fAimSpd - work->fInitSpd;

	for ( i = 0; i < work->nRingNum; i++){
	    _sceVu0CopyVector( pvecPos, (FVECTOR*)&work->matOrg.m[3][0]);
	    _sceVu0CopyVector( pvecDir, (FVECTOR*)&work->matOrg.m[2][0]);
	    
	    (*pfSpeed) = work->fInitSpd + 0.6f * fChgSpdRate;
	    (*pfSize)  = 10.f;
	    
	    (*pnDelay) = i * 10;

	    pvecPos++;
	    pvecDir++;
	    pfSpeed++;
	    pfSize++;
	    pnDelay++;
	}
    }

    { // 芯
	FVECTOR* 	pvecPos = work->vecBodyPos;
	FVECTOR* 	pvecDir = work->vecBodyDir;
	float*   	pfSpeed = work->fBodySpeed;
	float*   	pfSize  = work->fBodySize;
	short*		pnDelay = work->nBodyDelay;
	float 		fChgSpdRate  = work->fAimSpd - work->fInitSpd;

	for ( i = 0; i < work->nRingNum; i++){
	    _sceVu0CopyVector( pvecPos, (FVECTOR*)&work->matOrg.m[3][0]);
	    _sceVu0CopyVector( pvecDir, (FVECTOR*)&work->matOrg.m[2][0]);
	    
	    (*pfSpeed) = work->fInitSpd + 0.6f * fChgSpdRate;
	    (*pfSize)  = 10.f;
	    
	    (*pnDelay) = i * 10;

	    pvecPos++;
	    pvecDir++;
	    pfSpeed++;
	    pfSize++;
	    pnDelay++;
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
    tex = DG_GetTexture( 8781729 );

    // パケットの初期化
    InitPacket( work, work->prim, tex);

    // ---------- 本体
    // プリミティブ本体の作成
    work->prim_body = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->nRingNum, BODY_VERTS);
    if ( work->prim_body == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }

    // テクスチャの設定
    tex = DG_GetTexture( 10972307);

    // ---------- 芯
    // プリミティブ本体の作成
    work->prim_pole = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->nRingNum, BODY_VERTS);
    if ( work->prim_pole == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }

    // パケットの初期化
    InitPacketBody( work, work->prim_body, work->prim_pole, tex);

    {
	FVECTOR vecSprPos;
	SVECTOR vecSprRot;

	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){	// 飛抹座標取得
	    if ( ENE_GCL_GetFV( opt, &vecSprPos ) != 0 ){
		vecSprPos.vx = 0.f;	
		vecSprPos.vy = 0.f;
		vecSprPos.vz = 0.f;
		vecSprPos.vw = 1.f;
	    }
	}else{
	    vecSprPos.vx = 0.f;
	    vecSprPos.vy = 0.f;
	    vecSprPos.vz = 0.f;
	    vecSprPos.vw = 1.f;
	}

	if ( ( opt = GCL_GetOption( 'h' ) ) != NULL ){	// 飛抹回転情報取得
	    if ( GCL_NextStr() != NULL ){
		int vec[3];
		GCL_GetNextIV( vec );
		vecSprRot.vx = vec[0];
		vecSprRot.vy = vec[1];
		vecSprRot.vz = vec[2];
	    }else{
		vecSprRot.vx = 0;
		vecSprRot.vy = 0;
		vecSprRot.vz = 0;
	    }
	}else{
	    vecSprRot.vx = 0;
	    vecSprRot.vy = 0;
	    vecSprRot.vz = 0;
	}

	DG_SetPos2( &vecSprPos, &vecSprRot);
	DG_GetPos( &work->matSpread);
    }


    if ( ( opt = GCL_GetOption( 'j' ) ) != NULL ){	// 飛抹半径情報取得
	GCL_SetArgTop( opt );
	work->fSpreadMin = (float)GCL_GetNextInt();
	if ( (opt = GCL_NextStr()) != NULL){
	    GCL_SetArgTop( opt );
	    work->fSpreadMax = (float)GCL_GetNextInt();
	    opt = GCL_NextStr();
	}else{
	    work->fSpreadMax = 0.f;
	}
    }else{
	work->fSpreadMin = 0.f;
	work->fSpreadMax = 0.f;
    }

    { // 飛び出し口飛抹
	extern void* NewWaterSpread( int, FMATRIX*, int, int, float, float, int, int, float, float, float, int,
				     float, float, float, float);
	GV_SetActorChild( work, NewWaterSpread( work->name, &work->matSpread, 48, 8, 
						work->fSpreadMin, work->fSpreadMax,
						20, 40, 150.f, 120.f, 0.5f, 0x80808040, 
						4.f, 8.f, 10.f, 10.f));
    }

    work->ucFallSe    = 0;
    work->nFallSeTime = DG_TickCount;
    work->nFallSeIntv = COUNT_VMODE( TBL_SE_INTV[ MAO_Rand(TBL_SE_MAX)]);

    return 0;
}
// 送水管の流水
void* NewWaterLineFall( int name, int map )	
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, name, map ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}

