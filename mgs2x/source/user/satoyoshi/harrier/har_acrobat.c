/********************************************************************************/
/*	har_acrobat.c								*/
/*	ハリアアクロバット飛行 *har_acrobat					*/
/*	2001/06/19 H.Satoyoshi							*/
/*	$Id: har_acrobat.c,v 1.1.1.3 2002/11/19 11:48:19 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/


//#define MAX_NUMBER 1
//#define MAX_NUMBER 5

#define MAX_LEAD (30)
#define MAX_NUMBER 3
#define ACRO_SPEED (210)

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void view_auto_root						*/
/*	引数:	Work	*work							*/
/*	説明:	ルートを自動表示						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void view_auto_root(ACRO_POINT **acropoint, int number, int scale, int maxnum){
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
	printf ("Error har_acrobat \n");
	return;
    }

    FROM_POS = &((*acropoint[number]).position);
    FROM_VEC = (*acropoint[number]).speed;

    TO_POS = &(*acropoint[num_plus]).position;
    TO_VEC = (*acropoint[num_plus]).speed;

    _sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, (1.0f/TIME_BASE));
    _sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);


    View_HermiteRout(	FROM_POS, TO_POS, &FROM_VEC, &TO_VEC,
			(float)scale, 20);
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void acrobat							*/
/*	引数:	Work	*work							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void hermite_move(Work *work, ACRO_POINT **acropoint, int number, int max_num, int time, int max_time){		
    FVECTOR	*FROM_POS;
    FVECTOR	*TO_POS;
    FVECTOR	FROM_VEC;
    FVECTOR	TO_VEC;
    int		num_plus;


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
	printf ("Error har_acrobat \n");
	return;
    }


    FROM_VEC = (acropoint[number])->speed;
    TO_VEC = (acropoint[num_plus])->speed;

    FROM_POS = &( acropoint[number]->position );
    TO_POS = &( acropoint[num_plus]->position );

    _sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
    _sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);


    MT_HermiteLerpVecScale( &work->control.mov, 
			    FROM_POS,
			    TO_POS,
			    &FROM_VEC,
			    &TO_VEC,
			    1.0f*time/max_time,
			    (float)max_time);


#ifdef DEBUG_MODE    
    if (view_flg == ON){
	_sceVu0CopyVector(&verts_from[0], &(*acropoint[number]).position);
	_sceVu0CopyVector(&verts_from[1], &(*acropoint[number]).position);
	verts_from[0].vy += 10000.0f;
	verts_from[1].vy -= 10000.0f;
	
	_sceVu0CopyVector(&verts_to[0], &(*acropoint[num_plus]).position);
	_sceVu0CopyVector(&verts_to[1], &(*acropoint[num_plus]).position);
	verts_to[0].vy += 10000.0f;
	verts_to[1].vy -= 10000.0f;

	_sceVu0CopyVector(&verts_har[0], (FVECTOR*)work->body.objs->objs[0].world.m[3]);
	_sceVu0CopyVector(&verts_har[1], (FVECTOR*)work->body.objs->objs[0].world.m[3]);
	verts_har[0].vy += 40000.0f;
	verts_har[1].vy -= 40000.0f;

	if (HAR_RoutView == ON){	
	    NewLineView(verts_from, 1, 255, 0, 0);
	    NewLineView(verts_to , 1, 0, 255, 0);
	    NewLineView(verts_har , 1, 0, 0, 255);
	    view_auto_root(acropoint, number, max_time, max_num);
	    view_auto_root(acropoint, num_plus, max_time, max_num);
	}
    }
#endif





    {				// ****向きの計算
	int	aim_time;
	int	aim_number;
	int	aim_num_plus;
	SVECTOR	tmprot;
	float	dir;
	FVECTOR	aimpos, frompos;
	int	turnz;

	aim_number = number;
	//	aim_time = time + (work->l3_timer);

	if (work->l3_timer < MAX_LEAD*5){
	    work->l3_timer += TIME_BASE;
	}
	aim_time = time + (work->l3_timer);


	if (aim_time >= max_time){
	    aim_time -= max_time;
	    aim_number ++;
	    if (aim_number > max_num){
		aim_number = 0;
	    }
	}

	aim_num_plus = aim_number+1;
	if (aim_num_plus > max_num){
	    aim_num_plus = 0;
	}
	FROM_POS = &(*acropoint[aim_number]).position;
	FROM_VEC = (*acropoint[aim_number]).speed;
	TO_POS = &(*acropoint[aim_num_plus]).position;
	TO_VEC = (*acropoint[aim_num_plus]).speed;

	_sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
	_sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);
	
	MT_HermiteLerpVecScale( &frompos, 
				FROM_POS,
				TO_POS,
				&FROM_VEC,
				&TO_VEC,
				1.0f*aim_time/max_time,
				(float)max_time);

	aim_time = aim_time+TIME_BASE;
	if (aim_time >= max_time){
	    aim_time -= max_time;
	    aim_number ++;
	    if (aim_number > max_num){
		aim_number = 0;
	    }
	}
	aim_num_plus = aim_number+1;
	if (aim_num_plus > max_num){
	    aim_num_plus = 0;
	}
	FROM_POS = &(*acropoint[aim_number]).position;
	FROM_VEC = (*acropoint[aim_number]).speed;
	TO_POS = &(*acropoint[aim_num_plus]).position;
	TO_VEC = (*acropoint[aim_num_plus]).speed;
	
	_sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
	_sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);

	MT_HermiteLerpVecScale( &aimpos, 
				FROM_POS,
				TO_POS,
				&FROM_VEC,
				&TO_VEC,
				1.0f*aim_time/max_time,
				(float)max_time);
	
	tmprot.vx = work->control.turn.vx;
	tmprot.vy = work->control.turn.vy;
	tmprot.vz = work->control.turn.vz;
	
	Dir_from_2Vec(&frompos, &aimpos, &work->control.turn);
	tmprot.vx = work->control.turn.vx - tmprot.vx;
	tmprot.vy = work->control.turn.vy - tmprot.vy;
	if (tmprot.vx > 3000){
	    tmprot.vx -= 4096;
	}
	if (tmprot.vx < -3000){
	    tmprot.vx += 4096;
	}
	if (tmprot.vy > 3000){
	    tmprot.vy -= 4096;
	}
	if (tmprot.vy < -3000){
	    tmprot.vy += 4096;
	}
	if (abs(tmprot.vy) > 2000){	//向き＆上下の入れ替わり
	    turnz = work->control.turn.vz = work->control.rot.vz = tmprot.vz - 2048;
	    work->control.rot.vy = work->control.turn.vy;
	}
	else {
	    dir = atan2f(tmprot.vy, tmprot.vx);
	    turnz = 4095 & ( short )( ( dir * 2048.0F / PI ) + 2048.5F ) ; //* 四捨五入 */
	}
	work->control.turn.vz = turnz;
	//	work->control.rot.vx = work->control.turn.vx;
	//	work->control.rot.vy = work->control.turn.vy;

	work->control.interp = 16;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void get_setuzoku_num						*/
/*	引数:	ACRO_POINT*	acro_pos					*/
/*		int		flag	0:開始					*/
/*					1:終了					*/
/*	返値:	接続番号							*/
/*	説明:	接続場所を得る							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int get_setuzoku_num(ACRO_POINT* acro_pos, int flag){
    FVECTOR	tmppos;
    FVECTOR	tmpvec;
    
    tmppos = acro_pos->position;
    tmpvec = acro_pos->speed;
    
    _sceVu0Normalize(&tmpvec, &tmpvec);
    if (flag == GET_IN){
	_sceVu0ScaleVector(&tmpvec, &tmpvec, -30000.0f);
    }
    else if (flag == GET_OUT){
	_sceVu0ScaleVector(&tmpvec, &tmpvec, 30000.0f);
    }
    _sceVu0AddVector(&tmppos, &tmppos, &tmpvec);
    // グループ

#if 0
    printf ("-  %8.0f,%8.0f,%8.0f \n",
	    tmppos.vx,
	    tmppos.vy,
	    tmppos.vz);
#endif

    if(tmppos.vx <= -15000.0f){
	if (flag == GET_IN){
	    //printf ("Return 1\n");
	    return 1;
	}
	else if (flag == GET_OUT){
	    //printf ("Return 2\n");
	    return 2;
	}
    }
    else if(tmppos.vx >  15000.0f){
	if (flag == GET_IN){
	    //printf ("Return 3\n");
	    return 3;
	}
	else if (flag == GET_OUT){
	    //printf ("Return 0\n");
	    return 0;
	}
    }
    else if(tmppos.vy <=  -150000.0f){
	if (flag == GET_IN){
	    //printf ("Return 0\n");
	    return 0;
	}
	else if (flag == GET_OUT){
	    //printf ("Return 1\n");
	    return 1;
	}
    }
    else if(tmppos.vy > - 150000.0f){
	if (flag == GET_IN){
	    //printf ("Return 2\n");
	    return 2;
	}
	else if (flag == GET_OUT){
	    //printf ("Return 3\n");
	    return 3;
	}
    }
    return -1;
}




/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void set_acro							*/
/*	引数:	Work	*work							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_acro(Work *work){
    int	loop;
    int		rout_num;

    work->acpoint.acro.position = work->control.mov;
    work->acpoint.acro.speed = work->Speed;
    work->acpoint.time = -1;
    work->acpoint.max_t = 350*5;

    work->last_rout_num = setuzokuget(&work->control.mov, &work->Speed, GET_OUT);

    pointdata[0] = (ACRO_POINT*)&work->acpoint;
    pointdata[1] = &setuzoku_data[work->last_rout_num];
    
    rout_num = 2;
    while (work->last_rout_num != get_setuzoku_num(&acro_data1[0], GET_IN)){
	work->last_rout_num ++;
	if (work->last_rout_num >= 4){
	    work->last_rout_num = 0;
	}
	pointdata[rout_num] = &setuzoku_data[ work->last_rout_num ];
	//printf ("		IN			%d\n",	work->last_rout_num);
	rout_num ++;
    };
    
    
//    for (loop=0; loop<7; loop++){
    for (loop=0; loop<2; loop++){
	pointdata[rout_num] = &acro_data1[loop];
	rout_num ++;
    }
    work->last_rout_num = get_setuzoku_num(&acro_data1[loop], GET_OUT);
    pointdata[rout_num] = &setuzoku_data[ work->last_rout_num ];
    work->rout_num_max = rout_num;
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void acrobat_move						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int acrobat_move(Work *work){		
    static int time = 0;
    int max_time = ACRO_SPEED*TIME_BASE;
    static int number = 0;

    switch(HAR_MODE_L2){
    case 0:
	number = 0;
	time = 0;
	work->l3_timer = 0;
	set_acro(work);
	har_call_event_proc(work, HAR_EVENT_ACRO_START );
	HAR_MODE_L2++;
    case 1:
	hermite_move(work, pointdata, number, work->rout_num_max, time, max_time);
	//printf ("N:%d T:%d M:%d",number, time, max_time);

	time += TIME_BASE;
	if (time >= max_time){
	    time -= max_time;
	    number ++;
	    if (number >= work->rout_num_max){	// ***********終了チェック
		number = 0;
		har_call_event_proc(work, HAR_EVENT_ACRO_END );
		return 1;
	    }
	}
	break;
    }
    return 0;
}






