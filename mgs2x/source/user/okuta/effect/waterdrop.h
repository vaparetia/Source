/*
    waterdrop.h
    水滴:ピチョン君関連ヘッダ
    2001/07/27 Masafumi Okuta
    $Id: waterdrop.h,v 1.1.1.3 2002/11/19 11:47:54 Yoshizawa1 Exp $
*/

#ifndef __WATERDROP_H__
#define __WATERDROP_H__


extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))


#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define SIZE		(40)
#define GRAVITY		(9.8f * 10.f)
#define LIFE		(COUNT_VMODE(120))

#define WDP_CHK_HZX  (HZX_CHK_RECOIL_TYPE | HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR)

#define MAX_BOUND 	(32)

#define MAX_DROPS	(32)

#define N_PRIMS		(1)
#define N_VERTS		(MAX_DROPS)

typedef struct _WATERDROP_MNG {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;				// 名前ID
    int			map;				// マップID
    GV_MSG*		msg;				// 自分宛メッセージ	
    
    // 拡張管理
    int			nBoundNum;			// バウンディング数
    int			nSleep;				// スリープモード
    int			nPlayerIndex;			// プレイヤーがいるバウンディング番号 -1 ならいない
    int			nCntr[MAX_BOUND];		// カウンタ
    int			nTime[MAX_BOUND];		// カウンタ

    // データ
    int			nIntvMin[MAX_BOUND];		// 間隔最小
    int			nIntvMax[MAX_BOUND];		// 間隔最大
    FVECTOR		vecMinTbl[MAX_BOUND];		// 発生バウンディング最小点
    FVECTOR		vecMaxTbl[MAX_BOUND];		// 発生バウンディング最大点

    // 登録
    int			nDropNum;			// 水滴発生数
    FVECTOR*		vecDrop[MAX_DROPS];		// 位置データ

    // 描画系
    DG_PRIM2*		primAdd;			// 水滴加算
    DG_PRIM2*		primSub;			// 水滴減算
} WATERDROP_MNG;

typedef struct _WATERDROP {
    // 基本システム
    GV_ACT_EX		actor;
    
    // 拡張管理
    int			nCntr;				// カウンタ		
    float		fKillY;				// 消す高さ
    FVECTOR		vecPos;				// 位置
    FVECTOR		vecFloor;			// 着地法線

    WATERDROP_MNG*	pWdmng;				// マネージャ
} WATERDROP;


#endif // end of __WATERDROP_H__
