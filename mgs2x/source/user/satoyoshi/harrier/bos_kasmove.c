/********************************************************************************/
/*	har_kasacka.c								*/
/*	カサッカ								*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void view_auto_root						*/
/*	引数:	Work	*work							*/
/*	説明:	ルートを作成							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void make_kas_rout_bs(Kas_Work *work){
    int		loop;
    float	num;
    Work	*hwork;

    if ( GET_HAR_WORK == NULL )
        return;

    hwork = (Work*)GET_HAR_WORK;


    num = hwork->dist_new - 50000.0f;

    if (num < 0.0f){
	num = 0.0f;
    }
    if (num > 500000.0f){
	num = 500000.0f;
    }

    num = 1.0f - (1.0f+cosf(M_PI * num/500000.0f))/2.0f;

#ifdef DEBUG_MODE
    if (HAR_KRD == 1){
	if (GV_PadDataDirect[1].status & PAD_L1){
	    work->rout_num += 0.004f;
	    if (work->rout_num > 1.0f){
		work->rout_num = 1.0f;
	    }
	    printf ("%f,  %f\n",num,  work->rout_num); 
	}
	
	if (GV_PadDataDirect[1].status & PAD_L2){
	    work->rout_num -= 0.004f;
	    if (work->rout_num < 0.0f){
		work->rout_num = 0.0f;
	    }
	    printf ("%f,  %f\n",num,  work->rout_num); 
	}
    }
    else {
#endif
	
	if (work->rout_num < num){
	    work->rout_num += 0.004f;
	    if (work->rout_num > 1.0f){
		work->rout_num = 1.0f;
	    }
	    //	    printf ("%f,  %f\n",num,  work->rout_num); 
	}
	
	if (work->rout_num > num){
	    work->rout_num -= 0.004f;
	    if (work->rout_num < 0.0f){
		work->rout_num = 0.0f;
	    }
	    //	    printf ("%f,  %f\n",num,  work->rout_num); 
	}
	
#ifdef DEBUG_MODE
    }
#endif





    for (loop=0; loop<4; loop++){
	work->kas_data[loop].acro.position.vx = (kas_data_near[loop].acro.position.vx*work->rout_num)
	    +(kas_data_far[loop].acro.position.vx*(1.0f-work->rout_num));
	work->kas_data[loop].acro.position.vy = (kas_data_near[loop].acro.position.vy*work->rout_num)
	    +(kas_data_far[loop].acro.position.vy*(1.0f-work->rout_num));
	work->kas_data[loop].acro.position.vz = (kas_data_near[loop].acro.position.vz*work->rout_num)
	    +(kas_data_far[loop].acro.position.vz*(1.0f-work->rout_num));

	work->kas_data[loop].acro.speed.vx = (kas_data_near[loop].acro.speed.vx*work->rout_num)
	    +(kas_data_far[loop].acro.speed.vx*(1.0f-work->rout_num));
	work->kas_data[loop].acro.speed.vy = (kas_data_near[loop].acro.speed.vy*work->rout_num)
	    +(kas_data_far[loop].acro.speed.vy*(1.0f-work->rout_num));
	work->kas_data[loop].acro.speed.vz = (kas_data_near[loop].acro.speed.vz*work->rout_num)
	    +(kas_data_far[loop].acro.speed.vz*(1.0f-work->rout_num));

	work->kas_data[loop].max_t =
	    (kas_data_near[loop].max_t*work->rout_num) + (kas_data_far[loop].max_t*(1.0f-work->rout_num));
    }

}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void view_auto_root						*/
/*	引数:	Work	*work							*/
/*	説明:	ルートを自動表示						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void view_auto_root_ks_bs(ACRO_POINT_EX *acropoint, int number, int scale, int maxnum){
    FVECTOR	*FROM_POS;
    FVECTOR	*TO_POS;
    FVECTOR	FROM_VEC;
    FVECTOR	TO_VEC;
    short num_plus;

    num_plus = number+1;
    if (num_plus >= maxnum){
	num_plus = 0;
    }

    if (acropoint == NULL){
	printf ("Error in kasmove \n");
	return;
    }
    
    FROM_POS = &(acropoint[number].acro.position);
    FROM_VEC = acropoint[number].acro.speed;

    TO_POS = &(acropoint[num_plus].acro.position);
    TO_VEC = acropoint[num_plus].acro.speed;

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
static void hermite_move_ks_bs(Kas_Work *work, ACRO_POINT_EX *acropoint, int number, int max_num, float time){		
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
    if (num_plus >= max_num){
	num_plus = 0;
    }

    if (acropoint == NULL){
	printf ("Error in kasmove \n");
	return;
    }
    
    // *****行動を決定
    if (acropoint[number].time != -1.0f){
	if (action_flag!=number){
	    if (acropoint[number].time < 1.0f*time/acropoint[number].max_t){
		
		switch (acropoint[number].action){
		    
		case 1:{

		    if (work->item_reserve_flag == 1){
			if (har_gameover_check_ok()){
			    work->set_snake = SET_SNK_ITEM;
			    work->item_drop = ON;
			    work->item_aim_pos.vx = KAS_ITEM_DROP_POS.vx;
			    work->item_aim_pos.vy = KAS_ITEM_DROP_POS.vy;
			    work->item_aim_pos.vz = KAS_ITEM_DROP_POS.vz;
			    work->item_reserve_flag = 2;
			}
		    }

		    printf ("  			ITEM drop stop\n");

		}
		    break;
		case 2:
		    break;
		case 3:
		    break;
		case 4:
		    break;
		}
		action_flag=number;
	    }
	}
    }
    else {
	action_flag=-1;
    }
    
    
    FROM_POS = &(acropoint[number].acro.position);
    FROM_VEC = acropoint[number].acro.speed;
    
    TO_POS = &(acropoint[num_plus].acro.position);
    TO_VEC = acropoint[num_plus].acro.speed;
    
    _sceVu0ScaleVector(&FROM_VEC, &FROM_VEC, 1.0f/TIME_BASE);
    _sceVu0ScaleVector(&TO_VEC, &TO_VEC, 1.0f/TIME_BASE);
    
    MT_HermiteLerpVecScale( &work->control.mov, 
			    FROM_POS,
			    TO_POS,
			    &FROM_VEC,
			    &TO_VEC,
			    1.0f*time/acropoint[number].max_t,
			    (float)acropoint[number].max_t);
    
#ifdef DEBUG_MODE    
    if (view_flg == ON){
	float max_time0;
	float max_time1;
	_sceVu0CopyVector(&verts_from[0], &acropoint[number].acro.position);
	_sceVu0CopyVector(&verts_from[1], &acropoint[number].acro.position);
	verts_from[0].vy += 10000.0f;
	verts_from[1].vy -= 10000.0f;
	
	_sceVu0CopyVector(&verts_to[0], &acropoint[num_plus].acro.position);
	_sceVu0CopyVector(&verts_to[1], &acropoint[num_plus].acro.position);
	verts_to[0].vy += 10000.0f;
	verts_to[1].vy -= 10000.0f;
	
	max_time0 = acropoint[number].max_t;
	max_time1 = acropoint[num_plus].max_t;
	
	_sceVu0CopyVector(&verts_har[0], (FVECTOR*)work->body.objs->objs[0].world.m[3]);
	_sceVu0CopyVector(&verts_har[1], (FVECTOR*)work->body.objs->objs[0].world.m[3]);
	verts_har[0].vy += 40000.0f;
	verts_har[1].vy -= 40000.0f;


	if (HAR_ViewKacRout == 1){
	    NewLineView(verts_from, 1, 255, 0, 0);
	    NewLineView(verts_to , 1, 0, 255, 0);
	    NewLineView(verts_har , 1, 0, 0, 255);
	    view_auto_root_ks_bs(acropoint, number, max_time0, max_num);
	    view_auto_root_ks_bs(acropoint, num_plus, max_time1, max_num);
	}
    }
#endif

    //   ******** カサッカの方向
    {
	FVECTOR tmpfrot;

	Dir_from_2VecF(&work->control.mov, &GM_PlayerFindPos, &tmpfrot);
	work->control.turn.vy = 4095 & ( short )( ( tmpfrot.vy * 2048.0F / M_PI ) - 1023.5F );


    }

}


#define GET_IN	0
#define GET_OUT	1

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_kasacka						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	カサッカ飛行							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int move_kasaka_bs(Kas_Work *work){		
    static int max_time = 0;

    make_kas_rout_bs(work);

    work->rout_num_max = 4;
    max_time = work->kas_data[work->rout_number].max_t;

    hermite_move_ks_bs(work, work->kas_data, work->rout_number, work->rout_num_max, work->rout_time);
    
    work->rout_time += TIME_BASE*work->ks_speed/(1.0f+work->rout_num);

    if (work->rout_time >= max_time){
	work->rout_time -= max_time;
	work->rout_number ++;
	if (work->rout_number >= work->rout_num_max){	// ***********終了チェック
	    work->rout_number = 0;
	}
    }
    return 0;
}



