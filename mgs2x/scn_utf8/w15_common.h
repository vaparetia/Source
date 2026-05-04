/*
	w15_common.h                     
	    ＢＣ連絡橋

	1999/12/09 Y.Matsuhana         
	$Id: w15_common.h,v 1.126 2001/11/06 10:38:05 usr03379 Exp $                      

	
*/

// 各種スイッチ用define
// ------------------------------------
// サウンド
#define		SOUND			1		// サウンドデータは最初に呼ぶ
#define		BGM			1		// BGMは敵兵コマンダーの後に呼ぶ
// キャラクター関係
#define		RAIDEN			1		// SNAKEとは排他
#define		SNAKE			0
#define		ENEMY			1		// コマンダーと警備兵(敵を出すときは必須)
#define		SUPPORT			0		// サポート兵
#define		ATTACKER		0		// 攻撃兵
#define		SHIELD			0		// 盾兵(1) or 通常攻撃兵(0)
// 仮置きキャラ
#define		DOLL			0
// 監視カメラ関係
#define		CYPHER			1		// サイファー
// ドア
#define		WT_DOOR			0		// 水密ドア
#define		SLIDE_DOOR		1		// スライドドア
// 入出トラップ
#define		LOAD_TRAP		1
#define		STAGE_END		1		// ステージ終了時処理を使用可にする
// フォグ
#define		FOG			1
// エフェクト関係
#define		EFFECT			0		// エフェクト全般
#define		BLUR			0		// ブラー
#define		Z_MANAGER		0		// Ｚフォーカス管理(敵のライト等で仕様)
#define		BODY_SHADOW		0		// 敵のライトによって出る影
#define		BOKASHI			0		// 遠近ぼかし
// 子画面
#define		SUB_CAMERA		0
// カメラ関係
#define		CLEAR_CAMERA		0		// クリアリング専用カメラ(要子画面)
// イベント
#define		EVENT			1		// イベント
// 天井
#define		CHARA_CEIL		1		// 天井君
// 透明壁
#define		TR_WALL			0		// シナリオ置きの透明壁
// ライト関係
#define		SPOT_LIGHT		0		// スポットライト
#define		CHARA_LIGHT		1		// キャラ専用ライト
// オブジェ関係
#define		LOCKER			0		// ロッカー
#define		SEA			1		// 海面
#define		SKY			1		// 空
// アイテム
#define		ITEM			1
// 強制モーション
#define		JP_MOTION		0		// ジャンプモーション
#define		BY_MOTION		1		// ビヨンドモーション

// デモ
#define		SCN_DEMO			1	//	シナリオデモ
#define		POLY_DEMO			0


// デバッグ関係
#ifdef	d:FIX_MODE
	#define		DEBUG_CHARA	0		// デバッグキャラ全体(FIX時は0)
#else
	#define		DEBUG_CHARA	0		// デバッグキャラ全体
#endif
#define		FALLDOWN_RESET		0		// 落下したらリセット

#define		CHARA_CAMERA		0		// キャラ追従カメラ
#define		TARGET_CHARA		敵兵:01		// キャラ追従カメラのターゲット
#define		SCREENSHOT		0		// スクリーンショット
#define		USB_KEY			0		// USBキーボード

#define MULTI_MODEL	0		/* E3ビデオ用特別仕様(2000.4.8) */
#define VIDEO_SPECIAL	0		/* E3ビデオ用特別仕様(2000.4.8) */
#define LOD				0

// ------------------------------------
// マップ関係
#if d:W15A
	//	ステージ
	#define		KMS_NAME0		w15a
	//	ライト
	#define		LT2_NAME		w15a

	#define		KMS_NAME14		w15a_obj01	//	壊れ部分

#else
	//	ステージ
	#define		KMS_NAME0		w15b
	//	ライト
	#define		LT2_NAME		w15b
	//	壊れ部分
	#define		KMS_NAME14		w15b_obj01

#endif

//	ＡＢ共通
#define		KMS_NAME1		w15_24a		//	シェル１
#define		KMS_NAME2		w15_17a		//	ＣＤ連絡橋＆Ｄ脚
#define		KMS_NAME3		w15_13a		//	ＡＢ連絡橋
#define		KMS_NAME4		w15_12a		//	Ａ脚

#define		KMS_NAME5		w15_14a		//	Ｂ脚

#define		KMS_NAME6		w15_19a		//	ＤＥ連絡橋
#define		KMS_NAME7		w15_23a		//	ＦＡ連絡橋

#define		KMS_NAME8		w15_16a		//	Ｃ脚

#define		KMS_NAME9		w15_20a		//	Ｅ脚
#define		KMS_NAME10		w15_other00 //	浮き橋１
#define		KMS_NAME11		w15_other01 //	浮き橋２
#define		KMS_NAME12		w15_other02 //	浮き橋３
#define		KMS_NAME13		w15_crane	//	クレーン

#define		KMS_NAME_null	null		//	ダミー

//	プットオブジェおき
#define		KMS_NAME15		w15_14b		//	Ｂ脚ロードエリア
#define		KMS_NAME16		w15_16b		//	Ｃ脚ロードエリア


//	アタリ
#define		HZX_NAME		w15a
										//	0		プレイヤーアタリ／跳弾アタリ
										//	1～10	遠景跳弾アタリ
										//	11		影用アタリ
										//	12		レンズフレア用アタリ

//	ドア
#define		DOOR_NAME1		w15a_dr00



// キャラクター関係
#if d:RAIDEN
	#define		PLAYER		ライデン	// モーションができるまで名前はスネーク
#else
	#define		PLAYER		スネーク
#endif

//	強制モーション
#define		BY_HIGH					1		// 高所ビヨンドモーション

//	薬莢系
#if d:W15A
	#define		ENE_P_GOL			1	//	都市迷彩ゴル
	#define		ENE_SHOT			1	// ショットガン兵
	#define		ENE_HIGH_TECH 		1	// ハイテク兵
	#define		ENE_SHIELD 			1	// 盾兵
#else
	#define		ENE_P_GOL			1	//	都市迷彩ゴル
	#define		ENE_SHOT			1	// ショットガン兵
	#define		ENE_HIGH_TECH 		1	// ハイテク兵
	#define		ENE_SHIELD 			1	// 盾兵
#endif

//	ステージ系
#if d:W15A
	#define		STAGE_W15A			1	//	昼
#else
	#define		STAGE_W15B			1	//	夜
#endif

#define STAGE_PLANT					1	//	プラント編


// ローカル変数
//	$f:w15a_敵配置フラグ
#define STRUT_B		0					//	Ｂ脚から来たときの敵配置
#define STRUT_C		1					//	Ｃ脚から来たときの敵配置

//	$f:w15a_サイファー破壊フラグ		//	サイファーを破壊したかどうか

//	その他
#define CTDOWN_TIMER_P		1		//	爆弾イベント用


// ステージ固有モーションリスト(Snake)
// ------------------------------------
#include "rai_w15a.mh"
/*
enum モーションリスト {
	none_drop_start = 0,		// ビヨンド開始
	sna_non_drop_start_behind_t,	// ビヨンド開始(ビハインドから)
	none_drop,			// ビヨンド静止中
	none_drop_r,			// ビヨンド右移動
	none_drop_l,			// ビヨンド左移動
	none_drop_end_fence,		// ビヨンド終了(壁なし)
	none_drop_end_wall,		// ビヨンド終了(壁あり)
	none_drop_idle_tired,		// ビヨンド疲れ
	non_elude_fall_start,		// 落下モーション(落ちはじめ)
	non_elude_fall,			// 落下中モーション
	non_elude_fall_damage,		// 落下してダメージ
	non_elude_fall_out,		// 落下して死亡
	non_elude_fall_normal,		// 落下しても無事着地
	non_drop_dam,				// エルード中のダメージ
	non_jump_up,			// １ｍジャンプ
	non_jump_down_start,	// １ｍジャンプ降りるスタート
	non_jump_down_loop,		// １ｍジャンプ降りるループ
	non_jump_down_end,		// １ｍジャンプ降りるエンド
	non_jump_up_behind,		// ビハインド１ｍジャンプ
	non_carry_body_throw1,	//死体捨て
};
*/


// include
// ------------------------------------
#include "stdch.h"
#include "sload.h"
#include "beyond.h"
#include "beyond2.h"
#include "fall.h"
#include "sound.h"
#include "enevoice.h"
#include "w15_kamome.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif
#include "item_info.h"
#include "vibration.h"

#include "w15_d_common.h"
#include "ene_item.h"

// マップ設定
// ------------------------------------
chara マップ ＢＣ連絡橋 -k d:KMS_NAME0 \
			-k d:KMS_NAME1 \
			-k d:KMS_NAME2 \
			-k d:KMS_NAME3 \
			-k d:KMS_NAME4 \
			-k d:KMS_NAME6 \
			-k d:KMS_NAME7 \
			-k d:KMS_NAME9 \
			-k d:KMS_NAME10 \
			-k d:KMS_NAME11 \
			-k d:KMS_NAME12 \
			-k d:KMS_NAME13 \
			-k d:KMS_NAME14 \

			-h d:HZX_NAME,0 -l d:LT2_NAME

//	ＨＺＸ追加登録
//	跳弾アタリ用マップ
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 1
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

//	グループ１２はレンズフレア用ダミー
command 追加ＨＺＸグループ登録 \
	-hzx_id 12 -add_id 0

//	グループ１１は影用

//	デバッグ用
//eval ($w:p_story = d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了)
//eval($w:p_story = d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了)

command マップ設定 ＢＣ連絡橋 -set {
	//	デモが起きるかどうかのチェック
	@サイファー遭遇デモ開始チェック
#if d:SOUND
	@サウンド設定	// データは最初に呼ぶ
#endif
	//	ノードアクセス関連
	if ( $f:w14a_ノードフラグ == 1 || $f:w16a_ノードフラグ == 1) {
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
	@敵配置チェック
	@コマンダー設定
#endif
#if d:CYPHER
	@サイファー配置チェック		//	サイファー再セットチェック用
	@サイファー設定
#endif
#if d:BGM	
	@ＢＧＭ設定	// プレイヤー→敵兵→BGMの順で呼ぶ
#endif
#if d:SLIDE_DOOR
	@ドア設置
	@主観禁止壁設定
#endif
#if d:LOAD_TRAP
	@ＢＣ連絡橋ロード設定
#endif
	@オブジェクト設定
#if d:SEA
	@海面設置
	@海面管理設定
#endif
#if d:SKY
	@空設置
#endif
#if d:CHARA_CEIL
	@天井設定
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
	@煙設定
	@水設定
	@ＢＣ連絡橋風設定
	@影設定
	@カメラセット
#if d:BY_MOTION
	@ＢＣ連絡橋ビヨンド設定
#endif
	@ＢＣ連絡橋カメラ設定

	@エルード落下死設定
	@ジャンプ落下死設定
	@死体捨て設定

#if d:SCN_DEMO
//	@サイファー遭遇デモ設定		//	デモはなし
	@シナリオデモキャラセット
# endif

# if d:CHARA_LIGHT
	@キャラ用ライト設定		// マップ設定の最後に呼ぶ
# endif
	@爆弾タイマー設置	//	爆弾イベント用
	@プラント編全体マップ設定  d:ＢＣ連絡橋マップ	//	全体マップ表示

}

//※※通信機のincludeは移動させないこと！！※※
#include "w15_cdc.ch"
#include "p_force_cdc.ch"


//初期マップの表示
// -----------------------------------
command マップ表示 	-add ＢＣ連絡橋
@ＢＣ連絡橋開始時処理

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

//	朝の場合
#if d:W15A
		chara サウンドマネージャー ＳＤマネ -pak 0

//	昼の場合
#else
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
#endif

	//		ＳＥ設定
		@サウンド効果音設定
		command 弾痕ノーマル -n 0
		command 弾痕ノーマル -n 1
		command 弾痕ノーマル -n 3

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 3
		command 跳弾ＳＥのみ -n 4

		//	水蒸気
		command 弾痕ノーマル -n 5
		command 弾痕ノーマル -n 6
		command 跳弾水蒸気小 -n 5
		command 跳弾水蒸気中 -n 6

	}
	proc ローカル変数初期化 {
		#if d:DEBUG_PRINT 
			print 'localvar_init'
		#endif
	}
	proc 設定値変更 {
		#if d:DEBUG_PRINT 
			print 'status_set'
		#endif
		@デモ直後処理	//	デモ直後
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
		#if d:RAIDEN
				chara プレイヤー d:PLAYER \
					-p $i:プレイヤー初期Ｘ位置 $i:プレイヤー初期Ｙ位置 $i:プレイヤー初期Ｚ位置 \
					-d 0,$i:プレイヤー初期方向,0 \
					-s $b:プレイヤー初期姿勢 \
					-l 4000 \
					-m rai_def \
					-a raiden \
					-o rai_w15a \
					-i 0 

				//髪の毛
				@ライデン髪の毛設定 d:NORMAL_HAIR

				//	ＬＯＤ機能
				@プレイヤーＬＯＤ設定 d:C_DISTANCE_TYPE 5000
//				@プレイヤーＬＯＤ設定 d:P_STATE_TYPE d:TRP_STATE_BEHIND

		#else
				chara プレイヤー d:PLAYER \
					-m sna_def \
					-a snake \
					-p $i:プレイヤー初期Ｘ位置 $i:プレイヤー初期Ｙ位置 $i:プレイヤー初期Ｚ位置 \
					-d 0,$i:プレイヤー初期方向,0 \
					-l 4000 \
					-s $b:プレイヤー初期姿勢 \
					-o sna_w00a \
					-i 0 
				//バンダナ
				chara ロープモデル３ バンダナ１ \
					-n sna_bdn1 \
					-d 1 \
					-y d:PLAYER \
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
					-y d:PLAYER \
					-z 12 \
					-x 0,50,-90 \
					-m 22 \
					-p 250 \
					-k 350 \
					-o 110 \
					-f 1
		#endif
		#if d:MULTI_MODEL
			chara マルチウェイトキャラ マルチ \
				-name d:PLAYER \
				-model sna_mmt

			chara 臨時顔アニメキャラ アニメキャラ \
				-chara	d:PLAYER
		#endif
		#if d:LOD
			// 距離によるＬＯＤの使用
			chara ＬＯＤ制御 プレイヤーＬＯＤ \
						-name d:PLAYER \
						-type 1 \
						-model sna_def_sh \
						-value 2000
		#endif
	}

//###################################################################################
//								敵兵
//###################################################################################

	proc 敵配置チェック {
//		自分の来た場所で敵配置変更
		if( $s:登場ポイント == sp_w14a_w15a_0 ) {
			eval($f:w15a_敵配置フラグ = d:STRUT_B)
		} else {
			eval($f:w15a_敵配置フラグ = d:STRUT_C)
		}
	}





//	敵兵配置

//	基本セット

	//	警備兵
	proc 警備兵セット {

		//	ストーリーフラグや難易度で分岐
		//	ファットマン戦後から
		if ($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {

			//	サイファー遭遇デモを見た次の回から
			if ( ($f:w23a_サイファー遭遇デモフラグ == d:TRUE) && ($s:登場ポイント != サイファー遭遇デモ後) ) {
				@警備兵０
			}
		}
	}

	//	攻撃兵
	proc 攻撃兵セット {

		//	ストーリーフラグや難易度で分岐

		//	エイムズ死亡前
		if ($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {


			//	ファットマン戦後
			if ($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {
				//	警備兵がいるとき
				if ( ($f:w23a_サイファー遭遇デモフラグ == d:TRUE) && ($s:登場ポイント != サイファー遭遇デモ後) ) {

					//	イージー以下
					if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//						@攻撃兵４	//	敵１人	ノーマル
						@攻撃兵８	//	敵１人	ノーマル（サポート化）

					//	ハード以下
					} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//						@攻撃兵０	//	敵２人	ノーマル
						@攻撃兵６	//	敵２人	ノーマル（サポート化）

					//	エクストリーム
					} else {
						@攻撃兵Ａ	//	敵４人	ノーマル（サポート化）
					}

				//	警備兵がいないとき
				} else {

					//	イージー以下
					if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//						@攻撃兵４	//	敵１人	ノーマル
						@攻撃兵１１	//	敵１人	ノーマル（巡回）

					//	ハード以下
					} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//						@攻撃兵０	//	敵２人	ノーマル
						@攻撃兵１０	//	敵２人	ノーマル（巡回）

					//	エクストリーム
					} else {
						@攻撃兵Ｃ	//	敵４人	ノーマル（巡回）
					}
				}

			//	ファットマン戦前
			} else {
				//	サイファーが登場したら（ピーターと会ったあとから）アタッカーも登場する
				if ($w:p_story >= d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {

					//	イージー以下
					if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//						@攻撃兵４	//	敵１人	ノーマル
						@攻撃兵１１	//	敵１人	ノーマル（巡回）

					//	ハード以下
					} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//						@攻撃兵０	//	敵２人	ノーマル
						@攻撃兵１０	//	敵２人	ノーマル（巡回）

					//	エクストリーム
					} else {
						@攻撃兵Ｃ	//	敵４人	ノーマル（巡回）
					}
				}
			}

		//	エイムズ死亡後
		} else {

			//	警備兵がいるとき
			if ( ($f:w23a_サイファー遭遇デモフラグ == d:TRUE) && ($s:登場ポイント != サイファー遭遇デモ後) ) {

				//	イージー以下
				if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//					@攻撃兵５	//	敵１人	ハイテクノーマル
					@攻撃兵９	//	敵１人	ハイテクノーマル（サポート化）

				//	ハード以下
				} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//					@攻撃兵３	//	敵２人	ハイテクノーマル
					@攻撃兵７	//	敵２人	ハイテクノーマル（サポート化）

				//	エクストリーム
				} else {
					@攻撃兵Ｂ	//	敵４人	ハイテクノーマル（サポート化）
				}

			//	警備兵がいないとき（ありえないはず）
			} else {

				//	イージー以下
				if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//					@攻撃兵５	//	敵１人	ハイテクノーマル
					@攻撃兵１３	//	敵１人	ハイテクノーマル（巡回）

				//	ハード以下
				} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//					@攻撃兵３	//	敵２人	ハイテクノーマル
					@攻撃兵１２	//	敵２人	ハイテクノーマル（巡回）

				//	エクストリーム
				} else {
					@攻撃兵Ｄ	//	敵４人	ハイテクノーマル（巡回）
				}
			}

		}
	}

	//	@攻撃兵０	//	敵２人	ノーマル
	//	@攻撃兵１	//	敵２人	盾兵
	//	@攻撃兵２	//	敵２人	ショットガン
	//	@攻撃兵３	//	敵２人	ハイテクノーマル
	//	@攻撃兵４	//	敵１人	ノーマル
	//	@攻撃兵５	//	敵１人	ハイテクノーマル

	//	@攻撃兵６	//	敵２人	ノーマル（サポート化）
	//	@攻撃兵７	//	敵２人	ハイテクノーマル（サポート化）
	//	@攻撃兵８	//	敵１人	ノーマル（サポート化）
	//	@攻撃兵９	//	敵１人	ハイテクノーマル（サポート化）

	//	@攻撃兵１０	//	敵２人	ノーマル（巡回）
	//	@攻撃兵１１	//	敵１人	ノーマル（巡回）
	//	@攻撃兵１２	//	敵２人	ハイテクノーマル（巡回）
	//	@攻撃兵１３	//	敵１人	ハイテクノーマル（巡回）

	//	@攻撃兵Ａ	//	敵４人	ノーマル（サポート化）
	//	@攻撃兵Ｂ	//	敵４人	ハイテクノーマル（サポート化）
	//	@攻撃兵Ｃ	//	敵４人	ノーマル（巡回）
	//	@攻撃兵Ｄ	//	敵４人	ハイテクノーマル（巡回）

	//	サイファー
	proc サイファーセット{

		//	ストーリーフラグや難易度で分岐
		//	ピーターと会ったあとから敵配置
		if ($w:p_story >= d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {

			//	サイファー遭遇デモを見ていないとき
			if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
				//	潜入モード時はデモ、それ以外は通常ゲーム
				if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
					//	デモ用サイファー
					print 'DEMO_CYPHER'
					@デモサイファー
				} else {
					//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@サイファー０
				}

			//	それ以外
			} else {
				//	サイファー遭遇デモ直後
				if ( $s:登場ポイント == サイファー遭遇デモ後 ) {
					//	デモ直後サイファー
					print 'GAME_CYPHER'
					@デモ直後サイファー

				//	通常ゲーム
				} else {
					//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@サイファー０
				}
			}

		}
	}



//	コマンダーセット

	//	警備兵
	proc 警備兵コマンダー {
		#if d:DEBUG_PRINT
			print 'wccomm_set'
		#endif

		chara 警備コマンダー wccomm -e { 警備兵セット }

	}

	//	攻撃兵
	proc 攻撃兵コマンダー {
		#if d:DEBUG_PRINT
			print 'atcomm_set'
		#endif

		chara 攻撃コマンダー atcomm \
			-e { 攻撃兵セット } \
			-wait (60*3) \			/*危険開始時の出動遅らせ時間*/
			-avoidwait (60*5) \		/*回避モードで始まる場合のみ有効*/
			-status 0x01
	}

	//	コマンダー

	//	司令官音声
	resource ghq_area_alert_voice {
	w15a:
		$t{
			// PLAREA0
			t:vc040140,	//	指揮官　　「了解。応援部隊をＢＣ連絡橋に派遣する。敵を包囲せよ」
			t:vc040141,	//	指揮官　　「了解。増援部隊、ＢＣ連絡橋へ急行！侵入者を逃がすな！」
			-2,
			// PLAREA1
			t:vc040136,	//	指揮官　　「了解。応援部隊、ＢＣ連絡橋・Ｂ脚入口へ向かえ！敵を逃がすな！」
			t:vc040137,	//	指揮官　　「了解。増援部隊、ＢＣ連絡橋へ急行！Ｂ脚入口を守れ！」
			-2,
			// PLAREA2
			t:vc040138,	//	指揮官　　「了解。応援部隊、ＢＣ連絡橋・Ｃ脚入口へ向かえ！敵を逃がすな！」
			t:vc040139,	//	指揮官　　「了解。増援部隊、ＢＣ連絡橋へ急行！Ｃ脚入口を守れ！」
			-1
		}
	}

	resource ghq_caution_voice {

	//	攻撃兵が残っていない場合
	w15a:
		$t{
			t:vc040601,	//	指揮官　　「了解、総員、警戒を強化せよ」

			t:vc040491,	//	指揮官　　「攻撃チームは引き返せ、警戒チームはそのまま現場に残れ」
			t:vc040492,	//	指揮官　　「攻撃チームは撤収、警戒チームは警戒を続行せよ」
			t:vc040493,	//	指揮官　　「攻撃チーム帰投せよ、警戒チームはそのまま警戒に当たれ」
			t:vc040496,	//	指揮官　　「攻撃チーム、配置に戻れ」
			t:vc040497,	//	指揮官　　「攻撃チーム、撤収せよ」
			-2,
			t:vc040422,	//	指揮官　　「ＢＣ連絡橋との連絡が途絶えた」
			-3,
			t:vc040607,	//	カメラ　　「異状発生…」
			-1
		}

	//	攻撃兵が残っている場合
	w15a-2:
		$t{
			// PLAREA0
			t:vc040304,	//	指揮官　　「了解。ＢＣ連絡橋に増員を派遣する。警備を強化しろ」
			t:vc040305,	//	指揮官　　「了解。ＢＣ連絡橋を増員する。警戒を強化しろ」

			t:vc040300,	//	指揮官　　「了解。ＢＣ連絡橋を増員。Ｂ脚入口の警備を固めろ」
			t:vc040301,	//	指揮官　　「了解。ＢＣ連絡橋・Ｂ脚入口に増員を送る。警戒を強化しろ」

			t:vc040302,	//	指揮官　　「了解。ＢＣ連絡橋を増員。Ｃ脚入口の警備を固めろ」
			t:vc040303,	//	指揮官　　「了解。ＢＣ連絡橋・Ｃ脚入口に増員を送る。警戒を強化しろ」
			-2,
			t:vc040422,	//	指揮官　　「ＢＣ連絡橋との連絡が途絶えた」
			-3,
			t:vc040607,	//	カメラ　　「異状発生…」
			-1
		}
	}

	proc コマンダー設定 {
		#if d:DEBUG_PRINT
			print 'commander_set'
		#endif
		//	ファットマン戦後から警備兵登場
		if ($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {

			//	サイファー遭遇デモを見た次の回から警備兵登場
			if ( ($f:w23a_サイファー遭遇デモフラグ == d:TRUE) && ($s:登場ポイント != サイファー遭遇デモ後) ) {
				chara コマンダー 敵兵セット \
					-w 警備兵コマンダー \
					-a 攻撃兵コマンダー \
					-r -53250,0,-96000 ＢＣ連絡橋 -48500,0,-42500 ＢＣ連絡橋 \
					-c -53250,0,-96000 ＢＣ連絡橋 -48500,0,-42500 ＢＣ連絡橋 \
					-v [ene_plant_voice:01] \
					-x [ghq_area_alert_voice:w15a] \
					-y [ghq_caution_voice:w15a-2] \
					-z $i:敵標準麻酔持続 \
					-o $i:敵標準気絶持続 \
					-b $w:敵標準再発生数 \
					-s d:ENE_STAGE_GPS \
					-f 1 0 t:vc044023 t:vc044023_len \
					-m $w:スタートアラートモード \
					-h $i:標準警戒時間		// 警戒時間

			//	サイファー遭遇デモを見た直後は警備兵まだはいない
			} else {
				chara コマンダー 敵兵セット \
					-a 攻撃兵コマンダー \
					-r -53250,0,-96000 ＢＣ連絡橋 -48500,0,-42500 ＢＣ連絡橋 \
					-c -53250,0,-96000 ＢＣ連絡橋 -48500,0,-42500 ＢＣ連絡橋 \
					-v [ene_plant_voice:02] \
					-x [ghq_area_alert_voice:w15a] \
					-y [ghq_caution_voice:w15a-2] \
					-z $i:敵標準麻酔持続 \
					-o $i:敵標準気絶持続 \
					-b $w:敵標準再発生数 \
					-s d:ENE_STAGE_GPS \
					-m $w:スタートアラートモード \
					-h $i:標準警戒時間		// 警戒時間
			}

		//	ファットマン戦までは警備兵登場しない
		} else {
			//	サイファーが登場したら（ピーターと会ったあとから）アタッカーも登場する
			if ($w:p_story >= d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
				chara コマンダー 敵兵セット \
					-a 攻撃兵コマンダー \
					-r -53250,0,-96000 ＢＣ連絡橋 -48500,0,-42500 ＢＣ連絡橋 \
					-c -53250,0,-96000 ＢＣ連絡橋 -48500,0,-42500 ＢＣ連絡橋 \
					-v [ene_plant_voice:02] \
					-x [ghq_area_alert_voice:w15a] \
					-y [ghq_caution_voice:w15a-2] \
					-z $i:敵標準麻酔持続 \
					-o $i:敵標準気絶持続 \
					-b $w:敵標準再発生数 \
					-s d:ENE_STAGE_GPS \
					-m $w:スタートアラートモード \
					-h $i:標準警戒時間		// 警戒時間
			}
		}

	}

	//	サイファー設定
	proc サイファー設定 {
		@サイファーセット
	}



//	警備兵セット

	//	警備兵１人
	proc 警備兵０ {
		#if d:DEBUG_PRINT
			print 'watcher_set'
		#endif

		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara 警備兵 敵兵:01 -r 1 -d -52000,0,-73000 ＢＣ連絡橋 -n 1 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s d:ENE_STATUS_REPORT \		//	定時連絡兵
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w15a_guard_ve_01] [dogtag:plt_w15a_guard_ea_01] [dogtag:plt_w15a_guard_no_01] \
					[dogtag:plt_w15a_guard_ha_01] [dogtag:plt_w15a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
			chara 警備兵 敵兵:01 -r 1 -d -52000,0,-47000 ＢＣ連絡橋 -n 1 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s d:ENE_STATUS_REPORT \		//	定時連絡兵
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w15a_guard_ve_01] [dogtag:plt_w15a_guard_ea_01] [dogtag:plt_w15a_guard_no_01] \
					[dogtag:plt_w15a_guard_ha_01] [dogtag:plt_w15a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		}
	}



//	攻撃兵セット

	//	ノーマル２人
	proc 攻撃兵０ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
	}

	//	盾兵２人
	proc 攻撃兵１ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_SHIELD	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_SHIELD	//	$w:装備タイプ
	}

	//	ショットガン兵２人
	proc 攻撃兵２ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_SHOTGUN	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_SHOTGUN	//	$w:装備タイプ
	}

	//	ハイテクノーマル２人
	proc 攻撃兵３ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
	}

	//	ノーマル１人
	proc 攻撃兵４ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara アタッカー 敵兵:61 \
				-r 2 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
				chara アタッカー 敵兵:61 \
				-r 3 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}

	//	ハイテクノーマル１人
	proc 攻撃兵５ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara アタッカー 敵兵:61 \
				-r 2 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
			chara アタッカー 敵兵:61 \
				-r 3 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		}
	}

	//	ノーマル２人（サポート兵化）
	proc 攻撃兵６ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-s 0x40000000 \		//	サポート兵化
			-b 敵兵:01 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
	}

	//	ハイテクノーマル２人（サポート化）
	proc 攻撃兵７ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-s 0x40000000 \		//	サポート兵化
			-b 敵兵:01 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
	}

	//	ノーマル１人（サポート兵化）
	proc 攻撃兵８ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara アタッカー 敵兵:61 \
				-r 2 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
				chara アタッカー 敵兵:61 \
				-r 3 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}

	//	ハイテクノーマル１人（サポート化）
	proc 攻撃兵９ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara アタッカー 敵兵:61 \
				-r 2 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
			chara アタッカー 敵兵:61 \
				-r 3 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		}
	}

	//	ノーマル２人（巡回）
	proc 攻撃兵１０ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif

			print 'patrol_set'

		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 0 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
	}

	//	ノーマル１人（巡回）
	proc 攻撃兵１１ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara アタッカー 敵兵:61 \
				-r 2 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 1 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
				chara アタッカー 敵兵:61 \
				-r 3 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 1 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}

	//	ハイテクノーマル２人（巡回）
	proc 攻撃兵１２ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 1 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
	}

	//	ハイテクノーマル１人（巡回）
	proc 攻撃兵１３ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		if ($f:w15a_敵配置フラグ == d:STRUT_B) {
			chara アタッカー 敵兵:61 \
				-r 2 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 1 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else if ($f:w15a_敵配置フラグ == d:STRUT_C) {
			chara アタッカー 敵兵:61 \
				-r 3 \				//$b:初期ルート \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 1 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		}
	}


	//	ノーマル４人（サポート化）
	proc 攻撃兵Ａ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-s 0x40000000 \		//	サポート兵化
			-b 敵兵:01 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:63 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:64 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
	}

	//	ハイテクノーマル４人（サポート化）
	proc 攻撃兵Ｂ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-s 0x40000000 \		//	サポート兵化
			-b 敵兵:01 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:63 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:64 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
	}

	//	ノーマル４人（巡回）
	proc 攻撃兵Ｃ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif

			print 'patrol_set'

		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 0 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:63 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:64 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
	}

	//	ハイテクノーマル４人（巡回）
	proc 攻撃兵Ｄ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 1 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:61 \
			-r 2 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 3 \				//$b:初期ルート \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
	}



//	サイファーセット

	//	サイファー配置チェック
	proc サイファー配置チェック {
		print '+++++++++++++++++++++++++++++++++++++++++++++++++CYPHER01' $w:グローバルロード回数
//		@壊れ物再セットチェック $b:w15a_破壊ロード回数[0]
		@サイファー再セットチェック $b:w15a_破壊ロード回数[0]

		if ($f:再セットフラグ) {
			print 'CYPHER01_REBIRTH'
			eval ($f:w15a_サイファー破壊フラグ = d:FALSE)
		}
	}

	//	サイファー破壊プロック
	proc サイファー破壊 {
		print 'CYPHER01_DESTORY'
		eval ($b:w15a_破壊ロード回数[0] = $w:グローバルロード回数)
		eval ($f:w15a_サイファー破壊フラグ = d:TRUE)
		print '##########################################CYPHER01' $b:w15a_破壊ロード回数[0]
	}

	//	デモ用配置
	proc デモサイファー {
		if ($f:w15a_サイファー破壊フラグ == d:FALSE) {
			chara サイファ デモサイファー:01 \
				-route 6 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	52000 10000  80000 \
						52000 10000 200000 \
				-speed 25
		}
	}

	//	デモ直後用配置
	proc デモ直後サイファー {
		if ($f:w15a_サイファー破壊フラグ == d:FALSE) {
			chara サイファ  サイファー:01 \
				-route 5 3 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-p サイファー破壊 \
				-home	52000 10000  80000 \
						52000 10000 200000 \
				-speed 15
		}
	}

	//	サイファー１機
	proc サイファー０ {
		#if d:DEBUG_PRINT 
			print 'cypher_set'
		#endif

		if ($f:w15a_サイファー破壊フラグ == d:FALSE) {
			if ($f:w15a_敵配置フラグ ==  d:STRUT_B) {
				chara サイファ  サイファー:01 \
					-route 4 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	52000 10000  80000 \
							52000 10000 200000 \
					-speed 25
			} else if ($f:w15a_敵配置フラグ ==  d:STRUT_C) {
				chara サイファ  サイファー:01 \
					-route 5 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	52000 10000  80000 \
							52000 10000 200000 \
					-speed 15
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

/*
	//	w15_d_common.hに移行

	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif
		chara ドア Ｂ脚ドア \
			-m d:DOOR_NAME1 \
			-d 0 0 0 \
			-p -55500 ,0,-43625 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＢＣ連絡橋 ＢＣ連絡橋 \
			-A Ｂ脚ドアランプ \
			-exec{
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｂ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｂ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｂ脚ドアランプ 0 -55500 0 -43625 1

		trap dr001 ？ -mask ？ \
			-exec {
				if ($3 == 入る) {
					mesg ドア Ｂ脚ドア open $2
				} else if ($3 == 出る) {
					mesg ドア Ｂ脚ドア close $2
				}
			}
		chara ドア Ｃ脚ドア \
			-m d:DOOR_NAME1 \
			-d 0 0 0 \
			-p -56750,0,-76375 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＢＣ連絡橋 ＢＣ連絡橋 \
			-A Ｃ脚ドアランプ \
			-exec{
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｃ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｃ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｃ脚ドアランプ 0 -56750 0 -76375 0

		trap dr002 ？ -mask ？ \
			-exec {
				if ($3 == 入る) {
					mesg ドア Ｃ脚ドア open $2
				} else if ($3 == 出る) {
					mesg ドア Ｃ脚ドア close $2
				}
			}
	}
*/

	proc 主観禁止壁設定 {

		@主観禁止壁 Ｂ脚ドア壁 -55500 0 -43625 0 d:SD_SLIDE_OUTSIDE
		@主観禁止床 Ｂ脚ドア床１ Ｂ脚ドア床２ -55500 0 -43625 0 d:SD_SLIDE_OUTSIDE
		trap dr001 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ｂ脚ドア壁 on
				} else {
					mesg 透明壁 Ｂ脚ドア壁 off
				}
			}

		@主観禁止壁 Ｃ脚ドア壁 -56750 0 -76375 0 d:SD_SLIDE_OUTSIDE
		@主観禁止床 Ｃ脚ドア床１ Ｃ脚ドア床２ -56750 0 -76375 0 d:SD_SLIDE_OUTSIDE
		trap dr002 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ｃ脚ドア壁 on
				} else {
					mesg 透明壁 Ｃ脚ドア壁 off
				}
			}



	
	}

//###################################################################################
//									開始時処理
//###################################################################################

	proc デモ直後処理 {
		
//		eval ($s:直前のエリア = d012p01)
		//	デモ直後に来たとき
		if ($s:直前のエリア == d012p01) {
			eval ($i:プレイヤー初期Ｘ位置 = -53500) ;
			eval ($i:プレイヤー初期Ｙ位置 = 0) ;
			eval ($i:プレイヤー初期Ｚ位置 = -45000) ;
			eval ($i:プレイヤー初期方向 = 1024) ;
			eval ($b:プレイヤー初期姿勢 = d:FA_END_SQUAT) ;
		}
	}

	proc ＢＣ連絡橋開始時処理{
		#if d:DEBUG_PRINT 
		print 'STAGE_START'
		#endif
		//	デモ直後に来たとき
		if ($s:直前のエリア == d012p01) {
			@ステージ開始時処理  d:LOAD_TYPE:NO_MOVE_TYPE 0 0 0
			@姿勢制御設定
		//	それ以外
		} else {
			//	サイファー遭遇デモの後
			if ( $s:登場ポイント == サイファー遭遇デモ後 ) {
				@ステージ開始時処理  d:LOAD_TYPE:NO_MOVE_TYPE 0 0 0

			//	それ以外の通常ゲーム
			} else {
				if( $s:登場ポイント == sp_w14a_w15a_0 ){
					@ステージ開始時処理（ステージ名表示） \
					d:LOAD_TYPE:SL_DOOR_TYPE Ｂ脚ドア 2500 d:DEFAULT_M_TYPE \
					d:wn_ＢＣ連絡橋

//					@Ｂ脚ロード時カメラ設定	//	真上真下のときはわかりにくいのでなし
				} else if( $s:登場ポイント == sp_w16a_w15a_0 ){
					@ステージ開始時処理（ステージ名表示） \
					d:LOAD_TYPE:SL_DOOR_TYPE Ｃ脚ドア 2500 d:DEFAULT_M_TYPE \
					d:wn_ＢＣ連絡橋

//					@Ｃ脚ロード時カメラ設定	//	真上真下のときはわかりにくいのでなし
				}
			}
		}
	}

//###################################################################################
//									ロード設定
//###################################################################################

	proc ロードプロック {
		if ($s:呼び出しプロック == mv_w15a_w14a_0) {
			@mv_w15a_w14a_0
		} else if ($s:呼び出しプロック == mv_w15a_w16a_0) {
			@mv_w15a_w16a_0
		}
	}

	proc ＢＣ連絡橋ロード設定 {
		trap ld001 d:PLAYER \
			-mask 入る \
			-exec {
				// Ｂ脚（w14a）へ
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
					@ステージ終了時処理（ステージ名表示） \
					-54750 0 -42000 0 mv_w15a_w14a_0 d:DEFAULT_M_TYPE \
					d:wn_Ｂ脚変電室
					@Ｂ脚ロード時カメラ設定
					//	先読み
					preseek 'w14a'
				}
			}
		trap ld002 d:PLAYER \
			-mask 入る \
			-exec {
				// Ｃ脚（w16a）へ
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
					@ステージ終了時処理（ステージ名表示） \
					-56000 0 -78000 2048 mv_w15a_w16a_0 d:DEFAULT_M_TYPE \
					d:wn_Ｃ脚食堂
					@Ｃ脚ロード時カメラ設定
					//	先読み
					//	デモまでは"a"に飛ばす
					if ($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
						preseek 'w16a'
					//	デモ後は"b"に飛ばす
					} else {
						preseek 'w16b'
					}
				}
			}
	}

	proc Ｂ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p -51726,4201,-44976 \
						-t -54412,1679,-44355 \
						-a 200 \
						-i 0 0 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}

	proc Ｃ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p (-58151 +4250),3935,-74901 \
						-t (-60229 +4250),1592,-75634 \
						-a 200 \
						-i 0 0 0 0 \
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

	proc 姿勢制御設定 {
		chara プロック連続実行 姿勢制御管理人 \
			-proc 姿勢制御管理
	}

	proc 姿勢制御管理 {
		command パッドチェック \
			-no_use
		if ($status != 1) {
			mesg プロック連続実行 姿勢制御管理人 kill

			command パッド操作 \
				-release
			mesg プレイヤー d:PLAYER stance \
				d:FA_END_STAND -1 -1 0
			command パッド操作 \
				-cancel
		}
	}


//###################################################################################
//									オブジェクト
//###################################################################################

	proc オブジェクト設定 {
		#if d:DEBUG_PRINT
			print 'w15a_bridge_set'
		#endif
//			chara プットオブジェ 中央連絡橋破壊後 -m w15a_obj01 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME

		chara プットオブジェ Ｂ脚ロードエリア \
			-m 	d:KMS_NAME15 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｂ脚ロードエリア off

		chara プットオブジェ Ｃ脚ロードエリア \
			-m 	d:KMS_NAME16 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｃ脚ロードエリア off
	}

/*
	//	w15_d_common.hに移行

	proc 海面設置 {
		#if d:DEBUG_PRINT
			print 'w15a_sea_set'
		#endif

	//	朝
	#if d:W15A
		chara プラント海面 seaseasea \
			-tex oil m_oil_sea_alp_ovl_mod0222 m_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
			-pos -52000,-40000,-60000
	//	昼
	#else
		chara プラント海面 seaseasea \
			-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
			-pos -52000,-40000,-60000
	#endif

		chara 水面監視水飛沫 落下水飛沫 \
			-n d:PLAYER \
			-n 敵兵

		command 水位設定 -lv -40000
		command 弾水飛沫処理
	}

	proc 空設置 {
		#if d:DEBUG_PRINT
			print 'w15a_sky_set'
		#endif

	//	朝
	#if d:W15A
#if 0
		chara 昼空ドーム 朝 \
			-col 90 110 95 \
			-pos -52000,-40000,-60000	//中心の位置

		chara 太陽 ソル \
			-p -52000,-40000,-60000 \		//中心の位置
			-f 2200 -1400 4300				//平行光源の-pと同じ値
#endif

		@朝空	-52000 -40000 -60000 2200 -1400 4300

	//	昼
	#else
#if 0
		chara 昼空ドーム 昼 \
			-col 150 175 175 \
			-pos -52000,-40000,-60000	//中心の位置

		chara 太陽 ソル \
			-p -52000,-40000,-60000 \		//中心の位置
			-f 2200 -1400 4300				//平行光源の-pと同じ値
#endif

		@昼空	-52000 -40000 -60000 2200 -1400 4300

	#endif

	}
*/

	proc 水位設定 {
		command 水位設定 -lv -40000
	}

	proc 海面管理設定 {
		chara 水面監視水飛沫 落下水飛沫 \
			-n d:PLAYER \
			-n 敵兵
		command 弾水飛沫処理
	}

	proc 天井設定 {
		//	Ｂ脚天井
		chara 天井君 Ｂ脚 -m d:KMS_NAME5 -p 0,0,0  -l d:LT2_NAME -c 0,0,-46000 -a 5
		//	Ｃ脚天井
		chara 天井君 Ｃ脚 -m d:KMS_NAME8 -p 0,0,0  -l d:LT2_NAME -c 0,0,-74000 -a 4

	}


//###################################################################################
//									アイテム
//###################################################################################

	proc アイテム設定 {
		#if d:DEBUG_PRINT 
		print 'item_set'
		#endif

		@アイテム再セットチェック $b:w15a_取得ロード回数[0] 
		if ($f:再セットフラグ) {
			@チャフ_初期設置 チャフ:01 -47750 0 -60000 0		//	壊れ橋付け根
		}
	}

	proc アイテム取得時番号設定 $:ボックス名 {
	#if d:DEBUG_PRINT
		print '*** Item No Set ***'
	#endif

		if($:ボックス名 == チャフ:01) {
			eval($b:w15a_取得ロード回数[0] = $w:グローバルロード回数);
		}
	}


//###################################################################################
//									 エフェクト
//###################################################################################
/*
	//	w15_common.hに移行
	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

	//	朝
	#if d:W15A

	//	明るさ可変機能
	@明るさ可変機能 194 199 163 -20000 175000 ha002 0


	//	昼
	#else
	@明るさ可変機能 220 237 198 -15000 175000 ha002 0


	#endif

	}
*/

	proc エフェクト設定 {
		#if d:DEBUG_PRINT
			print 'effect_set'
		#endif

		#if d:BLUR
		chara ブラー okajibura 
		#endif
		#if d:Z_MANAGER
		chara Ｚフォーカス管理 Z_manager -z 5000 10000 -t 30
		mesg Ｚフォーカス管理 Z_manager パラメータ３ \
			1000 \
			6000 \
			30
		#endif
		#if d:BOKASHI
		chara 近景ぼかし near_deforcus \
			-plane	3 \
			-near	0 \
			-far	3000 
		chara 遠景ぼかし far_blur \
			-plane 2 \
			-near 50000 \
			-far 100000 \
			-enable
		#endif
		@プレイヤー振動設定 sna_w00a
	}

	proc キャラ用ライト設定 {

	//	朝
	#if d:W15A
		chara 環境光 Amb \
//			-c 10 25 15
			-c 33 53 50

		chara 平行光 Para \
//			-c 133 133 128 \
			-c 201 169 103 \
			-p 2200,-1400,4300

	//	昼
	#else
		chara 環境光 Amb \
			-c 55 63 38

		chara 平行光 Para \
			-c 163 164 150 \
//			-p -2000,-4000,2200
			-p -2600,-3900,1900

	#endif
	}

	proc ＢＣ連絡橋風設定 {
		@風設定 ca001
	}
	
	proc 影設定 {
		#if d:DEBUG_PRINT
		print'SHADOW'
		#endif

	#if d:W15A	//	朝のみ
		//	影写り用モデル
		chara 影投影モデル 影用モデル \
			-model w15a_sdw \
			-pos -1,1,-1

		//	影写り用ハザード　敵兵が影を発見
		command	影投影ハザード \
			-name ＢＣ連絡橋 \
			-group 11
		
		chara 影管理 影管理人 \
			-type 0 \
			-length 6000 \
//			-p 1000,-1200,500
			-p 7100,-7000,6900
		
		trap sh001 ？ \
			-mask  ？ \
			-exec {
				if ($3 == 入る) {
					mesg 影管理 影管理人 add $2 影用モデル
				} else if ($3 == 出る) {
					mesg 影管理 影管理人 del $2
				}
			}
	#endif
	}

	proc 煙設定 {

	#if d:W15A	//	朝のみ

#if 0
		//	上パイプ
		chara テスト昇り煙 煙１ \
			-radius		300 \	// 煙半径 500
			-pos		-48750 -1500 -67500	// 発生位置
#endif
		//	下パイプ
		chara テスト昇り煙 煙２ \
			-radius		250 \	// 煙半径 500
			-pos		-49000 -1500 -51500		// 発生位置

		//	壊れ接続部左
		chara テスト昇り煙 煙３ \
			-radius		125 \	// 煙半径 500
			-pos		-44250 -2000 -58750		// 発生位置

		//	壊れ接続部右
		chara テスト昇り煙 煙３ \
			-radius		125 \	// 煙半径 500
			-pos		-34000 -1750 -61500	// 発生位置
	#endif

	}

	proc 水設定 {
	#if d:W15A	//	朝のみ
		chara 送水管流水 ぱいぷみず \
			-unit 		12	\
			-vert 		16 	\
			-nring		16	\
			-pos		-48750 -1400 -69000 \
			-rot		64, 0, 0	\
			-fally		-10300	\
//			-fally		-6300	\
			-esprpos 	-48750 -1800 -68000 \
			-hsprrot	1424, 0, 0	\
			-jsprrad	200, 300 	\
			-offset		10, 0, 0	\
			-ang		128, 164, 0	\
			-initspd	50		\
			-speed		65		\
			-binitsize	5		\
			-daimsize	600		\
			-color		0x40404020	\
			-gravity	4
	#endif
	}

//###################################################################################
//									カメラ
//###################################################################################

	proc カメラセット {
		#if d:DEBUG_PRINT 
		print 'defaultcamera_set'
		#endif

			chara カメラ ステージカメラ	-c 0 -l 3 -p 0 
			chara カメラ 通路カメラ		-c 0 -l 2 -p 50 
			chara カメラ 小部屋カメラ	-c 0 -l 2 -p 40 
			chara カメラ ビヨンドカメラ	-c 0 -l 2 -p 20 
			chara カメラ 張り付きカメラ	-c 0 -l 2 -p 10 
			chara カメラ シナリオカメラ	-c 0 -l 2 -p 7
			chara カメラ ロードカメラ	-c 0 -l 1 -p 6
			chara カメラ ゲームオーバーカメラ	-c 0 -l 1 -p 5

			// デフォルトカメラ
			chara カメラ設定 ステージカメラ \
				-c 0 \	
				-b -32000,-32000,-32000 32000,32000,32000 \
				-l -32000,-32000,-32000 32000,32000,32000 \
				-r 640,2048,0 \
				-f 6000 \
				-a 200 \
				-i 1 -1 0 0 \
				-s 1
	}
	proc ＢＣ連絡橋カメラ設定 {
		#if d:DEBUG_PRINT 
		print 'camera_set'
		#endif
		//	連絡橋
		trap ca001 d:PLAYER \
			-camera \
			-mask ＊ \
			-exec {
				@CS_スムーズカメラ降順 $6 -46000 -60000 800 300
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				chara カメラ設定 通路カメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,-58811 d:CAM_MAX,11919,-46804 \
					-l d:CAM_MIN,d:CAM_MIN,-68666 d:CAM_MAX,d:CAM_MAX,-49640 \
					-r $i:補完ＲＸ,2048,0 \
					-f 11000 \
					-a 200 \
					-i 0 -1 0 0 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためリミットを見る
					-s $3
			}

		//	Ｂ脚
		trap ca002 d:PLAYER \
			-camera \
			-mask ？ \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \
					-b -54189,d:CAM_MIN,-44568 -51202, 5969,-44568 \
			 		-l -54189,d:CAM_MIN,-45351 -51202, 1347,-44768 \
					-r 913,2048,0 \
					-f 4689 \
					-a 200 \
					-i 0 -1 0 0 \
					-s $3
			}

		//	Ｃ脚
		trap ca003 d:PLAYER \
			-camera \
			-mask ？ \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \
					-b -56123,d:CAM_MIN,-74988 -51202, 5969,-74663 \
					-l -56123,d:CAM_MIN,-75783 -51202, 1347,-75457 \
					-r 913,2048,0 \
					-f 4689 \
					-a 200 \
					-i 0 -1 0 0 \
					-s $3
			}


		//	張り付き移動

		//	左側
		trap bh001 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 1024 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラ $3
			}

		//右側
		//	Ｂ脚側
		trap bh002 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラ $3
			}
		//	中央
		trap bh003 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラ $3
			}
		//	Ｃ脚側
		trap bh004 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 3072 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラ $3
			}

		//	壊れ付近
		//南側
		trap bh005 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 2048 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラ $3
			}
		//	北側
		trap bh006 d:PLAYER \
			-mask ？ \
			-state 8 \
			-dir 0 256 \	//方向指定
			-camera \			//カメラトラップであることを明示
			-exec {
				@張り付き移動カメラ $3
			}





	}

	proc 張り付き移動カメラ {
		chara カメラ設定 張り付きカメラ \
			-c 0 \	

//			-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//			-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \

			-b d:CAM_MIN,d:CAM_MIN,-70929 d:CAM_MAX,d:CAM_MAX,-47716 \
			-l d:CAM_MIN,d:CAM_MIN,-71554 d:CAM_MAX,d:CAM_MAX,-48341 \

			-r 973,2048,0 -f 8000 \
			-a 200 \
			-i 2 2 0 0 \
			-s $1 \
			-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
	}


//###################################################################################
//									ビヨンド
//###################################################################################

	proc ＢＣ連絡橋ビヨンド設定 {
		#if d:DEBUG_PRINT 
		print 'beyond_set'
		#endif

	//	北側
		//	エルード発動
		trap by001 d:PLAYER \
			-mask いる \
			-button 3 \		//ビヨンドボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@東向きビヨンドハイ発動 by001 -51000 $6 -67750 -62250 0x00002
			}
		//	発動カメラ
		trap by001 d:PLAYER \
			-camera \
			-mask いる \
			-state 6 \	// 強制モーション中
			-dir 1024 256 \	// 方向指定
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
		trap by001 d:PLAYER \
			-camera \
			-mask ？ \
			-state 5 \	// エルード中
			-dir 3072 256 \	// 方向指定
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
		trap by001 d:PLAYER \
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

	//	南側
		//	エルード発動
		trap by002 d:PLAYER \
			-mask いる \
			-button 3 \		//ビヨンドボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@東向きビヨンドハイ発動 by002 -51000 $6 -57750 -52250 0x00002
			}
		//	発動カメラ
		trap by002 d:PLAYER \
			-camera \
			-mask いる \
			-state 6 \	// 強制モーション中
			-dir 1024 256 \	// 方向指定
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
			-camera \
			-mask ？ \
			-state 5 \	// エルード中
			-dir 3072 256 \	// 方向指定
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

	//	中央棟連絡橋北側
		//	エルード発動
		trap by004 d:PLAYER \
			-mask いる \
			-button 3 \		//ビヨンドボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@北向きビヨンドハイ発動 by004 -61000 $4 -48750 -47000 0x00002
			}
		//	発動カメラ
		trap by004 d:PLAYER \
			-camera \
			-mask いる \
			-state 6 \	// 強制モーション中
			-dir 2048 256 \	// 方向指定
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,7731,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
					-r 537,2285,0 -f 6278 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by004 d:PLAYER \
			-camera \
			-mask ？ \
			-state 5 \	// エルード中
			-dir 0 256 \	// 方向指定
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 965,2285,0 -f 6248 \
					-a 247 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by004 d:PLAYER \
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

	//	中央棟連絡橋南側
		//	エルード発動
		trap by005 d:PLAYER \
			-mask いる \
			-button 3 \		//ビヨンドボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@南向きビヨンドハイ発動 by005 -59000 $4 -48750 -47000 0x00002
			}
		//	発動カメラ
		trap by005 d:PLAYER \
			-camera \
			-mask いる \
			-state 6 \	// 強制モーション中
			-dir 0 256 \	// 方向指定
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,7731,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
					-r 554,2348,0 -f 7999 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}
		//	エルードカメラ
		trap by005 d:PLAYER \
			-camera \
			-mask ？ \
			-state 5 \	// エルード中
			-dir 2048 256 \	// 方向指定
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 3922,2348,0 -f 6824 \
					-a 303 \
					-i 3 2 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
		//	張り付きカメラ
		trap by005 d:PLAYER \
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



	//	おっとっとエルード
		//	エルード発動
		trap by003 d:PLAYER \
			-mask 入る \
			-exec {
				command プレイヤー状態取得 ;
				if	( ($status == 0) || ($status & d:PFLAG_WALK) || \
					($status & d:PFLAG_DAMAGED) || ($status & d:PFLAG_SUBJECT) ) {
					@東向き片手ビヨンド発動 by003 -46750 $6 -61000 -59000 0 \
					(d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_NO_RETURNSTAMP )
				}
			}
		//	発動カメラ
		trap by003 d:PLAYER \
			-camera \
			-mask いる \
			-state 6 \	// 強制モーション中
			-dir 1024 256 \	// 方向指定
			-exec {
				chara カメラ設定 ビヨンドカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
/*					-r 600,2527,0 -f 8000 \
					-a 200 \
*/
					-r 204,1350,0 -f 6479 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1
			}

		//	エルードカメラ
		trap by003 d:PLAYER \
			-camera \
			-mask ？ \
			-state 5 \	// エルード中
			-dir 3072 256 \	// 方向指定
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


	}



//###################################################################################
//									 シナリオデモ
//###################################################################################

//	サイファー遭遇デモ開始チェック
	proc サイファー遭遇デモ開始チェック {
		//	デモはなし
		eval($f:w23a_サイファー遭遇デモフラグ = d:TRUE)

		if ($w:p_story >= d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) {
			eval($f:w23a_サイファー遭遇デモフラグ = d:TRUE)
		}
	}

//	サイファー遭遇デモ
	proc サイファー遭遇デモ設定 {
		//	ピーターと会ったあと
		if ($w:p_story >= d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
			trap sc001 d:PLAYER  \
				-mask 入る \
				-e {
					//	ＦＡ連絡橋でサイファー遭遇デモを見ていない場合
					if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
						//	サイファー遭遇デモフラグを立てる（危険モード時はデモは二度と見ない）
						eval($f:w23a_サイファー遭遇デモフラグ = d:TRUE)

						//	潜入モード時のみデモが流れる
						if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
							@サイファー遭遇デモ管理設定
						}

					}
				}
		}
	}

	proc サイファー遭遇デモ管理設定 {
		chara プロック連続実行 サイファー遭遇デモ管理人 \
		-p サイファー遭遇デモ管理
	}

	proc サイファー遭遇デモ管理 {
		command パッドチェック \
			-no_use
		//	パッドが戻ったら（ステージ開始処理が終了したら）
		if ($status != 1) {
			//	なぜかロードカメラ死なないので
			chara カメラ設定 ロードカメラ \
				-s -1
			@サイファー遭遇デモ
			mesg プロック連続実行 サイファー遭遇デモ管理人 kill
		}
	}

	proc サイファー遭遇デモ {

		//	シナリオデモ開始処理を行う
		@シナリオデモ開始処理

		//	パッドデモ再生＆デモ用サイファー起動
		chara パッドデモ サイファー遭遇デモライデン \
			-file  w15a_paddemo_n00 \
			-e サイファー遭遇デモ終了処理

		mesg パッドデモ サイファー遭遇デモライデン start

		chara delay サイファースタートディレイ \
			-t 90 \
			-e {
				mesg サイファ デモサイファー:01 ルート変更 5 1
			}

		//	キャンセルできるように
//		chara デモキャンセルチェック サイファー遭遇デモ -p サイファー遭遇デモ終了後処理
		chara デモキャンセルチェック サイファー遭遇デモ -p サイファー遭遇デモ終了処理

		//	パッドデモ終了
		chara delay パッドデモ終了ディレイ \
			-t 600 \
			-p サイファー遭遇デモ終了処理

	}

	proc サイファー遭遇デモ終了処理 {
		@rt_P009_01_R01_サイファー遭遇１無線デモ１
	}


	proc サイファー遭遇１無線デモ１終了処理 {
		@サイファー遭遇デモ終了後処理
	}

	proc サイファー遭遇デモ終了後処理 {
		//	登場ポイントを変更
		eval ($s:登場ポイント = サイファー遭遇デモ後)
		eval ($i:プレイヤー初期Ｘ位置 = -53500)
		eval ($i:プレイヤー初期Ｙ位置 = 0)
		eval ($i:プレイヤー初期Ｚ位置 = -75000)
		eval ($i:プレイヤー初期方向 =  2048)
		eval ($b:プレイヤー初期姿勢 = d:FA_END_STAND)

		//	シナリオデモ終了処理を行う
		@シナリオデモ終了処理

		mesg パッドデモ サイファー遭遇デモライデン kill
		mesg デモキャンセルチェック サイファー遭遇デモ kill

		restart -s
	}



//###################################################################################
//									ストーリーフラグ
//###################################################################################
	//	デモ専用ステージに依存


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
				#if d:DEBUG_PRINT
					print 'gameoverstart'
				#endif
				if ( $3 == 入る ) {
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3960,2864,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					@エルード落下死開始処理
				} else if ( $3 == 出る ) {
					@ゲームオーバー終了処理
					@エルード落下死終了処理
				}
			}

		trap de002 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				#if d:DEBUG_PRINT
					print 'gameoverstart'
				#endif
				if ( $3 == 入る ) {
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3960,2864,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					@エルード落下死開始処理
				} else if ( $3 == 出る ) {
					@ゲームオーバー終了処理	
					@エルード落下死終了処理
				}
			}

		trap de004 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				#if d:DEBUG_PRINT
					print 'gameoverstart'
				#endif
				if ( $3 == 入る ) {
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3852,3735,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					@エルード落下死開始処理
				} else if ( $3 == 出る ) {
					@ゲームオーバー終了処理	
					@エルード落下死終了処理
				}
			}

		trap de005 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				#if d:DEBUG_PRINT
					print 'gameoverstart'
				#endif
				if ( $3 == 入る ) {
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3852,2348,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-z d:CAMERA_NO_CUSHION \
						-s 1

					@エルード落下死開始処理
				} else if ( $3 == 出る ) {
					@ゲームオーバー終了処理	
					@エルード落下死終了処理
				}
			}
/*		
		//	ジャンプ落下死につなげるのでなし
		trap de003 d:PLAYER \
			-mask 入る \
			-state d:TRP_STATE_ELUDE_FALL \
			-exec {
				#if d:DEBUG_PRINT
					print 'gameoverstart'
				#endif
					chara カメラ設定 ゲームオーバーカメラ \
						-c 0 \
						-b d:CAM_MIN,-6000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3960,2864,0 -f 4340 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					@ジャンプ落下死発動 0x0004 -41000
			}
*/
	}


	//	ジャンプ落下死開始処理
	proc ジャンプ落下死開始処理 {
		chara カメラ設定 ゲームオーバーカメラ \
				-c 0 \
				-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 3800,2400,0 -f 4340 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1

		@ゲームオーバー開始処理
	}

	//	ジャンプ落下死終了処理
	//	fall.h内に記述　ゲームオーバー終了処理と同じ

	proc ジャンプ落下死設定 {

		trap de003 d:PLAYER \
			-mask 入る \
			-exec {
				#if d:DEBUG_PRINT
					print 'JUMPDEAD1'
				#endif
					command プレイヤー状態取得
						@ジャンプ落下死発動 0x0004 -41000

			}

	}



//###################################################################################
//									特殊処理
//###################################################################################
	proc 死体消し{
		chara カメラ設定 シナリオカメラ \
			-s -1
	}

	proc 死体捨て設定 {
		trap th001 d:PLAYER \
			-mask 入る \
			-state d:TRP_STATE_ENEMY_PULL \
			-exec {
				if (`@死体捨てチェック`) {
					#if d:DEBUG_PRINT
						print 'STROW'
					#endif
					//	死体捨て
					mesg プレイヤー d:PLAYER motion \
						d:モーションリスト:non_carry_body_throw1 \
						3072 \
						-1 \
						0 \
						(d:FA_NO_CHECK_SEG | d:FA_NO_CHECK_FLR | d:FA_NO_WEAPON) \
						-47250 -60500 //x,z,yの順


					mesg コマンダー 敵兵セット 引き釣られ兵消す
					command ゲット捕まえられ兵の名前 $i:敵兵モデル名


					//捨てられ敵兵
					/*指定された順にモーション再生後プロック呼んで消滅*/
					chara デモ人形 死体 \
						-pos (-47250 - 430 ),0,( -60500 + 69 ) \
						-dir 3072 \
						-code $i:敵兵モデル名 \
						-mar corpse_w15a \
						-list 0 \			//$i:再生順番 ... \ /*最大8個*/
						-end 死体消し	//$p:終了時プロック

					chara カメラ設定 シナリオカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 743,2392,0 -f 7133 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara delay 死体捨て水音ディレイ \
							-t 200 \
							-e {
//								command ＳＥセットモード -s d:se_code:SD_P_INWTERL1 \
								command ＳＥセットモード -s d:se_code:SD_E_INWTERL4 \
								-p	$i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ \
								-m 1
							}
				}
			}

	}






