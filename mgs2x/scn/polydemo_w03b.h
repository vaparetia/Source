/*
	polydemo_w03b.h
	    右舷長廊下デモ用ヘッダ

	1999/02/02 S.Nojiri         
	$Id: polydemo_w03b.h,v 1.21 2001/09/26 13:03:43 usr03635 Exp $                      

	
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
#include "demo_vardef.h"
#include "demo_stdch.h"
#include "sload.h"

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif

//###################################################################################
//								ストーリーフラグ & デモ再生
//###################################################################################
command マップ設定 右舷長廊下１ 右舷長廊下３ 中央長廊下 -set {

print '長廊下デモステージ_001'

// デモ環境かどうか。
#if d:MGS2_DEMO

// 不要プログラムの起動阻害
//	eval($s:d_num = t08a1D)
	eval($s:d_num = t09a1D)
	eval($w:t_story = d:ST_T08a1Dハッチ閉鎖ポリゴンデモ開始)
//	@船倉の一部セット
//	@シネマスクリーンセット
//	@リアル影セット

chara デモアクトデバッグ demodebug  -i t:t09a1D -d 't09a1D' -l   \
	-p 兵隊３人影出す 兵隊３人影引っ込める 警戒スネークオン 警戒スネークオフ \
	   オセロット影投影オン オセロット影投影オフ

#else


print '長廊下デモステージ_002'

if($w:t_story == d:長廊下兵との戦闘突入ポリデモ開始) {

	// デモ特有の変数代入
	eval($s:d_num = t08a1D)
	// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
	@デモではレーダー不要

print '長廊下デモステージ_003'
	@デモモデル差し替え処理
	proc デモ再生_t08a1D {
		chara demo デモ再生 \
			-i t:t08a1D \
			-p 兵隊３人影出す 兵隊３人影引っ込める \
			-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
			-next t08a1D終了
	}

print '長廊下デモステージ_004'

	@デモ再生_t08a1D

print '長廊下デモステージ_005'


} else if( $w:t_story == d:ST_T08a1Dハッチ閉鎖ポリゴンデモ開始 ) {

print '長廊下デモステージ_006'

	// デモ特有の変数代入
	eval($s:d_num = t09a1D)

print '長廊下デモステージ_007'
	@デモモデル差し替え処理
	proc デモ再生_t09a1D {
		chara demo デモ再生 \
			-i t:t09a1D \
			-p 警戒スネークオン 警戒スネークオフ \
			   オセロット影投影オン オセロット影投影オフ \
			-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
			-next t09a1D終了
	}

print '長廊下デモステージ_008'

	@デモ再生_t09a1D

// デモの時以外は、以下を通過する。
} else {

	eval($s:d_num = not_demo)

}

#endif

print 'でもデモでもデモでも_000'
	// デモ仕様のセット
	// どちらのデモでも共通して使用するもの

print '長廊下デモステージ_009'

	if($s:d_num == t08a1D || $s:d_num == t09a1D) {
print '長廊下デモステージ_010'

		@船倉の一部セット
print '長廊下デモステージ_011'

		@シネマスクリーンセット
print '長廊下デモステージ_012'

		@廊下奥のドアセット
print '長廊下デモステージ_013'

		@リアル影セット
print '長廊下デモステージ_014'

		chara デモキャンセルチェック デモキャンセル \
			-p demo_cancel
print '長廊下デモステージ_015'

	}
print 'でもデモでもデモでも_006'

} // < 「command マップ設定 右舷長廊下１ 右舷長廊下３ 中央長廊下」 の閉じカッコ

//###################################################################################
//									デモのみ仕様各種
//###################################################################################
// ダミープロック
proc dummy_proc {}

// デモキャンセル
proc demo_cancel {
	command StreamStopAll
}

// デモ終了時プロック
proc t08a1D終了 {
	mesg デモキャンセルチェック デモキャンセル kill
	eval($w:t_story = d:長廊下兵との戦闘突入ポリデモ終了)
	print '$w:t_story = d:長廊下兵との戦闘突入ポリデモ終了'

	// リスタート後、ゲーム用の仕様をセットするためにフラグを戻す
	eval($s:d_num = not_demo)

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			@イベント兵登場デモ終了
		}
}

// デモ終了時プロック
proc t09a1D終了 {
	mesg デモキャンセルチェック デモキャンセル kill
	eval($w:t_story = d:ST_T08a1Dハッチ閉鎖ポリゴンデモ終了)
	print '$w:t_story = d:ST_T08a1Dハッチ閉鎖ポリゴンデモ終了'

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			// このフラグでレーダーを出す。
			eval($f:global_polygon_demo = 0)

			load 'd10t'
//			@mv_w03b_w04a_demo
		}
}

// デモ用、船倉の一部
proc 船倉の一部セット {
	chara プットオブジェ 船倉の一部 -m w03b3 -r 0,0,0 -p 0,0,(-28500-39500) -s 100,100,100 -l d:LT2_NAME
}

// シネマスクリーン
proc シネマスクリーンセット {
	chara シネマスクリーン シネマ \
		-top d:CINEMA_BELT_UPPER \
		-bottom d:CINEMA_BELT_LOWER \
		-ftime 60	//デフォルト値（ゼロは駄目）
	mesg シネマスクリーン シネマ フェードアウト 1
}

proc リアル影セット {
/*
デモスタート時はOFFで、プロック呼び出しでON/OFFできるようにする。
*/
/*
平行光で影を出すプログラム。点光源でのでないとうまくないか？
*/
#if 1
	if($s:d_num == t08a1D) {
		chara 影投影モデル 長廊下一部分モデル \
			-model sdw03b_cvtd \
			-pos 0 2 (-28500-39500)

		chara 影管理 長廊下一部分 \
			-type 0 \
			-length 6000
	} else if($s:d_num == t09a1D) {
		// 2001/09/16 Uehara Masatoshi [新規追加]
		chara 影投影モデル 長廊下の影もでる \
			-m w03b_sdw00 \
			-p 0 2 (-28500-39500)

		chara 影管理 影かんり \
			-t 0 \
			-p -300 -1000 200
	}
#endif
}

proc 兵隊３人影出す {
	mesg 影管理 長廊下一部分 demo_add gbs_defA 長廊下一部分モデル
	mesg 影管理 長廊下一部分 demo_add gbs_defB 長廊下一部分モデル
	mesg 影管理 長廊下一部分 demo_add gbs_defC_無線兵 長廊下一部分モデル
}

proc 兵隊３人影引っ込める {
	mesg 影管理 長廊下一部分 del gbs_defA
	mesg 影管理 長廊下一部分 del gbs_defB
	mesg 影管理 長廊下一部分 del gbs_defC_無線兵
}

// 2001/09/16 Uehara Masatoshi [新規追加]
proc 警戒スネークオン {
	mesg 影管理 影かんり demo_add sna_mh01  長廊下の影もでる		// スネーク
	mesg 影管理 影かんり demo_add usp_sub02 長廊下の影もでる		// ＵＳＰ
	mesg 影管理 影かんり demo_add usp_amo   長廊下の影もでる		// ＵＳＰ弾倉
}
proc 警戒スネークオフ {
	mesg 影管理 影かんり del sna_mh01
	mesg 影管理 影かんり del usp_sub02
	mesg 影管理 影かんり del usp_amo
}

proc オセロット影投影オン {
	mesg 影管理 影かんり parallel 500 -500 0
	mesg 影管理 影かんり demo_add rev_cort01  長廊下の影もでる		// スネーク
}

proc オセロット影投影オフ {
	mesg 影管理 影かんり del rev_cort01
}		


proc 廊下奥のドアセット {
	chara プットオブジェ  廊下奥のドア \
		-m demo_w00_sdr_r \
		-p 17325,-5000,(-3950-39500-57000-28500) \
		-r 0,2048,0 \
		-s 100,100,100 \
		-l d:LT2_NAME \
		-f 0x0001
}

