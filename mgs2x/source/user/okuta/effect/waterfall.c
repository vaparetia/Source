//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterfall.c
    流れ落ちる水
    2001/07/24 Masafumi Okuta
    $Id: waterfall.c,v 1.1.1.3 2002/11/19 11:47:54 Yoshizawa1 Exp $
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

extern float GM_WaterLevel;

extern int   DG_TickCount;

#define BODY_R (0x40)
#define BODY_G (0x40)
#define BODY_B (0x40)
#define BODY_A (0x30)
#define POLE_R (0x30)
#define POLE_G (0x30)
#define POLE_B (0x30)
#define POLE_A (0x30)

#define BODY_RAD (500.f)
#define POLE_RAD (510.f)

#define BUBBLE_LIFE_MIN (30)
#define BUBBLE_LIFE_MAX (40)

#define BUBBLE_FALL_MIN (40)
#define BUBBLE_FALL_MAX (50)

#define BUBBLE_GRABITY  (0.2f)

enum{ // メッセージ
WATER_MESSAGE_ACTIVE,	// アクティブモード
WATER_MESSAGE_SLEEP,	// スリープモード
WATER_MESSAGE_FEEDOUT,	// フェードアウト
};
enum{ // メッセージ
SPREAD_MESSAGE_ACTIVE,	// アクティブモード
SPREAD_MESSAGE_SLEEP,	// スリープモード
SPREAD_MESSAGE_STOP,	// 生成終了
SPREAD_MESSAGE_RESTART,	// 生成再開
SPREAD_MESSAGE_KILL,	// 破棄
SPREAD_MESSAGE_RAD_CHG,	// 半径変更
};

#define TBL_SE_MAX (4)
static int TBL_SE_INTV[TBL_SE_MAX] = { 8, 11, 16, 18};

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
    int			nBubbleNum;			// 泡数
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

    u_char		ucFallSe;			// 落下水音
    u_char		ucHitSe;			// プレイヤーが当たっている時の音	
    int			nFallSeTime;
    int			nHitSeTime;
    int			nFallSeIntv;
    int			nHitSeIntv;

    // 飛抹
//    FVECTOR*		vecPos;
    FVECTOR*		vecDir;
    float*		fSpeed;
    float*		fSize;

    // 水流本体
    FVECTOR*		vecBodyPos;
    FVECTOR*		vecBodyDir;
    float*		fBodySpeed;
    float*		fBodySize;
    short*		nBodyDelay;

    // 泡
    FVECTOR*		vecBubblePos;
    FVECTOR*		vecBubbleStep;
    float*		fBubbleSpeed;
    short*		nBubbleLife;

    // 描画用
    DG_PRIM2*		prim;		
    DG_PRIM2*		prim_body;		
    DG_PRIM2*		prim_pole;		

    int			nSpread;
    FMATRIX		matSpread;
    float		fSpreadMin;
    float		fSpreadMax;

    int			nPlSpread;
    int			nPlSpreadName;
    int			nPlSpreadFlag;
    FMATRIX		matPlSpread;
    float		fPlSpreadMin;
    float		fPlSpreadMax;

    float		fMoveDropX;
    float		fMoveDropY;
} Work;

// メッセージチェック
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

// 位置初期化
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
    float fChgSizeRate = work->fAimSize - work->fInitSize;

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
    (*pfSize)  = work->fInitSize + rnd() * fChgSizeRate;
}

// 水流本体のリセット
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

// 未使用泡ワークインデックス取得
static int GetFreeBubble( Work* 	work)
{
    int i;

    short* pnLife   = work->nBubbleLife;
    for ( i = 0; i < work->nBubbleNum; i++){
	if ( (*pnLife) == 0 ){
	    return i;
	}
	pnLife++;
    }

    return (-1);
}

// 泡動作
static void ActBubble( Work* work)
{
    int i;
    FVECTOR	vec;
    FVECTOR*	pvecPos	 = work->vecBubblePos;
    FVECTOR*	pvecDir  = work->vecBubbleStep;
    float*	pfSpeed  = work->fBubbleSpeed;
    short*	pnLife   = work->nBubbleLife;

    for ( i = 0; i < work->nBubbleNum; i++){
	if ( (*pnLife) > 0 ){
	    // 移動ベクトル生成
	    _sceVu0ScaleVector( &vec, pvecDir, (*pfSpeed) );
	    // 位置更新
	    _sceVu0AddVector( pvecPos, pvecPos, &vec);
	    // 重力反映
	    pvecDir->vy += BUBBLE_GRABITY;
	    _sceVu0Normalize( pvecDir, pvecDir);

	    (*pnLife)--;
	}

	pvecPos++;
	pvecDir++;
	pfSpeed++;
	pnLife++;
    }
}

// 水流本体動作
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
    DG_PRIM2_UVRGB*	uvrgbAdd1;
    DG_PRIM2_UVRGB*	uvrgbAdd2;
    FVECTOR*		pvecMemSub;
    DG_PRIM2_UVRGB*	uvrgbSub1;
    DG_PRIM2_UVRGB*	uvrgbSub2;

    pvecMemAdd = prim_add->pos[prim_add->buffer_clock];
    uvrgbAdd1  = prim_add->uvrgb[0];
    uvrgbAdd2  = prim_add->uvrgb[1];
    pvecMemSub = prim_sub->pos[prim_sub->buffer_clock];
    uvrgbSub1  = prim_sub->uvrgb[0];
    uvrgbSub2  = prim_sub->uvrgb[1];


    prim_sub->flag |= DG_PRIM_AS_CUSTOMWORLD;
    prim_sub->as_world.m[3][0] = 5.f; // 疑似バンプ

    
    if ( work->nCntr % COUNT_VMODE(120) == 0 ){
	extern int OK_EX0_Flag;
	extern FVECTOR OK_EX0_Pos;
	float ftemp;
	float radius;
	    
	OK_EX0_Flag = 1;
	ftemp = (float)(BP_PS2_rand()&4095)*0.01f;
	radius = ((irnd()>>8)&1)? 100.f*sin( ftemp ):-100.f*sin( ftemp );
	radius = ((irnd()>>8)&1)? 100.f+radius:-100.f+radius;
	ftemp = (float)(BP_PS2_rand()&4095)*0.025f;
	OK_EX0_Pos.vx = work->matSpread.m[3][0] + radius*sinf( ftemp );
	OK_EX0_Pos.vz = work->matSpread.m[3][2] + radius*cosf( ftemp );
	OK_EX0_Pos.vy = GM_WaterLevel;
    }

    if ( abs( DG_TickCount - work->nFallSeTime) >= work->nFallSeIntv && 
	 DG_Chanls[0].eye.m[3][1] >= GM_WaterLevel){
	if ( work->ucFallSe ) GM_SeSetMode( SD_A_FALLWTR1, (FVECTOR*)&work->matSpread.m[3][0], GM_SEMODE_NORMAL); 
	else 		      GM_SeSetMode( SD_A_FALLWTR2, (FVECTOR*)&work->matSpread.m[3][0], GM_SEMODE_NORMAL); 
	work->ucFallSe = (~work->ucFallSe & 1);
	work->nFallSeTime = DG_TickCount;
	work->nFallSeIntv = COUNT_VMODE( TBL_SE_INTV[ MAO_Rand(TBL_SE_MAX)]);
    }


    for ( i = 0; i < nNum; i++){
	FVECTOR vec;

	pvecTmp  = SCR_POS;
	pvecVert = SCR_POS;

	if ( (*pnDelay) > 0){
	    (*pnDelay)--;
	}else{
	    (*pfSize) = (*pfSize) * 0.99f + fRad * 0.01f;

	    for ( j = 0; j < N_VERTS_RING; j++){
		int nAng = 4096 * j / N_VERTS_RING;
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

	    if ( pvecPos->vy < work->fFallY ){ // 一定の高さまでいったらリセット
		int nRes;
		if ( (nRes = GetFreeBubble(work)) >= 0 ){
		    work->vecBubblePos[nRes].vx = pvecPos->vx + frnd() * (*pfSize);
		    work->vecBubblePos[nRes].vy = work->fFallY - rnd() * 200.f;
		    work->vecBubblePos[nRes].vz = pvecPos->vz + frnd() * (*pfSize);
		    work->vecBubblePos[nRes].vw = 1.f;

		    work->vecBubbleStep[nRes].vx = (work->vecBubblePos[nRes].vx - pvecPos->vx) * 0.002f;
		    work->vecBubbleStep[nRes].vy = -(*pfSize) * 2.f;
		    work->vecBubbleStep[nRes].vz = (work->vecBubblePos[nRes].vz - pvecPos->vz) * 0.002f;
		    work->vecBubbleStep[nRes].vw = 1.f;
		    _sceVu0Normalize( &work->vecBubbleStep[nRes], &work->vecBubbleStep[nRes]);

		    work->fBubbleSpeed[nRes] = (float)MAO_GetRandom( BUBBLE_FALL_MIN, BUBBLE_FALL_MAX);
		    work->nBubbleLife[nRes] = MAO_GetRandom( BUBBLE_LIFE_MIN, BUBBLE_LIFE_MAX) + 64;

		}
		if (!work->nSpread){
		    extern void* NewWaterSpread( int, FMATRIX*, int, int, float, float, int, int, 
						 float, float, float, int, float, float, float, float);
		    extern void* NewWaterRippleMan( int, FVECTOR*);

		    work->matSpread.m[3][1] = GM_WaterLevel;
		    _sceVu0CopyVector( (FVECTOR*)&work->matSpread.m[3][0], pvecPos);
		    GV_SetActorChild( work, NewWaterSpread( work->name, &work->matSpread, 32, 3,
							    work->fSpreadMin, work->fSpreadMax,
							    20, 40, 100.f, 200.f, 1.f / 8.0f, 0x80808040, 
							    8.f, 4.f, 10.f, 5.f));
		    GV_SetActorChild( work, NewWaterRippleMan( work->name, pvecPos ));
		    work->nSpread = 1;
		    
		}

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

    if (!work->nPlSpread){ // 生成
	extern void* NewWaterSpread( int, FMATRIX*, int, int, float, float, int, int, 
				     float, float, float, int, float, float, float, float);
	GV_SetActorChild( work, NewWaterSpread( work->nPlSpreadName, &work->matPlSpread, 32, 6,
						work->fPlSpreadMin, work->fPlSpreadMax,
						40, 80, 150.f, 150.f, 1.f / 8.0f, 0x80808040, 
						1.f, 1.f, 5.f, 5.f));
	work->nPlSpread = 1;
    }else{
//	int bWaterDrops = 0;
	SVECTOR vecRot;
	static int nNode[3] = {
	    HUMAN21_ATAMA,
	    HUMAN21_HIDARI_KATA,
	    HUMAN21_MIGI_KATA,
	};

	vecRot.vx = 256; // MAO_GetRandom( -1024, 1024);
	vecRot.vy = MAO_Rand( 4096);
	vecRot.vz = 0;

	DG_SetPos( &GM_PlayerBody->objs->objs[ nNode[BP_PS2_rand() % 3] ].world);
	DG_RotatePos( &vecRot);
	DG_GetPos( &work->matPlSpread);
	work->matPlSpread.m[3][1] +=  rnd() * 100.f;
	work->fPlSpreadMin = 50.f;
	work->fPlSpreadMax = 100.f;

	// 画面水滴
	if ( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) &&
	     ( MAO_SphereCheck( &GM_PlayerPosition, (FVECTOR*)&work->matSpread.m[3][0], 1500) ) &&
	     DG_Chanls[0].eye.m[3][1] >= GM_WaterLevel ){
	    FVECTOR vecSub;
	    extern void* NewScrDrop( int );
	    extern void SetDropForce( float, float );
#if 0
	    FVECTOR vecScr;
	    _sceVu0SubVector( &vecSub, &GM_PlayerPosition, (FVECTOR*)&work->matSpread.m[3][0]);
	    _sceVu0Normalize( &vecSub, &vecSub);
	    MAO_AbsVec( &vecScr, &vecSub, &DG_Chanls[0].eye_pers);
	    if ( vecScr.vw != 0.f){
		vecScr.vx /= vecScr.vw;
		vecScr.vy /= vecScr.vw;
		vecScr.vz /= vecScr.vw;
	    }
	    if ( BP_PS2_rand() % 10 == 0 )NewScrDrop( 0 );
printf("%f %f\n", vecScr.vx, vecScr.vy);
	    SetDropForce( vecScr.vx * frnd(), vecScr.vy * frnd()); // 水滴を動かす
#else
	    float fDist, fRate;
	    fDist = _MAO_FVec2Len3( &GM_PlayerPosition, (FVECTOR*)&work->matSpread.m[3][0]);
	    fRate = fDist * 20.f / 1500.f;

	    _sceVu0SubVector( &vecSub, (FVECTOR*)&work->matSpread.m[3][0], (FVECTOR*)DG_Chanls[0].eye.m[3]);
	    _sceVu0Normalize( &vecSub, &vecSub);
	    
	    if ( fDist < 300.f || _sceVu0InnerProduct( &vecSub, (FVECTOR*)DG_Chanls[0].eye.m[2]) > -0.2f){
		NewScrDrop( 0 );
		work->fMoveDropX = work->fMoveDropX * 0.9f + (fRate * frnd()) * 0.1f;
		work->fMoveDropY = work->fMoveDropY * 0.9f + (fRate * frnd()) * 0.1f;
		SetDropForce( work->fMoveDropX, work->fMoveDropY); // 水滴を動かす
	    }
#endif
	}

	// 水飛抹
	if ( MAO_SphereCheck( &GM_PlayerPosition, (FVECTOR*)&work->matSpread.m[3][0], 550) &&
	     DG_Chanls[0].eye.m[3][1] >= GM_WaterLevel ){
	    if ( !work->nPlSpreadFlag ){
		work->nPlSpreadFlag = 1;
		{ 
		    GV_MSG msg ;
		    u_int buffer[  ] = { SPREAD_MESSAGE_RESTART };
	
		    msg.address = work->nPlSpreadName;
		    msg.message = buffer;
		    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
		    GV_SendMessage( &msg ) ;
		}
	    }
	    
	    {	// 振動
		extern void *NewPadVibration( char *script, int type );
		static	u_char	WaterFallVib[] = { 56, 1, 0, 0 } ;
		NewPadVibration( WaterFallVib, 2);
	    }

	    // 効果音
	    if ( abs( DG_TickCount - work->nHitSeTime) >= work->nHitSeIntv && 
		  DG_Chanls[0].eye.m[3][1] >= GM_WaterLevel ){
		if ( work->ucHitSe )	    GM_SeSetMode( SD_A_FALLHIT1, &GM_PlayerPosition, GM_SEMODE_NORMAL); 
		else			    GM_SeSetMode( SD_A_FALLHIT2, &GM_PlayerPosition, GM_SEMODE_NORMAL); 
		work->ucHitSe = (~work->ucHitSe & 1);
		work->nHitSeTime = DG_TickCount;
		work->nHitSeIntv = COUNT_VMODE( TBL_SE_INTV[ MAO_Rand(TBL_SE_MAX)]); // COUNT_VMODE( 15 + MAO_GetRandom( -8, 8));
	    }
	}else{
	    if ( work->nPlSpreadFlag ){
		work->nPlSpreadFlag = 0;
		{ 
		    GV_MSG msg ;
		    u_int buffer[  ] = { SPREAD_MESSAGE_STOP };
	
		    msg.address = work->nPlSpreadName;
		    msg.message = buffer;
		    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
		    GV_SendMessage( &msg ) ;
		}
	    }
	}
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
    FVECTOR*		pvecPrim;
    FVECTOR*		pvecPrimPre;
    DG_PRIM2_UVRGBWH*	uvrgbwh;

    // メッセージチェック
    CheckMessage( work);

    // スリープモードチェック
    if ( work->nSleep ){
	// 非表示
	SET_FLAG( work->prim->flag, 	 DG_PRIM2_INVISIBLE);
	SET_FLAG( work->prim_body->flag, DG_PRIM2_INVISIBLE);
	SET_FLAG( work->prim_pole->flag, DG_PRIM2_INVISIBLE);
	return;
    }else{
        // 表示
	UNSET_FLAG( work->prim->flag, 	   DG_PRIM2_INVISIBLE);
	UNSET_FLAG( work->prim_body->flag, DG_PRIM2_INVISIBLE);
	UNSET_FLAG( work->prim_pole->flag, DG_PRIM2_INVISIBLE);
    }

    // マップ設定
    GM_GroupPrim2( work->prim, work->map );
    GM_GroupPrim2( work->prim_body, work->map );
    GM_GroupPrim2( work->prim_pole, work->map );

    // バッファ切替え
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

    pvecPrim    = work->prim->pos[work->prim->buffer_clock];
    pvecPrimPre = work->prim->pos[(work->prim->buffer_clock+1) % 2];
    uvrgbwh  = work->prim->uvrgb[work->prim->buffer_clock];
    for ( i = 0; i < work->nPrimNum; i++){
	for ( j = 0; j < work->nVertNum; j++){
	    FVECTOR vec;

	    _sceVu0ScaleVector( &vec, pvecDir, (*pfSpeed) );
	    _sceVu0AddVector( pvecPrim, pvecPrimPre, &vec);
	    pvecDir->vy -= work->fGravity;
	    _sceVu0Normalize( pvecDir, pvecDir);

//	    _sceVu0CopyVector( pvecPrim, pvecPos);

	    (*pfSize) = (*pfSize) * 0.99f + work->fAimSize * 0.01f;

	    if ( pvecPrim->vy < work->fFallY ){
		ResetPos( work, pvecPrim, pvecDir, pfSpeed, pfSize, uvrgbwh);
	    }

//	    pvecPos++;
	    pvecDir++;
	    pfSpeed++;
	    pfSize++;

	    pvecPrim++;
	    pvecPrimPre++;
	    uvrgbwh++;
	}
    }
#if 0
GV_SET_PRFC_CLOCK();
#endif

    ActRingBody(work, work->nRingNum, work->prim_body, work->prim_pole, work->vecBodyPos, work->vecBodyDir, 
		work->fBodySpeed, work->fBodySize, work->nBodyDelay, BODY_RAD, BODY_HEIGHT);

    ActBubble( work);

    work->nCntr++;
#if 0
    DEBUG_Locate( 240, 10, 0 );
    DEBUG_Printf("KMFEATHER-EXECTIME = %.3f\n", (float)GV_GET_PRFC_CLOCK() / 60.f / 262.f );
    if ( GV_Time % 300 == 0){
	FMATRIX mat;
	extern void* NewOneSpread( FMATRIX*,
				   int,	
				   int,
				   float,
				   float,
				   int,	
				   int,	
				   float,
				   float,
				   float,
				   int,	
				   float,
				   float,
				   float,
				   float);
	DG_SetPos2( &GM_PlayerControl->mov, &GM_PlayerControl->rot);
	DG_GetPos( &mat);
	NewOneSpread( &mat, 32, 32,
		      work->fSpreadMin, work->fSpreadMax,
		      40, 80, 100.f, 200.f, 1.f / 8.0f, 0x80808040, 
		      4.f, 8.f, 5.f, 10.f);
    }
#endif
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

    if ( work->vecBodyPos != NULL) 	GV_Free( work->vecBodyPos);
    if ( work->vecBodyDir != NULL) 	GV_Free( work->vecBodyDir);
    if ( work->fBodySpeed != NULL) 	GV_Free( work->fBodySpeed);
    if ( work->fBodySize  != NULL) 	GV_Free( work->fBodySize);
    if ( work->nBodyDelay != NULL) 	GV_Free( work->nBodyDelay);

    if ( work->vecBubblePos != NULL) 	GV_Free( work->vecBubblePos);
    if ( work->vecBubbleStep != NULL) 	GV_Free( work->vecBubbleStep);
    if ( work->fBubbleSpeed != NULL) 	GV_Free( work->fBubbleSpeed);
    if ( work->nBubbleLife != NULL) 	GV_Free( work->nBubbleLife);
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
    work->nBubbleNum 	= GCL_GetOptionValue( 'e', 12 ); // 泡数

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

    {
	FVECTOR* 	pvecDir = work->vecDir;
	float*   	pfSpeed = work->fSpeed;
	float*   	pfSize  = work->fSize;
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

		pvecDir++;
		pfSpeed++;
		pfSize++;
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
    //    tex = DG_GetTexture( 12020883 );
//    tex = DG_GetTexture( 7733153 );
    tex = DG_GetTexture( 15638432);
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
//    tex = DG_GetTexture( 6400230);
//    tex = DG_GetTexture( 10984814);
//    tex = DG_GetTexture( 15638432);
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

    //------------------------------------- 泡
    // 泡位置ワーク
    if( ( work->vecBubblePos = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nBubbleNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Bubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecBubblePos, sizeof(FVECTOR) * work->nBubbleNum );

    // 泡移動ベクトルワーク
    if( ( work->vecBubbleStep = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nBubbleNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Bubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecBubbleStep, sizeof(FVECTOR) * work->nBubbleNum );

    // 泡速度ワーク
    if( ( work->fBubbleSpeed = (float *)GV_Malloc(sizeof(float) * work->nBubbleNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Bubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fBubbleSpeed, sizeof(float) * work->nBubbleNum );

    // 泡生存時間ワーク
    if( ( work->nBubbleLife = (short *)GV_Malloc(sizeof(short) * work->nBubbleNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! Bubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nBubbleLife, sizeof(short) * work->nBubbleNum );

    {	// 泡
	extern void* NewWaterBubble( int, FVECTOR*, short*, int, int);
	GV_SetActorChild( work, NewWaterBubble( work->name, work->vecBubblePos, work->nBubbleLife, work->nBubbleNum, 0));
    }
    work->nSpread = 0;
    _sceVu0CopyMatrix( &work->matSpread, &work->matOrg);
    work->fSpreadMin = 200.f;
    work->fSpreadMax = 300.f;

    work->nPlSpread = 0;
    work->nPlSpreadFlag = 1;
    work->nPlSpreadName = GV_StrCode("プレイヤー流水飛抹");
    _sceVu0CopyMatrix( &work->matPlSpread, &work->matOrg);
    work->fPlSpreadMin = 50.f;
    work->fPlSpreadMax = 100.f;

    work->ucFallSe = 0;
    work->ucHitSe = 0;

    work->nFallSeTime = DG_TickCount;
    work->nHitSeTime = DG_TickCount;

    work->nFallSeIntv = COUNT_VMODE( TBL_SE_INTV[ MAO_Rand(TBL_SE_MAX)]); // COUNT_VMODE( 15 + MAO_GetRandom( -8, 8));
    work->nHitSeIntv = COUNT_VMODE( TBL_SE_INTV[ MAO_Rand(TBL_SE_MAX)]); // COUNT_VMODE( 15 + MAO_GetRandom( -8, 8));

    work->fMoveDropX = 0.f;
    work->fMoveDropY = 0.f;
    return 0;
}

void* NewWaterFall( int name, int map )	
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

