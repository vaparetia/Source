/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"


//@rp_大佐デフォルト無線セット
	command 無線設定 -base 14085 t:RPC_w18a_デフォルト

@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

if($w:ライデン懸垂回数 < 400) {
	chara プロック連続実行  w18a_武羅鎖我裡拳硬鬼 \
		-proc rp_懸垂監視野郎 \
		-time -1
}

if( (!$f:rfp_w18a_爆弾解体カウンタースタートした) && \
			(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
				$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１終了) ){
		eval($i:rfp_w18a_爆弾解体開始時刻 = $i:プレイタイム)
		eval($f:rfp_w18a_爆弾解体カウンタースタートした = 1)
		command varsave $i:rfp_w18a_爆弾解体開始時刻 $f:rfp_w18a_爆弾解体カウンタースタートした
}


//奈落エルードセット
/*
trap by001 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by002 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by003 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}


trap by020 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by021 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by022 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by023 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by024 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by025 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by026 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}

trap by027 d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}
*/

trap r_dead_elude d:PLAYER \
	-mask ? \
	-exec {
		print 'r_dead_elude in'
		if($3 == 入る){
			print 'r_dead_elude in'
			eval($f:rfp_w18a_奈落エルード地帯にいる = 1)
		}else if($3 == 出る) {
			print 'r_dead_elude out'
			eval($f:rfp_w18a_奈落エルード地帯にいる = 0)
		}
	}


//解体後戻ってきたら時間たったとみなしごはっち。
if( ( ($f:w18a_爆弾処理完了1) && ($f:w18a_爆弾処理完了2) && ($f:w18a_爆弾処理完了3) )  && \
		(!$f:rfp_w18a_爆弾処理完了後時間たった) ){
	eval($f:rfp_w18a_爆弾処理完了後時間たった = 1)
}


trap r_door_shell2 d:PLAYER \
	-mask ? \
	-exec {
		print 'r_door_shell2'
		if($w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了) {
				command 無線設定 -over $3 14085 t:RPC_w18a_シェル２への扉
		}
	}

proc rp_w18a_ハッチＣＡＬＬ終了 {
	eval($f:rfp_w18a_ハッチＣＡＬＬ中 = 0)
}

proc rp_ハッチを開けた無線系処理 {
	if(!$f:rfp_w18a_ハッチ開けました) {
		eval($f:rfp_w18a_ハッチ開けました = 1)
	}
	if($f:rfp_w18a_ハッチＣＡＬＬ中){
		command 無線設定 -reset
		eval($f:rfp_w18a_ハッチＣＡＬＬ中 = 0)
	}
}

trap r_hatch d:PLAYER \
	-mask ? \
	-exec {
		print 'r_hatch'
		if( ( (!$f:w18a_爆弾処理完了1) && (!$f:w18a_爆弾処理完了2) && (!$f:w18a_爆弾処理完了3) ) ){
				command 無線設定 -over $3 14085 t:RPC_Ｄ脚＿ハッチ
		}
		if( ($3 == 入る) && (!$f:rfp_w18a_ハッチ開けました) && (!$f:rfp_w18a_ハッチＣＡＬＬ中) && \
				(!$f:rfp_PC_Ｄ脚＿ハッチ聞いた) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) ){
			eval($f:rfp_w18a_ハッチＣＡＬＬ中 = 1)
			chara delay ハッチＣＡＬＬディレイたんハァハァ \
				-time 60 \
				-exec {
					if( (!$f:rfp_w18a_ハッチ開けました) && \
							(!$f:rfp_PC_Ｄ脚＿ハッチ聞いた) && \
							($f:rfp_w18a_ハッチＣＡＬＬ中) && \
							(`@rp_強制ＣＡＬＬしてもいいよ`) ){
								command 無線設定 \
									-call 14085 t:RPC_Ｄ脚＿ハッチＣＡＬＬ d:CODEC_CALL rp_w18a_ハッチＣＡＬＬ終了
					}
			}
		}
		if( ($3 == 出る) && ($f:rfp_w18a_ハッチＣＡＬＬ中) && (!$f:rfp_w18a_ハッチ開けました) ){
			eval($f:rfp_w18a_ハッチＣＡＬＬ中 = 0)
			mesg delay ハッチＣＡＬＬディレイたんハァハァ kill
			command 無線設定 -reset
		}
	}






