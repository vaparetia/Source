/*
	st_common.h
	    ストリーキング用共通プロック
	2002/06/21 T.Ohtani

	$Id: st_common.h,v 1.10 2002/07/29 05:32:44 usr13731 Exp $
*/
// define
#define 退場ディレイ時間 180

proc ストリーキングプロックセット $:p_スネークファイル $:p_雷電ファイル $:p_クリア条件フラグ $:p_ゴールＸ $:p_ゴールＹ $:p_ゴールＺ {
	#if d:DEBUG_PRINT
		print 'vr_common_proc_set'
	#endif

	#ifndef d:NO_VR_EFFECT
		@ＶＲステータスセット
	#endif
	@全ミッション共通システム設定 $:p_クリア条件フラグ
	@常駐キャラ設定											// stdch.hで定義
	@各ミッション用ゴール表示 $:p_ゴールＸ $:p_ゴールＹ $:p_ゴールＺ
	@サウンド設定
	@A_プレイヤー設定 $:p_スネークファイル $:p_雷電ファイル		// a_stdproc.hで定義
	@ＶＲエフェクト設置
	@ＶＲ空設置
	@ＶＲステージ変形設定
}


proc ストリーキングネクストステージ $:p_次ステージ {
	// 時間を記憶
	eval ($i:A_ストリーキングミッションタイム = `%ＶＲゲットタイム` );
	command varsave $i:A_ストリーキングミッションタイム

	// 退場処理
	command メニュー設定 -menu off -gage off  -radar off -subwin off // 表示オフ
	chara プレイヤ退場	d:PLAYER				// プレイヤ消去エフェクト
	mesg プレイヤー 	d:PLAYER shadow 0		// 影消去
	mesg ＶＲ床マーカー ＶＲ床マーカー 非表示	// 床マーカー非表示
	mesg ＶＲ壁マーカー ＶＲ壁マーカー 非表示	// 壁マーカー非表示

	// VRウィンドウスキップフラグを寝かす。
	command ＶＲウィンドウ状態のリセット

	// フェードアウト
	chara フェードインアウト フェードアウト \
		-c 0,0,0 128 \
		-t 90 \//d:FADEOUT_TIME
		-p $:p_次ステージ

}
