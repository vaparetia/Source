//=============================================================================
// w20bcd_common.h
// Ｅ脚・ヘリポート(共通)
// 2001/06/07 S.Yamashita
// $Id: w20bcd_common.h,v 1.100 2002/05/09 13:07:49 usr01475 Exp $
//

// [登場ポイント]
//   w20a					sp_w20a1_w20b_0		w20b/w20c/w20d
//   オルガ遭遇後			sp_demo1_w20b_0		w20b
//   ファットマン登場後		sp_demo2_w20b_0		w20c
//   ファットマン死亡後		sp_demo3_w20b_0		w20c
//   忍者遭遇後				sp_demo4_w20b_0		w20d

//=============================================================================
// define
//=============================================================================

//-----------------------------------------------------------------------------
// スイッチ

//----- w20b
// #define STAGE_PLANT 1
// #define STAGE_W20B  1
//
// #define ENE_P_GOL 1

//----- w20c
// #define STAGE_PLANT 1
// #define STAGE_W20C  1

//----- w20d
// #define STAGE_PLANT 1
// #define STAGE_W20D  1
//
// #define ENE_P_GOL     1
// #define ENE_HIGH_TECH 1

#define RAIDEN     1
#if d:RAIDEN
#define PLAYER     ライデン
#else
#define PLAYER     スネーク
#endif

#define ENEMY_SET      1		// 敵兵の配置
#define STAGE_END      1		// ロードプロック
#define START_CAMERA   1		// ステージ開始時のカメラ
#define CTDOWN_TIMER_P 1		// 爆弾タイマー開始用
#define BOMB_STAGE     w20b		// 爆弾解体後処理用
#define POLYDEMO_LOAD  1		// ポリデモへロード

#define SCREENSHOT     0		// スクリーンショット
#define KEYBOARD       0		// ＵＳＢキーボード

//-----------------------------------------------------------------------------
// マップ関係

#define KMS_NAME1  w20b_12a			// Ａ脚					２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME2  w20b_14a			// Ｂ脚					２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME3  w20b_15a			// ＢＣ連絡橋			２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME4  w20b_17a			// ＣＤ連絡橋			２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME5  w20b_21a			// ＥＦ連絡橋			２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME6  w20b_23a			// ＦＡ連絡橋			２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME7  w20b_24a			// Ｓ１中央棟			２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME8  w20b_crane		// クレーン				２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME9  w20b_h			// Ｈ脚					２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME10 w20b_hi			// ＨＩ連絡橋			２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME11 w20b_other00		// 西側オイルフェンス０	２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME12 w20b_other01		// 西側オイルフェンス１	２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示
#define KMS_NAME13 w20b_other02		// 西側オイルフェンス２	２Ｆ俯瞰○  屋上俯瞰○  エルード○  主観時のみ表示

#define KMS_NAME14 w20b_i			// Ｉ脚					２Ｆ俯瞰×  屋上俯瞰○  エルード×  屋上以外で表示
#define KMS_NAME15 w20b_jk			// ＪＫ連絡橋			２Ｆ俯瞰×  屋上俯瞰○  エルード×  屋上以外で表示
#define KMS_NAME16 w20b_k			// Ｋ脚					２Ｆ俯瞰×  屋上俯瞰○  エルード×  屋上以外で表示
#define KMS_NAME17 w20b_31a			// Ｓ２中央棟			２Ｆ俯瞰×  屋上俯瞰○  エルード×  屋上以外で表示
#define KMS_NAME18 w20b_20a			// Ｅ脚					２Ｆ俯瞰×  屋上俯瞰△  エルード×  中央以外で表示
#define KMS_NAME19 w20b_19a			// ＤＥ連絡橋			２Ｆ俯瞰○  屋上俯瞰×  エルード×  ２Ｆ以外で表示
#define KMS_NAME20 w20b_32a			// 狙撃ステージＡ		２Ｆ俯瞰○  屋上俯瞰×  エルード×  ２Ｆ以外で表示
#define KMS_NAME21 w20b_32c			// 狙撃ステージＣ		２Ｆ俯瞰○  屋上俯瞰×  エルード×  ２Ｆ以外で表示

#define KMS_NAME22 w20b_25a			// ＤＧ連絡橋			２Ｆ俯瞰×  屋上俯瞰×  エルード×  常に表示
#define KMS_NAME23 w20b_32b			// 狙撃ステージＢ		２Ｆ俯瞰×  屋上俯瞰×  エルード×  常に表示
#define KMS_NAME24 w20b_gl			// ＬＧ連絡橋			２Ｆ俯瞰×  屋上俯瞰×  エルード×  常に表示

#define OBJ_NAME1  w20b_hri			// ハリアー
#define OBJ_NAME2  w20b_horo		// ホロ
#define OBJ_NAME3  w20b_dammy_sea	// にせ海面

#define DOOR_NAME1 w20b_dr00		// 扉



#ifdef d:STAGE_W20B
	#define HZX_NAME  w20b
	#define LT2_NAME  w20b
	#define KMS_NAME0 w20b
#endif
#ifdef d:STAGE_W20C
	#define HZX_NAME  w20c
	#define LT2_NAME  w20c
	#define KMS_NAME0 w20c
#endif
#ifdef d:STAGE_W20D
	#define HZX_NAME  w20c
	#define LT2_NAME  w20c
	#define KMS_NAME0 w20c
#endif





//=============================================================================
// include
//=============================================================================

#include "rai_w20b.mh"	// ステージ固有モーションリスト

#include "stdch.h"
#include "sload.h"
#include "beyond.h"
#include "beyond3.h"
#include "fall.h"
#include "jump.h"
#include "sound.h"
#include "enevoice.h"
#include "item_info.h"

#include "boss_stdproc.h"

#if d:KEYBOARD 
#include "usbkbd.h"
#endif

//-----------------------------------------------------------------------------
// ローカル

eval($f:アイテム再セット１ = 0)
eval($f:アイテム再セット２ = 0)
eval($f:アイテム再セット３ = 0)
eval($f:アイテム再セット４ = 0)
eval($f:アイテム再セット５ = 0)

eval($f:Ｓローカルフラグ１ = 0)
#define _C_FUKAN       0	// 俯瞰時
#define _C_SHUKAN      1	// 主観時
eval($f:Ｓローカルフラグ２ = 0)
eval($f:Ｓローカルフラグ３ = 0)
eval($w:Ｓローカル変数Ｗ１ = 0)
#define _R_2F          0	// ２Ｆにいる
#define _R_FUCHI       1	// 屋上の淵にいる
#define _R_NAKA        2	// 屋上の真ん中辺にいる
#define _R_ELUDE       3	// エルード中
#define _R_FALL_GOVER 10	// 落下死
eval($i:Ｓローカル変数Ｉ１ = 0)		// ワーク
eval($i:Ｓローカル変数Ｉ２ = 0)		// ワーク
eval($i:Ｓローカル変数Ｉ３ = 0)		// ワーク

#include "stageset.h"		// ステージ設定共通ヘッダ





//=============================================================================
// メイン
//=============================================================================

#ifndef d:BOSS_MODE
chara マップ ヘリポート \
	-k d:KMS_NAME0 \
	-k d:KMS_NAME22 \	// ＤＧ連絡橋
	-k d:KMS_NAME23 \	// 狙撃ステージ
	-k d:KMS_NAME24 \	// ＬＧ連絡橋
	-h d:HZX_NAME, 0 \
	-l d:LT2_NAME

command マップ設定 ヘリポート -set {

	//----- 基本設定
	@特別処理
	@サウンド設定
	@SS_跳弾弾痕設定
	@水位設定
	@レーダーオンオフチェック $f:w20a_ノードフラグ
	@常駐キャラ設定

	@プレイヤー設定
#if d:ENEMY_SET
	@敵兵設定
#endif
	@ＢＧＭ設定
	@爆弾タイマー設置
	@プラント編全体マップ設定 d:Ｅ脚マップ

	//----- ビヨンド、ジャンプ
	@ビヨンド設定
	@ジャンプ設定

	//----- オブジェクト
	@ドア設定
	@アイテム設定
	@爆弾解体設定

	@その他オブジェクト設定

	//----- フォグ、カメラ、ロード
	@SS_フォグ_キャラ用ライト設定
	@カメラ設定
	@ストーリー設定
	@ロード設定

	//----- ボスラッシュ
	@ボスラッシュ開始時キャラ設定
}
#endif

#include "w20bcd_cdc.ch"			// ※※ 通信機の include は移動させないこと！！ ※※
#include "p_force_cdc.ch"

#ifndef d:BOSS_MODE
command マップ表示 -show ヘリポート

// 遠景登録
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  1
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  2
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  3
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  4
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  5
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  6
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  7
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  8
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id  9
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 10
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 11
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 12
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 13
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 14
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 15
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 16
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 17
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 18
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 19
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 20
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 21
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 22
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 23
command 追加ＨＺＸグループ登録 -hzx_id 0 -add_id 24

@ステージ進入時処理
#endif





//=============================================================================
// プロシージャ
//=============================================================================

//-----------------------------------------------------------------------------
// ライデン状態
proc ＳＥＴライデン状態 $:状態 {
	if($w:Ｓローカル変数Ｗ１ != d:_R_FALL_GOVER) {
		eval($w:Ｓローカル変数Ｗ１ = $:状態)
	}
}
proc ＧＥＴライデン状態 $:状態 {
	if($w:Ｓローカル変数Ｗ１ == $:状態) {
		return 1
	} else {
		return 0
	}
}

//=============================================================================
// 基本設定
//=============================================================================

//-----------------------------------------------------------------------------
// 特別処理
proc 特別処理 {
#if d:DEBUG_PRINT
	print '*** Special Proc ***'
#endif

	if($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) {

		//---------------------------------------------------------
		// ファットマン用進入禁止ゾーン
		command セットゾーンフラグ d:HZX_ZONE_ZINTRPT 59000 11500 -78750


		//---------------------------------------------------------
		// ファットマン戦プレイ回数
		if($w:w20c_ファットマン戦プレイ回数 < 100) {
			// プレイ数をカウント
			eval($w:w20c_ファットマン戦プレイ回数 = $w:w20c_ファットマン戦プレイ回数 + 1)
			command varsave $w:w20c_ファットマン戦プレイ回数
		}
		if($w:w20c_ファットマン戦プレイ回数 == 6) {

			// レーダーＯＮ
			if(($w:ゲーム設定 < d:LEVEL_NORMAL) && ($f:w20a_ノードフラグ == 0)) {
				eval($f:w20a_ノードフラグ       = 1)
				command varsave $f:w20a_ノードフラグ
				eval($f:w20c_レーダーＯＮフラグ = 1)
				command varsave $f:w20c_レーダーＯＮフラグ
			}
		} else if($w:w20c_ファットマン戦プレイ回数 == 11) {

			// レーダーＯＮ
			if(($w:ゲーム設定 == d:LEVEL_NORMAL) && ($f:w20a_ノードフラグ == 0)) {
				eval($f:w20a_ノードフラグ       = 1)
				command varsave $f:w20a_ノードフラグ
				eval($f:w20c_レーダーＯＮフラグ = 1)
				command varsave $f:w20c_レーダーＯＮフラグ
			}
		}
	}
}

//-----------------------------------------------------------------------------
// サウンド設定
proc サウンド設定 {
#if d:DEBUG_PRINT
	print '*** Sound Set ***'
#endif

	if($w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) {

		// ### ファットマン登場前 ###
		chara サウンドマネージャー ＳＤマネ -pak 0	// パック番号

	} else if($w:p_story < d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始) {

		// ### ファットマン戦 ###
		chara サウンドマネージャー ＳＤマネ -pak 0	// パック番号
		mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_01

	} else if($w:p_story < d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１開始) {

		// ### 最後の爆弾 ###
		chara サウンドマネージャー ＳＤマネ -pak 0	// パック番号
		mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06

	} else {

		// ### 忍者登場後 ###
		if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
			chara サウンドマネージャー ＳＤマネ -pak 0	// パック番号

		} else {
		#ifdef d:JAPANESE	//----- 日本語版
			chara サウンドマネージャー ＳＤマネ -pak 0	// パック番号
		#else				//----- 英語版
			chara サウンドマネージャー ＳＤマネ -pak 1	// パック番号
		#endif
		}
	}

	@サウンド効果音設定
}

//-----------------------------------------------------------------------------
// 水位設定
proc 水位設定 {
#if d:DEBUG_PRINT
	print '*** Water Level Set ***'
#endif

	command 水位設定 -lv -40000
	command 弾水飛沫処理
}

//-----------------------------------------------------------------------------
// プレイヤー設定
proc プレイヤー設定 {
#if d:DEBUG_PRINT
	print '*** Player Set ***'
#endif

	// オルガデモ直後は武器をハンドガンに
	if($s:登場ポイント == sp_demo1_w20b_0) {
		@デモ後武器アイテム制御 d:デモ後銃装備 d:デモ後特殊装備なし
	}
	// ファットマン戦・ファットマン戦後は武器を初期化
	if(    ($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) \
		|| ($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了)) {
		@デモ後武器アイテム制御 d:デモ後銃装備 d:デモ後特殊装備なし
	}
	if($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) {
		eval($w:アイテム = d:アイテム:センサーＡ);
	}
	// 忍者デモ直後は装備解除
	if($s:登場ポイント == sp_demo4_w20b_0) {
		@デモ後武器アイテム制御 d:デモ後必ず素手 d:デモ後特殊装備なし
	}

	// ファットマン戦後ライフをフルに
	if($s:登場ポイント == sp_demo3_w20b_0) {
		eval($w:体力 = $w:体力最大)
	}

	// プレイヤー設定
	//   $:強制モーションファイル $:ＬＯＤ距離
	@SS_プレイヤー設定 rai_w20b 6000

	// 死体上で匍匐ダメ
	if($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了) {
		command プレイヤー死体上立たせ
	}
}

//-----------------------------------------------------------------------------
// 敵兵設定

// 警備兵
//   �� 2	２Ｆの敵兵
//   �� 3	屋上の敵兵
//
// 守備位置
//   59750  5500  -80500 ヘリポート
//   56500 11500  -91250 ヘリポート
//   49500 11500  -84625 ヘリポート
//   56500 11500 -101000 ヘリポート
//
// 再発生位置
//   60750 0 -90250 ヘリポート

//----- 警備兵設定
//        $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ $:ＩＤ
proc 警備兵セット１ {
#if d:DEBUG_PRINT 
	print '*** Watcher Set 1 ***'
#endif

	if($s:登場ポイント == sp_demo1_w20b_0) {
		@SS_警備兵設定 敵兵:01 5 0 ($i:SS_敵兵ステータス) 59750  5500  -80500 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚２Ｆ兵)
		@SS_警備兵設定 敵兵:02 3 0 ($i:SS_敵兵ステータス) 56500 11500  -91250 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚ヘリポート兵)

	} else {
		@SS_警備兵設定 敵兵:01 2 0 ($i:SS_敵兵ステータス) 59750  5500  -80500 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚２Ｆ兵)
		@SS_警備兵設定 敵兵:02 3 0 ($i:SS_敵兵ステータス) 56500 11500  -91250 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚ヘリポート兵)
	}
}

proc 警備兵セット２ {
#if d:DEBUG_PRINT 
	print '*** Watcher Set 2 ***'
#endif

	if($s:登場ポイント == sp_demo1_w20b_0) {
		@SS_警備兵設定 敵兵:01 5 0 ($i:SS_敵兵ステータス) 59750  5500  -80500 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚２Ｆ兵)
		@SS_警備兵設定 敵兵:02 3 0 ($i:SS_敵兵ステータス) 56500 11500  -91250 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚ヘリポート兵)
		@SS_警備兵設定 敵兵:03 4 0 ($i:SS_敵兵ステータス) 49500 11500  -84625 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚ヘリポート応援兵)

	} else {
		@SS_警備兵設定 敵兵:01 2 0 ($i:SS_敵兵ステータス) 59750  5500  -80500 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚２Ｆ兵)
		@SS_警備兵設定 敵兵:02 3 0 ($i:SS_敵兵ステータス) 56500 11500  -91250 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚ヘリポート兵)
		@SS_警備兵設定 敵兵:03 4 0 ($i:SS_敵兵ステータス) 49500 11500  -84625 ヘリポート (d:SS_警備兵ＩＤ:Ｅ脚ヘリポート応援兵)
	}
}

//----- アタッカー設定
//        $:名前 $:ルート $:ステータス $:装備
proc アタッカーセット１ {
#if d:DEBUG_PRINT 
	print '*** Attacker Set 1 ***'
#endif

	if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
#ifndef d:STAGE_W20C
		@SS_アタッカー設定 敵兵:21 1 ($i:SS_敵兵ステータス) 0
		@SS_アタッカー設定 敵兵:22 1 ($i:SS_敵兵ステータス) 0
#endif
	} else {
#ifdef d:STAGE_W20D
		@SS_アタッカー設定 敵兵:21 1 ($i:SS_敵兵ステータス) 4
		@SS_アタッカー設定 敵兵:22 1 ($i:SS_敵兵ステータス) 4
#endif
	}
}





//----- コマンダー
proc 警備兵コマンダー {
#if d:DEBUG_PRINT 
	print '*** Watcher Commander ***'
#endif

	if($w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) {
		// 爆弾解体
		if ($w:ゲーム設定 < d:LEVEL_NORMAL) {
			chara 警備コマンダー wccomm -e { 警備兵セット１ }
		} else {
			chara 警備コマンダー wccomm -e { 警備兵セット２ }
		}

	} else if($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) {
		// センサーＢ
		if ($w:ゲーム設定 < d:LEVEL_NORMAL) {
			chara 警備コマンダー wccomm -e { 警備兵セット１ }
		} else {
			chara 警備コマンダー wccomm -e { 警備兵セット２ }
		}

	} else if($w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了) {
		// フォーチュン戦後・ファットマン戦


	} else if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
		// ファットマン戦後
		if($s:登場ポイント != sp_demo4_w20b_0) {
			if ($w:ゲーム設定 < d:LEVEL_NORMAL) {
				chara 警備コマンダー wccomm -e { 警備兵セット１ }
			} else {
				chara 警備コマンダー wccomm -e { 警備兵セット２ }
			}
		}

	} else {
		// エイムズ後
		if ($w:ゲーム設定 < d:LEVEL_NORMAL) {
			chara 警備コマンダー wccomm -e { 警備兵セット１ }
		} else {
			chara 警備コマンダー wccomm -e { 警備兵セット２ }
		}
	}
}

proc 攻撃兵コマンダー {
#if d:DEBUG_PRINT 
	print '*** Attacker Commander ***'
#endif

	chara 攻撃コマンダー atcomm -e { アタッカーセット１ }
}





//----- 敵兵設定

#include "w20b_kamome.gcl"

proc 敵兵設定 {
#if d:DEBUG_PRINT
	print '*** Enemy Set ***'
#endif

	//---------------------------------------------------------
	// 敵兵ステータス
	eval($i:SS_敵兵ステータス = d:ENE_STATUS_NO_FINGER)	// 通常は 0

	//---------------------------------------------------------
	// ファットマン
	if($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) {
#ifdef d:STAGE_W20C
		if($w:ゲーム設定 < d:LEVEL_NORMAL) {
			@ファットマン配置_ＥＡＳＹ
		} else {
			@ファットマン配置_ＨＡＲＤ
		}
#endif

	//---------------------------------------------------------
	// コマンダー
	//   -w 警備コマンダーproc  -a 攻撃コマンダーproc  -r 復活場所 マップＩＤ  -c 待機場所 マップＩＤ  -v 音声  -m 危険モード  -p クリアリングエリア ルート ポイント プロック  -x 司令官危険音声  -y 司令官警戒音声  -g 銃ライト  -z 麻酔持続時間  -b 危険モードでの復活人数 
	} else if(($f:w20b_オルガデモフラグ == 0) \													// オルガデモ前
		||    (    (d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story) \
				&& ($w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了)) \					// ファットマン戦前後
		||    ($s:登場ポイント == sp_demo4_w20b_0)) {											// 忍者デモ直後
		// 潜入モードへ戻す
		// ALERT_MODE_SENNYU
		// ALERT_MODE_KIKEN
		// ALERT_MODE_KAIHI
		// ALERT_MODE_TANSAKU
		eval($w:スタートアラートモード = d:ALERT_MODE_SENNYU)

	} else {
#ifndef d:STAGE_W20C

		//---------------------------------------------------------
		// 敵兵状態やりとり
		eval($f:Ｓローカルフラグ２ = 1)
	#ifdef d:STAGE_W20B
		@敵兵開始状態チェック w20b ヘリポート 敵兵:01 42000  5500  -97000 3072 \
			                  w20b ヘリポート 敵兵:02 43000 11500 -106000 3072
	#elifdef d:STAGE_W20D
		@敵兵開始状態チェック w20d ヘリポート 敵兵:01 42000  5500  -97000 3072 \
			                  w20d ヘリポート 敵兵:02 43000 11500 -106000 3072
	#endif

		//---------------------------------------------------------
		// コマンダー
		//   -w 警備コマンダーproc  -a 攻撃コマンダーproc  -r 復活場所 マップＩＤ  -c 待機場所 マップＩＤ  -v 音声  -m 危険モード  -p クリアリングエリア ルート ポイント プロック  -x 司令官危険音声  -y 司令官警戒音声  -g 銃ライト  -z 麻酔持続時間  -b 危険モードでの復活人数 
		chara コマンダー 敵兵セット \
			-w 警備兵コマンダー \
			-a 攻撃兵コマンダー \
			-d ヘリポート \
			-r 60750 0 -90250 ヘリポート \
			-c 60750 0 -90250 ヘリポート \
			-v [ene_plant_voice:01] \
			-x [ghq_area_alert_voice:w20b] \
			-y [ghq_caution_voice:w20b] \
			-z $i:敵標準麻酔持続 \
			-h $i:標準警戒時間 \
			-o $i:敵標準気絶持続 \
			-s d:ENE_STAGE_GPS \
			-b $w:敵標準再発生数 \
			-m $w:スタートアラートモード
#endif
	}



#ifdef d:STAGE_W20C
	// かもめ
	@w20b_かもめ_15
#endif
}

//-----------------------------------------------------------------------------
// ＢＧＭ設定
proc ＢＧＭ設定 {
#if d:DEBUG_PRINT
	print '*** BGM Set ***'
#endif

	if($w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) {

		// ### ファットマン登場前 ###
		chara ＢＧＭマネージャー ＢＧＭマネ -s 0x00000001	// 潜入モードの曲を他のキャラが管理
		chara ＢＧＭ風音 ＢＧＭ風音 -s d:BGM_WINDNOISE_WEATHER_FAIN

	} else if($w:p_story < d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始) {

		// ### ファットマン戦 ###


	} else if($w:p_story < d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１開始) {

		// ### 最後の爆弾 ###


	} else if($w:p_story >= d:ST:P036_13_R04忍者登場１３無線デモ４終了) {

		// ### 忍者登場後 ###
		chara ＢＧＭマネージャー ＢＧＭマネ -s 0x00000001	// 潜入モードの曲を他のキャラが管理
		chara ＢＧＭ風音 ＢＧＭ風音 -s d:BGM_WINDNOISE_WEATHER_FAIN
	}
}





//=============================================================================
// ビヨンド、ジャンプ
//=============================================================================

//-----------------------------------------------------------------------------
// ビヨンド設定
proc ビヨンド設定 {
#if d:DEBUG_PRINT
	print '*** Beyond Set ***'
#endif

// 縦横方向ビヨンド

//----- ビヨンド１
	trap by001 d:PLAYER \
		-mask 入る \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@南向き柵なしビヨンド発動 $1 -77500 $4 45000 55000 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by001) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by001 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   0,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 597,2171,0 \
				-f 3800 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by001 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   2048 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 597,2186,0 \
				-f 6154 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド２
	trap by003 d:PLAYER \
		-mask 入る \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@西向き柵なしビヨンド発動 $1 41000 $6 -110000 -81500 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by003) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by003 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   3072,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 580,2228,0 \
				-f 5919 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by003 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   1024 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 205,1988,0 \
				-f 5402 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド３
	trap by005 d:PLAYER \
		-mask 入る \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@北向き柵なしビヨンド発動 $1 -114000 $4 45000 68000 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by005) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by005 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   2048,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 835,2063,0 \
				-f 7423 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by005 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   0 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 895,1868,0 \
				-f 8598 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド４
	trap by007 d:PLAYER \
		-mask 入る \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@東向き柵なしビヨンド発動 $1 72000 $6 -110000 -81500 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by007) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by007 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   1024,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 580,1868,0 \
				-f 5919 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by007 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   3072 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 340,2273,0 \
				-f 8363 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド５
	trap by009 d:PLAYER \
		-mask 入る \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@南向き柵なしビヨンド発動 $1 -77500 $4 63000 68000 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by009) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by009 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   0,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 580,2228,0 \
				-f 5919 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by009 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   2048 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 490,2183,0 \
				-f 6295 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

// 斜め方向ビヨンド

//----- ビヨンド６
	trap by002 d:PLAYER \
		-mask いる \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@斜め柵無しビヨンド発動 $1 $4 $6 45000 -77500 41000 -81500 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by002) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by002 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   3584,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 580,2228,0 \
				-f 5919 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by002 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   1536 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 402,1781,0 \
				-f 7000 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド７
	trap by004 d:PLAYER \
		-mask いる \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@斜め柵無しビヨンド発動 $1 $4 $6 41000 -110000 45000 -114000 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by004) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by004 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   2560,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 640,1808,0 \
				-f 6436 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by004 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   512 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 507,1372,0 \
				-f 10055 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド８
	trap by006 d:PLAYER \
		-mask いる \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@斜め柵無しビヨンド発動 $1 $4 $6 68000 -114000 72000 -110000 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by006) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by006 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   1536,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 580,2228,0 \
				-f 5919 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by006 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   3584 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 492,2974,0 \
				-f 7846 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

//----- ビヨンド９
	trap by008 d:PLAYER \
		-mask いる \
		-exec {
			command プレイヤー状態取得 ;
			if(    ($status == 0) \
				|| ($status & d:PFLAG_WALK) \
				|| ($status & d:PFLAG_DAMAGED) \
				|| ($status & d:PFLAG_SUBJECT)) {
				if(`@斜め柵無しビヨンド発動 $1 $4 $6 72000 -81500 68000 -77500 12500 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD | d:BY_NO_STAMP | d:BY_RESET_HEIGHT | d:BY_NO_RETURNSTAMP)` == by008) {
					@ＳＥＴライデン状態 (d:_R_ELUDE)
				}
			}
		}
	trap by008 d:PLAYER \
		-mask  いる \
		-state d:TRP_STATE_FORCE \
		-dir   512,256 \
		-camera \			//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 580,2228,0 \
				-f 5919 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1
		}
	trap by008 d:PLAYER \
		-mask  ？ \
		-state d:TRP_STATE_ELUDE \
		-dir   2560 256 \		//方向指定
		-camera \				//カメラトラップであることを明示
		-exec {
			chara カメラ設定 モーションカメラ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 402,2595,0 \
				-f 7000 \
				-a 200 \
				-i 2 2 0 0 \
				-z d:CAMERA_NO_CUSHION \
				-s $3

			if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

	//---------------------------------------------------------
	// ↓ 落下死設定 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
	//
	//    ジャンプ落下死発動
	//     ↓   ↓  モーションが発動する
	//     ↓   ↓
	//     ↓  ジャンプ落下死開始処理
	//     ↓   ↓  ★ 落下カメラを設定
	//     ↓   ↓
	//     ↓  ゲームオーバー開始処理
	//     ↓       すべてがＯＦＦになる
	//     ↓
	//    ゲームオーバー終了処理
	//     ↓  ★ 落下振動を設定
	//     ↓
	//    ゲームオーバー背景設定
	//         ★ ゲームオーバーPICTを設定
	command プラグイン落下
	chara   水面監視水飛沫 落下水飛沫 -n d:PLAYER

	trap fl001 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 1)
			@ジャンプ落下死発動_特 0x0016 7000
		}
	trap fl002 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 2)
			@ジャンプ落下死発動_特 0x0016 7000
		}
	trap fl003 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 3)
			@ジャンプ落下死発動_特 0x0016 7250
		}
	trap fl004 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 4)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl005 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 5)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl006 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 6)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl007 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 7)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl008 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 8)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl009 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 9)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl010 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 10)
			@ジャンプ落下死発動_特 0x0006 -41000
		}
	trap fl011 d:PLAYER \
		-mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
			eval($i:Ｓローカル変数Ｉ１ = 11)
			@ジャンプ落下死発動_特 0x0016 -12000
		}
}

//-----------------------------------------------------------------------------
// 落下死処理
proc ジャンプ落下死発動_特 $:フラグ $:高さ {
#if d:DEBUG_PRINT
	print '*** Jump Gameover Start ***'
#endif

	mesg プレイヤー d:PLAYER fall \
		d:モーションリスト:non_elude_fall_high \
		d:モーションリスト:non_elude_fall_high \
		d:モーションリスト:non_elude_fall_high \
		$:高さ \	//	この高さまで落ちたらゲームオーバー終了処理が呼ばれる
		$:フラグ \
		ジャンプ落下死開始処理 \
		ゲームオーバー終了処理
}
proc ジャンプ落下死開始処理 {
	// 落下カメラ

	// ### 落下ゲームオーバー ###

	if($i:Ｓローカル変数Ｉ１ <=  3) {
		if($i:プレイヤー位置Ｚ > -89750) {
			chara カメラ設定 モーションカメラＳ \
				-c 1 \	
				-p ($i:プレイヤー位置Ｘ - 600),17000,($i:プレイヤー位置Ｚ + 5000) \
				-t ($i:プレイヤー位置Ｘ      ),12001,($i:プレイヤー位置Ｚ +  500) \
				-a 200 \
				-i 0 0 0 0 \
				-s 1
		} else if($i:プレイヤー位置Ｚ > -95000) {
			chara カメラ設定 モーションカメラＳ \
				-c 1 \	
				-p 39663,15211,-83843 \
				-t 40438,9832,-90289 \
				-a 200 \
				-i 0 0 0 0 \
				-s 1
		} else {
			chara カメラ設定 モーションカメラＳ \
				-c 1 \	
				-p 40369,14033,-88358 \
				-t 40698,9607,-95508 \
				-a 200 \
				-i 0 0 0 0 \
				-s 1
		}

	// ### 溺死ゲームオーバー西 ###

	} else if($i:Ｓローカル変数Ｉ１ ==  4) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p ($i:プレイヤー位置Ｘ - 1050),17669,($i:プレイヤー位置Ｚ + 2900) \
			-t ($i:プレイヤー位置Ｘ -  200),11001,($i:プレイヤー位置Ｚ +  500) \
			-a 200 \
			-i 0 0 0 0 \
			-s 1

	// ### 溺死ゲームオーバー北西 ###

	} else if($i:Ｓローカル変数Ｉ１ ==  5) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p 36900,22310,-108742 \
			-t 41300,11451,-110474 \
			-a 200 \
			-i 0 0 0 0 \
			-s 1

	// ### 溺死ゲームオーバー北 ###

	} else if($i:Ｓローカル変数Ｉ１ ==  6) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p ($i:プレイヤー位置Ｘ - 250),19543,($i:プレイヤー位置Ｚ -  600) \
			-t ($i:プレイヤー位置Ｘ      ),10993,($i:プレイヤー位置Ｚ - 1300) \
			-a 200 \
			-i 0 0 0 0 \
			-s 1

	// ### 溺死ゲームオーバー北東 ###

	} else if($i:Ｓローカル変数Ｉ１ ==  7) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p 76100,22310,-108742 \
			-t 71700,11451,-110474 \
			-a 200 \
			-i 0 0 0 0 \
			-s 1

	// ### 溺死ゲームオーバー東 ###

	} else if(($i:Ｓローカル変数Ｉ１ ==  8) || ($i:Ｓローカル変数Ｉ１ == 11)) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p ($i:プレイヤー位置Ｘ + 1050),17669,($i:プレイヤー位置Ｚ + 2900) \
			-t ($i:プレイヤー位置Ｘ +  200),11001,($i:プレイヤー位置Ｚ +  500) \
			-a 200 \
			-i 0 0 0 0 \
			-s 1

	// ### 溺死ゲームオーバー南東 ###

	} else if($i:Ｓローカル変数Ｉ１ ==  9) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p 71629,19550,-76285 \
			-t 70710,12865,-78118 \
			-a 200 \
			-i 0 0 0 0 \
			-s 1

	// ### 溺死ゲームオーバー南 ###

	} else if($i:Ｓローカル変数Ｉ１ ==  10) {
		chara カメラ設定 モーションカメラＳ \
			-c 1 \	
			-p 66521,19108,-74659 \
			-t 66074,13179,-76711 \
			-a 200 \
			-i 0 0 0 0 \
			-s 1
	}

	@ゲームオーバー開始処理
}
proc ゲームオーバー終了処理 {
	// 落下振動
	if(($i:Ｓローカル変数Ｉ１ > 3) && ($i:Ｓローカル変数Ｉ１ < 11)) {
		// 溺死ゲームオーバー
		chara パッド振動 落下振動 -vibfile drop_sea
	}

/* この処理はゲームオーバー処理プロックで一括してやったほうが確実なためそっちに移行しました。2002.03.06
	// フェードイン
	chara delay ディレイ \
		#ifdef d:PAL
		-time 93 \
		#else
		-time 112 \
		#endif
		-exec {
			chara フェードインアウト フェードイン \
				-i 255,255,255 128 \
				-c 255,255,255 0 \
				-t 60
		}
*/
	@ゲームオーバー背景設定
}
proc ゲームオーバー背景設定 {
	// command ゲームオーバー背景設定 -tri tri名 -strcode 背景データ -mode モード(0:通常 1:爆発 2:水溺れ 3:その他)
	if(($i:Ｓローカル変数Ｉ１ > 3) && ($i:Ｓローカル変数Ｉ１ < 11)) {
		// 溺死ゲームオーバー
		command ゲームオーバー背景設定 -t gmov -s dekisi_alp_ovl -m 2
	}
}

//-----------------------------------------------------------------------------
// ジャンプ設定
proc ジャンプ設定 {
#if d:DEBUG_PRINT
	print '*** Jump Set ***'
#endif

	command プラグイン自動ジャンプ
	@ジャンプ発動 ky001 1000 50
}





//=============================================================================
// オブジェクト
//=============================================================================

//-----------------------------------------------------------------------------
// ドア設定
proc ドア設定 {
#if d:DEBUG_PRINT
	print '*** Door Set ***'
#endif

	// ドア
	//   -m オブジェクト名  -d 向き  -p 場所  -slide スライド量  -time 時間  -between マップ名１ マップ名２  -exec { $:開閉フラグ }

	// Ｅ脚屋内へのドア
	@ドアランプ設定 Ｅ脚屋内へのドアランプ 3072 62625 4500 -91000    1
	@主観禁止壁     Ｅ脚屋内へのドア壁          62625 4500 -91000 3072 d:SD_SLIDE_OUTSIDE
	chara ドア Ｅ脚屋内へのドア \
		-m d:DOOR_NAME1 \
		-d 0 3072 0 \
		-p 62625 4500 -91000 \
		-s d:SD_SLIDE_OUTSIDE \
		-t d:SD_OPEN_TIME \
		-A Ｅ脚屋内へのドアランプ \
		-b ヘリポート ヘリポート \
		-exec {
			if(!($:開閉フラグ)) {
			} else {
			}
		}
	trap dr001 ？ -mask ？ \
		-exec {
			if($3 == 入る) {
				mesg ドア Ｅ脚屋内へのドア open
			} else if($3 == 出る) {
				mesg ドア Ｅ脚屋内へのドア close
			}
		}
	trap dr001 d:PLAYER -mask ？ \
		-state d:TRP_STATE_SUBJECT \
		-exec {
			if($3 == 入る) {
				mesg 透明壁 Ｅ脚屋内へのドア壁 on
			} else if($3 == 出る) {
				mesg 透明壁 Ｅ脚屋内へのドア壁 off
			}
		}
}

//-----------------------------------------------------------------------------
// アイテム設定

#define _ITEMBOX_Y     11500

#define _SOCOM_01_X    63625
#define _SOCOM_01_Z   -90750
#define _SOCOM_02_X    49375
#define _SOCOM_02_Z  -101750
#define _SOCOM_03_X    46000
#define _SOCOM_03_Z   -81875
#define _M9_01_X       66250
#define _M9_01_Z     -103500
#define _RATION_01_X   65750
#define _RATION_01_Z  -83500

proc アイテム設定 {
#if d:DEBUG_PRINT
	print '*** Item Set ***'
#endif

	// ダンボール
	@ダンボールＣ_初期設置 ダンボール:01 51500 11500 -84400 0 25

	// スタン
	@アイテム再セットチェック $b:w20b_取得ロード回数[0]
	if(    ($f:再セットフラグ) \
		|| ($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了)) {
		@スタン_初期設置 スタン:01 47000 11500 -96250 0
	}

	// クレイモア
	if($s:登場ポイント != sp_demo4_w20b_0) {
		@アイテム再セットチェック $b:w20b_取得ロード回数[1]
		if($f:再セットフラグ) {
			@クレイモア_初期設置 クレイモア:01 42000 5500 -99000 0
		}
	}

	// ファットマン戦特別ソコム弾
	if(    (d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story) \
		&& ($w:p_story < d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始)) {		// ファットマン戦前〜中
		@アイテム再セットチェック $b:w20b_取得ロード回数[2]
		if($f:再セットフラグ) {
			@弾薬_ソコム_初期設置 ソコム弾薬:04 59000 11500 -80500 0
		}
	}

	if($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) {

		// ソコム弾薬
		@弾薬_ソコム_初期設置 ソコム弾薬:01 d:_SOCOM_01_X d:_ITEMBOX_Y d:_SOCOM_01_Z 0
		eval($f:アイテム再セット１ = 1)
		// ソコム弾薬
		@弾薬_ソコム_初期設置 ソコム弾薬:02 d:_SOCOM_02_X d:_ITEMBOX_Y d:_SOCOM_02_Z 0
		eval($f:アイテム再セット２ = 1)
		// ソコム弾薬
		@弾薬_ソコム_初期設置 ソコム弾薬:03 d:_SOCOM_03_X d:_ITEMBOX_Y d:_SOCOM_03_Z 0
		eval($f:アイテム再セット３ = 1)
		// Ｍ９弾薬
		@弾薬_Ｍ９_初期設置 Ｍ９弾薬:01 d:_M9_01_X d:_ITEMBOX_Y d:_M9_01_Z 0
		eval($f:アイテム再セット４ = 1)
		// レーション
		@レーション_初期設置 レーション:01 d:_RATION_01_X d:_ITEMBOX_Y d:_RATION_01_Z 0
		eval($f:アイテム再セット５ = 1)

		// アイテム再セット
		trap ev103 d:PLAYER \
			-mask いる \
			-exec {
				// ソコム弾薬
				if(    ($w:武器弾数Ｒ[d:武器:ソコム] < 12) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (d:_SOCOM_01_X -1000) (d:_ITEMBOX_Y      ) (d:_SOCOM_01_Z -1000) \
							       (d:_SOCOM_01_X +1000) (d:_ITEMBOX_Y +1000) (d:_SOCOM_01_Z +1000)` == 0)){

					if($f:アイテム再セット１ == 0) {
						@弾薬_ソコム_初期設置 ソコム弾薬:01 d:_SOCOM_01_X d:_ITEMBOX_Y d:_SOCOM_01_Z 0
						eval($f:アイテム再セット１ = 1)
					}
				}

				// ソコム弾薬
				if(    ($w:武器弾数Ｒ[d:武器:ソコム] < 12) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (d:_SOCOM_02_X -1000) (d:_ITEMBOX_Y      ) (d:_SOCOM_02_Z -1000) \
							       (d:_SOCOM_02_X +1000) (d:_ITEMBOX_Y +1000) (d:_SOCOM_02_Z +1000)` == 0)){

					if($f:アイテム再セット２ == 0) {
						@弾薬_ソコム_初期設置 ソコム弾薬:02 d:_SOCOM_02_X d:_ITEMBOX_Y d:_SOCOM_02_Z 0
						eval($f:アイテム再セット２ = 1)
					}
				}

				// ソコム弾薬
				if(    ($w:武器弾数Ｒ[d:武器:ソコム] < 12) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (d:_SOCOM_03_X -1000) (d:_ITEMBOX_Y      ) (d:_SOCOM_03_Z -1000) \
							       (d:_SOCOM_03_X +1000) (d:_ITEMBOX_Y +1000) (d:_SOCOM_03_Z +1000)` == 0)){

					if($f:アイテム再セット３ == 0) {
						@弾薬_ソコム_初期設置 ソコム弾薬:03 d:_SOCOM_03_X d:_ITEMBOX_Y d:_SOCOM_03_Z 0
						eval($f:アイテム再セット３ = 1)
					}
				}

				// Ｍ９弾薬
				if(    ($w:武器弾数Ｒ[d:武器:Ｍ９] < 15) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (d:_M9_01_X -1000) (d:_ITEMBOX_Y      ) (d:_M9_01_Z -1000) \
							       (d:_M9_01_X +1000) (d:_ITEMBOX_Y +1000) (d:_M9_01_Z +1000)` == 0)){

					if($f:アイテム再セット４ == 0) {
						@弾薬_Ｍ９_初期設置 Ｍ９弾薬:01 d:_M9_01_X d:_ITEMBOX_Y d:_M9_01_Z 0
						eval($f:アイテム再セット４ = 1)
					}
				}

				// レーション
				if(    ($w:アイテム数Ｒ[d:アイテム:レーション] < 1) \
					&& (`command カメラ視界チェック -camera 0x01 \
							-bound (d:_RATION_01_X -1000) (d:_ITEMBOX_Y      ) (d:_RATION_01_Z -1000) \
							       (d:_RATION_01_X +1000) (d:_ITEMBOX_Y +1000) (d:_RATION_01_Z +1000)` == 0)){

					if($f:アイテム再セット５ == 0) {
						@レーション_初期設置 レーション:01 d:_RATION_01_X d:_ITEMBOX_Y d:_RATION_01_Z 0
						eval($f:アイテム再セット５ = 1)
					}
				}
			}
	}
}

#ifndef d:BOSS_MODE
proc アイテム取得時番号設定 $:ボックス名 {
#if d:DEBUG_PRINT
	print '*** Item No Set ***'
#endif

	if($:ボックス名 == スタン:01) {
		eval($b:w20b_取得ロード回数[0] = $w:グローバルロード回数);
	} else if($:ボックス名 == クレイモア:01) {
		eval($b:w20b_取得ロード回数[1] = $w:グローバルロード回数);
	} else if($:ボックス名 == ソコム弾薬:04) {
		eval($b:w20b_取得ロード回数[2] = $w:グローバルロード回数);

	} else if($:ボックス名 == ソコム弾薬:01) {
		eval($f:アイテム再セット１ = 0)
	} else if($:ボックス名 == ソコム弾薬:02) {
		eval($f:アイテム再セット２ = 0)
	} else if($:ボックス名 == ソコム弾薬:03) {
		eval($f:アイテム再セット３ = 0)
	} else if($:ボックス名 == Ｍ９弾薬:01) {
		eval($f:アイテム再セット４ = 0)
	} else if($:ボックス名 == レーション:01) {
		eval($f:アイテム再セット５ = 0)
	}

#if d:DEBUG_PRINT
	print $:ボックス名
	print $w:グローバルロード回数
#endif
}
#endif

//-----------------------------------------------------------------------------
// 爆弾解体設定
proc 爆弾解体設定 {
#if d:DEBUG_PRINT
	print '*** Bomb Set ***'
#endif

	// 解体Ｃ４ -p 位置 -r 回転 -t タイプ (０／１：臭いあり／なし) -s 初期状態 (０／１：通常／凍結) -e つける敵の名前 関節番号 -prOc プロック
	// 爆弾検知領域 -t トラップ名 -rgb red green blue
	if($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) {

		// ### 爆弾解体イベント ###
		@SS_爆弾解体設定 w20b匂い爆弾 $f:w20b_爆弾処理完了 59540 11700 -107000 -1024 0 0

	} else if($w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) {

		// ### ヘリポート爆弾 ###

		chara 解体Ｃ４ ヘリポート爆弾 \
			-position 56000 11500 -104500 \
			-rotate -1024 -800 0 \
			-type 0 \
			-state 0 \
			-proc ヘリポート爆弾解体

			chara 爆弾検知領域 Bomb_Type_A -t bm000

	} else if($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了) {

		// ### 最後の爆弾 ###
#if 0
		chara 解体Ｃ４ 最後の爆弾 \
			-position 53500 11500 -103000 \
			-rotate -1024 1024 0 \
			-type 0 \
			-state 0 \
			-big 20 \				// ２倍
			-proc 最後の爆弾解体
#endif
		chara 解体Ｃ４ 最後の爆弾 \
			-position 53400 11500 -103000 \
			-rotate -1024 1024 0 \
			-type 0 \
			-state 0 \
			-model c4_kaitai_c1 \
			-ice_model c4_kaitai_c1_frost_cm \
			-size 300,250,250 \
			-lamp_shift 91,16,229 \
			-proc 最後の爆弾解体

			chara 爆弾検知領域 Bomb_Type_A -t bm999

		command 全体マップ爆弾配置 -bomb d:Ｅ脚マップ

		// 爆弾制御
		trap ev104 ファットマン死体 -mask ＊ \
			-exec {
				if($3 == いる) {
					#if d:DEBUG_PRINT
						print '*** Fatman ON the Last Bomb ***'
					#endif
					mesg 解体Ｃ４ 最後の爆弾 spray_check 0
				} else {
					#if d:DEBUG_PRINT
						print '*** Fatman OFF the Last Bomb ***'
					#endif
					mesg 解体Ｃ４ 最後の爆弾 spray_check 1
				}
			}

		// タイマー
		chara タイマー 最後の爆弾タイマー -t 6000 -p 最後の爆弾タイマー時間切れ -s 0x00
	}
}
proc ヘリポート爆弾解体 {
#if d:DEBUG_PRINT
	print '*** Heliport Bomb Frozen ***'
#endif

	if($2 == 0) {

		// ### 爆発 ###
		@爆発ゲームオーバー処理（簡易版） d:BOMB_EFFECT_ON

	} else if($2 == 1) {

		// ### 解体 ###
		@爆弾タイマー残り時間保存
		command 全体マップ爆弾配置解除 -bomb d:Ｅ脚マップ
		command メニュー設定 -radio off	//この間に無線されると悪。

		chara delay ディレイ \
			-time 30 \
			-exec {
				command ＳＥセットＶＰ -s d:se_code:SD_A_BF_ROLL1 -v 0x3f -p 0x20
			}

		// ファットマン登場デモ
		chara delay ディレイ \
			-time 140 \
			-exec {
//				command メニュー設定 -radio on	//祝復帰！	// ポリデモロード直前に復帰させる
				if(`command ゲームオーバーチェック` == 0 ) {
					preseek 'd021p01'
					@ポリゴンデモロード d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始
				}
			}
	}
}
proc 最後の爆弾解体 {
#if d:DEBUG_PRINT
	print '*** Last Bomb Frozen ***'
#endif

	if($2 == 0) {

		// ### 爆発 ###
		@爆発ゲームオーバー処理（簡易版） d:BOMB_EFFECT_ON

	} else if($2 == 1) {

		// ### 解体 ###
		command 全体マップ爆弾配置解除 -bomb d:Ｅ脚マップ
		command セットタイマーステータス -s d:TMR_WAIT		// 停止
		mesg タイマー 最後の爆弾タイマー 表示切り替え 0
		mesg 爆弾検知領域 Bomb_Type_A 消去 900
		chara カメラ設定 サブカメラ３ -s 0
		mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_FOUTS_S
		command メニュー設定 -radio off	//この間に無線されると悪。

		chara delay ディレイ \
			-time 60 \
			-exec {
				command メニュー設定 -radio on	//祝復帰！
				mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_07
				@rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１
			}
	}
}
proc 最後の爆弾タイマー時間切れ {
#if d:DEBUG_PRINT
	print '*** Last Bomb Exploded ***'
#endif

	mesg 解体Ｃ４ 最後の爆弾 blast
}





//-----------------------------------------------------------------------------
// その他オブジェクト設定
proc その他オブジェクト設定 {
#if d:DEBUG_PRINT
	print '*** Object Set ***'
#endif

	//---------------------------------------------------------
	// プレイヤー状況を判断
	trap ca001 d:PLAYER -mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_2F)
		}
	trap ca002 d:PLAYER -mask 入る \
		-exec {
			@ＳＥＴライデン状態 (d:_R_2F)
		}
	trap ca003 d:PLAYER -mask 入る \
		-exec {
			if(`@ＧＥＴライデン状態 (d:_R_2F)`) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}
	trap ev001 d:PLAYER -mask ？ \
		-exec {
			if($3 == 入る) {
				@ＳＥＴライデン状態 (d:_R_NAKA)
			} else if($3 == 出る) {
				@ＳＥＴライデン状態 (d:_R_FUCHI)
			}
		}

	//---------------------------------------------------------
	// プットオブジェ
	//   -m オブジェクト名  -r 向き  -p 場所  -s スケール  -l 光源  -f フラグ  -t ＬＯＤ閾値
	@SS_プットオブジェ Ａ脚					d:KMS_NAME1
	@SS_プットオブジェ Ｂ脚					d:KMS_NAME2
	@SS_プットオブジェ ＢＣ連絡橋			d:KMS_NAME3
	@SS_プットオブジェ ＣＤ連絡橋			d:KMS_NAME4
	@SS_プットオブジェ ＥＦ連絡橋			d:KMS_NAME5
	@SS_プットオブジェ ＦＡ連絡橋			d:KMS_NAME6
	@SS_プットオブジェ Ｓ１中央棟			d:KMS_NAME7
	@SS_プットオブジェ クレーン				d:KMS_NAME8
	@SS_プットオブジェ Ｈ脚					d:KMS_NAME9
	@SS_プットオブジェ ＨＩ連絡橋			d:KMS_NAME10
	@SS_プットオブジェ 西側オイルフェンス０	d:KMS_NAME11
	@SS_プットオブジェ 西側オイルフェンス１	d:KMS_NAME12
	@SS_プットオブジェ 西側オイルフェンス２	d:KMS_NAME13

	@SS_プットオブジェ Ｉ脚					d:KMS_NAME14
	@SS_プットオブジェ ＪＫ連絡橋			d:KMS_NAME15
	@SS_プットオブジェ Ｋ脚					d:KMS_NAME16
	@SS_プットオブジェ Ｓ２中央棟			d:KMS_NAME17
	@SS_プットオブジェ Ｅ脚					d:KMS_NAME18
	@SS_プットオブジェ ＤＥ連絡橋			d:KMS_NAME19
	@SS_プットオブジェ 狙撃ステージＡ		d:KMS_NAME20
	@SS_プットオブジェ 狙撃ステージＣ		d:KMS_NAME21

	chara プロック連続実行 連続実行 \
		-time -1 \
		-exec {
			command プレイヤー状態取得

//			if(`@ＧＥＴライデン状態 (d:_R_FALL_GOVER)`) {
			if($w:Ｓローカル変数Ｗ１ == d:_R_FALL_GOVER) {		// プロックを実行すると下でプレイヤー状態が取得できない
			//	#if d:DEBUG_PRINT
			//		print '*** Game Over Mode ***'
			//	#endif

				mesg プットオブジェ Ａ脚                 on
				mesg プットオブジェ Ｂ脚                 on
				mesg プットオブジェ ＢＣ連絡橋           on
				mesg プットオブジェ ＣＤ連絡橋           on
				mesg プットオブジェ ＥＦ連絡橋           on
				mesg プットオブジェ ＦＡ連絡橋           on
				mesg プットオブジェ Ｓ１中央棟           on
				mesg プットオブジェ クレーン             on
				mesg プットオブジェ Ｈ脚                 on
				mesg プットオブジェ ＨＩ連絡橋           on
				mesg プットオブジェ 西側オイルフェンス０ on
				mesg プットオブジェ 西側オイルフェンス１ on
				mesg プットオブジェ 西側オイルフェンス２ on

				mesg プットオブジェ Ｉ脚                 on
				mesg プットオブジェ ＪＫ連絡橋           on
				mesg プットオブジェ Ｋ脚                 on
				mesg プットオブジェ Ｓ２中央棟           on
				mesg プットオブジェ Ｅ脚                 on
				mesg プットオブジェ ＤＥ連絡橋           on
				mesg プットオブジェ 狙撃ステージＡ       on
				mesg プットオブジェ 狙撃ステージＣ       on

			} else if(($status & d:PFLAG_SUBJECT) || ($status & d:PFLAG_BEHIND) || ($status & d:PFLAG_INTRUDE)) {
			//	#if d:DEBUG_PRINT
			//		print '*** Shukan - Behind Mode ***'
			//	#endif

				eval($f:Ｓローカルフラグ１ = d:_C_SHUKAN)
				mesg プットオブジェ Ａ脚                 on
				mesg プットオブジェ Ｂ脚                 on
				mesg プットオブジェ ＢＣ連絡橋           on
				mesg プットオブジェ ＣＤ連絡橋           on
				mesg プットオブジェ ＥＦ連絡橋           on
				mesg プットオブジェ ＦＡ連絡橋           on
				mesg プットオブジェ Ｓ１中央棟           on
				mesg プットオブジェ クレーン             on
				mesg プットオブジェ Ｈ脚                 on
				mesg プットオブジェ ＨＩ連絡橋           on
				mesg プットオブジェ 西側オイルフェンス０ on
				mesg プットオブジェ 西側オイルフェンス１ on
				mesg プットオブジェ 西側オイルフェンス２ on

				mesg プットオブジェ Ｉ脚                 on
				mesg プットオブジェ ＪＫ連絡橋           on
				mesg プットオブジェ Ｋ脚                 on
				mesg プットオブジェ Ｓ２中央棟           on
				mesg プットオブジェ Ｅ脚                 on
				mesg プットオブジェ ＤＥ連絡橋           on
				mesg プットオブジェ 狙撃ステージＡ       on
				mesg プットオブジェ 狙撃ステージＣ       on

/*
				if($i:プレイヤー位置Ｘ < 52500) {
					mesg プットオブジェ Ａ脚                 on
					mesg プットオブジェ Ｂ脚                 on
					mesg プットオブジェ ＢＣ連絡橋           on
					mesg プットオブジェ ＣＤ連絡橋           on
					mesg プットオブジェ ＥＦ連絡橋           on
					mesg プットオブジェ ＦＡ連絡橋           on
					mesg プットオブジェ Ｓ１中央棟           on
					mesg プットオブジェ クレーン             on
					mesg プットオブジェ Ｈ脚                 on
					mesg プットオブジェ ＨＩ連絡橋           on
					mesg プットオブジェ 西側オイルフェンス０ on
					mesg プットオブジェ 西側オイルフェンス１ on
					mesg プットオブジェ 西側オイルフェンス２ on

					mesg プットオブジェ Ｉ脚                 on
					mesg プットオブジェ ＪＫ連絡橋           on
					mesg プットオブジェ Ｋ脚                 on
					mesg プットオブジェ Ｓ２中央棟           on
					mesg プットオブジェ Ｅ脚                 on
					mesg プットオブジェ ＤＥ連絡橋           on
					mesg プットオブジェ 狙撃ステージＡ       on
					mesg プットオブジェ 狙撃ステージＣ       on

				} else {
					if($i:プレイヤー位置Ｘ < 62000) {
						if($i:プレイヤー位置Ｚ < -100500) {
							// 左上
						} else if($i:プレイヤー位置Ｚ < -87000) {
							// 左中
						} else {
							// 左下
						}
					} else {
						if($i:プレイヤー位置Ｚ < -100500) {
							// 右上
						} else if($i:プレイヤー位置Ｚ < -87000) {
							// 右中
						} else {
							// 右下
						}
					}
				}
*/

			} else {
			//	#if d:DEBUG_PRINT
			//		print '*** Fukan Mode ***'
			//	#endif

				eval($f:Ｓローカルフラグ１ = d:_C_FUKAN)

				mesg プットオブジェ Ａ脚                 off
				mesg プットオブジェ Ｂ脚                 off
				mesg プットオブジェ ＢＣ連絡橋           off
				mesg プットオブジェ ＣＤ連絡橋           off
				mesg プットオブジェ ＥＦ連絡橋           off
				mesg プットオブジェ ＦＡ連絡橋           off
				mesg プットオブジェ Ｓ１中央棟           off
				mesg プットオブジェ クレーン             off
				mesg プットオブジェ Ｈ脚                 off
				mesg プットオブジェ ＨＩ連絡橋           off
				mesg プットオブジェ 西側オイルフェンス０ off
				mesg プットオブジェ 西側オイルフェンス１ off
				mesg プットオブジェ 西側オイルフェンス２ off

				if(`@ＧＥＴライデン状態 (d:_R_2F)`) {			// ２Ｆにいる
					mesg プットオブジェ Ｉ脚                 on
					mesg プットオブジェ ＪＫ連絡橋           on
					mesg プットオブジェ Ｋ脚                 on
					mesg プットオブジェ Ｓ２中央棟           on
					mesg プットオブジェ Ｅ脚                 on
					mesg プットオブジェ ＤＥ連絡橋           off
					mesg プットオブジェ 狙撃ステージＡ       off
					mesg プットオブジェ 狙撃ステージＣ       off

				} else if(`@ＧＥＴライデン状態 (d:_R_FUCHI)`) {	// 屋上の淵にいる
					mesg プットオブジェ Ｉ脚                 off
					mesg プットオブジェ ＪＫ連絡橋           off
					mesg プットオブジェ Ｋ脚                 off
					mesg プットオブジェ Ｓ２中央棟           off
					mesg プットオブジェ Ｅ脚                 on
					mesg プットオブジェ ＤＥ連絡橋           on
					mesg プットオブジェ 狙撃ステージＡ       on
					mesg プットオブジェ 狙撃ステージＣ       on

				} else if(`@ＧＥＴライデン状態 (d:_R_NAKA)`) {	// 屋上の中にいる
					mesg プットオブジェ Ｉ脚                 off
					mesg プットオブジェ ＪＫ連絡橋           off
					mesg プットオブジェ Ｋ脚                 off
					mesg プットオブジェ Ｓ２中央棟           off
					mesg プットオブジェ Ｅ脚                 off
					mesg プットオブジェ ＤＥ連絡橋           on
					mesg プットオブジェ 狙撃ステージＡ       on
					mesg プットオブジェ 狙撃ステージＣ       on

				} else if(`@ＧＥＴライデン状態 (d:_R_ELUDE)`) {	// エルード中
					mesg プットオブジェ Ｉ脚                 on
					mesg プットオブジェ ＪＫ連絡橋           on
					mesg プットオブジェ Ｋ脚                 on
					mesg プットオブジェ Ｓ２中央棟           on
					mesg プットオブジェ Ｅ脚                 on
					mesg プットオブジェ ＤＥ連絡橋           on
					mesg プットオブジェ 狙撃ステージＡ       on
					mesg プットオブジェ 狙撃ステージＣ       on
				}
			}
		}





	//---------------------------------------------------------
	// ハリアー
#ifndef d:BOSS_MODE
	if($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) {
#ifdef d:STAGE_W20B
		// ハリアー
		@SS_プットオブジェ ハリアー d:OBJ_NAME1
#endif
	}
#endif

	//---------------------------------------------------------
	// chara 透明壁 -p 頂点１Ｘ 頂点１Ｙ 頂点１Ｚ 頂点１高さ 頂点２Ｘ 頂点２Ｙ 頂点２Ｚ 頂点２高さ -a アトリビュート -m マップ名
#ifndef d:BOSS_MODE
	if(    (d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了 <= $w:p_story) \
		&& ($w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了)) {
		chara 透明壁 階段入口 \
			-p 58000 11500 -79500 3000 \
			   60000 11500 -79500 3000 \
			-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER | d:HZX_SEG_NO_ENEMY))
	}
#endif

	//---------------------------------------------------------
	// ファットマン引きずりイベント
#ifndef d:BOSS_MODE
	if($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了) {
		// 死体君
		//   -m モデル名 -d ポーズ -p 場所 -r 方向
		//	 -n 初期モーション番号  -s ステータス -e マルチウェイトモデル
		//	 -q アイテムプロック 確率 ...2 -f 掴みシフト ...3 -a far名
		//	 -j アイテムプロック VERYEASY  EASY  NORMAL  HARD  EXTREME
		//	 -b プロック ...
		if($f:w20c_ファットマン死亡フラグ == 1) {
			// 死んでいる
			eval($i:Ｓローカル変数Ｉ１ = (d:CORPS_ST_EVM))
		} else {
			// 生きている
			eval($i:Ｓローカル変数Ｉ１ = (d:CORPS_ST_EVM | d:CORPS_ST_LIVE | d:CORPS_ST_PIYO))
		}
		chara 死体君 ファットマン死体 \
			-m fat_def_sh_mt \
			-d 1 \							// 壁もたれ
			-p 53475,(11500+50),-103000 \
			-r 0,1024,0 \
			-s $i:Ｓローカル変数Ｉ１ \
			-f {
				// モーションが始まって１６フレーム目のシフト座標
				{ 0, -421, 1192 },			// 仰向け足
				{ 26, -394, 940 },			// 仰向け頭
				{ 0, -346, 1192 },			// うつぶせ足
				{ 46, -317, 946 },			// うつ伏せ頭
				// 持ち上げられてからのシフト座標
				{ 0, -382, 422 },			// 仰向け足
				{ 0, (-114-7), (291+120) },	// 仰向け頭
				{ 0, -294, 464 },			// うつぶせ足
			} \
			-e fat_nonc_dead_mh_mt \
			-j ドッグタグ_敵兵	[ dogtag:plt_w20c_fatman_ve ] \
								[ dogtag:plt_w20c_fatman_ea ] \
								[ dogtag:plt_w20c_fatman_no ] \
								[ dogtag:plt_w20c_fatman_ha ] \
								[ dogtag:plt_w20c_fatman_vh ]
		chara ファットマン死体ＩＫ補正 死体ＩＫ補正 -chara ファットマン死体

		if($w:コンフィグ設定 & d:CONFIG_BLOOD_OFF) {
			chara アニメテクスチャセット アニテク \
				-tex fat_nonc_dead_mh_mt fat_ooze_blood89_alp_ovl \
				-anmtex fat_no_ooze_blood
		}

		chara 透明壁 ファットマン壁 \
			-p 53000 11500 -105000 3000 \
			   58750 11500 -105000 3000 \
			-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_ENEMY))
		chara 透明壁 ファットマン壁 \
			-p 58750 11500 -105000 3000 \
			   58750 11500 -101000 3000 \
			-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_ENEMY))
		chara 透明壁 ファットマン壁 \
			-p 58750 11500 -101000 3000 \
			   53000 11500 -101000 3000 \
			-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_ENEMY))
		chara 透明壁 ファットマン壁 \
			-p 53000 11500 -101000 3000 \
			   53000 11500 -105000 3000 \
			-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_ENEMY))
	}
#endif





	//---------------------------------------------------------
	// 海面
	chara プラント海面 海面 \
		-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
		-l \
		-pos 52000,-40000,-90000 \
		-wave 1 2 3

	//---------------------------------------------------------
	// にせ海面
	chara プットオブジェ にせ海面 -m d:OBJ_NAME3  -r 0,0,0 -p 0,0,0 -s 400,400,400 -l d:LT2_NAME
	mesg  プットオブジェ にせ海面 off

	//---------------------------------------------------------
	// 海拡張
	chara 拡張海表示 海拡張 \
		-tex d_oil_sea_alp_ovl_mod0222 \
		-pos 52000,-40000,-90000 \
		-lines 2

	//---------------------------------------------------------
	// 海面制御
	trap ev001 d:PLAYER -mask ＊ \
		-exec {
			if($3 == いる) {
				mesg プラント海面   海面 表示状態 0
				mesg プラント海面   海面          slepp
				mesg プットオブジェ にせ海面      on
			} else if($3 == 出る) {
				mesg プラント海面   海面 表示状態 1
				mesg プットオブジェ にせ海面      off
			}
		}
	trap ev003 d:PLAYER -mask ＊ \
		-exec {
			if($3 == いる) {
				if($f:Ｓローカルフラグ１ == d:_C_FUKAN) {
					mesg プラント海面   海面 表示状態 0
					mesg プラント海面   海面          slepp
					mesg プットオブジェ にせ海面      on
				} else {
					mesg プラント海面   海面 表示状態 1
					mesg プットオブジェ にせ海面      off
				}
			} else if($3 == 出る) {
				mesg プラント海面   海面 表示状態 1
				mesg プットオブジェ にせ海面      off
			}
		}

	//---------------------------------------------------------
	// 空・太陽
	@昼空（レンズフレア指定） 52000 -30000 -90000 -800 -4400 2100 25

	//---------------------------------------------------------
	// 風
	@風基本設定

	//---------------------------------------------------------
	// 電灯虫（廃止）

	// chara 電灯虫 -n 匹数 // ４以上は４単位で -p 位置 // プレイヤー誘導時は無視 
	//   -b フラグ // 壁との当たり見るか 1:あり, 0:なし -s バウンド１ // 最小 -l バウンド２ // 最大
//	chara 電灯虫 電灯虫 \
//		-n 12 \
//		-p 63000,7250,-90250 \
//		-b 1 \
//		-s 63000,4500,-91500 \
//		-l 63625,8000,-89000

	//---------------------------------------------------------
	// プット頂点アニメオブジェ  -m $s:モデル名  -b $i:ベースフレーム数  -w $i:ランダムビット幅  -r $v:向き  -p $v:場所  -l $s:光源
	chara プット頂点アニメオブジェ ホロ \
		-m d:OBJ_NAME2 \
		-r 0,0,0 \
		-p 51250,11500,-82750 \
		-b 10 \
		-w 2

	//---------------------------------------------------------
	// バグ修正壁
	chara 透明壁 バグ修正壁 \
		-p 63000 12500 -77500 3000 \
		   63000 12500 -75000 3000 \
		-a (d:HZX_SEG_ATR_ALL & ~(d:HZX_SEG_NO_PLAYER | d:HZX_SEG_NO_ENEMY))
}





//=============================================================================
// フォグ、カメラ、ロード
//=============================================================================

//-----------------------------------------------------------------------------
// カメラ設定
proc カメラ設定 {
#if d:DEBUG_PRINT
	print '*** Camera Set ***'
#endif

	// カメラ
	//   -c チャンネル番号  -l カメラレベル  -p 優先度
	//
	//   シナリオカメラは最大16個までおける。
	//   チャンネル番号 ：
	//       ０：通常画面 １：小窓
	//   カメラレベル ：
	//       ０：カットインカメラ（主観より強） 強
	//       １：カットインカメラ（主観より弱） ↑
	//       ２：エリアカメラ		   ↓
	//       ３：ステージデフォルトカメラ	   弱
	//   優先度 ：
	//       エリアカメラに有効
	//       数字が小さい程優先度高
	chara カメラ ステージカメラ		-c 0 -l 3 -p  0
	chara カメラ サブカメラ１		-c 0 -l 2 -p 50
	chara カメラ サブカメラ２		-c 0 -l 2 -p 40
	chara カメラ サブカメラ３		-c 0 -l 2 -p 30
	chara カメラ モーションカメラ	-c 0 -l 2 -p 20
	chara カメラ モーションカメラＳ	-c 0 -l 2 -p 10
	chara カメラ でもデモカメラ		-c 0 -l 0 -p  0

	// カメラ設定
	//   -c カメラタイプ  -b バウンド小 バウンド大  -l リミット小 リミット大  -p カメラ位置  -t ターゲット位置  -r カメラ角度  -f カメラトラック  -a カメラ画角  -i カメラ補完方法（ＩＮ） カメラ補完方法（ＯＵＴ） 補完時間（ＩＮ） 補完時間（ＯＵＴ）  -d パッドフラグ  -z 各種フラグ  -k ＫＩＬＬ  -s ０オフ／１オン  -o ＯＮになるまでの待ち時間  -E スライドタイプ スライド待ち時間 スライド距離１ スライド距離２ スライドスピード [スライドフラグ]  -N 内部補完時間
	//
	//   -b -l は必要に応じて設定。-p -t -r はタイプに応じて使用するものが取得される。省略したオプションは、前回設定を引き継ぐ。
	//   フラグ：
	//       CAMERA_CUT_IN (カットインカメラ) CAMERA_NO_CUSHION (遊びなしカメラ) CAMERA_BOUNDCHANGE_QUICK (バウンド変更を即時に反映) CAMERA_INTERP_CAMERA (補完カメラ設定)
	//   カメラタイプ：
	//       ０：上俯瞰追従型（トラックと角度を指定） １：位置・角度固定型（ターゲット位置とカメラ位置を指定） ２：位置固定、追従型（カメラ位置を指定） ３：位置固定、Ｘ角度追従型（カメラ位置を指定） ４：ターゲット固定、プレイヤー追従カメラ（ターゲットとトラックを指定）
	//   補完方法：
	//       −１：指定なし ０：一瞬 １：補完シューン ２：補完ウニョーン ３：補完ウーニャーン ４：一定時間で
	//       補完時間は４番指定のとき有効。補完（ＩＮ）の指定は必須。２つのカメラ間の補完方法は、ＯＵＴ側カメラのＯＵＴ補完関数が指定してある場合にはその関数を使用し、指定がない場合には、ＩＮ側カメラのＩＮ補完関数を使用するようになっている。
	//   パッドフラグ：
	//       このフラグを指定しておくと、パッドの上が画面の上になるように自動調節する。
	//   スライドカメラ設定方法：
	//       スライドタイプ (０／１ − 左右タイプ／前後タイプ) スライド待ち時間 (スライドするまでの待ちフレーム数) スライド距離１・２ (左右タイプの場合 左右スライド距離。前後タイプの場合 接近距離、離れた距離) スライドスピード (そのまんま) スライドフラグ (1にしておくと、入った時スライドしていない状態から始まります。)
	//   補完カメラの設定方法：
	//       補完カメラとは？ − 設定したカメラに指定フレーム数かけて移動するカメラ。例として、オルガ戦のパンカメラ。
	//       設定方法 − -z に d:CAMERA_INTERP_CAMERA を指定。さらに -interp_time に補完フレーム数を指定。

	// デフォルトカメラ
	chara カメラ設定 ステージカメラ \
		-c 0 \
		-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
		-l 44000    ,13090    ,-108400   69000    ,d:CAM_MAX,-79500 \
		-r 555,2048,0 \
		-f 8600 \
		-a 200 \
		-i 1 -1 0 0 \
		-s 1

	// ２階
	trap ca001 d:PLAYER \
		-camera -mask ＊ \
		-exec {
			if($4 < 47000) {
				@CS_スムーズカメラ昇順 $4 43000 47000 40000 47000
				eval($i:補完ＢＸ = $i:CS_結果１)
				if($4 < 43000) {
					eval($i:補完ＬＸ = 43000)
				} else {
					eval($i:補完ＬＸ = $4)
				}

				eval($i:補完ＢＹ = 10000)
				eval($i:補完ＬＹ =  7000)

				eval($i:補完ＢＺ = $6+7500)
				eval($i:補完ＬＺ = $6)

			} else if($4 > 58750) {
				@CS_スムーズカメラ昇順 $4 58750 62750 58750 64000
				eval($i:補完ＢＸ = $i:CS_結果１)
				if($4 > 62750) {
					eval($i:補完ＬＸ = 62750)
				} else {
					eval($i:補完ＬＸ = $4)
				}

				@CS_スムーズカメラ昇順 $6 -89500 -85500 10900 10000
				eval($i:補完ＢＹ = $i:CS_結果１)
				@CS_スムーズカメラ昇順 $6 -89500 -85500 6000 7000
				eval($i:補完ＬＹ = $i:CS_結果１)

				if($6 < -90000) {
					eval($i:補完ＬＺ = -90000)
				} else {
					eval($i:補完ＬＺ = $6)
				}
				eval($i:補完ＢＺ = $i:補完ＬＺ+7500)

			} else {
				eval($i:補完ＢＸ = $4)
				eval($i:補完ＬＸ = $4)

				eval($i:補完ＢＹ = 10000)
				eval($i:補完ＬＹ =  7000)

				eval($i:補完ＢＺ = $6+7500)
				eval($i:補完ＬＺ = $6)
			}

			chara カメラ設定 サブカメラ１ \
				-c 1 \
				-p $i:補完ＢＸ,$i:補完ＢＹ,$i:補完ＢＺ \
				-t $i:補完ＬＸ,$i:補完ＬＹ,$i:補完ＬＺ \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}

	// 階段
	trap ca002 d:PLAYER \
		-camera -mask ＊ \
		-exec {
			eval($i:補完ＢＸ = $4)
			eval($i:補完ＬＸ = $4)

			@CS_スムーズカメラ昇順 $4 52000 58000 10000 15500
			eval($i:補完ＢＹ = $i:CS_結果１)
			@CS_スムーズカメラ昇順 $4 52000 58000  7000 13000
			eval($i:補完ＬＹ = $i:CS_結果１)

			eval($i:補完ＢＺ = $6+7500)
			eval($i:補完ＬＺ = $6)

			chara カメラ設定 サブカメラ１ \
				-c 1 \
				-p $i:補完ＢＸ,$i:補完ＢＹ,$i:補完ＢＺ \
				-t $i:補完ＬＸ,$i:補完ＬＹ,$i:補完ＬＺ \
				-a 200 \
				-i 3 3 0 0 \
				-s $3
		}

	// ヘリポート
	trap ca003 d:PLAYER \
		-camera -mask ？ \
		-exec {
			chara カメラ設定 サブカメラ１ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,19540    ,d:CAM_MAX \
				-l 44000    ,13090    ,-108400   69000    ,d:CAM_MAX,-79500 \
				-r 555,2048,0 \
				-f 8600 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3 \
				-z d:CAMERA_NO_CUSHION
		}

	// ハリアー
#ifndef d:BOSS_MODE
	if($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) {
		trap ca101 d:PLAYER \
			-camera -mask ？ \
			-exec {
				chara カメラ設定 サブカメラ３ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 910,2048,0 \
					-f 9000 \
					-a 200 \
					-i 3 3 0 0 \
					-s $3 \
					-z d:CAMERA_NO_CUSHION
			}
	}
#endif

	// ファットマン引きずり
#ifndef d:BOSS_MODE
	if($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了) {
		trap ca101 d:PLAYER \
			-camera -mask ？ \
			-exec {
				if($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了) {
					chara カメラ設定 サブカメラ３ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 910,2048,0 \
						-f 9000 \
						-a 200 \
						-i 3 3 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION
				}
			}
	}
#endif

	// オブジェクト裏
	trap ca102 d:PLAYER \
		-camera -mask ？ \
		-state d:TRP_STATE_SQUAT d:TRP_STATE_GROUND \
		-exec {
			chara カメラ設定 サブカメラ２ \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 880,2048,0 \
				-f 10000 \
				-a 200 \
				-i 3 3 0 0 \
				-s $3 \
				-z d:CAMERA_NO_CUSHION
		}

	// 張付き用
	trap ca301 d:PLAYER \
		-camera -mask ＊ \
		-state d:TRP_STATE_CAUTION -d 1536 256 \
		-exec {
			chara カメラ設定 サブカメラ２ \
//				-c 1 \
//				-p 41648,13338,-78027 \
//				-t 43650,6899,-81389 \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 503,1698,0 \
				-f 8098 \
				-a 200 \
				-i 2 2 0 0 \
				-s $3
		}
	trap ca302 d:PLAYER \
		-camera -mask ＊ \
		-state d:TRP_STATE_CAUTION -d 2048 256 \
		-exec {
			chara カメラ設定 サブカメラ２ \
//				-c 1 \
//				-p 47646,12204,-76713 \
//				-t 47646,6646,-79760 \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 607,2048,0 \
				-f 7419 \
				-a 200 \
				-i 2 2 0 0 \
				-s $3
		}
	trap ca303 d:PLAYER \
		-camera -mask ＊ \
		-state d:TRP_STATE_CAUTION -d 2048 256 \
		-exec {
			chara カメラ設定 サブカメラ２ \
//				-c 1 \
//				-p 58516,12981,-74908 \
//				-t 56522,7519,-79660 \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 530,2308,0 \
				-f 7509 \
				-a 200 \
				-i 2 2 0 0 \
				-s $3
		}
	trap ca304 d:PLAYER \
		-camera -mask ＊ \
		-state d:TRP_STATE_CAUTION -d 2560 256 \
		-exec {
			chara カメラ設定 サブカメラ２ \
//				-c 1 \
//				-p 63551,11874,-77547 \
//				-t 62435,7559,-80022 \
				-c 0 \
				-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
				-r 427,2183,0 \
				-f 8500 \
				-a 200 \
				-i 2 2 0 0 \
				-s $3
		}
}

proc スタートカメラセット {
#if d:DEBUG_PRINT
	print '*** Start Camera Set ***'
#endif

	if($s:登場ポイント == sp_w20a1_w20b_0) {
		chara カメラ設定 モーションカメラ \
			-c 1 \
			-p 64278,7484,-86208 \
			-t 63060,5804,-89915 \
			-a 200 \
			-i 0 2 0 0 \
			-s 1
	}
}

proc スタートカメラオフ {
#if d:DEBUG_PRINT
	print '*** Start Camera Off ***'
#endif

	chara カメラ設定 モーションカメラ -s -1
}

//-----------------------------------------------------------------------------
// ストーリー設定
proc ストーリー設定 {
#if d:DEBUG_PRINT
	print '*** Story Set ***'
#endif

	// オルガ登場デモ
	if($f:w20b_オルガデモフラグ == 0) {
		if($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) {
			trap ev002 d:PLAYER \
				-mask 入る \
				-exec {
					if($f:w20b_オルガデモフラグ == 0) {
						if(`command ゲームオーバーチェック` == 0 ) {
							chara カメラ設定 でもデモカメラ \
								-c 1 \
								-p 54837,8359,-78436 \
								-t 60116,6610,-81491 \
								-a 200 \
								-i 0 0 0 0 \
								-s 1
							command パッド操作 -release
							chara delay ディレイ \
								-time 25 \ 
								-exec{
									if(`command ゲームオーバーチェック` == 0 ) {
			//							@ポリゴンデモロード d:ST:... の変わりに↓
											preseek 'd021p01'
											command 無線設定 -reset
											command パッド操作 -release
											command プレイヤー無敵セット
											mesg コマンダー 敵兵セット 視界オフ

											// ポリデモのときはレーダーオフするためのフラグ
											eval($f:global_polygon_demo = 1);

											// フェードインスタート
											chara フェードインアウト フェードアウト \
												-c 0,0,0 128 \
												-t d:FADEOUT_TIME \
												-p ポリデモロードスタート
									}
								}
						}
					}
				}
		} else {
			eval($f:w20b_オルガデモフラグ = 1)
		}
	}


#ifndef d:BOSS_MODE
	// ファットマン死亡シナリオデモ
	if($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１終了) {
		@ファットマン死亡シナリオデモ_キャラセット
	}
#endif


	// 忍者登場デモ
	if(    (d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１終了 <= $w:p_story) \
		|| ($w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了)) {
		trap ev102 d:PLAYER \
			-mask 入る \
			-exec {
				if($w:p_story == d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了) {
					if(`command ゲームオーバーチェック` == 0 ) {
						// レーダーＯＦＦ
						if($f:w20c_レーダーＯＮフラグ == 1) {
							eval($f:w20a_ノードフラグ = 0)
							command varsave $f:w20a_ノードフラグ
						}

						eval ($w:アイテム数Ｒ[d:アイテム:カード]     = 2)
						eval ($w:アイテム数Ｒ[d:アイテム:ゴル兵制服] = 1)
						eval ($w:アイテム数Ｒ[d:アイテム:携帯電話]   = 1)
						preseek 'r_plt0'
						@ポリゴンデモロード d:ST:P036_01_P01忍者登場１ポリゴンデモ１開始
					}
				}
			}
	}
}

//-----------------------------------------------------------------------------
// ファットマン死亡シナリオデモ_キャラセット
proc ファットマン死亡シナリオデモ_キャラセット {
#if d:DEBUG_PRINT
	print '*** Fatman Dead SDemo: Chara Set ***'
#endif

	@シナリオデモキャラセット
}

//-----------------------------------------------------------------------------
// ファットマン死亡シナリオデモ開始
proc ファットマン死亡シナリオデモ開始 {
#if d:DEBUG_PRINT
	print '*** Fatman Dead SDemo: Start ***'
#endif

	chara デモキャンセルチェック デモキャン -p ファットマン死亡シナリオデモキャンセル
	@シナリオデモ開始処理

	// パンカメラ
	chara カメラ設定 でもデモカメラ \
		-c 1 \
		-p 76549,13884,-89218 \
		-t 74443,14392,-88945 \
		-a 200 \
		-i 0 0 0 0 \
		-s 1
	chara delay ディレイ \
		-time 40 \ 
		-exec{
			if($f:Ｓローカルフラグ３ == 0) {
				command ＳＥセットＶＰ -s d:se_code:SD_A_AF_ROLL1 -v 0x3f -p 0x20
			}
		}
	chara delay ディレイ \
		-time (40+84) \ 
		-exec{
			if($f:Ｓローカルフラグ３ == 0) {
				command ＳＥセットＶＰ -s d:se_code:SD_V_FATHAA01 -v 0x28 -p 0x20
			}
		}
	chara delay ディレイ \
		-time (40+216) \ 
		-exec{
			if($f:Ｓローカルフラグ３ == 0) {
				command ＳＥセットＶＰ -s d:se_code:SD_V_FATHAA01 -v 0x18 -p 0x20
			}
		}
	chara delay ディレイ \
		-time 65 \ 
		-exec{
			if($f:Ｓローカルフラグ３ == 0) {
				command セットサウンドコード -c d:SNG_PLAY_07
			}
		}
	chara delay ディレイ \
		-time 100 \ 
		-exec{
			chara カメラ設定 でもデモカメラ \
				-c 1 \
				-p 74491,13884,-100894 \
				-t 72388,14392,-100615 \
				-a 200 \
				-z d:CAMERA_INTERP_CAMERA \ //補完カメラフラグを指定。
				-interp_time 310 \ 			//補完時間を指定
				-s 1
		}

	chara delay ディレイ \
		-time 450 \ 
		-exec{
			@ファットマン死亡シナリオデモキャンセル
		}
}

//-----------------------------------------------------------------------------
// ファットマン死亡シナリオデモキャンセル
proc ファットマン死亡シナリオデモキャンセル {
#if d:DEBUG_PRINT
	print '*** Fatman Dead SDemo: Cancel ***'
#endif

	mesg デモキャンセルチェック デモキャン kill

	if($f:Ｓローカルフラグ３ == 0) {
		eval($f:Ｓローカルフラグ３ = 1)

		command ＳＥセットＶＰ -s d:se_code:SD_A_AF_MUTE1 -v 0x3f -p 0x20
		command セットサウンドコード -c d:SNG_FOUTS_S

		if(`command ゲームオーバーチェック` == 0 ) {
			preseek 'd021p01'
			@ポリゴンデモロード d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始
		}
	}
}

//-----------------------------------------------------------------------------
// ロード設定
proc ロード設定 {
#if d:DEBUG_PRINT
	print '*** Load Set ***'
#endif

	// @ステージ終了時処理 目標位置  終了時方向  呼び出しプロックに入る  入ってゆく姿勢

	// Ｅ脚屋内へ (w20a)
	trap ld001 d:PLAYER \
		-mask 入る \
		-exec {
			if($f:ロードチェックＯＮフラグ == 1) {
				@爆弾タイマー残り時間保存
				eval($s:登場ポイント = sp_w20a1_w20b_0)
				@スタートカメラセット

				// 敵兵状態やりとり
				if($f:Ｓローカルフラグ２ == 1) {
					@敵兵状態保存 敵兵:01 敵兵:02
				}

				preseek 'w20a'
				@ステージ終了時処理（ステージ名表示） 60750 4500 -90250 0 mv_w20b_w20a1_0 d:DEFAULT_M_TYPE d:wn_Ｅ脚集配場２Ｆ
			}
		}
}

proc ロードプロック {
#if d:DEBUG_PRINT
	print '*** Load Proc ***'
#endif

	if($s:呼び出しプロック == mv_w20b_w20a1_0) {
		// Ｅ脚屋内へ
		@mv_w20b_w20a1_0
	}
}

proc ポリデモロードスタート {
#if d:DEBUG_PRINT
	print '*** Poly-Demo Load Start ***'
#endif

	// オルガ登場デモ
	if($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) {
		if(`command ゲームオーバーチェック` == 0 ) {
			load "d021p01"
		}

	// ファットマン登場デモ
	} else if($w:p_story == d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始) {
		command メニュー設定 -radio on	//祝復帰！
		if(`command ゲームオーバーチェック` == 0 ) {
			load "d021p01"
		}
		// r_plt0 との違い
		// rai/rai_non_carry_body_start_f_fat.mt3	//死体運び抱えうつぶせ開始
		// rai/rai_non_carry_body_start_b.mt3		//死体運び抱えあおむけ開始
		// rai/rai_non_carry_body_idle_fat.mt3		//死体運び抱え静止
		// rai/rai_non_carry_body_walk_fat.mt3		//死体運び抱え歩き
		// rai/rai_non_carry_body_end_fat.mt3		//死体運び抱え下ろし

	// ファットマン死亡デモ
	} else if($w:p_story == d:ST:P034_01_P01ファットマン死亡１ポリゴンデモ１開始) {
		if(`command ゲームオーバーチェック` == 0 ) {
			load "d021p01"
		}

	// 忍者登場デモ
	} else if($w:p_story == d:ST:P036_01_P01忍者登場１ポリゴンデモ１開始) {
		if(`command ゲームオーバーチェック` == 0 ) {
			load "d021p01" -resident 'r_plt0';
		}
	}
}





//=============================================================================
// ステージ進入時処理
//=============================================================================

proc ステージ進入時処理 {
#if d:DEBUG_PRINT
	print '*** Stage Start Proc ***'
#endif

	// @ステージ開始時処理 タイプ(スライドドア、水密ドア、何もなし)   ドアの名前   移動距離   状態(前のステージ状態、走り、歩き)
	if($s:登場ポイント == sp_w20a1_w20b_0) {
		@ステージ開始時処理（ステージ名表示） d:LOAD_TYPE:SL_DOOR_TYPE Ｅ脚屋内へのドア 2000 d:DEFAULT_M_TYPE d:wn_Ｅ脚ヘリポート

	} else {
		@ステージ開始時処理 d:LOAD_TYPE:NO_MOVE_TYPE 0 0 d:DEFAULT_M_TYPE
	}
}
