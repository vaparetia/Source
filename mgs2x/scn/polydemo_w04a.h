/*
	polydemo_w04a.h                     
	    第１船倉デモ用ヘッダ

	1999/02/02 S.Nojiri         
	$Id: polydemo_w04a.h,v 1.13 2001/08/22 14:32:30 usr03635 Exp $                      

	
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
#include "d04a_cdc.ch"

#define CINEMA_SCREEN	1

#define SCREEN_SHOT_DEMO 0

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif


//###################################################################################
//						ストーリーフラグ & 強制無線 & ポリデモ再生
//###################################################################################

command マップ設定 船倉１ 通路１長廊下小部屋 -set {

#if d:SCREEN_SHOT_DEMO
	chara スクリーンショット screen
#endif

// デモ環境かどうか。
#if d:MGS2_DEMO

// 不要プログラムの起動阻害
	eval($s:d_num = DEMO_ENV)

// chara船倉兵、デモ時対応
	eval($w:船倉兵モード = 1)

chara デモアクトデバッグ demodebug  -i t:data_dummy2 -d 'data_dummy2' -l   \
	-p  dummy_proc

#else

if($w:t_story < d:ST_T09b1R船倉無線機デモ１開始) { 

// chara船倉兵、デモ時対応。無線も１にする必要有り。
	eval($w:船倉兵モード = 1)

	#if d:開発中です

		chara デバッグポーズ debgupause 'ST_T09b_CODEC_DEMO' -p 開発中ST_T09b終了

	#else

		@rt_強制ＣＡＬＬ_T_09b1R船倉無線機デモ１

	#endif

} else if($w:t_story < d:ST_T10a1D船倉兵士を見るポリゴンデモ開始) {

print 'テスト_000'

	// デモ特有の変数代入
	eval($s:d_num = t10a1D)
	// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
	@デモではレーダー不要

// chara船倉兵、デモ時対応
	eval($w:船倉兵モード = 1)

	chara デモキャンセルチェック デモキャンセル \
		-p demo_cancel

	@デモモデル差し替え処理
	proc デモ再生_t10a1D {
		#if d:デモ前五秒間待ち
			chara デモアクトデバッグ demodebug  -i t:t10a1D -d 't10a1D' -s 't10a1D' \
				-next t10a1D終了 \
				-p dummy_proc
		#else
			chara demo デモ再生 \
				-i t:t10a1D \
				-p dummy_proc \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next t10a1D終了
		#endif
	}

	#if d:デモ前五秒間待ち
		eval($w:t_story = d:ST_T10a1D船倉兵士を見るポリゴンデモ開始-1)
		@global_五秒待ちセット
		if($f:global_デバッグポーズ通過){
			@デモ再生_t10a1D
		}
	#else
		@デモ再生_t10a1D
	#endif

} else if($w:t_story < d:ST_T10b1R船倉無線機デモ２開始) {

// chara船倉兵、デモ時対応。無線も１にする必要有り。
	eval($w:船倉兵モード = 1)

	#if d:開発中です

		chara デバッグポーズ debgupause 'ST_T10b1R_CODEC_DEMO' -p 開発中ST_T10b1R終了

	#else

		@rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２

	#endif

// デモの時以外は、以下を通過する。
} else {

	eval($s:d_num = not_demo)

}

#endif

// デモ仕様のセット
if($s:d_num != not_demo){
	@シネマスクリーンセット
	@デモ中のムービー再生セット
}

} // < 「command マップ設定 船倉１ 通路１」 の閉じカッコ




//###################################################################################
//									デモのみ仕様各種
//###################################################################################
//---------------------
// 開発中のみ仕様
//---------------------
proc 開発中ST_T09b終了 {
	eval($w:t_story = d:ST_T09b1R船倉無線機デモ１終了)
	restart -s
}

proc 開発中ST_T10b1R終了 {
	eval($w:t_story = d:ST_T10b1R船倉無線機デモ２終了)

	// デモが終わったので、0に戻す（レーダー表示のため）
	eval($f:global_polygon_demo = 0)
	restart -s
}


//---------------------
// 製品版用仕様
//---------------------
// ダミープロック
proc dummy_proc {}

// デモキャンセル
proc demo_cancel {
	command StreamStopAll
}

// デモ終了時プロック
proc t10a1D終了 {
print 't10a1D終了_000'
	mesg デモキャンセルチェック デモキャンセル kill
	eval($w:t_story = d:ST_T10a1D船倉兵士を見るポリゴンデモ終了)
	print '$w:t_story = d:ST_T10a1D船倉兵士を見るポリゴンデモ終了'

// chara船倉兵、デモ時対応終了
// 無線の方で１にしないと落ちるので、０に戻すのは
// rt_強制ＣＡＬＬ_10b1R船倉無線機デモ２終了　の中で実施。
//	eval($w:船倉兵モード = 0)

print 't10a1D終了_001'
	chara delay ディレイ \
		-time 1 \ 
		-exec{
print 't10a1D終了_002'
			restart
		}
}

// シネマスクリーン
proc シネマスクリーンセット {
	#if d:CINEMA_SCREEN
		chara シネマスクリーン シネマ \
			-top d:CINEMA_BELT_UPPER \
			-bottom d:CINEMA_BELT_LOWER \
			-ftime 60	//デフォルト値（ゼロは駄目）
		mesg シネマスクリーン シネマ フェードアウト 1
	#endif
}

//デモのとき
if($s:d_num != not_demo) {

	// 船倉巡回兵のパラメータ
	//<船倉兵:a0> vrs->3
	eval($i:w04a_sola0_x = -7000)
	eval($i:w04a_sola0_y = -17000)
	eval($i:w04a_sola0_z = -17500)
	//<船倉兵:a1> vrs->3
	eval($i:w04a_sola1_x = -7000)
	eval($i:w04a_sola1_y = -17000)
	eval($i:w04a_sola1_z = -17500)
	//<船倉兵:a2> vrs->5
	eval($i:w04a_sola2_x = -8000)
	eval($i:w04a_sola2_y = -17000)
	eval($i:w04a_sola2_z = -6000)
	//<船倉兵:a3> vrs->1
	eval($i:w04a_sola3_x = -7000)
	eval($i:w04a_sola3_y = -17000)
	eval($i:w04a_sola3_z = -21250)
	//<船倉兵:a4> vrs->7
	eval($i:w04a_sola4_x = 11000)
	eval($i:w04a_sola4_y = -9000)
	eval($i:w04a_sola4_z = -18750)

	command マップ設定 船倉１ 通路１ 長廊下小部屋 -set {
		/* サウンドマネージャー＞敵兵コマンダー＞ＢＧＭマネージャー
		   の順番は、遵守すること */
	print '順番順番_000'
		@エフェクト設定
	print '順番順番_001'
		@ぼかしセット
	print '順番順番_002'
		@ドア設置／船倉１−通路１
	print '順番順番_003'
		@船倉１箱設置
	print '順番順番_004'
	}

	command マップ設定 船倉１ -set {
	print '順番順番_005'
		@プロジェクタライト設定
	print '順番順番_006'
		@船倉１水密ドア設置
	print '順番順番_007'
//		@長廊下壊れライト設定
	print '順番順番_008'
	
		#if d:SEALS_1_FG
	print '順番順番_009'
			@デモ用船倉１海兵隊
	print '順番順番_010'
		#endif
	}

	//初期マップの表示
	command マップ表示 	-add 船倉１

}

//###################################################################################
//									デモ用海兵隊設定
//###################################################################################
	proc デモ用船倉１海兵隊セット {
		chara コマンダー 敵兵セット -w 	デモ用船倉１海兵隊 \
			-v [ene_tank_voice:01]

	}

	// 設置オブジェクト
	// ------------------------------------
	proc デモ用船倉１海兵隊 {

		command カメラチェックキャラワーク確保

		if($w:ゲーム設定 <= d:LEVEL_NORMAL){

			chara 船倉兵 SEALS_1 -a { \
				#include "w04a_seals_matrix.h"		//船倉兵の配列（共通）
				船倉兵:a0,{$i:w04a_sola0_x,$i:w04a_sola0_y,$i:w04a_sola0_z},{0,2048,0 },11,	101, 0, 0x0014 \
				船倉兵:a4,{$i:w04a_sola4_x,$i:w04a_sola4_y,$i:w04a_sola4_z},{0,2048,0 },11,	101, 0, 0x0014 \

				// デモ時はパンツ兵でなくて普通兵
				船倉兵:14,{( 2250+10*2),-17000,(-14500+08*2)},{0,2048,0 },	10,	100, 2, 0x0020 \

			} \
			-t 100000,100000 \
			-s 4000,-10000,-22000 \
			-p 140,-15600,-1300 -104,1894,0 256 \
			-m $w:船倉兵モード

		}else if($w:ゲーム設定 == d:LEVEL_HARD){

			chara 船倉兵 SEALS_1 -a { \
				#include "w04a_seals_matrix.h"		//船倉兵の配列（共通）
				船倉兵:a1,{$i:w04a_sola1_x,$i:w04a_sola1_y,$i:w04a_sola1_z},{0,2048,0 },11,	101, 0, 0x0014 \
				船倉兵:a2,{$i:w04a_sola2_x,$i:w04a_sola2_y,$i:w04a_sola2_z},{0,2048,0 },11,	101, 0, 0x0014 \
				船倉兵:a4,{$i:w04a_sola4_x,$i:w04a_sola4_y,$i:w04a_sola4_z},{0,2048,0 },11,	101, 0, 0x0014 \

				// デモ時はパンツ兵でなくて普通兵
				船倉兵:14,{( 2250+10*2),-17000,(-14500+08*2)},{0,2048,0 },	10,	100, 2, 0x0020 \

			} \
			-t 100000,100000 \
			-s 4000,-10000,-22000 \
			-p 140,-15600,-1300 -104,1894,0 256 \
			-m $w:船倉兵モード


		}else{

			chara 船倉兵 SEALS_1 -a { \
				#include "w04a_seals_matrix.h"		//船倉兵の配列（共通）
				船倉兵:a3,{$i:w04a_sola3_x,$i:w04a_sola3_y,$i:w04a_sola3_z},{0,2048,0 },11,	101, 0, 0x0014 \
				船倉兵:a4,{$i:w04a_sola4_x,$i:w04a_sola4_y,$i:w04a_sola4_z},{0,2048,0 },11,	101, 0, 0x0014 \

				// デモ時はパンツ兵でなくて普通兵
				船倉兵:14,{( 2250+10*2),-17000,(-14500+08*2)},{0,2048,0 },	10,	100, 2, 0x0020 \

			} \
			-t 100000,100000 \
			-s 4000,-10000,-22000 \
			-p 140,-15600,-1300 -104,1894,0 256 \
			-m $w:船倉兵モード

		}

	}

// プロジェクタへのムービー再生用に必要
proc デモ中のムービー再生セット {
	chara スポットライト背景イメージ指定 ムービー
}
