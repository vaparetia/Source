//
//	wp_sna_bossrush.h	
//
//	2001/09/17	M.Sonoyama
//  $Id: wp_sna_bossrush.h,v 1.2 2002/07/26 11:47:37 usr04761 Exp $

// ボスラッシュスネーク武器

// 共有モデル
pack_all	weapon_cm.tri \
    weapon/chaff_grenade/cgr_frg1_cm.dar \
	weapon/usp/usp_emb.dar 

// 上記以外のモデル
pack_all	weapon_nm.tri \
	weapon/m92_sna/m92_sna.dar \
	weapon/m92_sna/m92_sna_sub.dar \
	weapon/m92_sna/m92_sna_amo.dar \
	weapon/m92_sna/m92_bul2.dar \
	weapon/usp/usp.dar \
	weapon/usp/usp_amo.dar \
	weapon/usp/usp_sub.dar \
	weapon/usp/usp_sp.dar \
	weapon/usp/usp_sp_sub.dar \
	weapon/fms/fms_emb.dar \
	weapon/chaff_grenade \
	weapon/stun_grenade \
	goods/sling_test \
	weapon/grenade \
    weapon/aks/aks_amo.dar \
    weapon/mkr/mkr.dar \
    weapon/cls \
    weapon/stg \
	item/cigarettes \
    item/scope_rai/scp_rai.dar \
    item/digicame_rai/dcm_rai.dar \
    item/tgl

pack_trnall	wp_trans.tri \
	effect/bonbori/rcm_l_msk.bmp \
	effect/laser_sight/lsight_msk.bmp \
	effect/smoke/smoke_lp1_alp.bmp \
	effect/crash/pump02_alp.bmp \
	effect/blood/blood_2bw_msk.bmp \
	effect/weapon_effect/muzzlefrash_01_alp.bmp \
	effect/bakuha/bombpowder7_msk.bmp \
	effect/bakuha/fire3_alp.bmp \
	effect/bakuha/bombpowder6_msk.bmp \
	effect/weapon_effect/muzzlefrash_sev_01_alp.bmp \
    effect/footstamp/fstamp03_alp.bmp \
	effect/splash/splash05_alp.bmp

pack_all	c_box.tri \
    item/cbx_a/cbx_a_futa1_mt.dar \
    item/cbx_a/cbx_a_futa2_mt.dar \
    item/cbx_a/cbx_a_futa3_mt.dar \
    item/cbx_a/cbx_a_futa4_mt.dar \
    item/cbx_a/cbx_a_mt.dar 

pack_all	c_box_sub.tri \
	item/cbx_sub/cbx_sub.dar

weapon		m92_sna/m92_sna.kms
weapon		m92_sna/m92_sna_sub.kms
weapon		m92_sna/m92_sna_amo.kms
weapon		m92_sna/m92_bul2.kms
weapon		usp/usp_emb.kms
weapon		usp/usp.kms
weapon		usp/usp_amo.kms
weapon		usp/usp_sub.kms
weapon		usp/usp_sp.kms
weapon		usp/usp_sp_sub.kms

weapon		grenade
weapon		stg
weapon		cls
weapon		fms/fms_emb.kms
weapon		aks/aks_amo.kms	/* 必要ないが、管理上入れる */
weapon		mkr/mkr.kms		/* 重野くん対応待ち */
weapon		chaff_grenade
weapon		stun_grenade
goods		sling_test

item		cbx_a
item		cbx_sub/cbx_sub.kms
item		cigarettes
item		tgl
item		scope_rai
item		digicame_rai

cvd			item/cbx_a

pack_all	golequip.tri \
            human/gbs_def/gbs_knif.dar \
            human/gbs_def/gbs_mag.dar 
human		gbs_def/gbs_knif.kms
human       gbs_def/gbs_mag.kms

rename		m92_sna.kms			m92.kms
rename		m92_sna_sub.kms		m92_sub.kms
rename		m92_sna_amo.kms		m92_amo.kms

// こば４
//サイト コバ４
l2d 2D/sight/stg/stg.l2d 
l2d 2D/sight/stg/stg_alt.l2d // スティンガーチャフ時警告
l2d 2D/sight/dcm/dcm.l2d
#ifdef MGS2_XBOX
l2d 2D/sight/tdcm_x/tdcm.l2d 
#else
l2d 2D/sight/tdcm/tdcm.l2d 
#endif
l2d 2D/sight/dcm_shat/dcm_shat.l2d 
l2d 2D/sight/scope/scope.l2d
l2d 2D/sight/tgl/tgl.l2d

// tanker dcm empty raw image
2D  raw_image/tdcm_empty.raw 

//weapon      abakan/abk.kms
