/********************************************************************************/
/*	CYP_Wbk_Check.c								*/
/*	白びっくり  *CYP_Wbk_Check						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cam_wbk.c,v 1.1.1.3 2002/11/19 11:48:13 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/
/********************************************************************************/
/*	extern									*/
/********************************************************************************/
/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int CYP_Wbk_Check		** EYE_LAYER_CALL_ENEMY **	*/
/*	引数:	Work	*work							*/
/*	説明:	白びチェック							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Wbk_Check(Work *work){
    return 0;

    if (   CHECK_ENE_FLG(EYECK_ENE_MASUI) || 
	   CHECK_ENE_FLG(EYECK_ENE_OUCH) || 
	   CHECK_ENE_FLG(EYECK_ENE_DAMAGED) || 
	   CHECK_ENE_FLG(EYECK_ENE_CAPTURED) 
	 )
    {
	if (work->ACTION_LAYER > EYE_LAYER_BIKKURI){
	    SV_G1_STEP(work, MOD_WBK);
	    work->ACTION_LAYER = EYE_LAYER_BIKKURI;
	    return 1;
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void sub_call0							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void sub_wbk0(Work *work){

    if( work->discv_time > 240*5 ){		//通報

	CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_WHT_QE_MIN);

	SV_G2_STEP(work, SUB_MOD_ALERT);
	work->discv_time = 0;
	return;
    }
    
    /*びっくりセット*/
    if( ((CALL_HMARK_DELAY+TIME_BASE) > work->discv_time)&&(work->discv_time >= CALL_HMARK_DELAY) ){
	CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_WHT_AT_MIN);
	GM_SeSetMode( SD_E_CAMFND01 , work->eye.eyepos,		//カシャ
		      GM_SEMODE_BOMB );
    }
    
    Dir_from_2Vec(work->eye.eyepos, &work->eye.accident_pos,
		  &work->ctrl.turn);

    work->discv_time += TIME_BASE;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void sub_call1							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void sub_wbk1(Work *work){

    if( work->discv_time > 240*5 ){
	SV_G1_STEP(work, MOD_NORMAL);
	work->ACTION_LAYER = EYE_LAYER_Normal;
	work->rctrl.flag &= ~(RADAR_RADIO);
	return;
    }

    Dir_from_2Vec(work->eye.eyepos,&work->eye.accident_pos,
		  &work->ctrl.turn);


    work->discv_time += TIME_BASE;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActCall							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cam_wbk_control(Work *work){

    work->eye.trgpos = &GM_PlayerFindPos ;

    switch (G2_STEP)
    {
    case SUB_MOD_WARN:
	sub_wbk0(work);
	break;
	
    case SUB_MOD_ALERT:
	sub_wbk1(work);
	break;
    }

    //  通報キャンセル
}


