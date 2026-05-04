/*
	polydemo_w11c.h
	    フォーチュン戦

	2001/01/18 H.Yoshiike         
	$Id: polydemo_w11c.h,v 1.56 2001/09/27 08:20:11 usr03635 Exp $                      

	
*/

/*
注意！　以下のevalは、stdch.hとdemo_stdch.h中の2Dレイアウトドライバ 
の二重呼びを回避するためのものです。各"polydemo_***.h" のこの位置、
include の前に必ずおくこと！！
*/
/* 絶対ここ！　場所も変えない！ */
eval($s:d_num = co_game_demo_stage)
/* 以上！ */

// include
// ------------------------------------
#include "5sec_bfr_demo.h"
#include "demo_stdch.h"
#include "demo_vardef.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif

#define エマ虫を使う	0

#define ダミーのデモデータでは起動させない 1

// キャスティングシアター対応。
#include "lowpoly.h"
@ローポリ設定

//###################################################################################
//								ストーリーフラグ & デモ再生
//###################################################################################
command マップ設定 昇降機ホール -set {

// デモ環境かどうか。
#if d:MGS2_DEMO


// 不要プログラムの起動阻害
	eval($s:d_num = P028_01_p01)

//	@デモ仕様セット

//	@ふなむしセット
	@フォーチュンの涙セット
chara デモアクトデバッグ demodebug  -i t:p028_01_p01 -d 'p028_01_p01' -l  -s 'p028_01_p01'  \
		-p 昇降機ドア開く 昇降機ドア閉じる \
		   昇降機スイッチ緑点灯 昇降機スイッチ赤点灯 \
		   昇降機スイッチ光移動_上から下へ 昇降機スイッチ光移動_下から上へ \
		   ふなむしオフ ふなむしオン \
		   ふなむし逃げる ふなむし戻ってくる \
		   フォーチュン涙アニメ開始 \
		   フォーチュン涙アニメ停止 \
		-begin ふなむしセット \		// ふなむしはデモが再生してから呼ばないとだめ


#else


if($w:p_story == d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１開始) {

	/* フォーチュン戦前のデモ */

	// デモ特有の変数代入
	eval($s:d_num = P028_01_p01)
	// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
	@デモではレーダー不要
	@デモモデル差し替え処理
	proc デモ再生_P028_01_p01 {
		chara demo デモ再生 \
			-i t:p028_01_p01 \ // 最初のpは小文字？
			-p 昇降機ドア開く 昇降機ドア閉じる \
			   昇降機スイッチ緑点灯 昇降機スイッチ赤点灯 \
			   昇降機スイッチ光移動_上から下へ 昇降機スイッチ光移動_下から上へ \
			   ふなむしオフ ふなむしオン \
			   ふなむし逃げる ふなむし戻ってくる \
			-begin ふなむしセット \		// ふなむしはデモが再生してから呼ばないとだめ
			-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
			-next P028_01_p01終了
	}

	@デモ再生_P028_01_p01

} else if($w:p_story == d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１開始) {

	/* フォーチュン戦終了後のデモ　ヴァンプ撃たれる */

	// デモ特有の変数代入
	eval($s:d_num = P031_01_p01)
	// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
	@デモではレーダー不要
	@デモモデル差し替え処理
	proc デモ再生_P031_01_p01 {
		chara demo デモ再生 \
			-i t:p031_01_p01 \
			-p 昇降機ドア開く 昇降機ドア閉じる \
			   昇降機スイッチ緑点灯 昇降機スイッチ赤点灯 \
			   昇降機スイッチ光移動_上から下へ 昇降機スイッチ光移動_下から上へ \
			   フォーチュン涙アニメ開始 \
			   フォーチュン涙アニメ停止 \
			-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
			-next P031_01_p01終了
	}

	@デモ再生_P031_01_p01

// デモの時以外は、以下を通過する。
} else {

	@ゲームステージへ戻る

}

#endif

// デモ仕様のセット
if($s:d_num != not_demo){
	// 共通して使用
	chara デモキャンセルチェック デモキャンセル \
		-p demo_cancel

	if($s:d_num == P028_01_p01){
		@ノード端末設定
	}

print 'デモ再生中_000'

	@南壁セット
print 'デモ再生中_001'
	@天井セット
print 'デモ再生中_002'
	@シネマスクリーンセット
print 'デモ再生中_003'
	@デモ用エレベータシャフトセット
print 'デモ再生中_004'
	if($s:d_num == P031_01_p01){
		@フォーチュンの涙セット
	}
print 'デモ再生中_005'


}

} // < 「command マップ設定 昇降機ホール」 の閉じカッコ

// 初期マップ表示は、ゲームgclの方で実施。

//###################################################################################
//									デモのみ仕様各種
//###################################################################################
// ダミープロック
proc dummy_proc {}

// デモキャンセル
proc demo_cancel {
	mesg delay ふなむしディレイ kill
	command StreamStopAll
}

// デモ終了時プロック
proc P028_01_p01終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill
	eval($w:p_story = d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１終了)
	print '$w:p_story = d:ST:P028_01_P01爆弾解体後フォーチュン再登場１ポリゴンデモ１終了'

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			// ゲームへ
			@ゲームステージへ戻る
			restart -s
		}
}

proc P031_01_p01終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill
	eval($w:p_story = d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了)
	print '$w:p_story = d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了'

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			// ゲームへ
			@ゲームステージへ戻る
			@mv_w11c_w12a_0
		}
}


// シネマスクリーン
proc シネマスクリーンセット {
	chara シネマスクリーン シネマ \
		-top d:CINEMA_BELT_UPPER \
		-bottom d:CINEMA_BELT_LOWER \
		-ftime 60	//デフォルト値（ゼロは駄目）
	mesg シネマスクリーン シネマ フェードアウト 1
}

proc 南壁セット {
	chara プットオブジェ 南壁 \
		-m d:WALL_NAME1 \
		-r 0 0 0 \
		-p 0 0 0 \
		-l d:LT2_NAME \
		-s 100 100 100
}

proc 天井セット {
	chara プットオブジェ 天井 \
		-m d:CEIL_NAME2 \
		-r 0 0 0 \
		-p 0 0 0 \
		-l d:LT2_NAME \
		-s 100 100 100
}

proc 昇降機ドア開く {
	@昇降機到着
}

proc 昇降機ドア閉じる {
	@昇降機出発
}

proc 昇降機スイッチ緑点灯 {
	mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 緑に点灯
}

proc 昇降機スイッチ赤点灯 {
	mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 赤に点灯
}

proc 昇降機スイッチ光移動_上から下へ {
	mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 下ボタンを押す
}

proc 昇降機スイッチ光移動_下から上へ {
	mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 上ボタンを押す
}

// 延長したエレベータシャフト（デモ用）
proc デモ用エレベータシャフトセット {
	chara プットオブジェ 延長エレベータシャフト \
		-m w11_elv_append \
		-r 0 0 0 \
		-p 0 0 0 \
		-l d:LT2_NAME \
		-s 100 100 100
}

proc ふなむしセット {
chara delay ふなむしディレイ -time 60 -exec {

	#if d:エマ虫を使う
		chara エマ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu001
		chara エマ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu002
		chara エマ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu003
		chara エマ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu004
		chara エマ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu005
	#else
print '----------------------------------------11111111111111111111demo'
		chara デモ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu001 \
			-n_rfg 2 \
			-refuge refuge1 escape1 \
					refuge2 escape2
		chara デモ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu002 \
			-n_rfg 2 \
			-refuge refuge1 escape1 \
					refuge2 escape2
		chara デモ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu003 \
			-n_rfg 2 \
			-refuge refuge1 escape1 \
					refuge2 escape2
		chara デモ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu004 \
			-n_rfg 2 \
			-refuge refuge1 escape1 \
					refuge2 escape2
		chara デモ虫 かさかさ \
			-group 28 \
			-atrap all_funa01 \
			-trap fu005 \
			-n_rfg 2 \
			-refuge refuge1 escape1 \
					refuge2 escape2
	#endif

	chara 船虫踏み潰し管理 かさかさ親分 \
		-group 28 \
		-atrap all_funa01 \
		-n_obj 1 \
		-obj for_coat_mh_demo

	chara 潰れふなむし ぺちゃんこ

	}
}
proc ふなむしオフ {
	#if d:エマ虫を使う
		mesg エマ虫 かさかさ 非表示
	#else
		mesg デモ虫 かさかさ 非表示
	#endif
}
proc ふなむしオン {
	#if d:エマ虫を使う
		mesg エマ虫 かさかさ 表示
	#else
		mesg デモ虫 かさかさ 表示
	#endif
}
proc ふなむし逃げる {
	#if d:エマ虫を使う
	#else
		mesg デモ虫 かさかさ 複数撤収 0 7 0
		mesg デモ虫 かさかさ 複数撤収 8 15 1
		mesg デモ虫 かさかさ 複数撤収 16 23 2
		mesg デモ虫 かさかさ 複数撤収 24 31 3
	#endif
}
proc ふなむし戻ってくる {
	#if d:エマ虫を使う
	#else
		mesg デモ虫 かさかさ 複数起動 0 7
		mesg デモ虫 かさかさ 複数起動 8 15
		mesg デモ虫 かさかさ 複数起動 16 23
		mesg デモ虫 かさかさ 複数起動 24 31
	#endif
}

// フォーチュンの涙
proc フォーチュンの涙セット {
print 'フォーチュン涙セット'
	chara アニメテクスチャセット フォーチュン涙 \
		-tex zzz_tears ter_ovl_sub_alp_0001 \
		-anmtex tears_sub \		// ratファイル
		-interval 4 \
		-flag 0x0002 // 停止状態で起動

	chara アニメテクスチャセット フォーチュン涙 \
		-tex zzz_tears ter_rgb_ovl_alp_0001 \
		-anmtex tears_rgb \		// ratファイル
		-interval 4 \
		-flag 0x0002 // 停止状態で起動
}

proc フォーチュン涙アニメ開始 {
	mesg アニメテクスチャセット フォーチュン涙 start
}
proc フォーチュン涙アニメ停止 {
	mesg アニメテクスチャセット フォーチュン涙 stop
}

proc ノード端末設定 {
	chara ノード端末ディスプレイ NewNodeLamp -r -512	-p 3500 -45000 -9500
}

