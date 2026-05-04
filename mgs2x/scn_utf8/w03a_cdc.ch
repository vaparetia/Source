/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"



command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト 
@rt_オタコンＳＡＶＥ＿デフォルト無線セット

command マップ設定 左舷長廊下０ 左舷長廊下１ 左舷長廊下２ 左舷長廊下３ 中央長廊下 -set {
trap no_dr000 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr000'
			command 無線設定 \
				-over $3 14112 t:RTO_w03a_開かない水密扉dr000
	}

trap no_dr100 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr100'
			command 無線設定 \
				-over $3 14112 t:RTO_w03a_開かない水密扉dr100
	}

trap no_dr200 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr200'
			command 無線設定 \
				-over $3 14112 t:RTO_w03a_開かない水密扉dr200
	}


//---------------------------------
//北のエリア
//--------------------------------
//いちばんにし
	trap no_dr300 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr300'
			command 無線設定 \
				-over $3 14112 t:RTO_w03a_開かない水密扉dr300
	}
	trap no_dr300 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			eval($f:rft_w03a_開かない水密扉dr300開けた = 1)
	}

	//まんなかみまみかわ
	trap no_dr301 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr301'
			command 無線設定 \
				-over $3 14112 t:RTO_w03a_開かない水密扉dr301
	}
	trap no_dr301 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			eval($f:rft_w03a_開かない水密扉dr301開けた = 1)
	}

	//まんなかみまみかわ
	trap no_dr302 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr302'
			command 無線設定 \
				-over $3 14112 t:RTO_w03a_開かない水密扉dr302
	}
	trap no_dr302 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			eval($f:rft_w03a_開かない水密扉dr302開けた = 1)
	}


trap r_north_in d:PLAYER \
	-mask 入る \
	-exec {
print 'ぐばびりんちょ はいった'
		if(!$f:rft_w03a_長廊下北にいまし) {
			eval($f:rft_w03a_長廊下北にいまし = 1)
		}
	}

trap r_north_out d:PLAYER \
	-mask 入る \
	-exec {
print 'ぐばびりんちょ 出た'
		if($f:rft_w03a_長廊下北にいまし) {
			eval($f:rft_w03a_長廊下北にいまし = 0)
		}
	}

}


