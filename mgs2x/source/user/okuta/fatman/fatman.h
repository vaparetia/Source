/*
    fatman.c
    ファットマン関連ヘッダ
    2001/03/22 Masafumi Okuta
    $Id: fatman.h,v 1.1.1.3 2002/11/19 11:47:59 Yoshizawa1 Exp $
*/
#ifndef __FATMAN_H__
#define __FATMAN_H__

#define SPEED_CHG_1002ADD	// ８の字対策速度変更ルーチン

// データ用マクロ
#define MODEL_NAME	(11755189)	// GV_StrCode("fat_def_sh_mt") 
#define MW_MODEL_NAME	(5463733)	// GV_StrCode("fat_def_mh_mt") 
#define BASE_MOTION	(4964704)	// GV_StrCode("fatman") 
#define DAMAGE_MOTION	(4964704)	// GV_StrCode("fatman") 
#define CAPTURE_MOTION	(4964704)	// GV_StrCode("fatman") 
#define DRAG_MOTION	(4964704)	// GV_StrCode("fatman") 
#define GLK_MODEL	(109035)	// GV_StrCode("glk") 
#define FACEANIME_NAME	(7866389)	// GV_StrCode("ファット顔アニメ") 
#define FAT_FAR_NAME	(2495489)	// GV_StrCode("fat_boss") 

// 便利マクロ
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

// yano
#ifdef BP_PSX2_ASM
#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})
#else
#define FABS(_x) DG_FABS(_x)
#endif

extern float BP_AdjustTick3(float);
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))



// システム
#define SMP_FAINT	    (10)			// 気絶値
#define SMP_FAINT_COUNT	    COUNT_VMODE((60*30))	// 気絶時間
#define SMP_SLEEP_COUNT	    COUNT_VMODE((60*60*1 + 3))	// 睡眠時間
#define SMP_DOWN_DAM	    (10)			// ダウンダメージ量
#define VIEW_ANGLE	    (768)			// 視野角度
#define INVINCIBLE_TIME	    (COUNT_VMODE(180))		// 無敵時間	
#define INVINCIBLE_HEAD	    (COUNT_VMODE(240))		// 頭部ダメージ無敵時間	
#define INVINCIBLE_MID      (COUNT_VMODE(15))		// 無敵時間	
#define INVINCIBLE_LIGHT    (COUNT_VMODE(10))		// 無敵時間	
#define INVINCIBLE_FALL     (COUNT_VMODE(90))		// 無敵時間	
#define FAT_INTERP_DEF	    (COUNT_VMODE(60))		// 補間時間
#define FAT_NAVI_DIST	    (1500)			// ナビ距離
#define FAT_R_SPHERE_DEF    (450)			// 壁との反発半径（デフォルト）	
#define FAT_R_SPHERE_MOV    (400)			// 壁との反発半径（移動時）	

#define FAT_BREAK_SPEED     (0.8f)  			// ローラーブレード破損時の速度

#define FAT_ROUTEDATA_NUM   (182)			// ルートデータ数
#define FAT_BEZIER_INTERP   (COUNT_VMODE(256))		// ルートデータ補間時間

#define FAT_SPEED	    STEP_VMODEF(130.f)		// 速度

#define FAT_ELUDE_FALL_NUM  (13)			// エルード落下
#define FAT_INTRUDE_ADDR_NUM (8)			// イントルードエリア数

// 爆弾関連
#define FAT_C4_LEVEL	    (4)				// C4設置難易度最大値
#define FAT_C4_PATTERN      (16)			// C4設置パターン最大値
#define FAT_C4_POSNUM	    (8)				// １パターンあたりのC4設置場所最大値	
#define MAX_ZONE_DATA 	    (FAT_C4_LEVEL * FAT_C4_PATTERN * FAT_C4_POSNUM)	// GCL指定設置データ最大値
#define MAX_BOMB_NUM	    (32)			// 爆弾設置可能数:表示最大数にあたる
#define FAT_C4_POSDATA	    (256)			// ポイントデータ
#define BOMB_TIME	    COUNT_VMODE((2700))		// 爆発までの時間
#define BOMB_RADAR_FEED_OUT COUNT_VMODE((600))		// レーダー上のもや表示フェードアウト時間
#define PUT_DIST	    (5000/250) 			// 設置ポイント
#define FAT_TURN_PUT_MAX    (32)			// 回転設置用参考ゾーン数

#define FAT_DEMINE_PROV_TIME    COUNT_VMODE(600)	// 爆弾解体挑発までの時間
#define FAT_DEMINE_TALK_BLANK   COUNT_VMODE(600)	// 爆弾解体挑発までの時間

// フェーズ切替え用
#define FAT_SHOOT_PHASE_TIME COUNT_VMODE((3600))	// 銃撃フェーズの長さ
#define FAT_BOMB_PHASE_RATE  (50)			// 爆弾フェーズから起動フェーズに強制で移行するダメージ量
#define FAT_SHOOT_PHASE_RATE (50)			// 銃撃フェーズの終了判定用耐久値減り量

// 銃撃戦関連
#define TERROR_SHOOT_TIME   COUNT_VMODE((30))		// 威嚇射撃開始時間
#define GLK_BUL_NUM	    (20)			// グロック弾数
#define GLK_INTERVAL	    (3)				// グロック発射間隔
#define GLK_DIST	    (6000)			// グロック発射距離
#define GLK_RANGE	    (20000)			// グロック射程
#define NEAR_DIST	    (2000)			// 接近回避
#define POINT_MOVE_DIST	    (1200)			// 指定点移動終了距離
#define ESC_NEAR_DIST	    (4000)			// 接近回避判定発動距離
#define CAUTION_NOISE_DIST  (8000)			// 警戒移行用音判定距離

#define FAT_INTRUDE_TIME    COUNT_VMODE((300))	// イントルードタイマー最大値
#define FAT_INTRUDE_DEC	    COUNT_VMODE((3))	// イントルードタイマー減衰値
#define FAT_BEHIND_TIME     COUNT_VMODE((900))	// ビハインドタイマー最大値
#define FAT_BEHIND_DEC	    COUNT_VMODE((3))	// ビハインドタイマー減衰値
#define FAT_ELUDE_DIV_DIST  (4000)		// エルードFAR, NEAR分岐距離

// 装填
#define GLK_CART_OUT        (0.03478f)		// カートリッジ破棄
#define GLK_RUN_CART_OUT    (0.03478f)		// カートリッジ破棄
#define GLK_RUN_CART_APPEAR (0.17391f)		// 新カートリッジ登場
#define GLK_CART_APPEAR     (0.17391f)		// 新カートリッジ登場
#define GLK_RELOAD_TIME	    (0.33043f)		// 装填時間       
#define GLK_RUN_RELOAD_TIME (0.64347f)		// 装填時間       
#define GLK_START_RELOAD    (0.42666f)		// 銃撃フェーズ開始の装填
#define FAT_BEHIND_DATA_NUM (128)		// 対ビハインド用データワーク数
#define FAT_STRM_MAX	    (64)		// ストリーム数

#define FAT_BACKSHOT_DIR    (4)			// 逆走撃ち用データワーク方向数
#define FAT_BACKSHOT_NUM    (5)			// 逆走撃ち用データワーク１方向当たりの数

#define FAT_QUAKE_TIME	    ( COUNT_VMODE(60) ) // 倒れ地震時間

// npc->act->status使用
#define FAT_ACT_STATUS_DMG_STAND 	0x00010000	// ダメージチェック:立ち
#define FAT_ACT_STATUS_DMG_RUN 		0x00020000	// ダメージチェック:走行時
#define FAT_ACT_STATUS_DMG_DOWN 	0x00040000	// ダメージチェック:ダウン
#define FAT_ACT_STATUS_DMG_HEAD 	0x00080000	// ダメージチェック:頭のみ
#define FAT_ACT_STATUS_DMG_DEAD 	0x00100000	// ダメージチェック:頭のみ
#define FAT_ACT_STATUS_DOWN_END 	0x00200000	// ダウンチェック  :復帰
#define FAT_ACT_STATUS_EYE_CLPOSE 	0x00400000	// 目閉じ

enum{	// ファットマン情報
FAT_INFO_CLEAR	 	= 0x00000000,	// クリア用
FAT_INFO_SEE_PL	 	= 0x00000001,	// プレイヤーが見えた
FAT_INFO_SEE_AIM	= 0x00000002,	// 目標位置捕捉
FAT_INFO_SEE_CAUTION	= 0x00000004,	// 警戒位置捕捉
FAT_INFO_LOCKED  	= 0x00000008,	// ロックオンされた
FAT_INFO_NOISE   	= 0x00000010,	// 物音
FAT_INFO_FINE 	 	= 0x00000020,	// 元気
FAT_INFO_WOUND 	 	= 0x00000040,	// 軽傷
FAT_INFO_BAD 	 	= 0x00000080,	// 重傷
FAT_INFO_DYING 	 	= 0x00000100,	// 死にかけ
FAT_INFO_NARC    	= 0x00000200,	// 麻酔
FAT_INFO_STUN	 	= 0x00000400,	// 痺れ
FAT_INFO_SHOOT	 	= 0x00000800,	// 発射
FAT_INFO_ATTACK	 	= 0x00001000,	// 攻撃した
FAT_INFO_DOWN	 	= 0x00002000,	// 倒れてる
FAT_INFO_BODY_CAUTION 	= 0x00004000,	// ブラストスーツ部分狙撃直後
FAT_INFO_HEAD_CAUTION 	= 0x00008000,	// 頭部狙撃直後
FAT_INFO_ROLL_CAUTION 	= 0x00010000,	// ローラーブレード部分狙撃直後
FAT_INFO_ELUDE_NEAR 	= 0x00100000,	// エルード近距離
FAT_INFO_ELUDE_FAR 	= 0x00200000,	// エルード遠距離
FAT_INFO_PL_DEAD	= 0x00400000,	// プレイヤー死亡
};

// 警戒時間
#define BODY_CAUT_TIME 		COUNT_VMODE((180)) 
#define HEAD_CAUT_TIME 		COUNT_VMODE((180)) 
#define ROLL_CAUT_TIME 		COUNT_VMODE((180))
#define NARC_CAUT_TIME 		COUNT_VMODE((300)) 
#define STUN_CAUT_TIME 		COUNT_VMODE((300))
#define ATTACK_CAUT_TIME 	COUNT_VMODE((120))

// 思考ルーチン
enum {	// work->think1 : 大判別
    TH1_NORMAL, // 通常時
    TH1_DAMAGE,	// ダメージ時
    TH1_EVENT,	// イベント時
};
enum {	// work->think2	: 行動パターン
    // 通常時
    TH2_BOMB_PHASE,	// 爆弾フェーズ
    TH2_DISPOSAL_PHASE,	// 解体フェーズ
    TH2_SHOOT_PHASE, 	// 銃撃フェーズ
    // ダメージ時
    TH2_START_DAMAGE, 	// ダメージ開始
    TH2_SYSTEM_DAMAGE, 	// システムダメージ
    // イベント時
    TH2_EVENT,	// イベント時
};
enum {	// work->think3 : 詳細フェーズ
    // システム
    TH3_PHASE_START,		// フェーズ開始
    TH3_READY,			// 警戒		
    // 移動
    TH3_STAND, 			// 直立
    TH3_MOVE,			// 移動
    TH3_TURN_BACK,		// 旋回(後)
    TH3_TURN_FRONT,		// 旋回(前)		5
    TH3_REVERSE,		// 反転
    TH3_STOP, 			// 停止
    // 共通
    TH3_POINT_MOVE,		// 指定点移動	
    TH3_RELOAD,			// 装填		
    TH3_RUN_RELOAD,		// 走り装填		10
    TH3_RUN_SHOOT,		// 走り撃ち 	
    TH3_BACK_SHOOT,		// 逆走撃ち 	
    TH3_FEINT_SHOOT,		// 牽制撃ち	
    TH3_ELUDE_FALL,		// エルード落とし移動	
    TH3_ELUDE_KILL,		// エルード指きり	15	
    TH3_ELUDE_SHOOT,		// エルード銃撃  
    TH3_OVERLOOK,		// 見渡す	
    TH3_BLEATHLESS, 		// 息切れ		
    TH3_STANDBY,		// 待機			
    // 攻撃フェーズ用
    TH3_BODYATTACK,		// 体当り		20	
    // 爆弾フェーズ用
    TH3_RUN_PUT, 		// 走りながら爆弾設置   
    TH3_TURN_PUT, 		// 回転しながら爆弾設置 
    TH3_STOP_PUT, 		// 停止して爆弾設置     
    // 解体フェーズ用
    TH3_COUNT_FIRST,		// フェーズ開始		
    TH3_COUNT_ATTRACT,		// 解体中の余興分岐	25
    TH3_COUNT_DANCE,		// 解体中の余興踊り	
    TH3_COUNT_KAMOME,		// 解体中の余興カモメ 	
    TH3_COUNT_KAMOME2,		// 解体中の余興カモメ２ 	
    TH3_COUNT_BACKNAVI,		// 解体中の余興背後ナビ 	
    TH3_COUNT_BOMBGUARD,	// 解体中のまちぶせ 	30
    TH3_COUNT_STAND,		// 解体中の余興待機		
    TH3_COUNT_MOVE,		// 解体中の余興移動	
    // ダメージフェーズ用
    TH3_WAIT, 			// 硬直			
    TH3_WAKEUP,			// 起き上がり		
};

enum { // アクション 
    PAD_NONE,			// 空っぽ:システム上存在
    // 移動系
    PAD_RUN_ACT,		// ローラーブレード走行
    PAD_TURN_LEFT,		// 左ターン
    PAD_TURN_RIGHT,		// 右ターン
    PAD_TURN_BACK,		// 後ろへターン
    PAD_TURN_FRONT,		// 前へターン	5

    PAD_REVERSE,		// 反転
    PAD_STOP,			// 停止
    PAD_POINT_MOVE,		// 指定点移動
    PAD_SHOT_READY,		// 銃構え
    PAD_SHOT_ACT,		// グロック発射	10

    PAD_RUNSHOT_READY,		// 走り撃ち構え
    PAD_RUNSHOT_ACT,		// グロック発射:走り撃ち
    PAD_BACKSHOT_READY,		// 逆走撃ち構え
    PAD_BACKSHOT_ACT,		// グロック発射:逆走撃ち
    PAD_RELOAD,			// 立ちグロック装填	15
    PAD_RUN_RELOAD,		// 走りグロック装填

    PAD_WAKEUP,			// 起き上がり

    PAD_SET_BOMB_STOP,		// 停止設置
    PAD_SET_BOMB_RUN,		// 走り設置
    PAD_SET_BOMB_TURN_L,	// 回転設置:左
    PAD_SET_BOMB_TURN_R,	// 回転設置:右

    PAD_COUNTDOWN_START,	// 爆弾起動
    PAD_COUNTDOWN_STARTDMG, 	// 爆弾起動:ダメージによる
    PAD_COUNTDOWN_START_FAILED,	// 爆弾起動失敗
	
    PAD_FEINT_SHOOT,		// 牽制撃ち
    PAD_LIMIT_SHOOT,		// 制限撃ち
    PAD_TOBIDASI_L,		// 飛び出し左
    PAD_TOBIDASI_R,		// 飛び出し右
    PAD_SLIDE_SHOOT_L,		// スライド撃ち
    PAD_SLIDE_SHOOT_R,		// スライド撃ち
    PAD_BODYATTACK,		// 体当り

    PAD_OVERLOOK,		// 見渡す
    PAD_BLEATHLESS,		// 息切れ
    PAD_ODORI,			// 踊り
    PAD_SQUAT,			// 座る
    PAD_HOLDUP,			// ホールドアップ:仮置き

    PAD_REPAIR,			// ローラーブレード修理
    PAD_CIRCLE_DANCE,		// 回転ダンス
    PAD_CIRCLE_DANCE2,		// 回転ダンス２
    PAD_KAMOME,			// カモメ戯れ
    PAD_BACKNAVI,		// 背後ナビ

    PAD_ELUDE_KILL,		// エルード指きり
    PAD_SHOOTPHASE_START,	// 銃撃フェーズスタート
    PAD_BOMBPHASE_START,	// 爆弾フェーズスタート
} ;
enum {	// 子ターゲット
    // LV0
    // LV1
    FAT_TRG_CHILD_BLASTNECK_F,	// 襟:前
    FAT_TRG_CHILD_BLASTNECK_B,	// 襟:後
    FAT_TRG_CHILD_BLASTNECK_L,	// 襟:左
    FAT_TRG_CHILD_BLASTNECK_R,	// 襟:右
    FAT_TRG_CHILD_HEAD,		// 頭
    FAT_TRG_CHILD_HEART,	// 心臓
    FAT_TRG_CHILD_ROLL_R,	// ローラーブレード右
    FAT_TRG_CHILD_ROLL_L,	// ローラーブレード左
    // LV2
    FAT_TRG_CHILD_ARMR1,	// 右上腕
    FAT_TRG_CHILD_ARMR2,	// 右下腕
    FAT_TRG_CHILD_ARML1,	// 左上腕
    FAT_TRG_CHILD_ARML2,	// 左下腕
    FAT_TRG_CHILD_LEGR1,	// 右ふともも
    FAT_TRG_CHILD_LEGR2,	// 右すね
    FAT_TRG_CHILD_LEGL1,	// 左ふともも
    FAT_TRG_CHILD_LEGL2,	// 左すね
    // LV3
    FAT_TRG_CHILD_BPDY,		// 体

    CHILD_TARGET_NUM, // 子ターゲット数
} ;

enum{ // エリア
FATC4_AREA_HERI_LEFT,	// ヘリポート左エリア
FATC4_AREA_HERI_CENTER,	// ヘリポート中央エリア
FATC4_AREA_HERI_RIGHT,  // ヘリポート右エリア
FATC4_AREA_CONT_LEFT,	// コンテナエリア左半分
FATC4_AREA_CONT_RIGHT,	// コンテナエリア右半分
FATC4_AREA_BANQ_LEFT,	// 棚エリア左半分
FATC4_AREA_BANQ_RIGHT,	// 棚エリア右半分
FATC4_AREA_MAX,		// エリア数
};

// C4種類フラグ
#define FAT_TYPE_UNDER 0x0001	// 下置き
#define FAT_TYPE_STAND 0x0002	// 立ち置き
#define FAT_TYPE_UPPER 0x0004	// 伸び置き
#define FAT_TYPE_RUN   0x0008	// 走り置き
#define FAT_TYPE_CHECK 0x0007 	// 走り以外設置タイプ取得用

#define FAT_SE_BLANK (COUNT_VMODE(45)) // SEブランクタイム

enum{ // ファットマンストリーム	
FAT_STRM_PUT_1,		// まずは一つ
FAT_STRM_PUT_2,		// 次は二つ
FAT_STRM_PUT_3,		// それでは三つ
FAT_STRM_SHOOT_START, 	// 爆弾おあずけ
FAT_STRM_SHOOT_START_2,	// 爆弾おやすみ

FAT_STRM_SHOOT_START_3,	// 爆弾ひとやすみ
FAT_STRM_BOMB_START, 	// 爆弾開始 さあ爆弾のじかんだ
FAT_STRM_BOMB_START_2, 	// 爆弾開始 おまちかねの爆弾
FAT_STRM_BOMB_START_3, 	// 爆弾開始 そろそろ爆弾
FAT_STRM_GAMEOVER, 	// げーむおーばー ふぁっとへっど

FAT_STRM_BOMB_PROV, 	// 挑発1  	ばか	
FAT_STRM_BOMB_PROV_02, 	// 挑発2	のろま	
FAT_STRM_BOMB_PROV_03, 	// 挑発3	どこさがして
FAT_STRM_BOMB_PROV_04, 	// 挑発4	そんなところで
FAT_STRM_BOMB_PROV_05, 	// 挑発5	なにをやって

FAT_STRM_BOMB_PROV_06, 	// 挑発6	そこかな
FAT_STRM_BOMB_PROV_07, 	// 挑発7	そこだ
FAT_STRM_BOMB_PROV_08, 	// 挑発8	はやく
FAT_STRM_BOMB_PROV_09, 	// 挑発9	そこには
FAT_STRM_BOMB_PROV_10, 	// 挑発10	うろうろ

FAT_STRM_BOMB_PROV_11, 	// 挑発11	どうしたFOXHOUNDのながなくぞ
FAT_STRM_BOMB_SET1, 	// セット１	ここにせっと		
FAT_STRM_BOMB_SET2, 	// セット２	つぎはここだ
FAT_STRM_PROV_01, 	// 嘲笑		ぬはは
FAT_STRM_PROV_02, 	// 嘲笑		わはは

FAT_STRM_PROV_03, 	// 嘲笑		にはは
FAT_STRM_BOMB_20, 	// ２０		
FAT_STRM_BOMB_20_02, 	// ２０		長い
FAT_STRM_BOMB_15, 	// １５
FAT_STRM_BOMB_15_02, 	// １５		長い

FAT_STRM_BOMB_10, 	// １０
FAT_STRM_BOMB_10_02, 	// １０		長い	
FAT_STRM_BOMB_05, 	// ５
FAT_STRM_BOMB_05_02, 	// ５		長い	
FAT_STRM_BOMB_LESS, 	// そろそろ	長い

FAT_STRM_BOMB_LESS_02, 	// そろそろ
FAT_STRM_WHERE_01,	// どこだ	どこにいった
FAT_STRM_WHERE_02,	// どこだ	どこにかくれた
FAT_STRM_IMMOTAL_01,	// 効かない	なにかしたか
FAT_STRM_IMMOTAL_02,	// 効かない	すーつ

FAT_STRM_IMMOTAL_03,	// 効かない	どこをねらって
FAT_STRM_IMMOTAL_04,	// 効かない	なんのつもり		
FAT_STRM_OTTOTO_01,	// あぶない	ぬおー
FAT_STRM_OTTOTO_02,	// あぶない	あぶないあぶない		
FAT_STRM_ELUDE_01,	// エルードキル	しねー

FAT_STRM_ELUDE_02,	// エルードキル くらえー
};

enum{	// 移動形式
FAT_MOVEFORM_ZONE,	// ゾーン移動
FAT_MOVEFORM_ROUTE,	// ルート移動
FAT_MOVEFORM_TO_ROUTE,	// ゾーン移動->ルート移動
FAT_MOVEFORM_TO_ZONE,	// ルート移動->ゾーン移動
};

// 攻撃力
#define BODYATTACK_POW		(20)	// 体当り攻撃
#define BODYCLASH_POW		(5)	// 接触
#define CAPTUREEVADE_POW	(10)	// 掴み回避カウンター
#define ROLLERBLADE_POW		(5)	// ローラーブレード攻撃
#define BODYEVADE_POW		(3)	// 体弾き攻撃

// ダメージ関連

#define FAT_PUTINTV_CAUT_TIME		(COUNT_VMODE(360))	// ファットマンが爆弾設置に手間取っていると感じる時間
#define FAT_PUTINTV_CHGMODE_TIME	(COUNT_VMODE(900))	// ファットマンが爆弾設置方法を走りに変える時間

#define FAT_LOST_TRG_LIMIT		(COUNT_VMODE(1800))	// ファットマンがプレイヤーを見失っている時間リミット
#define FAT_LOST_TRG_THINK		(COUNT_VMODE(600))	// ファットマンがプレイヤーを見失っていらいらする時間

#define FAT_VITAL_MAX			(COUNT_VMODE(3600))	// ファットマン持久力
#define FAT_VITAL_JP_VEASY_MAX		(COUNT_VMODE(2400))	// ファットマン持久力:日本版V-EASY
#define FAT_VITAL_JP_EASY_MAX		(COUNT_VMODE(3000))	// ファットマン持久力:日本版EASY
#define FAT_LOST_VITAL			(COUNT_VMODE(4))	// ファットマン持久力減少値
#define FAT_RECOVER_VITAL		(COUNT_VMODE(2))	// ファットマン持久力回復値

#define	FAT_UnSetActStatus( a,b )	(a)->status &= ~b	// ファットマンステータス用

enum{ // C4設置ポイント検索
FAT_C4_SEARCH_AREA,	// 対となるエリア優先
FAT_C4_SEARCH_DIST,	// 距離判定優先
FAT_C4_SEARCH_DIR,	// 方向差最小優先
FAT_C4_SEARCH_TYPE,	// 特定種類優先
FAT_C4_SEARCH_OTHER,	// 現在選択しているもの以外を優先
};

enum{ // 走り状態
FAT_RUNMODE_NORMAL,	// 通常
FAT_RUNMODE_TURN_L,	// 左旋回
FAT_RUNMODE_TURN_R,	// 右旋回
};

enum { // ターゲットサイズ
FAT_TARGET_SIZE_STAND,	// 立ち状態
FAT_TARGET_SIZE_DOWN,	// ダウン状態 
FAT_TARGET_SIZE_SQUAT,	// しゃがみ
};

enum{ // カートリッジコントロール用ステップ値
CART_STEP_RELEASE, 	// 外す
CART_STEP_HOLD, 	// 手に持つ
CART_STEP_RELOAD, 	// 装填
CART_STEP_END,	 	// 終了
};

enum{ // GCLに渡すファットマン情報 :: procFatInfoコール時に渡される値
FAT_GCLINFO_PHASE_BOMB,		// 爆弾フェーズ開始
FAT_GCLINFO_PHASE_DEMINE,	// 解体フェーズ開始
FAT_GCLINFO_PHASE_SHOOT,	// 銃撃フェーズ開始
FAT_GCLINFO_STARTUP_BOMB,	// Ｃ４爆弾起動
FAT_GCLINFO_CLEAR_BOMB,		// Ｃ４爆弾解体完了
FAT_GCLINFO_GAME_CLEAR,		// ファットマン戦終了
FAT_GCLINFO_FALL,		// ファットマン転倒
};


enum{ // フェーズ進行レベル
FAT_PHASELEVEL_1,
FAT_PHASELEVEL_2,
FAT_PHASELEVEL_3,
FAT_PHASELEVEL_4,
FAT_PHASELEVEL_5,
};

//--------------------------- デバッグ関連
#ifdef DEBUG_MODE
#define MAO_DEBUG
#endif

#ifdef MAO_DEBUG
#define FAT_SetNextPointWithNavigate( work, nZone, vecPos){ printf("with navi = %s : %d :: %d\n", __FILE__, __LINE__, nZone); FAT_SetNPointWithNavigate( work, nZone, vecPos); }
#define FAT_SetThink3( work, nNewThink3){ printf("think3 = %s : %d :: %d -> %d\n", __FILE__, __LINE__, work->think3, nNewThink3); FAT_SetTh3( work, nNewThink3); }
#define FAT_SetNextPoint( work, nNextZone, pvecNext){ FAT_SetNPoint( work, nNextZone, pvecNext); printf(" NextZone = %s : %d :: %d\n%f %f %f\n", __FILE__, __LINE__, nNextZone, pvecNext->vx, pvecNext->vy, pvecNext->vz); }
#define FAT_SetNextWithRelayPoint( work, nRelayZone, nAimZone, pvecRelay, pvecAim) { FAT_SetNWithRelayPoint( work, nRelayZone, nAimZone, pvecRelay, pvecAim); printf("RelayZone = %s : %d :: %d\n%f %f %f\n", __FILE__, __LINE__, nRelayZone, pvecRelay->vx, pvecRelay->vy, pvecRelay->vz); printf("AimZone = %s : %d :: %d\n%f %f %f\n", __FILE__, __LINE__, nAimZone, pvecAim->vx, pvecAim->vy, pvecAim->vz); }
#define FAT_SetNextZone( work, nNextZone ) {  FAT_SetNZone( work, nNextZone ); printf(" NextZone = %s : %d :: %d\n", __FILE__, __LINE__, nNextZone);  }
#define FAT_SetNextZone2( work, nNextZone ) { printf(" NextZone2 = %s : %d \n", __FILE__, __LINE__); FAT_SetNZone2( work, nNextZone ); }
#define FAT_SetNextZoneFrom2( work, nNextZone ) { printf(" NextZoneFrom2 = %s : %d \n", __FILE__, __LINE__); FAT_SetNZoneFrom2( work, nNextZone ); }
#define FAT_SetNewAim( work, nNextZone, pvec ) { printf(" NextNewAim = %s : %d \n", __FILE__, __LINE__); FAT_SetNAim( work, nNextZone, pvec ); }
#else
#define FAT_SetNextPointWithNavigate( work, nZone, vecPos){ FAT_SetNPointWithNavigate( work, nZone, vecPos); }
#define FAT_SetThink3( work, nNewThink3){ FAT_SetTh3( work, nNewThink3); }
#define FAT_SetNextPoint( work, nNextZone, pvecNext){ FAT_SetNPoint( work, nNextZone, pvecNext);  }
#define FAT_SetNextWithRelayPoint( work, nRelayZone, nAimZone, pvecRelay, pvecAim) { FAT_SetNWithRelayPoint( work, nRelayZone, nAimZone, pvecRelay, pvecAim); }
#define FAT_SetNextZone( work, nNextZone ) 	{ FAT_SetNZone( work, nNextZone ); }
#define FAT_SetNextZone2( work, nNextZone ) 	{ FAT_SetNZone2( work, nNextZone ); }
#define FAT_SetNextZoneFrom2( work, nNextZone ) { FAT_SetNZoneFrom2( work, nNextZone ); }
#define FAT_SetNewAim( work, nNextZone, pvec )  { FAT_SetNAim( work, nNextZone, pvec ); }
#endif

// 爆弾設置場所構造体
typedef struct {
    int		nTrapName;	// トラップ用名前
    int		nType;		// 設置タイプ:床,壁など
    int		nArea;		// 設置エリア
    int		nStatus;	// 爆弾設置状態 : 毎フレーム更新される（爆弾が存在する時は1）
    int 	zoneAddr;	// ゾーンアドレス
    FVECTOR	vecPos;		// 本体位置
    FVECTOR	vecPut;		// 設置時にファットマンが立つ位置
    SVECTOR	vecRot;		// 本体回転情報
}FAT_PUTPOINT_DATA;

// 爆弾情報構造体
typedef struct {
    int			nID;		// ID : C4起動関数の返り値 存在しない時は -1
    int			nStatus;	// 状態
    int			nMapID;		// 設置マップID 存在しない時は -1
    int			nCount;		// 爆発までの時間
    int			nCamDist;	// カメラからの距離
    int			nSeNum;		// 音番号
    int			nLastSeTime;	// 最後にSEが鳴った時間

    FMATRIX		matPos;		// 設置位置
    FAT_PUTPOINT_DATA*	pputData;	// 設置場所データ

    void*		pvRadar;	// レーダーワーク
}FAT_C4_DATA;

// ファットマン構造体
typedef	struct	_fatman{
    // 基本システム系
    GV_ACT_EX		actor;				// 親子関係登録用	system/libgv/libgv.h
    OBJECT		body;				// モデル 		game/g_struct.h
    OBJECT		gun;				// 銃モデル 		game/g_struct.h
    WEAPON_EF_CTRL	wctrl;				// 武器制御		user/skoba/weapon_old/
    CONTROL		control;			// 移動制御		game/g_struct.h
    FMATRIX		lights[2];			// 光源
    int			name;				// 名前ID
    int			map;				// 所属マップID

    // NPCシステム＆ナビゲート系
    NPCWORK		npc;				// NPCキャラ用ワーク	user/korekado/npc
    NAVIGATE 		navigate;			// ゾーンナビゲート	game/g_struct.h
    NAVITARGET		navitrg;			// ナビゲート用の目標	game/g_struct.h
    RADAR_CTRL		rctrl;				// レーダー表示		game/g_struct.h
    
    // ターゲット関係
    TARGET		deftrg;				// 防御ターゲット
    TARGET		pushtrg;			// ぶつかりターゲット
    TARGET		def_child[CHILD_TARGET_NUM];	// 子ターゲット
    POWER_TARGET	powDefTrg;			// ダメージ計算用
    POWER_TARGET	powDefChild[CHILD_TARGET_NUM];	// ダメージ計算用子ターゲット
    TARGET		attack;				// 攻撃用ターゲット	   :体当り用
    POWER_TARGET	pow;				// ダメージ攻撃用ターゲット:体当り用
    TARGET		atcEvade;			// 攻撃用ターゲット	   :体弾き用
    POWER_TARGET	powEvade;			// ダメージ攻撃用ターゲット:体弾き用
    TARGET		atcElude;			// 攻撃用ターゲット	   :エルード用
    POWER_TARGET	powElude;			// ダメージ攻撃用ターゲット:エルード用
    TARGET		atcBlow;			// 攻撃用ターゲット	   :掴みはじき用
    POWER_TARGET	powBlow;			// ダメージ攻撃用ターゲット:掴みはじき用
    TARGET		atcRollerL;			// 攻撃用ターゲット	   :ローラーブレード左
    POWER_TARGET	powRollerL;			// ダメージ攻撃用ターゲット:ローラーブレード左
    TARGET		atcRollerR;			// 攻撃用ターゲット	   :ローラーブレード右
    POWER_TARGET	powRollerR;			// ダメージ攻撃用ターゲット:ローラーブレード右
    CAPTURE_TARGET	capture;			// 捕まりターゲット
    HOMING_TRG		homing;				// ホーミングターゲット

    // ゲージ関連
    GM_GageSet		gage;				// ゲージ管理		game/g_struct.h
    int			m9_faint;			// M9耐久値
    
    // その他（システム系）
    int			nRollerBreak;			// ローラーブレード破損フラグ
    int			bKillFlag;			// Kill or Faintフラグ
    int			bGameClear;			// ゲームクリアフラグ
    int			bGameOver;			// タイムアウトゲームオーバーフラグ
    int			bCallClearProc;			// クリアプロシージャコール許可
    int			nFaceChara;			// 顔キャラ名
    FVECTOR		vecPrePos;			// 前のフレームでの位置
    FVECTOR		vecStep;			// 移動ベクトル
    FVECTOR		vecPreDmgStep;			// ダメージに入る前の移動ベクトル
    int			nMoveDist;			// 移動量
    SVECTOR		vecKoshiRot;			// 腰の向き
    int			nEvadePush;			// 重なり防止フラグ
    int			nEvadeConse;			// 重なり防止連続発動時間
    FVECTOR		vecPush;			// 重なり防止押し戻し方向

    float		fSpeed;				// 速度

    // プロシージャ
    int			procShootC4Over;		// C4狙撃ゲームオーバーフラグ
    int			procGameOver;			// ゲームオーバー時に呼ばれるプロシージャ
    int			procKillClear;			// 通常クリア時に呼ばれるプロシージャ
    int			procSleepClear;			// 気絶クリア時に呼ばれるプロシージャ
    int			procFatInfo;			// 特定条件で呼ばれるファットマン情報プロシージャ

    // パラメータ
    int			nBodyDurableMax;		// 身体最大耐久値
    int			nBodyDurableRecover;		// 身体耐久値回復時間
    int			nVitality;			// 体力
    float		fPowerRate;			// 攻撃力割合		

    // 爆弾難易度発動レート
    float		fBombDifficlutRate[FAT_C4_LEVEL]; // 難易度レート
    
    // ファットマンステータス
    int			nAimIndex;			// グロックが狙う関節
    float		fFatPlayRate;			// モーション再生レート
    float		fFatMotSpeed;			// モーション再生速度
    int			nBullet;			// 残弾数
    int			bAdjustFlag;			// アジャスト計算フラグ
    SVECTOR		svecAdjust;			// 照準合わせよう回転角度
    int			bRollerStop;			// ローラーブレード停止フラグ:毎フレーム０
    int			nRollerBack;			// 後向き移動継続時間
    int			bReloadFlag;			// リロード要請フラグ:毎フレーム０
    int			nPhaseCntr;			// フェーズ継続時間
    FVECTOR		vecFatDir;			// ファットマンの向き	
    int			nInviCntr;			// 無敵時間
    int			nNoChkDmgFlag;			// 特殊ダメージフラグ	
    int			nFatPrevInfo;			// ファットマン情報(１フレーム前)
    int			nFatInfo;			// ファットマン情報
    int			nNarcTime;			// 最後に麻酔弾を受けてからの時間
    int			nStunTime;			// 最後にスタングレネードを受けてからの時間
    int			nAttackTime;			// 最後に攻撃を与えてからの時間
    int			nBehindCntr;			// ビハインドカウンタ
    int 		nIntrudeCntr;			// イントルードカウンタ
    int			nBodyDurable;			// 身体耐久
    int			nBodyDurTime;			// 身体耐久
    int			nBodyDmgTime;			// ブラストスーツ部分が最後にダメージを受けてからの時間
    int			nHeadDmgTime;			// 頭部が最後にダメージを受けてからの時間
    int			nRollDmgTime;			// ローラーブレードが最後にダメージを受けてからの時間
    int			nPrevZone;			// 前のフレームでのゾーン		

    // 周辺情報
    int			nDist;				// 視界距離
    float		fEyeAng;			// 視野角度
    int			nLostTrgCntr;			// ターゲットロストカウンタ

    int			nFatArea;			// ファットマンのエリア
    int			nPlaArea;			// プレイヤーのエリア
    int			nAimArea;			// 目標(プレイヤー)のエリア
    int			nAimZone;			// 目標(プレイヤー)ゾーン
    int			aim_dis;			// 目標(プレイヤー)との距離 
    int			aim_dir;			// 目標(プレイヤー)への方向 
    int			pl_dis;				// プレイヤーとの距離 
    int			pl_dir;				// プレイヤーへの方向 
    int			pl_parts;			// ファットマンから見えているプレイヤーの部位
    int			fat_parts;			// プレイヤーから見えているファットマンの部位
    FVECTOR		vecAimPos;			// 目標(プレイヤー）の位置
    FVECTOR		vecPlaDiff;			// プレイヤーとファットマンの位置差分単位ベクトル
    FVECTOR		vecAimDiff;			// 目標(プレイヤー)とファットマンの位置差分単位ベクトル

    int			nEvadeRad;			// 避け半径
    int			nVitalMax;			// 体力カウンタMAX
    int			nVitalCntr;			// 体力カウンタ（０でばてる）
    int			nFireCurrent;			// 発射数	
    int			nFireIntv;			// 発射間隔カウンタ
    int			nHeadDmgFlag;			// 頭部ダメージフラグ

    int			nVibCensorTime;			// 生体センサー用時間

    //------------------- 爆弾関連 : fatc4.c -------------------
    // 爆弾システム系
    int			nPutPosNum;			// 設置ポイント数
    int			nBombPutMin;			// 設置可能数判定用最小
    int			nBombPutMax;			// 設置可能数判定用最大
    int			nBombBaseTime;			// タイマー基本値
    int			nBombAddTime;			// タイマー加算値（1つ増えるといくつ増えるか）

    void*		pvCounter;			// カウンタワークへのポインタ
    int			bCountSay;			// ５,４,３,２,１カウントダウン発声フラグ
    int			bBombActive;			// 爆弾カウントダウンフラグ	
    int			nBombTimer;			// 爆弾起爆までの時間
    int			nBombCntr;			// 爆弾カウンタ
    int			nPutIntvCntr;			// 新設置位置を設定してからのフレーム数
    int			nPutNum;			// 現在の爆弾フェーズで設置した数
    int			nBombNum;			// 爆弾存在数 : 毎フレーム０クリアされる
    int			nBombCountNum;			// カウントダウン中の爆弾数 : 毎フレーム０クリアされる
    int			nFreezeNum;			// 凍らせた数 : ボタン押した時にクリア
    int			nBombMax;			// 許容爆弾数 : 同時に置ける数の上限
    int			nRunningPut;			// 走り設置段階
    int			bPutFlag;			// 設置フラグ
    int			bPutIndexUsed;			// 爆弾設置可能ポイント用インデックスを用いて設置したフラグ

    FVECTOR		vecLastBombPos;			// アクティブな爆弾位置(ゲームオーバー)

    // C4設置位置関連
    int			nC4Level;			// C4設置難易度
    int			nC4Pattern;			// C4設置パターン番号
    int			nC4Index;			// C4設置インデックス      
    int			nC4IndexData[FAT_C4_LEVEL][FAT_C4_PATTERN][FAT_C4_POSNUM];	// 設置場所インデックスデータ
    int			nPointTblUsed[FAT_C4_LEVEL][FAT_C4_PATTERN];			// C4設置使用フラグ
    FAT_PUTPOINT_DATA	datPoint[FAT_C4_POSDATA];	// C4設置場所データ
    FAT_C4_DATA		bombData[MAX_BOMB_NUM];		// C4管理用データ
    FAT_PUTPOINT_DATA*	pCurPosData;			// 現在使用している設置位置データ
    FAT_C4_DATA*	pdatGuard;			// ガード場所データ

    // ルーチン
    int			nPhase;				// フェーズ
    int			nPrevPhase;			// 前のフェーズ
    int			nPhaseNum;			// フェーズ数:爆弾<->銃撃切替え用
    int			nPhaseStartLife;		// フェーズ開始時の耐久値
    int			nPhaseStartFaint;		// フェーズ開始時の気絶値
    int			nLastBombLife;			// 最後の爆弾設置時の耐久値
    int			nLastBombFaint;			// 最後の爆弾設置時の気絶値
    int			nNextZone;			// 目標zone
    int			nNextZone2;			// 目標zone2
    FVECTOR 		vecNextPos;			// 行きたい場所
    FVECTOR 		vecNextPos2;			// 行きたい場所2
    int			nNextZone2Valid;		// 目標zone2有効フラグ
    int			think1;				// 思考レベル:優先レベル
    int			think2;				// 思考レベル:フェーズレベル
    int			think3;				// 思考レベル:動作レベル
    int			count3;				// 動作レベル用カウンタ
    int			nStep;				// 思考内ステップ
    int			nStepCntr;			// 思考内ステップカウンタ	
    int			nShootCntr;			// 発射数
    int			nShootLimit;			// 発射数制限
    int			bNext3Valid;			// nNextThink3有効フラグ
    int			nNextThink3;			// ブレード制御時用次の行動ルーチン番号
    int			nBeforeDmgThink3;		// ダメージ処理前のルーチン
    int			bDemineTrue;			// 解体フェーズ本気モードフラグ
    int			( *CheckThink)( struct _fatman* );	// フェーズ毎の分岐用

    // ベジェ移動関連
    float		fFixBezierRate;			// ベジェ曲線同士の補正レート 
    float		fBezierRate;			// ベジェ曲線用
    float		fBezierOne;			// 1フレームの補間割合変化量
    FMATRIX		matBezier;			// ベジェ曲線用
    FVECTOR		vecPrevDir;			// 計算前の方向
    FVECTOR		vecIncMod1;			// 傾き計算方程式（t）の係数
    FVECTOR		vecIncMod2;			// 傾き計算方程式の係数２
    int			nBezIndex;			// ベジェ補間用移動データのインデックス
    FVECTOR		vecBezStart;			// 現在使用しているベジェ曲線の始点
    FVECTOR		vecBezEnd;			// 現在使用しているベジェ曲線の終点
    FVECTOR		vecBezCtrl;			// 現在使用しているベジェ曲線の制御点
    FVECTOR		vecShift;			// モーションシフト値
    int			nBezAimZone;			// 目標ゾーン
    int			nRunMode;			// 走行時のフラグ	
    int			nMoveForm;			// 移動形式
    int			nQuakeCntr;			// 揺れ時間
    float		nTurnMotRate;			// ターン時のモーションレート
    int			nBezInterp;			// ベジェ移動時の補間

    // フェーズ関連 : 難度調節用
    int			nBombPhaseCntr;				// 爆弾フェーズを行なった回数
    int			nShootPhaseCntr;			// 銃撃フェーズを行なった回数	
    int			bBombStartFlag;				// 起爆フラグ
    int			nConsTime;				// 連続ダメージカウンタ

    // 解体フェーズ
    int			nPlDemineC4Time;		// プレイヤーがＣ４を最後に処理してからの時間
    int			nPlDemineTalkBlank;		// 解体フェーズ台詞ブランク

    // 銃撃フェーズ
    int			nLooseShootMin;				// わざと外す時の幅:最小
    int			nLooseShootMax;				// わざと外す時の幅:最大 
    int			nLooseDecay;				// わざと外す時の幅減衰量
    FVECTOR		vecShootTrg;				// ターゲット位置	

    // エフェクト系
    int			bSpecialEffect;				// 特殊エフェクトフラグ
    int			nShadowFlag;				// 影フラグ
    int			nCartCtrl;			// カートリッジコントロール用
    int			nCartStep;			// カートリッジコントロール用ステップ
    int			nHeadmark;			// ヘッドマークコントロール用

    // カモメ連係
    int			codeKmmngName;			// かもめマネージャ名
    int			bKamomeRest;			// カモメが止まっているか
    int			bPrevKmRest;			// カモメが止まっているか:1フレーム前

    // サウンド関連
    int			nSoundID;			// サウンドからのキャラ判別用ID
    int			nSeCntr;			// 内蔵SEカウンタ:０で再生可能
    int			nStrmOnce;			// ストリーム１思考１回呼出フラグ
    int			nStrmHandle;			// ストリームハンドラ
    int			nStrmIndex;			// 現在のストリームインデックス
    int			nStrmIndexNext;			// 次のストリームインデックス
    int			bStrmProcFlag;			// ストリームプロシージャフラグ
    int			nStrmProc;			// ストリームプロシージャ
    int			nStrmCntr;			// ストリームカウンタ:０で再生可能
    int			nStrmDataNum;			// ストリームデータ数
    int			nStrmNo[FAT_STRM_MAX];		// ストリーム番号:GCL指定
    int			nStrmFlag[FAT_STRM_MAX];	// ストリーム再生:GCL指定
    int			nStrmLen[FAT_STRM_MAX];		// ストリーム長さ:GCL指定

    // データ
    u_char		pbBezierPass[FAT_ROUTEDATA_NUM * 32];	// ベジェ曲線データの通過ゾーンデータ
    int			nRouteZone[FAT_ROUTEDATA_NUM][2];	// ルートデータの始点,終点のゾーン
    int			nEludeZone[FAT_ELUDE_FALL_NUM];		// エルード落としの参照点	
    int			nIntrudeZone[FAT_INTRUDE_ADDR_NUM];	// イントルード攻撃回避ゾーン	
    int			nAttractZone[2];			// 解体フェーズ:余興ポジション
    int			nAreaZone[FATC4_AREA_MAX][5];		// エリア参照ゾーン
    int			nTurnPutRefZone[FAT_TURN_PUT_MAX];	// 回転設置ゾーンデータ

#ifdef DEBUG_MODE
    // debug
    int			nPrevThink1;
    int			nPrevThink2;
    int			nPrevThink3;
    int			nPrevCntr3;
    int			nPrevNextThink3;
    int			nFatFixFace;

    int			nDbgLevel;
    int			nDbgPattern;
    int			nDbgIndex;
#endif
} Work ;

#ifdef DEBUG_MODE
typedef	struct _DbgWork {
    GV_ACT_EX	actor;
    int		count;
    int		where;
    void*	pvFatWork;
} DebugView;
#endif

// inline 関数

// 設置ポイントの種類取得
static inline int FAT_PointType( FAT_PUTPOINT_DATA* ppoint ){ return ppoint->nType; }


// 思考チェック関数設定
typedef	int ( *FATCHECKTHK )( Work* );
static inline void FAT_SetCheckThink( Work* work, FATCHECKTHK func){
    work->CheckThink = func;
}

// 不正ゾーンチェック 1で不正
static inline int FAT_CheckFalseZone( int nZone )
{
    if ( nZone < 0 || nZone == HZX_NO_ZONE ) return 1;

    return 0;
}

#endif // __FATMAN_H__
    


