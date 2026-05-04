/********************************************************************************/
/*	Sv_camera.c								*/
/*	監視カメラ  *NewSvCamera						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cam_check.c,v 1.1.1.3 2002/11/19 11:48:13 Yoshizawa1 Exp $		*/
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

extern int		GM_DestroyNikita( void );

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Chaff_Check		** EYE_LAYER_CHAFF **		*/
/*	引数:	Work	*work							*/
/*	説明:	チャフチェック							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Chaff_Check(Work *work){
    
    if(GM_CheckGameStatus(STATE_CHAFF)){	// チャフチェック
	if (work->ACTION_LAYER > EYE_LAYER_CHAFF){
	    SV_G1_STEP(work, MOD_CHAFF);
	    work->ACTION_LAYER = EYE_LAYER_CHAFF;
	    return 1;
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Alert_Check		** EYE_LAYER_ALERT **		*/
/*	引数:	Work	*work							*/
/*	説明:	危険モードチェック						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Emma_Alert_Check(Work *work){
 
    if ( CHECK_PC_FLG(EYECK_PC_EMMA_IN_SIGHT) &&
	 !CHECK_PC_FLG(EYECK_PC_IN_SIGHT) ){
	   if (work->ACTION_LAYER > EYE_LAYER_EMMA_ALERT){
	       SV_G1_STEP(work, MOD_EMMA_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_EMMA_ALERT;
	       return 1;
	   }
    }

    return 0;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int CYP_Evasion_Check		**  **		*/
/*	引数:	Work	*work							*/
/*	説明:							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Evasion_Check(Work *work){
    if(GM_AlertMode==ALERT_MODE_AVOID){   
	if (work->ACTION_LAYER > EYE_LAYER_EVASION){
	    SV_G1_STEP(work, MOD_EVASION);
	    work->ACTION_LAYER = EYE_LAYER_EVASION;
	    return 1;
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Alert_Check		** EYE_LAYER_ALERT **		*/
/*	引数:	Work	*work							*/
/*	説明:	危険モードチェック						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Alert_Check(Work *work){
    
    if ( CHECK_PC_FLG(EYECK_PC_IN_SIGHT) &&
	 !CHECK_PC_FLG(EYECK_PC_IN_UNIFORM) ){
	   if (work->ACTION_LAYER > EYE_LAYER_PC_ALERT){
	       SV_G1_STEP(work, MOD_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_PC_ALERT;
	       return 1;
	   }
    }
    if ( GM_AlertMode == ALERT_MODE_ALERT ){
	   if (work->ACTION_LAYER > EYE_LAYER_PC_ALERT){
	       SV_G1_STEP(work, MOD_ALERT);
	       SV_G2_STEP(work, SUB_MOD_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_PC_ALERT;
	       return 1;
	   }
    }

    return 0;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Call_Check		** EYE_LAYER_CALL_ENEMY **		*/
/*	引数:	Work	*work							*/
/*	説明:	呼びチェック							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Call_Check(Work *work){

    if (work->camera_type == GUN){
	return 0;
    }
    
    if (	 CHECK_PC_FLG(EYECK_PC_IN_SIGHT) &&
		 CHECK_PC_FLG(EYECK_PC_IN_UNIFORM) &&
		 CHECK_PC_FLG(EYECK_PC_PLONE) ){
	if ( !COM_AccidentToID(work->com_uniq_id) && 
	     (work->ACTION_LAYER > EYE_LAYER_CALL_ENEMY) ){
	    SV_G1_STEP(work, MOD_CALL);
	    work->ACTION_LAYER = EYE_LAYER_CALL_ENEMY;
	    work->call_type = CALL_PC_MISTAKEN_DAMAGED;
	    return 1;
	}
    }
    
    if ( !COM_AccidentToID(work->eye.accident_uniq_id) && 
	 (work->eye.accident_uniq_id != 0) &&
	 ( CHECK_ENE_FLG(EYECK_ENE_SLEEPING) || 
	   CHECK_ENE_FLG(EYECK_ENE_FAINT) || 
	   CHECK_ENE_FLG(EYECK_ENE_DEAD) ) )
    {
	if (work->ACTION_LAYER > EYE_LAYER_CALL_ENEMY){

printf (" ######################################################  呼びます : %d\n",work->eye.accident_uniq_id);

	    SV_G1_STEP(work, MOD_CALL);
	    work->ACTION_LAYER = EYE_LAYER_CALL_ENEMY;
	    work->call_type = CALL_ENE_DOWN_BODY;
	    work->call_id = work->eye.accident_uniq_id;
	    return 1;
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Nikita_Check		** EYE_LAYER_CALL_ENEMY **	*/
/*	引数:	Work	*work							*/
/*	説明:	ニキータチェック						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Nikita_Check(Work *work){

    if (work->camera_type != GUN){
	return 0;
    }
    
    if (CHECK_PC_FLG(EYECK_NIKITA_IN_SIGHT)){
	if (work->ACTION_LAYER > EYE_LAYER_CALL_ENEMY){
	    SV_G1_STEP(work, MOD_NIKITA_SHOOT);
	    work->ACTION_LAYER = EYE_LAYER_CALL_ENEMY;
	    red_bikkuri_set(work);
	    return 1;
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Break_Check		** EYE_LAYER_BREAK **		*/
/*	引数:	Work	*work							*/
/*	説明:	壊れチェック							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int SV_Break_Check(Work *work){
    
    if (work->SV_Break_Flag == 0){
	return 0;
    }
    if (work->ACTION_LAYER > EYE_LAYER_BREAK){
	
	work->ACTION_LAYER = EYE_LAYER_BREAK;	
	if (work->SV_Break_Flag == SV_BREAK_CRUSH){
	    CameraCrush(work);
	    return 1;
	}
	if (work->SV_Break_Flag == SV_BREAK_VANISH){
	    CameraVanish(work);
	    return 1;
	}
    }
    else {
	return 0;
    }
    printf ("Satoyoshi SV_CAMERA SV_Break_Flag Error\n");
    ASSERT(0);
    return 0;
}




#define CHECK_FUNC_NUM	(8)

static SV_FUNCLIST Check_Funk[CHECK_FUNC_NUM]= 
{
    SV_Break_Check,
    SV_Chaff_Check,
    SV_Alert_Check,
    SV_Emma_Alert_Check,
    SV_Evasion_Check,
    SV_Call_Check,
    SV_Nikita_Check,
    SV_Wbk_Check,
};

