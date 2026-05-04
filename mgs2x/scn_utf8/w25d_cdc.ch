/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

//@rp_大佐デフォルト無線セット
proc rp_w25d_デフォルト無線セット {
	@rpc_大佐＿通常無線セット＿連絡橋
	@rp_プリスキン＿デフォルト無線セット
	@rp_ローズデフォルト無線セット
	@rp_エマ＿デフォルト無線セット
}
@rp_w25d_デフォルト無線セット

proc rp_w25d_プリスキンいなくなるの監視 {
	if( ($f:rfp_PS_プリスキン連絡途絶しましたん) && \
			(!$f:rfp_w25d_プリスキンいなくなるの監視ストップ) ){
		eval($f:rfp_w25d_プリスキンいなくなるの監視ストップ = 1)
		command 無線設定 -allclear
		@rp_w25d_デフォルト無線セット
		mesg プロック連続実行  w25d_プリスキンいなくなるの監視さま kill
	}
}

if(!$f:rfp_PS_プリスキン連絡途絶しましたん){
	chara プロック連続実行  w25d_プリスキンいなくなるの監視さま \
		-proc rp_w25d_プリスキンいなくなるの監視 \
		-time -1
}


chara 携帯端末 くそメール -skin htl -file t:mobile

proc PHONE_消火終了 {
	print 'PHONE_消火終了'
	eval($f:rfp_PHONE_消火聞いた = 1)
	command varsave $f:rfp_PHONE_消火聞いた
}

trap r_fire d:PLAYER \
	-mask ? \
	-exec {
		if( (!$f:ステージ置き炎鎮火)  && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				($3 == 入る) && (!$f:rfp_PHONE_w25d_消火ＣＡＬＬした) && (!$f:rfp_PHONE_消火聞いた) ){
			eval($f:rfp_PHONE_w25d_消火ＣＡＬＬした = 1)
			command 携帯呼出 -call t:PHONE_消火 -p PHONE_消火終了
		}
		if($3 == 出る) {
			if($f:rfp_PHONE_w25d_消火ＣＡＬＬした){
				command 携帯呼出 -reset
				eval($f:rfp_PHONE_w25d_消火ＣＡＬＬした = 0)
			}
		}
	}

	trap r_fixed_bridge d:PLAYER \
		-mask ? \
		-exec {
				print 'r_fixed_bridge'
				command 無線設定 -over $3 14085 t:RPC_w25d_はしなおた
		}


	trap r_K_door d:PLAYER \
		-mask ? \
		-exec {
				print 'r_K_door'
				command 無線設定 -over $3 14085 t:RPC_w25d_Ｋ脚入口
		}

