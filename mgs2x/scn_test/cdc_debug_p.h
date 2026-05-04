//
//	cdc_debug_p.h
//	プラント用っち
//	無線モーションチェック用おもしろデバッグファイル select.gclから呼び出されちゃう。
//
//	2000/10/24	T.Fukushima
//	$Id: cdc_debug_p.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $
extern command セットサウンドコード

#include "cdc_debug_peter.h"
#include "cdc_debug_p_demo_vc.h"
#include "cdc_debug_emma.h"
#include "cdc_debug_colonel.h"
#include "cdc_debug_potacon.h"
#include "cdc_debug_rose.h"
#include "cdc_debug_snake.h"
#include "cdc_debug_colonel_add.h"	//大佐追加収録
#include "cdc_debug_colonel_diver.h"	//潜水ライデンしゅこぱー

if ($b:res_mode == d:RES_RAIDEN) {
	chara 2Dレイアウトドライバ 2Dレイアウトドライバちゃん
	chara 無線システム Codec -player rai_def
}

/*
//インチキサウンド設定
chara サウンドマネージャー ＳＤマネ \
	-pak 0	// パック番号
mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800
chara ＢＧＭマネージャー ＢＧＭマネ
*/




proc 終わり_p {
//	command セットサウンドコード -c d:SNG_FOUTP_SS
	restart
}

proc enter_codec_debug_mode_p {
	eval($f:rfp_enter_codec_debug_mode_p = 1)
	restart
}

proc clear_flag_p {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_face_camera_check_p = 0)
	eval($f:rfp_enter_p_P001_02_R01nt_P006_01_R01 = 0)
	eval($f:rfp_enter_p_P007_01_R01_P016_01_R01 = 0)
	eval($f:rfp_enter_p_P017_01_R01_P026_01_R01 = 0)
	eval($f:rfp_enter_p_P027_01_R01_P040_02_R02 = 0)
	eval($f:rfp_enter_p_P040_06a_R02a_P049_02_R01 = 0)
	eval($f:rfp_enter_p_P049_05_R02_P052_01_R01 = 0)
	eval($f:rfp_enter_p_P053_01_R01_P062_05_R02 = 0)
	eval($f:rfp_enter_p_P064_01_R01_P070_23_R04 = 0)
	eval($f:rfp_enter_p_P071_01_R01_P080_21_R02 = 0)
	eval($f:rfp_enter_p_P080_23_R03_P080_25_R04 = 0)
}


proc exit_codec_debug_mode_p {
	@clear_flag_p
	eval($f:rfp_enter_codec_debug_mode_p = 0)
	restart
}

proc back_to_root_p_demo {
	@clear_flag_p
	restart
}

proc back_to_root_p {
	eval($f:rfp_enter_demo_p = 0)
	@clear_flag_p
	restart
}


/*---------------------------------------------------------
このへんに呼び出しプロックを書いちゃうぞ
---------------------------------------------------------*/
proc rp_d_P001_02_R01オープニング潜入２無線デモ１nt {
	eval($w:タンカー編クリア回数 = 0)	//ＸＸＸＸＸ
	command 無線設定 \
	-call 14085 \
		t:RPD_P001_02_R01_オープニング潜入２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P001_02_R01オープニング潜入２無線デモ１et {
	eval($w:タンカー編クリア回数 = 1)	//ＸＸＸＸＸ
	command 無線設定 \
	-call 14085 \
		t:RPD_P001_02_R01_オープニング潜入２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc rp_d_P001_04_R02オープニング潜入４無線デモ２ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P001_04_R02_オープニング潜入４無線デモ２ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P002_02_R01Ａ脚底部連結ハッチ２無線デモ１ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P002_02_R01_Ａ脚底部連結ハッチ２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P003_02_R02スネーク昇降機上昇２無線デモ１nt {
	eval($w:タンカー編クリア回数 = 0)	//ＸＸＸＸＸＸＸＸＸ
	command 無線設定 \
	-call 14085 \
		t:RPD_P003_02_R02_スネーク昇降機上昇２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
proc	rp_d_P003_02_R02スネーク昇降機上昇２無線デモ１et {
	eval($w:タンカー編クリア回数 = 1)	//ＸＸＸＸＸＸＸＸＸ
	command 無線設定 \
	-call 14085 \
		t:RPD_P003_02_R02_スネーク昇降機上昇２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P004_02_R01ノード初接続２無線デモ１ {
	eval($w:タンカー編クリア回数 = 1)	//ＸＸＸＸＸＸＸＸＸ
	command 無線設定 \
	-call 14085 \
		t:RPD_P004_02_R01_ノード初接続２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P005_03_R01ライデン昇降機上昇３無線デモ１ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P005_03_R01ライデン昇降機上昇３無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P005_05_R02ライデン昇降機上昇５無線デモ２ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P005_05_R02_ライデン昇降機上昇５無線デモ２ \
		d:CODEC_DIRECT 終わり_p 
}

// ------------------------------------------------
proc	rp_d_P006_01_R01Ａ脚金網１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P006_01_R01_Ａ脚金網１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P007_01_R01Ａ脚ノード前１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P007_01_R01_Ａ脚ノード前１無線デモ１＿１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P008_01_R01ＡＢ連絡橋１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P008_01_R01_ＡＢ連絡橋１無線デモ１＿１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P009_01_R01_サイファー遭遇１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P009_01_R01_サイファー遭遇１無線デモ１＿１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P010_04_R01ヴァンプ遭遇４無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P010_04_R01_ヴァンプ遭遇４無線デモ１＿１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P011_01_R01ＢＣ連絡橋ＣＡＬＬ１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P011_01_R01_ＢＣ連絡橋ＣＡＬＬ１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P012_02_R01_フォーチュン遭遇２無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P012_02_R01_フォーチュン遭遇２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P013_01_R01_ライデン危機無線１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P013_01_R01_ライデン危機無線１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P014_05_R01ピーター遭遇５無線デモ１ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P014_05_R01_ピーター遭遇５無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P015_01_R1爆弾解体最初の一つ１無線デモ１ {
	command 無線設定 \
	-call 14085 \
		t:RPD_P015_01_R1_爆弾解体最初の一つ１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P016_01_R1爆弾解体半分解体１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P016_01_R1_爆弾解体半分解体１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P017_01_R01爆弾解体中ローズその１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P017_01_R01_爆弾解体中ローズその１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P018_01_R01爆弾解体中ローズその２無線デモ１ {
	command 無線設定 \
	-call 14112 \ 
		t:RPD_P018_01_R01_爆弾解体中ローズその２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P019_01_R01爆弾解体中ローズその３無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P019_01_R01_爆弾解体中ローズその３無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P020_01_R01_地雷原１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P020_01_R01_地雷原１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P020_01_R01_地雷原１無線デモ１＿２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P020_01_R01_地雷原１無線デモ１＿２ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P021_02_R01_爆弾解体オルガ登場２無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P021_02_R01_爆弾解体オルガ登場２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P022_01_R01爆弾解体最後から二つ目１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P022_01_R01_爆弾解体最後から二つ目１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P023_01_R01爆弾解体最後の一つ１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P023_01_R01_爆弾解体最後の一つ１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P023_01_R01爆弾解体最後の一つ１無線デモ１＿センサーＢなし {
	eval($w:アイテム数Ｒ[d:アイテム:センサーＢ] = -1)
	command 無線設定 \
	-call 14112 \
		t:RPD_P023_01_R01_爆弾解体最後の一つ１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P024_01_R01爆弾解体センサーＢ入手１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P024_01_R01_爆弾解体センサーＢ入手１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P025_01_R01爆弾解体昇降機下１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P025_01_R01_爆弾解体昇降機下１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P026_01_R01爆弾解体終了１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P026_01_R01_爆弾解体終了１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P027_01_R01爆弾解体後昇降機ホール１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P027_01_R01_爆弾解体後昇降機ホール１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P030_01_R01フォーチュン戦中無線無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P030_01_R01_フォーチュン戦中無線無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P035_01_R01ファットマン爆弾解体終了１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P035_01_R01_ファットマン爆弾解体終了１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P035_01_R01ファットマン爆弾解体終了１無線デモ１＿２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P035_01_R01_ファットマン爆弾解体終了１無線デモ１＿２ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P036_03_R01忍者登場３無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P036_03_R01_忍者登場３無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P036_08_R02忍者登場８無線デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P036_08_R02_忍者登場８無線デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P036_10_R03忍者登場１０無線デモ３ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P036_10_R03_忍者登場１０無線デモ３ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P036_13_R04忍者登場１３無線デモ４ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P036_13_R04_忍者登場１３無線デモ４ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P037_01_R01スネークの遺体１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P037_01_R01_スネークの遺体１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P040_02_R02エイムズ発見２無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P040_02_R02_エイムズ発見２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P040_06a_R02aエイムズ発見６a無線デモ２a {
	command 無線設定 \
	-call 14112 \
		t:RPD_P040_06_R02_エイムズ発見６無線デモ２＿１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P040_06c_R02bエイムズ発見６c無線デモ２b {
	command 無線設定 \
	-call 14112 \
		t:RPD_P040_06_R02_エイムズ発見６無線デモ２＿２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P041_02_R01エイムズ死亡後２無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P041_02_R01_エイムズ死亡後２無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P042_01_R01シェル１シェル２連絡橋１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P042_01_R01_シェル１シェル２連絡橋１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P044_01_R01赤外線センサークリア１無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P044_01_R01_赤外線センサークリア１無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P044_01_R01赤外線センサークリア１無線デモ１＿２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P044_01_R01_赤外線センサークリア１無線デモ１＿２ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P046_02_R01ハリアー戦勝利２無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P046_02_R01_ハリアー戦勝利２無線デモ１＿１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P046_02_R01ハリアー戦勝利２無線デモ１＿２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P046_02_R01_ハリアー戦勝利２無線デモ１＿２ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P047_05_R01電撃床前オルガ５無線デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P047_05_R01_電撃床前オルガ５無線デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P049_02_R01大統領２無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P049_02_R01_大統領２無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}


// ------------------------------------------------
proc	rp_d_P049_05_R02大統領５無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P049_02_R01_大統領５無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P049_08_R03大統領８無線機デモ３ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P049_08_R03_大統領８無線機デモ３ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P049_10_R04大統領１０無線機デモ４ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P049_10_R04_大統領１０無線機デモ４ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P049_12_R05大統領１２無線機デモ５ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P049_12_R05_大統領１２無線機デモ５ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P050_01_R01タンカー編整理１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P050_01_R01_タンカー編整理１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P050_04_R02タンカー編整理４無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P050_04_R02_タンカー編整理４無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P050_06_R03タンカー編整理６無線機デモ３ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P050_06_R03_タンカー編整理６無線機デモ３ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P051_01_R01エマとオタコン１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P051_01_R01_エマとオタコン１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}

/*
proc	rp_d_P051_01_R01エマとオタコン１無線機デモ１＿２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P051_01_R01_エマとオタコン１無線機デモ１＿２ \
		d:CODEC_DIRECT 終わり_p
}
*/

// ------------------------------------------------
proc	rp_d_P052_01_R01ローズ無線１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P052_01_R01_ローズ無線１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P053_01_R01息継ぎ１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P053_01_R01_息継ぎ１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P057_02_R01ヴァンプ戦終了２無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P057_02_R01_ヴァンプ戦終了２無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}

proc	rp_d_P057_02_R01ヴァンプ戦終了２無線機デモ１＿２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P057_02_R01_ヴァンプ戦終了２無線機デモ１＿２ \
		d:CODEC_DIRECT 終わり_p
}

// ------------------------------------------------
proc	rp_d_P058_02_R01エマ救出２無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P058_02_R01_エマ救出２無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P058_04_R02エマ救出４無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P058_04_R02_エマ救出４無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P058_07_R03エマ救出７無線機デモ３ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P058_07_R03_エマ救出７無線機デモ３ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P059_03_R01エマ休憩３無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P059_03_R01_エマ休憩３無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P061_01_R01エマとオタコン１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P061_01_R01_エマとオタコン１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P062_03_R01エマとＡＩ３無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P062_03_R01_エマとＡＩ３無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P062_05_R02エマとＡＩ５無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P062_05_R02_エマとＡＩ５無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P064_01_R01エマライデンローズ無線１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P064_01_R01_エマライデンローズ無線１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P065_02_R01エマＬ脚２無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P065_02_R01_エマＬ脚２無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P065_06_R02Ｌ脚エマ６無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P065_06_R02_Ｌ脚エマ６無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P067_01_R01エマ狙撃１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P067_01_R01_エマ狙撃１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P069_02_R01ヴァンプ狙撃終了２無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P069_02_R01_ヴァンプ狙撃終了２無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P069_04_R02ヴァンプ狙撃終了４無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P069_04_R02_ヴァンプ狙撃終了４無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P070_10_R01ＡＧ起動１０無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P070_10_R01_ＡＧ起動１０無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P070_18_R02ＡＧ起動１８無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P070_18_R02_ＡＧ起動１８無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P070_20_R03ＡＧ起動２０無線機デモ３ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P070_20_R03_ＡＧ起動２０無線機デモ３ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P070_23_R04ＡＧ起動２３無線機デモ４ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P070_23_R04_ＡＧ起動２３無線機デモ４ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P071_01_R01拷問部屋脱出１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P071_01_R01_拷問部屋脱出１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P072_01_R01大佐混乱１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P072_01_R01_大佐混乱１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P073_01_R01通路Ａ１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P073_01_R01_通路Ａ１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P073_03_R02通路Ａ３無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P073_03_R02_通路Ａ３無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P074_02_R01通路Ａ刀後２無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P074_02_R01_通路Ａ刀後２無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P075_01_R01オタコン無事１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P075_01_R01_オタコン無事１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P076_01_R01大佐ＡＩ１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P076_01_R01_大佐ＡＩ１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P077_01_R01ローズ拉致１無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P077_01_R01_ローズ拉致１無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P080_19_R01ＡＧ浮上１９無線機デモ１ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P080_19_R01_ＡＧ浮上１９無線機デモ１ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P080_21_R02ＡＧ浮上２１無線機デモ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P080_21_R02_ＡＧ浮上２１無線機デモ２ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P080_23_R03ＡＧ浮上２３無線機デモ３ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P080_23_R03_ＡＧ浮上２３無線機デモ３ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------
proc	rp_d_P080_25_R04ＡＧ浮上２５無線機デモ４ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P080_25_R04_ＡＧ浮上２５無線機デモ４ \
		d:CODEC_DIRECT 終わり_p
}
// ------------------------------------------------


/*---------------------------------------------------
顔チェック用
----------------------------------------------------*/
proc rp_d_P_ダミー＿顔カメラ＿ライデン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿ライデン \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿潜水ライデン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿潜水ライデン \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿裸ライデン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿裸ライデン \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿大佐 {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿大佐 \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿ローズ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿ローズ \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿プリスキン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿プリスキン \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿スネーク {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿スネーク \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿ひげそりスネーク {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿ひげそりスネーク \
		d:CODEC_DIRECT 終わり_p
}


proc rp_d_P_ダミー＿顔カメラ＿ピーター {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿ピーター \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿オタコン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿オタコン \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿忍者 {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿忍者 \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿オルガ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿オルガ \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿エイムズ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿エイムズ \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿大統領 {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿大統領 \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿エマ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿エマ \
		d:CODEC_DIRECT 終わり_p
}

/*
proc rp_d_P_ダミー＿顔カメラ＿エマ２ {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿エマ２ \
		d:CODEC_DIRECT 終わり_p
}
*/

proc rp_d_P_ダミー＿顔カメラ＿エマプレイヤー側 {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿エマプレイヤー側 \
		d:CODEC_DIRECT 終わり_p
}


proc rp_d_P_ダミー＿顔カメラ＿ゴルルコ兵マスクなしライデン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿ゴルルコ兵マスクなしライデン \
		d:CODEC_DIRECT 終わり_p
}

proc rp_d_P_ダミー＿顔カメラ＿ゴルルコ兵マスクありライデン {
	command 無線設定 \
	-call 14112 \
		t:RPD_P_ダミー＿顔カメラ＿ゴルルコ兵マスクありライデン \
		d:CODEC_DIRECT 終わり_p
}


/*---------------------------------------------------
セレクトキャラセットだにゅん。
----------------------------------------------------*/
proc enter_demo_p {
	eval($f:rfp_enter_demo_p = 1)
	restart
}

proc enter_face_camera_check_p {
	eval($f:rfp_enter_face_camera_check_p = 1)
	restart
}

proc enter_peter_p {
	eval($f:rfp_enter_codec_debug_mode_p_peter =1);
	restart
}

proc enter_emma_p {
	eval($f:rfp_enter_codec_debug_mode_p_emma =1);
	restart
}

proc enter_colonel_p {
	eval($f:rfp_enter_codec_debug_mode_p_colonel =1);
	restart
}

proc enter_otacon_p {
	eval($f:rfp_enter_codec_debug_mode_p_otacon =1);
	restart
}

proc enter_rose_p {
	eval($f:rfp_enter_codec_debug_mode_p_rose =1);
	restart
}

proc enter_snake_p {
	eval($f:rfp_enter_codec_debug_mode_p_snake =1);
	restart
}

proc enter_colonel_add_p {
	eval($f:rfp_enter_codec_debug_mode_p_colonel_add =1);
	restart
}

proc enter_colonel_diver_p {
	eval($f:rfp_enter_codec_debug_mode_p_colonel_diver =1);
	restart
}

proc codec_set_root_p {
	chara select root -s {
		'demo' enter_demo_p
		'demo(vc)' enter_demo_p_vc
		'peter' enter_peter_p
		'emma' enter_emma_p
		'colonel' enter_colonel_p
		'otacon' enter_otacon_p
		'rose' enter_rose_p
		'snake' enter_snake_p
		'colonel_add' enter_colonel_add_p
		'colonel_diver' enter_colonel_diver_p
		'face_camera_check' enter_face_camera_check_p
		'exit' exit_codec_debug_mode_p
	}
}


proc codec_set_demo_p {
	chara select demo -s {
		'P001_02_R01nt_P006_01_R01' enter_p_P001_02_R01nt_P006_01_R01 
		'P007_01_R01_P016_01_R01' enter_p_P007_01_R01_P016_01_R01 
		'P017_01_R01_P026_01_R01' enter_p_P017_01_R01_P026_01_R01 
		'P027_01_R01_P040_02_R02' enter_p_P027_01_R01_P040_02_R02 
		'P040_06a_R02a_P049_02_R01' enter_p_P040_06a_R02a_P049_02_R01 
		'P049_05_R02_P052_01_R01' enter_p_P049_05_R02_P052_01_R01 
		'P053_01_R01_P062_05_R02' enter_p_P053_01_R01_P062_05_R02 
		'P064_01_R01_P070_23_R04' enter_p_P064_01_R01_P070_23_R04 
		'P071_01_R01_P080_21_R02' enter_p_P071_01_R01_P080_21_R02 
		'P080_23_R03_P080_25_R04' enter_p_P080_23_R03_P080_25_R04 
		'back' back_to_root_p
		'exit' exit_codec_debug_mode_p
	}
}


proc enter_p_P001_02_R01nt_P006_01_R01 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P001_02_R01nt_P006_01_R01 = 1)
	restart
}

proc enter_p_P007_01_R01_P016_01_R01 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P007_01_R01_P016_01_R01 = 1)
	restart
}

proc enter_p_P017_01_R01_P026_01_R01 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P017_01_R01_P026_01_R01 = 1)
	restart
}

proc enter_p_P027_01_R01_P040_02_R02 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P027_01_R01_P040_02_R02 = 1)
	restart
}

proc enter_p_P040_06a_R02a_P049_02_R01 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P040_06a_R02a_P049_02_R01 = 1)
	restart
}

proc enter_p_P049_05_R02_P052_01_R01 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P049_05_R02_P052_01_R01 = 1)
	restart
}
proc enter_p_P053_01_R01_P062_05_R02 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P053_01_R01_P062_05_R02 = 1)
	restart
}
proc enter_p_P064_01_R01_P070_23_R04 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P064_01_R01_P070_23_R04 = 1)
	restart
}
proc enter_p_P071_01_R01_P080_21_R02 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P071_01_R01_P080_21_R02 = 1)
	restart
}

proc enter_p_P080_23_R03_P080_25_R04 {
//	eval($f:rfp_enter_demo_p = 0)
	eval($f:rfp_enter_p_P080_23_R03_P080_25_R04 = 1)
	restart
}


proc codec_set_face_camera_check_p {
	chara select face_camera_check_p -s {
		'raiden_normal' rp_d_P_ダミー＿顔カメラ＿ライデン
		'raiden_diver' rp_d_P_ダミー＿顔カメラ＿潜水ライデン
		'raiden_naked' rp_d_P_ダミー＿顔カメラ＿裸ライデン
		'colonel' rp_d_P_ダミー＿顔カメラ＿大佐
		'rose' rp_d_P_ダミー＿顔カメラ＿ローズ
		'Plissken' rp_d_P_ダミー＿顔カメラ＿プリスキン
		'Snake' rp_d_P_ダミー＿顔カメラ＿スネーク
		'peter' rp_d_P_ダミー＿顔カメラ＿ピーター
		'otacon' rp_d_P_ダミー＿顔カメラ＿オタコン
		'ninja' rp_d_P_ダミー＿顔カメラ＿忍者
		'ollga' rp_d_P_ダミー＿顔カメラ＿オルガ
		'Aims'	rp_d_P_ダミー＿顔カメラ＿エイムズ
		'president' rp_d_P_ダミー＿顔カメラ＿大統領
		'emma' rp_d_P_ダミー＿顔カメラ＿エマ
//		'emma2' rp_d_P_ダミー＿顔カメラ＿エマ２
		'emma_player_side' rp_d_P_ダミー＿顔カメラ＿エマプレイヤー側
		'raiden_gbsbody' rp_d_P_ダミー＿顔カメラ＿ゴルルコ兵マスクなしライデン
		'raiden_gbshead' rp_d_P_ダミー＿顔カメラ＿ゴルルコ兵マスクありライデン
		'Snake shaved' rp_d_P_ダミー＿顔カメラ＿ひげそりスネーク
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}


proc codec_set_p_P001_02_R01nt_P006_01_R01 {
	chara select P001_02_R01nt_P006_01_R01 -s {
		'P001_02_R01_nt' rp_d_P001_02_R01オープニング潜入２無線デモ１nt
		'P001_02_R01_et' rp_d_P001_02_R01オープニング潜入２無線デモ１et
		'P001_04_R02' rp_d_P001_04_R02オープニング潜入４無線デモ２ 
		'P002_02_R01' rp_d_P002_02_R01Ａ脚底部連結ハッチ２無線デモ１ 
		'P003_02_R02_nt' rp_d_P003_02_R02スネーク昇降機上昇２無線デモ１nt
		'P003_02_R02_et' rp_d_P003_02_R02スネーク昇降機上昇２無線デモ１et
		'P004_02_R01' rp_d_P004_02_R01ノード初接続２無線デモ１ 
		'P005_03_R01' rp_d_P005_03_R01ライデン昇降機上昇３無線デモ１ 
		'P005_05_R02' rp_d_P005_05_R02ライデン昇降機上昇５無線デモ２ 
		'P006_01_R01' rp_d_P006_01_R01Ａ脚金網１無線デモ１ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P007_01_R01_P016_01_R01 {
	chara select P007_01_R01_P016_01_R01 -s {
		'P007_01_R01' rp_d_P007_01_R01Ａ脚ノード前１無線デモ１ 
		'P008_01_R01' rp_d_P008_01_R01ＡＢ連絡橋１無線デモ１ 
		'P009_01_R01' rp_d_P009_01_R01_サイファー遭遇１無線デモ１ 
		'P010_04_R01' rp_d_P010_04_R01ヴァンプ遭遇４無線デモ１ 
		'P011_01_R01' rp_d_P011_01_R01ＢＣ連絡橋ＣＡＬＬ１無線デモ１ 
		'P012_02_R01' rp_d_P012_02_R01_フォーチュン遭遇２無線デモ１ 
		'P013_01_R01' rp_d_P013_01_R01_ライデン危機無線１無線デモ１ 
		'P014_05_R01' rp_d_P014_05_R01ピーター遭遇５無線デモ１ 
		'P015_01_R01' rp_d_P015_01_R1爆弾解体最初の一つ１無線デモ１ 
		'P016_01_R01' rp_d_P016_01_R1爆弾解体半分解体１無線デモ１ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P017_01_R01_P026_01_R01 {
	chara select P017_01_R01_P026_01_R01 -s {
		'P017_01_R01' rp_d_P017_01_R01爆弾解体中ローズその１無線デモ１ 
		'P018_01_R01' rp_d_P018_01_R01爆弾解体中ローズその２無線デモ１ 
		'P019_01_R01' rp_d_P019_01_R01爆弾解体中ローズその３無線デモ１ 
		'P020_01_R01_1' rp_d_P020_01_R01_地雷原１無線デモ１ 
		'P020_01_R01_2' rp_d_P020_01_R01_地雷原１無線デモ１＿２
		'P021_02_R01' rp_d_P021_02_R01_爆弾解体オルガ登場２無線デモ１ 
		'P022_01_R01' rp_d_P022_01_R01爆弾解体最後から二つ目１無線デモ１ 
		'P023_01_R01 sensor B' rp_d_P023_01_R01爆弾解体最後の一つ１無線デモ１ 
		'P023_01_R01 no sensor B' rp_d_P023_01_R01爆弾解体最後の一つ１無線デモ１＿センサーＢなし
		'P024_01_R01' rp_d_P024_01_R01爆弾解体センサーＢ入手１無線デモ１ 
		'P025_01_R01' rp_d_P025_01_R01爆弾解体昇降機下１無線デモ１ 
		'P026_01_R01' rp_d_P026_01_R01爆弾解体終了１無線デモ１ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P027_01_R01_P040_02_R02 {
	chara select P027_01_R01_P040_02_R02 -s {
		'P027_01_R01' rp_d_P027_01_R01爆弾解体後昇降機ホール１無線デモ１ 
		'P030_01_R01' rp_d_P030_01_R01フォーチュン戦中無線無線デモ１ 
		'P035_01_R01' rp_d_P035_01_R01ファットマン爆弾解体終了１無線デモ１ 
		'P035_01_R01_2' rp_d_P035_01_R01ファットマン爆弾解体終了１無線デモ１＿２ 
		'P036_03_R01' rp_d_P036_03_R01忍者登場３無線デモ１ 
		'P036_08_R02' rp_d_P036_08_R02忍者登場８無線デモ２ 
		'P036_10_R03' rp_d_P036_10_R03忍者登場１０無線デモ３ 
		'P036_13_R04' rp_d_P036_13_R04忍者登場１３無線デモ４ 
		'P037_01_R01' rp_d_P037_01_R01スネークの遺体１無線デモ１ 
		'P040_02_R02' rp_d_P040_02_R02エイムズ発見２無線デモ１ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P040_06a_R02a_P049_02_R01 {
	chara select P040_06a_R02a_P049_02_R01 -s {
		'P040_06a_R02a' rp_d_P040_06a_R02aエイムズ発見６a無線デモ２a
		'P040_06c_R02b' rp_d_P040_06c_R02bエイムズ発見６c無線デモ２b 
		'P041_02_R01' rp_d_P041_02_R01エイムズ死亡後２無線デモ１ 
		'P042_01_R01' rp_d_P042_01_R01シェル１シェル２連絡橋１無線デモ１ 
		'P044_01_R01_1' rp_d_P044_01_R01赤外線センサークリア１無線デモ１ 
		'P044_01_R01_2' rp_d_P044_01_R01赤外線センサークリア１無線デモ１＿２ 
		'P046_02_R01' rp_d_P046_02_R01ハリアー戦勝利２無線デモ１ 
		'P046_02_R01_2' rp_d_P046_02_R01ハリアー戦勝利２無線デモ１＿２ 
		'P047_05_R01' rp_d_P047_05_R01電撃床前オルガ５無線デモ１ 
		'P049_02_R01' rp_d_P049_02_R01大統領２無線機デモ１ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P049_05_R02_P052_01_R01 {
	chara select P049_05_R02_P052_01_R01 -s {
		'P049_05_R02' rp_d_P049_05_R02大統領５無線機デモ２ 
		'P049_08_R03' rp_d_P049_08_R03大統領８無線機デモ３ 
		'P049_10_R04' rp_d_P049_10_R04大統領１０無線機デモ４ 
		'P049_12_R05' rp_d_P049_12_R05大統領１２無線機デモ５ 
		'P050_01_R01' rp_d_P050_01_R01タンカー編整理１無線機デモ１ 
		'P050_04_R02' rp_d_P050_04_R02タンカー編整理４無線機デモ２ 
		'P050_06_R03' rp_d_P050_06_R03タンカー編整理６無線機デモ３ 
		'P051_01_R01' rp_d_P051_01_R01エマとオタコン１無線機デモ１ 
//		'P051_01_R01' rp_d_P051_01_R01エマとオタコン１無線機デモ１＿２ 
		'P052_01_R01' rp_d_P052_01_R01ローズ無線１無線機デモ１ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P053_01_R01_P062_05_R02 {
	chara select P053_01_R01_P062_05_R02 -s {
		'P053_01_R01' rp_d_P053_01_R01息継ぎ１無線機デモ１ 
		'P057_02_R01' rp_d_P057_02_R01ヴァンプ戦終了２無線機デモ１ 
		'P057_02_R01_2' rp_d_P057_02_R01ヴァンプ戦終了２無線機デモ１＿２ 
		'P058_02_R01' rp_d_P058_02_R01エマ救出２無線機デモ１ 
		'P058_04_R02' rp_d_P058_04_R02エマ救出４無線機デモ２ 
		'P058_07_R03' rp_d_P058_07_R03エマ救出７無線機デモ３ 
		'P059_03_R01' rp_d_P059_03_R01エマ休憩３無線機デモ１ 
		'P061_01_R01' rp_d_P061_01_R01エマとオタコン１無線機デモ１ 
		'P062_03_R01' rp_d_P062_03_R01エマとＡＩ３無線機デモ１ 
		'P062_05_R02' rp_d_P062_05_R02エマとＡＩ５無線機デモ２ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P064_01_R01_P070_23_R04 {
	chara select P064_01_R01_P070_23_R04 -s {
		'P064_01_R01' rp_d_P064_01_R01エマライデンローズ無線１無線機デモ１ 
		'P065_02_R01' rp_d_P065_02_R01エマＬ脚２無線機デモ１ 
		'P065_06_R02' rp_d_P065_06_R02Ｌ脚エマ６無線機デモ２ 
		'P067_01_R01' rp_d_P067_01_R01エマ狙撃１無線機デモ１ 
		'P069_02_R01' rp_d_P069_02_R01ヴァンプ狙撃終了２無線機デモ１ 
		'P069_04_R02' rp_d_P069_04_R02ヴァンプ狙撃終了４無線機デモ２ 
		'P070_10_R01' rp_d_P070_10_R01ＡＧ起動１０無線機デモ１ 
		'P070_18_R02' rp_d_P070_18_R02ＡＧ起動１８無線機デモ２ 
		'P070_20_R03' rp_d_P070_20_R03ＡＧ起動２０無線機デモ３ 
		'P070_23_R04' rp_d_P070_23_R04ＡＧ起動２３無線機デモ４ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P071_01_R01_P080_21_R02 {
	chara select P071_01_R01_P080_21_R02 -s {
		'P071_01_R01' rp_d_P071_01_R01拷問部屋脱出１無線機デモ１ 
		'P072_01_R01' rp_d_P072_01_R01大佐混乱１無線機デモ１ 
		'P073_01_R01' rp_d_P073_01_R01通路Ａ１無線機デモ１ 
		'P073_03_R02' rp_d_P073_03_R02通路Ａ３無線機デモ２ 
		'P074_02_R01' rp_d_P074_02_R01通路Ａ刀後２無線機デモ１ 
		'P075_01_R01' rp_d_P075_01_R01オタコン無事１無線機デモ１ 
		'P076_01_R01' rp_d_P076_01_R01大佐ＡＩ１無線機デモ１ 
		'P077_01_R01' rp_d_P077_01_R01ローズ拉致１無線機デモ１ 
		'P080_19_R01' rp_d_P080_19_R01ＡＧ浮上１９無線機デモ１ 
		'P080_21_R02' rp_d_P080_21_R02ＡＧ浮上２１無線機デモ２ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}

proc codec_set_p_P080_23_R03_P080_25_R04 {
	chara select P080_23_R03_P080_25_R04 -s {
		'P080_23_R03' rp_d_P080_23_R03ＡＧ浮上２３無線機デモ３ 
		'P080_25_R04' rp_d_P080_25_R04ＡＧ浮上２５無線機デモ４ 
		'back' back_to_root_p_demo
		'exit' exit_codec_debug_mode_p
	}
}




proc codec_debug_list_set_demo {
	if($f:rfp_enter_p_P001_02_R01nt_P006_01_R01){
		@codec_set_p_P001_02_R01nt_P006_01_R01

	}else if($f:rfp_enter_p_P007_01_R01_P016_01_R01){
		@codec_set_p_P007_01_R01_P016_01_R01

	}else if($f:rfp_enter_p_P017_01_R01_P026_01_R01){
		@codec_set_p_P017_01_R01_P026_01_R01

	}else if($f:rfp_enter_p_P027_01_R01_P040_02_R02){
		@codec_set_p_P027_01_R01_P040_02_R02
		 
	}else if($f:rfp_enter_p_P040_06a_R02a_P049_02_R01){
		@codec_set_p_P040_06a_R02a_P049_02_R01 

	}else if($f:rfp_enter_p_P049_05_R02_P052_01_R01){
		@codec_set_p_P049_05_R02_P052_01_R01

	}else if($f:rfp_enter_p_P053_01_R01_P062_05_R02){
		@codec_set_p_P053_01_R01_P062_05_R02

	}else if($f:rfp_enter_p_P064_01_R01_P070_23_R04){
		@codec_set_p_P064_01_R01_P070_23_R04

	}else if($f:rfp_enter_p_P071_01_R01_P080_21_R02){
		@codec_set_p_P071_01_R01_P080_21_R02

	}else if($f:rfp_enter_p_P080_23_R03_P080_25_R04){
		@codec_set_p_P080_23_R03_P080_25_R04
		 

	}else {
		@clear_flag_p
		@codec_set_demo_p
//		@codec_debug_list_set_p
	}
}


proc codec_debug_list_set_p {
	if($f:rfp_enter_demo_p) {
		@codec_debug_list_set_demo
	
	}else if($f:rfp_enter_codec_debug_mode_p_demo_vc) {
		@codec_debug_list_set_p_demo_vc

	}else if($f:rfp_enter_codec_debug_mode_p_peter) {
		@codec_debug_list_p_peter

	}else if($f:rfp_enter_codec_debug_mode_p_emma) {
		@codec_debug_list_p_emma
	
	}else if($f:rfp_enter_codec_debug_mode_p_colonel) {
		@codec_debug_list_p_colonel

	}else if($f:rfp_enter_codec_debug_mode_p_otacon) {
		@codec_debug_list_p_otacon

	}else if($f:rfp_enter_codec_debug_mode_p_rose) {
		@codec_debug_list_p_rose

	}else if($f:rfp_enter_codec_debug_mode_p_snake) {
		@codec_debug_list_p_snake

	}else if($f:rfp_enter_codec_debug_mode_p_colonel_add) {
		@codec_debug_list_p_colonel_add

	}else if($f:rfp_enter_codec_debug_mode_p_colonel_diver) {
		@codec_debug_list_p_colonel_diver

	//顔チェック
	}else if($f:rfp_enter_face_camera_check_p){
		@codec_set_face_camera_check_p

	}else {
		@codec_set_root_p
	}
}
