//
//	p_emma.cdc
//	プラント編エマ無線
//
//	2001/06/15	T.Fukushima
//	$Id: p_emma.cdc,v 1.20 2002/06/07 09:53:01 usr01475 Exp $

/* メッセージ定義 */
#define CODEC_FILE 1
#include "vardef.h"			// 武器・アイテム関係のdefine
#include "cdc_proc_p.h"

#include "p_emma_e.cm"
#include "p_emma_e.ct"
#include "p_snake_e.cm"
#include "p_snake_e.ct"

#include "p_emma_f.cm"
#include "p_emma_f.ct"
#include "p_snake_f.cm"
#include "p_snake_f.ct"

#include "p_emma_g.cm"
#include "p_emma_g.ct"
#include "p_snake_g.cm"
#include "p_snake_g.ct"

#include "p_emma_s.cm"
#include "p_emma_s.ct"
#include "p_snake_s.cm"
#include "p_snake_s.ct"

#include "p_emma_i.cm"
#include "p_emma_i.ct"
#include "p_snake_i.cm"
#include "p_snake_i.ct"

#include "p_emma_j.cm"
#include "p_emma_j.ct"
#include "p_snake_j.cm"
#include "p_snake_j.ct"

proc rp_無線デフォルト前処理＿エマ {
	@rp_無線デフォルト前処理
	if(`prefreq` != 14152){
		eval($w:rfp_callcount = 1)
	}
}

proc rpe_ばぐっち {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	@rp_PE_ばぐっち
}

proc rpe_エマ水中＿普通 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_水中＿普通１聞いた) {
		@rp_PE_水中＿普通１
		eval($f:rfp_PE_水中＿普通１聞いた = 1 )
	
	}else if(!$f:rfp_PE_水中＿普通２聞いた) {
		@rp_PE_水中＿普通２
		eval($f:rfp_PE_水中＿普通２聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿普通３聞いた) {
		@rp_PE_水中＿普通３
		eval($f:rfp_PE_水中＿普通３聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿普通４聞いた) {
		@rp_PE_水中＿普通４
		eval($f:rfp_PE_水中＿普通４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_水中＿普通１

		}else if($w:rfp_rand < 2){
			@rp_PE_水中＿普通２
		}else if($w:rfp_rand < 3){
			@rp_PE_水中＿普通３
		}else {
			@rp_PE_水中＿普通４
		}
	}

}

proc rpe_エマ水中＿Ｏ２少ない {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_水中＿Ｏ２少ない１聞いた) {
		@rp_PE_水中＿Ｏ２少ない１
		eval($f:rfp_PE_水中＿Ｏ２少ない１聞いた = 1 )
	
	}else if(!$f:rfp_PE_水中＿Ｏ２少ない２聞いた) {
		@rp_PE_水中＿Ｏ２少ない２
		eval($f:rfp_PE_水中＿Ｏ２少ない２聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿Ｏ２少ない３聞いた) {
		@rp_PE_水中＿Ｏ２少ない３
		eval($f:rfp_PE_水中＿Ｏ２少ない３聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿Ｏ２少ない４聞いた) {
		@rp_PE_水中＿Ｏ２少ない４
		eval($f:rfp_PE_水中＿Ｏ２少ない４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_水中＿Ｏ２少ない１

		}else if($w:rfp_rand < 2){
			@rp_PE_水中＿Ｏ２少ない２
		}else if($w:rfp_rand < 3){
			@rp_PE_水中＿Ｏ２少ない３
		}else {
			@rp_PE_水中＿Ｏ２少ない４
		}
	}
}

proc rpe_エマ水中＿ダメージ{
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_水中＿ダメージ１聞いた) {
		@rp_PE_水中＿ダメージ１
		eval($f:rfp_PE_水中＿ダメージ１聞いた = 1 )
	
	}else if(!$f:rfp_PE_水中＿ダメージ２聞いた) {
		@rp_PE_水中＿ダメージ２
		eval($f:rfp_PE_水中＿ダメージ２聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿ダメージ３聞いた) {
		@rp_PE_水中＿ダメージ３
		eval($f:rfp_PE_水中＿ダメージ３聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿ダメージ４聞いた) {
		@rp_PE_水中＿ダメージ４
		eval($f:rfp_PE_水中＿ダメージ４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_水中＿ダメージ１

		}else if($w:rfp_rand < 2){
			@rp_PE_水中＿ダメージ２
		}else if($w:rfp_rand < 3){
			@rp_PE_水中＿ダメージ３
		}else {
			@rp_PE_水中＿ダメージ４
		}
	}
}

proc rpe_エマ水中＿息継ぎ {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_水中＿息継ぎ１聞いた) {
		@rp_PE_水中＿息継ぎ１
		eval($f:rfp_PE_水中＿息継ぎ１聞いた = 1 )
	
	}else if(!$f:rfp_PE_水中＿息継ぎ２聞いた) {
		@rp_PE_水中＿息継ぎ２
		eval($f:rfp_PE_水中＿息継ぎ２聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿息継ぎ３聞いた) {
		@rp_PE_水中＿息継ぎ３
		eval($f:rfp_PE_水中＿息継ぎ３聞いた = 1 )

	}else if(!$f:rfp_PE_水中＿息継ぎ４聞いた) {
		@rp_PE_水中＿息継ぎ４
		eval($f:rfp_PE_水中＿息継ぎ４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_水中＿息継ぎ１

		}else if($w:rfp_rand < 2){
			@rp_PE_水中＿息継ぎ２
		}else if($w:rfp_rand < 3){
			@rp_PE_水中＿息継ぎ３
		}else {
			@rp_PE_水中＿息継ぎ４
		}
	}
}

proc rpe_エマ＿手つなぎ＿ダメージ {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿ダメージ１聞いた) {
		@rp_PE_手つなぎ＿ダメージ１
		eval($f:rfp_PE_手つなぎ＿ダメージ１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿ダメージ２聞いた) {
		@rp_PE_手つなぎ＿ダメージ２
		eval($f:rfp_PE_手つなぎ＿ダメージ２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿ダメージ３聞いた) {
		@rp_PE_手つなぎ＿ダメージ３
		eval($f:rfp_PE_手つなぎ＿ダメージ３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿ダメージ４聞いた) {
		@rp_PE_手つなぎ＿ダメージ４
		eval($f:rfp_PE_手つなぎ＿ダメージ４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿ダメージ１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿ダメージ２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿ダメージ３
		}else {
			@rp_PE_手つなぎ＿ダメージ４
		}
	}
}

proc rpe_エマ＿手つなぎ＿寝てる {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿寝てる１聞いた) {
		@rp_PE_手つなぎ＿寝てる１
		eval($f:rfp_PE_手つなぎ＿寝てる１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿寝てる２聞いた) {
		@rp_PE_手つなぎ＿寝てる２
		eval($f:rfp_PE_手つなぎ＿寝てる２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿寝てる３聞いた) {
		@rp_PE_手つなぎ＿寝てる３
		eval($f:rfp_PE_手つなぎ＿寝てる３聞いた = 1 )

	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿寝てる１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿寝てる２

		}else {
			@rp_PE_手つなぎ＿寝てる３
		}
	}
}

proc rpe_エマ＿手つなぎ＿気絶 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	@rp_PE_手つなぎ＿気絶

}

proc rpe_エマ＿手つなぎ＿殴られた {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿殴られた１聞いた) {
		@rp_PE_手つなぎ＿殴られた１
		eval($f:rfp_PE_手つなぎ＿殴られた１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿殴られた２聞いた) {
		@rp_PE_手つなぎ＿殴られた２
		eval($f:rfp_PE_手つなぎ＿殴られた２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿殴られた３聞いた) {
		@rp_PE_手つなぎ＿殴られた３
		eval($f:rfp_PE_手つなぎ＿殴られた３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿殴られた４聞いた) {
		@rp_PE_手つなぎ＿殴られた４
		eval($f:rfp_PE_手つなぎ＿殴られた４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿殴られた１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿殴られた２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿殴られた３
		}else {
			@rp_PE_手つなぎ＿殴られた４
		}
	}
}

proc rpe_エマ＿手つなぎ＿ＬＩＦＥ少ない {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない１聞いた) {
		@rp_PE_手つなぎ＿ＬＩＦＥ少ない１
		eval($f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない２聞いた) {
		@rp_PE_手つなぎ＿ＬＩＦＥ少ない２
		eval($f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない３聞いた) {
		@rp_PE_手つなぎ＿ＬＩＦＥ少ない３
		eval($f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない４聞いた) {
		@rp_PE_手つなぎ＿ＬＩＦＥ少ない４
		eval($f:rfp_PE_手つなぎ＿ＬＩＦＥ少ない４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿ＬＩＦＥ少ない１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿ＬＩＦＥ少ない２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿ＬＩＦＥ少ない３
		}else {
			@rp_PE_手つなぎ＿ＬＩＦＥ少ない４
		}
	}
}

proc rpe_エマ＿手つなぎ＿座り込み {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿座り込み１聞いた) {
		@rp_PE_手つなぎ＿座り込み１
		eval($f:rfp_PE_手つなぎ＿座り込み１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿座り込み２聞いた) {
		@rp_PE_手つなぎ＿座り込み２
		eval($f:rfp_PE_手つなぎ＿座り込み２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿座り込み３聞いた) {
		@rp_PE_手つなぎ＿座り込み３
		eval($f:rfp_PE_手つなぎ＿座り込み３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿座り込み４聞いた) {
		@rp_PE_手つなぎ＿座り込み４
		eval($f:rfp_PE_手つなぎ＿座り込み４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿座り込み１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿座り込み２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿座り込み３
		}else {
			@rp_PE_手つなぎ＿座り込み４
		}
	}
}


proc rpe_エマ＿手つなぎ＿離れた {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿離れた１聞いた) {
		@rp_PE_手つなぎ＿離れた１
		eval($f:rfp_PE_手つなぎ＿離れた１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿離れた２聞いた) {
		@rp_PE_手つなぎ＿離れた２
		eval($f:rfp_PE_手つなぎ＿離れた２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿離れた３聞いた) {
		@rp_PE_手つなぎ＿離れた３
		eval($f:rfp_PE_手つなぎ＿離れた３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿離れた４聞いた) {
		@rp_PE_手つなぎ＿離れた４
		eval($f:rfp_PE_手つなぎ＿離れた４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿離れた１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿離れた２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿離れた３
		}else {
			@rp_PE_手つなぎ＿離れた４
		}
	}
}

proc rpe_エマ＿手つなぎ＿いたずら {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿いたずら１聞いた) {
		@rp_PE_手つなぎ＿いたずら１
		eval($f:rfp_PE_手つなぎ＿いたずら１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿いたずら２聞いた) {
		@rp_PE_手つなぎ＿いたずら２
		eval($f:rfp_PE_手つなぎ＿いたずら２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿いたずら３聞いた) {
		@rp_PE_手つなぎ＿いたずら３
		eval($f:rfp_PE_手つなぎ＿いたずら３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿いたずら４聞いた) {
		@rp_PE_手つなぎ＿いたずら４
		eval($f:rfp_PE_手つなぎ＿いたずら４聞いた = 1 )

	}else {
		rand 11
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿いたずら１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿いたずら２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿いたずら３
		}else if($w:rfp_rand < 4){
			@rp_PE_水中＿普通４
		}else if($w:rfp_rand < 5){
			@rp_PE_手つなぎ＿ダメージ４
		}else if($w:rfp_rand < 6){
			@rp_PE_手つなぎ＿殴られた１
		}else if($w:rfp_rand < 7){
			@rp_PE_手つなぎ＿殴られた３
		}else if($w:rfp_rand < 8){
			@rp_PE_手つなぎ＿殴られた４
		}else if($w:rfp_rand < 9){
			@rp_PE_手つなぎ＿虫４
		}else if($w:rfp_rand < 10){
			@rp_PE_狙撃＿ライデンに撃たれた３
		}else {
			@rp_PE_狙撃＿ライデンに撃たれた４
		}
	}
}

proc rpe_エマ＿手つなぎ＿虫 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_手つなぎ＿虫１聞いた) {
		@rp_PE_手つなぎ＿虫１
		eval($f:rfp_PE_手つなぎ＿虫１聞いた = 1 )
	
	}else if(!$f:rfp_PE_手つなぎ＿虫２聞いた) {
		@rp_PE_手つなぎ＿虫２
		eval($f:rfp_PE_手つなぎ＿虫２聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿虫３聞いた) {
		@rp_PE_手つなぎ＿虫３
		eval($f:rfp_PE_手つなぎ＿虫３聞いた = 1 )

	}else if(!$f:rfp_PE_手つなぎ＿虫４聞いた) {
		@rp_PE_手つなぎ＿虫４
		eval($f:rfp_PE_手つなぎ＿虫４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_手つなぎ＿虫１

		}else if($w:rfp_rand < 2){
			@rp_PE_手つなぎ＿虫２
		}else if($w:rfp_rand < 3){
			@rp_PE_手つなぎ＿虫３
		}else {
			@rp_PE_手つなぎ＿虫４
		}
	}
}


proc rpe_エマ＿狙撃＿普通 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_狙撃＿普通１聞いた) {
		@rp_PE_狙撃＿普通１
		eval($f:rfp_PE_狙撃＿普通１聞いた = 1 )
	
	}else if(!$f:rfp_PE_狙撃＿普通２聞いた) {
		@rp_PE_狙撃＿普通２
		eval($f:rfp_PE_狙撃＿普通２聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿普通３聞いた) {
		@rp_PE_狙撃＿普通３
		eval($f:rfp_PE_狙撃＿普通３聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿普通４聞いた) {
		@rp_PE_狙撃＿普通４
		eval($f:rfp_PE_狙撃＿普通４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_狙撃＿普通１

		}else if($w:rfp_rand < 2){
			@rp_PE_狙撃＿普通２
		}else if($w:rfp_rand < 3){
			@rp_PE_狙撃＿普通３
		}else {
			@rp_PE_狙撃＿普通４
		}
	}
}

proc rpe_PE_狙撃＿ジョニー後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	@rp_PE_狙撃＿ジョニー後
}

proc rpe_PE_狙撃＿悪口後 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	@rp_PE_狙撃＿悪口後
}


proc rpe_エマ＿狙撃＿ＬＩＦＥ少ない {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_狙撃＿ＬＩＦＥ少ない１聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ少ない１
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ少ない１聞いた = 1 )
	
	}else if(!$f:rfp_PE_狙撃＿ＬＩＦＥ少ない２聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ少ない２
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ少ない２聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ＬＩＦＥ少ない３聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ少ない３
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ少ない３聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ＬＩＦＥ少ない４聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ少ない４
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ少ない４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_狙撃＿ＬＩＦＥ少ない１

		}else if($w:rfp_rand < 2){
			@rp_PE_狙撃＿ＬＩＦＥ少ない２
			
		}else if($w:rfp_rand < 3){
			@rp_PE_狙撃＿ＬＩＦＥ少ない３
		}else {
			@rp_PE_狙撃＿ＬＩＦＥ少ない４
		}
	}
}

proc rpe_エマ＿狙撃＿ＬＩＦＥ回復 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_狙撃＿ＬＩＦＥ回復１聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ回復１
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ回復１聞いた = 1 )
	
	}else if(!$f:rfp_PE_狙撃＿ＬＩＦＥ回復２聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ回復２
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ回復２聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ＬＩＦＥ回復３聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ回復３
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ回復３聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ＬＩＦＥ回復４聞いた) {
		@rp_PE_狙撃＿ＬＩＦＥ回復４
		eval($f:rfp_PE_狙撃＿ＬＩＦＥ回復４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_狙撃＿ＬＩＦＥ回復１

		}else if($w:rfp_rand < 2){
			@rp_PE_狙撃＿ＬＩＦＥ回復２
		}else if($w:rfp_rand < 3){
			@rp_PE_狙撃＿ＬＩＦＥ回復３
		}else {
			@rp_PE_狙撃＿ＬＩＦＥ回復４
		}
	}
}

proc rpe_エマ＿狙撃＿ライデンに撃たれた {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_狙撃＿ライデンに撃たれた１聞いた) {
		@rp_PE_狙撃＿ライデンに撃たれた１
		eval($f:rfp_PE_狙撃＿ライデンに撃たれた１聞いた = 1 )
	
	}else if(!$f:rfp_PE_狙撃＿ライデンに撃たれた２聞いた) {
		@rp_PE_狙撃＿ライデンに撃たれた２
		eval($f:rfp_PE_狙撃＿ライデンに撃たれた２聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ライデンに撃たれた３聞いた) {
		@rp_PE_狙撃＿ライデンに撃たれた３
		eval($f:rfp_PE_狙撃＿ライデンに撃たれた３聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ライデンに撃たれた４聞いた) {
		@rp_PE_狙撃＿ライデンに撃たれた４
		eval($f:rfp_PE_狙撃＿ライデンに撃たれた４聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_狙撃＿ライデンに撃たれた１

		}else if($w:rfp_rand < 2){
			@rp_PE_狙撃＿ライデンに撃たれた２
		}else if($w:rfp_rand < 3){
			@rp_PE_狙撃＿ライデンに撃たれた３
		}else {
			@rp_PE_狙撃＿ライデンに撃たれた４
		}
	}
}

proc rpe_エマ＿狙撃＿ヴァンプ {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_狙撃＿ヴァンプ１聞いた) {
		@rp_PE_狙撃＿ヴァンプ１
		eval($f:rfp_PE_狙撃＿ヴァンプ１聞いた = 1 )
	
	}else if(!$f:rfp_PE_狙撃＿ヴァンプ２聞いた) {
		@rp_PE_狙撃＿ヴァンプ２
		eval($f:rfp_PE_狙撃＿ヴァンプ２聞いた = 1 )

	}else if(!$f:rfp_PE_狙撃＿ヴァンプ３聞いた) {
		@rp_PE_狙撃＿ヴァンプ３
		eval($f:rfp_PE_狙撃＿ヴァンプ３聞いた = 1 )

	}else {
		rand 4
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_狙撃＿ヴァンプ１

		}else if($w:rfp_rand < 2){
			@rp_PE_狙撃＿ヴァンプ２

		}else {
			@rp_PE_狙撃＿ヴァンプ３
		}
	}
}


proc rpe_水中 {
	//ふつー
	if(`%プレイヤー状態取得` & d:PFLAG_WATER_SURFACE){
		if(!$f:rfp_らくらく水面にいましゃ){
			@rpe_エマ水中＿息継ぎ
		}else {
			@rpe_手つなぎ＿普通
		}

	}else if(`%水中エマＯ２ゲット` < ($w:Ｏ２最大 / 6) ){
		@rpe_エマ水中＿Ｏ２少ない
	
	//水中エマダメージ状態判定専用コマンドＸＸＸ
	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_ATTKED_BY_OTHERS){
		@rpe_エマ水中＿ダメージ

	}else {
		@rpe_エマ水中＿普通
	}
}

proc rpe_PE_水中＿普通１ {
	@rp_PE_水中＿普通１
}

proc rpe_手つなぎ＿普通 {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_水中＿普通１聞いた){
		@rp_PE_水中＿普通１
		eval($f:rfp_PE_水中＿普通１聞いた = 1)
	
	}else if(!$f:rfp_PE_狙撃＿普通１聞いた){
		@rp_PE_狙撃＿普通１
		eval($f:rfp_PE_狙撃＿普通１聞いた = 1)
	
	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_水中＿普通１
		}else {
			@rp_PE_狙撃＿普通１
		}
	}
}

proc rpe_エマ救出後＿兄妹喧嘩前振り {
	if(!$f:rfp_PE_ひといき＿オタコン会話１前１聞いた){
		@rp_PE_ひといき＿オタコン会話１前１
		@rp_PE_ひといき＿オタコン会話１前２
		eval($f:rfp_PE_ひといき＿オタコン会話１前１聞いた = 1)

	}else if(!$f:rfp_PE_ひといき＿オタコン会話２前聞いた){
		@rp_PE_ひといき＿オタコン会話２前
		eval($f:rfp_PE_ひといき＿オタコン会話２前聞いた = 1)

	}else if(!$f:rfp_PE_ひといき＿オタコン会話３前聞いた){
		@rp_PE_ひといき＿オタコン会話３前
		eval($f:rfp_PE_ひといき＿オタコン会話３前聞いた = 1)
		
	}else {
		rand 3
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_ひといき＿オタコン会話１前１

		}else if($w:rfp_rand < 2){
			@rp_PE_ひといき＿オタコン会話２前

		}else {
			@rp_PE_ひといき＿オタコン会話３前
		}
	}
}

proc rpe_エマ救出後＿兄妹喧嘩２ {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	@rpe_エマ救出後＿兄妹喧嘩前振り
	@rpd_オタコン顔設定１
	@rpd_エマ顔表示プレイヤー側
	@rp_PS_エマ救出後＿兄妹喧嘩２＿２
}

proc rpe_PE_ＲＭＡ {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	@rp_PE_ＲＭＡ
}

proc rpe_エマ＿首絞められ {
	@rpd_ライデン顔設定デフォルト
	@rpd_エマ顔設定１
	@rpd_顔表示

	if(!$f:rfp_PE_エマ＿首絞められ１聞いた) {
		@rp_PE_狙撃＿ヴァンプ１
		eval($f:rfp_PE_エマ＿首絞められ１聞いた = 1 )
	
	}else if(!$f:rfp_PE_エマ＿首絞められ２聞いた) {
		@rp_PE_狙撃＿ヴァンプ２
		eval($f:rfp_PE_エマ＿首絞められ２聞いた = 1 )

	}else {
		rand 2
		eval($w:rfp_rand = $status)

		if($w:rfp_rand < 1){
			@rp_PE_狙撃＿ヴァンプ１

		}else {
			@rp_PE_狙撃＿ヴァンプ２
		}
	}
}

proc rpe_陸上 {
	command ゲットエマライフ $i:エマライフ現在値
	command ゲットエマ座標 $i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数

	if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_AM_SLEEPING){
		@rpe_エマ＿手つなぎ＿寝てる
		
	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_AM_FAINTED ){
		@rpe_エマ＿手つなぎ＿気絶

	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_AM_CAPTURED ){
		@rpe_エマ＿首絞められ


	}else if(`@rp_エマにセクハラ中`){
		@rpe_エマ＿手つなぎ＿いたずら

	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_ATTKED_BY_PLAYER ){
		@rpe_エマ＿手つなぎ＿殴られた

	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_ATTKED_BY_OTHERS ){
		@rpe_エマ＿手つなぎ＿ダメージ


	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_DISLIKE_WORMS ){
		@rpe_エマ＿手つなぎ＿虫

	}else if(!`@rp_エマが近くにいます`) {
		@rpe_エマ＿手つなぎ＿離れた

	}else if( (`@rp_エマたん瀕死` ) || ($w:アラートモード == d:ALERT_MODE_ALERT) ){
		@rpe_エマ＿手つなぎ＿ＬＩＦＥ少ない

	//普通
	}else {
		//普通でかつ連続な感じ
		if( ($w:rfp_callcount > 1) && \
			(!$f:rfp_PS_プリスキン連絡途絶しましたん) && \
			($f:rfp_PS_エマ救出後＿兄妹喧嘩１聞いた) && \
			($w:p_story < d:ST:P063_01_P01カード五１ポリゴンデモ１開始) && \
			(!$f:rfp_PS_エマ救出後＿兄妹喧嘩２聞いた) ){
				@rpe_エマ救出後＿兄妹喧嘩２
				eval($f:rfp_PS_エマ救出後＿兄妹喧嘩２聞いた = 1)

		}else if( ($w:rfp_callcount > 1) && \
					(!$f:rfp_PE_ＲＭＡ聞いた) && \
					($w:p_story >= d:ST:P062_08_P04エマとＡＩ８ポリゴンデモ４終了) ){
				@rpe_PE_ＲＭＡ
				eval($f:rfp_PE_ＲＭＡ聞いた = 1)

		}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_SAT_DOWN){
			@rpe_エマ＿手つなぎ＿座り込み
		}else {
			@rpe_手つなぎ＿普通
		}
	}
}


proc rpe_エマ＿狙撃 {
	command ゲットエマライフ $i:エマライフ現在値
	if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_AM_SLEEPING){
		@rpe_エマ＿手つなぎ＿寝てる
	
	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_AM_FAINTED ){
		@rpe_エマ＿手つなぎ＿気絶

	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_ATTKED_BY_PLAYER ){
		@rpe_エマ＿狙撃＿ライデンに撃たれた

	}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_ATTKED_BY_OTHERS ){
		@rpe_エマ＿手つなぎ＿ダメージ

	}else if( (`@rp_エマたん瀕死`) || ($f:w32a_危険ＢＧＭフラグ) ){
		@rpe_エマ＿狙撃＿ＬＩＦＥ少ない

	}else {
		//普通でかつ連続な感じ
		if( ($f:rfp_w32a_エマ音声イベントジョニー聞いた) && \
			(!$f:rfp_PE_狙撃＿ジョニー後聞いた) ){
				@rpe_PE_狙撃＿ジョニー後
				eval($f:rfp_PE_狙撃＿ジョニー後聞いた = 1)

		}else if( ($f:rfp_w32a_エマ音声イベント悪口聞いた) && \
			(!$f:rfp_PE_狙撃＿悪口後聞いた) ){
				@rpe_PE_狙撃＿悪口後
				eval($f:rfp_PE_狙撃＿悪口後聞いた = 1)
		
		}else if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_SAT_DOWN){
			@rpe_エマ＿狙撃＿ＬＩＦＥ回復
		}else {
			@rpe_エマ＿狙撃＿普通
		}
	}
}

proc rpe_エマデフォルト {
	if(d:ST:P058_07_R03エマ救出７無線機デモ３終了 <= $w:p_story && \
			$w:p_story < d:ST:P065_06_R02Ｌ脚エマ６無線機デモ２終了 ){
		if($f:エマと一緒に水の中){
			@rpe_水中
		}else {
			@rpe_陸上
		}

	}else if(d:ST:P065_06_R02Ｌ脚エマ６無線機デモ２終了 <= $w:p_story && \
			$w:p_story < d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１開始){
		@rpe_エマ＿狙撃

	}else if(d:ST:P068_01_P01ヴァンプ狙撃前１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story < d:ST:P069_01_P01ヴァンプ狙撃終了１ポリゴンデモ１開始){
		@rpe_エマ＿狙撃＿ヴァンプ

	}else {
		@rpe_ばぐっち
	}
}

block codec RPE_エマデフォルト 14152 d:プリスキン無線１ {
	@rp_無線デフォルト前処理＿エマ
	@rp_短縮ダイアル＿エマセット
	command ゲットエマ状態 $b:rfp_ゲットされたエマ状態
	@rpe_エマデフォルト
}







//モーション班用デバッグスクリプトェウァアアアア
#if d:MGS2_SCN
#include "p_emma_dbg.h"
#endif
