/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

command 無線設定 \
	-speech 1

command 無線設定 \
	-base 14112 t:RTO_船倉イベント
@rt_オタコンＳＡＶＥ＿デフォルト無線セット

proc rt_w04c_懸垂監視 {
//print '$w:スネーク懸垂回数:' $w:スネーク懸垂回数
	@rt_懸垂ＣＡＬＬ
}

chara プロック連続実行  w04c_武羅鎖我裡拳硬鬼 \
	-proc rt_w04c_懸垂監視 \
	-time -1


command マップ設定 通路２ 船倉３ -set {

//たんまっつ
trap r_ws d:PLAYER \
	-mask ? \
	-exec {
			print 'r_ws'
			command 無線設定 \
				-over $3 14112 t:RTO_電送機
	}


//てれば兵
trap r_tv d:PLAYER \
	-mask ? \
	-exec {
			print 'r_tv'
			command 無線設定 \
				-over $3 14112 t:RTO_船倉ＴＶ兵
	}

trap r_pole_enable d:PLAYER \
	-mask ? \
	-exec {
			print 'r_pole_under'
			command 無線設定 \
				-over $3 14112 t:RTO_ポール下
	}

trap r_pole_able d:PLAYER \
	-mask ? \
	-exec {
			print 'r_pole_able'
			command 無線設定 \
				-over $3 14112 t:RTO_ポール上
	}


}

/*
eval($f:rft_演説１陸軍に対抗再生した = 1)
eval($f:rft_演説２海軍からの圧力再生した = 1)
eval($f:rft_演説４ＮＭＤ再生した = 1)
*/

