/*
	a_gameover.h
	    ゲームオーバーに関する関数をまとめたファイル(アナザー用)

	2002/06/04 H.Yoshiike
	$Id: a_gameover.h,v 1.14 2002/08/29 07:15:18 usr13731 Exp $


*/

// ファイルが二重呼びされたときの対処
#ifndef d:A_GAMEOVER_H
#define A_GAMEOVER_H	1


#include "sound.h"

proc A_ゲームオーバー処理 {
	#if d:DEBUG_PRINT
		print 'a_gameover!!!!!!!!!!'
	#endif

	command セットサウンドコード -c d:SNG_FOUTS_S
	command セットサウンドコード -c d:SE_JOUCHUU_OFF
	// スタート直後に死んだ場合マップネームを消す
	mesg ２Ｄスプライト表示 マップネーム表示 kill

	#ifdef d:GAMEOVER_ADD
	@追加ゲームオーバー処理
	#endif

	#ifdef d:STAGE_VR
		switch ( $s:選択プレイヤー ) {
			case ( 前作スネーク ) {
				command ゲームオーバー音声設定 -vox t:vc030223;	// VRmission_endBGM(前作スネーク使用時)
			}
			case ( 裸ライデン ) {
				command ゲームオーバー音声設定 -vox t:vc030225;	// VRmission_endBGM(hadaka mission)
			}
			default {
				command ゲームオーバー音声設定 -vox t:vc030221;	// VRmission_endBGM
			}
		}
	#else
				command ゲームオーバー音声設定 -vox t:vc030222;	// Alternativemission_endBGM
				// ゲームオーバー後n_title の適切な画面に戻るための処理
				$f:スネークテイルズフラグ = d:TRUE;
				$f:ミッションズフラグ = d:FALSE;
	#endif
}

proc A_コンティニュー処理 {
	#if d:DEBUG_PRINT
		print "continue"
	#endif

	#ifdef d:CONTINUE_ADD
	@追加コンティニュー処理
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			@A_爆弾タイマー加算処理
		}
		case ( d:MISSION:バラエティ ) {
			#ifdef d:STAGE_SP03A
			// ザコサバイバルはリスタート時最初のテロップから開始させるため、変数を初期値に戻す
				$i:プレイヤー初期Ｘ位置 = 0;
				$i:プレイヤー初期Ｙ位置 = 0;
				$i:プレイヤー初期Ｚ位置 = 2000;
				$i:プレイヤー初期方向 = 2048 ;
				$f:ロードチェックＯＮフラグ = 0;
				$f:sp03a_ザコサバイバルゲームフラグ = 0;
				$b:ステージ番号 = 0;
				$f:A_一括初期化拒否フラグ = 0;
				command varsave
			#endif
		}
		case ( d:MISSION:ストリーキング ) {

			#ifdef d:STAGE_ST01A
			$i:A_ストリーキングミッションタイム = `%ＶＲゲットタイム`;
				command varsave $i:A_ストリーキングミッションタイム
			#endif
		}
		default {
		}
	}

	if ( $b:ミッション番号 == d:MISSION:爆弾解体 || $b:ミッション番号 == d:MISSION:スネークテイルズ ) {
		if ( $w:体力 <= $w:出血開始体力 ) {
			// ロード時出欠していたら止血されるところまで回復
			$w:体力 = $w:出血開始体力;
		}
	}
}

proc A_爆弾タイマー加算処理 {
	// 一回でもロードをかけてリスタートしたら、タイムを加算する
	if ( $f:爆弾解体ミッションＶＲウィンドウ表示フラグ == 0 ) {
		$i:爆弾解体タイマー残り時間 = `%ＶＲゲットタイム`;
		command varsave $i:爆弾解体タイマー残り時間
	}
}


#else

print 'a_gameover.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


