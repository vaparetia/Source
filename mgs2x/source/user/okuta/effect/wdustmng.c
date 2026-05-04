//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    wdustmng.c
    水中ゴミマネージャ
    2001/07/21 Masafumi Okuta
    $Id: wdustmng.c,v 1.1.1.3 2002/11/19 11:47:55 Yoshizawa1 Exp $
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
#include	"wdust.h"

// 表示ワークの更新
static  void UpdateViewList( WDUST_MNG *work )
{
    int i, j, t;
    FMATRIX mat;
    int nViewNum = 0;
    int	nViewFlagTbl[N_POS];	// 表示フラグテーブル
    int	nAlreadyFlagTbl[N_POS];	// 存在フラグテーブル
    int	nViewList[N_POS];	// 表示リスト
    int	nViewDist[N_POS];	// 距離リスト

    // 表示チェック
    for ( i = 0; i < work->nDataNum; i++ ){
	int	nCamDist;
	FVECTOR vecB1, vecB2;
	extern int KR_CameraDis( FVECTOR* pos );

	DG_SetPos2( &work->vecPosTbl[i], &DG_ZeroSVector);
	DG_GetPos( &mat);

	vecB1.vx = work->vecSizeTbl[i].vx;
	vecB1.vy = work->vecSizeTbl[i].vy;
	vecB1.vz = work->vecSizeTbl[i].vz;
	vecB1.vw = 1.f;
	vecB2.vx = -work->vecSizeTbl[i].vx;
	vecB2.vy = -work->vecSizeTbl[i].vy;
	vecB2.vz = -work->vecSizeTbl[i].vz;
	vecB2.vw = 1.f;

	if ( DG_BoundCheck( &mat, &vecB1, &vecB2 ) != -1 && 
	     ( nCamDist = KR_CameraDis( &work->vecPosTbl[i]) ) < VISIBLE_CAM_DIST ){ // 表示チェック
	    nViewFlagTbl[i] = 1;	
	    nViewList[ nViewNum ] = i;
	    nViewDist[ nViewNum ] = nCamDist;
	    nViewNum++;
	}else{
	    nViewFlagTbl[i] = 0;
	}
    }

    // 存在チェック
    for ( i = 0; i < work->nDataNum; i++ ){
	nAlreadyFlagTbl[i] = 0; 
    }
    for ( i = 0; i < work->nWorkNum; i++ ){
	if ( work->nIndexTbl[i] >= 0 ) nAlreadyFlagTbl[work->nIndexTbl[i]] = 1; 
    }

    // 表示リスト生成
    for ( i = 1; i < nViewNum; i++ ){ // 挿入ソート
	j = i;
	while ( j >= 1 && nViewDist[j-1] > nViewDist[j] ){
	    // リスト更新
	    t = nViewList[j];
	    nViewList[j] = nViewList[j-1];
	    nViewList[j-1] = t;
	    // 距離更新
	    t = nViewDist[j];
	    nViewDist[j] = nViewDist[j-1];
	    nViewDist[j-1] = t;
	    j--;
	}    
    }

    // ワーク更新
    for ( i = 0; i < work->nWorkNum; i++ ){
	work->nPrevIndex[i] = work->nIndexTbl[i]; // インデックスを保存
	if ( work->nIndexTbl[i] < 0 || !nViewFlagTbl[ work->nIndexTbl[i] ] ){ // 非表示だったor非表示になった
	    work->nIndexTbl[i] = -1;	// 表示を止める
	    for ( j = 0; j < nViewNum; j++ ){ 
		if ( !nAlreadyFlagTbl[ nViewList[j] ] ){ // 表示可能がいる
		    work->nIndexTbl[i] = nViewList[j];
		    nAlreadyFlagTbl[ nViewList[j] ] = 1;
		    break;
		}
	    }
	}
    }
}
// プレイヤーの関節位置を記憶のダミー版
static void UpdatePlayerDataDummy( WDUST_MNG* work )
{
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[0][0], &DG_ZeroVector);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[1][0], &DG_ZeroVector);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[2][0], &DG_ZeroVector);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[3][0], &DG_ZeroVector);

    // 手と足の移動ベクトルを取得
    _sceVu0CopyVector( (FVECTOR*)&work->matPlMove.m[0][0], &DG_ZeroVector);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlMove.m[1][0], &DG_ZeroVector);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlMove.m[2][0], &DG_ZeroVector);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlMove.m[3][0], &DG_ZeroVector);

    // 手と足それぞれの移動量を取得
    work->vecPlMoveDist.vx = 0.f;
    work->vecPlMoveDist.vy = 0.f;
    work->vecPlMoveDist.vz = 0.f;
    work->vecPlMoveDist.vw = 0.f;
}
// プレイヤーの関節位置を記憶
static void UpdatePlayerData( WDUST_MNG* work )
{
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[0][0], (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE).m[3][0]);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[1][0], (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_MIGI_TE).m[3][0]);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[2][0], (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_KAKATO).m[3][0]);
    _sceVu0CopyVector( (FVECTOR*)&work->matPlNode.m[3][0], (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_MIGI_KAKATO).m[3][0]);

    // 手と足の移動ベクトルを取得
    _sceVu0SubVector( (FVECTOR*)&work->matPlMove.m[0][0], (FVECTOR*)&work->matPlNode.m[0][0], (FVECTOR*)&work->matPlNodePre.m[0][0] );
    _sceVu0SubVector( (FVECTOR*)&work->matPlMove.m[1][0], (FVECTOR*)&work->matPlNode.m[1][0], (FVECTOR*)&work->matPlNodePre.m[1][0] );
    _sceVu0SubVector( (FVECTOR*)&work->matPlMove.m[2][0], (FVECTOR*)&work->matPlNode.m[2][0], (FVECTOR*)&work->matPlNodePre.m[2][0] );
    _sceVu0SubVector( (FVECTOR*)&work->matPlMove.m[3][0], (FVECTOR*)&work->matPlNode.m[3][0], (FVECTOR*)&work->matPlNodePre.m[3][0] );

    // 手と足それぞれの移動量を取得
    work->vecPlMoveDist.vx = _MAO_FVecLen3( (FVECTOR*)&work->matPlMove.m[0][0] );
    work->vecPlMoveDist.vy = _MAO_FVecLen3( (FVECTOR*)&work->matPlMove.m[1][0] );
    work->vecPlMoveDist.vz = _MAO_FVecLen3( (FVECTOR*)&work->matPlMove.m[2][0] );
    work->vecPlMoveDist.vw = _MAO_FVecLen3( (FVECTOR*)&work->matPlMove.m[3][0] );

    // 正規化
    _sceVu0Normalize( (FVECTOR*)&work->matPlMove.m[0][0], (FVECTOR*)&work->matPlMove.m[0][0]);
    _sceVu0Normalize( (FVECTOR*)&work->matPlMove.m[1][0], (FVECTOR*)&work->matPlMove.m[1][0]);
    _sceVu0Normalize( (FVECTOR*)&work->matPlMove.m[2][0], (FVECTOR*)&work->matPlMove.m[2][0]);
    _sceVu0Normalize( (FVECTOR*)&work->matPlMove.m[3][0], (FVECTOR*)&work->matPlMove.m[3][0]);

}
// プレイヤーの関節位置を記憶
static inline void GetPlayerPos( WDUST_MNG* work )
{
#ifdef BP_PSX2_ASM // yano

    asm volatile ("
        lq	$4, 0x00(%1)	# 左手
        lq	$5, 0x10(%1)	# 右手
        lq	$6, 0x20(%1)	# 左足
        lq	$7, 0x30(%1)	# 右足
        sq	$4, 0x00(%0)	# 1フレーム前の左手
        sq	$5, 0x10(%0)	# 1フレーム前の右手
        sq	$6, 0x20(%0)	# 1フレーム前の左足
        sq	$7, 0x30(%0)	# 1フレーム前の右足
    ":: "r"((FVECTOR*)&work->matPlNodePre), "r"((FVECTOR*)&work->matPlNode) : "$4", "$5", "$6", "$7");
#else
	_sceVu0CopyVector( &work->matPlNodePre.m[0], &work->matPlNode.m[0] );
	_sceVu0CopyVector( &work->matPlNodePre.m[1], &work->matPlNode.m[1] );
	_sceVu0CopyVector( &work->matPlNodePre.m[2], &work->matPlNode.m[2] );
	_sceVu0CopyVector( &work->matPlNodePre.m[3], &work->matPlNode.m[3] );
#endif
}

// 動作関数
static	void Act( WDUST_MNG *work )
{
    // プレイヤーデータ取得
    GetPlayerPos( work);

    if ( GM_PlayerBody != NULL) UpdatePlayerData( work);
    else			UpdatePlayerDataDummy( work);

    // リスト更新
    UpdateViewList( work );
}

// 破棄関数
static	void Die( WDUST_MNG *work )
{

}

// シナリオ呼出時初期化
static int GetResources( WDUST_MNG* work, int name, int map )
{
    int i;
    char* opt;
    extern int ENE_GCL_GetFV( char *ptr, FVECTOR *fvec );

    work->name   = name;
    work->map    = map;
    work->hzx_id = GM_GetHzxGroupID( GM_CurrentStageMap ); 	

    work->nWorkNum = GCL_GetOptionValue( 'u', 4 ); // ユニット数
    if ( work->nWorkNum > N_UNIT ){
	work->nWorkNum = N_UNIT;
    }

    work->nPrimNum = GCL_GetOptionValue( 'p', 32 ); // プリミティブ数
    if ( work->nPrimNum > N_PRIM ){
	work->nPrimNum = N_PRIM;
    }

    if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){  // 中心座標取得
	work->nDataNum = 0;
	for ( i = 0; i < N_POS; i++ ){
	    if ( ENE_GCL_GetFV( opt, &work->vecPosTbl[i] ) != 0 ){
		 _sceVu0CopyVector( &work->vecPosTbl[i], &DG_ZeroVector);
	    }else{
		work->nDataNum++;
	    }
	}
    }else{
#ifdef DEBUG_MODE
	MAO_PRINTF("Water DustManager GCL opt 'pos' not found!!");
	ASSERT(0);
#endif
    }	

    if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){	// サイズ取得
	for ( i = 0; i < N_POS; i++ ){
	    if ( ENE_GCL_GetFV( opt, &work->vecSizeTbl[i] ) != 0 ){
		 _sceVu0CopyVector( &work->vecSizeTbl[i], &DG_ZeroVector);
	    }
	}
    }else{
#ifdef DEBUG_MODE
	MAO_PRINTF("Water DustManager GCL opt 'size' not found!!");
	ASSERT(0);
#endif
    }	

    for ( i = 0; i < N_POS; i++ ){
	if ( i < work->nDataNum ){
	    _sceVu0SubVector(&work->vecBoundMinTbl[i], &work->vecPosTbl[i], &work->vecSizeTbl[i]);
	    _sceVu0AddVector(&work->vecBoundMaxTbl[i], &work->vecPosTbl[i], &work->vecSizeTbl[i]);
	}else{
	    _sceVu0CopyVector(&work->vecBoundMinTbl[i], &DG_ZeroVector);
	    _sceVu0CopyVector(&work->vecBoundMaxTbl[i], &DG_ZeroVector);
	}
    }

    // リスト更新
    UpdateViewList( work );

    {
	for ( i = 0; i < work->nWorkNum; i++ ){
	    extern void* NewWaterDustProg( int, int, int, FVECTOR*, FVECTOR*, WDUST_MNG*, int);
	    work->nPrevIndex[i] = -1;
	    if ( work->nIndexTbl[i] < 0 ){
		NewWaterDustProg( GV_StrCode("水中ゴミ"), work->map, work->nPrimNum, 
				  &work->vecPosTbl[ 0 ], &work->vecSizeTbl[ 0 ], work, i );
	    }else{
		NewWaterDustProg( GV_StrCode("水中ゴミ"), work->map, work->nPrimNum, 
				  &work->vecPosTbl[ work->nIndexTbl[i] ], &work->vecSizeTbl[ work->nIndexTbl[i] ], work, i );
	    }
	}
    }

    // プレイヤーデータ取得
    if ( GM_PlayerBody != NULL) UpdatePlayerData( work);
    else			UpdatePlayerDataDummy( work);

    GetPlayerPos( work);
    
    return 0;
}

// 水中ゴミマネージャ
void* NewWaterDustManager( name, where )
int	name;
int	where;
{
    WDUST_MNG*	work;

    work = (WDUST_MNG *)GV_NewActor( GV_ACTOR_AFTER, sizeof( WDUST_MNG ) ) ;

    if( work != NULL){
	GV_SetActor( &(work->actor), Act, Die) ;
	GV_ActorEX( &work->actor );
	if( GetResources( work, name, where ) < 0){
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }				

    return (void *)work;
}
