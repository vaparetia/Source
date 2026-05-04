/*
	polydemo_w51a.h
	    アーセナル上デモステージ

	2000/11/16 H.Yoshiike         
	$Id: polydemo_w51a.h,v 1.24 2001/09/20 04:31:11 usr04549 Exp $   
*/


// include
// ------------------------------------
#include "5sec_bfr_demo.h"
#include "p_force_cdc.ch"

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif

#if d:MGS2_DEMO
	#define DEMO_SET_STAGE 1	// 再生するステージを切り替える
								// 0 : p080_05_03, p080_07_04, p080_09_05
								// 1 : 他
#endif


/***************************************************************************************************/
// マップ設定
/***************************************************************************************************/
command マップ設定 アーセナルギア上 -set {
//	デモ環境かゲーム環境かで起動を読み分ける。
// ------------------------------------
	#if d:MGS2_DEMO
	// デモ環境デバッグ用
chara デモアクトデバッグ demodebug  -i t:p080_05_p03 -d 'p080_05_p03' -l   \
					-p ダミープロック \
					   影上げて 影下げて

		/* デモ班環境では無条件でオフ、って関係ないか？じつは */
		eval($w:武器弾数Ｒ[d:武器:ブレード] = 0);
		eval( $s:d_num = demo );

	if (d:DEMO_SET_STAGE == 0) {
		eval($s:d_num = p080_05_p03)
//		eval($s:d_num = p080_07_p04)
//		eval($s:d_num = p080_09_p05)
	}

	#else
		@ポリデモ再生

	#endif

	if ( $s:d_num != not_demo ) {
	// デモ再生時にはシネマスクリーンとデモキャンセルをセット
		@デモ開始時シネマスクリーンセット	// demo_stdproc.hで定義

		// ポリデモの時だけデモキャンセルを起動させる
		if ( $s:d_num == p080_05_p03 || $s:d_num == p080_07_p04 || $s:d_num == p080_09_p05 ){
			chara デモキャンセルチェック デモキャンセル \
				-p ポリデモキャンセル
		}

		// リアル影セット
		if (($s:d_num == p080_05_p03) || \
			($s:d_num == p080_07_p04) || \
			($s:d_num == p080_09_p05)) {
			@リアル影セット
			@ＲＡＹ影セット
		}

	}
}


// ポリデモ処理
// ------------------------------------
	proc ポリデモ再生 {
		#if d:DEBUG_PRINT
			print 'polydemo_start!!  p_story ='
			print $w:p_story
		#endif

		if( $w:p_story == d:ST:P080_05_P03ＡＧ浮上５ポリゴンデモ３開始 ) {
			/* ソリダスとフォーチュン、対立 */
			/* ここから p080_28_p12 までのデモ中では、
			ライデンは刀を持たせない。デモからゲームに戻る際に
			刀を持たせる（＝装備数を１に戻す） */
			eval($w:武器弾数Ｒ[d:武器:ブレード] = 0);

			// デモ特有の変数代入
			eval( $s:d_num = p080_05_p03 );

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p080_05_p03 \
				-p ダミープロック \
				   影上げて 影下げて \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// ポリデモ終了後ムービーデモへ

		} else if ( $w:p_story == d:ST:P080_06_M02ＡＧ浮上６ムービーデモ２開始 ) {
			/* 星条旗の映像　ＧＷについてソリダスが語る */

			// デモ特有の変数代入
			eval( $s:d_num = p080_06_m02 );

			chara MOVIE再生 p080_06_m02 -file t:p080_06_m02 -top 0, d:MOVIE_TOP_Y -size 512, 320 \
				-proc ポリデモ終了

		} else if($w:p_story == d:ST:P080_07_P04ＡＧ浮上７ポリゴンデモ４開始) {
			/* オセロット　演習（Ｓ３計画）の目的を語る */

			// デモ特有の変数代入
			eval( $s:d_num = p080_07_p04 );

			// 風制御はパラメータが生きているので設定しなおす。010913 田中(信)
			@デモ風制御
			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p080_07_p04 \
				-p ダミープロック \
				   影上げて 影下げて \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// ポリデモ終了後ムービーデモへ

		} else if ( $w:p_story == d:ST:P080_08_M03ＡＧ浮上８ムービーデモ３開始 ) {
			/* シャドーモセスと今回の類似点ムービー */

			// デモ特有の変数代入
			eval( $s:d_num = p080_08_m03 );

			chara MOVIE再生 p080_08_m03 -file t:p080_08_m03 -top 0, d:MOVIE_TOP_Y -size 512, 320 \
				-proc ポリデモ終了

		} else if ( $w:p_story == d:ST:P080_09_P05ＡＧ浮上９ポリゴンデモ５開始 ) {
			/* フォーチュン撃たれる */

			// デモ特有の変数代入
			eval( $s:d_num = p080_09_p05 );

			// 風制御はパラメータが生きているので設定しなおす。010913 田中(信)
			@デモ風制御
			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p080_09_p05 \
//				-i t:data_dummy_raiden \
				-p ダミープロック \
				   影上げて 影下げて \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// ポリデモ終了後ムービーデモへ

		} else if ( $w:p_story == d:ST:P080_10_M04ＡＧ浮上１０ムービーデモ４開始 ) {
			/* フォーチュンが何故弾に当たらなかったのかの理由説明 by オセ */

			// デモ特有の変数代入
			eval( $s:d_num = p080_10_m04 );

			chara MOVIE再生 p080_10_m04 -file t:p080_10_m04 -top 0, d:MOVIE_TOP_Y -size 512, 320 \
				-proc ポリデモ終了		// ムービー終了後ロード

 		} else {
			// このフラグが立っていると、親（ゲームの）GCLの方で必要なキャラが起動する。
			@ゲームステージへ戻る

		}

	}





	// 終了proc
	// ------------------------------------
	proc ポリデモ終了 {
		#if d:DEBUG_PRINT
			print 'polydemo_end!!  d_num ='
			print $s:d_num
		#endif

		@デモ終了時シネマスクリーンオフ

		// ポリデモの時だけでもキャンセルを起動しているので
		if ( $s:d_num == p080_05_p03 || $s:d_num == p080_07_p04 || $s:d_num == p080_09_p05 ){
			mesg デモキャンセルチェック デモキャンセル kill
		}

		if ( $s:d_num == p080_05_p03 ) {
			eval( $w:p_story = d:ST:P080_05_P03ＡＧ浮上５ポリゴンデモ３終了 );
			eval( $w:p_story = d:ST:P080_06_M02ＡＧ浮上６ムービーデモ２開始 );
			chara delay ディレイ -time 1 -exec{ restart -s }

		} else if ( $s:d_num == p080_06_m02 ) {
			eval( $w:p_story = d:ST:P080_06_M02ＡＧ浮上６ムービーデモ２終了 );
			eval( $w:p_story = d:ST:P080_07_P04ＡＧ浮上７ポリゴンデモ４開始 );
			chara delay ディレイ -time 1 -exec{ restart -s }

		} else if ( $s:d_num == p080_07_p04 ) {
			eval( $w:p_story = d:ST:P080_07_P04ＡＧ浮上７ポリゴンデモ４終了 );
			eval( $w:p_story = d:ST:P080_08_M03ＡＧ浮上８ムービーデモ３開始 );
			chara delay ディレイ -time 1 -exec{ restart -s }

		} else if ( $s:d_num == p080_08_m03 ) {
			eval( $w:p_story = d:ST:P080_08_M03ＡＧ浮上８ムービーデモ３終了 );
			eval( $w:p_story = d:ST:P080_09_P05ＡＧ浮上９ポリゴンデモ５開始 );
			chara delay ディレイ -time 1 -exec{ restart -s }

		} else if ( $s:d_num == p080_09_p05 ) {
			eval( $w:p_story = d:ST:P080_09_P05ＡＧ浮上９ポリゴンデモ５終了 );
			eval( $w:p_story = d:ST:P080_10_M04ＡＧ浮上１０ムービーデモ４開始 );
			chara delay ディレイ -time 1 -exec{ restart -s }

		} else if ( $s:d_num == p080_10_m04 ) {
			eval( $w:p_story =  d:ST:P080_10_M04ＡＧ浮上１０ムービーデモ４終了 );
			eval( $w:p_story = d:ST:P080_11_P06ＡＧ浮上１１ポリゴンデモ６開始 );
			chara delay ディレイ -time 1 -exec{ @mv_P080_11_P06_0 }

		}
	}

// 各proc設定
//-------------------------------------------------------------------------------------
// [追加] 2001/09/05 Uehara Masatoshi
proc リアル影セット {
	chara 影投影モデル アーセナル頭の影もでる \
		-m w51a0k \
		-p 0 0 0

	chara 影投影モデル アーセナル頭の低位置影もでる \
		-m w51a0k \
		-p 0 -25 0

	chara 影管理 影かんり \
		-t 0 \
		-p -3800 -3000 -1300

	mesg 影管理 影かんり demo_add "rai_def ライデン"         アーセナル頭の影もでる	// ライデン
	mesg 影管理 影かんり demo_add "sna_def スネーク"         アーセナル頭の影もでる	// スネーク
	mesg 影管理 影かんり demo_add "sol_def ソリダス"         アーセナル頭の影もでる	// ソリダス
	mesg 影管理 影かんり demo_add "sol_snakearm_mh_mt"       アーセナル頭の影もでる	// 蛇手
	mesg 影管理 影かんり demo_add "demo_kwt_uni_mount"       アーセナル頭の影もでる	// 共和刀
	mesg 影管理 影かんり demo_add "demo_mst_uni_mount"       アーセナル頭の影もでる	// 民主刀
	mesg 影管理 影かんり demo_add "for_addhand フォーチュン" アーセナル頭の影もでる	// フォーチュン
	mesg 影管理 影かんり demo_add "demo_lnr"                 アーセナル頭の影もでる	// リニアガン
	mesg 影管理 影かんり demo_add "rev_def オセロット"       アーセナル頭の影もでる	// オセロット

}
proc 影上げて {
	mesg 影管理 影かんり demo_add "rai_def ライデン" アーセナル頭の影もでる			// ライデン
}
proc 影下げて {
	mesg 影管理 影かんり demo_add "rai_def ライデン" アーセナル頭の低位置影もでる	// ライデン
}
proc ＲＡＹ影セット {
	chara プットオブジェ ＲＡＹ影 \
		-m w51a_ray_shadow \
		-r 0 0 0 \
		-p 0 0 0 \
		-s 100 100 100 \
		-l w51a
}

proc デモ風制御 {
	// 風
	chara 風制御 wind \
		-i 0 \
		-j 0 \
		-r -1024 3584 0 \
		-a 256 \
		-b 10 \
		-c 150
}

