/********************************************************************************/
/*	har_acrobat.c								*/
/*	ハリアアクロバット飛行 *har_acrobat					*/
/*	2001/06/19 H.Satoyoshi							*/
/*	$Id: har_bakudan.c,v 1.1.1.3 2002/11/19 11:48:19 Yoshizawa1 Exp $		*/
/********************************************************************************/

#include "BP_Misc.h"
/********************************************************************************/
/*	Program									*/
/********************************************************************************/

//  *************接続飛行ルート
ACRO_POINT_EX	setuzoku_data_ex[] = {
    {       {{ 200000.0f,  80000.0f,  -340000.0f,  1.0f},
	     {-1500.0f,    0.0f,  -1500.0f,    1.0f}},
	    -1, 0, 350*5
    },
    {       {{-200000.0f,  80000.0f,  -340000.0f,  1.0f},
	     {-1500.0f,    0.0f,   1500.0f,    1.0f}},
	    -1, 0, 350*5
    },
    {       {{-200000.0f,  80000.0f,    60000.0f,  1.0f},
	     { 1500.0f,    0.0f,   1500.0f,    1.0f}},
	    -1, 0, 350*5
    },
    {       {{ 200000.0f,  80000.0f,    60000.0f,  1.0f},
	     { 1500.0f,    0.0f, -1500.0f,    1.0f}},
	    -1, 0, 350*5
    }
};

ACRO_POINT_EX	gun_data[] = {


    {   {{ 200000.0f, 110000.0f, 200000.0f,  1.0f},
	 {-700.0f,  0.0f, -2100.0f,    1.0f}},
	0.001f, 2, 150*5
    },
    {       {{20000.0f, 110000.0f,  50000.0f,  1.0f},
	     {0.0f,    0.0f,   -1000.0f,    1.0f}},
	    0.39f, 1, 60*5
    },
    {       {{6500.0f, 110000.0f,  -103000.0f,  1.0f},
	     {0.0f,    0.0f,   -1500.0f,    1.0f}},
	    0.01, 5, 100*5
	    //	    0.01, 5, 60*5
    },
    {      {{-20000.0f, 110000.0f,  -290000.0f,  1.0f},
	     {0.0f,    0.0f,   -1500.0f,    1.0f}},
	    0.95f, 4, 290*5
	   //	    0.95f, 4, 250*5
    },
};

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void view_auto_root						*/
/*	引数:	Work	*work							*/
/*	説明:	ルートを自動表示						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void view_auto_root_ex(ACRO_POINT_EX **acropoint, int number, int scale, int maxnum){
    FVECTOR	*FROM_POS;
    FVECTOR	*TO_POS;
    FVECTOR	FROM_VEC;
    FVECTOR	TO_VEC;
    short num_plus;
    
    num_plus = number+1;
    if (num_plus > maxnum){
	num_plus = 0;
    }

    if (acropoint[number] == NULL){
	printf ("Error har_bakudan \n");
	return;
    }
    
    FROM_POS = &(*acropoint[number]).acro.position;
    FROM_VEC = (*acropoint[number]).acro.speed;
    
    TO_POS = &(*acropoint[num_plus]).acro.position;
    TO_VEC = (*acropoint[num_plus]).acro.speed;
    
    _sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, (1.0f/TIME_BASE));
    _sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);
    
    
    View_HermiteRout(	FROM_POS, TO_POS, &FROM_VEC, &TO_VEC,
			(float)scale, 20);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void hermite_move_ex						*/
/*	引数:	Work	*work							*/
/*	説明:	クラスター爆撃へ						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void hermite_move_ex(Work *work, ACRO_POINT_EX **acropoint, int number, int max_num, int time){		
    short	num_plus;
    FVECTOR	*FROM_POS;
    FVECTOR	*TO_POS;
    FVECTOR	FROM_VEC;
    FVECTOR	TO_VEC;
    static int action_flag = -1;	// 行動フラグ
    
#ifdef DEBUG_MODE    
    static short view_flg = ON;
    FVECTOR	verts_from[2];
    FVECTOR	verts_to[2];
    FVECTOR	verts_har[2];
    
    if (GV_PadDataDirect[1].press & PAD_AL){
	view_flg = (view_flg==ON) ? OFF : ON;
    }
#endif
    
    num_plus = number+1;
    if (num_plus > max_num){
	num_plus = 0;
    }

    if (acropoint[number] == NULL){
	printf ("Error har_bakudan \n");
	return;
    }    
    
    // *****行動を決定
    if ((*acropoint[number]).time != -1.0f){
	if (action_flag!=number){
	    if ((*acropoint[number]).time < 1.0f*time/(*acropoint[number]).max_t){
		
		switch ((*acropoint[number]).action){
		    
		case 1:
		    GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_FIRE, 0);
		    har_call_event_proc(work, HAR_EVENT_CLS_BOMB );
		    printf ("            *********bombing    *********\n");
		    break;
		case 2:
		    GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_EQUIP, 0);
		    work->headmark_timer = 0;
		    work->flare_shoot_num = 5;
		    har_call_event_proc(work, HAR_EVENT_CLS_START );
		    break;
		case 3:
		    break;
		case 4:
		    //		    if(work->claster_num == 1){	//クラスター初回
		    //			Har_StageChange( work );
		    //		    }
		    break;
		case 5:
		    //dummy
		    break;
		}
		action_flag=number;
	    }
	}
    }
    else {
	action_flag=-1;
    }
    
    
    if (work->flare_shoot_num > 0){
	work->headmark_timer += 5;
	if (work->headmark_timer > 230){
	    Har_FlareShoot(work, 3000.0f);	//フレア撃ち
	    work->headmark_timer -= (150+RAND(161));
	    work->flare_shoot_num --;
	}
	
	if (work->flare_shoot_num == 0){
	    work->headmark_timer = 999;
	}
    }
    
    
    FROM_POS = &(*acropoint[number]).acro.position;
    FROM_VEC = (*acropoint[number]).acro.speed;
    
    TO_POS = &(*acropoint[num_plus]).acro.position;
    TO_VEC = (*acropoint[num_plus]).acro.speed;
    
    _sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
    _sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);
    
    MT_HermiteLerpVecScale( &work->control.mov, 
			    FROM_POS,
			    TO_POS,
			    &FROM_VEC,
			    &TO_VEC,
			    1.0f*time/(*acropoint[number]).max_t,
			    (float)(*acropoint[number]).max_t);
    
#ifdef DEBUG_MODE    
    if (view_flg == ON){
	float max_time0;
	float max_time1;
	_sceVu0CopyVector(&verts_from[0], &(*acropoint[number]).acro.position);
	_sceVu0CopyVector(&verts_from[1], &(*acropoint[number]).acro.position);
	verts_from[0].vy += 10000.0f;
	verts_from[1].vy -= 10000.0f;
	
	_sceVu0CopyVector(&verts_to[0], &(*acropoint[num_plus]).acro.position);
	_sceVu0CopyVector(&verts_to[1], &(*acropoint[num_plus]).acro.position);
	verts_to[0].vy += 10000.0f;
	verts_to[1].vy -= 10000.0f;
	
	max_time0 = (*acropoint[number]).max_t;
	max_time1 = (*acropoint[num_plus]).max_t;
	
	_sceVu0CopyVector(&verts_har[0], (FVECTOR*)work->body.objs->objs[0].world.m[3]);
	_sceVu0CopyVector(&verts_har[1], (FVECTOR*)work->body.objs->objs[0].world.m[3]);
	verts_har[0].vy += 40000.0f;
	verts_har[1].vy -= 40000.0f;


	if (HAR_RoutView == ON){
	    NewLineView(verts_from, 1, 255, 0, 0);
	    NewLineView(verts_to , 1, 0, 255, 0);
	    NewLineView(verts_har , 1, 0, 0, 255);
	    view_auto_root_ex(acropoint, number, max_time0, max_num);
	    view_auto_root_ex(acropoint, num_plus, max_time1, max_num);
	}
    }
#endif
    
    {				// ****向きの計算
	int	aim_time;
	int	aim_number;
	int	aim_num_plus;
	SVECTOR	tmprot;
	float	dir;
	FVECTOR	frompos, aimpos;
	
	aim_number = number;
	aim_time = time + (30*TIME_BASE);
	
	while (aim_time >= (*acropoint[aim_number]).max_t){
	    aim_time -= (*acropoint[aim_number]).max_t;
	    aim_number ++;
	    if (aim_number > max_num){
		aim_number = 0;
	    }
	}
	
	aim_num_plus = aim_number+1;
	if (aim_num_plus > max_num){
	    aim_num_plus = 0;
	}
	FROM_POS = &(*acropoint[aim_number]).acro.position;
	FROM_VEC = (*acropoint[aim_number]).acro.speed;
	TO_POS = &(*acropoint[aim_num_plus]).acro.position;
	TO_VEC = (*acropoint[aim_num_plus]).acro.speed;
	
	_sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
	_sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);
	
	MT_HermiteLerpVecScale( &frompos, 
				FROM_POS,
				TO_POS,
				&FROM_VEC,
				&TO_VEC,
				1.0f*aim_time/(*acropoint[aim_number]).max_t,
				(float)(*acropoint[aim_number]).max_t);
	
	aim_time = aim_time+TIME_BASE*5;
	if (aim_time >= (*acropoint[aim_number]).max_t){
	    aim_time -= (*acropoint[aim_number]).max_t;
	    aim_number ++;
	    if (aim_number > max_num){
		aim_number = 0;
	    }
	}
	
	aim_num_plus = aim_number+1;
	if (aim_num_plus > max_num){
	    aim_num_plus = 0;
	}
	FROM_POS = &(*acropoint[aim_number]).acro.position;
	FROM_VEC = (*acropoint[aim_number]).acro.speed;
	TO_POS = &(*acropoint[aim_num_plus]).acro.position;
	TO_VEC = (*acropoint[aim_num_plus]).acro.speed;
	
	_sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
	_sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);
	
	MT_HermiteLerpVecScale( &aimpos, 
				FROM_POS,
				TO_POS,
				&FROM_VEC,
				&TO_VEC,
				1.0f*aim_time/(*acropoint[aim_number]).max_t,
				(float)(*acropoint[aim_number]).max_t);
	
	tmprot.vx = work->control.turn.vx;
	tmprot.vy = work->control.turn.vy;
	tmprot.vz = work->control.turn.vz;
	
	{
	    FVECTOR nowrot;
	    FVECTOR testrot;
	    static FVECTOR oldrot;
	    static float hisrotx[10];
	    static float hisroty[10];
	    int eturnz;
	    int loop;
	    Dir_from_2VecF(&frompos, &aimpos, &nowrot);
	    
	    //	    printf ("rt:%f    %f\n",nowrot.vx, nowrot.vy);
	    //	    NewLineView(&frompos, 1, 255, 0, 0);
	    
	    testrot.vx = oldrot.vx - nowrot.vx;
	    testrot.vy = oldrot.vy - nowrot.vy;
	    
	    if (testrot.vx > M_PI*2.0f){
		testrot.vx -= M_PI*2.0f;
	    }
	    if (testrot.vx < -M_PI*2.0f){
		testrot.vx += M_PI*2.0f;
	    }
	    if (testrot.vy > M_PI*2.0f){
		testrot.vy -= M_PI*2.0f;
	    }
	    if (testrot.vy < -M_PI*2.0f){
		testrot.vy += M_PI*2.0f;
	    }
	    
	    hisrotx[9] = testrot.vx;
	    hisroty[9] = testrot.vy;
	    for (loop=0; loop<9; loop++){
		testrot.vx += hisrotx[loop];
		testrot.vy += hisroty[loop];
		hisrotx[loop] = hisrotx[loop+1];
		hisroty[loop] = hisroty[loop+1];
	    }
	    testrot.vx = testrot.vx/10.0f;
	    testrot.vy = testrot.vy/10.0f;
	    
	    dir = atan2f(testrot.vy*1000.0f, testrot.vx*1000.0f);
	    
	    //	    if (((M_PI-0.2f) > fabs(dir))&&(fabs(dir) > 0.2f)){ 
	    eturnz = 4095 & ( short )( ( dir * 2048.0F / M_PI ) + 0.5F ) ; //* 四捨五入 */
	    
	    oldrot = nowrot;
	    
	    work->control.rot.vx = work->control.turn.vx = 4095&(short)((nowrot.vx*2048.0F/M_PI)+0.5F);
	    work->control.rot.vy = work->control.turn.vy = 4095&(short)((nowrot.vy*2048.0F/M_PI)+0.5F);

	    if ( (*acropoint[number]).action == 1 || (*acropoint[number]).action == 5){
		work->control.turn.vz = 0;
	    }
	    else {
		work->control.turn.vz = eturnz/4;
	    }

	}
	work->control.interp = 16;
    }
}    

    
#define GET_IN	0
#define GET_OUT	1

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void acrobat_move						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_bakudan(Work* work){
    short	loop;
    int		rout_num;

    work->acpoint.acro.position = work->control.mov;
    work->acpoint.acro.speed = work->Speed;

    
    _sceVu0Normalize(&work->acpoint.acro.speed, &work->acpoint.acro.speed);

    _sceVu0ScaleVector(&work->acpoint.acro.speed, &work->acpoint.acro.speed, 2121.0f);

printf("Speed = %f %f %f\n", work->Speed.vx, work->Speed.vy, work->Speed.vz);


    work->acpoint.time = -1;
    work->acpoint.max_t = 350*5;

    work->last_rout_num = setuzokuget(&work->control.mov, &work->Speed, GET_OUT);

    pointexdat[0] = (ACRO_POINT_EX*)&work->acpoint;
    pointexdat[1] = (ACRO_POINT_EX*)&setuzoku_data_ex[work->last_rout_num];


    {
	FVECTOR	shift;
	int dist;
	_sceVu0SubVector (&shift,
			  &(*pointexdat[0]).acro.position,
			  &(*pointexdat[1]).acro.position);
	dist = (int)_FVecLen3( &shift);

	if (dist < 10000){
	    work->last_rout_num ++;
	    if (work->last_rout_num >= 4){
		work->last_rout_num = 0;
	    }
	    pointexdat[1] = (ACRO_POINT_EX*)&setuzoku_data_ex[work->last_rout_num];
	}
    }


    
    rout_num = 2;
    while (work->last_rout_num != 2){
	work->last_rout_num ++;
	if (work->last_rout_num >= 4){
	    work->last_rout_num = 0;
	}
	pointexdat[rout_num] = (ACRO_POINT_EX*)&setuzoku_data_ex[ work->last_rout_num ];
	printf ("rnum  ---------  %d  %d \n",rout_num, work->last_rout_num);
	rout_num ++;
    };
    
    if ( BP_IsPAL()==TRUE )
      gun_data[1].time = 0.325f;
    else
      gun_data[1].time = 0.39f;

    for (loop=0; loop<4; loop++){
	pointexdat[rout_num] = &gun_data[loop];
	rout_num ++;
    }
    work->last_rout_num = get_setuzoku_num((ACRO_POINT*)&gun_data[loop-1], GET_OUT);
    pointexdat[rout_num] = (ACRO_POINT_EX*)&setuzoku_data_ex[ work->last_rout_num ];
    work->rout_num_max = rout_num;

    printf ("max  ---------  %d \n", work->rout_num_max);
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void acrobat_move						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int bakudan_move(Work *work){		
    static int time = 0;
    static int max_time = 0;
    static short number = 0;
    static int   bomb_start_time;
    static int	time_accell;
    static int cls_max_time;
    
    switch(HAR_MODE_L2){
    case 0:
	number = 0;
	set_bakudan(work);
	HAR_MODE_L3 = 0;
	time_accell = 0;	//ルート移動の加速
	bomb_start_time = -1;
	HAR_MODE_L2++;
	time = 0;
    case 1:
	
	//  爆弾エフェクト制御
	switch(HAR_MODE_L3){
	case 0:
	    if ( (((Work*)GET_HAR_WORK)->event_id == HAR_EVENT_CLS_START)&&(bomb_start_time == -1)){
		SetHarSE(work, SD_E_H_SWIDM4, 155*5);	//音声 フライ媒音
		bomb_start_time = 0;
	    }
	    if ( ((Work*)GET_HAR_WORK)->event_id == HAR_EVENT_CLS_BOMB){

		SetSolVoice(work, 16, 2);	//音声 ”死ねぇーー！！

		HAR_MODE_L3++;
		bomb_start_time = 0;
	    }
	    switch( GM_GameLevel ){
	    case GM_LEVEL_VERYEASY:
		cls_max_time = 1600;
		break;
	    case ST_LEV_EASY:
		cls_max_time = 1400;
		break;
	    case ST_LEV_NORMAL:
		cls_max_time = 1230;
		break;

	    case ST_LEV_UPNORM:
		cls_max_time = 1100;
		break;

	    case GM_LEVEL_HARD:
		cls_max_time = 1000;
		break;
	    case GM_LEVEL_E_EXTREME:
	    case GM_LEVEL_EXTREME:
		cls_max_time = 600;
		break;
	    }
	    break;
	case 1:
	{
	    bomb_start_time += TIME_BASE;
	    
	    
	    if (bomb_start_time>cls_max_time){
		NewHarrierClasterExplosion(&work->claster_bomb_pos, &work->claster_bomb_flag, work->brake_proc_id);
		bomb_start_time = 0;
		cls_max_time = 1000000;
		HAR_MODE_L3++;
	    }
	    //	    printf ("T: %d\n", bomb_start_time);
	}
	    break;
	case 2:

	    if (work->claster_bomb_flag != 0){
		static int seflag = 1;
		printf ("GET   %d\n", work->claster_bomb_flag);
		call_pad_vibration(VIB_CL_EXP);		//パッド振動
		
		// **************爆発SE
		if (seflag){
		    if (RAND(2)==0){
			GM_SeSetMode (SD_E_CLMBOM04, &work->claster_bomb_pos, GM_SEMODE_BOMB);//SE爆発１
		    }
		    else {
			GM_SeSetMode (SD_E_CLMBOM05, &work->claster_bomb_pos, GM_SEMODE_BOMB);//SE爆発２
		    }
		    seflag = 0;
		}
		else {
		    seflag = 1;
		}
		
		if (work->claster_bomb_flag & 2){
		    if (GM_CheckPlayerStatus( PLAYER_BEYOND )){

			printf ("%d:%d",GV_StrCode("by003"), PL_GetEludeTrapCode());

			if (GV_StrCode("by003") ==  PL_GetEludeTrapCode()){
			    send_player_message();
			    work->fire_damage_point = 28*16+15;
			}
			if ( (GV_StrCode("by005") ==  PL_GetEludeTrapCode()) && 
			     (!GM_CheckPlayerStatusEX(I64(0), PLAYER2_ELUDE_FALL)) ){
			    send_player_message();
			    work->fire_damage_point = 28*16+15;
			}
		    }
		    else {
			send_player_message();
			work->fire_damage_point = 28*16+15;
		    }
		}
		if (work->claster_bomb_flag & 8){
		    if (GM_CheckPlayerStatus( PLAYER_BEYOND )){

			printf ("%d:%d",GV_StrCode("by003"), PL_GetEludeTrapCode());

			if ( (GV_StrCode("by003") ==  PL_GetEludeTrapCode()) || 
			    (GM_GameLevel >= GM_LEVEL_HARD) ){

			    send_player_message();
			    work->fire_damage_point = 28*16+15;
			}
		    }
		}
		if (work->claster_bomb_flag & 4){

		    printf ("#############Bomb SE Call \n");

		    GM_SeSet(GM_PAN_CENTER, 0x3f, SD_A_BRIDGEX1);
		    //		    GM_SeSetMode (SD_A_BRIDGEX1, &work->claster_bomb_pos, GM_SEMODE_BOMB);
		    har_call_event_proc(work, HAR_EVENT_BRIDGE_BRAKE);
		}

	    }
	    break;
	}

	// ルート移動
	max_time = (*pointexdat[number]).max_t;

	hermite_move_ex(work, &pointexdat[0], number, work->rout_num_max, time);

	time += TIME_BASE;
	if (work->rout_num_max-4>number){
	    if (time_accell <  TIME_BASE*12){
		time_accell += 1;
	    }
	    time += time_accell/8;
	} 

	if (time >= max_time){
	    time -= max_time;
	    number ++;

	    printf("####Next %d\n",number);

	    
	    if (number >= work->rout_num_max){	// ***********終了チェック
		number = 0;
		har_call_event_proc(work, HAR_EVENT_CLS_END );
		if( GM_Vitality > 5){
		    SetSolVoice(work, 6, 3); // ==しぶとい奴だ
		}
		return 1;
	    }
	    
	    if( (*pointexdat[number]).action == 1){
		har_call_event_proc(work, HAR_EVENT_CLS_TYOKUSIN);
	    }
	    
	    printf ("nuber++  ---------  %d \n", number);
	}
	break;

    }



    return 0;
}


