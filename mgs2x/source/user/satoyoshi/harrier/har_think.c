/********************************************************************************/
/*	Har_think.c								*/
/*	ハリアの思考というか、行動パターンの記述				*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id: har_think.c,v 1.1.1.3 2002/11/19 11:48:25 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

#define SET_STEP_L1(_mode){	\
    work->har_mode_l1 = (_mode);\
    work->har_mode_l2 = 0;	\
    work->har_mode_l3 = 0;	\
    work->har_mode_l4 = 0;	\
    work->l1_timer = 0;		\
    work->l2_timer = 0;		\
    work->l3_timer = 0;		\
    work->l4_timer = 0;		\
}





int mode_list[11] = {
    H_MOD_L1_PROGRAM_MOVE,
    H_MOD_L1_ACROBAT_MOVE,
    H_MOD_L1_PROGRAM_MOVE,
    H_MOD_L1_BURNING,
    H_MOD_L1_PROGRAM_MOVE,
    H_MOD_L1_HOVER_GUN,
    H_MOD_L1_HOVER_MP,
    H_MOD_L1_PROGRAM_MOVE,
    H_MOD_L1_HOVER_GUN,
    H_MOD_L1_PROGRAM_MOVE,
    H_MOD_L1_HOVER_MP
};

int mode_list2[4] = {
    H_MOD_L1_AMRAM,
    H_MOD_L1_HOVER_MP,
    H_MOD_L1_PROGRAM_MOVE,
    H_MOD_L1_HOVER_GUN,
};

void next_move(Work* work){	

    set_targbodyall_skip(work, ON);	//スキップ４




#ifdef DEBUG_MODE
    if (HAR_DbgMode){
	switch (HAR_DbgMode){
	case 1:
	    SET_STEP_L1(H_MOD_L1_CLASTER);
	    work->control.step = DG_ZeroVector;
	    break;
	case 2:
	    SET_STEP_L1(H_MOD_L1_HOVER_MP);
	    work->control.step = DG_ZeroVector;
	    break;

	case 3:
	    SET_STEP_L1(H_MOD_L1_BURNING);
	    work->control.step = DG_ZeroVector;
	    break;
	case 4:
	    SET_STEP_L1(H_MOD_L1_HOVER_GUN);
	    work->control.step = DG_ZeroVector;
	    break;
	case 5:
	    SET_STEP_L1(H_MOD_L1_AMRAM);
	    work->control.step = DG_ZeroVector;
	    break;

	case 6:
	    SET_STEP_L1(H_MOD_L1_PROGRAM_MOVE);
	    work->control.step = DG_ZeroVector;
	    break;


	case 99:
	    SET_STEP_L1(H_MOD_L1_STOP);
	    work->control.step = DG_ZeroVector;
	    break;
	}
	return;
    }
#endif

    if ( (work->claster_num==0) &&	// クラスター爆弾
	 (work->gageset.value < work->gageset.max*3/5)){
	work->save_before_claster_act = HAR_MODE_L1;
	SET_STEP_L1(H_MOD_L1_CLASTER);
	work->control.step = DG_ZeroVector;
	work->claster_num = 1;
	work->mode_num = -1;

    }
    else if (work->claster_num==1){	// 後半戦
	work->mode_num++;
	if (work->mode_num >= 4){
	    work->mode_num = 0;
	}
	SET_STEP_L1(mode_list2[work->mode_num]);
	work->control.step = DG_ZeroVector;
    }
    else {
	printf ("Change %d\n", work->mode_num);
	
	SET_STEP_L1(mode_list[work->mode_num]);
	work->mode_num++;
	if (work->mode_num >= 11){
	    work->mode_num = 8;
	}
	
    }

}
