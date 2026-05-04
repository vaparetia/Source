//
// 全体マップ表示用のモデルをまとめたリソース定義
//
// 2001/07/05  S.Yamashita
// $Id: a_allmap_res.h,v 1.3 2002/07/25 11:25:25 usr03692 Exp $

resource A_全体マップ {
  プラント:
    $s:{
      a_w12,
      ab_w13,
      b_w14,
      bc_w15,
      c_w16,
      cd_w17,
      d_w18,
      de_w19,
      e_w20,
      ef_w21,
      f_w22,
      af_w23,
      s1_center_w24,
      dg_w25,
      g_w25,
	  gh,
	  h,
	  hi,
	  i,
	  ij,
	  j,
	  jk,
	  k,
      lk_w25,
      l_w25,
      gl_w25,
      s2_center_w31,
      l_w32,
      other,
      " "
    }
  /* タンカー編の分はこちらのリソースとして定義する */
/*
  タンカー:
    $s:{
	  tmap_base,
	  tmap_w00a_kanpan,
	  tmap_w00b_okujyou,
	  tmap_w01a_1f,
	  tmap_w01b_2f,
	  tmap_w01c_3f,
	  tmap_w01d_4f,
	  tmap_w01e_5f,
	  tmap_w02a_engineroom,
	  tmap_w03a_nagarouka,
	  tmap_w04a_sensou1,
	  tmap_w04b_sensou2,
	  tmap_w04c_sensou3,
      " "
    }
*/
  タンカー:
    $s:{
	  map_tanker_body,
	  map_tanker_00a,
	  map_tanker_01a,
	  map_tanker_01f,
	  map_tanker_01b,
	  map_tanker_01c,
	  map_tanker_01d,
	  map_tanker_01e,
	  map_tanker_00b,
	  map_tanker_02a,
	  map_tanker_03b,
	  map_tanker_03a,
	  map_tanker_03_other,
	  map_tanker_04a,
	  map_tanker_04b,
	  map_tanker_04c,
      " "
    }
  /* アーセナル編の分はこちらのリソースとして定義する */
  アーセナル:
    $s:{
	  map_41_mt,
	  map_42_mt,
	  map_43_mt,
	  map_44_mt,
	  map_45_mt,
      " "
    }
};
