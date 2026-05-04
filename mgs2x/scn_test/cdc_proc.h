//
//	cdc_proc.h
//	無線スクリプト共通関数用
//	.cdcの中で使う共通関数を書いてあるのではなかろうか？
//	2000/11/23	T.Fukushima
//	$Id: cdc_proc.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $

//欧州版
#ifdef d:PAL
	#ifdef d:SPANISH
		#include "menu_s.cm"
		#include "menu_s.ct"

	#elseifdef d:ITALIAN
		#include "menu_i.cm"
		#include "menu_i.ct"

	#elseifdef d:EU
		#include "menu_e.cm"
		#include "menu_e.ct"

		#include "menu_f.cm"
		#include "menu_f.ct"

		#include "menu_g.cm"
		#include "menu_g.ct"

	#else
		#include "menu_e.cm"
		#include "menu_e.ct"

		#include "menu_f.cm"
		#include "menu_f.ct"

		#include "menu_g.cm"
		#include "menu_g.ct"

		#include "menu_s.cm"
		#include "menu_s.ct"

		#include "menu_i.cm"
		#include "menu_i.ct"
	#endif
#else

//日本語版
#ifdef JAPANESE
#include "menu_j.cm"
#include "menu_j.ct"
#endif

//英語版
#ifdef ENGLISH
#include "menu_e.cm"
#include "menu_e.ct"
#endif

#endif


#include "p_face.h"

//仮のすけ
#define 顔その1	t:f00a

proc rto_顔設定デフォルト {
	// スネーク設定
	face       1 sna_radio_mh_mt スネーク
	lightvec   1 -30  -35  -40
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
	facecamera 1 \
	  994500   \   // far
	  11100    \   // zoom
	  59       \   // heading
	  -3750    \   // pan
	  86       \   // pitch
	  0            // gain
	facelimit 1 40000 512 512 113 170


	// オタコン設定
	face       0 otc_radio_mh_mt オタコン
	lightvec   0 30  -35  -40
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
	facecamera 0 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
	facelimit 0 40000 512 512 113 170


	object 	0 \
			otc_glasses_mt \
			オタコン \
			0 \
			12


//	@rpd_オタコン顔設定１

	facedelay 0 75 75
	facedelay 1 75 75
}

proc rto_モーション設定デフォルト {
	defmtn 0 cdc_otacon // オタコンデフォルトモーション
	defmtn 1 cdc_snake // スネークデフォルトモーション
	dispen    // 両者の顔表示を開始する
}

proc rto_連続無線回数インクリメント {
	callcount
	eval($w:rft_callcount = $status)
}

proc rto_デフォルト前処理 {
//	@rto_顔設定デフォルト
//	@rto_モーション設定デフォルト
	@rt_タンカースネーク顔設定１
	@rt_タンカーオタコン顔設定１
	@rpd_顔表示
	player スネーク
	@rto_連続無線回数インクリメント
}




/*
proc rt_ＳＡＶＥ実体 {
print 'ぶったらるよん'
	savemode
	eval($w:rft_ＳＡＶＥ返り値 = $status)
}
*/

proc rt_うなづきデフォルト＿是１ {
	mindvoice 	0 \	//是
				t:vc122501 \	//スネーク　「ふむ」
				t:vc122505 \	//スネーク　「なるほど」
				t:vc122509 \	//スネーク　「そうだ」
				t:vc122513 	//スネーク　「その通りだ」
}

proc rt_うなづきデフォルト＿是２ {
	mindvoice 	0 \	//是
				t:vc122502 \	//スネーク　「そうか」
				t:vc122506 \	//スネーク　「総武線そうかもな」
				t:vc122510 \	//スネーク　「そうだな」
				t:vc122514 	//スネーク　「そうに違いない」
}

proc rt_うなづきデフォルト＿是３ {
	mindvoice 	0 \	//是
				t:vc122503 \	//スネーク　「ああ」
				t:vc122507 \	//スネーク　「わかった」
				t:vc122511 \	//スネーク　「よし」
				t:vc122515 	//スネーク　「行くぞ！」
}

proc rt_うなづきデフォルト＿是４ {
	mindvoice 	0 \	//是
				t:vc122504 \	//スネーク　「ほぅ」
				t:vc122508 \	//スネーク　「うむ」
				t:vc122512 \	//スネーク　「うむ！」
				t:vc122516 	//スネーク　「おお（賛成の意）」
}

proc rt_うなづきデフォルト＿否１ {
	mindvoice 	1 \	//否
				t:vc122517 \	//スネーク　「そうか？」
				t:vc122521 \	//スネーク　「そうかぁ？」
				t:vc122525 \	//スネーク　「違う」
				t:vc122529 	//スネーク　「そんなわけないだろ！」
}

proc rt_うなづきデフォルト＿否２ {
	mindvoice 	1 \	//否
				t:vc122518 \	//スネーク　「あ？」
				t:vc122522 \	//スネーク　「まさか……」
				t:vc122526 \	//スネーク　「そうじゃないだろ」
				t:vc122530 	//スネーク　「ふざけるな！」
}

proc rt_うなづきデフォルト＿否３ {
	mindvoice 	1 \	//否
				t:vc122519 \	//スネーク　「うーん……」
				t:vc122523 \	//スネーク　「いや」
				t:vc122527 \	//スネーク　「違うだろ」
				t:vc122531 	//スネーク　「絶対違う！」
}

proc rt_うなづきデフォルト＿否４ {
	mindvoice 	1 \	//否
				t:vc122520 \	//スネーク　「む？」
				t:vc122524 \	//スネーク　「えぇ？」
				t:vc122528 \	//スネーク　「あぁん」
				t:vc122532 	//スネーク　「ケッ」
}

proc rt_無茶指令＿否１＿何様だ系 {
	mindvoice 	1 \	//否
				t:vc122547 \	//スネーク　「偉そうだな……」
				t:vc122548 \	//スネーク　「何様のつもりだ？」
				t:vc122548 \	//スネーク　「何様のつもりだ？」
				t:vc122534 	//スネーク　「お前がやれ！」
}

proc rt_無茶指令＿否２＿軽く言ってくれる系 {
	mindvoice 	1 \	//否
				t:vc122555 \	//スネーク　「軽く言ってくれる……」
				t:vc122553 \	//スネーク　「勝手なことを……」
				t:vc122553 \	//スネーク　「勝手なことを……」
				t:vc122534 	//スネーク　「お前がやれ！」
}

proc rt_うんちく＿否１＿わからん系 {
	mindvoice 	1 \	//否
				t:vc122539 \	//スネーク　「何を言ってるんだ……？」
				t:vc122540 \	//スネーク　「わからん」
				t:vc122540 \	//スネーク　「わからん」
				t:vc122541 	//スネーク　「さっぱりわからん！」
}

proc rt_うんちく＿否２＿うんざり系 {
	mindvoice 	1 \	//否
				t:vc122542 \	//スネーク　「話が長いな……」
				t:vc122543 \	//スネーク　「どうでもいいことを……」
				t:vc122543 \	//スネーク　「どうでもいいことを……」
				t:vc122552 	//スネーク　「本当にわかってるのか？」
}

proc rt_うんちく＿是１＿詳しいな系 {
	mindvoice 	0 \	//是
				t:vc122501 \	//スネーク　「ふむ」
				t:vc122505 \	//スネーク　「なるほど」
				t:vc122556 \	//スネーク　「詳しいな……」
				t:vc122556 	//スネーク　「詳しいな……」
}

proc rt_疑惑＿否１＿信じられん系 {
	mindvoice 	1 \	//否
				t:vc122537 \	//スネーク　「嘘っぽいな……」
				t:vc122535 \	//スネーク　「いいかげんなことを……」
				t:vc122535 \	//スネーク　「いいかげんなことを……」
				t:vc122538 	//スネーク　「信じられん……」
}

proc rt_エロエロ＿是１＿たまららん系 {
	mindvoice 	0 \	//是
				t:vc122544 \	//スネーク　「うーん。たまらん」
				t:vc122544 \	//スネーク　「うーん。たまらん」
				t:vc122604 \	//スネーク　「……もっこり」
				t:vc122604	//スネーク　「……もっこり」
}

proc rt_つっこまれ＿是１＿なんで知ってるんだ系 {
	mindvoice 	0 \	//是
				t:vc122533 \	//スネーク　「なんで知ってるんだ？」
				t:vc122533 \	//スネーク　「なんで知ってるんだ？」
				t:vc122549 \	//スネーク　「見てたのか……？」
				t:vc122549 	//スネーク　「見てたのか……？」
}

proc rt_つっこまれ＿否１＿お前が言うな系 {
	mindvoice 	1 \	//否
				t:vc122550 \	//スネーク　「お前もな！」
				t:vc122545 \	//スネーク　「ほっとけ！」
				t:vc122545 \	//スネーク　「ほっとけ！」
				t:vc122551 	//スネーク　「お前が言うな！」
}

proc rt_キテレツさん＿否１＿なんだこいつ系 {
	mindvoice 	1 \	//否
				t:vc122524 \	//スネーク　「えぇ？」
				t:vc122539 \	//スネーク　「何を言ってるんだ……？」
				t:vc122539 \	//スネーク　「何を言ってるんだ……？」
				t:vc122554 	//スネーク　「なんだこいつ？」
}

proc rt_騙された＿否１＿騙したな系 {
	mindvoice 	1 \	//否
				t:vc122535 \	//スネーク　「いいかげんなことを……」
				t:vc122535 \	//スネーク　「いいかげんなことを……」
				t:vc122536 \	//スネーク　「騙したな……」
				t:vc122536 	//スネーク　「騙したな……」
}


proc rt_謎＿是１＿俺が主役だ系 {
	mindvoice 	0 \	//是
				t:vc122601 \	//スネーク　「俺が主人公だ」
				t:vc122602 \	//スネーク　「俺が主役だ！」
				t:vc122602 \	//スネーク　「俺が主役だ！」
				t:vc122603	//スネーク　「主役は俺だ」
}

proc rt_謎＿是２＿らりるれろ系 {
	mindvoice 	0 \	//是
				t:vc122605 \	//スネーク　「らりるれろ……」
				t:vc122606 \	//スネーク　「らりるれろ！」
				t:vc122606 \	//スネーク　「らりるれろ！」
				t:vc122607	//スネーク　「らりるれろぉ」
}

proc rt_うなづきデフォルト＿是{
	rand 9
	eval($w:rft_rand = $status)
	
	if( ($w:rft_rand < 1) && \
		( ($w:タンカー編クリア回数 >= 1) || \
			($w:プラント編クリア回数 >= 1) ) ){
		@rt_謎＿是１＿俺が主役だ系

	}else if( $w:rft_rand < 3 ) {
		@rt_うなづきデフォルト＿是１

	}else if( $w:rft_rand < 5 ) {
		@rt_うなづきデフォルト＿是２

	}else if( $w:rft_rand < 7 ) {
		@rt_うなづきデフォルト＿是３

	}else {
		@rt_うなづきデフォルト＿是４
	}
}

proc rt_うなづきデフォルト＿否 {
	rand 4
	eval($w:rft_rand = $status)
	
	if($w:rft_rand < 1){
		@rt_うなづきデフォルト＿否１

	}else if( $w:rft_rand < 2 ) {
		@rt_うなづきデフォルト＿否２

	}else if( $w:rft_rand < 3 ) {
		@rt_うなづきデフォルト＿否３

	}else {
		@rt_うなづきデフォルト＿否４
	}
}

proc rt_うなづきデフォルト {
	@rt_うなづきデフォルト＿是
	@rt_うなづきデフォルト＿否

}

proc rt_無茶指令＿否＿お前がやれ系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		rand 2
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1) {
			@rt_無茶指令＿否１＿何様だ系
		}else {
			@rt_無茶指令＿否２＿軽く言ってくれる系
		}
	}else {
		@rt_うなづきデフォルト＿否
	}
}

proc rt_うんちく＿否＿わからん系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		rand 2
		eval($w:rft_rand = $status)
		if($w:rft_rand < 1) {
			@rt_うんちく＿否１＿わからん系
		}else {
			@rt_うんちく＿否２＿うんざり系
		}
	}else {
		@rt_うなづきデフォルト＿否
	}
}

proc rt_疑惑＿否＿信じられん系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_疑惑＿否１＿信じられん系
	}else {
		@rt_うなづきデフォルト＿否
	}
}

proc rt_うんちく＿是＿詳しいな系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_うんちく＿是１＿詳しいな系
	}else {
		@rt_うなづきデフォルト＿是
	}
}

proc rt_エロエロ＿是＿たまららん系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_エロエロ＿是１＿たまららん系
	}else {
		@rt_うなづきデフォルト＿是
	}
}

proc rt_つっこまれ＿是＿なんで知ってるんだ系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_つっこまれ＿是１＿なんで知ってるんだ系
	}else {
		@rt_うなづきデフォルト＿是
	}
}

proc rt_つっこまれ＿否＿お前が言うな系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_つっこまれ＿否１＿お前が言うな系
	}else {
		@rt_うなづきデフォルト＿否
	}
}

proc rt_キテレツさん＿否＿なんだこいつ系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_キテレツさん＿否１＿なんだこいつ系
	}else {
		@rt_うなづきデフォルト＿否
	}
}

proc rt_騙された＿否＿騙したな系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if($w:rft_rand < $:特殊確率) {
		@rt_騙された＿否１＿騙したな系
	}else {
		@rt_うなづきデフォルト＿否
	}
}

proc rt_謎＿是＿らりるれろ系デフォルト $:特殊確率 {
	rand 100
	eval($w:rft_rand = $status)
	if( ($w:rft_rand < $:特殊確率) && \
		( ($w:タンカー編クリア回数 >= 1) || \
			($w:プラント編クリア回数 >= 1) ) ){
		@rt_謎＿是２＿らりるれろ系
	}else {
		@rt_うなづきデフォルト＿是
	}
}

