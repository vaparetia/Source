/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"
extern command ゲットゲームステータス


@rt_オタコンＳＡＶＥ＿デフォルト無線セット

command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト

//フラグをおっ立ててやるのだ。もうビンビン。
if($w:t_story < d:ST_船内入った){
	eval($w:t_story = d:ST_船内入った)
}



//ロッカー状態判定用
extern command ロッカー状態

command マップ設定 ２Ｆ廻廊部 \
-set{
	//右舷甲板への水密扉。レベルによっては開くから注意だ！！
//eval($w:ゲーム設定 = d:LEVEL_HARD)

		trap r_door_right d:PLAYER \
		-mask ? \
		-exec {
			print 'r_door_right'
			if($w:t_story < d:ST_T06b1Rオルガ戦勝利後無線機デモ終了){
				//hard以上は開かないっぽい
				if($w:ゲーム設定 >= d:LEVEL_HARD){
					command 無線設定 \
						-over $3 14112 t:RTO_２Ｆ右舷レベルによって開かない扉
					}else {
					command 無線設定 \
						-over $3 14112 t:RTO_２Ｆ水密扉近くオルガ戦前
					}
			}else {
				command 無線設定 \
				-over $3 14112 t:RTO_２Ｆ水密扉近くオルガ戦後
			}
	}

	//左舷開かないの水密扉であることよ。
	trap dr003 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
			if($w:ゲーム設定 >= d:LEVEL_HARD){
				eval($f:rft_w01b右舷開かない扉開けた = 1)
			}
		}
		
	trap r_door_left d:PLAYER \
		-mask ? \
		-exec {
			print 'r_door_left'
			if($w:t_story < d:ST_T06b1Rオルガ戦勝利後無線機デモ終了){
				command 無線設定 \
				-over $3 14112 t:RTO_２Ｆ左舷開かない水密扉近く
			}
	}




	//かーげーの出るあたりだぜ、め〜ん
	trap r_shadow d:PLAYER \
	-mask ? \
	-exec {
			print 'r_shadow'
			command 無線設定 \
				-over $3 14112 t:RTO_影エリア
	}

	//リフレッシュへ続くはずの開かない扉。ぐるま。
	trap r_no_open_door d:PLAYER \
	-mask ? \
	-exec {
			print 'r_no_open_door'
			command 無線設定 \
				-over $3 14112 t:RTO_２Ｆ開かない扉
	}


	//蒸気パイプのみの近く
	trap r_near_pipe d:PLAYER \
	-mask ? \
	-exec {
			print 'r_near_pipe'
			command 無線設定 \
				-over $3 14112 t:RTO_蒸気パイプ近く
	}

	//蒸気パイプ＋くぼみ
	trap r_kubomi d:PLAYER \
	-mask ? \
	-exec {
			print 'r_kubomi'
			command 無線設定 \
				-over $3 14112 t:RTO_二階通路くぼみ
	}

	//蒸気パイプ＋くぼみ＋ロッカー
	trap r_kubomi_locker d:PLAYER \
	-mask ? \
	-exec {
			print 'r_kubomi_locker'
			command 無線設定 \
				-over $3 14112 t:RTO_二階通路くぼみロッカー
	}

}

