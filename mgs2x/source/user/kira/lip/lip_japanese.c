/* ---------------------------------------------------------------- */
	/*
		動きへの変換テーブル
	*/

struct SLipStruct mtn_cnv_jpn[] = {

  /* 母音 */
  {PHJP_a, {{0.0, {LIP_A, LIP_unknown, 0.0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_i, {{0.0, {LIP_I, LIP_unknown, 0.0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_u, {{0.0, {LIP_U, LIP_unknown, 0.0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_e, {{0.0, {LIP_E, LIP_unknown, 0.0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_o, {{0.0, {LIP_O, LIP_unknown, 0.0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_N, {{0.0, {LIP_N, LIP_unknown, 0.0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},

  /* 半母音 */
  {PHJP_wa, {{0.0, {LIP_U, LIP_O, 0.2}},
	     {0.33, {LIP_A, LIP_S, 0.0}},
	     {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_ya, {{0.0, {LIP_I, LIP_E, 0.2}},
	     {0.33, {LIP_A, LIP_S, 0.0}},
	     {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  
  {PHJP_yu, {{0.0, {LIP_I, LIP_E, 0.2}},
	     {0.33, {LIP_U, LIP_S, 0.0}},
	     {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_yo, {{0.0, {LIP_I, LIP_E, 0.2}},
	     {0.33, {LIP_O, LIP_S, 0.0}},
	     {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  
  /* 子音 */
  {PHJP_k, {{0.0, {LIP_U, LIP_E, 0.15}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_h, {{0.0, {LIP_U, LIP_O, 0.2}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_r, {{0.0, {LIP_U, LIP_E, 0.2}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  
  {PHJP_g, {{0.0, {LIP_U, LIP_E, 0.2}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_s, {{0.0, {LIP_U, LIP_I, 0.2}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_t, {{0.0, {LIP_U, LIP_O, 0.01}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_n, {{0.0, {LIP_S, LIP_U, 0.001}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_z, {{0.0, {LIP_I, LIP_E, 0.3}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_ts, {{0.0, {LIP_U, LIP_I, 0.3}},
	     {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_d, {{0.0, {LIP_U, LIP_O, 0.01}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_w, {{0.0, {LIP_U, LIP_S, 0}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_m, {{0.0, {LIP_S, LIP_N, 0.1}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_b, {{0.0, {LIP_S, LIP_S, 0.0}},
	    {0.9, {LIP_U, LIP_S, 0.7}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_p, {{0.0, {LIP_S, LIP_S, 0.0}},
	    {0.9, {LIP_U, LIP_S, 0.7}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}},
  
  {PHJP_y, {{0.0, {LIP_I, LIP_E, 0.2}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 

  {PHJP_j, {{0.0, {LIP_I, LIP_U, 0.2}},
	    {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 
  
  {PHONE_sil, {{0.0, {LIP_S, LIP_S, 0.0}},
	       {1.0, {LIP_unknown, LIP_unknown, 0.0}}}}, 
  
  {PHONE_unknown, {{1.0, {LIP_unknown, LIP_unknown, 0.0}}}}
};
