/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

extern command ゲット敵兵状態拡張版
extern command ゲット敵兵状態

//@rp_大佐デフォルト無線セット
//@rpc_大佐＿通常無線セット＿連絡橋
//@rpc_大佐＿通常無線セット＿連絡橋かもめ
command 無線設定 \
	-base 14085 t:RPC_ガンサイファーかもめ連絡橋＿大佐デフォルト



proc rp_w21b_ヴァンプ狙撃後開かない扉ＣＡＬＬ終了 {
	eval($f:rfp_w21b_ヴァンプ狙撃後開かない扉聞いた = 1)
	command varsave $f:rfp_w21b_ヴァンプ狙撃後開かない扉聞いた
}


if(d:ST:P069_02_R01ヴァンプ狙撃終了２無線機デモ１終了 <= $w:p_story ){
	trap r_no_open_door d:PLAYER \
		-mask ? \
		-exec {
			if( ($3 == 入る) && (!$f:rfp_w21b_ヴァンプ狙撃後開かない扉聞いた) && \
					(!$f:rfp_w21b_ヴァンプ狙撃後開かない扉ＣＡＬＬした) && \
					(`@rp_通常ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w21b_ヴァンプ狙撃後開かない扉ＣＡＬＬした = 1)
				command 無線設定 -call 14180 t:RPS_ヴァンプ狙撃以降開かない扉ＣＡＬＬ \
					d:CODEC_CALL rp_w21b_ヴァンプ狙撃後開かない扉ＣＡＬＬ終了
			}
			if( ($3 == 出る) && ($f:rfp_w21b_ヴァンプ狙撃後開かない扉ＣＡＬＬした) ){
				eval($f:rfp_w21b_ヴァンプ狙撃後開かない扉ＣＡＬＬした = 0)
				command 無線設定 -reset
			}
			if( ($3 == 入る) || ($3 ==出る) ){
				print 'r_no_open_door'
				command 無線設定 -over $3 14180 t:RPS_ヴァンプ狙撃以降開かない扉
			}
	}
}

@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

proc rp_w21a_地雷原１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $f:rfp_w21a_地雷原１無線デモ１した = 1);
	command varsave $f:rfp_w21a_地雷原１無線デモ１した
}

proc rp_w21a_地雷原１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P020_01_R01_地雷原１無線デモ１ \
		d:CODEC_DEMO rp_w21a_地雷原１無線デモ１終了 \
	-cont 14085 t:RPD_P020_01_R01_地雷原１無線デモ１＿２
}

trap r_mrx d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_mrx'
		if( ($w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了) && \ 
				(!$f:rfp_w21a_地雷原１無線デモ１した) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) ){
			@rp_w21a_地雷原１無線デモ１
		}
	}

//eval($w:アイテム数Ｒ[d:アイテム:地雷探知器]  = -1)

if($w:p_story < d:ST:P038_01_S01網膜チェック１シナリオデモ１開始){
	trap r_mine d:PLAYER \
		-mask ? \
		-exec {
			print 'r_mine' 
			command 無線設定 -over $3 14085 t:RPC_w21a_地雷地帯
			if($w:アイテム数Ｒ[d:アイテム:地雷探知器] <= 0){
				if($w:p_story < d:ST:P027_01_R01爆弾解体後昇降機ホール１無線デモ１開始){
					@rp_プリスキン＿無線セット 0 $3 t:RPS_w21a_地雷地帯＿ピーター付き
				}else {
					@rp_プリスキン＿無線セット 0 $3 t:RPS_w21a_地雷地帯＿オタコン付き
				}
			}
		}
}

trap r_pi001 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴１Ａ t:RPC_落し穴共通１
	}
trap r_pi002 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴１Ｂ t:RPC_落し穴共通２
	}

trap r_pi003 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴２Ａ t:RPC_落し穴＿奈落エルード１
	}
trap r_pi004 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴２Ｂ t:RPC_落し穴＿奈落エルード２
	}
trap r_pi005 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴３Ａ t:RPC_落し穴＿奈落エルード１
	}
trap r_pi006 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴３Ｂ t:RPC_落し穴＿奈落エルード２
	}
trap r_pi007 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴４Ａ t:RPC_落し穴＿奈落エルード１
	}
trap r_pi008 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴４Ｂ t:RPC_落し穴＿奈落エルード２
	}
trap r_pi009 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴５Ａ t:RPC_落し穴＿奈落エルード１
	}
trap r_pi010 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴５Ｂ t:RPC_落し穴＿奈落エルード２
	}
trap r_pi011 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴６Ａ t:RPC_落し穴＿奈落エルード１
	}
trap r_pi012 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴６Ｂ t:RPC_落し穴＿奈落エルード２
	}
trap r_pi013 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴７Ａ t:RPC_落し穴＿奈落エルード１
	}
trap r_pi014 d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 落し穴７Ｂ t:RPC_落し穴＿奈落エルード２
	}


trap by001 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_地雷＿奈落エルード
	}
trap by002 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_地雷＿奈落エルード
	}
trap by003 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_地雷＿奈落エルード
	}
trap by004 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_地雷＿奈落エルード
	}



//eval($w:アイテム数Ｒ[d:アイテム:地雷探知器] = -1)

