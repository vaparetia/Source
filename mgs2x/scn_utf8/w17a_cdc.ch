/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

extern command ゲットゲームステータス

//@rp_大佐デフォルト無線セット
proc w17a_デフォルト無線セット {
	command 無線設定 -allclear
	@rpc_大佐＿通常無線セット＿連絡橋
	if(!$f:rfp_w17a_ダンボール男発生中){
		@rp_プリスキン＿デフォルト無線セット
	}
	@rp_ローズデフォルト無線セット
	@rp_ピーター＿デフォルト無線セット
}
if(!$f:rfp_w17a_ダンボール男用無線セットした){
	@w17a_デフォルト無線セット
}else {
	eval($f:rfp_w17a_ダンボール男用無線セットした =0)
	eval($f:rfp_w17a_ダンボール男発生中 = 0)
}

proc rp_ダンボール男監視 {
	command ＮＰＣスネークの状態
//	print 'ＮＰＣスネークの状態;' $status
	if($status == 1) {
		eval($f:rfp_箱男攻撃した = 1)
	}
}

proc rp_ダンボール男監視野郎参上 {
	chara プロック連続実行  ダンボール男監視野郎 \
	-proc rp_ダンボール男監視 \
	-time -1
	eval($f:rfp_w17a_ダンボール男発生中 = 1)
	eval($f:rfp_w17a_ダンボール男用無線セットした = 1)
	@w17a_デフォルト無線セット
}

proc rp_ダンボール男監視野郎も退場 {
	mesg プロック連続実行 ダンボール男監視野郎 kill
	eval($f:rfp_w17a_ダンボール男発生中 = 0)
	eval($i:rfp_箱男退場時刻 = $i:プレイタイム)
	command varsave $i:rfp_箱男退場時刻 $f:rfp_w17a_ダンボール男発生中
	@rp_プリスキン＿デフォルト無線セット
}

//if($w:ゲーム設定 >= d:LEVEL_NORMAL) {
	trap r_s_camera d:PLAYER \
		-mask * \
		-exec {
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'r_s_camera'
				command 無線設定 -over $3 14085 t:RPC_監視カメラ近く
			}
			if($3 == いる ){
				@カメラ再セットチェック $b:w17a_破壊ロード回数[5]
				eval($f:rfp_そこの監視カメラ生きてます = $f:再セットフラグ)
			}
		}
//}


trap by001 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}

trap by002 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}

trap r_byond d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}

trap r_pit d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴１ t:RPC_落し穴共通１
	}

trap r_pit_byond_pi002 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴２ t:RPC_落し穴＿奈落エルード１
	}

trap r_pit_byond_pi003 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴３ t:RPC_落し穴＿奈落エルード２
	}

trap r_pit_byond_pi004 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴４ t:RPC_落し穴＿奈落エルード１
	}

trap r_pit_byond_pi005 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴５ t:RPC_落し穴＿奈落エルード２
	}

