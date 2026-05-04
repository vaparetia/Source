/*
	polydemo_w46a.h
	    メタル戦デモステージ

	2000/09/29 H.Yoshiike         
	$Id: polydemo_w46a.h,v 1.38 2002/07/31 07:40:40 usr02774 Exp $                      

	
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

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif


// マップ設定
// ------------------------------------

command マップ設定 メタル戦ステージ -set {

// デモ環境かどうか。
#if d:MGS2_DEMO

// 不要プログラムの起動阻害
	eval($s:d_num = P036_04_p03)

chara デモアクトデバッグ demodebug  -i t:p079_03_p02 -d 'p079_03_p02' -l  -s 'p079_03_p02'  \
		-p  null

#else

// ゲームＧＣＬと共用で入ってくる部分なので、== で条件を見る。
if($w:p_story <= d:ST:P079_01_P01ＲＡＹ戦前１ポリゴンデモ１開始) {
	print'd:ST:P079_01_P01ＲＡＹ戦前１ポリゴンデモ１開始'

	/* 暗闇からソリダスの声 */

		// デモ特有の変数代入
		eval($s:d_num = P079_01_p01)
		// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
		@デモではレーダー不要
		@デモモデル差し替え処理
		proc デモ再生_P079_01_p01 {
			chara demo デモ再生 \
				-i t:p079_01_p01 \
				-p null \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next P079_01_p01終了
		}

		@デモ再生_P079_01_p01

// ゲームＧＣＬと共用で入ってくる部分なので、== で条件を見る。
} else if($w:p_story == d:ST:P079_03_P02ＲＡＹ戦前３ポリゴンデモ２開始) {
	print'd:ST:P079_03_P02ＲＡＹ戦前３ポリゴンデモ２開始'

	/* 「私のジャックとして死んでくれ」 */

		// デモ特有の変数代入
		eval($s:d_num = P079_03_p02)
		// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
		@デモではレーダー不要
		@デモモデル差し替え処理
		proc デモ再生_P079_03_p02 {
			chara demo デモ再生 \
				-i t:p079_03_p02 \
				-p null \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next P079_03_p02終了
		}

		@デモ再生_P079_03_p02
#ifdef d:XBOX
		command ＸＢＯＸ用パッチ_w46a_demo
#endif


	//	ここから別ステージへ

	#if 0
// ゲームＧＣＬと共用で入ってくる部分なので、== で条件を見る。
} else if($w:p_story == d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１開始) {
	print'd:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１開始'

	/* オルガ死亡 */

		// デモ特有の変数代入
		eval($s:d_num = P080_01_p01)
		// polydemo_**.h の最初のデモの時には必ず呼ぶこと。レーダー明示的にオフさせる。
		@デモではレーダー不要

		@デモモデル差し替え処理
		proc デモ再生_P080_01_p01 {
			chara demo デモ再生 \
				-i t:p080_01_p01 \
				-p null \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next P080_01_p01終了
		}

		@デモ再生_P080_01_p01

	} else if( d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P080_02_M01ＡＧ浮上２ムービーデモ１開始) {
		print'd:ST:P080_02_M01ＡＧ浮上２ムービーデモ１開始'
		eval($w:p_story = d:ST:P080_02_M01ＡＧ浮上２ムービーデモ１開始)

		/* アーセナルに異常発生 */

		// デモ特有の変数代入
		eval($s:d_num = P080_02_m01)

		#if 1
			chara デバッグポーズ pause 'MOVIE_P080_02_m01' -p P080_02_m01終了
//			chara MOVIE再生 p080_02_m01 -file t:p080_02_m01 -top 0, 36 -size 512, 320 -proc P080_02_m01終了

		#else
			/* 無線 */
			/* P080_02_m01終了 */
		#endif

	} else if( d:ST:P080_02_M01ＡＧ浮上２ムービーデモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２開始) {
		print'd:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２開始'
		eval($w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２開始)

		/* スネークとライデン捕らえられる */

		// デモ特有の変数代入
		eval($s:d_num = P080_03_p02)

		@デモモデル差し替え処理
		proc デモ再生_P080_03_p02 {
			chara demo デモ再生 \
				-i t:p080_03_p02 \
//				-i t:data_dummy_raiden \
				-p null \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next P080_03_p02終了
		}

		@デモ再生_P080_03_p02

	//	ここまで
	#endif


	// デモの時以外は、以下を通過する。
	} else {

		// このフラグが立っていると、親（ゲームの）GCLの方で必要なキャラが起動する。
		@ゲームステージへ戻る

	}





#endif

// デモ仕様のセット
	if($s:d_num != not_demo){
		// 共通して使用するもの
		chara デモキャンセルチェック デモキャンセル \
			-p demo_cancel
		@シネマスクリーンセット

		@フォグ設定	// w46a.gcl にて定義
		@demo_空設定
		@demo_もやもや設定
		@demo_雷設定

		if ($s:d_num == P080_02_m01) {
			@量産型レイセット
		}

	}

} // < 「command マップ設定 メタル戦ステージ」の閉じカッコ


//初期マップの表示
// ゲームgclの方で実施


//###################################################################################
//									デモのみ仕様各種
//###################################################################################
// ダミープロック
proc null {}

// デモキャンセル
proc demo_cancel {
	command StreamStopAll
}

// リスタート
proc restart_proc {
	restart
}

// デモ終了時プロック
proc P079_01_p01終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill

	print '$w:p_story = d:ST:P079_01_P01ＲＡＹ戦前１ポリゴンデモ１終了'
	eval($w:p_story = d:ST:P079_01_P01ＲＡＹ戦前１ポリゴンデモ１終了)

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			//	ゲーム用へフラグを変更する
			@ゲームステージへ戻る
			if($w:アイテム == d:アイテム:ゴル兵制服){
				//	レジデントゼロへ読み替える
				load "w46a" -r 'r_plt0'
			} else {
				// ゲームへ
				restart
			}
		}
}

proc P079_03_p02終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill

	print '$w:p_story = d:ST:P079_03_P02ＲＡＹ戦前３ポリゴンデモ２終了'
	eval($w:p_story = d:ST:P079_03_P02ＲＡＹ戦前３ポリゴンデモ２終了)

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			//	ゲーム用へフラグを変更する
			@ゲームステージへ戻る
			if($w:アイテム == d:アイテム:ゴル兵制服){
				//	レジデントゼロへ読み替える
				load "w46a" -r 'r_plt0'
			} else {
				// ゲームへ
				restart -s
			}
		}
}

proc P080_01_p01終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill
	#if d:DEMO_SKIP_ENABLE
		eval($w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２終了)
		print '$w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２終了'
	#else
		print '$w:p_story = d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１終了'
		eval($w:p_story = d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１終了)
	#endif

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			restart
		}
}

proc P080_02_m01終了 {
	@デモ終了時シネマスクリーンオフ
	#if d:DEMO_SKIP_ENABLE
		eval($w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２終了)
		print '$w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２終了'
	#else
		eval($w:p_story = d:ST:P080_02_M01ＡＧ浮上２ムービーデモ１終了)
	#endif
//	restart
	@mv_w46a_d080p01_0
}

proc P080_03_p02終了 {
	@デモ終了時シネマスクリーンオフ
	mesg デモキャンセルチェック デモキャンセル kill

	print '$w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２終了'
	eval($w:p_story = d:ST:P080_03_P02ＡＧ浮上３ポリゴンデモ２終了)

	chara delay ディレイ \
		-time 1 \ 
		-exec{
			// ＡＧ上デモへ
			#if 0
				eval($w:p_story = d:ST:P080_05_P03ＡＧ浮上５ポリゴンデモ３開始)
				load 'd080p03'
			#else
				load 'w51a'
			#endif
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



//###################################################################################
//									 オブジェクト
//###################################################################################

	proc 量産型レイセット {
//		レイ配置
		chara プットオブジェ ray1 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 0,0,-400000 \
			-l d:LT2_NAME
		chara プットオブジェ ray2 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 10000,0,-380000 \
			-l d:LT2_NAME
		chara プットオブジェ ray3 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -10000,0,-390000 \
			-l d:LT2_NAME
		chara プットオブジェ ray4 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 20000,0,-360000 \
			-l d:LT2_NAME
		chara プットオブジェ ray5 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -20000,0,-370000 \
			-l d:LT2_NAME
		chara プットオブジェ ray6 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 30000,0,-340000 \
			-l d:LT2_NAME
		chara プットオブジェ ray7 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -30000,0,-350000 \
			-l d:LT2_NAME
		chara プットオブジェ ray8 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 40000,0,-320000 \
			-l d:LT2_NAME
		chara プットオブジェ ray9 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -40000,0,-330000 \
			-l d:LT2_NAME
		chara プットオブジェ ray10 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 50000,0,-300000 \
			-l d:LT2_NAME
		chara プットオブジェ ray11 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -50000,0,-310000 \
			-l d:LT2_NAME
		chara プットオブジェ ray12 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 60000,0,-280000 \
			-l d:LT2_NAME
		chara プットオブジェ ray13 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -60000,0,-290000 \
			-l d:LT2_NAME
		chara プットオブジェ ray14 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 70000,0,-260000 \
			-l d:LT2_NAME
		chara プットオブジェ ray15 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -70000,0,-270000 \
			-l d:LT2_NAME
		chara プットオブジェ ray16 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 80000,0,-240000 \
			-l d:LT2_NAME
		chara プットオブジェ ray17 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -80000,0,-250000 \
			-l d:LT2_NAME
		chara プットオブジェ ray18 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 90000,0,-220000 \
			-l d:LT2_NAME
		chara プットオブジェ ray19 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -90000,0,-230000 \
			-l d:LT2_NAME
		chara プットオブジェ ray20 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 100000,0,-200000 \
			-l d:LT2_NAME
		chara プットオブジェ ray21 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -100000,0,-210000 \
			-l d:LT2_NAME
		chara プットオブジェ ray22 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 110000,0,-180000 \
			-l d:LT2_NAME
		chara プットオブジェ ray23 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -110000,0,-190000 \
			-l d:LT2_NAME
		chara プットオブジェ ray24 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p 120000,0,-160000 \
			-l d:LT2_NAME
		chara プットオブジェ ray25 -m pdray_def_mt -r 0,0,0 -s 100,100,100 \
			-p -120000,0,-170000 \
			-l d:LT2_NAME
}



//###################################################################################
//									 エフェクト
//###################################################################################

proc demo_空設定 {
	chara 新空 cloud \
		-m w00_sky \
		-g 1 // $i:設定するとデモ中でもゲーム環境と同じにする
}

proc demo_もやもや設定 {
	chara	ＲＡＹステージ環境設定 もやもや君
}

proc demo_雷設定 {
	chara  新雷フラッシュ thunder \
		-probability 360 -thunder_disable
}

