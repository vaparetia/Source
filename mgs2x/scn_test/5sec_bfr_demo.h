/*
	5sec_bfr_demo.h
	    デモ再生前に五秒間待つ

	2000/08/02 S.Hirano
	$Id: 5sec_bfr_demo.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $

*/


// デモ再生前に五秒間待つ
//----------------------------------------------------------
#define デモ前五秒間待ち 0

//----------------------------------------------------------
#if d:デモ前五秒間待ち
proc global_ボタン押された {
	chara delay デモディレイ -time 300 -exec {
		eval($f:global_デバッグポーズ通過 = 1)
		restart
	}
}

proc global_五秒待ちセット {
	if(!$f:global_デバッグポーズ通過){
		command パッド操作 -cancel
		chara デバッグポーズ pause '5_SEC_TO_START' -p global_ボタン押された
	}
}
#endif