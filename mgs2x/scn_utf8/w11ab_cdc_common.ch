/**************************************************
		無線でぽいいん
**************************************************/

extern command ロッカー状態


@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット


trap r_no_open_door d:PLAYER \
	-mask ? \
	-exec {
		print 'no_open_door'
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			command 無線設定 -over $3 14085 t:RPC_w11＿開かない金網扉＿潜水ライデン
		}else {
			command 無線設定 -over $3 14085 t:RPC_w11b＿開かない金網扉＿普通
		}
	}


trap r_hofuku d:PLAYER \
	-mask ? \
	-exec {
		print 'r_hofuku'
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			command 無線設定 -over $3 14085 t:RPC_ホフクポイント＿潜水ライデン
		}
	}


trap r_duct d:PLAYER \
	-mask ? \
	-exec {
		print 'r_duct'
		if($3 == 入る){
			eval($f:rfp_w11_ダクトの近くにいます = 1)

		}else if($3 == 出る){
			eval($f:rfp_w11_ダクトの近くにいます = 0)
		}

	}

trap r_mushi d:PLAYER \
	-mask ? \
	-exec {
		print 'r_mushi'
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			if($w:p_story < d:ST:P004_02_R01ノード初接続２無線デモ１終了){
				command 無線設定 -over $3 14085 t:RPC_フナムシ＿潜水
			}
		}else {
			command 無線設定 -over $3 14085 t:RPC_フナムシ＿普通
		}
	}


trap r_water d:PLAYER \
	-mask ? \
	-exec {
		print 'r_water'
		eval($f:rfp_水中入った = 1)
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			command 無線設定 -over $3 14085 t:RPC_水中＿潜水ライデン
		}else {
			command 無線設定 -over $3 14085 t:RPC_w11b_水中
		}
	}


proc rp_w11ab_初入水＿水中ＣＡＬＬ終了 {
	command パッド操作 -cancel
	command varsave $f:rfp_w11ab_初入水＿水中ＣＡＬＬした $f:rfp_PC_水中＿操作説明聞いた
}

proc rp_w11ab__初入水＿水上ＣＡＬＬディレイキャンセル {
	if($f:rfp_w11ab_初入水＿水上ＣＡＬＬディレイ発行中){
		eval($f:rfp_w11ab_初入水＿水上ＣＡＬＬディレイ発行中 = 0)
		mesg delay w11ab_初入水＿水上ＣＡＬＬディレイ kill
		command 無線設定 -reset
	}
}

proc rp_w11ab_初入水＿水上ＣＡＬＬ終了 {
	command varsave $f:rfp_PC_水中操作説明＿水上聞いた $f:rfp_PC_水中＿操作説明聞いた
}

if($w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) {
	trap r_water d:PLAYER \
		-mask * \
		-exec {
			if( ($3 == いる) && (!$f:rfp_w11ab_初入水＿水中ＣＡＬＬした) ){
				command プレイヤー状態取得
				eval($i:rfp_プレイヤー状態 = $status)
				if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
						(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE )) && \
						(`@rp_強制ＣＡＬＬしてもいいよ`) && \
						(!$f:rfp_w11ab_初入水＿水中ＣＡＬＬした) ){
					@rp_w11ab__初入水＿水上ＣＡＬＬディレイキャンセル
					command パッド操作 -release
					chara delay 水中ＣＡＬＬディレイ王 \
						-time 80 \
						-exec {
							if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
									($s:エリア == "w11a") ){
										command 無線設定 -call 14085 \
																t:RPC_w11a_初入水＿水中ＣＡＬＬ＿潜水 d:CODEC_DEMO \
																rp_w11ab_初入水＿水中ＣＡＬＬ終了
							}else {
										command 無線設定 -call 14085 \
																t:RPC_w11a_初入水＿水中ＣＡＬＬ＿普通 d:CODEC_DEMO \
																rp_w11ab_初入水＿水中ＣＡＬＬ終了
							}
						}
					eval($f:rfp_w11ab_初入水＿水中ＣＡＬＬした = 1)
				}
			}

			if( ($3 == 入る) && \
					(!$f:rfp_w11ab_初入水＿水中ＣＡＬＬした) && \
					(!$f:rfp_w11ab_初入水＿水上ＣＡＬＬディレイ発行中) && \
					(!$f:rfp_PC_水中操作説明＿水上聞いた) ){
						eval($f:rfp_w11ab_初入水＿水上ＣＡＬＬディレイ発行中 = 1)
						chara delay w11ab_初入水＿水上ＣＡＬＬディレイ -t 480 -exec {
							if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE ) && \
									(`@rp_強制ＣＡＬＬしてもいいよ`) && \
									(!$f:rfp_PC_水中操作説明＿水上聞いた) ){
								if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
										($s:エリア == "w11a") ){
									command 無線設定 -call 14085 \
											t:RPC_w11ab_初入水＿水上ＣＡＬＬ＿潜水 d:CODEC_CALL \
											rp_w11ab_初入水＿水上ＣＡＬＬ終了
								}else {
									command 無線設定 -call 14085 \
											t:RPC_w11ab_初入水＿水上ＣＡＬＬ＿普通 d:CODEC_CALL \
											rp_w11ab_初入水＿水上ＣＡＬＬ終了
								}
							}
						}
					}
			if($3 == 出る){
					@rp_w11ab__初入水＿水上ＣＡＬＬディレイキャンセル 
			}
	}
}





trap r_hashigo d:PLAYER \
	-mask ? \
	-exec {
		print 'r_hashigo'
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			command 無線設定 -over $3 14085 t:RPC_水中＿潜水ライデン＿はしご
		}else if($s:エリア == "w11b"){
			command 無線設定 -over $3 14085 t:RPC_水中＿w11b＿はしご
		}
	}

if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
	trap r_box d:PLAYER \
		-mask ? \
		-exec {
			print 'r_box'
				command 無線設定 -over $3 14085 t:RPC_段差上り＿潜水ライデン
		}

	trap r_boat d:PLAYER \
		-mask ? \
		-exec {
			print 'r_boat'
				command 無線設定 -over $3 14085 t:RPC_海底ドック潜水艇＿潜水ライデン
		}

	trap r_suits d:PLAYER \
		-mask ? \
		-exec {
			print 'r_suits'
				command 無線設定 -over $3 14085 t:RPC_ジムスーツ＿潜水ライデン
		}
}

	//ロッカー対犬。
	//一番ひだりん
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			@rp_ロッカー無線セット $3 ロッカー01 t:RPC_w11_ロッカー共通１＿潜水
		}else {
			@rp_ロッカー無線セット $3 ロッカー01 t:RPC_w11_ロッカー共通１＿普通
		}
	}
	//まんなか気分
	trap lk002 d:PLAYER \
	-mask * \
	-exec {
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			@rp_ロッカー無線セット $3 ロッカー02 t:RPC_w11_ロッカー共通２＿潜水
		}else {
			@rp_ロッカー無線セット $3 ロッカー02 t:RPC_w11_ロッカー共通２＿普通
		}
	}

	//みぎ？
	trap lk003 d:PLAYER \
	-mask * \
	-exec {
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			@rp_ロッカー無線セット $3 ロッカー03 t:RPC_w11_ロッカー共通１＿潜水
		}else {
			@rp_ロッカー無線セット $3 ロッカー03 t:RPC_w11_ロッカー共通１＿普通
		}
	}

proc rp_w11a_水密扉ＣＡＬＬ終了 {
	command パッド操作 -cancel
}

proc rp_w11a_水密扉ＣＡＬＬ {
/*
	command パッド操作 -release
	command 無線設定 \
		-call 14085 t:RPC_水密扉＿潜水ライデン d:CODEC_DEMO rp_w11a_水密扉ＣＡＬＬ終了
*/
	command 無線設定 \
		-call 14085 t:RPC_水密扉＿潜水ライデン d:CODEC_CALL 
}



//水密扉でぴょん
proc rp_水密扉＿潜水ライデン終了  {
	eval($f:rfp_PC_水密扉聞いた = 1)
	command varsave $f:rfp_PC_水密扉聞いた
}

	trap r_suimitu d:PLAYER \
		-mask ? \
		-exec {
			if($w:p_story < d:ST:P002_01_S01Ａ脚底部連結ハッチ１シナリオデモ１開始){
				command 無線設定 -over $3 14085 t:RPC_水密扉＿潜水ライデン

				//んーコールん
				if($3 == 入る){
					print 'r_suimitu in'
					if( (!$f:rfp_PC_水密扉聞いた) && (`@rp_通常ＣＡＬＬしてもいいよ`) && \
							( ($w:タンカー編クリア回数 < 1) && ($w:プラント編クリア回数 < 1) ) && \
							(!$f:rfp_w11a_水密扉ＣＡＬＬした) ){
						eval($f:rfp_w11a_水密扉ＣＡＬＬした = 1 )
						command 無線設定 \
							-call 14085 t:RPC_水密扉＿潜水ライデン d:CODEC_CALL rp_水密扉＿潜水ライデン終了
					}
				}else if($3 == 出る) {
					print 'r_suimitu out'
					 //コールキャンセル出来次第いれるよるぐ
					if($f:rfp_w11a_水密扉ＣＡＬＬした){
						print 'コールキャンセルぷりりんちょ'
						command 無線設定 -reset
						eval($f:rfp_w11a_水密扉ＣＡＬＬした = 0)
					}
				}
			}
		}

if($w:p_story < d:ST:P002_01_S01Ａ脚底部連結ハッチ１シナリオデモ１開始){
	trap dr001 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state 0 \
		-exec {
				if($f:rfp_w11a_水密扉ＣＡＬＬした){
					print 'コールキャンセルぷりりんちょ'
					command 無線設定 -reset
					eval($f:rfp_w11a_水密扉ＣＡＬＬした = 0)
				}
		}
}

proc rp_海底ドックエルード＿無線セット $:出るいるその他 {
	if( ($1 == 入る) || ($1 == 出る) ) {
		if(!(d:ST:P025_01_R01爆弾解体昇降機下１無線デモ１終了 <= $w:p_story && \
					$w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始) ){
			command プレイヤー状態取得
			//つまり水中にいない
			if( (!($status & d:PFLAG_WATER_SURFACE)) && (!($status & d:PFLAG_WATER)) ){

				print 'byond'
				if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
					command 無線設定 -over $1 14085 t:RPC_w11b_エルード＿潜水
				}else {
					command 無線設定 -over $1 14085 t:RPC_エルード＿普通
				}
			}
		}
	}
	if($:出るいるその他 == いる){
		@rp_懸垂監視野郎
	}
}



trap r_by001 d:PLAYER \
	-mask * \
	-exec {
		@rp_海底ドックエルード＿無線セット $3
	}


trap by002 d:PLAYER \
	-mask * \
	-exec {
		@rp_海底ドックエルード＿無線セット $3
	}

trap by003 d:PLAYER \
	-mask * \
	-exec {
		@rp_海底ドックエルード＿無線セット $3
	}

trap by004 d:PLAYER \
	-mask * \
	-exec {
		@rp_海底ドックエルード＿無線セット $3
	}

trap by005 d:PLAYER \
	-mask * \
	-exec {
		@rp_海底ドックエルード＿無線セット $3
	}

trap by006 d:PLAYER \
	-mask * \
	-exec {
		@rp_海底ドックエルード＿無線セット $3
	}


//しょうこうきほーるん
trap r_node d:PLAYER \
	-mask ? \
	-exec {
		print 'r_node'
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			command 無線設定 -over $3 14085 t:RPC_w11＿ノード＿潜水ライデン
		}
	}

trap r_elv d:PLAYER \
	-mask ? \
	-exec {
		print 'r_elv'
		if($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了){
			command 無線設定 -over $3 14085 t:RPC_w11＿昇降機＿潜水ライデン
		}
	}

proc rp_w11a_敵兵おっきした警告ＣＡＬＬ終了 {
//	command パッド操作 -cancel
	command varsave $f:rfp_w11a_敵兵おっきした警告ＣＡＬＬした
}

proc rp_w11a_敵兵おっきした警告ＣＡＬＬ {
	if( ($w:アラートモード != 1) && (!$f:rfp_PC_海底ドック敵覚醒聞いた) && \
			(!$f:rfp_w11a_敵兵おっきした警告ＣＡＬＬした) && \
			(`@rp_強制ＣＡＬＬしてもいいよ`) ){
//		command パッド操作 -release
		command 無線設定 \
			-call 14085 t:RPC_w11a_敵兵おっきした警告ＣＡＬＬ＿潜水ライデン d:CODEC_DEMO rp_w11a_敵兵おっきした警告ＣＡＬＬ終了
		eval($f:rfp_w11a_敵兵おっきした警告ＣＡＬＬした = 1 )
	}
}

enum rfp_w11ab_いまいるのは {
	昇降機ホール = 0,
	連結通路,
	発着口,
}

trap r_hall d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_hall'
		eval($b:rfp_w11ab_今どこにいますか = d:rfp_w11ab_いまいるのは:昇降機ホール)
	}

trap r_roka d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_roka'
		eval($b:rfp_w11ab_今どこにいますか = d:rfp_w11ab_いまいるのは:連結通路)
	}

trap r_dock d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_dock'
		eval($b:rfp_w11ab_今どこにいますか = d:rfp_w11ab_いまいるのは:発着口)
	}



