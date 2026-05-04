/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ゲットエマ手繋ぎ状況
extern command ゲットエマ状態
extern command ゲットエマライフ
extern command ゲットエマ座標
extern command VecLen
extern command ゲット敵兵状態
extern command ゲット敵兵状態拡張版


//@rp_大佐デフォルト無線セット
@rpc_大佐＿通常無線セット＿連絡橋
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_エマ＿デフォルト無線セット


//eval($w:武器弾数Ｒ[6] = -1)

trap r_L d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_L in'
			eval($s:rfp_w25c_いまいるところ = "L")
		}

trap r_KL d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_KL in'
			eval($s:rfp_w25c_いまいるところ = "KL")
		}




trap r_window d:PLAYER \
	-mask ? \
	-exec {
			print 'r_window'
			command 無線設定 -over $3 14085 t:RPC_Ｌ脚外周＿窓
	}

trap r_harituki d:PLAYER \
	-mask ? \
	-exec {
			print 'r_harituki'
			command 無線設定 -over $3 14085 t:RPC_Ｌ脚外周＿張り付き
	}


trap r_harituki_syagami d:PLAYER \
	-mask ? \
	-exec {
			print 'r_harituki_syagami'
			command 無線設定 -over $3 14085 t:RPC_Ｌ脚外周＿しゃがみ
	}

trap r_K_door d:PLAYER \
	-mask ? \
	-exec {
			print 'r_K_door'
			command 無線設定 -over $3 14085 t:RPC_ＫＬ連絡橋＿Ｋ脚入口
	}

trap r_gun_cypher d:PLAYER \
	-mask ? \
	-exec {
			if(!$f:rfp_w25c_ローリング飛び越え成功した){
				eval($f:rfp_w25c_ローリング飛び越え成功した = 1)
				command varsave $f:rfp_w25c_ローリング飛び越え成功した
			}
			print 'r_gun_cypher'
			if( ($w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) || \
					($f:エマ存在フラグ) ){
				command 無線設定 -over $3 14085 t:RPC_ガンサイファーかもめ連絡橋＿大佐デフォルト
			}
	}

proc rp_w25c_ローリング飛び越えＣＡＬＬ終了 {
	eval($f:rfp_w25c_ローリング飛び越えＣＡＬＬ聞いた = 1)
	command varsave $f:rfp_w25c_ローリング飛び越えＣＡＬＬ聞いた
}

trap r_rolling d:PLAYER \
	-mask * \
	-exec {
		if( ($3 == 入る) || ($3 == いる) ){
			if(!$f:rfp_w25c_ローリング飛び越え地帯にはいったんきょ){
				eval($f:rfp_w25c_ローリング飛び越え地帯にはいったんきょ = 1)
				eval($i:rfp_w25c_ローリング飛び越え地帯侵入時刻 = $i:プレイタイム)
				command varsave $f:rfp_w25c_ローリング飛び越え地帯にはいったんきょ $i:rfp_w25c_ローリング飛び越え地帯侵入時刻
			}
			if( \
					( ( ($i:プレイタイム - $i:rfp_w25c_ローリング飛び越え地帯侵入時刻) / d:FRAME_RATE) >= 900) && \
					(!$f:rfp_w25c_ローリング飛び越えＣＡＬＬした) && \
					(!$f:rfp_w25c_ローリング飛び越えＣＡＬＬ聞いた) && \
					(!$f:rfp_w25c_ローリング飛び越え成功した) && \
					(`@rp_通常ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w25c_ローリング飛び越えＣＡＬＬした = 1)
				command 無線設定 -call 14085 t:RPC_w25c_ローリング飛び越えＣＡＬＬ \
					d:CODEC_CALL rp_w25c_ローリング飛び越えＣＡＬＬ終了
			}
		}
		if( ($3 == 出る) && ($f:rfp_w25c_ローリング飛び越えＣＡＬＬした) ){
			eval($f:rfp_w25c_ローリング飛び越えＣＡＬＬした = 0)
			if(`%無線状態` == 2){
				command 無線設定 -reset
			}
		}

		command 無線設定 -over $3 14085 t:RPC_w25c_ローリング飛び越え地帯
	}



