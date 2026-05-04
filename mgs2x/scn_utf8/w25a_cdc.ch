/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ゲットカサッカライフ
extern command ゲットカサッカステータス

proc rp_ハリアー戦中レーダー使用可能ＣＡＬＬ {
	eval($f:rfp_PC_ハリアー戦＿レーダー聞いた = 1)
	command varsave $f:rfp_PC_ハリアー戦＿レーダー聞いた
}

if ( $w:ゲーム設定  <= d:LEVEL_NORMAL ) {
	if ( $f:ハリアー戦中 ) {
		if( ($w:w25a_ハリアー戦プレイ回数 >= 10) && \
				(!$f:rfp_ハリアー戦中レーダー使用可能ＣＡＬＬした) && \
				(!$f:rfp_PC_ハリアー戦＿レーダー聞いた) && \
				(!($w:コンフィグ設定 & d:CONFIG_RADAR_OFF) ) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) ){
			// レーダーＯＮ
				eval($f:rfp_ハリアー戦中レーダー使用可能ＣＡＬＬした = 1)
				command 無線設定 -call 14085 t:RPC_ハリアー戦中レーダー使用可能ＣＡＬＬ \
					d:CODEC_CALL rp_ハリアー戦中レーダー使用可能ＣＡＬＬ
		}
	}
}


proc r_w25a_赤外線センサー一つも壊してません {
	if( (!$f:制御装置破壊フラグ[0]) && \
		(!$f:制御装置破壊フラグ[1]) && \
		(!$f:制御装置破壊フラグ[2]) && \
		(!$f:制御装置破壊フラグ[3]) && \
		(!$f:制御装置破壊フラグ[4]) && \
		(!$f:制御装置破壊フラグ[5]) && \
		(!$f:制御装置破壊フラグ[6]) && \
		(!$f:制御装置破壊フラグ[7]) && \
		(!$f:制御装置破壊フラグ[8]) && \
		(!$f:制御装置破壊フラグ[9]) && \
		(!$f:制御装置破壊フラグ[10]) && \
		(!$f:制御装置破壊フラグ[11]) && \
		(!$f:制御装置破壊フラグ[12]) && \
		(!$f:制御装置破壊フラグ[13]) ){
		return 1
	}else {
		return 0
	}
}



proc rp_w25a_ＰＳＧ１ハマリＣＡＬＬ終了 {
	eval($f:rfp_w25a_ＰＳＧ１ハマリＣＡＬＬ聞いた = 1)
	command varsave $f:rfp_w25a_ＰＳＧ１ハマリＣＡＬＬ聞いた
}


proc rp_w25a_ＰＳＧ１ハマリ防止監視 {
	if( ( ( ($i:プレイタイム - $i:rfp_w25a_狙撃イベント開始時刻) / d:FRAME_RATE) >= 600) && \
			(!$f:rfp_w25a_ＰＳＧ１ハマリ防止ＣＡＬＬした) && \
			(!$f:rfp_w25a_ＰＳＧ１ハマリＣＡＬＬ聞いた) && \
			($w:武器弾数Ｒ[d:武器:ＰＳＧ１] > 0) && \
			(`@rp_通常ＣＡＬＬしてもいいよ`) && \
			(!`@r_w25a_赤外線センサー一つも壊してません`) && \
			( (!$f:制御装置破壊フラグ[4]) || \
				( (!$f:制御装置破壊フラグ[12]) && ($w:ゲーム設定 >= d:LEVEL_EXTREME) ) ) ){
		eval($f:rfp_w25a_ＰＳＧ１ハマリ防止ＣＡＬＬした = 1)
		command 無線設定 -call 14180 t:RPS_w25a_ＰＳＧ１ハマリＣＡＬＬ \
			d:CODEC_CALL rp_w25a_ＰＳＧ１ハマリＣＡＬＬ終了
		mesg プロック連続実行  w25a_ＰＳＧ１ハマリ防止監視さん kill
	}
}


if( ($w:p_story < d:ST:シェル１シェル２連絡橋赤外線センサークリア) || \
		(!$f:rfp_w25a_ＰＳＧ１ハマリＣＡＬＬ聞いた) ){
	chara プロック連続実行  w25a_ＰＳＧ１ハマリ防止監視さん \
		-proc rp_w25a_ＰＳＧ１ハマリ防止監視 \
		-time -1
	if( (!$f:rfp_w25a_狙撃イベントカウントスタートした) && \
			($w:武器弾数Ｒ[d:武器:ＰＳＧ１] > 0) ){
		eval($f:rfp_w25a_狙撃イベントカウントスタートした = 1)
		eval($i:rfp_w25a_狙撃イベント開始時刻 = $i:プレイタイム)
		command varsave $i:rfp_w25a_狙撃イベント開始時刻 $f:rfp_w25a_狙撃イベントカウントスタートした
	}
}


proc rp_赤外線狙撃イベント終了時無線処理 {
	command 無線設定 -allclear
	command 無線設定 -base 14085 t:RPC_w25a_大佐デフォルト＿狙撃中
	@rp_ローズデフォルト無線セット
}


//@rp_大佐デフォルト無線セット

if(d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:シェル１シェル２連絡橋赤外線センサークリア){
		command 無線設定 -base 14085 t:RPC_w25a_大佐デフォルト＿狙撃中

}else if(d:ST:P045_01_P01ハリアー登場１ポリゴンデモ１終了 <= $w:p_story && \
			$w:p_story <  d:ST:P046_01_P01ハリアー戦勝利１ポリゴンデモ１開始 ){
		command 無線設定 -base 14085 t:RPC_大佐デフォルト＿ハリアー戦中

}else {
//	@rpc_大佐＿通常無線セット＿連絡橋
	@rpc_大佐＿通常無線セット＿連絡橋かもめ
}

@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット


//eval($w:武器弾数Ｒ[4] = -1)
if( ($w:p_story >= d:ST:P042_01_R01シェル１シェル２連絡橋１無線デモ１終了) && \
		($w:武器弾数Ｒ[d:武器:ＰＳＧ１] >= 0) && \
			(!$f:rfp_w25a_狙撃イベントカウントスタートした) ){
	eval($i:rfp_w25a_狙撃イベント開始時刻 = $i:プレイタイム) 
	eval($f:rfp_w25a_狙撃イベントカウントスタートした = 1)
	command varsave $i:rfp_w25a_狙撃イベント開始時刻 $f:rfp_w25a_狙撃イベントカウントスタートした
}



command マップ設定 ＤＧ連絡橋1F -set {
	if($w:p_story < d:ST:シェル１シェル２連絡橋赤外線センサークリア) {
		chara 透明壁 自殺ダイブ防止壁さま \
		-position -1000,-1000,-141500 4000 \
			      1000,-1000,-141500 4000 \
		-attribute (d:HZX_SEG_NO_ENEMY | d:HZX_SEG_NO_BULLET | d:HZX_SEG_NO_MISSILE | \
					d:HZX_SEG_NO_C4 | d:HZX_SEG_NO_RECOIL | d:HZX_SEG_NO_HARITSUKI | \
					d:HZX_SEG_NO_DISP_RADAR | d:HZX_SEG_NO_BULLETHOLE | d:HZX_SEG_NO_SPRAY | \
					d:HZX_SEG_NO_ENEMY_EYES)
		mesg 透明壁 自殺ダイブ防止壁さま off

	trap r_sensor_call d:PLAYER \
		-mask 入る \
		-exec {
			print 'r_sensor_call'
			if( ($w:p_story < d:ST:シェル１シェル２連絡橋赤外線センサークリア) && \
					(!$f:rfp_w25a_センサー前自殺ダイブＣＡＬＬした) && \
					(`@rp_強制ＣＡＬＬしてもいいよ`) ){
				eval($f:rfp_w25a_センサー前自殺ダイブＣＡＬＬした = 1)
				mesg 透明壁 自殺ダイブ防止壁さま on
				command 無線設定 -reset
				command パッド操作 -release
				command 無線設定 \
					-call 14085 t:RPC_w25a_自殺ダイブＣＡＬＬ d:CODEC_DEMO rp_w25a_自殺ダイブＣＡＬＬ終了
			}
		}
	}
}

proc rp_w25a_自殺ダイブＣＡＬＬ終了 {
	command パッド操作 -cancel
	command varsave $f:rfp_w25a_センサー前自殺ダイブＣＡＬＬした
	mesg 透明壁 自殺ダイブ防止壁さま kill
}


//


