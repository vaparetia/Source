//
//	w31a_game.h	
//	シェル２中央棟１Ｆ-大統領イベント時
//
//	written by H.Yoshiike 2000.Nov
//
//	$Id: w31a_game.h,v 1.8 2001/08/06 08:48:03 usr03682 Exp $


// w31共通要素
#include	"w31ad_common.h"

// キャラクタ 
// 大統領
pack_all	jam_def_mt.tri \
			human/jam_def/jam_def_sh_mt.dar \
			human/jam_def/jam_def_deadface.dar \
			human/jam_def/jam_tie.dar

pack_all	jam_def_mh_mt.tri \
			human/jam_def/jam_def_mh_mt.dar \
			human/jam_def/jam_def_deadface.dar

human		jam_def/jam_bounding.kms
human		jam_def/jam_bounding2.kms
human		jam_def/jam_def_sh_mt.kms
human		jam_def/jam_def_mh_mt.evm
human		jam_def/jam_tie.kms
human		jam_def/jam_def_deadface.evm

// オルガ
pack_all	org_def_mt.tri \
			human/org_def/org_radio_forhand.dar \
			human/org_plant/org_plant_sh_mt.dar

pack_all	org_def_mh_mt.tri \
			human/org_plant/org_plant_mh_mt.dar \
			human/org_plant/org_plant_hair_mh.dar

human	org_def/org_radio_forhand.kms
human	org_plant/org_plant_sh_mt.kms
human	org_plant/org_plant_mh_mt.evm
human	org_plant/org_plant_hair_mh.evm
human	org_plant/org_plant_dummy.kms

// 強制モーション
mtn		rai_w31a.mls
mtn		jam.mls
mtn		corps.mls
mtn		org_w31a.mls
far		jam_face.far


// パッドデモ
rpd		w31a_paddemo.rpd

// 振動データ
vib		program/rai_spark.vib
vib		program/rai_non_dam_sps_f.vib

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
