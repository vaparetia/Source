/*
	vardef.h
	    標準で使用する変数等の設定

	2000/08/29 H.Yoshiike
	$Id: vardef.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $
*/

// ファイルが二重呼びされたときの対処
#ifndef d:VARDEF_H
#define VARDEF_H	1

// -------------------------------------------------------------------------------------------
// ゲームとデモで共用するdefine用ヘッダーファイル
// -------------------------------------------------------------------------------------------
#include "equip.h"			// 武器・アイテム関係のdefine

//#define シナリオデモカメラアングル作成	0
#include "sd_vardef.h"		// シナリオデモカメラアングル作成用
#include "story.h"			// ストーリーフラグのdefine




// -------------------------------------------------------------------------------------------
// ゲームとデモで共用するdefine
// -------------------------------------------------------------------------------------------

// フェードイン・アウトの時間
// ------------------------------------
#define	FADEIN_TIME		30
#define	FADEOUT_TIME	60

// シネマスクリーンの黒帯サイズ
//----------------------------------------------------------
#define CINEMA_SCREEN_ITSELF 1

#if d:CINEMA_SCREEN_ITSELF
	#ifdef MGS2_TRIAL

	// 2 対 1。512 x 256  体験版はこのサイズ
	#define CINEMA_BELT_UPPER	24
	#define CINEMA_BELT_LOWER	56

	#else

	// 1.88 対 1。アメリカの35フィルム相当（1.85 対 1）。512 x 272
	#define CINEMA_BELT_UPPER	16
	#define CINEMA_BELT_LOWER	48
	#define	MOVIE_TOP_Y			32

	#endif
#else
	// シネマスクリーン無し。
	#define CINEMA_BELT_UPPER	0
	#define CINEMA_BELT_LOWER	0
#endif

// タンカー編・連続ポリデモ、ムービー、無条件スキップ
//----------------------------------------------------------
#define TANKER_DEMO_ALL_SKIP	0

// プラント編・連続ポリデモ、ムービー、無条件スキップ
//----------------------------------------------------------
#define PLANT_DEMO_ALL_SKIP		0

// シネマスクリーンのフェードタイム
// ------------------------------------
#define	C_FADE_TIME		30

// ２Ｄテクスチャ表示用のdefine
// ------------------------------------
#define	FLAG_RIGHT 		0x0001
#define	FLAG_BOTTOM 	0x0002
#define	FLAG_CENTER 	0x0010


// 真偽
// ------------------------------------
#ifndef d:TRUE
#define	TRUE			1
#endif
#ifndef d:FALSE
#define	FALSE			0
#endif

// キー
// ------------------------------------
// trap -key & command パッド操作用(こちらを推奨)
#define	PAD_PUNCH		0x0020	// PAD_A(○)と同じ
#define	PAD_CRAWL		0x0040	// PAD_B(×)と同じ
#define	PAD_ACTION		0x0010	// PAD_X(△)と同じ
#define	PAD_WEAPON		0x0080	// PAD_Y(□)と同じ

// trap -button用
#define	PUNCH_BUTTON	0
#define	WEAPON_BUTTON	1
#define	CRAWL_BUTTON	2
#define	ACTION_BUTTON	3

// パッドデモ時の表示のオンオフ
// ------------------------------------
#define		ALL_OFF		0x0000
#define		MENU_ON		0x0001
#define		GAGE_ON		0x0002
#define		RADAR_ON	0x0004
#define		SUBWIN_ON	0x0008
#define		RADIO_ON	0x0010
#define		PAUSE_ON	0x0020

// パッドデモ再生状況
// ------------------------------------
#define		PADDEMO_PLAY	1
#define		PADDEMO_END		0


// カメラ最大最小
// ------------------------------------
#define CAM_MAX			 1000000
#define CAM_MIN			 -1000000


// 子画面サイズ
// ------------------------------------
#define		SUB_WIDTH		160
#define		SUB_HEIGHT		64
#define		SUB_MARGIN		16


// スライドドアの開閉時間
// ------------------------------------
#define		SD_OPEN_TIME	10	// 開閉し終わるまでの時間
#define		SD_CLOSE_TIME	10	// 閉まり始まるまでの時間(閉まるまでのタイムラグプログラムができたら導入)

// スライドドアのスライド量
// ------------------------------------
#define		SD_SLIDE_OUTSIDE  1450		// 脚と連絡橋の間のドアのスライド量
#define		SD_SLIDE_OUTSIDE2 1630		// 脚と連絡橋の間のドアのスライド量(斜めドア)
#define		SD_SLIDE_INSIDE   1510		// 室内のドアのスライド量
#define		SD_SLIDE_ARSENAL_LEFT -475	// アーセナル内左ドアのスライド量
#define		SD_SLIDE_ARSENAL_RIGHT 1500	// アーセナル内右ドアのスライド量


// スライドドアのドアランプシフト量
// ------------------------------------
#define		SD_LAMP_SHIFT_1  1750	// ＸかＺ方向のシフト量１
#define		SD_LAMP_SHIFT_2   130	// ＸかＺ方向のシフト量２
#define		SD_LAMP_SHIFT_3S 1750	// 1957*sin63.4
#define		SD_LAMP_SHIFT_3C  876	// 1957*cos63.4
#define		SD_LAMP_SHIFT_4S  100	// 112*sin63.4
#define		SD_LAMP_SHIFT_4C   50	// 112*cos63.4
#define		SD_LAMP_SHIFT_Y  2325	// Ｙ方向のシフト量

// 向手ステージスペシャル
#define		SD_LAMP_SHIFT_5S  876	// 1957*sin26.6
#define		SD_LAMP_SHIFT_5C 1750	// 1957*cos26.6
#define		SD_LAMP_SHIFT_6S   50	// 112*sin26.6
#define		SD_LAMP_SHIFT_6C  100	// 112*cos26.6

// 方向
// ------------------------------------
#define		NORTH		2048
#define		SOUTH		0
#define		EAST		1024
#define		WEST		3072
#define		NORTH_EAST	1536
#define		NORTH_WEST	2560
#define		SOUTH_EAST	512
#define		SOUTH_WEST	3584

// 壁床効果音設定関係
// ------------------------------------
// 変換テーブルＩＤ -id
#define		SE_PLAYER		0	// プレイヤーが対象
#define		SE_ENEMY01		1	// 警備兵＆攻撃兵01(最初に登録された人)
#define		SE_ENEMY02		2	// 警備兵＆攻撃兵02(２番目に登録された人)
#define		SE_ENEMY03		3	// 警備兵＆攻撃兵03(３番目に登録された人)
#define		SE_ENEMY04		4	// 警備兵＆攻撃兵04(４番目登録された人)
// 音を鳴らす対象タイプ -type
#define		SE_FLOOR		0	// 床が対象
#define		SE_WALL			1	// 壁が対象



// ステージ名テクスチャ
// ------------------------------------
#define		wn_船尾甲板							senbi_kanpan_alp_ovl
#define		wn_船尾甲板右舷						senbi_kanpan_right_alp_ovl
#define		wn_船尾甲板左舷						senbi_kanpan_left_alp_ovl

#define		wn_船橋一階居住区					1f_live_alp_ovl
#define		wn_船橋一階居住区右舷				1f_live_right_alp_ovl
#define		wn_船橋一階居住区左舷				1f_live_left_alp_ovl
#define		wn_船橋一階リフレッシュルーム右舷	refre_right_alp_ovl
#define		wn_船橋一階リフレッシュルーム		refre_alp_ovl
#define		wn_船橋二階居住区					2f_live_alp_ovl
#define		wn_船橋二階居住区右舷				2f_live_right_alp_ovl
#define		wn_船橋二階居住区左舷				2f_live_left_alp_ovl
#define		wn_船橋三階居住区左舷				3f_live_left_alp_ovl
#define		wn_船橋三階居住区					3f_live_alp_ovl
#define		wn_船橋四階居住区					4f_live_alp_ovl
#define		wn_船橋五階操舵室					5f_souda_alp_ovl

#define		wn_航海甲板ウイング					kanpan_alp_ovl
#define		wn_航海甲板右舷ウイング				kanpan_right_alp_ovl
#define		wn_航海甲板左舷ウイング				kanpan_left_alp_ovl

#define		wn_機関室							kikan_alp_ovl
#define		wn_機関室右舷						kikan_right_alp_ovl
#define		wn_機関室左舷						kikan_left_alp_ovl
#define		wn_機関室右舷入口					kikan_ent_right_alp_ovl
#define		wn_機関室左舷入口					kikan_ent_left_alp_ovl

#define		wn_第二甲板右舷						kanpan_02_right_alp_ovl
#define		wn_第二甲板左舷						kanpan_02_left_alp_ovl

#define		wn_第一船倉							sensou_01_alp_ovl
#define		wn_第二船倉							sensou_02_alp_ovl
#define		wn_第三船倉							sensou_03_alp_ovl

#define		wn_Ａ脚底部海底ドック				a_kaitei_alp_ovl
#define		wn_Ａ脚屋上							a_okujou_alp_ovl
#define		wn_Ａ脚ポンプ室						a_pump_alp_ovl
#define		wn_ＡＢ連絡橋						ab_renraku_alp_ovl
#define		wn_Ｂ脚変電室						b_henden_alp_ovl
#define		wn_ＢＣ連絡橋						bc_renraku_alp_ovl
#define		wn_Ｃ脚食堂							c_shokudou_alp_ovl
#define		wn_ＣＤ連絡橋						cd_renraku_alp_ovl
#define		wn_Ｄ脚第一沈殿池					d_chinden_alp_ovl
#define		wn_ＤＥ連絡橋						de_renraku_alp_ovl
#define		wn_Ｅ脚集配場						e_shuhai_alp_ovl
#define		wn_Ｅ脚集配場１Ｆ					e_shuhai_1f_alp_ovl
#define		wn_Ｅ脚集配場２Ｆ					e_shuhai_2f_alp_ovl
#define		wn_Ｅ脚集配場Ｂ１					e_shuhai_b1_alp_ovl
#define		wn_Ｅ脚ヘリポート					e_heliport_alp_ovl
#define		wn_ＥＦ連絡橋						ef_renraku_alp_ovl
#define		wn_Ｆ脚倉庫							f_souko_alp_ovl
#define		wn_Ｆ脚倉庫１Ｆ						f_souko_1f_alp_ovl
#define		wn_Ｆ脚倉庫Ｂ１						f_souko_b1_alp_ovl
#define		wn_ＦＡ連絡橋						fa_renraku_alp_ovl

#define		wn_シェル１中央棟１Ｆ				1_chuou_1f_alp_ovl
#define		wn_シェル１中央棟Ｂ１				1_chuou_b1_alp_ovl
#define		wn_シェル１中央棟Ｂ１集会場			1_chuou_b1_shukai_alp_ovl
#define		wn_シェル１中央棟Ｂ２電算室			1_chuou_b2_densan_alp_ovl

#define		wn_ＤＧ連絡橋						dg_renraku_alp_ovl
#define		wn_ＬＧ連絡橋						lg_renraku_alp_ovl
#define		wn_Ｌ脚外周							l_gaishu_alp_ovl
#define		wn_ＫＬ連絡橋						kl_renraku_alp_ovl
#define		wn_Ｌ脚汚水処理場					l_osui_alp_ovl
#define		wn_Ｌ脚下部オイルフェンス			l_oilfence_alp_ovl

#define		wn_シェル２中央棟１Ｆ空気清浄室		2_chuou_1f_kuki_alp_ovl
#define		wn_シェル２中央棟Ｂ１第一濾過室		2_chuou_b1_roka_01_alp_ovl
#define		wn_シェル２中央棟Ｂ１第二濾過室		2_chuou_b1_roka_02_alp_ovl

#define		wn_アーセナルギア胃					ag_stmach_alp_ovl
#define		wn_アーセナルギア空腸				ag_jejunum_alp_ovl
#define		wn_アーセナルギア上行結腸			ag_ascending_alp_ovl
#define		wn_アーセナルギア回腸				ag_ileum_alp_ovl
#define		wn_アーセナルギアＳ状結腸			ag_sigmoid_alp_ovl
#define		wn_アーセナルギア直腸				ag_rectum_alp_ovl

#define		wn_青山二丁目						aoyama_alp_ovl
#define		wn_麻布十番							azabu_alp_ovl
#define		wn_船橋市							funabashi_alp_ovl
#define		wn_ニューヨーク52番街				newyork_alp_ovl

#define		wn_フェデラルホール					federal_alp_ovl

// キャラ付随水飛沫が出水後持続する時間
#define		CHARA_SPLASH_TIME	240



//ドッグタグ用ステージコード
enum ステージ名 {
	船尾甲板					= 0,
	船橋一階居住区,
	船橋一階リフレッシュルーム,
	船橋二階居住区,
	船橋三階居住区,
	船橋四階居住区,
	船橋五階操舵室,
	航海甲板ウィング,
	機関室,
	第二甲板右舷,
	第二甲板左舷,
	第一船倉,
	第二船倉,
	第三船倉,
	Ａ脚底部海底ドック,
	Ａ脚屋上,
	Ａ脚ポンプ室,
	ＡＢ連絡橋,
	Ｂ脚変電室,
	ＢＣ連絡橋,
	Ｃ脚食堂,
	ＣＤ連絡橋,
	Ｄ脚第一沈殿池,
	ＤＥ連絡橋,
	Ｅ脚集配場,
	Ｅ脚ヘリポート,
	ＥＦ連絡橋,
	Ｆ脚倉庫,
	ＦＡ連絡橋,
	シェル１中央棟１Ｆ,
	シェル１中央棟Ｂ１,
	シェル１中央棟Ｂ１集会場,
	シェル１中央棟Ｂ２電算室,
	ＤＧ連絡橋,
	ＤＧ連絡橋〜ＬＧ連絡橋,
	Ｌ脚外周〜ＫＬ連絡橋,
	ＫＬ連絡橋,
	Ｌ脚汚水処理場,
	シェル２中央棟１Ｆ空気清浄室,
	シェル２中央棟Ｂ１第一濾過室,
	シェル２中央棟Ｂ１第二濾過室,
	Ｌ脚下部オイルフェンス,
	アーセナルギア胃,
	アーセナルギア空腸,
	アーセナルギア上行結腸,
	アーセナルギア回腸,
	アーセナルギアＳ状結腸,
	アーセナルギア直腸,
	フェデラルホール,
	プラント編スタート,
	タンカー編クリア,
	プラント編クリア
}

// プレイヤー定義
// ------------------------------------
#ifndef d:PLAYER
	#define PLAYER	スネーク
#endif

// プレイヤー出現時姿勢
// ------------------------------------
#define		P_F_STANCE_STAND		0		// 立ち
#define		P_F_STANCE_CROUCH		1		// しゃがみ
#define		P_F_STANCE_CRAWL		2		// 匍匐
#define		P_F_STANCE_INTRUDE		3		// イントルード

// 武器・アイテム配列設定
// ------------------------------------
dim $w:武器最大数[ d:武器種類 ]
dim $w:武器弾数[ d:武器種類 ]
dim $w:アイテム最大数[ d:アイテム種類 ]
dim $w:アイテム数[ d:アイテム種類 ]
dim $w:武器最大数Ｒ[ d:武器種類 ]
dim $w:武器弾数Ｒ[ d:武器種類 ]
dim $w:アイテム最大数Ｒ[ d:アイテム種類 ]
dim $w:アイテム数Ｒ[ d:アイテム種類 ]


// レベルアップする懸垂回数
// ------------------------------------
#define		HAND_P_UP_LV1	100
#define		HAND_P_UP_LV2	200

// Ｏ２ゲージの長さ
// ------------------------------------
#define		O2_GAGE			160

// アラートモード
// ------------------------------------
#define ALERT_MODE_SENNYU	0
#define ALERT_MODE_KIKEN	1
#define ALERT_MODE_KAIHI	2
#define ALERT_MODE_TANSAKU	3

dim $w:スネークグリップ最大[ 4 ]
dim $w:ライデングリップ最大[ 4 ]
dim $i:クリアコード[ 4 ]

#define 	アイテム水飛沫有	1
#define 	アイテム水飛沫無	0

// エリア履歴配列設定
// ------------------------------------
dim $s:エリア履歴[ 4 ]


// ボスラッシュモード用enum
// ------------------------------------
enum boss_no {
	B_OLGA = 0,
	B_FATMAN,
	B_HARRIER,
	B_VAMP,
	B_RAY,
	B_SOLIDUS,
	B_ALL
}

dim $i:経過時間[ 7 ]
dim $i:気絶フラグ[ 6 ]


// ローポリ劇場用
// ------------------------------------
dim $s:新キャスト[ 5 ]
dim $s:新モデル種類[ 5 ]

dim $s:元キャスト[ 5 ]
dim $s:元モデル種類[ 5 ]


// 各ステージ別アイテム再セット用配列設定
// ------------------------------------
dim $b:w00a_取得ロード回数[ 8 ]
dim $b:w00b_取得ロード回数[ 3 ]
dim $b:w00c_取得ロード回数[ 7 ]		// 4+3
dim $b:w01a_取得ロード回数[ 9 ]		// 6+3
dim $b:w01f_取得ロード回数[ 8 ]		// 5+3
dim $b:w01b_取得ロード回数[ 7 ]		// 4+3
dim $b:w01c_取得ロード回数[ 5 ]		// 2+3
dim $b:w01d_取得ロード回数[ 6 ]
dim $b:w01e_取得ロード回数[ 5 ]		// 2+3
dim $b:w02a_取得ロード回数[ 11 ]	// 8+3
dim $b:w03a_取得ロード回数[ 5 ]
dim $b:w03b_取得ロード回数[ 5 ]
dim $b:w04a_取得ロード回数[ 5 ]
dim $b:w04b_取得ロード回数[ 5 ]
dim $b:w04c_取得ロード回数[ 5 ]

dim $b:w11a_取得ロード回数[ 16 ]	// 9
dim $b:w12a_取得ロード回数[ 4 ]
dim $b:w12b_取得ロード回数[ 7 ]
dim $b:w13a_取得ロード回数[ 5 ]
dim $b:w14a_取得ロード回数[ 5 ]
dim $b:w15a_取得ロード回数[ 5 ]
dim $b:w16a_取得ロード回数[ 4 ]
dim $b:w17a_取得ロード回数[ 5 ]
dim $b:w18a_取得ロード回数[ 4 ]
dim $b:w19a_取得ロード回数[ 5 ]
dim $b:w20a_取得ロード回数[ 20 ]	// 12
dim $b:w20b_取得ロード回数[ 4 ]
dim $b:w21a_取得ロード回数[ 5 ]
dim $b:w22a_取得ロード回数[ 60 ]	// 50
dim $b:w23a_取得ロード回数[ 5 ]
dim $b:w24a_取得ロード回数[ 11 ]	// 11
dim $b:w24b_取得ロード回数[ 4 ]
dim $b:w24c_取得ロード回数[ 3 ]
dim $b:w24d_取得ロード回数[ 10 ]	// 8+2
dim $b:w25c_取得ロード回数[ 5 ]
dim $b:w25d_取得ロード回数[ 5 ]
dim $b:w28a_取得ロード回数[ 5 ]
dim $b:w31a_取得ロード回数[ 32 ]	// 25
dim $b:w31b_取得ロード回数[ 11 ]	// 9+2
dim $b:w31c_取得ロード回数[ 25 ]	// 13
dim $b:w41a_取得ロード回数[ 5 ]
dim $b:w42a_取得ロード回数[ 3 ]
dim $b:w43a_取得ロード回数[ 5 ]
dim $b:w45a_取得ロード回数[ 5 ]

// 各ステージ別壊れ物用配列設定
// ------------------------------------
dim $b:w00a_破壊ロード回数[ 40 ]
dim $s:w00a_壊れライト名[ 40 ]
dim $f:w00a_壊れライトフラグ[ 40 ]
dim $i:w01a_ロッカーロード回数[ 8 ]
dim $b:w01a_破壊ロード回数[ 8 ]
dim $i:w01b_ロッカーロード回数[ 1 ]
dim $b:w01b_破壊ロード回数[ 1 ]
dim $i:w01c_ロッカーロード回数[ 1 ]
dim $b:w01c_破壊ロード回数[ 1 ]
dim $b:w01c_カメラ破壊ロード回数[ 1 ]
dim $b:w01c_カメラ破壊フラグ[ 1 ]
dim $b:w01d_破壊ロード回数[ 1 ]
dim $i:w02a_ロッカーロード回数[ 7 ]
dim $b:w02a_破壊ロード回数[ 7 ]
dim $b:w11a_破壊ロード回数[ 8 ]
dim $b:w11a_揺れライト破壊ロード回数[ 12 ]
dim $f:w11a_揺れライト壊れフラグ[ 12 ]
dim $b:w11a_ロッカー破壊ロード回数[ 3 ]
dim $i:w11a_ロッカー状態[ 3 ]
dim $b:w12b_破壊ロード回数[ 8 ]
dim $i:w12b_ロッカー状態[ 4 ]
dim $b:w14a_破壊ロード回数[ 10 ]
dim $i:w14a_ロッカー状態[ 7 ]
dim $b:w15a_破壊ロード回数[ 5 ]
dim $b:w16a_破壊ロード回数[ 35 ]
dim $i:w16a_ロッカー状態[ 6 ]
dim $b:w17a_破壊ロード回数[ 10 ]
dim $b:w18a_破壊ロード回数[ 30 ]
dim $b:w20a_破壊ロード回数[ 40 ]		// 37
dim $b:w20a_カメラ破壊ロード回数[ 3 ]
dim $b:w20a_カメラ破壊フラグ[ 3 ]
dim $b:w21a_破壊ロード回数[ 20 ]
dim $b:w22a_破壊ロード回数[ 12 ]		// 12
dim $f:w22a_ライト破壊状態フラグ[ 12 ]	// 12
dim $b:w22a_ロッカー破壊ロード回数[ 11 ]
dim $i:w22a_ロッカー状態[ 11 ]
dim $b:w23a_破壊ロード回数[ 5 ]
dim $i:w24a_ロッカーロード回数[ 9 ]
dim $b:w24a_破壊ロード回数[ 9 ]
dim $b:w24a_カメラ破壊ロード回数[ 5 ]
dim $b:w24a_カメラ破壊フラグ[ 5 ]
dim $b:w24b_破壊ロード回数[ 6 ]
dim $i:w24b_ロッカー状態[ 6 ]

dim $f:w24c_壊れライトフラグ[ 12 ]
dim $s:w24c_壊れライト名[ 12 ]

dim $i:w24d_ロッカーロード回数[ 6 ]
dim $b:w24d_破壊ロード回数[ 6 ]
dim $b:w31a_破壊ロード回数[ 30 ]		// 27
dim $b:w31a_破壊フラグ[ 30 ]			// 22
dim $b:w31b_機雷破壊フラグ[ 3 ]
dim $b:w31c_破壊ロード回数[ 4 ]			// 4
dim $i:w41a_ロッカーロード回数[ 1 ]
dim $i:w41a_ロッカー状態[ 1 ]

// 各ステージ別配列設定
// ------------------------------------
dim $b:w22a_クリアリング回避回数[ 10 ]	//	9
dim $f:w31c_ロッカー開閉フラグ[ 8 ]		//	8
dim $f:w31c_爆発物フラグ[ 10 ]			//	6
dim $f:w31c_クレイモアフラグ[ 5 ]		//	4


//タンカーデジカメ初期化	//１枚分多く定義！！
	dim $w:タンカーデジカメ写真属性[7]


//くしゃみ変数配列
	dim $b:タンカーくしゃみ間隔配列[20]
	dim $b:タンカーくしゃみ回数配列[20]

//船倉兵モデル番号（パンツ兵用）
	dim $b:w04_船倉兵モデル番号[ 20 ]


//演説ボリューム制御
	dim $b:演説ボリューム[2]
	dim $b:演説パン角度[2]
	dim $i:法線大きさ２[2]


	dim $w:最短ルートナンバー[13]


// -------------------------------------------------------------------------------------------
// ゲームとデモで共用するdefineここまで
// -------------------------------------------------------------------------------------------



#ifndef d:DEMO_VAR
// -------------------------------------------------------------------------------------------
// ゲームのみで使用する変数設定
// -------------------------------------------------------------------------------------------

// 敵兵視界(難易度によって変化)
// ------------------------------------
#ifndef d:CODEC_FILE
if ($w:ゲーム設定 == d:LEVEL_VERYEASY) {
#ifdef d:JAPANESE
	eval($w:敵潜入視力 = 3500);
#else
	eval($w:敵潜入視力 = 4000);
#endif
	eval($w:敵危険視力 = ((5000*3)/2));
	eval($w:敵回避視力 = 5000);
	eval($w:敵標準聴力 = 5000);
#ifdef d:JAPANESE
	eval($w:敵標準体力 = 500);
#else
	eval($w:敵標準体力 = 3000);
#endif
	eval($w:敵標準気絶 = 3);
	eval($i:敵標準麻酔持続 = 60*60*9);
	eval($i:敵標準気絶持続 = 60*60);
	eval($w:敵標準再発生数 = 0);
	eval($w:サイファー標準再発生数 = 0);
} else if ($w:ゲーム設定 == d:LEVEL_EASY) {
	eval($w:敵潜入視力 = 5000);
	eval($w:敵危険視力 = ((6000*3)/2));
	eval($w:敵回避視力 = 6000);
	eval($w:敵標準聴力 = 6000);
	eval($w:敵標準体力 = 3000);
	eval($w:敵標準気絶 = 7);
	eval($i:敵標準麻酔持続 = 60*60*9);
	eval($i:敵標準気絶持続 = 60*60);
	eval($w:敵標準再発生数 = 3);
	eval($w:サイファー標準再発生数 = 3);
} else if ($w:ゲーム設定 == d:LEVEL_NORMAL) {
#ifdef d:JAPANESE
	eval($w:敵潜入視力 = 5500);
#else
	eval($w:敵潜入視力 = 6000);
#endif
	eval($w:敵危険視力 = ((6000*3)/2));
	eval($w:敵回避視力 = 6000);
	eval($w:敵標準聴力 = 6000);
	eval($w:敵標準体力 = 3000);
	eval($w:敵標準気絶 = 9);
	eval($i:敵標準麻酔持続 = 60*60*5);
#ifdef d:JAPANESE
	eval($i:敵標準気絶持続 = 60*45);
#else
	eval($i:敵標準気絶持続 = 60*30);
#endif
	eval($w:敵標準再発生数 = 10);
	eval($w:サイファー標準再発生数 = 10);
} else if ($w:ゲーム設定 == d:LEVEL_HARD) {
	eval($w:敵潜入視力 = 7000);
	eval($w:敵危険視力 = ((8000*3)/2));
	eval($w:敵回避視力 = 8000);
	eval($w:敵標準聴力 = 8000);
	eval($w:敵標準体力 = 4000);
	eval($w:敵標準気絶 = 14);
	eval($i:敵標準麻酔持続 = 60*60*3);
	eval($i:敵標準気絶持続 = 60*30);
	eval($w:敵標準再発生数 = 16);
	eval($w:サイファー標準再発生数 = 16);
} else if ($w:ゲーム設定 == d:LEVEL_EXTREME) {
	eval($w:敵潜入視力 = 7000);
	eval($w:敵危険視力 = ((8000*3)/2));
	eval($w:敵回避視力 = 8000);
	eval($w:敵標準聴力 = 8000);
	eval($w:敵標準体力 = 6000);
	eval($w:敵標準気絶 = 19);
	eval($i:敵標準麻酔持続 = 60*60*3);
	eval($i:敵標準気絶持続 = 60*30);
	eval($w:敵標準再発生数 = 16);	// 発見されたらアウトなので意味はないが一応
	eval($w:サイファー標準再発生数 = 16);
} else if ($w:ゲーム設定 == d:LEVEL_E_EXTREME) {
	eval($w:敵潜入視力 = 10000);
	eval($w:敵危険視力 = ((10000*3)/2));
	eval($w:敵回避視力 = 10000);
	eval($w:敵標準聴力 = 10000);
	eval($w:敵標準体力 = 10000);
	eval($w:敵標準気絶 = 24);
	eval($i:敵標準麻酔持続 = 60*40);
	eval($i:敵標準気絶持続 = 60*30);
	eval($w:敵標準再発生数 = 16);	// 発見されたらアウトなので意味はないが一応
	eval($w:サイファー標準再発生数 = 16);
}

#ifdef d:STAGE_PLANT
if ( $w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１開始 ) {
	// 最初の海底ドックは一番短い
	eval( $i:標準警戒時間 = 60*45 );
} else if ( $w:p_story >= d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１開始 && \
	$w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了 ) {
	// 通常
	eval( $i:標準警戒時間 = 60*60 );
} else if ( $w:p_story >= d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了 && \
	$w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始 ) {
	// エイムズ死亡後はすごく長い
	eval( $i:標準警戒時間 = 60*60*2 );
} else {
	// エマイベントのときは通常に戻す
	eval( $i:標準警戒時間 = 60*60 );
}
#else
// タンカー編は固定
eval( $i:標準警戒時間 = 60*60 );

#endif

#endif

// -------------------------------------------------------------------------------------------
// ゲームのみで使用するdefineとdim宣言、変数設定ここまで
// -------------------------------------------------------------------------------------------
#endif







// -------------------------------------------------------------------------------------------
// ゲームとデモで共用するdefine（デバッグ用）
// -------------------------------------------------------------------------------------------
// 体験版用
// ------------------------------------
#ifdef d:MGS2_TRIAL
	// こちらの値は固定
	#define 体験版			1
#else
	#define 体験版			0
#endif
// デバッグ用
// ------------------------------------
#ifndef d:DEBUG_PRINT
#ifdef d:FIX_MODE		// ここはmakefileで切り分ける
	#define		DEBUG_PRINT		0	// こっちは0で固定
#else
	#define		DEBUG_PRINT		1
#endif
#endif

#ifndef d:NO_POLYDEMO	// このフラグが1だとポリデモに入らずリスタート
#ifdef d:FIX_MODE		// ここはmakefileで切り分ける
	#define		NO_POLYDEMO		0	// こっちは0で固定
#else
	#define		NO_POLYDEMO		0
#endif
#endif



#else

print 'vardef.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif
