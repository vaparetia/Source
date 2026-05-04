/*
	a21_common.h
		ＥＦ連絡橋（アナザーミッション用）

	2002/02/04 S.Mukaide
	$Id: a21_common.h,v 1.30 2002/10/02 07:25:44 usr03379 Exp $

*/

//	ステージ用define
// ------------------------------------
#if d:W21A
	#define		STAGE_W21A			1	//	昼
#else
	#define		STAGE_W21B			1	//	夕方
#endif

#define			STAGE_PLANT			1	//	プラント編
#define			STAGE_ALT			1	//	オルタナティブ系のステージでは全て定義する。ゲームオーバーなどで使用
#define			ANOTHER				1	//	VR,オルタナティブ双方で定義。a_stdproc.hで定義されているため、stdch.h内では、使用することができない

// エマ関係(テイルズＢで使用するため)
#define		WITH_EMMA		1		// エマと一緒のロード処理をするのに必要
#define		EMMA_STAGE		a21a	// エマと一緒のロード処理をするのに必要
#define		EMMA_GEOVER_CAMERA	1	// エマのゲームオーバー時のカメラ

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
#define		HZX_NAME			a21a
												//	0		プレイヤーアタリ／跳弾アタリ
												//	1		レンズフレア用アタリ
												//	2～18	遠景用跳弾アタリ

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

#define		DOOR_NAME_ALT		w21a_dr00x		//	オルタナ/テイルズＢ用カード無しドア（Ｆ脚側）
#define		DOOR_NAME_ALT_1		w21a_dr01x		//	オルタナ/テイルズＢ用カード無しドア（中央棟側）
#define		DOOR_NAME_ALT_2		w21a_dr01x2		//	オルタナ/テイルズＢ用カード１ドア（Ｅ脚側）


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
#define START_CAMERA		1		//	開始処理用カメラ



//	司令官音声
resource ghq_area_alert_voice {
	w21a:
		$t{

#if 0
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
#endif


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



//	ドア設定用ステージ内ローカル変数(a_mode.hよりも先に定義する)
dim $$b:ドアフラグ[ 3 ]
dim $$s:ドアモデル名[ 3 ]

command	配列セット $$b:ドアフラグ[0] {
		0,	0,	0
}
command	配列セット $$s:ドアモデル名[0] {
		0,	0,	0
}



// include
// ------------------------------------
#include "rai_a21a.mh"		// 強制モーションリスト

#include "stdch.h"
#include "enevoice.h"
#include "a_mode.h"	// アナザー各モード別変数設定用ファイル

#include "sload.h"
#include "beyond.h"
#include "fall.h"
#include "sound.h"
#include "item_info.h"
#include "w21_kamome.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif

// アナザー用でマップ設定前に設定する
// ------------------------------------
@アナザー用マップ定義前設定

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
	@常駐キャラ設定
	@ローカル変数初期化
	@設定値変更
	@プラグイン設定
	@水位設定

	@A_プレイヤー設定 sna_a21a rai_a21a
}

// アナザー用でマップ設定後に設定する
// ------------------------------------
@アナザー用マップ定義後設定

command マップ設定 ＥＦ連絡橋 -set {
	@弾痕跳弾設定
// マップ切り替え、ロード、ドア
// ------------------------------------
#if d:SLIDE_DOOR
	@ドア設置
	@主観禁止壁設定
#endif
#if d:LOAD_TRAP
	@ロード設定
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
#if d:CHARA_CEIL
	@天井設定
#endif

#if d:PIT
	if ( ($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
		($b:tales_story_no == d:TALES_NO:B) ) {
		//	テイルズＢではエマの関係上落し穴は落ちない
	} else {
		@落し穴配置チェック
		@落し穴設定
	}
#endif
//	@かもめ設定		//	w21_kamome.hに記述	オルタナは無し
#if d:FOG
	@フォグ設定
#endif
#if d:EFFECT
//	@エフェクト設定
#endif
	@ＥＦ連絡橋風設定
#if d:BY_MOTION
	@ＥＦ連絡橋ビヨンド設定
#endif
	@カメラセット
	@ＥＦ連絡橋カメラ設定

	@エルード落下死設定

#if d:CHARA_LIGHT
	@キャラ用ライト設定
# endif

//	@プラント編全体マップ設定 d:ＥＦ連絡橋マップ	//	全体マップ表示

#if d:DEBUG_CHARA
//	@デバッグキャラセット
#endif

}

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

	proc ローカル変数初期化 {
		#if d:DEBUG_PRINT 
			print 'localvar_init'
		#endif

		eval($f:w21a_双眼鏡兵覗きフラグ = d:FALSE)
		eval($i:w21a_敵兵モード = 0)
		eval($f:w21a_双眼鏡兵所定位置フラグ = d:FALSE)

//	インテグラル
#if 0
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
#endif
	}

	proc 設定値変更 {
		#if d:DEBUG_PRINT 
			print 'status_set'
		#endif

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

	proc 弾痕跳弾設定 {
		command 弾痕ノーマル -n 0
		command 弾痕ノーマル -n 1
		command 弾痕ノーマル -n 3
		command 弾痕ノーマル -n 4

		command 跳弾ノーマル -n 0
		command 跳弾ノーマル -n 1
		command 跳弾ノーマル -n 3
		command 跳弾ノーマル -n 4
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
		//	Ｅ脚ドア
		@ドアモデル名セット 0 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2 d:DOOR_NAME_ALT_2 d:DOOR_NAME_ALT_2
		@A_ドア開閉追加設定 Ｅ脚ドア $$b:ドアフラグ[0] 61000 0 -76375 2048 d:SD_SLIDE_OUTSIDE

		//	Ｆ脚ドア
		@ドアモデル名セット 1 d:DOOR_NAME_ALT d:DOOR_NAME_ALT d:DOOR_NAME_ALT d:DOOR_NAME_LOAD1 d:DOOR_NAME_LOAD1
		@A_ドア開閉追加設定 Ｆ脚ドア $$b:ドアフラグ[1] 57000 0 -43625 0 d:SD_SLIDE_OUTSIDE

		//	中央棟ドア
		@ドアモデル名セット 2 d:DOOR_NAME_ALT_1 d:DOOR_NAME_ALT_1 d:DOOR_NAME_ALT_1 d:DOOR_NAME_LOAD2 d:DOOR_NAME_LOAD2
		@A_ドア開閉追加設定 中央棟ドア $$b:ドアフラグ[2] 19125 -1500 -54250 1024 d:SD_SLIDE_OUTSIDE


		//	Ｅ脚ドア
		chara ドア Ｅ脚ドア \
			-m $$s:ドアモデル名[0] \
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
					if ($3 == 入る) {
						mesg ドア Ｅ脚ドア open $2
					} else {
						if ($$f:エマゲームオーバーフラグ == 0) {
							mesg ドア Ｅ脚ドア close $2
						}
					}
			}

		//	Ｆ脚ドア
		chara ドア Ｆ脚ドア \
			-m $$s:ドアモデル名[1] \
			-d 0,0,0 \
			-p 57000,0,-43625 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＥＦ連絡橋 ＥＦ連絡橋 \
			-A Ｆ脚ドアランプ \
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｆ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｆ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｆ脚ドアランプ 0 57000 0 -43625 1

		trap dr003 ？ \
			-mask ？ \	
			-exec {
					if ($3==入る) {
						mesg ドア Ｆ脚ドア open $2
					} else {
						if ($$f:エマゲームオーバーフラグ == 0) {
							mesg ドア Ｆ脚ドア close $2
						}
					}
			}

		//	中央棟ドア
		chara ドア 中央棟ドア \
			-m $$s:ドアモデル名[2] \
			-d 0,1024,0 \
			-p 19125,-1500,-54250 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＥＦ連絡橋 ＥＦ連絡橋 \
			-A 中央棟ドアランプ \
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
						if ($$f:エマゲームオーバーフラグ == 0) {
							mesg ドア 中央棟ドア close $2
						}
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
		// Ｅ脚から
		if ($s:登場ポイント == Ｅ脚から) {
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE Ｅ脚ドア 2250 d:DEFAULT_M_TYPE \
			d:wn_ＥＦ連絡橋

		// Ｆ脚から
		} else if ($s:登場ポイント == Ｆ脚から) {
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE Ｆ脚ドア 2250 d:DEFAULT_M_TYPE \
			d:wn_ＥＦ連絡橋

		// 中央棟から
		} else if ($s:登場ポイント == シェル１中央棟から) {
			@ステージ開始時処理（ステージ名表示） \
			d:LOAD_TYPE:SL_DOOR_TYPE 中央棟ドア 2500 d:DEFAULT_M_TYPE \
			d:wn_ＥＦ連絡橋
		}
	}

	proc スタートカメラセット {
		//	Ｅ脚
		if ($s:登場ポイント == Ｅ脚から) {
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
		} else if ($s:登場ポイント == Ｆ脚から) {
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
		} else if ($s:登場ポイント == シェル１中央棟から) {
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

		if ($s:呼び出しプロック == mv_a21a_a20a0_0) {
			//	テイルズＢのＥ脚戻る前、エマを連れているときにここを通った場合は絶対にエマはＥＦ連絡橋に置きっぱなし！
			if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
				($b:tales_story_no == d:TALES_NO:B) && \
				($b:tales_story_b == d:TALES_B:Ｅ脚戻る前)) {
				//	ＥＦ連絡橋にエマがいるときは
				if ($f:エマ存在フラグ == 1) {
					$s:エマ存在ステージ = a21a;
				}
			}
			// Ｅ脚１Ｆへ
			@mv_a21a_a20a0_0
		} else if ($s:呼び出しプロック == mv_a21a_a22a0g_0) {
			// Ｆ脚１Ｆへ
			@mv_a21a_a22a0g_0
		} else if ($s:呼び出しプロック == mv_a21a_a24a_0) {
			// 中央棟へ
			@mv_a21a_a24a_0

		} else if ($s:呼び出しプロック == load_tales_book) {
			//	テイルズＢのＥ脚戻る前、エマを連れているときにここを通った場合はＥ脚にいるのが確定！
			if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
				($b:tales_story_no == d:TALES_NO:B) && \
				($b:tales_story_b == d:TALES_B:Ｅ脚戻る前)) {
				//	ＥＦ連絡橋にエマがいるときは
				if ($f:エマ存在フラグ == 1) {
					$s:エマ存在ステージ = a20e;
				}
			}
			// 紙芝居へ
			@load_tales_book
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
					//	双眼鏡兵状態やりとりのため
					@A_双眼鏡兵状態保存 敵兵:01

					//	テイルズＢのＥ脚戻る前はこちらを通る
					if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
						($b:tales_story_no == d:TALES_NO:B) && \
						($b:tales_story_b == d:TALES_B:Ｅ脚戻る前)) {

						//	エマと手をつないでいるときは
						command ゲットエマ手繋ぎ状況 $$i:エマ手繋ぎ状況
						if ($$i:エマ手繋ぎ状況 > 0) {
							$b:tales_story_b = d:TALES_B:Ｅ脚戻り中;
							@ステージ終了時処理 \
							60250 0 -78000 2048 load_tales_book d:DEFAULT_M_TYPE
							//	ロード時カメラ
							@Ｅ脚ロード時カメラ設定

						//	手をつないでいない場合
						} else {

							//	エマがa20eにいるとき（向こう側に吹っ飛ばしたりした場合）は
							if ($s:エマ存在ステージ == a20e) {
								$b:tales_story_b = d:TALES_B:Ｅ脚戻り中;
								@ステージ終了時処理 \
								60250 0 -78000 2048 load_tales_book d:DEFAULT_M_TYPE
								//	ロード時カメラ
								@Ｅ脚ロード時カメラ設定

							//	エマを置きっぱなしにした場合は
							} else {
								// Ｅ脚１Ｆへ
								@ステージ終了時処理（ステージ名表示） \
								60250 0 -78000 2048 mv_a21a_a20a0_0 d:DEFAULT_M_TYPE \
								d:wn_Ｅ脚集配場１Ｆ
								//	ロード時カメラ
								@Ｅ脚ロード時カメラ設定
								//	先読み
								//	テイルズＢはこちらを通る
								if ( ($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
									 ($b:tales_story_no == d:TALES_NO:B) ) {
									preseek 'a20e'
								} else {
									preseek 'a20a'
								}
							}
						}

					//	それ以外はＥ脚へ
					} else {
						//	爆弾イベント用
//						@爆弾タイマー残り時間保存

						// Ｅ脚１Ｆへ
						@ステージ終了時処理（ステージ名表示） \
						60250 0 -78000 2048 mv_a21a_a20a0_0 d:DEFAULT_M_TYPE \
						d:wn_Ｅ脚集配場１Ｆ
						//	ロード時カメラ
						@Ｅ脚ロード時カメラ設定
						//	先読み
						//	テイルズＢはこちらを通る
						if ( ($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
							 ($b:tales_story_no == d:TALES_NO:B) ) {
							preseek 'a20e'
						} else {
							preseek 'a20a'
						}
					}
				}
			}

		trap ld003 d:PLAYER \
			-mask 入る \
			-exec {
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	双眼鏡兵状態やりとりのため
					@A_双眼鏡兵状態保存 敵兵:01

					//	テイルズＢのＦ脚入る前はこちらを通る
					if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
						($b:tales_story_no == d:TALES_NO:B) && \
						($b:tales_story_b == d:TALES_B:Ｆ脚入る前)) {
						$b:tales_story_b = d:TALES_B:Ｆ脚入り中;
						@ステージ終了時処理 \
						57750 0 -42000 0 load_tales_book d:DEFAULT_M_TYPE
						//	ロード時カメラ
						@Ｆ脚ロード時カメラ設定

					//	それ以外はＦ脚へ
					} else {
						//	爆弾イベント用
//						@爆弾タイマー残り時間保存

						//	エマが捕まってから再会するまではＦ脚は潜入モードからなのでＢＧＭフェードアウトする
						if (($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
							($b:tales_story_no == d:TALES_NO:B) && \
 							($b:tales_story_b > d:TALES_B:Ｂ脚ノードアクセス前) && \
							($b:tales_story_b < d:TALES_B:エマ再会中 )) {
							command セットサウンドコード -c d:SNG_FOUTS_S
						}
						
						// Ｆ脚１Ｆへ
						@ステージ終了時処理（ステージ名表示） \
						57750 0 -42000 0 mv_a21a_a22a0g_0 d:DEFAULT_M_TYPE \
						d:wn_Ｆ脚倉庫１Ｆ
						//	ロード時カメラ
						@Ｆ脚ロード時カメラ設定
						//	先読み
						//	テイルズＢはこちらを通る
						if ( ($b:ミッション番号 == d:MISSION:スネークテイルズ) && \
							 ($b:tales_story_no == d:TALES_NO:B) ) {
							preseek 'a22b'
						} else {
							preseek 'a22a'
						}
					}
				}
			}

		trap ld004 d:PLAYER \
			-mask 入る \
			-exec {
				if( $f:ロードチェックＯＮフラグ == 1 ) {
					//	双眼鏡兵状態やりとりのため
					@A_双眼鏡兵状態保存 敵兵:01

					//	爆弾イベント用
//					@爆弾タイマー残り時間保存

					// 中央棟へ
					@ステージ終了時処理（ステージ名表示） \
					17750 -1500 -55000 3072 mv_a21a_a24a_0 d:DEFAULT_M_TYPE \
					d:wn_シェル１中央棟１Ｆ
					//	ロード時カメラ
					@中央棟ロード時カメラ設定
					//	先読み
					preseek 'a24a'
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
			-pos 62000,-40000,-60000
	#else
		//	夕
		chara プラント海面 seaseasea \
			-tex oil e_oil_sea_alp_ovl_mod0222 e_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
			-pos 52000,-40000,-60000

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

		@昼空（レンズフレア指定）	52000 -30000 -60000 -2000 -4000 2200 1

	#else

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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
				if ($3 == 出る) {
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
		trap ca002 d:PLAYER \
			-camera \
			-mask ？ \
			-exec {
				chara カメラ設定 小部屋カメラ \
					-c 0 \
					-b  51202,d:CAM_MIN,-44568  57842, 5969,-44568 \
			 		-l  51202,d:CAM_MIN,-45351  57842, 1347,-44768 \
					-r 913,2048,0 \
					-f 4689 \
					-a 200 \
					-i 0 -1 0 0 \
					-s $3
			}

		// 中央棟側通路
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
					@CS_スムーズカメラ降順 $6 -46000 -60000 650 350
					eval ($i:補完ＲＸ = $i:CS_結果１);
				} else {
					@CS_スムーズカメラ降順 $6 -46000 -60000 650 350
					eval ($i:補完ＲＸ１ = $i:CS_結果１)
					@CS_スムーズカメラ降順 $4 47000 43000 $i:補完ＲＸ１ 550
					eval( $i:補完ＲＸ = $i:CS_結果１ );
				}

				chara カメラ設定 通路カメラ \
					-c 0 \
					-b 26581,d:CAM_MIN,-56388 d:CAM_MAX,11919,-46804 \
					-l 26581,d:CAM_MIN,-67129 d:CAM_MAX,d:CAM_MAX,-49640 \
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
