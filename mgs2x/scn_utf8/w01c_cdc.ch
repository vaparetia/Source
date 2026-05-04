/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

extern 	command ゲットゲームステータス 
//eval($w:ゲーム設定 = d:LEVEL_EASY)

//フラグをおっ立ててやるのだ。もうビンビン。
if($w:t_story < d:ST_船内入った){
	eval($w:t_story = d:ST_船内入った)
}

@rt_オタコンＳＡＶＥ＿デフォルト無線セット
	command 無線設定 \
		-base 14112 t:RTO_オタコンデフォルト

command マップ設定 	三階 -s {
	//command extern宣言 ロッカーな私にはexternが必要なのだ？
	extern command ロッカー状態

	proc rt_監視カメラＣＡＬＬ終了 {
		eval($f:rft_w01cパッドデモ後監視カメラＣＡＬＬ発動中 = 0)
	}

	trap r_s_camera d:PLAYER \
		-mask * \
		-exec {
			if($3 == 入る){
				print 'r_s_camera in'
				command 無線設定 \
					-over $3 14112 t:RTO_w01c_監視カメラ

			}else if($3 == 出る){
				print 'r_s_camera out'
				command 無線設定 \
					-over $3 14112 t:RTO_w01c_監視カメラ
				if($f:rft_w01cパッドデモ後監視カメラＣＡＬＬ発動中){
					command 無線設定 -reset
					eval($f:rft_w01cパッドデモ後監視カメラＣＡＬＬ発動中 = 0)
				}

			}else if( ($w:ゲーム設定 <= d:LEVEL_EASY) && ($3 == いる) ){
				if( ($f:パッドデモ再生フラグ == d:PADDEMO_END) && \
						(!$f:rft_w01cパッドデモ後監視カメラＣＡＬＬした) && \
						(!$f:rft_監視カメラ聞いた) && \
						(!$f:rft_w01cパッドデモ後監視カメラＣＡＬＬ発動中) && \
						(`@rt_通常ＣＡＬＬしてもいいよ`) ){

							command 無線設定 \
								-call 14112 t:RTO_監視カメラＣＡＬＬ d:CODEC_CALL rt_監視カメラＣＡＬＬ終了
							eval($f:rft_w01cパッドデモ後監視カメラＣＡＬＬした = 1)
							eval($f:rft_w01cパッドデモ後監視カメラＣＡＬＬ発動中 = 1)

						command varsave $f:rft_w01cパッドデモ後監視カメラＣＡＬＬした
				}
			}
	}

	//ロッカー近く
	trap r_locker d:PLAYER \
		-mask ? \
		-exec {
			print 'r_locker'
			command 無線設定 \
				-over $3 14112 t:RTO_三階ロッカー
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


