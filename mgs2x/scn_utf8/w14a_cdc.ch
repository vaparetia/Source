/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ＮＰＣスネークの状態


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

//解体後戻ってきたら時間たったとみなしごはっち。
if( ($f:w14a_爆弾処理完了) && (!$f:rfp_w14a_爆弾処理完了後時間たった) ){
	eval($f:rfp_w14a_爆弾処理完了後時間たった = 1)
}



trap r_near_bomb d:PLAYER \
	-mask ? \
	-exec {
		print 'r_near_bomb'
		if(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
				$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) {
			command 無線設定 -over $3 14025 t:RPP_w14a_変電盤爆弾近く
		}
	}



//眠りプリスキン
trap r_snake d:PLAYER \
	-mask ? \
	-exec {
		print 'r_snake'
		if($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始){
			command 無線設定 -over $3 14085 t:RPC_眠りプリスキン
		}
	}

trap r_navy_colonel d:PLAYER \
	-mask ? \
	-exec {
		print 'r_navy_colonel'
		if($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始){
			command 無線設定 -over $3 14085 t:RPC_海軍大佐死体
		}
	}

proc rp_w14a_眠りプリスキン監視 {
//	command ＮＰＣスネークの状態
//	print 'ＮＰＣスネークの状態:' $status

	if( (`%ＮＰＣスネークの状態` == 5) && (!$f:rfp_眠りプリスキン撃ちやがった) ){
		eval($f:rfp_眠りプリスキン撃ちやがった = 1)
		print 'うちやがったー！'
	}
}


if( d:ST:P010_10_P05ヴァンプ遭遇１０ポリゴンデモ５終了 <= $w:p_story && \
		$w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始 ) {
	eval($f:rfp_眠りプリスキン撃ちやがった = 0)
	chara プロック連続実行  w14a_眠りプリスキン監視マン \
		-proc rp_w14a_眠りプリスキン監視 \
		-time -1
}





trap r_cry_byond d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_鳴り床エルード
	}

trap r_cry_floor d:PLAYER \
	-mask ? \
	-exec {
		print 'r_cry_floor'
		command 無線設定 -over $3 14085 t:RPC_鳴り床
	}





//左ロッカー
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:01
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk001 ひだりゃ'
				command 無線設定 -over $3 14085 t:RPC_ロッカー共通１
			}
	}

//まなかロッカー あかんちん
	trap lk002 d:PLAYER \
	-mask * \
	-exec {
//		@rp_ロッカー無線セット $3 ロッカー:02 t:RPC_開かないロッカー共通１
//		@rp_開かないロッカーフラグセット $3 $f:rfp_w14a_lk002_開かないロッカー開けた
		@rp_ロッカー無線セット $3 ロッカー:02 t:RPC_ロッカー共通２
	}
/*
	trap lk002 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    2048,512           0,512 \
		-exec {
			eval($f:rfp_w14a_lk002_開かないロッカー開けた =  1)
		}
*/

//みぎょロッカー
	trap lk003 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:03 t:RPC_開かないロッカー共通１
		@rp_開かないロッカーフラグセット $3 $f:rfp_w14a_lk003_開かないロッカー開けた
	}
	trap lk003 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON,   -1 \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    2048,512           0,512 \
		-exec {
			eval($f:rfp_w14a_lk003_開かないロッカー開けた =  1)
		}


/*
	trap lk003 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:03
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk003 みぎゅ'
				command 無線設定 -over $3 14085 t:RPC_ロッカー共通１
			}
	}
*/

//変電盤上
	trap lk004 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:04
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk004 偏澱蛮うえ'
				command 無線設定 -over $3 14085 t:RPC_変電盤共通１
			}
	}

//変電盤下
	trap lk005 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:05
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk005 偏澱蛮下'
				command 無線設定 -over $3 14085 t:RPC_変電盤共通２
			}
	}

//変電室変電盤北
	trap lk006 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:06
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk006 変電室 偏澱蛮北'
				command 無線設定 -over $3 14085 t:RPC_変電盤共通１
			}
	}

//変電室変電盤南
	trap lk007 d:PLAYER \
	-mask * \
	-exec {
			if($3 == いる){
				command ロッカー状態 -n ロッカー:07
				eval($i:rfp_ロッカー状態返り値 = $status)
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				print 'lk006 変電室 偏澱蛮南'
				command 無線設定 -over $3 14085 t:RPC_変電盤共通２
			}
	}

