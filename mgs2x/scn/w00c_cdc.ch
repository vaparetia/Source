/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"


@rt_オタコンＳＡＶＥ＿デフォルト無線セット
command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト

proc rt_w04a_懸垂監視 {
//print '$w:スネーク懸垂回数:' $w:スネーク懸垂回数
	@rt_懸垂ＣＡＬＬ
}

chara プロック連続実行  w04a_武羅鎖我裡拳硬鬼 \
	-proc rt_w04a_懸垂監視 \
	-time -1


//オルガ撃ちやがった chara 死体君 -b 指定
proc rt_オルガ死体撃ちやがった {
	if( (!$f:rft_オルガ死体撃ちやがった聞いた) && \
			(`@rt_通常ＣＡＬＬしてもいいよ`) && \
			(!$f:rft_オルガ死体撃ちやがったＣＡＬＬ中) ){
		eval($f:rft_オルガ死体撃ちやがったＣＡＬＬ中 = 1)
		command 無線設定 \
					-call 14112 t:RTO_オルガ死体撃ちやがった d:CODEC_CALL オルガ死体撃ちやがった終了
	}
}

proc オルガ死体撃ちやがった終了{
	eval($f:rft_オルガ死体撃ちやがったＣＡＬＬ中 = 0)
}

//キャンセルも入れよーね

trap r_play_with_olga d:PLAYER \
	-mask いる \
	-exec {	// 
		if((!$f:rft_オルガ死体聞いた) && (!$f:rfp_w00c_オルガさん失踪しました) && (!$f:オルガ失踪) ){
			command コントロール座標取得 オルガ気絶 $w:rtf_オルガ気絶Ｘ $w:rtf_オルガ気絶Ｙ $w:rtf_オルガ気絶Ｚ
			command VecLen $w:rtf_オルガ気絶Ｘ $w:rtf_オルガ気絶Ｙ $w:rtf_オルガ気絶Ｚ $4 $5 $6
			eval($w:rft_w00c_オルガスネーク相対距離 = $status)
		}
	}

trap r_play_with_olga d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			if(!$f:rft_w00c_オルガいぢり地帯いる){
				print 'r_play_with_olga in'
				eval($f:rft_w00c_オルガいぢり地帯いる = 1)
			}

		}else if($3 == 出る) {
			if($f:rft_w00c_オルガいぢり地帯いる){
				print 'r_play_with_olga out'
				eval($f:rft_w00c_オルガいぢり地帯いる = 0)
				if($f:rft_オルガ死体撃ちやがったＣＡＬＬ中){
					command 無線設定 -reset
					eval($f:rft_オルガ死体撃ちやがったＣＡＬＬ中 = 0)
				}
			}
		}
		command 無線設定 \
				-over $3 14112 t:RTO_オルガいぢり地帯
	}

	trap ev102 d:PLAYER -mask 入る \
		-exec {
			print 'ぐるぐるっちょへにへに'
			eval($f:rfp_w00c_オルガさん失踪しました = 1)
		}



trap r_mast d:PLAYER \
	-mask ? \
	-exec {
		print 'r_masit in'
		command 無線設定 \
				-over $3 14112 t:RTO_マスト最上部
	}

