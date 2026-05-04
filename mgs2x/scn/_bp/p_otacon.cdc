//
//	p_otacon.cdc
//	プラント編オタコン無線
//
//	2001/06/15	T.Fukushima
//	$Id: p_otacon.cdc,v 1.22 2002/06/07 09:53:01 usr01475 Exp $

/* メッセージ定義 */
#define CODEC_FILE 1
#include "vardef.h"			// 武器・アイテム関係のdefine
#include "cdc_proc_p.h"

#include "p_otacon_e.cm"
#include "p_otacon_e.ct"

#include "p_otacon_f.cm"
#include "p_otacon_f.ct"

#include "p_otacon_g.cm"
#include "p_otacon_g.ct"

#include "p_otacon_s.cm"
#include "p_otacon_s.ct"

#include "p_otacon_i.cm"
#include "p_otacon_i.ct"

#include "p_otacon_j.cm"
#include "p_otacon_j.ct"

proc rp_無線デフォルト前処理＿オタコン {
	@rp_無線デフォルト前処理
	if(`prefreq` != 14112){
		eval($w:rfp_callcount = 1)
	}
}

proc rpo_ばぐっち {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_ばぐっちおたた
}

proc rpo_PO_天狗通路Ｂ＿北に行け {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗通路Ｂ＿北に行け
}



proc rpo_PO_スネーク死んだら終わり {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_スネーク死んだら終わり
}

proc rpo_PO_天狗通路＿天狗兵 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	if(!$f:rfp_PO_天狗通路＿天狗兵聞いた) {
		@rp_PO_天狗通路＿天狗兵１
		@rp_PC_天狗通路＿天狗兵＿刀１
		@rp_PC_天狗通路＿天狗兵＿刀２
		eval($f:rfp_PO_天狗通路＿天狗兵聞いた = 1)
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PC_天狗通路＿天狗兵＿刀１
		}else {
			@rp_PC_天狗通路＿天狗兵＿刀２
		}
	}
}

proc rpo_PO_天狗通路Ａ＿メタル {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	//大佐会話と連携
	if(!$f:rfp_PC_天狗通路Ａ＿メタル聞いた){
		@rp_PO_天狗通路Ａ＿メタル１
		eval($f:rfp_PC_天狗通路Ａ＿メタル聞いた = 1)
	}
	@rp_PO_量産型ＲＡＹうんちく１
	eval($f:rfp_PO_量産型ＲＡＹうんちく１聞いた = 1)
	@rp_PO_天狗通路Ａ＿メタル２
}

proc rpo_PO_天狗兵＿Ｐ９０ {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗兵＿Ｐ９０１
	if(!$f:rfp_PO_天狗兵＿Ｐ９０聞いた){
		@rp_PO_天狗兵＿Ｐ９０２
		eval($f:rfp_PO_天狗兵＿Ｐ９０聞いた = 1)
	}
}

proc rpo_PO_天狗兵＿強化服 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗兵＿強化服
}

proc rpo_PO_大佐判明前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_大佐判明前
}


proc rpo_PO_天狗ラッシュ＿基本 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗ラッシュ＿基本
}

proc rpo_PO_天狗ラッシュ＿みねうち {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗ラッシュ＿みねうち
}


//ちょーかり。天狗会話等ＸＸＸＸＸＸＸＸＸＸＸＸｘｘ
proc rpo_連続無線オタコンデフォルト{
	@rpo_PO_天狗通路Ｂ＿北に行け
}


proc rpo_PO_天狗ラッシュ前 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗ラッシュ前
}

proc rpo_PO_メタル戦＿チャフ {
	if(!$f:rfp_PO_メタル戦＿チャフ聞いた) {
		@rp_PO_メタル戦＿チャフ１
		eval($f:rfp_PO_メタル戦＿チャフ聞いた = 1 )
	}
	@rp_PO_メタル戦＿チャフ２
}

proc rpo_PO_メタル戦＿きりがない {
	if(!$f:rfp_PO_メタル戦＿きりがない聞いた) {
		@rp_PO_メタル戦＿きりがない１
		eval($f:rfp_PO_メタル戦＿きりがない聞いた = 1 )
	}
	@rp_PO_メタル戦＿きりがない２
}

proc rpo_PO_メタル戦＿ミッソー {
	@rp_PO_メタル戦＿ミッソー１
	if(!$f:rfp_PO_メタル戦＿ミッソー聞いた) {
		@rp_PO_メタル戦＿ミッソー２
		eval($f:rfp_PO_メタル戦＿ミッソー聞いた = 1 )
	}
	@rp_PO_メタル戦＿ミッソー３
}

proc rpo_PO_ＲＡＹ戦＿口の中 {
	if(!$f:rfp_PO_ＲＡＹ戦＿口の中聞いた) {
		@rp_PO_ＲＡＹ戦＿口の中１
		@rp_PO_ＲＡＹ戦＿口の中２
		eval($f:rfp_PO_ＲＡＹ戦＿口の中聞いた = 1 )
	}
	@rp_PO_ＲＡＹ戦＿口の中３
}

proc rpo_PO_ソリダス戦＿加速装置 {
	if(!$f:rfp_PO_ソリダス戦＿加速装置聞いた) {
		@rp_PO_ソリダス戦＿加速装置１
		eval($f:rfp_PO_ソリダス戦＿加速装置聞いた = 1)
	}
	@rp_PO_ソリダス戦＿加速装置炎

}



proc rpo_メタルギアＲＡＹ戦中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	if(!$f:rfp_PO_メタル戦＿スティンガー聞いた) {
		@rp_PO_メタル戦＿スティンガー
		eval($f:rfp_PO_メタル戦＿スティンガー聞いた = 1 )
	
	}else if(!$f:rfp_PO_メタル戦＿チャフ聞いた) {
		@rpo_PO_メタル戦＿チャフ
		eval($f:rfp_PO_メタル戦＿チャフ聞いた = 1 )

	}else if(!$f:rfp_PO_メタル戦＿刀聞いた) {
		@rp_PO_メタル戦＿刀
		eval($f:rfp_PO_メタル戦＿刀聞いた = 1 )
	
	}else if(!$f:rfp_PO_メタル戦＿クイックチェンジ聞いた) {
		@rp_PO_メタル戦＿クイックチェンジ
		eval($f:rfp_PO_メタル戦＿クイックチェンジ聞いた = 1)
		
	}else if(!$f:rfp_PO_メタル戦＿踏み付け聞いた) {
		@rp_PO_メタル戦＿踏み付け
		eval($f:rfp_PO_メタル戦＿踏み付け聞いた = 1)
	
	//メタル倒したよＸＸＸＸＸＸＸＸＸＸＸＸｘ
	}else if( (!$f:rfp_PO_メタル戦＿きりがない聞いた) && ($i:w46a_メタル戦撃墜数 >= 3) ){
		@rpo_PO_メタル戦＿きりがない
		eval($f:rfp_PO_メタル戦＿きりがない聞いた = 1 )
/*
	}else if(!$f:rfp_PO_メタル戦＿ＲＧＢ６聞いた) {
		@rp_PO_メタル戦＿ＲＧＢ６
		eval($f:rfp_PO_メタル戦＿ＲＧＢ６聞いた = 1)
*/	
	}else if(!$f:rfp_PO_メタル戦＿ミッソー近く無効聞いた) {
		@rp_PO_メタル戦＿ミッソー近く無効
		eval($f:rfp_PO_メタル戦＿ミッソー近く無効聞いた = 1 )
/*
	}else if(!$f:rfp_PO_メタル戦＿ロックオン聞いた) {
		@rp_PO_メタル戦＿ロックオン
		eval($f:rfp_PO_メタル戦＿ロックオン聞いた = 1 )
*/
	}else if(!$f:rfp_PO_メタル戦＿側転聞いた) {
		@rp_PO_メタル戦＿側転
		eval($f:rfp_PO_メタル戦＿側転聞いた = 1)
/*
	//まじすか？ＸＸＸＸＸＸＸＸＸＸＸｘ
	}else if(!$f:rfp_PO_メタル戦＿法則聞いた) {
		@rp_PO_メタル戦＿法則
		eval($f:rfp_PO_メタル戦＿法則聞いた = 1 )
	
	}else if(!$f:rfp_PO_メタル戦＿隙を突け聞いた) {
		@rp_PO_メタル戦＿隙を突け
		eval($f:rfp_PO_メタル戦＿隙を突け聞いた = 1 )
*/	
	}else if(!$f:rfp_PO_メタル戦＿小回りきかない聞いた) {
		@rp_PO_メタル戦＿小回りきかない
		eval($f:rfp_PO_メタル戦＿小回りきかない聞いた = 1)
	
	}else if(!$f:rfp_PO_メタル戦＿ミッソー聞いた) {
		@rpo_PO_メタル戦＿ミッソー
		eval($f:rfp_PO_メタル戦＿ミッソー聞いた = 1 )
	
	}else if(!$f:rfp_PO_ＲＡＹ戦＿口の中聞いた) {
		@rpo_PO_ＲＡＹ戦＿口の中
		eval($f:rfp_PO_ＲＡＹ戦＿口の中聞いた = 1 )
	
	}else if(!$f:rfp_PO_量産型ＲＡＹうんちく１聞いた){
		@rp_PO_量産型ＲＡＹうんちく１
		eval($f:rfp_PO_量産型ＲＡＹうんちく１聞いた = 1)
	
	}else if(!$f:rfp_PO_メタル戦＿人工筋肉聞いた) {
		@rp_PO_メタル戦＿人工筋肉
		eval($f:rfp_PO_メタル戦＿人工筋肉聞いた = 1 )
	
	}else if(!$f:rfp_PO_メタル戦＿装甲聞いた) {
		@rp_PO_メタル戦＿装甲
		eval($f:rfp_PO_メタル戦＿装甲聞いた = 1)

	}else {
		rand 11
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PO_メタル戦＿スティンガー

		}else if($w:rfp_rand < 2){
			@rpo_PO_メタル戦＿チャフ

		}else if($w:rfp_rand < 3){
			@rp_PO_メタル戦＿刀

		}else if($w:rfp_rand < 4){
			@rp_PO_メタル戦＿クイックチェンジ

		}else if($w:rfp_rand < 5){
			@rp_PO_メタル戦＿踏み付け
		
		}else if( ($w:rfp_rand < 6) && ($i:w46a_メタル戦撃墜数 >= 3) ){
			//メタル倒したよＸＸＸＸＸＸＸＸＸＸＸＸｘ
			@rpo_PO_メタル戦＿きりがない
/*
		}else if($w:rfp_rand < 7){
			@rp_PO_メタル戦＿ＲＧＢ６
*/

		}else if($w:rfp_rand < 7){
			@rp_PO_メタル戦＿ミッソー近く無効

/*
		}else if($w:rfp_rand < 9){
			@rp_PO_メタル戦＿ロックオン
*/

		}else if($w:rfp_rand < 8){
			@rp_PO_メタル戦＿側転
/*
		}else if($w:rfp_rand < 10){
			//まじすか？ＸＸＸＸＸＸＸＸＸＸＸｘ
			@rp_PO_メタル戦＿法則

		}else if($w:rfp_rand < 11){
			@rp_PO_メタル戦＿隙を突け
*/
		}else if($w:rfp_rand < 9){
			@rp_PO_メタル戦＿小回りきかない

		}else if($w:rfp_rand < 10){
			@rpo_PO_ＲＡＹ戦＿口の中

		}else{
			@rpo_PO_メタル戦＿ミッソー
		}
	}
}


proc rpo_ソリダス戦中 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	if(!$f:rfp_PO_ソリダス戦＿刀聞いた) {
		@rp_PO_ソリダス戦＿刀
		eval($f:rfp_PO_ソリダス戦＿刀聞いた = 1)
		
	}else if( (!$f:rfp_PO_ソリダス戦＿蛇手聞いた) && \
				(!$f:rfp_w61a_ソリダス蛇手外れました) ){
		@rp_PO_ソリダス戦＿蛇手
		eval($f:rfp_PO_ソリダス戦＿蛇手聞いた = 1 )

	}else if( (!$f:rfp_PO_ソリダス戦＿蛇手ミッソー聞いた) && \
				(!$f:rfp_w61a_ソリダス蛇手外れました) ){
		@rp_PO_ソリダス戦＿蛇手ミッソー
		eval($f:rfp_PO_ソリダス戦＿蛇手ミッソー聞いた = 1 )
	
	}else if(!$f:rfp_PO_ソリダス戦＿正面攻撃無効聞いた) {
		@rp_PO_ソリダス戦＿正面攻撃無効
		eval($f:rfp_PO_ソリダス戦＿正面攻撃無効聞いた = 1 )
	
	}else if(!$f:rfp_PO_ソリダス戦＿接近しすぎ注意聞いた) {
		@rp_PO_ソリダス戦＿接近しすぎ注意
		eval($f:rfp_PO_ソリダス戦＿接近しすぎ注意聞いた = 1)
	
	}else if(!$f:rfp_PO_ソリダス戦＿突き有効聞いた) {
		@rp_PO_ソリダス戦＿突き有効
		eval($f:rfp_PO_ソリダス戦＿突き有効聞いた = 1 )
	
	}else if(!$f:rfp_PO_ソリダス戦＿加速装置聞いた) {
		@rpo_PO_ソリダス戦＿加速装置
		eval($f:rfp_PO_ソリダス戦＿加速装置聞いた = 1)
		
	}else if(!$f:rfp_PO_ソリダス戦＿死角聞いた) {
		@rp_PO_ソリダス戦＿死角
		eval($f:rfp_PO_ソリダス戦＿死角聞いた = 1 )
/*
	}else if(!$f:rfp_PO_ソリダス戦＿ローリング回避聞いた) {
		@rp_PO_ソリダス戦＿ローリング回避
		eval($f:rfp_PO_ソリダス戦＿ローリング回避聞いた = 1 )
	}else if(!$f:rfp_PO_ソリダス戦＿ソリダス防御聞いた) {
		@rp_PO_ソリダス戦＿ソリダス防御
		eval($f:rfp_PO_ソリダス戦＿ソリダス防御聞いた = 1 )
*/	
	
	}else if(!$f:rfp_PO_ソリダス戦＿マッスルスーツうんちく１聞いた) {
		@rp_PO_ソリダス戦＿マッスルスーツうんちく１
		eval($f:rfp_PO_ソリダス戦＿マッスルスーツうんちく１聞いた = 1 )

	}else {
		rand 9
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PO_ソリダス戦＿刀

		}else if( ($w:rfp_rand < 2) && \
				(!$f:rfp_w61a_ソリダス蛇手外れました) ){
			@rp_PO_ソリダス戦＿蛇手

		}else if( ($w:rfp_rand < 3) && \
				(!$f:rfp_w61a_ソリダス蛇手外れました) ){
			@rp_PO_ソリダス戦＿蛇手ミッソー

		}else if($w:rfp_rand < 4){
			@rp_PO_ソリダス戦＿正面攻撃無効

		}else if($w:rfp_rand < 5){
			@rp_PO_ソリダス戦＿接近しすぎ注意

		}else if($w:rfp_rand < 6){
			@rp_PO_ソリダス戦＿突き有効

		}else if($w:rfp_rand < 7){
			@rpo_PO_ソリダス戦＿加速装置

/*
		}else if($w:rfp_rand < 8){
			@rp_PO_ソリダス戦＿ソリダス防御

		}else if($w:rfp_rand < 9){
			@rp_PO_ソリダス戦＿ローリング回避
*/
		}else {
			@rp_PO_ソリダス戦＿死角
		}
	}
}

proc rpo_オタコンデフォルト {
	if(d:ST:P075_01_R01オタコン無事１無線機デモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P077_02_P01天狗兵降下ラッシュポリゴンデモ１開始 ){

		if($s:エリア == "w44a") {
			//天狗兵全滅なら1
			if($w:Ｓローカル変数Ｗ１ >= 1){
				@rpo_PO_天狗通路Ｂ＿北に行け
			}else {
				if(!$f:rfp_PO_天狗通路Ｂ＿北に行け聞いた) {
					@rpo_PO_天狗通路Ｂ＿北に行け
					eval($f:rfp_PO_天狗通路Ｂ＿北に行け聞いた = 1 )
				
				}else if(!$f:rfp_PO_スネーク死んだら終わり聞いた){
					@rpo_PO_スネーク死んだら終わり
					eval($f:rfp_PO_スネーク死んだら終わり聞いた = 1)

				}else if(!$f:rfp_PO_天狗通路＿天狗兵聞いた) {
					@rpo_PO_天狗通路＿天狗兵
					eval($f:rfp_PO_天狗通路＿天狗兵聞いた = 1)

				}else if(!$f:rfp_PO_天狗通路Ａ＿メタル聞いた) {
					@rpo_PO_天狗通路Ａ＿メタル
					eval($f:rfp_PO_天狗通路Ａ＿メタル聞いた = 1)

				}else if(!$f:rfp_PO_天狗兵＿Ｐ９０聞いた){
					@rpo_PO_天狗兵＿Ｐ９０
					eval($f:rfp_PO_天狗兵＿Ｐ９０聞いた = 1)

				//連続
				}else if( ($w:rfp_callcount > 1) && (!$f:rfp_PO_天狗兵＿強化服聞いた) ){
					@rpo_PO_天狗兵＿強化服
					eval($f:rfp_PO_天狗兵＿強化服聞いた = 1)

				}else if( ($w:rfp_callcount > 1) && (!$f:rfp_PO_大佐判明前聞いた) ){
					@rpo_PO_大佐判明前
					eval($f:rfp_PO_大佐判明前聞いた = 1)

				//無線一回目
				}else {
					rand 4
					eval($w:rfp_rand = $status)

					if($w:rfp_rand < 2){
						@rpo_PO_天狗通路Ｂ＿北に行け

					}else if($w:rfp_rand < 3){
						@rpo_PO_天狗通路＿天狗兵

					}else {
						@rpo_PO_スネーク死んだら終わり
					}
				}
			}
			
		}else if($s:エリア == "w45a") {
				@rpo_PO_天狗ラッシュ前
//				eval($f:rfp_PO_天狗ラッシュ前聞いた = 1)

		}else{
			@rpo_ばぐっち
		}

	}else if(d:ST:P077_02_P01天狗兵降下ラッシュポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P078_01_P01ＡＧフォーチュン登場１ポリゴンデモ１開始 ){

		if(!$f:rfp_PO_天狗ラッシュ＿基本聞いた){
			@rpo_PO_天狗ラッシュ＿基本
			eval($f:rfp_PO_天狗ラッシュ＿基本聞いた = 1 )
		
		}else if(!$f:rfp_PO_天狗通路＿天狗兵聞いた) {
				@rpo_PO_天狗通路＿天狗兵
				eval($f:rfp_PO_天狗通路＿天狗兵聞いた = 1)

		}else if(!$f:rfp_PO_天狗ラッシュ＿みねうち聞いた) {
			@rpo_PO_天狗ラッシュ＿みねうち
			eval($f:rfp_PO_天狗ラッシュ＿みねうち聞いた = 1 )

		}else if(!$f:rfp_PO_天狗兵＿Ｐ９０聞いた){
			@rpo_PO_天狗兵＿Ｐ９０
			eval($f:rfp_PO_天狗兵＿Ｐ９０聞いた = 1)

		//連続
		}else if( ($w:rfp_callcount > 1) && (!$f:rfp_PO_天狗兵＿強化服聞いた) ){
			@rpo_PO_天狗兵＿強化服
			eval($f:rfp_PO_天狗兵＿強化服聞いた = 1)

		}else {
			rand 3
			eval($w:rfp_rand = $status)

			if($w:rfp_rand < 1){
				@rpo_PO_天狗ラッシュ＿基本

			}else if($w:rfp_rand < 2){
				@rpo_PO_天狗通路＿天狗兵

			}else {
				@rpo_PO_天狗ラッシュ＿みねうち
			}
		}

	}else if(d:ST:P079_03_P02ＲＡＹ戦前３ポリゴンデモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:P080_01_P01ＡＧ浮上１ポリゴンデモ１開始){

		@rpo_メタルギアＲＡＹ戦中


	}else if(d:ST:P080_28_P12ＡＧ浮上２８ポリゴンデモ１２終了 <= $w:p_story && \
			$w:p_story < d:ST:P082_01_P01エンディング１ポリゴンデモ１開始){
		@rpo_ソリダス戦中


	}else {
		@rpo_ばぐっち
	}
}





block codec RPO_オタコンデフォルト 14112 d:スネーク無線１ {
	@rp_無線デフォルト前処理＿オタコン
	@rpo_オタコンデフォルト
}


proc rpo_PO_天狗通路Ｂ＿戻れない {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗通路Ｂ＿戻れない
}

block codec RPO_w44a_南開かない扉 14112 d:スネーク無線１ {
	@rp_無線デフォルト前処理＿オタコン
	if(!$f:rfp_PO_天狗通路Ｂ＿戻れない聞いた){
		@rpo_PO_天狗通路Ｂ＿戻れない
		eval($f:rfp_PO_天狗通路Ｂ＿戻れない聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if($w:rfp_rand < 3){
			@rpo_PO_天狗通路Ｂ＿戻れない
		}else {
		@rpo_オタコンデフォルト
		}

	//連続無線
	}else {
		@rpo_オタコンデフォルト
	}
}

proc rpo_PO_天狗通路ドア全滅 {
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_天狗通路ドア全滅
}

block codec RPO_w44a_天狗通路ドア扉 14112 d:スネーク無線１ {
	@rp_無線デフォルト前処理＿オタコン
	if( (!$f:rfp_PO_天狗通路ドア全滅聞いた) && (!$f:ローカルフラグ２) ){
		@rpo_PO_天狗通路ドア全滅
		eval($f:rfp_PO_天狗通路ドア全滅聞いた = 1 )

	//無線一回目
	}else if($w:rfp_callcount <= 1){
		rand 4
		eval($w:rfp_rand = $status)
		if( ($w:rfp_rand < 3) && (!$f:ローカルフラグ２) ){
			@rpo_PO_天狗通路ドア全滅
		}else {
			@rpo_オタコンデフォルト
		}

	//連続無線
	}else {
		@rpo_オタコンデフォルト
	}
}

block codec RPO_w61a_ソリダス戦救済ＣＡＬＬ 14112 d:スネーク無線１ {
	@rp_無線デフォルト前処理＿オタコン
	@rpd_ライデン顔設定デフォルト
	@rpd_オタコン顔設定１
	@rpd_顔表示

	@rp_PO_ソリダス戦＿正面攻撃無効
	@rp_PO_ソリダス戦＿死角
}


//モーション班用デバッグスクリプトェウァアアアア
#if d:MGS2_SCN
#include "p_otacon_dbg.h"
#endif

