//
//	wp_rai_bossrush.h	
//	ライデン武器／ボスラッシュ用
//	2001/09/17	M.Sonoyama
//	$Id: wp_rai_bossrush.h,v 1.5 2002/09/18 13:31:37 usr03682 Exp $

// 武器もテクスチャはまとめておく
// アイテムもまとめます（99/12/16 M.Sonoyama）
// （シングルウェイト）マルチテクスチャモデル
//pack_all	weapon_mt.tri \
//	weapon/m92/m92_amo_mt.dar \
//	weapon/m92/m92_sub_mt.dar \
//	weapon/usp/usp_amo_mt.dar \
//	weapon/usp/usp_sub_mt.dar \
//	weapon/fms/fms_sub_mt.dar

// 共有モデル
pack_all	weapon_cm.tri \
    weapon/chaff_grenade/cgr_frg1_cm.dar \
	weapon/usp/usp_emb.dar \
	weapon/fms/fms_emb.dar \
    world/w01f/goods/book_hahen.dar 

// 上記以外のモデル
pack_all	weapon_nm.tri \
	weapon/m92_rai/m92_rai.dar \
	weapon/m92_rai/m92_rai_sub.dar \
	weapon/m92_rai/m92_rai_amo.dar \
	weapon/m92_sna/m92_bul2.dar \
	weapon/usp/usp.dar \
    weapon/aks_rai/aks_rai.dar \
    weapon/aks_rai/aks_rai_sub.dar \
    weapon/aks_rai/aks_rai_sp.dar \
    weapon/aks_rai/aks_sp_rai_sub.dar \
    weapon/aks_rai/aks_amo_rai_sub.dar \
    weapon/aks/aks_amo.dar \
    weapon/usp/usp_amo.dar \
	weapon/m4_rai \
	weapon/scm \
    weapon/chaff_grenade/cgr_frg1_cm.dar \
	weapon/chaff_grenade_rai \
	weapon/stun_grenade_rai \
	weapon/stun_grenade \
	goods/sling_test \
    weapon/cfr/cfr.dar \
    weapon/psg \
    weapon/grenade/grn.dar \
    weapon/grenade_rai \
    weapon/clm \
    weapon/nkt \
    weapon/stg \
    weapon/rgb \
    weapon/cls \
    weapon/dmp \
    weapon/syuukanshi/shuukanshi.dar \
    weapon/mkr/mkr.dar \
    weapon/hfb/hfb_sht.dar \
    weapon/hfb/hfb_grip_mount.dar \
    world/w01f/goods/book_naka.dar \
    world/w01f/goods/book_hahen.dar \
	item/cigarettes \
    item/tgl \
    item/ngl \
    item/scope_rai/scp_rai.dar \
    item/digicame_rai/dcm_rai.dar \
	item/cbx_sub/cbx_sub.dar

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

#if 0
pack_all	c_box_cm.tri \
    item/cbx_a/cbx_a_frg1_cm.dar \    
    item/cbx_a/cbx_a_frg2_cm.dar \    
    item/cbx_a/cbx_a_frg3_cm.dar
#endif

#if 0
pack_all	c_box.tri \
    item/cbx_a/cbx_a_mt.dar \
    item/cbx_b/cbx_b_dammy.dar \	
    item/cbx_c/cbx_c_dammy.dar \	
    item/cbx_d/cbx_d_dammy.dar \	
    item/cbx_e/cbx_e_dammy.dar

pack_all	c_box_futa.tri \
    item/cbx_a/cbx_a_futa1_mt.dar \
    item/cbx_a/cbx_a_futa2_mt.dar \
    item/cbx_a/cbx_a_futa3_mt.dar \
    item/cbx_a/cbx_a_futa4_mt.dar \
    item/cbx_b/cbx_b_futa1_mt.dar \
    item/cbx_b/cbx_b_futa2_mt.dar \
    item/cbx_b/cbx_b_futa3_mt.dar \
    item/cbx_b/cbx_b_futa4_mt.dar \
    item/cbx_c/cbx_c_futa1_mt.dar \
    item/cbx_c/cbx_c_futa2_mt.dar \
    item/cbx_c/cbx_c_futa3_mt.dar \
    item/cbx_c/cbx_c_futa4_mt.dar \
    item/cbx_d/cbx_d_futa1_mt.dar \
    item/cbx_d/cbx_d_futa2_mt.dar \
    item/cbx_d/cbx_d_futa3_mt.dar \
    item/cbx_d/cbx_d_futa4_mt.dar \
    item/cbx_e/cbx_e_futa1_mt.dar \
    item/cbx_e/cbx_e_futa2_mt.dar \
    item/cbx_e/cbx_e_futa3_mt.dar \
    item/cbx_e/cbx_e_futa4_mt.dar 

#endif

pack_all	c_box.tri \
    item/cbx_a/cbx_a_mt.dar \
    item/cbx_b/cbx_b_mt.dar \
    item/cbx_c/cbx_c_mt.dar \
    item/cbx_d/cbx_d_mt.dar \
    item/cbx_e/cbx_e_mt.dar \
    item/cbx_a/cbx_a_futa1_mt.dar \
    item/cbx_a/cbx_a_futa2_mt.dar \
    item/cbx_a/cbx_a_futa3_mt.dar \
    item/cbx_a/cbx_a_futa4_mt.dar \
    item/cbx_b/cbx_b_futa1_mt.dar \
    item/cbx_b/cbx_b_futa2_mt.dar \
    item/cbx_b/cbx_b_futa3_mt.dar \
    item/cbx_b/cbx_b_futa4_mt.dar \
    item/cbx_c/cbx_c_futa1_mt.dar \
    item/cbx_c/cbx_c_futa2_mt.dar \
    item/cbx_c/cbx_c_futa3_mt.dar \
    item/cbx_c/cbx_c_futa4_mt.dar \
    item/cbx_d/cbx_d_futa1_mt.dar \
    item/cbx_d/cbx_d_futa2_mt.dar \
    item/cbx_d/cbx_d_futa3_mt.dar \
    item/cbx_d/cbx_d_futa4_mt.dar \
    item/cbx_e/cbx_e_futa1_mt.dar \
    item/cbx_e/cbx_e_futa2_mt.dar \
    item/cbx_e/cbx_e_futa3_mt.dar \
    item/cbx_e/cbx_e_futa4_mt.dar 

weapon		m92_rai/m92_rai.kms
weapon		m92_rai/m92_rai_sub.kms
weapon		m92_rai/m92_rai_amo.kms
weapon		m92_sna/m92_bul2.kms
weapon		usp/usp.kms
weapon		usp/usp_emb.kms	
weapon		usp/usp_amo.kms	
weapon		aks_rai/aks_rai.kms 
weapon		aks_rai/aks_rai_sub.kms
weapon		aks_rai/aks_rai_sp.kms
weapon		aks_rai/aks_sp_rai_sub.kms
weapon		aks_rai/aks_amo_rai_sub.kms
weapon		aks/aks_amo.kms
goods		sling_test
weapon		fms/fms_emb.kms

weapon		m4_rai
weapon		scm
weapon		cfr/cfr.kms
weapon		psg
weapon		chaff_grenade/cgr_frg1_cm.kms
weapon		chaff_grenade_rai
weapon		stun_grenade_rai
weapon		stun_grenade
weapon		clm
weapon		grenade/grn.kms
weapon		grenade_rai
weapon		nkt
weapon		stg
weapon		rgb
weapon		cls
weapon		dmp
weapon      syuukanshi/shuukanshi.kms

weapon		mkr/mkr.kms			/* 重野くん対応待ち */

weapon		hfb/hfb_sht.kms
weapon		hfb/hfb_grip_mount.kms

item		cbx_sub/cbx_sub.kms

item		cbx_a/cbx_a_mt.kms
item		cbx_b/cbx_b_mt.kms
item		cbx_c/cbx_c_mt.kms
item		cbx_d/cbx_d_mt.kms
item		cbx_e/cbx_e_mt.kms

item		cbx_a/cbx_a_futa1_mt.kms
item		cbx_a/cbx_a_futa2_mt.kms
item		cbx_a/cbx_a_futa3_mt.kms
item		cbx_a/cbx_a_futa4_mt.kms
item		cbx_b/cbx_b_futa1_mt.kms
item		cbx_b/cbx_b_futa2_mt.kms
item		cbx_b/cbx_b_futa3_mt.kms
item		cbx_b/cbx_b_futa4_mt.kms
item		cbx_c/cbx_c_futa1_mt.kms
item		cbx_c/cbx_c_futa2_mt.kms
item		cbx_c/cbx_c_futa3_mt.kms
item		cbx_c/cbx_c_futa4_mt.kms
item		cbx_d/cbx_d_futa1_mt.kms
item		cbx_d/cbx_d_futa2_mt.kms
item		cbx_d/cbx_d_futa3_mt.kms
item		cbx_d/cbx_d_futa4_mt.kms
item		cbx_e/cbx_e_futa1_mt.kms
item		cbx_e/cbx_e_futa2_mt.kms
item		cbx_e/cbx_e_futa3_mt.kms
item		cbx_e/cbx_e_futa4_mt.kms

kms_archiver	cbx	\
            cbx_a_mt.kms \
            cbx_b_mt.kms \
            cbx_c_mt.kms \
            cbx_d_mt.kms \
            cbx_e_mt.kms

kms_archiver    cbx_futa1 \
            cbx_a_futa1_mt.kms \
            cbx_b_futa1_mt.kms \
            cbx_c_futa1_mt.kms \
            cbx_d_futa1_mt.kms \
            cbx_e_futa1_mt.kms 

kms_archiver    cbx_futa2 \
            cbx_a_futa2_mt.kms \
            cbx_b_futa2_mt.kms \
            cbx_c_futa2_mt.kms \
            cbx_d_futa2_mt.kms \
            cbx_e_futa2_mt.kms 

kms_archiver    cbx_futa3 \
            cbx_a_futa3_mt.kms \
            cbx_b_futa3_mt.kms \
            cbx_c_futa3_mt.kms \
            cbx_d_futa3_mt.kms \
            cbx_e_futa3_mt.kms 

kms_archiver    cbx_futa4 \
            cbx_a_futa4_mt.kms \
            cbx_b_futa4_mt.kms \
            cbx_c_futa4_mt.kms \
            cbx_d_futa4_mt.kms \
            cbx_e_futa4_mt.kms 

// cvdは共通で使い回す
cvd			item/cbx_a
cvd			item/cbx_e

item		cigarettes
item		tgl
item		ngl
item		scope_rai
item		digicame_rai

world/w01f/goods	book_naka.kms 
world/w01f/goods	book_hahen.kms
cvd 				world/w01f/goods/book_naka.cv2 vn

#if 0
//ボデーアーマー
pack_all	bodyarmor_mt.tri \
            item/rba_bodyarmor/rba_def_mh_mt.dar 

item		rba_bodyarmor/rba_def_mh_mt.evm
#endif

//サイト コバ４
l2d 2D/sight/stg/stg.l2d 
l2d 2D/sight/stg/stg_alt.l2d // スティンガーチャフ時警告
l2d 2D/sight/dcm/dcm.l2d
l2d 2D/sight/psg/psg.l2d
l2d 2D/sight/tgl/tgl.l2d
l2d 2D/sight/ngl/ngl.l2d
l2d 2D/sight/nkt/nkt.l2d
l2d 2D/sight/dcm_shat/dcm_shat.l2d 
l2d 2D/sight/scope/scope.l2d

// 長廊下戦で必要なので入れます。
pack_all	golequip.tri \
            human/gbs_def/gbs_knif.dar \
            human/gbs_def/gbs_mag.dar 
human		gbs_def/gbs_knif.kms
human       gbs_def/gbs_mag.kms

#if 0
//ゴルルコ兵装備
pack_all	golequip_mt.tri \
            human/rai_gbs/rai_gbs_mt.dar \
            human/rai_gbs/rai_gbs_sh_mt.dar

pack_all	golequip.tri \
            human/rai_gbs/rai_gbs_gbshead.dar \
            human/gbs_def/gbs_knif.dar \
            human/gbs_def/gbs_mag.dar 

human		rai_gbs/rai_gbs_mt.kms 
human		rai_gbs/rai_gbs_sh_mt.kms
human		rai_gbs/rai_gbs_gbshead.kms
human		gbs_def/gbs_knif.kms
human       gbs_def/gbs_mag.kms

//ゴルルコ主観腕
pack_all	golhand.tri \
            human/rah_raiden_hand/rah_gbs_mh.dar
human		rah_raiden_hand/rah_gbs_mh.evm

//ゴルキャップ主観
pack_all	goruuuu.tri \
            goods/raiden_gbssub/raiden_gbssub_alp_ovl.bmp
#endif

//ＲＧＢモーション
mtn			rgb.mls

// PSG１レンズモデル
effect	scr_effect/psg_lenz_fr.kms
effect	scr_effect/psg_lenz_bd.kms

//リネーム
rename		m92_rai.kms			m92.kms
rename		m92_rai_sub.kms		m92_sub.kms
rename		m92_rai_amo.kms		m92_amo.kms

