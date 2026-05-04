/*	無線機モードスクリプト*/

/* メッセージ定義 */
#include "story.h"

#include "cdc_proc.h"

//日本語版
#ifdef JAPANESE
#include "t_demo_j.cm"
#include "t_demo_j.ct"
#endif

//英語版
#ifdef ENGLISH
#include "t_demo_e.cm"
#include "t_demo_e.ct"
#endif



block codec RTO_00b1Rオープニング無線機デモ 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050101
	@vc050204
	eval($w:t_story = d:ST_T00b1Rオープニング無線機デモ終了)
}



block codec RTO_03b1Rカモフ登場後無線機デモ体験版 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050301
	eval($w:t_story = d:ST_T03b1Rカモフ登場後無線機デモ終了)
}


block codec RTO_03b1Rカモフ登場後無線機デモ製品版 14112 d:顔その1 {
	@rto_デフォルト前処理
	eval($w:t_story = d:ST_T03b1Rカモフ登場後無線機デモ終了)
}


block codec RTO_04a0R操舵室無線機デモ 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050401
	eval($w:t_story = d:ST_操舵室無線機デモ終了)
}



/*-------------------------------------------------------
でばっぐす
--------------------------------------------------------*/
block codec RTO_vc050101 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050101
}

block codec RTO_vc050204 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050204
}


block codec RTO_vc050301 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050301
}

block codec RTO_vc050401 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050401
}

