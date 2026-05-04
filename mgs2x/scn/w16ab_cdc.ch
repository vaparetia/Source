/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

//eval($b:Ｃ４爆弾設置数 = 1)
//eval($f:w20b_オルガデモフラグ = 1)


//解体後戻ってきたら時間たったとみなしごはっち。
if( ($f:w16b_爆弾処理完了) && (!$f:rfp_w16b_爆弾処理完了後時間たった) ){
	eval($f:rfp_w16b_爆弾処理完了後時間たった = 1)
}


trap r_card_door d:PLAYER \
	-mask ? \
	-exec {
		print 'r_card_door'
		if($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) {
			command 無線設定 -over $3 14085 t:RPC_カードドア
		}
	}


trap r_cbox d:PLAYER \
	-mask ? \
	-exec {
		print 'r_cbox'
			command 無線設定 -over $3 14085 t:RPC_ダンボール地帯
	}

trap r_peter d:PLAYER \
	-mask ? \
	-exec {
		print 'r_peter:' $b:rfp_ピーターちょっかい回数
			if(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
					$w:p_story < d:ST:P022_01_R01爆弾解体最後から二つ目１無線デモ１開始) {
				command 無線設定 -over $3 14085 t:RPC_天岩戸
				command 無線設定 -over $3 14025 t:RPP_天岩戸
			}
	}

trap r_near_bomb d:PLAYER \
	-mask ? \
	-exec {
		print 'r_near_bomb'
		if(d:ST:P014_15_P07ピーター遭遇１５ポリゴンデモ７終了 <= $w:p_story && \
				$w:p_story < d:ST:P023_01_R01爆弾解体最後の一つ１無線デモ１開始) {
			command 無線設定 -over $3 14025 t:RPP_w16b_女子トイレ爆弾近く
		}
	}


eval($b:rfp_ピーターちょっかい回数 = 0)
proc ピーターちょっかい回数インクリメント {
	eval($b:rfp_ピーターちょっかい回数 = $b:rfp_ピーターちょっかい回数 + 1)
}

	// ドアをノック
	trap ev006 d:PLAYER \
		-mask いる \
		-button d:PUNCH_BUTTON \
		-state  d:TRP_STATE_CAUTION \
		-dir    0,512 \
		-exec {
			@ピーターちょっかい回数インクリメント
	}

	trap ev006 d:PLAYER \
		-mask いる \
		-and \
		-button d:ACTION_BUTTON, d:ACTION_BUTTON \
		-state  d:TRP_STATE_STAND, d:TRP_STATE_CAUTION \
		-dir    2048,512           0,512 \
		-exec {
			@ピーターちょっかい回数インクリメント
	}



trap r_koware_mono d:PLAYER \
	-mask ? \
	-exec {
		print 'r_koware_mono'
			command 無線設定 -over $3 14085 t:RPC_コワレモノ近辺
	}


	//衝撃の男子便所
	proc rp_w16_便器チェック $:便器Ｘ $:便器Ｙ $:便器Ｚ {
		if( (`%カメラ視界チェック -camera 0x03 -pos $:便器Ｘ,$:便器Ｙ,$:便器Ｚ`) && \
				(`%プレイヤー状態取得` & d:PFLAG_SUBJECT ) ){
			eval($f:rfp_w16_便器見てる = 1)
	//		print 'うわーちょーみてるよー'
		}else {
			eval($f:rfp_w16_便器見てる = 0)
		}
	/*
		if(`%プレイヤー状態取得` & d:PFLAG_SQUAT) {
			eval($f:rfp_w16_便器しゃがんでる = 1)
		}else {
			eval($f:rfp_w16_便器しゃがんでる = 0)
		}
		if( ($f:rfp_w16_便器見てる) && ($f:rfp_w16_便器しゃがんでる) ){
			print 'うげげ便器しゃがんで見てるよー'
		}
	*/
	}

	trap r_M_small01 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -62750 -0 -87000
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
					print 'r_M_small01'
					command 無線設定 -over $3 14085 t:RPC_男子便所便器
				}
			}
		}

	trap r_M_small02 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -61625 -0 -87000
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
					print 'r_M_small02'
					command 無線設定 -over $3 14085 t:RPC_男子便所便器
				}
			}
		}

	trap r_M_big03 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -60500 -0 -86125
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
							print 'r_M_big03'
							command 無線設定 -over $3 14085 t:RPC_男子便所便器
				}
			}
		}
		
	trap r_M_big04 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -59375 -0 -86125
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
							print 'r_M_big04'
							command 無線設定 -over $3 14085 t:RPC_男子便所便器
				}
			}
		}



	//ウワサの女子便所
	trap r_joshiben d:PLAYER \
		-mask ? \
		-exec {
			if($w:p_story < d:ST:P026_01_R01爆弾解体終了１無線デモ１開始){
				@rp_プリスキン＿無線セット 0 $3 t:RPS_女子便所
			}
			if($3 == 入る){
				print 'r_joshiben in'
				eval($i:rfp_w16_女子トイレ開始時刻 = $i:プレイタイム)
				command varsave $i:rfp_w16_女子トイレ開始時刻

				eval($f:rfp_女子トイレにいます = 1)
				if(!$f:rfp_女子トイレに入った){
					eval($f:rfp_女子トイレに入った = 1)
				}
			}
			if($3 == 出る) {
				print 'r_joshiben out'
				eval($f:rfp_女子トイレにいます = 0)
			}
		}

	trap r_F_big01 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -47125 -0 -86125
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
							print 'r_F_big01'
							command 無線設定 -over $3 14085 t:RPC_女子便所便器
				}
			}
		}

	trap r_F_big02 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -45875 -0 -86125
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
							print 'r_F_big02'
							command 無線設定 -over $3 14085 t:RPC_女子便所便器
				}
			}
		}

	trap r_F_big03 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -44875 -0 -86125
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
						print 'r_F_big03'
						command 無線設定 -over $3 14085 t:RPC_女子便所便器
				}
			}
		}

	trap r_F_big04 d:PLAYER \
		-mask * \
		-exec {
			if($3 == いる) {
				@rp_w16_便器チェック -43750 -0 -86125
			}
			if( ($3 == 入る) || ($3 == 出る) ){
				if( ($w:p_story < d:ST:P014_01_P01ピーター遭遇１ポリゴンデモ１開始) || \
						($f:w16b_爆弾処理完了) ){
							print 'r_F_big04'
							command 無線設定 -over $3 14085 t:RPC_女子便所便器
				}
			}
		}



//eval($w:武器弾数Ｒ[4] = -1)
