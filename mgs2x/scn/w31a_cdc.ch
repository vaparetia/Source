/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ゲットエマ手繋ぎ状況
extern command ゲットエマ状態
extern command ゲットエマライフ
extern command ゲットエマ座標
extern command VecLen

chara 携帯端末 くそメール -skin htl -file t:mobile

proc rp_PHONE_リモコン近道終了 {
	eval($f:rfp_PHONE_リモコン近道聞いた = 1)
	command varsave $f:rfp_PHONE_リモコン近道聞いた
}

proc rp_w31a_リモコンミサイルはまりのぞき {
	if( ($w:rfp_w31a_ニキータ今の数 - $w:武器弾数Ｒ[d:武器:ニキータ]) == 1){
		eval($w:rfp_w31a_ニキータ今の数 = $w:武器弾数Ｒ[d:武器:ニキータ] )
		eval($w:rfp_w31a_ニキータ使った数 = $w:rfp_w31a_ニキータ使った数 + 1)

	}else if($w:武器弾数Ｒ[d:武器:ニキータ] > $w:rfp_w31a_ニキータ今の数 ){
		eval($w:rfp_w31a_ニキータ今の数 = $w:武器弾数Ｒ[d:武器:ニキータ] )
	}
//print '$w:rfp_w31a_ニキータ今の数:'$w:rfp_w31a_ニキータ今の数
//print '$w:rfp_w31a_ニキータ使った数:' $w:rfp_w31a_ニキータ使った数
	if( 	($w:rfp_w31a_ニキータ使った数 >= 5) && (!$f:rfp_PHONE_リモコン近道聞いた) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				(!(`%プレイヤー状態取得 -d` & d:PFLAG_NIKITA_ALIVE)) && \
				(!$f:rfp_PHONE_リモコン近道ＣＡＬＬした) && \
				($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始 ) && \
				($w:ゲーム設定 <= d:LEVEL_HARD) ){
			eval($f:rfp_PHONE_リモコン近道ＣＡＬＬした = 1)
			chara delay w31b_りもこんでぃれい -t 60 -exec {
				if( (`@rp_強制ＣＡＬＬしてもいいよ`) && ($s:登場ポイント != 電源パネルポリゴンデモへ) ) {
					command 携帯呼出 -call t:PHONE_リモコン近道 -p rp_PHONE_リモコン近道終了
				}
			}
	}
}

/*
	if( ($f:rfp_PHONE_リモコン近道ＣＡＬＬした) && ($w:rfp_w31a_ニキータ使った数 == 10) ){
		eval($f:rfp_PHONE_リモコン近道ＣＡＬＬした = 0)
		eval($w:rfp_w31a_ニキータ使った数 = 5)
	}
*/



if( ($w:p_story < d:ST:P048_01_P01電源パネル破壊１ポリゴンデモ１開始) && \
		(!$f:rfp_PHONE_リモコン近道聞いた) ){

			eval($w:rfp_w31a_ニキータ今の数 = $w:武器弾数Ｒ[d:武器:ニキータ] )
			chara プロック連続実行  w31a_リモコンミサイルはまりのぞきさん \
			-proc rp_w31a_リモコンミサイルはまりのぞき \
			-time -1

}


//if( ($w:p_story <= d:ST:P049_01_P01大統領１ポリゴンデモ１開始 ) || \
//			(d:ST:P049_12_R05大統領１２無線機デモ５終了 <= $w:p_story ) ){
	@rp_w31a_デフォルト無線セット
//}

proc rp_w31a_デフォルト無線セット {
	@rp_大佐デフォルト無線セット
	@rp_プリスキン＿デフォルト無線セット
	@rp_ローズデフォルト無線セット
	@rp_エマ＿デフォルト無線セット
}

trap r_prez_room d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			command 無線設定 -allclear
		}
		if($3 == 出る){
			@rp_w31a_デフォルト無線セット
		}
	}

//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)


trap r_elc_floor d:PLAYER \
	-mask ? \
	-exec {
			print 'r_elc_floor'
			command 無線設定 -over $3 14085 t:RPC_シェル２中央棟１Ｆ＿電撃床
	}

trap r_dark_room d:PLAYER \
	-mask ? \
	-exec {
			print 'r_dark_room'
			if($3 == 入る){
				eval($f:rfp_31a_暗い部屋にいまそかり = 1)
			}else if($3 == 出る){
				eval($f:rfp_31a_暗い部屋にいまそかり = 0)
			}
			command 無線設定 -over $3 14085 t:RPC_シェル２中央棟１Ｆ＿暗い
	}


trap r_duct d:PLAYER \
	-mask ? \
	-exec {
			print 'r_duct'
			command 無線設定 -over $3 14085 t:RPC_シェル２中央棟１Ｆ＿ダクト前
	}



/*
proc	rp_P051_01_R01エマとオタコン１無線機デモ１ {
	eval($f:rfp_w31a_P051_01_R01エマとオタコン１無線機デモ１した = 1)
	command パッド操作 -release
	command 無線設定 \
	-call 14112 \
		t:RPD_P051_01_R01_エマとオタコン１無線機デモ１ \
		d:CODEC_DIRECT rp_P051_01_R01エマとオタコン１無線機デモ１終了 \
	-f ( d:CODEC_I_AC_O_FD | d:CODEC_FDOUT_BREAK )
}


proc rp_P051_01_R01エマとオタコン１無線機デモ１終了 {
	eval( $i:プレイヤー初期Ｘ位置 = $i:プレイヤー位置Ｘ ) ;
	eval( $i:プレイヤー初期Ｙ位置 = $i:プレイヤー位置Ｙ ) ;
	eval( $i:プレイヤー初期Ｚ位置 = $i:プレイヤー位置Ｚ ) ;
	eval( $i:プレイヤー初期方向 = $w:プレイヤー方向 )
	eval( $s:登場ポイント = エマとオタコン１無線機デモ１＿２ );
	@mv_init_wmovie_0
}

proc	rp_P051_01_R01エマとオタコン１無線機デモ１＿２ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P051_01_R01_エマとオタコン１無線機デモ１＿２ \
		d:CODEC_DIRECT rp_P051_01_R01エマとオタコン１無線機デモ１＿２終了 \
	-f ( d:CODEC_I_FD_O_FA | d:CODEC_FDIN_BREAK | d:CODEC_FDOUT_BREAK )
}

proc rp_P051_01_R01エマとオタコン１無線機デモ１＿２終了 {
	command パッド操作 -cancel
	eval( $s:登場ポイント = エマとオタコン１無線機デモ１＿２終了);
}

	trap r_emma_call d:PLAYER \
		-mask 入る \
		-exec {
			if ( (d:ST:P050_06_R03タンカー編整理６無線機デモ３終了 <= $w:p_story && \
							$w:p_story < d:ST:P055_01_P01ヴァンプ戦前１ポリゴンデモ１開始)  && \
						(!$f:rfp_w31a_P051_01_R01エマとオタコン１無線機デモ１した) && \
					 (`command ゲームオーバーチェック` == 0 ) ) {
					@rp_P051_01_R01エマとオタコン１無線機デモ１
			}else if($s:登場ポイント == エマとオタコン１無線機デモ１＿２){
				@rp_P051_01_R01エマとオタコン１無線機デモ１＿２
			}
		}
	
*/



