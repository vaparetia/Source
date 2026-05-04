/*
    kamome.h
    カモメヘッダ
    2001/05/02 Masafumi Okuta
    $Id: kamome.h,v 1.1.1.3 2002/11/19 11:48:02 Yoshizawa1 Exp $
*/

#ifndef __KAMOME_H__
#define __KAMOME_H__

enum{ // モデル名
KMM_MDLNAME_NORM,	// 通常
KMM_MDLNAME_LIGHT,	// 簡易
KMM_MDLNAME_ONE1,	// ワンピースモデル１
KMM_MDLNAME_ONE2,	// ワンピースモデル２
KMM_MDLNAME_MAX,	// 最大数	
};

#define MODEL_NAME	(8885877)	// GV_StrCode("kmo_noalp") : 2177191
#define BASE_MOTION	(4737761)	// GV_StrCode("kamome")  : 4737761
//#define MODEL_NAME	(2185720)	// GV_StrCode("kmo_low") : 
//#define MODEL_NAME	(2177191)	// GV_StrCode("kmo_def") : 2177191
//#define MW_MODEL_NAME	(8885877)	// GV_StrCode("kmo_noalp") : 2177191
#define MASUIDAN_MODEL	(2512988) /* GV_StrCode("m92_bul2") */

#define MAX_KAMOME_NUM (100)	// 最大かもめ数

// オフセット
#define KAMOME_Y_OFFSET		(128.f)
#define KMM_FLOOR_OFFSET 	(70.f)
#define KMM_FLOOR_LIMIT 	(-500000.f)
#define KMM_LIMIT_HEIGHT	(-60000)	// これ以下になったら消す

// 移動関連
#define KMM_ROUTE_MAX  		(16)			// ルート数
#define KMM_MOVE_TIME  		(1)			// ルート移動カウント用
#define KMM_ROUTE_TIME 		(256)			// １ルートに使う時間
#define KMM_DIV_DIST 		(20) 			// 分割基準距離
#define KMM_BASE_SPEED  	( (float)KMM_DIV_DIST )	// 基本速度
#define KMM_SPEED_DEF		(30.f)			
#define KMM_ROT_INTERP 		(30)			// 旋回補間
#define KMM_ROTATE_ANG 		(6)			// 旋回:ベース 8
#define KMM_ROTADD_ANG 		(8.f)			// 旋回:加算分 8
#define KMM_SPDDECAY_RATE 	(0.5f)			// 速度減衰割合

// 傾き
#define KMM_INCLINE_ANG (900)
#define KMM_X_ANG	(256)

#define ELUDE_DIST_CHECK (800)			// エルード時の飛び立ち反能距離

// 補間時間
#define	ACT_INTERP_DEF		0 // 8*5	// 1/300単位 
#define	ACT_INTERP_M		60*1 	// 1/300単位 
#define	ACT_INTERP_SLOWLY	60*5 	// 1/300単位 
#define	ACT_INTERP_VERY_SLOWLY	60*30 	// 1/300単位 

// スクラッチパッド
#define	MEM_SCR_TOP		((void*)(SCRPAD_ADDR))

// モーションレイヤー
#define	LAYER_BASE	0
#define	LAYER_OVER1	1
#define	LAYER_OVER2	2


// 関節
#define KAMOME16_ATAMA	(3)

// 音
#define KMM_SE_ATTACK_NUM	(4)		// 攻撃音最大数
#define KMM_SE_NEAR_NUM		(4)		// 近接かもめ取得最大数
#define KMM_SE_AREA	        (9000.f)	// 近接音鳴らし判定エリア

// 死体またぎ
#define KMM_CORP_CHK_W		(400.f)		// 死体またぎ発動幅
#define KMM_CORP_CHK_H		(1250.f)	// 死体またぎ発動高さ	

#define KMM_ANGRY_NUM		(20)		// これだけ殺したら怒って攻撃


#define KMM_CBOX_TIME		(COUNT_VMODE(360))		// 段ボールチェック時間
#define KMM_SLEEP_MIN		(COUNT_VMODE(3600))		// 最小睡眠時間
#define KMM_SLEEP_MAX		(COUNT_VMODE(4000))		// 最大睡眠時間
#define KMM_FEINT_MIN		(COUNT_VMODE(1200))		// 最小気絶時間
#define KMM_FEINT_MAX		(COUNT_VMODE(1500))		// 最大気絶時間

#define KMM_FALL_ATC_NUM	(20)				// 落下攻撃

#define KMM_FALL_SPD 		(250.f)
#define KMM_FALL_ADD 		(5.f)


// ----- 列挙子 --------------------------------------------------------------------------- 

enum{ // 思考レベル１
TH1_NORMAL,	// 通常
TH1_DAMAGE,	// ダメージ
};

enum{ // 思考レベル２
TH2_NORMAL,	// 通常
TH2_DAMAGE,	// ダメージ
};

enum{ // 思考レベル３
// 待機
TH3_REST,		// 羽安め
TH3_RESTGROUND,		// 地面で羽安め
// 自由飛行
TH3_FLYING,		// 飛行
TH3_HOVERING,		// 滑空
// 特殊動作系
TH3_FLIPMOVE,		// ばたつき移動			
TH3_FLIPFLOP,		// ばたつく			5
TH3_BACKWARD,		// 後退
TH3_LANDMOVE,		// 着地移動
TH3_LANDING,		// 着地
TH3_ATCMOVE,		// 攻撃移動
TH3_ATTACK,		// 攻撃				10
TH3_TAKEOFF,		// 緊急飛び立ち
TH3_EVADE,		// 緊急逃げ
TH3_BACKOFF,		// 緊急後退
TH3_CONCMOVE,		// キャラに付随するまでの移動	
TH3_CONCOMITANT,	// キャラに付随して移動		15
TH3_CAMZ_FIX,		// カメラ方向に固定ホバリング	
TH3_CBOX_LAND,		// 段ボール着地点へ移動
TH3_CBOX_LANDING,	// 段ボール着地
TH3_CBOX_REST,		// 段ボールの上で羽休め		
TH3_ELUDEMOVE,		// エルード妨害移動		20
TH3_ELUDELAND,		// エルード妨害着地		
TH3_ELUDING,		// エルード妨害中
TH3_CTRL_LAND,		// コントロールを持っているキャラに着地移動
TH3_CTRL_LANDING,	// コントロールを持っているキャラに着地羽ばたき
TH3_CTRL_REST,		// コントロールを持っているキャラに着地中	25
TH3_ROUTE_MOVE,		// ルート移動
// 特殊
TH3_FEINTFALL,		// 気絶落下			
TH3_FEINT,		// 気絶				
TH3_SLEEPFALL,		// 睡眠落下			
TH3_GROUNDSLEEP,	// 地面で睡眠			30
TH3_STANDSLEEP,		// 立ったまま睡眠		
TH3_WAKEUP,		// 起きる			
TH3_STANDWAKEUP,	// 立ち状態で起きる		
// ダメージ系
TH3_WAIT,		// 待ち				
TH3_FALL,		// 落下				35
TH3_DEAD,		// 死亡				

// チェック用
TH3_CHECK_FREEMOVE_START 	= TH3_FLYING, 	  	// 自由飛行
TH3_CHECK_FREEMOVE_END   	= TH3_HOVERING, 
TH3_CHECK_CBOX_START 	 	= TH3_CBOX_LAND, 	// 段ボール
TH3_CHECK_CBOX_END 	 	= TH3_CBOX_REST, 
TH3_CHECK_NOISE_REACT_END	= TH3_FLIPFLOP,		// 音反応
TH3_CHECK_TAKEOFF_REACT_END	= TH3_ATCMOVE,		// 飛び立ち反応
TH3_CHECK_DAMAGE_START   	= TH3_WAIT, 	  	// ダメージ
TH3_CHECK_FEINT_START		= TH3_FEINTFALL,	// 気絶開始
TH3_CHECK_SLEEP_START   	= TH3_SLEEPFALL,   	// 睡眠開始
TH3_CHECK_FEINT_END		= TH3_FEINT,		// 気絶終了
TH3_CHECK_SLEEP_END   		= TH3_STANDSLEEP,   	// 睡眠終了
TH3_CHECK_WAKEUP_END   		= TH3_WAKEUP,   	// 睡眠終了
};

enum{ // カモメ初期ルーチン
KMM_INITTHK_REST,	// 羽やすめ
KMM_INITTHK_FLYAWAY,	// 飛び立ち
KMM_INITTHK_FLYING,	// 飛行
KMM_INITTHK_FLIPFLOP,	// ばたつき
KMM_INITTHK_GROUNDREST,	// 地面で羽やすめ
};

enum{ // パッドチェック
PAD_NONE,	// なにもない
PAD_IDLE,	// アイドリング
PAD_IDLEGROUND,	// アイドリング(地面)
PAD_FLYING,	// 飛行
PAD_HOVERING,	// 滑空
PAD_LANDING,	// 着地
PAD_FLIPFLOP,	// ばたつく
PAD_CAUTION,	// 危険ばたつき
PAD_TAKEOFF,	// 緊急飛び立ち
PAD_POKE,	// つつく
PAD_FALL,	// 落下
PAD_GROUNDSLEEP,// 地面で睡眠
PAD_STANDSLEEP, // 立ったまま睡眠
PAD_WAKEUP,	// 起きる
PAD_STANDWAKEUP,// 立ち状態の起きる
PAD_DAMAGE,	// ダメージ
PAD_FEINT,	// 気絶
PAD_DEAD,	// 死亡
};
#if 1
enum{ // モーション
KMM_MOT_IDLE,		// アイドリング
KMM_MOT_IDLE02   = KMM_MOT_IDLE,	// アイドリング２
KMM_MOT_FLYING_START,	// 飛び開始
KMM_MOT_FLYING,		// 飛び
KMM_MOT_FLYING_END,	// 飛び終了
KMM_MOT_FLYING02 = KMM_MOT_FLYING_END,	// 飛び２
KMM_MOT_FLYING03_START,	// 飛び３開始
KMM_MOT_FLYING03,	// 飛び３
KMM_MOT_FLYING03_END,	// 飛び３終了
KMM_MOT_HOVER,		// 滑空
KMM_MOT_HOVER02,	// 滑空２
KMM_MOT_FLYAWAY,	// 飛び去り
KMM_MOT_FLYAWAY_QUICK,	// 飛び去り
KMM_MOT_LANDING,	// 着地
KMM_MOT_WALK_START,	// 歩き
KMM_MOT_WALK_END,	// 歩き終わり
KMM_MOT_WALK_A,		// 歩き１
KMM_MOT_WALK_B = KMM_MOT_WALK_A,	// 歩き２
KMM_MOT_PICK_A,		// つつき１
KMM_MOT_PICK_B = KMM_MOT_PICK_A,	// つつき２
KMM_MOT_SURPRISE,	// 驚き
KMM_MOT_SURPRISE_BACK,	// 驚き後退
//KMM_MOT_ATTACK_A,	// 攻撃１
//KMM_MOT_ATTACK_B,	// 攻撃２
KMM_MOT_ATTACK_C,	// 攻撃３
KMM_MOT_DAM1_START,	// 上空でのダメージ
KMM_MOT_DEAD,		// 
KMM_MOT_DAM1_END,	// 
KMM_MOT_FALL,		// 
KMM_MOT_DAM2_START 	= KMM_MOT_DAM1_START,	// 低空でのダメージ
KMM_MOT_DAM2 		= KMM_MOT_DEAD,		// 
KMM_MOT_DAM2_END 	= KMM_MOT_DAM1_END,	// 
KMM_MOT_FALL2 		= KMM_MOT_FALL,		// 
KMM_MOT_SLEEP1,		// 
KMM_MOT_SLEEP2 		= KMM_MOT_SLEEP1,	// 
KMM_MOT_WAKE_HIGH,	// 起きる
KMM_MOT_WAKE_LOW	= KMM_MOT_WAKE_HIGH,	// 起きる
KMM_MOT_MAX,		// 最大数
};
#else
enum{ // モーション
KMM_MOT_IDLE,		// アイドリング
KMM_MOT_IDLE02,		// アイドリング２
KMM_MOT_FLYING_START,	// 飛び開始
KMM_MOT_FLYING,		// 飛び
KMM_MOT_FLYING_END,	// 飛び終了
KMM_MOT_FLYING02,	// 飛び２
KMM_MOT_FLYING03_START,	// 飛び３開始
KMM_MOT_FLYING03,	// 飛び３
KMM_MOT_FLYING03_END,	// 飛び３終了
KMM_MOT_HOVER,		// 滑空
KMM_MOT_HOVER02,	// 滑空２
KMM_MOT_FLYAWAY,	// 飛び去り
KMM_MOT_FLYAWAY_QUICK,	// 飛び去り
KMM_MOT_LANDING,	// 着地
KMM_MOT_WALK_START,	// 歩き
KMM_MOT_WALK_END,	// 歩き終わり
KMM_MOT_WALK_A,		// 歩き１
KMM_MOT_WALK_B,		// 歩き２
KMM_MOT_PICK_A,		// つつき１
KMM_MOT_PICK_B,		// つつき２
KMM_MOT_SURPRISE,	// 驚き
KMM_MOT_SURPRISE_BACK,	// 驚き後退
KMM_MOT_ATTACK_A,	// 攻撃１
KMM_MOT_ATTACK_B,	// 攻撃２
KMM_MOT_ATTACK_C,	// 攻撃３
KMM_MOT_DAM1_START,	// 上空でのダメージ
KMM_MOT_DEAD,		// 
KMM_MOT_DAM1_END,	// 
KMM_MOT_FALL,		// 
KMM_MOT_DAM2_START,	// 低空でのダメージ
KMM_MOT_DAM2,		// 
KMM_MOT_DAM2_END,	// 
KMM_MOT_FALL2,		// 
KMM_MOT_SLEEP1,		// 
KMM_MOT_SLEEP2,		// 
KMM_MOT_WAKE_HIGH,	// 起きる
KMM_MOT_WAKE_LOW,	// 起きる
KMM_MOT_MAX,		// 最大数
};
#endif
enum{ // かもめの状態
KMM_STATUS_DAMAGE 	= 0x00000001,	// ダメージ中
KMM_STATUS_DEAD   	= 0x00000002,	// 死亡
KMM_STATUS_SLEEP   	= 0x00000004,	// 睡眠中
KMM_STATUS_FEINT   	= 0x00000008,	// 気絶
KMM_STATUS_TRGOFF   	= 0x00000010,	// 当たりなし
KMM_STATUS_GROUND   	= 0x00000100,	// 着地している
};

enum{ // かもめふらぐ
KMM_FLAG_DIRECT_ATC   	= 0x00000001, 	// 直攻撃
KMM_FLAG_SE_WING_WEAK  	= 0x00000010,	// よわいつばさ音
KMM_FLAG_SE_WING_STRG  	= 0x00000020,	// つよいつばさ音
KMM_FLAG_SE_WING_FLIP  	= 0x00000040,	// つよいつばさ音

KMM_FLAG_SE_PICK  	= 0x00000100,	// つつき
KMM_FLAG_SE_DOWN  	= 0x00000200,	// 倒れ
KMM_FLAG_SE_WALK  	= 0x00000400,	// 足音


KMM_FLAG_SE_WING_CHK  	= 0x000000f0,	// つばさ音チェック
};

enum{ // かもめマネージャに対するメッセージ
KMNG_MESS_VOID,			// からっぽ
KMNG_MESS_ALL_LAND_LINE,	// みんな着地:線
KMNG_MESS_ALL_LAND_AREA,	// みんな着地:エリア
KMNG_MESS_ALL_FLY,		// みんな飛び立て
KMNG_MESS_ALL_ATTACK,		// みんな攻撃

KMNG_MESS_ALL_CONCOMITANT,	// みんなで頭上付随  5
KMNG_MESS_ALL_FLIPFLOP,		// みんな集合:トラップ指定
KMNG_MESS_ALL_AGGREGATE,	// みんな集合:エリア指定
KMNG_MESS_ALL_TAKEOFF,		// 緊急逃げ
KMNG_MESS_ALL_DEAD,		// 死亡

KMNG_MESS_CHANGE_AREA_TRAP,	// トラップ内のかもめエリア変更	  10
KMNG_MESS_CHANGE_AREA_SPHERE,	// 半径内のかもめエリア変更	 
KMNG_MESS_BLOCKELUDE_LINE,	// エルード妨害
KMNG_MESS_NUM_AGGREGATE,	// 指定数集合(全検索)
KMNG_MESS_TRAP_AGGREGATE,	// 指定トラップ内にいるカモメを集合

KMNG_MESS_AREA_AGGREGATE,	// 指定エリア内にいるカモメを集合 15
KMNG_MESS_TAKEOFF_SPHERE,	// 指定球内緊急逃げ
KMNG_MESS_DEPLOY_SPHERE,	// 指定球内緊急散開
KMNG_MESS_TAKEOFF_TRAP,		// 指定トラップ内緊急逃げ
KMNG_MESS_DEPLOY_TRAP,		// 指定トラップ内緊急散開

KMNG_MESS_LAND_LINE_SPHERE,	// 指定球内から搾取して線着地	  20
KMNG_MESS_LAND_LINE_TRAP,	// 指定トラップ内から搾取して線着地
KMNG_MESS_LAND_AREA_SPHERE,	// 指定球内から搾取して空間着地
KMNG_MESS_LAND_AREA_TRAP,	// 指定トラップ内から搾取して空間着地
KMNG_MESS_AREA_DEAD,		// 指定エリア内のカモメを落下（死亡）

KMNG_MESS_FALL_SPEED,		// 落下速度設定			  25		  
KMNG_MESS_ADD_Y,		// 指定エリア内のカモメの高さを足す			  
KMNG_MESS_CHANGE_AREA_BOX,	// 箱指定型飛行エリアチェンジ
KMNG_MESS_TAKEOFF_BOX,		// 箱指定型緊急離陸
KMNG_MESS_DEPLOY_BOX,		// 箱指定型緊急回避

KMNG_MESS_CHG_ELUDEDIST,	// エルード時の反応距離変更	  30
KMNG_MESS_NUM_ATTACK,		// 指定数で攻撃			  
KMNG_MESS_COMEON,		// 呼び寄せ			  
KMNG_MESS_TAKEOFF_TURNBACK,	// 元の飛行エリアに戻る離陸			  
KMNG_MESS_COMEON_END,		// 呼び寄せやめさせる

KMNG_MESS_MAX,			// メッセージ数
};


enum{ // かもめマネージャの基本思考
KMNG_BASE_NORMAL, 	// 基本状態:特徴なし
KMNG_BASE_LIGHT,	// 基本状態:計算量少ないver
KMNG_BASE_BARDY, 	// 基本状態:ずうずうしい
KMNG_BASE_ESCAPE, 	// 基本状態:敏感
KMNG_BASE_ANGRY, 	// 基本状態:怒り
};
enum{ // かもめマネージャの特殊思考 : OR指定
KMNG_ROUTINE_ELUDE	= 0x0010, // エルード邪魔発動
KMNG_ROUTINE_FRIEND	= 0x0020, // 恩返し発動
KMNG_ROUTINE_PROVE	= 0x0040, // 挑発発動
KMNG_ROUTINE_BLOCK	= 0x0080, // 妨害発動
};

enum{ // かもめモードフラグ
KMM_MODE_NORMAL,	// 通常
KMM_MODE_BARDY,		// ずうずうしい
KMM_MODE_NOISE,		// ざわざわ
KMM_MODE_ANGRY,		// 怒り
KMM_MODE_ESCAPE,	// 逃げ
KMM_MODE_FRIEND,	// 恩返し
KMM_MODE_PROVE,		// 挑発
};
enum{ // かもめ周辺情報フラグ
KMM_INFO_NOISE_S	= 0x00000001,		// 音発生
KMM_INFO_NOISE_M	= 0x00000002,		// 音発生
KMM_INFO_NOISE_L	= 0x00000004,		// 音発生
KMM_INFO_NOISE_CHECK	= 0x0000000f,		// 音発生チェック用
KMM_INFO_CAUTION  	= 0x00000010,		// 危険
KMM_INFO_ATTACK_MOVE  	= 0x00000100,		// 攻撃移動中のかもめがいる
KMM_INFO_ATTACK	  	= 0x00000200,		// 攻撃しているかもめがいる
KMM_INFO_CBOX		= 0x00000400,		// 段ボールに止まるかもめがいる
KMM_INFO_PL_CBOX	= 0x00001000,		// 段ボールに止まれる条件がそろった
KMM_INFO_PL_EYE_CBOX	= 0x00002000,		// 目玉段ボールを装備している
};

enum{ // ヘッドマーク識別
KMM_HEADMARK_ZZZ,	// 眠り		
KMM_HEADMARK_PIYO,	// 眠り		
};

typedef	struct	_KMMACT{ // かもめ動作制御
    u_int 		time;		// 動作継続時間
    int			status;		// 状態
    short		dir;		// 方向
    short		act_end;	// 動作完了フラグ
    int			pad;		// 動作変更確認用
    
    // パッド
    int			set_mot;
    int			set_pad;

    int*		headmark;	// ヘッドマーク操作 

    // モーション
    int			current_mot;	// 使用中のモーション番号

}KMMACT;

typedef	struct	_KMMTHINK{ // かもめ動作制御
    int			nThink1;	// システム
    int			nThink2;	// パターン
    int			nThink3;	// 思考
    int			nCount3;	// カウンタ

    int			nThink3Delay;	// 遅延思考
    int			nDelay;		// 遅延カウンタ

#ifdef DEBUG_MODE
    int			nLastThink3;	// ダメージ喰らう前の最後のルーチン
    int			nPrevThink3;
#endif    

    int			nThinkTime;	// 思考継続時間
}KMMTHINK;

typedef	struct	_KAMOME{ // かもめワーク
    // システム
    GV_ACT_EX		actor;		// 動作用
    OBJECT		body;		// 体モデル
    OBJECT		bodyLodStop;	// 体モデル
    OBJECT		bodyLodFlying;	// 体モデル

    // 管理
    short		nNumber;	// 通し番号
    int			nFlag;		// かもめフラグ
    int			nMode;		// モード
    int			nMessage;	// メッセージ種類
    int			nMsgIndex;	// メッセージインデックス : -1 で 空
    int			nLod;		// LODフラグ	
    void*		pvHeadMark;	// ヘッドマーク用ワーク	

    // 動作制御用	
    KMMACT		action;

    // 思考制御用
    KMMTHINK		think;

    // モーション
    MEMMOT_CTRL		mmt_ctrl;	// メモリモーション制御
    SVECTOR		vecAdj;		// アジャスト
    u_char		ucWing;		// はばたき
    u_char		ucSeaSplush;	// 水面しぶきチェック

    // 移動用 
    u_char		bChgRouteIndex;	// ルート変更
    FVECTOR		mov;		// 位置			
    FVECTOR		step;		// 移動量		
    FVECTOR		vecDistance;	// 移動量		
    float		fMotStartHeight;// モーション最初のフレームの高さ
    FVECTOR		vecAppend;	// 追加要素		
    SVECTOR		rot;		// 回転			
    SVECTOR		turn;		// 目標方向		
    FMATRIX		matOrg;		// 座標系(処理軽減のためしかたなし)
    int			interp;		// 回転補間時間	    
    int			nIdleTime;	// アイドリング時間
    float		fSpeed;		// 速度
    FVECTOR		vecMoveArea1;	// 自由飛行エリア（最小値）
    FVECTOR		vecMoveArea2;	// 自由飛行エリア（最大値）

    // エルミート補間用
    short		nRouteIndex;	// ルートインデックス
    int			nRouteCntr;	// ルートカウンタ
    int			nRouteTime;	// ルート時間
    FVECTOR		vecStartPos;	// 始点位置	
    FVECTOR		vecAimPos;	// 終点位置	
    FVECTOR		vecStartZ;	// 始点Z軸方向
    FVECTOR		vecAimZ;	// 終点Z軸方向	
    FVECTOR		vecMove;	// 位置
    FVECTOR		vecRoute[KMM_ROUTE_MAX]; // 飛行ルート

    // 着地用
    int			nHeight;	// 高さ
    int			nRestTime;	// 羽休め時間
    int			nLandDist;	// 着地点までの距離
    int			nLandYRot;	// 着地	
    FVECTOR		vecLand;	// 着地点	
    FVECTOR		vecLandZ;	// 着地時の向き

    int			nNode;		// ノード
    FVECTOR		vecOffset;	// オフセット

    // 落下
    FVECTOR		vecFallSpd;	// 落下

    // 集合
    FVECTOR		vecAggAim;	// 集合時の注目
    u_short		nNear;		// 隣接チェック

    // ダメージ系
    u_short		nSleep;		// 睡眠時間

    // おまけ
    u_short		usDannaWait;	// フン待ちカウンタ

    // 参照関連
    int			nAimName;	// 参照対象の名前
    CONTROL*		pctrlAim;	// 参照対象のコントロール
    TARGET*		ptrgAim;	// 参照対象の親ターゲット

    // マップ関連
    int			map;		// 現在いるマップID
    HZX_GROUP_ID	hzx_id;		// HZXのグループID 

    // メッセージ
    GV_MSG*		msg;		// 自分宛メッセージ	
    u_int		name;		// キャラクタＩＤ	

    // ターゲット	
    TARGET		trg;		// 防御ターゲット
    FVECTOR		vecForce;	// 力積
    short		nLife;		// 耐久値

    // サウンド関連
    short		nAtcSeCntr;	// 攻撃音
    short		nWingSeCntr;	// つばさ音
    short		nSayCntr;	// 鳴き声

    // 関数ポインタ
    void                ( *actmode_call )( struct _KAMOME*, int );   // 動作制御関数
    int                 ( *CheckPad )( struct _KAMOME* );   	     // パッドチェック関数
    int			( *CheckDamage)( struct _KAMOME * ) ;	     // ダメージチェック関数

    // リスト
    struct _KAMOME* 	pkmmPrev;				     // カモメリスト（前）
    struct _KAMOME* 	pkmmNext;	 			     // カモメリスト（後）
    void*		pvManager;				     // かもめマネージャへのポインタ

    // システム関数群
    void*		( *ActFunc)( struct _KAMOME * ) ;	     // Act関数
    void*		( *DieFunc)( struct _KAMOME * ) ;	     // Die関数
}KAMOME ;

typedef	void	( *KMMACTMODE )( KAMOME*, int );
typedef	int	( *KMMPADCHECK )( KAMOME* );
typedef	int	( *KMMDMGCHECK )( KAMOME* );

// 動作関数更新
static inline void KMM_SetActMode( KAMOME* kamome, KMMACTMODE actmode)
{
    kamome->actmode_call = actmode;
    kamome->action.time  = 0;
}
static inline void KMM_SetModeFromPad( KAMOME* kamome, KMMACTMODE actmode, int mot, int pad )
{
    KMMACT *act ;

    act = &kamome->action ;
    act->set_mot = mot ;
    act->set_pad = pad ;
    KMM_SetActMode( kamome, actmode ) ;
}

static inline void KMM_SetCheckPad( KAMOME* kamome, KMMPADCHECK checkpad )
{
    kamome->CheckPad 	 = checkpad;
}

// 思考切替え
static inline void KMM_SetThink3( KAMOME* kamome, int think3)
{
#ifdef DEBUG_MODE
    kamome->think.nPrevThink3 = kamome->think.nThink3;
    // ダメージ喰らう前の最後のルーチン用として確保
    if ( think3 < TH3_CHECK_FEINT_START){
	kamome->think.nLastThink3 = kamome->think.nThink3;
    }
#endif
    
    kamome->think.nThink3  = think3;
    kamome->think.nCount3  = 0;
}
// 思考切替え
static inline void KMM_SetThink3Delay( KAMOME* kamome, int think3, int nDelay)
{
    kamome->think.nThink3Delay  = think3;
    kamome->think.nDelay  	= nDelay;
}
// 遅延思考切替え
static inline int KMM_SetThink3FromDelay( KAMOME* kamome )
{
    if ( kamome->think.nThink3Delay > 0){
	if ( kamome->think.nDelay == 0){
	    KMM_SetThink3( kamome, kamome->think.nThink3Delay);
	    kamome->think.nThink3Delay  	= -1; 
	    kamome->think.nDelay  		= 0;
	    return 1;
	}else if ( kamome->think.nDelay > 0){
	    kamome->think.nDelay--;
	}
    }

    return 0;
}
// 思考切替え
static inline void KMM_SetThink2( KAMOME* kamome, int think2, int think3)
{
    kamome->think.nThink2  = think2;
    KMM_SetThink3( kamome, think3);
}
// 思考切替え
static inline void KMM_SetThink1( KAMOME* kamome, int think1, int think2, int think3)
{
    kamome->think.nThink1  = think1;
    KMM_SetThink2( kamome, think2, think3);
}

// ダメージチェック関数設定
static void KMM_SetCheckDamage( KAMOME* kamome, KMMDMGCHECK checkdamage )
{
    kamome->CheckDamage = checkdamage;
}
typedef	struct _KAMOME_MNG{ // かもめマネージャ
    GV_ACT_EX		actor;		// 動作用
    int			name;		// 名前
    int			map;		// マップ番号
    int			time;		// 時間

    int			bPigeon;	// ハトモードフラグ	

    int			nLodDist;			// LOD発動距離
    int			nMdlName[KMM_MDLNAME_MAX];	// モデルネーム

    int			bSeedFlag;	// ランダムシード固定フラグ
    int			nSeed;		// ランダムシード :

    // 管理システム関連
    int			nBaseRoutine;	// 基本思考
    int			nCheck;		// チェック
    int			nMode;		// モード
    int			nPrevInfo;	// 一つ前の周辺情報
    int			nInfo;		// 周辺情報
    int			nPrevGameStatus;	// 一フレーム前のゲームステータス 

    FVECTOR		vecNoise;	// 音発生地点
    FVECTOR		vecCaution;	// 危険発生地点
    FVECTOR		vecElude;	// エルード地点
    float		fBaseSpeed;	// 基本速度
    int			nEludeDist;	// エルード時飛び立ち距離
    int			nFallSpeed;	// 落下速度
    int			nDeadHeight;	// これ以下に入ると死亡
    int			procDamage;	// カモメがダメージを受けた時に呼ぶプロシージャ

    // メッセージ関連
    GV_MSG*		msg;		// 自分宛メッセージ	
    int			nMessNum;	// メッセージ数
    u_char		ucMsgCntr[KMNG_MESS_MAX];	// メッセージ処理カウンタ   : 毎フレーム０ メッセージ取得時に自分は何番目に受け取っているかを取得できる
    u_char		ucMsgMax[KMNG_MESS_MAX];	// 同一メッセージ内の最大数 : 毎フレーム０ メッセージ取得時に最新になる

    // ターゲット関連
    TARGET		attack;		// 攻撃ターゲット
    POWER_TARGET	power;		// 攻撃ターゲット

    TARGET		trgFall;	// 落下攻撃ターゲット
    POWER_TARGET	powFall;	// 落下攻撃ターゲット

    CONTROL		control;	// 制御

    short		nKamomeNum;	// かもめ数
    short		nInitKmmNum;	// 初期かもめ数

    int			nWingSeOffset[KMM_SE_NEAR_NUM];	

    // メモリモーション
    MEMMOT_DATA		mmtData;	// メモリモーション用データ

    // おまけ
    short		nCboxCntr;	// 段ボール
    short		nCboxNum;
    short		nAngryCntr;	// 怒りカウンタ
    short		nAngryLimit;	// 限界

    int			nDannaTrap;	// フントラップ
    int			nDannaCntr;	// フンカウンタ
    int			nDannaPlCntr;	// 主観フンカウンタ

    // リスト関連
    KAMOME*		pkmmAtcSe[KMM_SE_ATTACK_NUM];	// 攻撃音をならすカモメリスト
    KAMOME*		pkmmNearSe[KMM_SE_NEAR_NUM];	// 近くにいて音をならすカモメリスト

    KAMOME*		pkmmFallAtc[KMM_FALL_ATC_NUM];	// 落下攻撃かもめ

    KAMOME* 		pkmmTop;	// かもめリスト先頭
    KAMOME* 		pkmmEnd;	// かもめリスト最後
}KAMOME_MNG;

// 宣言

void*	NewKamomeTestProg( int, int ,FVECTOR*, int, int, int, void*, int, FVECTOR*, FVECTOR*);
extern int KMMNG_CheckKamomeIn( FVECTOR* pvecCenter, int nRadius);		// かもめ範囲内チェック
extern int KMMNG_CheckLandKamomeInBox( FVECTOR*, float,  float,  float);	// 着地かもめ範囲内チェック
// 便利マクロ

#define	KMM_ActStatus( a,b )		(a)->status |= (b)
#define	KMM_ActStatusOff( a,b )		(a)->status &= ~(b)
#define	KMM_SetFlag( a,b )		(a)->nFlag |= (b)
#define	BODYWORLD(a,b) 			(a)->objs->objs[(b)].world

static inline int KMM_DemoRand( KAMOME_MNG* kmmng, int a ){  
    if ( kmmng->bSeedFlag ) return (GM_IRnd( &kmmng->nSeed ) % a); 
    else		    return (BP_PS2_rand() % a);
} 
static inline int KMM_GetDemoRandom( KAMOME_MNG* kmmng, int min, int max ){ 
    if ( kmmng->bSeedFlag ) return (GM_IRnd( &kmmng->nSeed ) % ((max - min) + 1) + min); 
    else		    return (BP_PS2_rand() % ((max - min) + 1) + min); 
}
static inline float KMM_GetFRnd( KAMOME_MNG* kmmng){ 
    if ( kmmng->bSeedFlag ) return (GM_FRnd( &kmmng->nSeed )); 
    else		    return (rnd());
}

extern void KMM_InsertKamome( KAMOME_MNG*, KAMOME*);	// かもめをリストに加える
extern void KMM_RemoveKamome( KAMOME_MNG*, KAMOME*);	// かもめをリストから外す

enum{ // LOD
KMM_LOD_NONE,
KMM_LOD_REST,
KMM_LOD_FLYING,
};

enum{ // DMG-Proc Type
KMM_DMGPROC_DEAD,	// 死亡
KMM_DMGPROC_SLEEP,	// 睡眠
KMM_DMGPROC_FEINT,	// 気絶
};

#endif // end of  __KAMOME_H__



