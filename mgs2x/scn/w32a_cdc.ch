/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

extern command ゲットエマライフ
extern command ゲットエマ状態
extern command ＮＰＣスネークの状態

command 無線設定 \
	-base 14085 t:RPC_大佐デフォルト＿エマ狙撃中

@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_エマ＿デフォルト無線セット


	trap r_hashigo d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON \
		-state  d:TRP_STATE_STAND \
		-dir    2048,512 \
		-exec {
			if( (!$f:rfp_w32a_ハシゴＣＡＬＬ中) && (!$f:rfp_w32a_ハシゴＣＡＬＬ聞いた) && \
					(`@rp_強制ＣＡＬＬしてもいいよ`) ){
				command 無線設定 -call 14085 t:RPC_w32a_ハシゴＣＡＬＬ d:CODEC_CALL 
				eval($f:rfp_w32a_ハシゴＣＡＬＬ中 = 1)
			}
		}


	trap r_hashigo d:PLAYER \
		-mask 出る \
		-exec {
			if($f:rfp_w32a_ハシゴＣＡＬＬ中){
				if(`%無線状態` < 3){
					command 無線設定 -reset
				}
				eval($f:rfp_w32a_ハシゴＣＡＬＬ中 = 0)
			}
		}


proc rp_w32_スネークがみえちっています {
	if (`%カメラ視界チェック -camera 0x03 -bound 57000,-36000,-88500 59000,-34000,-86500` == 1){
		return 1
	}else {
		return 0
	}
}

/*
proc rp_ＮＰＣスネーク監視 {
	eval($b:rfp_ＮＰＣスネークの状態 = `%ＮＰＣスネークの状態`)
	print 'どがえぼぼーん:' $b:rfp_ＮＰＣスネークの状態
//	eval($f:もばもば = `@rp_w32_スネークがみえちっています`)
//	print 'もばもば' $f:もばもば
}

chara プロック連続実行  ＮＰＣスネーク監視 \
	-proc rp_ＮＰＣスネーク監視 \
	-time -1
*/

