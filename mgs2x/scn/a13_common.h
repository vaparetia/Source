/*
	a13_common.h
	    ＡＢ連絡橋（アナザーミッション用）

	2000/11/30 Y.Matsuhana
	$Id: a13_common.h,v 1.31 2002/08/14 07:13:29 usr03379 Exp $

*/

//	ステージ用define
// ------------------------------------
#if d:W13A
	#define		STAGE_W13A		1	//	朝
#else
	#define		STAGE_W13B		1	//	昼
#endif

#define		STAGE_PLANT			1	//	プラント編
#define		STAGE_ALT			1	//	オルタナティブ系のステージでは全て定義する。ゲームオーバーなどで使用
#define		ANOTHER				1	//	VR,オルタナティブ双方で定義。a_stdproc.hで定義されているため、stdch.h内では、使用することができない

// 各種スイッチ用define
// ------------------------------------
// サウンド
#define		SOUND			1		// サウンドデータは最初に呼ぶ
#define		BGM			1		// BGMは敵兵コマンダーの後に呼ぶ
// デモ
#define		DEMO			0
// キャラクター関係
#define		RAIDEN			1		// SNAKEとは排他
#define		SNAKE			0
#define		ENEMY			1		// コマンダーと警備兵(敵を出すときは必須)
#define		SUPPORT			0		// サポート兵
#define		ATTACKER		1		// 攻撃兵
#define		SHIELD			0		// 盾兵(1) or 通常攻撃兵(0)
// 仮置きキャラ
#define		DOLL			0
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
#define		ITEM			0

// 強制モーション
#define		JP_MOTION		0		// ジャンプモーション
#define		BY_MOTION		1		// ビヨンドモーション

//	罠
#define		NARIYUKA		1


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


// 各種設定用define
// ------------------------------------
// マップ関係
//ＡＢ個別
#if d:W13A
	//	ステージ
	#define		KMS_NAME0		w13a
	//	ライト
	#define		LT2_NAME		w13a

#else
	//	ステージ
	#define		KMS_NAME0		w13b
	//	ライト
	#define		LT2_NAME		w13b
#endif

//	ＡＢ共通
#define		KMS_NAME1		w13_12a			//	Ａ脚

#define		KMS_NAME2		w13_24a			//	Ｆ脚
#define		KMS_NAME3		w13_23a			//	ＦＡ連絡橋
#define		KMS_NAME4		w13_21a			//	ＥＦ連絡橋
#define		KMS_NAME5		w13_17a			//	ＣＤ連絡橋
#define		KMS_NAME6		w13_15a			//	ＢＣ連絡橋基部

#define		KMS_NAME7		w13_14a			//	Ｂ脚

#define		KMS_NAME8		w13_15b			//	ＢＣ連絡橋壊れ前
#define		KMS_NAME9		w13_15c			//	ＢＣ連絡橋壊れ後

#define		KMS_NAME10		w13_other00		//	浮き橋１
#define		KMS_NAME11		w13_other01		//	浮き橋２
#define		KMS_NAME12		w13_crane		//	クレーン
#define		KMS_NAME13		w13_20a			//	Ｅ脚

//	プットオブジェおき
#define		KMS_NAME14		w13_14b			//	Ｂ脚ロードエリア
#define		KMS_NAME15		w13_12b			//	Ａ脚ロードエリア


//	アタリ
#define		HZX_NAME		a13a
										//	0		プレイヤーアタリ／跳弾アタリ
										//	1		影用アタリ
										//	2		レンズフレア用アタリ
										//	3〜13	跳弾アタリ(「4」は橋壊れ前)


//	ドア
#define		DOOR_NAME1		w13a_dr00
#define		DOOR_NAME2		w13a_dr01

// キャラクター関係
#if d:RAIDEN
	#define		PLAYER		ライデン	// モーションができるまで名前はスネーク
#else
	#define		PLAYER		スネーク
#endif

//	強制モーション
#define		BY_HIGH			1		// 高所ビヨンドモーション

//	薬莢系
#if d:W13A
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

// ローカル変数
//	$f:w13a_敵配置フラグ
#define STRUT_A		0		//	Ａ脚から来たときの敵配置
#define STRUT_B		1		//	Ｂ脚から来たときの敵配置

//	$f:w13a_エルード落下死フラグ	使用してない

//	$f:w13a_エルード説明デモフラグ		エルード説明デモをみたかどうか
//	$f:w13a_かもめ危険フラグ 			かもめ管理用



//	司令官音声
resource ghq_area_alert_voice {
	w13a:
		$t{
			// PLAREA0
			t:vc040126,	//	指揮官　　「了解。増援部隊、ＡＢ連絡橋に向かえ！敵を逃がすな！」
			t:vc040127,	//	指揮官　　「了解。バックアップをＡＢ連絡橋に送る！侵入者を包囲せよ！」
/*
			t:vc040494,	//	指揮官　　「了解。攻撃チーム、現場に向かえ」
			t:vc040495,	//	指揮官　　「了解。攻撃チーム、出動せよ」
*/
			-2,
			// PLAREA1
			t:vc040122,	//	指揮官　　「了解。応援部隊、ＡＢ連絡橋・Ａ脚入口へ向かえ！敵を逃がすな！」
			t:vc040123,	//	指揮官　　「了解。増援部隊をＡＢ連絡橋に派遣する。Ａ脚入口を固めろ！」
			-2,
			// PLAREA2
			t:vc040124,	//	指揮官　　「了解。応援部隊、ＡＢ連絡橋へ向かえ！Ｂ脚入口を固めろ！」
			t:vc040125,	//	指揮官　　「了解。増援部隊をＡＢ連絡橋・Ｂ脚入口に送る。侵入者を止めろ！」
			-1
		}
}

resource ghq_caution_voice {

	//	攻撃兵が残っていない場合
	w13a:
		$t{
			t:vc040601,	//	指揮官　　「了解、総員、警戒を強化せよ」

			t:vc040491,	//	指揮官　　「攻撃チームは引き返せ、警戒チームはそのまま現場に残れ」
			t:vc040492,	//	指揮官　　「攻撃チームは撤収、警戒チームは警戒を続行せよ」
			t:vc040493,	//	指揮官　　「攻撃チーム帰投せよ、警戒チームはそのまま警戒に当たれ」
			t:vc040496,	//	指揮官　　「攻撃チーム、配置に戻れ」
			t:vc040497,	//	指揮官　　「攻撃チーム、撤収せよ」
			-2,
			t:vc040420,	//	指揮官　　「ＡＢ連絡橋との連絡が途絶えた」
			-1
		}

	//	攻撃兵が残っている場合
	w13a-2:
		$t{
			t:vc040290,	//	指揮官　　「了解。ＡＢ連絡橋に増員を派遣する。警戒を強化しろ」
			t:vc040291,	//	指揮官　　「了解。ＡＢ連絡橋を増員する。警備を強化しろ」

			t:vc040286,	//	指揮官　　「了解。ＡＢ連絡橋を増員。Ａ脚入口を固めろ」
			t:vc040287,	//	指揮官　　「了解。ＡＢ連絡橋・Ａ脚入口に増員を派遣する。警戒を強化しろ！」

			t:vc040288,	//	指揮官　　「了解。ＡＢ連絡橋を増員する。Ｂ脚入口を固めろ」
			t:vc040289,	//	指揮官　　「了解。ＡＢ連絡橋・Ｂ脚入口に増員を派遣する。警戒を強化しろ！」
			-2,
			t:vc040420,	//	指揮官　　「ＡＢ連絡橋との連絡が途絶えた」
			-1
		}
}



//	ドア設定用ステージ内ローカル変数(a_mode.hよりも先に定義する)
dim $$b:ドアフラグ[ 2 ]
dim $$s:ドアモデル名[ 2 ]

command	配列セット $$b:ドアフラグ[0] {
	0,	0
}
command	配列セット $$s:ドアモデル名[0] {
	0,	0
}



// モーション関係
// ------------------------------------
#include "rai_a13a.mh"

// include
// ------------------------------------
#include "stdch.h"
#include "enevoice.h"
#include "a_mode.h"	// アナザー各モード別変数設定用ファイル

#include "sload.h"
#include "beyond.h"
#include "fall.h"
#include "sound.h"
#include "obl_trap.h"
#include "item_info.h"
#include "w13_kamome.h"
#include "vibration.h"
#include "ene_item.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif

// アナザー用でマップ設定前に設定する
// ------------------------------------
@アナザー用マップ定義前設定

// マップ設定
// ------------------------------------
chara マップ ＡＢ連絡橋 -k d:KMS_NAME0 \
			-k d:KMS_NAME2 \
			-k d:KMS_NAME3 \
			-k d:KMS_NAME4 \
			-k d:KMS_NAME5 \
			-k d:KMS_NAME6 \
			-k d:KMS_NAME10 \
			-k d:KMS_NAME11 \
			-k d:KMS_NAME12 \
			-k d:KMS_NAME13 \

			-h d:HZX_NAME,0 -l d:LT2_NAME

//	ＨＺＸ追加登録
//	跳弾アタリ用マップ
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 3
/*
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 4
*/
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
	-hzx_id 0 -add_id 11
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 12
command 追加ＨＺＸグループ登録 \
	-hzx_id 0 -add_id 13


//	グループ２はレンズフレア用ダミー
command 追加ＨＺＸグループ登録 \
	-hzx_id 2 -add_id 0

//	グループ１は影用



command マップ設定 ＡＢ連絡橋 -set {
	@常駐キャラ設定
	@ローカル変数初期化
	@設定値変更
	@プラグイン設定
	@水位設定
	@A_プレイヤー設定 sna_a13a rai_a13a
#if d:ENEMY
	@敵配置チェック
#endif
}

// アナザー用でマップ設定後に設定する
// ------------------------------------
@アナザー用マップ定義後設定

command マップ設定 ＡＢ連絡橋 -set {
	@弾痕跳弾設定
// マップ切り替え、ロード、ドア
// ------------------------------------
#if d:SLIDE_DOOR
	@ドア設置
	@主観禁止壁設定
#endif
#if d:LOAD_TRAP
	@ＡＢ連絡橋ロード設定
#endif
// ステージ、オブジェクト関連
// ------------------------------------
#if d:SEA
	@海面設置
#endif
#if d:SKY
	@空設置
#endif
	@天井設定
	@オブジェクト設定
//	@かもめ設定			//	w13_kamome.hに記述
#if d:FOG
	@フォグ設定
#endif
#if d:EFFECT
//	@エフェクト設定
#endif
	@ＡＢ連絡橋風設定
	@影設定
	@カメラセット
#if d:BY_MOTION
	@ＡＢ連絡橋ビヨンド設定
#endif
	@ＡＢ連絡橋カメラ設定

	@エルード落下死設定

#if d:CHARA_LIGHT
	@キャラ用ライト設定		// マップ設定の最後に呼ぶ
#endif

//	@プラント編全体マップ設定  d:ＡＢ連絡橋マップ	//	全体マップ表示

#if d:DEBUG_CHARA
//	@デバッグキャラセット
#endif

}

// 全てのセットが終わったところでフェードイン
//初期マップの表示
command マップ表示 -show ＡＢ連絡橋

// 全てのセットが終わったところでフェードイン
@ＡＢ連絡橋開始時処理



//###################################################################################
//									各proc設定
//###################################################################################



//###################################################################################
//									全マップ共通
//###################################################################################

	proc ローカル変数初期化 {
		#if d:DEBUG_PRINT 
			print 'localvar_init'
		#endif
		//	かもめ危険フラグを初期化
		eval ($f:w13a_かもめ危険フラグ = d:FALSE)
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

	proc 弾痕跳弾設定 {
		command 弾痕ノーマル -n 0
		command 弾痕ノーマル -n 1
		command 弾痕ノーマル -n 3
		command 弾痕ノーマル -n 4
		command 弾痕ノーマル -n 5

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 3
		command 跳弾ノーマル -n 4
		command 跳弾ノーマル -n 5
	}

//###################################################################################
//									 敵兵
//###################################################################################

	proc 敵配置チェック {
//		自分の来た場所で敵配置変更
		if ($s:登場ポイント == Ａ脚から) {
			eval($f:w13a_敵配置フラグ = d:STRUT_A)
		} else  if ($s:登場ポイント == Ｂ脚から) {
			eval($f:w13a_敵配置フラグ = d:STRUT_B)
		}
	}

//###################################################################################
//									 ドア＆マップ切り替え
//###################################################################################

	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif
/*
		@ドアモデル名セット $:配列番号 \
			$:ドア名１				$:ドア名２				$:ドア名３ \
			$:ドア名４				$:ドア名５
			d:DOOR_FLAG_CLOSE 		d:DOOR_FLAG_OPEN		d:DOOR_FLAG_ENEMY_ONLY
			d:DOOR_FLAG_CLOSE_LEVEL	d:DOOR_FLAG_OPEN_LEVEL

		@A_ドア開閉追加設定 $:ドア名 $:フラグ $:ドア位置Ｘ $:ドア位置Ｙ $:ドア位置Ｚ $:ドア回転 $:ドアスライド幅
*/
		//	Ａ脚ドア
		@ドアモデル名セット 0 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1
		@A_ドア開閉追加設定 Ａ脚ドア $$b:ドアフラグ[0] 19125 0 -5750 1024 d:SD_SLIDE_OUTSIDE

		//	Ｂ脚ドア
		@ドアモデル名セット 1 d:DOOR_NAME2 d:DOOR_NAME2 d:DOOR_NAME2 d:DOOR_NAME2 d:DOOR_NAME2
		@A_ドア開閉追加設定 Ｂ脚ドア $$b:ドアフラグ[1] -17350 0 6050 1327 d:SD_SLIDE_OUTSIDE2


		//	Ａ脚ドア
		chara ドア Ａ脚ドア \
			-m $$s:ドアモデル名[0] \
			-d 0 1024 0 \
			-p 19125 0 -5750 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＡＢ連絡橋 ＡＢ連絡橋 \
			-A Ａ脚ドアランプ \
			-exec{
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ａ脚ロードエリア off
				} else {
					mesg プットオブジェ Ａ脚ロードエリア on
				}
			}

		@ドアランプ設定 Ａ脚ドアランプ 1024 19125 0 -5750 1

		trap dr001 ？ -mask ？ \
			-exec {
				if ( $3 == 入る ) {
					mesg ドア Ａ脚ドア open $2
print '############################TRAP_IN ' $2
				} else {
					mesg ドア Ａ脚ドア close $2
print '############################TRAP_OUT ' $2
				}
			}

		//	Ｂ脚ドア
		chara ドア Ｂ脚ドア \
			-m $$s:ドアモデル名[1] \
			-d 0 1327 0 \
			-p -17350 0 6050 \
			-slide d:SD_SLIDE_OUTSIDE2 \
			-time d:SD_OPEN_TIME \
			-between ＡＢ連絡橋 ＡＢ連絡橋 \
			-A Ｂ脚ドアランプ \
			-r -18250 0 5313 -16625 0 4750 \	//	オルタナ用追加
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｂ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｂ脚ロードエリア on
				}
			}

		@ドアランプ設定 Ｂ脚ドアランプ 1326 -17349 0 6051 0

		trap dr002 ？ -mask ？ \
			-exec {
				if ( $3 == 入る ) {
					mesg ドア Ｂ脚ドア open $2
				} else {
					mesg ドア Ｂ脚ドア close $2
				}
			}
	}

	proc 主観禁止壁設定 {

		@主観禁止壁 Ａ脚ドア壁 19125 0 -5750 1024 d:SD_SLIDE_OUTSIDE
		@主観禁止床 Ａ脚ドア床１ Ａ脚ドア床２ 19125 0 -5750 1024 d:SD_SLIDE_OUTSIDE

		trap dr001 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if( $3 == 入る ) {
					mesg 透明壁 Ａ脚ドア壁 on
				} else {
					mesg 透明壁 Ａ脚ドア壁 off
				}
			}

		@主観禁止壁 Ｂ脚ドア壁 -17350 0 6050 1337 d:SD_SLIDE_OUTSIDE2
		@主観禁止床 Ｂ脚ドア床１ Ｂ脚ドア床２ -17350 0 6050 1337 d:SD_SLIDE_OUTSIDE2
		trap dr002 d:PLAYER \
			-mask ？ \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				if( $3 == 入る ) {
					mesg 透明壁 Ｂ脚ドア壁 on
				} else {
					mesg 透明壁 Ｂ脚ドア壁 off
				}
			}
	}

//###################################################################################
//									 開始時処理
//###################################################################################

	proc ＡＢ連絡橋開始時処理 {
		//	Ａ脚から
		if ($s:登場ポイント == Ａ脚から) {
			// 全てのセットが終わったところでフェードイン
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE Ａ脚ドア 3250 d:DEFAULT_M_TYPE \
			d:wn_ＡＢ連絡橋
			@Ａ脚ロード時カメラ設定

		//	Ｂ脚から
		} else if ($s:登場ポイント == Ｂ脚から) {
			// 全てのセットが終わったところでフェードイン
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE Ｂ脚ドア 3250 d:DEFAULT_M_TYPE \
			d:wn_ＡＢ連絡橋
			@Ｂ脚ロード時カメラ設定

		//	テイルズＥ開始紙芝居後から
		} else if ($s:登場ポイント == テイルズＥ開始紙芝居後から) {
			// 全てのセットが終わったところでフェードイン
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:NO_MOVE_TYPE 紙芝居 0 d:DEFAULT_M_TYPE \
			d:wn_ＡＢ連絡橋
		//	テイルズＥＡＢ連絡橋到着紙芝居後から
		} else if ($s:登場ポイント == テイルズＥＡＢ連絡橋到着紙芝居後から) {
			// 全てのセットが終わったところでフェードイン
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:NO_MOVE_TYPE 紙芝居 0 d:DEFAULT_M_TYPE \
			d:wn_ＡＢ連絡橋
		//	テイルズＥゴルルゴン撮影終了後から
		} else if ($s:登場ポイント == テイルズＥゴルルゴン撮影終了後から) {
			// 全てのセットが終わったところでフェードイン
			@ステージ開始時処理 \
			d:LOAD_TYPE:NO_MOVE_TYPE 紙芝居 0 d:DEFAULT_M_TYPE
		}
		/*	$s:登場ポイント == テイルズＥ敵兵全滅イベント終了後からのときは
			フェードインが特別なので、オルガ登場シナリオデモ内で開始処理を行う	*/
	}

//###################################################################################
//									 ロード
//###################################################################################

	proc ロードプロック {
		#if d:DEBUG_PRINT 
			print 'load_proc'
		#endif

		if ($s:呼び出しプロック == mv_a13a_a12b0_0) {
			//	Ａ脚へ
			@mv_a13a_a12b0_0
		} else if ($s:呼び出しプロック == mv_a13a_a14a_0) {
			//	Ｂ脚へ
			@mv_a13a_a14a_0
		} else if ($s:呼び出しプロック == load_tales_book) {
			// 紙芝居へ
			@load_tales_book
		}
	}

	proc ＡＢ連絡橋ロード設定 {
		#if d:DEBUG_PRINT 
			print 'load_set'
		#endif
		// Ａ脚（a12a）へ
		trap ld001 d:PLAYER \
			-mask いる \
			-exec {
				if ($f:ロードチェックＯＮフラグ == 1 ) {
					//	爆弾イベント用
//					@爆弾タイマー残り時間保存
					@ステージ終了時処理（ステージ名表示）\
					21250 0 -6500 1024 mv_a13a_a12b0_0 d:DEFAULT_M_TYPE \
					d:wn_Ａ脚ポンプ室
					@Ａ脚ロード時カメラ設定
					//	先読み
					preseek 'a12b'
				}
			}
		// Ｂ脚（w14a）へ
		trap ld002 d:PLAYER \
			-mask いる \
			-exec {
				@ポリトラップ $4 $6 -18250,4000 -17000,6500 -20500,6500 -20500,4000 ;
				if(( $i:ポリトラップ結果 >= 0 ) && ( $f:ロードチェックＯＮフラグ == 1 )){
					//	テイルズＤのシールズ全滅前はこちらを通る
					if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
						($b:tales_story_no == d:TALES_NO:D) && \
						($b:tales_story_d == d:TALES_D:シールズ全滅前)) {
						@ステージ終了時処理 \
						-19000 0 6000 3375 load_tales_book d:DEFAULT_M_TYPE
						@Ｂ脚ロード時カメラ設定

					//	それ以外はＢ脚へ
					} else {
					//	テイルズＤはＢ脚の敵はいないのでＢＧＭフェードアウトする
						if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
							($b:tales_story_no == d:TALES_NO:D)) {
							command セットサウンドコード -c d:SNG_FOUTS_S
						}
						//	爆弾イベント用
//						@爆弾タイマー残り時間保存
						@ステージ終了時処理（ステージ名表示）\
						-19000 0 6000 3375 mv_a13a_a14a_0 d:DEFAULT_M_TYPE \
						d:wn_Ｂ脚変電室
						@Ｂ脚ロード時カメラ設定
						//	先読み
						preseek 'a14a'
					}
				}
			}
	}

	proc Ａ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p 17425,4646,-5176 -t 17815,1745,-6796 \
						-a 200 \
						-i 0 1 0 0 \
						-s 1

		chara プロック連続実行 カメラ戻し管理人 \
			-proc カメラ戻し管理
	}
	proc Ｂ脚ロード時カメラ設定 {
		chara カメラ設定 ロードカメラ \
						-c 1 \
						-p -15381,4434,6437 -t -16762,1731,4495 \
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
	#if d:W13A
		//	フォーチュンと遭遇するまではＢＣ連絡橋は壊れない
		//	ＢＣ連絡橋壊れ前
		if ($w:p_story < d:ST:P012_01_P01フォーチュン遭遇１ポリゴンデモ１開始) {
			chara プットオブジェ 中央連絡橋破壊前 -m d:KMS_NAME8 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		
		//	ＢＣ連絡橋壊れ後
		} else {
			chara プットオブジェ 中央連絡橋破壊後 -m d:KMS_NAME9 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		}
	//	昼
	#else
		//	ＢＣ連絡橋壊れ後
			chara プットオブジェ 中央連絡橋破壊後 -m d:KMS_NAME9 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
	#endif

		chara プットオブジェ Ｂ脚ロードエリア \
			-m 	d:KMS_NAME14 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｂ脚ロードエリア off

		chara プットオブジェ Ａ脚ロードエリア \
			-m 	d:KMS_NAME15 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ａ脚ロードエリア off

	}

	proc 水位設定 {
		command 水位設定 -lv -40000
	}
	proc 海面設置 {
		#if d:DEBUG_PRINT
			print 'w13a_sea_set'
		#endif

	#if d:W13A
		//	朝
		chara プラント海面 seaseasea \
			-tex oil m_oil_sea_alp_ovl_mod0222 m_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
			-single_mode \	// シングルテクスチャモード
			-pos 0,-40000,10000

		//	上下運動止める
		mesg プラント海面 seaseasea 上下運動計算処理 0

	#else
		//	昼
		if ( ($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
			 ($b:tales_story_no == d:TALES_NO:E) ) {
			chara プラント海面 seaseasea \
				-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
				-w 2,199,10 \
				-l \
				-pos 0,-40000,10000
		} else {
			chara プラント海面 seaseasea \
				-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
				-w 1,2,3 \
				-l \
				-pos 0,-40000,10000
		}

	#endif

		chara 水面監視水飛沫 落下水飛沫 \
			-n d:PLAYER \
			-n 敵兵
		command 弾水飛沫処理
	}

	proc 空設置 {
		#if d:DEBUG_PRINT
			print 'w13a_sky_set'
		#endif

	//	朝
	#if d:W13A
	print 'MORNING'
		@朝空（レンズフレア指定）	0 -40000 0	3650 -1200 3200	2
	//	昼
	#else
		@昼空（レンズフレア指定）	0 -40000 0	-1900 -3200 3200	2
	#endif

	}

	proc 天井設定 {
		#if d:DEBUG_PRINT
			print 'ceil_set'
		#endif
		chara 天井君 Ａ脚天井 -m d:KMS_NAME1 -p 0,0,0  -l d:LT2_NAME -c 14000,0,0 -a 1
		chara 天井君 Ｂ脚天井 -m d:KMS_NAME7 -p 0,0,0  -l d:LT2_NAME -c -14000,0,0 -a 0
	
	}

//###################################################################################
//									 エフェクト
//###################################################################################

	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

	#if d:W13A
	//	朝
	//	明るさ可変機能
	@明るさ可変機能 194 199 163 -20000 175000 ha002 0

	#else
	//	昼
	@明るさ可変機能 220 237 198 -15000 175000 ha002 0

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

	//	朝
	#if d:W13A

		chara 環境光 環境光源 \
			 -c 25 40 45

		chara 平行光 平行光源 \
			 -c 200 184 118 \
			 -p 3650 -1200 3200

	//	昼
	#else

		chara 環境光 環境光源 \
			-c 55 63 38

		chara 平行光 平行光源 \
			-c 163 164 150 \
			-p -1900 -3200 3200

	#endif

	}

	proc ＡＢ連絡橋風設定 {
		@風設定 ca002
	}

	proc 影設定 {
		#if d:DEBUG_PRINT
		print'SHADOW'
		#endif

	#if d:W13A	//	朝のみ

		//	影写り用モデル
		chara 影投影モデル 影用モデル \
			-model w13a_sdw \
			-pos -1,1,-1

		//	影写り用ハザード　敵兵が影を発見
		command	影投影ハザード \
			-name ＡＢ連絡橋 \
			-group 1
		
		chara 影管理 影管理人 \
			-type 0 \
			-length 6000 \
			-p 2200,-1750,2650
		
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
			-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-136000 \
			-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,-136500 \
			-r 505,2351,0 \
			-f 5500 \
			-a 200 \
			-i 1 -1 0 0 \
			-s 1
	}
	proc ＡＢ連絡橋カメラ設定 {
		#if d:DEBUG_PRINT
			print 'w13a_camera_set'
		#endif

		// Ａ脚側出入口付近
		trap ca004 d:PLAYER \
			-mask ？ \
			-camera \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 1 \	
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN 17753,d:CAM_MAX,-5669 \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN 17753, d:CAM_MAX,-6526 \
					-p 17384,6141,-5481 -t 17384,1695,-6942 \
					-r 904,2048,0 -f 4684 \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためリミットを見る
					-s $3

			}
		// Ａ脚通路
		trap ca001 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {

//-p 15791,2470,3473 -t 14584,1477,-2455 \
//-p 15644,4199,2742 -t 14787,1441,-1461 \

					@CS_スムーズカメラ昇順 $6 -4000 1000 15644 15369
					eval( $i:補完ＰＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -4000 1000 4199 5554
					eval( $i:補完ＰＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -4000 1000 2742 1384
					eval( $i:補完ＰＺ = $i:CS_結果１ );


					@CS_スムーズカメラ昇順 $6  -4000 1000 14787 15166
					eval( $i:補完ＴＸ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -4000 1000 1441 -484
					eval( $i:補完ＴＹ = $i:CS_結果１ );

					@CS_スムーズカメラ昇順 $6  -4000 1000 -1461 385
					eval( $i:補完ＴＺ = $i:CS_結果１ );

		
				chara カメラ設定 小部屋カメラ \
					-c 1 \
					-b 14375,d:CAM_MIN,d:CAM_MIN 16125,d:CAM_MAX,1875 \
					-l 14375,d:CAM_MIN,d:CAM_MIN 16125,d:CAM_MAX,1875 \
					-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
					-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためリミットを見る
					-s $3
			}


		// 連絡橋
		trap ca002 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {
				//	プレイヤーのＸ座標に沿ってＸロットが変化
				@CS_スムーズカメラ昇順 $4       0 12000 450 650

				//	最終的に求めたＸロットを代入
				eval( $i:補完ＲＸ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -2700 -7823 -13000
				eval( $i:補完ＢＸ１ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -2700 -7823 -13000
				eval( $i:補完ＬＸ１ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -2700 13500 14000
				eval( $i:補完ＢＸ２ = $i:CS_結果１ );

				@CS_スムーズカメラ降順 $5  -600 -2700 8861 9405
				eval( $i:補完ＬＸ２ = $i:CS_結果１ );

				chara カメラ設定 通路カメラ \
					-c 0 \	
					-b $i:補完ＢＸ１,d:CAM_MIN,d:CAM_MIN $i:補完ＢＸ２,10500,d:CAM_MAX \
					-l $i:補完ＬＸ１,d:CAM_MIN,d:CAM_MIN $i:補完ＬＸ２,d:CAM_MAX,d:CAM_MAX \

					-r $i:補完ＲＸ,2496,0 -f 11000 \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-s $3
			}

		// Ｂ脚通路
		trap ca003 d:PLAYER \
			-mask ＊ \
			-camera \
			-exec {

//-p -15551,4295,5801 -t -16537,1565,1044 \
//-p -15123,2928,6528 -t -16739,1582,-1282 \
//-p -16143,6185,4797 -t -16256,1541,4257 \

				@CS_スムーズカメラ昇順 $6 1000 4000 -15551 -16143
				eval( $i:補完ＰＸ = $i:CS_結果１ );

				@CS_スムーズカメラ昇順 $6 1000 4000 4295 6185
				eval( $i:補完ＰＹ = $i:CS_結果１ );

				@CS_スムーズカメラ昇順 $6 1000 4000 5801 4797
				eval( $i:補完ＰＺ = $i:CS_結果１ );


				@CS_スムーズカメラ昇順 $6 1000 4000 -16537 -16256
				eval( $i:補完ＴＸ = $i:CS_結果１ );

				@CS_スムーズカメラ昇順 $6 1000 4000 1565 1541
				eval( $i:補完ＴＹ = $i:CS_結果１ );

				@CS_スムーズカメラ昇順 $6 1000 4000 1044 4257
				eval( $i:補完ＴＺ = $i:CS_結果１ );

				chara カメラ設定 小部屋カメラ \
					-c 1 \
					-b -16500,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l -16500,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-p $i:補完ＰＸ,$i:補完ＰＹ,$i:補完ＰＺ \
					-t $i:補完ＴＸ,$i:補完ＴＹ,$i:補完ＴＺ \
					-a 200 \
					-i 0 -1 0 0 \
					-d 1 \
					-z d:CAMERA_LIMITBOUND_EXIST \	//	ビハインドのためリミットを見る
					-s $3
			}

	//	張り付き移動
		//	南側通路
		trap bh001 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 0 256 \	// 方向指定
			-camera \
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 240,2048,0 -f 7000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}
	
		//	北側通路
		trap bh002 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				chara カメラ設定 張り付きカメラ \
					-c 0 \	
					-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
					-r 240,0,0 -f 7000 \
					-a 200 \
					-i 2 2 0 0 \
					-s $3 \
					-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
			}

		//	Ａ脚入口付近北側
		trap bh101 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 0 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
	
		//	Ａ脚入口付近南側
		trap bh102 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	Ａ脚側南北通路
		trap bh103 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 3072 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	Ｂ脚側南北通路
		trap bh104 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 1024 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}

		//	Ｂ脚入口付近北側
		trap bh105 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 0 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
	
		//	Ｂ脚入口付近南側
		trap bh106 d:PLAYER \
			-mask ？ \
			-state 8 \	//張り付き
			-dir 2048 256 \	// 方向指定
			-camera \
			-exec {
				@張り付き移動カメラ１Ｆ $3
			}
	}

	proc 張り付き移動カメラ１Ｆ {

//-p 10190,12618,986 -t 9997,1622,751 \
//-p -10017,12618,-516 -t -10209,1622,-751 \

		chara カメラ設定 張り付きカメラ \
			-c 0 \
			-b -10017,d:CAM_MIN,-516 10190,d:CAM_MAX,986 \
			-l -10209,d:CAM_MIN,-751 9997,d:CAM_MAX,751 \
			-r 1006,2496,0 -f 11000 \
			-a 200 \
			-i 2 2 0 0 \
			-s $1 \
			-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
	
	}






//###################################################################################
//									 ビヨンド
//###################################################################################

	proc ＡＢ連絡橋ビヨンド設定 {
		#if d:DEBUG_PRINT
			print 'w13a_beyond_set'
		#endif

		// 南側ルート
			//	エルード発動
			trap by001 d:PLAYER \
				-mask いる \
				-button 3 \		//ビヨンドボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@南向きビヨンドハイ発動 by001 5000 $4 -7375 7375 0x00002
				}
			//	発動カメラ
			trap by001 d:PLAYER \
				-camera \
				-mask いる \
				-state 6 \	// 強制モーション中
				-dir 0 256 \	// 方向指定
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,5053,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
						-r 385,2450,0 -f 6500 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			//	エルードカメラ
			trap by001 d:PLAYER \
				-camera \
				-mask ？ \
				-state 5 \	// エルード中
				-dir 2048 256 \	// 方向指定
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 385,2450,0 -f 6500 \
						-a 272 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION
				}
			//	張り付きカメラ
			trap by001 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 2048 256 \	// 方向指定
				-camera \
				-exec {
					chara カメラ設定 張り付きカメラ \
						-c 0 \	
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 1006,2496,0 -f 11000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
				}

		// 北側ルート
			//	エルード発動
			trap by002 d:PLAYER \
				-mask いる \
				-button 3 \		//ビヨンドボタン
				-state 0,4 \	//立ち状態or張り付き
				-exec {
					@北向きビヨンドハイ発動 by002 -5000 $4 -7375 7375 0x00002
				}
			//	発動カメラ
			trap by002 d:PLAYER \
				-camera \
				-mask いる \
				-state 6 \	// 強制モーション中
				-dir 2048 256 \	// 方向指定
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,5598,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,1319,d:CAM_MAX \
						-r 496,2450,0 -f 6181 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1
				}
			//	エルードカメラ
			trap by002 d:PLAYER \
				-camera \
				-mask ？ \
				-state 5 \	// エルード中
				-dir 0 256 \	// 方向指定
				-exec {
					chara カメラ設定 ビヨンドカメラ \
						-c 0 \
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 997,2450,0 -f 6181 \
						-a 274 \
						-a 200 \
						-i 3 2 0 0 \
						-s $3 \
						-z d:CAMERA_NO_CUSHION
				}
			//	張り付きカメラ
			trap by002 d:PLAYER \
				-mask ？ \
				-state 8 \	//張り付き
				-dir 0 256 \	// 方向指定
				-camera \
				-exec {
					chara カメラ設定 張り付きカメラ \
						-c 0 \	
						-b d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-l d:CAM_MIN,d:CAM_MIN,d:CAM_MIN d:CAM_MAX,d:CAM_MAX,d:CAM_MAX \
						-r 1006,2496,0 -f 11000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
				}
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
						-r 3852,3560,0 -f 4340 \
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
	#if d:FALLDOWN_RESET
		trap ev001 d:PLAYER \
			-mask ？\
			-exec { restart } ;
	#endif
}
