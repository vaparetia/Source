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

chara 携帯端末 くそメール -skin htl -file t:mobile


@rpc_大佐＿泳ぎ無線セット	//水中で水中会話が出るよ。多分。


@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_エマ＿デフォルト無線セット


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
proc rp_エマ監視 {
	command ゲットエマ状態 $b:rfp_ゲットされたエマ状態
	print '$b:rfp_ゲットされたエマ状態:' $b:rfp_ゲットされたエマ状態
}


chara プロック連続実行  エマ監視くん \
	-proc rp_エマ監視 \
	-time -1
*/


proc rp_w31b_初入水＿水中ＣＡＬＬ終了 {
	command パッド操作 -cancel
	command varsave $f:rfp_w31b_初入水＿水中ＣＡＬＬした $f:rfp_PC_水中＿操作説明聞いた
}

proc rp_w31b__初入水＿水上ＣＡＬＬディレイキャンセル {
	if($f:rfp_w31b_初入水＿水上ＣＡＬＬディレイ発行中){
		eval($f:rfp_w31b_初入水＿水上ＣＡＬＬディレイ発行中 = 0)
		mesg delay w31b_初入水＿水上ＣＡＬＬディレイ kill
		command 無線設定 -reset
	}
}

proc rp_w31b_初入水＿水上ＣＡＬＬ終了 {
	command varsave $f:rfp_PC_水中操作説明＿水上聞いた $f:rfp_PC_水中＿操作説明聞いた
}

if($w:p_story < d:ST:P058_01_P01エマ救出１ポリゴンデモ１開始) {
	trap r_water_call d:PLAYER \
		-mask * \
		-exec {
			if( ($3 == いる) && (!$f:rfp_w31b_初入水＿水中ＣＡＬＬした) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) ){
				command プレイヤー状態取得
				eval($i:rfp_プレイヤー状態 = $status)
				if( (!$f:入出水処理中フラグ) && \
						($i:rfp_プレイヤー状態 & d:PFLAG_WATER) && \
						(!($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE )) && \
						(!$f:rfp_w31b_初入水＿水中ＣＡＬＬした) ){
					@rp_w31b__初入水＿水上ＣＡＬＬディレイキャンセル
					command パッド操作 -release
					command パッド操作 -release
					eval($f:rfp_w31b_初入水＿水中ＣＡＬＬした = 1)
					chara delay 水中ＣＡＬＬディレイ王 \
						-time 60 \
						-exec {
							command 無線設定 -call 14085 \
																t:RPC_w31b_初入水＿水中ＣＡＬＬ d:CODEC_DEMO \
																rp_w31b_初入水＿水中ＣＡＬＬ終了
					}
				}
			}

			if( ($3 == 入る) && \
					(!$f:rfp_w31b_初入水＿水中ＣＡＬＬした) && \
					(!$f:rfp_w31b_初入水＿水上ＣＡＬＬディレイ発行中) && \
					(!$f:rfp_PC_水中操作説明＿水上聞いた) ){
						eval($f:rfp_w31b_初入水＿水上ＣＡＬＬディレイ発行中 = 1)
						chara delay w31b_初入水＿水上ＣＡＬＬディレイ -t 480 -exec {
							if( ($i:rfp_プレイヤー状態 & d:PFLAG_WATER_SURFACE ) && \
									(`@rp_通常ＣＡＬＬしてもいいよ`) && \
									(!$f:rfp_PC_水中操作説明＿水上聞いた) ){
									command 無線設定 \
										-call 14085 \
										t:RPC_w31b_初入水＿水上ＣＡＬＬ d:CODEC_CALL \
										rp_w31b_初入水＿水上ＣＡＬＬ終了
							}
						}
					}
			if($3 == 出る){
					@rp_w31b__初入水＿水上ＣＡＬＬディレイキャンセル 
			}
	}
}

if($w:ゲーム設定 >= d:LEVEL_EASY ) {
	trap r_kirai_1 d:PLAYER \
		-mask * \
		-exec {
			@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く $f:w31b_機雷01爆発した
			@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く $f:w31b_機雷01爆発した
		}
}

trap r_kirai_2 d:PLAYER \
	-mask * \
	-exec {
		if($w:ゲーム設定 >= d:LEVEL_NORMAL){
			if( ($f:w31b_機雷02爆発した) && ($f:w31b_機雷03爆発した) ) {
				@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く 1 
				@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く 1
			}else {
				@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く 0 
				@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く 0
			}
		}else {
			@rp_水中機雷無線セット $3 14085 t:RPC_水中機雷近く $f:w31b_機雷02爆発した
			@rp_水中機雷無線セット $3 14180 t:RPS_水中機雷近く $f:w31b_機雷02爆発した
		}
	}

//開かない水密扉東
trap r_no_open_door_wdr001 d:PLAYER \
	-mask * \
	-exec {
			@rp_水中水密扉共通 $3 $f:rfp_w31b_開かない水密扉開けたwdr001 t:RPC_水中開かない水密扉
	}
	trap wdr001 d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state d:TRP_STATE_WATER \
		-exec {
			eval($f:rfp_w31b_開かない水密扉開けたwdr001 = 1)
		}


//開かない水密扉西
trap r_no_open_door_wdr004 d:PLAYER \
	-mask * \
	-exec {
			@rp_水中水密扉共通 $3 $f:rfp_w31b_開かない水密扉開けたwdr004 t:RPC_水中開かない水密扉
	}
		trap wdr004 d:PLAYER \
			-mask いる \
			-button d:ACTION_BUTTON \
			-state d:TRP_STATE_WATER \
			-exec {
			eval($f:rfp_w31b_開かない水密扉開けたwdr004 = 1)
	}



//あいちゃう扉東
trap r_open_door_wdr002 d:PLAYER \
	-mask * \
	-exec {
			@rp_水中水密扉共通 $3 $f:rfp_w31b_開く水密扉開けたwdr002 t:RPC_水中開く水密扉
	}
	trap wdr002_R d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state d:TRP_STATE_WATER \
		-exec {
			eval($f:rfp_w31b_開く水密扉開けたwdr002 = 1)
	}

//あいちゃう扉西
trap r_open_door_wdr003 d:PLAYER \
	-mask * \
	-exec {
			@rp_水中水密扉共通 $3 $f:rfp_w31b_開く水密扉開けたwdr003 t:RPC_水中開く水密扉
	}
	trap wdr003_R d:PLAYER \
		-mask いる \
		-button d:ACTION_BUTTON \
		-state d:TRP_STATE_WATER \
		-exec {
			eval($f:rfp_w31b_開く水密扉開けたwdr003 = 1)
	}



trap r_no_open_card_door d:PLAYER \
	-mask ? \
	-exec {
			print 'r_no_open_card_door'
			command 無線設定 -over $3 14085 t:RPC_水中＿カード扉
	}


trap r_north d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_north in'
			eval($s:rfp_w31b_いまいるところ = "north")
	}

trap r_south d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_south in'
			eval($s:rfp_w31b_いまいるところ = "south")
	}

trap r_west d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_west in'
			eval($s:rfp_w31b_いまいるところ = "west")
	}


trap r_peter d:PLAYER \
	-mask ? \
	-exec {
			print 'r_peter'
			if($w:p_story < d:ST:P057_01_P01ヴァンプ戦終了１ポリゴンデモ１開始){
				@rp_プリスキン＿無線セット 0 $3 t:RPS_ピーター死体
			}
	}


//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)

proc rp_P053_01_R01息継ぎ１無線機デモ１ {
	eval( $f:rfp_w31b_P053_01_R01息継ぎ１無線機デモ１聞いた = 1 );
	command パッド操作 -release
	command 無線設定 \
	-call 14180 \
		t:RPD_P053_01_R01_息継ぎ１無線機デモ１ \
		d:CODEC_DEMO rp_P053_01_R01息継ぎ１無線機デモ１終了
}
proc rp_P053_01_R01息継ぎ１無線機デモ１終了 {
	command パッド操作 -cancel
	command varsave $f:rfp_w31b_P053_01_R01息継ぎ１無線機デモ１聞いた
}


//P053_01_R01息継ぎ１無線機デモ１開始
trap r_otacon_call d:PLAYER \
	-mask 入る \
	-exec {
		print 'r_otacon_call'

		if( (d:ST:P050_06_R03タンカー編整理６無線機デモ３終了 <= $w:p_story && \
					$w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				(!$f:rfp_w31b_P053_01_R01息継ぎ１無線機デモ１聞いた) ){
			@rp_P053_01_R01息継ぎ１無線機デモ１
		}
	}

trap r_otacon_call_last d:PLAYER \
	-mask いる \
	-exec {
		if( (!$f:入出水処理中フラグ) && \
				(d:ST:P050_06_R03タンカー編整理６無線機デモ３終了 <= $w:p_story && \
					$w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				(!$f:rfp_w31b_P053_01_R01息継ぎ１無線機デモ１聞いた) ){
			@rp_P053_01_R01息継ぎ１無線機デモ１
		}
	}
		

proc rp_PHONE_フナムシ終了 {
	eval($f:rfp_PHONE_フナムシ聞いた = 1)
	command varsave $f:rfp_PHONE_フナムシ聞いた
}


if( $w:p_story >= d:ST:P062_08_P04エマとＡＩ８ポリゴンデモ４終了){
	trap r_mushi d:PLAYER \
		-mask ? \
		-exec {
			if($f:エマ存在フラグ) {
				print 'r_mushi'
				command 無線設定 -over $3 14085 t:RPC_フナムシ＿エマ虫いや

				if( ($3 == 入る) && ($f:rfp_w31b_エマ虫嫌がりカウント開始) && \
						( ( ($i:プレイタイム - $i:rfp_w31b_エマ虫嫌がり開始時刻) / d:FRAME_RATE) >=  60) && \
						(!$f:rfp_PHONE_フナムシ聞いた) && \
						(!$f:rfp_PHONE_フナムシＣＡＬＬした) && \
						(`@rp_強制ＣＡＬＬしてもいいよ`) ){
							eval($f:rfp_PHONE_フナムシＣＡＬＬした = 1)
							command 携帯呼出 -call t:PHONE_フナムシ -p rp_PHONE_フナムシ終了
				}
			}
		}

	trap r_mushi d:PLAYER \
		-mask いる \
		-exec {
			if($f:エマ存在フラグ) {
				if(!$f:rfp_w31b_エマ虫いやがった){
					command ゲットエマ状態 $b:rfp_ゲットされたエマ状態
					if($b:rfp_ゲットされたエマ状態 == d:EMA_F_I_DISLIKE_WORMS){
						eval($f:rfp_w31b_エマ虫いやがった = 1)
						if(!$f:rfp_w31b_エマ虫嫌がりカウント開始){
							eval($f:rfp_w31b_エマ虫嫌がりカウント開始 = 1)
							eval($i:rfp_w31b_エマ虫嫌がり開始時刻 = $i:プレイタイム)
							command varsave $f:rfp_w31b_エマ虫嫌がりカウント開始 $i:rfp_w31b_エマ虫嫌がり開始時刻
						}
					}
				}
			if( ($w:武器 == d:武器:凍結スプレー) && (!$f:rfp_PHONE_フナムシＣＡＬＬした) ){
				eval($f:rfp_PHONE_フナムシＣＡＬＬした = 1)
			}
		}
	}
}


