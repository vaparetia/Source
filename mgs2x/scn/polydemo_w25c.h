/*
	polydemo_w25c.h                     
	    w25c1-Ｌ脚（倉庫前）デモ用ヘッダ

	2000/11/30 Y.Matsuhana         
	$Id: polydemo_w25c.h,v 1.10 2001/08/22 14:32:31 usr03635 Exp $                      

*/

/*
注意！　以下のevalは、stdch.hとdemo_stdch.h中の2Dレイアウトドライバ 
の二重呼びを回避するためのものです。各"polydemo_***.h" のこの位置、
include の前に必ずおくこと！！
*/
/* 絶対ここ！　場所も変えない！ */
eval($s:d_num = co_game_demo_stage)
/* 以上！ */

// includes
// ------------------------------------
#include "5sec_bfr_demo.h"
#include "demo_stdch.h"
#include "demo_stdch2.h"
#include "demo_vardef.h"

#include "sea_surface.h"

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif


//###################################################################################
//								ストーリーフラグ & デモ再生
//###################################################################################
command マップ設定 タイトロープ２ -set {

// デモ環境かどうか。
#if d:MGS2_DEMO

// 不要プログラムの起動阻害
	eval($s:d_num = P036_04_p03)

chara デモアクトデバッグ demodebug  -i t:data_dummy_raiden -d 'data_dummy_raiden' -l   \
		-p  dummy_proc

#else


// ゲームＧＣＬと共用で入ってくる部分なので、== で条件を見る。
if($w:p_story == d:ST:P063_01_P01カード五１ポリゴンデモ１開始) {
	print'd:ST:P063_01_P01カード五１ポリゴンデモ１開始'
//	eval($w:p_story = d:ST:P063_01_P01カード五１ポリゴンデモ１開始)

	/* エマからカード５をもらう */
	/* 本来はここでカードを渡す代入文を入れる所だが、
	   実際にはデモに入る前にゲームgclの方で代入してある
	*/

	// デモ特有の変数代入
	eval($s:d_num = P063_01_p01)
	// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
	@デモではレーダー不要
	@デモモデル差し替え処理
	chara demo デモ再生 \		// Y.Matsuhana 2001.7.10 一部変更
//		-i t:p063_01_p01 \
		-i t:data_dummy_raiden \
		-p dummy_proc \
		-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
		-next P063_01_p01終了


// デモ以外の場合こちらを通る
} else {

	@ゲームステージへ戻る

}

#endif


// デモ仕様のセット
	if($s:d_num != not_demo){
		// 共通して使用するもの
		chara デモキャンセルチェック デモキャンセル \
			-p demo_cancel

		@シネマスクリーンセット

		@demo_ステージモデルセット
		#if 0
			@空と海面設置 // w25d.gcl で定義
		#else
			@プラントステージでの汎用空セット 夕 w25d
			@demo_海面設置
		#endif
		#if 1
			@フォグ設定
		#else
			@demo_フォグ設定
		#endif
		@demo_ドア設置

	}

} // < 「command マップ設定 タイトロープ２」 の閉じカッコ


// 初期マップ表示
// ゲームgclの方で実施。

// 各proc設定
//-------------------------------------------------------------------------------------
//###################################################################################
//									デモのみ仕様各種
//###################################################################################
// ダミープロック
proc dummy_proc {}

// デモキャンセル
proc demo_cancel {
	command StreamStopAll
}

// リスタート
proc restart_proc {
	restart
}

// デモ終了時プロック
proc P063_01_p01終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill
	eval($w:p_story = d:ST:P063_01_P01カード五１ポリゴンデモ１終了)
	print '$w:p_story = d:ST:P063_01_P01カード五１ポリゴンデモ１終了'

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			// ゲームへ
			@ゲームステージへ戻る
			restart
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


	// ドア
	// ------------------------------------
	proc demo_ドア設置 {
		chara プットオブジェ 中央棟へのドア \
			-m d:DOOR_NAME_LOAD \
			-r 0,1024,0 \
			-p 19125,-1500,-234250 \
			-s 100,100,100 \
			-l d:LT2_NAME

		chara プットオブジェ Ｌ脚ドア \
			-m d:DOOR_NAME \
			-r 0,3072,0 \
			-p -8875,0,-231500 \
			-s 100,100,100 \
			-l d:LT2_NAME
	}

	// フォグ
	// ------------------------------------
	proc demo_フォグ設定 {
		@プラントステージでのフォグセット 昼 0 
	}

	// エフェクト関係
	// -----------------------------------
	proc demo_エフェクト設定 {
//			chara ブラー ブラー
//			chara Ｚフォーカス管理 Z_manager
//			mesg Ｚフォーカス管理 Z_manager パラメータ 5000 30
			chara ＯＦＦ制御 off_man
			mesg ＯＦＦ制御 off_man ボディシャドウＯＦＦ
	}

// 各マップ別設定
// ------------------------------------
// Ｌ脚
	// ステージモデル自体
	proc demo_ステージモデルセット {
		chara プットオブジェ Ｌ脚南壁 \
			-m w25d1 \	// 2001.7.10 Y.Matsuhana 最終モデル納品のため一部変更
			-r 0,0,0 \
			-p 0,0,0 \
			-s 100,100,100 \
			-l d:LT2_NAME
	}


	// 海面設置
	// ------------------------------------
	proc demo_海面設置 {
		#if d:DEBUG_PRINT
			print 'w25c_sea_set'
		#endif

		@プラント海面セット 夕 w25c 1
	}

