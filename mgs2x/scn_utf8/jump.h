/*
	jump.h                     
	    ジャンプモーション用proc

	2000/08/28 H.Yoshiike         
	$Id: jump.h,v 1.3 2000/11/15 11:22:48 usr03682 Exp $                      

	
*/

// ファイルが二重呼びされたときの対処
#ifndef d:JUMP_H
#define JUMP_H	1


/*
※このファイルをincludeする際は、list以下でステージ固有のmlsファイルを作成した後に次の例のようなモーションリストをgclに記述してください。

enum モーションリスト {
	non_jump_up,				// ジャンプアップ
	non_jump_down_start,		// ジャンプダウンスタート
	non_jump_down_loop,			// ジャンプダウンループ
	non_jump_down_end			// ジャンプダウンエンド
	non_jump_up_behind			// 張りつきからのジャンプ
};
*/


// ジャンプモーション用proc
//-----------------------------------------------------
proc ジャンプ発動 $:トラップ名 $:登れる高さ $:高さ幅 {
	command 自動ジャンプ設定 \
		-motions \
			d:モーションリスト:non_jump_up \			// ジャンプアップ \
			d:モーションリスト:non_jump_down_start \	// ジャンプダウンスタート \
			d:モーションリスト:non_jump_down_loop \		// ジャンプダウンループ \
			d:モーションリスト:non_jump_down_end \		// ジャンプダウンエンド \
			d:モーションリスト:non_jump_up_behind \		// 張りつきからのジャンプ \
		-trap $:トラップ名 \							// 処理をするトラップ名（省略するといつも処理する） \
		-climb_height $:登れる高さ $:高さ幅				// $i:登れる高さ $i:高さ幅
}

#else

print 'jump.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


