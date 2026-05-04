/********************************************************************************/
/*	Sv_camera.c								*/
/*	監視カメラ  *NewSvCamera						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cyp_call.c,v 1.1.1.3 2002/11/19 11:48:14 Yoshizawa1 Exp $		*/
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
#ifdef KP_XBOX	// サイファ音は遮蔽計算なしにする
#define	GM_SeSetFromVolCurve( a, b, c ) GM_SeSetFromVolCurveAddr( a, b, c, GM_INVALID_ADDR )
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif

#define	SV_DISCV_DELAY		(90*5)	//	捉えてから通報までの時間差
#define	SV_HMARK_DELAY		(60*5)	//	捉えてからビックリまで*/
#define	SV_ALERT_TIME		(120*5)	//	捉えてからビックリまで*/


#define CALL_HMARK_DELAY	(30*5)	//
#define	SV_CALL_DELAY		(120*5)	//
#define	SV_CALL_END_TIME	(300*5)	//


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Call_Check		** EYE_LAYER_CALL_ENEMY **		*/
/*	引数:	Work	*work							*/
/*	説明:	呼びチェック							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CYP_Call_Check(Work *work){
    
    if ( (	 CHECK_PC_FLG(EYECK_PC_IN_SIGHT) &&
		 CHECK_PC_FLG(EYECK_PC_IN_UNIFORM) &&
		 CHECK_PC_FLG(EYECK_PC_PLONE) ) &&
	 ( !COM_AccidentToID(work->com_uniq_id) && 
	   (work->ACTION_LAYER > EYE_LAYER_CALL_ENEMY) ) ){
	SV_G1_STEP(work, CYP_CALL);
	work->ACTION_LAYER = EYE_LAYER_CALL_ENEMY;
	work->call_type = CALL_PC_MISTAKEN_DAMAGED;
	return 1;
    }
    
    if ( !COM_AccidentToID(work->eye.accident_uniq_id) && 
	 (work->eye.accident_uniq_id != 0) &&
	 ( CHECK_ENE_FLG(EYECK_ENE_SLEEPING) || 
	   CHECK_ENE_FLG(EYECK_ENE_FAINT) || 
	   CHECK_ENE_FLG(EYECK_ENE_DEAD) ) )
    {
	if (work->ACTION_LAYER > EYE_LAYER_CALL_ENEMY){
	    SV_G1_STEP(work, CYP_CALL);
	    work->ACTION_LAYER = EYE_LAYER_CALL_ENEMY;
	    work->call_type = CALL_ENE_DOWN_BODY;
	    work->call_uid = work->eye.accident_uniq_id;
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
static void sub_call0(Work *work){


    if (work->call_type == CALL_PC_MISTAKEN_DAMAGED){	// プレイヤー変装ダウン
	if( !CHECK_PC_FLG(EYECK_PC_PLONE) ){
	    SV_G1_STEP(work, CYP_NORMAL);
	    work->ACTION_LAYER = EYE_LAYER_Normal;
	    return;
	}
    }

    
    if( work->discv_time > SV_CALL_DELAY ){		//通報


	if (work->call_type == CALL_ENE_DOWN_BODY){

	    int tmp = 
	    COM_SetAccident( work->call_uid, &work->eye.accident_pos,	//チェックの通報
			     work->control.hzx_id, DEV_ACCIDENT_DELAY_TIME, ENE_ACCIDENT_MECA_ENEDOWN) ;
	    printf("#####Call uid %d\n", work->call_uid);
	    printf("#####Call Set Accident %d\n", tmp);
	}
	else if (work->call_type == CALL_PC_MISTAKEN_DAMAGED){
	    printf ("Player Call\n");
	    COM_SetAccident( work->com_uniq_id, work->eye.trgpos,	//チェックの通報
			     work->control.hzx_id, DEV_ACCIDENT_DELAY_TIME, ENE_ACCIDENT_MECA_PLAYER) ;

	}
	work->rctrl.flag |= (RADAR_RADIO);
	SV_G2_STEP(work, SUB_MOD_ALERT);
	work->discv_time = 0;
	return;
    }
    
    /*びっくりセット*/
    if( ((CALL_HMARK_DELAY+TIME_BASE) > work->discv_time)&&(work->discv_time >= CALL_HMARK_DELAY) ){
	CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_RED_AT_MIN);
	GM_SeSetMode( SD_E_CAMFND01 , work->eye.eyepos,		//カシャ
		      GM_SEMODE_BOMB );
    }
    
    /*目標物へ向く*/
    if (work->call_type == CALL_ENE_DOWN_BODY){
	Dir_from_2Vec(work->eye.eyepos,&work->eye.accident_pos,
		      &work->control.turn);
    }
    else {
	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
		      &work->control.turn);
    }
    if (work->control.turn.vx > 3000){
	work->control.turn.vx -= 4096;
    }


    // ****************アクシデントの場所******************
    {	
	TracePlayer3D(work, &work->eye.accident_pos);
    }

    work->discv_time += TIME_BASE;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void sub_call1							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void sub_call1(Work *work){

    //  通報キャンセル
    if (work->call_type == CALL_PC_MISTAKEN_DAMAGED){	// プレイヤー変装ダウン
	if( !CHECK_PC_FLG(EYECK_PC_PLONE) ){
	    SV_G1_STEP(work, CYP_NORMAL);
	    work->ACTION_LAYER = EYE_LAYER_Normal;
	    COM_UnsetAccident(work->com_uniq_id);
	    return;
	}
    }
    
    if( work->discv_time > (DEV_ACCIDENT_DELAY_TIME) ){
	SV_G1_STEP(work, CYP_NORMAL);
	work->ACTION_LAYER = EYE_LAYER_Normal;
	work->rctrl.flag &= ~(RADAR_RADIO);
	return;
    }

    /*目標物へ向く*/
    if (work->call_type == CALL_ENE_DOWN_BODY){
	Dir_from_2Vec(work->eye.eyepos,&work->eye.accident_pos,
		      &work->control.turn);
    }
    else {
	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
		      &work->control.turn);
    }
    if (work->control.turn.vx > 3000){
	work->control.turn.vx -= 4096;
    }


    // ****************アクシデントの場所******************
    {	
	TracePlayer3D(work, &work->eye.accident_pos);
    }

    work->rctrl.flag |= (RADAR_RADIO);
    work->discv_time ++;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActCall							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_call_control(Work *work){

    work->eye.trgpos = &GM_PlayerFindPos ;

    switch (G2_STEP)
    {
    case SUB_MOD_WARN:
	sub_call0(work);
	break;
	
    case SUB_MOD_ALERT:
	sub_call1(work);
	break;
    }

    //  通報キャンセル
}
