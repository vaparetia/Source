/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"



@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット

if($w:p_story >= d:ST:P070_23_R04ＡＧ起動２３無線機デモ４終了){
	eval($f:rfp_w41a_拘束開放されました = 1)
}

proc rp_w41a_拷問実行監視 {
//	print 'みてますよー$w:p_story:' $w:p_story
	if( ($w:p_story == d:ST:P070_23_R04ＡＧ起動２３無線機デモ４終了) && \
			(!$f:rfp_w41a_拷問実行監視やめた) ){
		eval($f:rfp_w41a_拷問実行監視やめた = 1)
		@ゲームステージへ戻る
		mesg プロック連続実行  w41a_拷問実行監視えもん kill
		chara delay 拘束解除ディレイ \
			-t 60 \
			-exec {
				@w41a_拘束開放処理
			}
	}
}

if($w:p_story < d:ST:P070_23_R04ＡＧ起動２３無線機デモ４終了){
		chara プロック連続実行  w41a_拷問実行監視えもん \
		-proc rp_w41a_拷問実行監視 \
		-time -1
}
	




if($w:p_story < d:ST:P072_01_R01大佐混乱１無線機デモ１終了){
	trap r_Mrtorture d:PLAYER \
		-mask ? \
		-exec {
			if($3 == 入る) {
				print 'r_Mrtorture in'
				eval($f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ = 1)
			}
			if($3 == 出る) {
				print 'r_Mrtorture out'
				eval($f:rfp_w41a_拷問部屋にいるのＤＥＡＴＨ = 0)
			}
		}

	trap r_no_open_door d:PLAYER \
		-mask ? \
		-exec {
			print 'r_no_open_door'
			command 無線設定 -over $3 14085 t:RPC_拷問部屋＿開かない扉
		}


	trap r_node d:PLAYER \
		-mask ? \
		-exec {
			print 'r_node'
			command 無線設定 -over $3 14085 t:RPC_w41a_ノード近く
		}
}


if(d:ST:P072_01_R01大佐混乱１無線機デモ１終了 <= $w:p_story && \
		$w:p_story < d:ST:P073_02_P01通路Ａ２ポリゴンデモ１開始) {
	@rp_連続発狂ＣＡＬＬ初期化
	chara delay 混乱無線後発狂ＣＡＬＬディレイ \
		-time 60 \
		-exec {
			chara プロック連続実行 発狂ＣＡＬＬ連続実行 \
			-proc rp_連続発狂ＣＡＬＬ \
			-time -1
		}
}




//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)
