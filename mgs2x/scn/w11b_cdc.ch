/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
#include "w11ab_cdc_common.ch"
extern command VecLen

	command 無線設定 \
	-base 14085 t:RPC_w11b_大佐デフォルト

/*
proc rp_うご {
	command VecLen	-10000,-43800,25200 \
				$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ
	print 'うごー:' $status
}

	chara プロック連続実行  うご \
		-proc rp_うご \
		-time -1
*/

if(!$f:rfp_w11b_無臭Ｃ４解体開始カウントスタートした){
	eval($f:rfp_w11b_無臭Ｃ４解体開始カウントスタートした = 1)
	eval($i:イベント開始時刻 = $i:プレイタイム)
	command varsave $i:イベント開始時刻 $f:rfp_w11b_無臭Ｃ４解体開始カウントスタートした
}


