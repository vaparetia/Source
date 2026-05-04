/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ＮＰＣスネークのライフ
extern command ＮＰＣスネークの状態
extern command 無線設定
extern command メニュー設定

@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット

proc rp_w43a_連続発狂ＣＡＬＬ起動 {
	if($w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数 <= 0){
		@rp_連続発狂ＣＡＬＬ初期化
	}
	chara プロック連続実行 発狂ＣＡＬＬ連続実行 \
		-proc rp_連続発狂ＣＡＬＬ \
		-time -1
}

if($w:p_story < d:ST:P073_01_R01通路Ａ１無線機デモ１開始) {
	print 'ぐあだらでれん'
	command 無線設定 -speech 1
}

if($w:p_story < d:ST:P073_01_R01通路Ａ１無線機デモ１開始) {
	trap r_call_start d:PLAYER \
		-mask いる \
		-exec {
				if( (!$f:rfp_w43a_連続発狂ＣＡＬＬ起動した) && \
						($f:ロードチェックＯＮフラグ) ){
					eval($f:rfp_w43a_連続発狂ＣＡＬＬ起動した = 1)
					@rp_w43a_連続発狂ＣＡＬＬ起動
				}
		}
}


proc rp_P073_01_R01通路Ａ１無線機デモ１ {
//	command StreamStopAll
//	command 無線設定 -speech 0

	if($w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数 >= 7){
		print 'ローズさん強制ＣＡＬＬ とらずにはおれぬよ'
		command パッド操作 -release
		command 無線設定 -reset
		eval($f:rfp_w43a_ローズさん強制ＣＡＬＬ中 = 1)
		command 無線設定 \
		-call 14096 \
			t:RPD_P073_01_R01_通路Ａ１無線機デモ１ \
			d:CODEC_DEMO rp_P073_01_R01通路Ａ１無線機デモ１終了 \
			-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) 
		eval($w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数 = $w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数 +1)

	}else{
		if(!$f:rfp_w43a_ローズさん強制ＣＡＬＬ中){
			command 無線設定 -reset
		}
		eval($w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数 = $w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数 +1)
		print 'ローズさん通常ＣＡＬＬ取るが吉！:' $w:rfp_w43a_ローズさんＣＡＬＬ鳴らした回数
		command 無線設定 \
		-call 14096 \
			t:RPD_P073_01_R01_通路Ａ１無線機デモ１ \
			d:CODEC_CALL rp_P073_01_R01通路Ａ１無線機デモ１終了 \
			-f ( 	d:CODEC_I_AC_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK ) 
	}
}

proc rp_P073_01_R01通路Ａ１無線機デモ１終了 {
	//w43a.gcl内に記述
	@通路Ａ１無線機デモ１終了処理
}

if($w:p_story < d:ST:P073_01_R01通路Ａ１無線機デモ１開始) {
	trap r_rose_call d:PLAYER \
		-mask * \
		-exec {
			if( ($w:p_story < d:ST:P073_01_R01通路Ａ１無線機デモ１開始) && \
					( ($w:rfp_w43a_PC_発狂ＣＡＬＬ聞いた回数 >= 5 ) || ( ($i:ステージプレイ時間 / d:FRAME_RATE) >= 600) ) ){

				if($3 == 入る) {
//					command StreamIsPlay
//					print 'r_rose_call in' $status
					mesg プロック連続実行 発狂ＣＡＬＬ連続実行 kill
				}

				if($3 == 出る) {
					if(!$f:rfp_w43a_ローズさん強制ＣＡＬＬ中){
						command 無線設定 -reset
					}
					eval($f:rfp_w43a_ローズＣＡＬＬ中 = 0)
					eval($f:rfp_w43a_ローズＣＡＬＬ待ち時間中 = 0)
					@rp_発狂ＣＡＬＬ待ち時間セット
					@rp_w43a_連続発狂ＣＡＬＬ起動
				}

				if($3 == いる){
					if( (!$f:rfp_w43a_ローズＣＡＬＬ中) && (!$f:rfp_w43a_ローズＣＡＬＬ待ち時間中) && \
							(`@rp_強制ＣＡＬＬしてもいいよ`) ){

						@rp_P073_01_R01通路Ａ１無線機デモ１
						eval($f:rfp_w43a_ローズＣＡＬＬ中 = 1)
					}

					if( ($f:rfp_w43a_ローズＣＡＬＬ中) && (!$f:rfp_w43a_ローズＣＡＬＬ待ち時間中) ){
						eval($w:rfp_w43a_ローズさんＣＡＬＬカウンター = $w:rfp_w43a_ローズさんＣＡＬＬカウンター + 1 )
//						print '$w:rfp_w43a_ローズさんＣＡＬＬカウンター:' $w:rfp_w43a_ローズさんＣＡＬＬカウンター

						if($w:rfp_w43a_ローズさんＣＡＬＬカウンター >= 700) {
							eval($f:rfp_w43a_ローズＣＡＬＬ中 = 0)
							eval($f:rfp_w43a_ローズＣＡＬＬ待ち時間中 = 1)
							eval($w:rfp_w43a_ローズさんＣＡＬＬカウンター = 0 )
							command rand 6
							eval($w:rfp_w43a_ローズＣＡＬＬ待ち時間 = ($status * 10) + 90 )
							print '$w:rfp_w43a_ローズＣＡＬＬ待ち時間:' $w:rfp_w43a_ローズＣＡＬＬ待ち時間
						}
				}
				if( (!$f:rfp_w43a_ローズＣＡＬＬ中) && ($f:rfp_w43a_ローズＣＡＬＬ待ち時間中) ){
					eval($w:rfp_w43a_ローズさんＣＡＬＬ待ち時間中カウンター = $w:rfp_w43a_ローズさんＣＡＬＬ待ち時間中カウンター + 1)
//					print '$w:rfp_w43a_ローズさんＣＡＬＬ待ち時間中カウンター:' $w:rfp_w43a_ローズさんＣＡＬＬ待ち時間中カウンター
					if($w:rfp_w43a_ローズさんＣＡＬＬ待ち時間中カウンター >= $w:rfp_w43a_ローズＣＡＬＬ待ち時間) {
						eval($f:rfp_w43a_ローズＣＡＬＬ待ち時間中 = 0)
						eval($w:rfp_w43a_ローズさんＣＡＬＬ待ち時間中カウンター = 0)
					}
				}
			}
		}
	}
}


if($w:p_story < d:ST:P072_01_R01大佐混乱１無線機デモ１終了) {
	eval($w:p_story = d:ST:P072_01_R01大佐混乱１無線機デモ１終了)
}


//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)
