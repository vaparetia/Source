/********************************************************************************/
/*	harp_nor.c								*/
/*	ハリアープログラム移動 ノーマル機銃攻撃					*/
/*	2001/06/15 H.Satoyoshi							*/
/*	$Id: harp_amram.c,v 1.1.1.3 2002/11/19 11:48:26 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_to_player						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	プレイヤーへ 次は避け						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void move_to_amram(Work *work, int dist){
    static char dmg_flag;
    static FVECTOR tmpvec;

    switch (HAR_MODE_L4){

    case 0:
	//	if (fabs(work->control.mov.vx) > 1100000.0f){
	if (dist < 900000.0f){
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_AMRAM|HAR_SIGNAL_EQUIP, 0);
	    work->max_speed = 3300.0f ;
	    work->rot_speed = 100;
	    work->goal_pos.vy = 2300.0f;
	    tmpvec = work->control.mov;

	    tmpvec.vx = (tmpvec.vx+GM_PlayerFindPos.vx*5.0f)/6.0f;
	    tmpvec.vy = (tmpvec.vy+GM_PlayerFindPos.vy*5.0f)/6.0f;
	    tmpvec.vz = (tmpvec.vz+GM_PlayerFindPos.vz*5.0f)/6.0f;

	    NewHarrierFlash(&tmpvec);

	    dmg_flag = OFF;	
	    HAR_MODE_L4 ++ ;

	    GM_JimakuSeSetMode (SD_V_SOLAT601, &work->control.mov, GM_SEMODE_BOMB);	// あの世に・
	    har_call_event_proc(work, HAR_EVENT_AMR_START);
	}
	break;

    case 1:
	if (dist < 860000){
	    work->max_speed = 2500.0f ;
	    set_targnzl_lock(work, ON);	//ノズルロック可能
	    HAR_MODE_L4 ++ ;
	}
	break;

    case 2:
	if (dist < 850000){
	    har_call_event_proc(work, HAR_EVENT_AMR_FIRE);
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_AMRAM|HAR_SIGNAL_FIRE, 0);
	    work->target_speed = 1800.0f ;
	    work->accel_speed = 60;
	    //	    work->max_speed = 3000.0f ;
	    HAR_MODE_L4 ++ ;
	}
	break;

    case 3:
	if (dist < 400000){
	    work->max_speed = 2300.0f ;
	    HAR_MODE_L4 ++ ;

	}
	break;

    case 4:
	// **************避けるよん
	if (fabs(work->goal_pos.vx - work->control.mov.vx) < 35000.0f){
	    if (work->gun_attack_millor_flg&MILLOR_X_FLAG){
		work->gun_attack_millor_flg &= ~MILLOR_X_FLAG;
	    }
	    else {
		work->gun_attack_millor_flg |= MILLOR_X_FLAG;
	    }
	    HAR_MODE_L3 = L3_ATCK_NORM_ESCAPE;
	    
	    work->rot_speed = 300;
	    work->target_speed = 1200.0f ;
	    work->accel_speed = 20;
	    

	    {
		FVECTOR	tmpvec = {0.0f, 0.0f, 50000.0f};
		FMATRIX	tmpmat;
		DG_SetPos(&work->body.objs->world);
		DG_MovePos(&tmpvec);
		DG_GetPos(&tmpmat);
		
		_sceVu0CopyVector(&work->goal_pos, (FVECTOR *)tmpmat.m[3]);
	    }
	    work->goal_pos.vy = 8000.0f;
	    work->gun_attack_last_dist = -1;
	    work->gun_attack_flag = OFF;
	    
	    // ターゲット元通り
	    set_targbodyall_small(work);
	    set_targbodyall_skip(work, OFF);
	}
	break;
    }

    work->goal_pos.vx = work->p_position.vx;

    // ****だめーじを受けていない
    if (work->damage_num != 0){	    
	work->goal_pos.vz = work->p_position.vz;
    }
    // ****だめーじを受けていたら 狙い位置がよれる
    else {
	if (work->p_position.vz > 0.0f){
	    work->goal_pos.vz = -3000.0f;
	}
	else {
	    work->goal_pos.vz = 3000.0f;
	}
	work->goal_pos.vy = 5000.0f;
    }
    work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mode_normal_up						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0	継続中						*/
/*			1	終了						*/
/*	説明:	上段機銃攻撃							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int atack_amram(Work *work)
{
    FVECTOR	sht;
    int		dist;
    int		flag = OFF;

    //	距離の計算
    SAT_Minus_FVECTOR(&sht, &work->goal_pos, &work->control.mov);
    dist = (int)_FVecLen3( &sht);
    
    // 近付いているかどうかのチェック
    if (work->gun_attack_last_dist == -1){
	work->gun_attack_last_dist = dist;
    }
    
    switch (HAR_MODE_L3){


    case L3_ATCK_NORM_INIT: 	    //MENU_Printf( "TO START");
	work->rot_speed = 140;
	work->target_speed = 5500.0f ;
	work->accel_speed = 90;
	
	work->goal_pos.vx = 1280000.0f*P_MOVE_X_MIL;
	work->goal_pos.vy = RAND(600)*1000.0f;
	work->goal_pos.vz = (RAND(475)*1000.0f)*P_MOVE_Z_MIL;

	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;
	work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;

	set_targnzl_lock(work, OFF);	//ノズルロック不可能
	set_hartarg_skip(work, ON);	//本体には当たり無し


	//	距離の計算	近距離アムラーム発射防止
	SAT_Minus_FVECTOR(&sht, &work->goal_pos, &work->control.mov);
	dist = (int)_FVecLen3( &sht);

	HAR_MODE_L3 = L3_ATCK_NORM_START;
	break;


	// ***********スタート地点に向かっている
    case L3_ATCK_NORM_START: 	    //MENU_Printf( "TO START");
	if (move_to_start(work, dist)){

	    HAR_MODE_L3 = L3_ATCK_NORM_PLAYER_UP;
	    HAR_MODE_L4 = 0;
	    work->damage_num = 0; // ダメージ回数をリセット
	    start_target(work, dist);
	}
	break;


	// ***********プレイヤに	橋の上
    case L3_ATCK_NORM_PLAYER_UP:	    //MENU_Printf( "TO PLAYER");	    
	move_to_amram(work, dist);
	work->wing_smoke_flg = SET_FLG_OFF;
	break;
	
	// ***********回避に向かっている
    case L3_ATCK_NORM_ESCAPE:	    //MENU_Printf( "TO ESCAPE");
	move_to_escape(work, dist);
	break;
	
	// ***********橋に向かっている
    case L3_ATCK_NORM_BRIDGE:	    //MENU_Printf( "TO BRIDGE");
	if ( ((work->control.mov.vz+W25A_STAGE_Z_SHIFT)*P_MOVE_Z_MIL-W25A_STAGE_Z_SHIFT)
		> (work->control.mov.vx*P_MOVE_X_MIL*2.0f/3.0f + 35000.0f - W25A_STAGE_Z_SHIFT)){
	    flag = ON;
	}
	break;
    }
    
    if (work->gun_attack_last_dist > dist){
	work->gun_attack_last_dist = dist;
	work->gun_attack_flag = ON;
    }
    if (flag == ON){
	select_root(work);
    }
    return	flag;
}


