//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    charapread.c
    キャラ付随飛抹
    2001/08/01 Masafumi Okuta
    $Id: charaspread.c,v 1.4 2002/11/23 12:16:43 Yoshizawa1 Exp $
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

#define	SET_FLAG( a,b )		(a) |= (b)
#define	UNSET_FLAG( a,b )	(a) &= ~(b)
#define	CLEAR_FLAG( a ) 	(a) = 0
#define	BODYWORLD(a,b) 		(a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVADD  	(SCRPAD_ADDR + 0x1000)
#define SCR_UVSUB  	(SCRPAD_ADDR + 0x2000)

#define GRAVITY 	(P_GRAVITY * 0.4f)

#define MAX_NODE	(8)	
#define MAX_SPEED	(STEP_VMODEF(60.f))

enum{ // メッセージ
SPREAD_MESSAGE_ACTIVE,	// アクティブモード
SPREAD_MESSAGE_SLEEP,	// スリープモード
SPREAD_MESSAGE_STOP,	// 生成終了
SPREAD_MESSAGE_RESTART,	// 生成再開
SPREAD_MESSAGE_KILL,	// 破棄
};

static u_short TBL_NODE_INDEX[2][MAX_NODE] = {
    { // 俯瞰
	HUMAN21_HIDARI_KAKATO, 	HUMAN21_MIGI_KAKATO, 
	HUMAN21_HIDARI_TE, 	HUMAN21_MIGI_TE, 
	HUMAN21_HIDARI_ASHI2, 	HUMAN21_MIGI_ASHI2, 
	HUMAN21_HIDARI_UDE2, 	HUMAN21_MIGI_UDE2,
    },
    { // 主観
	HUMAN21_HIDARI_KAKATO, 	HUMAN21_MIGI_KAKATO, 
	HUMAN21_HIDARI_KAKATO, 	HUMAN21_MIGI_KAKATO, 
	HUMAN21_HIDARI_KAKATO, 	HUMAN21_MIGI_KAKATO, 
	HUMAN21_HIDARI_KAKATO, 	HUMAN21_MIGI_KAKATO, 
    },
};

typedef struct _CHARASPREAD {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;				// 名前ID
    GV_MSG*		msg;				// 自分宛メッセージ	

    // 拡張システム
    int			nMode;				// 生成モード
    int			nSleep;				// スリープモード
    int			nSleepCntr;			// スリープカウンタ
    int			nFeed;				// フェードモード
    int			nFeedCntr;			// フェードカウンタ
    short		nIndexNum;			// 参照関節数
    CONTROL*		control;			// 参照するコントロール

    FVECTOR*		vecSub;				// 参照位置の差分ベクトル列
    FVECTOR*		vecCenterPrev;			// 参照する位置列（1フレーム前）
    FVECTOR*		vecPrevSub;			// 参照位置の差分ベクトル列（1フレーム前）

    short*		nAddCurr;			// 現在の生成する数
    short*		nAddMax;			// 生成する数
    short*		nLife;				// 滴の生存時間列
    FVECTOR*		vecStep;			// 滴の移動量
    
    // 引数データ
    int			nConcName;			// 付随するキャラの名前ID
    short		nPrims;				// プリミティブ数 
    short		nVerts;				// 頂点数
    short		nNodePattern;			// ノードパターン
    short		nAddNum;			// 1フレームに登録できる数
    int			nDeadCntr;			// 破棄カウンタ
    float		fBaseSize;			// 基本サイズ
    float		fAddSize; 			// ランダム加算サイズ
    int			nRGBAAdd;			// 加算プリミティブのRGBA値
    int			nRGBASub;			// 減算プリミティブのRGBA値	
    int			nLifeMin;			// 生存時間最小
    int			nLifeMax;			// 生存時間最大

    // 描画ワーク
    DG_PRIM2*		primAdd;			// 加算
    DG_PRIM2*		primSub;			// 減算
    
    DG_PRIM2_UVRGBWH*	uvrgbSub1;			// 減算用バッファ１
    DG_PRIM2_UVRGBWH*	uvrgbSub2;			// 減算用バッファ２
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
	case SPREAD_MESSAGE_ACTIVE:	// アクティブモード
	    work->nSleep 	= 0;
	    break;
	case SPREAD_MESSAGE_SLEEP:	// スリープモード
//	    work->nSleep 	= 1;
	    work->nSleepCntr 	= COUNT_VMODE(360);
	    break;
	case SPREAD_MESSAGE_STOP:
	    work->nSleep 	= 0;
	    work->nFeed  	= 1;
	    work->nFeedCntr  	= COUNT_VMODE(360);
	    break;
	case SPREAD_MESSAGE_RESTART:
	    work->nSleep 	= 0;
	    work->nFeed  	= 0;
	    work->nFeedCntr  	= work->nLifeMax;
	    work->primAdd->flag &= ~DG_PRIM2_INVISIBLE;
	    work->primSub->flag &= ~DG_PRIM2_INVISIBLE;
	    break;
	case SPREAD_MESSAGE_KILL:	// 破棄
	    GV_DestroyActor( work ) ;
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

// プリミティブ更新
static void UpdatePacket( Work* work)
{
    u_short nAlpAdd = (u_short)(work->nRGBAAdd & 0x000000ff);
    u_short nAlpSub = (u_short)(work->nRGBASub & 0x000000ff);
    int i, j, nNewAdd, nAddIndex;
    int nCurrClock, nPrevClock;
    short*		pnLife;
    FVECTOR*		pvecStep;

    FVECTOR*		pvecPrim;
    DG_PRIM2_UVRGBWH*	uvrgbwhAdd;
    DG_PRIM2_UVRGBWH*	uvrgbwhSub;

    FVECTOR*		pvecPrimPrev;   
    DG_PRIM2_UVRGBWH*	uvrgbwhAddPrev; 
    DG_PRIM2_UVRGBWH*	uvrgbwhSubPrev; 
    FVECTOR*		pvecPrimCurr;   
    DG_PRIM2_UVRGBWH*	uvrgbwhAddCurr; 
    DG_PRIM2_UVRGBWH*	uvrgbwhSubCurr; 

    CONTROL*		ctrl = work->control;

    GM_GroupPrim2( work->primAdd, GM_CurrentStageMap );
    GM_GroupPrim2( work->primSub, GM_CurrentStageMap );
    nPrevClock = work->primAdd->buffer_clock;
     //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->primAdd ) )
   {
      return;
   }
    DG_SwitchBuffPrim2( work->primSub );
    nCurrClock = work->primAdd->buffer_clock;

    pvecPrimPrev   = work->primAdd->pos[nPrevClock];
    uvrgbwhAddPrev = work->primAdd->uvrgb[nPrevClock];
    uvrgbwhSubPrev = work->primSub->uvrgb[nPrevClock];
    pvecPrimCurr   = work->primAdd->pos[nCurrClock];
    uvrgbwhAddCurr = work->primAdd->uvrgb[nCurrClock];
    uvrgbwhSubCurr = work->primSub->uvrgb[nCurrClock];

    pnLife = work->nLife;
    pvecStep = work->vecStep;

    nNewAdd = 0;
    nAddIndex = 0;
#ifndef KP_WINDOWS
    for ( i = 0; i < work->nPrims; i++ ){
#else
    for ( i = work->nPrims; i > 0; i-- ){
#endif
#if 0
	pvecPrim = (FVECTOR*)SCR_POS;
	uvrgbwhAdd  = (DG_PRIM2_UVRGBWH*)SCR_UVADD;
	uvrgbwhSub  = (DG_PRIM2_UVRGBWH*)SCR_UVSUB;
	// スクラッチパッドに転送
	Mao_CopyMemToScr( pvecPrimPrev, SCR_POS, sizeof(FVECTOR), work->nVerts );
	Mao_CopyMemToScr( uvrgbwhAddPrev, SCR_UVADD, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
	Mao_CopyMemToScr( uvrgbwhSubPrev, SCR_UVSUB, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
#else 
	pvecPrim = pvecPrimCurr;
	uvrgbwhAdd = uvrgbwhAddCurr;
	uvrgbwhSub = uvrgbwhSubCurr;
	memcpy( pvecPrim, pvecPrimPrev, sizeof(FVECTOR) * work->nVerts);
	memcpy( uvrgbwhAdd, uvrgbwhAddPrev, sizeof(DG_PRIM2_UVRGBWH) * work->nVerts);
	memcpy( uvrgbwhSub, uvrgbwhSubPrev, sizeof(DG_PRIM2_UVRGBWH) * work->nVerts);
#endif
#ifndef KP_WINDOWS
	for ( j = 0; j < work->nVerts; j++ ){
#else
	for ( j = work->nVerts; j>0; j-- ){
#endif
	    if ( (*pnLife) > 0 ){ // 動作中
		_sceVu0AddVector( pvecPrim, pvecPrim, pvecStep); // 移動
		pvecStep->vy += ((GRAVITY) * (float)uvrgbwhAdd->w / work->fBaseSize); 
		(*pnLife)--;
#if 0
		if ( uvrgbwhAdd->a > (*pnLife)){
		    uvrgbwhAdd->a = (*pnLife);
		}
		if ( uvrgbwhSub->a > (*pnLife) ){
		    uvrgbwhSub->a = (*pnLife);
		}
#endif
	    }else if ( !GM_CheckPlayerStatus( PLAYER_INTRUDE | PLAYER_WATCH) && 
		       !work->nFeed && !work->nSleep && nAddIndex < MAX_NODE){ // 新規生成
		if ( work->nAddCurr[nAddIndex] < work->nAddMax[nAddIndex]){
		    MAO_InterpVec( pvecPrim, (FVECTOR*)&BODYWORLD( ctrl->object, 
				   TBL_NODE_INDEX[work->nMode][nAddIndex]).m[3][0],
				   &work->vecCenterPrev[nAddIndex], rnd());
		    pvecPrim->vx += frnd() * 30.f;
		    pvecPrim->vy += frnd() * 30.f;
		    pvecPrim->vz += frnd() * 30.f;
		    _sceVu0CopyVector( pvecStep, &work->vecSub[nAddIndex]);
		    MAO_InterpVec( pvecStep, &work->vecSub[nAddIndex], &work->vecPrevSub[nAddIndex], rnd());

		    pvecStep->vx += frnd() * 5.f;
		    pvecStep->vy += frnd() * 5.f;
		    pvecStep->vz += frnd() * 5.f;
		    (*pnLife) = MAO_GetRandom( work->nLifeMin, work->nLifeMax);
		    uvrgbwhAdd->a = nAlpAdd;
		    uvrgbwhSub->a = nAlpSub;
		    nNewAdd++;

		    // 数チェック
		    work->nAddCurr[nAddIndex]++;
		}
		if ( work->nAddCurr[nAddIndex] >= work->nAddMax[nAddIndex]){
		    nAddIndex++;
		}
	    }else{
		uvrgbwhAdd->a = uvrgbwhSub->a = 0;
	    }
	    pvecPrim++;
	    uvrgbwhAdd++;
	    uvrgbwhSub++;
	    pnLife++;
	    pvecStep++;
	}
#if 0
	// メモリに転送
	Mao_CopyScrToMem( pvecPrimCurr, SCR_POS, sizeof(FVECTOR), work->nVerts );
	Mao_CopyScrToMem( uvrgbwhAddCurr, SCR_UVADD, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
	Mao_CopyScrToMem( uvrgbwhSubCurr, SCR_UVSUB, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
#endif
	pvecPrimPrev += work->nVerts;
	pvecPrimCurr += work->nVerts;		
	uvrgbwhAddPrev += work->nVerts;		
	uvrgbwhAddCurr += work->nVerts;		
	uvrgbwhSubPrev += work->nVerts;		
	uvrgbwhSubCurr += work->nVerts;		
    }
    // 生存時間設定があった
    if ( !work->nFeed && work->nDeadCntr > 0){
	work->nDeadCntr--;
	if ( work->nDeadCntr == 0){
	    work->nSleep  	= 0;
	    work->nFeed  	= 1;
	    work->nFeedCntr  	= COUNT_VMODE(360);
	}
    }

}
// 動作関数
static void Act( Work* work)
{
    int i;
    float fSpeed;
    CONTROL* ctrl;

    ctrl = work->control = GM_SearchWhere(work->nConcName);

    if ( ctrl == NULL){
	GV_DestroyActor( work );
	return;
    }

    // 非表示／表示チェック
    if ( (ctrl->object->objs->flag & DG_FLAG_INVISIBLE0) &&  
	 (ctrl->object->objs->flag & DG_FLAG_INVISIBLE1) ){
	if ( ctrl->object->evmobj != NULL){
	    if ( (ctrl->object->evmobj->flag & DG_EVMOBJ_INVISIBLE0) && 
		 (ctrl->object->evmobj->flag & DG_EVMOBJ_INVISIBLE1) ){
		work->primAdd->flag |= DG_PRIM2_INVISIBLE;
		work->primSub->flag |= DG_PRIM2_INVISIBLE;
	    }else{
		work->primAdd->flag &= ~DG_PRIM2_INVISIBLE;
		work->primSub->flag &= ~DG_PRIM2_INVISIBLE;
	    }
	}else{
	    work->primAdd->flag |= DG_PRIM2_INVISIBLE;
	    work->primSub->flag |= DG_PRIM2_INVISIBLE;
	}
    }else{
	work->primAdd->flag &= ~DG_PRIM2_INVISIBLE;
	work->primSub->flag &= ~DG_PRIM2_INVISIBLE;
    }

    if ( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
	work->nMode = 1;
    }else{
	work->nMode = 0;
    }

    { // キャラの関節データ
	int i;
	CONTROL* ctrl = work->control;
	
	short*	 pnCurr = work->nAddCurr;
	short*	 pnMax  = work->nAddMax;
	FVECTOR* pvecSub = work->vecSub;
	FVECTOR* pvecCenterPrev = work->vecCenterPrev;
	FVECTOR* pvecPrevSub = work->vecPrevSub;
	// 差分ベクトル生成
	for ( i = 0; i < MAX_NODE; i++){
	    _sceVu0CopyVector( pvecPrevSub, pvecSub);
	    _sceVu0SubVector( pvecSub, (FVECTOR*)&BODYWORLD( ctrl->object, 
			      TBL_NODE_INDEX[work->nMode][i]).m[3][0], 
			      pvecCenterPrev);
	    fSpeed = _MAO_FVecLen3( pvecSub);
	    if ( fSpeed >= MAX_SPEED){ // 制限チェック
		_sceVu0ScaleVector( pvecSub, pvecSub, MAX_SPEED / fSpeed);
	    }

	    // 生成数を決定
	    (*pnCurr) = 0;
	    (*pnMax) = MAO_Rand(2) + (int)( fSpeed * 5.f / MAX_SPEED );
	    if ( (*pnMax) >= 2 ) (*pnMax) = 2;

	    pnCurr++;  
	    pnMax++;  
	    pvecSub++;
	    pvecCenterPrev++;
	    pvecPrevSub++;
	}
    }

    // メッセージチェック
    CheckMessage( work);

    // スリープモードチェック
//    if ( !work->nSleep ){
    {
	// プリミティブ更新
	UpdatePacket( work);
    }

    {
	FVECTOR* pvecCenterPrev = work->vecCenterPrev;
	// 差分ベクトル生成
	for ( i = 0; i < MAX_NODE; i++){
	    // 前の座標として取得
	    _sceVu0CopyVector( pvecCenterPrev, (FVECTOR*)&BODYWORLD( ctrl->object, 
							   TBL_NODE_INDEX[work->nMode][i]).m[3][0] );
	    pvecCenterPrev++;
	}
    }

    // スリープチェック
    if ( !work->nSleep ){
	if ( work->nSleepCntr > 0 ){
	    work->nSleepCntr--;
	    if ( work->nSleepCntr == 0){
		work->nSleep = 1; // 生成を停止
	    }
	}
    }

    // フェードチェック
    if ( work->nFeed && work->nFeedCntr > 0){
	work->nFeedCntr--;
	if ( work->nFeedCntr == 0 ){
	    GV_DestroyActor( work ) ;
	}
    }
}

static void Die( Work* work)
{
    work->primAdd       = MAO_FreePrim2( work->primAdd );
    work->primSub       = MAO_FreePrim2( work->primSub );

    if ( work->vecSub != NULL ) 	GV_Free( work->vecSub );
    if ( work->vecCenterPrev != NULL ) 	GV_Free( work->vecCenterPrev );
    if ( work->vecPrevSub != NULL ) 	GV_Free( work->vecPrevSub );

    if ( work->nAddCurr != NULL )    	GV_Free( work->nAddCurr );
    if ( work->nAddMax != NULL )    	GV_Free( work->nAddMax );
    if ( work->nLife != NULL ) 	   	GV_Free( work->nLife );
    if ( work->vecStep != NULL )   	GV_Free( work->vecStep );
    if ( work->uvrgbSub1 != NULL ) 	GV_Free( work->uvrgbSub1 );
    if ( work->uvrgbSub2 != NULL ) 	GV_Free( work->uvrgbSub2 );
}

static int InitPacket( Work* 	work,
		       DG_TEX*	tex)
{
    int i,j;
    u_char		r, g, b, a;
    DG_PRIM2_UVRGBWH*	uvrgbwhAdd1;
    DG_PRIM2_UVRGBWH*	uvrgbwhAdd2;
    DG_PRIM2_UVRGBWH*	uvrgbwhSub1;
    DG_PRIM2_UVRGBWH*	uvrgbwhSub2;
    DG_PRIM2_UVRGBWH*	uvrgbwh;
    FVECTOR* 		pvecPos1;
    FVECTOR* 		pvecPos2;

    //------- 加算
    r = (u_char)((work->nRGBAAdd & 0xff000000) >> 24);
    g = (u_char)((work->nRGBAAdd & 0x00ff0000) >> 16);
    b = (u_char)((work->nRGBAAdd & 0x0000ff00) >> 8);
    a = (u_char)((work->nRGBAAdd & 0x000000ff));
    
    uvrgbwhAdd1 = work->primAdd->uvrgb[0];
    uvrgbwhAdd2 = work->primAdd->uvrgb[1];
    uvrgbwhSub1 = work->primSub->uvrgb[0];
    uvrgbwhSub2 = work->primSub->uvrgb[1];

    for ( i = 0; i < work->nPrims; i++ ){
	uvrgbwh  = (DG_PRIM2_UVRGBWH*)SCR_UVADD;
	for ( j = 0; j < work->nVerts; j++ ){
	    float fSize = work->fBaseSize + work->fAddSize * rnd();
	    uvrgbwh->w  = (int)fSize;
	    uvrgbwh->h  = (int)fSize;
	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );/* 左上 */
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );/* 左上 */
	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );/* 右下 */
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );/* 右下 */
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f0 = 0x8fff; // キックしない
	    uvrgbwh->f1 = 0x8fff;

	    uvrgbwh->r  = r;
	    uvrgbwh->g  = g;
	    uvrgbwh->b  = b;
	    uvrgbwh->a  = a;

	    uvrgbwh++;

	}
	// メモリに転送
	Mao_CopyScrToMem( uvrgbwhAdd1, SCR_UVADD, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
	Mao_CopyScrToMem( uvrgbwhAdd2, SCR_UVADD, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
	Mao_CopyScrToMem( uvrgbwhSub1, SCR_UVADD, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
	Mao_CopyScrToMem( uvrgbwhSub2, SCR_UVADD, sizeof(DG_PRIM2_UVRGBWH), work->nVerts );
	
	uvrgbwhAdd1 += work->nVerts;
	uvrgbwhAdd2 += work->nVerts;
	uvrgbwhSub1 += work->nVerts;
	uvrgbwhSub2 += work->nVerts;
    }
    pvecPos1 = work->primAdd->pos[0];
    pvecPos2 = work->primAdd->pos[1];
    for ( i = 0; i < work->nPrims; i++ ){
	for ( j = 0; j < work->nVerts; j++ ){
	    _sceVu0CopyVector( pvecPos1, &DG_ZeroVector);
	    _sceVu0CopyVector( pvecPos2, &DG_ZeroVector);
	    pvecPos1++;
	    pvecPos2++;
	}
    }

    //------- 減算
    uvrgbwhSub1  = work->primSub->uvrgb[0];
    uvrgbwhSub2  = work->primSub->uvrgb[1];

    r = (u_char)(work->nRGBASub & 0xff000000) >> 24;
    g = (u_char)(work->nRGBASub & 0x00ff0000) >> 16;
    b = (u_char)(work->nRGBASub & 0x0000ff00) >> 8;
    a = (u_char)(work->nRGBASub & 0x000000ff);
    
    for ( i = 0; i < work->nPrims; i++ ){
	for ( j = 0; j < work->nVerts; j++ ){
	    uvrgbwhSub2->r = uvrgbwhSub1->r = r;
	    uvrgbwhSub2->g = uvrgbwhSub1->g = g;
	    uvrgbwhSub2->b = uvrgbwhSub1->b = b;
	    uvrgbwhSub2->a = uvrgbwhSub1->a = 0x00;//a;
	    uvrgbwhSub1++;
	    uvrgbwhSub2++;
	}
    }

    return 0;
}


static int GetResources( Work*		work, 
			 int		name,
			 int		map)
{
    int i;

    work->nMode		= 0;
    work->nSleep    	= 0;
    work->nFeed    	= 0;
    work->nFeedCntr    	= 0;
    work->nIndexNum    	= 0;
    
    // 参照用ワーク
    // 生成する数
    if( ( work->nAddCurr = (short *)GV_Malloc(sizeof(short) * MAX_NODE ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nAddCurr, sizeof(short) * MAX_NODE );

    // 生成する数
    if( ( work->nAddMax = (short *)GV_Malloc(sizeof(short) * MAX_NODE ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nAddMax, sizeof(short) * MAX_NODE );

    // 関節の移動ベクトル
    if( ( work->vecSub = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MAX_NODE ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecSub, sizeof(FVECTOR) * MAX_NODE );

    // 関節の１フレーム前の位置ベクトル
    if( ( work->vecCenterPrev = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MAX_NODE ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecCenterPrev, sizeof(FVECTOR) * MAX_NODE );

    // 関節の１フレーム前の移動ベクトル
    if( ( work->vecPrevSub = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MAX_NODE ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecPrevSub, sizeof(FVECTOR) * MAX_NODE );

    // コントロール検索
    work->control = GM_SearchWhere(work->nConcName);
    if ( work->control == NULL){
	MAO_PRINTF(" Chara Not found!! :: %d", work->nConcName);
	return -1 ;
    }

    { // キャラの関節データ
	CONTROL* ctrl = work->control;
	FVECTOR* pvecSub = work->vecSub;
	FVECTOR* pvecCenterPrev = work->vecCenterPrev;
	FVECTOR* pvecPrevSub = work->vecPrevSub;
	for ( i = 0; i < MAX_NODE; i++){
	    _sceVu0CopyVector( pvecSub, &DG_ZeroVector);
	    _sceVu0CopyVector( pvecCenterPrev, (FVECTOR*)&BODYWORLD( ctrl->object, 
			       TBL_NODE_INDEX[work->nMode][i]).m[3][0] );
	    _sceVu0CopyVector( pvecPrevSub, pvecSub);
	    pvecSub++;
	    pvecCenterPrev++;
	    pvecPrevSub++;
	}
    }

    // 生存時間
    if( ( work->nLife = (short *)GV_Malloc(sizeof(short) * work->nPrims * work->nVerts ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nLife, sizeof(short) * work->nPrims * work->nVerts );

    // 移動ベクトル
    if( ( work->vecStep = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nPrims * work->nVerts ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecStep, sizeof(FVECTOR) * work->nPrims * work->nVerts );
    
    //------------------------ 描画系ワーク
    // 減算用のバッファを確保
    if( ( work->uvrgbSub1 = (DG_PRIM2_UVRGBWH*)GV_Malloc(sizeof(DG_PRIM2_UVRGBWH) * work->nPrims * work->nVerts ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->uvrgbSub1, sizeof(DG_PRIM2_UVRGBWH) * work->nPrims * work->nVerts );
    if( ( work->uvrgbSub2 = (DG_PRIM2_UVRGBWH*)GV_Malloc(sizeof(DG_PRIM2_UVRGBWH) * work->nPrims * work->nVerts ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->uvrgbSub2, sizeof(DG_PRIM2_UVRGBWH) * work->nPrims * work->nVerts );


    { // 描画設定
	DG_TEX*	tex = DG_GetTexture( 4642619 ); // drop02_msk

	work->primAdd = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
				       work->nPrims, work->nVerts);
	if ( work->primAdd == NULL ){
	    printf("null prim\n");
	    return -1;
	}
	// テクスチャ設定
	DG_ConfigPrim2Tex( work->primAdd, tex );	// プリミティブにテクスチャを設定
	DG_SetPrim2Alpha( work->primAdd, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	work->primSub = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_NOBUFFER, 
				       work->nPrims, work->nVerts);
	if ( work->primSub == NULL ){
	    printf("null prim\n");
	    return -1;
	}
	// バッファ設定
	DG_SetPrim2Buffer( work->primSub, work->primAdd->pos[0], work->primAdd->pos[1], 
			   work->uvrgbSub1, work->uvrgbSub2 );
	// テクスチャ設定
	DG_ConfigPrim2Tex( work->primSub, tex );	// プリミティブにテクスチャを設定
	DG_SetPrim2Alpha( work->primSub, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	
   work->primSub->flag |= DG_PRIM_AS_CUSTOMWORLD;
	work->primSub->as_world.m[3][0] = 10.f;

	InitPacket( work, tex);
    }

    // メッセージチェック
    CheckMessage( work);

    return 0;
}

// キャラ付随水飛抹
void* NewCharaSpreadProg( int		name,
			  int		nConcName,
			  short		nNodePattern,
			  short		nAddNum,
			  short		nPrims,
			  short		nVerts,
			  float		fBaseSize,
			  float		fAddSize,
			  int 		nRGBAAdd, 
			  int 		nRGBASub,
			  int		nLifeMin,
			  int		nLifeMax)
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	// 引数のデータを取得
	work->name	    	= name;
	work->nConcName	    	= nConcName;
	work->nPrims		= nPrims;
	work->nVerts		= nVerts;
	if ( work->nVerts > 32 ) work->nVerts = 32;
	work->nNodePattern 	= nNodePattern;
	work->nAddNum		= nAddNum;
	work->fBaseSize		= fBaseSize;
	work->fAddSize		= fAddSize;
	work->nRGBAAdd		= nRGBAAdd;
	work->nRGBASub		= nRGBASub;
	work->nLifeMin  	= nLifeMin;
	work->nLifeMax  	= nLifeMax;

	if ( GetResources( work, name, GM_CurrentStageMap ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    
    return (void *)work ;
}

// キャラ付随水飛抹
void* NewCharaSpreadScn( int		name,
			 int		map)
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	// 引数のデータを取得
	work->name	    	= name;
	work->nConcName	    	= GCL_GetOptionValue( 'n', GV_StrCode("てすてす") );	// 付随させる名前ID
	work->nPrims		= GCL_GetOptionValue( 'p', 12 ); 			// プリミティブ数
	work->nVerts		= GCL_GetOptionValue( 'v', 32 ); 			// 頂点数
	if ( work->nVerts > 32 ) work->nVerts = 32;
	work->nNodePattern 	= GCL_GetOptionValue( 'o', 0 ); 			// ノード参照パターン
	work->nAddNum		= GCL_GetOptionValue( 'a', 32 ); 			// 1フレームに生成する数
	work->nDeadCntr		= GCL_GetOptionValue( 't', -1 ); 			// 生存時間
	work->fBaseSize		= (float)GCL_GetOptionValue( 'b', 10 ); 		// 基本サイズ
	work->fAddSize		= (float)GCL_GetOptionValue( 's', 10 ); 		// ランダム加算サイズ
	work->nRGBAAdd		= GCL_GetOptionValue( 'r', 0x40404040 ); 		// 加算プリミティブRGBA値
	work->nRGBASub		= GCL_GetOptionValue( 'g', 0x30303040 ); 		// 減算プリミティブRGBA値
	work->nLifeMin  	= GCL_GetOptionValue( 'l', 30 ); 			// 生存時間最小
	work->nLifeMax  	= GCL_GetOptionValue( 'm', 40 ); 			// 生存時間最大

	if ( GetResources( work, name, map ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    
    return (void *)work ;
}



