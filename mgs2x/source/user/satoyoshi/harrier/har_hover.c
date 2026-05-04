/********************************************************************************/
/*	Har_ver.c								*/
/*	ハリアホヴァー攻撃							*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_hover.c,v 1.1.1.3 2002/11/19 11:48:21 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/


#define HAR_HIGHT (4200.0f)


#define	TURN_RIGHT	(1)
#define	TURN_LEFT      (-1)
#define HMOVE_TIMER	(work->hov_atk_timer)
#define NOW_ROUT	(work->hov_now_point)
#define ACCELL		(work->hov_accell)


#define CK_UPPER_LIMIT(_t)	\
    if (HMOVE_TIMER >= (_t)){	\
	HMOVE_TIMER = HMOVE_TIMER-(_t);	\
	NOW_ROUT += 1;		\
    }

#define CK_LOWER_LIMIT(_t)	\
    if (HMOVE_TIMER <= 0.0f){	\
	HMOVE_TIMER = (_t)+HMOVE_TIMER;\
	NOW_ROUT -= 1;		\
    }


#define check_max(_p)	\
{			\
    far_length = ((_p).vx-(GM_PlayerFindPos.vx*6))*((_p).vx-(GM_PlayerFindPos.vx*6)) +	\
	((_p).vz-GM_PlayerFindPos.vz)*((_p).vz-GM_PlayerFindPos.vz);		\
    if (far_length > max_length){						\
        max_length = far_length;						\
	far_point = loop;							\
    }										\
    loop++;									\
}
#define TIME_YUMI   (130.0f)
#define TIME_UNAKA  (100.0f)
#define TIME_DOWNUP (40.0f)
#define TIME_DNAKA  (50.0f)



#define HOV_MOV_NORMAL	0
#define HOV_MOV_TO_DR	1
#define HOV_MOV_TO_UR	2
#define HOV_MOV_TO_DL	3
#define HOV_MOV_TO_UL	4
#define HOV_MOV_DOWNR	5
#define HOV_MOV_DOWNL	6





    FVECTOR P1 = {     0.0f, 8500.0f, -170000.0f, 1.0f};		//北の端
    FVECTOR S1 = {   500.0f,    0.0f,       0.0f, 1.0f};
    FVECTOR MP2 = { 12500.0f, 6500.0f, -160000.0f, 1.0f};
    FVECTOR S2 = {     0.0f, -300.0f,     400.0f, 1.0f};
    FVECTOR P3 = { 14500.0f, 3200.0f, -150000.0f, 1.0f};		//右真中
    FVECTOR P3DMY = { 15000.0f, 3000.0f, -150000.0f, 1.0f};		//右真中
    FVECTOR S3 = {     0.0f,    0.0f,     500.0f, 1.0f};
    FVECTOR P4 = { 12500.0f, 6500.0f, -140000.0f, 1.0f};
    FVECTOR S4 = {     0.0f,  300.0f,     400.0f, 1.0f};
    FVECTOR P5 = {     0.0f, 8500.0f, -130000.0f, 1.0f};		//南の端
    FVECTOR S5 = {  -500.0f,    0.0f,       0.0f, 1.0f};
    FVECTOR P6 = {-12500.0f, 6500.0f, -140000.0f, 1.0f};
    FVECTOR S6 = {     0.0f, -300.0f,    -400.0f, 1.0f};
    FVECTOR P7 = {-14500.0f, 3200.0f, -150000.0f, 1.0f};		//左真中
    FVECTOR P7DMY = {-15000.0f, 3200.0f, -150000.0f, 1.0f};		//左真中
    FVECTOR S7 = {     0.0f,    0.0f,    -500.0f, 1.0f};
    FVECTOR P8 = {-12500.0f, 6500.0f, -160000.0f, 1.0f};
    FVECTOR S8 = {     0.0f,  300.0f,    -400.0f, 1.0f};

    FVECTOR ZERO = {     0.0f,    0.0f,       0.0f, 1.0f};
#if 0
    FVECTOR PDR1 = { 11500.0f, -900.0f, -158000.0f, 1.0f};
    FVECTOR PDR2 = { 12500.0f, -1500.0f, -150000.0f, 1.0f};		//右真中
    FVECTOR PDR3 = { 11500.0f, -900.0f, -142000.0f, 1.0f};
#else
    FVECTOR PDR1 = { 11500.0f, -400.0f, -158000.0f, 1.0f};
    FVECTOR PDR2 = { 12500.0f, -1000.0f, -150000.0f, 1.0f};		//右真中
    FVECTOR PDR3 = { 11500.0f, -400.0f, -142000.0f, 1.0f};
    FVECTOR SDR2 = {     0.0f,    0.0f,    1500.0f, 1.0f};
#endif

#if 0
    FVECTOR PDL1 = {-10500.0f, -1200.0f, -141000.0f, 1.0f};
    FVECTOR PDL2 = {-12500.0f, -1800.0f, -150000.0f, 1.0f};		//左真中
    FVECTOR PDL3 = {-10500.0f, -1200.0f, -159000.0f, 1.0f};
    FVECTOR SDL2 = {     0.0f,    0.0f,   -1500.0f, 1.0f};
#else
    FVECTOR PDL1 = {-14000.0f, 	500.0f, -142000.0f, 1.0f};
    FVECTOR PDL2 = {-12500.0f,  -400.0f, -150000.0f, 1.0f};		//左真中
    FVECTOR PDL3 = {-12000.0f,  -900.0f, -159000.0f, 1.0f};
    FVECTOR SDL2 = {     0.0f,     0.0f,   -1500.0f, 1.0f};
#endif



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void set_harrier_hov_point					*/
/*	引数:	Work	*work							*/
/*	説明:	ホバー銃撃時のハリアの場所					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_harrier_hov_point(Work *work){
    switch (NOW_ROUT){
    case 0:
	get_hermove( &work->control.mov, &P1, &MP2, &S1, &S2, HMOVE_TIMER, TIME_YUMI);
	CK_UPPER_LIMIT(TIME_YUMI)
	else if(HMOVE_TIMER <= 0.0f){
	    HMOVE_TIMER = TIME_YUMI+HMOVE_TIMER;
	    NOW_ROUT = 7;
	}	
	    break;
	case 1:
	    get_hermove( &work->control.mov, &MP2, &P3, &S2, &S3, HMOVE_TIMER, TIME_UNAKA);
	    CK_UPPER_LIMIT(TIME_UNAKA)
	    else CK_LOWER_LIMIT(TIME_YUMI)
	    break;
	case 2:
	    get_hermove( &work->control.mov, &P3, &P4, &S3, &S4, HMOVE_TIMER, TIME_UNAKA);
	    CK_UPPER_LIMIT(TIME_UNAKA)
	    else CK_LOWER_LIMIT(TIME_UNAKA)
	    break;
	case 3:
	    get_hermove( &work->control.mov, &P4, &P5, &S4, &S5, HMOVE_TIMER, TIME_YUMI);
	    CK_UPPER_LIMIT(TIME_YUMI)
	    else CK_LOWER_LIMIT(TIME_UNAKA)
	    break;
	case 4:
	    get_hermove( &work->control.mov, &P5, &P6, &S5, &S6, HMOVE_TIMER, TIME_YUMI);
	    CK_UPPER_LIMIT(TIME_YUMI)
	    else CK_LOWER_LIMIT(TIME_YUMI)
	    break;
	case 5:
	    get_hermove( &work->control.mov, &P6, &P7, &S6, &S7, HMOVE_TIMER, TIME_UNAKA);
	    CK_UPPER_LIMIT(TIME_UNAKA)
	    else CK_LOWER_LIMIT(TIME_YUMI)
	    break;
	case 6:
	    get_hermove( &work->control.mov, &P7, &P8, &S7, &S8, HMOVE_TIMER, TIME_UNAKA);
	    CK_UPPER_LIMIT(TIME_UNAKA)
	    else CK_LOWER_LIMIT(TIME_UNAKA)
	    break;
	case 7:
	    get_hermove( &work->control.mov, &P8, &P1, &S8, &S1, HMOVE_TIMER, TIME_YUMI);
	    if (HMOVE_TIMER >= TIME_YUMI){
		HMOVE_TIMER = HMOVE_TIMER - TIME_YUMI;
		NOW_ROUT = 0;
	    }
	    else CK_LOWER_LIMIT(TIME_UNAKA)
	    break;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void set_harrier_hov_sita_point					*/
/*	引数:	Work	*work							*/
/*	説明:	ホバー銃撃時のハリアの場所（下）				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_harrier_hov_sita_point(Work *work, int which){
    float sabun = HMOVE_TIMER;

#if 0    
    sat_viewp(&PDR1, 200, 0, 0);
    sat_viewp(&PDR2, 200, 0, 0);
    sat_viewp(&PDR3, 200, 0, 0);
    
    sat_viewp(&PDL1, 200, 0, 0);
    sat_viewp(&PDL2, 200, 0, 0);
    sat_viewp(&PDL3, 200, 0, 0);
#endif    
    

    HMOVE_TIMER = TIME_DNAKA*cosf((work->hov_mv_timer%3000)*M_PI*2/3000);

    if (sabun == 0.0f){
	sabun = TIME_DNAKA*cosf((work->hov_mv_timer%3000)*M_PI*2/3000);
    }

    //printf ("\nHOV1:%f\n", HMOVE_TIMER);

    sabun = HMOVE_TIMER-sabun;

    //printf ("Sub:%f\n", sabun);

    work->control.turn.vz = (int)(-200*(sabun));

    if (which){	// **********************右
	if (HMOVE_TIMER<=0.0f){
	    get_hermove( &work->control.mov, &PDR1, &PDR2, &ZERO, &SDR2, TIME_DNAKA+HMOVE_TIMER, TIME_DNAKA);
	    if (HMOVE_TIMER<-TIME_DNAKA){
		HMOVE_TIMER=-TIME_DNAKA;
	    }
	}
	else {
	    get_hermove( &work->control.mov, &PDR2, &PDR3, &SDR2, &ZERO, HMOVE_TIMER, TIME_DNAKA);
	    if (HMOVE_TIMER>TIME_DNAKA){
		HMOVE_TIMER=TIME_DNAKA;
	    }
	}
    }
    else {	// **********************左
	if (HMOVE_TIMER<=0.0f){
	    get_hermove( &work->control.mov, &PDL1, &PDL2, &ZERO, &SDL2, TIME_DNAKA+HMOVE_TIMER, TIME_DNAKA);
	    if (HMOVE_TIMER<-TIME_DNAKA){
		HMOVE_TIMER=-TIME_DNAKA;
	    }
	}
	else {
	    get_hermove( &work->control.mov, &PDL2, &PDL3, &SDL2, &ZERO, HMOVE_TIMER, TIME_DNAKA);
	    if (HMOVE_TIMER>TIME_DNAKA){
		HMOVE_TIMER=TIME_DNAKA;
	    }
	}
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void set_harrier_hov_turn					*/
/*	引数:	Work	*work							*/
/*	説明:	ホバー銃撃時のハリアの向き					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_harrier_hov_turn(Work *work){
    SVECTOR	aimrot;
    SVECTOR	tmprot;
    FVECTOR	tmppos;
    static float yturn_sp=0.0f;
    static float	round = 0.0f;
    
    round += 0.04f;

    
    if (round >= M_PI*2){
	round -= M_PI*2;
    }

    
    tmppos.vx = GM_PlayerFindPos.vx;
    tmppos.vy = 500.0f*cosf(round)+GM_PlayerFindPos.vy-800.0f;
    tmppos.vz = GM_PlayerFindPos.vz;
    
    Dir_from_2Vec(&work->gun_pos, &tmppos, &tmprot);


    if (HAR_MODE_L3 == HOV_MOV_DOWNR){		// ==============脚の下
	tmppos.vx -= 3000.0f;
    }
    else if (HAR_MODE_L3 == HOV_MOV_DOWNL){	// ==============脚の下
	tmppos.vx += 3000.0f;
    }
    else {
	tmppos.vx = 0.0f;
    }

    if ( (GM_GameLevel >= ST_LEV_EASY) &&
	 ( (HAR_MODE_L3 == HOV_MOV_DOWNR)||
	   (HAR_MODE_L3 == HOV_MOV_DOWNL) ) ){
	tmppos.vz = (work->control.mov.vz+GM_PlayerFindPos.vz)/2.0f;
    }
    else {
	tmppos.vz = -150000.0f;
    }
    
    Dir_from_2Vec(&work->gun_pos, &tmppos, &aimrot);
    
    if (tmprot.vx < 0){
	tmprot.vx = 0;
    }
    if (aimrot.vx < 0){
	aimrot.vx = 0;
    }
    else if (tmprot.vx > 330){
	tmprot.vx = 330;
    }

    

    if (GM_GameLevel >= GM_LEVEL_HARD){
	aimrot.vy = tmprot.vy;
    }

    if ( (HAR_MODE_L3 == HOV_MOV_DOWNR)||
	 (HAR_MODE_L3 == HOV_MOV_DOWNL) ){
	work->control.turn.vx = aimrot.vx;
    }
    else {
	work->control.turn.vx = tmprot.vx;
    }


    {
	int sabun = aimrot.vy - work->control.turn.vy;


	yturn_sp = yturn_sp*0.98;	
	if ( ((sabun > 5)&&(sabun < 2048)) || (sabun <= -2048) ){
	    yturn_sp += 0.3f;
	}
	else if ( ((sabun<-5)&&(sabun>-2048)) || (sabun >= 2048) ){
	    yturn_sp -= 0.3f;
	}
	else {
	    yturn_sp = yturn_sp*0.95;
	}
	work->control.turn.vy += (int)yturn_sp;

	if (work->control.turn.vy<0){
	    work->control.turn.vy += 4096;
	}
	work->control.turn.vy = work->control.turn.vy%4096;
    }
}

static float	MTURN;


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_harrier_pos						*/
/*	引数:	Work	*work							*/
/*	説明:	ホバー銃撃時にハリアーを動かす					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void move_harrier_pos(Work *work, int far_point){
    float		max_speed;

    MTURN += 0.013f;
    if (MTURN >= M_PI*2){
	MTURN -= M_PI*2;
    }
    if (1){
	int	sabun = far_point-NOW_ROUT;
	
	// *****************右廻り
	if ((2400>work->hov_mv_timer)&&(work->hov_mv_timer>1800)){
	    ACCELL += 0.03f;	    
	    if (work->l_elr_rot < 0.30f){	//エルロン
		work->l_elr_rot = work->l_elr_rot + 0.03f;
		work->r_elr_rot = work->r_elr_rot + 0.03f;
	    }
	}
	// *****************左廻り
	else if ((5100>work->hov_mv_timer)&&(work->hov_mv_timer>4500)){
	    ACCELL -= 0.03f;	    
	    if (work->l_elr_rot > -0.30f){	//エルロン
		work->l_elr_rot = work->l_elr_rot - 0.03f;
		work->r_elr_rot = work->r_elr_rot - 0.03f;
	    }
	}
	// *****************狙い廻り
	else {	// プレイヤーがいる半対側に廻りこむ
	    ACCELL = ACCELL*0.995;
	    if ( ((sabun > 0)&&(sabun < 4))||
		 (sabun <= -4) ){
		ACCELL += 0.03f;	    
		if (work->l_elr_rot < 0.30f){	//エルロン
		    work->l_elr_rot = work->l_elr_rot + 0.03f;
		    work->r_elr_rot = work->r_elr_rot + 0.03f;
		}
	    }
	    else if ( ((sabun < 0)&&(sabun > -4))||
		      (sabun >= 4) ){
		ACCELL -= 0.03f;	    
		if (work->l_elr_rot > -0.30f){	//エルロン
		    work->l_elr_rot = work->l_elr_rot - 0.03f;
		    work->r_elr_rot = work->r_elr_rot - 0.03f;
		}
	    }
	    else {
		ACCELL = ACCELL*0.98;
		work->l_elr_rot = work->l_elr_rot * 0.95f;
		work->r_elr_rot = work->r_elr_rot * 0.95f;
	    }
	}

	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    max_speed = 0.5f;
	    break;
	case ST_LEV_EASY:
	    max_speed = 0.7f;
	    break;
	case ST_LEV_NORMAL:
	    max_speed = 0.9f;
	    break;

	case ST_LEV_UPNORM:
	    max_speed = 1.05f;
	    break;

	case GM_LEVEL_HARD:
	    max_speed = 1.2f;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    max_speed = 1.4f;
	    break;
	default:
	    max_speed = 0.9f;
	    break;
	}

	if (ACCELL > max_speed){
	    ACCELL = max_speed;
	}
	if (ACCELL < -max_speed){
	    ACCELL= -max_speed;
	}
	HMOVE_TIMER = HMOVE_TIMER+ACCELL+0.5f*cosf(MTURN);
	work->control.turn.vz = -140*(ACCELL+0.5f*cosf(MTURN));
    }
    else {
	if (GV_PadDataDirect[1].status & PAD_L2){
	    HMOVE_TIMER += 1.0f;
	}
	else if (GV_PadDataDirect[1].status & PAD_L1){
	    HMOVE_TIMER -= 1.0f;
	}
	else {

	    HMOVE_TIMER += 0.4f+0.25f*cosf(MTURN);

	    
	    if ( (NOW_ROUT==far_point)||(NOW_ROUT==(far_point-1)) ||	//指定地点ならそっちを向く
		 ((NOW_ROUT==8)&&(far_point==0)) ){
	    }
	    else {
		HMOVE_TIMER += 0.6f;
	    }


	}
    }
}

#if 0
    sat_viewp(&PDR1, 200, 0, 0);
    sat_viewp(&PDR2, 200, 0, 0);
    sat_viewp(&PDR3, 200, 0, 0);

    sat_viewp(&PDL1, 200, 0, 0);
    sat_viewp(&PDL2, 200, 0, 0);
    sat_viewp(&PDL3, 200, 0, 0);

    if (far_point==0)
	sat_viewp(&P1,200, 250, 200);
    else sat_viewp(&P1,0, 250, 0);

    if (far_point==1)
	sat_viewp(&MP2, 200, 250, 200);
    else sat_viewp(&MP2, 0, 250, 0);

    if (far_point==2)
	sat_viewp(&P3, 200, 250, 200);
    else sat_viewp(&P3, 0, 250, 0);

    if (far_point==3)
	sat_viewp(&P4, 200, 250, 200);
    else sat_viewp(&P4, 0, 250, 0);

    if (far_point==4)
	sat_viewp(&P5, 200, 250, 200);
    else sat_viewp(&P5, 0, 250, 0);

    if (far_point==5)
	sat_viewp(&P6, 200, 250, 200);
    else sat_viewp(&P6, 0, 250, 0);

    if (far_point==6)
	sat_viewp(&P7, 200, 250, 200);
    else sat_viewp(&P7, 0, 250, 0);

    if (far_point==7)
	sat_viewp(&P8, 200, 250, 200);
    else sat_viewp(&P8, 0, 250, 0);
#endif


#define TURN_R_FLAG 2000

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void hover_gun_sub						*/
/*	引数:	Work	*work							*/
/*	返値:	int	0: 処理中						*/
/*			1: 終了							*/
/*	説明:	ホバー銃撃							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int hover_gun_sub(Work *work){
    int	far_point = 0;
    //====================== プレイヤーが同じ位置にジッとしていたら回り込む
    static FVECTOR pc_save_pos[20];	//===為に使う保存場所
    static int SAME_POS_COUNT=0;//===為に使うカウンター
    static int MAWARI_FLAG=0;	//===為に使うフラグ

    work->hov_mv_timer += TIME_BASE;

    if (work->hov_mv_timer>6000){
	work->hov_mv_timer -= 6000;
    }
    if (HAR_MODE_L3 == HOV_MOV_NORMAL){
	if (MAWARI_FLAG){
	    work->hov_mv_timer = TURN_R_FLAG;
	}
    }
    {	//最大値チェック
	int	far_length;
	int	max_length = 0;
	int	loop=0;

	check_max(P1)
	check_max(MP2)
	check_max(P3DMY)
	check_max(P4)
	check_max(P5)
	check_max(P6)
	check_max(P7DMY)
	check_max(P8)
    }

    switch(HAR_MODE_L3){
    case HOV_MOV_NORMAL:	// ==========脚の上
	move_harrier_pos(work, far_point);
	set_harrier_hov_point(work);	//ハリアーの場所を確定
	// =================================================
	if (GM_PlayerFindPos.vy < -1000.0f){ 	// ********下に移動
	    if ((work->control.mov.vx > 11000.0f)&&(MAWARI_FLAG != HOV_MOV_DOWNR)){	// *****十分に右にいる
		ACCELL = 0.0f;
		HAR_MODE_L3 = HOV_MOV_TO_DR;
		work->l3_timer = 0;
		work->her_fromp = work->control.mov;
		HMOVE_TIMER = 0.0f;
	    }
	    else if ((work->control.mov.vx < -11000.0f)&&(MAWARI_FLAG != HOV_MOV_DOWNL)){	// *****十分に左にいる
		ACCELL = 0.0f;
		HAR_MODE_L3 = HOV_MOV_TO_DL;
		work->l3_timer = 0;
		work->her_fromp = work->control.mov;
		HMOVE_TIMER = 0.0f;
	    }
	}
	break;

    case HOV_MOV_TO_DR:	// ********下に移動中 右
	if (work->control.mov.vz > -150000.0f){
	    get_hermove(&work->control.mov, &work->her_fromp, &PDR3,
			&ZERO, &ZERO, work->l3_timer*1.0f, 600.0f);
	    work->l3_timer += TIME_BASE;
	    if (work->l3_timer > 120*5){	// ****つなぎ
		work->hov_mv_timer = 0;
		ACCELL = 0.0f;
		HAR_MODE_L3 = HOV_MOV_DOWNR;
	    }
	}
	else {
	    get_hermove(&work->control.mov, &work->her_fromp, &PDR1,
			&ZERO, &ZERO, work->l3_timer*1.0f, 600.0f);
	    work->l3_timer += TIME_BASE;
	    if (work->l3_timer > 120*5){	// ****つなぎ
		work->hov_mv_timer = 1500;
		ACCELL = 0.0f;
		HAR_MODE_L3 = HOV_MOV_DOWNR;
	    }
	}
	break;

    case HOV_MOV_TO_DL:	// ********下に移動中 左
	if (work->control.mov.vz < -150000.0f){
	    get_hermove(&work->control.mov, &work->her_fromp, &PDL3,
			&ZERO, &ZERO, work->l3_timer*1.0f, 600.0f);
	    work->l3_timer += TIME_BASE;
	    if (work->l3_timer > 120*5){	// ****つなぎ
		work->hov_mv_timer = 0;
		ACCELL = 0.0f;
		HAR_MODE_L3 = HOV_MOV_DOWNL;
	    }
	}
	else {
	    get_hermove(&work->control.mov, &work->her_fromp, &PDL1,
			&ZERO, &ZERO, work->l3_timer*1.0f, 600.0f);
	    work->l3_timer += TIME_BASE;
	    if (work->l3_timer > 120*5){	// ****つなぎ
		work->hov_mv_timer = 1500;
		ACCELL = 0.0f;
		HAR_MODE_L3 = HOV_MOV_DOWNL;
	    }
	}
	break;

    case HOV_MOV_TO_UR:
	get_hermove(&work->control.mov, &work->her_fromp, &P3,
		    &ZERO, &ZERO, work->l3_timer*1.0f, 600.0f);
	work->l3_timer += TIME_BASE;
	if (work->l3_timer > 120*5){	// ****つなぎ
	    work->hov_mv_timer = 0;
	    ACCELL = 0.0f;
	    HAR_MODE_L3 = HOV_MOV_NORMAL;
	    HMOVE_TIMER = 0;
	    NOW_ROUT = 2;
	}
	break;

    case HOV_MOV_TO_UL:
	get_hermove(&work->control.mov, &work->her_fromp, &P7,
		    &ZERO, &ZERO, work->l3_timer*1.0f, 600.0f);
	work->l3_timer += TIME_BASE;
	if (work->l3_timer > 120*5){	// ****つなぎ
	    work->hov_mv_timer = 0;
	    ACCELL = 0.0f;
	    HAR_MODE_L3 = HOV_MOV_NORMAL;
	    HMOVE_TIMER = 0;
	    NOW_ROUT = 6;
	}
	break;
	
    case HOV_MOV_DOWNR:
	set_harrier_hov_sita_point(work, 1);	//ハリアーの場所を確定
	// =================================================
	if (MAWARI_FLAG == HOV_MOV_DOWNL){
	    MAWARI_FLAG = 0;
	    SAME_POS_COUNT = 0;
	}
	if ((GM_PlayerFindPos.vy > 0.0f)||(MAWARI_FLAG)){ 	// ********上に移動
	    ACCELL = 0.0f;
	    HAR_MODE_L3 = HOV_MOV_TO_UR;
	    work->l3_timer = 0;
	    work->her_fromp = work->control.mov;
	    HMOVE_TIMER = 0.0f;
	}
	break;

    case HOV_MOV_DOWNL:
	set_harrier_hov_sita_point(work, 0);	//ハリアーの場所を確定
	// =================================================
	if (MAWARI_FLAG == HOV_MOV_DOWNR){
	    MAWARI_FLAG = 0;
	    SAME_POS_COUNT = 0;
	}
	if ((GM_PlayerFindPos.vy > 0.0f)||(MAWARI_FLAG)){ 	// ********上に移動
	    ACCELL = 0.0f;
	    HAR_MODE_L3 = HOV_MOV_TO_UL;
	    work->l3_timer = 0;
	    work->her_fromp = work->control.mov;
	    HMOVE_TIMER = 0.0f;
	}
	break;
    }
    //===================回り込みチェック
    if ( (HAR_MODE_L3==HOV_MOV_DOWNR)||(HAR_MODE_L3==HOV_MOV_DOWNL) ){
	int	MAX_SAME_POS_COUNT;

	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    MAX_SAME_POS_COUNT = 1700;
	    break;
	case ST_LEV_EASY:
	    MAX_SAME_POS_COUNT = 1700;
	    break;
	case ST_LEV_NORMAL:
	    MAX_SAME_POS_COUNT = 1700;
	    break;

	case ST_LEV_UPNORM:
	    MAX_SAME_POS_COUNT = 1600;
	    break;

	case GM_LEVEL_HARD:
	    MAX_SAME_POS_COUNT = 1500;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    MAX_SAME_POS_COUNT = 1300;
	    break;
	default:
	    MAX_SAME_POS_COUNT = 1200;
	    break;
	}


	if ( (fabs(pc_save_pos[0].vx - GM_PlayerFindPos.vx)<200.0f)&&
	     (fabs(pc_save_pos[0].vz - GM_PlayerFindPos.vz)<200.0f) ){
	    if (SAME_POS_COUNT < MAX_SAME_POS_COUNT){
		SAME_POS_COUNT += TIME_BASE;
	    }
	}
	else {
	    SAME_POS_COUNT = SAME_POS_COUNT * 0.9f; 
	}
	if (SAME_POS_COUNT >= MAX_SAME_POS_COUNT){
	    if (MAWARI_FLAG == 0){
		MAWARI_FLAG = HAR_MODE_L3;
	    }
	}
	else {
	    MAWARI_FLAG = 0;
	}
    }

    set_harrier_hov_turn(work);		//ハリアーの向きを確定

    {
	int	loop;
	pc_save_pos[19] = GM_PlayerFindPos;
	for(loop=0; loop<19; loop++){
	    pc_save_pos[loop]=pc_save_pos[loop+1];
	}
    }


    if (work->damage_num >= 3){		// 一定回数以上のダメージを受けたら終了
	return 1;
    }
    return 0;
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void init_hoverin						*/
/*	引数:	Work	*work							*/
/*	説明:	ホヴァー							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int init_hovering(Work* work){
    FVECTOR P1 = {0.0f,  5000.0f, -230000.0f, 1.0f};
    FVECTOR P2 = {31000.0f, 4000.0f, -180000.0f, 1.0f};
    FVECTOR P3 = {12581.0f, 6500.0f, -160000.0f, 1.0f};

    FVECTOR S1 = {200.0f, 0.0f,  300.0f, 1.0f};
    FVECTOR S2 = {0.0f, 0.0f, 200.0f, 1.0f};
    FVECTOR S3 = {0.0f, 0.0f, 60.0f, 1.0f};
    
    work->l2_timer += TIME_BASE;

    switch(HAR_MODE_L3){
    case 0:
	get_hermove(&work->control.mov, &work->her_fromp, &P1,
		    &work->her_froms, &S1, work->l2_timer*1.0f, 500.0f);
	
	Dir_from_2Vec(&work->control.mov, &P1, &work->control.turn);
	if (work->l2_timer >= 100 * 5){
	    HAR_MODE_L3++;
	    work->l2_timer = 0;
	    switch (RAND(3)){	//=====================しゃべる
	    case 0:		//蜂のす
		SetHarSE(work, SD_V_SOLAT301, 150*5);
		break;
	    case 1:		//でぇ
		SetHarSE(work, SD_V_SOLAT201, 180*5);
		break;
	    case 2:		//くたばれ
		SetHarSE(work, SD_V_SOLAT401, 130*5);
		break;
	    }
	}
	break;

    case 1:
	get_hermove(&work->control.mov, &P1, &P2,
		    &S1, &S2, work->l2_timer*1.0f, 500.0f);
	if (GM_GameLevel == GM_LEVEL_HARD){
	    FVECTOR tmppos;
	    tmppos = GM_PlayerFindPos;
	    tmppos.vz = (work->control.mov.vz+GM_PlayerFindPos.vz)/2.0f;
	    tmppos.vy = GM_PlayerFindPos.vy-2000.0f;    
	    Dir_from_2Vec(&work->gun_pos, &tmppos, &work->control.turn);
	}
	else if (GM_GameLevel >= GM_LEVEL_EXTREME){
	    FVECTOR tmppos;
	    tmppos = GM_PlayerFindPos;
	    tmppos.vy = GM_PlayerFindPos.vy-5000.0f;    
	    Dir_from_2Vec(&work->gun_pos, &tmppos, &work->control.turn);
	}
	else {
	    work->control.turn.vx = 280;
	    work->control.turn.vy = -1024;
	    work->control.turn.vz = -100;
	}
	if (work->l2_timer >= 100 * 5){
	    HAR_MODE_L3++;
	    work->l2_timer = 0;
	}
	break;

    case 2:
	if ( GM_GameLevel >= GM_LEVEL_HARD){
	    SET_GUN_MODE(GUN_FIRE_FLG_DELAY, GUN_FIRE_FLG_HIT);
	}
	get_hermove(&work->control.mov, &P2, &P3,
		    &S2, &S3, work->l2_timer*1.0f, 700.0f);
	if (work->l2_timer >= 140 * 5){
	    SET_GUN_MODE(GUN_FIRE_FLG_DELAY, GUN_FIRE_FLG_HIT);
	    work->l2_timer = 0;
	    return 1;
	}
	break;
    }

    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void end_hovering						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int end_hovering(Work* work){
    FVECTOR P1 = {0.0f,  5000.0f, -50000.0f, 1.0f};
    FVECTOR P2 = {0.0f, 0000.0f, -80000.0f, 1.0f};
    FVECTOR P3 = {0.0f, 25000.0f, -120000.0f, 1.0f};

    FVECTOR S1 = {0.0f, 0.0f, 100.0f, 1.0f};
    FVECTOR S2 = {0.0f, 0.0f,500.0f, 1.0f};
    FVECTOR S3 = {0.0f, 500.0f, 100.0f, 1.0f};

    P3.vx = GM_PlayerFindPos.vx;
    P3.vz = GM_PlayerFindPos.vz;

    if ( (135*5 > work->l2_timer)&&(work->l2_timer >= (135*5-TIME_BASE)) ){
	printf ("AS1\n");
	GM_SeSetMode (SD_E_H_HOVAS1, &work->control.mov, GM_SEMODE_BOMB);
    }


    if (work->l2_timer < 150 * 5){
	get_hermove(&work->control.mov, &work->her_fromp, &P3,
		    &DG_ZeroVector, &S3, work->l2_timer*1.0f, 750.0f);
    }
    else if (work->l2_timer < (150+100) * 5){
	get_hermove(&work->control.mov, &P3, &P2,
		    &S3, &S2, (work->l2_timer-750)*1.0f, 500.0f);
    }
    else if (work->l2_timer < (250+100) * 5){
	get_hermove(&work->control.mov, &P2, &P1,
		    &S2, &S1, (work->l2_timer-1250)*1.0f, 500.0f);
    }
    else {
	return 1;
    }
    work->l2_timer += TIME_BASE;

    if (work->control.turn.vy>2048){
	work->control.turn.vy -= 4096;
    }
    if (work->control.turn.vy<-2048){
	work->control.turn.vy += 4096;
    }

    work->control.rot.vx = work->control.turn.vx = work->control.turn.vx* 0.98;
    work->control.rot.vy = work->control.turn.vy = work->control.turn.vy* 0.98;
    work->control.rot.vz = work->control.turn.vz = work->control.turn.vz* 0.98;

    return 0;
}




/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void control_hover						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int control_hover(Work* work){
    static int end_timer;

    //printf("VZ: %d %d %d %f", HAR_MODE_L2, HAR_MODE_L3, work->control.turn.vz, HMOVE_TIMER);


    switch(HAR_MODE_L2){
    case 0:
	work->her_fromp = work->control.mov;
	work->her_froms = work->Speed;
	work->l2_timer = 0;
	har_call_event_proc(work, HAR_EVENT_VULC_START );
	HAR_MODE_L3=0;
	HAR_MODE_L2++;
    case 1:

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


	if (init_hovering(work)){
	    HAR_MODE_L2++;
	    HAR_MODE_L3 = 0;	//
	    work->l2_timer = 0;
	    work->l3_timer = 0;	//  **hover_gunの中でダメージの後の隙タイマ
	    work->hover_rot = 0.0f;
	    work->target_pos = work->control.mov;
	    work->hover_rot = 0;
	    work->hov_nowroty = M_PI / 2.0f;
	    work->damage_num = 0;
	    work->control.step = DG_ZeroVector;
	    HMOVE_TIMER = 0;
	    work->hov_mv_timer = 0;
	    NOW_ROUT = 1;
	    ACCELL = 0.0f;
	    har_call_event_proc(work, HAR_EVENT_VULC_FIRE );
	    end_timer = 0;
	    MTURN = 0.0f;
	}
	break;

    case 2:

	if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	    work->noz_fr_rot = work->noz_fr_rot - 0.003f;
	}
	else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	    work->noz_fr_rot = work->noz_fr_rot + 0.003f;
	}


	if ( (hover_gun_sub(work)) || (end_timer > 25*300) ){
	    work->her_fromp = work->control.mov;
	    work->l2_timer = 0;
	    HAR_MODE_L2++;
	    SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	    GM_SeSetMode (SD_E_H_HOVUP1, &work->control.mov, GM_SEMODE_BOMB);
	    har_call_event_proc(work, HAR_EVENT_VULC_FIREEND );
	}
	if ((17*300)>=end_timer && end_timer>(17*300-TIME_BASE) && work->damage_num <= 1){
	    SetSolVoice( work, 13, 3);	    // vc104530	//そんなものか！13
	}
	end_timer += TIME_BASE;
	break;

    case 3:
	work->l_elr_rot = work->l_elr_rot * 0.95f;
	work->r_elr_rot = work->r_elr_rot * 0.95f;
	work->r_flap_rot = work->r_flap_rot * 0.95f;//フラップ
	work->cover_rot = work->cover_rot * 0.96f;//カバー
	work->noz_fr_rot = work->noz_fr_rot * 0.98f;//ノズル
	work->pan_hokan = ON;			//パンに補間を入れる
	if (end_hovering(work)){
	    work->burn_flag = OFF;
	    har_call_event_proc(work, HAR_EVENT_VULC_END );
	    return 1;
	}
	break;
    }
    //printf("  %d %d\n", HAR_MODE_L3, work->control.turn.vz);
    return 0;
}







