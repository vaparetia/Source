/********************************************************************************/
/*	harp_nor.c								*/
/*	ハリアープログラム移動 ノーマル機銃攻撃					*/
/*	2001/06/15 H.Satoyoshi							*/
/*	$Id: harp_clast.c,v 1.1.1.3 2002/11/19 11:48:26 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_cls_start						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	爆撃開始							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void move_cls_start(Work *work, int dist){

    // **************爆撃地点に向かって
    if ((work->gun_attack_last_dist < dist)&(work->gun_attack_flag == ON)){
	HAR_MODE_L3 = HAR_PMOVE_BOMB;

	    GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_EQUIP, 0);

	// ***極点での溜め時間
	work->rot_speed = 700;
	work->max_speed = 1300.0f;
	
	work->goal_pos.vx = 0;
	work->goal_pos.vy = 45000.0f;
	work->goal_pos.vz = -110000.0f;    

	
	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;

	// ターゲット巨大化
	set_targbodyall_big(work);
	set_hartarg_skip(work, OFF);

	//ぷろっくコール
	har_call_event_proc(work, HAR_EVENT_CLS_START );
	HAR_MODE_L3 = L3_ATCK_CLAST_BOMB;
    }
}




/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_cls_bomb						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	爆撃								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void move_cls_bomb(Work *work, int dist){
    // **************爆撃地点に向かって
    if ((dist<2000)&(work->gun_attack_flag == ON)){
	HAR_MODE_L3 = L3_ATCK_CLAST_END;

	    GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_FIRE, 0);
	    har_call_event_proc(work, HAR_EVENT_CLS_BOMB );

	// ***極点での溜め時間
	work->rot_speed = 400;
	work->max_speed = 1300.0f;
	
	work->goal_pos.vx = -200000.0f;
	work->goal_pos.vy = 45000.0f;
	work->goal_pos.vz = -500000.0f;
	
	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;

	work->se_time = 0;	//ステージ破壊タイミング
	work->claster_num++;
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_cls_end						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	爆撃								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int move_cls_end(Work *work, int dist){
    if(work->claster_num == 1){	//クラスター初回
	work->se_time += 5;
	if ((7000>work->se_time)&&( work->se_time > 900)){
	    if ((work->se_time%1000)>950){
		work->se_time += 1000;
		work->se_time -= 45;
		Har_StageChange( work );
	    }
	}
    }
    if ((work->gun_attack_last_dist < dist)&(work->gun_attack_flag == ON)){
	har_call_event_proc(work, HAR_EVENT_CLS_END );
	return ON;
    }    
	return OFF;
}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mode_claster						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0	継続中						*/
/*			1	終了						*/
/*	説明:	クラスター爆弾攻撃						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int atack_claster(Work *work)
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

    case L3_ATCK_CLAST_INIT:
	// ***極点での溜め時間
	work->rot_speed = 400;
	work->max_speed = 2300.0f;
	
	work->goal_pos.vx = 200000.0f;	// 目標はプレイヤ
	work->goal_pos.vy = 45000.0f;
	work->goal_pos.vz = 500000.0f;
	
	work->gun_attack_last_dist = -1;
	work->gun_attack_flag = OFF;
	HAR_MODE_L3 = L3_ATCK_CLAST_START;
	break;
	
    case L3_ATCK_CLAST_START:
	move_cls_start(work, dist);
	break;

    case L3_ATCK_CLAST_BOMB:
	move_cls_bomb(work, dist);
	break;

    case L3_ATCK_CLAST_END:
	if (move_cls_end(work, dist) == ON){
	    flag = ON;
	}
	break;
    }

    if (work->gun_attack_last_dist > dist){
	work->gun_attack_last_dist = dist;
	work->gun_attack_flag = ON;
    }
    return	flag;
}



