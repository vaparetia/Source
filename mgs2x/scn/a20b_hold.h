/*
	a20b_hold.h
		Ｅ脚ヘリポート(アナザーミッション用武装解除９面)

	2002/06/10 T.Ohtani
	$Id: a20b_hold.h,v 1.20 2002/10/02 07:47:47 usr13731 Exp $

	memo: メモリいっぱいでハイテク兵でませぬ。
*/

//------------------------------------------------------------
// 各ミッション別で使用するdefine
//------------------------------------------------------------
#define MISSION_HOLD 1

enum E_R{
	敵兵出現位置 =0,
	雷電道１,
	雷電道２,
	雷電道３,
	雷電道４,
	雷電道５,
	蛇道１,
	蛇道２,
	蛇道３,
	蛇道４,
	蛇道５,
	昔蛇１ = 11,
	昔蛇２,
	昔蛇３,
	昔蛇４,
	昔蛇５,
	昔蛇６,
	滝蛇Ｅ = 17
}


// 警備兵ステータス
#define A20B_WATCHER_STATUS		d:ENE_STATUS_NO_FINGER d:ENE_STATUS_NORMAL 0 0 0 ヘリポート


//------------------------------------------------------------
// mode.hで呼び出されるプロック
//------------------------------------------------------------
proc 武装解除ミッション用マップ定義前設定 {
	#if d:DEBUG_PRINT
		print 'proc_set1-hold_misson'
	#endif
	@全ミッション共通システム設定 d:VR_GOAL_SCN
	@武装解除ミッション９面ＶＲウィンドウ設定
	@ＶＲクリア設定 p:load_alt_hold_a31a p:A_ＲＥＴＲＹプロック

	command セットルートオフセット	d:SCN:武装解除 // a_vardef.h のenum
}

proc 武装解除ミッション用マップ定義後設定 {
	#if d:DEBUG_PRINT
		print 'proc_set2-hold_misson'
	#endif

	// 全マップ共通で使用する基本キャラの設定
	command マップ設定 ヘリポート -set {
		if( $s:選択プレイヤー == 前作スネーク ){
			@各ミッション用ゴール表示 50000 11500 -96250	//	a_stdproc.hで定義	ゴール座標Ｘ、Ｙ、Ｚの順
		}else{
			@各ミッション用ゴール表示 56500 11500 -104000	//	a_stdproc.hで定義	ゴール座標Ｘ、Ｙ、Ｚの順
		}
		@武装解除ミッション用サウンド設定	// データは最初に呼ぶ
		@武装解除ミッション用コマンダー設定
		@武装解除ミッション用変数設定		// ドアのスイッチなどの変数を設定
		@武装解除ミッション用ＢＧＭ設定		// プレイヤー→敵兵→BGMの順で呼ぶ
		@武装解除ミッション用アイテム設置
	}
}

//------------------------------------------------------------
// ＶＲウィンドウの設定と表示
//------------------------------------------------------------
proc 武装解除ミッション９面ＶＲウィンドウ設定 {
	#if d:DEBUG_PRINT
		print 'vr_window_set-hold up LEVEL9'
	#endif

	@ＶＲウィンドウ設定	[VRMSG_J:ALT_HLD_a20b] [VRMSG_E:ALT_HLD_a20b] [VRMSG_G:ALT_HLD_a20b] \	// 日、英、独
						[VRMSG_F:ALT_HLD_a20b] [VRMSG_I:ALT_HLD_a20b] [VRMSG_S:ALT_HLD_a20b] \	// 仏、伊、西
						[VRMSG_K:ALT_HLD_a20b] \												// 韓
						p:武装解除ミッション９面ステージ開始時処理								// 終了プロック


	// ＶＲウィンドウを表示する
	mesg ＶＲウィンドウ ＶＲウィンドウ君 開く

	// スタート時とクリア時に使用するカットインカメラ
	chara カメラ カットインカメラ		-c 0 -l 0 -p  0
	if( $s:選択プレイヤー == 前作スネーク ){
		// スタート時のカメラ(必要な場合)
		chara カメラ設定 カットインカメラ \
			-c 1 \
			-p 50000,27884,-95456 -t 50000,12761,-95920 \
			-r 1004,2048,0 -f 15130 \
			-a 263 \
			-i 0 0 d:START_CAMERA_CHANGE_TIME d:START_CAMERA_CHANGE_TIME \
			-s 1
	}else{
		// スタート時のカメラ(必要な場合)
		chara カメラ設定 カットインカメラ \
			-c 1 \
			-p 56500,24286,-96808 -t 56500,11500,-104000 \
			-r 690,2048,0 -f 14670 \
			-a 200 \
			-i 0 -1 0 0 \
			-s 1
	}

}



//------------------------------------------------------------
// ステージ開始時処理
//------------------------------------------------------------
proc 武装解除ミッション９面ステージ開始時処理 {
	#if d:DEBUG_PRINT
		print 'game_start!!!!!'
	#endif

	// プレイヤーやパッドの制御等を行う a_stdproc.hで定義
	@全ミッション共通ステージ開始時処理

	// カメラワークスタート(必要な場合)
	// 全体図
	if( $s:選択プレイヤー == 前作スネーク ){
		chara カメラ設定 カットインカメラ \
			-c 1 \
			-p 50000,19540,-90583 -t 50000,13090,-96250 \
			-r 555,2048,0 -f 8600 \
			-a 200 \
			-i 0 0 d:START_CAMERA_CHANGE_TIME d:START_CAMERA_CHANGE_TIME \
			-s 1
	}else{
		chara カメラ設定 カットインカメラ \
			-c 1 \
			-p 59000,19540,-75333 -t 59000,13090,-81000 \
			-r 555,2048,0 -f 8600 \
			-a 200 \
			-i 0 0 d:START_CAMERA_CHANGE_TIME d:START_CAMERA_CHANGE_TIME \
			-s 1
	}

	chara delay ディレイ \
		-time d:START_CAMERA_CHANGE_TIME \
		-exec {
			chara カメラ設定 カットインカメラ -s -1
		}
}




//------------------------------------------------------------
// クリアしたときの処理
//------------------------------------------------------------
proc 武装解除ミッション用クリア時処理 {
	#if d:DEBUG_PRINT
		print 'clear'
	#endif
	if( $s:選択プレイヤー == 前作スネーク ){
		// 強制モーションを発動
		@全ミッション共通クリアモーション発動 d:モーションリスト:non_mclr_1 2163

		// カメラを移動
		chara カメラ設定 カットインカメラ \
			-c 1 \
			-p 49495,12431,-99994 -t 49947,12514,-97459 \
			-r -21,115,0 -f 2577 \
			-a 200 \
			-i 2 2 0 0 \
			-s 1
	}else{
		// 強制モーションを発動
		@全ミッション共通クリアモーション発動 d:モーションリスト:non_mclr_1 0

		// カメラを移動
		chara カメラ設定 カットインカメラ \
			-c 1 \
			-p 57118,12387,-99573 -t 56642,12502,-102528 \
			-r -25,2152,0 -f 2995 \
			-a 200 \
			-i 2 2 0 0 \
			-s 1

	}
}



//------------------------------------------------------------
// サウンドに関する設定
//------------------------------------------------------------
proc 武装解除ミッション用サウンド設定 {
	#if d:DEBUG_PRINT
		print 'sound_set-hold_misson'
	#endif

	chara サウンドマネージャー ＳＤマネ -pak 0
	@サウンド効果音設定
}

proc 武装解除ミッション用ＢＧＭ設定 {
	#if d:DEBUG_PRINT
		print 'bgm_set-hold_misson'
	#endif

	chara ＢＧＭマネージャー ＢＧＭマネ
	chara ＢＧＭ風音 ＢＧＭ風音 -s d:BGM_WINDNOISE_WEATHER_FAIN
}



//------------------------------------------------------------
// 変数に関する設定
//------------------------------------------------------------
proc 武装解除ミッション用変数設定 {
	#if d:DEBUG_PRINT
		print 'stage_var_set-hold_misson'
	#endif

	//------------------------------------------------------------
	// ドアに関する設定(0-開かない 1-開く)
	//------------------------------------------------------------
	$$b:ドアフラグ[0] = d:DOOR_FLAG_ENEMY_ONLY;	// Ｅ脚屋内へのドア
	//------------------------------------------------------------
	// 武器／アイテムに関する設定
	//------------------------------------------------------------
	// デフォルトアイテム
	@武装解除キャラ別標準装備設定

}



//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
proc 武装解除ミッション用警備兵セット {
	#if d:DEBUG_PRINT
		print 'watcher_set-hold_misson'
	#endif
	switch( $s:選択プレイヤー ){
		case( ライデン ){
			@A_武装解除警備兵 敵兵:01 d:E_R:雷電道１ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:02 d:E_R:雷電道２ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:03 d:E_R:雷電道３ 1 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:04 d:E_R:雷電道４ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:雷電道５ 0 d:A20B_WATCHER_STATUS
		}
		case( プリスキン ){
			@A_武装解除警備兵 敵兵:01 d:E_R:雷電道１ 2 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:02 d:E_R:雷電道２ 2 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:03 d:E_R:雷電道３ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:04 d:E_R:雷電道４ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:雷電道５ 0 d:A20B_WATCHER_STATUS
		}
		case( スネーク ){
			@A_武装解除警備兵 敵兵:01 d:E_R:蛇道１ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:02 d:E_R:蛇道２ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:03 d:E_R:蛇道３ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:04 d:E_R:蛇道４ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:蛇道５ 0 d:A20B_WATCHER_STATUS
		}
		case( タキシードスネーク ){
			@A_武装解除警備兵 敵兵:01 d:E_R:蛇道１ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:02 d:E_R:蛇道２ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:03 d:E_R:蛇道３ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:04 d:E_R:蛇道４ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:蛇道５ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:06 d:E_R:滝蛇Ｅ 0 d:A20B_WATCHER_STATUS
		}
		case( 前作スネーク ){
			@A_武装解除警備兵 敵兵:01 d:E_R:昔蛇１ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:02 d:E_R:昔蛇２ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:03 d:E_R:昔蛇３ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:04 d:E_R:昔蛇４ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:昔蛇５ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:昔蛇６ 0 d:A20B_WATCHER_STATUS
		}
		default{
			// ライデン
			@A_武装解除警備兵 敵兵:01 d:E_R:雷電道１ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:02 d:E_R:雷電道２ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:03 d:E_R:雷電道３ 1 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:04 d:E_R:雷電道４ 0 d:A20B_WATCHER_STATUS
			@A_武装解除警備兵 敵兵:05 d:E_R:雷電道５ 0 d:A20B_WATCHER_STATUS
		}
	}
}

proc 武装解除ミッション用攻撃兵セット {
	#if d:DEBUG_PRINT
		print 'attacker_set-hold_misson'
	#endif
}

proc 武装解除ミッション用警備兵コマンダー {
	#if d:DEBUG_PRINT
		print 'wccomm_set-hold_misson'
	#endif

	chara 警備コマンダー wccomm -e {武装解除ミッション用警備兵セット}
}

proc 武装解除ミッション用攻撃兵コマンダー {
	#if d:DEBUG_PRINT
		print 'atcomm_set-hold_misson'
	#endif

	chara 攻撃コマンダー atcomm -e {武装解除ミッション用攻撃兵セット}
}



proc 武装解除ミッション用コマンダー設定 {
	#if d:DEBUG_PRINT
		print 'commander_set-hold_misson'
	#endif

	chara コマンダー 敵兵セット \
		-w 武装解除ミッション用警備兵コマンダー \
//		-a 武装解除ミッション用攻撃兵コマンダー \
		-d ヘリポート \
		-r 60750 0 -90250 ヘリポート \
		-c 60750 0 -90250 ヘリポート \
		-v [ene_plant_voice:03] \
//		-x [ghq_area_alert_voice:w20b] \
//		-y [ghq_caution_voice:w20b] \
		-z $i:敵標準麻酔持続 \
		-h $i:標準警戒時間 \
		-o $i:敵標準気絶持続 \
		-s (d:ENE_STAGE_GPS | d:ENE_STAGE_VR_ANOTHER) \
		-b $w:敵標準再発生数 \
		-m $w:スタートアラートモード


}



//------------------------------------------------------------
// アイテムに関する設定
//------------------------------------------------------------
proc 武装解除ミッション用アイテム取得時番号設定 $:ボックス名 {
	#if d:DEBUG_PRINT
		print 'item_no_set-hold_misson'
	#endif
}


proc 武装解除ミッション用アイテム設置 {
	#if d:DEBUG_PRINT
		print 'a20b_item_set-hold_misson'
	#endif
}
