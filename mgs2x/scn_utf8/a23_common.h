/*
	a23_common.h
	    ＦＡ連絡橋（アナザーミッション用）

	2002/02/04 S.Mukaide
	$Id: a23_common.h,v 1.12 2002/07/23 07:16:25 usr03379 Exp $

	
*/

//	ステージ用define
// ------------------------------------
#if d:W23A
	#define		STAGE_W23A			1	//	朝
#else
	#define		STAGE_W23B			1	//	昼
#endif

#define			STAGE_PLANT			1	//	プラント編
#define			STAGE_ALT			1	//	オルタナティブ系のステージでは全て定義する。ゲームオーバーなどで使用
#define			ANOTHER				1	//	VR,オルタナティブ双方で定義。a_stdproc.hで定義されているため、stdch.h内では、使用することができない

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
#define		HZX_NAME			a23a
										//	0		プレイヤーアタリ／跳弾アタリ
										//	1～8	遠景跳弾アタリ
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

// ローカル変数
//	$b:w23a_敵配置フラグ	プレイヤーがどちらから来たかか敵配置を変える
#define		STRUT_A			0		//	Ａ脚から来たときの敵配置
#define		STRUT_F_1F		1		//	Ｆ脚１階から来たときの敵配置
#define		STRUT_F_B1		2		//	Ｆ脚Ｂ１から来たときの敵配置

//	$f:w23a_サイファー遭遇デモフラグ	//	サイファー遭遇デモを見たか
//	$f:w23a_サイファー01破壊フラグ 		//	サイファー:01を破壊したかどうか
//	$f:w23a_サイファー02破壊フラグ 		//	サイファー:02を破壊したかどうか



//	司令官音声
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



//	ドア設定用ステージ内ローカル変数(a_mode.hよりも先に定義する)
dim $$b:ドアフラグ[ 3 ]
dim $$s:ドアモデル名[ 3 ]

command	配列セット $$b:ドアフラグ[0] {
	0,	0,	0
}
command	配列セット $$s:ドアモデル名[0] {
	0,	0,	0
}



// モーション関係
// ------------------------------------
#include "rai_a23a.mh"

// include
// ------------------------------------
#include "stdch.h"
#include "enevoice.h"
#include "a_mode.h"	// アナザー各モード別変数設定用ファイル

#include "sload.h"
#include "beyond.h"
#include "fall.h"
#include "sound.h"
#include "item_info.h"
#include "obl_trap.h"
#include "w23_kamome.h"
#include "ene_item.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif

// アナザー用でマップ設定前に設定する
// ------------------------------------
@アナザー用マップ定義前設定

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



command マップ設定 ＦＡ連絡橋 -set {
	@常駐キャラ設定
	@ローカル変数初期化
	@設定値変更
	@プラグイン設定
	@水位設定
	@A_プレイヤー設定 sna_a23a rai_a23a
#if d:ENEMY
	@敵配置チェック
#endif
}

// アナザー用でマップ設定後に設定する
// ------------------------------------
@アナザー用マップ定義後設定

command マップ設定 ＦＡ連絡橋 -set {
	@弾痕跳弾設定
// マップ切り替え、ロード、ドア
// ------------------------------------
#if d:SLIDE_DOOR
	@ドア設置
	@主観禁止壁設定
#endif
#if d:LOAD_TRAP
	@ＦＡ連絡橋ロード設定
#endif
// ステージ、オブジェクト関連
// ------------------------------------
	@オブジェクト設定
#if d:SEA
	@海面設定
#endif
#if d:SKY
	@空設定
#endif
	@天井設定

	@かもめ設定		//	w23_kamome.hに記述

#if d:FOG
	@フォグ設定
#endif
#if d:EFFECT
//	@エフェクト設定
#endif
	@ＦＡ連絡橋風設定
	@影設定
	@カメラセット
#if d:BY_MOTION
	@ＦＡ連絡橋ビヨンド設定
#endif
	@ＦＡ連絡橋カメラ設定

	@エルード落下死設定

#if d:CHARA_LIGHT
	@キャラ用ライト設定		// マップ設定の最後に呼ぶ
#endif

//	@プラント編全体マップ設定  d:ＦＡ連絡橋マップ	//	全体マップ表示

#if d:DEBUG_CHARA
//	@デバッグキャラセット
#endif

}

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

	proc 弾痕跳弾設定 {
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

//###################################################################################
//									 敵兵
//###################################################################################

	proc 敵配置チェック {
		//	Ａ脚から
		if ($s:登場ポイント == Ａ脚から) {
			eval($b:w23a_敵配置フラグ = d:STRUT_A)
		//	Ｆ脚１Ｆから
		} else if ($s:登場ポイント == Ｆ脚１Ｆから) {
			eval($b:w23a_敵配置フラグ = d:STRUT_F_1F)
		//	Ｆ脚Ｂ１から
		} else if ($s:登場ポイント == Ｆ脚Ｂ１から) {
			eval($b:w23a_敵配置フラグ = d:STRUT_F_B1)
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
		@ドアモデル名セット 0 d:DOOR_NAME_LOAD1 d:DOOR_NAME_LOAD1 d:DOOR_NAME_LOAD1 d:DOOR_NAME_LOAD1 d:DOOR_NAME_LOAD1
		@A_ドア開閉追加設定 Ａ脚ドア $$b:ドアフラグ[0] -19125 0 -5750 1024 d:SD_SLIDE_OUTSIDE

		//	Ｆ脚１Ｆドア
		@ドアモデル名セット 1 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2
		@A_ドア開閉追加設定 Ｆ脚１Ｆドア $$b:ドアフラグ[1] 17350 0 8300 722 d:SD_SLIDE_OUTSIDE2

		//	Ｆ脚Ｂ１ドア
		@ドアモデル名セット 2 d:DOOR_NAME_LOAD3 d:DOOR_NAME_LOAD3 d:DOOR_NAME_LOAD3 d:DOOR_NAME_LOAD3 d:DOOR_NAME_LOAD3
		@A_ドア開閉追加設定 Ｆ脚Ｂ１ドア $$b:ドアフラグ[2] 21150 -4500 750 1745 d:SD_SLIDE_OUTSIDE2


		//	Ａ脚ドア
		chara ドア Ａ脚ドア \
			-m $$s:ドアモデル名[0] \
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

		//	Ｆ脚１Ｆドア
		chara ドア Ｆ脚１Ｆドア \
			-m $$s:ドアモデル名[1] \
			-d 0,721,0 \
			-p 17350,0,8300 \
			-slide d:SD_SLIDE_OUTSIDE2 \
			-time d:SD_OPEN_TIME \
			-between ＦＡ連絡橋 ＦＡ連絡橋 \
			-A Ｆ脚１Ｆドアランプ \
			-r 17125 0 7563 18188 0 7563 \	//	オルタナ用追加
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

		//	Ｆ脚Ｂ１ドア
		chara ドア Ｆ脚Ｂ１ドア \
			-m $$s:ドアモデル名[2] \
			-d 0,1745,0 \
			-p 21150,-4500,750 \
			-slide d:SD_SLIDE_OUTSIDE2 \
			-time d:SD_OPEN_TIME \
			-between ＦＡ連絡橋 ＦＡ連絡橋 \
			-A Ｆ脚Ｂ１ドアランプ \
			-r 20313 -4500 750 20375 -4500 -313 \	//	オルタナ用追加
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
		// Ａ脚１Ｆから
		if ($s:登場ポイント == Ａ脚から) {
			@ステージ開始時処理（ステージ名表示）\
			d:LOAD_TYPE:SL_DOOR_TYPE Ａ脚ドア 3500 d:DEFAULT_M_TYPE \
			d:wn_ＦＡ連絡橋
			@Ａ脚ロード時カメラ設定
		// Ｆ脚１Ｆから
		} else if ($s:登場ポイント == Ｆ脚１Ｆから) {
				@ステージ開始時処理（ステージ名表示）\
				d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚１Ｆドア 2750 d:DEFAULT_M_TYPE \
				d:wn_ＦＡ連絡橋
				@Ｆ脚１Ｆロード時カメラ設定
		// Ｆ脚Ｂ１から
		} else if ($s:登場ポイント == Ｆ脚Ｂ１から) {
			@ステージ開始時処理（ステージ名表示）\
			d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚Ｂ１ドア 3250 d:DEFAULT_M_TYPE \
			d:wn_ＦＡ連絡橋
			@Ｆ脚Ｂ１ロード時カメラ設定
		}

	}

//###################################################################################
//									 ロード
//###################################################################################

	proc ロードプロック {
		#if d:DEBUG_PRINT 
			print 'load_proc'
		#endif

		if ($s:呼び出しプロック == mv_a23a_a12b0_0) {
			// Ａ脚１Ｆへ
			@mv_a23a_a12b0_0
		} else if ($s:呼び出しプロック == mv_a23a_a22a0h_0) {
			// Ｆ脚１Ｆへ
			@mv_a23a_a22a0h_0
		} else if ($s:呼び出しプロック == mv_a23a_a22a1c_0) {
			// Ｆ脚Ｂ１へ
			@mv_a23a_a22a1c_0
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
					-21000 0 -6500 3072 mv_a23a_a12b0_0 d:DEFAULT_M_TYPE \
					d:wn_Ａ脚ポンプ室
					@Ａ脚ロード時カメラ設定
					//	先読み
					preseek 'a12b'
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
					19750 0 8000 721 mv_a23a_a22a0h_0 d:DEFAULT_M_TYPE \
					d:wn_Ｆ脚倉庫１Ｆ
					@Ｆ脚１Ｆロード時カメラ設定
					//	先読み
					preseek 'a22a'
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
					21500 -4500 -2250 1745 mv_a23a_a22a1c_0 d:DEFAULT_M_TYPE \
					d:wn_Ｆ脚倉庫Ｂ１
					@Ｆ脚Ｂ１ロード時カメラ設定
					//	先読み
					preseek 'a22a'
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
			chara プットオブジェ 中央連絡橋破壊前 -m d:KMS_NAME10 \
				-r 0 0 0 -p 0 0 0  -l d:LT2_NAME

		//	ＢＣ連絡橋壊れ後
		} else {
			chara プットオブジェ 中央連絡橋破壊後 -m d:KMS_NAME11 \
				-r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		}
	//	昼
	#else
		//	ＢＣ連絡橋壊れ後
			chara プットオブジェ 中央連絡橋破壊後 -m d:KMS_NAME11 \
				-r 0 0 0 -p 0 0 0  -l d:LT2_NAME
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
			-pos 0,-40000,15000

	#else
		//	昼
		chara プラント海面 seaseasea \
			-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
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

		@朝空（レンズフレア指定）	0 -40000 0 1000 -1500 4600 10


	#else

		@昼空（レンズフレア指定）	0 -40000 0 -2400 -4200 1160 10

	#endif

	}

	proc 天井設定 {
		#if d:DEBUG_PRINT
			print 'ceil_set'
		#endif
		chara 天井君 Ａ脚天井 -m d:KMS_NAME1 -p 0,0,0  -l d:LT2_NAME -c -14000,0,0 -a 0
		chara 天井君 Ｆ脚天井 -m d:KMS_NAME8 -p 0,0,0  -l d:LT2_NAME -c 14000,0,0 -a 1
		chara 天井君 Ｆ脚廊下 -m d:KMS_NAME9 -p 0,0,0  -l d:LT2_NAME -c 14000,0,0 0,287,0 -a 1 2 -e 2 1
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

					//	高さ－３１００のときのＸロットを求める。
					@CS_スムーズカメラ昇順 $4  -12000 12000 800 400
					//	求めたＸロットを別の変数に代入。このＸロットからスムーズにつながる
					eval ($i:現状ＲＸ２ = $i:CS_結果１);

					//	求めた変数からプレイヤーのＹ座標に沿ってＸロットが変化
					@CS_スムーズカメラ降順 $5  -600 -3100 $i:現状ＲＸ１ $i:現状ＲＸ２

				} else {
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
					-s $3

			}

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
						-r 881,1583,0 -f 10000 \
						-a 200 \
						-i 2 2 0 0 \
						-s $3 \
						-z (d:CAMERA_NO_CUSHION | d:CAMERA_NO_PAD_ADJUST)
				}

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
					@張り付き移動カメラＢ１ $3
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
					@張り付き移動カメラＢ１ $3
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
					@張り付き移動カメラＢ１ $3
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
