/*
	polydemo_a31a.h
	    シェル２中央棟１Ｆ

	2002/06/11 S.Mukaide         
	$Id: polydemo_a31a.h,v 1.2 2002/10/08 01:06:54 usr01475 Exp $                      

	
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
	@ポリデモ再生
	// デモ再生時にはシネマスクリーンとデモキャンセルをセット
	@デモ開始時シネマスクリーンセット	// demo_stdproc.hで定義
	chara デモキャンセルチェック デモキャンセル \
		-p ポリデモキャンセル

	@demo_エレベーター設置

}

// ポリデモ処理
// ------------------------------------
	proc ポリデモ再生 {
		#if d:DEBUG_PRINT
			print 'polydemo_start!!  p_story ='
			print $w:p_story
		#endif

		/* 電源パネル破壊シーン */
			#if d:DEBUG_PRINT
				print '$w:p_story = ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始'
			#endif

			@デモモデル差し替え処理
			chara demo デモ再生 \
				-i t:vr_p048_01_p01 \
				-p 壊れ電源パネルセット 壊れ前電源パネルセット \
				-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
				-next ポリデモ終了		// ゲームへ
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

		eval ($b:tales_story_d = d:TALES_D:海軍大佐語り前)

		chara delay ディレイ \
			-time 1 \
			-exec {
				restart -s
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
		mesg プットオブジェ 壊れ前電源パネル off

		mesg 電源パネルライト 電源パネルライト 破棄

		chara プットオブジェ 壊れ電源パネル \
			-m aw31a_panel1 -r 0,0,0 -p 0,0,0 -s 100,100,100 -l d:LT2_NAME
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
