//
//	item_tank.h
//
//	written by S.Hirano 2000.Aug
//

// タンカー編共通アイテムボックス


// タンカー編で使用するアイテムボックス
// テクスチャをパック
#ifdef MGS2_LANG_ENGLISH
// 英語版
//----------------------------------
pack_all	item_box.tri \
// アモのボックス
				itembox/amo/handgun_amo_ibox.dar \
				itembox/amo/rifle_amo_ibox.dar \
// アモのボックスの影
				itembox/amo/handgun_amo_ibox_sh.dar \
				itembox/amo/rifle_amo_ibox_sh.dar \
// アモのラベル
				itembox/amo/m92_amo_label.dar \
				itembox/amo/usp_amo_label.dar \
				itembox/amo/fms_amo_label.dar \

// アイテムのボックス
				itembox/item/ration_ibox.dar \
				itembox/item/medicine_ibox.dar \
				itembox/item/box2_ibox.dar \
				itembox/item/a_p_sensor_ibox.dar \
				itembox/item/dogtag_ibox.dar \
// アイテムのボックスの影
				itembox/item/ration_ibox_sh.dar \
				itembox/item/medicine_ibox_sh.dar \
				itembox/item/box2_ibox_sh.dar \
				itembox/item/dogtag_ibox_sh.dar \

// アイテムのラベル
				itembox/item/rtn_label.dar \
				itembox/item/sbs_label.dar \
				itembox/item/etp_label.dar \
				itembox/item/dzp2_label.dar \
				itembox/item/cbx_label.dar \

// ウェポンのボックス
				itembox/weapon/handgun_ibox.dar \
				itembox/weapon/rifle_ibox.dar \
				itembox/weapon/grenade_ibox.dar \
// ウェポンのボックスの影
				itembox/weapon/handgun_ibox_sh.dar \
				itembox/weapon/rifle_ibox_sh.dar \
				itembox/weapon/grenade_ibox_sh.dar \
// ウェポンのラベル
				itembox/weapon/usp_label.dar \
				itembox/weapon/scm_label.dar \
				itembox/weapon/fms_label.dar \
				itembox/weapon/cgr_label.dar \
				itembox/weapon/sgr_label.dar \
				itembox/weapon/gre_label.dar
#else
// 日本語版
//----------------------------------
pack_all	item_box.tri \
// アモのボックス
				itembox/amo/handgun_amo_ibox.dar \
				itembox/amo/rifle_amo_ibox.dar \
// アモのボックスの影
				itembox/amo/handgun_amo_ibox_sh.dar \
				itembox/amo/rifle_amo_ibox_sh.dar \
// アモのラベル
				itembox/amo/m92_amo_label.dar \
				itembox/amo/usp_amo_label.dar \
				itembox/amo/fms_amo_label.dar \

// アイテムのボックス
				itembox/item/ration_ibox.dar \
				itembox/item/medicine_ibox.dar \
				itembox/item/box2_ibox.dar \
				itembox/item/a_p_sensor_ibox.dar \
				itembox/item/dogtag_ibox.dar \
// アイテムのボックスの影
				itembox/item/ration_ibox_sh.dar \
				itembox/item/medicine_ibox_sh.dar \
				itembox/item/box2_ibox_sh.dar \
				itembox/item/dogtag_ibox_sh.dar \

// アイテムのラベル
				itembox/item/rtn_label.dar \
				itembox/item/sbs_label.dar \
				itembox/item/etp_label.dar \
				itembox/item/dzp_label.dar \
				itembox/item/cbx_label.dar \

// ウェポンのボックス
				itembox/weapon/handgun_ibox.dar \
				itembox/weapon/rifle_ibox.dar \
				itembox/weapon/grenade_ibox.dar \
// ウェポンのボックスの影
				itembox/weapon/handgun_ibox_sh.dar \
				itembox/weapon/rifle_ibox_sh.dar \
				itembox/weapon/grenade_ibox_sh.dar \
// ウェポンのラベル
				itembox/weapon/usp_label.dar \
				itembox/weapon/scm_label.dar \
				itembox/weapon/fms_label.dar \
				itembox/weapon/cgr_label.dar \
				itembox/weapon/sgr_label.dar \
				itembox/weapon/gre_label.dar \
				itembox/weapon/usp_sp_label.dar
#endif

// モデル
itembox		amo/handgun_amo_ibox.kms
itembox		amo/rifle_amo_ibox.kms
itembox		amo/m92_amo_label.kms
itembox		amo/usp_amo_label.kms
itembox		amo/fms_amo_label.kms
itembox		amo/m4_amo_label.kms
itembox		amo/handgun_amo_ibox_sh.kms
itembox		amo/rifle_amo_ibox_sh.kms
itembox		item/ration_ibox.kms
itembox		item/medicine_ibox.kms
itembox		item/box2_ibox.kms
itembox		item/dogtag_ibox.kms
itembox		item/a_p_sensor_ibox.kms
itembox		item/ration_ibox_sh.kms
itembox		item/medicine_ibox_sh.kms
itembox		item/box2_ibox_sh.kms
itembox		item/dogtag_ibox_sh.kms
itembox		item/rtn_label.kms
itembox		item/sbs_label.kms
itembox		item/etp_label.kms
#ifdef MGS2_LANG_ENGLISH
itembox		item/dzp2_label.kms
#else
itembox		item/dzp_label.kms
#endif
itembox		item/cbx_label.kms
itembox		weapon/handgun_ibox.kms
itembox		weapon/rifle_ibox.kms
itembox		weapon/grenade_ibox.kms
itembox		weapon/usp_label.kms
itembox		weapon/handgun_ibox_sh.kms
itembox		weapon/rifle_ibox_sh.kms
itembox		weapon/m4_label.kms
itembox		weapon/grenade_ibox_sh.kms
itembox		weapon/scm_label.kms
itembox		weapon/fms_label.kms
itembox		weapon/cgr_label.kms
itembox		weapon/sgr_label.kms
itembox		weapon/gre_label.kms
itembox		weapon/usp_sp_label.kms

