//
//	menuicon.h
//	装備メニュー用アイコン
//	
//	2001/07/02	K.Takabe
//	$Id: menuicon.h,v 1.8 2002/02/15 01:58:13 usr01475 Exp $

// メニューアイコン
// pack_allに'-s'と付けることでパレット変更エフェクトの影響を受けなくなる

//ある程度メドが立ったら、要らない奴は消すこと！！！
pack_all	'-s' menuicon.tri \
	menu/menu_icon/acd_alp_ovl.bmp \
	menu/menu_icon/aks_alp_ovl.bmp \
	menu/menu_icon/aks_sp_alp_ovl.bmp \
	menu/menu_icon/aks_sp2_alp_ovl.bmp \
	menu/menu_icon/aps_alp_ovl.bmp \
	menu/menu_icon/bam_alp_ovl.bmp \
	menu/menu_icon/bnd_alp_ovl.bmp \
	menu/menu_icon/blt_alp_ovl.bmp \
	menu/menu_icon/bsn1_alp_ovl.bmp \
	menu/menu_icon/bsn2_alp_ovl.bmp \
	menu/menu_icon/camera_alp_ovl.bmp \
	menu/menu_icon/cfr_alp_ovl.bmp \
	menu/menu_icon/cls_alp_ovl.bmp \
	menu/menu_icon/cbx_a_alp_ovl.bmp \
	menu/menu_icon/cbx_b_alp_ovl.bmp \
	menu/menu_icon/cbx_c_alp_ovl.bmp \
	menu/menu_icon/cbx_d_alp_ovl.bmp \
	menu/menu_icon/cbx_e_alp_ovl.bmp \
	menu/menu_icon/cbx_f_alp_ovl.bmp \
	menu/menu_icon/chaff_alp_ovl.bmp \
	menu/menu_icon/cray_alp_ovl.bmp \
	menu/menu_icon/dmp_alp_ovl.bmp \
	menu/menu_icon/dog_alp_ovl.bmp \
	menu/menu_icon/empty_alp_ovl.bmp \
	menu/menu_icon/fms_alp_ovl.bmp \
	menu/menu_icon/funa_alp_ovl.bmp \
	menu/menu_icon/gol1_alp_ovl.bmp \
	menu/menu_icon/gol2_alp_ovl.bmp \
	menu/menu_icon/gre_alp_ovl.bmp \
	menu/menu_icon/hfb_alp_ovl.bmp \
	menu/menu_icon/hfb2_alp_ovl.bmp \
	menu/menu_icon/htl_alp_ovl.bmp \
	menu/menu_icon/m4a_nm_alp_ovl.bmp \
	menu/menu_icon/m9_alp_ovl.bmp \
	menu/menu_icon/mag_alp_ovl.bmp \
	menu/menu_icon/medi_alp_ovl.bmp \
	menu/menu_icon/mnd_alp_ovl.bmp \
	menu/menu_icon/mod_alp_ovl.bmp \
	menu/menu_icon/ngl_alp_ovl.bmp \
	menu/menu_icon/niki_alp_ovl.bmp \
	menu/menu_icon/no_use_alp_ovl.bmp \
	menu/menu_icon/psg_alp_ovl.bmp \
	menu/menu_icon/psg2_alp_ovl.bmp \
	menu/menu_icon/ration_alp_ovl.bmp \
	menu/menu_icon/rgb_alp_ovl.bmp \
	menu/menu_icon/sam_alp_ovl.bmp \
	menu/menu_icon/scm2_alp_ovl.bmp \
	menu/menu_icon/scm_alp_ovl.bmp \
	menu/menu_icon/scm_sp2_alp_ovl.bmp \
	menu/menu_icon/scope_alp_ovl.bmp \
	menu/menu_icon/shv_alp_ovl.bmp \
	menu/menu_icon/smoke_alp_ovl.bmp \
	menu/menu_icon/stin_alp_ovl.bmp \
	menu/menu_icon/stun_alp_ovl.bmp \
	menu/menu_icon/styp_alp_ovl.bmp \
	menu/menu_icon/tgl_alp_ovl.bmp \
	menu/menu_icon/usp_alp_ovl.bmp \
	menu/menu_icon/usp_sp2_alp_ovl.bmp \
	menu/menu_icon/usp_sp_alp_ovl.bmp \
	menu/menu_icon/win_alp_ovl.bmp \
	menu/menu_icon/wig1_alp_ovl.bmp \
	menu/menu_icon/wig2_alp_ovl.bmp \
	menu/menu_icon/wpb_alp_ovl.bmp \
	menu/menu_icon/cbox_dry_alp_ovl.bmp \
	menu/menu_icon/cbox_wet_alp_ovl.bmp 

// 最後の段ボールtextureは最終的に必要なくなる

#ifdef MGS2_XBOX_1
pack_all	'-s' menu.tri effect/font/font_alp_ovl.bmp effect/font/debug_font_alp_ovl.bmp effect/font/font_mini_alp_ovl.bmp effect/sys_icon/cd_err_alp_ovl.bmp
#endif
