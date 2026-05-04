//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmfeather.c
    かもめの羽根
    2001/07/06 Masafumi Okuta
    $Id: kmfeather.c,v 1.1.1.3 2002/11/19 11:47:51 Yoshizawa1 Exp $
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
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"

#include "BP_Misc.h"

#ifdef DEBUG_MODE
//#define CALC_ACT_TIME
#endif

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))

// チェックしないフロア
#define KMF_HZX_CHK_TYPE	(HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR)
//#define KMF_HZX_PLAYER_ONLY	(HZX_FLOOR_IK|HZX_FLOOR_NO_PLAYER)
#define KMF_HZX_PLAYER_ONLY	(HZX_CHK_NOCHECK_FLOOR)

#define MODEL_NAME		(2688585)	// モデル名 kmo_hane.mdl

#define LIMIT_LIST_NUM		(16)		// 同時発生最大数

#define INIT_POS_RAND		(80.f)		// 初期位置ランダム幅
#define INIT_STEP_BASE		(70.f)		// 初期移動量(基本)
#define INIT_STEP_ADD		(50.f)		// 初期移動量(加算)

#define INIT_SPEED_MIN		(75)		// 初期スピード
#define INIT_SPEED_MAX		(100)		// 初期スピード
#define AIM_SPEED_MIN		(10)		// 目標スピード
#define AIM_SPEED_MAX		(20)		// 目標スピード

#define ROT_SPEED_MIN		(12)		// 回転最小
#define ROT_SPEED_MAX		(24)		// 回転最大

#define BASE_SPEED_BASE		(1.f)		// 基本速度(基本)
#define BASE_SPEED_ADD		(0.2f)		// 基本速度(加算)

#define SCALE_BASE		(1.f)		// スケーリング(基本)
#define SCALE_ADD		(2.f)		// スケーリング(加算)

#define DELAY_MIN		(20)		// 直進時間(最小)
#define DELAY_MAX		(30)		// 直進時間(最大)

#define LIFE_MIN		(400)		// 生存時間(最小)
#define LIFE_MAX		(600)		// 生存時間(最大)

#define KMF_GRAVITY 		(-9.8f)		// 重力
#define KMF_BIND		( 3.f)		// 揚力

#define COL_R			(128)		// R 128
#define COL_G			(128)		// G 128
#define COL_B			(128)		// B 128
#define COL_A			(128)		// Alpha

#define KMF_HZX_CHECK_INTV	(COUNT_VMODE(30))	// ハザードチェック時間

// スクラッチパッド
#define	SCR_POS		(SCRPAD_ADDR + 0x1000)	// 位置
#define	SCR_STEP	(SCRPAD_ADDR + 0x2000)	// ステップ
#define	SCR_ROT		(SCRPAD_ADDR + 0x3000)	// 回転

// 外部関数
extern int  	DG_QueueComdlObjs( DG_COMDL* );
extern void 	DG_DequeueComdlObjs( DG_COMDL* );
extern float 	_TS_Sin( int );
extern void 	TS_VecToRot( SVECTOR*, FVECTOR* );
extern float 	_TS_Sin( int );
extern int 	OK_GetLocalWind_SH( FVECTOR*, FVECTOR* );
extern void* 	NewRippleStrip( FVECTOR*, float, int );

extern FVECTOR 	G_wind;
extern int 	G_wind_intense;
extern int 	G_wind_intense_max;


enum{
KMFEATHER_NOT_GROUND,		// 接地していない
KMFEATHER_GROUND,		// 接地している
KMFEATHER_WATER,		// 着水している
};

typedef struct _KMM_FEATHER{
    // 基本システム
    GV_ACT_EX			actor;
    HZX_GROUP_ID		map_id;			// マップID
    
    // 管理
    u_short			usFeatherNum;		// 羽根数
    u_short			usLife;			// 生存時間
    FVECTOR			vecCenter;		// 中心
    FVECTOR			vecColor;		// 色
    int				nRandSeed;		// ランダムシード

    // 羽根用ワーク
    FMATRIX*			matOrg;			// 移動用マトリクス
    SVECTOR*			vecRot;			// 回転
    SVECTOR*			vecRotSpd;		// 角速度
    SVECTOR*			vecRotAim;		// 角速度(補間先)
    FVECTOR*			vecSpeed;		// 移動ベクトル
    FVECTOR*			vecBound;		// バウンドボックス
    FVECTOR*			vecWind;		// 風計算用
    u_char*			ucFlrFlag;		// 接地
    u_char*			ucUpperFlag;		// 上昇フラグ
    u_short*			nDelay;			// 影響ディレイ
    u_short*			usOneLife;		// 一枚毎の生存時間
    float*			fSpeedZ;		// スピード
    float*			fAimSpdZ;		// 目標速度	
    float*			fScale;			// 拡大率
    int*			nHeight;		// 高さ
    
    // 描画系
    DG_COMDL*			comdl;			// co-model

    // 管理リスト
    struct _KMM_FEATHER* 	pworkPrev;		// リスト:一つ前
    struct _KMM_FEATHER* 	pworkNext;		// リスト:一つ後
}Work;

// リスト用
static Work* _pworkTop = NULL;
static Work* _pworkEnd = NULL;

static inline int KMF_GetRandom( int* pnSeed, int min, int max)
{
    if ( (*pnSeed) < 0) return ( BP_PS2_rand() % ((max - min) + 1) + min );
    else 		return ( GM_IRnd( pnSeed) % ((max - min) + 1) + min );
}
static inline int KMF_Rand( int* pnSeed, int a)
{
    if ( (*pnSeed) < 0) return ( BP_PS2_rand() % a );
    else 		return ( GM_IRnd( pnSeed) % a );
}
// 0.f～1.f
static inline float KMF_Rnd( int* pnSeed)
{
    if ( (*pnSeed) < 0) return ( rnd() );
    else 		return ( GM_Rnd( pnSeed) );
}

#ifdef CALC_ACT_TIME
// かもめ処理時間ダンプ
static void DbgDumpExecTime( int nTime)
{
    DEBUG_Locate( 240, 10, 0 );
    DEBUG_Printf("KMFEATHER-EXECTIME = %.3f\n", (float)nTime / 60.f / 262.f );
}
#endif

// 羽根ワークをリストに加える
static void KMF_InsertList( Work* pwork)	// リストに加えるワークへのポインタ
{
    if ( _pworkTop == NULL){ 	// 先頭に追加
	_pworkTop = _pworkEnd 	= pwork;
	pwork->pworkPrev 	= NULL;
	pwork->pworkNext 	= NULL;
    }else { 			// 最後尾に追加
	_pworkEnd->pworkNext 	= pwork;
	pwork->pworkPrev 	= _pworkEnd;
	pwork->pworkNext 	= NULL;
	_pworkEnd 	  	= pwork;
    }
}
// 羽根ワークをリストから外す
static void KMF_RemoveList( Work* pwork)	// リストから外すワークへのポインタ
{
    if ( pwork->pworkPrev == NULL ){ // 先頭
	_pworkTop = pwork->pworkNext;
	if ( pwork->pworkNext != NULL){
	    pwork->pworkNext->pworkPrev = NULL;
	}
    }else{
	pwork->pworkPrev->pworkNext = pwork->pworkNext;
    }

    if ( pwork->pworkNext == NULL ){ // 最後
	_pworkEnd = pwork->pworkPrev;
	if ( pwork->pworkPrev != NULL){
	    pwork->pworkPrev->pworkNext = NULL;
	}
    }else{
	pwork->pworkNext->pworkPrev = pwork->pworkPrev;
    }
}
// 羽根ワーク登録数取得
static int KMF_GetListNum( void )	
{
    int nNum = 0;

    Work* pwork;

    for ( pwork = _pworkTop; pwork != NULL; pwork = pwork->pworkNext ){
	nNum++;
    }

    return (nNum);
}

/* 床から平面バンダリを抽出（高さは調べない） */
static void FloorToBoundary( HZX_FLR *flr, FVECTOR *fv )
{
    /* x:min */
    fv->vx = flr->p1.x;
    fv->vx = (fv->vx < flr->p2.x)?fv->vx:flr->p2.x;
    fv->vx = (fv->vx < flr->p3.x)?fv->vx:flr->p3.x;
    fv->vx = (fv->vx < flr->p4.x)?fv->vx:flr->p4.x;

    /* z:min */
    fv->vy = flr->p1.z;
    fv->vy = (fv->vy < flr->p2.z)?fv->vy:flr->p2.z;
    fv->vy = (fv->vy < flr->p3.z)?fv->vy:flr->p3.z;
    fv->vy = (fv->vy < flr->p4.z)?fv->vy:flr->p4.z;

    /* x:max */
    fv->vz = flr->p1.x;
    fv->vz = (fv->vz > flr->p2.x)?fv->vz:flr->p2.x;
    fv->vz = (fv->vz > flr->p3.x)?fv->vz:flr->p3.x;
    fv->vz = (fv->vz > flr->p4.x)?fv->vz:flr->p4.x;

    /* z:max */
    fv->vw = flr->p1.z;
    fv->vw = (fv->vw > flr->p2.z)?fv->vw:flr->p2.z;
    fv->vw = (fv->vw > flr->p3.z)?fv->vw:flr->p3.z;
    fv->vw = (fv->vw > flr->p4.z)?fv->vw:flr->p4.z;
}	


// 動作関数
static	void Act( Work *work )
{
    int 		i;
    int			nCheckTime;
    FMATRIX*		pmat;
    SVECTOR*		pvecRot;
    DG_COMDL_POS*	comdl_pos;

    int nRadBase = (GV_Time & 4095);
    static FVECTOR vecY = { 0.f, KMF_GRAVITY, 0.f, 1.f }; // 重力ベクトル

#ifdef CALC_ACT_TIME
    int time;
GV_SET_PRFC_CLOCK();
#endif

    // マップ更新
    work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

    // 位置更新
    pmat      = work->matOrg;
    pvecRot   = work->vecRot;
    comdl_pos = work->comdl->pos;

    // フロアチェック時間
    nCheckTime = GV_Time % KMF_HZX_CHECK_INTV;

    for ( i = 0; i < work->usFeatherNum; i++){
	float 		fCos;

	// 傾きを取得
	fCos = _TS_Sin( pvecRot->vx ); 

	if ( work->usOneLife[i] > 0 ){ // 生存チェック
	    int	    nDir;
	    FVECTOR vecGravity;
	    FVECTOR vecBind;
	    FVECTOR vecNewDir;

	    switch ( work->ucFlrFlag[i] ){ 
	    case KMFEATHER_NOT_GROUND:
		nDir = GV_DiffDirAbs( pvecRot->vx, 2048);
		if ( nDir > 1024 ){
		    if ( !work->ucUpperFlag[i] ){
			if ( BP_PS2_rand() % 2 == 0 ){
			    work->vecRotAim[i].vx = -KMF_GetRandom( &work->nRandSeed, ROT_SPEED_MIN, ROT_SPEED_MAX);
			}else{
			    work->vecRotAim[i].vx = KMF_GetRandom( &work->nRandSeed, ROT_SPEED_MIN, ROT_SPEED_MAX);
			}
			work->vecRotAim[i].vy = KMF_GetRandom( &work->nRandSeed, -ROT_SPEED_MIN, ROT_SPEED_MIN);

			if ( BP_PS2_rand() % 2 == 0 ){
			    work->fAimSpdZ[i] *= -1.f;
			}

			work->ucUpperFlag[i] = 1;
			work->fAimSpdZ[i] *= 0.8f;
		    }
		}else{
		    work->ucUpperFlag[i] = 0;
		}

      if ( BP_IsPAL()==TRUE )
   		GV_NearExp8PVPAL( &work->vecRotAim[i], &work->vecRotSpd[i], 3 );
      else
   		GV_NearExp8PV( &work->vecRotAim[i], &work->vecRotSpd[i], 3 );

		// 速度補間
		work->fSpeedZ[i] = work->fSpeedZ[i] * 0.95f + work->fAimSpdZ[i] * 0.05f;
	    
		// 拘束ベクトル + 重力ベクトル
		_sceVu0SubVector( &vecBind, &work->vecCenter, (FVECTOR*)&pmat->m[3][0]);
		_sceVu0Normalize( &vecBind, &vecBind);
#if 0
		nDir = GV_DiffDirAbs( nDir, 1024 );
		_sceVu0ScaleVector( &vecBind, &vecBind, KMF_BIND * ((float)nDir / 1024.f) );
		_sceVu0AddVector( &vecGravity, &vecBind, &vecY );
#else
		_sceVu0ScaleVector( &vecBind, &vecBind, KMF_BIND * (1.f - DG_FABS(fCos)) );
		_sceVu0AddVector( &vecGravity, &vecBind, &vecY );
#endif		
		// 速度ベクトルを作成
		_sceVu0ScaleVector( &work->vecSpeed[i], (FVECTOR*)&pmat->m[2][0], work->fSpeedZ[i]);

		// 角度計算
		pvecRot->vx += (work->vecRotSpd[i].vx + (work->vecRotSpd[i].vx * DG_FABS( fCos)) );
		pvecRot->vx &= 4095;
		pvecRot->vy += work->vecRotSpd[i].vy;
		pvecRot->vy &= 4095;
	    	pvecRot->vz += work->vecRotSpd[i].vz;
		pvecRot->vz &= 4095;


		// マトリクス生成
		DG_SetPos2( (FVECTOR*)&pmat->m[3][0], pvecRot);
		DG_GetPos( pmat);
		_sceVu0ScaleVector( &vecNewDir, (FVECTOR*)&pmat->m[2][0], work->fSpeedZ[i]);
		_sceVu0AddVector( &work->vecSpeed[i], &vecNewDir, &work->vecSpeed[i] );
		_sceVu0AddVector( &work->vecSpeed[i], &vecGravity, &work->vecSpeed[i] );

		// 風計算
		{
		    float	fInner, fStrong;
		    FVECTOR 	vecWindDir;
		    // その地点の風を受け取る
		    OK_GetLocalWind_SH( (FVECTOR*)&pmat->m[3][0], &vecWindDir);
		    _sceVu0Normalize( &vecWindDir, &vecWindDir);
		    // 風力
		    fStrong = (float)(G_wind_intense < 100) ? G_wind_intense : 100;
		    // 風向ベクトルとの羽根の方向の内積
		    fInner = DG_FABS(_sceVu0InnerProduct( &pmat->m[2][0], &vecWindDir));
		    // 風力ベクトル作成
		    _sceVu0ScaleVector( &vecWindDir, &vecWindDir, (fInner) * (float)fStrong * 0.8f);
		    // 前のベクトルと補間
		    MAO_InterpVec( &work->vecWind[i], &work->vecWind[i], &vecWindDir, 0.05f);
		    // 移動ベクトルに反映
		    _sceVu0AddVector( &work->vecSpeed[i], &work->vecWind[i], &work->vecSpeed[i] );
		    if ( pmat->m[2][1] > 0.f )  work->vecRotAim[i].vx += (int)((fInner) * 32);
		    else			work->vecRotAim[i].vx -= (int)((fInner) * 32);
		    if ( work->vecRotAim[i].vx >= ROT_SPEED_MAX ){
			work->vecRotAim[i].vx = ROT_SPEED_MAX;
		    }else if ( work->vecRotAim[i].vx <= -ROT_SPEED_MAX ){
			work->vecRotAim[i].vx = -ROT_SPEED_MAX;
		    }
		}

		_sceVu0AddVector( (FVECTOR*)&pmat->m[3][0], (FVECTOR*)&pmat->m[3][0], &work->vecSpeed[i] );

		// 高さチェック
		if ( pmat->m[3][1] < GM_WaterLevel ){ // 水面
		    pvecRot->vx = 0;
		    pvecRot->vz = 0;
		    work->ucFlrFlag[i] = KMFEATHER_WATER;
		    pmat->m[3][1] = GM_WaterLevel + 30.f;
		    NewRippleStrip( (FVECTOR*)&pmat->m[3][0], 1500.f, 120); // 波紋
		}else if ( pmat->m[3][1] < work->nHeight[i] ){ // 地面
		    pmat->m[3][1] = work->nHeight[i];
		    pvecRot->vx = 0;
		    pvecRot->vz = 0;
		    work->ucFlrFlag[i] = KMFEATHER_GROUND;
		}else{
		    // 床チェック
		    if ( i % KMF_HZX_CHECK_INTV == nCheckTime ){	   
			HZX_FLR	flr[2];
			int	flr_atrs[2];
			float	flr_height[2];
			if( HZX_LevelHazardCheck( work->map_id, (FVECTOR*)&pmat->m[3][0], 
						  KMF_HZX_CHK_TYPE, KMF_HZX_PLAYER_ONLY ) & 1){
			    HZX_GetLevelHazard( flr, flr_atrs );
			    HZX_GetLevelHeight( flr_height );

//			    if ( work->nHeight[i] > (int)flr_height[0] ){
				work->nHeight[i] = (int)flr_height[0];
				FloorToBoundary( flr, &work->vecBound[i] );
//			    }
			}else{
			    work->nHeight[i] = -2100000000;
			}
		    }
		}
		break;
	    case KMFEATHER_GROUND:	// 接地している
		// しばし動く
		_sceVu0ScaleVector( &work->vecSpeed[i], (FVECTOR*)&pmat->m[2][0], work->fSpeedZ[i] );
		// 風計算
		{
		    float	fInner, fStrong;
		    FVECTOR 	vecWindDir;
		    // その地点の風を受け取る
		    OK_GetLocalWind_SH( (FVECTOR*)&pmat->m[3][0], &vecWindDir);
		    _sceVu0Normalize( &vecWindDir, &vecWindDir);
		    // 風力
		    fStrong = (float)((G_wind_intense < 100) ? G_wind_intense : 100);
		    // 風向ベクトルとの羽根の方向の内積
		    fInner = DG_FABS(_sceVu0InnerProduct( &pmat->m[2][0], &vecWindDir));
		    // 風力ベクトル作成
		    _sceVu0ScaleVector( &vecWindDir, &vecWindDir, (fInner) * (float)fStrong * 0.7f);
		    // 前のベクトルと補間
		    MAO_InterpVec( &work->vecWind[i], &work->vecWind[i], &vecWindDir, 0.05f);
		    // 移動ベクトルに反映
		    _sceVu0AddVector( &work->vecSpeed[i], &work->vecWind[i], &work->vecSpeed[i] );
#if 0
		    work->vecRotAim[i].vx = -(int)(fInner * 36.f);
		    pvecRot->vx += work->vecRotAim[i].vx;
#endif
		}
		pmat->m[3][0] += work->vecSpeed[i].vx;
		if ( work->vecSpeed[i].vy > 0.f) pmat->m[3][1] += work->vecSpeed[i].vy;
		pmat->m[3][2] += work->vecSpeed[i].vz;
		work->fSpeedZ[i] *= 0.95f;

		// 床の位置が変わったら
		if ( (int)pmat->m[3][1] - 10 > work->nHeight[i] ){
		    work->ucFlrFlag[i] = KMFEATHER_NOT_GROUND;
		    work->usOneLife[i] -= (work->usOneLife[i] <= 8) ? work->usOneLife[i] : 8;
		}else{
		    // 床チェック
		    if ( i % KMF_HZX_CHECK_INTV == nCheckTime ){	  
			HZX_FLR	flr[2];
			int	flr_atrs[2];
			float	flr_height[2];
			if( HZX_LevelHazardCheck( work->map_id, (FVECTOR*)&pmat->m[3][0], 
						  KMF_HZX_CHK_TYPE, KMF_HZX_PLAYER_ONLY ) & 1){
			    HZX_GetLevelHazard( flr, flr_atrs );
			    HZX_GetLevelHeight( flr_height );
			    if ( work->nHeight[i] > (int)flr_height[0] ){
				work->nHeight[i] = (int)flr_height[0];
			    }
			}else{
			    work->nHeight[i] = -2100000000;
			}
		    }
		}
		// マトリクス生成
		DG_SetPos2( (FVECTOR*)&pmat->m[3][0], pvecRot);
		DG_GetPos( pmat);
		pmat->m[3][3] = 1.f;
		break;
	    case KMFEATHER_WATER:		// 着水している
		// 移動ベクトル減衰
		_sceVu0ScaleVector( &work->vecSpeed[i], &work->vecSpeed[i], 0.9f);

		// マトリクス生成
		DG_SetPos2( (FVECTOR*)&pmat->m[3][0], pvecRot);
		DG_GetPos( pmat);

		// 座標更新
		pmat->m[3][0] += work->vecSpeed[i].vx + ( 20.f * _TS_Sin( (nRadBase + i * 128) & 4095 ) );
		pmat->m[3][1]  = GM_WaterLevel + 30.f + ( 20.f * _TS_Sin( (nRadBase + i * 128) & 2047 ) );
		pmat->m[3][2] += work->vecSpeed[i].vz + ( 20.f * _TS_Sin( (nRadBase + i * 128 + 1024 ) & 4095 ) );
		pmat->m[3][3]  = 1.f;
		break;
	    }

	    // 描画設定
	    DG_SetPos( pmat);
	    {
		FVECTOR vecScale = { work->fScale[i], work->fScale[i], work->fScale[i], 1.f };
		DG_ScalePos( &vecScale); // スケーリング
	    }
	    DG_GetPos( &comdl_pos->world );
	    comdl_pos->world.m[3][3] = 1.f;

	    // α値更新
	    if ( work->usOneLife[i] > 0 ) work->usOneLife[i]--;
	    if ( work->usOneLife[i] < 128) comdl_pos->color.vw = work->usOneLife[i];
	}else{
	    comdl_pos->color.vw = 0;
	}

	pmat++;
	pvecRot++;
	comdl_pos++;
    }

    // 寿命チェック
    if ( work->usLife > 0){
	work->usLife--;
	if ( work->usLife <= 0){
	    GV_DestroyActor( work );
	    return;
	}
    }
#ifdef CALC_ACT_TIME
    time = GV_GET_PRFC_CLOCK();
    DbgDumpExecTime( time);
#endif

}

// 破棄関数
static	void Die( Work *work )
{
    // co-modelを登録から外して解放
    DG_DequeueComdlObjs( work->comdl );
    DG_FreeComdl( work->comdl );

    // リストから外す
    KMF_RemoveList( work);

    // 可変ワークとして確保したメモリを解放
    if ( work->matOrg != NULL) 		GV_Free( work->matOrg);
    if ( work->vecRot != NULL)  	GV_Free( work->vecRot);
    if ( work->vecRotSpd != NULL)  	GV_Free( work->vecRotSpd);
    if ( work->vecRotAim != NULL)  	GV_Free( work->vecRotAim);
    if ( work->vecSpeed != NULL) 	GV_Free( work->vecSpeed);
    if ( work->vecBound != NULL) 	GV_Free( work->vecBound);
    if ( work->vecWind != NULL) 	GV_Free( work->vecWind);
    if ( work->ucFlrFlag != NULL)  	GV_Free( work->ucFlrFlag);
    if ( work->ucUpperFlag != NULL)  	GV_Free( work->ucUpperFlag);
    if ( work->nDelay != NULL)  	GV_Free( work->nDelay);
    if ( work->usOneLife != NULL) 	GV_Free( work->usOneLife);
    if ( work->fSpeedZ != NULL) 	GV_Free( work->fSpeedZ);
    if ( work->fAimSpdZ != NULL) 	GV_Free( work->fAimSpdZ);
    if ( work->fScale != NULL) 		GV_Free( work->fScale);
    if ( work->nHeight != NULL)  	GV_Free( work->nHeight);
}

// 初期設定
static	int GetResources( Work*		work,
			  FVECTOR* 	pvecCenter, 	// 中心
			  FVECTOR* 	pvecForce,	// 力積
			  int		nNum,		// 羽根数
			  int		nRandSeed)	// ランダムシード
{
    int 		i;
    DG_DEF*		def;
    DG_COMDL*		comdl;
    DG_COMDL_POS*	comdl_pos;
    FMATRIX*		pmat;
    SVECTOR*		pvecRot;
    SVECTOR		vecBaseRot;
    FVECTOR		vec;

    float		flr_height[2];
    HZX_FLR		flr[2];
    int			flr_atrs[2];
    int 		init_flr_flag;
    FVECTOR		vecFlr;

    work->map_id  = GM_GetHzxGroupID( GM_CurrentStageMap ); 	// マップID取得
    work->usLife  = LIFE_MAX;					// 寿命
    work->usFeatherNum = nNum;					// 枚数
    work->nRandSeed = nRandSeed;

    // 位置ワーク
    if( ( work->matOrg = (FMATRIX *)GV_Malloc(sizeof(FMATRIX) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->matOrg,	sizeof(FMATRIX) * work->usFeatherNum );

    // 回転ワーク
    if( (work->vecRot = (SVECTOR *)GV_Malloc(sizeof(SVECTOR) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecRot,	sizeof(SVECTOR) * work->usFeatherNum );

    // 角速度ワーク
    if( (work->vecRotSpd = (SVECTOR *)GV_Malloc(sizeof(SVECTOR) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecRotSpd, sizeof(SVECTOR) * work->usFeatherNum );

    // 角速度(補間先)ワーク
    if( (work->vecRotAim = (SVECTOR *)GV_Malloc(sizeof(SVECTOR) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecRotAim, sizeof(SVECTOR) * work->usFeatherNum );

    // バウンディングボックスワーク
    if( (work->vecSpeed = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecSpeed, sizeof(FVECTOR) * work->usFeatherNum );

    // バウンディングボックスワーク
    if( (work->vecBound = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecBound, sizeof(FVECTOR) * work->usFeatherNum );

    // 風計算ワーク
    if( (work->vecWind = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecWind, sizeof(FVECTOR) * work->usFeatherNum );

    // 接地フラグ
    if( (work->ucFlrFlag = (u_char *)GV_Malloc(sizeof(u_char) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->ucFlrFlag, sizeof(u_char) * work->usFeatherNum );

    // 上昇フラグ
    if( (work->ucUpperFlag = (u_char *)GV_Malloc(sizeof(u_char) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->ucUpperFlag, sizeof(u_char) * work->usFeatherNum );

    // 初期移動時間
    if( (work->nDelay = (u_short *)GV_Malloc(sizeof(u_short) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nDelay, sizeof(u_short) * work->usFeatherNum );

    // 生存時間
    if( (work->usOneLife = (u_short *)GV_Malloc(sizeof(u_short) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->usOneLife, sizeof(u_short) * work->usFeatherNum );

    // 速度
    if( (work->fSpeedZ = (float *)GV_Malloc(sizeof(float) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fSpeedZ, sizeof(float) * work->usFeatherNum );

    // 目標速度
    if( (work->fAimSpdZ = (float *)GV_Malloc(sizeof(float) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fAimSpdZ, sizeof(float) * work->usFeatherNum );

    // スケーリング
    if( (work->fScale = (float *)GV_Malloc(sizeof(float) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fScale, sizeof(float) * work->usFeatherNum );

    // 接地高さ
    if( (work->nHeight = (int *)GV_Malloc(sizeof(int) * work->usFeatherNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Feather Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nHeight, sizeof(int) * work->usFeatherNum );

    // co-modelの登録
    def = (DG_DEF*)GV_GetCache( GV_CacheID( MODEL_NAME, 'k' ) ) ;
    if ( def == NULL ) return -1;
    comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, work->usFeatherNum, 0 );
    if( comdl == NULL ) return -1;
    DG_QueueComdlObjs( comdl );
    work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

    { // フロア
	init_flr_flag = HZX_LevelHazardCheck( work->map_id, pvecCenter, KMF_HZX_CHK_TYPE, KMF_HZX_PLAYER_ONLY);
	if( init_flr_flag & 1 ){
	    HZX_GetLevelHazard( flr, flr_atrs );
	    FloorToBoundary( flr, &vecFlr );
	    HZX_GetLevelHeight( flr_height );
	}else{
	    vecFlr.vx = -400000.f;
	    vecFlr.vy = 400000.f;
	    vecFlr.vz = -400000.f;
	    vecFlr.vw = 400000.f;
	    flr_height[0] = -400000.f;
	    flr_height[1] = -400000.f;
	}
    }

    // ベースとなる回転情報を生成
    _sceVu0Normalize( &vec, pvecForce);
    TS_VecToRot( &vecBaseRot, &vec );

    // 中心
    _sceVu0CopyVector( &work->vecCenter, pvecCenter);

    pmat      = &work->matOrg[ 0 ];		
    pvecRot   = &work->vecRot[ 0 ];		
    comdl_pos = comdl->pos;
    for ( i = 0; i < work->usFeatherNum; i++){
	FVECTOR vecPos;

	// 回転情報作成
	pvecRot->vx = -KMF_GetRandom( &work->nRandSeed, 32, 512 ); // vecBaseRot.vx
	pvecRot->vy = vecBaseRot.vy + KMF_GetRandom( &work->nRandSeed, -512, 512 );
	pvecRot->vz = 0.f; // KMF_Rand( 4096 );

	// 座標設定
	vecPos.vx = 0.f;
	vecPos.vy = 0.f;
	vecPos.vz = KMF_Rnd( &work->nRandSeed) * 200.f;
	vecPos.vw = 1.f;
	
	// マトリクス生成
	DG_SetPos2( pvecCenter, pvecRot);
	DG_PutVector( &vecPos, &vecPos, 1);
	DG_GetPos( pmat);
	_sceVu0CopyVector( (FVECTOR*)&pmat->m[3][0], &vecPos);

	pvecRot->vx += KMF_GetRandom( &work->nRandSeed, -512, 512 ); 
	pvecRot->vy += KMF_GetRandom( &work->nRandSeed, -512, 512 ); 

	// 色
	comdl_pos->color.vx = (int)(COL_R * 0.5f + DG_ColorMatrix.m[3][0] * 0.25f + DG_ColorMatrix.m[0][0] * 0.25f);
	comdl_pos->color.vy = (int)(COL_G * 0.5f + DG_ColorMatrix.m[3][1] * 0.25f + DG_ColorMatrix.m[0][1] * 0.25f);
	comdl_pos->color.vz = (int)(COL_B * 0.5f + DG_ColorMatrix.m[3][2] * 0.25f + DG_ColorMatrix.m[0][2] * 0.25f);
	comdl_pos->color.vw = COL_A;

	// 表示位置設定
	DG_SetPos( pmat);
	DG_GetPos( &comdl_pos->world );

	// 移動パラメータ設定
	work->vecRotAim[i].vx = work->vecRotSpd[i].vx = 
	    KMF_GetRandom( &work->nRandSeed, ROT_SPEED_MIN, ROT_SPEED_MAX)*( (BP_PS2_rand() % 2 == 0)? -1 : 1 );
	work->vecRotAim[i].vy = work->vecRotSpd[i].vy = KMF_GetRandom( &work->nRandSeed, -ROT_SPEED_MIN, ROT_SPEED_MIN);
	work->vecRotAim[i].vz = work->vecRotSpd[i].vz = 0; 

	_sceVu0CopyVector( &work->vecSpeed[i], &DG_ZeroVector); // 移動ベクトル
	_sceVu0CopyVector( &work->vecBound[i], &vecFlr); 	// ハザードチェック箱
	_sceVu0CopyVector( &work->vecWind[i], &DG_ZeroVector); 	// 風ベクトル
	work->ucFlrFlag[i] 	= KMFEATHER_NOT_GROUND;
	work->ucUpperFlag[i] 	= 0;
	work->nDelay[i] 	= COUNT_VMODE(KMF_GetRandom( &work->nRandSeed, DELAY_MIN, DELAY_MAX));
	work->usOneLife[i]	= COUNT_VMODE(KMF_GetRandom( &work->nRandSeed, LIFE_MIN, LIFE_MAX));
	work->fSpeedZ[i] 	= (float)KMF_GetRandom( &work->nRandSeed, INIT_SPEED_MIN, INIT_SPEED_MAX );
	work->fAimSpdZ[i] 	= (float)KMF_GetRandom( &work->nRandSeed, AIM_SPEED_MIN, AIM_SPEED_MAX );
	work->fScale[i]		= (KMF_Rnd( &work->nRandSeed) * SCALE_ADD + SCALE_BASE);
	work->nHeight[i] 	= (int)flr_height[0];

	pmat++;
	pvecRot++;
	comdl_pos++;
    }

    // リストに追加
    KMF_InsertList( work);

    return 0;
}

// かもめさんの羽根
void *NewKamomeFeather( FVECTOR* 	pvecCenter, 	// 中心
			FVECTOR* 	pvecForce,	// 力積
			int		nNum,		// 羽根数
			int		nRandSeed)	// ランダムシード
{
    Work*	work;

    { // 同時発生数チェック
	int nListNum = KMF_GetListNum();
	if ( nListNum > LIMIT_LIST_NUM ){ 
	    return NULL;
	}
    }

    work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) );
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor);
	if ( GetResources( work, pvecCenter, pvecForce, nNum, nRandSeed) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void *)work ;
}
