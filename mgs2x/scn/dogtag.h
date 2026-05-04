/*
	dogtag.h
		ドッグタグ用ヘッダ

	2001/05/10 S.Nojiri
	$Id: dogtag.h,v 1.5 2001/06/30 08:00:23 usr01717 Exp $

*/




//###################################################################################
//								ドッグタグ設定
//###################################################################################


enum dogtag_list {

//w00a
	tnk_w00a_guard_ve_01	= 0,
	tnk_w00a_guard_ea_01,
	tnk_w00a_guard_no_01,
	tnk_w00a_guard_ha_01,
	tnk_w00a_guard_vh_01,

	tnk_w00a_guard_ve_02,
	tnk_w00a_guard_ea_02,
	tnk_w00a_guard_no_02,
	tnk_w00a_guard_ha_02,
	tnk_w00a_guard_vh_02,

	tnk_w00a_guard_ve_03,
	tnk_w00a_guard_ea_03,
	tnk_w00a_guard_no_03,
	tnk_w00a_guard_ha_03,
	tnk_w00a_guard_vh_03,

//w00c
	tnk_w00c_olga,

	tnk_w00c_guard_ve_01,
	tnk_w00c_guard_ea_01,
	tnk_w00c_guard_no_01,
	tnk_w00c_guard_ha_01,
	tnk_w00c_guard_vh_01,

	tnk_w00c_guard_ve_02,
	tnk_w00c_guard_ea_02,
	tnk_w00c_guard_no_02,
	tnk_w00c_guard_ha_02,
	tnk_w00c_guard_vh_02,

//w01a
	tnk_w01a_guard_ve_01,
	tnk_w01a_guard_ea_01,
	tnk_w01a_guard_no_01,
	tnk_w01a_guard_ha_01,
	tnk_w01a_guard_vh_01,

//w01b
	tnk_w01b_guard_ve_01,
	tnk_w01b_guard_ea_01,
	tnk_w01b_guard_no_01,
	tnk_w01b_guard_ha_01,
	tnk_w01b_guard_vh_01,

	tnk_w01b_guard_no_02,
	tnk_w01b_guard_ha_02,
	tnk_w01b_guard_vh_02,

//w01f
	tnk_w01f_guard_ve_01,
	tnk_w01f_guard_ea_01,
	tnk_w01f_guard_no_01,
	tnk_w01f_guard_ha_01,
	tnk_w01f_guard_vh_01,

	tnk_w01f_guard_ve_02,
	tnk_w01f_guard_ea_02,
	tnk_w01f_guard_no_02,
	tnk_w01f_guard_ha_02,
	tnk_w01f_guard_vh_02,

	tnk_w01f_guard_ve_03,
	tnk_w01f_guard_ea_03,
	tnk_w01f_guard_no_03,
	tnk_w01f_guard_ha_03,
	tnk_w01f_guard_vh_03,

//w01d
	tnk_w01d_guard_ve_01,
	tnk_w01d_guard_ea_01,
	tnk_w01d_guard_no_01,
	tnk_w01d_guard_ha_01,
	tnk_w01d_guard_vh_01,

	tnk_w01d_guard_ve_02,
	tnk_w01d_guard_ea_02,
	tnk_w01d_guard_no_02,
	tnk_w01d_guard_ha_02,
	tnk_w01d_guard_vh_02,

	tnk_w01d_guard_ve_03,
	tnk_w01d_guard_ea_03,
	tnk_w01d_guard_no_03,
	tnk_w01d_guard_ha_03,
	tnk_w01d_guard_vh_03,

	tnk_w01d_guard_ve_04,
	tnk_w01d_guard_ea_04,
	tnk_w01d_guard_no_04,
	tnk_w01d_guard_ha_04,
	tnk_w01d_guard_vh_04,

//w03a
	tnk_w03a_guard_ve_01,
	tnk_w03a_guard_ea_01,
	tnk_w03a_guard_no_01,
	tnk_w03a_guard_ha_01,
	tnk_w03a_guard_vh_01,

	tnk_w03a_guard_ve_02,
	tnk_w03a_guard_ea_02,
	tnk_w03a_guard_no_02,
	tnk_w03a_guard_ha_02,
	tnk_w03a_guard_vh_02,

	tnk_w03a_guard_ve_03,
	tnk_w03a_guard_ea_03,
	tnk_w03a_guard_no_03,
	tnk_w03a_guard_ha_03,
	tnk_w03a_guard_vh_03,


//w02a
	tnk_w02a_guard_ve_01,
	tnk_w02a_guard_ea_01,
	tnk_w02a_guard_no_01,
	tnk_w02a_guard_ha_01,
	tnk_w02a_guard_vh_01,

	tnk_w02a_guard_ve_02,
	tnk_w02a_guard_ea_02,
	tnk_w02a_guard_no_02,
	tnk_w02a_guard_ha_02,
	tnk_w02a_guard_vh_02,

	tnk_w02a_guard_ve_03,
	tnk_w02a_guard_ea_03,
	tnk_w02a_guard_no_03,
	tnk_w02a_guard_ha_03,
	tnk_w02a_guard_vh_03,

	tnk_w02a_guard_ve_04,
	tnk_w02a_guard_ea_04,
	tnk_w02a_guard_no_04,
	tnk_w02a_guard_ha_04,
	tnk_w02a_guard_vh_04,

	tnk_w02a_guard_ve_05,
	tnk_w02a_guard_ea_05,
	tnk_w02a_guard_no_05,
	tnk_w02a_guard_ha_05,
	tnk_w02a_guard_vh_05,

	tnk_w02a_guard_ve_06,
	tnk_w02a_guard_ea_06,
	tnk_w02a_guard_no_06,
	tnk_w02a_guard_ha_06,
	tnk_w02a_guard_vh_06,

	tnk_w02a_guard_ve_07,
	tnk_w02a_guard_ea_07,
	tnk_w02a_guard_no_07,
	tnk_w02a_guard_ha_07,
	tnk_w02a_guard_vh_07,

};










