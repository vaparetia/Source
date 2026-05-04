/*
	a15_common.h
	    ＢＣ連絡橋（アナザーミッション用）

	2002/01/30 S.Mukaide
	$Id: a15_common.h,v 1.26 2002/08/29 07:15:42 usr13731 Exp $


*/

//	ステージ用define
// ------------------------------------
#if d:W15A
	#define		STAGE_W15A		1	//	昼
#else
	#define		STAGE_W15B		1	//	夜
#endif

#define		STAGE_PLANT			1	//	プラント編
#define		STAGE_ALT			1	//	オルタナティブ系のステージでは全て定義する。ゲームオーバーなどで使用
#define		ANOTHER				1	//	VR,オルタナティブ双方で定義。a_stdproc.hで定義されているため、stdch.h内では、使用することができない

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
#define		ITEM			0
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
#define		HZX_NAME		a15a
										//	0		プレイヤーアタリ／跳弾アタリ
										//	1～10	遠景跳弾アタリ
										//	11		影用アタリ
										//	12		レンズフレア用アタリ

//	ドア
#define		DOOR_NAME1		w15a_dr00

#define		DOOR_NAME_ALT	w15a_dr00x	//	オルタナ/テイルズＢ用カード１ドア（Ｃ脚側）



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

// ローカル変数
//	$f:w15a_敵配置フラグ
#define STRUT_B		0					//	Ｂ脚から来たときの敵配置
#define STRUT_C		1					//	Ｃ脚から来たときの敵配置

//	$f:w15a_サイファー破壊フラグ		//	サイファーを破壊したかどうか



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



//	ドア設定用ステージ内ローカル変数(a_mode.hよりも先に定義する)
dim $$b:ドアフラグ[ 2 ]
dim $$s:ドアモデル名[ 2 ]

command	配列セット $$b:ドアフラグ[0] {
	0,	0
}
command	配列セット $$s:ドアモデル名[0] {
	0,	0
}



// ステージ固有モーションリスト
// ------------------------------------
#include "rai_a15a.mh"

// include
// ------------------------------------
#include "stdch.h"
#include "enevoice.h"
#include "a_mode.h"		// アナザーミッション用共通プロック。stdch.hの下でinclude

#include "sload.h"
#include "beyond.h"
#include "beyond2.h"
#include "fall.h"
#include "sound.h"
#include "w15_kamome.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif
#include "item_info.h"
#include "vibration.h"

#include "w15_d_common.h"
#include "ene_item.h"

// アナザー用でマップ設定前に設定する
// ------------------------------------
@アナザー用マップ定義前設定

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



command マップ設定 ＢＣ連絡橋 -set {
	@常駐キャラ設定
	@ローカル変数初期化
	@設定値変更
	@プラグイン設定
	@水位設定
	@A_プレイヤー設定 sna_a15a rai_a15a
#if d:ENEMY
	@敵配置チェック
#endif
}

// アナザー用でマップ設定後に設定する
// ------------------------------------
@アナザー用マップ定義後設定

command マップ設定 ＢＣ連絡橋 -set {
	@弾痕跳弾設定
// マップ切り替え、ロード、ドア
// ------------------------------------
#if d:SLIDE_DOOR
	@A_ドア設置
	@主観禁止壁設定
#endif
#if d:LOAD_TRAP
	@ＢＣ連絡橋ロード設定
#endif

// ステージ、オブジェクト関連
// ------------------------------------
	@オブジェクト設定
#if d:SEA
	@海面設置		//	w15_d_common.hに記述
	@海面管理設定
#endif
#if d:SKY
	@空設置			//	w15_d_common.hに記述
#endif
#if d:CHARA_CEIL
	@天井設定
#endif
	@かもめ設定		//	w15_kamome.hに記述
#if d:FOG
	@フォグ設定		//	w15_d_common.hに記述
#endif
#if d:EFFECT
//	@エフェクト設定
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

# if d:CHARA_LIGHT
	@キャラ用ライト設定		// マップ設定の最後に呼ぶ
# endif

//	@プラント編全体マップ設定  d:ＢＣ連絡橋マップ	//	全体マップ表示

}

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
			command プラグイン落下
	}

	proc 弾痕跳弾設定 {
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

//###################################################################################
//								敵兵
//###################################################################################

	proc 敵配置チェック {
//		自分の来た場所で敵配置変更
		if ($s:登場ポイント == Ｂ脚から) {
			eval ($f:w15a_敵配置フラグ = d:STRUT_B)
		} else if ($s:登場ポイント == Ｃ脚から) {
			eval ($f:w15a_敵配置フラグ = d:STRUT_C)
		}
	}

//###################################################################################
//									 ドア＆マップ切り替え
//###################################################################################

	//	インテグラル用ドア配置（本編はデモ共用ファイルに記述のためこちらに切り分けた）
	proc A_ドア設置 {
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
		//	Ｂ脚ドア
		@ドアモデル名セット 0 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1
		@A_ドア開閉追加設定 Ｂ脚ドア $$b:ドアフラグ[0] -55500 0 -43625 0 d:SD_SLIDE_OUTSIDE

		//	Ｃ脚ドア
		@ドアモデル名セット 1 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME1 d:DOOR_NAME_ALT d:DOOR_NAME_ALT
		@A_ドア開閉追加設定 Ｃ脚ドア $$b:ドアフラグ[1] -56750 0 -76375 0 d:SD_SLIDE_OUTSIDE


		//	Ｂ脚ドア
		chara ドア Ｂ脚ドア \
			-m $$s:ドアモデル名[0] \
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

		//	Ｃ脚ドア
		chara ドア Ｃ脚ドア \
			-m $$s:ドアモデル名[1] \
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

	proc ＢＣ連絡橋開始時処理{
		#if d:DEBUG_PRINT
		print 'STAGE_START'
		#endif

		//	Ｂ脚から
		if ($s:登場ポイント == Ｂ脚から) {
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE Ｂ脚ドア 2500 d:DEFAULT_M_TYPE \
			d:wn_ＢＣ連絡橋
//			@Ｂ脚ロード時カメラ設定	//	真上真下のときはわかりにくいのでなし

		//	Ｃ脚から
		} else if ($s:登場ポイント == Ｃ脚から) {
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE Ｃ脚ドア 2500 d:DEFAULT_M_TYPE \
			d:wn_ＢＣ連絡橋
//			@Ｃ脚ロード時カメラ設定	//	真上真下のときはわかりにくいのでなし
		}

	}

//###################################################################################
//									ロード設定
//###################################################################################

	proc ロードプロック {
		if ($s:呼び出しプロック == mv_a15a_a14a_0) {
			@mv_a15a_a14a_0
		} else if ($s:呼び出しプロック == mv_a15a_a16a_0) {
			@mv_a15a_a16a_0
		}
	}

	proc ＢＣ連絡橋ロード設定 {
		trap ld001 d:PLAYER \
			-mask 入る \
			-exec {
				// Ｂ脚（w14a）へ
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	テイルズＢのＢ脚ノードアクセス前はＢ脚の敵はいないのでＢＧＭフェードアウトする
					if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
						($b:tales_story_no == d:TALES_NO:B) && \
						($b:tales_story_b == d:TALES_B:Ｂ脚ノードアクセス前)) {
						command セットサウンドコード -c d:SNG_FOUTS_S
					}
					//	テイルズＤはＢ脚の敵はいないのでＢＧＭフェードアウトする
					if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
						($b:tales_story_no == d:TALES_NO:D)) {
						command セットサウンドコード -c d:SNG_FOUTS_S
					}

					@A_爆弾タイマー停止

					//	先読み
					preseek 'a14a'

					@ステージ終了時処理（ステージ名表示） \
					-54750 0 -42000 0 mv_a15a_a14a_0 d:DEFAULT_M_TYPE \
					d:wn_Ｂ脚変電室
					@Ｂ脚ロード時カメラ設定
				}
			}
		trap ld002 d:PLAYER \
			-mask 入る \
			-exec {
				// Ｃ脚（w16a）へ
				if( $f:ロードチェックＯＮフラグ == 1 ) {
#if 0
					//	テイルズＢのＢ脚ノードアクセス前はＣ脚の敵はいないのでＢＧＭフェードアウトする
					if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
						($b:tales_story_no == d:TALES_NO:B) && \
						($b:tales_story_b == d:TALES_B:Ｂ脚ノードアクセス前)) {
						command セットサウンドコード -c d:SNG_FOUTS_S
					}
#endif
					@A_爆弾タイマー停止

					//	先読み
					preseek 'a16a'

					@ステージ終了時処理（ステージ名表示） \
					-56000 0 -78000 2048 mv_a15a_a16a_0 d:DEFAULT_M_TYPE \
					d:wn_Ｃ脚食堂
					@Ｃ脚ロード時カメラ設定
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

//###################################################################################
//									オブジェクト
//###################################################################################

	proc オブジェクト設定 {
		#if d:DEBUG_PRINT
			print 'w15a_bridge_set'
		#endif

		chara プットオブジェ Ｂ脚ロードエリア \
			-m 	d:KMS_NAME15 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｂ脚ロードエリア off

		chara プットオブジェ Ｃ脚ロードエリア \
			-m 	d:KMS_NAME16 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ Ｃ脚ロードエリア off
	}

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
//									 エフェクト
//###################################################################################

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
			-c 33 53 50

		chara 平行光 Para \
			-c 201 169 103 \
			-p 2200,-1400,4300

	//	昼
	#else
		chara 環境光 Amb \
			-c 55 63 38

		chara 平行光 Para \
			-c 163 164 150 \
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
								command ＳＥセットモード -s d:se_code:SD_E_INWTERL4 \
								-p	$i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ \
								-m 1
							}

					// ALT-HOLDUP用 気絶/眠り兵捨てたらゲームオーバー処理。
					// 02.08.29 T.Ohtani
					if( $b:ミッション番号 == d:MISSION:武装解除 ){
						chara delay 死体捨てゲームオーバーディレイ \
							-t 260 \
							-e {
								command セット敵兵殺したフラグ
							}
					}

				}
			}

	}






