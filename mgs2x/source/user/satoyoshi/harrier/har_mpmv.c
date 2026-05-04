/********************************************************************************/
/*	Har_main.c								*/
/*	ハリアメイン *NewHarrier						*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_mpmv.c,v 1.1.1.3 2002/11/19 11:48:23 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/
#if 0

static int control_mpod(Work* work){
    
    
    FVECTOR	sht;
    int		dist;
    
    //	距離の計算
    SAT_Minus_FVECTOR(&sht, &work->goal_pos, &work->control.mov);
    dist = (int)_FVecLen3( &sht);

    // 近付いているかどうかのチェック
    if (work->gun_attack_last_dist == -1){
	work->gun_attack_last_dist = dist;
    }
	

    
    switch (HAR_MODE_L2){
	
    case 0:
    {
	Har_Move2Goal(work);
	work->rot_speed = 100;
	work->target_speed = 1400.0f ;
	work->accel_speed = 45;
	
	//	if (RAND(2)){	右側浮上攻撃のみ
	if (1){
	    work->goal_pos.vx = 20000.0f;
	}
	else {
	    work->goal_pos.vx = -20000.0f;
	}
	work->goal_pos.vy = -30000.0f;
	//		work->goal_pos.vz = -W25A_STAGE_Z_SHIFT;
	work->goal_pos.vz = GM_PlayerFindPos.vz;
	
	HAR_MODE_L3 = HAR_PMOVE_CHANGE;
	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;
	
	set_targnzl_lock(work, ON);	//ノズルロック可能
	
	HAR_MODE_L2 ++;
    }
    break;
    
    
    // ***********モード変更地点に向かっている
    case 1:
	if (work->p_position.vy > -1000.0f){
	    if (work->control.mov.vx < 0.0f){
		work->goal_pos.vz = ((HAR_MOVE*)act20)->begin_pos.vz;
	    } 
	    else {
		work->goal_pos.vz = ((HAR_MOVE*)act10)->begin_pos.vz;

#if 0
		work->goal_pos.vz = -150000.0f - (1.0f*(work->goal_pos.vz+150000.0f));
#endif

	    } 
	}
	else {
	    if (work->control.mov.vx < 0.0f){
		work->goal_pos.vz = ((HAR_MOVE*)act21)->begin_pos.vz;
	    } 
	    else {
		work->goal_pos.vz = ((HAR_MOVE*)act11)->begin_pos.vz;
	    } 
	}
	//	act_move->begin_pos


	Har_Move2Goal(work);
	if ( dist < 100000){
	    work->target_speed = 600.0f ;
	    work->accel_speed = 30;
	    HAR_MODE_L2 ++;

	}
	break;

	// ***********	アクション再生モードに
    case 2:
	Har_Move2Goal(work);
	if ( dist < 2000){
	    set_targnzl_lock(work, ON);		//ノズルロック可能
	    set_targbodyall_small(work);	//全体は小さく
	    set_targbodyall_skip(work, ON);	//当たり無し
	    set_hartarg_skip(work, OFF);	//本体には当たりあり
	    HAR_MODE_L2 ++;
	    {
		if (work->p_position.vy > -1000.0f){
		    if (work->control.mov.vx < 0.0f){
			RUN_ACTION (work, act20);
			work->camera_ensyutu_flag = OFF;
			//	    printf ("act20 1\n");
		    } 
		    else {
			RUN_ACTION (work, act10);
			work->camera_ensyutu_flag = OFF;
			//	    printf ("act10 1\n");
		    } 
		}
		else {
		    if (work->control.mov.vx < 0.0f){
			RUN_ACTION (work, act21);
			work->camera_ensyutu_flag = OFF;
		    } 
		    else {
			RUN_ACTION (work, act11);
			work->camera_ensyutu_flag = OFF;
		    } 
		}
	    }
	    
	}
	break;


    case 3:
	//			**********終了

#if 0
	work->control.mov.vz = -150000.0f - (1.0f*(work->control.mov.vz+150000.0f));
	work->control.turn.vy = -1024 - (work->control.turn.vy+1024);
	work->control.rot.vy = -1024 - (work->control.rot.vy+1024);
	work->control.turn.vz = -1 * work->control.turn.vz;
	work->control.rot.vz = -1 * work->control.rot.vz;
	work->control.step.vz = -1.0f * work->control.step.vz;
#endif

	if (har_mpatack(work)){
	    work->damage_num=0;	// ダメージ受けた回数
	    HAR_MODE_L2 ++;
	}
#if 0
	work->control.mov.vz = -150000.0f - (1.0f*(work->control.mov.vz+150000.0f));
	work->control.turn.vy = -1024 - (work->control.turn.vy+1024);
	work->control.rot.vy = -1024 - (work->control.rot.vy+1024);
	work->control.turn.vz = -1 * work->control.turn.vz;
	work->control.rot.vz = -1 * work->control.rot.vz;
	work->control.step.vz = -1.0f * work->control.step.vz;
#endif
	break;



	
    case 4:
	if (work->har_act_suki<340){
	    work->har_act_suki++;
	    if (work->damage_num!=0){
		work->har_act_suki+=60;
		work->damage_num = 0;
	    }
	    if (work->control.mov.vy > -2000.0f ){
		work->control.mov.vy += 10.0f;
	    }
	    else {
		if (work->control.mov.vy > -27000.0f ){	//海突っ込み防止
		    work->control.mov.vy -= 20.0f;
		}
	    }
	}
	else {
	    har_call_event_proc(work, HAR_EVENT_RM_END);
	    HAR_MODE_L2 ++;

	    work->rot_speed = 600;
	    work->target_speed = 1000.0f ;
	    work->accel_speed = 40;
	    
	    {
		FVECTOR	tmpvec = {0.0f, 0.0f, 50000.0f};
		FMATRIX	tmpmat;
		DG_SetPos(&work->body.objs->world);
		DG_MovePos(&tmpvec);
		DG_GetPos(&tmpmat);
		
		_sceVu0CopyVector(&work->goal_pos, (FVECTOR *)tmpmat.m[3]);
	    }
	    if (work->control.mov.vy > 0){
		work->goal_pos.vy = 12000.0f;
	    }
	    else {
		work->goal_pos.vy = - 12000.0f;
	    }
	    
	    work->gun_attack_last_dist = -1;
	    work->gun_attack_flag = OFF;
	    
	    // ターゲット元通り
	    set_targbodyall_small(work);
	    set_targbodyall_skip(work, OFF);
	    work->har_act_suki = 0;
	}  break;
	
	
	
	
	// ***********回避に向かっている
    case 5:
	Har_Move2Goal(work);
	if ((work->gun_attack_last_dist < dist)&(work->gun_attack_flag == ON)){


    // **************橋

	
	work->rot_speed = 74;
	work->target_speed = 1000.0f ;
	work->accel_speed = 55;
	
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

	work->goal_pos.vy = 40000.0f;

	HAR_MODE_L2 ++;	    
	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;
	work->goal_pos.vz -= W25A_STAGE_Z_SHIFT;
	
	work->headmark_timer = 0;
	work->flare_shoot_num = 0;
	
	set_targbodyall_skip(work, OFF);
	set_targbodyall_fat(work);

	}
	break;


	// ***********橋に向かっている
    case 6:
	Har_Move2Goal(work);
	if (  ( ((work->control.mov.vz+W25A_STAGE_Z_SHIFT)*P_MOVE_Z_MIL-W25A_STAGE_Z_SHIFT)
		> (work->control.mov.vx*P_MOVE_X_MIL*2.0f/3.0f + 35000.0f - W25A_STAGE_Z_SHIFT) ) & (work->gun_attack_flag == ON)  ){
	    return 1;
	}
	break;
    }

    
    if (work->gun_attack_last_dist > dist){
	work->gun_attack_last_dist = dist;
	work->gun_attack_flag = ON;
    }
    return 0;
}

#endif













