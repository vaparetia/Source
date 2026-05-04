PTARGET_INFO PtargInfo0[ PTARGET_LEVEL0_NUM ] = {
	{ 0, HUMAN21_KOSHI, 0, 0},
	{ 0, HUMAN21_ATAMA, 0, 0},
	{ 0, HUMAN21_ATAMA, 0, 0}
} ;
PTARGET_INFO PtargInfo1[ PTARGET_LEVEL1_NUM ] = {
#if 1
	{ EM_dam_out_b_cap01, HUMAN21_ATAMA, DownBack, 0},
	{ EM_dam_out_b_cap02, HUMAN21_MUNE, DownBack, 0},
	{ EM_dam_ball_cap01, HUMAN21_KOSHI, DownBack, 0}
#else
	{ EM_dam_out, HUMAN21_ATAMA, DownBack, 0},
	{ EM_dam_out, HUMAN21_MUNE, DownBack, 0},
	{ EM_dam_ball, HUMAN21_KOSHI, DownBack, 0}
#endif
} ;
PTARGET_INFO PtargInfo2[ PTARGET_LEVEL2_NUM ] = {
	{ EM_dam_arm_r, HUMAN21_MIGI_UDE1, 0, 0},
	{ EM_dam_arm_r, HUMAN21_MIGI_UDE2, 0, 0},
	{ EM_dam_arm_l, HUMAN21_HIDARI_UDE1, 0, 0},
	{ EM_dam_arm_l, HUMAN21_HIDARI_UDE2, 0, 0},
	{ EM_dam_leg_r, HUMAN21_MIGI_ASHI1, 0, 0},
	{ EM_dam_leg_r, HUMAN21_MIGI_ASHI2, 0, 0},
	{ EM_dam_leg_l, HUMAN21_HIDARI_ASHI1, 0, 0},
	{ EM_dam_leg_l, HUMAN21_HIDARI_ASHI2, 0, 0},
} ;
PTARGET_INFO PtargInfo3[ PTARGET_LEVEL3_NUM ] = {
	{ EM_dam_gun_3, HUMAN21_MUNE, 0, 0}
};

PTARGET_INFO PtargInfoFall[ PTARGET_FALL_NUM ] = {
	{ EM_dam_out_f_cap01, HUMAN21_MUNE, DownFront, 0},
	{ EM_dam_out_f_cap02, HUMAN21_MUNE, DownFront, 0},
	{ EM_dam_out_b_cap02, HUMAN21_MUNE, DownBack, 0},
	{ EM_dam_out_b_cap03, HUMAN21_MUNE, DownBack, 0},
	{ EM_dam_out_b_cap04, HUMAN21_MUNE, DownBack, 0}
};

PTARGET Ptarg={
	{PTARGET_LEVEL0_NUM, PTARGET_LEVEL1_NUM, PTARGET_LEVEL2_NUM, PTARGET_LEVEL3_NUM, 
		PTARGET_FALL_NUM },
	{PtargInfo0, PtargInfo1, PtargInfo2, PtargInfo3, PtargInfoFall }
} ;
