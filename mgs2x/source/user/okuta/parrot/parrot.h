/*
    parrot.h
    オウム関連ヘッダ
    2001/05/12 Masafumi Okuta
    $Id: parrot.h,v 1.1.1.3 2002/11/19 11:48:07 Yoshizawa1 Exp $
*/

#ifndef __PARROT_H__
#define __PARROT_H__

#define MODEL_NAME	(8943251)	// GV_StrCode("bird_cage")       : オウムシングルウェイト
#define MW_MODEL_NAME	(15282852)	// GV_StrCode("par_def_mh")    : オウムマルチウェイト
#define CAGE_MODEL_NAME	(8943251)	// GV_StrCode("bird_cage")     : とりかご
#define BASE_MOTION	(4904762)	// GV_StrCode("parrot")	       : 基本モーション
#define DAMAGE_MOTION	(4904762)	// GV_StrCode("parrot")        : ダメージモーション
#define CAPTURE_MOTION	(4904762)	// GV_StrCode("parrot")        : つかみモーション
#define DRAG_MOTION	(4904762)	// GV_StrCode("parrot")       : 
#define SMP_FACE_FAR	(107009)	// GV_StrCode("ema") 	       : 顔

#define CHILD_TARGET_NUM	(11)	// 子ターゲット数

#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

#define PAR_LIFE	(20)
#define PAR_FAINT	(10)
#define PAR_FAINT_COUNT	(60*30)
#define PAR_SLEEP_COUNT	(60*60*1 + 3)
#define PAR_DOWN_DAM	(10)

#define PAR_STRM_MAX 	(32) 			// ストリーム数

#define PAR_VIEW_DIR 	(256)			// 可視角度

#define PAR_CALL_ENE	(3) 		  	// 敵兵呼びまでのカウンタ
#define PAR_CALL_INTV	(COUNT_VMODE(180)) 	// 敵兵呼び間隔
#define PAR_CALL_DELAY  (COUNT_VMODE(30))	// 敵兵呼びディレイ


enum {	// 思考レベル:システムレベル	
TH1_NORMAL,  		// ノーマル
TH1_DAMAGE, 		// ダメージ
};
enum {	// work->think2
TH2_ATARAXIA, 		//
TH2_PANIC, 		//
TH2_SAMPLE_DAMAGE, 	//	
TH2_SYSTEM_DAMAGE, 	//
};
enum {	// work->think3
TH3_READY, 		// 準備
TH3_STAND, 		// 待機
TH3_SAY, 		// 喋る
TH3_ERO_LISTEN,		// エロ本聞き耳
TH3_ERO_SPEAK,		// エロ本ありかを語る
TH3_EAMES_LISTEN,	// あんたがエイムズラーニング
TH3_EAMES_SPEAK,	// あんたがエイムズ
TH3_ENEMY_CALL,		// 敵兵呼出
TH3_FLIPFLOP,		// ばたばた
TH3_SLEEP, 		// 眠る
TH3_WAIT, 		// ダメージ待機 
TH3_WAKEUP,		// 起き上がり  
};

/* アクション */
enum {
PAD_NONE,
PAD_IDLE,	// 待機
PAD_ATTENTION,	// 注目
PAD_TALK,	// しゃべり
PAD_FLIPFLOP,	// ばたつく
PAD_SLEEP,	// ねている
PAD_WAKEUP,	// 起きる
};


enum{ // エリア
PAR_AREA_1,	// 西壁上
PAR_AREA_2,	// 西壁下
PAR_AREA_3,	// 北壁上
PAR_AREA_4,	// 北壁中
PAR_AREA_5,	// 北壁下
PAR_AREA_6,	// 電気板への通路
PAR_AREA_7,	// 電気板前
PAR_AREA_8,	// 電気板前下
PAR_AREA_MAX,
};

enum {
PAR_TARGET_SIZE_STAND,	// 立ち状態
PAR_TARGET_SIZE_DOWN,	// ダウン状態 
PAR_TARGET_SIZE_SQUAT,	// しゃがみ
} ;

enum{ // 喋り内容
PAR_SUBJECT_NORM,	// 通常
PAR_SUBJECT_ERO,	// エロ本ネタ
PAR_SUBJECT_EIMS,	// エイムズネタ
};

enum { // メッセージ種類
PAR_MESSAGE_TALK_ON,		// 喋り許可
PAR_MESSAGE_TALK_OFF,		// 喋り不許可
PAR_MESSAGE_ERO_LISTEN,		// エロ本聞く
PAR_MESSAGE_EAMES_LISTEN,	// エイムズ聞く
PAR_MESSAGE_IDLE,		// アイドリング
PAR_MESSAGE_ERO_ALLOW,		// エロ本喋り許可
PAR_MESSAGE_EAMES_ALLOW,	// エイムズ喋り許可
PAR_MESSAGE_NOISE_RATE,		// 物音発生確率
PAR_MESSAGE_SUBJECT_RATE,	// 喋り内容の割合
} ;

typedef	struct	{ // オウムワーク
    GV_ACT_EX		actor;
    OBJECT		body;
    CONTROL		control;
    FMATRIX		lights[2];
    
    NPCWORK		npc;
    NAVIGATE 		navigate;
    NAVITARGET		navitrg;
    RADAR_CTRL		rctrl;

    TARGET		deftrg;				// 防御ターゲット 
    TARGET		pushtrg;			// ぶつかりターゲット 
    TARGET		dummytrg;			// 弾検知用ダミーターゲット 
    CAPTURE_TARGET	capture;			// 捕まりターゲット 
//  GM_GageSet		gage;				// ゲージ
    HOMING_TRG		homing;				// ホーミングターゲット

    FVECTOR 		vecInitPos;			// 止まり位置


    // とりかご
    OBJECT		objCage;			// とりかご
    FMATRIX		matCage;			// とりかごマトリクス
    int			nInterpCage;			// とりかご回転補間
    SVECTOR		vecCageRot;			// とりかご揺れ量
    SVECTOR		vecCageRotAim;			// とりかご揺れ量

    int			map;				// 
    
    int			headmark;			// 

    FVECTOR 		vecFacePos;			// 頭の位置

    // sampleキャラクター専用 
    int			think1;
    int			think2;
    int			think3;
    int			count3;
    int			bInit;				// 思考初期化フラグ
    int 		nNextThink3;
    int			bNextThink3Valid;		
    FVECTOR		vecAimPos;			// 注視点
    SVECTOR		svecAdjust;

    int			nIdleTime;			// アイドリング時間

    int			mode;				// シナリオ実験用 

    int			pl_dis;				// プレイヤーとの距離 
    int			pl_dir;				// プレイヤーへの方向 

    int			nDifficult;			// 難易度

    // イベントフラグ関連
    int			nAttackCntr;			// 狙われカウンタ
    int			nAttackNum;			// 狙われた回数
    int			nPlayerAttention;		// プレイヤー注目
    int			nEroLearn;			// エロ本イベント用
    int			nEamesLearn;			// エイムズイベント用
    int			nNoiseRate;			// 物音確率

    int			nCallEnemyFlag;			// 敵呼びフラグ
    int			nCallEnemyCntr;			// 敵呼びカウンタ

    // サウンド関連
    int			nTalkFlag;			// 喋りフラグ
    int			nStrmMode;			// 音声モード
    int			nSoundID;			// サウンドからのキャラ判別用ID
    int			nSeCntr;			// 内蔵SEカウンタ:０で再生可能
    int			nStrmHandle;			// ストリームハンドラ
    int			nStrmIndex;			// 現在のストリームインデックス
    int			nStrmIndexNext;			// 次のストリームインデックス
    int			bStrmProcFlag;			// ストリームプロシージャフラグ
    int			nStrmProc;			// ストリームプロシージャ
    int			nStrmCntr;			// ストリームカウンタ:０で再生可能
    int			nStrmDataNum;			// ストリームデータ数
    int			nStrmNo[PAR_STRM_MAX];		// ストリーム番号:GCL指定
    int			nStrmFlag[PAR_STRM_MAX];	// ストリーム再生:GCL指定
    int			nStrmLen[PAR_STRM_MAX];		// ストリーム長さ:GCL指定

    int			nSubjectRate[3];		// 喋りの内容レート

    // 拡張データ
    int			nMaxLife;			// 耐久値
    int			nAttentionTime;			// 注目時間
    int			nTalkBlank1;			// 喋りブランク最小
    int			nTalkBlank2;			// 喋りブランク最大
} PARROT;

enum{ // ストリーミング用インデックス
PAR_STRM_TALK01,  // 　　「オハヨ、オハヨ」
PAR_STRM_TALK02,  // 　　「コンニチハ！」
PAR_STRM_TALK03,  // 　　「調子はどう？」
PAR_STRM_TALK04,  // 　　「オヤツオヤツ～！」
PAR_STRM_HELP01,  // 　　「サイテー！」

PAR_STRM_RARE,    // 　　「金星蟹！」
PAR_STRM_ENEMY01, // 　　「敵を発見、応援を頼む！」
PAR_STRM_ENEMY02, // 　　「敵だ！誰か来てくれ！」
PAR_STRM_ENEMY03, // 　　「配置に戻れ！」
PAR_STRM_EAMES01, // 　　「あんたがエイムズだな」

PAR_STRM_EAMES02, // 　　「あんたがエイムズだなあんたがエイムズだな」
PAR_STRM_ERO01,   // 　　「スゴイスゴイ」
PAR_STRM_ERO02,   // 　　「ボインボイン」
PAR_STRM_ERO03,   // 　　「ロッカーの上、ロッカーの上」
PAR_STRM_OTACON,  // 　　「お兄ちゃん、お兄ちゃん」
};
#ifdef DEBUG_MODE
#define PAR_StreamRequest( work, nStrm) { printf(" Request = %s : %d \n", __FILE__, __LINE__);  PAR_StreamReq( work, nStrm); }
#else
#define PAR_StreamRequest( work, nStrm) { PAR_StreamReq( work, nStrm); }
#endif

#endif // __PARROT_H__
