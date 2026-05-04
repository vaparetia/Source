/*
    wdust.h
    水中ゴミマネージャ
    2001/07/21 Masafumi Okuta
    $Id: wdust.h,v 1.1.1.3 2002/11/19 11:47:55 Yoshizawa1 Exp $
*/

#ifndef __WDUST_H__
#define __WDUST_H__
    

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))


#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

// ワーク
#define N_UNIT		(4)	// ユニット数
#define N_POS		(32)	// 位置データ数
#define N_PRIM		(64)	// プリミティブ数
#define N_VERTS		(32)	// スプライト数

#define COL_R		(164)
#define COL_G		(164)
#define COL_B		(128)
#define COL_A		(32)
#define COL_ADD_A	(32)

// スプライトサイズ
#define BASE_SIZE	(15.f)		
#define ADD_SIZE	(5.f)		 

#define MAX_DIST	(200.f)
#define AFFECT_BASE	(200.f)
#define AFFECT_ADD	(400.f)

#define PUSH_LEVEL	(4.f)
#define PLSWIM_LEVEL	(0.8f)

#define	SCR_POS		(SCRPAD_ADDR + 0x1000)	// 位置 0x400 使用
#define	SCR_SPD		(SCRPAD_ADDR + 0x2000)	// 速度 0x400 使用

// チェックしないフロア
#define WDUST_HZX_CHK_TYPE	(HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR)
#define WDUST_HZX_PLAYER_ONLY	(HZX_CHK_NOCHECK_FLOOR)

#define VISIBLE_CAM_DIST (12000)

// マネージャ
typedef	struct	_WDUST_MNG{
    // 基本システム
    GV_ACT_EX	actor;
    int		name;				// 名前
    int		map;				// 所属マップ
    int		hzx_id;				// ハザードID

    // 拡張システム
    int		nWorkNum;			// ユニット数
    int		nPrimNum;			// プリミティブ数
    int		nPrevIndex[N_UNIT];		// １フレーム前の表示するインデックス列
    int		nIndexTbl[N_UNIT];		// 表示するインデックス列

    // プレイヤー情報
    FMATRIX  	matPlNode;			// プレイヤーノード
    FMATRIX  	matPlNodePre;			// プレイヤーノード1フレーム前	
    FMATRIX  	matPlMove;			// プレイヤーノード移動ベクトル
    FVECTOR	vecPlMoveDist;			// プレイヤーノード移動距離

    // データワーク
    int		nDataNum;			// データ数
    FVECTOR	vecPosTbl[N_POS];		// 中心位置
    FVECTOR	vecSizeTbl[N_POS];		// サイズ
    FVECTOR	vecBoundMinTbl[N_POS];		// バウンド最小
    FVECTOR	vecBoundMaxTbl[N_POS];		// バウンド最大
}WDUST_MNG;

typedef	struct	_WATERDUST{
    // 基本システム
    GV_ACT_EX	actor;
    int		name;				// 名前
    int		map;				// 所属マップ
    int		hzx_id;				// ハザードID

    // 拡張システム
    int		nPrimNum;			// プリミティブ数
    FVECTOR	vecSize;			// 判定箱サイズ
    FVECTOR	vecCenter;			// 判定箱中心
    FVECTOR	vecBound1;			// 判定箱最小
    FVECTOR	vecBound2;			// 判定箱最大

    int		nIndex;				// 通し番号

    float	fOffDist;			// 計算を止める距離

    // プレイヤー情報
#if 0
    FMATRIX  	matPlNode;			// プレイヤーノード
    FMATRIX  	matPlNodePre;			// プレイヤーノード1フレーム前	
    FMATRIX  	matPlMove;			// プレイヤーノード移動ベクトル
    FVECTOR	vecPlMoveDist;			// プレイヤーノード移動距離
#endif
    // 可変ワーク
    float*	fAffectDist;			// 影響距離
    FVECTOR*	vecInitPos;			// プリミティブの中心の初期座標			N_UNIT_VERTS
    FVECTOR*	vecSpeed;			// 移動ベクトル					N_UNIT_VERTS

    // 描画関連
    DG_PRIM2*	prim;				// 本体

    // 外部との通信
    WDUST_MNG*  wdmng;				// マネージャへのポインタ
}WATERDUST;

#endif // end of __WDUST_H__
