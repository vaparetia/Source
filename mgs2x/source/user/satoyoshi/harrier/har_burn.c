/********************************************************************************/
/*	Har_burn.c								*/
/*	ハリア焼き攻撃								*/
/*	2001/07/1 H.Satoyoshi							*/
/*	$Id: har_burn.c,v 1.1.1.3 2002/11/19 11:48:19 Yoshizawa1 Exp $		*/
/********************************************************************************/

static int   BURN_TUIBI_SPEED;
static float ACCEL_SPEEDX;	//Ｘ方向加速度
static float ACCEL_SPEEDZ;	//Z方向加速度
static float RADIUS;		//減速度 正確さに関わる
static float MV_DIST;

static int   YURE;
static float BREAK;		//行き過ぎ防止ブレーキ

static float TURN_VZ;		//揺れ幅の設定
static float TURN_VX;

static int   BURN_END_TIME;	//終了後の隙

#if 0
#define		BURN_TUIBI_SPEED	(39)

#define		ACCEL_SPEEDX		(0.2f)
#define		ACCEL_SPEEDZ		(0.3f)

#define		RADIUS			(0.998f)
#define		MV_DIST			(50.0f)

#define		YURE			(5)
#define		BREAK			(0.99f)

#define		TURN_VZ			(3.8f)
#define		TURN_VX			(3.5f)
#endif


/********************************************************************************/
/*	include files								*/
/********************************************************************************/
extern int OK_LOCAL_WIND2_FLAG;
extern int OK_SetDataLocalWind( FVECTOR *boundary0, FVECTOR *boundary1, float intense, SVECTOR *rot );
extern int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void init_burn_game_level					*/
/*	引数:	Work	*work							*/
/*	説明:	焼き攻撃難易度調整						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void init_burn_game_level(Work *work)
{

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	BURN_TUIBI_SPEED = 39;
	ACCEL_SPEEDX = 0.1f;
	ACCEL_SPEEDZ = 0.15f;
	RADIUS	 = 0.9995f;
	BREAK	 = 0.995f;
	TURN_VZ	 = 4.0f;
	TURN_VX	 = 1.9f;
	BURN_END_TIME = 700;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;
    case ST_LEV_EASY:
	BURN_TUIBI_SPEED = 35;
	ACCEL_SPEEDX = 0.17f;
	ACCEL_SPEEDZ = 0.20f;
	RADIUS	 = 0.9985f;
	BREAK	 = 0.995f;
	TURN_VZ	 = 3.9f;
	TURN_VX	 = 1.0f;
	BURN_END_TIME = 500;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;

    case ST_LEV_NORMAL:
	BURN_TUIBI_SPEED = 20;
	ACCEL_SPEEDX = 0.5f;
	ACCEL_SPEEDZ = 0.8f;
	RADIUS	 = 0.994f;
	BREAK	 = 0.983f;
	TURN_VZ	 = 3.5f;
	TURN_VX	 = 0.7f;
	BURN_END_TIME = 360;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;

    case ST_LEV_UPNORM:
	BURN_TUIBI_SPEED = 20;
	ACCEL_SPEEDX = 0.65f;
	ACCEL_SPEEDZ = 1.0f;
	RADIUS	 = 0.994f;
	BREAK	 = 0.979f;
	TURN_VZ	 = 2.7f;
	TURN_VX	 = 0.9f;
	BURN_END_TIME = 330;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;


    case GM_LEVEL_HARD:
	BURN_TUIBI_SPEED = 20;
	ACCEL_SPEEDX = 0.8f;
	ACCEL_SPEEDZ = 1.2f;
	RADIUS	 = 0.994f;
	BREAK	 = 0.975f;
	TURN_VZ	 = 2.0f;
	TURN_VX	 = 1.1f;
	BURN_END_TIME = 300;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	BURN_TUIBI_SPEED = 1;
	ACCEL_SPEEDX = 1.5f;
	ACCEL_SPEEDZ = 2.3f;
	RADIUS	 = 0.99f;
	BREAK	 = 0.95f;
	TURN_VZ	 = 1.3f;
	TURN_VX	 = 1.5f;
	BURN_END_TIME = 100;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;
    default:
	BURN_TUIBI_SPEED = 30;
	ACCEL_SPEEDX = 0.2f;
	ACCEL_SPEEDZ = 0.3f;
	RADIUS	 = 0.998f;
	BREAK	 = 0.99f;
	TURN_VZ	 = 3.8f;
	TURN_VX	 = 3.5f;
	BURN_END_TIME = 360;

	MV_DIST	 = 50.0f;
	YURE	 = 5;
	break;
    }


}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void act_burning						*/
/*	引数:	Work	*work							*/
/*	説明:	焼き攻撃							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int act_burning(Work *work)
{
    short		loop;
    FMATRIX		tmpmat;
    FVECTOR		tmpvec;
    static FVECTOR	poslist[40];
    float		step;
    static FVECTOR	speed;		

    {	// ==============================髪の毛なびきのローカル風設定
	FVECTOR bound0 = {5000.0f, 5000.0f, 5000.0f, 1.0f};
	FVECTOR bound1 = {-5000.0f, -5000.0f, -5000.0f, 1.0f};
	SVECTOR tmprot;

	OK_LOCAL_WIND2_FLAG = 0;

	bound0.vx += GM_PlayerFindPos.vx;
	bound0.vy += GM_PlayerFindPos.vy;
	bound0.vz += GM_PlayerFindPos.vz;
	bound1.vx += GM_PlayerFindPos.vx;
	bound1.vy += GM_PlayerFindPos.vy;
	bound1.vz += GM_PlayerFindPos.vz;

	Dir_from_2Vec(&work->control.mov, &GM_PlayerFindPos, &tmprot);
	{
	    float tmp = 1500.0f - GM_PlayerFindPos.vy;
	    if (tmp < 1000.0f){
		tmp = 1000.0f;
	    }
	    OK_SetDataLocalWind( &bound0, &bound1, 15000.0f*1000.0f/tmp , &tmprot );
	}
    }

    poslist[BURN_TUIBI_SPEED] = GM_PlayerFindPos;


    for (loop=0; loop<39; loop++){
	poslist[loop] = poslist[loop+1];
    }

    for (loop=0; loop<3; loop++){
	work->azi_m_time[loop] += TIME_BASE;
	if (work->azi_m_time[loop] > work->azi_m_time_end[loop]){
	    work->azi_m_time[loop] = 0;
	    work->azi_m_time_end[loop] = (RAND(30)+60)*TIME_BASE;
	    work->azi_mov_old[loop] = work->azi_mov_now[loop];

	    if (loop == 1){
		work->azi_mov_now[loop] = (RAND(59)-29)*40;
	    }
	    else if (loop == 0){
		work->azi_mov_now[loop] = (RAND(YURE))*50;
	    }
	    else {
		work->azi_mov_now[loop] = (RAND(YURE)-(YURE/2))*50;
	    }


	}
	
	step = (1.0f-sinf( (M_PI*work->azi_m_time[loop]/work->azi_m_time_end[loop])-M_PI/2.0f))/2.0f;
	if (loop == 0){
	    tmpvec.vx = work->azi_mov_now[loop]*(1.0f-step) + work->azi_mov_old[loop]*step;
	}
	if (loop == 1){
	    tmpvec.vy = work->azi_mov_now[loop]*(1.0f-step) + work->azi_mov_old[loop]*step;
	}
	if (loop == 2){
	    tmpvec.vz = work->azi_mov_now[loop]*(1.0f-step) + work->azi_mov_old[loop]*step;
	}
    }    

    if (poslist[0].vx+tmpvec.vx - work->control.mov.vx > MV_DIST){
	if (speed.vx < 0){
	    speed.vx = speed.vx * BREAK;
	}
	speed.vx += ACCEL_SPEEDX;

	if (work->l_elr_rot > -0.12f){	//エルロン
	    work->l_elr_rot = work->l_elr_rot - 0.003f;
	    work->r_elr_rot = work->r_elr_rot - 0.003f;
	}
	if (work->lader_rot > -0.12f){
	    work->lader_rot = work->lader_rot - 0.005f; 
	}
    }
    else if (poslist[0].vx+tmpvec.vx - work->control.mov.vx < -MV_DIST){
	if (speed.vx > 0){
	    speed.vx = speed.vx * BREAK;
	}
	speed.vx -= ACCEL_SPEEDX;

	if (work->l_elr_rot < 0.12f){	//エルロン
	    work->l_elr_rot = work->l_elr_rot + 0.003f;
	    work->r_elr_rot = work->r_elr_rot + 0.003f;
	}
	if (work->lader_rot < 0.12f){
	    work->lader_rot = work->lader_rot + 0.005f; 
	}
    }
    else {	//エルロン
	work->l_elr_rot = work->l_elr_rot * 0.95f;
	work->r_elr_rot = work->r_elr_rot * 0.95f;
	work->lader_rot = work->lader_rot * 0.95f; 
    }


    if (poslist[0].vz+tmpvec.vz - work->control.mov.vz > MV_DIST){
	if (speed.vz < 0){
	    speed.vz = speed.vz * BREAK;
	}
	speed.vz += ACCEL_SPEEDZ;

	if (work->talewing_rot < 0.09f){	//水平尾翼
	    work->talewing_rot = work->talewing_rot + 0.002f; 
	}

    }
    else if (poslist[0].vz+tmpvec.vz - work->control.mov.vz < -MV_DIST){
	if (speed.vz > 0){
	    speed.vz = speed.vz * BREAK;
	}
	speed.vz -= ACCEL_SPEEDZ;

	if (work->talewing_rot > -0.09f){	//水平尾翼
	    work->talewing_rot = work->talewing_rot - 0.002f; 
	}

    }
    else {
	work->talewing_rot = work->talewing_rot * 0.95f; 
    }

    speed.vx = speed.vx * RADIUS;
    speed.vz = speed.vz * RADIUS;

    work->control.mov.vx += speed.vx;
    work->control.mov.vz += speed.vz;

    work->control.turn.vz = speed.vx*TURN_VZ;
    work->control.turn.vx = -speed.vz*TURN_VX;










    {	//煙り発生位置更新
	_sceVu0CopyVector(&work->burn_smoke_pos, &work->control.mov);
	work->burn_smoke_pos.vy = -1000.0f;
    }
    for (loop=0; loop<2; loop++){	// ***********攻撃ターゲット発生
	GM_MoveTarget2 (&work->noz_of_tgt[loop], &work->nozel[loop+2]);
	GM_PutTarget(&work->noz_of_tgt[loop]);

#ifdef DEBUG_MODE
	if (HAR_TGT_VIEW==1){
	    NewTargetView2(&work->noz_of_tgt[loop],
			   255, 50, 50);
	}
#endif

    }

    for (loop=0; loop<2; loop++){
	FVECTOR tmpvec = {0.0f, 0.0f, 1000.0f, 0.0f};
	_sceVu0CopyMatrix(&tmpmat, &work->nozel[loop+2]);
	_sceVu0CopyVector((FVECTOR*)tmpmat.m[3], &DG_ZeroVector);
	DG_SetPos(&tmpmat);
	DG_MovePos(&tmpvec);
	DG_GetPos(&tmpmat);
	_sceVu0CopyVector(&work->burn_dir[loop], (FVECTOR*)tmpmat.m[3] );
	_sceVu0Normalize (&work->burn_dir[loop], &work->burn_dir[loop]);
    }
    har_burning_se(work);



    {
	float	aimvy;
	// 高さ   3800 - 
	if (work->control.mov.vz <= -150000.0f){
	    aimvy = 4000.0f +
		(7800.0f - 4000.0f)*(-150000.0f-work->control.mov.vz)/(162000.0f-150000.0f);
		//		(7000.0f - 3800.0f)*(-150000.0f-work->control.mov.vz)/(162000.0f-150000.0f);
	    if (work->control.mov.vy > 7000.0f){
		aimvy = 7000.0f;
	    }
	    
	    
	    
	}
	else if (work->control.mov.vz >= -146000.0f){
	    aimvy = 4000.0f +
		(7800.0f - 4000.0f)*(work->control.mov.vz+146000.0f)/(-142500.0f+146000.0f);
	    //		(7000.0f - 3800.0f)*(work->control.mov.vz+146000.0f)/(-142500.0f+146000.0f);
	    if (work->control.mov.vy > 7000.0f){
		aimvy = 7000.0f;
	    }
	}
	else {
	    aimvy = 3800.0f;
	}

	if (aimvy - work->control.mov.vy > 60.0f){
	    work->control.mov.vy += 60.0f;
	}
	else if (aimvy - work->control.mov.vy < -60.0f){
	    work->control.mov.vy -= 60.0f;
	}
	else {
	    work->control.mov.vy = aimvy;
	}
    }

    if (work->r_flap_rot < 0.7f){
	work->r_flap_rot = work->r_flap_rot + 0.007f;//フラップ
    }
    if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	work->noz_fr_rot = work->noz_fr_rot - 0.003f;
    }
    else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	work->noz_fr_rot = work->noz_fr_rot + 0.003f;
    }
    if (work->cover_rot > -0.7f){
	work->cover_rot = work->cover_rot - 0.007f;//カバー
    }
    
    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void init_burning						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int init_burning(Work* work){
    FVECTOR P1 = {0.0f,  5000.0f,  -70000.0f, 1.0f};
    FVECTOR P2 = {0.0f, 30000.0f, -100000.0f, 1.0f};
    FVECTOR P3 = {0.0f, 10000.0f, -150000.0f, 1.0f};

    FVECTOR S1 = {0.0f, 0.0f, -500.0f, 1.0f};
    FVECTOR S2 = {0.0f, 0.0f,-1000.0f, 1.0f};
    FVECTOR S3 = {0.0f,-500.0f,  0.0f, 1.0f};

    P3.vx = GM_PlayerFindPos.vx;
    P3.vz = GM_PlayerFindPos.vz;

    work->l2_timer += TIME_BASE;



    if ( (150*5 > work->l2_timer)&&(work->l2_timer >= (150*5-TIME_BASE)) ){
	printf ("UP1\n");
	GM_SeSetMode (SD_E_H_HOVAS1, &work->control.mov, GM_SEMODE_BOMB);
    }

    if (work->l2_timer < 130 * 5){
	get_hermove(&work->control.mov, &work->her_fromp, &P1,
		    &work->her_froms, &S1, work->l2_timer*1.0f, 650.0f);

	Dir_from_2Vec(&work->control.mov, &P1, &work->control.turn);
    }
    else if (work->l2_timer < (130 + 60) * 5){
	get_hermove(&work->control.mov, &P1, &P2,
		    &S1, &S2, (work->l2_timer-650)*1.0f, 300.0f);

	if (work->r_flap_rot < 0.7){
	    work->r_flap_rot = work->r_flap_rot + 0.002f;//フラップ
	}
	if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	    work->noz_fr_rot = work->noz_fr_rot - 0.003f;
	}
	else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	    work->noz_fr_rot = work->noz_fr_rot + 0.003f;
	}


    }
    else if (work->l2_timer < (190 + 60) * 5){
	get_hermove(&work->control.mov, &P2, &P3,
		    &S2, &S3, (work->l2_timer-950)*1.0f, 300.0f);


	if (work->r_flap_rot < 0.7){
	    work->r_flap_rot = work->r_flap_rot + 0.002f;//フラップ
	}
	if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
	    work->noz_fr_rot = work->noz_fr_rot - 0.003f;
	}
	else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
	    work->noz_fr_rot = work->noz_fr_rot + 0.003f;
	}


    }
    else {
	return 1;
    }
    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void end_burning						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int end_burning(Work* work){
    FVECTOR P1 = {0.0f,  5000.0f, -230000.0f, 1.0f};
    FVECTOR P2 = {0.0f, 30000.0f, -200000.0f, 1.0f};
    FVECTOR P3 = {0.0f, 17000.0f, -150000.0f, 1.0f};

    FVECTOR S1 = {0.0f, 0.0f, -500.0f, 1.0f};
    FVECTOR S2 = {0.0f, 0.0f,-1000.0f, 1.0f};
    FVECTOR S3 = {0.0f, 0.0f,  0.0f, 1.0f};

    P3.vx = GM_PlayerFindPos.vx;
    P3.vz = GM_PlayerFindPos.vz;

    //エルロンを元に戻す
    work->l_elr_rot = work->l_elr_rot * 0.95f;
    work->r_elr_rot = work->r_elr_rot * 0.95f;
    work->lader_rot = work->lader_rot * 0.95f; 
    work->talewing_rot = work->talewing_rot * 0.95f; //水平尾翼

    if ( (105*5 > work->l2_timer)&&(work->l2_timer >= (105*5-TIME_BASE)) ){
	printf ("AS1\n");
	GM_SeSetMode (SD_E_H_HOVAS1, &work->control.mov, GM_SEMODE_BOMB);
    }


    if (work->l2_timer < 120 * 5){
	get_hermove(&work->control.mov, &work->her_fromp, &P3,
		    &DG_ZeroVector, &S3, work->l2_timer*1.0f, 600.0f);

	work->r_flap_rot = work->r_flap_rot * 0.98f;//フラップ
	work->noz_fr_rot = work->noz_fr_rot * 0.98f;//ノズル
	work->cover_rot = work->cover_rot * 0.96f;//カバー

    }
    else if (work->l2_timer < (120+60) * 5){
	get_hermove(&work->control.mov, &P3, &P2,
		    &S3, &S2, (work->l2_timer-600)*1.0f, 300.0f);


	work->r_flap_rot = work->r_flap_rot * 0.96f;//フラップ
	work->noz_fr_rot = work->noz_fr_rot * 0.96f;//ノズル


    }
    else if (work->l2_timer < (180+60) * 5){
	get_hermove(&work->control.mov, &P2, &P1,
		    &S2, &S1, (work->l2_timer-900)*1.0f, 300.0f);


	work->r_flap_rot = work->r_flap_rot * 0.96f;//フラップ
	work->noz_fr_rot = work->noz_fr_rot * 0.96f;//ノズル



    }
    else {
	return 1;
    }
    work->l2_timer += TIME_BASE;
    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void control_burning						*/
/*	引数:	Work	*work							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int control_burning(Work* work){
    static int nige_time = 0;

    switch(HAR_MODE_L2){
    case 0:
	work->her_fromp = work->control.mov;
	work->her_froms = work->Speed;
	work->l2_timer = 0;
	har_call_event_proc(work, HAR_EVENT_BURN_START);
	init_burn_game_level(work);	//難易度設定
	HAR_MODE_L2++;
    case 1:
	if (init_burning(work)){
	    HAR_MODE_L2++;
	    work->damage_num = 0;

	    work->burn_flag = ON;
	    work->se_time = 0;
	    nige_time = 0;
	    NewHarrierVernierSmoke ( &work->burn_smoke_pos, &work->burn_flag);
	    work->control.step = DG_ZeroVector;
	    //	    switch (RAND(2)){
	    //	    case 0:
		GM_JimakuSeSetMode (SD_V_SOLAT701, &work->control.mov, GM_SEMODE_BOMB);//やけしね
		//		    break;
		//	    case 1:		//でぇあ
		//		GM_SeSetMode (SD_V_SOLAT202, &work->control.mov, GM_SEMODE_BOMB);
		//		    break;
		//	    }
	    har_call_event_proc(work, HAR_EVENT_BURN_ATACK);
	}
	break;

    case 2:
    {
	int damnum;	// ==================ダメージ耐える回数
	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    damnum = 6;
	    break;
	case ST_LEV_EASY:
	    damnum = 4;
	    break;
	case ST_LEV_NORMAL:
	    damnum = 2;
	    break;

	case ST_LEV_UPNORM:
	    damnum = 3;
	    break;


	case GM_LEVEL_HARD:
	    damnum = 3;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    damnum = 5;
	    break;
	default:
	    damnum = 2;
	    break;
	}
	
	act_burning(work);
	
	if (work->p_position.vy < -1000.0f){
	    nige_time += TIME_BASE*3;
	}
	else {
	    if (GM_GameLevel >= GM_LEVEL_EXTREME){
		nige_time += 1;
	    }
	    else {
		nige_time += TIME_BASE;
	    }
	}
	if ((nige_time > BURN_END_TIME*5*3)||(work->damage_num >= damnum)){	// 6秒以上逃げると終了
	    work->her_fromp = work->control.mov;
	    work->l2_timer = 0;
	    work->burn_flag = OFF;
	    GM_SeSetMode (SD_E_H_HOVUP1, &work->control.mov, GM_SEMODE_BOMB);
	    har_call_event_proc(work, HAR_EVENT_BURN_END);
	    HAR_MODE_L2++;
	}
    }
    break;
    
    case 3:
	work->pan_hokan = ON;			//パンに補間を入れる
	if (end_burning(work)){
	    FVECTOR bound0 = {1000000.0f, 1000000.0f, 1000000.0f, 1.0f};
	    FVECTOR bound1 = {1000001.0f, 1000001.0f, 1000001.0f, 1.0f};
	    SVECTOR tmprot;
	    Dir_from_2Vec(&work->control.mov, &GM_PlayerFindPos, &tmprot);
	    OK_SetDataLocalWind( &bound0, &bound1, 5000.0f, &tmprot );
	    return 1;
	}
	break;
    }
    return 0;
}











