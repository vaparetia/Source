//
//	w31c_vamp.h
//	ボス戦　ヴァンプに必要
//
//	T.Shibata 2001/06/21
//
//	$Id: w31c_vamp.h,v 1.7 2001/08/06 10:33:30 usr10750 Exp $

// 
#ifndef W31C_VAMP_H
#define W31C_VAMP_H

mtn		w31c_vmp_base.mls

human	vmp_def/vmp_naked_light_mh_mt.evm
human	vmp_def/vmp_hair_naked_mh_mt.evm

human	vmp_coat/vmp_coat_bounding.kms
human	vmp_def/vmp_naked_bounding.kms
human	vmp_def/vmp_naked_shadow.kms
human	vmp_def/vmp_naked_sh.kms
weapon	vamp_knife/vkf_big_inv.kms

human	vmp_def/vmp_parts_vkpa_mh.evm
human	vmp_def/vmp_parts_vkpb_mh.evm

weapon	vamp_knife/vkf_small_cm.kms

//テスト用あがってたらごめんなさい　柴田
debug	sasaki/dbg_vmp_naked_hair_mh_mt.evm

//水滴エフェクト
debug	shibata/awa_model/awa_model2.kms
cvd		debug/shibata/awa_model/awa_model2.cv2


pack_trnall stage_effect.tri \
	debug/shibata/staffroll/alpha03_alp_ovl.bmp \
	effect/wave/wave12_alp_ovl.bmp \
	effect/blood/blood_2bw_alp.bmp

#endif
