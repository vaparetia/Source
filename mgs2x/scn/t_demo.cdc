/*	無線機モードスクリプト*/

/* メッセージ定義 */
#include "story.h"
#define 顔その1	t:f00a

#include "cdc_proc.h"

#include "t_demo_e.cm"
#include "t_demo_e.ct"

#include "t_demo_f.cm"
#include "t_demo_f.ct"

#include "t_demo_g.cm"
#include "t_demo_g.ct"

#include "t_demo_s.cm"
#include "t_demo_s.ct"

#include "t_demo_i.cm"
#include "t_demo_i.ct"

#include "t_demo_j.cm"
#include "t_demo_j.ct"

proc rto_うなづきテスト１ {
	mindvoice 	0 \	//是
				t:vc122501	\	//スネーク　「ふむ」
				t:vc122505	\	//スネーク　「なるほど」
				t:vc122509	\	//スネーク　「そうだ」
				t:vc122513	//スネーク　「その通りだ」

	mindvoice 	1 \	//否
				t:vc122517	\	//スネーク　「そうか？」
				t:vc122521	\	//スネーク　「そうかぁ？」
				t:vc122525	\	//スネーク　「違う」
				t:vc122529	//スネーク　「そんなわけないだろ！」
}


block codec RTO_00b1Rオープニング無線機デモ 14112 d:顔その1 {
	@rto_デフォルト前処理

tracecamera 0 1
	@vc050101
	@vc050204
	eval($w:t_story = d:ST_T00b1Rオープニング無線機デモ終了)
	//無線セット
	memregist 0 0 14112
	memregist 1 1 14096

}



block codec RTO_03b1Rカモフ登場後無線機デモ体験版 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050301
	eval($w:t_story = d:ST_T03b1Rカモフ登場後無線機デモ終了)
}


block codec RTO_04a0R操舵室無線機デモ 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050401
	eval($w:t_story = d:ST_操舵室無線機デモ終了)
}

block codec RTO_06b1Rオルガ戦勝利後無線機デモ 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080101
	eval($w:t_story = d:ST_T06b1Rオルガ戦勝利後無線機デモ終了)
}

//製品版ばん
block codec RTO_03b1Rカモフ登場後無線機デモ製品版 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050302
	eval($w:t_story = d:ST_T03b1Rカモフ登場後無線機デモ終了)
}

block codec RTO_06b1Rオルガ戦勝利後無線機デモ１ 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080101
	@vc080102	//むびＸＸＸＸＸｘ追加
	@vc080201
	eval($w:t_story = d:ST_T06b1Rオルガ戦勝利後無線機デモ終了)
}

block codec RTO_09b1R船倉無線機デモ１ 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080301
	eval($w:t_story = d:ST_T09b1R船倉無線機デモ１終了)
}

//うほほ
block codec RTO_10b1R船倉無線機デモ２ 14112 d:顔その1 {
	@rto_デフォルト前処理
//	@vc080402
	@vc080401
	@vc080402	//むび
	@vc080403
	@vc080404	//むび
	@vc080405
	

	eval($w:t_story = d:ST_T10b1R船倉無線機デモ２終了)
}




/*-------------------------------------------------------
でばっぐす
--------------------------------------------------------*/
block codec RTO_vc050101 14112 d:顔その1 {
	@rto_うなづきテスト１

	@rto_デフォルト前処理
tracecamera 0 1
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

// 以下は体験版後分
// 新規作成分
block codec RTO_vc050302 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc050302
}

// 新規作成分
block codec RTO_vc080101 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080101
}

// 新規作成分
block codec RTO_vc080201 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080201
}

// 新規作成分
block codec RTO_vc080301 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080301
}

// 新規作成分
block codec RTO_vc080401 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080401
}

// 新規作成分
block codec RTO_vc080402 14112 d:顔その1 {
	@rto_デフォルト前処理
	@vc080402
}

