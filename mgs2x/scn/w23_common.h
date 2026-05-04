/*
	w23_common.h                     
	    ＦＡ連絡橋

	2000/09/29 H.Yoshiike         
	$Id: w23_common.h,v 1.109 2001/10/29 12:10:29 usr03379 Exp $                      

	
*/

// 各種スイッチ用define
// ------------------------------------
// サウンド
#define		SOUND			1		// サウンドデータは最初に呼ぶ
#define		BGM				1		// BGMは敵兵コマンダーの後に呼ぶ
// キャラクター関係
#define		RAIDEN			1		// SNAKEとは排他
#define		SNAKE			0
#define		ENEMY			1		// コマンダーと警備兵(敵を出すときは必須)
#define		SUPPORT			0		// サポート兵
#define		ATTACKER		0		// 攻撃兵
#define		SHIELD			0		// 盾兵(1) or 通常攻撃兵(0)
// 監視カメラ関係
#define		CYPHER			1		// サイファー
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
#define		CHARA_CEIL		0		// 天井君
// 透明壁
#define		TR_WALL			0		// シナリオ置きの透明壁
// ライト関係
#define		SPOT_LIGHT		0		// スポットライト
#define		CHARA_LIGHT		1		// キャラ専用ライト
// オブジェ関係
#define		SEA				1		// 海面
#define		SKY				1		// 空

// アイテム
#define		ITEM			1
// 強制モーション
#define		BY_MOTION		1		// ビヨンドモーション

//	デモ
#define		SCN_DEMO		1		//	シナリオデモ

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



#define		MTN_REC			0		//	モーションレコーダー


// 各種設定用define
// ------------------------------------
// マップ関係

//	ＡＢ個別
#if d:W23A
	//	ステージ
	#define		KMS_NAME0			w23a
	//	ライト
	#define		LT2_NAME			w23a

	#define		KMS_NAME9			w23a_obj00	//	Ｆ脚１階

#else
	//	ステージ
	#define		KMS_NAME0			w23b
	//	ライト
	#define		LT2_NAME			w23b

	#define		KMS_NAME9			w23b_obj00	//	Ｆ脚１階

#endif

//	ＡＢ共通
#define		KMS_NAME1			w23_12a			//	Ａ脚天井

#define		KMS_NAME2			w23_13a			//	ＡＢ連絡橋
#define		KMS_NAME3			w23_15a			//	ＢＣ連絡橋
#define		KMS_NAME4			w23_19a			//	ＤＥ連絡橋
#define		KMS_NAME5			w23_20a			//	Ｅ脚
#define		KMS_NAME6			w23_21a			//	ＥＦ連絡橋
#define		KMS_NAME7			w23_24a			//	シェル１中央棟

#define		KMS_NAME8			w23_22a			//	Ｆ脚天井
#define		KMS_NAME10			w23_15b			//	ＢＣ連絡橋破壊前
#define		KMS_NAME11			w23_15c			//	ＢＣ連絡橋破壊後

#define		KMS_NAME12			w23_crane		//	クレーン
#define		KMS_NAME13			w23_other00		//	浮き橋３
#define		KMS_NAME14			w23_other01		//	浮き橋２

#define		KMS_NAME_null		null			//	ダミー

//	アタリ
#define		HZX_NAME			w23a
										//	0		プレイヤーアタリ／跳弾アタリ
										//	1〜8	遠景跳弾アタリ
										//	9		影用アタリ
										//	10		レンズフレア用アタリ

//	ドア
#define		DOOR_NAME_LOAD1		w23a_dr00		// ロード用
#define		DOOR_NAME_LOAD2		w23a_dr01		// ロード用
#define		DOOR_NAME_LOAD3		w23a_dr02		// ロード用

//	プットオブジェおき
#define		KMS_NAME15			w23_22b			//	Ｆ脚１Ｆロードエリア
#define		KMS_NAME16			w23_22c			//	Ｆ脚Ｂ１ロードエリア
#define		KMS_NAME17			w23_12b			//	Ａ脚ロードエリア

// キャラクター関係
#if d:RAIDEN
	#define		PLAYER		ライデン	// モーションができるまで名前はスネーク
#else
	#define		PLAYER		スネーク
#endif

//	強制
#define		BY_HIGH			1		// 高所ビヨンドモーション

//	薬莢系
#if d:W23A
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
#if d:W23A
	#define		STAGE_W23A			1	//	朝
#else
	#define		STAGE_W23B			1	//	昼
#endif

#define			STAGE_PLANT			1	//	プラント編


// ローカル変数
//	$b:w23a_敵配置フラグ	プレイヤーがどちらから来たかか敵配置を変える
#define		STRUT_A			0		//	Ａ脚から来たときの敵配置
#define		STRUT_F_1F		1		//	Ｆ脚１階から来たときの敵配置
#define		STRUT_F_B1		2		//	Ｆ脚Ｂ１から来たときの敵配置

//	$f:w23a_サイファー遭遇デモフラグ	//	サイファー遭遇デモを見たか
//	$f:w23a_サイファー01破壊フラグ 		//	サイファー:01を破壊したかどうか
//	$f:w23a_サイファー02破壊フラグ 		//	サイファー:02を破壊したかどうか


//	その他
#define CTDOWN_TIMER_P		1		//	爆弾イベント用

// モーション関係
// ------------------------------------
#include "rai_w23a.mh"

// include
// ------------------------------------
#include "stdch.h"
#include "sload.h"
#include "beyond.h"
#include "fall.h"
#include "sound.h"
#include "enevoice.h"
#include "item_info.h"
#include "obl_trap.h"
#include "w23_kamome.h"
#include "ene_item.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif


// マップ設定
// ------------------------------------
//	プレイヤー用マップ
chara マップ ＦＡ連絡橋	-k d:KMS_NAME0 \
						-k d:KMS_NAME2 \
						-k d:KMS_NAME3 \
						-k d:KMS_NAME4 \
						-k d:KMS_NAME5 \
						-k d:KMS_NAME6 \
						-k d:KMS_NAME7 \
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

//	グループ１０はレンズフレア用ダミー
command 追加ＨＺＸグループ登録 \
	-hzx_id 10 -add_id 0

//	朝はシェル１中央棟/クレーンも見る
#if d:W23A
	command 追加ＨＺＸグループ登録 \
		-hzx_id 10 -add_id 5
#endif

//	グループ９は影用


/*	シナリオプレビュー用*/
#include "scn_demo_prvw.h"

	//	デバッグ用	必ず消すこと！
//	eval($w:p_story = d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了)
//	eval($w:p_story = d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了)
	//	デバッグ用	必ず消すこと！
//	if ($w:p_story >= d:ST:P010_01_P01ヴァンプ遭遇１ポリゴンデモ１開始) {
//		eval ($f:w23a_サイファー遭遇デモフラグ = 1);
//	}

command マップ設定 ＦＡ連絡橋 -set {
	//	敵配置チェック
	@敵配置チェック
	//	デモが起きるかどうかのチェック
	@サイファー遭遇デモ開始チェック
#if d:SOUND	
	@サウンド設定	// データは最初に呼ぶ
#endif
	//	ノードアクセス関連
	if ( $f:w22a_ノードフラグ == 1 || $f:w12b_ノードフラグ == 1) {
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
	@モード切替え判定
	@コマンダー設定
#endif
#if d:CYPHER
	@サイファー配置チェック		//	サイファー復活用
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
	@ＦＡ連絡橋ロード設定
#endif
	@オブジェクト設定
#if d:SEA
	@海面設定
#endif
#if d:SKY
	@空設定
#endif
	@天井設定

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
	@ＦＡ連絡橋風設定
	@影設定
	@カメラセット
#if d:BY_MOTION
	@ＦＡ連絡橋ビヨンド設定
#endif
	@ＦＡ連絡橋カメラ設定

	@エルード落下死設定

#if d:SCN_DEMO
	@サイファー遭遇デモ設定
	@シナリオデモキャラセット
# endif

#if d:CHARA_LIGHT
	@キャラ用ライト設定		// マップ設定の最後に呼ぶ
#endif

	@爆弾タイマー設置	//	爆弾イベント用
	@プラント編全体マップ設定  d:ＦＡ連絡橋マップ	//	全体マップ表示

#if d:DEBUG_CHARA
//	@デバッグキャラセット
#endif
#if d:MTN_REC
	@シナリオデモ作成用
# endif

}

// 無線用include ※位置は動かさないこと
#include "w23_cdc.ch"
#include "p_force_cdc.ch"

//初期マップの表示
command マップ表示 -show ＦＡ連絡橋


// 全てのセットが終わったところでフェードイン
@ＦＡ連絡橋開始時処理

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
#if d:W23A
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

		//	水蒸気
		command 弾痕ノーマル -n 6
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
	}

	proc プラグイン設定 {
		#if d:DEBUG_PRINT
			print 'plugin_set'
		#endif
	}

//###################################################################################
//									 プレイヤー
//###################################################################################

	proc プレイヤー設定 {
		#if d:DEBUG_PRINT
			print 'player_set'
		#endif

		#if d:RAIDEN
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s $b:プレイヤー初期姿勢 \
				-m rai_def \
				-a raiden \
				-o rai_w23a
//				-f d:PLY_NO_FOOT_SHADOW

			//	髪の毛
			@ライデン髪の毛設定 d:NORMAL_HAIR

			//	ＬＯＤ機能
			@プレイヤーＬＯＤ設定 d:C_DISTANCE_TYPE 5000
//			@プレイヤーＬＯＤ設定 d:P_STATE_TYPE d:TRP_STATE_BEHIND

		#else if d:SNAKE
			chara プレイヤー d:PLAYER \
				-p $i:プレイヤー初期Ｘ位置,$i:プレイヤー初期Ｙ位置,$i:プレイヤー初期Ｚ位置 \
				-d 0,$i:プレイヤー初期方向,0 \
				-s $b:プレイヤー初期姿勢 \
				-m sna_def -a snake -o rai_w23a

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
//									 敵兵
//###################################################################################

	proc 敵配置チェック {
//		自分の来た場所で敵配置変更
		if ($s:登場ポイント == サイファー遭遇デモ終了後) {
			//	配置はそのまま
		} else {
			if ( $s:登場ポイント == sp_w12b0_w23a_0) {
				eval($b:w23a_敵配置フラグ = d:STRUT_A)
			} else if ( $s:登場ポイント == sp_w22a0h_w23a_0) {
				eval($b:w23a_敵配置フラグ = d:STRUT_F_1F)
			} else if ( $s:登場ポイント == sp_w22a1c_w23a_0) {
				eval($b:w23a_敵配置フラグ = d:STRUT_F_B1)
			}
		}
	}

//	敵兵配置

//	基本セット

	//	警備兵
	proc 警備兵セット {

		//	ストーリーフラグや難易度で分岐

		//	エイムズ死亡まで
		if ($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {

			//	ハード以下
			if ($w:ゲーム設定 <= d:LEVEL_HARD) {
				//	ピーターと会うまで
				if ($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
					//	警備兵いない	//

				//	ピーターと会った後
				} else {
					@警備兵０	//	敵１人	１Ｆ〜Ｂ１
				}

			//	エクストリーム
			} else {
				//	ピーターと会うまで（警備兵いる）
				if ($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
				print '#################################################################################watcher1'
					@警備兵１	//	警備兵１人	１Ｆ〜Ｂ１その２（両端まで）

				//	ピーターと会った後
				} else {
					@警備兵１	//	警備兵１人	１Ｆ〜Ｂ１その２（両端まで）
				}
			}

		//	エイムズ死亡後
		} else {

			//	エクストリーム
			if ($w:ゲーム設定 >= d:LEVEL_EXTREME) {
				@警備兵１	//	警備兵１人	１Ｆ〜Ｂ１その２（両端まで）
			}

		}
	}


	//	@警備兵０	//	敵１人	１Ｆ〜Ｂ１
	//	@警備兵１	//	警備兵１人	１Ｆ〜Ｂ１その２（両端まで）



	//	攻撃兵
	proc 攻撃兵セット {

		//	ストーリーフラグや難易度で分岐

		//	エイムズ死亡まで
		if ($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {

			//	ピーターと会うまで（警備兵いない場合）
			if ($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {

				//	イージー以下
				if ($w:ゲーム設定 <= d:LEVEL_EASY) {
					@攻撃兵１１	//	敵１人	ノーマル（巡回）

				//	ハード以下
				} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
					@攻撃兵１０	//	敵２人	ノーマル（巡回）

				//	エクストリーム（警備兵いる）
				} else {
//					@攻撃兵Ｃ	//	敵４人	ノーマル（巡回）
//					@攻撃兵Ｅ	//	敵３人	ノーマル（巡回）
					@攻撃兵Ｆ	//	敵３人	ノーマル（サポート化）
				}

			//	ピーターと会った後（警備兵いる場合）
			} else {

				//	イージー以下
				if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//					@攻撃兵４	//	敵１人	ノーマル
					@攻撃兵８	//	敵１人	ノーマル（サポート兵化）

				//	ハード以下
				} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//					@攻撃兵０	//	敵２人	ノーマル
					@攻撃兵６	//	敵２人	ノーマル（サポート兵化）

				//	エクストリーム
				} else {
					@攻撃兵Ａ	//	敵４人	ノーマル（サポート兵化）
				}
			}

		//	エイムズ死亡後
		} else {

			//	イージー以下
			if ($w:ゲーム設定 <= d:LEVEL_EASY) {
//				@攻撃兵５	//	敵１人	ハイテクノーマル
				@攻撃兵１３	//	敵１人	ハイテクノーマル（巡回）

			//	ハード以下
			} else if ($w:ゲーム設定 <= d:LEVEL_HARD) {
//				@攻撃兵３	//	敵２人	ハイテクノーマル
				@攻撃兵１２	//	敵２人	ハイテクノーマル（巡回）

			//	エクストリーム（エクストリームのときは警備兵いる）
			} else {
				@攻撃兵Ｂ	//	敵４人	ハイテクノーマル（サポート兵化）
			}

		}
	}


	//	@攻撃兵０	//	敵２人	ノーマル
	//	@攻撃兵１	//	敵２人	盾兵
	//	@攻撃兵２	//	敵２人	ショットガン
	//	@攻撃兵３	//	敵２人	ハイテクノーマル
	//	@攻撃兵４	//	敵１人	ノーマル
	//	@攻撃兵５	//	敵１人	ハイテクノーマル

	//	@攻撃兵６	//	敵２人	ノーマル（サポート兵化）
	//	@攻撃兵７	//	敵２人	ハイテクノーマル（サポート兵化）
	//	@攻撃兵８	//	敵１人	ノーマル（サポート兵化）
	//	@攻撃兵９	//	敵１人	ハイテクノーマル（サポート兵化）

	//	@攻撃兵１０	//	敵２人	ノーマル（巡回）
	//	@攻撃兵１１	//	敵１人	ノーマル（巡回）
	//	@攻撃兵１２	//	敵２人	ハイテクノーマル（巡回）
	//	@攻撃兵１３	//	敵１人	ハイテクノーマル（巡回）

	//	@攻撃兵Ａ	//	敵４人	ノーマル（サポート兵化）
	//	@攻撃兵Ｂ	//	敵４人	ハイテクノーマル（サポート兵化）
	//	@攻撃兵Ｃ	//	敵４人	ノーマル（巡回）
	//	@攻撃兵Ｄ	//	敵４人	ハイテクノーマル（巡回）

	//	@攻撃兵Ｅ	//	敵３人	ノーマル（巡回）
	//	@攻撃兵Ｆ	//	敵３人	ノーマル（サポート化）


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

		chara 攻撃コマンダー atcomm -e { 攻撃兵セット }  -status 0x01
	}

	//	コマンダー

	//	司令官
	resource ghq_area_alert_voice {
	w23a:
		$t{
			// PLAREA0
			t:vc040232,	//	指揮官　　「了解。応援部隊をＦＡ連絡橋に派遣する。敵を包囲せよ」
			t:vc040233,	//	指揮官　　「了解。増援部隊、ＦＡ連絡橋へ急行！侵入者を逃がすな！」
			-2,
			// PLAREA1
			t:vc040230,	//	指揮官　　「了解。応援部隊、ＦＡ連絡橋・Ａ脚入口へ向かえ！敵を逃がすな！」
			t:vc040231,	//	指揮官　　「了解。増援部隊、ＦＡ連絡橋へ急行！Ａ脚入口を守れ！」
			-2,
			// PLAREA2
			t:vc040228,	//	指揮官　　「了解。応援部隊、ＦＡ連絡橋・Ｆ脚入口へ向かえ！敵を逃がすな！」
			t:vc040229,	//	指揮官　　「了解。増援部隊、ＦＡ連絡橋へ急行！Ｆ脚入口を守れ！」
			-1
		}
	}

	resource ghq_caution_voice {

	//	攻撃兵が残っていない場合
	w23a:
		$t{
			t:vc040601,	//	指揮官　　「了解、総員、警戒を強化せよ」

			t:vc040491,	//	指揮官　　「攻撃チームは引き返せ、警戒チームはそのまま現場に残れ」
			t:vc040492,	//	指揮官　　「攻撃チームは撤収、警戒チームは警戒を続行せよ」
			t:vc040493,	//	指揮官　　「攻撃チーム帰投せよ、警戒チームはそのまま警戒に当たれ」
			t:vc040496,	//	指揮官　　「攻撃チーム、配置に戻れ」
			t:vc040497,	//	指揮官　　「攻撃チーム、撤収せよ」
			-2,
			t:vc040431,	//	指揮官　　「ＦＡ連絡橋との連絡が途絶えた」
			-3,
			t:vc040611,	//	カメラ　　「異状発生…」
			-1

		}

	//	攻撃兵が残っている場合
	w23a-2:
		$t{
			t:vc040389,	//	指揮官　　「了解。ＦＡ連絡橋に増員を派遣する。警戒を強化しろ」

			t:vc040385,	//	指揮官　　「了解。ＦＡ連絡橋・Ｆ脚入口に増員。警備を強化しろ」
			t:vc040386,	//	指揮官　　「了解。ＦＡ連絡橋に増員。Ｆ脚入口を固めろ」

			t:vc040387,	//	指揮官　　「了解。ＦＡ連絡橋・Ａ脚入口に増員。警戒を強化しろ」
			t:vc040388,	//	指揮官　　「了解。ＦＡ連絡橋に増員。Ａ脚入口を固めろ」
			-2,
			t:vc040431,	//	指揮官　　「ＦＡ連絡橋との連絡が途絶えた」
			-3,
			t:vc040611,	//	カメラ　　「異状発生…」
			-1
		}
	}




	proc コマンダー設定 {
		#if d:DEBUG_PRINT
			print 'commander_set'
		#endif


		//	エイムズ死亡前
		if ($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {

			//	ピーターと会うまで（警備兵いない場合）
			if ($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {

				//	ハード以下（このときは攻撃兵のみ）
				if ($w:ゲーム設定 <= d:LEVEL_HARD) {
					chara コマンダー 敵兵セット \
						-a 攻撃兵コマンダー \
						-r -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-c -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-v [ene_plant_voice:02] \
						-x [ghq_area_alert_voice:w23a] \
						-y [ghq_caution_voice:w23a-2] \
						-z $i:敵標準麻酔持続 \
						-o $i:敵標準気絶持続 \
						-b $w:敵標準再発生数 \
						-s d:ENE_STAGE_GPS \
						-m $w:スタートアラートモード \
						-h $i:標準警戒時間		// 警戒時間

				//	エクストリーム（警備兵いる）
				} else {
					chara コマンダー 敵兵セット \
						-w 警備兵コマンダー \
						-a 攻撃兵コマンダー \
						-r -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-c -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-v [ene_plant_voice:01] \
						-x [ghq_area_alert_voice:w23a] \
						-y [ghq_caution_voice:w23a-2] \
						-z $i:敵標準麻酔持続 \
						-o $i:敵標準気絶持続 \
						-b $w:敵標準再発生数 \
						-s d:ENE_STAGE_GPS \
						-m $w:スタートアラートモード \
						-h $i:標準警戒時間		// 警戒時間
				}

			//	ピーターと会った後（警備兵いる場合）
			} else {
				chara コマンダー 敵兵セット \
					-w 警備兵コマンダー \
					-a 攻撃兵コマンダー \
					-r -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
					-c -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
					-v [ene_plant_voice:01] \
					-x [ghq_area_alert_voice:w23a] \
					-y [ghq_caution_voice:w23a-2] \
					-z $i:敵標準麻酔持続 \
					-o $i:敵標準気絶持続 \
					-b $w:敵標準再発生数 \
					-s d:ENE_STAGE_GPS \
					-m $w:スタートアラートモード \
					-h $i:標準警戒時間		// 警戒時間
			}

		//	エイムズ死亡後
		} else {

				//	ハード以下（このときは攻撃兵のみ）
				if ($w:ゲーム設定 <= d:LEVEL_HARD) {
					chara コマンダー 敵兵セット \
						-a 攻撃兵コマンダー \
						-r -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-c -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-v [ene_plant_voice:02] \
						-x [ghq_area_alert_voice:w23a] \
						-y [ghq_caution_voice:w23a-2] \
						-z $i:敵標準麻酔持続 \
						-o $i:敵標準気絶持続 \
						-b $w:敵標準再発生数 \
						-s d:ENE_STAGE_GPS \
						-b $w:敵標準再発生数 \
						-m $w:スタートアラートモード \
						-h $i:標準警戒時間		// 警戒時間

				//	エクストリーム（このときは警備兵いる）
				} else {
					chara コマンダー 敵兵セット \
						-w 警備兵コマンダー \
						-a 攻撃兵コマンダー \
						-r -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-c -28875,0,-375 ＦＡ連絡橋 26500,0,5500 ＦＡ連絡橋 25000,-4500,-3500 ＦＡ連絡橋 \
						-v [ene_plant_voice:01] \
						-x [ghq_area_alert_voice:w23a] \
						-y [ghq_caution_voice:w23a-2] \
						-z $i:敵標準麻酔持続 \
						-o $i:敵標準気絶持続 \
						-b $w:敵標準再発生数 \
						-s d:ENE_STAGE_GPS \
						-m $w:スタートアラートモード \
						-h $i:標準警戒時間		// 警戒時間
				}

		}

	}





//	警備兵セット

	//	警備兵１人	１Ｆ〜Ｂ１
	proc 警備兵０ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara 警備兵 敵兵:01 -r 11 -d 13000,0,0 ＦＡ連絡橋 -n 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w23a_guard_ve_01] [dogtag:plt_w23a_guard_ea_01] [dogtag:plt_w23a_guard_no_01] \
					[dogtag:plt_w23a_guard_ha_01] [dogtag:plt_w23a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			chara 警備兵 敵兵:01 -r 10 -d -13000,0,0 ＦＡ連絡橋 -n 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w23a_guard_ve_01] [dogtag:plt_w23a_guard_ea_01] [dogtag:plt_w23a_guard_no_01] \
					[dogtag:plt_w23a_guard_ha_01] [dogtag:plt_w23a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			chara 警備兵 敵兵:01 -r 10 -d -13000,0,0 ＦＡ連絡橋 -n 1 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w23a_guard_ve_01] [dogtag:plt_w23a_guard_ea_01] [dogtag:plt_w23a_guard_no_01] \
					[dogtag:plt_w23a_guard_ha_01] [dogtag:plt_w23a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		}
	}

	//	警備兵１人	１Ｆ〜Ｂ１その２（両端まで）
	proc 警備兵１ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara 警備兵 敵兵:01 -r 15 -d 13000,0,0 ＦＡ連絡橋 -n 0 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w23a_guard_ve_01] [dogtag:plt_w23a_guard_ea_01] [dogtag:plt_w23a_guard_no_01] \
					[dogtag:plt_w23a_guard_ha_01] [dogtag:plt_w23a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			chara 警備兵 敵兵:01 -r 15 -d -13000,0,0 ＦＡ連絡橋 -n 7 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w23a_guard_ve_01] [dogtag:plt_w23a_guard_ea_01] [dogtag:plt_w23a_guard_no_01] \
					[dogtag:plt_w23a_guard_ha_01] [dogtag:plt_w23a_guard_vh_01] \
				-q 引きずり１ $b:アイテム率１ 引きずり２ $b:アイテム率２ 引きずり３ $b:アイテム率３ \
				-a ホールドアップ１ $b:アイテム率１ ホールドアップ２ $b:アイテム率２ ホールドアップ３ $b:アイテム率３
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			chara 警備兵 敵兵:01 -r 15 -d -13000,0,0 ＦＡ連絡橋 -n 3 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-j ドッグタグ_敵兵 \
					[dogtag:plt_w23a_guard_ve_01] [dogtag:plt_w23a_guard_ea_01] [dogtag:plt_w23a_guard_no_01] \
					[dogtag:plt_w23a_guard_ha_01] [dogtag:plt_w23a_guard_vh_01] \
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
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
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
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_SHIELD	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
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
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_SHOTGUN	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
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
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
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
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
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

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
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
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
	}

	//	ハイテクノーマル２人（サポート兵化）
	proc 攻撃兵７ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif

			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
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
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}

	//	ハイテクノーマル１人（サポート兵化）
	proc 攻撃兵９ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
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
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 11 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 10 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}

		chara アタッカー 敵兵:62 \
			-r 6 \
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
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 11 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 10 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}

	//	ハイテクノーマル２人（巡回）
	proc 攻撃兵１２ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 11 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 10 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		}

		chara アタッカー 敵兵:62 \
			-r 6 \
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

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:61 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 11 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-c 10 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
		}
	}


	//	ノーマル４人（サポート兵化）
	proc 攻撃兵Ａ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif
			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

			chara アタッカー 敵兵:63 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

			chara アタッカー 敵兵:64 \
				-r 14 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

	}

	//	ハイテクノーマル４人（サポート兵化）
	proc 攻撃兵Ｂ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
		#endif

			chara アタッカー 敵兵:61 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-s 0x40000000 \		//	サポート兵化
				-b 敵兵:01 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

			chara アタッカー 敵兵:62 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

			chara アタッカー 敵兵:63 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

			chara アタッカー 敵兵:64 \
				-r 14 \
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
		chara アタッカー 敵兵:61 \
			-r 5 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 11 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 6 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 10 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:63 \
			-r 5 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:64 \
			-r 14 \
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
			-r 5 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 11 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 6 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 10 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:63 \
			-r 5 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ

		chara アタッカー 敵兵:64 \
			-r 14 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_HITECH_1	//	$w:装備タイプ
	}

	//	ノーマル３人（巡回）
	proc 攻撃兵Ｅ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
			print 'attacker_E'
		#endif

		chara アタッカー 敵兵:61 \
			-r 5 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 11 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 6 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-c 10 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:63 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			chara アタッカー 敵兵:63 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:63 \
				-r 14 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}

	//	ノーマル３人（サポート化）
	proc 攻撃兵Ｆ {
		#if d:DEBUG_PRINT
			print 'attacker_set'
			print 'attacker_F'
		#endif

		chara アタッカー 敵兵:61 \
			-r 5 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-s 0x40000000 \		//	サポート兵化
			-b 敵兵:01 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		chara アタッカー 敵兵:62 \
			-r 6 \
			-h $w:敵標準聴力 \
			-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
			-l $w:敵標準体力 \
			-f $w:敵標準気絶 \
			-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			chara アタッカー 敵兵:63 \
				-r 5 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			chara アタッカー 敵兵:63 \
				-r 6 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		} else {
			chara アタッカー 敵兵:63 \
				-r 14 \
				-h $w:敵標準聴力 \
				-i $w:敵潜入視力,$w:敵危険視力,$w:敵回避視力 \
				-l $w:敵標準体力 \
				-f $w:敵標準気絶 \
				-e d:ENE_EQUIP_TYPE_NORMAL	//	$w:装備タイプ
		}
	}



//	サイファーセット

	//	再セットチェック
	proc サイファー配置チェック {
		//	壊れ物再セットチェック
		print '+++++++++++++++++++++++++++++++++++++++++++++++++CYPHER' $w:グローバルロード回数
//		@壊れ物再セットチェック $b:w23a_破壊ロード回数[0]
		@サイファー再セットチェック $b:w23a_破壊ロード回数[0]

		if ($f:再セットフラグ) {
			print 'CYPHER01_REBIRTH'
			eval ($f:w23a_サイファー01破壊フラグ = d:FALSE)
		}

//		@壊れ物再セットチェック $b:w23a_破壊ロード回数[1]
		@サイファー再セットチェック $b:w23a_破壊ロード回数[1]

		if ($f:再セットフラグ) {
			print 'CYPHER02_REBIRTH'
			eval ($f:w23a_サイファー02破壊フラグ = d:FALSE)
		}
	}

	//	破壊プロック
	proc サイファー破壊 {

		if ($1 == サイファー:01) {
			print 'CYPHER01_DESTORY'
			eval ($b:w23a_破壊ロード回数[0] = $w:グローバルロード回数)
			eval ($f:w23a_サイファー01破壊フラグ = d:TRUE)
			print '##########################################CYPHER01' $b:w23a_破壊ロード回数[0]

		} else if ($1 == サイファー:02) {
			print 'CYPHER02_DESTORY'
			eval ($b:w23a_破壊ロード回数[1] = $w:グローバルロード回数)
			eval ($f:w23a_サイファー02破壊フラグ = d:TRUE)
		}

	}


	proc サイファー設定 {

		#if d:DEBUG_PRINT 
			print 'cypher_set'
		#endif

		//	フォーチュン戦まで
		if ($w:p_story < d:ST:P029_01フォーチュン戦開始) {
			//	サイファー１機
			if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
				//	潜入モード時はデモ、それ以外は通常ゲーム
				if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
					//	デモ用サイファー
					print 'DEMO_CYPHER'
					@デモ用サイファー１
				} else {
					//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@ゲーム用サイファー１
				}
			} else {
				if ($s:登場ポイント == サイファー遭遇デモ終了後) {
				//	デモ直後サイファー
					print 'GAME_CYPHER'
					@デモ直後サイファー１
				} else {
				//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@ゲーム用サイファー１
				}
			}

		//	ファットマン戦まで
		} else if ($w:p_story < d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {
		//	サイファー出ない

		//	変装イベント終了まで
		} else if ($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
			//	サイファー１機
			if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
				//	潜入モード時はデモ、それ以外は通常ゲーム
				if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
					//	デモ用サイファー
					print 'DEMO_CYPHER'
					@デモ用サイファー１
				} else {
					//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@ゲーム用サイファー１
				}
			} else {
				if ($s:登場ポイント == サイファー遭遇デモ終了後) {
				//	デモ直後サイファー
					print 'GAME_CYPHER'
					@デモ直後サイファー１

				} else {
				//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@ゲーム用サイファー１
				}
			}

		//	その後警戒モード
		} else {
		//	サイファー２機
			if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
				//	潜入モード時はデモ、それ以外は通常ゲーム
				if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
					//	デモ用サイファー
					print 'DEMO_CYPHER'
					@デモ用サイファー２
				} else {
					//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@ゲーム用サイファー２
				}
			} else {
				if ($s:登場ポイント == サイファー遭遇デモ終了後) {
				//	デモ直後サイファー
					print 'GAME_CYPHER'
					@デモ直後サイファー２

				} else {
				//	ゲーム用サイファー
					print 'GAME_CYPHER'
					@ゲーム用サイファー２
				}
			}
		}

	}

	//	デモ用サイファー
	proc デモ用サイファー１ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) { 
			chara サイファ  デモサイファー:01 \
				-route 7 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			chara サイファ  デモサイファー:01 \
				-route 8 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			chara サイファ  デモサイファー:01 \
				-route 9 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
		}
	}

	proc デモ用サイファー２ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) { 
			chara サイファ  デモサイファー:01 \
				-route 7 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
			chara サイファ  デモサイファー:02 \
				-route 1 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			chara サイファ  デモサイファー:01 \
				-route 8 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
			chara サイファ  デモサイファー:02 \
				-route 3 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			chara サイファ  デモサイファー:01 \
				-route 9 0 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
			chara サイファ  デモサイファー:02 \
				-route 3 5 \ 
				-type 0 \ /*0-NORMAL,1-GUN*/
				-i 6000 \
				-home	 140000 10000 0 \
						-140000 10000 0 \
				-speed 25
		}
	}

	//	デモ直後サイファー
	proc デモ直後サイファー１ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) { 
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 2 2 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 2 5 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 0 5 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		}
	}

	proc デモ直後サイファー２ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) { 
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 1 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
			if ($f:w23a_サイファー02破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:02 \
					-route 2 2 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 2 5 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
			if ($f:w23a_サイファー02破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:02 \
					-route 3 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 0 5 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
			if ($f:w23a_サイファー02破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:02 \
					-route 3 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		}
	}

	//	ゲーム用サイファー
	proc ゲーム用サイファー１ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) { 
		#if d:DEBUG_PRINT 
		print 'cypher_set_A'
		#endif	
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 2 3 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
		#if d:DEBUG_PRINT 
		print 'cypher_set_F_1F'
		#endif
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 2 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			#if d:DEBUG_PRINT 
			print 'cypher_set_F_B1'
			#endif
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 0 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		}
	}

	proc ゲーム用サイファー２ {
		if ($b:w23a_敵配置フラグ == d:STRUT_A) { 
		#if d:DEBUG_PRINT 
		print 'cypher_set_A'
		#endif
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 1 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
			if ($f:w23a_サイファー02破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:02 \
					-route 2 3 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
		#if d:DEBUG_PRINT 
		print 'cypher_set_F_1F'
		#endif
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 2 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
			if ($f:w23a_サイファー02破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:02 \
					-route 3 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_B1) {
			#if d:DEBUG_PRINT 
			print 'cypher_set_F_B1'
			#endif
			if ($f:w23a_サイファー01破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:01 \
					-route 0 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
			if ($f:w23a_サイファー02破壊フラグ == d:FALSE) {
				chara サイファ  サイファー:02 \
					-route 3 0 \ 
					-type 0 \ /*0-NORMAL,1-GUN*/
					-i 6000 \
					-p サイファー破壊 \
					-home	 140000 10000 0 \
							-140000 10000 0 \
					-speed 25
			}
		}
	}



//###################################################################################
//									 ＢＧＭ
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

		chara ドア Ａ脚ドア \
			-m d:DOOR_NAME_LOAD1 \
			-d 0,1024,0 \
			-p -19125,0,-5750 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＦＡ連絡橋 ＦＡ連絡橋 \
			-A Ａ脚ドアランプ \
			-exec {
				if ( !($:開閉フラグ) ){
					mesg プットオブジェ Ａ脚ロードエリア off
				} else {
					mesg プットオブジェ Ａ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ａ脚ドアランプ 1024 -19125 0 -5750 0
		trap dr001 ？ \
			-mask ？ \	
			-exec {
				if ($3 == 入る) {
					mesg ドア Ａ脚ドア open $2
				} else {
					mesg ドア Ａ脚ドア close $2
				}
			}

		chara ドア Ｆ脚１Ｆドア \
			-m d:DOOR_NAME_LOAD2 \
			-d 0,721,0 \
			-p 17350,0,8300 \
			-slide d:SD_SLIDE_OUTSIDE2 \
			-time d:SD_OPEN_TIME \
			-between ＦＡ連絡橋 ＦＡ連絡橋 \
			-A Ｆ脚１Ｆドアランプ \
			-exec {
				if ( !($:開閉フラグ) ){
					mesg プットオブジェ Ｆ脚１Ｆロードエリア off
				} else {
					mesg プットオブジェ Ｆ脚１Ｆロードエリア on
				}
			}
		@ドアランプ設定 Ｆ脚１Ｆドアランプ 722 17350 0 8300 1
		trap dr002 ？ \
			-mask ？ \	
			-exec {
				if ($3 == 入る) {
					mesg ドア Ｆ脚１Ｆドア open $2
				} else {
					mesg ドア Ｆ脚１Ｆドア close $2
				}
			}

		chara ドア Ｆ脚Ｂ１ドア \
			-m d:DOOR_NAME_LOAD3 \
			-d 0,1745,0 \
			-p 21150,-4500,750 \
			-slide d:SD_SLIDE_OUTSIDE2 \
			-time d:SD_OPEN_TIME \
			-between ＦＡ連絡橋 ＦＡ連絡橋 \
			-A Ｆ脚Ｂ１ドアランプ \
			-exec {
				if ( !($:開閉フラグ) ){
					mesg プットオブジェ Ｆ脚Ｂ１ロードエリア off
				} else {
					mesg プットオブジェ Ｆ脚Ｂ１ロードエリア on
				}
			}
		@ドアランプ設定 Ｆ脚Ｂ１ドアランプ 1745 21150 -4500 750 1
		trap dr003 ？ \
			-mask ？ \	
			-exec {
				if ($3 == 入る) {
					mesg ドア Ｆ脚Ｂ１ドア open $2
				} else {
					mesg ドア Ｆ脚Ｂ１ドア close $2
				}
			}
	}

	proc 主観禁止壁設定 {

		@主観禁止壁 Ａ脚ドア壁 -19125 0 -5750 1024 d:SD_SLIDE_OUTSIDE
		@主観禁止床 Ａ脚ドア床１ Ａ脚ドア床２ -19125 0 -5750 1024 d:SD_SLIDE_OUTSIDE
		trap dr001 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ａ脚ドア壁 on
				} else {
					mesg 透明壁 Ａ脚ドア壁 off
				}
			}

		@主観禁止壁 Ｆ脚１Ｆドア壁 17350 0 8300 722 d:SD_SLIDE_OUTSIDE2
		@主観禁止床 Ｆ脚１Ｆドア床１ Ｆ脚１Ｆドア床２ 17350 0 8300 722 d:SD_SLIDE_OUTSIDE2
		trap dr002 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ｆ脚１Ｆドア壁 on
				} else {
					mesg 透明壁 Ｆ脚１Ｆドア壁 off
				}
			}


		@主観禁止壁 Ｆ脚Ｂ１ドア壁 21150 -4500 750 1745 d:SD_SLIDE_OUTSIDE2
		@主観禁止床 Ｆ脚Ｂ１ドア床１ Ｆ脚Ｂ１ドア床２ 21150 -4500 750 1745 d:SD_SLIDE_OUTSIDE2
		trap dr003 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if ($3 == 入る) {
					mesg 透明壁 Ｆ脚Ｂ１ドア壁 on
				} else {
					mesg 透明壁 Ｆ脚Ｂ１ドア壁 off
				}
			}

	}



//###################################################################################
//									 開始時処理
//###################################################################################

	proc ＦＡ連絡橋開始時処理 {

		//	サイファー遭遇デモを見た直後はここから
		if ($s:登場ポイント == サイファー遭遇デモ終了後) {
				@ステージ開始時処理 d:LOAD_TYPE:NO_MOVE_TYPE 0 0 0

		//	サイファー遭遇デモを見た直後以外はここから
		} else {
			if( $s:登場ポイント == sp_w12b0_w23a_0 ) {
				// Ａ脚１Ｆから
				@ステージ開始時処理（ステージ名表示）\
				d:LOAD_TYPE:SL_DOOR_TYPE Ａ脚ドア 3500 d:DEFAULT_M_TYPE \
				d:wn_ＦＡ連絡橋
				@Ａ脚ロード時カメラ設定
			} else if( $s:登場ポイント == sp_w22a0h_w23a_0 ) {
				// Ｆ脚１Ｆから
				@ステージ開始時処理（ステージ名表示）\
				d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚１Ｆドア 2750 d:DEFAULT_M_TYPE \
				d:wn_ＦＡ連絡橋
				@Ｆ脚１Ｆロード時カメラ設定
			} else if( $s:登場ポイント == sp_w22a1c_w23a_0 ) {
				// Ｆ脚Ｂ１から
				@ステージ開始時処理（ステージ名表示）\
				d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚Ｂ１ドア 3250 d:DEFAULT_M_TYPE \
				d:wn_ＦＡ連絡橋
				@Ｆ脚Ｂ１ロード時カメラ設定
			}
		}
	}

//###################################################################################
//									 ロード
//###################################################################################

	proc ロードプロック {
		#if d:DEBUG_PRINT 
			print 'load_proc'
		#endif

		if ($s:呼び出しプロック == mv_w23a_w12b0_0) {
			// Ａ脚１Ｆへ
			@mv_w23a_w12b0_0
		} else if ($s:呼び出しプロック == mv_w23a_w22a0h_0) {
			// Ｆ脚１Ｆへ
			@mv_w23a_w22a0h_0
		} else if ($s:呼び出しプロック == mv_w23a_w22a1c_0) {
			// Ｆ脚Ｂ１へ
			@mv_w23a_w22a1c_0
		}
	}
	proc ＦＡ連絡橋ロード設定 {
		#if d:DEBUG_PRINT 
			print 'load_set'
		#endif

		trap ld001 d:PLAYER \
			-mask いる \
			-exec {
				if ($f:ロードチェックＯＮフラグ == 1) {
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
					// Ａ脚１Ｆへ
					@ステージ終了時処理（ステージ名表示） \
					-21000 0 -6500 3072 mv_w23a_w12b0_0 d:DEFAULT_M_TYPE \
					d:wn_Ａ脚ポンプ室
					@Ａ脚ロード時カメラ設定
					//	先読み
					preseek 'w12b'
				}
			}

		trap ld002 d:PLAYER \
			-mask いる \
			-exec {
				@ポリトラップ $4 $6 18250,6250 20500,6250 20500,8750 17000,8750;
				if(( $i:ポリトラップ結果 >= 0 ) && ( $f:ロードチェックＯＮフラグ == 1 )){
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
					// Ｆ脚１Ｆへ
					@ステージ終了時処理（ステージ名表示） \
					19750 0 8000 721 mv_w23a_w22a0h_0 d:DEFAULT_M_TYPE \
					d:wn_Ｆ脚倉庫１Ｆ
					@Ｆ脚１Ｆロード時カメラ設定
					//	先読み
					preseek 'w22a'
				}
			}

		trap ld003 d:PLAYER \
			-mask いる \
			-exec {
				@ポリトラップ $4 $6 19125,-125 19125,-2625 21125,-2625 21125,875 ;
				if(( $i:ポリトラップ結果 >= 0 ) && ( $f:ロードチェックＯＮフラグ == 1 )){
					//	爆弾イベント用
					@爆弾タイマー残り時間保存
					// Ｆ脚Ｂ１へ
					@ステージ終了時処理（ステージ名表示） \
					21500 -4500 -2250 1745 mv_w23a_w22a1c_0 d:DEFAULT_M_TYPE \
					d:wn_Ｆ脚倉庫Ｂ１
					@Ｆ脚Ｂ１ロード時カメラ設定
					//	先読み
					preseek 'w22a'
				}
			}
	}

	proc Ａ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \	
						-p -17165,4822,-4362 \
						-t -18230,603,-8457 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc Ｆ脚１Ｆロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 14595,4205,8981 \
						-t 16875,1822,6586 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc Ｆ脚Ｂ１ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 20231,361,2290 \
						-t 19546,-2225,1248 \
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
//									 オブジェクト
//###################################################################################

	proc オブジェクト設定 {

	//	朝
	#if d:W23A
		//	フォーチュンと遭遇するまではＢＣ連絡橋は壊れない
		//	ＢＣ連絡橋壊れ前
		if ($w:p_story < d:ST:P012_01_P01フォーチュン遭遇１ポリゴンデモ１開始) {
			chara プットオブジェ 中央連絡橋破壊前 -m d:KMS_NAME10 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		
		//	ＢＣ連絡橋壊れ後
		} else {
			chara プットオブジェ 中央連絡橋破壊後 -m d:KMS_NAME11 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		}
	//	昼
	#else
		//	ＢＣ連絡橋壊れ後
			chara プットオブジェ 中央連絡橋破壊後 -m d:KMS_NAME11 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
	#endif
	
		chara プットオブジェ Ｆ脚１Ｆロードエリア \
			-m 	d:KMS_NAME15 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｆ脚１Ｆロードエリア off

		chara プットオブジェ Ｆ脚Ｂ１ロードエリア \
			-m 	d:KMS_NAME16 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
			mesg プットオブジェ Ｆ脚Ｂ１ロードエリア off

		chara プットオブジェ Ａ脚ロードエリア \
			-m 	d:KMS_NAME17 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
			mesg プットオブジェ Ａ脚ロードエリア off

	}

	proc 水位設定 {
		command 水位設定 -lv -40000
	}

	proc 海面設定 {
		#if d:DEBUG_PRINT
			print 'w23a_sea_set'
		#endif

	#if d:W23A
		//	朝
		chara プラント海面 seaseasea \
			-tex oil m_oil_sea_alp_ovl_mod0222 m_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
//			-pos 0,-40000,0
			-pos 0,-40000,15000

	#else
		//	昼
		chara プラント海面 seaseasea \
			-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
//			-pos 0,-40000,0
			-pos 0,-40000,15000

	#endif

		chara 水面監視水飛沫 落下水飛沫 \
			-n d:PLAYER \
			-n 敵兵
		command 弾水飛沫処理
	}

	proc 空設定 {
		#if d:DEBUG_PRINT
			print 'w23a_sky_set'
		#endif
	#if d:W23A

//		@朝空	0 -40000 0 1000 -1500 4600
		@朝空（レンズフレア指定）	0 -40000 0 1000 -1500 4600 10


	#else

//		@昼空	0 -40000 0 -2400 -4200 1160
		@昼空（レンズフレア指定）	0 -40000 0 -2400 -4200 1160 10

	#endif

	}

	proc 天井設定 {
		#if d:DEBUG_PRINT
			print 'ceil_set'
		#endif
		chara 天井君 Ａ脚天井 -m d:KMS_NAME1 -p 0,0,0  -l d:LT2_NAME -c -14000,0,0 -a 0
		chara 天井君 Ｆ脚天井 -m d:KMS_NAME8 -p 0,0,0  -l d:LT2_NAME -c 14000,0,0 -a 1
//		chara 天井君 Ｆ脚廊下 -m d:KMS_NAME9 -p 0,0,0  -l d:LT2_NAME -c 14000,0,0 0,1000,0 -a 1 2 -e 2 1
		chara 天井君 Ｆ脚廊下 -m d:KMS_NAME9 -p 0,0,0  -l d:LT2_NAME -c 14000,0,0 0,287,0 -a 1 2 -e 2 1
	}



//###################################################################################
//									 アイテム
//###################################################################################

	proc アイテム設定 {
		#if d:DEBUG_PRINT
			print 'item_set'
		#endif

		@アイテム再セットチェック $b:w23a_取得ロード回数[0] 
		if ($f:再セットフラグ) {
			@チャフ_初期設置 チャフ:01 -13000 -4500 2000 0		//	Ｂ１西側
		}
	}

	proc アイテム取得時番号設定 $:ボックス名 {
	#if d:DEBUG_PRINT
		print '*** Item No Set ***'
	#endif

		if($:ボックス名 == チャフ:01) {
			eval($b:w23a_取得ロード回数[0] = $w:グローバルロード回数);
		}
	}

//###################################################################################
//									 エフェクト
//###################################################################################

	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

	//	朝の場合
	#if d:W23A
		@明るさ可変機能 194 199 163 -20000 175000 ha002 -4500


	//昼の場合
	#else
		@明るさ可変機能 220 237 198 -15000 175000 ha002 -4500


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

	//	朝の場合
	#if d:W23A
		chara 環境光 環境光源 \
			-c 33 53 50

		chara 平行光 平行光源 \
			-c 201 169 103 \
			-p 1000,-1500,4600

	//昼の場合
	#else
		chara 環境光 環境光源 \
//			-c 59 61 47
			-c 55 63 38

		chara 平行光 平行光源 \
//			-c 166 167 147 \
//			-p -2400,-4200,1160
			-c 163 164 150 \
			-p -2400,-4200,1160

	#endif
	}


	proc ＦＡ連絡橋風設定 {
		@風設定 ca003
	}

	proc 影設定 {
		#if d:DEBUG_PRINT
		print'SHADOW'
		#endif

	#if d:W23A	//	朝のみ
		//	影写り用モデル
		chara 影投影モデル 影用モデル \
			-model w23a_sdw \
			-pos -1,1,-1

		//	影写り用ハザード　敵兵が影を発見

		command	影投影ハザード \
			-name ＦＡ連絡橋 \
			-group 9
		
		chara 影管理 影管理人 \
			-type 0 \
			-length 10000 \
			-p 515 -1500 2405 \
			-alpha 64
		
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

//###################################################################################
//									 カメラ
//###################################################################################

	proc カメラセット {
		#if d:DEBUG_PRINT
			print 'stage_camera_set'
		#endif

		chara カメラ ステージカメラ			-c 0 -l 3 -p 0 
		chara カメラ 通路カメラ				-c 0 -l 2 -p 50
		chara カメラ 小部屋カメラ			-c 0 -l 2 -p 40
		chara カメラ ビヨンドカメラ			-c 0 -l 2 -p 20
		chara カメラ 張り付きカメラ			-c 0 -l 2 -p 10
		chara カメラ ロードカメラ			-c 0 -l 1 -p 6
		chara カメラ ゲームオーバーカメラ	-c 0 -l 1 -p 5
		chara カメラ シナリオカメラ			-c 0 -l 1 -p 4

		// デフォルトカメラ
		chara カメラ設定 ステージカメラ \
			-c 0 \	
			-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
			-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//			-r 425,2048,0 \
			-r 800,2048,0 \
			-f 7500 \
			-a 200 \
			-i 2 2 0 0 \
			-s 1
	}

	proc ＦＡ連絡橋カメラ設定 {
		#if d:DEBUG_PRINT
			print 'w23a_camera_set'
		#endif

		// Ａ脚ドア前
		trap ca007 d:PLAYER \
			-camera \
			-mask ＊ \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 1 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-5500 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-p -17299,6488,-5355 -t -17299,227,-7552 \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためバウンドをみる
					-s $3
		}



		// Ａ脚通路
		trap ca001 d:PLAYER \
			-camera \
			-mask ＊ \
			-exec {
//-p -15593,4160,2843 -t -14972,2299,-156 \
//-p -15247,5567,1157 -t -15163,2004,748 \

					@CS_スムーズカメラ昇順 $6  -3000 1000 -15593 -15247
					eval( $i:補完ＰＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -3000 1000 4160 5567
					eval( $i:補完ＰＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -3000 1000 2843 1157
					eval( $i:補完ＰＺ = $i:CS_結果１ );


					@CS_スムーズカメラ昇順 $6  -3000 1000 -14972 -15163
					eval( $i:補完ＴＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -3000 1000 2299 2004
					eval( $i:補完ＴＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -3000 1000 -156 748
					eval( $i:補完ＴＺ = $i:CS_結果１ );
		
				chara カメラ設定 小部屋カメラ \
					-c 1 \
					-b -15750,d:CAM_MIN,d:CAM_MIN -14375,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
					-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためバウンドをみる
					-s $3
		}



		// 連絡橋
# if 0
		trap ca003 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {

				//	高さ１３０以上のとき
				if ($5 > 130) {
					//	プレイヤーのＸ座標に沿ってＸロットが変化
					@CS_スムーズカメラ昇順 $4  -12000 12000 650 450

				//	高さ１３０以下のとき
				} else if ($5 > -3100) {
					//	高さ１３０のときのＸロットを求める。
					@CS_スムーズカメラ昇順 $4  -12000 12000 650 450
					//	プレイヤーのＸ座標に沿って変化するＸロットを別の変数に代入
					eval ($i:現状ＲＸ１ = $i:CS_結果１);

					//	高さ−３１００のときのＸロットを求める。
//					@CS_スムーズカメラ昇順 $4  -12000 12000 480 400
					@CS_スムーズカメラ昇順 $4  -12000 12000 800 400
					//	求めたＸロットを別の変数に代入。このＸロットからスムーズにつながる
					eval ($i:現状ＲＸ２ = $i:CS_結果１);

					//	求めた変数からプレイヤーのＹ座標に沿ってＸロットが変化
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＲＸ１ $i:現状ＲＸ２

				} else {
//					@CS_スムーズカメラ昇順 $4  -12000 12000 480 400
					@CS_スムーズカメラ昇順 $4  -12000 12000 800 400
				}

				//	最終的に求めたＸロットを代入
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -3000 -13000 -13852
				eval( $i:補完ＢＸ１ = $i:CS_結果１ );
				@CS_スムーズカメラ降順 $5  -600 -3000 -8775 -8603
				eval( $i:補完ＬＸ１ = $i:CS_結果１ );
				@CS_スムーズカメラ降順 $5  -600 -3000 3392 8197
				eval( $i:補完ＢＸ２ = $i:CS_結果１ );
				@CS_スムーズカメラ降順 $5  -600 -3000 8519 13902
				eval( $i:補完ＬＸ２ = $i:CS_結果１ );



				if ($5 > 130) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 10393 8158
				} else if ($5 > -3100) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 10393 8158
					eval ($i:現状ＢＹ１ = $i:CS_結果１);
					@CS_スムーズカメラ昇順 $4  -8000 8000 -227 -842
					eval ($i:現状ＢＹ２= $i:CS_結果１);
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＢＹ１ $i:現状ＢＹ２
				} else {
					@CS_スムーズカメラ昇順 $4  -8000 8000 -227 -842
				}

				eval( $i:補完ＢＹ = $i:CS_結果１ );

				if ($5 > 130) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 1154 1154
				} else if ($5 > -3100) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 1154 1154
					eval ($i:現状ＬＹ１ = $i:CS_結果１);
					@CS_スムーズカメラ昇順 $4  -8000 8000 -2998 -3035
					eval ($i:現状ＬＹ２= $i:CS_結果１);
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＬＹ１ $i:現状ＬＹ２
				} else {
					@CS_スムーズカメラ昇順 $4  -8000 8000 -2998 -3035
				}

				eval( $i:補完ＬＹ = $i:CS_結果１ );

				chara カメラ設定 通路カメラ \
					-c 0 \
					-b $i:補完ＢＸ１,$i:補完ＢＹ,d:CAM_MIN $i:補完ＢＸ２,10500,d:CAM_MAX \
					-l $i:補完ＬＸ１,$i:補完ＬＹ,d:CAM_MIN $i:補完ＬＸ２,1154,d:CAM_MAX \
					-r $i:補完ＲＸ,1600,0 -f 11000 \
					-a 200 \
					-d 1 \
					-i 0 -1 0 0 \
//					-z d:CAMERA_NO_CUSHION \
					-s $3

			}

#else

		trap ca003 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {

				//	高さ１３０以上のとき
				if ($5 > 130) {
					//	プレイヤーのＸ座標に沿ってＸロットが変化
					@CS_スムーズカメラ昇順 $4  -12000 12000 650 450

				//	高さ１３０以下のとき
				} else if ($5 > -3100) {
					//	高さ１３０のときのＸロットを求める。
					@CS_スムーズカメラ昇順 $4  -12000 12000 650 450
					//	プレイヤーのＸ座標に沿って変化するＸロットを別の変数に代入
					eval ($i:現状ＲＸ１ = $i:CS_結果１);

					//	高さ−３１００のときのＸロットを求める。
//					@CS_スムーズカメラ昇順 $4  -12000 12000 480 400
					@CS_スムーズカメラ昇順 $4  -12000 12000 800 400
//					@CS_スムーズカメラ昇順 $4  -12000 12000 712 400
					//	求めたＸロットを別の変数に代入。このＸロットからスムーズにつながる
					eval ($i:現状ＲＸ２ = $i:CS_結果１);

					//	求めた変数からプレイヤーのＹ座標に沿ってＸロットが変化
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＲＸ１ $i:現状ＲＸ２

				} else {
//					@CS_スムーズカメラ昇順 $4  -12000 12000 480 400
					@CS_スムーズカメラ昇順 $4  -12000 12000 800 400
//					@CS_スムーズカメラ昇順 $4  -12000 12000 712 400
				}

				//	最終的に求めたＸロットを代入
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -3000 -13000 -13852
				eval( $i:補完ＢＸ１ = $i:CS_結果１ );
				@CS_スムーズカメラ降順 $5  -600 -3000 -8775 -8603
				eval( $i:補完ＬＸ１ = $i:CS_結果１ );
				@CS_スムーズカメラ降順 $5  -600 -3000 3392 8197
				eval( $i:補完ＢＸ２ = $i:CS_結果１ );
				@CS_スムーズカメラ降順 $5  -600 -3000 8519 13902
				eval( $i:補完ＬＸ２ = $i:CS_結果１ );



				if ($5 > 130) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 10393 8158
				} else if ($5 > -3100) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 10393 8158
					eval ($i:現状ＢＹ１ = $i:CS_結果１);
					@CS_スムーズカメラ昇順 $4  -8000 8000 -227 -842
					eval ($i:現状ＢＹ２= $i:CS_結果１);
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＢＹ１ $i:現状ＢＹ２
				} else {
					@CS_スムーズカメラ昇順 $4  -8000 8000 -227 -842
				}

				eval( $i:補完ＢＹ = $i:CS_結果１ );

				if ($5 > 130) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 1154 1154
				} else if ($5 > -3100) {
					@CS_スムーズカメラ昇順 $4  -12000 12000 1154 1154
					eval ($i:現状ＬＹ１ = $i:CS_結果１);
					@CS_スムーズカメラ昇順 $4  -8000 8000 -2998 -3035
					eval ($i:現状ＬＹ２= $i:CS_結果１);
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＬＹ１ $i:現状ＬＹ２
				} else {
					@CS_スムーズカメラ昇順 $4  -8000 8000 -2998 -3035
				}

				eval( $i:補完ＬＹ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -3100 11000 15000
				eval( $w:補完距離 = $i:CS_結果１ );

				chara カメラ設定 通路カメラ \
					-c 0 \
					-b $i:補完ＢＸ１,$i:補完ＢＹ,d:CAM_MIN $i:補完ＢＸ２,10500,d:CAM_MAX \
					-l $i:補完ＬＸ１,$i:補完ＬＹ,d:CAM_MIN $i:補完ＬＸ２,1154,d:CAM_MAX \
					-r $i:補完ＲＸ,1600,0 -f $w:補完距離 \
					-a 200 \
					-d 1 \
					-i 0 -1 0 0 \
//					-z d:CAMERA_NO_CUSHION \
					-s $3

			}

#endif


		trap ca005 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {

//-p 15989,4160,8344 -t 17119,1191,2884 \
//-p 16273,5583,7766 -t 16454,1602,6884 \

					@CS_スムーズカメラ昇順 $6 1000 5500 15989 16273
					eval( $i:補完ＰＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 1000 5500 4160 5583
					eval( $i:補完ＰＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 1000 5500 8344 7766
					eval( $i:補完ＰＺ = $i:CS_結果１ );


					@CS_スムーズカメラ昇順 $6 1000 5500 17119 16454
					eval( $i:補完ＴＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 1000 5500 1191 1602
					eval( $i:補完ＴＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6 1000 5500 2884 6884
					eval( $i:補完ＴＺ = $i:CS_結果１ );

				chara カメラ設定 小部屋カメラ \
					-c 1 \
					-b 14375,d:CAM_MIN,d:CAM_MIN 16500,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
					-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためバウンドをみる
					-s $3 \

			}




		// Ｆ脚Ｂ１
		trap ca006 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {

//-p 14972,-4,3784 -t 15158,-2247,2870 \
//-p 18747,-19,3820 -t 18934,-2262,2902 \
//-p 18753,-17,3831 -t 19196,-3046,1699 \

				if ($6 > 1500) {
					@CS_スムーズカメラ昇順 $4 15000 18250 14972 18747
					eval( $i:補完ＰＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $4 15000 18250 -4 -19
					eval( $i:補完ＰＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $4 15000 18250 3784 3820
					eval( $i:補完ＰＺ = $i:CS_結果１ );


					@CS_スムーズカメラ昇順 $4 15000 18250 15158 18934
					eval( $i:補完ＴＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $4 15000 18250 -2247 -2262
					eval( $i:補完ＴＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $4 15000 18250 2870 2902
					eval( $i:補完ＴＺ = $i:CS_結果１ );

				} else {

					@CS_スムーズカメラ降順 $6 1750 750 18747 18753
					eval( $i:補完ＰＸ = $i:CS_結果１ );

					@CS_スムーズカメラ降順 $6 1750 750 -19 -17
					eval( $i:補完ＰＹ = $i:CS_結果１ );

					@CS_スムーズカメラ降順 $6 1750 750 3820 3831
					eval( $i:補完ＰＺ = $i:CS_結果１ );


					@CS_スムーズカメラ降順 $6 1750 750 18934 19196
					eval( $i:補完ＴＸ = $i:CS_結果１ );

					@CS_スムーズカメラ降順 $6 1750 750 -2262 -3046
					eval( $i:補完ＴＹ = $i:CS_結果１ );

					@CS_スムーズカメラ降順 $6 1750 750 2902 1699
					eval( $i:補完ＴＺ = $i:CS_結果１ );
				}

				chara カメラ設定 小部屋カメラ \
					-c 1 \
					-b 14350,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,3900 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
					-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
					-a 139 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためバウンドをみる
					-s $3

			}

	//	張り付き移動
		//	１Ｆ
		//	Ａ脚入口付近北側
		trap bh001 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 0 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
	
		//	Ａ脚入口付近南側
		trap bh002 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	Ｆ脚入口付近北側
		trap bh003 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 0 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
	
		//	Ｆ脚入口付近南側
		trap bh004 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}


		//	Ｂ１
		//	左から
		trap bh101 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラＢ１ $3
			}

		trap bh102 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラＢ１ $3
			}

		trap bh103 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラＢ１ $3
			}

		trap bh104 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラＢ１ $3
			}

		trap bh105 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラＢ１ $3
			}

		trap bh106 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラＢ１ $3
			}


		//	階段
		//	左
		trap bh201 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ階段 $3
			}

		trap bh202 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ階段 $3
			}

		trap bh203 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ階段 $3
			}
		//	右
		trap bh204 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ階段 $3
			}

		trap bh205 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ階段 $3
			}

		trap bh206 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ階段 $3
			}

	}

	proc 張り付き移動カメラ１Ｆ {
		chara カメラ設定 張り付きカメラ \
		-c 0 \	
/*
		-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
		-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
*/
		-b -11329,13692,1388 8690,14311,1388 \
		-l -9478,1003,-751 10541,1622,-751 \
		
		-r 881,1583,0 -f 10000 \
		-a 200 \
		-i 2 2 0 0 \
		-s $1 \
		-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
	
	}

	proc 張り付き移動カメラＢ１ {
		chara カメラ設定 張り付きカメラ \
		-c 0 \	

//		-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//		-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \

//		-b -12852,7044,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,3700 \
//		-l -12031,-2877,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,2751 \

		-b -12852,7044,6343 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
		-l -12031,-2877,5394 d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \

		-r 942,1583,0 -f 10000 \
		-a 200 \
		-i 2 2 0 0 \
		-s $1 \
		-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
	}

	proc 張り付き移動カメラ階段 {
		@張り付き移動カメラＢ１ $1
	}


//###################################################################################
//									 ビヨンド
//###################################################################################

	proc ＦＡ連絡橋ビヨンド設定 {
		#if d:DEBUG_PRINT
			print 'w23a_beyond_set'
		#endif

// 連絡橋１Ｆ北側
			//	エルード発動
			trap by001 d:PLAYER \
				-mask いる \
				-button d:ACTION_BUTTON \		//アクションボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@北向きビヨンドハイ発動 by001 -1000 $4 -7750 7750 0x00000002
				}
			//	発動カメラ
			trap by001 d:PLAYER \
				-mask いる \
				-state 6 \
				-dir 2048 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,3212,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
						-r 222,1310,0 -f 5652 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			//	エルードカメラ
			trap by001 d:PLAYER \
				-mask ？ \
				-state 5 \
				-dir 0 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 881,1480,0 -f 6619 \
						-a 227 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION
				}
			//	張り付きカメラ
			trap by001 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 0 256 \	// 方向指定
				-camera \
				-exec {
					chara カメラ設定 張り付きカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//						-r 881,1583,0 -f 6619 \
						-r 881,1583,0 -f 10000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
				}

// 連絡橋１Ｆ南側
			//	エルード発動
			trap by002 d:PLAYER \
				-mask いる \
				-button d:ACTION_BUTTON \		//アクションボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@南向きビヨンド発動 by002 1000 $4 -5750 5750 0x00000002
				}
			//	発動カメラ
			trap by002 d:PLAYER \
				-mask いる \
				-state 6 \
				-dir 0 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,3212,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
						//-r 406,1483,0 -f 10410 \
						//-a 200 \
						-r 406,1444,0 -f 6348 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			//	エルードカメラ
			trap by002 d:PLAYER \
				-mask ？ \
				-state 5 \
				-dir 2048 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 484,1778,0 -f 13605 \
						-a 693 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION
				}
			//	張り付きカメラ
			trap by002 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 2048 256 \	// 方向指定
				-camera \
				-exec {
					chara カメラ設定 張り付きカメラ \
						-c 0 \	
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//						-r 881,1583,0 -f 6619 \
						-r 881,1583,0 -f 10000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
				}


/*	ここはエルードなし
			// 連絡橋Ｂ１北側
			trap by003 d:PLAYER \
				-mask いる \
				-button d:ACTION_BUTTON \		//アクションボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@北向きビヨンドハイ発動 by003 1000 $4 -5250 5250 0x00000002
				}
			trap by003 d:PLAYER \
				-mask いる \
				-state 6 \
				-dir 2048 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 500,1685,0 -f 6560 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			trap by003 d:PLAYER \
				-mask ？ \
				-state 5 \
				-dir 0 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 500,1685,0 -f 6560 \
						-a 200 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION
				}
*/


// 連絡橋Ｂ１南側左
			//	エルード発動
			trap by004 d:PLAYER \
				-mask いる \
				-button d:ACTION_BUTTON \		//アクションボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@南向きビヨンドハイ発動 by004 3000 $4 -11750 -10250 0x00000002
				}
			//	発動カメラ
			trap by004 d:PLAYER \
				-mask いる \
				-state 6 \
				-dir 0 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b -13696,-1891,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l -9378,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-3181,d:CAM_MAX \
						-r 150,1464,0 -f 5602 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			//	エルードカメラ
			trap by004 d:PLAYER \
				-mask ？ \
				-state 5 \
				-dir 2048 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b -13874,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l -10943,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3904,1794,0 -f 8065 \
						-a 276 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION 
				}
			//	張り付きカメラ
				trap by004 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 2048 256 \	// 方向指定
				-camera \
				-exec {
#if 0
					chara カメラ設定 張り付きカメラ \
						-c 0 \	
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//						-r 942,1583,0 -f 6817 \
						-r 942,1583,0 -f 10000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
#else
					@張り付き移動カメラＢ１ $3
#endif
				}

// 連絡橋Ｂ１南側中央
			//	エルード発動
			trap by005 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \		//アクションボタン
			-state 0,4 \	//立ち状態or張り付き
			-exec {
				@南向きビヨンドハイ発動 by005 3000 $4 -1750 1750 0x00000002
			}
			//	発動カメラ
			trap by005 d:PLAYER \
				-mask いる \
				-state 6 \
				-dir 0 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,-1891,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-3181,d:CAM_MAX \
						-r 150,1464,0 -f 5602 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			//	エルードカメラ
			trap by005 d:PLAYER \
				-mask ？ \
				-state 5 \
				-dir 2048 256 \	//方向指定
				-camera \			//カメラトラップであることを明示
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 3904,1794,0 -f 8065 \
						-a 276 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION 
				}
			//	張り付きカメラ
			trap by005 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 2048 256 \	// 方向指定
				-camera \
				-exec {
#if 0
					chara カメラ設定 張り付きカメラ \
						-c 0 \	
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//						-r 942,1583,0 -f 6817 \
						-r 942,1583,0 -f 10000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
#else
					@張り付き移動カメラＢ１ $3
#endif
				}

// 連絡橋Ｂ１南側右
			//	エルード発動
			trap by006 d:PLAYER \
				-mask いる \
				-button d:ACTION_BUTTON \		//アクションボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@南向きビヨンドハイ発動 by006 3000 $4 10250 11625 0x00000002
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
						-b d:CAM_MIN,-1891,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,-3181,d:CAM_MAX \
						-r 150,1464,0 -f 5602 \
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
						-r 3904,1794,0 -f 8065 \
						-a 276 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION 
				}
			//	張り付きカメラ
			trap by006 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 2048 256 \	// 方向指定
				-camera \
				-exec {
#if 0
					chara カメラ設定 張り付きカメラ \
						-c 0 \	
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
//						-r 942,1583,0 -f 6817 \
						-r 942,1583,0 -f 10000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
#else
					@張り付き移動カメラＢ１ $3
#endif
				}

	}



//###################################################################################
//									 シナリオデモ
//###################################################################################

//	サイファー遭遇デモ開始チェック
	proc サイファー遭遇デモ開始チェック {
		//	Ｆ脚から来たときはデモはなし
		if ($b:w23a_敵配置フラグ != d:STRUT_A) {
			eval($f:w23a_サイファー遭遇デモフラグ = d:TRUE)
		}

		if ($w:p_story >= d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) {
			eval($f:w23a_サイファー遭遇デモフラグ = d:TRUE)
		}

		//	ヴァンプと遭遇したら必ずエルード説明デモは見ている
		if ($w:p_story >= d:ST:P010_01_P01ヴァンプ遭遇１ポリゴンデモ１開始) {
			eval ($f:w13a_エルード説明デモフラグ = d:TRUE);
		}
	}

//	ゲームモード切替え判定
	proc モード切替え判定 {
		//	フォーチュン戦後からファットマン戦までサイファーでない
		if ( ($w:p_story < d:ST:P029_01フォーチュン戦開始) || \
			($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) ) {

			//	サイファー遭遇デモを見ていない場合
			if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
				//	エルード説明デモを見ていないときは強制的に潜入モード
				if ($f:w13a_エルード説明デモフラグ == d:FALSE) {
					eval ($w:スタートアラートモード = d:ALERT_MODE_SENNYU)
				}
			}
		}
	}

//	サイファー遭遇デモ
	proc サイファー遭遇デモ設定 {
		trap sc000 d:PLAYER  \
			-mask 入る \
			-e {
				//	フォーチュン戦後からファットマン戦までサイファーでない
				if ( ($w:p_story < d:ST:P029_01フォーチュン戦開始) || \
					($w:p_story >= d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) ) {

					//	サイファー遭遇デモを見ていない場合
					if ($f:w23a_サイファー遭遇デモフラグ == d:FALSE) {
						//	サイファー遭遇デモフラグを立てる（危険モード時はデモは二度と見ない）
						eval($f:w23a_サイファー遭遇デモフラグ = d:TRUE)
						//	エルード説明デモを見ていないときは強制的に潜入モード
						if ($f:w13a_エルード説明デモフラグ == d:FALSE) {
							eval ($w:スタートアラートモード = d:ALERT_MODE_SENNYU)
						}
						//	潜入モード時のみデモが流れる
						if ($w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
							@サイファーデモ管理設定
						}

					}
				}
			}
	}

	proc サイファーデモ管理設定 {
		chara プロック連続実行 サイファーデモ管理人 \
			-p サイファーデモ管理
	}

	proc サイファーデモ管理 {
		command パッドチェック \
			-no_use
		//	パッドが戻ったら（ステージ開始処理が終了したら）
		if ($status != 1) {
			//	なぜかロードカメラ死なないので
			chara カメラ設定 ロードカメラ \
				-s -1
			@サイファー遭遇デモ
			mesg プロック連続実行 サイファーデモ管理人 kill
		}

	}


	proc サイファー遭遇デモ {
		//	シナリオデモ開始処理
		@シナリオデモ開始処理
		//	特殊装備、ダンボールも解除
		@特殊装備解除
		@ダンボール解除

		if ($b:w23a_敵配置フラグ == d:STRUT_A) {

//	PAL版のみ
#ifdef d:PAL
			//	パッドデモ設定
			chara パッドデモ サイファー遭遇デモライデン \
				-file  w23a_paddemo_n00_pal \
				-e サイファー遭遇デモ終了処理

			//	デモ用サイファー起動
			chara delay サイファースタートディレイ \
				-t 200 \
				-e {
					mesg サイファ デモサイファー:01 ルート変更 2 1
				}

			//	パッドデモ終了
			chara delay パッドデモ終了ディレイ \
				-t 770 \
				-p サイファー遭遇デモ終了処理

//	PAL版以外
#else
			//	パッドデモ設定
			chara パッドデモ サイファー遭遇デモライデン \
				-file  w23a_paddemo_n00 \
				-e サイファー遭遇デモ終了処理

			//	デモ用サイファー起動
			chara delay サイファースタートディレイ \
				-t 240 \
				-e {
					mesg サイファ デモサイファー:01 ルート変更 2 1
				}

			//	パッドデモ終了
			chara delay パッドデモ終了ディレイ \
				-t 720 \
				-p サイファー遭遇デモ終了処理
#endif

			//	パッドデモ再生
			mesg パッドデモ サイファー遭遇デモライデン start

			//	シナリオカメラ起動
			chara カメラ設定 シナリオカメラ \
				-c 1 \
				-p -14015,2806,-5288 -t -18321,1582,-6949 \
				-r 169,2832,0 -f 4775 \
				-a 133 \
				-i 0 -1 0 0 \
				-s 1

			trap ca001 d:PLAYER \
				-mask 入る \
				-exec {
					chara delay カメラ切替えディレイ \
						-t 10 \
						-e {
							chara カメラ設定 シナリオカメラ \
								-s -1
						}
				}

			//	キャンセルできるように
			chara デモキャンセルチェック サイファー遭遇デモ -p サイファー遭遇デモ終了処理

		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			//	パッドデモ再生＆デモ用サイファー起動
			chara パッドデモ サイファー遭遇デモライデン \
				-file  w23a_paddemo_n01 \
				-e サイファー遭遇デモ終了処理

			mesg パッドデモ サイファー遭遇デモライデン start

			chara delay サイファースタートディレイ \
				-t 150 \
				-e {
					mesg サイファ デモサイファー:01 ルート変更 2 4
				}

			//	キャンセルできるように
			chara デモキャンセルチェック サイファー遭遇デモ -p サイファー遭遇デモ終了処理

			//	パッドデモ終了
			chara delay パッドデモ終了ディレイ \
				-t 660 \
				-p サイファー遭遇デモ終了処理


		} else {
			//	パッドデモ再生＆デモ用サイファー起動
			chara パッドデモ サイファー遭遇デモライデン \
				-file  w23a_paddemo_n02 \
				-e サイファー遭遇デモ終了処理

			mesg パッドデモ サイファー遭遇デモライデン start

			chara delay サイファースタートディレイ \
				-t 120 \
				-e {
					mesg サイファ デモサイファー:01 ルート変更 0 4
				}

			//	キャンセルできるように
			chara デモキャンセルチェック サイファー遭遇デモ -p サイファー遭遇デモ終了処理

			//	パッドデモ終了
			chara delay パッドデモ終了ディレイ \
				-t 660 \
				-p サイファー遭遇デモ終了処理

		}

	}


	proc サイファー遭遇デモ終了処理 {
		@rt_P009_01_R01_サイファー遭遇１無線デモ１
	}

	proc サイファー遭遇１無線デモ１終了処理 {
		@サイファー遭遇デモ終了後処理
	}

	proc サイファー遭遇デモ終了後処理 {

		//	サイファー遭遇デモ終了後の登場ポイントにする
		eval($s:登場ポイント = サイファー遭遇デモ終了後 )

		//	プレイヤー初期配置修正
		if ($b:w23a_敵配置フラグ == d:STRUT_A) {
			eval ($i:プレイヤー初期Ｘ位置 = -14500)
			eval ($i:プレイヤー初期Ｙ位置 = 0)
			eval ($i:プレイヤー初期Ｚ位置 = -1375)
			eval ($i:プレイヤー初期方向 =  3072)
			eval ($b:プレイヤー初期姿勢 = d:FA_END_STAND)
		} else if ($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
			eval ($i:プレイヤー初期Ｘ位置 = 14500)
			eval ($i:プレイヤー初期Ｙ位置 = 0)
			eval ($i:プレイヤー初期Ｚ位置 = 1375)
			eval ($i:プレイヤー初期方向 =  1024)
			eval ($b:プレイヤー初期姿勢 = d:FA_END_STAND)
		} else {
			eval($i:プレイヤー初期Ｘ位置 = 14675)
			eval($i:プレイヤー初期Ｙ位置 = -4500)
			eval($i:プレイヤー初期Ｚ位置 = 3500)
			eval($i:プレイヤー初期方向 =  2048)
			eval($b:プレイヤー初期姿勢 = d:FA_END_STAND)
		}

		//	シナリオデモ終了処理を行う
		@シナリオデモ終了処理


		mesg パッドデモ サイファー遭遇デモライデン kill
		mesg デモキャンセルチェック サイファー遭遇デモ kill

		mesg delay サイファースタートディレイ kill
		mesg delay パッドデモ終了ディレイ kill
		mesg delay カメラ切替えディレイ kill

		restart -s
	}


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
							-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-r 3960,365,0 -f 4340 \
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
				if ($3 == 入る) {
						#if d:DEBUG_PRINT
							print 'gameoverstart'
						#endif
						chara カメラ設定 ゲームオーバーカメラ \
							-c 0 \
							-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-r 3960,1683,0 -f 4340 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1
	
						@エルード落下死開始処理
				} else if ( $3 == 出る ) {
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
							-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-r 3960,1683,0 -f 4340 \
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
				if ($3 == 入る) {
						#if d:DEBUG_PRINT
							print 'gameoverstart'
						#endif
						chara カメラ設定 ゲームオーバーカメラ \
							-c 0 \
							-b d:CAM_MIN,-5000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-l d:CAM_MIN,-40000,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
							-r 3960,1683,0 -f 4340 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1
	
						@エルード落下死開始処理
				} else if ( $3 == 出る ) {
					@ゲームオーバー終了処理
					@エルード落下死終了処理
				}
			}


	}
















// FIXデータはここまで
// -----------------------------------------------------------------------------------












// 以下仮キャラ用proc
// -----------------------------------------------------------------------------------
// 仮キャラ
// ------------------------------------
proc 水面セット {
	#if d:DEBUG_PRINT
		print 'water_set'
	#endif

	chara 頂点アニメブラー 仮水面 \
		-model w11a_sea \
		-base 10 \
		-width 1 \
		-flag 5 \
		-pos 0,0,0
}

	// 子画面設定 
	// ------------------------------------
	proc 子画面設定 {
		#if d:DEBUG_PRINT
			print 'sub_camera_set'
		#endif

		chara サブ画面 子画面 \
			-w (512-d:SUB_MARGIN-d:SUB_WIDTH),d:SUB_MARGIN (512-d:SUB_MARGIN),(d:SUB_HEIGHT+d:SUB_MARGIN)
	}


proc シナリオデモ作成用 {
	if ($b:w23a_敵配置フラグ == d:STRUT_A) {
		chara モーションレコーダー シナリオデモ１ \
				-t d:PLAYER \
				-m 21 \
				-o 'w23a_raiden_motion1.mtn' \
				-f 600 \
				-e 1 \
				-k 1

		chara モーションレコーダー シナリオデモ２ \
				-t デモサイファー:01 \
				-o 'w23a_cypher_motion1.mtn' \
				-f 600 \
				-e 1 \
				-k 1

	} else if($b:w23a_敵配置フラグ == d:STRUT_F_1F) {
		chara モーションレコーダー シナリオデモ１ \
				-t d:PLAYER \
				-m 21 \
				-o 'w23a_raiden_motion2.mtn' \
				-f 600 \
				-e 1 \
				-k 1

		chara モーションレコーダー シナリオデモ２ \
				-t デモサイファー:01 \
				-o 'w23a_cypher_motion2.mtn' \
				-f 600 \
				-e 1 \
				-k 1

	} else {
		chara モーションレコーダー シナリオデモ１ \
				-t d:PLAYER \
				-m 21 \
				-o 'w23a_raiden_motion3.mtn' \
				-f 600 \
				-e 1 \
				-k 1

		chara モーションレコーダー シナリオデモ２ \
				-t デモサイファー:01 \
				-o 'w23a_cypher_motion3.mtn' \
				-f 600 \
				-e 1 \
				-k 1
	}
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
