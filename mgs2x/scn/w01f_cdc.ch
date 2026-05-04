/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

//フラグをおっ立ててやるのだ。もうビンビン。
if($w:t_story < d:ST_船内入った){
	eval($w:t_story = d:ST_船内入った)
}

@rt_オタコンＳＡＶＥ＿デフォルト無線セット
command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト


command マップ設定 １Ｆリフレッシュルーム \
	-set{
	/* 製品版的には開きっぱなし
		//ざ・開かないドア。うげん
		trap r_no_open_door_right d:PLAYER \
			-mask ? \
			-exec {
				print 'r_no_open_door_right'
				command 無線設定 \
				-over $3 14112 t:RTO_エンジンルームへの階段近く体験版
			}
	*/


		//リフレッシュな昼下がり
		trap r_reflesh_room d:PLAYER \
			-mask ? \
			-exec {
				print 'r_reflesh_room'
				command 無線設定 \
					-over $3 14112 t:RTO_リフレッシュルーム
			}

		//冷凍ＴＶ
		trap r_TV d:PLAYER \
			-mask ? \
			-exec {
				print 'r_TV'
				command 無線設定 \
					-over $3 14112 t:RTO_テレビ会話
			}

		trap r_TV d:PLAYER \
			-mask いる \
//			-dir 3072,512 1024,512 \
			-dir 3072,512 \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				command 無線設定 \
					-over $3 14112 t:RTO_テレビ凝視
			}

		trap r_TV d:PLAYER \
			-mask いる \
			-state 0,1,2,3,4,5,6,8 \
			-exec {
				command 無線設定 \
					-over $3 14112 t:RTO_テレビ会話
			}

		trap r_no_open_door_2F d:PLAYER \
			-mask ? \
			-exec {
				print 'r_no_open_door_2F'
				command 無線設定 \
					-over $3 14112 t:RTO_リフレッシュ開かない扉
			}

	//蒸気パイプ注意んぐ
	trap r_near_pipe d:PLAYER \
	-mask ? \
	-exec {
			print 'r_near_pipe'
			command 無線設定 \
				-over $3 14112 t:RTO_蒸気パイプ近く
	}


	}
