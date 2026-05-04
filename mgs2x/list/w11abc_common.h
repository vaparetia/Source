//
//	w11abc_common.h	
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: w11abc_common.h,v 1.18 2001/08/14 01:37:16 usr04098 Exp $

// 各ステージ共通要素
#include	"p_common.h"
#include	"common.h"
#include	"c_effect.h"

// ライトは共通
lt2			w11.lt2

// アイテム(必要最小限に抑える)
pack_all	w11a_itembox.tri \
			itembox/weapon/handgun_ibox.dar \
			itembox/weapon/handgun_ibox_sh.dar \
			itembox/weapon/m92_label.dar \
			itembox/weapon/scm_label.dar \
			itembox/weapon/m92_label.dar \
			itembox/weapon/grenade_ibox.dar \
			itembox/weapon/grenade_ibox_sh.dar \
			itembox/weapon/cgr_label.dar \
			itembox/weapon/sgr_label.dar \
			itembox/amo/handgun_amo_ibox.dar \
			itembox/amo/handgun_amo_ibox_sh.dar \
			itembox/amo/m92_amo_label.dar \
			itembox/amo/scm_amo_label.dar \
			itembox/item/ration_ibox.dar \
			itembox/item/ration_ibox_sh.dar \
			itembox/item/rtn_label.dar \
			itembox/item/medicine_ibox.dar \
			itembox/item/medicine_ibox_sh.dar \
			itembox/item/sbs_label.dar \
			itembox/item/shv_label.dar \
			itembox/item/goggle_ibox.dar \
			itembox/item/goggle_ibox_sh.dar \
			itembox/item/tgl_label.dar \
			itembox/item/dogtag_ibox.dar \
			itembox/item/dogtag_ibox_sh.dar

itembox		weapon/handgun_ibox.kms
itembox		weapon/handgun_ibox_sh.kms
itembox		weapon/m92_label.kms
itembox		weapon/scm_label.kms
itembox		weapon/m92_label.kms
itembox		weapon/grenade_ibox.kms
itembox		weapon/grenade_ibox_sh.kms
itembox		weapon/cgr_label.kms
itembox		weapon/sgr_label.kms

itembox		amo/handgun_amo_ibox.kms
itembox		amo/handgun_amo_ibox_sh.kms
itembox		amo/m92_amo_label.kms
itembox		amo/scm_amo_label.kms

itembox		item/ration_ibox.kms
itembox		item/ration_ibox_sh.kms
itembox		item/rtn_label.kms
itembox		item/medicine_ibox.kms
itembox		item/medicine_ibox_sh.kms
itembox		item/sbs_label.kms
itembox		item/shv_label.kms
itembox		item/goggle_ibox.kms
itembox		item/goggle_ibox_sh.kms
itembox		item/tgl_label.kms
itembox		item/dogtag_ibox.kms
itembox		item/dogtag_ibox_sh.kms


// 水飛沫＆体燃え用cv2
cvd		human/sna_skl/sna_skl3.cvd vn

// ふなむし
pack_all	funa.tri \
			human/hnm_hunamushi/hnm_low_cm.dar \
			human/hnm_hunamushi/hnm_def.dar

pack_trnall	effect.tri \
			human/hnm_hunamushi/hnm_death_00_ovl_alp.bmp

human	hnm_hunamushi/hnm_low_cm.kms
human	hnm_hunamushi/hnm_def.kms


// その他のオブジェクト
pack_all	w11abc_other_obj.tri \
			world/elevator/elv_1/elv_1.dar

// 昇降機
world	elevator/elv_1/elv_1.kms
mtn   w11_shatter.mls
// 昇降機ボタン
pack_trnall  effect.tri \
	effect/ray_effect/ray_eye_bonbori_alp.bmp



// リネーム
