//=============================================================================
// w12ac_common.h
// Ａ脚・屋上(共通)
// 2001/06/12 S.Yamashita
// $Id: w12ac_common.h,v 1.86 2001/11/06 08:26:59 usr03692 Exp $
//

// [登場ポイント]
//   w11a				sp_w11a2_w12a_0		w12a
//   w11b				sp_w11b_w12a_0		w12c
//   w11c				sp_w11c_w12a_0		w12c
//   w12b(左)			sp_w12b0_w12a_0		w12a/w12c
//   w12b(右)			sp_w12b0_w12a_1		w12a/w12c

//=============================================================================
// define
//=============================================================================

//-----------------------------------------------------------------------------
// スイッチ

//----- w12a
// #define STAGE_PLANT 1
// #define STAGE_W12A  1
//
// #define ENE_P_GOL 1

//----- w12c
// #define STAGE_PLANT 1
// #define STAGE_W12C  1
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
#define BOMB_STAGE     w12a		// 爆弾解体後処理用

#define SCREENSHOT     0		// スクリーンショット
#define KEYBOARD       0		// ＵＳＢキーボード

//-----------------------------------------------------------------------------
// マップ関係

#define HZX_NAME   w12a

#define KMS_NAME1  w12a_12a				// Ａ脚壁面
#define KMS_NAME2  w12a_13a				// ＡＢ連絡橋
#define KMS_NAME3  w12a_15a				// ＢＣ連絡橋１
#define KMS_NAME4  w12a_20a				// Ｅ脚
#define KMS_NAME5  w12a_21a				// ＥＦ連絡橋
#define KMS_NAME6  w12a_23a				// ＦＡ連絡橋
#define KMS_NAME7  w12a_24a				// 中央棟
#define KMS_NAME8  w12a_other00			// オイルフェンス１
#define KMS_NAME9  w12a_other01			// オイルフェンス２
#define KMS_NAME10 w12a_other02			// オイルフェンス３
#define KMS_NAME11 w12a_other03			// オイルフェンス４
#define KMS_NAME12 w12a_15b				// ＢＣ連絡橋２(フォーチュン登場デモ前)
#define KMS_NAME13 w12a_15c				// ＢＣ連絡橋２(フォーチュン登場デモ後)

#define OBJ_NAME1  w12a_crane			// クレーン
#define OBJ_NAME2  w12a_elvwall			// エレベーター壁
#define OBJ_NAME3  w12a_objdr00			// 朝バージョン。閉まっている網扉
#define OBJ_NAME4  w12a_objdr01			// 朝バージョン。開いている網扉
#define OBJ_NAME5  w12a_elvwall_futa	// エレベータ蓋
#define OBJ_NAME7  w20b_dammy_sea		// にせ海面

#define OBJ_NAME10 w12_ami00
#define OBJ_NAME11 w12_ami01
#define OBJ_NAME12 w12_ami02
#define OBJ_NAME13 w12_ami03
#define OBJ_NAME14 w12_ami04
#define OBJ_NAME15 w12_ami05
#define OBJ_NAME16 w12_ami06
#define OBJ_NAME17 w12_ami07
#define OBJ_NAME20 w12_drami00
#define OBJ_NAME21 w12_drami01
#define OBJ_NAME22 w12_drami02

#define KMS_NAME14 w12_12b0				// Ａ脚内ロード部左
#define KMS_NAME15 w12_12b1				// Ａ脚内ロード部右



#ifdef d:STAGE_W12A
	#define LT2_NAME   w12a
	#define KMS_NAME0  w12a				// Ａ脚

	#define KMS_NAME20 w12a_sdw00		// 影投影モデル
	#define KMS_NAME21 w12a_sdw01		// 影投影モデル
	#define OBJ_NAME6  w12a_elvsk01		// 影付エレベーターの柵
	#define DOOR_NAME1 w12a_dr00		// 扉
#else
	#define LT2_NAME   w12c
	#define KMS_NAME0  w12c				// Ａ脚

	#define OBJ_NAME6  w12c_elvsk02		// 影付エレベーターの柵
	#define DOOR_NAME1 w12c_dr00		// 扉
#endif





//=============================================================================
// include
//=============================================================================

#include "rai_w12a.mh"	// ステージ固有モーションリスト

#include "stdch.h"
#include "sload.h"
#include "beyond.h"
#include "fall.h"
#include "jump.h"
#include "sound.h"
#include "enevoice.h"
#include "item_info.h"

#if d:KEYBOARD 
#include "usbkbd.h"
#endif

//-----------------------------------------------------------------------------
// ローカル

eval($f:Ｓローカルフラグ１ = 0)	// カモメ危険モード制御用

eval($w:Ｓローカル変数Ｗ１ = 0)
#define _R_NORMAL       0	// 通常
#define _R_ELUDE        1	// エルード中
#define _R_SLIP         2	// 糞滑り中
#define _R_NOT_OPEN     3	// 金網ドアが開かない中
#define _R_DROWN_GOVER 11	// 溺死
#define _R_FALL_GOVER  12	// 落下死
#define _R_CRASH_GOVER 13	// 激突死

eval($w:Ｓローカル変数Ｗ２ = 0)
#define _E_NORMAL       0	// 通常
#define _E_ON_UP_DEMO   1	// ＯＮ上昇エレベータデモ中
#define _E_ON_DOWN_DEMO 2	// ＯＮ降下エレベータデモ中
#define _E_GO_DOWN      3	// エレベータ降下中

eval($w:Ｓローカル変数Ｗ３ = 0)
#define _A_SOUTH        0	// 南側
#define _A_NORTH        1	// 北側
#define _A_NORTH_EW     2	// 北東・北西
#define _A_SAKU_HARI    3	// 柵張付き

eval($i:Ｓローカル変数Ｉ１ = 0)	// 海表示カウンター

#include "stageset.h"		// ステージ設定共通ヘッダ
#include "w12ac_sdemo.h"	// シナリオデモヘッダ





//=============================================================================
// メイン
//=============================================================================

chara マップ 屋上 \
	-k d:KMS_NAME0 \
	-k d:KMS_NAME7 \
	-h d:HZX_NAME, 0 \
	-l d:LT2_NAME

command マップ設定 屋上 -set {

	//----- 基本設定
	@特別処理
	@SS_サウンド設定
	@SS_跳弾弾痕設定
	@水位設定
	@レーダーオンオフチェック $f:w12b_ノードフラグ
	@常駐キャラ設定

	@昇降機設置
	@プレイヤー設定
#if d:ENEMY_SET
	@敵兵設定
#endif
	@SS_ＢＧＭ設定
	@爆弾タイマー設置
	@プラント編全体マップ設定 d:Ａ脚マップ

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
}

#include "w12ac_cdc.ch"			// ※※ 通信機の include は移動させないこと！！ ※※
#include "p_force_cdc.ch"

command マップ表示 -show 屋上

// 遠景登録
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

// 影投影
command	影投影ハザード -name 屋上 -group 21

@ステージ進入時処理





//=============================================================================
// プロシージャ
//=============================================================================

//-----------------------------------------------------------------------------
// ライデン状態
proc ＳＥＴライデン状態 $:状態 {
	if(    ($w:Ｓローカル変数Ｗ１ != d:_R_DROWN_GOVER) \
		&& ($w:Ｓローカル変数Ｗ１ != d:_R_FALL_GOVER ) \
		&& ($w:Ｓローカル変数Ｗ１ != d:_R_CRASH_GOVER)) {

		if($:状態 == d:_R_SLIP) {
			if($w:Ｓローカル変数Ｗ１ != d:_R_ELUDE) {
				eval($w:Ｓローカル変数Ｗ１ = $:状態)
			}
		} else {
			eval($w:Ｓローカル変数Ｗ１ = $:状態)
		}

	#if d:DEBUG_PRINT
		if       ($w:Ｓローカル変数Ｗ１ == d:_R_NORMAL) {
			print 'DEBUG PRINT: *** Rai Normal ***'
		} else if($w:Ｓローカル変数Ｗ１ == d:_R_ELUDE) {
			print 'DEBUG PRINT: *** Rai Elude ***'
		} else if($w:Ｓローカル変数Ｗ１ == d:_R_SLIP) {
			print 'DEBUG PRINT: *** Rai Slip ***'
		}
	#endif
	}
}
proc ＧＥＴライデン状態 $:状態 {
	if($w:Ｓローカル変数Ｗ１ == $:状態) {
		return 1
	} else {
		return 0
	}
}

//-----------------------------------------------------------------------------
// エレベータ状態
proc ＳＥＴエレベータ状態 $:状態 {
	eval($w:Ｓローカル変数Ｗ２ = $:状態)
}
proc ＧＥＴエレベータ状態 $:状態 {
	if($w:Ｓローカル変数Ｗ２ == $:状態) {
		return 1
	} else {
		return 0
	}
}

//-----------------------------------------------------------------------------
// エリア状態
proc ＳＥＴエリア状態 $:状態 {
	eval($w:Ｓローカル変数Ｗ３ = $:状態)
}
proc ＧＥＴエリア状態 $:状態 {
	if($w:Ｓローカル変数Ｗ３ == $:状態) {
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

	//---------------------------------------------------------
	// 昇降機用進入禁止ゾーン
	if(    (   ($w:p_story <  d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
			|| (d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story)) \
		&& ($f:w12a_Ａ脚ポンプ室爆弾解体前にセンサーＢゲット == 0)) {
		command セットゾーンフラグ d:HZX_ZONE_ZINTRPT 0 5000 -12250
	}
}

//-----------------------------------------------------------------------------
// 水位設定
proc 水位設定 {
#if d:DEBUG_PRINT
	print '*** Water Level Set ***'
#endif

	command 水位設定 -lv -40000
//	command 弾水飛沫処理
}

//-----------------------------------------------------------------------------
// プレイヤー設定
proc プレイヤー設定 {
#if d:DEBUG_PRINT
	print '*** Player Set ***'
#endif

	// フォーチュン戦直後はソコムを装備
	if($s:登場ポイント == sp_w11c_w12a_0) {
		@デモ後武器アイテム制御 d:デモ後必ず素手 d:デモ後特殊装備なし
	}

	// プレイヤー設定
	//   $:強制モーションファイル $:ＬＯＤ距離
	@SS_プレイヤー設定 rai_w12a 7000
}

//-----------------------------------------------------------------------------
// 敵兵設定

// 警備兵
//   �� 3(4)
//
// 守備位置
//       0 5000  -8000 屋上
//       0 5000 -15000 屋上
//    2750 5000 -12375 屋上
//   -2750 5000 -12375 屋上
//
// 再発生位置
//   -6250 0 -2250 屋上
//    6250 0 -2250 屋上

//----- 警備兵設定
//        $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ $:ＩＤ
proc 警備兵セット１ {
#if d:DEBUG_PRINT 
	print '*** Watcher Set 1 ***'
#endif

	@SS_警備兵設定 敵兵:01 3 1 ($i:SS_敵兵ステータス) 0 5000 -3000 屋上 (d:SS_警備兵ＩＤ:Ａ脚屋上兵)
}
proc 警備兵セット２ {
#if d:DEBUG_PRINT 
	print '*** Watcher Set 2 ***'
#endif

	@SS_警備兵設定 敵兵:01 4 0 ($i:SS_敵兵ステータス) 0 5000 -3000 屋上 (d:SS_警備兵ＩＤ:Ａ脚屋上兵)
}

//----- サポート兵設定
//        $:名前 $:ルート $:ステータス $:相手 $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
proc サポート兵セット１ {
#if d:DEBUG_PRINT
	print '*** Support Set 1 ***'
#endif

	if($s:登場ポイント == sp_w12b0_w12a_0) {
		@SS_サポート兵設定 敵兵:31 1 ($i:SS_敵兵ステータス) 敵兵:01
	} else {
		@SS_サポート兵設定 敵兵:31 2 ($i:SS_敵兵ステータス) 敵兵:01
	}
}

//----- アタッカー設定
//        $:名前 $:ルート $:ステータス $:装備
proc アタッカーセット１ {
#if d:DEBUG_PRINT 
	print '*** Attacker Set 1 ***'
#endif

	if($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
		@SS_アタッカー設定 敵兵:21 1 ($i:SS_敵兵ステータス) 0

	} else if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
		@SS_アタッカー設定 敵兵:21 1 ($i:SS_敵兵ステータス) 0
		@SS_アタッカー設定 敵兵:22 1 ($i:SS_敵兵ステータス) 0

	} else {
#ifdef d:STAGE_W12C
		@SS_アタッカー設定 敵兵:21 1 ($i:SS_敵兵ステータス) 4
		@SS_アタッカー設定 敵兵:22 1 ($i:SS_敵兵ステータス) 4
#endif
	}

	// サポート兵
	if($w:p_story >= d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
		@サポート兵セット１
	}
}
proc アタッカーセット２ {
#if d:DEBUG_PRINT 
	print '*** Attacker Set 2 ***'
#endif

	if($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
		@SS_アタッカー設定 敵兵:21 2 ($i:SS_敵兵ステータス) 0

	} else if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
		@SS_アタッカー設定 敵兵:21 2 ($i:SS_敵兵ステータス) 0
		@SS_アタッカー設定 敵兵:22 2 ($i:SS_敵兵ステータス) 0

	} else {
#ifdef d:STAGE_W12C
		@SS_アタッカー設定 敵兵:21 2 ($i:SS_敵兵ステータス) 4
		@SS_アタッカー設定 敵兵:22 2 ($i:SS_敵兵ステータス) 4
#endif
	}

	// サポート兵
	if($w:p_story >= d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
		@サポート兵セット１
	}
}





//----- コマンダー
proc 警備兵コマンダー {
#if d:DEBUG_PRINT 
	print '*** Watcher Commander ***'
#endif

	if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
		// 初登場時


	} else if($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
		// 爆弾解体前
		chara 警備コマンダー wccomm -e { 警備兵セット１ }

	} else if($w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) {
		// 爆弾解体
		if($f:w12a_Ａ脚ポンプ室爆弾解体前にセンサーＢゲット == 0) {
			chara 警備コマンダー wccomm -e { 警備兵セット１ }
		} else {
			chara 警備コマンダー wccomm -e { 警備兵セット２ }
		}

	} else if($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) {
		// センサーＢ
		if($s:登場ポイント != sp_w11b_w12a_0) {		// エレベータで登場時は敵兵なし
			chara 警備コマンダー wccomm -e { 警備兵セット２ }
		}

	} else if($w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了) {
		// フォーチュン戦後
		if($s:登場ポイント != sp_w11c_w12a_0) {		// エレベータで登場時は敵兵なし
			chara 警備コマンダー wccomm -e { 警備兵セット１ }
		}

	} else if($w:p_story < d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了) {
		// ファットマン戦後
		chara 警備コマンダー wccomm -e { 警備兵セット１ }

	} else {
		// エイムズ後
		chara 警備コマンダー wccomm -e { 警備兵セット１ }
	}
}

proc 攻撃兵コマンダー {
#if d:DEBUG_PRINT 
	print '*** Attacker Commander ***'
#endif

	if($s:登場ポイント == sp_w12b0_w12a_0) {
		chara 攻撃コマンダー atcomm -e { アタッカーセット１ }
	} else {
		chara 攻撃コマンダー atcomm -e { アタッカーセット２ }
	}
}





//----- 敵兵設定

#include "w12a_kamome.gcl"	// modified by Okuta 2001/05/07

proc 敵兵設定 {
#if d:DEBUG_PRINT
	print '*** Enemy Set ***'
#endif

	//---------------------------------------------------------
	// 敵兵ステータス
	eval($i:SS_敵兵ステータス = d:ENE_STATUS_NO_FINGER)	// 通常は 0

	//---------------------------------------------------------
	// コマンダー
	//   -w 警備コマンダーproc  -a 攻撃コマンダーproc  -r 復活場所 マップＩＤ  -c 待機場所 マップＩＤ  -v 音声  -m 危険モード  -p クリアリングエリア ルート ポイント プロック  -x 司令官危険音声  -y 司令官警戒音声  -g 銃ライト  -z 麻酔持続時間  -b 危険モードでの復活人数 
	if(    ($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) \	// 初登場
		|| ($s:登場ポイント == sp_w11b_w12a_0) \							// エレベータで登場
		|| ($s:登場ポイント == sp_w11c_w12a_0) \
		|| ((d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 <= $w:p_story) && ($w:p_story < d:ST:P032_01_P01ファットマン登場１ポリゴンデモ１開始))) {							// フォーチュン戦直後エレベータで登場
		// 潜入モードへ戻す
		// ALERT_MODE_SENNYU
		// ALERT_MODE_KIKEN
		// ALERT_MODE_KAIHI
		// ALERT_MODE_TANSAKU
		eval($w:スタートアラートモード = d:ALERT_MODE_SENNYU)

	} else {
		chara コマンダー 敵兵セット \
			-w 警備兵コマンダー \
			-a 攻撃兵コマンダー \
			-d 屋上 \
			-r -6250,0,-2250 屋上 6250,0,-2250 屋上 \
			-c -6250,0,-2250 屋上 6250,0,-2250 屋上 \
			-v [ene_plant_voice:01] \
			-x [ghq_area_alert_voice:w12a] \
			-y [ghq_caution_voice:w12a] \
			-z $i:敵標準麻酔持続 \
			-h $i:標準警戒時間 \
			-o $i:敵標準気絶持続 \
			-s d:ENE_STAGE_GPS \
			-b $w:敵標準再発生数 \
			-m $w:スタートアラートモード
	}

	//---------------------------------------------------------
	// かもめ
	if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
		@w12a_かもめ_40		// modified by Okuta 2001/05/07
	} else {
		@w12a_かもめ_20
	}

	// カモメ危険モード制御
	chara プロック連続実行 連続実行１ \
		-time -1 \
		-exec {
			if(($f:Ｓローカルフラグ１ == 1) && ($w:アラートモード == d:ALERT_MODE_SENNYU)) {
				eval($f:Ｓローカルフラグ１ = 0)
				@w12a_かもめ危険モード終了
			} else if(($f:Ｓローカルフラグ１ == 0) && ($w:アラートモード != d:ALERT_MODE_SENNYU)) {
				eval($f:Ｓローカルフラグ１ = 1)
				@w12a_かもめ危険モード開始
			} else if($f:Ｓローカルフラグ１ == 0) {
				@w12a_かもめ自由飛行管理
			}
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

	@SS_北ビヨンド登録 by001

	if((       ($w:p_story <  d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了) \
			|| (d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story)) \
		&& ($f:w12a_Ａ脚ポンプ室爆弾解体前にセンサーＢゲット == 0)) {
		@SS_南ビヨンド登録 by002
		@SS_北ビヨンド登録 by003
		@SS_東ビヨンド登録 by004
		@SS_西ビヨンド登録 by005
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
				@ＳＥＴライデン状態 (d:_R_DROWN_GOVER)	// 溺死
				@ジャンプ落下死発動 0x0006 -41000
		}
	trap fl002 d:PLAYER \
		-mask 入る \
		-exec {
			if(`@ＧＥＴエレベータ状態 (d:_E_NORMAL)`) {				// エレベータ通常時

				@ＳＥＴライデン状態 (d:_R_FALL_GOVER)	// 落下死
				@ジャンプ落下死発動 0x0016 -15000

			} else if(`@ＧＥＴエレベータ状態 (d:_E_GO_DOWN)`) {		// エレベータ降下中

				@ＳＥＴライデン状態 (d:_R_CRASH_GOVER)	// 激突死
			}
		}
}

//-----------------------------------------------------------------------------
// 落下死処理
proc ジャンプ落下死開始処理 {

	// 落下カメラ

	if(`@ＧＥＴライデン状態 (d:_R_DROWN_GOVER)`) {
		// 溺死
		chara カメラ設定 モーションカメラ \
			-c 1 \	
			-p 3106,14356,-18450 \
			-t 1404,7715,-17175 \
			-a 200 \
			-i 0 0 0 0 \
			-s 1
	} else {
		// 落下死
	}

	@ゲームオーバー開始処理
}
proc ゲームオーバー終了処理 {

	// 落下振動

	if(`@ＧＥＴライデン状態 (d:_R_DROWN_GOVER)`) {
		// 溺死
		chara パッド振動 落下振動 -vibfile drop_sea
	} else {
		// 落下死
	}

	@ゲームオーバー背景設定
}
proc ゲームオーバー背景設定 {

	// command ゲームオーバー背景設定 -tri tri名 -strcode 背景データ -mode モード(0:通常 1:爆発 2:水溺れ 3:その他)

	if(`@ＧＥＴライデン状態 (d:_R_DROWN_GOVER)`) {
		// 溺死
		command ゲームオーバー背景設定 -t gmov -s dekisi_alp_ovl -m 2
	} else {
		// 落下死
		command ゲームオーバー背景設定 -t gmov -s fall_alp_ovl -m 3
	}
}

//-----------------------------------------------------------------------------
// CB_ビヨンド
proc SS_CB_ビヨンド $:トラップ名 $:フラグ $:トリガ $:ＰＸ $:ＰＹ $:ＰＺ {
#if d:DEBUG_PRINT
	print '*** SS CB Beyond ***'
#endif

	//---------------------------------------------------------
	// ビヨンド発動処理
	if($:フラグ == 0) {

		// トラップ名  柵の位置  Ｘ座標／Ｚ座標  トラップ幅１  トラップ幅２  フラグ
		if($:トラップ名 == by001) {
			if(`@北向きビヨンド発動 $1 -16000 $4 -3625 3625 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD)` == by001) {
				@ＳＥＴライデン状態 (d:_R_ELUDE)	// エルード
			}
		} else if($:トラップ名 == by002) {
			if(`@南向きビヨンド発動 $1 -14000 $4 -1750 1750 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD)` == by002) {
				@ＳＥＴライデン状態 (d:_R_ELUDE)	// エルード
			}
		} else if($:トラップ名 == by003) {
			if(`@北向きビヨンド発動 $1 -10500 $4 -1750 1750 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD)` == by003) {
				@ＳＥＴライデン状態 (d:_R_ELUDE)	// エルード
			}
		} else if($:トラップ名 == by004) {
			if(`@東向きビヨンド発動 $1 -2000 $6 -13750 -10750 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD)` == by004) {
				@ＳＥＴライデン状態 (d:_R_ELUDE)	// エルード
			}
		} else if($:トラップ名 == by005) {
			if(`@西向きビヨンド発動 $1 2000 $6 -13750 -10750 (d:BY_NOFOOTSHADOW | d:BY_FALL_DEAD)` == by005) {
				@ＳＥＴライデン状態 (d:_R_ELUDE)	// エルード
			}
		}

	//---------------------------------------------------------
	// ビヨンドカメラ処理
	} else {
		if($:トラップ名 == by001) {
			if($:フラグ == 1) {

				//=====================================
				// ビヨンド１発動カメラ
				//=====================================
				if(`@ＧＥＴライデン状態 (d:_R_SLIP)` == 0) {	// 糞滑り中の誤発動防止
					chara カメラ設定 モーションカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,6000     ,d:CAM_MIN d:CAM_MAX,6000     ,d:CAM_MAX \
						-r 524,2184,0 \
						-f 7000 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}

			} else if($:フラグ == 2) {

				//=====================================
				// ビヨンド１中カメラ
				//=====================================
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \ //-16704
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-16939 \
					-r 1002,2126,0 \
					-f 7000 \
					-a 200 \
					-i 2 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
				if($3 == 出る) {
					@ＳＥＴライデン状態 (d:_R_NORMAL)	// 通常
				}

			}
		} else if($:トラップ名 == by002) {
			if($:フラグ == 1) {

				//=====================================
				// ビヨンド２発動カメラ
				//=====================================
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,6000     ,d:CAM_MIN d:CAM_MAX,6000     ,d:CAM_MAX \
					-r 343,2326,0 \
					-f 5959 \
					-a 200 \
					-i 0 -1 0 0 \
					-s 1

			} else if($:フラグ == 2) {

				//=====================================
				// ビヨンド２中カメラ
				//=====================================
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 708,2326,0 \
					-f 5959 \
					-a 200 \
					-i 2 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
				if($3 == 出る) {
					@ＳＥＴライデン状態 (d:_R_NORMAL)	// 通常
				}

			}
		} else if($:トラップ名 == by003) {
			if($:フラグ == 1) {

				//=====================================
				// ビヨンド３発動カメラ
				//=====================================
				if(`@ＧＥＴエレベータ状態 (d:_E_ON_UP_DEMO)` == 0) {	// ＯＮ上昇エレベータデモ中の誤発動防止
					chara カメラ設定 モーションカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,6000     ,d:CAM_MIN d:CAM_MAX,6000     ,d:CAM_MAX \
						-r 418,2205,0 \
						-f 5489 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}

			} else if($:フラグ == 2) {

				//=====================================
				// ビヨンド３中カメラ
				//=====================================
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-10764 \
					-r 935,2206,0 \
					-f 5489 \
					-a 200 \
					-i 2 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
				if($3 == 出る) {
					@ＳＥＴライデン状態 (d:_R_NORMAL)	// 通常
				}

			}
		} else if($:トラップ名 == by004) {
			if($:フラグ == 1) {

				//=====================================
				// ビヨンド４発動カメラ
				//=====================================
				if(`@ＧＥＴライデン状態 (d:_R_NOT_OPEN)` == 0) {	// 金網ドアが開かない中の誤発動防止
					chara カメラ設定 モーションカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,6000     ,d:CAM_MIN d:CAM_MAX,6000     ,d:CAM_MAX \
						-r 465,2603,0 \
						-f 6013 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}

			} else if($:フラグ == 2) {

				//=====================================
				// ビヨンド４中カメラ
				//=====================================
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 730,2318,0 \
					-f 6013 \
					-a 200 \
					-i 2 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
				if($3 == 出る) {
					@ＳＥＴライデン状態 (d:_R_NORMAL)	// 通常
				}

			}
		} else if($:トラップ名 == by005) {
			if($:フラグ == 1) {

				//=====================================
				// ビヨンド５発動カメラ
				//=====================================
				if(`@ＧＥＴライデン状態 (d:_R_NOT_OPEN)` == 0) {	// 金網ドアが開かない中の誤発動防止
					chara カメラ設定 モーションカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,6000     ,d:CAM_MIN d:CAM_MAX,6000     ,d:CAM_MAX \
						-r 330,1613,0 \
						-f 6013 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}

			} else if($:フラグ == 2) {

				//=====================================
				// ビヨンド５中カメラ
				//=====================================
				chara カメラ設定 モーションカメラ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 675,1718,0 \
					-f 6013 \
					-a 200 \
					-i 2 2 0 0 \
					-z d:CAMERA_NO_CUSHION \
					-s $3
				if($3 == 出る) {
					@ＳＥＴライデン状態 (d:_R_NORMAL)	// 通常
				}

			}
		}
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

	// 左側のドア
	@ドアランプ設定 左側のドアランプ 0 -8750 5000 -9125 1
	@主観禁止壁     左側のドア壁       -8750 5000 -9125 0 d:SD_SLIDE_OUTSIDE
	chara ドア 左側のドア \
		-m d:DOOR_NAME1 \
		-d 0 0 0 \
		-p -8750 5000 -9125 \
		-s d:SD_SLIDE_OUTSIDE \
		-t d:SD_OPEN_TIME \
#ifdef d:STAGE_W12A
		-l 影投影モデル \			// 影投影モデル
#endif
		-A 左側のドアランプ \
		-b 屋上 屋上 \
		-exec {
			if(!($:開閉フラグ)) {
				mesg プットオブジェ Ａ脚内ロード部左 off	// ロード部モデル
			} else {
				mesg プットオブジェ Ａ脚内ロード部左 on		// ロード部モデル
			}
		}
	trap dr001 ？ -mask ？ \
		-exec {
			if($3 == 入る) {
				mesg ドア 左側のドア open
			} else if($3 == 出る) {
				mesg ドア 左側のドア close
			}
		}
	trap dr001 d:PLAYER -mask ？ \
		-state d:TRP_STATE_SUBJECT \
		-exec {
			if($3 == 入る) {
				mesg 透明壁 左側のドア壁 on
			} else if($3 == 出る) {
				mesg 透明壁 左側のドア壁 off
			}
		}

	// 右側のドア
	@ドアランプ設定 右側のドアランプ 2048 8750 5000 -9125 0
	@主観禁止壁     右側のドア壁          8750 5000 -9125 2048 d:SD_SLIDE_OUTSIDE
	chara ドア 右側のドア \
		-m d:DOOR_NAME1 \
		-d 0 2048 0 \
		-p 8750 5000 -9125 \
		-s d:SD_SLIDE_OUTSIDE \
		-t d:SD_OPEN_TIME \
#ifdef d:STAGE_W12A
		-l 影投影モデル \			// 影投影モデル
#endif
		-A 右側のドアランプ \
		-b 屋上 屋上 \
		-exec {
			if(!($:開閉フラグ)) {
				mesg プットオブジェ Ａ脚内ロード部右 off	// ロード部モデル
			} else {
				mesg プットオブジェ Ａ脚内ロード部右 on		// ロード部モデル
			}
		}
	trap dr002 ？ -mask ？ \
		-exec {
			if($3 == 入る) {
				mesg ドア 右側のドア open
			} else if($3 == 出る) {
				mesg ドア 右側のドア close
			}
		}
	trap dr002 d:PLAYER -mask ？ \
		-state d:TRP_STATE_SUBJECT \
		-exec {
			if($3 == 入る) {
				mesg 透明壁 右側のドア壁 on
			} else if($3 == 出る) {
				mesg 透明壁 右側のドア壁 off
			}
		}
}

//-----------------------------------------------------------------------------
// アイテム設定
proc アイテム設定 {
#if d:DEBUG_PRINT
	print '*** Item Set ***'
#endif

	// チャフ
	@アイテム再セットチェック $b:w12a_取得ロード回数[0]
	if($f:再セットフラグ) {
		@チャフ_初期設置 チャフ:01 6000 5000 -13000 0
	}

	// 止血剤
	@アイテム再セットチェック $b:w12a_取得ロード回数[1]
	if($f:再セットフラグ) {
		@止血剤_初期設置 止血剤:01 3000 6000 -6000 0
	}

	// Ｍ９弾薬
	@アイテム再セットチェック $b:w12a_取得ロード回数[2]
	if($f:再セットフラグ) {
		@弾薬_Ｍ９_初期設置 Ｍ９弾薬:01 -8000 5000 -2500 0
	}

	if($w:ゲーム設定 <= d:LEVEL_EASY) {
		// Ｍ９
		if($b:w12a_取得ロード回数[3] == 0) {
			@Ｍ９_初期設置 Ｍ９:01 -4800 5000 -12175 0 Ｍ９弾薬
		}
	}
}

proc アイテム取得時番号設定 $:ボックス名 {
#if d:DEBUG_PRINT
	print '*** Item No Set ***'
#endif

	if($:ボックス名 == チャフ:01) {
		eval($b:w12a_取得ロード回数[0] = $w:グローバルロード回数);
	} else if($:ボックス名 == 止血剤:01) {
		eval($b:w12a_取得ロード回数[1] = $w:グローバルロード回数);
	} else if($:ボックス名 == Ｍ９弾薬:01) {
		eval($b:w12a_取得ロード回数[2] = $w:グローバルロード回数);
	} else if($:ボックス名 == Ｍ９:01) {
		eval($b:w12a_取得ロード回数[3] = $w:グローバルロード回数);
	}

#if d:DEBUG_PRINT
	print $:ボックス名
	print $w:グローバルロード回数
#endif
}

//-----------------------------------------------------------------------------
// 爆弾解体設定
proc 爆弾解体設定 {
#if d:DEBUG_PRINT
	print '*** Bomb Set ***'
#endif

	// 爆弾解体設定
	//   $:爆弾名 $:処理状態フラグ $:Ｘ座標 $:Ｙ座標 $:Ｚ座標 $:Ｘ回転 $:Ｙ回転 $:Ｚ回転
	if($w:ゲーム設定 > d:LEVEL_NORMAL) {
		@SS_爆弾解体設定 w12a匂い爆弾 $f:w12a_爆弾処理完了 7000 6250 2391 0 2048 0

		//---------------------------------------------------------
		// スプレー当たり判定拡大
		trap ev050 d:PLAYER \
			-mask いる \
			-dir    0,512 \
			-exec {
			#if d:DEBUG_PRINT
				print '*** Spray Power Up ***'
			#endif
				command スプレー当たり判定拡大
			}
		trap ev050 d:PLAYER \
			-mask いる \
			-and \
			-dir    1024,512 2048,512 3072,512 \
			-exec {
			#if d:DEBUG_PRINT
				print '*** Spray Power Down ***'
			#endif
				command スプレー当たり判定通常
			}
		trap ev050 d:PLAYER \
			-mask 出る \
			-exec {
			#if d:DEBUG_PRINT
				print '*** Spray Power Down ***'
			#endif
				command スプレー当たり判定通常
			}
	}
}





//-----------------------------------------------------------------------------
// その他オブジェクト設定

proc その他オブジェクト設定 {
#if d:DEBUG_PRINT
	print '*** Object Set ***'
#endif

	//---------------------------------------------------------
	// プットオブジェ制御処理

	// プットオブジェ
	//   -m オブジェクト名  -r 向き  -p 場所  -s スケール  -l 光源  -f フラグ  -t ＬＯＤ閾値
	//
	//   カメラの種類       Ａ脚壁面   エレベーター壁   その他のプットオブジェ
	//     俯瞰カメラ               ×            ×                   ×
	//     柵張付きカメラ・北カメラ ×            ○                   ×
	//     -------------------------------------------------------------------
	//     主観カメラ               ○            ○                   ○
	//     ビハインドカメラ         ×            ×                   ○
	//     エルードカメラ           ○            ○                   ×
	//     -------------------------------------------------------------------
	//     落下死カメラ             ○            ○                   ○
	//     エレベータカメラ         ○            ○                   ○
	//     スタートカメラ           ×            ×                   ×
	@SS_プットオブジェ Ａ脚壁面         d:KMS_NAME1
	@SS_プットオブジェ ＡＢ連絡橋       d:KMS_NAME2
	@SS_プットオブジェ ＢＣ連絡橋１     d:KMS_NAME3
	@SS_プットオブジェ Ｅ脚             d:KMS_NAME4
	@SS_プットオブジェ ＥＦ連絡橋面     d:KMS_NAME5
	@SS_プットオブジェ ＦＡ連絡橋       d:KMS_NAME6
	@SS_プットオブジェ オイルフェンス１ d:KMS_NAME8
	@SS_プットオブジェ オイルフェンス２ d:KMS_NAME9
	@SS_プットオブジェ オイルフェンス３ d:KMS_NAME10
	@SS_プットオブジェ オイルフェンス４ d:KMS_NAME11
	if($w:p_story < d:ST:P012_01_P01フォーチュン遭遇１ポリゴンデモ１終了) {
		@SS_プットオブジェ ＢＣ連絡橋２ d:KMS_NAME12
	} else {
		@SS_プットオブジェ ＢＣ連絡橋２ d:KMS_NAME13
	}

	@SS_プットオブジェ クレーン         d:OBJ_NAME1
	@SS_プットオブジェ エレベータ壁     d:OBJ_NAME2
	@SS_プットオブジェ エレベータ蓋     d:OBJ_NAME5



	chara プロック連続実行 連続実行２ \
		-time -1 \
		-exec {
			if((`@ＧＥＴエレベータ状態 d:_E_ON_UP_DEMO`) || (`@ＧＥＴエレベータ状態 d:_E_ON_DOWN_DEMO`)) {

				//---------------------------------------------------------
				// エレベータデモ中

				mesg 汎用空 空 表示
				mesg 太陽 ソル 表示

				mesg プラント海面 海面 表示状態 1
				mesg プットオブジェ にせ海面    off
				mesg プットオブジェ Ａ脚壁面         on
				mesg プットオブジェ エレベータ壁     on
				mesg プットオブジェ エレベータ蓋     on
				eval($i:Ｓローカル変数Ｉ１ = 20)

				mesg プットオブジェ ＡＢ連絡橋       on
				mesg プットオブジェ ＢＣ連絡橋１     on
				mesg プットオブジェ Ｅ脚             on
				mesg プットオブジェ ＥＦ連絡橋面     on
				mesg プットオブジェ ＦＡ連絡橋       on
				mesg プットオブジェ オイルフェンス１ on
				mesg プットオブジェ オイルフェンス２ on
				mesg プットオブジェ オイルフェンス３ on
				mesg プットオブジェ オイルフェンス４ on
				mesg プットオブジェ ＢＣ連絡橋２     on
				mesg プットオブジェ クレーン         on

			} else if((`@ＧＥＴライデン状態 d:_R_ELUDE`      ) \
				||    (`@ＧＥＴライデン状態 d:_R_DROWN_GOVER`) \
				||    (`@ＧＥＴライデン状態 d:_R_FALL_GOVER` ) \
				||    (`@ＧＥＴライデン状態 d:_R_CRASH_GOVER`)) {

				//---------------------------------------------------------
				// エルード中かゲームオーバー処理中

				mesg 汎用空 空 表示
				mesg 太陽 ソル 表示

				mesg プラント海面 海面 表示状態 1
				mesg プットオブジェ にせ海面    off
				mesg プットオブジェ Ａ脚壁面         on
				mesg プットオブジェ エレベータ壁     on
				mesg プットオブジェ エレベータ蓋     on
				eval($i:Ｓローカル変数Ｉ１ = 20)

				mesg プットオブジェ ＡＢ連絡橋       on
				mesg プットオブジェ ＢＣ連絡橋１     on
				mesg プットオブジェ Ｅ脚             on
				mesg プットオブジェ ＥＦ連絡橋面     on
				mesg プットオブジェ ＦＡ連絡橋       on
				mesg プットオブジェ オイルフェンス１ on
				mesg プットオブジェ オイルフェンス２ on
				mesg プットオブジェ オイルフェンス３ on
				mesg プットオブジェ オイルフェンス４ on
				mesg プットオブジェ ＢＣ連絡橋２     on
				mesg プットオブジェ クレーン         on

			} else {
				command プレイヤー状態取得

				if(($status & d:PFLAG_SUBJECT) || ($status & d:PFLAG_BEHIND)) {

					//---------------------------------------------------------
					// 主観カメラ・ビハインドカメラ時

					mesg 汎用空 空 表示
					mesg 太陽 ソル 表示

					mesg プラント海面 海面 表示状態 1
					mesg プットオブジェ にせ海面    off
					mesg プットオブジェ Ａ脚壁面         on
					mesg プットオブジェ エレベータ壁     on
					mesg プットオブジェ エレベータ蓋     on
					eval($i:Ｓローカル変数Ｉ１ = 20)

					mesg プットオブジェ ＡＢ連絡橋       on
					mesg プットオブジェ ＢＣ連絡橋１     on
					mesg プットオブジェ Ｅ脚             on
					mesg プットオブジェ ＥＦ連絡橋面     on
					mesg プットオブジェ ＦＡ連絡橋       on
					mesg プットオブジェ オイルフェンス１ on
					mesg プットオブジェ オイルフェンス２ on
					mesg プットオブジェ オイルフェンス３ on
					mesg プットオブジェ オイルフェンス４ on
					mesg プットオブジェ ＢＣ連絡橋２     on
					mesg プットオブジェ クレーン         on

				} else {

					//---------------------------------------------------------
					// 俯瞰カメラ時

					mesg 汎用空 空 非表示
					mesg 太陽 ソル 非表示

					if((`@ＧＥＴエリア状態 d:_A_NORTH_EW`) || (`@ＧＥＴエリア状態 d:_A_SAKU_HARI`)) {
						mesg プラント海面 海面 表示状態 1
						mesg プットオブジェ にせ海面    off
						mesg プットオブジェ Ａ脚壁面         on
						mesg プットオブジェ エレベータ壁     on
						mesg プットオブジェ エレベータ蓋     on
						eval($i:Ｓローカル変数Ｉ１ = 20)

					} else {
						if($i:Ｓローカル変数Ｉ１ != 0) {
							eval($i:Ｓローカル変数Ｉ１ = $i:Ｓローカル変数Ｉ１ - 1)
						} else {
							mesg プラント海面 海面 表示状態 0
							mesg プットオブジェ にせ海面    on
							mesg プットオブジェ Ａ脚壁面         off
							mesg プットオブジェ エレベータ壁     off
							mesg プットオブジェ エレベータ蓋     off
						}
					}

					mesg プットオブジェ ＡＢ連絡橋       off
					mesg プットオブジェ ＢＣ連絡橋１     off
					mesg プットオブジェ Ｅ脚             off
					mesg プットオブジェ ＥＦ連絡橋面     off
					mesg プットオブジェ ＦＡ連絡橋       off
					mesg プットオブジェ オイルフェンス１ off
					mesg プットオブジェ オイルフェンス２ off
					mesg プットオブジェ オイルフェンス３ off
					mesg プットオブジェ オイルフェンス４ off
					mesg プットオブジェ ＢＣ連絡橋２     off
					mesg プットオブジェ クレーン         off
				}
			}
		}



	//---------------------------------------------------------
	// ロード部モデル
	@SS_プットオブジェ  Ａ脚内ロード部左 d:KMS_NAME14
	mesg プットオブジェ Ａ脚内ロード部左 off
	@SS_プットオブジェ  Ａ脚内ロード部右 d:KMS_NAME15
	mesg プットオブジェ Ａ脚内ロード部右 off

	//---------------------------------------------------------
	// 空・太陽・海面・影投影モデル
	if($w:p_story < d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了) {
#ifdef d:STAGE_W12A
		// 空・太陽

		//##### w12b のレンズフレアを調整 #####
		@朝空（レンズフレア指定） 0 -40000 0 1930 -1500 4350 17

		// 海面
		chara プラント海面 海面 \
			-tex oil m_oil_sea_alp_ovl_mod0222 m_oil_ref_add_alp_ovl_mod1120 \
			-l \
			-pos 0,-40000,0 \
			-wave 1 2 3

		// 影投影モデル -m 影投影専用モデル -p 設置位置
		if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
			chara 影投影モデル 影投影モデル -m d:KMS_NAME20 -p -3,6,-3
		} else {
			chara 影投影モデル 影投影モデル -m d:KMS_NAME21 -p -3,6,-3
		}

		// 影管理 -t 光源タイプ -l 光源距離(並行光源時のみ有効) -p 並行光源方向直接指定
		chara 影管理 影管理 -t 0 -l 6000 -p 94,-150,265
		trap ev005 ？ \
			-mask ？ \
			-exec {
				if($3 == 入る) {
					mesg 影管理 影管理 add $2 影投影モデル
				} else {
					mesg 影管理 影管理 del $2
				}
			}
#endif
	} else {
#ifdef d:STAGE_W12C
		// 空・太陽

		//##### w12b のレンズフレアを調整 #####
		@昼空（レンズフレア指定） 0 -40000 0 -1600 -3800 2800 17

		// 海面
		chara プラント海面 海面 \
			-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
			-l \
			-pos 0,-40000,0 \
			-wave 1 2 3

		// 陽炎
		chara 陽炎 陽炎 -h 5000
#endif
	}

	// 空・太陽制御
	//   プットオブジェと同時に制御

	// 海面制御
	//   プットオブジェと同時に制御

	//---------------------------------------------------------
	// にせ海面
	chara プットオブジェ にせ海面 -m d:OBJ_NAME7  -r 0,0,0 -p -52000,0,90000 -s 400,400,400 -l d:LT2_NAME
	mesg  プットオブジェ にせ海面 off

	//---------------------------------------------------------
	// 風
	@風基本設定

	//---------------------------------------------------------
	// 網扉
	if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
#ifdef d:STAGE_W12A
		@SS_プットオブジェ 網扉 d:OBJ_NAME3

		command 追加ＨＺＸグループ登録 -hzx_id 0  -add_id 1
		command 追加ＨＺＸグループ登録 -hzx_id 17 -add_id 18
#endif

		chara プットオブジェ ドア網_00 \
			-m d:OBJ_NAME20 \
			-r 0,0,0 \
			-p 3825,0,-12500 \
			-s 100,100,100 \
			-l d:LT2_NAME
		chara プットオブジェ ドア網_01 \
			-m d:OBJ_NAME20 \
			-r 0,0,0 \
			-p -4000,0,-12500 \
			-s 100,100,100 \
			-l d:LT2_NAME
	} else {
#ifdef d:STAGE_W12A
		@SS_プットオブジェ 網扉 d:OBJ_NAME4
#endif

		command 追加ＨＺＸグループ登録 -hzx_id 0  -add_id 2
		command 追加ＨＺＸグループ登録 -hzx_id 17 -add_id 19

		chara プットオブジェ ドア網_02 \
			-m d:OBJ_NAME21 \
			-r 0,0,0 \
			-p 0,0,-12500 \
			-s 100,100,100 \
			-l d:LT2_NAME
		chara プットオブジェ ドア網_03 \
			-m d:OBJ_NAME22 \
			-r 0,0,0 \
			-p 0,0,-12500 \
			-s 100,100,100 \
			-l d:LT2_NAME
	}

	//---------------------------------------------------------
	// 網扉
	chara プットオブジェ フェンス_00 \
		-m d:OBJ_NAME10 \
		-r 0,0,0 \
		-p 0,0,0 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_01 \
		-m d:OBJ_NAME11 \
		-r 0,0,0 \
		-p -8000,0,-12000 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_02 \
		-m d:OBJ_NAME12 \
		-r 0,0,0 \
		-p 8000,0,-12000 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_03 \
		-m d:OBJ_NAME13 \
		-r 0,0,0 \
		-p 3875,0,-14500 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_04 \
		-m d:OBJ_NAME13 \
		-r 0,0,0 \
		-p -4000,0,-14500 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_05 \
		-m d:OBJ_NAME14 \
		-r 0,0,0 \
		-p 5875,0,-10500 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_06 \
		-m d:OBJ_NAME14 \
		-r 0,0,0 \
		-p -5750,0,-10500 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_07 \
		-m d:OBJ_NAME15 \
		-r 0,0,0 \
		-p -4750,0,-11500 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_08 \
		-m d:OBJ_NAME16 \
		-r 0,0,0 \
		-p 4750,0,-11500 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_09 \
		-m d:OBJ_NAME17 \
		-r 0,0,0 \
		-p 5625,0,-14000 \
		-s 100,100,100 \
		-l d:LT2_NAME
	chara プットオブジェ フェンス_10 \
		-m d:OBJ_NAME17 \
		-r 0,0,0 \
		-p -5750,0,-14000 \
		-s 100,100,100 \
		-l d:LT2_NAME

	//---------------------------------------------------------
	// 糞滑り
	trap ev014 d:PLAYER -mask ＊ \
		-exec {
			if($3 == いる) {
				command プレイヤー状態取得
				if(    ($status & d:PFLAG_RUN) \
					&& (`@ＧＥＴライデン状態 d:_R_SLIP` == 0) \
					&& ($w:アイテム != d:アイテム:ダンボール) \
					&& ($w:アイテム != d:アイテム:ダンボールＢ) \
					&& ($w:アイテム != d:アイテム:ダンボールＣ) \
					&& ($w:アイテム != d:アイテム:ダンボールＤ) \
					&& ($w:アイテム != d:アイテム:ダンボールＥ)) {

					@ＳＥＴライデン状態 (d:_R_SLIP)

					mesg プレイヤー d:PLAYER motion d:モーションリスト:non_slip \
						$w:プレイヤー方向 $w:プレイヤー方向 d:FA_END_DOWN_BACK 0
					chara delay ディレイ \
						-time 60 \
						-exec {
							if(`@ＧＥＴライデン状態 d:_R_SLIP` == 1) {
								command セットノイズ -n d:NOISE_S -p $i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ -m 屋上
							//	command プレイヤーライフ値操作 -type 1 -mode 1 -value -3	// 最低１は残る
							}
						}
					chara delay ディレイ \
						-time 150 \
						-exec {
							if(`@ＧＥＴライデン状態 d:_R_SLIP` == 1) {
								@ＳＥＴライデン状態 (d:_R_NORMAL)
							}
						}
				}
			}
		}

	//---------------------------------------------------------
	// 金網開かないモーション
	if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
		trap dr101 d:PLAYER \
			-mask いる \
			-and \
			-button d:ACTION_BUTTON,   d:ACTION_BUTTON \
			-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
			-dir    3072,512           1024,512 \
			-exec {
				if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
					@ＳＥＴライデン状態 (d:_R_NOT_OPEN)
					mesg プレイヤー d:PLAYER motion \
						d:モーションリスト:non_fence_not_open 3072 3072 0 \
						(d:FA_NO_CHECK_TRP | d:FA_NO_WEAPON)  (-3000) (-12800)
					command	強制モーション終了プロック \
						-motion d:モーションリスト:non_fence_not_open \
						-proc 金網開かないモーション終了
				}
			}
		trap dr101 d:PLAYER \
			-mask 入る \
			-state d:TRP_STATE_FORCE \
			-camera \			//カメラトラップであることを明示
			-exec {
				if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
					if(`@ＧＥＴライデン状態 (d:_R_NOT_OPEN)` == 1) {	// 金網ドアが開かない中の誤発動防止
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p -2477,9469,-10190 \
							-t -3225,6616,-12183 \
							-a 200 \
							-i 0 2 0 0 \
							-s 1
					}
				}
			}
		trap dr102 d:PLAYER \
			-mask いる \
			-and \
			-button d:ACTION_BUTTON,   d:ACTION_BUTTON \
			-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
			-dir    1024,512           3072,512 \
			-exec {
				if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
					@ＳＥＴライデン状態 (d:_R_NOT_OPEN)
					mesg プレイヤー d:PLAYER motion \
						d:モーションリスト:non_fence_not_open 1024 1024 0 \
						(d:FA_NO_CHECK_TRP | d:FA_NO_WEAPON)  (3000) (-11950)
					command	強制モーション終了プロック \
						-motion d:モーションリスト:non_fence_not_open \
						-proc 金網開かないモーション終了
				}
			}
		trap dr102 d:PLAYER \
			-mask 入る \
			-state d:TRP_STATE_FORCE \
			-camera \			//カメラトラップであることを明示
			-exec {
				if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
					if(`@ＧＥＴライデン状態 (d:_R_NOT_OPEN)` == 1) {	// 金網ドアが開かない中の誤発動防止
						chara カメラ設定 モーションカメラ \
							-c 1 \
							-p 2477,9469,-10190 \
							-t 3225,6616,-12183 \
							-a 200 \
							-i 0 2 0 0 \
							-s 1
					}
				}
			}
	}

	//---------------------------------------------------------
	// クレイモア設置
	if(    ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) \
		&& ($w:p_story <  d:ST:P036_13_R04忍者登場１３無線デモ４終了)) {
		if($f:w12a_クレイモア１取得フラグ == 0) {
			chara 設置クレイモア クレイモア１ \
				-position 5000,5000,-12375 \
				-rotate   0,1024,0 \
				-proc クレイモア１終了
		}
		if($f:w12a_クレイモア２取得フラグ == 0) {
			chara 設置クレイモア クレイモア２ \
				-position -5000,5000,-12375 \
				-rotate   0,3072,0 \
				-proc クレイモア２終了
		}
		if($f:w12a_クレイモア３取得フラグ == 0) {
			chara 設置クレイモア クレイモア３ \
				-position 7000,5000,-11000 \
				-rotate   0,2048,0 \
				-proc クレイモア３終了
		}
		if($f:w12a_クレイモア４取得フラグ == 0) {
			chara 設置クレイモア クレイモア４ \
				-position -7000,5000,-11000 \
				-rotate   0,2084,0 \
				-proc クレイモア４終了
		}
	}
}

//-----------------------------------------------------------------------------
// 金網開かないモーション終了
proc 金網開かないモーション終了 {
#if d:DEBUG_PRINT
	print '*** Fence Not Open Motion End ***'
#endif

	chara カメラ設定 モーションカメラ -s 0
	@ＳＥＴライデン状態 (d:_R_NORMAL)
}

//-----------------------------------------------------------------------------
// クレイモア処理
proc クレイモア１終了 {
#if d:DEBUG_PRINT
	print '*** Claymore 1 End ***'
#endif
	if($1 != 0) {
		eval($f:w12a_クレイモア１取得フラグ = 1)
	}
}
proc クレイモア２終了 {
#if d:DEBUG_PRINT
	print '*** Claymore 2 End ***'
#endif
	if($1 != 0) {
		eval($f:w12a_クレイモア２取得フラグ = 1)
	}
}
proc クレイモア３終了 {
#if d:DEBUG_PRINT
	print '*** Claymore 3 End ***'
#endif
	if($1 != 0) {
		eval($f:w12a_クレイモア３取得フラグ = 1)
	}
}
proc クレイモア４終了 {
#if d:DEBUG_PRINT
	print '*** Claymore 4 End ***'
#endif
	if($1 != 0) {
		eval($f:w12a_クレイモア４取得フラグ = 1)
	}
}





//=============================================================================
// カメラ、ロード
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
	chara カメラ でもデモカメラ２	-c 0 -l 0 -p  5
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
		-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
		-r 500,2048,0 \
		-f 7000 \
		-a 200 \
		-i 1 -1 0 0 \
		-s 1

	// 南側
	trap ca001 d:PLAYER \
		-camera -mask ＊ \
		-exec {
			if(`@ＧＥＴエレベータ状態 (d:_E_ON_UP_DEMO)`) {		// ＯＮ上昇エレベータデモ中
				chara カメラ設定 でもデモカメラ \
					-c 1 \	
					-p 1036,9146,-4255 \
					-t 613,6310,-8760 \
					-a 200 \
					-i 0 3 0 0 \
					-s 1
			}

			// 通常
			@CS_スムーズカメラ昇順 $4 -8000 8000 -6500 6500
			eval($i:補完ＢＸ = $i:CS_結果１)
			@CS_スムーズカメラ昇順 $6 -12500 -2000 11000 12500
			eval($i:補完ＢＹ = $i:CS_結果１)
			@CS_スムーズカメラ昇順 $6 -12500 -2000 -7000 3500
			eval($i:補完ＢＺ = $i:CS_結果１)

			chara カメラ設定 サブカメラ１ \
				-c 0 \
				-b $i:補完ＢＸ,$i:補完ＢＹ,$i:補完ＢＺ $i:補完ＢＸ,$i:補完ＢＹ,$i:補完ＢＺ \
				-l $i:補完ＢＸ,6000       ,-12500      $i:補完ＢＸ,6000       ,-2000 \
				-r 900,2048,0 \
				-f 7000 \
				-a 200 \
				-i 2 2 0 0 \
				-s $3 \
				-z d:CAMERA_NO_CUSHION

			if($3 == いる) {
				if($i:プレイヤー位置Ｚ > -11500) {
					@ＳＥＴエリア状態 (d:_A_SOUTH)
				} else {
					if(`@ＧＥＴエリア状態 (d:_A_SAKU_HARI)` == 0) {
						@ＳＥＴエリア状態 (d:_A_NORTH)
					}
				}
			}
		}

	// 北側
	trap ca002 d:PLAYER \
		-camera -mask ？ \
		-exec {
			chara カメラ設定 サブカメラ１ \
				-c 0 \
				-b -6250,d:CAM_MIN,d:CAM_MIN 6250,d:CAM_MAX,d:CAM_MAX \
				-l -6250,d:CAM_MIN,d:CAM_MIN 6250,d:CAM_MAX,-11625 \
				-r 880,2048,0 \
				-f 8600 \
				-a 200 \
				-i 2 2 0 0 \
				-s $3

			if($3 == 入る) {
				@ＳＥＴエリア状態 (d:_A_NORTH_EW)
			}
		}

	// エレベータ筒内
	trap ca003 d:PLAYER \
		-camera -mask ？ \
		-exec {
			chara カメラ設定 サブカメラ２ \
				-c 1 \	
				-p 1304,8319,-10633 \
				-t -389,721,-12434 \
				-a 187 \
				-i 0 0 0 0 \
				-s $3
		}

	// 柵張り付きカメラ(北柵・エレベータ柵)
	trap ca004 d:PLAYER \
		-camera -mask ＊ \
		-state d:TRP_STATE_CAUTION \
		-exec {
			if($i:プレイヤー位置Ｚ < -10500) {
				chara カメラ設定 サブカメラ２ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 720,2138,0 \
					-f 7000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			} else {
				chara カメラ設定 サブカメラ２ \
					-c 0 \
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 800,2138,0 \
					-f 7000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3
			}

			if($3 == いる) {
				@ＳＥＴエリア状態 (d:_A_SAKU_HARI)
			} else {
				@ＳＥＴエリア状態 (d:_A_NORTH)
			}
		}



	// エレベータデモカメラ
	trap ca005 d:PLAYER \
		-camera -mask ＊ \
		-exec {
			if(`@ＧＥＴエレベータ状態 d:_E_ON_UP_DEMO`) {			// ＯＮ上昇エレベータデモ用カメラ
				chara カメラ設定 でもデモカメラ２ \
					-c 1 \
					-p 1126,-13000, -8386 \
					-t    0,    $5,-12741 \
					-a 200 \
					-i 0 0 0 0 \
					-s $3
			} else if(`@ＧＥＴエレベータ状態 d:_E_ON_DOWN_DEMO`) {	// ＯＮ降下エレベータデモ用カメラ
				if($i:プレイヤー位置Ｘ > 0) {
					chara カメラ設定 でもデモカメラ２ \
						-c 1 \
						-p -1303,-13000,-16390 \
						-t   403,    $5,-12588 \
						-a 200 \
						-i 0 0 0 0 \
						-s $3
				} else {
					chara カメラ設定 でもデモカメラ２ \
						-c 1 \
						-p 1303,-13000,-16390 \
						-t -403,    $5,-12588 \
						-a 200 \
						-i 0 0 0 0 \
						-s $3
				}
			}
		}
}

proc スタートカメラセット {
#if d:DEBUG_PRINT
	print '*** Start Camera Set ***'
#endif

	if($s:登場ポイント == sp_w12b0_w12a_0) {
		chara カメラ設定 モーションカメラ \
			-c 1 \
			-p -13311,6741,-10268 \
			-t -7919,6582,-10925 \
			-a 200 \
			-i 0 2 0 0 \
			-s 1

	} else if($s:登場ポイント == sp_w12b0_w12a_1){
		chara カメラ設定 モーションカメラ \
			-c 1 \
			-p 13311,6741,-10268 \
			-t 7919,6582,-10925 \
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

	//---------------------------------------------------------
	// 強制無線
	if($w:p_story < d:ST:P006_01_R01Ａ脚金網１無線デモ１終了) {
		trap ev001 d:PLAYER \
			-mask 入る \
			-exec {
				if($w:p_story < d:ST:P006_01_R01Ａ脚金網１無線デモ１終了) {
					@rt_P006_01_R01Ａ脚金網１無線デモ１
				}
			}
	}

	//---------------------------------------------------------
	// 強制無線
	if(    (d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story) \
		&& ($w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了)) {
		trap ev012 d:PLAYER \
			-mask 入る \
			-exec {
				if(    (d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 <= $w:p_story) \
					&& ($w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了)) {
					@rt_P025_01_R01爆弾解体昇降機下１無線デモ１
				}
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

	// ポンプ室左側へ (w12b、w12d)
	trap ld001 d:PLAYER \
		-mask 入る \
		-exec {
			if($f:ロードチェックＯＮフラグ == 1) {
				@爆弾タイマー残り時間保存
				preseek 'w12b'
				@ステージ終了時処理（ステージ名表示） -8000 5000 -7750 0 mv_w12a_w12b0_0 d:DEFAULT_M_TYPE d:wn_Ａ脚ポンプ室
			}
		}

	// ポンプ室右側へ (w12b、w12d)
	trap ld002 d:PLAYER \
		-mask 入る \
		-exec {
			if($f:ロードチェックＯＮフラグ == 1) {
				@爆弾タイマー残り時間保存
				preseek 'w12b'
				@ステージ終了時処理（ステージ名表示） 8000 5000 -7750 0 mv_w12a_w12b0_1 d:DEFAULT_M_TYPE d:wn_Ａ脚ポンプ室
			}
		}

	// 海底ドックへ (w11b)
	trap ld003 d:PLAYER \
		-mask 入る \
		-exec {
			if(    ($f:ロードチェックＯＮフラグ == 1) \
				&& (d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 <= $w:p_story) \
				&& ($w:p_story < d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了)) {

				preseek 'w11b'
				@ステージ終了時処理（ステージ名表示） 0 0 0 0 mv_w12a_w11b_0 d:NO_MOVE_M_TYPE d:wn_Ａ脚底部海底ドック
			}
		}
}

proc ロードプロック {
#if d:DEBUG_PRINT
	print '*** Load Proc ***'
#endif

	if($s:呼び出しプロック == mv_w12a_w12b0_0) {
		// ポンプ室左側へ
		@mv_w12a_w12b0_0

	} else if($s:呼び出しプロック == mv_w12a_w12b0_1) {
		// ポンプ室右側へ
		@mv_w12a_w12b0_1

	} else if($s:呼び出しプロック == mv_w12a_w11b_0) {
		if($w:アラートモード != d:ALERT_MODE_SENNYU) {
			command セットサウンドコード -c d:SNG_FOUTS_S
		}
		// 海底ドックへ
		@mv_w12a_w11b_0
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

	if($s:登場ポイント == sp_w11a2_w12a_0) {
		@ステージ開始時処理 d:LOAD_TYPE:NO_MOVE_TYPE 0 0 d:DEFAULT_M_TYPE

	} else if($s:登場ポイント == sp_w11b_w12a_0) {
		@シナリオデモキャラセット
		@シナリオデモ開始処理
		@ＳＥＴエレベータ状態 (d:_E_ON_UP_DEMO)

		@ステージ開始時処理（昇降機） d:wn_Ａ脚屋上

	} else if($s:登場ポイント == sp_w11c_w12a_0) {
		@シナリオデモキャラセット
		@シナリオデモ開始処理
		@ＳＥＴエレベータ状態 (d:_E_ON_UP_DEMO)

		@ステージ開始時処理（昇降機） d:wn_Ａ脚屋上

	} else if($s:登場ポイント == sp_w12b0_w12a_0) {
		@ステージ開始時処理（ステージ名表示） d:LOAD_TYPE:SL_DOOR_TYPE 左側のドア 2000 d:DEFAULT_M_TYPE d:wn_Ａ脚屋上

	} else if($s:登場ポイント == sp_w12b0_w12a_1) {
		@ステージ開始時処理（ステージ名表示） d:LOAD_TYPE:SL_DOOR_TYPE 右側のドア 2000 d:DEFAULT_M_TYPE d:wn_Ａ脚屋上
	}
}
