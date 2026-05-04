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
int init_start(Work* work){
    FVECTOR P[4];
    FVECTOR S[4];
    int	time[5];	

     SET_VECTOR(P[0],  1418.0f,  11901.0f, -211965.0f);
     SET_VECTOR(P[1],     0.0f,  20000.0f, -190000.0f);
     SET_VECTOR(P[2],     0.0f,  8000.0f, -170000.0f);
     SET_VECTOR(P[3], 20000.0f,  35000.0f,  0.0f);
     SET_VECTOR(S[0], 0.0f,    0.0f,    0.0f);
     SET_VECTOR(S[1], 0.0f,    0.0f,  150.0f);
     SET_VECTOR(S[2], 0.0f,    0.0f,  200.0f);
     SET_VECTOR(S[3], 700.0f,    0.0f,  700.0f);

    time[0] = 100;
    time[1] = 200;
    time[2] = 200;

    work->l3_timer += TIME_BASE;

    switch(HAR_MODE_L3){
    case 0:
	work->control.turn.vx = work->control.rot.vx = 200;
	work->control.turn.vy = work->control.rot.vy = 0;
	work->control.turn.vz = work->control.rot.vz = 0;
	HAR_MODE_L3++;
	break;
	
    default:
    {	
	int num = HAR_MODE_L3;

	if (HAR_MODE_L3==3){
	    if (work->l3_timer <= 600){
		work->control.turn.vx = (300-work->l3_timer)*2/3;
	    }
	    work->control.turn.vz = -450*work->l3_timer/(time[2]*5);
	    work->control.turn.vy = 512*work->l3_timer/(time[2]*5);
	}


	get_hermove(&work->control.mov, &P[num-1], &P[num],
		    &S[num-1], &S[num], work->l3_timer*1.0f, time[num-1]*5.0f);

	if (work->l3_timer > time[num-1]*5.0f){
	    work->l3_timer = 1;
	    HAR_MODE_L3++;
	    if (HAR_MODE_L3==3){
		printf ("AS1\n");
	    	GM_SeSetMode (SD_E_H_HOVAS1, &work->control.mov, GM_SEMODE_BOMB);
	    }
	}
	
	if (HAR_MODE_L3==4){
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
int control_start(Work* work){

    switch(HAR_MODE_L2){
    case 0:
	work->l2_timer = 0;
	HAR_MODE_L2++;
    case 1:
	if (init_start(work)){
	    work->l2_timer = 0;
	    return 1;
	}
	break;
    }
    return 0;
}








