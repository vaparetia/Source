/********************************************************************************/
/*	harp_nor.c								*/
/*	ハリアープログラム移動 ノーマル機銃攻撃					*/
/*	2001/06/15 H.Satoyoshi							*/
/*	$Id: harp_norm.c,v 1.1.1.3 2002/11/19 11:48:26 Yoshizawa1 Exp $		*/
/********************************************************************************/
static float   VULC_SPEED;


/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void init_vulca_game_level					*/
/*	引数:	Work	*work							*/
/*	説明:	機銃攻撃難易度調整						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void init_vulca_game_level(Work *work)
{
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	VULC_SPEED = 0.7f;
	break;
    case ST_LEV_EASY:
	VULC_SPEED = 0.8f;
	break;
    case ST_LEV_NORMAL:
	VULC_SPEED = 1.0f;
	break;

    case ST_LEV_UPNORM:
	VULC_SPEED = 1.25f;
	break;

    case GM_LEVEL_HARD:
	VULC_SPEED = 1.5f;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	VULC_SPEED = 1.9f;
	break;
    default:
	VULC_SPEED = 1.0f;
	break;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void start_target						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	スタート							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void start_target(Work *work, int dist){
    // ***極点での溜め時間
    work->rot_speed = 1500;
    work->max_speed = 3000.0f;
    work->goal_pos.vx = work->p_position.vx;	// 目標はプレイヤ
    work->goal_pos.vy = 2300.0f;
    work->goal_pos.vz = work->p_position.vz;
    
    SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
    work->gun_attack_last_dist = -1;
    work->gun_attack_flag = OFF;
    work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;    
    work->save_stinger_num = GM_WeaponNum(WP_Stinger);
    // フレア系ワーク初期か
    work->headmark_timer = 999;
    work->flare_shoot_num = 0;
    
    work->gun_atk_dmg_flag = OFF;
    
    // ターゲット巨大化
    set_targbodyall_big(work);
    set_hartarg_skip(work, OFF);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_to_escape						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	避けへ  次は橋							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void move_to_escape(Work *work, int dist){

    //    sat_viewp(&work->goal_pos, 250, 0, 0);

    // **************橋
    if ((work->gun_attack_last_dist < dist)&(work->gun_attack_flag == ON)){	
	work->rot_speed = 74;
	work->target_speed = 750.0f ;
	work->accel_speed = 50;
	work->goal_pos.vx = 70000.0f*P_MOVE_X_MIL;

	//  ****焼きやホヴァーにつなげる
	if (mode_list[work->mode_num] == H_MOD_L1_HOVER_GUN){
	    work->gun_attack_millor_flg |=  MILLOR_Z_FLAG;
	    printf ("		hover\n");
	}
	else if (mode_list[work->mode_num] == H_MOD_L1_BURNING){
	    work->gun_attack_millor_flg &= ~MILLOR_Z_FLAG;	    
	    printf ("		burn\n");
	}

	work->goal_pos.vz = 90000.0f*P_MOVE_Z_MIL;

	work->goal_pos.vy = 25000.0f;

	HAR_MODE_L3 = L3_ATCK_NORM_BRIDGE;    
	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;
	work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;
	
	work->headmark_timer = 0;
	work->flare_shoot_num = 0;
	
	set_targbodyall_skip(work, OFF);
	set_targbodyall_fat(work);

    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_to_start						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	スタート地点へ  次はプレイヤ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int move_to_start(Work *work, int dist){
    
    // **************プレイヤに向かって
    if ((work->gun_attack_last_dist < dist)&(work->gun_attack_flag == ON)){
	return ON;
    }
    return OFF;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_to_player						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	プレイヤーへ 次は避け						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void move_to_player(Work *work, int dist){
    static char dmg_flag;
    static FVECTOR tmpvec;

    //    sat_viewp(&work->goal_pos, 250, 0, 0);
    //    printf ("M:%d %f\n", HAR_MODE_L4, work->control.mov.vy);

    switch (HAR_MODE_L4){
	
    case 0:
	if (dist < 900000.0f){
	    if (work->control.mov.vx > 0.0f){
		work->goal_pos.vx =  3000.0f;
	    }
	    else {
		work->goal_pos.vx = -3000.0f;
	    }


	    work->max_speed = 3300.0f ;
	    work->rot_speed = 100;
	    work->goal_pos.vy = -10000.0f;
	    tmpvec = work->control.mov;
	    
	    tmpvec.vx = (tmpvec.vx+GM_PlayerFindPos.vx*5.0f)/6.0f;
	    tmpvec.vy = (tmpvec.vy+GM_PlayerFindPos.vy*5.0f)/6.0f;
	    tmpvec.vz = (tmpvec.vz+GM_PlayerFindPos.vz*5.0f)/6.0f;
	    
	    NewHarrierFlash(&tmpvec);
	    har_call_event_proc(work, HAR_EVENT_GUNATCK_KIRA);
	    dmg_flag = OFF;	
	    HAR_MODE_L4 ++ ;
	}
	break;
	
    case 1:
	if (dist < 830000){
	    work->max_speed = 3201.0f*VULC_SPEED;
	    set_targnzl_lock(work, ON);	//ノズルロック可能
	    set_targbodyall_big(work);
	    set_targbodyall_skip(work, OFF);
	    HAR_MODE_L4 ++ ;
	}
	break;
    case 2:
	if (dist < 780000){
	    if (GM_GameLevel>=GM_LEVEL_EXTREME){
		GV_CallChildSignalFunc(work, HAR_SIGNAL_MPOD|HAR_SIGNAL_FIRE, 0);
	    }
	    SET_GUN_MODE(GUN_FIRE_FLG_AIM, GUN_FIRE_FLG_HIT);
	    work->max_speed = 3150.0f*VULC_SPEED;
	    har_call_event_proc(work, HAR_EVENT_GUNATCK_FIRE);
	    work->goal_pos.vy = -7500.0f;
	    HAR_MODE_L4 ++ ;
	}
	break;
	
    case 3:
	if (dist < 300000){
	    work->max_speed = 2900.0f*VULC_SPEED;
	    HAR_MODE_L4 ++ ;
	}
	break;
	
    case 4:
	// **************避けるよん
	if (fabs(work->goal_pos.vx - work->control.mov.vx) < 23000.0f){
	    if (work->gun_attack_millor_flg&MILLOR_X_FLAG){
		work->gun_attack_millor_flg &= ~MILLOR_X_FLAG;
	    }
	    else {
		work->gun_attack_millor_flg |= MILLOR_X_FLAG;
	    }
	    HAR_MODE_L3 = L3_ATCK_NORM_ESCAPE;
	    
	    work->rot_speed = 300;
	    work->target_speed = 1100.0f ;
	    work->accel_speed = 20;
	    
	    if (work->vul_hit_flg & GUN_FIRE_FLG_HIT){	//通常撃ち
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_HIT);
	    }
	    else {
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	    }
	    
	    {
		FVECTOR	tmpvec = {0.0f, 0.0f, 50000.0f, 1.0f};
		FMATRIX	tmpmat;
		DG_SetPos(&work->body.objs->objs[0].world);
		DG_MovePos(&tmpvec);
		DG_GetPos(&tmpmat);
		
		work->goal_pos = *(FVECTOR*)tmpmat.m[3];
	    }
	    work->goal_pos.vy = -12000.0f;
	    work->gun_attack_last_dist = -1;
	    work->gun_attack_flag = OFF;
	    
	    // ターゲット元通り
	    set_targbodyall_small(work);
	    set_targbodyall_skip(work, OFF);
	}

	if (dist < 70000){
	    if (work->talewing_rot < 0.15f){	//水平尾翼
		work->talewing_rot = work->talewing_rot + 0.02f; 
	    }
	    if (work->r_flap_rot < 0.7f){
		work->r_flap_rot = work->r_flap_rot + 0.072f;//フラップ
	    }
	    if (work->cover_rot > -0.7f){
		work->cover_rot = work->cover_rot - 0.06f;//カバー
	    }
	}

	break;
    }
    
    // ****だめーじを受けていない
    if (dmg_flag == OFF){	    
	if (work->damage_num != 0){
	    
	    if ( (GM_GameLevel==GM_LEVEL_VERYEASY)||
		 (GM_GameLevel==ST_LEV_EASY)	  ||
		 (GM_GameLevel==ST_LEV_UPNORM)	  ||
		 (GM_GameLevel==ST_LEV_NORMAL) ){
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	    }
	    
	    dmg_flag = ON;	
	}
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
	work->goal_pos.vy = -8000.0f;
	set_targbodyall_small(work);
	set_targbodyall_skip(work, ON);
    }
    work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;
}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_to_player2						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	プレイヤーへ 次は避け						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void move_to_player2(Work *work, int dist){
    static char dmg_flag;
    static FVECTOR tmpvec;

    switch (HAR_MODE_L4){

    case 0:
	//	if (fabs(work->control.mov.vx) > 1100000.0f){
	if (dist < 900000.0f){
	    work->max_speed = 3300.0f ;
	    work->rot_speed = 100;
	    work->goal_pos.vy = 2300.0f;
	    tmpvec = work->control.mov;

	    tmpvec.vx = (tmpvec.vx+GM_PlayerFindPos.vx*5.0f)/6.0f;
	    tmpvec.vy = (tmpvec.vy+GM_PlayerFindPos.vy*5.0f)/6.0f;
	    tmpvec.vz = (tmpvec.vz+GM_PlayerFindPos.vz*5.0f)/6.0f;

	    NewHarrierFlash(&tmpvec);
	    har_call_event_proc(work, HAR_EVENT_GUNATCK_KIRA);
	    dmg_flag = OFF;	
	    HAR_MODE_L4 ++ ;
	}
	break;

    case 1:
	if (dist < 830000){
	    work->max_speed = 3201.0f*VULC_SPEED;
	    set_targnzl_lock(work, ON);	//ノズルロック可能
	    set_targbodyall_big(work);
	    set_targbodyall_skip(work, OFF);
	    HAR_MODE_L4 ++ ;
	}
	break;

    case 2:
	if (dist < 780000){
	    if (GM_GameLevel>=GM_LEVEL_EXTREME){
		GV_CallChildSignalFunc(work, HAR_SIGNAL_MPOD|HAR_SIGNAL_FIRE, 0);
	    }
	    SET_GUN_MODE(GUN_FIRE_FLG_AIM, GUN_FIRE_FLG_HIT);
	    work->max_speed = 3150.0f*VULC_SPEED;
	    har_call_event_proc(work, HAR_EVENT_GUNATCK_FIRE);

	    HAR_MODE_L4 ++ ;
	}
	break;

    case 3:
	if (dist < 300000){
	    work->max_speed = 2900.0f*VULC_SPEED;
	    HAR_MODE_L4 ++ ;
	}
	break;

    case 4:
	// **************避けるよん
	if (fabs(work->goal_pos.vx - work->control.mov.vx) < 23000.0f){
	    if (work->gun_attack_millor_flg&MILLOR_X_FLAG){
		work->gun_attack_millor_flg &= ~MILLOR_X_FLAG;
	    }
	    else {
		work->gun_attack_millor_flg |= MILLOR_X_FLAG;
	    }
	    HAR_MODE_L3 = L3_ATCK_NORM_ESCAPE;
	    
	    work->rot_speed = 300;
	    work->target_speed = 1100.0f ;
	    work->accel_speed = 20;
	    
	    if (work->vul_hit_flg & GUN_FIRE_FLG_HIT){	//通常撃ち
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_HIT);
	    }
	    else {
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	    }

	    {
		FVECTOR	tmpvec = {0.0f, 0.0f, 50000.0f};
		FMATRIX	tmpmat;
		DG_SetPos(&work->body.objs->world);
		DG_MovePos(&tmpvec);
		DG_GetPos(&tmpmat);
		
		_sceVu0CopyVector(&work->goal_pos, (FVECTOR *)tmpmat.m[3]);
	    }
	    work->goal_pos.vy = 12000.0f;
	    work->gun_attack_last_dist = -1;
	    work->gun_attack_flag = OFF;
	    
	    // ターゲット元通り
	    set_targbodyall_small(work);
	    set_targbodyall_skip(work, OFF);
	}

	if (dist < 70000){
	    if (work->talewing_rot < 0.15f){	//水平尾翼
		work->talewing_rot = work->talewing_rot + 0.02f; 
	    }
	    if (work->r_flap_rot < 0.7f){
		work->r_flap_rot = work->r_flap_rot + 0.072f;//フラップ
	    }
	    if (work->cover_rot > -0.7f){
		work->cover_rot = work->cover_rot - 0.06f;//カバー
	    }
	}

	break;
    }

    work->goal_pos.vx = work->p_position.vx;

    // ****だめーじを受けていない
    if (dmg_flag == OFF){	    
	if (work->damage_num != 0){

	    if ( (GM_GameLevel==GM_LEVEL_VERYEASY)||
		 (GM_GameLevel==ST_LEV_EASY)	  ||
		 (GM_GameLevel==ST_LEV_UPNORM)	  ||
		 (GM_GameLevel==ST_LEV_NORMAL) ){
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	    }

	    dmg_flag = ON;	
	    work->goal_pos.vy += 3000.0f;
	}
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

	set_targbodyall_small(work);
	set_targbodyall_skip(work, ON);
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
static int atack_normal_up(Work *work)
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
	work->max_speed = (float)_FVecLen3(&work->Speed);

	work->control.step = work->Speed;

	_sceVu0AddVector(&work->control.mov, &work->control.mov, &work->control.step);

	work->rot_speed = 140;
	work->target_speed = 5500.0f ;
	work->accel_speed = 90;
	
	work->goal_pos.vx = 1280000.0f*P_MOVE_X_MIL;
	work->goal_pos.vz = (RAND(475)*1000.0f)*P_MOVE_Z_MIL;

	{
	    int	tmppos;
	    switch( GM_GameLevel ){
	    case GM_LEVEL_VERYEASY:
		tmppos = RAND(401)+RAND(401)+RAND(401);
		if (tmppos>=600){
		    tmppos = tmppos-600;
		}
		else {
		    tmppos = 600-tmppos;
		}
		break;
	    case ST_LEV_EASY:
		tmppos = RAND(601)+RAND(601);
		if (tmppos>=600){
		    tmppos = tmppos-600;
		}
		else {
		    tmppos = 600-tmppos;
		}
		break;
	    case ST_LEV_NORMAL:
		tmppos = RAND(601);
		break;

	    case ST_LEV_UPNORM:
		tmppos = RAND(601);
		break;

	    case GM_LEVEL_HARD:
		tmppos = RAND(601)+RAND(601);
		if (tmppos>=600){
		    tmppos = 1200-tmppos;
		}
		else {
		    tmppos = tmppos;
		}
		break;
	    case GM_LEVEL_E_EXTREME:
	    case GM_LEVEL_EXTREME:
		tmppos = RAND(401)+RAND(401)+RAND(401);
		if (tmppos>=600){
		    tmppos = 1200-tmppos;
		}
		else {
		    tmppos = tmppos;
		}
		break;
	    default:
		tmppos = RAND(601);
		break;
	    }
	    work->goal_pos.vy = tmppos*1000.0f;
	}

	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;
	work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;

	set_targnzl_lock(work, OFF);	//ノズルロック不可能
	set_hartarg_skip(work, ON);	//本体には当たり無し
	HAR_MODE_L3 = L3_ATCK_NORM_START;
	init_vulca_game_level(work);	//ゲームレベルでスピード
	har_call_event_proc(work, HAR_EVENT_GUNATCK_START);
	break;

	// ***********スタート地点に向かっている
    case L3_ATCK_NORM_START: 	    //MENU_Printf( "TO START");
	if (move_to_start(work, dist)){
	    if (GM_PlayerFindPos.vy > -1000.0f){
		HAR_MODE_L3 = L3_ATCK_NORM_PLAYER_UP;
	    }
	    else {
		HAR_MODE_L3 = L3_ATCK_NORM_PLAYER_DOWN;
	    }
	    HAR_MODE_L4 = 0;
	    work->damage_num = 0;	// ダメージをリセット
	    start_target(work, dist);
	}
	break;


	// ***********プレイヤに	橋の上
    case L3_ATCK_NORM_PLAYER_UP:	    //MENU_Printf( "TO PLAYER");	    
	move_to_player2(work, dist);
	break;
	// ***********プレイヤに	橋の下
    case L3_ATCK_NORM_PLAYER_DOWN:	    //MENU_Printf( "TO PLAYER");	    
	move_to_player(work, dist);
	break;
	
	// ***********回避に向かっている
    case L3_ATCK_NORM_ESCAPE:	    //MENU_Printf( "TO ESCAPE");
	move_to_escape(work, dist);
	break;

	// ***********橋に向かっている
    case L3_ATCK_NORM_BRIDGE:	    //MENU_Printf( "TO BRIDGE");
	if (  ( ((work->control.mov.vz+W25A_STAGE_Z_SHIFT)*P_MOVE_Z_MIL-W25A_STAGE_Z_SHIFT)
		> (work->control.mov.vx*P_MOVE_X_MIL*2.0f/3.0f + 35000.0f - W25A_STAGE_Z_SHIFT) ) & (work->gun_attack_flag == ON)  ){
	    flag = ON;
	    har_call_event_proc(work, HAR_EVENT_GUNATCK_END);
	}
	break;
    }
    
    if (work->gun_attack_last_dist > dist){
	work->gun_attack_last_dist = dist;
	work->gun_attack_flag = ON;
    }
    if (flag == ON){
	select_root(work);

	work->talewing_rot = 0.0f; 
	work->r_flap_rot = 0.0f;//フラップ
	work->cover_rot = 0.0f;//カバー

    }
    return	flag;
}










