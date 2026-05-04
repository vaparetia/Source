/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ゲットエマ手繋ぎ状況
extern command ゲットエマ状態
extern command 水中エマＯ２ゲット
extern command ゲットエマライフ
extern command ゲットエマ座標
extern command VecLen



trap r_kirai01 d:PLAYER \
	-mask * \
	-exec {
		@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く $f:w31c_機雷01爆発した
		@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く $f:w31c_機雷01爆発した
	}

trap r_kirai02 d:PLAYER \
	-mask * \
	-exec {
		if ( $w:ゲーム設定 >= d:LEVEL_EASY ) {
			@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く $f:w31c_機雷02爆発した
			@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く $f:w31c_機雷02爆発した
		}
	}

trap r_kirai03 d:PLAYER \
	-mask * \
	-exec {
		@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く $f:w31c_機雷03爆発した
		@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く $f:w31c_機雷03爆発した
	}

trap r_kirai04 d:PLAYER \
	-mask * \
	-exec {
		if ( $w:ゲーム設定 >= d:LEVEL_EASY ) {
			@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く $f:w31c_機雷04爆発した
			@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く $f:w31c_機雷04爆発した
		}
	}

trap r_kirai56 d:PLAYER \
	-mask * \
	-exec {
		if ( $w:ゲーム設定 >= d:LEVEL_EASY ) {
			if( ($f:w31c_機雷05爆発した) && ($f:w31c_機雷06爆発した) ) {
				@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く 1 
				@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く 1
			}else {
				@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く 0 
				@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く 0
			}
		}
	}




chara 携帯端末 くそメール -skin htl -file t:mobile

proc PHONE_w31_ヴァンプ地雷終了 {
	eval($f:rfp_PHONE_w31_ヴァンプ地雷聞いた = 1)
	command varsave $f:rfp_PHONE_w31_ヴァンプ地雷聞いた
}

if( ($w:p_story >= d:ST:P059_03_R01エマ休憩３無線機デモ１終了) && \
		($w:アイテム数Ｒ[d:アイテム:携帯電話] > 0) ){
	trap r_mine d:PLAYER \
		-mask ? \
		-exec {
			if( ($3 == 入る) && \
					( (!$f:w31c_クレイモアフラグ[0]) || (!$f:w31c_クレイモアフラグ[1]) || (!$f:w31c_クレイモアフラグ[2])) && \
					(!$f:rfp_w31_ヴァンプ地雷ＣＡＬＬした) && \
					(`@rp_強制ＣＡＬＬしてもいいよ`) && \
					(!$f:rfp_PHONE_w31_ヴァンプ地雷聞いた) ){
				eval($f:rfp_w31_ヴァンプ地雷ＣＡＬＬした = 1)
				command 携帯呼出 -call t:PHONE_地雷 -p PHONE_w31_ヴァンプ地雷終了
			}
			if($3 == 出る) {
				print 'r_mine_out'
				if($f:rfp_w31_ヴァンプ地雷ＣＡＬＬした){
					command 携帯呼出 -reset
					eval($f:rfp_w31_ヴァンプ地雷ＣＡＬＬした = 0)
				}
			}
		}
}



if(d:ST:P055_04_P03ヴァンプ戦前４ポリゴンデモ３終了 <= $w:p_story && \
			$w:p_story <  d:ST:P057_01_P01ヴァンプ戦終了１ポリゴンデモ１開始 ){
	command 無線設定 \
	-base 14085 t:RPC_大佐デフォルト＿ヴァンプ戦中
}else {
	@rpc_大佐＿泳ぎ無線セット	//水中で水中会話が出るよ。多分。
}
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_エマ＿デフォルト無線セット



trap ev101 d:PLAYER \
	-mask 入る \
	-exec {
			eval($f:rfp_w31c_ロッカー部屋にはいったぷり = 1)
			print 'ぶろぶろぼげーん'
	}

trap r_emma_room d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る) {
			print 'r_emma_room in $i:プレイヤー位置Ｙ' $i:プレイヤー位置Ｙ
			eval($f:rfp_w31c_エマ部屋にいまそかり = 1)
			if($w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始){
				command 無線設定 -over $3 14085 t:RPC_エマ救出前＿エマ部屋
			}
	
		}else if($3 == 出る) {
			if( $i:プレイヤー位置Ｙ >= -6500){
				print 'r_emma_room out $i:プレイヤー位置Ｙ' $i:プレイヤー位置Ｙ
				eval($f:rfp_w31c_エマ部屋にいまそかり = 0)
				if($w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始){
					command 無線設定 -over $3 14085 t:RPC_エマ救出前＿エマ部屋
				}
			}
		}
	}


trap r_raku_suimen d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る) {
			print 'r_raku_suimen in'
			eval($f:rfp_らくらく水面にいましゃ = 1)

		}else if($3 == 出る) {
			print 'r_raku_suimen out'
			eval($f:rfp_らくらく水面にいましゃ = 0)
		}
	}


/*
trap r_emma_room d:PLAYER \
	-mask いる \
	-exec {

		command ゲットエマ状態 $b:うじょじょる
//		print 'うじょじょる:' $b:うじょじょる
			command ゲットエマ座標 $i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数
			command VecLen	$i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数 \
							$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ
		print 'くそえまとの距離:' $status

	}
*/


//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_w31c_影縫い引っかかったぼよーん = 1)


	//えろっかー。開いてる時に主観で見るとばかにされる。
		trap r_peeping_emma d:PLAYER \
			-mask いる \
//			-state d:TRP_STATE_WATER_SURFACE \
			-exec {
				if(!$f:rfp_w31c_ロッカー部屋にはいったぷり){
					if(`%プレイヤー状態取得` & d:PFLAG_WATER_SURFACE) {

						if(`%カメラ視界チェック -camera 0x03 -pos -14500,-7000,-253650`){
							eval($w:rfp_w31c_エマノゾキカウンター = $w:rfp_w31c_エマノゾキカウンター + 1)
							print 'エマ覗いてます。:' $w:rfp_w31c_エマノゾキカウンター
							eval($f:rfp_w31c_エマのぞいた = 1)
							
							if( ($w:rfp_w31c_エマノゾキカウンター >= 500) && (!$f:rfp_w31c_エマのぞきＣＡＬＬ中) && \
									(`@rp_通常ＣＡＬＬしてもいいよ`) && \
									(!$f:rfp_w31c_エマのぞきＣＡＬＬ聞いた) ){
								command 無線設定 -call 14085 t:RPC_w31c_エマのぞきＣＡＬＬ d:CODEC_CALL 
								eval($f:rfp_w31c_エマのぞきＣＡＬＬ中 = 1)
							}
							
						}else {
							eval($f:rfp_w31c_エマのぞいた = 0)
						}

				}else {
					if($f:rfp_w31c_エマのぞきＣＡＬＬ中){
						command 無線設定 -reset
						eval($f:rfp_w31c_エマのぞきＣＡＬＬ中 = 0)
					}
					eval($w:rfp_w31c_エマノゾキカウンター = 0)
				}
			}
		}

	trap r_peeping_emma d:PLAYER \
		-mask ? \
		-exec {
			if(!$f:rfp_w31c_ロッカー部屋にはいったぷり){
				print 'r_peeping_emma'
				command 無線設定 -over $3 14085 t:RPC_エマのぞき
			}
		}


	proc rp_w31c_エマと初出水ＣＡＬＬ終了 {
		command パッド操作 -cancel
		command varsave $f:rfp_w31c_エマと初出水ＣＡＬＬした
	}

if($w:p_story >= d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了) {
	trap r_emma_call d:PLAYER \
		-mask いる \
			-exec {
				if( (!$f:入出水処理中フラグ) && (!$f:rfp_w31c_エマと初出水ＣＡＬＬした) && \
						(`@rp_強制ＣＡＬＬしてもいいよ`) ){
					command パッド操作 -release
					command 無線設定 -call 14085 \
														t:RPC_w31c_エマと初出水ＣＡＬＬ d:CODEC_DEMO \
														rp_w31c_エマと初出水ＣＡＬＬ終了
					eval($f:rfp_w31c_エマと初出水ＣＡＬＬした = 1)
				}
		}
}

if($w:p_story >= d:ST:P057_02_R01ヴァンプ戦終了２無線機デモ１終了){
	trap r_near_pool d:PLAYER \
		-mask ? \
		-exec {
			print 'r_near_pool'
			@rp_プリスキン＿無線セット 0 $3 t:RPS_w31c_プール近く
		}
}

/*
				command プレイヤー状態取得
				eval($i:rfp_プレイヤー状態 = $status)
				if( ($i:rfp_プレイヤー状態 & d:PFLAG_SUBJECT) && ($i:プレイヤー位置Ｙ <= -6890) ){
					print 'うらららー$w:プレイヤー方向:'$w:プレイヤー方向
					command 無線設定 -over $3 14112 t:RTO_前かがみ
*/

/*`%カメラ視界チェック -camera 0x03 -pos 1000,-8500,-57500`
		trap r_peeping_emma d:PLAYER \
			-mask いる \
			-state 0,1,2,3,4,5,6,8 \
			-exec {
				print 'えろくなーい'
			command 無線設定 -over $3 14112 t:RTO_ロッカーlk000
			}
*/



