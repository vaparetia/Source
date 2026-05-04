/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト 
@rt_オタコンＳＡＶＥ＿デフォルト無線セット

command マップ設定 中央長廊下 右舷長廊下３ 右舷長廊下１ -set {

/*
trap no_dr000 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr000'
			command 無線設定 \
				-over $3 14112 t:RTO_w03b_開かない水密扉dr000
	}
*/

trap no_dr001 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr001'
//		@rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２
			command 無線設定 \
				-over $3 14112 t:RTO_w03b_開かない水密扉dr001
	}

trap no_dr100 d:PLAYER \
	-mask ? \
	-exec {
			print 'no_dr100'
			command 無線設定 \
				-over $3 14112 t:RTO_w03b_開かない水密扉dr100
	}

}


/*
proc rt_強制ＣＡＬＬ_T_10b1R船倉無線機デモ２ {
	command 無線設定 \
	-call 14112 t:RTO_10b1R船倉無線機デモ２ d:CODEC_DIRECT rt_強制ＣＡＬＬ_10b1R船倉無線機デモ２終了 \
	-f d:CODEC_I_FD_O_FD \
	-d
}

proc rt_強制ＣＡＬＬ_10b1R船倉無線機デモ２終了 {
	eval($w:t_story = d:ST_T10b1R船倉無線機デモ２終了)
	print '$w:t_story = d:ST_T10b1R船倉無線機デモ２終了'

	// ゲームに戻すにあたって０に。
	eval($w:船倉兵モード = 0)
	eval($s:d_num = not_demo)

	// デモが終わったので、0に戻す（レーダー表示のため）
	eval($f:global_polygon_demo = 0)

	@mv_w03b_w04a_demo
//	restart -s
}
*/
