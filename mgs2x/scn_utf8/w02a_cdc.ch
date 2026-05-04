/**************************************************
		無線でぽいいん
**************************************************/
#include "cdc_common_t.h"

extern command ゲットゲームステータス
extern command ロッカー状態
@rt_オタコンＳＡＶＥ＿デフォルト無線セット


command 無線設定 \
	-base 14112 t:RTO_w02a_オタコンデフォルト

proc rt_w02a_懸垂監視 {
//print '$w:スネーク懸垂回数:' $w:スネーク懸垂回数
	@rt_懸垂ＣＡＬＬ
}

chara プロック連続実行  w02a_武羅鎖我裡拳硬鬼 \
	-proc rt_w02a_懸垂監視 \
	-time -1



trap r_raven_hakkaku d:PLAYER \
	-mask 入る \
	-exec {
		if( ($f:w02a_レイブンイベントフラグ) && (!$f:rft_レイブン影正体発覚した) ){
			eval($f:rft_レイブン影正体発覚した = 1)
			command varsave $f:rft_レイブン影正体発覚した
		}
}

trap r_raven_bibili d:PLAYER \
	-mask ? \
	-exec {
		print 'r_raven_bibili'
		command 無線設定 \
			-over $3 14112 t:RTO_w02a_天田麗文ビビリ地帯
}




proc rto_w02a_右舷ロッカー無線セット {
	//ザ・ロッカーズ trapは寄生モノ。変更があったら変更しなきゃバグリンチョ。

		// 右舷ロッカー左から１番目
	trap lk001 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk001 右舷ロッカー左から１番目'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_ロッカーlk001
	}

	// 右舷ロッカー左から２番目
	trap lk002 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk002 右舷ロッカー左から２番目'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_ロッカーlk002
	}


		// 右舷ロッカー左から３番目
	trap lk003 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk003 右舷ロッカー左から３番目'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_ロッカーlk003
	}

// 右舷ロッカー左から４番目 開かないんＺ
	trap lk004 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk004 右舷ロッカー左から４番目 開かないんＺ'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_開かないロッカーlk004
	}
	trap lk004 d:PLAYER \
		-and \
		-mask いる \
		-state 0,4 \
		-dir 2048,512 0,512 \
		-button d:ACTION_BUTTON,-1 \
		-exec {
			eval($f:rft_w02a_開かないロッカーlk004 = 1)
	}



}


//	@rt_w02a_レイブン話解除
	@rto_w02a_右舷ロッカー無線セット
	


trap r_engin_room d:PLAYER \
	-mask ? \
	-exec {
		print 'r_engin_room'
		if($3 == 入る){
			eval($f:rft_w02a_機関室にいるみょ = 1)

		}else if($3 == 出る){
			eval($f:rft_w02a_機関室にいるみょ = 0)
		}
	}




//-----------------------------------------
//赤外線トラップ的な今日このごろ
//------------------------------------------
//-17750,-5000,-26000 -157500,-5000,-26000
proc rt_w02a_赤外線センサー一発氏に防止透明壁 {

	chara 透明壁 一発爆死回避さん \
	-position -17750,-5000,-25875 2500 \
		      -15750,-5000,-25875 2500 \
	-map 左舷小部屋 \
	-attribute (d:HZX_SEG_NO_ENEMY | d:HZX_SEG_NO_BULLET | d:HZX_SEG_NO_MISSILE | \
				d:HZX_SEG_NO_C4 | d:HZX_SEG_NO_RECOIL | d:HZX_SEG_NO_HARITSUKI | \
				d:HZX_SEG_NO_DISP_RADAR | d:HZX_SEG_NO_BULLETHOLE | d:HZX_SEG_NO_SPRAY | \
				d:HZX_SEG_NO_ENEMY_EYES)
	mesg 透明壁 一発爆死回避さん off
}

proc rt_w02a_赤外線ＣＡＬＬ機関室終了 {
	//パッドオフをオンにするとは！
	print 'radio end. pad on!! むぎゃっち！'
	eval($f:rft_赤外線ＣＡＬＬ機関室聞いた = 1)
//	eval($f:rft_w02a_赤外線強制ＣＡＬＬ発動中 = 0)
	mesg 透明壁 一発爆死回避さん off
	command パッド操作 -cancel

//	command プレイヤー無敵解除
//	mesg コマンダー 敵兵セット 視界オン
	//いっぺん聞いたらウザイからならさないよん
	command varsave $f:rft_赤外線ＣＡＬＬ機関室聞いた
}


proc rt_赤外線ＣＡＬＬ {
	//撃たれるといやーん
//	mesg コマンダー 敵兵セット 視界オフ
//	command プレイヤー無敵セット
	command パッド操作 -release

	//command StreamStopAll	//ふごふご停止
	//print '$f:rft_w01d赤外線立ち止まったろん:'$f:rft_w01d赤外線立ち止まったろん
	eval($f:rft_w02a_赤外線強制ＣＡＬＬ発動中 = 1)
	command 無線設定 \
		-call 14112 t:RTO_赤外線ＣＡＬＬ機関室 d:CODEC_DEMO rt_w02a_赤外線ＣＡＬＬ機関室終了
}

proc rt_w02a_赤外線センサー強制ＣＡＬＬ通り過ぎ版トラップセット {
	trap r_senser_kamikaze d:PLAYER \
		-mask 入る \
		-exec {
			print 'r_senser_kamikaze in'
			eval($f:rft_w02a_赤外線立ち止まったろん = 0)

			if( (!$f:rft_赤外線ＣＡＬＬ機関室聞いた) && (`@rt_重要ＣＡＬＬしてもいいよ`) && \
					($w:t_story < d:ST_機関室赤外線センサークリア) ){
				//発見～危険の間はカッコ悪いから鳴らさないよ。えへへ。

				command ゲットゲームステータス $i:game_staus
				if( ($w:アラートモード == 1) || ($i:game_staus & 1) ){
					print '危険だからＣＡＬＬはやめてみました。'
					//eval($f:rft_赤外線ＣＡＬＬ機関室以外聞いた = 1) //へんだから聞いた事にしちゃうよ
					mesg 透明壁 一発爆死回避さん off

				}else{
					mesg 透明壁 一発爆死回避さん on
					@rt_赤外線ＣＡＬＬ
				}
			}
	}

	trap r_senser_kamikaze d:PLAYER \
		-mask 出る \
		-exec {
			print 'r_senser_kamikaze out'
			mesg 透明壁 一発爆死回避さん off
	}
}


proc rt_w02a_赤外線センサー強制ＣＡＬＬ立ち止まり版トラップセット {
	trap r_senser_stop d:PLAYER \
		-mask * \
		-exec {

			if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){

				if($3 == いる){
					command ゲットゲームステータス $i:game_staus
					eval($w:rfp_w02a_赤外線に立ちつくすカウント = $w:rfp_w02a_赤外線に立ちつくすカウント +1)

//print '$w:rfp_w02a_赤外線に立ちつくすカウント:' $w:rfp_w02a_赤外線に立ちつくすカウント
//print '$w:プレイヤー方向:' $w:プレイヤー方向

					if( ($w:rfp_w02a_赤外線に立ちつくすカウント >= 60) && \
							(!$f:rft_赤外線ＣＡＬＬ機関室聞いた) && \
							($w:t_story < d:ST_機関室赤外線センサークリア) && \
							(1792 <= $w:プレイヤー方向 && $w:プレイヤー方向 <= 2304) && \
							(!$f:rft_w02a_赤外線強制ＣＡＬＬ発動中) && \
						  (! ( ($w:アラートモード == d:ALERT_MODE_ALERT) || ($i:game_staus & d:STATE_DETECT) ) ) && \
							(`@rt_重要ＣＡＬＬしてもいいよ`) ){
						eval($f:rft_w02a_赤外線立ち止まったろん = 1)
						@rt_赤外線ＣＡＬＬ
					}
				}
				if($3 == 出る){
					eval($w:rfp_w02a_赤外線に立ちつくすカウント = 30)
					eval($f:rft_w02a_赤外線立ち止まったろん = 0)
				}
				if($3 == 入る){
					eval($f:rft_w02a_赤外線立ち止まったろん = 1)
				}
			}

			if( ($3 == 入る) || ($3 == 出る) ){
				command 無線設定 \
					-over $3 14112 t:RTO_赤外線機関室
			}
	}
}

/*
	if($3 == 入る){
			print 'r_senser_stop in あちょちょーむ'
			eval($f:rft_w02a_赤外線立ち止まったろん = 1)

			if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){
				chara delay 強制ＣＡＬＬディレイ -time 100 -exec { 
					print 'exec delay ぬらりひょん:'
					if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){


						//危険モードもしくは敵に見つかってる時は素通りりす。
						command ゲットゲームステータス $i:game_staus
						if( ($w:アラートモード == 1) || ($i:game_staus & 1) ){
							print 'だって危険だし'

						//delay 発効。強制ＣＡＬＬ。
						}else {


//							command パッド操作 -release
							@rt_赤外線ＣＡＬＬ
						}
					}
				}
			}
			//ＳＥＮＤ無線だばだば
			command 無線設定 \
				-over $3 14112 t:RTO_赤外線ＣＡＬＬ機関室

		}else if($3 == 出る){
//			print 'r_senser_stop out ずばばん'
//			if($f:rft_w02a_赤外線強制ＣＡＬＬ発動中){
				command 無線設定 -reset
				command プレイヤー無敵解除
				mesg コマンダー 敵兵セット 視界オン
//				eval($f:rft_w02a_赤外線強制ＣＡＬＬ発動中 = 0)
			}
			if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){
				eval($f:rft_w02a_赤外線立ち止まったろん = 0)
				mesg delay 強制ＣＡＬＬディレイ kill
			}
			command 無線設定 \
				-over $3 14112 t:RTO_赤外線ＣＡＬＬ機関室
		}
	}
*/

proc rt_w02a_赤外線角ＣＡＬＬ判定いる {
	if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){
		command プレイヤー状態取得 -d
		if( ($status & $i:rft_w02a_覗き込み方向) && (`@rt_重要ＣＡＬＬしてもいいよ`) && \
				($w:t_story < d:ST_機関室赤外線センサークリア) ){
//			print 'かなり覗き込んでますな！:'$i:rft_覗き込み方向

			//発見～危険の間はカッコ悪いから鳴らさないよ。えへへ。
			command ゲットゲームステータス $i:game_staus
			if( ($w:アラートモード == 1) || ($i:game_staus & 1) ){
				print '危険だからＣＡＬＬはやめてみました。'
//				eval($f:rft_w02a_赤外線強制ＣＡＬＬ発動中 = 0)
			}else{
				eval($f:rft_w02a_赤外線立ち止まったろん = 1)
				@rt_赤外線ＣＡＬＬ
			}
		}
	}
}


proc rt_w02a_赤外線センサー角東トラップセット {
	trap r_senser_corner_e d:PLAYER \
		-mask いる \
		-exec {
			eval($i:rft_w02a_覗き込み方向 = d:PFLAG_PEEP_L)
			@rt_w02a_赤外線角ＣＡＬＬ判定いる
		}
}


proc rt_w02a_赤外線センサー角西トラップセット {
	trap r_senser_corner_w d:PLAYER \
		-mask いる \
		-exec {
print 'いるよーん'
			eval($i:rft_w02a_覗き込み方向 = d:PFLAG_PEEP_R)
			@rt_w02a_赤外線角ＣＡＬＬ判定いる
		}
}



/*
//角に張りついた時のＣＡＬＬ
proc rt_w02a_赤外線角ＣＡＬＬ判定いる {
	if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){
		command プレイヤー状態取得 -d
		if( ($status & $i:rft_w02a_覗き込み方向) && (!$f:rft_w02a_赤外線ＣＡＬＬ発動中) ){
			print 'かなり覗き込んでますな！:'$i:rft_覗き込み方向
			eval($f:rft_w02a_赤外線ＣＡＬＬ発動中 = 1)
			@rt_w02a_赤外線通常ＣＡＬＬ
		}
	}
}

*/


proc rto_w02a_左舷ロッカー無線セット {
	//ザ・ロッカーズ trapは寄生モノ。変更があったら変更しなきゃバグリンチョ。

	// 左舷北 さわり男
	trap lk011 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk011 左上 さわり男'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_ロッカーlk011
	}

		// 左舷真ん中
	trap lk012 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk012 四次元女'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_ロッカーlk012
	}

	// 左舷南
	trap lk013 d:PLAYER \
	-mask ? \
	-exec {
			print 'lk013 あかん'
			command 無線設定 \
				-over $3 14112 t:RTO_w02a_開かないロッカーlk013
	}

	trap lk013 d:PLAYER \
		-and \
		-mask いる \
		-state 0,4 \
		-dir 3072,512 1024,512 \
		-button d:ACTION_BUTTON,-1 \
		-exec {
			eval($f:rft_w02a_開かないロッカー開けた_lk013 = 1)
	}

}


command マップ設定 左舷小部屋 -set {

	@rto_w02a_左舷ロッカー無線セット

	#ifdef d:GERMAN_TRIAL
	#else
		//まさかの一発死回避用いんびぢぶる。
		if(!$f:rft_赤外線ＣＡＬＬ機関室聞いた){
			@rt_w02a_赤外線センサー一発氏に防止透明壁
		}
		//赤外線そんさー。一発死にって方言？
		@rt_w02a_赤外線センサー強制ＣＡＬＬ通り過ぎ版トラップセット

		//赤外線さんそー。立ち止まったアナタに素敵なプレゼント
		@rt_w02a_赤外線センサー強制ＣＡＬＬ立ち止まり版トラップセット

		//赤外線せんそー。覗き好きな人用
		@rt_w02a_赤外線センサー角東トラップセット
		@rt_w02a_赤外線センサー角西トラップセット
	#endif
//	eval($f:rft_赤外線見た = 1)
//	eval($w:武器弾数[2] = -1)


}

