//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmfeather.c
    かもめの羽根
    2001/07/06 Masafumi Okuta
    $Id: parfeather.c,v 1.1.1.3 2002/11/19 11:47:52 Yoshizawa1 Exp $
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
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))


// チェックしないフロア
#define KMF_HZX_PLAYER_ONLY	(HZX_FLOOR_IK|HZX_FLOOR_NO_PLAYER)
//#define KMF_HZX_PLAYER_ONLY	(HZX_FLOOR_NO_ENEMY|HZX_FLOOR_NO_BULLET|HZX_FLOOR_NO_MISSILE)

#define MODEL_NAME		(2698063)	// モデル名 par_hane.mdl

#define INIT_SPEED_MIN		(10)		// 初期スピード
#define INIT_SPEED_MAX		(20)		// 初期スピード
#define AIM_SPEED_MIN		(10)		// 目標スピード
#define AIM_SPEED_MAX		(20)		// 目標スピード

#define ROT_SPEED_MIN		(12)		// 回転最小
#define ROT_SPEED_MAX		(24)		// 回転最大

#define BASE_SPEED_BASE		(1.f)		// 基本速度(基本)
#define BASE_SPEED_ADD		(0.2f)		// 基本速度(加算)

#define SCALE_BASE		(1.f)		// スケーリング(基本)
#define SCALE_ADD		(0.f)		// スケーリング(加算)

#define DELAY_MIN		(20)		// 直進時間(最小)
#define DELAY_MAX		(30)		// 直進時間(最大)

#define LIFE_MIN		(400)		// 生存時間(最小)
#define LIFE_MAX		(600)		// 生存時間(最大)

#define KMF_GRAVITY 		(-9.8f)		// 重力
#define KMF_BIND		( 5.f)		// 揚力

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
extern int  	DG_QueueComdlObjs( DG_COMDL *comdl );
extern void 	DG_DequeueComdlObjs( DG_COMDL *comdl );
extern float 	_TS_Sin( int s );
extern void 	TS_VecToRot( SVECTOR *rot, FVECTOR *vec );

extern int 	OK_GetLocalWind_SH( FVECTOR *pos, FVECTOR *output );
extern FVECTOR 	G_wind;
extern int 	G_wind_intense;
extern int 	G_wind_intense_max;

// メッセージ用名前
#define PARFEATHER_NAME		(GV_StrCode("オウムの羽根"))	

enum{ // メッセージの種類
PARF_MESS_DESTROY,	// 破棄メッセージ
};

typedef struct _PAR_FEATHER{
    // 基本システム
    GV_ACT_EX			actor;
    HZX_GROUP_ID		map_id;			// マップID

    int				nName;			// 名前
    GV_MSG*			msg;			// 自分宛メッセージ	

    int				nSeed;			// ランダムシード

    int				nKill;			// キルフラグ
    
    // 管理
    u_short			usFeatherNum;		// 羽根数
    u_short			usLife;			// 生存時間
    FVECTOR			vecCenter;		// 中心
    FVECTOR			vecColor;		// 色
    float			fGravity;		// 重力
    float			fFlow;			// 揚力

    // 羽根用ワーク
    FMATRIX*			matOrg;			// 移動用マトリクス
    SVECTOR*			vecRot;			// 回転
    SVECTOR*			vecRotSpd;		// 角速度
    SVECTOR*			vecRotAim;		// 角速度(補間先)
    FVECTOR*			vecBound;		// バウンドボックス
    FVECTOR*			vecWind;		// 風計算用
    u_char*			ucFlrFlag;		// 接地
    u_char*			ucUpperFlag;		// 上昇フラグ
    u_short*			nDelay;			// 影響ディレイ
    float*			fSpeedZ;		// スピード
    float*			fAimSpdZ;		// 目標速度	
    float*			fScale;			// 拡大率
    int*			nHeight;		// 高さ
    
    // 描画系
    DG_COMDL*			comdl;			// co-model

}Work;

// シード固定ランダム関数
static inline int PAR_IRand( Work* work, int a ){  
    return (GM_IRnd( &work->nSeed ) % a); 
}
static inline int PAR_GetRandom( Work* work, int min, int max ){  
    return (GM_IRnd( &work->nSeed ) % ((max - min) + 1) + min); 
}
static inline float PAR_FRand( Work* work ){  
    return (GM_FRnd( &work->nSeed )); 
}
static inline float PAR_Rand( Work* work ){  
    return (GM_Rnd( &work->nSeed )); 
}

#ifdef CALC_ACT_TIME
// かもめ処理時間ダンプ
static void DbgDumpExecTime( int nTime)
{
    DEBUG_Locate( 240, 10, 0 );
    DEBUG_Printf("KMFEATHER-EXECTIME = %.3f\n", (float)nTime / 60.f / 262.f );
}
#endif
// メッセージ取得
static void PARF_RecieveMessage( Work* work)
{
    GV_MSG*	msg;
    int 	n_msg, code;

    n_msg = GV_ReceiveMessage( work->nName, &work->msg ); // メッセージ取得

    msg = work->msg;

    // 個人宛メッセージを反映
    while ( n_msg-- > 0 ){ // 自分に対して飛ばされるメッセージ
	code = msg->message[ 0 ] ;

	switch( code ){
	case PARF_MESS_DESTROY:
	    GV_DestroyActor( work );
	    break;
	}
	msg++;
    }
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
    
    FVECTOR vecY = { 0.f, work->fGravity, 0.f, 1.f }; // 重力ベクトル

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

    PARF_RecieveMessage( work);

    // フロアチェック時間
    nCheckTime = GV_Time % KMF_HZX_CHECK_INTV;

    for ( i = 0; i < work->usFeatherNum; i++){
	float 		fCos;

	// 傾きを取得
	fCos = _TS_Sin( pvecRot->vx ); 

	{ // 生存チェック
	    if ( !work->ucFlrFlag[i]){ // 未設置
		int	    nDir;
		FVECTOR vecGravity;
		FVECTOR vecBind;
		FVECTOR vecSpd;
		FVECTOR vecNewDir;

		nDir = GV_DiffDirAbs( pvecRot->vx, 2048);

		if ( nDir > 1024 ){
		    if ( !work->ucUpperFlag[i] ){
			if ( PAR_IRand( work, 2) == 0 ){
			    work->vecRotAim[i].vx = -PAR_GetRandom( work, ROT_SPEED_MIN, ROT_SPEED_MAX);
			}else{
			    work->vecRotAim[i].vx = PAR_GetRandom( work, ROT_SPEED_MIN, ROT_SPEED_MAX);
			}
			work->vecRotAim[i].vy = PAR_GetRandom( work, -ROT_SPEED_MIN, ROT_SPEED_MIN);

			if ( PAR_IRand( work, 2) == 0 ){
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
		_sceVu0ScaleVector( &vecBind, &vecBind, work->fGravity * ((float)nDir / 1024.f) );
		_sceVu0AddVector( &vecGravity, &vecBind, &vecY );
#else
		_sceVu0ScaleVector( &vecBind, &vecBind, work->fGravity * (1.f - DG_FABS(fCos)) );
		_sceVu0AddVector( &vecGravity, &vecBind, &vecY );
#endif		
		// 速度ベクトルを作成
		_sceVu0ScaleVector( &vecSpd, (FVECTOR*)&pmat->m[2][0], work->fSpeedZ[i]);

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
		_sceVu0AddVector( &vecSpd, &vecNewDir, &vecSpd );
		_sceVu0AddVector( &vecSpd, &vecGravity, &vecSpd );

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
		    _sceVu0AddVector( &vecSpd, &work->vecWind[i], &vecSpd );
		    if ( pmat->m[2][1] > 0.f )  work->vecRotAim[i].vx += (int)((fInner) * 32);
		    else			work->vecRotAim[i].vx -= (int)((fInner) * 32);
		    if ( work->vecRotAim[i].vx >= ROT_SPEED_MAX ){
			work->vecRotAim[i].vx = ROT_SPEED_MAX;
		    }else if ( work->vecRotAim[i].vx <= -ROT_SPEED_MAX ){
			work->vecRotAim[i].vx = -ROT_SPEED_MAX;
		    }
		}

		_sceVu0AddVector( (FVECTOR*)&pmat->m[3][0], (FVECTOR*)&pmat->m[3][0], &vecSpd );

		// 高さチェック
		if ( pmat->m[3][1] <= work->nHeight[i] ){
		    pmat->m[3][1] = work->nHeight[i];
		    pvecRot->vx = 0;
		    pvecRot->vz = 0;
		    work->ucFlrFlag[i] = 1;
		}
		// 床チェック
		{
		    if ( i % KMF_HZX_CHECK_INTV == nCheckTime ){	   
			HZX_FLR	flr[2];
			int	flr_atrs[2];
			float	flr_height[2];
			if( HZX_LevelHazardCheck( GM_CurrentStageMap, (FVECTOR*)&pmat->m[3][0], 
						  HZX_CHK_FIX, KMF_HZX_PLAYER_ONLY ) & 1){
			    HZX_GetLevelHazard( flr, flr_atrs );
			    HZX_GetLevelHeight( flr_height );

			    if ( work->nHeight[i] > (int)flr_height[0] ){
				work->nHeight[i] = (int)flr_height[0];
				FloorToBoundary( flr, &work->vecBound[i] );
			    }
			}
		    }
		}
	    }else{ // 接地している
		FVECTOR vecSpd;
		HZX_FLR	flr[2];
		int	flr_atrs[2];
		float	flr_height[2];

		// しばし動く
		_sceVu0ScaleVector( &vecSpd, (FVECTOR*)&pmat->m[2][0], work->fSpeedZ[i] );
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
		    _sceVu0AddVector( &vecSpd, &work->vecWind[i], &vecSpd );
#if 0
		    work->vecRotAim[i].vx = -(int)(fInner * 36.f);
		    pvecRot->vx += work->vecRotAim[i].vx;
#endif
		}
		pmat->m[3][0] += vecSpd.vx;
		if ( vecSpd.vy > 0.f) pmat->m[3][1] += vecSpd.vy;
		pmat->m[3][2] += vecSpd.vz;
		work->fSpeedZ[i] *= 0.95f;

		// 床の位置が変わったら
		if ( abs((int)pmat->m[3][1] - work->nHeight[i]) > 200 ){
		    work->ucFlrFlag[i] = 0;
		}else{
		    // 床チェック
		    if ( i % KMF_HZX_CHECK_INTV == nCheckTime ){	   
			if( HZX_LevelHazardCheck( GM_CurrentStageMap, (FVECTOR*)&pmat->m[3][0],
						  HZX_CHK_FIX, KMF_HZX_PLAYER_ONLY ) & 1){
			    HZX_GetLevelHazard( flr, flr_atrs );
			    HZX_GetLevelHeight( flr_height );
			    // 床の位置が変わった
			    if ( (int)flr_height[0] < work->nHeight[i] ){
				work->nHeight[i] = (int)flr_height[0];
			    }
			}
		    }
		}
		// マトリクス生成
		DG_SetPos2( (FVECTOR*)&pmat->m[3][0], pvecRot);
		DG_GetPos( pmat);
	    }

	    // 描画設定
	    DG_SetPos( pmat);
	    {
		FVECTOR vecScale = { work->fScale[i], work->fScale[i], work->fScale[i], 1.f };
		DG_ScalePos( &vecScale); // スケーリング
	    }
	    DG_GetPos( &comdl_pos->world );
	    comdl_pos->world.m[3][3] = 1.f;

	}

	if ( comdl_pos->color.vw > work->usLife) comdl_pos->color.vw = work->usLife;

	pmat++;
	pvecRot++;
	comdl_pos++;
    }

    // 寿命チェック
    if ( work->nKill && work->usLife > 0){
	work->usLife--;
	if ( work->usLife <= 0){
	    GV_DestroyActor( work );
	    return;
	}
    }

#ifdef CALC_ACT_TIME
    time = GV_GET_PRFC_CLOCK();
    DbgDumpExecTime( time);
MENU_Locate( 240, 50, 0 );
MENU_SetColor( 0x00, 0x00, 0x80 );
MENU_Printf("WIND-SPD : %d\n", G_wind_intense );
#endif

}

// 破棄関数
static	void Die( Work *work )
{
    // co-modelを登録から外して解放
    DG_DequeueComdlObjs( work->comdl );
    DG_FreeComdl( work->comdl );

    // 可変ワークとして確保したメモリを解放
    if ( work->matOrg != NULL) 		GV_Free( work->matOrg);
    if ( work->vecRot != NULL)  	GV_Free( work->vecRot);
    if ( work->vecRotSpd != NULL)  	GV_Free( work->vecRotSpd);
    if ( work->vecRotAim != NULL)  	GV_Free( work->vecRotAim);
    if ( work->vecBound != NULL) 	GV_Free( work->vecBound);
    if ( work->vecWind != NULL) 	GV_Free( work->vecWind);
    if ( work->ucFlrFlag != NULL)  	GV_Free( work->ucFlrFlag);
    if ( work->ucUpperFlag != NULL)  	GV_Free( work->ucUpperFlag);
    if ( work->nDelay != NULL)  	GV_Free( work->nDelay);
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
			  int		nKill,		// キルフラグ
			  float		fInitSpdMin,	// 初期速度最小
			  float		fInitSpdMax,	// 初期速度最大
			  float		fSpdMin,	// 速度最小
			  float		fSpdMax,	// 速度最大
			  float		fGravity,	// 重力
			  float		fFlow,		// 揚力
			  int		nSeed)		// ランダムシード
{
    int 		i;
    DG_DEF*		def;
    DG_COMDL*		comdl;
    DG_COMDL_POS*	comdl_pos;
    FMATRIX*		pmat;
    SVECTOR*		pvecRot;
    SVECTOR		vecBaseRot;
    FVECTOR		vec;

    float		fSpdInitRand;
    float		fSpdRand;

    float		flr_height[2];
    HZX_FLR		flr[2];
    int			flr_atrs[2];
    int 		init_flr_flag;
    FVECTOR		vecFlr;

    work->map_id  	= GM_GetHzxGroupID( GM_CurrentStageMap ); 	// マップID取得

    work->nName		= PARFEATHER_NAME;				// メッセージ用名前

    work->usLife  	= LIFE_MAX;					// 寿命
    work->usFeatherNum 	= nNum;						// 枚数
    work->nKill		= nKill;					// キルフラグ
    work->nSeed 	= nSeed;					// ランダムシード

    work->fGravity	= fGravity;
    work->fFlow		= fFlow;

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
	init_flr_flag = HZX_LevelHazardCheck( GM_CurrentStageMap, pvecCenter, HZX_CHK_F_FLOOR, KMF_HZX_PLAYER_ONLY);
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

    fSpdInitRand = fInitSpdMax - fInitSpdMin;
    fSpdRand     = fSpdMax - fSpdMin;

    pmat      = &work->matOrg[ 0 ];		
    pvecRot   = &work->vecRot[ 0 ];		
    comdl_pos = comdl->pos;
    for ( i = 0; i < work->usFeatherNum; i++){
	FVECTOR vecPos;

	// 回転情報作成
	pvecRot->vx = -PAR_GetRandom( work, 256, 512 ); 
	pvecRot->vy = vecBaseRot.vy + PAR_GetRandom( work, -512, 512 );
	pvecRot->vz = 0.f; 

	// 座標設定
	vecPos.vx = 0.f;
	vecPos.vy = 0.f;
	vecPos.vz = rnd() * 200.f;
	vecPos.vw = 1.f;
	
	// マトリクス生成
	DG_SetPos2( pvecCenter, pvecRot);
	DG_PutVector( &vecPos, &vecPos, 1);
	DG_GetPos( pmat);
	_sceVu0CopyVector( (FVECTOR*)&pmat->m[3][0], &vecPos);

	pvecRot->vx += PAR_GetRandom( work, -512, 512 ); 
	pvecRot->vy += PAR_GetRandom( work, -512, 512 ); 

	// 色
	comdl_pos->color.vx = COL_R;
	comdl_pos->color.vy = COL_G;
	comdl_pos->color.vz = COL_B;
	comdl_pos->color.vw = COL_A;

	// 表示位置設定
	DG_SetPos( pmat);
	DG_GetPos( &comdl_pos->world );

	// 移動パラメータ設定
	work->vecRotAim[i].vx = work->vecRotSpd[i].vx = 
	    PAR_GetRandom( work, ROT_SPEED_MIN, ROT_SPEED_MAX)*( ( PAR_IRand( work, 2) == 0)? -1 : 1 );
	work->vecRotAim[i].vy = work->vecRotSpd[i].vy = PAR_GetRandom( work, -ROT_SPEED_MIN, ROT_SPEED_MIN);
	work->vecRotAim[i].vz = work->vecRotSpd[i].vz = 0; 

	_sceVu0CopyVector( &work->vecBound[i], &vecFlr); // ハザードチェック箱作成
	_sceVu0CopyVector( &work->vecWind[i], &DG_ZeroVector); // 風ベクトル
	work->ucFlrFlag[i] 	= 0;
	work->ucUpperFlag[i] 	= 0;
	work->nDelay[i] 	= COUNT_VMODE( PAR_GetRandom( work, DELAY_MIN, DELAY_MAX) );
	work->fSpeedZ[i] 	= fInitSpdMin + fSpdInitRand * PAR_Rand( work);
	work->fAimSpdZ[i] 	= fSpdMin + fSpdRand * PAR_Rand( work);
	work->fScale[i]		= (rnd() * SCALE_ADD + SCALE_BASE);
	work->nHeight[i] 	= (int)flr_height[0];

	pmat++;
	pvecRot++;
	comdl_pos++;
    }

    return 0;
}

// おうむの羽根
void* NewParrotFeather( FVECTOR* 	pvecCenter, 	// 中心
			FVECTOR* 	pvecForce,	// 力積
			int		nNum,		// 羽根数
			int		nKill,		// キルフラグ
			float		fInitSpdMin,	// 初期速度最小
			float		fInitSpdMax,	// 初期速度最大
			float		fSpdMin,	// 速度最小
			float		fSpdMax,	// 速度最大
			float		fGravity,	// 重力
			float		fFlow,		// 揚力
			int		nSeed)		// ランダムシード
{
    Work*	work;

    work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) );
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor);
	if ( GetResources( work, pvecCenter, pvecForce, nNum, nKill, 
			   fInitSpdMin, fInitSpdMax, fSpdMin, fSpdMax, fGravity, fFlow, nSeed) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void *)work ;
}
