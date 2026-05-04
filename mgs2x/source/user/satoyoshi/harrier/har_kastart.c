/********************************************************************************/
/*	Har_start.c								*/
/*	ハリア開始								*/
/*	2001/07/15 H.Satoyoshi							*/
/*	$Id:	*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/



/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void init_start							*/
/*	引数:	Work	*work							*/
/*	説明:	ハリア戦スタートの移動						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int init_start(Kas_Work* work){
    FVECTOR P[5];
    FVECTOR S[5];
    int	time[5];	


     SET_VECTOR(P[0],  14020.0f, 20000.0f,  -135617.0f);
     SET_VECTOR(P[1],  -6000.0f,  -10000.0f,  -100000.0f);
     SET_VECTOR(P[2],  -21000.0f, 8000.0f,  -150000.0f);
     SET_VECTOR(P[3],  -20000.0f, 15000.0f,  -150000.0f);
     SET_VECTOR(P[4],  0.0f, 20000.0f,  -105000.0f);  

     SET_VECTOR(S[0], 0.0f,   600.0f,    0.0f);
     SET_VECTOR(S[1], -500.0f,  -500.0f,  -200.0f);
     SET_VECTOR(S[2], 100.0f,   40.0f,    0.0f);
     SET_VECTOR(S[3], 0.0f,   0.0f,    0.0f);
     SET_VECTOR(S[4], 200.0f,  0.0f,  0.0f);

    time[0] = 200;
    time[1] = 400;
    time[2] = 300;
    time[3] = 300;

    work->kas_timer += TIME_BASE;

    switch(KAS_MODE){
    case 0:
	work->control.turn.vx = work->control.rot.vx = 0;
	work->control.turn.vy = work->control.rot.vy = 4096*222/365;
	work->control.turn.vz = work->control.rot.vz = 0;
	KAS_MODE++;
	break;
	
    default:
    {	
	int num = KAS_MODE;
	get_hermove(&work->control.mov, &P[num-1], &P[num],
		    &S[num-1], &S[num], work->kas_timer*1.0f, time[num-1]*5.0f);
	if (work->kas_timer > time[num-1]*5.0f){
	    work->kas_timer = 1;
	    if (KAS_MODE==2){
		if (work->item_reserve_flag == 1){
			if (har_gameover_check_ok()){
			    if (work->is_bossrush != ON ){
				work->set_snake = SET_SNK_ITEM;
				work->item_drop = ON;
				work->item_aim_pos.vx = KAS_ITEM_DROP_POS.vx;
				work->item_aim_pos.vy = KAS_ITEM_DROP_POS.vy;
				work->item_aim_pos.vz = KAS_ITEM_DROP_POS.vz;
				work->item_reserve_flag = 2;
			    }
			}
		}
		    printf ("  			ITEM drop stop\n");
	    }
	    KAS_MODE++;
	}

	//   ******** カサッカの方向
	if (KAS_MODE>1){
	    FVECTOR tmpfrot;
	    float tmpry;
	    Dir_from_2VecF(&work->control.mov, &GM_PlayerFindPos, &tmpfrot);
	    tmpry = ( short )( ( tmpfrot.vy * 2048.0F / M_PI ) - 1023.5F );

	    if (tmpry > 2048 ){
		tmpry -= 4096;
	    }
	    else if (tmpry < -2048 ){
		tmpry += 4096;
	    }


	    if (tmpry-work->control.turn.vy>5){
		work->control.turn.vy += 15;
	    }
	    else if (tmpry-work->control.turn.vy<-5){
		work->control.turn.vy -= 15;
	    }
	}
	
	if (KAS_MODE==5){
	    return 1;
	}
	
    }
	break;
    }
    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void control_burning						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int control_kas_start(Kas_Work* work){
    
    
    if (init_start(work)){
	work->rout_time = 0;
	work->rout_number = 3;
	return 1;
    }
    
    return 0;
}








