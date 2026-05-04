/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ゲットゲームステータス

@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

//eval($b:Ｃ４爆弾処理数 = 4)

proc rp_w12b_敵兵倒したＣＡＬＬ {
	command varsave $f:rfp_w12a_敵兵倒したＣＡＬＬした
}

proc rp_w12b_いろいろ監視 {
	command ゲット敵兵状態 敵兵:01 $i:rfp_敵兵状態１
	command ゲット敵兵状態拡張版 敵兵:01 $i:rfp_敵兵状態２
	command ゲットゲームステータス $i:game_staus
	
	//敵兵倒しました
	if((	($i:rfp_敵兵状態１ & 0x00004000) || \ //気絶
				($i:rfp_敵兵状態１ & 0x80000000) || \	//死亡
				($i:rfp_敵兵状態２ & 0x00000002) ) && \	//寝てます
			(`@rp_通常ＣＡＬＬしてもいいよ`) && \
			($w:アラートモード == d:ALERT_MODE_SNEAK) && \
			(!$f:rfp_w12a_敵兵倒したＣＡＬＬした) ) {
		eval($f:rfp_w12a_敵兵倒したＣＡＬＬした = 1)
			command 無線設定 -call 14085 t:RPC_w12b_敵兵倒したＣＡＬＬ \
				d:CODEC_CALL rp_w12b_敵兵倒したＣＡＬＬ
	}
	//くりゃーりんぐ
	if( ($i:game_staus & d:STATE_CLEARING) && \
			($f:クリアリングフラグ０) && \
			( ($w:タンカー編クリア回数 < 1) && ($w:プラント編クリア回数 < 1) ) && \
			(!$f:rfp_w11a_クリアリングＣＡＬＬ聞いた) && \
			(!$f:rfp_w12b_クリアリングＣＡＬＬ聞いた) && \
			(!$f:rfp_w12b_クリアリングＣＡＬＬした) ){
				eval($f:rfp_w12b_クリアリングＣＡＬＬした = 1)
				chara delay クリアリングＣＡＬＬディレイさん \
					-time 120 \
					-exec {
						command ゲットゲームステータス $i:game_staus
						if( ($i:game_staus & d:STATE_CLEARING) && \
								($f:クリアリングフラグ０) && \
								(`@rp_強制ＣＡＬＬしてもいいよ`) ){
									eval($f:rfp_w12b_クリアリングＣＡＬＬ中  = 1)
									command 無線設定 -call 14085 t:RPC_w12b_クリアリングＣＡＬＬ \
										d:CODEC_CALL rp_w12b_クリアリングＣＡＬＬ終了
						}else {
							eval($f:rfp_w12b_クリアリングＣＡＬＬした = 0)
						}
					}
	}
	if($f:rfp_w12b_クリアリングＣＡＬＬ中){
		if( (!($i:game_staus & d:STATE_CLEARING)) || \
				(!$f:クリアリングフラグ０) || \
				(`%無線状態` == 0) ){
				command 無線設定 -reset
				eval($f:rfp_w12b_クリアリングＣＡＬＬ中 = 0)
			}
	}

}

proc rp_w12b_クリアリングＣＡＬＬ終了 {
	eval($f:rfp_w12b_クリアリングＣＡＬＬ聞いた = 1)
	eval($f:rfp_w12b_クリアリングＣＡＬＬ中 = 0)
	command varsave $f:rfp_w12b_クリアリングＣＡＬＬ聞いた
}


proc rp_w12b_いろいろＣＡＬＬに監視すれば？{
	if(d:ST:P007_01_R01Ａ脚ノード前１無線デモ１終了 <= $w:p_story && \
				$w:p_story < d:ST:Ａ脚出たところでＳＥＡＬＳ無線傍受終了){
		chara プロック連続実行  w12b_いろいろ監視さん \
			-proc rp_w12b_いろいろ監視 \
			-time -1
	}
}
@rp_w12b_いろいろＣＡＬＬに監視すれば？

//解体後戻ってきたら時間たったとみなしごはっち。
if( ($f:w12b_爆弾処理完了) && (!$f:rfp_w12b_爆弾処理完了後時間たった) ){
	eval($f:rfp_w12b_爆弾処理完了後時間たった = 1)
}


if($w:ゲーム設定 >= d:LEVEL_EASY) {		// レベルイージー以上にて出現
	trap r_s_camera d:PLAYER \
		-mask * \
		-exec {
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'r_s_camera'
				command 無線設定 -over $3 14085 t:RPC_監視カメラ近く
			}
			if($3 == いる ){
				@カメラ再セットチェック $b:w12b_破壊ロード回数[4]
				eval($f:rfp_そこの監視カメラ生きてます = $f:再セットフラグ)
			}
		}
}


//ロッカー対四十雀
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:01
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk001 ひだりヒダーリ'
				command 無線設定 -over $3 14085 t:RPC_ロッカー共通１
			}
	}
	//となりあわせな連中は微妙に互い違い
	trap lk002 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:02
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk001 ひだりミーギ'
				command 無線設定 -over $3 14085 t:RPC_ロッカー共通２
			}
	}

	trap lk003 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:03
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk003 みぎヒダーリ'
				command 無線設定 -over $3 14085 t:RPC_ロッカー共通１
			}
	}

	trap lk004 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:04
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk004 みぎみぎ'
				command 無線設定 -over $3 14085 t:RPC_ロッカー共通２
			}
	}

trap r_node d:PLAYER \
	-mask ? \
	-exec {
			print 'r_node'
			command 無線設定 -over $3 14085 t:RPC_ノード共通
	}


trap dr005 d:PLAYER \
	-mask ? \
	-exec {
			print 'dr005'
			if($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始){
				command 無線設定 -over $3 14085 t:RPC_カードドア
			}
	}


trap r_hofuku_call d:PLAYER \
	-mask * \
	-exec {
		if($3 == いる) {
			eval($i:rfp_w12b_ホフクパイプ近くカウンタ = $i:rfp_w12b_ホフクパイプ近くカウンタ + 1)
//			print '$i:rfp_w12b_ホフクパイプ近くカウンタ:' $i:rfp_w12b_ホフクパイプ近くカウンタ
			if( ($i:rfp_w12b_ホフクパイプ近くカウンタ >= 400 ) && \
					(!$f:rfp_w12b_ホフクパイプ近くＣＡＬＬ聞いた) && \
					(!$f:w12b_爆弾処理完了) && ($w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) && \
					(!$f:rfp_w12b_ホフクパイプ近くＣＡＬＬ中) && \
					(`@rp_通常ＣＡＬＬしてもいいよ`) && \
					(!$f:rfp_w12b_ホフクパイプ近くパイプ下にいます) ){

				command 無線設定 -call 14025 t:RPP_爆弾ヒント＿Ａ脚ホフクＣＡＬＬ d:CODEC_CALL 
				eval($f:rfp_w12b_ホフクパイプ近くＣＡＬＬ中 = 1)
			}
		}
		if($3 == 出る){
			eval($i:rfp_w12b_ホフクパイプ近くカウンタ = 0)
			if($f:rfp_w12b_ホフクパイプ近くＣＡＬＬ中){
				eval($f:rfp_w12b_ホフクパイプ近くＣＡＬＬ中 = 0)
				command 無線設定 -reset
			}
		}
	}
trap r_hofuku_call_cancel d:PLAYER \
	-mask 入る \
	-exec {
		eval($f:rfp_w12b_ホフクパイプ近くＣＡＬＬ聞いた = 1)
	}

trap r_hofuku_call_pipe d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る) {
			eval($f:rfp_w12b_ホフクパイプ近くパイプ下にいます = 1)
		}
		if($3 == 出る) {
			eval($f:rfp_w12b_ホフクパイプ近くパイプ下にいます = 0)
		}
	}


