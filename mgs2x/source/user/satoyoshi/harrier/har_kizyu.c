/********************************************************************************/
/*	har_acrobat.c								*/
/*	ハリアアクロバット飛行 *har_acrobat					*/
/*	2001/06/19 H.Satoyoshi							*/
/*	$Id: har_kizyu.c,v 1.1.1.3 2002/11/19 11:48:22 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/

ACRO_POINT* LAST_ACROPOINT;

//  *************接続飛行ルート
ACRO_POINT_EX	setuzoku_data_ex[] = {
    {       {{ 200000.0f,  80000.0f,  -340000.0f,  1.0f},
	    {-1500.0f,    0.0f,  -1500.0f,    1.0f}},
              -1, 1, 350*TIME_BASE
    },
    {       {{-200000.0f,  80000.0f,  -340000.0f,  1.0f},
	    {-1500.0f,    0.0f,   1500.0f,    1.0f}},
              -1, 1, 350*TIME_BASE
    },
    {       {{-200000.0f,  80000.0f,    60000.0f,  1.0f},
	    { 1500.0f,    0.0f,   1500.0f,    1.0f}},
              -1, 1, 350*TIME_BASE
    },
    {       {{ 200000.0f,  80000.0f,    60000.0f,  1.0f},
	    { 1500.0f,    0.0f, -1500.0f,    1.0f}},
              -1, 1, 350*TIME_BASE
    }
};

ACRO_POINT_EX	gun_data[] = {
    {  {{400000.0f, 90000.0f,  -140000.0f,  1.0f},
	    {-1000.0f,  0.0f, 0.0f,    1.0f}},
	    0.5, 1, 250*5
    },
    {       {{30000.0f,  4000.0f,  -150000.0f,  1.0f},
	    {-3000.0f,    -500.0f,    0.0f,    1.0f}},
	    0.5f, 2, 10*5
    },
    {       {{-1000.0f,  3000.0f,  -150000.0f,  1.0f},
	    {-3000.0f,    0.0f,    0.0f,    1.0f}},
	    0.5f, 2, 50*5
    },
    {       {{-120000.0f,  6500.0f,  -150000.0f,  1.0f},
	    {-3000.0f,    100.0f,    0.0f,    1.0f}},
	    -1, 0, 150*5
    }
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
/*	名前:	void acrobat							*/
/*	引数:	Work	*work							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void hermite_move_ex(Work *work, ACRO_POINT_EX **acropoint, int number, int max_num, int time){		
    short	num_plus;
    FVECTOR	*FROM_POS;
    FVECTOR	*TO_POS;
    FVECTOR	FROM_VEC;
    FVECTOR	TO_VEC;

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

    if ((*acropoint[number]).time != -1.0f){
	if ((*acropoint[number]).time > 1.0f*time/(*acropoint[number]).max_t){

	    switch ((*acropoint[number]).action){
		
	    case 1:
		SET_GUN_MODE(GUN_FIRE_FLG_AIM, GUN_FIRE_FLG_HIT);
		break;
	    case 2:
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
		break;
		
	    }
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
	
	NewLineView(verts_from, 1, 255, 0, 0);
	NewLineView(verts_to , 1, 0, 255, 0);
	NewLineView(verts_har , 1, 0, 0, 255);
	view_auto_root_ex(acropoint, number, max_time0, max_num);
	view_auto_root_ex(acropoint, num_plus, max_time1, max_num);
    }
#endif

    {				// ****向きの計算
	int	aim_time;
	int	aim_number;
	int	aim_num_plus;
	SVECTOR	tmprot;
	float	dir;
	FVECTOR	frompos, aimpos;
	int	turnz;

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
	    
	    NewLineView(&frompos, 1, 255, 0, 0);

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


#if 0
	    }
	    else {
		eturnz = 0;
	    }
#endif

	    oldrot = nowrot;
	    
	    work->control.rot.vx = work->control.turn.vx = 4095&(short)((nowrot.vx*2048.0F/M_PI)+0.5F);
	    work->control.rot.vy = work->control.turn.vy = 4095&(short)((nowrot.vy*2048.0F/M_PI)+0.5F);

	    work->control.turn.vz = eturnz;
	}
	work->control.interp = 16;
    }
}


#define GET_IN	0
#define GET_OUT	1

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void acrobat_move						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int kizyu_move(Work *work){		
    static int time = 0;
    static int max_time = 0;
    static short number = 0;
    static char flag = OFF;
    static ACRO_POINT_EX* pointdata[30];
    short	loop;

    // *************セット関数
    if (flag == OFF){
	pointdata[0] = (ACRO_POINT_EX*)&setuzoku_data_ex[ work->last_rout_num ];

	work->rout_num_max = 1;
	while (work->last_rout_num != get_setuzoku_num((ACRO_POINT*)&gun_data[0], GET_IN)){
	    work->last_rout_num ++;
	    if (work->last_rout_num >= 4){
		work->last_rout_num = 0;
	    }
	    pointdata[work->rout_num_max] = (ACRO_POINT_EX*)&setuzoku_data_ex[ work->last_rout_num ];
	    //printf ("		IN			%d\n",	work->last_rout_num);

	    work->rout_num_max ++;
	};


	for (loop=0; loop<4; loop++){
	    pointdata[work->rout_num_max] = &gun_data[loop];
	    work->rout_num_max ++;
	}
	work->last_rout_num = get_setuzoku_num((ACRO_POINT*)&gun_data[loop-1], GET_OUT);


	//	printf ("		OUT			%d  %d\n",	work->rout_num_max, loop);

	pointdata[work->rout_num_max] = (ACRO_POINT_EX*)&setuzoku_data_ex[ work->last_rout_num ];
	flag = ON;
    }

    max_time = (*pointdata[number]).max_t;

    hermite_move_ex(work, &pointdata[0], number, work->rout_num_max, time);

    time += TIME_BASE;
    if (time >= max_time){
	time -= max_time;
	number ++;
	if (number >= work->rout_num_max){	// ***********終了チェック
	    number = 0;
	    //printf("Next  %d\n",number);
	    flag = OFF;
	    return 1;
	}
	//printf("%d\n",number);
    }
    return 0;
}


