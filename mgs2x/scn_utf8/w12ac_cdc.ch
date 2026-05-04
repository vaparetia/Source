/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

chara 携帯端末 くそメール -skin htl -file t:mobile

//@rp_大佐デフォルト無線セット
@rpc_大佐＿通常無線セット＿連絡橋かもめ
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

//eval($f:rfp_PR_ローズさんが激怒してます！ = 1)
//eval($b:Ｃ４爆弾設置数 = 1)
//eval($w:ゲーム設定 = d:LEVEL_HARD)

//解体後戻ってきたら時間たったとみなしごはっち。
if( ($f:w12a_爆弾処理完了) && (!$f:rfp_w12a_爆弾処理完了後時間たった) ){
	eval($f:rfp_w12a_爆弾処理完了後時間たった = 1)
}

/*
proc PHONE_w12_左ヴァンプ地雷 {
	eval($f:rfp_PHONE_w12_左ヴァンプ地雷聞いた = 1)
	command varsave $f:rfp_PHONE_w12_左ヴァンプ地雷聞いた
}

proc PHONE_w12_右ヴァンプ地雷 {
	eval($f:rfp_PHONE_w12_右ヴァンプ地雷聞いた = 1)
	command varsave $f:rfp_PHONE_w12_右ヴァンプ地雷聞いた
}

if( ($w:p_story >= d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了) && \
		($w:アイテム数Ｒ[d:アイテム:携帯電話] > 0) ){
	trap r_mine_left d:PLAYER \
		-mask ? \
		-exec {
			if( ($3 == 入る) && \
					(!$f:w12a_クレイモア２取得フラグ) && (!$f:w12a_クレイモア４取得フラグ)  && \
					(!$f:rfp_w12_左ヴァンプ地雷ＣＡＬＬした) && \
					(!$f:rfp_PHONE_w12_左ヴァンプ地雷聞いた) && \
					(`@rp_強制ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w12_左ヴァンプ地雷ＣＡＬＬした = 1)
				command 携帯呼出 -call t:PHONE_地雷 -p PHONE_w12_左ヴァンプ地雷
			}
			if($3 == 出る) {
				print 'r_mine_left out'
				if($f:rfp_w12_左ヴァンプ地雷ＣＡＬＬした){
					command 携帯呼出 -reset
					eval($f:rfp_w12_左ヴァンプ地雷ＣＡＬＬした = 0)
				}
			}
		}


	trap r_mine_right d:PLAYER \
		-mask ? \
		-exec {
			if( ($3 == 入る) && \
					(!$f:w12a_クレイモア１取得フラグ) && (!$f:w12a_クレイモア３取得フラグ)  && \
					(!$f:rfp_w12_右ヴァンプ地雷ＣＡＬＬした) && \
					(!$f:rfp_PHONE_w12_右ヴァンプ地雷聞いた) && \
					(`@rp_強制ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w12_右ヴァンプ地雷ＣＡＬＬした = 1)
				command 携帯呼出 -call t:PHONE_地雷 -p PHONE_w12_右ヴァンプ地雷
			}
			if($3 == 出る) {
				if($f:rfp_w12_右ヴァンプ地雷ＣＡＬＬした){
					command 携帯呼出 -reset
					eval($f:rfp_w12_右ヴァンプ地雷ＣＡＬＬした = 0)
				}
			}
		}
}
*/

/*
trap r_sensorb_call d:PLAYER \
	-mask はいる \
	-exec {
		if( ( d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１開始 ) && \
			($w:アイテム数Ｒ[d:アイテム:センサーＢ] >= 0) ){
				@rp_P024_01_R01爆弾解体センサーＢ入手１無線デモ連続実行
		}
	}
*/

trap r_hofuku_fence d:PLAYER \
	-mask ? \
	-exec {
		print 'r_hofuku_fence'
		if( ($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１開始) && \
				(!$f:rfp_w12a_金網通り抜けた) ){
			command 無線設定 -over $3 14085 t:RPC_Ａ脚屋上ホフク金網

			if($3 == 入る) {
				if( (!$f:rfp_PC_Ａ脚屋上ホフク金網聞いた) && (!$f:rfp_Ａ脚屋上ホフク金網ＣＡＬＬした) && \
						(`@rp_強制ＣＡＬＬしてもいいよ`) ){
					command 無線設定 -call 14085 t:RPC_Ａ脚屋上ホフク金網ＣＡＬＬ d:CODEC_CALL 
					eval($f:rfp_Ａ脚屋上ホフク金網ＣＡＬＬした = 1)
				}
			}else if($3 == 出る) {
				if($f:rfp_Ａ脚屋上ホフク金網ＣＡＬＬした){
					eval($f:rfp_Ａ脚屋上ホフク金網ＣＡＬＬした = 0)
					command 無線設定 -reset
				}
			}
		}
	}


proc rp_w12c_地雷原１無線デモ１終了 {
	command パッド操作 -cancel
	eval( $f:rfp_w21a_地雷原１無線デモ１した = 1);
	eval( $f:rfp_w12c_地雷原１無線デモ１した = 1);
	command varsave $f:rfp_w21a_地雷原１無線デモ１した $f:rfp_w12c_地雷原１無線デモ１した
}

proc rp_w12c_地雷原１無線デモ１ {
	command パッド操作 -release
	command 無線設定 \
	-call 14048 \
		t:RPD_P020_01_R01_地雷原１無線デモ１ \
		d:CODEC_DEMO rp_w12c_地雷原１無線デモ１終了 \
	-cont 14085 t:RPD_P020_01_R01_地雷原１無線デモ１＿２
}

trap r_mine d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_mrx'
		if( (d:ST:P031_01_P01フォーチュン戦終了１ポリゴンデモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P036_13_R04忍者登場１３無線デモ４終了) && \ 
				(!$f:rfp_w21a_地雷原１無線デモ１した) && \
				(!$f:rfp_w12c_地雷原１無線デモ１した) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) ){
			@rp_w12c_地雷原１無線デモ１
		}
	}


trap r_hofuku_fence_through d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_hofuku_fence_through'
		if(!$f:rfp_w12a_金網通り抜けた) {
			eval($f:rfp_w12a_金網通り抜けた = 1 )
		}
	}


trap r_unko d:PLAYER \
	-mask ? \
	-exec {
		print 'r_unko'
		command 無線設定 -over $3 14085 t:RPC_うんこ
	}

trap by001 d:PLAYER \
	-mask * \
	-exec {
		if($3 == 入る){
			if(`%rand 3` < 1){
				@rp_エルード＿無線セット $3 t:RPC_奈落エルード
				eval($f:rfp_w12ac_奈落エルード地帯にいます = 1)
			}else {
				print 'r_unko'
				command 無線設定 -over $3 14085 t:RPC_うんこ
				eval($f:rfp_w12ac_奈落エルード地帯にいます = 0)
			}
		}
		if( ($3 == いる) && ($f:rfp_w12ac_奈落エルード地帯にいます) ){
				@rp_エルード＿無線セット $3 t:RPC_奈落エルード
		}
		if($3 ==出る){
			if($f:rfp_w12ac_奈落エルード地帯にいます) {
				eval($f:rfp_w12ac_奈落エルード地帯にいます = 0)
				@rp_エルード＿無線セット $3 t:RPC_奈落エルード

			}else {
				command 無線設定 -over $3 14085 t:RPC_うんこ
				eval($f:rfp_w12ac_奈落エルード地帯にいます = 0)
			}
		}
	}

if($s:エリア == "w12a"){
trap by002 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_w11_昇降機エルード
	}

trap by003 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_w11_昇降機エルード
	}

trap by004 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_w11_昇降機エルード
	}

trap by005 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_w11_昇降機エルード
	}
}


trap r_box d:PLAYER \
	-mask ? \
	-exec {
		print 'r_box'
		command 無線設定 -over $3 14085 t:RPC_段差上り＿普通
	}


trap r_no_open_fence d:PLAYER \
	-mask ? \
	-exec {
		print 'r_no_open_fence'
		if($w:p_story < d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了) {
			command 無線設定 -over $3 14085 t:RPC_w12a_Ａ脚屋上鍵扉閉鎖状態
		}
	}


proc rp_敵兵がエレベータ内にいますよＣＡＬＬ終了 {
	eval($f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬ聞いた = 1)
	command varsave $f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬ聞いた
}

if($s:エリア == "w12c"){
	trap r_elv_inside ? \
		-mask ? \
		-exec {
			if(d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始){

				if($2 != d:PLAYER){
					if($3 == 入る){
					eval($w:rfp_w12c_エレベータ内敵数 = $w:rfp_w12c_エレベータ内敵数 + 1)
					}else {
						eval($w:rfp_w12c_エレベータ内敵数 = $w:rfp_w12c_エレベータ内敵数 - 1)
					}
			}
		}
	}

	trap r_elv_inside d:PLAYER \
		-mask * \
		-exec {
			if(d:ST:P024_01_R01爆弾解体センサーＢ入手１無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始){

				if(  ($3 == 入る) || ($3 == いる)  ){
					if( ($w:rfp_w12c_エレベータ内敵数 >= 1) && \
							($w:アラートモード != d:ALERT_MODE_ALERT) && \
							(!$f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬした) && \
							(!$f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬ聞いた) && \
							(`@rp_通常ＣＡＬＬしてもいいよ` ) ){
						eval($f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬした = 1)
						command 無線設定 -call 14085 t:RPC_敵兵がエレベータ内にいますよＣＡＬＬ \
							d:CODEC_CALL rp_敵兵がエレベータ内にいますよＣＡＬＬ終了
					}
				}
				if( ($3 == いる) && ($f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬした) ){
					if($w:rfp_w12c_エレベータ内敵数 <= 0){
						if(`%無線状態` == 2){
							command 無線設定 -reset
						}
						eval($f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬした = 0)
					}
				}
				if( ($3 == 出る) && ($f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬした) ){
					eval($f:rfp_w12c_敵兵がエレベータ内にいますよＣＡＬＬした = 0)
					if(`%無線状態` == 2){
						command 無線設定 -reset
					}
				}
		}
	}
}


