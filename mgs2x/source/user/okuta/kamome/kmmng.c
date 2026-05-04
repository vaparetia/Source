//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmmng.c
    カモメマネージャテスト
    2001/05/02 Masafumi Okuta
    $Id: kmmng.c,v 1.1.1.3 2002/11/19 11:48:03 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"../conv/memmot.h"
#include	"../../korekado/conv/define.h"

#include	"kamome.h"
#include	"kamome.def"

#ifdef DEBUG_MODE
#include	"debugmenu.h"	
#endif

#include	"kmsound.c"
#include	"kmsend.c"	// メッセージ送信処理
#include	"kmmess.c"	// メッセージ受信処理
#include	"kmact.c"	// 動作制御
#include	"kmmove.c"	// 移動処理
#include	"kmthink.c"	// 思考処理
#include	"kmpre.c"	// 前後処理
#include	"kmtest.c"	// 本体初期化
#include	"kmginfo.c"	// 情報

#ifdef DEBUG_MODE
#include	"kmdbg.c"
#endif

// ローカル
static KAMOME_MNG* _kamome_mng = NULL; // かもめマネージャを外部から参照するため



// かもめをリストに加える
void KMM_InsertKamome( KAMOME_MNG*  kmmng,	// かもめマネージャ
		       KAMOME* 	   pKamome)	// リストに加えるかもめ
{
    if ( kmmng->pkmmTop == NULL){ // 先頭に追加
	kmmng->pkmmTop = kmmng->pkmmEnd = pKamome;
	pKamome->pkmmPrev = NULL;
	pKamome->pkmmNext = NULL;
    }else { // 最後尾に追加
	kmmng->pkmmEnd->pkmmNext = pKamome;
	pKamome->pkmmPrev 	 = kmmng->pkmmEnd;
	pKamome->pkmmNext 	 = NULL;
	kmmng->pkmmEnd 		 = pKamome;
    }
}
// かもめをリストから外す
void KMM_RemoveKamome( KAMOME_MNG*  kmmng,	// かもめマネージャ
		       KAMOME* 	   pKamome)	// リストから外すかもめ
{
    if ( pKamome->pkmmPrev == NULL ){ // 先頭
	kmmng->pkmmTop = pKamome->pkmmNext;
	if ( pKamome->pkmmNext != NULL){
	    pKamome->pkmmNext->pkmmPrev = NULL;
	}
    }else{
	pKamome->pkmmPrev->pkmmNext = pKamome->pkmmNext;
    }


    if ( pKamome->pkmmNext == NULL ){ // 最後
	kmmng->pkmmEnd = pKamome->pkmmPrev;
	if ( pKamome->pkmmPrev != NULL){
	    pKamome->pkmmPrev->pkmmNext = NULL;
	}
    }else{
	pKamome->pkmmNext->pkmmPrev = pKamome->pkmmPrev;
    }
}
// 指定エリア,指定範囲内にかもめがいるかどうか
int KMMNG_CheckKamomeIn( FVECTOR* pvecCenter, int nRadius)
{
    KAMOME*	pKamome;
    FVECTOR 	vec, vecBound1, vecBound2;

    if ( _kamome_mng == NULL ){ // マネージャがいない
	return 0;
    }

    { // バウンディングボックス
	float fRad = (float)nRadius;
	vecBound1.vx = pvecCenter->vx - fRad;
	vecBound1.vy = pvecCenter->vy - fRad;
	vecBound1.vz = pvecCenter->vz - fRad;
	vecBound2.vx = pvecCenter->vx + fRad;
	vecBound2.vy = pvecCenter->vy + fRad;
	vecBound2.vz = pvecCenter->vz + fRad;
    }

    // かもめリストチェック
    for ( pKamome = _kamome_mng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	// 状態チェックを入れる
	// 距離判定前にバウンドチェックである程度弾いておく
	if ( pKamome->think.nThink3 == TH3_DEAD || 
	     !MAO_BoundCheck( &pKamome->mov, &vecBound1, &vecBound2 ) ) continue; 

	// 距離判定
	_sceVu0SubVector( &vec, &pKamome->mov, pvecCenter);
	if ( _FVecLen3( &vec ) < nRadius ){
	    return 1;	// カモメ存在!!
	}
    }
    
    return 0;
}
// 指定エリア内にかもめがいるかどうか
int KMMNG_CheckLandKamomeInBox( FVECTOR* pvecCenter, float fBoxX, float fBoxY, float fBoxZ)
{
    KAMOME*	pKamome;
    FVECTOR 	vecBound1, vecBound2;

    if ( _kamome_mng == NULL ){ // マネージャがいない
	return 0;
    }

    { // バウンディングボックス
	vecBound1.vx = pvecCenter->vx - fBoxX;
	vecBound1.vy = pvecCenter->vy - fBoxY;
	vecBound1.vz = pvecCenter->vz - fBoxZ;
	vecBound2.vx = pvecCenter->vx + fBoxX;
	vecBound2.vy = pvecCenter->vy + fBoxY;
	vecBound2.vz = pvecCenter->vz + fBoxZ;
    }

    // かもめリストチェック
    for ( pKamome = _kamome_mng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	// 状態チェックを入れる
	if ( pKamome->think.nThink3 != TH3_DEAD ){	
	    // バウンドチェック
	    if ( MAO_BoundCheck( &pKamome->vecLand, &vecBound1, &vecBound2 ) ){
		return 1;	// カモメ存在!!
	    }
	    // バウンドチェック
	    if ( MAO_BoundCheck( &pKamome->vecAimPos, &vecBound1, &vecBound2 ) ){
		return 1;	// カモメ存在!!
	    }
	}
    }
    
    return 0;
}
// 全カモメ着地系命令設定
static void KMM_SetAllLandOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 全カモメ飛び立ち命令設定
static void KMM_SetAllFlyOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 != TH3_REST ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage  = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 全カモメ攻撃命令設定
static void KMM_SetAllAttackOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 全カモメ頭上付随命令設定
static void KMM_SetAllConcOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 全カモメ死亡設定
static void KMM_SetAllDeadOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定球内のカモメの飛行チェンジ
static void KMM_SetChangeAreaInSphere( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nRad;
    FVECTOR 	vecCenter;
    KAMOME* 	pKamome;

    vecCenter.vx = (float)msg->message[ 1 ];	
    vecCenter.vy = (float)msg->message[ 2 ];
    vecCenter.vz = (float)msg->message[ 3 ];
    vecCenter.vw = 1.f;
    nRad 	 = msg->message[ 4 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_NOISE_REACT_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく

	if ( !MAO_SphereCheck( &pKamome->mov, &vecCenter, nRad) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定トラップ内のカモメの飛行チェンジ
static void KMM_SetChangeAreaInTrap( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR     vecFrom, vecTo;
    KAMOME* 	pKamome;

    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 1 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく

	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定箱内のカモメの飛行チェンジ
static void KMM_SetChgAreaInBoxOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR 	vecFrom, vecTo;
    KAMOME* 	pKamome;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_NOISE_REACT_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく

	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定エリア内カモメ集合命令(個数指定可能)
static void KMM_SetAggreFromAreaOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;
    int		nCnt, nKamome;	
    FVECTOR     vecFrom, vecTo;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];
    nKamome	= msg->message[ 7 ];
    nCnt	= 0;
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END || 
	     !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;

	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	nCnt++;
	if ( nKamome > 0 && nCnt >= nKamome ){ // 指定数チェック : ０以下ならしない
	    break;
	}
    }
}
// 指定球内のかもめ緊急飛び立ち
static void KMM_SetTakeOffInSphere( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nRad;
    KAMOME* 	pKamome;
    FVECTOR 	vecCenter;
    
    vecCenter.vx = (float)msg->message[ 1 ];	
    vecCenter.vy = (float)msg->message[ 2 ];
    vecCenter.vz = (float)msg->message[ 3 ];
    vecCenter.vw = 1.f;
    nRad 	 = msg->message[ 4 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 != TH3_REST ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_FEINT_START ) continue; // 睡眠,ダメージ中ははじく
	if ( !MAO_SphereCheck( &pKamome->mov, &vecCenter, nRad) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定球内のかもめ緊急散開
static void KMM_SetDeployInSphere( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nRad;
    KAMOME* 	pKamome;
    FVECTOR 	vecCenter;
    
    vecCenter.vx = (float)msg->message[ 1 ];	
    vecCenter.vy = (float)msg->message[ 2 ];
    vecCenter.vz = (float)msg->message[ 3 ];
    vecCenter.vw = 1.f;
    nRad 	 = msg->message[ 4 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	// 睡眠,ダメージ中ははじく
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 >= TH3_CHECK_TAKEOFF_REACT_END ||
	     !MAO_SphereCheck( &pKamome->mov, &vecCenter, nRad) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定トラップ内のかもめ緊急離陸
static void KMM_SetTakeOffInTrap( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;
    FVECTOR     vecFrom, vecTo;

    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 1 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	// 睡眠,ダメージ中ははじく
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 >= TH3_CHECK_TAKEOFF_REACT_END ||
	     !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;	
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定トラップ内のかもめ緊急散開
static void KMM_SetDeployInTrap( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;
    FVECTOR     vecFrom, vecTo;

    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 1 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	// 睡眠,ダメージ中ははじく
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 >= TH3_CHECK_TAKEOFF_REACT_END ||
	     !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;	
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定箱内のカモメの緊急離陸
static void KMM_SetTakeoffInBoxOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR 	vecFrom, vecTo;
    KAMOME* 	pKamome;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 != TH3_REST ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_FEINT_START ) continue; // 睡眠,ダメージ中ははじく
//	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定箱内のカモメの飛行チェンジ
static void KMM_SetDeployInBoxOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR 	vecFrom, vecTo;
    KAMOME* 	pKamome;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_NOISE_REACT_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく

	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}

// 指定トラップ内カモメ集合命令(個数指定可能)
static void KMM_SetAggreFromTrapOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;
    int		nCnt, nKamome;	
    FVECTOR     vecFrom, vecTo;
    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 1 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }
    nKamome	    = msg->message[ 2 ];	// 指定数
    nCnt	    = 0;
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END || 
	     !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;

	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	nCnt++;
	if ( nKamome > 0 && nCnt >= nKamome ){ // 指定数チェック : ０以下ならしない
	    break;
	}
    }
}


// 全カモメ集合命令設定:トラップ指定
static void KMM_SetAllFlipOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 全カモメ集合命令設定:エリア指定
static void KMM_SetAllAggreOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 全カモメ集合命令設定:エリア指定
static void KMM_SetAllTakeoffOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 != TH3_REST ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage  = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// カモメエルード妨害 : 線上指定
static void KMM_SetEludeOperationFromLine( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;

    FVECTOR     vecFrom, vecTo;
    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 1 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	// 条件チェック
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END || 
	     !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	return;
    }
}
// カモメ段ボール着地命令
static void KMM_SetLandCBoxOperation( KAMOME_MNG* kmmng)
{
    int		nNum;
    KAMOME* 	pKamome;

    // 段ボール着地可能チェック
    if ( !(kmmng->nInfo & KMM_INFO_PL_CBOX) ||
	  (kmmng->nInfo & KMM_INFO_CBOX) ) return;

    nNum = kmmng->nCboxNum;
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	int nX, nZ;

	// 条件チェック
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく

	// 着地点を算出
	nX = (int)((GM_PlayerBoxBody->objs->def->models[1].ux - GM_PlayerBoxBody->objs->def->models[1].lx) * 0.5);
	nZ = (int)((GM_PlayerBoxBody->objs->def->models[1].uz - GM_PlayerBoxBody->objs->def->models[1].lz) * 0.5);
	pKamome->vecLand.vx = ( nX == 0) ? 0 : KMM_GetDemoRandom( kmmng, -nX, nX);
	pKamome->vecLand.vy = GM_PlayerBoxBody->objs->def->models[1].uy;
	pKamome->vecLand.vz = ( nZ == 0) ? 0 : KMM_GetDemoRandom( kmmng, -nZ, nZ);

	KMM_SetThink3( pKamome, TH3_CBOX_LAND); // 段ボール移動

	// リミットチェック
	if (nNum++ > 4) break;
    }
}

// 音反応飛び立ちチェック
static void KMM_SetTakeOffOperation( KAMOME_MNG* kmmng, int code, FVECTOR* pvecPos, int nRadius)
{
    int		nDist;
    KAMOME* 	pKamome;
    FVECTOR 	vec, vecBound1, vecBound2;

    { // バウンディングボックス作成
	float fRad = (float)nRadius;
	vecBound1.vx = pvecPos->vx - fRad;
	vecBound1.vy = pvecPos->vy - fRad;
	vecBound1.vz = pvecPos->vz - fRad;
	vecBound2.vx = pvecPos->vx + fRad;
	vecBound2.vy = pvecPos->vy + fRad;
	vecBound2.vz = pvecPos->vz + fRad;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 >= TH3_CHECK_FREEMOVE_START ) continue;

	if ( pKamome->think.nThink3 >= TH3_CHECK_DAMAGE_START ) continue; // ダメージ中ははじく

	// 距離判定前にバウンドチェックである程度弾いておく
	if ( !MAO_BoundCheck( &pKamome->mov, &vecBound1, &vecBound2 ) ) continue; 

	// 距離判定
	_sceVu0SubVector( &vec, &pKamome->mov, pvecPos);

	nDist = _FVecLen3( &vec );
	if ( nDist < nRadius ){
	    int nDelay;
	    KAMOME_MNG* kmmng;
	    kmmng = (KAMOME_MNG*)pKamome->pvManager;

	    nDelay = (nDist / 50) + 1; // 距離による反応遅延
	    KMM_SetThink3Delay( pKamome, TH3_TAKEOFF, KMM_DemoRand( kmmng, COUNT_VMODE(nDelay) ));
	}
    }
}
// 指定球内のカモメの中から指定した数だけ線着地させる
static void KMM_SetLandLineInSphere( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nRad;
    FVECTOR 	vecCenter;
    KAMOME* 	pKamome;
    
    vecCenter.vx = (float)msg->message[ 2 ];	
    vecCenter.vy = (float)msg->message[ 3 ];
    vecCenter.vz = (float)msg->message[ 4 ];
    vecCenter.vw = 1.f;
    nRad 	 = msg->message[ 5 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END )     continue; 	// メッセージ取得状態に合わない
//	if ( pKamome->think.nThink3 >= TH3_CHECK_DAMAGE_START )    continue; 	// 睡眠,ダメージ中ははじく(略可)
	if ( !MAO_SphereCheck( &pKamome->mov, &vecCenter, nRad) ) continue; 	// 球内チェック
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	if ( kmmng->ucMsgMax[ pKamome->nMsgIndex] > msg->message[ 1 ]  ){ // リミットチェック
	    break;
	}
    }
}
// 指定球内のカモメの中から指定した数だけエリア着地させる
static void KMM_SetLandAreaInSphere( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nRad;
    FVECTOR 	vecCenter;
    KAMOME* 	pKamome;
    
    vecCenter.vx = (float)msg->message[ 2 ];	
    vecCenter.vy = (float)msg->message[ 3 ];
    vecCenter.vz = (float)msg->message[ 4 ];
    vecCenter.vw = 1.f;
    nRad 	 = msg->message[ 5 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END )     continue; 	// メッセージ取得状態に合わない
//	if ( pKamome->think.nThink3 >= TH3_CHECK_DAMAGE_START )    continue; 	// 睡眠,ダメージ中ははじく(略可)
	if ( !MAO_SphereCheck( &pKamome->mov, &vecCenter, nRad) ) continue; 	// 球内チェック
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	if ( kmmng->ucMsgMax[ pKamome->nMsgIndex] > msg->message[ 1 ]  ){ // リミットチェック
	    break;
	}
    }

}
// 指定トラップ内のカモメの中から指定した数だけ線着地させる
static void KMM_SetLandLineInTrap( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR     vecFrom, vecTo;
    KAMOME* 	pKamome;

    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 2 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
//	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく(略可)
	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	if ( kmmng->ucMsgMax[ pKamome->nMsgIndex] > msg->message[ 1 ]  ){ // リミットチェック
	    break;
	}
    }
#ifdef DEBUG_MODE
#if 0
    {
	FVECTOR vec1;
	FVECTOR vec2;
	void* NewMaoTestLine( FVECTOR*, FVECTOR*, u_char, u_char, u_char);
	vec1.vx = (float)msg->message[ 3 ];
	vec1.vy = (float)msg->message[ 4 ];
	vec1.vz = (float)msg->message[ 5 ];
	vec1.vw = 1.f;
	vec2.vx = (float)msg->message[ 6 ];
	vec2.vy = (float)msg->message[ 7 ];
	vec2.vz = (float)msg->message[ 8 ];
	vec2.vw = 1.f;
	GV_SetActorChild( kmmng, NewMaoTestLine( &vec1, &vec2, 0x00, 0xff, 0xff));
    }
#endif
#endif
}
// 指定トラップ内のカモメの中から指定した数だけエリア着地させる
static void KMM_SetLandAreaInTrap( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR     vecFrom, vecTo;
    KAMOME* 	pKamome;

    {
	int 		nTrapName;
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	nTrapName = msg->message[ 2 ];	// 検索するトラップ名取得
	HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	vecFrom.vx  = blk->tx + trp->b1.vx;
	vecFrom.vy  = blk->ty + trp->b1.vy;
	vecFrom.vz  = blk->tz + trp->b1.vz;
	vecTo.vx    = blk->tx + trp->b2.vx;
	vecTo.vy    = blk->ty + trp->b2.vy;
	vecTo.vz    = blk->tz + trp->b2.vz;
    }

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
//	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく(略可)
	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
	if ( kmmng->ucMsgMax[ pKamome->nMsgIndex] > msg->message[ 1 ]  ){ // リミットチェック
	    break;
	}
    }
}

// 指定エリア内のカモメ死亡設定
static void KMM_SetAreaDeadOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;
    FVECTOR     vecFrom, vecTo;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END || 
	     !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo)) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}

// 指定エリア内のカモメの高さたしこみ
static void KMM_SetYAddInAreaOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    KAMOME* 	pKamome;
    FVECTOR     vecFrom, vecTo;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo)) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// 指定数カモメ攻撃命令設定
static void KMM_SetNumAttackOperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nNum = 0;
    int		nMax, nName;
    KAMOME* 	pKamome;
    CONTROL*	pctrl;
    HOMING_TRG* phtrg;
    
    nMax    = msg->message[ 1 ];	
    nName   = msg->message[ 2 ]; 	 // 名前

    pctrl = NULL;
    if ( GM_PlayerControl != NULL && nName == GM_PlayerControl->name ){ // プレイヤー
	pctrl = GM_PlayerControl;
    }else{ // 敵兵
	phtrg = MAO_SearchEneHomingTrg( nName ); // 存在する敵兵をさがす
	if ( phtrg != NULL ){ 
	    pctrl = phtrg->ctrl; 
	}
    }
    
    if ( pctrl == NULL ) return ;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_FREEMOVE_END ) continue;
	if ( !MAO_SphereCheck( &pKamome->mov, &pctrl->mov, 8000) ) continue; 	// 球内チェック

	pKamome->nMessage  = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新

	if ( nNum++ >= nMax ){
	    break;
	}
    }
}

// 指定箱内のカモメからキャラの関節に着地カモメを呼ぶ
static void KMM_SetComeOnOpperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR 	vecFrom, vecTo;
    KAMOME* 	pKamome;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 < TH3_CHECK_FREEMOVE_START || 
	     pKamome->think.nThink3 > TH3_CHECK_NOISE_REACT_END ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_SLEEP_START ) continue; // 睡眠,ダメージ中ははじく

	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新

	return; // 一匹だけ
    }
}

// 指定箱内のカモメを元いた飛行エリアに戻す
static void KMM_SetTurnBackTakeOffOpperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    FVECTOR 	vecFrom, vecTo;
    KAMOME* 	pKamome;

    vecFrom.vx  = (float)msg->message[ 1 ];	
    vecFrom.vy  = (float)msg->message[ 2 ];
    vecFrom.vz 	= (float)msg->message[ 3 ];
    vecTo.vx 	= (float)msg->message[ 4 ];	
    vecTo.vy 	= (float)msg->message[ 5 ];
    vecTo.vz 	= (float)msg->message[ 6 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->nMsgIndex != -1 || pKamome->think.nThink3 != TH3_REST ) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_FEINT_START ) continue; // 睡眠,ダメージ中ははじく
	if ( !MAO_BoundCheck( &pKamome->mov, &vecFrom, &vecTo) ) continue;
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// カモメの付随をやめさせる
static void KMM_SetConcomitantEndOpperation( KAMOME_MNG* kmmng, GV_MSG* msg, int code, int nMess)
{
    int		nName;
    KAMOME* 	pKamome;
    
    nName = (float)msg->message[ 1 ];

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->think.nThink3Delay == TH3_CTRL_LAND ) {
	    pKamome->think.nThink3Delay = -1;
	    pKamome->think.nDelay       = 0;
	    continue;
	}
	if ( pKamome->nMsgIndex != -1 || (pKamome->think.nThink3 < TH3_CTRL_LAND && 
  	     pKamome->think.nThink3 > TH3_CTRL_REST ) || pKamome->nAimName != nName) continue;
	if ( pKamome->think.nThink3 >= TH3_CHECK_FEINT_START ) continue; // 睡眠,ダメージ中ははじく
	pKamome->nMessage = code; 
	pKamome->nMsgIndex = nMess;
	kmmng->ucMsgMax[ pKamome->nMsgIndex]++; // 同一メッセージ取得数更新
    }
}
// メッセージ受信
static void KMMNG_RecieveMessage( KAMOME_MNG* kmmng)
{
    int 	nMess;
    GV_MSG*	msg;
    KAMOME* 	pKamome;
    int 	n_msg, code;

    void	( *SendMessage)( KAMOME_MNG*, GV_MSG*, int, int);


    // name = 0ならなにもしない
    if ( kmmng->name == 0 ){ return; }

    n_msg = GV_ReceiveMessage( kmmng->name, &kmmng->msg ); // メッセージ取得

    msg = kmmng->msg;

    // メッセージ初期化
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	pKamome->nMessage  = 0; 	// 空にしておく
	pKamome->nMsgIndex = -1;
    }
    
    // メッセージを反映
    nMess = 0;
    while ( n_msg-- > 0 ){
	SendMessage = NULL;
    
	code = msg->message[ 0 ]; // メッセージの種類

	switch (code){
	case KMNG_MESS_ALL_LAND_LINE:	// みんな着地:線
	    SendMessage = KMM_SetAllLandOperation;
	    break;
	case KMNG_MESS_ALL_LAND_AREA:	// みんな着地:エリア
	    SendMessage = KMM_SetAllLandOperation;
	    break;
	case KMNG_MESS_ALL_FLY:		// みんなとびたて
	    SendMessage = KMM_SetAllFlyOperation;
	    break;
	case KMNG_MESS_ALL_ATTACK:	// みんな攻撃
	    SendMessage = KMM_SetAllAttackOperation;
	    break;
	case KMNG_MESS_ALL_FLIPFLOP:	// みんなでばたつき
	    SendMessage = KMM_SetAllFlipOperation;
	    break;
	case KMNG_MESS_ALL_AGGREGATE:	// みんな集合
	    SendMessage = KMM_SetAllAggreOperation;
	    break;
	case KMNG_MESS_ALL_TAKEOFF:	// みんな緊急離陸
	    SendMessage = KMM_SetAllTakeoffOperation;
	    break;
	case KMNG_MESS_ALL_DEAD:	// みんな死亡
	    SendMessage = KMM_SetAllDeadOperation;
	    break;
	case KMNG_MESS_ALL_CONCOMITANT:	// みんなで頭上付随
	    SendMessage = KMM_SetAllConcOperation;
	    break;
	case KMNG_MESS_CHANGE_AREA_TRAP:	// トラップ内のかもめエリア変更
	    SendMessage = KMM_SetChangeAreaInTrap;
	    break;
	case KMNG_MESS_CHANGE_AREA_SPHERE:	// 半径内のかもめエリア変更
	    SendMessage = KMM_SetChangeAreaInSphere;
	    break;
	case KMNG_MESS_BLOCKELUDE_LINE:	// エルード妨害:線上指定
	    SendMessage = KMM_SetEludeOperationFromLine;
	    break;
	case KMNG_MESS_TRAP_AGGREGATE:	// 指定トラップ内にいるカモメを集合させる
	    SendMessage = KMM_SetAggreFromTrapOperation;
	    break;
	case KMNG_MESS_AREA_AGGREGATE:	// 指定エリア内にいるカモメを集合させる
	    SendMessage = KMM_SetAggreFromAreaOperation;
	    break;
	case KMNG_MESS_TAKEOFF_SPHERE:  // 指定球内緊急飛び立ち
	    SendMessage = KMM_SetTakeOffInSphere;
	    break;
	case KMNG_MESS_DEPLOY_SPHERE:  	// 指定球内緊急散開
	    SendMessage = KMM_SetDeployInSphere;
	    break;
	case KMNG_MESS_TAKEOFF_TRAP:    // 指定トラップ内緊急飛び立ち
	    SendMessage = KMM_SetTakeOffInTrap;
	    break;
	case KMNG_MESS_DEPLOY_TRAP:  	// 指定トラップ内緊急散開
	    SendMessage = KMM_SetDeployInTrap;
	    break;
	case KMNG_MESS_LAND_LINE_SPHERE:// 指定球内から搾取して線着地	 
	    SendMessage = KMM_SetLandLineInSphere;
	    break;
	case KMNG_MESS_LAND_LINE_TRAP:	// 指定トラップ内から搾取して線着地
	    SendMessage = KMM_SetLandLineInTrap;
	    break;
	case KMNG_MESS_LAND_AREA_SPHERE:// 指定球内から搾取して空間着地
	    SendMessage = KMM_SetLandAreaInSphere;
	    break;
	case KMNG_MESS_LAND_AREA_TRAP:	// 指定トラップ内から搾取して空間着地
	    SendMessage = KMM_SetLandAreaInTrap;
	    break;
	case KMNG_MESS_AREA_DEAD:	// 指定範囲内死亡
	    SendMessage = KMM_SetAreaDeadOperation;
	    break;
	case KMNG_MESS_FALL_SPEED:	// 落下速度設定
	    kmmng->nFallSpeed = msg->message[ 1 ];
	    break;
	case KMNG_MESS_ADD_Y:		// 指定範囲内高さを加算
	    SendMessage = KMM_SetYAddInAreaOperation;
	    break;
	case KMNG_MESS_CHANGE_AREA_BOX:	// 指定範囲内の飛行エリア変更
	    SendMessage = KMM_SetChgAreaInBoxOperation;
	    break;
	case KMNG_MESS_TAKEOFF_BOX:	// 指定範囲内のカモメを離散
	    SendMessage = KMM_SetTakeoffInBoxOperation;
	    break;
	case KMNG_MESS_DEPLOY_BOX:	// 指定範囲内のカモメを離散
	    SendMessage = KMM_SetDeployInBoxOperation;
	    break;
	case KMNG_MESS_CHG_ELUDEDIST:   // エルード距離
	    kmmng->nEludeDist = msg->message[ 1 ];
	    break;
	case KMNG_MESS_NUM_ATTACK:	// 指定数で攻撃
	    SendMessage = KMM_SetNumAttackOperation;
	    break;
	case KMNG_MESS_COMEON:		// キャラ付随
	    SendMessage = KMM_SetComeOnOpperation;
	    break;
	case KMNG_MESS_TAKEOFF_TURNBACK:	// 元の飛行エリアに戻るためカモメを離陸
	    SendMessage = KMM_SetTurnBackTakeOffOpperation;
	    break;
	case KMNG_MESS_COMEON_END:	// キャラ付随止める
	    SendMessage = KMM_SetConcomitantEndOpperation;
	    break;
	}

	if ( SendMessage != NULL){
	    ( *SendMessage)( kmmng, msg, code, nMess);
	}
#ifdef DEBUG_MODE
printf("Message Recieved : In Kamome Manager : NO.%d = %d\n", nMess, code);
#endif
        nMess++;
	msg++;
    }
    kmmng->nMessNum = nMess;	// メッセージ数
}

// コントロール更新
static void KMMNG_ActControl( KAMOME_MNG* kmmng)

{
    CONTROL*	ctrl;
    KAMOME* 	kamome;

#ifdef DEBUG_MODE
    if ( KMM_DbgActControl(kmmng) ){
	return;
    }
#endif

    ctrl = &kmmng->control ;
    if ( kmmng->pkmmTop != NULL){ // 先頭に付随
	MEMMOT_DATA*	pmmtData;

	kamome = kmmng->pkmmTop;
	pmmtData = kamome->mmt_ctrl.pmmtData;
	ctrl->mov = kamome->mov;
	ctrl->rot = kamome->rot;
    }
}

// フン更新
static void KMMNG_ActDanna( KAMOME_MNG* kmmng)
{
    KAMOME* pKamome;
    KAMOME* pKamomeDanna = NULL;

    // ださない条件
    // フントラップがない
    // 危険モード
    if ( kmmng->nDannaTrap == 0 || GM_AlertMode == ALERT_MODE_ALERT ) 
	return;

    // 主観フン
    if ( kmmng->nDannaPlCntr == 0 ){
	FVECTOR vec1, vec2;
	vec1.vx = GM_PlayerPosition.vx - 1000.f;
	vec1.vy = GM_PlayerPosition.vy + 1000.f;
	vec1.vz = GM_PlayerPosition.vz - 1000.f;
	vec1.vw = 1.f;
	vec2.vx = GM_PlayerPosition.vx + 1000.f;
	vec2.vy = GM_PlayerPosition.vy + 100000.f;
	vec2.vz = GM_PlayerPosition.vz + 1000.f;
	vec2.vw = 1.f;
//	NewBoundingBoxView( &vec1, &vec2, 0xff, 0x00, 0x00);
	for ( pKamome = _kamome_mng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    if ( MAO_BoundCheck( &pKamome->mov, &vec1, &vec2) ){
		FVECTOR vecPla;
		FMATRIX mat;
		extern void* NewKamomeDanna( FVECTOR* pvecInitPos );

		if ( pKamome->mov.vy >= 3000.f ){
		    DG_SetPos2( &GM_PlayerPosition, &GM_PlayerControl->rot);
		    DG_GetPos( &mat);
		    vecPla.vx = (float)KMM_GetDemoRandom( kmmng, -100, 100);
		    vecPla.vy = 5000.f;
		    vecPla.vz = (float)KMM_DemoRand( kmmng, 150);
		    vecPla.vw = 1.f;
		    _sceVu0ApplyMatrix( &vecPla, &mat, &vecPla);
		}else{
		    _sceVu0CopyVector( &vecPla, &pKamome->mov);
		}
		NewKamomeDanna( &vecPla );
		kmmng->nDannaPlCntr = COUNT_VMODE( KMM_GetDemoRandom( kmmng, 500, 750) );
		break;
	    }
	}
    }else if ( kmmng->nDannaPlCntr > 0){
	kmmng->nDannaPlCntr--;
    }

    if ( kmmng->nDannaCntr > 0){
	kmmng->nDannaCntr--;
	return;
    }
    for ( pKamome = _kamome_mng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	extern int HZX_CheckInsideTrap( HZX_GROUP_ID hzx_id, FVECTOR *pos, int name_id );

	// カウンタ更新
	if ( pKamome->usDannaWait > 0 ){
	    pKamome->usDannaWait--;
	    continue;
	}

	// フンをだすカモメを決める
	if ( pKamomeDanna == NULL ){
	    // トラップチェック
	    if ( !HZX_CheckInsideTrap( pKamome->hzx_id, &pKamome->mov, kmmng->nDannaTrap) ){
		continue;
	    }
	    pKamomeDanna = pKamome;
	}
    }    
    
    // フンを出す
    if ( pKamomeDanna != NULL){
	FVECTOR vecDanna;
	extern void* NewKamomeDanna( FVECTOR* );

	vecDanna.vx = 0.f;
	vecDanna.vy = -20.f;
	vecDanna.vz = -50.f;
	vecDanna.vw = 1.f;
	_sceVu0ApplyMatrix( &vecDanna, &pKamomeDanna->matOrg, &vecDanna);
	NewKamomeDanna( &vecDanna );

	// フンカウンタ
	kmmng->nDannaCntr = COUNT_VMODE( KMM_GetDemoRandom( kmmng, 400, 600) );
	pKamomeDanna->usDannaWait = KMM_GetDemoRandom( kmmng, 10, 12);
    }

}

// ターゲット更新
static void KMMNG_ActTarget( KAMOME_MNG* kmmng)
{
    KAMOME* pKamome;
    static FVECTOR vecSize     = { 600.f,  600.f, 600.f };	// ターゲットサイズ
    static FVECTOR vecFallSize = { 200.f,  50.f, 200.f };	// ターゲットサイズ

    { // 攻撃ターゲット更新
	FVECTOR		vecForce;	// 力積
	FMATRIX 	mat;
	TARGET*		offence;	// 攻撃用ターゲット
	POWER_TARGET*	power;

	offence = &( kmmng->attack );
	power	= &( kmmng->power );
	// 直攻撃
	pKamome = kmmng->pkmmAtcSe[0];
	if ( pKamome != NULL && pKamome->pctrlAim != NULL && pKamome->ptrgAim != NULL &&  
	     pKamome->mov.vy < pKamome->pctrlAim->mov.vy + 500.f){
	    int nSide;

	    nSide = ( pKamome->ptrgAim->side == PLAYER_SIDE ) ? PLAYER_SIDE : ENEMY_SIDE;

	    GM_SetTarget( offence, TARGET_OFFENSE , 0, nSide, &vecSize, &DG_ZeroVector ); 
	    GM_SetTargetWeaponType( offence, WP_PUNCHR ); 	// 武器タイプを設定
	    DG_SetPos2( &pKamome->mov, &pKamome->rot ) ;
	    DG_GetPos( &mat);
	    GM_MoveTarget3( offence, &mat );	// 体に追従
	    _sceVu0ScaleVector( &vecForce, (FVECTOR*)&mat.m[2][0], 100.f);
	    GM_SetPowerTarget( offence, power, POWER_ONCE, 255, 0, 3, &vecForce );
	    GM_PutTarget( offence );				   // 攻撃ターゲット配置
	    GM_TargetSetDirectAttack( offence, pKamome->ptrgAim ); // 直接攻撃ターゲットをたたき込む
	}
    }
    { // 落下攻撃ターゲット更新
	int		i, nSeed, nIndex;
	FVECTOR		vecForce;	// 力積
	FMATRIX 	mat;
	TARGET*		offence;	// 攻撃用ターゲット
	POWER_TARGET*	power;

	offence = &( kmmng->trgFall );
	power	= &( kmmng->powFall );
	// 落下攻撃
	nSeed = kmmng->time % KMM_FALL_ATC_NUM;
	for ( i = 0; i < KMM_FALL_ATC_NUM + nSeed; i++){
	    nIndex = i % KMM_FALL_ATC_NUM;
	    pKamome = kmmng->pkmmFallAtc[nIndex];
	    if ( pKamome != NULL ){
		GM_SetTarget( offence, TARGET_OFFENSE , 0, ENEMY_SIDE, &vecFallSize, &DG_ZeroVector ); 
		GM_SetTargetWeaponType( offence, WP_STAMP ); 	// 武器タイプをふみつけで設定
		DG_SetPos2( &pKamome->mov, &pKamome->rot ) ;
		DG_GetPos( &mat);
		GM_MoveTarget3( offence, &mat );	// 体に追従
		_sceVu0ScaleVector( &vecForce, (FVECTOR*)&mat.m[2][0], 100.f);
		GM_SetPowerTarget( offence, power, POWER_ONCE, 255, 0, 10, &vecForce );
		GM_PutTarget( offence );				   // 攻撃ターゲット配置

		break;
	    }
	}
	// 落下かもめターゲットワーククリア
	for ( i = 0; i < KMM_FALL_ATC_NUM; i++){
	    kmmng->pkmmFallAtc[i] = NULL;
	}
    }
}
// 音
static void KMMNG_ActSound( KAMOME_MNG* kmmng)
{
    KAMOME* pKamome;

    { // かもめ音
	int i, nSeed, nIndex, bListFull, bWingFull, bSpNum;
	FVECTOR vec1, vec2;

	bListFull = 0;
	bWingFull = 0;
	bSpNum   = 0;
	for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    vec1.vx = pKamome->mov.vx - KMM_SE_AREA;
	    vec1.vy = pKamome->mov.vy - KMM_SE_AREA;
	    vec1.vz = pKamome->mov.vz - KMM_SE_AREA;
	    vec1.vw = 1.f;
	    vec2.vx = pKamome->mov.vx + KMM_SE_AREA;
	    vec2.vy = pKamome->mov.vy + KMM_SE_AREA;
	    vec2.vz = pKamome->mov.vz + KMM_SE_AREA;
	    vec2.vw = 1.f;
	    if ( MAO_BoundCheck( &GM_PlayerPosition, &vec1, &vec2) ){
		if ( !bListFull && !KMM_AddNearSeList( pKamome->pvManager, pKamome)){ // リストがいっぱい
		    bListFull = 1;
		}
		if ( bWingFull < KMM_SE_NEAR_NUM){ // つばさ音
		    if ( pKamome->nFlag & KMM_FLAG_SE_WING_WEAK){
			KAMOME_CallSe( pKamome, KMM_SE_FLY01 + kmmng->nWingSeOffset[bWingFull], &pKamome->mov, GM_SEMODE_NORMAL);
			kmmng->nWingSeOffset[bWingFull]++;
			if (kmmng->nWingSeOffset[bWingFull] >= KMM_SE_NEAR_NUM) kmmng->nWingSeOffset[bWingFull] = 0;
			bWingFull++;
		    }else if ( pKamome->nFlag & KMM_FLAG_SE_WING_STRG){
			KAMOME_CallSe( pKamome, KMM_SE_FLY02 + kmmng->nWingSeOffset[bWingFull], &pKamome->mov, GM_SEMODE_NORMAL);
			kmmng->nWingSeOffset[bWingFull]++;
			if (kmmng->nWingSeOffset[bWingFull] >= KMM_SE_NEAR_NUM) kmmng->nWingSeOffset[bWingFull] = 0;
			bWingFull++;
		    }else if ( pKamome->nFlag & KMM_FLAG_SE_WING_FLIP){
			KAMOME_CallSe( pKamome, KMM_SE_SOR01 + BP_PS2_rand() % 2, &pKamome->mov, GM_SEMODE_NORMAL);
			bWingFull++;
		    }
		    pKamome->nWingSeCntr = KMM_GetDemoRandom( kmmng, 20, 30);
		}

		if ( bSpNum < 8){
		    if ( pKamome->nFlag & KMM_FLAG_SE_PICK){
			KAMOME_CallSe( pKamome, KMM_SE_KMOTUTUK, &pKamome->mov, GM_SEMODE_NORMAL);
		    }
		    if ( pKamome->nFlag & KMM_FLAG_SE_DOWN){
			KAMOME_CallSe( pKamome, KMM_SE_KMODWN01, &pKamome->mov, GM_SEMODE_NORMAL);
		    }
		    if ( pKamome->nFlag & KMM_FLAG_SE_WALK){
			if (BP_PS2_rand() % 2)	KAMOME_CallSe( pKamome, KMM_SE_KMOFOOTL, &pKamome->mov, GM_SEMODE_NORMAL);
			else		KAMOME_CallSe( pKamome, KMM_SE_KMOFOOTR, &pKamome->mov, GM_SEMODE_NORMAL);
		    }
		    bSpNum++;
		}
	    }
	}
	nSeed = KMM_DemoRand( kmmng, KMM_SE_NEAR_NUM );
	for ( i = 0; i < KMM_SE_NEAR_NUM; i++ ){
	    nIndex = (nSeed + i) % KMM_SE_NEAR_NUM;
	    pKamome = kmmng->pkmmNearSe[i];
	    if ( pKamome != NULL){
		if ( pKamome->nSayCntr == 0){ // 鳴き声
		    KAMOME_Say( pKamome, KMM_SE_VOX01 + nIndex, GM_SEMODE_NORMAL);
		    pKamome->nSayCntr = KMM_GetDemoRandom( kmmng, 140, 180);
		}
	    }
	}	
	KMM_ClearSeList( kmmng ); // 音リストクリア
    }
}
// カモメ動作関数コール
static void KMM_ExecKamome( KAMOME_MNG* kmmng)
{
    KAMOME* 	pKamome;

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	pKamome->ActFunc( pKamome);
    }
}

// 動作関数
static void	KMMNG_Act( kmmng )
KAMOME_MNG*	kmmng;
{
#ifdef DEBUG_MODE
    int time;
GV_SET_PRFC_CLOCK();
#endif

    GM_CurrentMap = kmmng->map;

    // リストが空になった
    if ( kmmng->pkmmTop == NULL ){
	GV_DestroyActor( kmmng ); // 破棄
	return ;
    }

    // 状況更新
    KMMNG_UpdateInfo( kmmng);

    // メッセージ受信
    KMMNG_RecieveMessage( kmmng);

    // プログラムによる思考判断 (メッセージより優先は低い)
    KMMNG_JudgeBase( kmmng);

    // コントロール更新
    KMMNG_ActControl( kmmng);

    // ターゲット更新
    KMMNG_ActTarget( kmmng);

    // フン更新
    KMMNG_ActDanna( kmmng);

    // 音更新
    KMMNG_ActSound( kmmng);

    // カモメ処理
    KMM_ExecKamome( kmmng);

#ifdef DEBUG_MODE
    KMM_DbgProcess( kmmng); // デバック処理
#endif

#ifdef DEBUG_MODE
    time = GV_GET_PRFC_CLOCK();
    KMM_DbgDumpExecTime( kmmng, time);
#endif
}

// 破棄関数
static	void	KMMNG_Die( kmmng )
KAMOME_MNG*	kmmng;
{
    KAMOME* pKamome;
    KAMOME* pKamomeCurrent;

    _kamome_mng = NULL; // 外部参照用をクリア
    PL_EludeDisturbFunc = NULL; // プレイヤーエルードチェック関数設定

    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamomeCurrent ){
	pKamomeCurrent = pKamome->pkmmNext;
	KMM_RemoveKamome( kmmng, pKamome); // リストからはずす
    }
    GV_DestroyChild( kmmng );		   // 破棄

    GM_FreeTarget( &kmmng->attack);
    GM_FreeTarget( &kmmng->trgFall);
    GM_FreeControl( &kmmng->control ) ;
    MEMMOT_FreeWork( &kmmng->mmtData );
}
// GCLパラメータ反映
enum{ // GCLからの位置指定
KMM_GCL_POS_NONE	= 0x0000,	// 指定なし
KMM_GCL_POS_ONE		= 0x0100,	// 個別指定
KMM_GCL_POS_LINE	= 0x0200,	// ライン指定
KMM_GCL_POS_AREA	= 0x0400,	// エリア指定
};
enum{ // カモメ飛行エリア指定方式
KMM_FLYAREA_TRAP  = 0x00000100,		// トラップ指定
KMM_FLYAREA_BOUND = 0x00000200,		// バウンディングボックス指定
};
// GCLからデータ取得
#define KMM_POS_RAND_RATE (0.6f)	// ランダム幅
#define KMM_TRAP_MAX	  (16)		// トラップ飛行エリア最大数
#define KMM_LINE_MAX	  (32)		// ライン指定最大数
#define KMM_AREA_MAX	  (16)		// エリア最大数
static int KMM_GetGCLData( KAMOME_MNG* kmmng )
{
    void* 	pvKamome;
    char* 	optName		= NULL;		// 名前データ
    char* 	optPNumber	= NULL;		// 通し番号パラメータ
    char* 	optPos		= NULL;		// 初期位置
    char* 	optYRot		= NULL;		// 初期Y方向
    char* 	optTrap		= NULL;		// 飛行エリアトラップ
    char* 	optArea		= NULL;		// 飛行エリア座標指定
    char* 	optIndex	= NULL;		// 飛行エリアインデックス
    char* 	optInitMot	= NULL;		// 初期モーション
    char* 	optInitThink	= NULL;		// 初期思考
    short  	bPosFlag 	= 0;
    short 	nAreaNum 	= 0;
    short 	nTrapNum 	= 0;
    short 	nNameNum	= 0;
    short 	nPNumberNum	= 0;
    short 	nPosNum		= 0;
    short 	nYRotNum	= 0;
    int 	i, nName, nYRot, nInitMot, nInitThink, nUnit, nPnum, nPflag;
    float	fRate, fAddRate;
    FVECTOR	vecPos, vecOffset, vecArea1, vecArea2;

    int		nRate = 0;

    //-------- 初期位置
    // 線指定用
    float	fOneRate[KMM_LINE_MAX];
    int 	nDist[KMM_LINE_MAX], nLineDir[KMM_LINE_MAX], nUnitNum[KMM_LINE_MAX], nUnitCurr[KMM_LINE_MAX];
    FVECTOR 	vecDir[KMM_LINE_MAX], vecFrom[KMM_LINE_MAX], vecTo[KMM_LINE_MAX];
    // 空間指定
    float	fOneAreaRate[KMM_AREA_MAX];
    int 	nAreaUnitNum[KMM_LINE_MAX], nAreaUnitCurr[KMM_LINE_MAX];
    FVECTOR 	vecInitAreaMin[KMM_AREA_MAX], vecInitAreaMax[KMM_AREA_MAX], vecInitAreaScale[KMM_AREA_MAX];
    //-------- 飛行エリア
    FVECTOR 	vecAreaMin[KMM_AREA_MAX], vecAreaMax[KMM_AREA_MAX];
    FVECTOR 	vecTrapMin[KMM_TRAP_MAX], vecTrapMax[KMM_TRAP_MAX];

    
    for ( i = 0; i < KMM_LINE_MAX; i++){
	nUnitNum[i] = 0;
	nUnitCurr[i] = 0;
	nAreaUnitNum[i] = 0;
	nAreaUnitCurr[i] = 0;
    }

    // GCLからポインタ取得
    if ( ( optName = GCL_GetOption( 'n' ) ) == NULL ){	
	printf("Kamome Manager GCL -name Not found!!\n");
	ASSERT( 0 );
    }else{
	char* optDataNum = optName;
	for ( i = 0; i < MAX_KAMOME_NUM; i++){
	    // 名前パラメータの数からデータ数取得
	    if ( optDataNum != NULL ){
		GCL_SetArgTop( optDataNum );
		nName = GCL_GetNextInt();
		optDataNum = GCL_NextStr();
		nNameNum++;
	    }else{
		break;
	    }
	}
    }

    // 位置指定用番号データ
    if ( ( optPNumber = GCL_GetOption( 'u' ) ) != NULL ){
	char* optPN = optPNumber;
	for ( i = 0; i < MAX_KAMOME_NUM; i++){
	    // 位置指定用通し番号取得
	    if ( optPN != NULL ){
		GCL_SetArgTop( optPN );
		nPnum = GCL_GetNextInt();
		optPN = GCL_NextStr();
		nPNumberNum++;
	    }else{
		nPnum = 0;
		break;
	    }
	    nUnit  = (nPnum & 0x00ff);		// インデックス番号
	    nPflag = (nPnum & 0xff00);		// 指定フラグ

	    // 同一エリア,ライン内でのカモメ数カウント
	    if ( nPflag & KMM_GCL_POS_LINE){ 	// 線指定
		nUnitNum[ nUnit ]++;
	    }else if ( nPflag & KMM_GCL_POS_AREA){ 	// 空間指定
		nAreaUnitNum[ nUnit ]++;
	    }
	}
    }

    // 位置情報
    bPosFlag = KMM_GCL_POS_NONE;
    if ( ( optPos = GCL_GetOption( 'p' ) ) != NULL ){	// 位置直指定
	bPosFlag |= KMM_GCL_POS_ONE;    
    }
    if ( ( optPos = GCL_GetOption( 'l' ) ) != NULL ){	// 位置ライン指定
	for ( i = 0; i < KMM_LINE_MAX; i++){
	    if ( ENE_GCL_GetFV( optPos, &vecFrom[i] ) != 0 )	break;
	    if ( ENE_GCL_GetFV( optPos, &vecTo[i] ) != 0 )	break;

	    MAO_GetDiffVec3( &vecDir[i], &nDist[i], &nLineDir[i], &vecFrom[i], &vecTo[i]);
	    nLineDir[i] += 1024;	// 線に垂直な方向
	    if ( nUnitNum[ i ] == 0){
		fOneRate[i] = 0.f;
	    }else{
		fOneRate[i] = (1.f / (float)nUnitNum[ i ]) * KMM_POS_RAND_RATE; // 一匹あたりの乱数レートを出す
	    }
	    nUnitCurr[i] = 0;		// 現在の数
#ifdef DEBUG_MODE
#if 0
    {
	void* NewMaoTestLine( FVECTOR*, FVECTOR*, u_char, u_char, u_char);
	GV_SetActorChild( kmmng, NewMaoTestLine( &vecFrom[i], &vecTo[i], 0xff, 0x00, 0xff));
    }
#endif
#endif
	}
	bPosFlag |= KMM_GCL_POS_LINE;    
    }
    if ( ( optPos = GCL_GetOption( 'f' ) ) != NULL ){	// 位置エリア指定
	for ( i = 0; i < KMM_AREA_MAX; i++){
	    if ( ENE_GCL_GetFV( optPos, &vecInitAreaMin[i] ) != 0 )	break;
	    if ( ENE_GCL_GetFV( optPos, &vecInitAreaMax[i] ) != 0 )	break;

	    _sceVu0SubVector( &vecInitAreaScale[i], &vecInitAreaMax[i], &vecInitAreaMin[i]);
	    
	    if ( nAreaUnitNum[ i ] == 0){
		fOneAreaRate[i] = 0.f;
	    }else{
		nRate = (int)bp_sqrtf(nAreaUnitNum[ i ]); //BP_MATH - emulate PS2 sqrtf
		fOneAreaRate[i] = (1.f / (float)nRate) * KMM_POS_RAND_RATE; // 一匹あたりの乱数レートを出す
	    }
	}
	bPosFlag |= KMM_GCL_POS_AREA;    
    }

    if ( bPosFlag == KMM_GCL_POS_NONE ){	// 位置指定が存在しない
	printf("Kamome Manager GCL PosData Not found!!\n");
	ASSERT( 0 );
    }
    
    if ( ( optYRot = GCL_GetOption( 'y' ) ) == NULL ){	// 初期Y方向	
	printf("Kamome Manager GCL -yrot Not found!!\n");
    }

    // 飛行エリアトラップ名指定型
    if ( ( optTrap = GCL_GetOption( 't' ) ) != NULL ){ 
	for ( i = 0; i < KMM_TRAP_MAX; i++){
	    int 	nTrpName;
	    HZX_BLOCK* 	blk;
	    HZX_TRP* 	trp;

	    // データ取得
	    if ( optTrap == NULL ) break;
	    GCL_SetArgTop( optTrap );
	    nTrpName = GCL_GetNextInt();
	    optTrap = GCL_NextStr();

	    HZX_FindTrap(GM_GetHzxGroupID( kmmng->map ), nTrpName, &blk, &trp);
	    ASSERT( blk != NULL);
	    ASSERT( trp != NULL);
	    vecTrapMin[i].vx = (float)(blk->tx + trp->b1.vx);
	    vecTrapMin[i].vy = (float)(blk->ty + trp->b1.vy);
	    vecTrapMin[i].vz = (float)(blk->tz + trp->b1.vz);
	    vecTrapMax[i].vx = (float)(blk->tx + trp->b2.vx);
	    vecTrapMax[i].vy = (float)(blk->ty + trp->b2.vy);
	    vecTrapMax[i].vz = (float)(blk->tz + trp->b2.vz);
	    nTrapNum++;
	}
    }else{
	printf("Kamome Manager GCL -trap Not found. \n");
	printf("Kamome Manager GCL -areadata Not found. \n");
    }
    // 飛行エリア座標指定型
    if ( ( optArea = GCL_GetOption( 'a' ) ) != NULL ){ // 飛行エリア用座標指定
	for ( i = 0; i < KMM_AREA_MAX; i++){
	    if ( ENE_GCL_GetFV( optArea, &vecAreaMin[i] ) != 0 )   break;
	    if ( ENE_GCL_GetFV( optArea, &vecAreaMax[i] ) != 0 )   break;
	    nAreaNum++;
	}
    }else{
	printf("Kamome Manager GCL -trap Not found. \n");
	printf("Kamome Manager GCL -areadata Not found. \n");
    }
    
    if ( ( optIndex = GCL_GetOption( 'i' ) ) != NULL ){ // 飛行エリア用インデックス

    }

    if ( (optInitMot = GCL_GetOption( 'm' ) ) == NULL ){ // 初期モーション
	//	printf("Kamome Manager GCL -mot Not found. \n");
    }
    if ( (optInitThink = GCL_GetOption( 'r' ) ) == NULL ){ // 初期モーション
	//	printf("Kamome Manager GCL -routine Not found. \n");
    }

    // データ反映
    kmmng->nKamomeNum = 0;
    for ( i = 0; i < MAX_KAMOME_NUM; i++){
	// 名前取得
	if ( optName != NULL ){
	    GCL_SetArgTop( optName );
	    nName = GCL_GetNextInt();
	    optName = GCL_NextStr();
	}else{
	    break;
	}
	// 位置指定用フラグ取得
	if ( optPNumber != NULL ){
	    GCL_SetArgTop( optPNumber );
	    nPnum = GCL_GetNextInt();
	    optPNumber = GCL_NextStr();
	}else{
	    nPnum = 0;
	}

	nUnit  = (nPnum & 0x00ff);		// インデックス番号
	nPflag = (nPnum & 0xff00);		// 指定フラグ
	if ( nPflag == 0) nPflag = bPosFlag; 	// 指定がないとき


	nYRot = 0;	// 初期Y方向
//printf("%d ::: %d : 0x%04x  0x%04x\n", i, nUnit, nPflag, (nPnum & 0xff00));
	// 位置取得
	if ( nPflag & KMM_GCL_POS_ONE){ 	// 座標指定
	    if ( optPos != NULL ){
		GCL_SetArgTop( optPos );
		if ( ENE_GCL_GetFV( optPos, &vecPos ) == 0 ){
		    nPosNum++;
		}
		optPos = GCL_NextStr();
	    }

	    if ( optYRot != NULL ){ // 初期Y方向
		GCL_SetArgTop( optYRot );
		nYRot = GCL_GetNextInt();
		if ( nYRot < 0 ) nYRot = KMM_GetDemoRandom( kmmng, 0, 4096);
		optYRot = GCL_NextStr();
		nYRotNum++;
	    }else{
		nYRot = KMM_GetDemoRandom( kmmng, 0, 4096);
	    }
	}else if ( nPflag & KMM_GCL_POS_LINE){ 	// 線上指定
	    if ( nUnitCurr[nUnit] > 0 && nUnitCurr[nUnit] < nUnitNum[nUnit]){
		fAddRate = GM_FRnd( &kmmng->nSeed) * fOneRate[nUnit];
	    }else{
		fAddRate = 0.f;
	    }
	    if ( nUnitNum[nUnit] != 0 ){
		fRate = ((float)nUnitCurr[nUnit] / (float)nUnitNum[nUnit] + fAddRate);
	    }else{
		fRate = 0.f;
	    }

	    _sceVu0ScaleVector( &vecOffset, &vecDir[nUnit], fRate * (float)nDist[nUnit]);
	    _sceVu0AddVector( &vecPos, &vecFrom[nUnit], &vecOffset);

	    if ( optYRot != NULL ){ // 初期Y方向
		GCL_SetArgTop( optYRot );
		nYRot = GCL_GetNextInt();
		if ( nYRot < 0 ){
		    nYRot = nLineDir[nUnit] + KMM_GetDemoRandom( kmmng, -400, 400) + 
			((KMM_DemoRand( kmmng, 2) == 0)? 1024 : 3072);
		}
		optYRot = GCL_NextStr();
		nYRotNum++;
	    }else{
		nYRot = nLineDir[nUnit] + KMM_GetDemoRandom( kmmng, -400, 400) + 
			((KMM_DemoRand( kmmng, 2) == 0)? 1024 : 3072);
	    }
	    nUnitCurr[nUnit]++;
	}else if ( nPflag & KMM_GCL_POS_AREA){ 	// エリア指定
	    vecPos.vx = (float)KMM_GetDemoRandom( kmmng, (int)vecInitAreaMin[nUnit].vx, (int)vecInitAreaMax[nUnit].vx);
	    vecPos.vy = (float)KMM_GetDemoRandom( kmmng, (int)vecInitAreaMin[nUnit].vy, (int)vecInitAreaMax[nUnit].vy);
	    vecPos.vz = (float)KMM_GetDemoRandom( kmmng, (int)vecInitAreaMin[nUnit].vz, (int)vecInitAreaMax[nUnit].vz);
	    vecPos.vw = 1.f;

	    if ( optYRot != NULL ){ // 初期Y方向
		GCL_SetArgTop( optYRot );
		nYRot = GCL_GetNextInt();
		if ( nYRot < 0 ) nYRot = KMM_GetDemoRandom( kmmng, 0, 4096);
		optYRot = GCL_NextStr();
		nYRotNum++;
	    }else{
		nYRot = KMM_GetDemoRandom( kmmng, 0, 4096);
	    }

	    nAreaUnitCurr[nUnit]++;
	}

	// 飛行エリア用設定
	if ( optIndex != NULL ){
	    int nAreaIndex;
	    int nIndexData;
	    GCL_SetArgTop( optIndex );
	    nIndexData = GCL_GetNextInt();
	    optIndex = GCL_NextStr();

	    nAreaIndex = (nIndexData & 0xff);	// インデックス取得
	    if ( nIndexData & KMM_FLYAREA_TRAP ){ // トラップ指定	
#ifdef DEBUG_MODE
		if ( nAreaIndex >= nTrapNum ){
		    MAO_PRINTF("flying-area[trap] index over flow!!");
		    ASSERT(0);
		}
#endif
		vecArea1 = vecTrapMin[nAreaIndex];
		vecArea2 = vecTrapMax[nAreaIndex];
	    }else if ( nIndexData & KMM_FLYAREA_BOUND ){ // バウンディング指定
#ifdef DEBUG_MODE
		if ( nAreaIndex >= nAreaNum ){
		    MAO_PRINTF("flying-area[area] index over flow!!");
		    ASSERT(0);
		}
#endif
		vecArea1 = vecAreaMin[nAreaIndex];
		vecArea2 = vecAreaMax[nAreaIndex];
	    }else{
		if ( nTrapNum != 0){
		    vecArea1 = vecTrapMin[0];
		    vecArea2 = vecTrapMax[0];
		}else if ( nAreaNum != 0){
#ifdef DEBUG_MODE
		    if ( nAreaIndex >= nAreaNum ){
			MAO_PRINTF("flying-area[area] index over flow!!");
			ASSERT(0);
		    }
#endif
		    vecArea1 = vecAreaMin[nAreaIndex];
		    vecArea2 = vecAreaMax[nAreaIndex];
		}else{
		    vecArea1.vx = vecPos.vx - 500;
		    vecArea1.vy = vecPos.vy - 500;
		    vecArea1.vz = vecPos.vz - 500;
		    vecArea2.vx = vecPos.vx + 500;
		    vecArea2.vy = vecPos.vy + 500;
		    vecArea2.vz = vecPos.vz + 500;
		}
	    }
	}else{
	    if ( nTrapNum != 0){
		vecArea1 = vecTrapMin[0];
		vecArea2 = vecTrapMax[0];
	    }else if ( nAreaNum != 0){
		vecArea1 = vecAreaMin[0];
		vecArea2 = vecAreaMax[0];
	    }else{
		vecArea1.vx = vecPos.vx - 500;
		vecArea1.vy = vecPos.vy - 500;
		vecArea1.vz = vecPos.vz - 500;
		vecArea2.vx = vecPos.vx + 500;
		vecArea2.vy = vecPos.vy + 500;
		vecArea2.vz = vecPos.vz + 500;
	    }
	}

	if ( optInitMot != NULL ){ // 初期モーション
	    GCL_SetArgTop( optInitMot );
	    nInitMot = GCL_GetNextInt();
	    optInitMot = GCL_NextStr();
	}else{
	    nInitMot = KMM_MOT_IDLE;
	}

	if ( optInitThink != NULL ){ // 初期思考
	    GCL_SetArgTop( optInitThink );
	    nInitThink = GCL_GetNextInt();
	    optInitThink = GCL_NextStr();
	}else{
	    nInitThink = KMM_INITTHK_REST;
	}

	// カモメ呼出
	GV_SetActorChild( kmmng, (pvKamome = 
                NewKamomeTestProg( nName, kmmng->map, &vecPos, nYRot, nInitMot, nInitThink,
				   kmmng, kmmng->nKamomeNum, &vecArea1, &vecArea2 )) );
	
	if ( pvKamome == NULL){ // 失敗
#ifdef DEBUG_MODE 
	    MAO_PRINTF("Cannot Create Kamome memory allocation failed!!!\n");
	    ASSERT(0);
#endif
	    return -1;
	}

	// リストに挿入
	KMM_InsertKamome( kmmng, pvKamome);
	
	// かもめ数更新
	kmmng->nKamomeNum++;
    }
    kmmng->nInitKmmNum = kmmng->nKamomeNum;	// カモメ数を記憶
#ifdef DEBUG_MODE
printf("----------------------------------------------------------\n");
printf("----- Kamome Manager GCL Param -----\n");
printf("NAME  	 = %d\n", nNameNum);
printf("P-Number = %d\n", nPNumberNum);
printf("POS    	 = %d\n", nPosNum);
printf("YROT     = %d\n", nYRotNum);
printf("AREA     = %d\n", nAreaNum);
printf("TRAP     = %d\n", nTrapNum);
printf("----------------------------------------------------------\n");
printf("Kamome call end : %d\n", kmmng->nKamomeNum);
printf("----------------------------------------------------------\n");
#endif

    return 0;
}

// 初期化
static	int	KMMNG_GetResources( kmmng, name, where )
KAMOME_MNG*	kmmng;
int	name;
int	where;
{	
    int 	i;
    char* 	optMdl		= NULL;		// モデル名データ

    kmmng->name = name;
    kmmng->map  = where;
    kmmng->time = 0;

    kmmng->nPrevInfo = 0;
    kmmng->nInfo     = 0;

    kmmng->bPigeon = GCL_GetOptionValue( 'q', 0 ); // ハトフラグ

    // ランダムシード
    if ( GCL_GetOption( 's' ) != NULL ){
	kmmng->bSeedFlag = 1;
	kmmng->nSeed = GCL_GetNextInt();
    }else{
	kmmng->bSeedFlag = 0;
	kmmng->nSeed = BP_PS2_rand();
    }
#ifdef DEBUG_MODE
    printf("Rand Seed set for Kamome = %d\n", kmmng->nSeed);
#endif

    // 基本ルーチン
    if ( GCL_GetOption( 'b' ) != NULL ){
	kmmng->nBaseRoutine = GCL_GetNextInt();
    }else{
	kmmng->nBaseRoutine = KMNG_BASE_NORMAL;
    }
#ifdef DEBUG_MODE
    printf("Kamome Base Routine = %d\n", kmmng->nBaseRoutine);
#endif

    // 速度
    if ( GCL_GetOption( 'z' ) != NULL ){
	kmmng->fBaseSpeed = (float)GCL_GetNextInt();
    }else{
	kmmng->fBaseSpeed = KMM_SPEED_DEF;
    }
    // エルード判定距離
    kmmng->nEludeDist = ELUDE_DIST_CHECK;
    // 落下速度
    if ( GCL_GetOption( 'g' ) != NULL ){
	kmmng->nFallSpeed = GCL_GetNextInt();
    }else{
	kmmng->nFallSpeed = KMM_FALL_SPD;
    }
    // 消去する高さ
    if ( GCL_GetOption( 'e' ) != NULL ){
	kmmng->nDeadHeight = GCL_GetNextInt();
    }else{
	kmmng->nDeadHeight = (int)GM_WaterLevel; // KMM_LIMIT_HEIGHT;
    }
#ifdef DEBUG_MODE
    printf("kamome kill height = %d :: %d\n", kmmng->nDeadHeight, (int)GM_WaterLevel);
#endif
    
    // カモメがダメージを受けた時に呼ぶプロシージャを取得 ない時は 0 
    kmmng->procDamage = GCL_GetOptionValue( 'w', 0 );

    // GCLからLOD切替え距離を設定
    kmmng->nLodDist = GCL_GetOptionValue( 'x', 10000 );

    // GCLからモデル名取得
    if ( ( optMdl = GCL_GetOption( 'd' ) ) != NULL ){	
	for ( i = 0; i < KMM_MDLNAME_MAX; i++ ){
	    if ( optMdl != NULL ){
		GCL_SetArgTop( optMdl );
		kmmng->nMdlName[i] = GCL_GetNextInt();
		optMdl = GCL_NextStr();
//printf("kamome model name = %d\n", kmmng->nMdlName[i]);
                if ( kmmng->nMdlName[i] == 2185720 ){
		    kmmng->nMdlName[i] = 8885877; // kmo_low -> kmo_noalp
		}
	    }else{
		kmmng->nMdlName[i] = 0;
	    }
	}
    }else{
	printf("Kamome Manager GCL -datamdl Not found!!\n");
	for ( i = 0; i < KMM_MDLNAME_MAX; i++ ){
	    kmmng->nMdlName[i] = MODEL_NAME;
	}
    }

    // GCLからフンをだしてよいトラップを取得
    kmmng->nDannaTrap 	= GCL_GetOptionValue( 'c', 0 );
    kmmng->nDannaCntr 	= COUNT_VMODE( KMM_GetDemoRandom( kmmng, 90, 120) );
    kmmng->nDannaPlCntr	= COUNT_VMODE( KMM_GetDemoRandom( kmmng, 90, 120) );

    {
	CONTROL*	ctrl ;
    
	ctrl = &kmmng->control;
	GM_InitControl( ctrl, name, 0 ) ;
	ctrl->hzx_height = 750 ;
	ctrl->height = 1049.0F ;
	GM_ConfigControlHazard( ctrl, 1200, 450, 500 ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlMessageCheck( ctrl ) ;

	GM_ConfigControlAddressCheck( ctrl ) ;
	GM_ConfigControlMapCheck( ctrl ) ;

	ctrl->seg_flag |= HZX_TYPE_ENEMY ;
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

    }
    // メモリモーション初期設定
    if ( MEMMOT_MotionPreCalloc( &kmmng->mmtData, BASE_MOTION, kmmng->nMdlName[KMM_MDLNAME_NORM]) < 0){
printf("Memmot Cannot Alloc Memory!!\n");
	return -1;
    }

    // リスト初期設定
    kmmng->pkmmTop = kmmng->pkmmEnd = NULL;

    // GCLデータ読み込み＆カモメ呼出
    if ( KMM_GetGCLData( kmmng ) < 0){
printf("KMMNG Cannot Alloc Memory!!\n");
	return -1;
    }

    // SE管理初期化
    KMM_ClearSeList( kmmng );

    kmmng->nCboxCntr = 0;
    kmmng->nCboxNum  = 0;
    kmmng->nAngryCntr = 0;
    kmmng->nAngryLimit = 0;

    // メッセージ
    KMMNG_RecieveMessage( kmmng);

#ifdef DEBUG_MODE
    printf("struct KAMOME     work size = %d(byte)\n", sizeof(KAMOME));
    printf("struct KAMOME_MNG work size = %d(byte)\n", sizeof(KAMOME_MNG));

    KMM_InitDebugWork( kmmng);
#endif

    _kamome_mng = kmmng;	// 外部参照用設定
    PL_EludeDisturbFunc = KMMNG_CheckKamomeIn; // プレイヤーエルードチェック関数設定

    return 0;
}

// シナリオ呼出:カモメさんテスト
void*	NewKamomeManager( name, where )
int	name;
int	where;
{
    KAMOME_MNG*	kmmng;

    OPERATOR();
    kmmng = (KAMOME_MNG *)GV_NewActor(  GV_ACTOR_USER, sizeof( KAMOME_MNG ) );
    if ( kmmng != NULL ) {
	GV_SetActor( &( kmmng->actor ), KMMNG_Act, KMMNG_Die );
	GV_ActorEX( &kmmng->actor );
	if ( KMMNG_GetResources( kmmng, name, where ) < 0 ) {
	    GV_DestroyActor( kmmng );
	    return NULL;
	}
    }

    return kmmng;
}

// カモメ非表示
int NewCmdKamomeInvisible( void )
{	
    KAMOME_MNG* kmmng = _kamome_mng;
    
    if ( kmmng != NULL ){ // 存在する
	KAMOME*  pKamome;

	for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    if ( pKamome->body.objs != NULL) 	
		SET_FLAG( pKamome->body.flag, DG_FLAG_INVISIBLE);
	    if ( pKamome->bodyLodStop.objs != NULL) 	
		SET_FLAG( pKamome->bodyLodStop.flag, DG_FLAG_INVISIBLE);
	    if ( pKamome->bodyLodFlying.objs != NULL) 	
		SET_FLAG( pKamome->bodyLodFlying.flag, DG_FLAG_INVISIBLE);
	}
    }

    return 0;
}

// 死んだカモメ非表示
int NewCmdDeadKamomeInvisible( void )
{	
    KAMOME_MNG* kmmng = _kamome_mng;
    
    if ( kmmng != NULL ){ // 存在する
	KAMOME*  pKamome;

	for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    if ( pKamome->think.nThink3 < TH3_CHECK_FEINT_START ) continue;
	    if ( pKamome->body.objs != NULL) 	
		SET_FLAG( pKamome->body.flag, DG_FLAG_INVISIBLE);
	    if ( pKamome->bodyLodStop.objs != NULL) 	
		SET_FLAG( pKamome->bodyLodStop.flag, DG_FLAG_INVISIBLE);
	    if ( pKamome->bodyLodFlying.objs != NULL) 	
		SET_FLAG( pKamome->bodyLodFlying.flag, DG_FLAG_INVISIBLE);
	}
    }

    return 0;
}

// カモメ表示
int NewCmdKamomeVisible( void )
{	
    KAMOME_MNG* kmmng = _kamome_mng;
    
    if ( kmmng != NULL ){ // 存在する
	KAMOME*  pKamome;

	for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    if ( pKamome->body.objs != NULL) 	
		UNSET_FLAG( pKamome->body.flag, DG_FLAG_INVISIBLE);
	    if ( pKamome->bodyLodStop.objs != NULL) 	
		UNSET_FLAG( pKamome->bodyLodStop.flag, DG_FLAG_INVISIBLE);
	    if ( pKamome->bodyLodFlying.objs != NULL) 	
		UNSET_FLAG( pKamome->bodyLodFlying.flag, DG_FLAG_INVISIBLE);
	}
    }

    return 0;
}

// 付随カモメの数を取得
int GetConcKamomeNum( void )
{
    KAMOME_MNG* kmmng = _kamome_mng;
    int nNum = 0;

    if ( kmmng != NULL ){ // 存在する
	KAMOME*  pKamome;

	for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    if ( pKamome->think.nThink3 == TH3_CTRL_LANDING || 
		 pKamome->think.nThink3 == TH3_CTRL_REST ) 
		nNum++;
	}
    }

    return (nNum);
}
