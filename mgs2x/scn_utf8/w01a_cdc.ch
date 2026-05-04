/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

extern command ゲットゲームステータス
@rt_オタコンＳＡＶＥ＿デフォルト無線セット


//フラグをおっ立ててやるのだ。もうビンビン。
if($w:t_story < d:ST_船内入った){
	eval($w:t_story = d:ST_船内入った)
}


/*--------------------------------
比較的誰にも言えないロッカー部屋の秘密
-----------------------------------*/

command マップ設定 ロッカー部屋 \
-set{

	command 無線設定 \
		-base 14112 t:RTO_オタコンデフォルト

	//ザ・ロッカーズ trapは寄生モノ。変更があったら変更しなきゃバグリンチョ。
	//左上 ロッカー１
	trap lk000 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk000 左上'
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk000
	}

	//えろっかー。開いてる時に主観で見るとばかにされる。
		trap lk000 d:PLAYER \
			-mask いる \
//			-dir 3072,512 1024,512 \
			-dir 2048,512 \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				command ロッカー状態 -n ロッカー１
				if($status & 2){
					print 'もうエロエロっす！'
						command 無線設定 \
						-over $3 14112 t:RTO_前かがみ
					}
			}

		trap lk000 d:PLAYER \
			-mask いる \
			-state 0,1,2,3,4,5,6,8 \
			-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk000
			}


	//左下 ロッカー２
	trap lk001 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk001 左下'
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk001
	}

	//中央左下 ロッカー３
	trap lk002 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk002 中央左下'
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk002
	}

	//真中右下 ロッカー４
	trap lk003 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk003 中央右下'
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk003
	}

	//右側上 ロッカー５
	trap lk004 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk004 右側上 ロッカー５'
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk004
	}

	//右下
	trap lk005 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk005 右側下 ロッカー６'
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk005
	}

	//えろっかー。開いてる時に主観で見るとばかにされる。
		trap lk005 d:PLAYER \
			-mask いる \
//			-dir 3072,512 1024,512 \
			-dir 0,512 \
			-state d:TRP_STATE_SUBJECT \
			-exec {
				command ロッカー状態 -n ロッカー６
				if($status & 2){
					print 'もうエロエロっす！'
						command 無線設定 \
						-over $3 14112 t:RTO_前かがみ
					}
			}

		trap lk005 d:PLAYER \
			-mask いる \
			-state 0,1,2,3,4,5,6,8 \
			-exec {
			command 無線設定 \
				-over $3 14112 t:RTO_ロッカーlk005
			}


	//真中左上 壊れている ロッカー７
	trap lk006 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk006 右側上 ロッカー７ 壊れ'
			command 無線設定 \
				-over $3 14112 t:RTO_開かないロッカーlk006
	}

	//真中右上 壊れている ロッカー８
	trap lk007 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk008 右側上 ロッカー８ 壊れ'
			command 無線設定 \
				-over $3 14112 t:RTO_開かないロッカーlk007
	}

proc rt_w01aクリアリングＣＡＬＬ終了 {
	eval($f:rft_w01aクリアリング聞いた = 1)
	command varsave $f:rft_w01aクリアリング聞いた
}

	trap r_locker_room d:PLAYER \
		-mask * \
		-exec {
			if($3 == 入る){
				command 無線設定 \
					-base 14112 t:RTO_w01aロッカールームデフォルト

			}else if($3 == 出る){
				command 無線設定 \
					-base 14112 t:RTO_オタコンデフォルト

				//部屋から出たらＣＡＬＬリセット
				if($f:rft_w01aクリアリングＣＡＬＬ発動中){
					command 無線設定 -reset
					eval($f:rft_w01aクリアリングＣＡＬＬ発動中 = 0)	//うざいから消す？
//					eval($f:rft_w01aクリアリングＣＡＬＬ発動した = 0)	//うざいから消す？
				}

			}else if($3 == いる) {
				if( (!$f:rft_w01aクリアリングＣＡＬＬ発動中) && \
						(!$f:rft_w01aクリアリング聞いた) ){
					command ゲットゲームステータス $i:game_staus
					//くりゃーりんぐ中
					if( ($i:game_staus & 2) && \
							(`@rt_通常ＣＡＬＬしてもいいよ`) && \
							($f:rft_w01a_クリアリングしまくってやがる) ){
						command 無線設定 \
							-call 14112 t:RTO_w01aクリアリングＣＡＬＬ d:CODEC_CALL rt_w01aクリアリングＣＡＬＬ終了
						eval($f:rft_w01aクリアリングＣＡＬＬ発動中 = 1)
					}
				}

			if($f:rft_w01aクリアリングＣＡＬＬ発動中){
				command ゲットゲームステータス $i:game_staus
				command プレイヤー状態取得
				//くりゃーりんぐじゃなくなった、ロッカー入ったのでＣＡＬＬリセット
				if( (!($i:game_staus & 2)) || ($status & 128) || (!$f:rft_w01a_クリアリングしまくってやがる) ){
					command 無線設定 -reset
					eval($f:rft_w01aクリアリングＣＡＬＬ発動中 = 0)
					//eval($f:rft_w01aクリアリングＣＡＬＬ発動した = 0)
				}
			}
		}
	}

}



/*--------------------------------
そこはかとなく不思議な１Ｆ廻廊部の謎
-----------------------------------*/
command マップ設定 １Ｆ廻廊部  \
-set{

	//蒸気パイプ注意んぐ
	trap r_near_pipe d:PLAYER \
	-mask ? \
	-exec {
			print 'r_near_pipe'
			command 無線設定 \
				-over $3 14112 t:RTO_蒸気パイプ近く
	}

	//とれちゃうドア
	trap no_open_door d:PLAYER \
	-mask ? \
	-exec {
			print 'no_open_door'
			command 無線設定 \
				-over $3 14112 t:RTO_とれちゃうのデスカー
	}

	//足跡ちゃんズ
	trap r_foot_stamp d:PLAYER \
	-mask ? \
	-exec {
			print 'r_foot_stamp'
			command 無線設定 \
				-over $3 14112 t:RTO_足跡
	}

	//右舷開かないの水密扉であることよ。
	trap r_no_open_door_right d:PLAYER \
		-mask ? \
		-exec {
			print 'r_no_open_door_right'
				command 無線設定 \
				-over $3 14112 t:RTO_w01a右舷開かない水密扉近く
	}
	trap dr003 d:PLAYER \
		-mask いる -button d:ACTION_BUTTON -state 0 \
		-exec {
			eval($f:rft_w01a開かない水密扉開けた = 1)		//fukushima
	}
}


//eval($w:t_story = d:ST_船内入った)
//eval($w:武器弾数[2] = -1)

