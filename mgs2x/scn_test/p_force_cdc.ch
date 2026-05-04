/**************************************************
		プラント編強制無線
		//	2001/05/16	T.Fukushima
		p_force_cdc.ch
	$Id: p_force_cdc.ch,v 1.1 2002/02/01 06:08:32 usr01475 Exp $
**************************************************/
#ifndef P_FORCE_INCLUDE
#define P_FORCE_INCLUDE  1

extern command セットサウンドコード
extern command varsave

proc まっしろ退散エセフォグセット {
	chara フォグ fog \
		-c 0 0 0 \
		-n 0 \
		-f 0
}

// ------------------------------------------------
proc rt_P001_02_R01オープニング潜入２無線デモ１ {
//	eval($f:rfp_タンカーやった = 1)	//ＸＸＸＸＸ
	chara サウンドマネージャー ＳＤマネ -pak 0
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P001_02_R01_オープニング潜入２無線デモ１ \
//		t:RPD_P001_02_R01_オープニング潜入２無線デモ１ \
		d:CODEC_DIRECT rt_P001_02_R01オープニング潜入２無線デモ１終了 \
	-f ( d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}


proc rt_P001_02_R01オープニング潜入２無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P001_02_R01オープニング潜入２無線デモ１終了 );
	restart // added by S.Hirano 010612
}
//@rt_P001_02_R01オープニング潜入２無線デモ１

// ------------------------------------------------
proc rt_P001_04_R02オープニング潜入４無線デモ２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P001_04_R02_オープニング潜入４無線デモ２ \
		d:CODEC_DIRECT rt_P001_04_R02オープニング潜入４無線デモ２終了 \
	-f ( d:CODEC_I_FD_O_FA | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P001_04_R02オープニング潜入４無線デモ２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P001_04_R02オープニング潜入４無線デモ２終了 );
	// ゲームへ
	@ゲームステージへ戻る
	@mv_init_w11a0_0
}
//@rt_P001_04_R02オープニング潜入４無線デモ２

// ------------------------------------------------
proc	rt_P002_02_R01Ａ脚底部連結ハッチ２無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P002_02_R01_Ａ脚底部連結ハッチ２無線デモ１ \
		d:CODEC_DIRECT rt_P002_02_R01Ａ脚底部連結ハッチ２無線デモ１終了 \
	-f ( d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}

proc rt_P002_02_R01Ａ脚底部連結ハッチ２無線デモ１終了 {
	eval( $w:p_story = d:ST:P002_02_R01Ａ脚底部連結ハッチ２無線デモ１終了 );
#ifdef d:STAGE_W11A
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME \
		-p Ａ脚底部連結ハッチ２無線デモ１後スタート
#endif
}
// ------------------------------------------------
proc	rt_P003_02_R02スネーク昇降機上昇２無線デモ１ {
	//eval($f:rfp_タンカーやった = 1)	//ＸＸＸＸＸＸＸＸＸ
	chara サウンドマネージャー ＳＤマネ -pak -1
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P003_02_R02_スネーク昇降機上昇２無線デモ１ \
		d:CODEC_DIRECT rt_P003_02_R02スネーク昇降機上昇２無線デモ１終了 \
	-f ( d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}
proc rt_P003_02_R02スネーク昇降機上昇２無線デモ１終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P003_02_R02スネーク昇降機上昇２無線デモ１終了 );
	// ゲームへ
	@ゲームステージへ戻る
	@mv_init_w11a0_0
}

// ------------------------------------------------
proc	rt_P004_02_R01ノード初接続２無線デモ１ {
//	eval($f:rfp_タンカーやった = 1)	//ＸＸＸＸＸＸＸＸＸ
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P004_02_R01_ノード初接続２無線デモ１ \
		d:CODEC_DEMO rt_P004_02_R01ノード初接続２無線デモ１終了 \
		-f ( d:CODEC_I_AC_O_FA | d:CODEC_FDOUT_BREAK)
}

proc rt_P004_02_R01ノード初接続２無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P004_02_R01ノード初接続２無線デモ１終了 );
	restart -s
}

// ------------------------------------------------
proc	rt_P005_03_R01ライデン昇降機上昇３無線デモ１ {
	command セットサウンドコード -c d:SNG_PLAY_06		//
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P005_03_R01ライデン昇降機上昇３無線デモ１ \
		d:CODEC_DIRECT rt_P005_03_R01ライデン昇降機上昇３無線デモ１終了 \
		-f ( d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
		-d
}
proc rt_P005_03_R01ライデン昇降機上昇３無線デモ１終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S		//
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P005_03_R01ライデン昇降機上昇３無線デモ１終了 );
	// p005_04_p03のデモが実行するステージが変更になったので、修正。田中(信) 010810
	// restart
	// p005_04_p03のデモの実行するステージが更に変更になったので、修正。田中(信) 010826
	load "d005p03"
}
// ------------------------------------------------
proc	rt_P005_05_R02ライデン昇降機上昇５無線デモ２ {
	command セットサウンドコード -c d:SNG_PLAY_06		//
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P005_05_R02_ライデン昇降機上昇５無線デモ２ \
		d:CODEC_DIRECT rt_P005_05_R02ライデン昇降機上昇５無線デモ２終了 \
		-f ( d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
		-d
}

proc rt_P005_05_R02ライデン昇降機上昇５無線デモ２終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S		//
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P005_05_R02ライデン昇降機上昇５無線デモ２終了 );
	// ゲームへ
	@ゲームステージへ戻る
	@mv_w11a2_w12a_0
}
// ------------------------------------------------
proc	rt_P006_01_R01Ａ脚金網１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P006_01_R01_Ａ脚金網１無線デモ１ \
		d:CODEC_DEMO rt_P006_01_R01Ａ脚金網１無線デモ１終了
}
proc rt_P006_01_R01Ａ脚金網１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P006_01_R01Ａ脚金網１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P007_01_R01Ａ脚ノード前１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P007_01_R01_Ａ脚ノード前１無線デモ１＿１ \
		d:CODEC_DIRECT rt_P007_01_R01Ａ脚ノード前１無線デモ１終了 \
		-f (	d:CODEC_I_AC_O_FA | d:CODEC_FDOUT_BREAK )
}
proc rt_P007_01_R01Ａ脚ノード前１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了 );

	command varsave
	@rp_w12b_いろいろＣＡＬＬに監視すれば？
	@w12b_Ａ脚ノード前１無線デモ１後処理
}
// ------------------------------------------------
proc	rt_P008_01_R01ＡＢ連絡橋１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P008_01_R01_ＡＢ連絡橋１無線デモ１＿１ \
		d:CODEC_DIRECT rt_P008_01_R01ＡＢ連絡橋１無線デモ１終了 \
		-f (	d:CODEC_I_AC_O_FA | d:CODEC_FDOUT_BREAK )
}
proc rt_P008_01_R01ＡＢ連絡橋１無線デモ１終了 {
	command パッド操作 -cancel
	//以下はw13aで処理
	@ＡＢ連絡橋１無線デモ１終了処理
//	restart -s
}

proc	rt_P009_01_R01_サイファー遭遇１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P009_01_R01_サイファー遭遇１無線デモ１＿１ \
		d:CODEC_DEMO rt_P009_01_R01_サイファー遭遇１無線デモ１終了 \
		-f (	d:CODEC_I_AC_O_FA | d:CODEC_FDOUT_BREAK )

}
proc rt_P009_01_R01_サイファー遭遇１無線デモ１終了 {
	command パッド操作 -cancel
//	eval( $w:p_story = d:ST:P010_04_R01ヴァンプ遭遇４無線デモ１終了 );
	//以下はw15aまたはw23aで処理
	@サイファー遭遇１無線デモ１終了処理
}

// ------------------------------------------------
proc	rt_P010_04_R01ヴァンプ遭遇４無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P010_04_R01_ヴァンプ遭遇４無線デモ１＿１ \
		d:CODEC_DIRECT rt_P010_04_R01ヴァンプ遭遇４無線デモ１終了 \
	-f ( 	d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P010_04_R01ヴァンプ遭遇４無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P010_04_R01ヴァンプ遭遇４無線デモ１終了 );
	restart
}
// ------------------------------------------------
proc	rt_P011_01_R01ＢＣ連絡橋ＣＡＬＬ１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P011_01_R01_ＢＣ連絡橋ＣＡＬＬ１無線デモ１ \
		d:CODEC_DEMO rt_P011_01_R01ＢＣ連絡橋ＣＡＬＬ１無線デモ１終了
}
proc rt_P011_01_R01ＢＣ連絡橋ＣＡＬＬ１無線デモ１終了 {
	command パッド操作 -cancel
}

// ------------------------------------------------
proc	rt_P012_02_R01_フォーチュン遭遇２無線デモ１ {
//	command セットサウンドコード -c d:SNG_PLAY_06
	command パッド操作 -release
	@まっしろ退散エセフォグセット
	command 無線設定 \
	-call 14085 \
		t:RPD_P012_02_R01_フォーチュン遭遇２無線デモ１ \
		d:CODEC_DIRECT rt_P012_02_R01フォーチュン遭遇２無線デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P012_02_R01フォーチュン遭遇２無線デモ１終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P012_02_R01フォーチュン遭遇２無線デモ１終了 );
	// ゲームへ
	@ゲームステージへ戻る
	@mv_w14a_w15a_0
}
// ------------------------------------------------
proc	rt_P013_01_R01_ライデン危機無線１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P013_01_R01_ライデン危機無線１無線デモ１ \
		d:CODEC_DEMO rt_P013_01_R01ライデン危機無線１無線デモ１終了
}
proc rt_P013_01_R01ライデン危機無線１無線デモ１終了 {
	command パッド操作 -cancel
//	eval( $w:p_story = d:ST:P013_01_R01ライデン危機無線１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P014_05_R01ピーター遭遇５無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P014_05_R01_ピーター遭遇５無線デモ１ \
		d:CODEC_DEMO rt_P014_05_R01ピーター遭遇５無線デモ１終了 \
	-f ( 	d:CODEC_I_FA_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
				d:CODEC_NO_ALARM | d:CODEC_NO_ICON )
}
proc rt_P014_05_R01ピーター遭遇５無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P014_05_R01ピーター遭遇５無線デモ１終了 );
	// 次のw16aに入ったらすぐ主観デモが始まるので、もう一つ進める。
	eval( $w:p_story = d:ST:P014_06_I01ピーター遭遇６インタラクティブ主観デモ１開始 );
	// ゲームへ
	@ゲームステージへ戻る
	@mv_demo1_w16a_0
}
// ------------------------------------------------
proc	rt_P015_01_R1爆弾解体最初の一つ１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14025 \
		t:RPD_P015_01_R1_爆弾解体最初の一つ１無線デモ１ \
		d:CODEC_DIRECT rt_P015_01_R1爆弾解体最初の一つ１無線デモ１終了
}
proc rt_P015_01_R1爆弾解体最初の一つ１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P015_01_R1爆弾解体最初の一つ１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P016_01_R1爆弾解体半分解体１無線デモ１ {
	command パッド操作 -release
	eval($b:rfp_半分爆弾って何個なのだりう = $b:Ｃ４爆弾処理数)
	command 無線設定 \
	-call 14025 \
		t:RPD_P016_01_R1_爆弾解体半分解体１無線デモ１ \
		d:CODEC_DIRECT rt_P016_01_R1爆弾解体半分解体１無線デモ１終了
}
proc rt_P016_01_R1爆弾解体半分解体１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P016_01_R1爆弾解体半分解体１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P017_01_R01爆弾解体中ローズその１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P017_01_R01_爆弾解体中ローズその１無線デモ１ \
		d:CODEC_DEMO rt_P017_01_R01爆弾解体中ローズその１無線デモ１終了
}
proc rt_P017_01_R01爆弾解体中ローズその１無線デモ１終了 {
	command パッド操作 -cancel
// ストーリーフラグ廃止につきコメントアウト。by S.Hirano 010505
//	eval( $w:p_story = d:ST:P017_01_R01爆弾解体中ローズその１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P018_01_R01爆弾解体中ローズその２無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P018_01_R01_爆弾解体中ローズその２無線デモ１ \
		d:CODEC_DEMO rt_P018_01_R01爆弾解体中ローズその２無線デモ１終了
}
proc rt_P018_01_R01爆弾解体中ローズその２無線デモ１終了 {
	command パッド操作 -cancel
// ストーリーフラグ廃止につきコメントアウト。by S.Hirano 010505
//	eval( $w:p_story = d:ST:P018_01_R01爆弾解体中ローズその２無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P019_01_R01爆弾解体中ローズその３無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P019_01_R01_爆弾解体中ローズその３無線デモ１ \
		d:CODEC_DEMO rt_P019_01_R01爆弾解体中ローズその３無線デモ１終了
}
proc rt_P019_01_R01爆弾解体中ローズその３無線デモ１終了 {
	command パッド操作 -cancel
// ストーリーフラグ廃止につきコメントアウト。by S.Hirano 010505
//	eval( $w:p_story = d:ST:P019_01_R01爆弾解体中ローズその３無線デモ１終了 );
}

/*
proc	rt_P020_01_R01_地雷原１無線デモ１
//転居のお知らせ。w21_cdc.chへ引っ越しました。
//お近くにいらした時は是非お立ち寄りください。

*/


proc	rt_P021_02_R01_爆弾解体オルガ登場２無線デモ１ {
	command セットサウンドコード -c d:SNG_PLAY_06
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P021_02_R01_爆弾解体オルガ登場２無線デモ１ \
		d:CODEC_DIRECT rt_P021_02_R01_爆弾解体オルガ登場２無線デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d

}
proc rt_P021_02_R01_爆弾解体オルガ登場２無線デモ１終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S
	command パッド操作 -cancel
//	eval( $w:p_story = d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 );
	eval($f:d021p01_爆弾解体オルガ登場２無線デモ１開始 = 1)
	// ゲームへ
	@ゲームステージへ戻る
	@mv_demo1_w20b_0
}

// ------------------------------------------------
proc	rt_P022_01_R01爆弾解体最後から二つ目１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14025 \
		t:RPD_P022_01_R01_爆弾解体最後から二つ目１無線デモ１ \
		d:CODEC_DIRECT rt_P022_01_R01爆弾解体最後から二つ目１無線デモ１終了
}
proc rt_P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P023_01_R01爆弾解体最後の一つ１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14025 \
		t:RPD_P023_01_R01_爆弾解体最後の一つ１無線デモ１ \
		d:CODEC_DIRECT rt_P023_01_R01爆弾解体最後の一つ１無線デモ１終了
}
proc rt_P023_01_R01爆弾解体最後の一つ１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P024_01_R01爆弾解体センサーＢ入手１無線デモ１ {
	command パッド操作 -release
	// 無線内で画面フェードアウトと SNG_FOUTS_S をしてもらう
	if(($s:エリア == w12b) && (($s:登場ポイント == sp_w12a_w12b0_0 ) || ($s:登場ポイント == sp_w12a_w12b0_1 ))) {
	#if d:DEBUG_PRINT
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
	#endif
		eval($f:センサーＢ無線中にフェードアウト処理する = 1);
	} else if( ($s:登場ポイント == w16b_センサーＢ入手直後) \
			|| ($s:登場ポイント == sp_w20a_w12b0_0 ) \
			|| ($s:登場ポイント == sp_w20a_w14a_0 ) \
			|| ($s:登場ポイント == sp_w20a_w16a_0 ) \
			|| ($s:登場ポイント == sp_w20a_w22a_0 )) {
	#if d:DEBUG_PRINT
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
		print '*** Fade Out Flag ON ***'
	#endif
		eval($f:センサーＢ無線中にフェードアウト処理する = 1);
	} else {
	#if d:DEBUG_PRINT
		print '*** Fade Out Flag OFF ***'
		print '*** Fade Out Flag OFF ***'
		print '*** Fade Out Flag OFF ***'
		print '*** Fade Out Flag OFF ***'
		print '*** Fade Out Flag OFF ***'
	#endif
		eval($f:センサーＢ無線中にフェードアウト処理する = 0);
	}
	if($f:センサーＢ無線中にフェードアウト処理する){
		command 無線設定 \
		-call 14025 \
			t:RPD_P024_01_R01_爆弾解体センサーＢ入手１無線デモ１ \
			d:CODEC_DIRECT rt_P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 \
		-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) 

	}else {
		command 無線設定 \
		-call 14025 \
			t:RPD_P024_01_R01_爆弾解体センサーＢ入手１無線デモ１ \
			d:CODEC_DIRECT rt_P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了
	}
}

proc rt_P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 );
	eval( $f:w12a_Ａ脚ポンプ室爆弾解体前にセンサーＢゲット = 0 );
	command varsave $w:p_story
	command varsave $f:w12a_Ａ脚ポンプ室爆弾解体前にセンサーＢゲット

	command 全体マップ爆弾配置 -bomb d:Ａ脚マップ

	if(($s:エリア == w12b) && (($s:登場ポイント == sp_w12a_w12b0_0 ) || ($s:登場ポイント == sp_w12a_w12b0_1 ))) {
#ifdef d:STAGE_W12B
//		command セットサウンドコード -c d:SNG_FOUTS_S
		@w12b_センサーＢ無線後処理
#endif
	} else if( ($s:登場ポイント == w16b_センサーＢ入手直後) \
			|| ($s:登場ポイント == sp_w20a_w12b0_0 ) \
			|| ($s:登場ポイント == sp_w20a_w14a_0 ) \
			|| ($s:登場ポイント == sp_w20a_w16a_0 ) \
			|| ($s:登場ポイント == sp_w20a_w22a_0 )) {
//		command セットサウンドコード -c d:SNG_FOUTS_S
		restart -save

	} else {
		@爆弾タイマー開始
	}

}
// ------------------------------------------------
proc	rt_P025_01_R01爆弾解体昇降機下１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14025 \
		t:RPD_P025_01_R01_爆弾解体昇降機下１無線デモ１ \
		d:CODEC_DEMO rt_P025_01_R01爆弾解体昇降機下１無線デモ１終了
}
proc rt_P025_01_R01爆弾解体昇降機下１無線デモ１終了 {
//	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 );
}
// ------------------------------------------------
proc	rt_P026_01_R01爆弾解体終了１無線デモ１ {
	command パッド操作 -release
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_FOUTS_S
	chara delay ディレイ -time 60 -exec {
		command セットサウンドコード -c d:SNG_PLAY_06
	}
	chara delay ディレイ -time 90 -exec {
		command 無線設定 \
		-call 14085 \
			t:RPD_P026_01_R01_爆弾解体終了１無線デモ１ \
			d:CODEC_DIRECT rt_P026_01_R01爆弾解体終了１無線デモ１終了 \
			-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
	}
}
proc rt_P026_01_R01爆弾解体終了１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P026_01_R01爆弾解体終了１無線デモ１終了 );
//	command varsave
	restart -save
}

// ------------------------------------------------
proc	rt_P027_01_R01爆弾解体後昇降機ホール１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P027_01_R01_爆弾解体後昇降機ホール１無線デモ１ \
		d:CODEC_DEMO rt_P027_01_R01爆弾解体後昇降機ホール１無線デモ１終了 \
		-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P027_01_R01爆弾解体後昇降機ホール１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P027_01_R01爆弾解体後昇降機ホール１無線デモ１終了 );
//	command varsave
	restart -save
}
// ------------------------------------------------
proc	rt_P030_01_R01フォーチュン戦中無線無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P030_01_R01_フォーチュン戦中無線無線デモ１ \
		d:CODEC_DEMO rt_P030_01_R01フォーチュン戦中無線無線デモ１終了
}
proc rt_P030_01_R01フォーチュン戦中無線無線デモ１終了 {
	command パッド操作 -cancel
#ifdef d:STAGE_W11C
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_FOUTS_S
#endif
	eval( $w:p_story = d:ST:P030_01_R01フォーチュン戦中無線無線デモ１終了 );
	@ポリゴンデモスタート d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１開始
}
// ------------------------------------------------
proc	rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P035_01_R01_ファットマン爆弾解体終了１無線デモ１ \
		d:CODEC_DEMO rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 \
	-cont 14180 t:RPD_P035_01_R01_ファットマン爆弾解体終了１無線デモ１＿２ \
	-contc
}
//ＣＡＬＬ受信ＸＸＸＸＸＸＸＸＸｘ

proc rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 );
	eval( $w:p_story = d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 );
	@rp_プリスキン＿デフォルト無線セット
	@rp_w20c_懸垂監視ヤロウ参上
}

proc	rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１＿２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P035_01_R01_ファットマン爆弾解体終了１無線デモ１＿２ \
		d:CODEC_DIRECT rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１＿２終了
}
proc rt_P035_01_R01ファットマン爆弾解体終了１無線デモ１＿２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P035_01_R01ファットマン爆弾解体終了１無線デモ１終了 );
	@rp_プリスキン＿デフォルト無線セット
}

// ------------------------------------------------
proc	rt_P036_03_R01忍者登場３無線デモ１ {
	@まっしろ退散エセフォグセット
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P036_03_R01_忍者登場３無線デモ１ \
		d:CODEC_DIRECT rt_P036_03_R01忍者登場３無線デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
//	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
//				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
	-d
}

proc rt_P036_03_R01忍者登場３無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P036_03_R01忍者登場３無線デモ１終了 );
	command セットサウンドコード -c d:SNG_FOUTS_S
	restart
}
// ------------------------------------------------
proc	rt_P036_08_R02忍者登場８無線デモ２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P036_08_R02_忍者登場８無線デモ２ \
		d:CODEC_DIRECT rt_P036_08_R02忍者登場８無線デモ２終了 \
	-f ( 	d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}
proc rt_P036_08_R02忍者登場８無線デモ２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P036_08_R02忍者登場８無線デモ２終了 );
	//restart
	load 'wmovie'
}
// ------------------------------------------------
proc	rt_P036_10_R03忍者登場１０無線デモ３ {
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P036_10_R03_忍者登場１０無線デモ３ \
		d:CODEC_DIRECT rt_P036_10_R03忍者登場１０無線デモ３終了 \
	-f ( 	d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}
proc rt_P036_10_R03忍者登場１０無線デモ３終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P036_10_R03忍者登場１０無線デモ３終了 );
	command セットサウンドコード -c d:SNG_FOUTS_S
	restart
}
// ------------------------------------------------
proc	rt_P036_13_R04忍者登場１３無線デモ４ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P036_13_R04_忍者登場１３無線デモ４ \
		d:CODEC_DIRECT rt_P036_13_R04忍者登場１３無線デモ４終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P036_13_R04忍者登場１３無線デモ４終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P036_13_R04忍者登場１３無線デモ４終了 );
	command セットサウンドコード -c d:SNG_FOUTS_S
	@ゲームステージへ戻る
	@mv_demo4_w20b_0
}

// ------------------------------------------------
proc	rt_P037_01_R01スネークの遺体１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P037_01_R01_スネークの遺体１無線デモ１ \
		d:CODEC_DEMO rt_P037_01_R01スネークの遺体１無線デモ１終了
}
proc rt_P037_01_R01スネークの遺体１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P037_01_R01スネークの遺体１無線デモ１終了 );
	eval($f:rfp_スネークの遺体の話聞いた = 1)
	command varsave $w:p_story $f:rfp_スネークの遺体の話聞いた
}
// ------------------------------------------------

//野尻追加。p036は変装のため、ライデンのデータが場合によって異なります。
proc レジデント０交換リスタート {

	chara delay ディレイ -time 1 -exec {
		if($w:アイテム == d:アイテム:ゴル兵制服){
			load "d036p03" -r 'r_plt0'
		}else{
			restart
		}
	}

}


proc レジデント４交換リスタート {

	chara delay ディレイ -time 1 -exec {
		if($w:アイテム == d:アイテム:ゴル兵制服){
			load "d036p03" -r 'r_plt4'
		}else{
			restart
		}
	}
}

//野尻追加。p036は変装のため、ライデンのデータが場合によって異なります。


proc	rt_P040_02_R02エイムズ発見２無線デモ１ {
	command セットサウンドコード -c d:SNG_PLAY_05

	command パッド操作 -release
	command 無線設定 \
	-call 14072 \
		t:RPD_P040_02_R02_エイムズ発見２無線デモ１ \
		d:CODEC_DIRECT rt_P040_02_R02エイムズ発見２無線デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
//	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
//				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
	-d
}
proc rt_P040_02_R02エイムズ発見２無線デモ１終了 {

	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P040_02_R02エイムズ発見２無線デモ１終了 );

	restart

}

// ------------------------------------------------
proc	rt_P040_06a_R02aエイムズ発見６a無線デモ２a {
	command セットサウンドコード -c d:SNG_PLAY_05
	command パッド操作 -release
	command 無線設定 \
	-call 14072 \
		t:RPD_P040_06_R02_エイムズ発見６無線デモ２＿１ \
//		d:CODEC_DIRECT rt_P040_06a_R02aエイムズ発見６a無線デモ２a終了 \
		d:CODEC_DIRECT rt_P040_06c_R02bエイムズ発見６c無線デモ２b終了 \
	-f ( 	d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P040_06a_R02aエイムズ発見６a無線デモ２a終了 {
	command セットサウンドコード -c d:SNG_FOUTP_S
	command パッド操作 -cancel
// 英語版Ｂ対応 0109018 田中(信)
//	eval( $w:p_story = d:ST:P040_06a_R02aエイムズ発見６a無線デモ２a終了 );
	eval($w:p_story = d:ST:P040_06b_M00エイムズ発見６bムービーデモ０終了)
// 英語版Ｂ対応 0109018 田中(信)
//	load 'wmovie' -r 'r_plt0'
	load "d036p03"
}
// ------------------------------------------------
proc	rt_P040_06c_R02bエイムズ発見６c無線デモ２b {
	command セットサウンドコード -c d:SNG_FIN_S
	command パッド操作 -release
	command 無線設定 \
	-call 14072 \
		t:RPD_P040_06_R02_エイムズ発見６無線デモ２＿２ \
		d:CODEC_DIRECT rt_P040_06c_R02bエイムズ発見６c無線デモ２b終了 \
	-f ( 	d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P040_06c_R02bエイムズ発見６c無線デモ２b終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P040_06c_R02bエイムズ発見６c無線デモ２b終了 );
#if 1
	@レジデント４交換リスタート		//p_force_cdc.hの中
#else
	restart
#endif
}
// ------------------------------------------------
proc	rt_P041_02_R01エイムズ死亡後２無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P041_02_R01_エイムズ死亡後２無線デモ１ \
		d:CODEC_DIRECT rt_P041_02_R01エイムズ死亡後２無線デモ１終了
}
proc rt_P041_02_R01エイムズ死亡後２無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了 );
	print '$w:p_story = d:ST:P041_02_R01エイムズ死亡後２無線デモ１終了'



	eval( $i:プレイヤー初期Ｘ位置 = -2000 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = -4500 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = -66250 ) ;
	eval( $i:プレイヤー初期方向 = 3072 ) ;
	eval( $s:登場ポイント = 人質イベント後無線後 ) ;


	command varsave

	@シナリオデモ終了処理
}
// ------------------------------------------------
proc	rt_P042_01_R01シェル１シェル２連絡橋１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P042_01_R01_シェル１シェル２連絡橋１無線デモ１ \
		d:CODEC_DEMO rt_P042_01_R01シェル１シェル２連絡橋１無線デモ１終了
}
proc rt_P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 );
	// 以下はw25aにて処理
	@w25aシェル１シェル２連絡橋１無線デモ１終了処理
}
// ------------------------------------------------
proc	rt_P044_01_R01赤外線センサークリア１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P044_01_R01_赤外線センサークリア１無線デモ１ \
		d:CODEC_DEMO rt_P044_01_R01赤外線センサークリア１無線デモ１終了 \
	-cont 14085 t:RPD_P044_01_R01_赤外線センサークリア１無線デモ１＿２ \
		-f (d:CODEC_I_AC_O_FA | d:CODEC_FDOUT_BREAK)
}

proc rt_P044_01_R01赤外線センサークリア１無線デモ１終了 {
	eval( $w:p_story = d:ST:P044_01_R01赤外線センサークリア１無線デモ１終了 );
	eval( $w:p_story = d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１開始 ) ;
	command セットサウンドコード -c d:SNG_FOUTS_S
	if ( $w:アイテム == d:アイテム:ゴル兵制服 ) {
		load "d045p01" -r 'r_plt5' ;
	} else {
		load "d045p01" ;
	}
}

proc	rt_P044_01_R01赤外線センサークリア１無線デモ１＿２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P044_01_R01_赤外線センサークリア１無線デモ１＿２ \
		d:CODEC_DIRECT rt_P044_01_R01赤外線センサークリア１無線デモ１＿２終了
}
proc rt_P044_01_R01赤外線センサークリア１無線デモ１＿２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P044_01_R01赤外線センサークリア１無線デモ１終了 );
	restart ;
}

// ------------------------------------------------
proc	rt_P046_02_R01ハリアー戦勝利２無線デモ１ {
	// 欧州版にて下記を対応(2001.10.9)
	//chara サウンドマネージャー ＳＤマネ -pak -1
	// 
	command セットサウンドコード -c d:SNG_PLAY_06
	command パッド操作 -release
	@まっしろ退散エセフォグセット
	command 無線設定 \
	-call 14180 \
		t:RPD_P046_02_R01_ハリアー戦勝利２無線デモ１＿１ \
		d:CODEC_DIRECT rt_P046_02_R01ハリアー戦勝利２無線デモ１＿２終了 \ //rt_P046_02_R01ハリアー戦勝利２無線デモ１終了 \
	-cont 14085 t:RPD_P046_02_R01_ハリアー戦勝利２無線デモ１＿２ \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
	-d
}
proc rt_P046_02_R01ハリアー戦勝利２無線デモ１終了 {
	command パッド操作 -cancel
	//eval( $w:p_story = d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 );
	@rt_P046_02_R01ハリアー戦勝利２無線デモ１＿２
}
proc	rt_P046_02_R01ハリアー戦勝利２無線デモ１＿２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P046_02_R01_ハリアー戦勝利２無線デモ１＿２ \
		d:CODEC_DIRECT rt_P046_02_R01ハリアー戦勝利２無線デモ１＿２終了
}
proc rt_P046_02_R01ハリアー戦勝利２無線デモ１＿２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P046_02_R01ハリアー戦勝利２無線デモ１終了 );
	// ゲームへ
	command セットサウンドコード -c d:SNG_FOUTS_S
	@ゲームステージへ戻る
	@mv_w25a_w25b_0
}

// ------------------------------------------------
proc	rt_P047_05_R01電撃床前オルガ５無線デモ１ {
	//これはどうなの？ＸＸＸＸＸＸＸＸＸｘｘ
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P047_05_R01_電撃床前オルガ５無線デモ１ \
		d:CODEC_DIRECT rt_P047_05_R01電撃床前オルガ５無線デモ１終了 \
		-f (d:CODEC_I_AC_O_FA | d:CODEC_FDOUT_BREAK) \
		-d
}

proc rt_P047_05_R01電撃床前オルガ５無線デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P047_05_R01電撃床前オルガ５無線デモ１終了 );
	restart -s
}
// ------------------------------------------------
proc	rt_P049_02_R01大統領２無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14137 \
		t:RPD_P049_02_R01_大統領２無線機デモ１ \
		d:CODEC_DIRECT rt_P049_02_R01大統領２無線機デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
//	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
//				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
	-d
}

proc rt_P049_02_R01大統領２無線機デモ１終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P049_02_R01大統領２無線機デモ１終了 );
// 英語版Ｂ対応 010914 田中(信)
//	eval( $w:p_story = d:ST:P049_03_M01大統領３ムービーデモ１開始 );
	eval( $w:p_story = d:ST:P049_04_M02大統領４ムービーデモ２開始 );
	@mv_init_wmovie_0
}


// ------------------------------------------------
proc	rt_P049_05_R02大統領５無線機デモ２ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14137 \
		t:RPD_P049_02_R01_大統領５無線機デモ２ \
		d:CODEC_DIRECT rt_P049_05_R02大統領５無線機デモ２終了 \
		-f (d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
		-d
}

proc rt_P049_05_R02大統領５無線機デモ２終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P049_05_R02大統領５無線機デモ２終了 );
	eval($w:p_story = d:ST:P049_06_M03大統領６ムービーデモ３開始)
	restart -s
}
// ------------------------------------------------
proc	rt_P049_08_R03大統領８無線機デモ３ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14137 \
		t:RPD_P049_08_R03_大統領８無線機デモ３ \
		d:CODEC_DIRECT rt_P049_08_R03大統領８無線機デモ３終了 \
		-f (d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
		-d
}
proc rt_P049_08_R03大統領８無線機デモ３終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P049_08_R03大統領８無線機デモ３終了 );
	eval( $w:p_story = d:ST:P049_09_M05大統領９ムービーデモ５開始 );
	restart -s
}
// ------------------------------------------------
proc	rt_P049_10_R04大統領１０無線機デモ４ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14137 \
		t:RPD_P049_10_R04_大統領１０無線機デモ４ \
		d:CODEC_DIRECT rt_P049_10_R04大統領１０無線機デモ４終了 \
		-f (d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK) \
		-d
}
proc rt_P049_10_R04大統領１０無線機デモ４終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P049_10_R04大統領１０無線機デモ４終了 );
	eval( $w:p_story = d:ST:P049_11_P02大統領１１ポリゴンデモ２開始 );
	@mv_movie_w31a_0
}
// ------------------------------------------------
proc	rt_P049_12_R05大統領１２無線機デモ５ {
	command パッド操作 -release
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14085 \
		t:RPD_P049_12_R05_大統領１２無線機デモ５ \
		d:CODEC_DIRECT rt_P050_01_R01タンカー編整理１無線機デモ１終了 \
//	-f ( d:CODEC_I_AC_O_ ) \
	-f ( d:CODEC_I_AC_O_FD | d:CODEC_FDOUT_BREAK ) \
	-cont 14180 t:RPD_P050_01_R01_タンカー編整理１無線機デモ１
}
proc rt_P050_01_R01タンカー編整理１無線機デモ１終了 {
	eval( $w:p_story = d:ST:P050_01_R01タンカー編整理１無線機デモ１終了 );
	@mv_init_webdemo_0
}

// ------------------------------------------------
proc	rt_P050_04_R02タンカー編整理４無線機デモ２ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14180 \
		t:RPD_P050_04_R02_タンカー編整理４無線機デモ２ \
		d:CODEC_DIRECT rt_P050_04_R02タンカー編整理４無線機デモ２終了 \
	-f ( d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P050_04_R02タンカー編整理４無線機デモ２終了 {
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P050_04_R02タンカー編整理４無線機デモ２終了 );
	eval( $w:p_story = d:ST:P050_05_M03タンカー編整理５ムービーデモ３開始 );
	restart -s
}
// ------------------------------------------------
proc	rt_P050_06_R03タンカー編整理６無線機デモ３ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14180 \
		t:RPD_P050_06_R03_タンカー編整理６無線機デモ３ \
		d:CODEC_DIRECT rt_P050_06_R03タンカー編整理６無線機デモ３終了 \
	-f ( d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P050_06_R03タンカー編整理６無線機デモ３終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P050_06_R03タンカー編整理６無線機デモ３終了 );
	eval( $s:登場ポイント = タンカー編回想デモ終了 );
	@mv_movie_w31a_0
}

// ------------------------------------------------
proc	rt_P051_01_R01エマとオタコン１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P051_01_R01_エマとオタコン１無線機デモ１ \
		d:CODEC_DIRECT rt_P051_01_R01エマとオタコン１無線機デモ１終了
}
proc rt_P051_01_R01エマとオタコン１無線機デモ１終了 {
	command パッド操作 -cancel
// ストーリーフラグ廃止につきコメントアウト。by S.Hirano 010505
//	eval( $w:p_story = d:ST:P051_01_R01エマとオタコン１無線機デモ１終了 );
}

proc	rt_P051_01_R01エマとオタコン１無線機デモ１＿２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P051_01_R01_エマとオタコン１無線機デモ１＿２ \
		d:CODEC_DIRECT rt_P051_01_R01エマとオタコン１無線機デモ１＿２終了
}
proc rt_P051_01_R01エマとオタコン１無線機デモ１＿２終了 {
	command パッド操作 -cancel
// ストーリーフラグ廃止につきコメントアウト。by S.Hirano 010505
//	eval( $w:p_story = d:ST:P051_01_R01エマとオタコン１無線機デモ１終了 );
}

// ------------------------------------------------
proc	rt_P052_01_R01ローズ無線１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P052_01_R01_ローズ無線１無線機デモ１ \
		d:CODEC_DIRECT rt_P052_01_R01ローズ無線１無線機デモ１終了
}
proc rt_P052_01_R01ローズ無線１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P052_01_R01ローズ無線１無線機デモ１終了 );
}
// ------------------------------------------------
proc	rt_P053_01_R01息継ぎ１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P053_01_R01_息継ぎ１無線機デモ１ \
		d:CODEC_DIRECT rt_P053_01_R01息継ぎ１無線機デモ１終了
}
proc rt_P053_01_R01息継ぎ１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P053_01_R01息継ぎ１無線機デモ１終了 );
}

// ------------------------------------------------
proc	rt_P057_02_R01ヴァンプ戦終了２無線機デモ１ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1		// パックをロードせず前のものを使用
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_05
	command 無線設定 \
	-call 14085 \
		t:RPD_P057_02_R01_ヴァンプ戦終了２無線機デモ１ \
		d:CODEC_DIRECT rt_P057_02_R01ヴァンプ戦終了２無線機デモ１＿２終了 \
	-cont 14180 t:RPD_P057_02_R01_ヴァンプ戦終了２無線機デモ１＿２ \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc	rt_P057_02_R01ヴァンプ戦終了２無線機デモ１＿２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P057_02_R01_ヴァンプ戦終了２無線機デモ１＿２ \
		d:CODEC_DIRECT rt_P057_02_R01ヴァンプ戦終了２無線機デモ１＿２終了 \
	-f ( d:CODEC_I_FA_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P057_02_R01ヴァンプ戦終了２無線機デモ１＿２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了 );
	eval( $s:d_num = not_demo );
	command セットサウンドコード -c d:SNG_FOUTS_S
	chara delay ディレイ -time 1 -exec {
		@mv_w31b_w31c0_0
	}
}

// ------------------------------------------------
proc	rt_P058_02_R01エマ救出２無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14152 \
		t:RPD_P058_02_R01_エマ救出２無線機デモ１ \
		d:CODEC_DIRECT rt_P058_02_R01エマ救出２無線機デモ１終了 \
	-f ( d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P058_02_R01エマ救出２無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P058_02_R01エマ救出２無線機デモ１終了 );
	eval( $w:p_story = d:ST:P058_03_P02エマ救出３ポリゴンデモ２開始 );

	// ゲームからのＢＧＭをここでフェードアウト
	command セットサウンドコード -c d:SNG_FOUTS_S
	restart -s
}
// ------------------------------------------------
proc	rt_P058_04_R02エマ救出４無線機デモ２ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1		// パックをロードしない
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14152 \
		t:RPD_P058_04_R02_エマ救出４無線機デモ２ \
		d:CODEC_DIRECT rt_P058_04_R02エマ救出４無線機デモ２終了 \
	-f ( d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P058_04_R02エマ救出４無線機デモ２終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P058_04_R02エマ救出４無線機デモ２終了 );
	eval( $w:p_story = d:ST:P058_05_M01エマ救出５ムービーデモ１開始 );
	@mv_init_wmovie_0
}

// ------------------------------------------------
proc	rt_P058_07_R03エマ救出７無線機デモ３ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1		// パックをロードしない
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14152 \
		t:RPD_P058_07_R03_エマ救出７無線機デモ３ \
		d:CODEC_DIRECT rt_P058_07_R03エマ救出７無線機デモ３終了 \
	-f ( d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P058_07_R03エマ救出７無線機デモ３終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P058_07_R03エマ救出７無線機デモ３終了 );
	eval( $w:p_story = d:ST:P058_08_P04エマ救出８ポリゴンデモ４開始 );
	restart -save
}

// ------------------------------------------------
proc	rt_P059_03_R01エマ休憩３無線機デモ１ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1		// パックをロードしない
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14085 \
		t:RPD_P059_03_R01_エマ休憩３無線機デモ１ \
		d:CODEC_DEMO rt_P061_01_R01エマとオタコン１無線機デモ１終了 \
	-cont 14180 t:RPD_P061_01_R01_エマとオタコン１無線機デモ１ \
//		d:CODEC_DIRECT rt_P059_03_R01エマ休憩３無線機デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
//	-f ( d:CODEC_I_FA_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P059_03_R01エマ休憩３無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P059_03_R01エマ休憩３無線機デモ１終了 );
	@rt_P061_01_R01エマとオタコン１無線機デモ１
}

// ------------------------------------------------
proc	rt_P061_01_R01エマとオタコン１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P061_01_R01_エマとオタコン１無線機デモ１ \
		d:CODEC_DIRECT rt_P061_01_R01エマとオタコン１無線機デモ１終了 \
	-f ( d:CODEC_I_FA_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}

proc rt_P061_01_R01エマとオタコン１無線機デモ１終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P061_01_R01エマとオタコン１無線機デモ１終了 );
	eval( $s:d_num = not_demo );
	@mv_w31b_w31c0_0
}

// ------------------------------------------------
proc	rt_P062_03_R01エマとＡＩ３無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14152 \
		t:RPD_P062_03_R01_エマとＡＩ３無線機デモ１ \
		d:CODEC_DIRECT rt_P062_03_R01エマとＡＩ３無線機デモ１終了 \
	-f ( d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P062_03_R01エマとＡＩ３無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P062_03_R01エマとＡＩ３無線機デモ１終了 );
	restart
}
// ------------------------------------------------
proc	rt_P062_05_R02エマとＡＩ５無線機デモ２ {
	command パッド操作 -release
	command セットサウンドコード -c d:SNG_PLAY_06
	command 無線設定 \
	-call 14152 \
		t:RPD_P062_05_R02_エマとＡＩ５無線機デモ２ \
		d:CODEC_DIRECT rt_P062_05_R02エマとＡＩ５無線機デモ２終了 \
	-f ( d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P062_05_R02エマとＡＩ５無線機デモ２終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P062_05_R02エマとＡＩ５無線機デモ２終了 );
	restart
}
// ------------------------------------------------
proc	rt_P064_01_R01エマライデンローズ無線１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P064_01_R01_エマライデンローズ無線１無線機デモ１ \
		d:CODEC_DEMO rt_P064_01_R01エマライデンローズ無線１無線機デモ１終了
}
proc rt_P064_01_R01エマライデンローズ無線１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P064_01_R01エマライデンローズ無線１無線機デモ１終了 );
}
// ------------------------------------------------
proc	rt_P065_02_R01エマＬ脚２無線機デモ１ {
	command パッド操作 -release
	@まっしろ退散エセフォグセット
	command 無線設定 \
	-call 14152 \
		t:RPD_P065_02_R01_エマＬ脚２無線機デモ１ \
		d:CODEC_DIRECT rt_P065_02_R01エマＬ脚２無線機デモ１終了 \
	-f ( d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P065_02_R01エマＬ脚２無線機デモ１終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P065_02_R01エマＬ脚２無線機デモ１終了 );
	#if 0
		eval($w:p_story = d:ST:P065_02_R01エマＬ脚２無線機デモ１終了)
		// ゲームへ
		@ゲームステージへ戻る
		restart
	#else
		eval($w:p_story = d:ST:P065_03_P02エマＬ脚３ポリゴンデモ２開始)
		if ($w:アイテム == d:アイテム:ゴル兵制服){
		 load 'd065p02' -r 'r_plt5'
		} else {
		 load 'd065p02'
		}
	#endif
}
// ------------------------------------------------
proc	rt_P065_06_R02Ｌ脚エマ６無線機デモ２ {
	command パッド操作 -release
	@まっしろ退散エセフォグセット
	command 無線設定 \
	-call 14180 \
		t:RPD_P065_06_R02_Ｌ脚エマ６無線機デモ２ \
		d:CODEC_DIRECT rt_P065_06_R02Ｌ脚エマ６無線機デモ２終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P065_06_R02Ｌ脚エマ６無線機デモ２終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P065_06_R02Ｌ脚エマ６無線機デモ２終了 );
	// ゲームへ
	@ゲームステージへ戻る
	@mv_w28a_w32a_0
}
// ------------------------------------------------
proc	rt_P067_01_R01エマ狙撃１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P067_01_R01_エマ狙撃１無線機デモ１ \
		d:CODEC_DEMO rt_P067_01_R01エマ狙撃１無線機デモ１終了
}
proc rt_P067_01_R01エマ狙撃１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P067_01_R01エマ狙撃１無線機デモ１終了 );
	@スネーク到着無線終了
}
// ------------------------------------------------
proc	rt_P069_02_R01ヴァンプ狙撃終了２無線機デモ１ {
	command パッド操作 -release
	@まっしろ退散エセフォグセット
	command 無線設定 \
	-call 14180 \
		t:RPD_P069_02_R01_ヴァンプ狙撃終了２無線機デモ１ \
		d:CODEC_DIRECT rt_P069_02_R01ヴァンプ狙撃終了２無線機デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}

proc rt_P069_02_R01ヴァンプ狙撃終了２無線機デモ１終了 {
	// 注意！ ここのストーリーフラグはイレギュラーな動きをします。
	// この後はゲームに戻っていたのですが、ポリデモが追加になりました。
	// 通常、無線終了フラグがたち、ポリデモ開始フラグが立ちますが、
	// 無線終了フラグを見ている個所が複数あり、それをすべて修正し検証する時間はありません。
	// そのためここだけ、無線開始->ポリデモ開始->ポリデモ終了->無線終了という形にします。
	eval( $w:p_story = d:ST:P069_03_P02ヴァンプ狙撃終了２ポリゴンデモ２開始 );
	// 新しくデモが追加されたのでリスタートに変更。
	command セットサウンドコード -c d:SNG_FOUTS_S

	@まっしろ退散エセフォグセット
	chara delay ディレイ \
		-time 60 \ 
		-exec{
			command パッド操作 -cancel
			restart
		}
	
}
// ------------------------------------------------
proc	rt_P069_04_R02ヴァンプ狙撃終了４無線機デモ２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P069_04_R02_ヴァンプ狙撃終了４無線機デモ２ \
		d:CODEC_DIRECT rt_P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 {
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了 );
	restart -s
}

// ------------------------------------------------
proc	rt_P070_10_R01ＡＧ起動１０無線機デモ１ {
//	@まっしろ退散エセフォグセット
	chara フェードインアウト おもしろまっくろ \
		-c 0,0,0 128 \
		-t 0

	command パッド操作 -release
	command 無線設定 \
	-call 14096 \
		t:RPD_P070_10_R01_ＡＧ起動１０無線機デモ１ \
		d:CODEC_DEMO rt_P070_10_R01ＡＧ起動１０無線機デモ１終了 \
	-f ( 	d:CODEC_I_FA_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
				d:CODEC_NO_ICON  )
//	-f ( 	d:CODEC_I_FA_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P070_10_R01ＡＧ起動１０無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P070_10_R01ＡＧ起動１０無線機デモ１終了 );

	// 拷問部屋へ。polydemo_w41a.h でいきなりムービーを始めるために、
	//もう一個ストーリーフラグを進めておく。
	eval($w:p_story = d:ST:P070_11_M02ＡＧ起動１１ムービーデモ２開始)

	// 7/18向手修正
	// ムービー再生専用ステージに飛ばす
//	@mv_w24d_w41a_0
	@mv_init_wmovie_0
}
// ------------------------------------------------
proc	rt_P070_18_R02ＡＧ起動１８無線機デモ２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P070_18_R02_ＡＧ起動１８無線機デモ２ \
		d:CODEC_DIRECT rt_P070_18_R02ＡＧ起動１８無線機デモ２終了 \
	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
//	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
//				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
	-d
}
proc rt_P070_18_R02ＡＧ起動１８無線機デモ２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P070_18_R02ＡＧ起動１８無線機デモ２終了 );

	command セットサウンドコード -c d:SNG_FOUTS_S

	// 7/18向手修正
	// ムービー再生専用ステージに飛ばす
//	restart
	@mv_init_wmovie_0
}
// ------------------------------------------------
proc	rt_P070_20_R03ＡＧ起動２０無線機デモ３ {
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P070_20_R03_ＡＧ起動２０無線機デモ３ \
		d:CODEC_DIRECT rt_P070_20_R03ＡＧ起動２０無線機デモ３終了 \
	-f ( 	d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
	-d
}
proc rt_P070_20_R03ＡＧ起動２０無線機デモ３終了 {
	command パッド操作 -cancel
	command セットサウンドコード -c d:SNG_FOUTS_S
	eval( $w:p_story = d:ST:P070_20_R03ＡＧ起動２０無線機デモ３終了 );
	eval( $w:p_story = d:ST:P070_21_P12ＡＧ起動２１ポリゴンデモ１２開始 )
	load 'w42a'
}
// ------------------------------------------------
proc	rt_P070_23_R04ＡＧ起動２３無線機デモ４ {
	if($w:p_story < d:ST:P070_23_R04ＡＧ起動２３無線機デモ４終了){
		command パッド操作 -release
		command 無線設定 \
		-call 14096 \
			t:RPD_P070_23_R04_ＡＧ起動２３無線機デモ４ \
			d:CODEC_DEMO rt_P070_23_R04ＡＧ起動２３無線機デモ４終了
	//	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDOUT_BREAK ) \
	}
}

proc rt_P070_23_R04ＡＧ起動２３無線機デモ４終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P070_23_R04ＡＧ起動２３無線機デモ４終了 );
	command varsave $w:p_story
	// ゲームへ
	@ゲームステージへ戻る
	@w41a_拘束開放処理
//	restart
	// 8/13向手修正
//	@mv_w24d_w41a_0
}

// ------------------------------------------------
proc	rt_P071_01_R01拷問部屋脱出１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P071_01_R01_拷問部屋脱出１無線機デモ１ \
		d:CODEC_DEMO rt_P071_01_R01拷問部屋脱出１無線機デモ１終了
}
proc rt_P071_01_R01拷問部屋脱出１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P071_01_R01拷問部屋脱出１無線機デモ１終了 );
	command varsave $w:p_story
}
// ------------------------------------------------
//rt_P072_01_R01大佐混乱１無線機デモ１
//転居のお知らせ。7/19にw42a_cdc.chへ引っ越しました。
//お近くにいらした時は是非お立ち寄りください。
// ------------------------------------------------
//rt_P073_01_R01通路Ａ１無線機デモ１
//転居のお知らせ。7/19にw43a_cdc.chへ引っ越しました。
//お近くにいらした時は是非お立ち寄りください。
// ------------------------------------------------

proc	rt_P073_03_R02通路Ａ３無線機デモ２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P073_03_R02_通路Ａ３無線機デモ２ \
		d:CODEC_DIRECT rt_P073_03_R02通路Ａ３無線機デモ２終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
//	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
//				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
	-d
}
proc rt_P073_03_R02通路Ａ３無線機デモ２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P073_03_R02通路Ａ３無線機デモ２終了 );

	//以下はw43aで処理
	@通路Ａ３無線機デモ２終了処理
}

// ------------------------------------------------
proc	rt_P074_02_R01通路Ａ刀後２無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P074_02_R01_通路Ａ刀後２無線機デモ１ \
		d:CODEC_DIRECT rt_P074_02_R01通路Ａ刀後２無線機デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) \
//	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
//				d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) \
	-d
}
proc rt_P074_02_R01通路Ａ刀後２無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P074_02_R01通路Ａ刀後２無線機デモ１終了 );
	command セットサウンドコード -c d:SNG_FOUTS_S

#if 1
	eval($w:p_story = d:ST:P074_02_R01通路Ａ刀後２無線機デモ１終了)
	restart
#else
	// ポリデモを呼ぶ（今はポリデモを跳ばしてロード）
	eval($w:p_story = d:ST:P074_03_P02通路Ａ刀後３ポリゴンデモ２終了)	// 仮
	@ステージ終了時処理 34250 4000 -86000 2048 mv_w43a_w44a_0 d:DEFAULT_M_TYPE
#endif
}
// ------------------------------------------------
proc	rt_P075_01_R01オタコン無事１無線機デモ１ {
	command パッド操作 -release
	chara サウンドマネージャー ＳＤマネ -pak -1		// パックをロードしない
	mesg サウンドマネージャー ＳＤマネ SD_CODE d:SNG_PLAY_06
	command 無線設定 \
	-call 14112 \
		t:RPD_P075_01_R01_オタコン無事１無線機デモ１ \
		d:CODEC_DEMO rt_P075_01_R01オタコン無事１無線機デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
				 d:CODEC_NO_ICON ) 
}
proc rt_P075_01_R01オタコン無事１無線機デモ１終了 {
	eval( $w:p_story = d:ST:P075_01_R01オタコン無事１無線機デモ１終了 );
	command 無線設定 -base 14112 t:RPO_オタコンデフォルト
	command セットサウンドコード -c d:SNG_FOUTS_S
	command varsave $w:p_story
//	@オタコン無事１無線機デモ１後
	@ゲームステージへ戻る
	@mv_w43a_w44a_0
}
// ------------------------------------------------
proc	rt_P076_01_R01大佐ＡＩ１無線機デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P076_01_R01_大佐ＡＩ１無線機デモ１ \
		d:CODEC_DEMO rt_P076_01_R01大佐ＡＩ１無線機デモ１終了
}
proc rt_P076_01_R01大佐ＡＩ１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P076_01_R01大佐ＡＩ１無線機デモ１終了 );
	command varsave $w:p_story
	@大佐ＡＩ１無線機デモ１後
}
// ------------------------------------------------
proc	rt_P077_01_R01ローズ拉致１無線機デモ１ {
	command パッド操作 -release

	if ( $w:プレイヤーフラグ & d:PL_GAVE_SHAVER_TO_SNAKE ) {
		command 無線設定 \
		-call 14085 \
			t:RPD_P077_01_R01_ローズ拉致１無線機デモ１＿ひげそり \
			d:CODEC_DEMO rt_P077_01_R01ローズ拉致１無線機デモ１終了
	}else {
		command 無線設定 \
		-call 14085 \
			t:RPD_P077_01_R01_ローズ拉致１無線機デモ１ \
			d:CODEC_DEMO rt_P077_01_R01ローズ拉致１無線機デモ１終了
	}
}


proc rt_P077_01_R01ローズ拉致１無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P077_01_R01ローズ拉致１無線機デモ１終了 );
	command セットサウンドコード -c d:SNG_PLAY_06	//	向手修正9/5
}
// ------------------------------------------------
proc	rt_P080_19_R01ＡＧ浮上１９無線機デモ１ {
//	@まっしろ退散エセフォグセット
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P080_19_R01_ＡＧ浮上１９無線機デモ１ \
		d:CODEC_DEMO rt_P080_19_R01ＡＧ浮上１９無線機デモ１終了 \
	-f ( 	d:CODEC_I_AC_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK | \
			 d:CODEC_NO_ALARM | d:CODEC_NO_ICON ) 
}
proc rt_P080_19_R01ＡＧ浮上１９無線機デモ１終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P080_19_R01ＡＧ浮上１９無線機デモ１終了 );
// 英語版Ｂ対応 0109014 田中(信)
//	eval($w:p_story = d:ST:P080_20_M08ＡＧ浮上２０ムービーデモ８開始)
	eval($w:p_story = d:ST:P080_22_M09ＡＧ浮上２２ムービーデモ９開始)
	@mv_init_wmovie_0
}

// ------------------------------------------------
proc	rt_P080_21_R02ＡＧ浮上２１無線機デモ２ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P080_21_R02_ＡＧ浮上２１無線機デモ２ \
		d:CODEC_DIRECT rt_P080_21_R02ＡＧ浮上２１無線機デモ２終了 \
	-f ( d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P080_21_R02ＡＧ浮上２１無線機デモ２終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P080_21_R02ＡＧ浮上２１無線機デモ２終了 );
	eval( $w:p_story = d:ST:P080_22_M09ＡＧ浮上２２ムービーデモ９開始 );
	restart -s
}

// ------------------------------------------------
proc	rt_P080_23_R03ＡＧ浮上２３無線機デモ３ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P080_23_R03_ＡＧ浮上２３無線機デモ３ \
		d:CODEC_DIRECT rt_P080_23_R03ＡＧ浮上２３無線機デモ３終了 \
	-f ( d:CODEC_I_FD_O_FD | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )

}
proc rt_P080_23_R03ＡＧ浮上２３無線機デモ３終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P080_23_R03ＡＧ浮上２３無線機デモ３終了 );
	eval( $w:p_story = d:ST:P080_24_M10ＡＧ浮上２４ムービーデモ１０開始 );
	restart -s
}
// ------------------------------------------------
proc	rt_P080_25_R04ＡＧ浮上２５無線機デモ４ {
	command パッド操作 -release
	command 無線設定 \
	-call 14085 \
		t:RPD_P080_25_R04_ＡＧ浮上２５無線機デモ４ \
		d:CODEC_DIRECT rt_P080_25_R04ＡＧ浮上２５無線機デモ４終了 \
	-f ( d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}
proc rt_P080_25_R04ＡＧ浮上２５無線機デモ４終了 {
	command パッド操作 -cancel
	eval( $w:p_story = d:ST:P080_25_R04ＡＧ浮上２５無線機デモ４終了 );
	eval( $w:p_story = d:ST:P080_26_P11ＡＧ浮上２６ポリゴンデモ１１開始 );
	@mv_P080_14_P08_0
}
// ------------------------------------------------

#endif
