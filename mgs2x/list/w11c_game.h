//
//	w11c.src	
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: w11c_game.h,v 1.9 2001/05/16 04:09:52 usr04098 Exp $


#include	"w11abc_common.h"
#include	"w11bc_common.h"

// キャラクタ 
include		fort.h      //フォーチュンとリニアライフル

// 強制モーション
mtn		rai_w11c.mls		// ステージ固有

// ワールド ( 残りをまとめてしまう )
hazard		w11c.hzx

//ここで使われている特別なBMP
pack_trnall  add_effect.tri \
	debug/morita/w11c_test/lnr_dankon_hibi1_alp.bmp      \
	debug/morita/w11c_test/lnr_dankon_hibi2_alp.bmp      \
	debug/morita/w11c_test/lnr_dankon_hibi3_alp.bmp      \
	debug/morita/w11c_test/w11_ibx_danmen_burned_add.bmp \
	debug/morita/w11c_test/w11c2_fire1_alp.bmp           \
	debug/morita/w11c_test/w11c2_fire1_alp_grey.bmp      \
	debug/morita/w11c_test/w11c2_fire2_alp.bmp           \
	debug/morita/w11c_test/w11c2_fire2_alp_grey.bmp      \
	debug/morita/w11c_test/w11c2_smoke1_alp.bmp          \
	debug/morita/w11c_test/w11c2_smoke2_alp.bmp          \
	debug/morita/w11c_test/fire4_msk.bmp                 \
	debug/morita/w11c_test/fire5_msk.bmp                 \
	debug/morita/w11c_test/w11c2_fire3a_alp.bmp          \
	debug/morita/w11c_test/w11c2_fire3b_alp.bmp          \
	debug/morita/w11c_test/w11c2_fire3c_alp.bmp          \
	effect/laser_sight/lsight_msk.bmp                    \
	effect/smoke/smoke_lp1_alp.bmp                       \
	effect/bakuha/bombgas3_alp.bmp

// 昇降機に乗ってるヴァンプ
human/vmp_coat 	vmp_coat_mh_mt.evm
human/vmp_coat 	vmp_parts_coat_mh.evm \
	vmp_hair_coat_mh_mt.evm \
	vmp_coat_bounding.kms
human/vmp_def 	vmp_parts_vkpa_mh.evm


// リネーム
