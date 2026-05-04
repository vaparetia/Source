/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

//解体後戻ってきたら時間たったとみなしごはっち。
if( ($f:w22a_爆弾処理完了) && (!$f:rfp_w22a_爆弾処理完了後時間たった) ){
	eval($f:rfp_w22a_爆弾処理完了後時間たった = 1)
}

proc rp_w22a_赤外線ＣＡＬＬ終了 {
	eval($f:rfp_w22a_赤外線ＣＡＬＬ聞いた = 1)
	command varsave $f:rfp_w22a_赤外線ＣＡＬＬ聞いた
}

trap r_IR_sensor d:PLAYER \
	-mask ? \
	-exec {
		if( ($3 == 入る) || ($3 == 出る) ){
			print 'r_IR_sensor'
			command 無線設定 -over $3 14085 t:RPC_w22a_おもしろ赤外線センサー
		}
		if( ($3 == 入る) && (!$f:w22a_赤外線センサーフラグ) && \
				(!$f:rfp_w22a_赤外線ＣＡＬＬ聞いた) && \
				(!$f:rfp_w22a_赤外線ＣＡＬＬ中) && ($w:プラント編クリア回数 < 1) ){
			eval($f:rfp_w22a_赤外線ＣＡＬＬ中 = 1)
			command 無線設定 -call 14085 t:RPC_w22a_赤外線ＣＡＬＬ \
				d:CODEC_CALL rp_w22a_赤外線ＣＡＬＬ終了
		}
		if( ($3 == 出る) && ($f:rfp_w22a_赤外線ＣＡＬＬ中) ){
				eval($f:rfp_w22a_赤外線ＣＡＬＬ中 = 0)
				if(`%無線状態` == 2){
					command 無線設定 -reset
				}
		}
	}



trap dr061 d:PLAYER \
	-mask ? \
	-exec {
		print 'dr061'
		if($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) {
			command 無線設定 -over $3 14085 t:RPC_カードドア
		}
	}

trap r_box d:PLAYER \
	-mask ? \
	-exec {
		print 'r_box'
		command 無線設定 -over $3 14085 t:RPC_段差上り＿普通
	}


//１Ｆ左上小部屋Ａ
	//北ロッカー
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー01 t:RPC_ロッカー共通１
	}

	
	//南ロッカー 開かない
	trap lk002 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー02 t:RPC_開かないロッカー共通１
		@rp_開かないロッカーフラグセット $3 $f:rfp_w22a_lk002_開かないロッカー開けた
	}

	trap lk002 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir 1024,512 3072,512 \
		-exec {
			eval($f:rfp_w22a_lk002_開かないロッカー開けた =  1)
		}

//１Ｆ右上小部屋
	//北ロッカー
	trap lk011 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー11 t:RPC_ロッカー共通１
	}
	trap lk012 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー12 t:RPC_ロッカー共通２
	}



// １Ｆ中央左小部屋
	trap lk021 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー21 t:RPC_ロッカー共通１
	}


// １Ｆ中央右小部屋
	trap lk031 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー31 t:RPC_ロッカー共通１
	}

// １Ｆ左下小部屋
	trap lk041 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー41 t:RPC_ロッカー共通１
	}


//１Ｆろーかあろーか
trap by072 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}
trap by073 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}
trap by074 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}
trap by075 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}
trap r_byond d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}




// Ｂ１左下小部屋
	//ミナミ
	trap lk111 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー71 t:RPC_ロッカー共通１
	}

	//北あかん
	trap lk112 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー72 t:RPC_開かないロッカー共通１
		@rp_開かないロッカーフラグセット $3 $f:rfp_w22a_lk072_開かないロッカー開けた
	}

	trap lk112 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir 3072,512 1024,512 \
		-exec {
			eval($f:rfp_w22a_lk072_開かないロッカー開けた =  1)
		}

	trap lk113 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー71 t:RPC_ロッカー共通１
	}

	trap lk114 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー73 t:RPC_ロッカー共通２
	}


