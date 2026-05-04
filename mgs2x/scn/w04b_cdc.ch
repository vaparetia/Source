/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

command 無線設定 \
	-speech 1

command 無線設定 \
	-base 14112 t:RTO_船倉イベント
@rt_オタコンＳＡＶＥ＿デフォルト無線セット

proc rt_w04b_懸垂監視 {
//print '$w:スネーク懸垂回数:' $w:スネーク懸垂回数
	@rt_懸垂ＣＡＬＬ
}

chara プロック連続実行  w04b_武羅鎖我裡拳硬鬼 \
	-proc rt_w04b_懸垂監視 \
	-time -1


command マップ設定 船倉２ 通路１ 通路２ -set {

/*
//３Ｆ開かないドア小僧
trap r_3f_no_open_door d:PLAYER \
	-mask ? \
	-exec {
			print 'r_3f_no_open_door'
			command 無線設定 \
				-over $3 14112 t:RTO_船倉ロックドア
	}
*/

//鳴り床地帯
trap r_cryfloor d:PLAYER \
	-mask ? \
	-exec {
			print 'r_cryfloor'
			command 無線設定 \
				-over $3 14112 t:RTO_鳴り床
	}

/*
//はしご的な憂鬱
trap r_ladder d:PLAYER \
	-mask ? \
	-exec {
			print 'r_ladder'
			command 無線設定 \
				-over $3 14112 t:RTO_はしご
	}

//はしごと鳴り床のコソボ
trap r_ladder_cryfloor d:PLAYER \
	-mask ? \
	-exec {
			print 'r_ladder_cryfloor'
			command 無線設定 \
				-over $3 14112 t:RTO_はしご鳴り床
	}
*/


//皆死後
trap r_hatch d:PLAYER \
	-mask ? \
	-exec {
			print 'r_hatch'
			command 無線設定 \
				-over $3 14112 t:RTO_船倉地下ハッチ
	}

//ぷらじゃくてぇ
trap r_projecter d:PLAYER \
	-mask ? \
	-exec {
			print 'r_projecter'
			command 無線設定 \
				-over $3 14112 t:RTO_プロジェクタ
	}

trap r_underground d:PLAYER \
	-mask ? \
	-exec {
			print 'r_underground'
			command 無線設定 \
				-over $3 14112 t:RTO_船倉地下
	}

}