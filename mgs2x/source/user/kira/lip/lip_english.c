/* ---------------------------------------------------------------- */
	/*
		動きへの変換テーブル
	*/
static struct SLipStruct mtn_cnv_eng[] = {

  /* 音素と口の形の対応表 */
  {PHEN_at,     {{0.0, {LIP_A, LIP_E, 0.6}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_E,      {{0.0, {LIP_E, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_I,      {{0.0, {LIP_I, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_i,      {{0.0, {LIP_I, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_A,      {{0.0, {LIP_A, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_R,      {{0.0, {LIP_U, LIP_O, 0.35}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_c,      {{0.0, {LIP_O, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_a,      {{0.0, {LIP_O, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_U,      {{0.0, {LIP_U, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_u,      {{0.0, {LIP_U, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_aI,     {{0.0, {LIP_A, LIP_unknown, 0.0}},
		 {0.34, {LIP_I, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_aU,     {{0.0, {LIP_A, LIP_unknown, 0.0}},
		 {0.34, {LIP_U, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_EI,     {{0.0, {LIP_E, LIP_unknown, 0.0}},
		 {0.34, {LIP_I, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_cI,     {{0.0, {LIP_O, LIP_unknown, 0.0}},
		 {0.34, {LIP_I, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_cU,     {{0.0, {LIP_O, LIP_unknown, 0.0}},
		 {0.34, {LIP_U, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_l,      {{0.0, {LIP_E, LIP_A, 0.6}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_y,      {{0.0, {LIP_I, LIP_E, 0.65}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_w,      {{0.0, {LIP_U, LIP_O, 0.5}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_r,      {{0.0, {LIP_A, LIP_U, 0.5}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_bar,    {{0.0, {LIP_E, LIP_O, 0.55}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_x,      {{0.0, {LIP_A, LIP_O, 0.6}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_X,      {{0.0, {LIP_U, LIP_E, 0.7}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_L,      {{0.0, {LIP_U, LIP_E, 0.8}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_M,      {{0.0, {LIP_N, LIP_unknown, 0.0}},
		 {0.7, {LIP_S, LIP_E, 0.1}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_N,      {{0.0, {LIP_N, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_m,      {{0.0, {LIP_N, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_n,      {{0.0, {LIP_I, LIP_U, 0.6}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_G,      {{0.0, {LIP_U, LIP_O, 0.7}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_V,      {{0.0, {LIP_V, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_D,      {{0.0, {LIP_U, LIP_E, 0.6}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_z,      {{0.0, {LIP_I, LIP_U, 0.8}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_Z,      {{0.0, {LIP_I, LIP_E, 0.8}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_J,      {{0.0, {LIP_I, LIP_E, 0.6}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_b,      {{0.0, {LIP_N, LIP_unknown, 0.0}},
		 {0.3, {LIP_S, LIP_U, 0.2}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},

  {PHEN_d,      {{0.0, {LIP_U, LIP_E, 0.2}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_g,      {{0.0, {LIP_E, LIP_I, 0.5}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_h,      {{0.0, {LIP_A, LIP_E, 0.5}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_C,      {{0.0, {LIP_U, LIP_I, 0.8}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 
  
  {PHEN_S,      {{0.0, {LIP_E, LIP_I, 0.4}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_s,      {{0.0, {LIP_I, LIP_E, 0.1}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_T,      {{0.0, {LIP_E, LIP_I, 0.4}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_f,      {{0.0, {LIP_U, LIP_O, 0.4}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_p,      {{0.0, {LIP_N, LIP_unknown, 0.0}},
		 {0.3, {LIP_S, LIP_U, 0.2}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_t,      {{0.0, {LIP_U, LIP_unknown, 0.0}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHEN_k,      {{0.0, {LIP_U, LIP_E, 0.2}},
		 {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHONE_sil, {{0.0, {LIP_S, LIP_S, 0.0}},
	       {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 
  
  {PHONE_unknown, {{1.0, {LIP_unknown, LIP_unknown, 0.0}}}}
};
