/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"
extern command ゲットゲームステータス

//フラグをおっ立ててやるのだ。もうビンビン。
if($w:t_story < d:ST_船内入った){
	eval($w:t_story = d:ST_船内入った)
}

@rt_オタコンＳＡＶＥ＿デフォルト無線セット
command 無線設定 \
	-base 14112 t:RTO_オタコンデフォルト


/*--------------------------------------------
食堂。ミジン切り
----------------------------------------------*/
command マップ設定 四階食堂 -s {

	//食堂の思い出
	trap r_syokudo d:PLAYER \
		-mask ? \
		-exec {
			print 'r_syokudo'
			command 無線設定 \
				-over $3 14112 t:RTO_食堂
		}

	//死体のはらわた。
	trap r_deadman d:PLAYER \
		-mask ? \
		-exec {
			print 'r_deadman'
			command 無線設定 \
				-over $3 14112 t:RTO_食堂死体
		}

	//きゃんしかめらぁ？
	trap r_s_camera d:PLAYER \
		-mask ? \
		-exec {
			print 'r_s_camera'
			command 無線設定 \
				-over $3 14112 t:RTO_w01d_監視カメラ
	}

}


/*--------------------------------------------
倉庫。放置プレイ
----------------------------------------------*/
	//死体のはらわた。
command マップ設定 四階倉庫 -s {
	trap r_souko d:PLAYER \
		-mask ? \
		-exec {
			print 'r_souko'
			command 無線設定 \
				-over $3 14112 t:RTO_倉庫
		}

proc rt_w01dクリアリングＣＡＬＬ終了 {
	eval($f:rft_w01dクリアリング聞いた = 1)
	command varsave $f:rft_w01dクリアリング聞いた
}

	trap r_souko d:PLAYER \
		-mask いる \
		-exec {
			if( (!$f:rft_w01dクリアリングＣＡＬＬ発動中) && (!$f:rft_w01dクリアリング聞いた) ){
				command ゲットゲームステータス $i:game_staus
				//くりゃーりんぐ中
				if( ($i:game_staus & 2) && (`@rt_重要ＣＡＬＬしてもいいよ`) && ($f:rft_w01d_クリアリング中でございます)){
					command 無線設定 \
						-call 14112 t:RTO_w01dクリアリングＣＡＬＬ d:CODEC_CALL rt_w01dクリアリングＣＡＬＬ終了
					eval($f:rft_w01dクリアリングＣＡＬＬ発動中 = 1)
				}
			}

		if($f:rft_w01dクリアリングＣＡＬＬ発動中){
				command ゲットゲームステータス $i:game_staus
			//くりゃーりんぐじゃなくなった、のでＣＡＬＬリセット
			if(!$f:rft_w01d_クリアリング中でございます){
				command 無線設定 -reset
				eval($f:rft_w01dクリアリングＣＡＬＬ発動中 = 0)
			}
		}
	}
}


/*--------------------------------------------
廊下。立たされ小僧はどこにいる？
----------------------------------------------*/
/*
proc 赤外線センサー一発氏に防止透明壁北廊下 {
	chara 透明壁 一発爆死回避さん北 \
	-position 5000,9300,-11875 2000 \
		      7000,9300,-11875 2000 \
	-attribute (d:HZX_SEG_NO_ENEMY | d:HZX_SEG_NO_BULLET | d:HZX_SEG_NO_MISSILE | \
				d:HZX_SEG_NO_C4 | d:HZX_SEG_NO_RECOIL | d:HZX_SEG_NO_HARITSUKI | \
				d:HZX_SEG_NO_DISP_RADAR | d:HZX_SEG_NO_BULLETHOLE | d:HZX_SEG_NO_SPRAY | \
				d:HZX_SEG_NO_ENEMY_EYES)
	mesg 透明壁 一発爆死回避さん北 off
}

proc 赤外線センサー一発氏に防止透明壁南廊下 {
	chara 透明壁 一発爆死回避さん南 \
	-position 5000,9300,-11625 2000 \
		      7000,9300,-11625 2000 \
	-attribute (d:HZX_SEG_NO_ENEMY | d:HZX_SEG_NO_BULLET | d:HZX_SEG_NO_MISSILE | \
				d:HZX_SEG_NO_C4 | d:HZX_SEG_NO_RECOIL | d:HZX_SEG_NO_HARITSUKI | \
				d:HZX_SEG_NO_DISP_RADAR | d:HZX_SEG_NO_BULLETHOLE | d:HZX_SEG_NO_SPRAY | \
				d:HZX_SEG_NO_ENEMY_EYES)
	mesg 透明壁 一発爆死回避さん南 off
}

proc RTO_赤外線ＣＡＬＬ機関室以外終了 {
	//パッドオフをオンにするとは！
	print 'radio end. pad on!! むぎゃっち！'
	eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1)
	eval($f:rft_赤外線強制ＣＡＬＬ発動中 = 0)
	mesg 透明壁 一発爆死回避さん北 off
	mesg 透明壁 一発爆死回避さん南 off
	command パッド操作 -cancel
	command プレイヤー無敵解除
	mesg コマンダー 敵兵セット 視界オン
	//いっぺん聞いたらウザイからならさないよん
	command varsave $f:rft_赤外線ＣＡＬＬ機関室以外聞いた
}


proc 赤外線ＣＡＬＬ{
	//撃たれるといやーん
	mesg コマンダー 敵兵セット 視界オフ
	command プレイヤー無敵セット
	//command StreamStopAll	//ふごふご停止
	//print '$f:rft_w01d赤外線立ち止まったろん:'$f:rft_w01d赤外線立ち止まったろん
	eval($f:rft_赤外線強制ＣＡＬＬ発動中 = 1)
	command 無線設定 \
		-call 14112 t:RTO_赤外線ＣＡＬＬ機関室以外 d:CODEC_DEMO RTO_赤外線ＣＡＬＬ機関室以外終了
}

proc 赤外線センサー強制ＣＡＬＬ通り過ぎ版トラップセット {
	trap r_senser_kamikaze d:PLAYER \
		-mask 入る \
		-exec {
			print 'r_senser_kamikaze in'
			eval($f:rft_w01d赤外線立ち止まったろん = 0)

			if( (!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた) && (`@rt_重要ＣＡＬＬしてもいいよ`) ){
				print '俺は殺すぜディレイをむきき'
				mesg delay 強制ＣＡＬＬディレイ kill

				//発見〜危険の間はカッコ悪いから鳴らさないよ。えへへ。
				command ゲットゲームステータス $i:game_staus
				if( ($w:アラートモード == 1) || ($i:game_staus & 1) ){
					print '危険だからＣＡＬＬはやめてみました。'
					//eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1) //へんだから聞いた事にしちゃうよ
					mesg 透明壁 一発爆死回避さん北 off
					mesg 透明壁 一発爆死回避さん南 off

				}else{
					mesg 透明壁 一発爆死回避さん北 on
					mesg 透明壁 一発爆死回避さん南 on
					command パッド操作 -release
					@赤外線ＣＡＬＬ
				}
			}
	}

	trap r_senser_kamikaze d:PLAYER \
		-mask 出る \
		-exec {
			print 'r_senser_kamikaze out'
			mesg 透明壁 一発爆死回避さん北 off
			mesg 透明壁 一発爆死回避さん南 off
	}
}


proc 赤外線センサー強制ＣＡＬＬ立ち止まり版トラップセット {
	trap r_senser_stop d:PLAYER \
		-mask * \
		-exec {
//			print 'r_senser_stop in あちょちょーむ'

	if($3 == 入る){
			print 'r_senser_stop in あちょちょーむ'
			eval($f:rft_w01d赤外線立ち止まったろん = 1)

			if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
				chara delay 強制ＣＡＬＬディレイ -time 100 -exec { 
					print 'exec delay ぬらりひょん:'
					if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){

						//危険モードもしくは敵に見つかってる時は素通りりす。
						command ゲットゲームステータス $i:game_staus
						if( ($w:アラートモード == 1) || ($i:game_staus & 1) ){
							print 'だって危険だし'

						//delay 発効。強制ＣＡＬＬ。
						}else {
//							command パッド操作 -release
							@赤外線ＣＡＬＬ
						}
					}
				}
			}
			//ＳＥＮＤ無線だばだば
			command 無線設定 \
				-over $3 14112 t:RTO_赤外線機関室以外

		}else if($3 == 出る){
			print 'r_senser_stop out ずばばん'
			if($f:rft_赤外線強制ＣＡＬＬ発動中){
				command 無線設定 -reset
				command プレイヤー無敵解除
				mesg コマンダー 敵兵セット 視界オン
				eval($f:rft_赤外線強制ＣＡＬＬ発動中 = 0)
			}
			if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
				eval($f:rft_w01d赤外線立ち止まったろん = 0)
				mesg delay 強制ＣＡＬＬディレイ kill
			}
			command 無線設定 \
				-over $3 14112 t:RTO_赤外線機関室以外
		}
	}
}

proc 赤外線角ＣＡＬＬ判定いる {
	if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
		command プレイヤー状態取得 -d
		if($status & $i:rft_w01d覗き込み方向){
//			print 'かなり覗き込んでますな！:'$i:rft_w01d覗き込み方向

			//発見〜危険の間はカッコ悪いから鳴らさないよ。えへへ。
			command ゲットゲームステータス $i:game_status
			if( ($w:アラートモード == 1) || ($i:game_staus & 1) ){
				print '危険だからＣＡＬＬはやめてみました。'
				eval($f:rft_赤外線強制ＣＡＬＬ発動中 = 0)
			}else{
				eval($f:rft_w01d赤外線立ち止まったろん = 1)
				@赤外線ＣＡＬＬ
			}
		}
	}
}


proc 赤外線角ＣＡＬＬ判定出る{
	if( (!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた) && \
		($f:rft_赤外線強制ＣＡＬＬ発動中)  ){
//			command パッド操作 -release
		command 無線設定 -reset
		command プレイヤー無敵解除
		mesg コマンダー 敵兵セット 視界オン

		eval($f:rft_w01d赤外線立ち止まったろん = 0)
		eval($f:rft_赤外線強制ＣＡＬＬ発動中 = 0)
	}
}

proc 赤外線センサー角北トラップセット {
	trap r_senser_corner_n d:PLAYER \
		-mask いる \
		-exec {
			eval($i:rft_w01d覗き込み方向 = d:PFLAG_PEEP_L)
			@赤外線角ＣＡＬＬ判定いる
		}

	trap r_senser_corner_n d:PLAYER \
		-mask 出る \
		-exec {
			@赤外線角ＣＡＬＬ判定出る
	}
}

proc 赤外線センサー角南トラップセット {
	trap r_senser_corner_s d:PLAYER \
		-mask いる \
		-exec {
			eval($i:rft_w01d覗き込み方向 = d:PFLAG_PEEP_R)
			@赤外線角ＣＡＬＬ判定いる
		}

	trap r_senser_corner_s d:PLAYER \
		-mask 出る \
		-exec {
			@赤外線角ＣＡＬＬ判定出る
		}
}
*/

//何度もＣＡＬＬうざいよ。
proc rt_赤外線ＣＡＬＬ機関室以外通常終了 {
	eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 0)
	eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1)
	command varsave $f:rft_赤外線ＣＡＬＬ機関室以外聞いた
}

proc 赤外線通常ＣＡＬＬ {
	if(!$f:w01d_赤外線１スイッチ破壊フラグ){
		eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 1)
		command 無線設定 \
			-call 14112 t:RTO_赤外線ＣＡＬＬ機関室以外 d:CODEC_CALL rt_赤外線ＣＡＬＬ機関室以外通常終了
	}
}

proc 赤外線センサー近くトラップセット {
	trap r_senser_stop d:PLAYER \
		-mask * \
		-exec {
//			print 'r_senser_stop in あちょちょーむ'

			if($3 == 入る){
				print 'r_senser_stop in あちょちょーむ'
				eval($f:rft_w01d赤外線立ち止まったろん = 1)

				//$6 Z座標規制はマップ切り替え時に発生する出る入るを無効にするため
				if( ( ($6 < -12500 ) || ($6 > -11000) ) && (!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた) && \
						(!$f:rft_w01d赤外線ＣＡＬＬ発動中) && \
						(`@rt_重要ＣＡＬＬしてもいいよ`) ){
					@赤外線通常ＣＡＬＬ
				}
				//ＳＥＮＤ無線だばだば
				command 無線設定 \
					-over $3 14112 t:RTO_赤外線機関室以外

			//$6 Z座標規制はマップ切り替え時に発生する出る入るを無効にするため
			}else if( ($3 == 出る) &&  ( ($6 < -12500 ) || ($6 > -11000) ) ){
				print 'r_senser_stop out ずばばん'
				if($f:rft_w01d赤外線ＣＡＬＬ発動中){
					command 無線設定 -reset
					eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 0)
				}
				command 無線設定 \
					-over $3 14112 t:RTO_赤外線機関室以外
			}
			if($3 == いる){
				@rt_w01d_赤外線ＣＡＬＬ中キャンセルしようかやめようか
			}
		}

	//くぐっちゃったらＣＡＬＬなし。
	trap r_senser_kamikaze d:PLAYER \
		-mask 入る \
		-exec {
			eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1)
		}
}


//角に張りついた時のＣＡＬＬ
proc 赤外線角ＣＡＬＬ判定いる {
	if( (!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた) && (!$f:w01d_赤外線１スイッチ破壊フラグ) ){
		command プレイヤー状態取得 -d
		if( ($status & $i:rft_w01d覗き込み方向) && (!$f:rft_w01d赤外線ＣＡＬＬ発動中) && \
				(`@rt_重要ＣＡＬＬしてもいいよ`) ){
//			print 'かなり覗き込んでますな！:'$i:rft_覗き込み方向
			eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 1)
			@赤外線通常ＣＡＬＬ
		}
/* はりつき解除でＣＡＬＬ解除、はやめとこう
		command プレイヤー状態取得
			if( ($f:rft_w01d赤外線ＣＡＬＬ発動中) && \
					(!($status & d:PFLAG_CAUTION) ) ){
				command 無線設定 -reset
				eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 0)
			}
*/
	}
}

proc rt_w01d_赤外線ＣＡＬＬ中キャンセルしようかやめようか {
	if( ($f:rft_w01d赤外線ＣＡＬＬ発動中) && \
			( (`%無線状態` == 0) || \
			 ($f:w01d_赤外線１スイッチ破壊フラグ) ) ){
			command 無線設定 -reset
			eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 0)
	}
}

proc 赤外線センサー角北トラップセット {
	trap r_senser_corner_n d:PLAYER \
		-mask * \
		-exec {
			if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた) {
				if($3 == いる){
					eval($i:rft_w01d覗き込み方向 = d:PFLAG_PEEP_L)
					@赤外線角ＣＡＬＬ判定いる
					@rt_w01d_赤外線ＣＡＬＬ中キャンセルしようかやめようか
				}

				if($3 == 出る){
					//print 'スネークのＸ座標らしいぞ:'$4
					//隣接赤外線ＣＡＬＬエリアにいく時はキャンセルしない。
					if( ($4 < 5000) && ($f:rft_w01d赤外線ＣＡＬＬ発動中) ){
						command 無線設定 -reset
						eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 0)
					}
				}
			}
	}
}


proc 赤外線センサー角南トラップセット {
	trap r_senser_corner_s d:PLAYER \
		-mask * \
		-exec {
			if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
				if($3 == いる){
					eval($i:rft_w01d覗き込み方向 = d:PFLAG_PEEP_R)
					@赤外線角ＣＡＬＬ判定いる
					@rt_w01d_赤外線ＣＡＬＬ中キャンセルしようかやめようか
				}
				if($3 == 出る){
					//print 'スネークのＸ座標らしいぞ:'$4:rft_w01d_スネークＸ座標
					//隣接赤外線ＣＡＬＬエリアにいく時はキャンセルしない。
					if($4 < 5000){
						eval($f:rft_w01d赤外線ＣＡＬＬ発動中 = 0)
						command 無線設定 -reset
					}
				}
			}
	}
}




proc 階段魔人トラップセット {
	if($w:ゲーム設定 >= d:LEVEL_HARD){
		trap r_majin d:PLAYER \
			-mask * \
			-exec {
				if($3 == 入る){
					print 'r_majin in'
					command 無線設定 \
						-over $3 14112 t:RTO_階段魔人

				}else if($3 == 出る){
					command 無線設定 \
						-over $3 14112 t:RTO_階段魔人

					if($f:rft_w01d階段魔人ＣＡＬＬ発動中){
						command 無線設定 -reset 
						eval($f:rft_w01d階段魔人ＣＡＬＬ発動中 = 0)
					}

				}else if($3 == いる){
					if( ($w:アラートモード == 1) && (!$f:rft_w01d階段魔人ＣＡＬＬ発動中) && \
							(!$f:rft_w01d階段魔人聞いた) && (`@rt_重要ＣＡＬＬしてもいいよ`) ){
						command 無線設定 \
							-call 14112 t:RTO_w01d階段魔人ＣＡＬＬ d:CODEC_CALL
						eval($f:rft_w01d階段魔人ＣＡＬＬ発動中 = 1)

					}else if( ($w:アラートモード != 1) && ($f:rft_w01d階段魔人ＣＡＬＬ発動中) ){
						command 無線設定 -reset 
						eval($f:rft_w01d階段魔人ＣＡＬＬ発動中 = 0)
					}
				}

			}
	}
}


command マップ設定 \
	四階廊下_北半分  -s {
print 'きたきたきたはんぶんあらほらさっさー'
/*
	//まさかの一発死回避用いんびぢぶる。
	if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
		@赤外線センサー一発氏に防止透明壁北廊下
	}
	//赤外線そんさー。一発死にって方言？
	@赤外線センサー強制ＣＡＬＬ通り過ぎ版トラップセット

	//赤外線さんそー。立ち止まったアナタに素敵なプレゼント
	@赤外線センサー強制ＣＡＬＬ立ち止まり版トラップセット

	//赤外線せんそー。覗き好きな人用
	@赤外線センサー角北トラップセット
*/
	//赤外線センサー近く
	@赤外線センサー近くトラップセット

	//赤外線せんそー。覗き好きな人用
	@赤外線センサー角北トラップセット

	//蒸気パイプ注意んぐ
	trap r_near_pipe d:PLAYER \
	-mask ? \
	-exec {
			print 'r_near_pipe'
			command 無線設定 \
				-over $3 14112 t:RTO_蒸気パイプ近く
	}

}


command マップ設定 \
	四階廊下_南半分  -s {
print 'みなみはんぶんあらほらさっさー'
/*
	//まさかの一発死回避用いんびぢぶる。
	if(!$f:rft_赤外線ＣＡＬＬ機関室以外聞いた){
		@赤外線センサー一発氏に防止透明壁南廊下
	}
	//赤外線そんさー。一発死にって方言？
	@赤外線センサー強制ＣＡＬＬ通り過ぎ版トラップセット

	//赤外線さんそー。立ち止まったアナタに素敵なプレゼント
	@赤外線センサー強制ＣＡＬＬ立ち止まり版トラップセット 

	@赤外線センサー角南トラップセット
*/
	//赤外線センサー近く
	@赤外線センサー近くトラップセット
	@赤外線センサー角南トラップセット

//階段魔人警報。ＣＡＬＬにする？
//	@階段魔人トラップセット

}


