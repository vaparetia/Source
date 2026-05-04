/*
	w21_common.h                     
		ＥＦ連絡橋

	2000/11/02 H.Yoshiike         
	$Id: w21_common.h,v 1.137 2001/10/29 13:12:33 usr03379 Exp $                      

	
*/

// 各種スイッチ用define
// ------------------------------------
// サウンド
#define		SOUND			1		// サウンドデータは最初に呼ぶ
#define		BGM				1		// BGMは敵兵コマンダーの後に呼ぶ
// デモ
// キャラクター関係
#define		RAIDEN			1		// SNAKEとは排他
#define		SNAKE			0
#define		ENEMY			1		// コマンダーと警備兵(敵を出すときは必須)
#define		SUPPORT			0		// サポート兵
#define		ATTACKER		0		// 攻撃兵
#define		SHIELD			0		// 盾兵(1) or 通常攻撃兵(0)

#define		CYPHER			1		//	サイファー
// ドア
#define		SLIDE_DOOR		1		// スライドドア
// 入出トラップ
#define		LOAD_TRAP		1
#define		STAGE_END		1		// ステージ終了時処理を使用可にする
// フォグ
#define		FOG				1
// エフェクト関係
#define		EFFECT			0		// エフェクト全般
#define		BLUR			0		// ブラー
#define		Z_MANAGER		0		// Ｚフォーカス管理(敵のライト等で仕様)
#define		BODY_SHADOW		0		// 敵のライトによって出る影
// 子画面
#define		SUB_CAMERA		0
// カメラ関係
#define		CLEAR_CAMERA	0		// クリアリング専用カメラ(要子画面)

// 天井
#define		CHARA_CEIL		1		// 天井君
// 透明壁
#define		TR_WALL			0		// シナリオ置きの透明壁
// ライト関係
#define		CHARA_LIGHT		1		//	キャラ用ライト

// オブジェ関係
#define		SEA				1		// 海面
#define		LOCKER			0		// ロッカー
#define		SKY				1		// 空
// 罠関係
#define		CRAYMORE		1		// クレイモア地雷
#define		PIT				1		// 落とし穴

// アイテム
#define		ITEM			1

// デモ
#define		SCN_DEMO			1	//	シナリオデモ
#define		POLY_DEMO			0

// デバッグ関係
#ifdef	d:FIX_MODE
	#define		DEBUG_CHARA		0		// デバッグキャラ全体(FIX時は0)
#else
	#define		DEBUG_CHARA		0		// デバッグキャラ全体
#endif
#define		CHARA_CAMERA	0		// キャラ追従カメラ
#define		TARGET_CHARA	敵兵:01	// キャラ追従カメラのターゲット
#define		SCREENSHOT		0		// スクリーンショット
#define		USB_KEY			0		// USBキーボード



// 各種設定用define
// ------------------------------------
// マップ関係

//	アタリ
#define		HZX_NAME			w21a
												//	0		プレイヤーアタリ／跳弾アタリ
												//	1		レンズフレア用アタリ
												//	2〜18	遠景用跳弾アタリ

//ＡＢ個別
#if d:W21A
	//	ステージ
	#define		KMS_NAME0			w21a
	//	ライト
	#define		LT2_NAME			w21a

#else
	#define		KMS_NAME0			w21b
	//	ライト
	#define		LT2_NAME			w21b
#endif

#if d:W21A
	//	遠景
	#define		KMS_NAME8			w21_25a			//	ＤＧ連絡橋壊れ前
#else
	//	遠景
	#define		KMS_NAME8			w21_25b			//	ＤＧ連絡橋壊れ後
#endif

//	ＡＢ共通
#define		KMS_NAME1			w21_12a			//	Ａ脚
#define		KMS_NAME2			w21_13a			//	ＡＢ連絡橋
#define		KMS_NAME3			w21_17a			//	ＣＤ連絡橋
#define		KMS_NAME4			w21_19a			//	ＤＥ連絡橋

#define		KMS_NAME6			w21_23a			//	ＦＡ連絡橋
#define		KMS_NAME7			w21_24a			//	シェル１中央棟

#define		KMS_NAME12			w21_31a			//	シェル２中央棟
#define		KMS_NAME13			w21_32b			//	狙撃イベント１
#define		KMS_NAME14			w21_32a			//	狙撃イベント２
#define		KMS_NAME15			w21_crane		//	シェル１中央棟クレーン
#define		KMS_NAME16			w21_32c			//	狙撃イベント３
#define		KMS_NAME17			w21_l			//	Ｌ脚
#define		KMS_NAME18			w21_gla			//	ＧＬ連絡橋壊れ前	//	昼のみなのでそのうち移行

//	ドア
#define		DOOR_NAME_LOAD1		w21a_dr00		// ロード用
#define		DOOR_NAME_LOAD2		w21a_dr01		// ロード用

//	天井くんおき
#define		KMS_NAME5			w21_20a			//	Ｅ脚天井
#define		KMS_NAME9			w21_20b			//	ヘリポート
#define		KMS_NAME10			w21_22a			//	Ｆ脚天井

//	プットオブジェおき
#define		KMS_NAME11			w21_24b			//	シェル１中央棟ロードエリア
#define		KMS_NAME19			w21_20a0		//	Ｅ脚ロードエリア
#define		KMS_NAME20			w21_22b			//	Ｆ脚ロードエリア



// キャラクター関係
#if d:RAIDEN
	#define		PLAYER		ライデン
#else
	#define		PLAYER		スネーク
#endif

// 強制モーション
#define		BY_MOTION		1		// ビヨンドモーション
#define		BY_HIGH			1		// 高所ビヨンドモーション

//	薬莢系
#if d:W21A
	#define		ENE_P_GOL			1	//	都市迷彩ゴル
#endif

	#define		GUN_MECA			1	//	ガンサイファー	//	今はハングるのでコメントにしとく

//	ステージ系
#if d:W21A
	#define		STAGE_W21A			1	//	昼
#else
	#define		STAGE_W21B			1	//	夕方
#endif

#define			STAGE_PLANT			1	//	プラント編


// ローカル変数
//	$f:w21a_ガンサイファー起動フラグ	//	ガンサイファーを起動したかどうかのフラグ	しようしてない

//	$f:w21a_ガンサイファー１フラグ	//	ガンサイファー:01がどのルートにいるかのフラグ	しようしてない
//	$f:w21a_ガンサイファー２フラグ	//	ガンサイファー:02がどのルートにいるかのフラグ	しようしてない
//	$f:w21a_ガンサイファー３フラグ	//	ガンサイファー:03がどのルートにいるかのフラグ	しようしてない

//	$f:w21a_クレイモア１取得フラグ	//	クレイモアをとったかどうかのフラグ
//	$f:w21a_クレイモア２取得フラグ	//	クレイモアをとったかどうかのフラグ					しようしてない
//	$f:w21a_クレイモア３取得フラグ	//	クレイモアをとったかどうかのフラグ
//	$f:w21a_クレイモア４取得フラグ	//	クレイモアをとったかどうかのフラグ
//	$f:w21a_クレイモア５取得フラグ	//	クレイモアをとったかどうかのフラグ					しようしてない
//	$f:w21a_クレイモア６取得フラグ	//	クレイモアをとったかどうかのフラグ
//	$f:w21a_クレイモア７取得フラグ	//	クレイモアをとったかどうかのフラグ					しようしてない
//	$f:w21a_クレイモア８取得フラグ	//	クレイモアをとったかどうかのフラグ					しようしてない
//	$f:w21a_クレイモア９取得フラグ	//	クレイモアをとったかどうかのフラグ
//	$f:w21a_クレイモア１０取得フラグ	//	クレイモアをとったかどうかのフラグ
//	$f:w21a_クレイモア１１取得フラグ	//	クレイモアをとったかどうかのフラグ


//	$f:w21a_落し穴１Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴１Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴２Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴２Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴３Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴３Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴４Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴４Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴５Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴５Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴６Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴６Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴７Ａ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない
//	$f:w21a_落し穴７Ｂ上フラグ			//	落し穴の上にいるかどうかのフラグ					しようしてない

//	$f:w21a_落し穴１Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴１Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴２Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴２Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴３Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴３Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴４Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴４Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴５Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴５Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴６Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴６Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴７Ａ開いたフラグ		//	落し穴が開いているかどうかのフラグ
//	$f:w21a_落し穴７Ｂ開いたフラグ		//	落し穴が開いているかどうかのフラグ


//	$f:w21a_双眼鏡兵覗きフラグ			//	双眼鏡兵が覗いているかどうかのフラグ

//	$f:w21a_ガンサイファー01破壊フラグ	//	ガンサイファーが破壊されたかどうか
//	$f:w21a_ガンサイファー02破壊フラグ	//	ガンサイファーが破壊されたかどうか
//	$f:w21a_ガンサイファー03破壊フラグ	//	ガンサイファーが破壊されたかどうか

//	$i:w21a_敵兵モード					//	敵兵のモード
//	$f:w21a_双眼鏡兵所定位置フラグ		//	双眼鏡兵が所定位置にいるか

//	$f:w21a_ガンサイファー再セットフラグ	//	ガンサイファーの強制再セット用

#define		WAIT		0		//	待機中
#define		PATROL		1		//	巡回中

//	その他
#define CTDOWN_TIMER_P		1		//	爆弾イベント用
#define START_CAMERA		1		//	開始処理用カメラ

// include
// ------------------------------------
#include "rai_w21a.mh"		// 強制モーションリスト
#include "stdch.h"
#include "sload.h"
#include "beyond.h"
#include "fall.h"
#include "sound.h"
#include "enevoice.h"
#include "item_info.h"
#include "w21_kamome.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif


// マップ設定
// ------------------------------------
chara マップ ＥＦ連絡橋 \
		-k d:KMS_NAME0 \
		-k d:KMS_NAME1 \
		-k d:KMS_NAME2 \
		-k d:KMS_NAME3 \
		-k d:KMS_NAME4 \
		
		-k d:KMS_NAME6 \
		-k d:KMS_NAME7 \
		-k d:KMS_NAME8 \

		-k d:KMS_NAME12 \
		-k d:KMS_NAME13 \
		-k d:KMS_NAME14 \
		-k d:KMS_NAME15 \
		-k d:KMS_NAME16 \
		-k d:KMS_NAME17 \
		-k d:KMS_NAME18 \
		-h d:HZX_NAME,0 -l d:LT2_NAME

//	ＨＺＸ追加登録
//	跳弾アタリ用マップ
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 2
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 3
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 4
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 5
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 6
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 7
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 8
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 9
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 10
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 12
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 13
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 14
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 15
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 16
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 17
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 18

//	昼
#if d:W21A
//	Ｇ脚壊れ前
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 11
//	Ｅ脚影
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 20

//	夕
#else
//	Ｇ脚壊れ後
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 19
//	Ｅ脚影
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 21
#endif


//	昼
#if d:W21A
//	グループ１はレンズフレア用ダミー
command 追加ＨＺＸグループ登録 \
	-hzx_id 1 -add_id 0

//	夕
#else
//	グループ１はレンズフレア用ダミー
command 追加ＨＺＸグループ登録 \
	-hzx_id 1 -add_id 0
command 追加ＨＺＸグループ登録 \
	-hzx_id 1 -add_id 8		//	８番はＦ脚（この向こう側に太陽があるので透けるのを防ぐ）

#endif



command マップ設定 ＥＦ連絡橋 -set {
	//	デモが起きるかどうかのチェック
	@双眼鏡兵遭遇デモ開始チェック

#if d:SOUND	
	@サウンド設定	// データは最初に呼ぶ
#endif
	//	ノードアクセス関連
	if ( $f:w20a_ノードフラグ == 1 || $f:w22a_ノードフラグ == 1 \
		|| $f:w24a_ノードフラグ == 1) {
		@レーダーオンオフチェック 1
	} else {
		@レーダーオンオフチェック 0
	}
	@常駐キャラ設定
	@ローカル変数初期化
	@設定値変更
	@プラグイン設定
	@水位設定

	@プレイヤー設定
#if d:ENEMY
	@コマンダー設定
#endif
#if d:CYPHER
	@ガンサイファー配置チェック		//	ガンサイファー再セット用
	@ガンサイファー設定
#endif
#if d:BGM	
	@ＢＧＭ設定						//	プレイヤー→敵兵→BGMの順で呼ぶ
#endif
#if d:SLIDE_DOOR
	@ドア設置
	@主観禁止壁設定
#endif
#if d:LOAD_TRAP
	@ロード設定
#endif
	@オブジェクト設定
#if d:SEA
	@海面設定
#endif
#if d:SKY
	@空設定
#endif
#if d:CHARA_CEIL
	@天井設定
#endif

#if d:CRAYMORE
	@クレイモア設定
#endif
#if d:PIT
	@落し穴配置チェック
	@落し穴設定
#endif

	@かもめ設定

#if d:ITEM
	@アイテム設定
#endif
#if d:FOG
	@フォグ設定
#endif
#if d:EFFECT
	@エフェクト設定
#endif
	@ＥＦ連絡橋風設定
#if d:BY_MOTION
	@ＥＦ連絡橋ビヨンド設定
#endif
	@カメラセット
	@ＥＦ連絡橋カメラ設定

	@エルード落下死設定

#if d:SCN_DEMO
//	@双眼鏡兵遭遇デモ設定		//	開始時処理の中へ
	@シナリオデモキャラセット
	@双眼鏡兵発見デモ設定
# endif

#if d:CHARA_LIGHT
	@キャラ用ライト設定
# endif
	@爆弾タイマー設置	//	爆弾イベント用
	@プラント編全体マップ設定 d:ＥＦ連絡橋マップ	//	全体マップ表示

#if d:DEBUG_CHARA
//	@デバッグキャラセット
#endif
//	@システム設定	//	設定値まるごと変更

}


// 無線用include ※位置は動かさないこと
#include "w21_cdc.ch"

//初期マップの表示
command マップ表示 -show ＥＦ連絡橋


// 全てのセットが終わったところでフェードイン
@ＥＦ連絡橋開始時処理





//###################################################################################
//									各proc設定
//###################################################################################



//###################################################################################
//									全マップ共通
//###################################################################################

	proc サウンド設定 {
		#if d:DEBUG_PRINT 
			print 'sound_set'
		#endif

//	昼の場合
#if d:W21A
	//	英語版は
	#ifdef d:ENGLISH
		if ($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
			chara サウンドマネージャー ＳＤマネ -pak 0
		} else {
			chara サウンドマネージャー ＳＤマネ -pak 1
		}

	//	それ以外は
	#else
		chara サウンドマネージャー ＳＤマネ -pak 0
	#endif

//	夕の場合
#else
		chara サウンドマネージャー ＳＤマネ -pak 0
#endif

		//		ＳＥ設定
		@サウンド効果音設定
		command 弾痕ノーマル -n 0
		command 弾痕ノーマル -n 1
		command 弾痕ノーマル -n 3
		command 弾痕ノーマル -n 4

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 3
		command 跳弾ノーマル -n 4

	}

	proc ローカル変数初期化 {
		#if d:DEBUG_PRINT 
			print 'localvar_init'
		#endif
/*
		eval($f:w21a_ガンサイファー起動フラグ = d:FALSE)
		eval($f:w21a_ガンサイファー１フラグ = d:WAIT)
		eval($f:w21a_ガンサイファー１フラグ = d:WAIT)
		eval($f:w21a_ガンサイファー１フラグ = d:WAIT)
*/
/*
		eval ($f:w21a_落し穴１Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴１Ｂ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴２Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴２Ｂ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴３Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴３Ｂ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴４Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴４Ｂ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴５Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴５Ｂ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴６Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴６Ｂ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴７Ａ上フラグ = d:FALSE);
		eval ($f:w21a_落し穴７Ｂ上フラグ = d:FALSE);
*/
		eval($f:w21a_双眼鏡兵覗きフラグ = d:FALSE)
		eval($i:w21a_敵兵モード = 0)
		eval($f:w21a_双眼鏡兵所定位置フラグ = d:FALSE)

	//	エイムズ死亡後、シェル１から来たときは落し穴を無理矢理リセットする
		if ($w:p_story == d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
			if ($s:直前のエリア == w24a) {
				eval ($f:w21a_落し穴１Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴１Ｂ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴２Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴２Ｂ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴３Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴３Ｂ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴４Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴４Ｂ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴５Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴５Ｂ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴６Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴６Ｂ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴７Ａ開いたフラグ = d:FALSE);
				eval ($f:w21a_落し穴７Ｂ開いたフラグ = d:FALSE);
			}
		}

	//	日本語版
	#ifdef d:JAPANESE
		//	ベリーイージーでは常に落し穴復活
		if ($w:ゲーム設定 == d:LEVEL_VERYEASY) {
			eval ($f:w21a_落し穴１Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴１Ｂ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴２Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴２Ｂ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴３Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴３Ｂ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴４Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴４Ｂ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴５Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴５Ｂ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴６Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴６Ｂ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴７Ａ開いたフラグ = d:FALSE);
			eval ($f:w21a_落し穴７Ｂ開いたフラグ = d:FALSE);
		}
	#endif
	}

	proc 設定値変更 {
		#if d:DEBUG_PRINT 
			print 'status_set'
		#endif
/*
		eval($w:敵危険視力 = 10000);
		eval($w:敵回避視力 = 10000);
		eval($w:敵標準聴力 = 10000);
*/
		//	ここでは射程が伸びる
		command 主観射程距離設定 \
			-len 50000

	}

	proc プラグイン設定 {
		#if d:DEBUG_PRINT
			print 'plugin_set'
		#endif
		command プラグイン落下
	}



//###################################################################################
//									 プレイヤー
//###################################################################################

	proc プレイヤー設定 {
		#if d:DEBUG_PRINT
			print 'player_set'
		#endif

		#if d:RAIDEN
			//	ライデンくん
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-m rai_def -a raiden -o rai_w21a

			//	髪の毛
			@ライデン髪の毛設定 d:NORMAL_HAIR

			//	ＬＯＤ機能
			@プレイヤーＬＯＤ設定 d:C_DISTANCE_TYPE 5000
//			@プレイヤーＬＯＤ設定 d:P_STATE_TYPE d:TRP_STATE_BEHIND



		#else if d:SNAKE
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s 0 \
				-m sna_def -a snake -o rai_common

				//バンダナ
				chara ロープモデル３ バンダナ１ \
					-n sna_bdn1 \
					-d 1 \
					-y スネーク \
					-z 12 \
					-x 0,60,-90 \
					-m 12 \
					-p 250 \
					-k 350 \
					-o 110 \
					-f 1

				chara ロープモデル３ バンダナ２ \
					-n sna_bdn2 \
					-d 1 \
					-y スネーク \
					-z 12 \
					-x 0,50,-90 \
					-m 22 \
					-p 250 \
					-k 350 \
					-o 110 \
					-f 1
		#endif
	}



//###################################################################################
//								敵兵
//###################################################################################

//	敵兵配置

//	基本セット

	//	警備兵
	proc 警備兵セット {

		//	ストーリーフラグや難易度で分岐
		@警備兵０	//	敵１人	双眼鏡兵
	}





//	コマンダーセット

	//	警備兵コマンダー
	proc 警備兵コマンダー {
		#if d:DEBUG_PRINT
			print 'wccomm_set'
		#endif

			chara 警備コマンダー wccomm -e {警備兵セット}
	}

	//	コマンダー

	//	司令官音声
	resource ghq_area_alert_voice {
	w21a:
		$t{

			// PLAREA0
			t:vc040198,	//	指揮官　　「了解。増援部隊をＥＦ連絡橋に派遣する！侵入者を止めろ！」
			t:vc040199,	//	指揮官　　「了解。応援部隊、ＥＦ連絡橋へ急行せよ！敵を逃がすな」
			-2,
			// PLAREA1
			t:vc040194,	//	指揮官　　「了解。増援部隊、ＥＦ連絡橋に急行せよ！Ｅ脚入口を守れ！」
			t:vc040195,	//	指揮官　　「了解。増援部隊をＥＦ連絡橋・Ｅ脚入口に送る！奴を逃がすな！」
			-2,
			// PLAREA2
			t:vc040196,	//	指揮官　　「了解。応援部隊、ＥＦ連絡橋に向かえ！Ｆ脚入口を守れ！」
			t:vc040197,	//	指揮官　　「了解。増援部隊、ＥＦ連絡橋・Ｆ脚入口へ向かえ！敵を仕留めろ！」
			-1

/*
			// PLAREA0
			t:vc040007,	//	指揮官　　「了解。奴を逃がすな！」
			t:vc040009,	//	指揮官　　「了解。敵を殲滅せよ！」
			-2,
			// PLAREA1
			t:vc040007,	//	指揮官　　「了解。奴を逃がすな！」
			t:vc040009,	//	指揮官　　「了解。敵を殲滅せよ！」
			-2,
			// PLAREA2
			t:vc040007,	//	指揮官　　「了解。奴を逃がすな！」
			t:vc040009,	//	指揮官　　「了解。敵を殲滅せよ！」
			-1
*/
		}
	}

	resource ghq_caution_voice {
	w21a:
		$t{
/*
			// PLAREA0
			t:vc040601,	//	指揮官　　「了解、総員、警戒を強化せよ」

			t:vc040354,	//	指揮官　　「了解。ＥＦ連絡橋・Ｅ脚入口に増員を派遣する。警戒を強化しろ」
			t:vc040355,	//	指揮官　　「了解。ＥＦ連絡橋に増員。Ｅ脚入口の警備を固めろ」
			t:vc040356,	//	指揮官　　「了解。ＥＦ連絡橋・Ｆ脚入口に増員を派遣する。警戒を強化しろ」
			t:vc040357,	//	指揮官　　「了解。ＥＦ連絡橋に増員。Ｆ脚入口の警備を固めろ」
			t:vc040358,	//	指揮官　　「了解。ＥＦ連絡橋を増員する。警戒を強化しろ」
			-2,
			t:vc040429,	//	指揮官　　「ＥＦ連絡橋との連絡が途絶えた」
			-1
*/
			// PLAREA0
			t:vc040601,	//	指揮官　　「了解、総員、警戒を強化せよ」

			t:vc040496,	//	指揮官　　「攻撃チーム、配置に戻れ」
			t:vc040497,	//	指揮官　　「攻撃チーム、撤収せよ」
			-2,
			t:vc040429,	//	指揮官　　「ＥＦ連絡橋との連絡が途絶えた」
			-3,
			t:vc040610,	//	カメラ用「異状発生…」
			-1
		}
	}

	proc コマンダー設定 {
		#if d:DEBUG_PRINT
			print 'commander_set'
		#endif

	//	昼
	#if d:W21A

		//	日本語版
		#ifdef d:JAPANESE

		//	ベリーイージーでは双眼鏡兵いない
		if ($w:ゲーム設定 == d:LEVEL_VERYEASY) {
			chara コマンダー 敵兵セット \
				-v [ene_plant_voice:01] \
				-x [ghq_area_alert_voice:w21a] \
				-y [ghq_caution_voice:w21a] \
				-s d:ENE_STAGE_NO_STRM \
				-m $w:スタートアラートモード \
				-h $i:標準警戒時間 \		// 警戒時間
				-t $w:サイファー標準再発生数 \
				-u $w:サイファー標準再発生数 \
				-l ガンサイファー全滅

		//	イージー以上はいる
		} else {
			//	敵兵メモリーセットにおまかせ
			chara コマンダー 敵兵セット \
				-w 警備兵コマンダー \
				-r 60000,4500,-90250 ＥＦ連絡橋 \
				-c 60000,4500,-90250 ＥＦ連絡橋 \
				-v [ene_plant_voice:04] \
				-x [ghq_area_alert_voice:w21a] \
				-y [ghq_caution_voice:w21a] \
				-z $i:敵標準麻酔持続 \
				-o $i:敵標準気絶持続 \
				-b $w:敵標準再発生数 \
				-s d:ENE_STAGE_GPS \
				-m $w:スタートアラートモード \
				-h $i:標準警戒時間 \		// 警戒時間
				-t $w:サイファー標準再発生数 \
				-u $w:サイファー標準再発生数 \
				-l ガンサイファー全滅 \
				-e	1 0 双眼鏡兵覗き開始 \
					1 8 双眼鏡兵覗き終了
		
		}

		//	それ以外
		#else
		//	敵兵メモリーセットにおまかせ
		chara コマンダー 敵兵セット \
			-w 警備兵コマンダー \
			-r 60000,4500,-90250 ＥＦ連絡橋 \
			-c 60000,4500,-90250 ＥＦ連絡橋 \
			-v [ene_plant_voice:04] \
			-x [ghq_area_alert_voice:w21a] \
			-y [ghq_caution_voice:w21a] \
			-z $i:敵標準麻酔持続 \
			-o $i:敵標準気絶持続 \
			-b $w:敵標準再発生数 \
			-s d:ENE_STAGE_GPS \
			-m $w:スタートアラートモード \
			-h $i:標準警戒時間 \		// 警戒時間
			-t $w:サイファー標準再発生数 \
			-u $w:サイファー標準再発生数 \
			-l ガンサイファー全滅 \
			-e	1 0 双眼鏡兵覗き開始 \
				1 8 双眼鏡兵覗き終了
		#endif

	//	夕方
	#else
		chara コマンダー 敵兵セット \
			-v [ene_plant_voice:01] \
			-x [ghq_area_alert_voice:w21a] \
			-y [ghq_caution_voice:w21a] \
			-s d:ENE_STAGE_NO_STRM \
			-m $w:スタートアラートモード \
			-h $i:標準警戒時間 \		// 警戒時間
			-t $w:サイファー標準再発生数 \
			-u $w:サイファー標準再発生数 \
			-l ガンサイファー全滅
	#endif
	}





//	警備兵セット

	//	警備兵
	proc 警備兵０ {
		#if d:DEBUG_PRINT
			print 'watcher_set'
		#endif

		//	開始時の敵兵状態をセットする
		@Ｗ２１Ａ敵兵開始状態チェック	w21a ＥＦ連絡橋 敵兵:01 42000 5500 -90000 2048

		chara 警備兵 敵兵:01 \
//			-r 1 -n 14 -d 59000,11500,-78750 ＥＦ連絡橋 \
			-r 1 -n 14 -d 63750,4500,-90250 ＥＦ連絡橋 \
//			-i ($w:敵潜入視力 +8000), ($w:敵危険視力 +8000), ($w:敵回避視力 +8000) \
			-i $w:敵潜入視力, $w:敵危険視力, $w:敵回避視力 \
			-l $w:敵標準体力 -h $w:敵標準聴力 -f $w:敵標準気絶 \
			-s d:ENE_STATUS_ACCDNT_REPO \ /* 発見後アクシデント報告 */
			-j ドッグタグ_敵兵 \
				[dogtag:plt_w20b_guard_ve_01] [dogtag:plt_w20b_guard_ea_01] [dogtag:plt_w20b_guard_no_01] \
				[dogtag:plt_w20b_guard_ha_01] [dogtag:plt_w20b_guard_vh_01] \
			-z 23000 \
			-e 双眼鏡兵死亡
	}

	//	敵兵死亡処理
	proc 双眼鏡兵死亡 {
		//	殺したらフラグを立てる
//		eval ($f:w20b_２Ｆ敵兵死亡フラグ = d:TRUE)
	}





//	サイファーセット

	//	サイファー配置チェック
	//	コマンダー一括管理のためこちらに移行
	proc ガンサイファー配置チェック {
		print '++++++++++++++++++++++++++++++++++++++++++GUN_CYPHER' $w:グローバルロード回数
//		@壊れ物再セットチェック $b:w21a_破壊ロード回数[14]
		@サイファー再セットチェック $b:w21a_破壊ロード回数[14]

		if ($f:再セットフラグ) {
			print 'GUN_CYPHER_REBIRTH'
			eval ($f:w21a_ガンサイファー01破壊フラグ = d:FALSE)
		}

		//	狙撃イベント後始めてきた場合は必ずガンサイファーセット
		if ($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了) {
			if ($f:w21a_ガンサイファー再セットフラグ  == d:FALSE) {
				eval ($f:w21a_ガンサイファー再セットフラグ = d:TRUE)
				eval ($f:w21a_ガンサイファー01破壊フラグ = d:FALSE)
			}
		}

	}

	//	サイファー全滅プロック
	proc ガンサイファー全滅 {
		print 'GUN_CYPHER_DESTORY'
		eval ($b:w21a_破壊ロード回数[14] = $w:グローバルロード回数)
		eval ($f:w21a_ガンサイファー01破壊フラグ = d:TRUE)
		print '##########################################GUN_CYPHER' $b:w21a_破壊ロード回数[14]
	}

	//	サイファー設定
	//	コマンダー一括管理のためこちらに移行
	proc ガンサイファー設定 {
		@ガンサイファー０

	}

	proc ガンサイファー０ {

		//	サイファー
		if ($f:w21a_ガンサイファー01破壊フラグ == d:FALSE) {
			//	左上
			chara サイファ ガンサイファー:01 \
				-route 2 0 \ 
				-type 1 \ /*0-NORMAL,1-GUN*/
				-i 8000 16000 8000 8000 \
				-speed 15 \
				-kinkyuspeed 300 \
				-skipflag 1 \
				-fkasoku 200 275 \
				-zidouset	2 5 8

			//	右下
			chara サイファ  ガンサイファー:02 \
				-route 3 0 \ 
				-type 1 \ /*0-NORMAL,1-GUN*/
				-i 8000 16000 8000 8000 \
				-speed 30 \
				-kinkyuspeed 300 \
				-skipflag 1 \
				-fkasoku 200 275 \
				-zidouset	3 6 9

			//	左下
			chara サイファ  ガンサイファー:03 \
				-route 4 0 \ 
				-type 1 \ /*0-NORMAL,1-GUN*/
				-i 8000 16000 8000 8000 \
				-speed 60 \
				-kinkyuspeed 300 \
				-skipflag 1 \
				-fkasoku 200 275 \
				-zidouset	4 7 10


			//	ハード以下
			if ($w:ゲーム設定 <= d:LEVEL_HARD) {

				//	カード２を所持した以降は時間で来る
				if ( $w:p_story >= d:ST:P036_05_P04忍者登場５ポリゴンデモ４開始 ) {
					chara delay ガンサイファー呼び寄せディレイ \
						-t 300 \
						-e {
					print '##########################################################################YOBIYOSECHECK'
							mesg サイファ ガンサイファー:01 呼び寄せ
							mesg サイファ ガンサイファー:02 呼び寄せ
							mesg サイファ ガンサイファー:03 呼び寄せ
						}
				}

				//	狙撃イベント後は最初から巡回している
				if ($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了) {
					print '##########################################################################CYPCHECK'
					mesg サイファ ガンサイファー:01 呼び寄せルートワープ 8 0
					mesg サイファ ガンサイファー:02 呼び寄せルートワープ 9 2
					mesg サイファ ガンサイファー:03 呼び寄せルートワープ 10 0
				}

				//	ノーマル以上
				if ($w:ゲーム設定 >= d:LEVEL_NORMAL) {
						command ゲット敵兵状態 敵兵:01 $i:w21a_敵兵状態
						print '##########################################################################ENEMYCHECK'
						//	敵が死んでいたらサイファーが飛んでくる
						if ($i:w21a_敵兵状態 & 0x80000000) {
							print '##########################################################################ENEMY-DEAD'

							// Ｅ脚から
							if( $s:登場ポイント == sp_w20a0_w21a_0 ) {
								mesg サイファ ガンサイファー:02 呼び寄せルートワープ 9 0
								mesg サイファ ガンサイファー:03 呼び寄せルートワープ 10 0

							// Ｆ脚から
							} else if( $s:登場ポイント == sp_w22a0g_w21a_0 ) {
								mesg サイファ ガンサイファー:01 呼び寄せルートワープ 8 0
								mesg サイファ ガンサイファー:03 呼び寄せルートワープ 10 0

							// 中央棟から
							} else if( $s:登場ポイント == sp_w24a_w21a_0 ) {
								mesg サイファ ガンサイファー:01 呼び寄せルートワープ 8 0
								mesg サイファ ガンサイファー:02 呼び寄せルートワープ 9 0
							}
						}
				}

			//	エクストリーム
			} else {
				mesg サイファ ガンサイファー:01 呼び寄せルートワープ 8 0
				mesg サイファ ガンサイファー:02 呼び寄せルートワープ 9 2
				mesg サイファ ガンサイファー:03 呼び寄せルートワープ 10 0
			}

		}
	}




//###################################################################################
//								ＢＧＭ設定
//###################################################################################

	proc ＢＧＭ設定 {
		#if d:DEBUG_PRINT 
			print 'bgm_set'
		#endif

		chara ＢＧＭマネージャー ＢＧＭマネ \
			-s 0x00000001
		chara ＢＧＭ風音 風音 \
		-s d:BGM_WINDNOISE_WEATHER_FAIN

	}

//###################################################################################
//									 ドア＆マップ切り替え
//###################################################################################


	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		chara ドア Ｅ脚ドア \
			-m d:DOOR_NAME_LOAD2 \
			-d 0,2048,0 \
			-p 61000,0,-76375 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＥＦ連絡橋 ＥＦ連絡橋 \
			-A Ｅ脚ドアランプ \
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｅ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｅ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｅ脚ドアランプ 2048 61000 0 -76375 1

		trap dr001 ？ \
			-mask ？ \	
			-exec {
					if($3==入る){
						mesg ドア Ｅ脚ドア open
					}else{
						mesg ドア Ｅ脚ドア close
					}
			}

	#if d:W21A
		//	昼
		chara ドア Ｆ脚ドア \
			-m d:DOOR_NAME_LOAD1 \
			-d 0,0,0 \
			-p 57000,0,-43625 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＥＦ連絡橋 ＥＦ連絡橋 \
			-A Ｆ脚ドアランプ \
			-card_level 1 \
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｆ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｆ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｆ脚ドアランプ 0 57000 0 -43625 1

	#else
		//	夕方
		chara ドア Ｆ脚ドア \
			-m d:DOOR_NAME_LOAD1 \
			-d 0,0,0 \
			-p 57000,0,-43625 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＥＦ連絡橋 ＥＦ連絡橋 \
			-A Ｆ脚ドアランプ \
			-card_level 9999 \			//	無理矢理開かなくする
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｆ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｆ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｆ脚ドアランプ 0 57000 0 -43625 1
	#endif

		//	昼のみドアは開く
		trap dr003 ？ \
			-mask ？ \	
			-exec {
					if ($3==入る) {
						mesg ドア Ｆ脚ドア open $2
					} else {
						mesg ドア Ｆ脚ドア close $2
					}
			}

		chara ドア 中央棟ドア \
			-m d:DOOR_NAME_LOAD2 \
			-d 0,1024,0 \
			-p 19125,-1500,-54250 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＥＦ連絡橋 ＥＦ連絡橋 \
			-A 中央棟ドアランプ \
			-card_level 2 \
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ シェル１中央棟ロードエリア off
				} else {
					mesg プットオブジェ シェル１中央棟ロードエリア on
				}
			}
		@ドアランプ設定 中央棟ドアランプ 1024 19125 -1500 -54250 0

		trap dr004 ？ \
			-mask ？ \	
			-exec {
					if ($3==入る) {
						mesg ドア 中央棟ドア open $2
					} else {
						mesg ドア 中央棟ドア close $2
					}
			}

	}

	proc 主観禁止壁設定 {

		@主観禁止壁 Ｅ脚ドア壁 61000 0 -76375 2048 d:SD_SLIDE_OUTSIDE
		@主観禁止床 Ｅ脚ドア床１ Ｅ脚ドア床２ 61000 0 -76375 2048 d:SD_SLIDE_OUTSIDE
		trap dr001 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ｅ脚ドア壁 on
				} else {
					mesg 透明壁 Ｅ脚ドア壁 off
				}
			}

		@主観禁止壁 Ｆ脚ドア壁 57000 0 -43625 0 d:SD_SLIDE_OUTSIDE
		@主観禁止床 Ｆ脚ドア床１ Ｆ脚ドア床２ 57000 0 -43625 0 d:SD_SLIDE_OUTSIDE
		trap dr003 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ｆ脚ドア壁 on
				} else {
					mesg 透明壁 Ｆ脚ドア壁 off
				}
			}

		@主観禁止壁 中央棟ドア壁 19125 -1500 -54250 1024 d:SD_SLIDE_OUTSIDE
		@主観禁止床 中央棟ドア床１ 中央棟ドア床２ 19125 -1500 -54250 1024 d:SD_SLIDE_OUTSIDE
		trap dr004 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if( $3 == 入る ) {
					mesg 透明壁 中央棟ドア壁 on
				} else {
					mesg 透明壁 中央棟ドア壁 off
				}
			}

	}


//###################################################################################
//									開始時処理
//###################################################################################

	proc ＥＦ連絡橋開始時処理 {
		//	双眼鏡兵遭遇デモ後は
		if ($s:登場ポイント == 双眼鏡兵遭遇デモ後) {
				@ステージ開始時処理  d:LOAD_TYPE:NO_MOVE_TYPE 0 0 0

		//	それ以外は
		} else {
			if( $s:登場ポイント == sp_w20a0_w21a_0 ) {
				// Ｅ脚から
				@ステージ開始時処理（ステージ名表示） \
				d:LOAD_TYPE:SL_DOOR_TYPE Ｅ脚ドア 2250 d:DEFAULT_M_TYPE \
				d:wn_ＥＦ連絡橋

			} else if( $s:登場ポイント == sp_w22a0g_w21a_0 ) {
	//	日本語版
	#ifdef d:JAPANESE
				//	ベリーイージーでは敵はいない
				if ($w:ゲーム設定 == d:LEVEL_VERYEASY) {
					@ステージ開始時処理（ステージ名表示） \
					d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
					d:wn_ＥＦ連絡橋
				} else {
					// Ｆ脚から
					if ($f:w19a_双眼鏡兵遭遇デモフラグ == d:FALSE) {
						//	双眼鏡兵遭遇デモフラグを立てる（危険モード時はデモは二度と見ない）
						eval ($f:w19a_双眼鏡兵遭遇デモフラグ = d:TRUE)
						//	潜入モード時のみデモが流れる
						if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
							@ステージ開始時処理（デモ表示） \
							d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
							d:wn_ＥＦ連絡橋 \
//							w21a_paddemo_00 d:ALL_OFF 53500 $i:プレイヤー位置Ｙ -45000 0 d:FA_END_STAND
							w21a_paddemo_00 d:ALL_OFF 53500 1066 -45000 0 d:FA_END_STAND
						} else {
							@ステージ開始時処理（ステージ名表示） \
							d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
							d:wn_ＥＦ連絡橋	
						}
					} else {
						@ステージ開始時処理（ステージ名表示） \
						d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
						d:wn_ＥＦ連絡橋
					}
				
				}

	//	それ以外
	#else
				// Ｆ脚から
				if ($f:w19a_双眼鏡兵遭遇デモフラグ == d:FALSE) {
					//	双眼鏡兵遭遇デモフラグを立てる（危険モード時はデモは二度と見ない）
					eval ($f:w19a_双眼鏡兵遭遇デモフラグ = d:TRUE)
					//	潜入モード時のみデモが流れる
					if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
//	PAL版のみ
#ifdef d:PAL
						@ステージ開始時処理（デモ表示） \
						d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
						d:wn_ＥＦ連絡橋 \
						w21a_paddemo_00_pal d:ALL_OFF 53500 1066 -45000 0 d:FA_END_STAND
//	PAL版以外
#else
						@ステージ開始時処理（デモ表示） \
						d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
						d:wn_ＥＦ連絡橋 \
						w21a_paddemo_00 d:ALL_OFF 53500 1066 -45000 0 d:FA_END_STAND
#endif
					} else {
						@ステージ開始時処理（ステージ名表示） \
						d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
						d:wn_ＥＦ連絡橋	
					}
				} else {
					@ステージ開始時処理（ステージ名表示） \
					d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
					d:wn_ＥＦ連絡橋
				}
	#endif

			} else if( $s:登場ポイント == sp_w24a_w21a_0 ) {
				// 中央棟から
				@ステージ開始時処理（ステージ名表示） \
				d:LOAD_TYPE:SL_DOOR_TYPE 中央棟ドア 2500 d:DEFAULT_M_TYPE \
				d:wn_ＥＦ連絡橋
			}
		}
	}

	proc スタートカメラセット {
		//	Ｅ脚
		if( $s:登場ポイント == sp_w20a0_w21a_0 ) {
#if 0
			//	なし
			chara カメラ設定 ロードカメラ \
				-c 1 \
				-p 58151,3935,-74901 \
				-t 60229,1592,-75634 \
				-a 200 \
				-i 0 0 0 0 \
				-s 1
#endif
		//	Ｆ脚
		} else if( $s:登場ポイント == sp_w22a0g_w21a_0 ) {
#if 0
			//	なし
			chara カメラ設定 ロードカメラ \
				-c 1 \
				-p 54726,4201,-44976 \
				-t 57412,1679,-44355 \
				-a 200 \
				-i 0 0 0 0 \
				-s 1
#endif
		//	シェル１中央棟
		} else if( $s:登場ポイント == sp_w24a_w21a_0 ) {
			chara カメラ設定 ロードカメラ \
				-c 1 \
				-p 20563,2851,-53226 \
				-t 20120,718,-55066 \
				-a 200 \
				-i 0 1 0 0 \
				-s 1
		}
	}
	
	proc スタートカメラオフ {
		chara カメラ設定 ロードカメラ \
			-s -1
	}


//###################################################################################
//									ロード設定
//###################################################################################

	proc ロードプロック {
		#if d:DEBUG_PRINT 
			print 'load_proc'
		#endif

		if ($s:呼び出しプロック == mv_w21a_w20a0_0) {
			// Ｅ脚１Ｆへ
			@mv_w21a_w20a0_0
		} else if ($s:呼び出しプロック == mv_w21a_w22a0g_0) {
			// Ｆ脚１Ｆへ
			@mv_w21a_w22a0g_0
		} else if ($s:呼び出しプロック == mv_w21a_w24a_0) {
			// 中央棟へ
			@mv_w21a_w24a_0
		}
	}

	proc ロード設定 {
		#if d:DEBUG_PRINT 
			print 'load_set'
		#endif

		trap ld001 d:PLAYER \
			-mask 入る \
			-exec {
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
/*
					//	双眼鏡兵を復活させるためにフラグを寝かす
					if ($f:w20b_２Ｆ敵兵死亡フラグ == d:TRUE) {
						eval ($f:w20b_２Ｆ敵兵死亡フラグ = d:FALSE)
					}
*/
					//	双眼鏡兵の状態を記憶する
					@Ｗ２１Ａ敵兵状態保存 敵兵:01

					// Ｅ脚１Ｆへ
					@ステージ終了時処理（ステージ名表示） \
					60250 0 -78000 2048 mv_w21a_w20a0_0 d:DEFAULT_M_TYPE \
					d:wn_Ｅ脚集配場１Ｆ
					//	ロード時カメラ
					@Ｅ脚ロード時カメラ設定
					//	先読み
					preseek 'w20a'
				}
			}

		trap ld003 d:PLAYER \
			-mask 入る \
			-exec {
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
/*
					//	双眼鏡兵を復活させるためにフラグを寝かす
					if ($f:w20b_２Ｆ敵兵死亡フラグ == d:TRUE) {
						eval ($f:w20b_２Ｆ敵兵死亡フラグ = d:FALSE)
					}
*/
					//	双眼鏡兵の状態を記憶する
					@Ｗ２１Ａ敵兵状態保存 敵兵:01

					// Ｆ脚１Ｆへ
					@ステージ終了時処理（ステージ名表示） \
					57750 0 -42000 0 mv_w21a_w22a0g_0 d:DEFAULT_M_TYPE \
					d:wn_Ｆ脚倉庫１Ｆ
					//	ロード時カメラ
					@Ｆ脚ロード時カメラ設定
					//	先読み
					preseek 'w22a'
				}
			}

		trap ld004 d:PLAYER \
			-mask 入る \
			-exec {
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
/*
					//	双眼鏡兵を復活させるためにフラグを寝かす
					if ($f:w20b_２Ｆ敵兵死亡フラグ == d:TRUE) {
						eval ($f:w20b_２Ｆ敵兵死亡フラグ = d:FALSE)
					}
*/
					//	双眼鏡兵の状態を記憶する
					@Ｗ２１Ａ敵兵状態保存 敵兵:01

					//	カネヨシくん用
					if( (!$f:rfp_スネークの遺体の話聞いた) && \
						($w:p_story < d:ST:P037_01_R01スネークの遺体１無線デモ１開始) && \
						($w:アラートモード != d:ALERT_MODE_SENNYU ) ){
							mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_FOUTS_S
					}
					//	夕方は中央棟には敵はいない
					#if d:W21B
						if ($w:アラートモード != d:ALERT_MODE_SENNYU) {
							command セットサウンドコード -c d:SNG_FOUTS_S
						}
					#endif
					// 中央棟へ
					@ステージ終了時処理（ステージ名表示） \
					17750 -1500 -55000 3072 mv_w21a_w24a_0 d:DEFAULT_M_TYPE \
					d:wn_シェル１中央棟１Ｆ
					//	ロード時カメラ
					@中央棟ロード時カメラ設定
					//	先読み
					preseek 'w24a'
				}
			}
	}

	proc Ｅ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 58151,3935,-74901 \
						-t 60229,1592,-75634 \
						-a 200 \
						-i 0 0 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc Ｆ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 54726,4201,-44976 \
						-t 57412,1679,-44355 \
						-a 200 \
						-i 0 0 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc 中央棟ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 20563,2851,-53226 \
						-t 20120,718,-55066 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}

	proc カメラ戻し管理 {
		command パッドチェック \
			-no_use
		if ($status != 1) {
			chara カメラ設定 ロードカメラ \
				-s -1
			mesg プロック連続実行 カメラ戻し管理人 kill
		}
	}



//###################################################################################
//									オブジェクト
//###################################################################################

	proc オブジェクト設定 {

		chara プットオブジェ シェル１中央棟ロードエリア \
			-m 	d:KMS_NAME11 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ シェル１中央棟ロードエリア off

		chara プットオブジェ Ｅ脚ロードエリア \
			-m 	d:KMS_NAME19 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｅ脚ロードエリア off

		chara プットオブジェ Ｆ脚ロードエリア \
			-m 	d:KMS_NAME20 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｆ脚ロードエリア off
	}

	proc 水位設定 {
		command 水位設定 -lv -40000
	}

	proc 海面設定 {
		#if d:DEBUG_PRINT
			print 'w21a_sea_set'
		#endif

	#if d:W21A
		//	昼
		chara プラント海面 seaseasea \
			-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
//			-pos 52000,-40000,-60000
			-pos 62000,-40000,-60000
	#else
		//	夕
		chara プラント海面 seaseasea \
			-tex oil e_oil_sea_alp_ovl_mod0222 e_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
			-pos 52000,-40000,-60000

/*
		chara 拡張海表示 sea_serface_ex \
			-t	e_oil_sea_alp_ovl_mod0222 \
			-lines 1 \
			-pos	52000,-40000,-60000
*/
		chara 拡張海表示皿型 sea_serface_ex \
			-tex entyou_oil_sea \
			-pos 52000,-40000,-60000 \
			-color 128 224 224	\			/*色合い調整 省略可*/
			-lines	 0			\			/* ライン数 何ラインの拡張海に合わせるか */
			-radius  270000 330000 \		/* 内側：270000+60000*lines を目安に */
			-hight   15000		\		/*皿状のトップの高さ*/
			-devide  10					/* MAX:59  */

	#endif

		chara 水面監視水飛沫 落下水飛沫 \
			-n d:PLAYER \
			-n 敵兵
		command 弾水飛沫処理
	}

	proc 空設定 {
		#if d:DEBUG_PRINT
			print 'w21a_sky_set'
		#endif

	#if d:W21A

//		@昼空	52000 -40000 -60000 -2000 -4000 2200
		@昼空（レンズフレア指定）	52000 -30000 -60000 -2000 -4000 2200 1

	#else

//		@夕空	52000 -40000 -60000 -2000 -4000 2200
		@Ｗ２１Ｂ夕空（レンズフレア指定） 52000 -40000 -60000 -2834 -499 -4089 1

	#endif
	}

	proc 天井設定 {
		#if d:DEBUG_PRINT
			print 'hel_set'
		#endif

		chara 天井君 Ｅ脚天井 -m d:KMS_NAME5 -p 0,0,0  -l d:LT2_NAME -c 0,0,-74000 -a 4
		chara 天井君 ヘリポート -m d:KMS_NAME9 -p 0,0,0  -l d:LT2_NAME -c 0,0,-74000 -a 4
		chara 天井君 Ｆ脚天井 -m d:KMS_NAME10 -p 0,0,0  -l d:LT2_NAME -c 0,0,-46000 -a 5


	}


//###################################################################################
//									罠設定
//###################################################################################

	//	クレイモア
	proc クレイモア終了１ {
		print 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA'
		print $1
		if ($1 != 0 ) {
		print 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA'
			eval ($f:w21a_クレイモア１取得フラグ = d:TRUE)
		}
	}
/*
	proc クレイモア終了２ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア２取得フラグ = d:TRUE)
		}
	}
*/
	proc クレイモア終了３ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア３取得フラグ = d:TRUE)
		}
	}
	proc クレイモア終了４ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア４取得フラグ = d:TRUE)
		}
	}
/*
	proc クレイモア終了５ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア５取得フラグ = d:TRUE)
		}
	}
*/
	proc クレイモア終了６ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア６取得フラグ = d:TRUE)
		}
	}
/*
	proc クレイモア終了７ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア７取得フラグ = d:TRUE)
		}
	}
	proc クレイモア終了８ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア８取得フラグ = d:TRUE)
		}
	}
*/
	proc クレイモア終了９ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア９取得フラグ = d:TRUE)
		}
	}
	proc クレイモア終了１０ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア１０取得フラグ = d:TRUE)
		}
	}
	proc クレイモア終了１１ {
		if ($1 != 0 ) {
			eval ($f:w21a_クレイモア１１取得フラグ = d:TRUE)
		}
	}

	proc クレイモア設定 {
		#if d:DEBUG_PRINT 
			print 'craymore_set'
		#endif

//	お昼しかない
#if d:W21A	
	//	北左から
		//	トップ
		if ($f:w21a_クレイモア１取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷１ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position 49000,0,-65000 \
				-rotate 0,2048,0 \
				-proc クレイモア終了１
		}

/*
		if ($f:w21a_クレイモア２取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷２ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position 55000,0,-65000 \
				-rotate 0,2048,0 \
				-proc クレイモア終了２
		}
*/

		//	トップ下
		if ($f:w21a_クレイモア３取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷３ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position 52000,0,-62000 \
				-rotate 0,2048,0 \
				-proc クレイモア終了３
		}

		//	左サイド
		if ($f:w21a_クレイモア４取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷４ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
//				-position 49000,0,-61000 \
				-position 49000,0,-62000 \
				-rotate 0,2048,0 \
				-proc クレイモア終了４
		}


/*
		//	右サイド
		if ($f:w21a_クレイモア５取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷５ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position 55000,0,-61000 \
				-rotate 0,2048,0 \
				-proc クレイモア終了５
		}
*/


		//	ボランチ
		if ($f:w21a_クレイモア６取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷６ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position  50500,0,-60000 \
				-rotate 0,0,0 \
				-proc クレイモア終了６
		}


/*
		if ($f:w21a_クレイモア７取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷７ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position  53500,0,-60000 \
				-rotate 0,0,0 \
				-proc クレイモア終了７
		}
*/

		//	最終ライン
/*
		if ($f:w21a_クレイモア８取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷８ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position  49000,0,-58000 \
				-rotate 0,0,0 \
				-proc クレイモア終了８
		}
*/
		if ($f:w21a_クレイモア９取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷９ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position  52000,0,-58000 \
				-rotate 0,0,0 \
				-proc クレイモア終了９
		}
		if ($f:w21a_クレイモア１０取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷１０ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
				-position  55000,0,-58000 \
				-rotate 0,0,0 \
				-proc クレイモア終了１０
		}

		//	キーパー
		if ($f:w21a_クレイモア１１取得フラグ == d:FALSE ) {
			chara 設置クレイモア 地雷１１ \
//				-f 0x0001 \		//	デバッグ用アタリ表示
//				-position  52000,0,-55000 \
				-position  52000,0,-55000 \
				-rotate 0,0,0 \
				-proc クレイモア終了１１
		}

#endif

	}


	//	落し穴
	proc 落し穴配置チェック {

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[0]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴１Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[1]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴１Ｂ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[2]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴２Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[3]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴２Ｂ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[4]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴３Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[5]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴３Ｂ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[6]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴４Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[7]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴４Ｂ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[8]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴５Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[9]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴５Ｂ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[10]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴６Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[11]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴６Ｂ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[12]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴７Ａ開いたフラグ = d:FALSE)
		}

		@壊れ物再セットチェック $b:w21a_破壊ロード回数[13]
		if ($f:再セットフラグ) {
			eval ($f:w21a_落し穴７Ｂ開いたフラグ = d:FALSE)
		}

	}

	proc 落し穴設定 {

	//	昼のとき
	#if d:W21A
	//	一番西側
		//	左
		if ($f:w21a_落し穴１Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴１Ａ \
				-position 25500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr4 w21a_trap_dammy1 \
				-hazard 25000,-1500,-60625 \
						26000,-1500,-60625 \
						26000,-1500,-59375 \
						25000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定１Ａ
		} else if ($f:w21a_落し穴１Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴１Ａ \
				-position 25500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr4 w21a_trap_dammy1 \
				-hazard 25000,-1500,-60625 \
						26000,-1500,-60625 \
						26000,-1500,-59375 \
						25000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定１Ａ
		}

		//	右
		if ($f:w21a_落し穴１Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴１Ｂ \
				-position 26500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr3 w21a_trap_dammy1 \
				-hazard 26000,-1500,-60625 \
						27000,-1500,-60625 \
						27000,-1500,-59375 \
						26000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定１Ｂ
		} else if ($f:w21a_落し穴１Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴１Ｂ \
				-position 26500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr3 w21a_trap_dammy1 \
				-hazard 26000,-1500,-60625 \
						27000,-1500,-60625 \
						27000,-1500,-59375 \
						26000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定１Ｂ
		}


	//	西側から
		//	左
		if ($f:w21a_落し穴２Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴２Ａ \
				-position 29500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 29000,-1500,-60625 \
						30000,-1500,-60625 \
						30000,-1500,-59375 \
						29000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定２Ａ
		} else if ($f:w21a_落し穴２Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴２Ａ \
				-position 29500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 29000,-1500,-60625 \
						30000,-1500,-60625 \
						30000,-1500,-59375 \
						29000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定２Ａ
		}

		//	右
		if ($f:w21a_落し穴２Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴２Ｂ \
				-position 30500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 30000,-1500,-60625 \
						31000,-1500,-60625 \
						31000,-1500,-59375 \
						30000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定２Ｂ
		} else if ($f:w21a_落し穴２Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴２Ｂ \
				-position 30500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 30000,-1500,-60625 \
						31000,-1500,-60625 \
						31000,-1500,-59375 \
						30000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定２Ｂ
		}

		//	左
		if ($f:w21a_落し穴３Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴３Ａ \
				-position 31500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 31000,-1500,-60625 \
						32000,-1500,-60625 \
						32000,-1500,-59375 \
						31000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定３Ａ
		} else if ($f:w21a_落し穴３Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴３Ａ \
				-position 31500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 31000,-1500,-60625 \
						32000,-1500,-60625 \
						32000,-1500,-59375 \
						31000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定３Ａ
		}

		//	右
		if ($f:w21a_落し穴３Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴３Ｂ \
				-position 32500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 32000,-1500,-60625 \
						33000,-1500,-60625 \
						33000,-1500,-59375 \
						32000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定３Ｂ
		} else if ($f:w21a_落し穴３Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴３Ｂ \
				-position 32500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 32000,-1500,-60625 \
						33000,-1500,-60625 \
						33000,-1500,-59375 \
						32000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定３Ｂ
		}

		//	左
		if ($f:w21a_落し穴４Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴４Ａ \
				-position 33500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 33000,-1500,-60625 \
						34000,-1500,-60625 \
						34000,-1500,-59375 \
						33000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定４Ａ
		} else if ($f:w21a_落し穴４Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴４Ａ \
				-position 33500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 33000,-1500,-60625 \
						34000,-1500,-60625 \
						34000,-1500,-59375 \
						33000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定４Ａ
		}

		//	右
		if ($f:w21a_落し穴４Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴４Ｂ \
				-position 34500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 34000,-1500,-60625 \
						35000,-1500,-60625 \
						35000,-1500,-59375 \
						34000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定４Ｂ
		} else if ($f:w21a_落し穴４Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴４Ｂ \
				-position 34500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 34000,-1500,-60625 \
						35000,-1500,-60625 \
						35000,-1500,-59375 \
						34000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-status 1 \
				-se_pattern 1 \
				-proc 落し穴落下設定４Ｂ
		}


	//	間をはさんで
		//	左
		if ($f:w21a_落し穴５Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴５Ａ \
				-position 36500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 36000,-1500,-60625 \
						37000,-1500,-60625 \
						37000,-1500,-59375 \
						36000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定５Ａ
		} else if ($f:w21a_落し穴５Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴５Ａ \
				-position 36500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 36000,-1500,-60625 \
						37000,-1500,-60625 \
						37000,-1500,-59375 \
						36000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定５Ａ
		}

		//	右
		if ($f:w21a_落し穴５Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴５Ｂ \
				-position 37500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 37000,-1500,-60625 \
						38000,-1500,-60625 \
						38000,-1500,-59375 \
						37000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定５Ｂ
		} else if ($f:w21a_落し穴５Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴５Ｂ \
				-position 37500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 37000,-1500,-60625 \
						38000,-1500,-60625 \
						38000,-1500,-59375 \
						37000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定５Ｂ
		}

		//	左
		if ($f:w21a_落し穴６Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴６Ａ \
				-position 38500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 38000,-1500,-60625 \
						39000,-1500,-60625 \
						39000,-1500,-59375 \
						38000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定６Ａ
		} else if ($f:w21a_落し穴６Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴６Ａ \
				-position 38500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 38000,-1500,-60625 \
						39000,-1500,-60625 \
						39000,-1500,-59375 \
						38000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定６Ａ
		}

		//	右
		if ($f:w21a_落し穴６Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴６Ｂ \
				-position 39500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 39000,-1500,-60625 \
						40000,-1500,-60625 \
						40000,-1500,-59375 \
						39000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定６Ｂ
		} else if ($f:w21a_落し穴６Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴６Ｂ \
				-position 39500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 39000,-1500,-60625 \
						40000,-1500,-60625 \
						40000,-1500,-59375 \
						39000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定６Ｂ
		}

		//	左
		if ($f:w21a_落し穴７Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴７Ａ \
				-position 40500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 40000,-1500,-60625 \
						41000,-1500,-60625 \
						41000,-1500,-59375 \
						40000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定７Ａ
		} else if ($f:w21a_落し穴７Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴７Ａ \
				-position 40500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr2 w21a_trap_dammy1 \
				-hazard 40000,-1500,-60625 \
						41000,-1500,-60625 \
						41000,-1500,-59375 \
						40000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定７Ａ
		}

		//	右
		if ($f:w21a_落し穴７Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴７Ｂ \
				-position 41500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 41000,-1500,-60625 \
						42000,-1500,-60625 \
						42000,-1500,-59375 \
						41000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定７Ｂ
		} else if ($f:w21a_落し穴７Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴７Ｂ \
				-position 41500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21a_trap_dr1 w21a_trap_dammy1 \
				-hazard 41000,-1500,-60625 \
						42000,-1500,-60625 \
						42000,-1500,-59375 \
						41000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定７Ｂ
		}


	//	夕方のとき
	#else
	//	一番西側
		//	左
		if ($f:w21a_落し穴１Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴１Ａ \
				-position 25500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 25000,-1500,-60625 \
						26000,-1500,-60625 \
						26000,-1500,-59375 \
						25000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定１Ａ
		} else if ($f:w21a_落し穴１Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴１Ａ \
				-position 25500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 25000,-1500,-60625 \
						26000,-1500,-60625 \
						26000,-1500,-59375 \
						25000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定１Ａ
		}

		//	右
		if ($f:w21a_落し穴１Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴１Ｂ \
				-position 26500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 26000,-1500,-60625 \
						27000,-1500,-60625 \
						27000,-1500,-59375 \
						26000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定１Ｂ
		} else if ($f:w21a_落し穴１Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴１Ｂ \
				-position 26500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 26000,-1500,-60625 \
						27000,-1500,-60625 \
						27000,-1500,-59375 \
						26000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定１Ｂ
		}


	//	西側から
		//	左
		if ($f:w21a_落し穴２Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴２Ａ \
				-position 29500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 29000,-1500,-60625 \
						30000,-1500,-60625 \
						30000,-1500,-59375 \
						29000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定２Ａ
		} else if ($f:w21a_落し穴２Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴２Ａ \
				-position 29500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 29000,-1500,-60625 \
						30000,-1500,-60625 \
						30000,-1500,-59375 \
						29000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定２Ａ
		}
		
		//	右
		if ($f:w21a_落し穴２Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴２Ｂ \
				-position 30500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 30000,-1500,-60625 \
						31000,-1500,-60625 \
						31000,-1500,-59375 \
						30000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定２Ｂ
		} else if ($f:w21a_落し穴２Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴２Ｂ \
				-position 30500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 30000,-1500,-60625 \
						31000,-1500,-60625 \
						31000,-1500,-59375 \
						30000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-status 1 \
				-se_pattern 1 \
				-proc 落し穴落下設定２Ｂ
		}

		//	左
		if ($f:w21a_落し穴３Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴３Ａ \
				-position 31500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 31000,-1500,-60625 \
						32000,-1500,-60625 \
						32000,-1500,-59375 \
						31000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定３Ａ
		} else if ($f:w21a_落し穴３Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴３Ａ \
				-position 31500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 31000,-1500,-60625 \
						32000,-1500,-60625 \
						32000,-1500,-59375 \
						31000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定３Ａ
		}

		//	右
		if ($f:w21a_落し穴３Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴３Ｂ \
				-position 32500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 32000,-1500,-60625 \
						33000,-1500,-60625 \
						33000,-1500,-59375 \
						32000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定３Ｂ
		} else if ($f:w21a_落し穴３Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴３Ｂ \
				-position 32500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 32000,-1500,-60625 \
						33000,-1500,-60625 \
						33000,-1500,-59375 \
						32000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-status 1 \
				-se_pattern 1 \
				-proc 落し穴落下設定３Ｂ
		}

		//	左
		if ($f:w21a_落し穴４Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴４Ａ \
				-position 33500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 33000,-1500,-60625 \
						34000,-1500,-60625 \
						34000,-1500,-59375 \
						33000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定４Ａ
		} else if ($f:w21a_落し穴４Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴４Ａ \
				-position 33500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 33000,-1500,-60625 \
						34000,-1500,-60625 \
						34000,-1500,-59375 \
						33000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定４Ａ
		} 

		//	右
		if ($f:w21a_落し穴４Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴４Ｂ \
				-position 34500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 34000,-1500,-60625 \
						35000,-1500,-60625 \
						35000,-1500,-59375 \
						34000,-1500,-59375 \
				-fall_time 30 \
				-se_pattern 1 \
				-fall_pattern 4 \
				-proc 落し穴落下設定４Ｂ
		} else if ($f:w21a_落し穴４Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴４Ｂ \
				-position 34500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 34000,-1500,-60625 \
						35000,-1500,-60625 \
						35000,-1500,-59375 \
						34000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定４Ｂ
		}


	//	間をはさんで
		//	左
		if ($f:w21a_落し穴５Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴５Ａ \
				-position 36500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 36000,-1500,-60625 \
						37000,-1500,-60625 \
						37000,-1500,-59375 \
						36000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定５Ａ
		} else if ($f:w21a_落し穴５Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴５Ａ \
				-position 36500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 36000,-1500,-60625 \
						37000,-1500,-60625 \
						37000,-1500,-59375 \
						36000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定５Ａ
		}

		//	右
		if ($f:w21a_落し穴５Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴５Ｂ \
				-position 37500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 37000,-1500,-60625 \
						38000,-1500,-60625 \
						38000,-1500,-59375 \
						37000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定５Ｂ
		} else if ($f:w21a_落し穴５Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴５Ｂ \
				-position 37500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 37000,-1500,-60625 \
						38000,-1500,-60625 \
						38000,-1500,-59375 \
						37000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定５Ｂ
		}

		//	左
		if ($f:w21a_落し穴６Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴６Ａ \
				-position 38500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 38000,-1500,-60625 \
						39000,-1500,-60625 \
						39000,-1500,-59375 \
						38000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定６Ａ
		} else if ($f:w21a_落し穴６Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴６Ａ \
				-position 38500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 38000,-1500,-60625 \
						39000,-1500,-60625 \
						39000,-1500,-59375 \
						38000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定６Ａ
		}

		//	右
		if ($f:w21a_落し穴６Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴６Ｂ \
				-position 39500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 39000,-1500,-60625 \
						40000,-1500,-60625 \
						40000,-1500,-59375 \
						39000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定６Ｂ
		} else if ($f:w21a_落し穴６Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴６Ｂ \
				-position 39500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 39000,-1500,-60625 \
						40000,-1500,-60625 \
						40000,-1500,-59375 \
						39000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-status 1 \
				-se_pattern 1 \
				-proc 落し穴落下設定６Ｂ
		}

		//	左
		if ($f:w21a_落し穴７Ａ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴７Ａ \
				-position 40500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 40000,-1500,-60625 \
						41000,-1500,-60625 \
						41000,-1500,-59375 \
						40000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-proc 落し穴落下設定７Ａ
		} else if ($f:w21a_落し穴７Ａ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴７Ａ \
				-position 40500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr2 w21a_trap_dammy1 \
				-hazard 40000,-1500,-60625 \
						41000,-1500,-60625 \
						41000,-1500,-59375 \
						40000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 7 \
				-status 1 \
				-proc 落し穴落下設定７Ａ
		}

		//	右
		if ($f:w21a_落し穴７Ｂ開いたフラグ == d:FALSE) {
			chara 崩落床 落し穴７Ｂ \
				-position 41500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 41000,-1500,-60625 \
						42000,-1500,-60625 \
						42000,-1500,-59375 \
						41000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-proc 落し穴落下設定７Ｂ
		} else if ($f:w21a_落し穴７Ｂ開いたフラグ == d:TRUE) {
			chara 崩落床 落し穴７Ｂ \
				-position 41500,-1500,-60000 \
				-rotate 0,0,0 \
				-map ＥＦ連絡橋 \
				-model w21b_trap_dr1 w21a_trap_dammy1 \
				-hazard 41000,-1500,-60625 \
						42000,-1500,-60625 \
						42000,-1500,-59375 \
						41000,-1500,-59375 \
				-fall_time 30 \
				-fall_pattern 4 \
				-se_pattern 1 \
				-status 1 \
				-proc 落し穴落下設定７Ｂ
		}


	#endif



	//	落とし穴上存在判定

	//	一番西側
		//	左
		trap pi001 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴１Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴１Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴１Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[0] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴１Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴１Ａ fall
					}
				}
			}
		//	右
		trap pi002 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴１Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴１Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴１Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[1] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴１Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴１Ｂ fall
					}
				}
			}


	//	西側から
		//	左
		trap pi003 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴２Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴２Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴２Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[2] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴２Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴２Ａ fall
					}
				}
			}
		//	右
		trap pi004 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴２Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴２Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴２Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[3] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴２Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴２Ｂ fall
					}
				}
			}


		//	左
		trap pi005 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴３Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴３Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴３Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[4] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴３Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴３Ａ fall
					}
				}
			}
		//	右
		trap pi006 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴３Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴３Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴３Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[5] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴３Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴３Ｂ fall
					}
				}
			}


		//	左
		trap pi007 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴４Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴４Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴４Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[6] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴４Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴４Ａ fall
					}
				}
			}
		//	右
		trap pi008 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴４Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴４Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴４Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[7] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴４Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴４Ｂ fall
					}
				}
			}


	//	間をはさんで
		//	左
		trap pi009 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴５Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴５Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴５Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[8] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴５Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴５Ａ fall
					}
				}
			}
		//	右
		trap pi010 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴５Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴５Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴５Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[9] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴５Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴５Ｂ fall
					}
				}
			}


		//	左
		trap pi011 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴６Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴６Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴６Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[10] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴６Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴６Ａ fall
					}
				}
			}
		//	右
		trap pi012 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴６Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴６Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴６Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[11] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴６Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴６Ｂ fall
					}
				}
			}


		//	左
		trap pi013 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴７Ａ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴７Ａ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴７Ａ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[12] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴７Ａ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴７Ａ fall
					}
				}
			}
		//	右
		trap pi014 d:PLAYER \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					//	落し穴上にいるときは落し穴上フラグを立てる
//					eval ($f:w21a_落し穴７Ｂ上フラグ = d:TRUE)
				} else if ($3 == 出る) {
					//	落し穴上から出るときは落し穴上フラグをおろす
//					eval( $f:w21a_落し穴７Ｂ上フラグ = d:FALSE)

					//	床の状態を取得
					command 崩落床状態取得 \
					-name 落し穴７Ｂ
					//	床を踏んでいたら床を落とす
					if ( $status == 1) {
						eval ($b:w21a_破壊ロード回数[13] = $w:グローバルロード回数)
						eval ($f:w21a_落し穴７Ｂ開いたフラグ = d:TRUE)
						mesg 崩落床 落し穴７Ｂ fall
					}
				}
			}

	}

//	落下死設定
	//	一番西側
	//	左
	proc 落し穴落下設定１Ａ {
		mesg プレイヤー d:PLAYER position 25500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定１Ｂ {
		mesg プレイヤー d:PLAYER position 26500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}


	//	西側から
	//	左
	proc 落し穴落下設定２Ａ {
		mesg プレイヤー d:PLAYER position 29500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定２Ｂ {
		mesg プレイヤー d:PLAYER position 30500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}


	//	左
	proc 落し穴落下設定３Ａ {
		mesg プレイヤー d:PLAYER position 31500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定３Ｂ {
		mesg プレイヤー d:PLAYER position 32500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}


	//	左
	proc 落し穴落下設定４Ａ {
		mesg プレイヤー d:PLAYER position 33500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定４Ｂ {
		mesg プレイヤー d:PLAYER position 34500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}


	//	間をはさんで
	//	左
	proc 落し穴落下設定５Ａ {
		mesg プレイヤー d:PLAYER position 36500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定５Ｂ {
		mesg プレイヤー d:PLAYER position 37500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}


	//	左
	proc 落し穴落下設定６Ａ {
		mesg プレイヤー d:PLAYER position 38500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定６Ｂ {
		mesg プレイヤー d:PLAYER position 39500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}


	//	左
	proc 落し穴落下設定７Ａ {
		mesg プレイヤー d:PLAYER position 40500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}
	//	右
	proc 落し穴落下設定７Ｂ {
		mesg プレイヤー d:PLAYER position 41500,$i:プレイヤー位置Ｙ,-60000
		@ジャンプ落下死発動 0x0004 -41000
	}



//###################################################################################
//									アイテム
//###################################################################################

	proc アイテム設定 {
		#if d:DEBUG_PRINT
			print 'item_set'
		#endif

		@アイテム再セットチェック $b:w21a_取得ロード回数[0]
		if ($f:再セットフラグ) {
			@弾薬_ＡＫＳ_初期設置 ＡＫ弾:01 23000 -1500 -61250 0		//	Ｂ１東側
		}

		//	夕方のみ
		#if d:W21B
			@アイテム再セットチェック $b:w21a_取得ロード回数[1]
			if ($f:再セットフラグ) {
				@チャフ_初期設置 チャフ:01 58250 0 -45250 0				//	Ｆ脚通路
			}
		#endif

	}

	proc アイテム取得時番号設定 $:ボックス名 {
		#if d:DEBUG_PRINT
		print '*** Item No Set ***'
		#endif

		if($:ボックス名 == ＡＫ弾:01) {
			eval($b:w21a_取得ロード回数[0] = $w:グローバルロード回数);
		} else if($:ボックス名 == チャフ:01) {
			eval($b:w21a_取得ロード回数[1] = $w:グローバルロード回数);
		}

	}




//###################################################################################
//									 エフェクト
//###################################################################################

	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif


	//	昼の場合
	#if d:W21A
		@明るさ可変機能 220 237 198 -15000 175000 ha002 -1500


	//	夕方の場合
	#else
		@明るさ可変機能 153 95 50 -60000 320000 ha002 -1500

	#endif

	}



	proc エフェクト設定 {
		#if d:DEBUG_PRINT
			print 'effect_set'
		#endif

		#if d:BLUR
			chara ブラー ブラー
		#endif

		#if d:Z_MANAGER
			chara Ｚフォーカス管理 Z_manager
			mesg Ｚフォーカス管理 Z_manager パラメータ 5000 30
		#endif

		#if d:BODY_SHADOW
			chara ＯＦＦ制御 off_man
			mesg ＯＦＦ制御 off_man ボディシャドウＯＦＦ
		#endif
	}

	// キャラ用光源(マップの設置が終わってから設定することで、キャラだけに光を当てる)
	proc キャラ用ライト設定 {
		#if d:DEBUG_PRINT
			print 'chara_light_set'
		#endif

	//	昼の場合
	#if d:W21A
		chara 環境光 環境光源 \
			-c 55 63 38

		chara 平行光 平行光源 \
			-c 163 164 150 \
			-p -2000,-4000,2200

	//	夕の場合
	#else
		chara 環境光 環境光源 \
			-c 40 40 30

		chara 平行光 平行光源 \
			-c 102 122 86 \
//			-p -2000,-4000,2200
			-p -2834 -499 -4089
	#endif
	}

	proc ＥＦ連絡橋風設定 {
		@風設定 ca004
	}


//###################################################################################
//									カメラ
//###################################################################################

	proc カメラセット {
		#if d:DEBUG_PRINT
			print 'stage_camera_set'
		#endif

		chara カメラ ステージカメラ		-c 0 -l 3 -p 0
		chara カメラ 通路カメラ			-c 0 -l 2 -p 50
		chara カメラ 小部屋カメラ		-c 0 -l 2 -p 40
		chara カメラ ビヨンドカメラ		-c 0 -l 2 -p 20
		chara カメラ 張り付きカメラ		-c 0 -l 2 -p 10
		chara カメラ シナリオカメラ		-c 0 -l 0 -p 6
		chara カメラ ロードカメラ		-c 0 -l 1 -p 5
		chara カメラ ゲームオーバーカメラ	-c 0 -l 1 -p 4

		// デフォルトカメラ
		chara カメラ設定 ステージカメラ \
			-c 0 \	
			-b 22500,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-46525 \
			-l 22500,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-47525 \
			-r 450,2048,0 \
			-f 8000 \
			-a 200 \
			-i 2 -1 0 0 \
			-s 1
	}

	proc ＥＦ連絡橋カメラ設定 {
		#if d:DEBUG_PRINT
			print 'w21a_camera_set'
		#endif

		// Ｅ脚側通路
/*
		trap ca001 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \	
					-b 51444,6000,-74708 58250,6250,-74472 \
					-l 51444,d:CAM_MIN,-76027 58250,d:CAM_MAX,-75791 \
					-r 850,2048,0 \
					-f 5000 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}
*/
		trap ca001 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \
					-b  51202,d:CAM_MIN,-74988  59942, 5969,-74663 \
					-l  51202,d:CAM_MIN,-75783  59942, 1347,-75457 \
					-r 913,2048,0 \
					-f 4689 \
					-a 200 \
					-i 0 -1 0 0 \
					-s $3
			}

		// Ｆ脚側通路
/*
		trap ca002 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \	
					-b 45359,6000,-45145 58815,6250,-43799 \
					-l 45359,d:CAM_MIN,-45145 58815,d:CAM_MAX,-45000 \
					-r 850,2048,0 \
					-f 5000 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}
*/
		trap ca002 d:PLAYER \
			-camera \
			-mask ？ \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \
//					-b  54189,d:CAM_MIN,-44568  51202, 5969,-44568 \
//			 		-l  54189,d:CAM_MIN,-45351  51202, 1347,-44768 \
					-b  51202,d:CAM_MIN,-44568  57842, 5969,-44568 \
			 		-l  51202,d:CAM_MIN,-45351  57842, 1347,-44768 \
					-r 913,2048,0 \
					-f 4689 \
					-a 200 \
					-i 0 -1 0 0 \
					-s $3
			}





		// 中央棟側通路
/*
		trap ca003 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \	
					-b 20500,d:CAM_MIN,-59000 20750,2500,-54000 \
					-l 20500,d:CAM_MIN,-59000 20750,d:CAM_MAX,-54050 \
					-r 250,2048,0 \
					-f 10000 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			}
*/
		trap ca003 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
//-p 20552,2439,-52626 -t 20552,763,-55822 \
//-p 20552,4123,-54326 -t 20552,580,-55008 \

					@CS_スムーズカメラ昇順 $6 -60000 -55000 20552 20552
					eval( $i:補完ＰＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 -60000 -55000 2439 4123
					eval( $i:補完ＰＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 -60000 -55000 -52626 -54326
					eval( $i:補完ＰＺ = $i:CS_結果１ );


					@CS_スムーズカメラ昇順 $6 -60000 -55000 20552 20552
					eval( $i:補完ＴＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 -60000 -55000 763 580
					eval( $i:補完ＴＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 -60000 -55000 -55822 -55008
					eval( $i:補完ＴＺ = $i:CS_結果１ );

				chara カメラ設定 小部屋カメラ \
					-c 1 \	
					-b 20500,d:CAM_MIN,-59000 20750,d:CAM_MAX,-54000 \
					-l 20500,d:CAM_MIN,-59000 20750,d:CAM_MAX,-54050 \
					-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
					-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
					-a 200 \
					-i 0 0 0 0 \
//					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためリミットを見る
					-s $3
			}

		//	橋全体
		trap ca004 d:PLAYER \
			-camera \
			-mask ＊ \
			-exec {
				if ($4 >47000 ) {
//					@CS_スムーズカメラ降順 $6 -46000 -60000 800 350
					@CS_スムーズカメラ降順 $6 -46000 -60000 650 350
					eval ($i:補完ＲＸ = $i:CS_結果１);

				} else {
//					@CS_スムーズカメラ降順 $6 -46000 -60000 800 350
					@CS_スムーズカメラ降順 $6 -46000 -60000 650 350
					eval ($i:補完ＲＸ１ = $i:CS_結果１)

					@CS_スムーズカメラ降順 $4 47000 43000 $i:補完ＲＸ１ 550
					eval( $i:補完ＲＸ = $i:CS_結果１ );
				}

				chara カメラ設定 通路カメラ \
					-c 0 \
					-b 26581,d:CAM_MIN,-56388 d:CAM_MAX,11919,-46804 \
					-l 26581,d:CAM_MIN,-67129 d:CAM_MAX,d:CAM_MAX,-49640 \

//					-b 26581,d:CAM_MIN,-56388 d:CAM_MAX,11551,-46804 \
//					-l 26581,d:CAM_MIN,-67129 d:CAM_MAX,496,-49640 \

					-r $i:補完ＲＸ,2048,0 \
					-f 12500 \
					-a 200 \
					-i 0 -1 0 0 \
					-s $3
			}



		//	張り付き移動
		//１Ｆ
		//	左側
		//	Ｆ脚側から
		trap bh001 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh002 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh003 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh004 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh005 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	右側
		//	Ｆ脚側から
		trap bh006 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh007 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh008 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh009 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
		trap bh010 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	Ｆ脚側東西通路
		trap bh011 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	Ｅ脚側東西通路
		trap bh012 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 0 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	西側エリア
		//北側
		trap bh101 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラＢ１ $3
			}
		//南側
		trap bh102 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラＢ１ $3
			}
		//北側
		trap bh103 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラＢ１ $3
			}
		//南側
		trap bh104 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラＢ１ $3
			}

	}




	proc 張り付き移動カメラ１Ｆ {
		chara カメラ設定 張り付きカメラ \
			-c 0 \
//-p 51249,9573,-47458 -t 51249,1622,-48340 \
//-p 52751,9574,-70679 -t 52751,1623,-71561 \
			-b d:CAM_MIN,d:CAM_MIN,-70679 d:CAM_MAX,d:CAM_MAX,-47458 \
			-l d:CAM_MIN,d:CAM_MIN,-71561 d:CAM_MAX,d:CAM_MAX,-48340 \
			-r 952,2048,0 -f 8000 \
			-a 200 \
			-i 2 2 0 0 \
			-s $1 \
			-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
	}

	proc 張り付き移動カメラＢ１ {
		chara カメラ設定 張り付きカメラ \
			-c 0 \
			-b 24262,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l 24262,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-r 961,2048,0 -f 8000 \
			-a 200 \
			-i 2 2 0 0 \
			-s $1 \
			-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
	}
















	proc ＥＦ連絡橋ビヨンド設定 {
		#if d:DEBUG_PRINT
			print 'w21a_beyond_set'
		#endif

	// 南北通路左上
		//	エルード発動
		trap by001 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@西向きビヨンドハイ発動 by001 48500 $6 -65750 -62250 0x00000002
			}
		//	発動カメラ
		trap by001 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,7930,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
					-r 615,1703,0 -f 8000 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by001 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 524,1343,0 -f 7998 \
					-a 369 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by001 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 952,2048,0 -f 8000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}

	// 南北通路右上
		//	エルード発動
		trap by002 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@東向きビヨンドハイ発動 by002 55500 $6 -65750 -62250 0x00000002
			}
		//	発動カメラ
		trap by002 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,7731,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
					-r 600,2527,0 -f 8000 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by002 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 510,2528,0 -f 7999 \
					-a 372 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by002 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 973,2048,0 -f 8000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}
	
	// 南北通路左下
		//	エルード発動
		trap by003 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@西向きビヨンドハイ発動 by003 48500 $6 -57750 -54250 0x00000002
			}
		//	発動カメラ
		trap by003 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,7930,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
					-r 615,1703,0 -f 8000 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by003 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 524,1343,0 -f 7998 \
					-a 369 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by003 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 952,2048,0 -f 8000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}

	// 南北通路右下
		//	エルード発動
		trap by004 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@東向きビヨンドハイ発動 by004 55500 $6 -57750 -54250 0x00000002
			}
		//	発動カメラ
		trap by004 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,7731,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
					-r 600,2527,0 -f 8000 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by004 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 510,2528,0 -f 7999 \
					-a 372 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by004 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 973,2048,0 -f 8000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}

	// 東西通路上
		//	エルード発動
		trap by005 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@北向きビヨンドハイ発動 by005 -61000 $4 28250 42750 0x00000002
			}
		//	発動カメラ
		trap by005 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,4446,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-181,d:CAM_MAX \
//					-r 537,2285,0 -f 6278 \
					-r 537,2285,0 -f 10000 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by005 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//					-r 965,2285,0 -f 6248 \
					-r 965,2285,0 -f 12000 \
					-a 247 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by005 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 961,2048,0 -f 8000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}

	// 東西通路下
		//	エルード発動
		trap by006 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@南向きビヨンドハイ発動 by006 -59000 $4 28250 42750 0x00000002
			}
		//	発動カメラ
		trap by006 d:PLAYER \
			-mask いる \
			-state 6 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,5848,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-181,d:CAM_MAX \
					-r 554,2348,0 -f 7999 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by006 d:PLAYER \
			-mask ？ \
			-state 5 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//					-r 650,2348,0 -f 6824 \
					-r 650,2348,0 -f 12000 \
					-a 303 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by006 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 961,2048,0 -f 8000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}

	}


//###################################################################################
//									シナリオデモ
//###################################################################################

//	双眼鏡兵遭遇デモ開始チェック
	proc 双眼鏡兵遭遇デモ開始チェック {
		if ($w:p_story >= d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) {
			eval($f:w19a_双眼鏡兵遭遇デモフラグ = d:TRUE)
		}
	}

//	双眼鏡兵遭遇デモ
	proc 双眼鏡兵遭遇デモ設定 {

	//	日本語版
	#ifdef d:JAPANESE
		//	ベリーイージーでは敵はいない
		if ($w:ゲーム設定 >= d:LEVEL_EASY) {
			trap sc001 d:PLAYER  \
				-mask 入る \
				-e {
					//	ＤＥ連絡橋で双眼鏡兵遭遇デモを見ていない場合
					if ($f:w19a_双眼鏡兵遭遇デモフラグ == d:FALSE) {
						//	双眼鏡兵遭遇デモフラグを立てる（危険モード時はデモは二度と見ない）
						eval ($f:w19a_双眼鏡兵遭遇デモフラグ = d:TRUE)

						//	潜入モード時のみデモが流れる
						if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
							@双眼鏡兵遭遇デモ
						}
					}
				}
		}

	//	それ以外
	#else
		trap sc001 d:PLAYER  \
			-mask 入る \
			-e {
				//	ＤＥ連絡橋で双眼鏡兵遭遇デモを見ていない場合
				if ($f:w19a_双眼鏡兵遭遇デモフラグ == d:FALSE) {
					//	双眼鏡兵遭遇デモフラグを立てる（危険モード時はデモは二度と見ない）
					eval ($f:w19a_双眼鏡兵遭遇デモフラグ = d:TRUE)

					//	潜入モード時のみデモが流れる
					if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
						@双眼鏡兵遭遇デモ
					}
				}
			}
	#endif
	}

	proc 双眼鏡兵遭遇デモ {
		eval ($s:登場ポイント = 双眼鏡兵遭遇デモ後)
		eval ($i:プレイヤー初期Ｘ位置 = 53500)
		eval ($i:プレイヤー初期Ｙ位置 = 0)
		eval ($i:プレイヤー初期Ｚ位置 = -45000)
		eval ($i:プレイヤー初期方向 =  0)
		eval ($b:プレイヤー初期姿勢 = d:FA_END_STAND)

		chara デバッグポーズ 双眼鏡兵遭遇デモポーズ 'glasses_soldier_demo' -p 双眼鏡兵遭遇デモ終了処理
	
	}

	proc 双眼鏡兵遭遇デモ終了処理 {
		restart -s
	}


	proc 双眼鏡兵監視人 {
		#if d:DEBUG_PRINT
//			print 'KANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHI'
		#endif

		//	ゲームステータス取得
		command ゲットゲームステータス $i:ゲームステータス
		//	敵兵状態取得
		command ゲット敵兵モード 敵兵:01 $i:w21a_敵兵モード

		//	潜入モードのとき
		if ($w:アラートモード == d:ALERT_MODE_SENNYU) {
			//	発見〜危険の間
			if ( ($i:ゲームステータス & d:STATE_DETECT) == 0 ) {
				//	双眼鏡兵がプレイヤーを発見したら
				if ($i:w21a_敵兵モード & d:ENE_NOTICE_DETECT) {
					#if d:DEBUG_PRINT
					print 'DISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVER'
					#endif

					eval ($f:w19a_双眼鏡兵発見デモフラグ = d:TRUE)
					command パッド操作 -release
					
					//	シナリオカメラ起動
					chara カメラ設定 シナリオカメラ \
						-c 1 \
						-p 39651,-2234,-46432 -t 50354,5577,-64288 \
						-a 1001 \
						-i 0 0 0 0 \
						-s 1

					//	シネマスクリーン起動
					mesg シネマスクリーン シネマ フェードアウト 1
					//	サイトを消す
					command サイト表示制御 \
						-switch off

					//	シナリオカメラを殺す
					chara delay シナリオカメラ終了ディレイ \
						-t 60\
						-e {
							chara カメラ設定 シナリオカメラ \
								-s -1
							command パッド操作 -cancel
							mesg シネマスクリーン シネマ フェードイン 1
							//	サイトを出す
							command サイト表示制御 \
								-switch on
						}
				}
			}
		}
	}

	proc 双眼鏡兵発見デモ設定 {
		trap en001 敵兵:01 \
			-mask ＊ \
			-exec {
				if ($3 == いる) {
					if ($f:w21a_双眼鏡兵所定位置フラグ == d:FALSE) {
						eval ($f:w21a_双眼鏡兵所定位置フラグ =d:TRUE)
						print '######################################################IN'
					}
				} else if ($3 == 出る) {
					if ($f:w21a_双眼鏡兵所定位置フラグ == d:TRUE) {
						eval ($f:w21a_双眼鏡兵所定位置フラグ =d:FALSE)
						print '######################################################OUT'
					}
				}
			}

	//	日本語版
	#ifdef d:JAPANESE
		//	ベリーイージーでは敵はいない
		if ($w:ゲーム設定 >= d:LEVEL_EASY) {
			trap ca004 d:PLAYER \
				-mask いる \
				-exec {
					//	双眼鏡兵が生きているときのみ
					if ($b:w20b_２Ｆ敵兵状態 != d:DEAD) {
						//	双眼鏡兵発見デモを見ていない場合
						if ($f:w19a_双眼鏡兵発見デモフラグ == d:FALSE) {
							//	双眼鏡兵が所定位置にいるときのみ
							if ($f:w21a_双眼鏡兵所定位置フラグ == d:TRUE) {
								//	双眼鏡兵が覗いているときのみ
								if ($f:w21a_双眼鏡兵覗きフラグ == d:TRUE) {
									//	双眼鏡兵を監視する
									@双眼鏡兵監視人
								}
							}
						}
					}
				}
		}

	//	それ以外
	#else
		trap ca004 d:PLAYER \
			-mask いる \
			-exec {
				//	双眼鏡兵が生きているときのみ
				if ($b:w20b_２Ｆ敵兵状態 != d:DEAD) {
					//	双眼鏡兵発見デモを見ていない場合
					if ($f:w19a_双眼鏡兵発見デモフラグ == d:FALSE) {
						//	双眼鏡兵が所定位置にいるときのみ
						if ($f:w21a_双眼鏡兵所定位置フラグ == d:TRUE) {
							//	双眼鏡兵が覗いているときのみ
							if ($f:w21a_双眼鏡兵覗きフラグ == d:TRUE) {
								//	双眼鏡兵を監視する
								@双眼鏡兵監視人
							}
						}
					}
				}
			}
	#endif
	}

	//	覗き始めたら
	proc 双眼鏡兵覗き開始 {
		eval ($f:w21a_双眼鏡兵覗きフラグ = d:TRUE)
	
	}
	proc 双眼鏡兵覗き終了 {
		eval ($f:w21a_双眼鏡兵覗きフラグ = d:FALSE)
	}



//	旧バージョン
#if 0
	proc 双眼鏡兵監視人 {
		#if d:DEBUG_PRINT
//			print 'KANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHIKANSHI'
		#endif
		//	双眼鏡兵が生きているときのみ
//		if ($f:w20b_２Ｆ敵兵死亡フラグ == d:FALSE) {
		if ($b:w20b_２Ｆ敵兵状態 != d:DEAD) {
			//	双眼鏡兵が覗いているときのみ
			if ($f:w21a_双眼鏡兵覗きフラグ == d:TRUE) {
				//	双眼鏡兵発見デモを見ていない場合
				if ($f:w19a_双眼鏡兵発見デモフラグ == d:FALSE) {
					//	ゲームステータス取得
					command ゲットゲームステータス $i:ゲームステータス
					//	敵兵状態取得
					command ゲット敵兵モード 敵兵:01 $i:w21a_敵兵モード

					if ( ($i:ゲームステータス & d:STATE_DETECT) == 0 ) {
						//	双眼鏡兵がプレイヤーを発見したら
						if ($i:w21a_敵兵モード & d:ENE_NOTICE_DETECT) {
							#if d:DEBUG_PRINT
							print 'DISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVERDISCOVER'
							#endif

							eval ($f:w19a_双眼鏡兵発見デモフラグ = d:TRUE)
							command パッド操作 -release
					
							//	シナリオカメラ起動
								chara カメラ設定 シナリオカメラ \
									-c 1 \
									-p 39651,-2234,-46432 -t 50354,5577,-64288 \
									-a 1001 \
									-i 0 0 0 0 \
									-s 1

							//	シネマスクリーン起動
							mesg シネマスクリーン シネマ フェードアウト 1
							//	サイトを消す
							command サイト表示制御 \
								-switch off

							//	シナリオカメラを殺す
							chara delay シナリオカメラ終了ディレイ \
								-t 60\
								-e {
									chara カメラ設定 シナリオカメラ \
									-s -1
									command パッド操作 -cancel
									mesg シネマスクリーン シネマ フェードイン 1
									//	サイトを出す
									command サイト表示制御 \
									-switch on
								}
						}
					}
				}
			}
		}

	}
#endif

#if 0
	proc 双眼鏡兵発見デモ設定 {
		//	双眼鏡兵発見デモを見ていない場合
		if ($f:w19a_双眼鏡兵発見デモフラグ == d:FALSE) {
		//	双眼鏡兵を監視する
			chara プロック連続実行 双眼鏡兵監視 -p 双眼鏡兵監視人
		}
	}
#endif



//###################################################################################
//									ストーリーフラグ
//###################################################################################



//###################################################################################
//									特殊ゲームオーバー
//###################################################################################

	//	ゲームオーバー背景処理
	proc ゲームオーバー背景設定 {
		command ゲームオーバー背景設定 \
			-t gmov \
			-s dekisi_alp_ovl \
			-m 2
	}

	//	ゲームオーバー開始処理
	//	fall.h内に記述
	
	//	ゲームオーバー終了処理
	proc ゲームオーバー終了処理 {
//		command セットサウンドコード -c d:se_code:SD_V_POBORE01	//ＳＥおぼれ悲鳴
		chara パッド振動 落下振動 -vibfile drop_sea

		@ゲームオーバー背景設定

	}

	//	エルード落下死開始処理
	//	fall.h内に記述

	//	エルード落下死終了処理
	//	fall.h内に記述

	//	エルード落下死
	proc エルード落下死設定 {
		trap de001 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				if ($3 == 入る) {
					#if d:DEBUG_PRINT
						print 'gameoverstart'
					#endif
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3852,1100,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-z d:CAMERA_NO_CUSHION \
						-s 1

					@エルード落下死開始処理
				} else if ($3 == 出る) {
					@ゲームオーバー終了処理
					@エルード落下死終了処理
				}
			
			}
		trap de002 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				if ($3 == 入る) {
					#if d:DEBUG_PRINT
						print 'gameoverstart'
					#endif
					chara カメラ設定 ゲームオーバーカメラ \
					-c 0 \
					-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 3960,2557,0 -f 4340 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1

					@エルード落下死開始処理
				} else if ($3 == 出る) {
					@ゲームオーバー終了処理
					@エルード落下死終了処理
				}
			
			}
		trap de003 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				if ($3 == 入る) {
					#if d:DEBUG_PRINT
						print 'gameoverstart'
					#endif
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-7000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3852,3735,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					@エルード落下死開始処理
				} else if ($3 == 出る) {
					@ゲームオーバー終了処理
					@エルード落下死終了処理
				}

			}
		trap de004 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				if ($3 == 入る) {
				#if d:DEBUG_PRINT
					print 'gameoverstart'
				#endif
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-7000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3852,2348,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-z d:CAMERA_NO_CUSHION \
						-s 1

					@エルード落下死開始処理
				} else if ($3 == 出る) {
					@ゲームオーバー終了処理
					@エルード落下死終了処理
				}
			}

	}


	proc ジャンプ落下死開始処理 {
		chara カメラ設定 ゲームオーバーカメラ \
			-c 0 \
			-b d:CAM_MIN,-7000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-r 3852,2348,0 -f 4340 \
			-a 200 \
			-i 0 -1 0 0 \
			-z d:CAMERA_NO_CUSHION \
			-s 1

		@ゲームオーバー開始処理
	}



//###################################################################################
//									特殊処理
//###################################################################################

proc システム設定 {
	command システムコールバック \
		-weapon 武器射程設定
}

proc 武器射程設定 {
	if ( ($w:武器 == d:武器:Ｍ９) || ($w:武器 == d:武器:ソコム)) {
		command 主観射程距離設定 \
			-len 50000
	} else {
		command 主観射程距離設定 \
			-len 40000
	}

}


// FIXデータはここまで
// -----------------------------------------------------------------------------------












// 以下仮キャラ用proc
// -----------------------------------------------------------------------------------
// 仮キャラ
// ------------------------------------



	// 子画面設定 
	// ------------------------------------
	proc 子画面設定 {
		#if d:DEBUG_PRINT
			print 'sub_camera_set'
		#endif

		chara サブ画面 子画面 \
			-w (512-d:SUB_MARGIN-d:SUB_WIDTH),d:SUB_MARGIN (512-d:SUB_MARGIN),(d:SUB_HEIGHT+d:SUB_MARGIN)
	}


















// 以下デバッグ専用proc
// -----------------------------------------------------------------------------------
// デバッグキャラ
// ------------------------------------
proc デバッグキャラセット {
	#if d:DEBUG_PRINT
		print 'debug_chara_set'
	#endif

	#if d:CHARA_CAMERA
		chara キャラカメラ 敵兵カメラ \
			-c d:TARGET_CHARA \
			-s -920,610,200 \
			-r -200,1024,0 
	#endif

	#if d:SCREENSHOT
		chara スクリーンショット shot
	#endif

	#if d:USB_KEY
		chara USBキーボード実行 test \
			-e d:KBD_ESC {	
				mesg プレイヤー d:PLAYER position -5500, -13000, -500
			} \
	#endif
}
