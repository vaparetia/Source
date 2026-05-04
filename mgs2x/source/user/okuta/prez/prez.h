/*
    prez.h
    大統領
    2001/04/23 Masafumi Okuta
    $Id: prez.h,v 1.1.1.3 2002/11/19 11:48:08 Yoshizawa1 Exp $
*/
#ifndef __PREZ_H__
#define __PREZ_H__

#define MODEL_NAME	(11771545)	// GV_StrCode("jam_def_sh_mt") : 大統領シングルウェイト
#define MW_MODEL_NAME	(5480089)	// GV_StrCode("jam_def_mh_mt") : 大統領マルチウェイト
#define BASE_MOTION	(111757)	// GV_StrCode("jam") 	       : 基本モーション
#define DAMAGE_MOTION	(111757)	// GV_StrCode("jam")           : ダメージモーション
#define CAPTURE_MOTION	(137569)	// GV_StrCode("emacap")        : つかみモーション
#define DRAG_MOTION	(4452007)	// GV_StrCode("emadrag")       : 
#define PREZ_FAR_NAME	(2619877)	// GV_StrCode("jam_face")      : 顔
#define FACEANIME_NAME	(10253221)	// GV_StrCode("大統領顔アニメ") 

#define CHILD_TARGET_NUM	(11)		// 子ターゲット数

// パラメータ関連
#define PREZ_LIFE		(20)		// 耐久値
#define PREZ_FAINT		(10)		// 気絶値
#define PREZ_FAINT_COUNT	(60*30)		// 気絶時間
#define PREZ_SLEEP_COUNT	(60*60*1 + 3)	// 睡眠時間
#define PREZ_DOWN_DAM		(10)		// ダウンダメージ

#define PREZ_NOISE_POS_MAX (16)			// 聞き耳ポイント最大数
#define PREZ_PANIC_POS_MAX (64)			// パニック逃げポイント最大数
#define PREZ_STRM_MAX 	(32) 			// ストリーム最大数

#define PREZ_TUMBLE_TIME    COUNT_VMODE(180) 	// 転倒継続
#define PREZ_SCARED_TIME    COUNT_VMODE(300) 	// 怖がりしゃがみ時間

#define PREZ_VIEW_DIR 	(256)			// 可視角度
#define PREZ_NIKITA_EVADE	(2000)		// ニキータ回避
#define PREZ_NIKITA_DIV_HEAIGHT	(1000)		// ニキータ上下ルート分岐高さ

/*----------------------------------------------------------------*/
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

enum {	// work->think1
TH1_NORMAL,  		// ノーマル
TH1_DAMAGE, 		// ダメージ
};
enum {	// work->think2
TH2_ATARAXIA, 		// 平静
TH2_PANIC, 		// パニック
TH2_NORMAL_DAMAGE, 	// ダメージ（通常）
TH2_SYSTEM_DAMAGE, 	// ダメージ（システム）
};
enum {	// work->think3
TH3_READY, 		// 準備
TH3_STAND, 		// 立ち
TH3_MOVE, 		// 移動
TH3_ROUTE_STAND, 	// ルート待機
TH3_ROUTE_MOVE, 	// ルート移動
TH3_LISTEN_WAIT, 	// 聞き耳移動までのブランク	5
TH3_LISTEN, 		// 聞き耳	
TH3_NOTICE, 		// 注目
TH3_FIND, 		// 発見
TH3_SCARED, 		// 怖がり
TH3_TUMBLE, 		// 転倒				10
TH3_FOOTUP,		// 足あげ
TH3_SIDESTEP_L,		// 横飛び左
TH3_SIDESTEP_R,		// 横飛び右
TH3_BEHIND_B,		// 背中張りつき	
TH3_BEHIND_L,		// 左壁張りつき	
TH3_BEHIND_R,		// 右壁張りつき		
TH3_SLIDE,		// スライディング		
TH3_SUMMER,		// サマーソルト		
TH3_SLEEP,		// ねている	
TH3_AWAKE,		// 目がさめる
TH3_OVERLOOK,		// 見渡す
TH3_WAIT, 		// ダメージ待機 
TH3_WAKEUP,		// 起き上がり  	
};

enum{ // 反応
REACT_BASE,		// しゃがむorあしあげ
REACT_BEHIND,		// 張りつき
REACT_SIDESTEP,  	// 横飛び
REACT_TUMBLE,		// 転倒
REACT_SPECIAL,		// スライディングorサマーソルト
};

enum { // アクション 
PAD_NONE,	// 空:システム上存在
PAD_MOVE,	// 移動
PAD_RUN,	// 走り
PAD_FIND,	// ニキータ発見
PAD_LISTEN,	// 聞き耳
PAD_NOTICE,	// 注目
PAD_SCARED,	// おびえる
PAD_WAKEUP,	// 起き上がる		
PAD_TUMBLE,	// 転倒
PAD_FOOTUP,	// 足あげ
PAD_BEHIND_B,	// 背中張りつき
PAD_BEHIND_L,	// 左壁張りつき
PAD_BEHIND_R,	// 右壁張りつき
PAD_SLIDE,	// スライディング
PAD_SUMMER,	// サマーソルト
PAD_SIDESTEP_L,	// 横飛び左
PAD_SIDESTEP_R,	// 横飛び右
PAD_SLEEP,	// ねている
PAD_AWAKE,	// 起きる
PAD_OVERLOOK,	// きょろきょろ
};

enum{ // エリア
PREZ_AREA_1,	// 西壁上
PREZ_AREA_2,	// 西壁下
PREZ_AREA_3,	// 北壁上
PREZ_AREA_4,	// 北壁中
PREZ_AREA_5,	// 北壁下
PREZ_AREA_6,	// 電気板への通路
PREZ_AREA_7,	// 電気板前
PREZ_AREA_8,	// 電気板前下
PREZ_AREA_MAX,
};


enum { // ターゲットサイズインデックス
    PREZ_TARGET_SIZE_STAND,	// 立ち状態
    PREZ_TARGET_SIZE_DOWN,	// ダウン状態 
    PREZ_TARGET_SIZE_SQUAT,	// しゃがみ
    PREZ_TARGET_SIZE_FOOTUP,	// 足あげ
} ;

enum { // メッセージ種類
PREZ_MESSAGE_SLEEP,		// 寝かせる
PREZ_MESSAGE_CHG_ROUTE,		// ルート変更	
PREZ_MESSAGE_ROUTE_QUICK,	// すぐにルート変更
};

enum{ // ストリーム	
PREZ_STRM_PANIC01, 				// 「な、なんだ！？」
PREZ_STRM_PANIC02, 				// 「何をするつもりだ！？」
PREZ_STRM_PANIC03, 				// 「やめてくれ！！」
PREZ_STRM_PANIC04, 				// 「うわ！」
PREZ_STRM_PANIC05, 				// 「来るな！」
PREZ_STRM_NOISE01, 				// 「なんだ？」
PREZ_STRM_NOISE02, 				// 「音がした？」
PREZ_STRM_NOISE03, 				// 「誰かいるのか？」
PREZ_STRM_NOISE04, 				// 「誰だ？」
PREZ_STRM_NOISE05, 				// 「うるさい！！」
PREZ_STRM_COMP01,  				// 「なぜこんなことに……」
PREZ_STRM_COMP02,  				// 「奴め……」
PREZ_STRM_COMP03,  				// 「くそっ……！」
PREZ_STRM_COMP04,  				// 「（舌打ち。ちっ！等、数パターン）」
PREZ_STRM_COMP05,  				// 「（ため息。ふぅ……等、数パターン）」
PREZ_STRM_COMP06,  				// 「（ため息。ふぅ……等、数パターン）」
PREZ_STRM_COMP07,  				// 「（ため息。ふぅ……等、数パターン）」
PREZ_STRM_GAMEOVER = PREZ_STRM_PANIC04, 	// げーむおーばー
};

enum{ // 顔アニメ
PREZ_FACE_EYE_CLOSE,		// 目閉じ
PREZ_FACE_EYE_SCARE,		// 怖がる
PREZ_FACE_EYE_THINK,		// 考え中
PREZ_FACE_EYE_SURPRISE,		// 驚く
PREZ_FACE_EYE_BLINK,		// まばたき
};

#define PREZ_BOMB_DMG	      (20)		// ダメージ

#define PREZ_NOISE_LEVEL_MAX  (4)		// きき耳いらいら最大レベル
#define PREZ_NOISE_LEVEL_TIME (COUNT_VMODE(90))	// きき耳いらいらの回復時間

typedef	struct	{ // 大統領ワーク
    GV_ACT_EX		actor;
    OBJECT		body;
    CONTROL		control;
    FMATRIX		lights[2];		// キャラライト
    int			name;
    int			map;
    
    NPCWORK		npc;			// NPCシステム
    NAVIGATE 		navigate;		// ナビゲート
    NAVITARGET		navitrg;		// ナビゲート
    ROUTENAVI		rnavi;			// ルートナビ
    RADAR_CTRL		rctrl;			// レーダー制御

    TARGET		deftrg;			// 防御ターゲット 
    CAPTURE_TARGET	capture;		// 捕まりターゲット 
    GM_GageSet		gage;			// ゲージ

    TARGET		def_child[CHILD_TARGET_NUM];	// 子ターゲット
    
    int			headmark;		// ヘッドマーク

    int			nRouteWaitTime;		// ルート待ち時間

    int			nComplaintCntr;		// 愚痴時間カウンタ
    int			nComplaintMax;		// 愚痴時間

    int			nDownCntr;		// 倒れ時間カウンタ
    int			nDownTime;		// 倒れ時間

    int			nHeadInterp;		// 頭補間用カウンタ
    int			nHeadAdj;		// 頭アジャスト
    int			nNoticeCntr;		// 注目カウンタ
    SVECTOR		vecAdjust;		// アジャスト
    FVECTOR 		vecFacePos;		// 頭の位置

    int			nNoiseCntr;		// 雑音カウンタ
    int			nNoiseLevel;		// 雑音レベル

    int			nFaceChara;		// 顔キャラ名
    int			nFaceMess;		// 顔キャラメッセージ

    int			think1;			// システムレベル
    int			think2;			// モードレベル
    int			think3;			// 個々の思考
    int			count3;			// 個々の思考内カウンタ

    int 		nNextThink3;		// 次候補になる思考
    int			bNextThink3Valid;	// 次候補になる思考が有効かどうか
    int 		nNextZone;		// 次に向かうゾーン
    FVECTOR		vecNextPos;		// 次に向かう位置

    int			pl_dis;			// プレイヤーとの距離 
    int			pl_dir;			// プレイヤーへの方向 

    int			nDifficult;		// 難易度
    int			bDead;			// 死亡フラグ

    FVECTOR		vecLastBombPos; 	// 爆発地点

    int			bMotEnd;		// 持続モーション系終了フラグ

    // ニキータ関連
    int			bNikita;		// ニキータ存在フラグ
    FVECTOR		vecNikita;		// 一番近いニキータ(一つしか出ないので存在するニキータ位置になる)
    int 		nNikDir;		// 一番近いニキータのいるY方向
    int 		nNikDist;		// 一番近いニキータまでの距離
    FVECTOR		vecDiffNik;		// 一番近いニキータのいる方向
    int			bSeeNikita;		// ニキータ見える	
    int			bFindNikita;		// ニキータ発見
    int			nReactThink;		// 反応思考

    // サウンド関連
    int			nSoundID;			// サウンドからのキャラ判別用ID
    int			nSeCntr;			// 内蔵SEカウンタ:０で再生可能
    int			nStrmHandle;			// ストリームハンドラ
    int			nStrmIndex;			// 現在のストリームインデックス
    int			nStrmIndexNext;			// 次のストリームインデックス
    int			bStrmProcFlag;			// ストリームプロシージャフラグ
    int			nStrmProc;			// ストリームプロシージャ
    int			nStrmCntr;			// ストリームカウンタ:０で再生可能
    int			nStrmDataNum;			// ストリームデータ数
    int			nStrmNo[PREZ_STRM_MAX];		// ストリーム番号:GCL指定
    int			nStrmFlag[PREZ_STRM_MAX];	// ストリーム再生:GCL指定
    int			nStrmLen[PREZ_STRM_MAX];	// ストリーム長さ:GCL指定

    // GCLからの拡張データ
    int			nMaxLife;				// 耐久値
    int			procOver;				// 大統領死亡時のゲームオーバプロシージャ
    int			procFirstKnock;				// 大統領初回音反応プロシージャ
    int			nReactNikDist;				// ニキータ反応距離
    int			nReactBombDist; 			// 爆発音反応距離
    int			nNoiseTime;				// 聞き耳時間
    int			nNoticeTime;				// 聞き耳時間
    int			nPanicTime;				// パニック時間
    int			nNoisePosNum;				// 聞き耳ポイント数
    int			nPanicPosNum;				// パニック逃げポイント数
    FVECTOR		vecRelay;				// リレーポイント
    int			nRelayZone;				// リレーゾーン
    FVECTOR		vecNoisePos[PREZ_NOISE_POS_MAX];	// 聞き耳ポイント
    int			nNoiseZone[PREZ_NOISE_POS_MAX];		// 聞き耳ポイント
    int			nNoiseDir[PREZ_NOISE_POS_MAX];		// 聞き耳ポイント
    FVECTOR		vecPanicPos[PREZ_PANIC_POS_MAX];	// パニック逃げポイント
    int			nPanicZone[PREZ_PANIC_POS_MAX];		// パニック逃げポイント
    int			nEscArea[PREZ_AREA_MAX][PREZ_AREA_MAX];	// 逃げエリア
    int			nReact[PREZ_AREA_MAX][PREZ_AREA_MAX];	// 反応
    FVECTOR		vecNoiseTrap[2];			// ノイズ検知範囲バウンディングボックス

    // イベント用
    int			bFirstKnockEnd;				// 初回音反応終了
} Work;

#ifdef DEBUG_MODE
static int nDbgPrezInfoDraw = 0;
static int nDbgPrezLevelDraw = 0;
static int nDbgPrezTrgDraw = 0;
static int nDbgPrezMutekiDraw = 0;
enum{
PREZ_DRAW_OFF,
PREZ_DRAW_TRG,
};
enum{
PREZ_LEVEL_VERYEASY,
PREZ_LEVEL_EASY,
PREZ_LEVEL_NORMAL,
PREZ_LEVEL_HARD,
PREZ_LEVEL_EXTREME,
};
enum{
PREZ_DBGTRG_OFF,
PREZ_DBGTRG_VIEW,
};
enum{
PREZ_DBGMUTEKI_OFF,
PREZ_DBGMUTEKI_ON,
};
#endif // end of DEBUG_MODE

#endif // end of __PREZ_H__

