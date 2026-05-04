/*
	paddemo.h                     
	    パッドデモ関係をまとめたプロック(ロード時のものはloadproc.h)

	2000/10/20 H.Yoshiike         
	$Id: paddemo.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $                      

	
*/

// ファイルが二重呼びされたときの対処
#ifndef d:PADDEMO_H
#define PADDEMO_H	1

// キャンセルなしバージョン
// 終了時姿勢 ： ０／１／２／３ ： 立ち／しゃがみ／匍匐／イントルード
proc パッドデモ開始 $:デモファイル名  $:表示フラグ {
	#if d:DEBUG_PRINT 
		print 'pad_demo_start'
	#endif

	@シナリオデモ開始処理（ダンボールも解除）

	// パッドデモ再生フラグ
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_PLAY );

	chara パッドデモ パッドデモ \
		-file $:デモファイル名 \
		-end_proc パッドデモ終了

	if (($b:表示フラグ & d:MENU_ON) == 1) {
		command メニュー設定 -menu on
	}

	if (($b:表示フラグ & d:GAGE_ON) == 1) {
		command メニュー設定 -gage on
	}

	if (($b:表示フラグ & d:RADAR_ON) == 1) {
		command メニュー設定 -radar on
	}

	if (($b:表示フラグ & d:SUBWIN_ON) == 1) {
		command メニュー設定 -subwin on
	}

	mesg パッドデモ パッドデモ start
}

proc パッドデモ終了 {
	#if d:DEBUG_PRINT 
		print 'pad_demo_end'
	#endif

	@シナリオデモ終了処理
	mesg シネマスクリーン シネマ フェードイン 0
	command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_END );
}



// キャンセルありバージョン
// 終了時姿勢 ： ０／１／２／３ ： 立ち／しゃがみ／匍匐／イントルード
proc パッドデモ開始（キャンセルあり） $:デモファイル名  $:表示フラグ $:終了時位置Ｘ $:終了時位置Ｙ $:終了時位置Ｚ $:終了時方向 $:終了時姿勢 {
	#if d:DEBUG_PRINT 
		print 'pad_demo_start'
	#endif

	@シナリオデモ開始処理（ダンボールも解除）

	// パッドデモ再生フラグ
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_PLAY );

	chara デモキャンセルチェック デモキャンセル \
		-p パッドデモキャンセル

	chara パッドデモ パッドデモ \
		-file $:デモファイル名 \
		-end_proc パッドデモ終了（キャンセルあり）

	if (($b:表示フラグ & d:MENU_ON) == 1) {
		command メニュー設定 -menu on
	}

	if (($b:表示フラグ & d:GAGE_ON) == 1) {
		command メニュー設定 -gage on
	}

	if (($b:表示フラグ & d:RADAR_ON) == 1) {
		command メニュー設定 -radar on
	}

	if (($b:表示フラグ & d:SUBWIN_ON) == 1) {
		command メニュー設定 -subwin on
	}

	// 終了時位置を渡すために変数に格納
	eval($i:終了時位置Ｘ = $:終了時位置Ｘ);
	eval($i:終了時位置Ｙ = $:終了時位置Ｙ);
	eval($i:終了時位置Ｚ = $:終了時位置Ｚ);
	eval($i:終了時方向 = $:終了時方向);
	eval($b:終了時姿勢 = $:終了時姿勢);

	mesg パッドデモ パッドデモ start
}

proc パッドデモキャンセル {
	#if d:DEBUG_PRINT 
		print 'pad_demo_cancel'
	#endif

	mesg コマンダー 敵兵セット 視界オフ
	
	chara フェードインアウト フェードアウト \
		-c 0,0,0 128 \
		-t d:FADEOUT_TIME \
		-p パッドデモキャンセル後
}

proc パッドデモキャンセル後 {
	#if d:DEBUG_PRINT 
		print 'pad_demo_cancel_end'
	#endif

	mesg パッドデモ パッドデモ kill
	mesg デモキャンセルチェック デモキャンセル kill
	mesg シネマスクリーン シネマ フェードイン 0

	chara delay 強制移動ディレイ \
		-time 1 \
		-exec {
			mesg プレイヤー d:PLAYER position $i:終了時位置Ｘ $i:終了時位置Ｙ $i:終了時位置Ｚ
			mesg プレイヤー d:PLAYER stance $b:終了時姿勢 $i:終了時方向 $i:終了時方向 0
			// 主観系装備などは解除
			@特殊装備解除
			@ダンボール解除

			chara フェードインアウト フェードイン \
				-i 0,0,0 128 \
				-c 0,0,0 0 \
				-t d:FADEIN_TIME \
				-p パッドデモキャンセル後スタート
		}
}

proc パッドデモキャンセル後スタート {
	#if d:DEBUG_PRINT 
		print 'restart'
	#endif

	@シナリオデモ終了処理
	command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_END );
}


proc パッドデモ終了（キャンセルあり） {
	#if d:DEBUG_PRINT 
		print 'pad_demo_end'
	#endif

	@シナリオデモ終了処理
	mesg デモキャンセルチェック デモキャンセル kill
	mesg シネマスクリーン シネマ フェードイン 0
	command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_END );
}



//野尻が使用
proc デモ発動時デジカメはずし{

	if( $w:アイテム == d:アイテム:双眼鏡 || $w:アイテム == d:アイテム:デジカメ){
		eval( $w:デモ発動時アイテム = $w:アイテム )
		eval( $w:アイテム = d:アイテム:素手 )
	}

}


proc デモ終了時デジカメ復帰{
	if( $w:デモ発動時アイテム == d:アイテム:双眼鏡 || $w:アイテム == d:アイテム:デジカメ){
		eval( $w:アイテム = $w:デモ発動時アイテム )
	}

	eval( $w:デモ発動時アイテム = 0 )

}

#else

print 'paddemo.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


