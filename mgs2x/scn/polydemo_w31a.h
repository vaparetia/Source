/*
	polydemo_w31a.h
	    シェル２中央棟１Ｆ

	2000/11/16 H.Yoshiike         
	$Id: polydemo_w31a.h,v 1.30 2001/10/02 06:14:10 usr03635 Exp $                      

	
*/

// include
// ------------------------------------
#include "5sec_bfr_demo.h"
#include "demo_vardef.h"
#include "demo_stdch.h"
#include "demo_stdch2.h"
#include "story.h"
#include "sload.h"

#if d:USB_KEY
	#include "usbkbd.h"
#endif

#if d:PLANT_DEMO_ALL_SKIP
	#define		DEMO_SKIP_ENABLE 1
#else
	#define		DEMO_SKIP_ENABLE 0
#endif

// マップ設定
// ------------------------------------
command マップ設定 シェル２中央棟１Ｆ -set {
	//	デモ環境かゲーム環境かで起動を読み分ける。
	// ------------------------------------
	#if d:MGS2_DEMO
		// ドアをつけるときにはこの下のevalをコメントアウトする
		eval( $w:p_story = d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始 );
chara デモアクトデバッグ demodebug  -i t:p048_01_p01 -d 'p048_01_p01' -l   \
				-p 壊れ電源パネルセット 壊れ前電源パネルセット

		eval( $s:d_num = p048_01_p01 );
	#else
		@ポリデモ再生
	#endif

	if ( $s:d_num != not_demo ) {
		// デモ再生時にはシネマスクリーンとデモキャンセルをセット
		@デモ開始時シネマスクリーンセット	// demo_stdproc.hで定義
		chara デモキャンセルチェック デモキャンセル \
			-p ポリデモキャンセル

		// 大統領うろうろ
		if ( ($s:d_num == P047_03_p02) || ($s:d_num == P047_01_p01) || \
			($s:d_num == P048_01_p01) ) {
			// 大統領のゲージの表示を消す
			command メニュー設定 -gage off //ゲージのオンオフ
			@大統領設定
		}

		// 電源パネルのセット。破壊デモではセットせず。
		if ( $w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始 ){
			@壊れ前電源パネルセット
		} else if ( $w:p_story >= d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 ) {
			@壊れ電源パネルセット
		}

		@demo_エレベーター設置

		if ($s:d_num == p049_11_p02) {
			@リアル影セット
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

		
		if( $w:p_story == d:ST:P047_01_P01電撃床前オルガ１ポリゴンデモ１開始 ) {
		/* オルガが無線で話しているのを盗み聞き */
			#if d:DEBUG_PRINT
				print '$w:p_story = ST:P047_01_P01電撃床前オルガ１ポリゴンデモ１開始'
			#endif

			// デモ特有の変数代入
			eval( $s:d_num = p047_01_p01 );

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p047_01_p01 \
				-p ダミープロック \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// 主観デモへ

		} else if ( $w:p_story == d:ST:P047_03_P02電撃床前オルガ３ポリゴンデモ２開始 ) {
		/* ソリダスとオルガの会話　電撃床のスイッチが入ったことが示される */
			#if d:DEBUG_PRINT
				print '$w:p_story = ST:P047_03_P02電撃床前オルガ３ポリゴンデモ２開始'
			#endif

			// デモ特有の変数代入
			eval( $s:d_num = p047_03_p02 );

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p047_03_p02 \
//				-i t:data_dummy_raiden \
				-p ダミープロック \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// 主観デモ後無線→ゲームへ


		} else if ( $w:p_story == d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始 ) {
		/* 電源パネル破壊シーン */
			#if d:DEBUG_PRINT
				print '$w:p_story = ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始'
			#endif

			// デモ特有の変数代入
			eval( $s:d_num = p048_01_p01 );

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p048_01_p01 \
//				-i t:data_dummy_raiden \
				-p 壊れ電源パネルセット 壊れ前電源パネルセット \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// ゲームへ

		} else if ( $w:p_story == d:ST:P049_01_P01大統領１ポリゴンデモ１開始 ) {
		/* 大統領を発見　握られる */
			#if d:DEBUG_PRINT
				print '$w:p_story = ST:P049_01_P01大統領１ポリゴンデモ１開始'
			#endif

			// デモ特有の変数代入
			eval( $s:d_num = p049_01_p01 );

			// サウンドの設定
			chara サウンドマネージャー ＳＤマネ -pak -1
			mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p049_01_p01 \
				-p ダミープロック \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next rt_P049_02_R01大統領２無線機デモ１		// 無線後ムービーへ

		} else if ( $w:p_story == d:ST:P049_11_P02大統領１１ポリゴンデモ２開始 ) {
		/* 大統領からカード４とＭＯを受け取る */
			#if d:DEBUG_PRINT
				print '$w:p_story = ST:P049_11_P02大統領１１ポリゴンデモ２開始'
			#endif

			// デモ特有の変数代入
			eval( $s:d_num = p049_11_p02 );

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:p049_11_p02 \
				-p ダミープロック \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了							// ゲームへ
	
		// デモの時以外は、以下を通過する。
		} else {
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
		mesg デモキャンセルチェック デモキャンセル kill

		if ( $s:d_num == p047_01_p01 ) {
			eval( $w:p_story = d:ST:P047_01_P01電撃床前オルガ１ポリゴンデモ１終了 );
			eval( $w:p_story = d:ST:P047_02_I01電撃床前オルガ２インタラクティブ主観デモ１開始 );

		} else if ( $s:d_num == p047_03_p02 ) {
			eval( $w:p_story = d:ST:P047_03_P02電撃床前オルガ３ポリゴンデモ２終了 );
			eval( $w:p_story = d:ST:P047_04_I02電撃床前オルガ４インタラクティブ主観デモ２開始 );

		} else if ( $s:d_num == p048_01_p01 ) {
			eval( $w:p_story = d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１終了 );

		} else if ( $s:d_num == p049_11_p02 ) {
			eval( $w:p_story = d:ST:P049_11_P02大統領１１ポリゴンデモ２終了 );
		}

		@ゲームステージへ戻る
		if ( $w:アイテム == d:アイテム:ゴル兵制服 ) {
		// ゴル服を着ていたら常駐を読み直してゲームに戻す
			chara delay ディレイ -time 1 -exec{ load "w31a" -r 'r_plt0' }
		} else {
			chara delay ディレイ -time 1 -exec{ restart -s }
		}
	}


// 各proc設定
//-------------------------------------------------------------------------------------
// 各マップ別設定
// ------------------------------------
// シェル２中央棟１Ｆ
	// エレベーター設置
	proc demo_エレベーター設置 {
		chara プットオブジェ エレベーター \
			-m elv_2 -r 0,0,0 -p 0,0,-180000 -s 100,100,100 -l d:LT2_NAME
	}

	// 電源パネル
	proc 壊れ前電源パネルセット {
		chara 電源パネルライト 電源パネルライト

		chara プットオブジェ 壊れ前電源パネル \
			-m aw31a_panel0 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
	}

	proc 壊れ電源パネルセット {
		#if d:MGS2_DEMO
			mesg プットオブジェ 壊れ前電源パネル off
		#else
			if ( $w:p_story == d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始 ) {
				mesg プットオブジェ 壊れ前電源パネル off
			}
		#endif

		mesg 電源パネルライト 電源パネルライト 破棄

		chara プットオブジェ 壊れ電源パネル \
			-m aw31a_panel1 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
	}

proc リアル影セット {
	chara 影投影モデル 影もでる \
		-m w31ak \
		-p 0 0 0

	chara 影管理 影かんり \
		-t 0 \
		-p -100 -1000 100

	mesg 影管理 影かんり demo_add rai_def_addhand_mh_demo	// ライデン
	mesg 影管理 影かんり demo_add jam_def_addhand_mh_mt		// じょんそん大統領
}


// FIXデータはここまで
// -----------------------------------------------------------------------------------





// 以下仮キャラ用proc
// -----------------------------------------------------------------------------------
// 仮キャラ
// ------------------------------------







// 以下デバッグ専用proc
// -----------------------------------------------------------------------------------
// デバッグキャラ
// ------------------------------------
