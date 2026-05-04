/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

extern command エレベータ状態取得


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット

if($w:p_story >= d:ST:P069_04_R02ヴァンプ狙撃終了４無線機デモ２終了){
	trap r_mushi d:PLAYER \
		-mask ? \
		-exec {
			print 'r_mushi'
				command 無線設定 -over $3 14085 t:RPC_フナムシ＿普通
		}
}




//ロッカー小部屋ん
	// 左側一番上
/*
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:01 t:RPC_ロッカー共通１
	}
*/
	//あかなくなった
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:01 t:RPC_開かないロッカー共通１
		@rp_開かないロッカーフラグセット $3 $f:rfp_w24a_lk001_開かないロッカー開けた
	}

	trap lk001 d:PLAYER \
		-and \
		-mask いる \
		-state 0,4 \
		-dir 3072,512 1024,512 \
		-button d:ACTION_BUTTON,-1 \
		-exec {
			eval($f:rfp_w24a_lk001_開かないロッカー開けた =  1)
		}

	// 左側中央 アイズポスター
	trap lk002 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:02 t:RPC_ロッカー共通２
	}

	// 左側一番右
	trap lk003 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:03 t:RPC_ロッカー共通１
	}

		// 下側一番左
	trap lk004 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:04 t:RPC_ロッカー共通１
	}

		// 下側左から２番目
	trap lk005 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:05 t:RPC_ロッカー共通２
	}

		// 下側左から３番目	アイズポスターであかんちん
	trap lk006 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:06 t:RPC_ロッカー共通１
	}

/*
	trap lk006 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:06 t:RPC_開かないロッカー共通１
		@rp_開かないロッカーフラグセット $3 $f:rfp_w24a_lk006_開かないロッカー開けた
	}

	trap lk006 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir 0,512 2048,512 \
		-exec {
			eval($f:rfp_w24a_lk006_開かないロッカー開けた =  1)
		}
*/

		// 下側右から３番目
	trap lk007 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:07 t:RPC_ロッカー共通２
	}

	// 下側右から２番目
	trap lk008 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:08 t:RPC_ロッカー共通１
	}

	// 下側一番右
	trap lk009 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:09 t:RPC_ロッカー共通２
	}


trap r_box d:PLAYER \
	-mask ? \
	-exec {
		print 'r_box'
		command 無線設定 -over $3 14085 t:RPC_段差上り＿普通
	}



//ローかあろーか
//えれべた内部
trap elv d:PLAYER \
	-mask ? \
	-exec {
		print 'elv'
		command 無線設定 -over $3 14085 t:RPC_エレベータ内部
	}


//えれべたすいっちかく
//ＣＡＬＬ状態チェックをしなきゃだめりん
if($w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了){
	trap r_elv_switch d:PLAYER \
		-mask * \
		-exec {
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'r_elv_switch'
				command 無線設定 -over $3 14085 t:RPC_w24a_エレベータスイッチ前
			}
			if( ($3 == 出る) && ($f:rfp_w24a_エレベータ前ＣＡＬＬした) ){
				command 無線設定 -reset
				if( ($w:武器 != 15) || ($w:アイテム != 6) ){
					eval($f:rfp_w24a_エレベータ前ＣＡＬＬした = 0 )
				}
			}
			if($3 == いる){
				if( ( ($w:武器 != 15) || ($w:アイテム != 6) ) && \
						($w:p_story < d:ST:P038_01_S01網膜チェック１シナリオデモ１開始) && \
						(!$f:rfp_PC_シェル１中央棟１Ｆ＿エレベータ説明聞いた) && \
						(`@rp_通常ＣＡＬＬしてもいいよ`) && \
						($w:アラートモード != d:ALERT_MODE_ALERT) && \
						(!$f:rfp_w24a_エレベータ前ＣＡＬＬした) && (!$f:rfp_w24a_エレベータ呼ぶのに成功した) ){
					command 無線設定 -call 14085 t:RPC_w24a_エレベータ前ＣＡＬＬ d:CODEC_CALL 
					eval($f:rfp_w24a_エレベータ前ＣＡＬＬした = 1 )

				}else if( ($w:武器 == 15) && ($w:アイテム == 6) && \
						($f:rfp_w24a_エレベータ前ＣＡＬＬした) ){
					command 無線設定 -reset
				}
			}
		}

	trap elv_switch d:PLAYER \
		-mask 入る \
		-button d:ACTION_BUTTON \
		-dir 2048 512 \
		-exec {
			if( ($w:武器 == 15) && ($w:アイテム == 6) ){
				eval($f:rfp_w24a_エレベータ呼ぶのに成功した = 1)
			}
		}
}


//ろーか
trap r_roka d:PLAYER \
	-mask ? \
	-exec {
		print 'r_roka'
		if($3 == 入る) {
			eval($f:rfp_w24a_監視カメラ地帯にいましゃー = 1)
		}
		if($3 == 出る) {
			eval($f:rfp_w24a_監視カメラ地帯にいましゃー = 0)
		}
	}


//スネークの痛いデモ
if($w:p_story < d:ST:P037_01_R01スネークの遺体１無線デモ１開始) {
	trap r_demo d:PLAYER \
		-mask いる \
		-exec {
	//		print '$f:ロードチェックＯＮフラグ:'$f:ロードチェックＯＮフラグ
			if( ($f:ロードチェックＯＮフラグ) && \
					(!$f:rfp_スネークの遺体の話聞いた) && \
					($w:p_story < d:ST:P037_01_R01スネークの遺体１無線デモ１開始) ){
				@rt_P037_01_R01スネークの遺体１無線デモ１
			}
		}
}



//eval($w:武器弾数Ｒ[12] = -1)
