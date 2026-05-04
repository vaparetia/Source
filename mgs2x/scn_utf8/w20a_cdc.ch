/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"


command 無線設定 -base 14085 t:RPC_大佐＿Ｅ脚１Ｆ＿デフォルト 

@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

proc rp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側ＣＡＬＬ終了 {
	eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側聞いた = 1)
	command varsave $f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側聞いた
}
proc rp_w20a_ヴァンプ狙撃後開かない扉屋上側ＣＡＬＬ終了 {
	eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側聞いた = 1)
	command varsave $f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側聞いた
}


if(d:ST:P069_02_R01ヴァンプ狙撃終了２無線機デモ１終了 <= $w:p_story ){
	trap r_no_open_door_DE d:PLAYER \
		-mask ? \
		-exec {
			if( ($3 == 入る) && (!$f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側聞いた) && \
					(!$f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側ＣＡＬＬした) && \
					(`@rp_通常ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側ＣＡＬＬした = 1)
				command 無線設定 -call 14180 t:RPS_ヴァンプ狙撃以降開かない扉ＣＡＬＬ \
					d:CODEC_CALL rp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側ＣＡＬＬ終了
			}
			if( ($3 == 出る) && ($f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側ＣＡＬＬした) ){
				eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉ＤＥ側ＣＡＬＬした = 0)
				command 無線設定 -reset
			}
			if( ($3 == 入る) || ($3 ==出る) ){
				print 'r_no_open_door_DE'
				command 無線設定 -over $3 14180 t:RPS_ヴァンプ狙撃以降開かない扉
			}
	}
	trap r_no_open_door_HERI d:PLAYER \
		-mask ? \
		-exec {
			if( ($3 == 入る) && (!$f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側聞いた) && \
					(!$f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側ＣＡＬＬした) && \
					(`@rp_通常ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側ＣＡＬＬした = 1)
				command 無線設定 -call 14180 t:RPS_ヴァンプ狙撃以降開かない扉ＣＡＬＬ \
					d:CODEC_CALL rp_w20a_ヴァンプ狙撃後開かない扉屋上側ＣＡＬＬ終了
			}
			if( ($3 == 出る) && ($f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側ＣＡＬＬした) ){
				eval($f:rfp_w20a_ヴァンプ狙撃後開かない扉屋上側ＣＡＬＬした = 0)
				command 無線設定 -reset
			}
			if( ($3 == 入る) || ($3 ==出る) ){
				print 'r_no_open_door_HERI'
				command 無線設定 -over $3 14180 t:RPS_ヴァンプ狙撃以降開かない扉
			}
	}

}

if(d:ST:P069_02_R01ヴァンプ狙撃終了２無線機デモ１終了 <= $w:p_story ){
	trap r_emma_blood d:PLAYER \
		-mask ? \
		-exec {
			print 'r_emma_blood'
			command 無線設定 -over $3 14180 t:RPS_w20a_エマ血痕
			command 無線設定 -over $3 14085 t:RPC_w20a_エマ血痕
	}
}

//解体後戻ってきたら時間たったとみなしごはっち。
if( ($f:w20a_爆弾処理完了) && (!$f:rfp_w20a_爆弾処理完了後時間たった) ){
	eval($f:rfp_w20a_爆弾処理完了後時間たった = 1)
}


trap r_berukon d:PLAYER \
	-mask ? \
	-exec {
		print 'r_berukon'
		if($3 == 入る) {
			eval($f:rfp_w20a_ベルコンエリアにいますジミーはいません = 1)

		}else if($3 == 出る){
			eval($f:rfp_w20a_ベルコンエリアにいますジミーはいません = 0)
		}

	}


trap r_hashigo d:PLAYER \
	-mask ? \
	-exec {
		print 'r_hashigo'
		command 無線設定 -over $3 14085 t:RPC_Ｅ脚ハシゴ
	}


//べるこんのりばん

if($w:p_story < d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了){
	trap r_onbl001 d:PLAYER \
		-mask ? \
		-exec {
			print 'r_onbl001'
			@rp_プリスキン＿無線セット 0 $3 t:RPS_w20a_べるこんのりば
		}
	trap r_onbl101 d:PLAYER \
		-mask ? \
		-exec {
			print 'r_onbl101'
			@rp_プリスキン＿無線セット 0 $3 t:RPS_w20a_べるこんのりば
		}
}



