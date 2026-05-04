/********************************************************************************/
/*	har_mphov.c								*/
/*	ハリアホヴァー攻撃							*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id:									*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/

#define ZTURN	(work->zturn_sp)

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void hover_mpfire						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	ホバーミサイル							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int hover_mpfire(Work *work){	

    float	YTURNSP;
    float	YTURNBRK;
    float	Z_MV_BRK;
    float	Z_MV_AC;
    int		hv_ct, hv_pc; //狙い場所の重みつけ

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	YTURNSP=0.28f;
	YTURNBRK=0.93f;
	Z_MV_BRK=0.93f;
	Z_MV_AC=0.4f;
	hv_ct = 3;
	hv_pc = 2;
	break;
    case ST_LEV_EASY:
	YTURNSP=0.29f;
	YTURNBRK=0.94f;
	Z_MV_BRK=0.92f;
	Z_MV_AC=0.45f;
	hv_ct = 4;
	hv_pc = 3;
	break;
    case ST_LEV_NORMAL:
	YTURNSP	=0.3f;
	YTURNBRK=0.95f;
	Z_MV_BRK=0.90f;
	Z_MV_AC=0.5f;
	hv_ct = 1;
	hv_pc = 1;
	break;

    case ST_LEV_UPNORM:
	YTURNSP	=0.37f;
	YTURNBRK=0.92f;
	Z_MV_BRK=0.87f;
	Z_MV_AC=0.55f;
	hv_ct = 2;
	hv_pc = 2;
	break;

    case GM_LEVEL_HARD:
	YTURNSP=0.45f;
	YTURNBRK=0.90f;
	Z_MV_BRK=0.85f;
	Z_MV_AC=0.6f;
	hv_ct = 3;
	hv_pc = 4;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	YTURNSP=0.6f;
	YTURNBRK=0.85f;
	Z_MV_BRK=0.80f;
	Z_MV_AC=0.7f;
	hv_ct = 2;
	hv_pc = 3;
	break;
    default:
	YTURNSP	=0.3f;
	YTURNBRK=0.95f;
	Z_MV_BRK=0.94f;
	Z_MV_AC=0.5f;
	hv_ct = 1;
	hv_pc = 1;
	break;
    }

    if (work->l3_timer == 0){
	GV_CallChildSignalFunc(work, HAR_SIGNAL_MPOD|HAR_SIGNAL_FIRE, 0);
    }
    work->l3_timer += TIME_BASE;

    work->control.step.vz = -work->control.rot.vz/1.6f;

    work->control.turn.vx = 230;
    {	
	float posp_z = GM_PlayerFindPos.vz;


	// ====================modified 0928
	if ( (GM_GameLevel >= GM_LEVEL_EXTREME) || (GM_GameLevel == GM_LEVEL_EXTREME) ){
	    posp_z = -300000.0f-GM_PlayerFindPos.vz;
	    hv_ct = 0;
	    hv_pc = 1;
	}


	
	if (posp_z < -154000.0f){
	    posp_z = -154000.0f;
	}
	if (posp_z > -146000.0f){
	    posp_z = -146000.0f;
	}

	if (posp_z - work->control.mov.vz > 100.0f){
	    if (ZTURN > 0.0f){
		work->control.turn.vz = work->control.turn.vz*Z_MV_BRK;
	    }
	    ZTURN -= Z_MV_AC;

	    if (work->l_elr_rot < 0.30f){	//エルロン
		work->l_elr_rot = work->l_elr_rot + 0.03f;
		work->r_elr_rot = work->r_elr_rot + 0.03f;
	    }

	    
	}
	else if (posp_z - work->control.mov.vz < -100.0f){
	    if (ZTURN < 0.0f){
		work->control.turn.vz = work->control.turn.vz*Z_MV_BRK;
	    }
	    ZTURN += Z_MV_AC;
	    
	    
	    if (work->l_elr_rot > -0.30f){	//エルロン
		work->l_elr_rot = work->l_elr_rot - 0.03f;
		work->r_elr_rot = work->r_elr_rot - 0.03f;
	    }

	    
	}
	else {
	    ZTURN = ZTURN*0.95;
	    
	    work->l_elr_rot = work->l_elr_rot * 0.95f;
	    work->r_elr_rot = work->r_elr_rot * 0.95f;
	    
	}
	work->control.turn.vz = work->control.turn.vz*0.94;


	if (abs(work->control.turn.vz)>400){
	    work->control.turn.vz = work->control.turn.vz*0.9;
	}


	if ((ZTURN > 0.0f)&&(work->control.turn.vz<0)){
	    work->control.turn.vz = work->control.turn.vz*0.94;
	}
	if ((ZTURN < 0.0f)&&(work->control.turn.vz>0)){
	    work->control.turn.vz = work->control.turn.vz*0.94;
	}
	work->control.turn.vz += ZTURN;
    }


    {	//=======================プレイヤーの移動に追従して狙う
	float aim_mov_vx_u, aim_mov_vx_d;
	float aim_mov_vy_u, aim_mov_vy_d;
	int   aim_turn_vx_u, aim_turn_vx_d;
	int   pc_hight = (int)GM_PlayerFindPos.vy+2000;
	float   ue_box_hosei = 0.0f;

	if (GM_PlayerFindPos.vy > 600.0f){	//=========淵にいったら上に補正
	    ue_box_hosei = (GM_PlayerFindPos.vy-600.0f)*2.7f;
	}

	if (GM_PlayerFindPos.vz < -160000.0f){	//=========淵にいったらあくまでねらう
	    hv_ct = 0;
	    hv_pc = 1;
	}

	if (pc_hight>1000){
	    pc_hight=1000;
	}
	if (pc_hight<0){
	    pc_hight=0;
	}
	
	aim_mov_vx_u = 16000.0f-ue_box_hosei;
	aim_mov_vy_u = ue_box_hosei + 6000.0f - 1800.0f*(GM_PlayerFindPos.vx+2750.0f)/5500.0f;
	aim_turn_vx_u = 230;

	aim_mov_vx_d = 18000.0f;	//下後ろいっぱい
	aim_mov_vy_d = 300.0f + 1700.0f*(GM_PlayerFindPos.vx+2750.0f)/5500.0f;
	aim_turn_vx_d = 130 + 90*(GM_PlayerFindPos.vx+2750.0f)/5500.0f;

	work->control.mov.vx = ((pc_hight*aim_mov_vx_u)+((1000-pc_hight)*aim_mov_vx_d))/1000.0f;
	{
	    float aimy = ((pc_hight*aim_mov_vy_u)+((1000-pc_hight)*aim_mov_vy_d))/1000.0f;
	    if (aimy > work->control.mov.vy){
		if (aimy > work->control.mov.vy+60.0f){
		    work->control.mov.vy += 60.0f;
		}
		else {
		    work->control.mov.vy = aimy;
		}


		if (work->talewing_rot > -0.10f){	//水平尾翼
		    work->talewing_rot = work->talewing_rot - 0.02f; 
		}

		if (work->r_flap_rot < 0.7f){
		    work->r_flap_rot = work->r_flap_rot + 0.02f;//フラップ
		}

	    }
	    else if (aimy < work->control.mov.vy){
		if (aimy < work->control.mov.vy-60.0f){
		    work->control.mov.vy -= 60.0f;
		}
		else {
		    work->control.mov.vy = aimy;
		}


		if (work->talewing_rot < -0.10f){	//水平尾翼
		    work->talewing_rot = work->talewing_rot + 0.02f; 
		}

		if (work->r_flap_rot > 0.10f){
		    work->r_flap_rot = work->r_flap_rot - 0.02f;//フラップ
		}


	    }
	    else {
		work->talewing_rot = work->talewing_rot * 0.96f;
		work->r_flap_rot = (work->r_flap_rot-0.4f)*0.96f+0.4f;
	    }

	}
	work->control.turn.vx = (int)(((pc_hight*aim_turn_vx_u)+((1000-pc_hight)*aim_turn_vx_d))/1000.0f);


	// ====================modified 0928
	if ( (GM_GameLevel >= GM_LEVEL_EXTREME) || (GM_GameLevel == GM_LEVEL_EXTREME) ){
	    work->control.turn.vx -= 20*fabs( (GM_PlayerFindPos.vz-work->control.mov.vz)/6000.0f );
	}

	//	printf ("PH: %d\n",pc_hight);
    }



    if (1){
	FVECTOR	tmppos;
	SVECTOR	aimrot;
	int sabun;
	static float yturn_sp = 0.0f;

	tmppos = GM_PlayerFindPos;
	tmppos.vz = ((work->control.mov.vz*hv_ct)+(GM_PlayerFindPos.vz*hv_pc))/(1.0f*hv_ct+hv_pc);
	Dir_from_2Vec(&work->control.mov, &tmppos, &aimrot);

	sabun = aimrot.vy - work->control.turn.vy;

	yturn_sp = yturn_sp*0.98;	
	if ( ((sabun > 5)&&(sabun < 2048)) || (sabun <= -2048) ){
	    yturn_sp += YTURNSP;

	    if (work->lader_rot > -0.29f){	//垂直尾翼
		work->lader_rot = work->lader_rot - 0.02f; 
	    }

	}
	else if ( ((sabun<-5)&&(sabun>-2048)) || (sabun >= 2048) ){
	    yturn_sp -= YTURNSP;

	    if (work->lader_rot < 0.29f){	//垂直尾翼
		work->lader_rot = work->lader_rot + 0.02f; 
	    }

	}
	else {
	    yturn_sp = yturn_sp*YTURNBRK;

	    work->lader_rot = work->lader_rot * 0.95f; 	//垂直尾翼

	}
	work->control.turn.vy += (int)yturn_sp;

	if (work->control.turn.vy<0){
	    work->control.turn.vy += 4096;
	}
	work->control.turn.vy = work->control.turn.vy%4096;
    }


    if (work->l3_timer > 600*5){
	return 1;
    }
    else {
	return 0;
    }

}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void suki_act							*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	隙の挙動							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int suki_act(Work *work){	

    float	YTURNSP;
    float	YTURNBRK;
    float	Z_MV_BRK;
    float	Z_MV_AC;

    YTURNSP=0.3f;
    YTURNBRK=0.95f;
    Z_MV_BRK=0.80f;
    Z_MV_AC=0.7f;


    if (work->r_flap_rot < 0.7f){
	work->r_flap_rot = work->r_flap_rot + 0.02f;//フラップ
    }
    work->lader_rot = work->lader_rot * 0.95f; 	//垂直尾翼
    work->talewing_rot = work->talewing_rot * 0.96f; //水平尾翼





    work->l3_timer += TIME_BASE;

    work->control.step.vz = -work->control.rot.vz/1.7f;
    work->control.mov.vy -= abs(work->control.rot.vz)/2.7f;

    {	
	float posp_z = -150000.0f;

	if (posp_z - work->control.mov.vz > 800.0f){
	    if (ZTURN > 0.0f){
		work->control.turn.vz = work->control.turn.vz*Z_MV_BRK;
	    }
	    ZTURN -= Z_MV_AC;
	    if (work->l_elr_rot < 0.27f){	//エルロン
		work->l_elr_rot = work->l_elr_rot + 0.02f;
		work->r_elr_rot = work->r_elr_rot + 0.02f;
	    }

	}
	else if (posp_z - work->control.mov.vz < -800.0f){
	    if (ZTURN < 0.0f){
		work->control.turn.vz = work->control.turn.vz*Z_MV_BRK;
	    }
	    ZTURN += Z_MV_AC;
	    if (work->l_elr_rot > -0.27f){	//エルロン
		work->l_elr_rot = work->l_elr_rot - 0.02f;
		work->r_elr_rot = work->r_elr_rot - 0.02f;
	    }
	}
	else {
	    ZTURN = ZTURN*0.80;


	    work->l_elr_rot = work->l_elr_rot * 0.95f;
	    work->r_elr_rot = work->r_elr_rot * 0.95f;

	}
	work->control.turn.vz = work->control.turn.vz*0.90;
	if (abs(work->control.turn.vz)>400){
	    work->control.turn.vz = work->control.turn.vz*0.8;
	}


	if ((ZTURN > 0.0f)&&(work->control.turn.vz<0)){
	    work->control.turn.vz = work->control.turn.vz*0.90;
	}
	if ((ZTURN < 0.0f)&&(work->control.turn.vz>0)){
	    work->control.turn.vz = work->control.turn.vz*0.90;
	}
	work->control.turn.vz += ZTURN;
    }


    {	//=======================プレイヤーの移動に追従して狙う
	float aimy = 6700.0f;
	if (aimy > work->control.mov.vy){
	    if (aimy > work->control.mov.vy+30.0f){
		work->control.mov.vy += 30.0f;
	    }
	}
	if (aimy < work->control.mov.vy-1000.0f){
	    if (aimy < work->control.mov.vy-30.0f){
		work->control.mov.vy -= 30.0f;
	    }
	}
	work->control.mov.vy += 7.0f;

	work->control.turn.vy = work->control.turn.vy%4096;
	if (work->control.turn.vy>1024){
	    work->control.turn.vy -= 4096;
	}
	if (work->control.turn.vy<-3072){
	    work->control.turn.vy += 4096;
	}
	work->control.turn.vy = -1024+((work->control.turn.vy+1024)*0.98f);
	work->control.turn.vx = work->control.turn.vx*0.99f;
    }	

    {
	int	MAX_TIMER;

	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    MAX_TIMER = 580*5;
	    break;
	case ST_LEV_EASY:
	    MAX_TIMER = 520*5;
	    break;
	case ST_LEV_NORMAL:
	    MAX_TIMER = 440*5;
	    break;

	case ST_LEV_UPNORM:
	    MAX_TIMER = 390*5;
	    break;

	case GM_LEVEL_HARD:
	    MAX_TIMER = 340*5;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    MAX_TIMER = 300*5;
	    break;
	default:
	    MAX_TIMER = 480*5;
	    break;
	}
	
	if ( (work->l3_timer > MAX_TIMER)&&(work->control.mov.vy>6400.0f) ){
	    return 1;
	}
	else {
	    return 0;
	}
    }
}



/*
y = 6000.0f;
posz = -160000.0f;
posz = -140000.0f;
work->control.turn.vx = 230;
work->control.turn.vy = -1024;
work->control.turn.vz = 0;
 */


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void init_mphov							*/
/*	引数:	Work	*work							*/
/*	説明:	ホヴァー							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int init_mphov(Work* work){
    static float posz;

    FVECTOR P3 = {16000.0f, 6000.0f, 0.0f, 1.0f};

    FVECTOR S3 = {0.0f, 0.0f, 0.0f, 1.0f};
    int time3 = 200;
    static char se_flag;

    P3.vz = posz;

    work->l3_timer += TIME_BASE;


    switch(HAR_MODE_L3){
    case 0:
	posz = GM_PlayerFindPos.vz+(RAND(2)*3000.0f)-1500.0f;
	if (posz < -156000.0f)
	    posz = -156000.0f;
	if (posz > -144000.0f)
	    posz = -144000.0f;

	work->l3_timer = 0;
	har_call_event_proc(work, HAR_EVENT_RM_START);

	//	GM_SeSetMode (SD_E_H_HOVUP1, &work->control.mov, GM_SEMODE_BOMB);

	se_flag = OFF;

	_sceVu0CopyVector(&work->control.step, &DG_ZeroVector);

	HAR_MODE_L3++;
		
    case 1:

	if (work->l3_timer > time3 * 5){
	    work->l3_timer = time3 * 5;
	}
	get_hermove(&work->control.mov, &work->her_fromp, &P3,
		    &work->her_froms, &S3, work->l3_timer*1.0f, time3*5.0f);
	
	if (work->l3_timer == time3 * 5){
	    work->l3_timer = 0;
	    HAR_MODE_L3++;
	}

	if ((work->control.mov.vy>-8000.0f)&&(se_flag == OFF)){
	    GM_SeSetMode (SD_E_H_SWIDM3, &work->control.mov, GM_SEMODE_BOMB);
	    se_flag = ON;
	}
	break;

    case 2:
	return 1;
	break;
    }

    {
	SVECTOR aimturn;
	Dir_from_2Vec(&work->control.mov, &GM_PlayerFindPos, &aimturn);
	aimturn.vy = (3072+aimturn.vy)/2;


	work->control.turn.vy = work->control.turn.vy%4096;
	if (work->control.turn.vy < 1024){
	    work->control.turn.vy += 4096;
	}


	if (aimturn.vy > work->control.turn.vy){
	    if (aimturn.vy > work->control.turn.vy+30){
		work->control.turn.vy+=30;
	    }
	    else {
		work->control.turn.vy = aimturn.vy;
	    }
	}
	if (aimturn.vy < work->control.turn.vy){
	    if (aimturn.vy < work->control.turn.vy-30){
		work->control.turn.vy-=30;
	    }
	    else {
		work->control.turn.vy = aimturn.vy;
	    }
	}

	//printf ("T:%d \n", work->control.turn.vy);

    }
    work->control.turn.vx = 200;
    work->control.turn.vz = 0;

    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void end_hovering						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int end_mphov(Work* work){
    FVECTOR P1 = { 15000.0f, 10000.0f, -150000.0f, 1.0f};
    FVECTOR P2 = {-200000.0f, 10000.0f, -150000.0f, 1.0f};

    FVECTOR S1 = {200.0f, 0.0f,  300.0f, 1.0f};
    FVECTOR S2 = {0.0f, 0.0f, 0.0f, 1.0f};
    int time1 = 150;
    int time2 = 60;

    work->l2_timer += TIME_BASE;
    
    if (work->l2_timer < time1 * 5){
	get_hermove(&work->control.mov, &work->her_fromp, &P1,
		    &work->her_froms, &S1, work->l2_timer*1.0f, time1*5.0f);
    }
    else if (work->l2_timer < (time1+time2)*5.0f){
	get_hermove(&work->control.mov, &P1, &P2,
		    &S1, &S2, (work->l2_timer-(time1*5.0f))*1.0f, time2*5.0f);
	Dir_from_2Vec(&P1, &work->control.mov, &work->control.turn);
    }
    else {
	return 1;
    }
    return 0;
}




/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void control_hover						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int control_mphov(Work* work){

    FVECTOR	sht;
    int		dist;
    static	int	old_pc_vit;


    //	距離の計算
    SAT_Minus_FVECTOR(&sht, &work->goal_pos, &work->control.mov);
    dist = (int)_FVecLen3( &sht);

    // 近付いているかどうかのチェック
    if (work->gun_attack_last_dist == -1){
	work->gun_attack_last_dist = dist;
    }




    switch(HAR_MODE_L2){


    case 0:
    {
	Har_Move2Goal(work);
	work->rot_speed = 100;
	work->target_speed = 1400.0f ;
	work->accel_speed = 45;
	
	work->goal_pos.vx = 16000.0f;

	work->goal_pos.vy = -30000.0f;
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
	work->goal_pos.vz = GM_PlayerFindPos.vz+(RAND(2)*3000.0f)-1500.0f;
	if (work->goal_pos.vz < -156000.0f)
	    work->goal_pos.vz = -156000.0f;
	if (work->goal_pos.vz > -144000.0f)
	    work->goal_pos.vz = -144000.0f;

	Har_Move2Goal(work);

	if ( dist < 100000){
	    work->target_speed = 600.0f ;
	    work->accel_speed = 30;
	    HAR_MODE_L2 ++;
	}
	break;

    case 2:
	Har_Move2Goal(work);
	if ( dist < 2000){
	    set_targnzl_lock(work, ON);		//ノズルロック可能
	    set_targbodyall_small(work);	//全体は小さく
	    set_targbodyall_skip(work, ON);	//当たり無し
	    set_hartarg_skip(work, OFF);	//本体には当たりあり
	    HAR_MODE_L2 ++;
	}
	break;



    case 3:
	work->her_fromp = work->control.mov;
	work->her_froms = work->Speed;
	work->l2_timer = 0;
	work->l3_timer = 0;
	HAR_MODE_L3 = 0;
	HAR_MODE_L2++;
    case 4:

	if (work->cover_rot > -0.7f){
	    work->cover_rot = work->cover_rot - 0.007f;//カバー
	}
	if (work->r_flap_rot < 0.7f){
	    work->r_flap_rot = work->r_flap_rot + 0.02f;//フラップ
	}
	if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	    work->noz_fr_rot = work->noz_fr_rot - 0.003f;
	}
	else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	    work->noz_fr_rot = work->noz_fr_rot + 0.003f;
	}

	if (init_mphov(work)){
	    HAR_MODE_L2++;
	    old_pc_vit = GM_Vitality;	//笑うために保存
	    work->l3_timer = 0;
	    work->hover_rot = 0.0f;
	    work->target_pos = work->control.mov;
	    work->hover_rot = 0;
	    work->hov_nowroty = M_PI / 2.0f;
	    work->damage_num = 0;
	    work->control.step = DG_ZeroVector;
	    switch (RAND(3)){
	    case 0:		//よけられるか
		GM_JimakuSeSetMode (SD_V_SOLAT501, &work->control.mov, GM_SEMODE_BOMB);
		break;
	    case 1:		//でぇや
		SetHarSE(work, SD_V_SOLAT202, 20*5);
		break;
	    case 2:		//くらえ
		GM_JimakuSeSetMode (SD_V_SOLAT101, &work->control.mov, GM_SEMODE_BOMB);
		break;
	    }
	    har_call_event_proc(work, HAR_EVENT_RM_FIRE);
	    HAR_MODE_L3 = 0;
	    if (GM_PlayerFindPos.vz > work->control.mov.vz){
		ZTURN = -12.0f;
	    }
	    else {
		ZTURN = 12.0f;
	    }
	}
	break;

    case 5:
	if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	    work->noz_fr_rot = work->noz_fr_rot - 0.003f;
	}
	else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	    work->noz_fr_rot = work->noz_fr_rot + 0.003f;
	}
	if (hover_mpfire(work)){
	    work->her_fromp = work->control.mov;
	    work->l2_timer = 0;
	    work->l3_timer = 0;
	    HAR_MODE_L3 = 0;
	    work->control.step = DG_ZeroVector;
	    HAR_MODE_L2++;
	    work->damage_num=0;	// ダメージ受けた回数
	}

	//笑うぜっ！
	if (old_pc_vit != -1){
	    if (GM_Vitality+2 < old_pc_vit){
		SetSolVoice( work, 15, 3);	    //	//ははははは
		old_pc_vit = -1;
	    }
	    else {
		old_pc_vit = GM_Vitality;
	    }
	}
	break;

    case 6:
	if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	    work->noz_fr_rot = work->noz_fr_rot - 0.003f;
	}
	else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	    work->noz_fr_rot = work->noz_fr_rot + 0.003f;
	}
	if (suki_act(work)){
	    work->l2_timer = 0;
	    work->l3_timer = 0;
	    HAR_MODE_L3 = 0;
	    HAR_MODE_L2++;
	}
	break;

    case 7:
	har_call_event_proc(work, HAR_EVENT_RM_END);
	HAR_MODE_L2 ++;


	SetHarSE(work, SD_E_H_SWIDM1, 60);
	//	GM_SeSetMode (SD_E_H_SWIDM1, &work->control.mov, GM_SEMODE_BOMB);	// トビサリ
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
	break;
	
	// ***********回避に向かっている
    case 8:


	work->r_flap_rot = work->r_flap_rot * 0.95f;//フラップ
	work->l_elr_rot = work->l_elr_rot * 0.95f;
	work->r_elr_rot = work->r_elr_rot * 0.95f;
	work->cover_rot = work->cover_rot * 0.96f;//カバー
	work->noz_fr_rot = work->noz_fr_rot * 0.98f;//ノズル

	Har_Move2Goal(work);
	if ((work->gun_attack_last_dist < dist)&(work->gun_attack_flag == ON)){


    // **************橋

	
	work->rot_speed = 74;
	work->target_speed = 900.0f ;
	work->accel_speed = 45;
	
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
    case 9:
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

