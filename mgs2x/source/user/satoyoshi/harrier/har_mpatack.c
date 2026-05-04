/********************************************************************************/
/*	har_mpatack.c								*/
/*	ハリアロケット攻撃管理 							*/
/*	2001/06/26 H.Satoyoshi							*/
/*	$Id: har_mpatack.c,v 1.1.1.3 2002/11/19 11:48:23 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

#if 0
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void getmove							*/
/*	引数:	Work *work   						       	*/
/*	説明:	動きを取得する				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void getmove(Work *work){
    
    // *****メニュー表示中は操作を受け付けない
    if (work->menu_flag & HAR_DEB_MENU_OPEN){
	return;
    }
    
    // ************************************プレイヤ操作時
    if (!(work->menu_flag&HAR_DEB_CONTROL_FLAG)){
	Har_DBPCPadMove(work);
    }

    // ************************************ハリア操作時
    else {
	Har_DBHarPadMove(work);

	if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
	    MENU_Locate( 20, 425, 0 );
	    MENU_SetColor( 180, 0, 0 );
	    MENU_Printf( "MV Har"); 
	}
    }
}
#endif
#if 0
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void apply_move						*/
/*	引数:	Work *work   						       	*/
/*	説明:	動きを適用する				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void apply_move(Work *work){

    Har_DBControlVTOL(work);

    //攻撃系
    if (PRESS_PAD_R1){	//ミサイルポッド
	GV_CallChildSignalFunc(work, HAR_SIGNAL_MPOD|HAR_SIGNAL_FIRE, 0);
    }
    
    if (PRESS_PAD_R2){	//クラスター
	GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_FIRE, 0);
    }
    
    if (PRESS_PAD_L1){	//機銃
	
	if (work->vul_fire_flg&GUN_FIRE_FLG_NORM){
	    SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	}
	else {
	    SET_GUN_MODE(GUN_FIRE_FLG_NORM, GUN_FIRE_FLG_HIT);
	}

	work->gun_time = 0;
	work->gun_time_first = 0;
    }
    if (PRESS_PAD_L2){	//アムラーム
	GV_CallChildSignalFunc(work, HAR_SIGNAL_AMRAM|HAR_SIGNAL_FIRE, 0);
    }
    
    if (PRESS_PAD_A){	//リロード
	GV_CallChildSignalFunc(work, HAR_SIGNAL_AMRAM|HAR_SIGNAL_EQUIP, 0);
	GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_EQUIP, 0);
    }


}




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetPosition()					*/
/*	引数:	Work	*work							*/
/*	説明:	ハリアの座標を返す						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int har_mpatack(Work* work){
    if (work->hokan_timer>0){	// *******補間移動しますよ
    }
    else {
	getmove(work);
	if (work->camera_ensyutu_flag == OFF){	//プロック呼び専用フラグ
	    har_call_event_proc(work, HAR_EVENT_RM_FIRE);
	    work->camera_ensyutu_flag = ON;
	}
    }
    
    apply_move(work);
    
    // ****終了チェック
    if (work->play_timer == HAR_MOVE_FRAMES){
	return 1;
    }

    return 0;
}
    
#endif











