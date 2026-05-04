/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット


trap r_koware_mono d:PLAYER \
	-mask ? \
	-exec {
		print 'r_koware_mono'
			command 無線設定 -over $3 14085 t:RPC_コワレモノ近辺
	}

proc rp_w24c_人質イベント説明＿マイクなしＣＡＬＬ終了 {
	command パッド操作 -cancel
	command varsave $f:rfp_w24c_人質イベント説明＿マイクなしＣＡＬＬした
}

proc rp_w24c_人質イベント説明＿マイクありＣＡＬＬ終了 {
	command パッド操作 -cancel
	command varsave $f:rfp_w24c_人質イベント説明＿マイクありＣＡＬＬした
}

if( ( ($w:武器弾数Ｒ[d:武器:マイク] >= 0)  && \
				(!$f:rfp_w24c_人質イベント説明＿マイクありＣＡＬＬした) ) || \
		( ($w:武器弾数Ｒ[d:武器:マイク] < 0) && \
				(!$f:rfp_w24c_人質イベント説明＿マイクなしＣＡＬＬした) ) ){
		
	trap r_call d:PLAYER \
		-mask いる \
		-exec {
	@rp_人質部屋入った時のＣＡＬＬ
}
}

proc rp_人質部屋入った時のＣＡＬＬ {
	if( ($w:武器弾数Ｒ[d:武器:マイク] > 0)  && \
			($f:ロードチェックＯＮフラグ) && \
			(!$f:rfp_w24c_人質イベント説明＿マイクありＣＡＬＬした) ){
		command パッド操作 -release
		command 無線設定 \
			-call 14085 \
			t:RPC_w24c_人質イベント説明＿マイクありＣＡＬＬ \
			d:CODEC_DEMO rp_w24c_人質イベント説明＿マイクありＣＡＬＬ終了
		eval($f:rfp_w24c_人質イベント説明＿マイクありＣＡＬＬした = 1)
	}

	if( ($w:武器弾数Ｒ[d:武器:マイク] <= 0)  && \
			($f:ロードチェックＯＮフラグ) && \
			(!$f:rfp_w24c_人質イベント説明＿マイクなしＣＡＬＬした) ){
		command 無線設定 \
			-call 14085 \
			t:RPC_w24c_人質イベント説明＿マイクなしＣＡＬＬ \
			d:CODEC_DEMO rp_w24c_人質イベント説明＿マイクなしＣＡＬＬ終了
		command パッド操作 -release
		eval($f:rfp_w24c_人質イベント説明＿マイクなしＣＡＬＬした = 1)
	}
}


proc rp_w24c_人質殴ったＣＡＬＬ終了 {
	command varsave $f:rfp_w24c_人質殴ったＣＡＬＬした
}

proc rp_w24c_人質殴った {
	eval($w:rfp_w24c_人質殴った回数 = $w:rfp_w24c_人質殴った回数 + 1)
	print '殴りやがった！' $w:rfp_w24c_人質殴った回数
	if( ($w:rfp_w24c_人質殴った回数 >= 15) && \
			(`@rp_通常ＣＡＬＬしてもいいよ`) && \
			(!$f:rfp_w24c_人質殴ったＣＡＬＬした) && \
			(!$f:rfp_PC_人質＿人質殴った聞いた) ){
		eval($f:rfp_w24c_人質殴ったＣＡＬＬした = 1)
		command 無線設定 -call 14085 t:RPC_w24c_人質殴ったＣＡＬＬ \
				d:CODEC_CALL rp_w24c_人質殴ったＣＡＬＬ終了
	}
}


//eval($f:w24c_ギャルパンツ撮影した = 1)

//ギャルパンツ見え地帯
trap ev007 d:PLAYER \
	-mask ? \
	-exec {
		print 'ev007'
			command 無線設定 -over $3 14085 t:RPC_w24c_ギャルパンツ見え地帯
	}











//eval($w:武器弾数Ｒ[d:武器:マイク] = -1)

