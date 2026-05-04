//
//	w32b_game.h
//
//	written by H.Yoshiike 2000.Nov
//
//	$Id: w32b_game.h,v 1.7 2001/08/27 08:21:54 usr03379 Exp $

#include "w32_common_game.h"

// キャラクタ
	//	ヴァンプ
// 本体と他を分けているのは、本体のマルチと他のマルチは描画フェイズが異なるため
// bounding.kmsを一度に入れているのは表示されないため
pack_all vmp_parts.tri \
	human/vmp_def/vmp_parts_vkpa_mh.dar \
	human/vmp_def/vmp_naked_light_mh_mt.dar \
	human/vmp_def/vmp_naked_bounding.dar

//	human/vmp_coat/vmp_hair_coat_mh_mt.evm \	//	裸用に変更
//	human/vmp_def/vmp_hair_naked_mh_mt.dar \

//	human/vmp_coat/vmp_coat_bounding.kms	//	裸用に変更

human	vmp_def/vmp_naked_light_mh_mt.evm // 顔無しなの？
//human 	vmp_coat/vmp_coat_mh_mt.evm       // コート無しなの？

//human	vmp_coat/vmp_hair_coat_mh_mt.evm
human	vmp_def/vmp_hair_naked_mh_mt.evm 

//human	vmp_coat/vmp_coat_bounding.kms
human	vmp_def/vmp_naked_bounding.kms

human	vmp_def/vmp_parts_vkpa_mh.evm 

human	vmp_def/vmp_naked_shadow.kms		//麻酔刺さる用

//	モーション
	//	ヴァンプ
mtn		w32a_vamp.mls


// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム

