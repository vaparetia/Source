/********************************************************************************/
/*	har_mpod.c								*/
/*	ハリアのミサイルポッド弾頭						*/
/*	2001/03/30 H.Satoyoshi							*/
/*	$Id: har_mpod.c,v 1.1.1.3 2002/11/19 11:48:23 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

#define	ALL_COUNT (((Mpod_Work*)work)->allcount)
#define	SHOOT_COUNT (((Mpod_Work*)work)->shootcount)

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Mpod_Work	*work						*/
/*	説明:	終了処理:ワーク解放    						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Mp_Die(Mpod_Work *work)
{
    short	loop;
    DG_DequeueComdlObjs( work->mpod_warhead );
    DG_FreeComdl( work->mpod_warhead );
    for (loop=0; loop<MPOD_MISSILE_NUM; loop++){
	GM_FreeTarget( &work->mp_tgt_body[loop]);
    }
    GM_FreeObject(&(work->mpod_body));
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void get_mp_shift						*/
/*	引数:	Mpod_Work	*work						*/
/*	説明:	shift量を取得する						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void get_mp_shift(short num, FVECTOR *shift, char which)
{
    switch(num){
    case 0:
	shift->vx = 2010.0f;
	shift->vy = -100.0f;
	shift->vz = 1430.0f;
	break;
    case 1:
	shift->vx = 1950.0f;
	shift->vy = -100.0f;
	shift->vz = 1430.0f;
	break;
    case 2:
	shift->vx = 1890.0f;
	shift->vy = -100.0f;
	shift->vz = 1430.0f;
	break;
    case 3:
	shift->vx = 1860.0f;
	shift->vy = -160.0f;
	shift->vz = 1430.0f;
	break;
    case 4:
	shift->vx = 1830.0f;
	shift->vy = -220.0f;
	shift->vz = 1430.0f;
	break;
    case 5:
	shift->vx = 1860.0f;
	shift->vy = -280.0f;
	shift->vz = 1430.0f;
	break;
    case 6:
	shift->vx = 1890.0f;
	shift->vy = -340.0f;
	shift->vz = 1430.0f;
	break;
    case 7:
	shift->vx = 1950.0f;
	shift->vy = -340.0f;
	shift->vz = 1430.0f;
	break;
    case 8:
	shift->vx = 2010.0f;
	shift->vy = -340.0f;
	shift->vz = 1430.0f;
	break;
    case 9:
	shift->vx = 2050.0f;
	shift->vy = -280.0f;
	shift->vz = 1430.0f;
	break;
    case 10:
	shift->vx = 2080.0f;
	shift->vy = -220.0f;
	shift->vz = 1430.0f;
	break;
    case 11:
	shift->vx = 2050.0f;
	shift->vy = -160.0f;
	shift->vz = 1430.0f;
	break;
    case 12:
	shift->vx = 1990.0f;
	shift->vy = -160.0f;
	shift->vz = 1430.0f;
	break;
    case 13:
	shift->vx = 1920.0f;
	shift->vy = -160.0f;
	shift->vz = 1430.0f;
	break;
    case 14:
	shift->vx = 1890.0f;
	shift->vy = -220.0f;
	shift->vz = 1430.0f;
	break;
    case 15:
	shift->vx = 1920.0f;
	shift->vy = -280.0f;
	shift->vz = 1430.0f;
	break;
    case 16:
	shift->vx = 1990.0f;
	shift->vy = -280.0f;
	shift->vz = 1430.0f;
	break;
    case 17:
	shift->vx = 2010.0f;
	shift->vy = -220.0f;
	shift->vz = 1430.0f;
	break;
    case 18:
	shift->vx = 1950.0f;
	shift->vy = -220.0f;
	shift->vz = 1430.0f;
	break;
    }
    //左右の変換
    shift->vx = which * shift->vx ;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Mpod_Work	*work						*/
/*	説明:	エフェクとの位置と方向ベクトル設定				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_posvec(short num, Mpod_Work *work, FMATRIX *world )
{
    FMATRIX	tmpmat;
    FVECTOR	tmpshift = {0.0f, 0.0f, 1000.0f, 1.0f};

    //場所を渡す
    Fvec_from_Matrix(&work->position[num], world);

    //方向ベクトルを計算
    _sceVu0CopyMatrix(&tmpmat, world);
    tmpmat.m[ 3 ][ 0 ] = 0.0f;
    tmpmat.m[ 3 ][ 1 ] = 0.0f;
    tmpmat.m[ 3 ][ 2 ] = 0.0f;
    DG_SetPos(&tmpmat);
    DG_MovePos(&tmpshift);
    DG_GetPos(&tmpmat);
    Fvec_from_Matrix(&work->vector[num], &tmpmat);
    _sceVu0Normalize(&work->vector[num], &work->vector[num]);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mp_fire_act						*/
/*	引数:	Work	*work							*/
/*	説明:	ポッドから出てこようとするロケット				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mp_fire_act(Mpod_Work *work, short loop)
{
    OBJECT	*tmp;
    FVECTOR	shift;

    DG_COMDL_POS	*pos_s;
    pos_s =  &work->mpod_warhead->pos[loop];

    RP_TIMER +=TIME_BASE;

    tmp = work->oya_control->object;

    DG_SetPos( &tmp->objs->world);

    shift.vw = 0.0f;
    get_mp_shift(loop, &shift, work->which);	//ミサイルポッドSHIFT値を取得
    shift.vz -= 700.0f;

    //    shift.vz += RP_TIMER*300.0f;
    shift.vz += RP_TIMER*60.0f;

    DG_MovePos( &shift );




    // ****ポッドから解放
    if (RP_TIMER >= 25){
	SVECTOR	tmprot = {-6, -work->which*18, 0, 0};
	
	{	// ==================エクストリーム機銃対応ロケット発射
	    if ( (GET_HAR_WORK!=NULL)&&
		 (((Work*)GET_HAR_WORK)->har_mode_l1==H_MOD_L1_PROGRAM_MOVE) ){
		if ( ((Work*)GET_HAR_WORK)->har_mode_l3==L3_ATCK_NORM_PLAYER_UP){
		    tmprot.vx = 8;
		}
		else {
		    tmprot.vx = -8;
		}
		tmprot.vy = -work->which*8;
	    }


	    else {
		tmprot.vy = -work->which*40;
	    }
	}
	
	RP_MODE = HAR_MPMOD_FLYING;
	RP_TIMER = 0;
	work->mp_tgt_body[loop].class &= ~TARGET_SKIP;
	DG_RotatePos ( &tmprot );
    }


    DG_GetPos( &pos_s->world );
    set_posvec(loop, work, &pos_s->world );	//エフェクとの位置と方向ベクトル設定

}


//#define ROCKET_MAX_SPEED	(1300.0f)


static float CheckDistanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}

#define X_BORDER (-1650.0f)
#define Z_BORDER (-153875.0f)
#define Z_BORDER_R (-146175.0f)
/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mp_flying_act						*/
/*	引数:	Work	*work							*/
/*	説明:	飛んでくロケット						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mp_flying_act(Mpod_Work *work, short loop)
{
    //    FVECTOR		speed = {0.0f, 0.0f, 500.0f, 0.0f};	//ロケットのスピード
    //    FVECTOR		speed = {0.0f, 0.0f, 100.0f, 0.0f};	//ロケットのスピード
    FVECTOR		speed = {0.0f, 0.0f, 50.0f, 0.0f};	//ロケットのスピード
    FVECTOR		from_vec, to_vec;
    SVECTOR		randrot;				//ランダムな動き
    DG_COMDL_POS	*pos_s;
    float		max_speed;

    short dist = (int)_FVecLen3( &work->oya_control->step);

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	max_speed = 80.0f;
	break;
    case ST_LEV_EASY:
	max_speed = 90.0f;
	break;
    case ST_LEV_NORMAL:
	max_speed = 110.0f;
	break;

    case ST_LEV_UPNORM:
	max_speed = 145.0f;
	break;

    case GM_LEVEL_HARD:
	max_speed = 180.0f;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	max_speed = 320.0f;
	break;
    default:
	max_speed = 60.0f;
	break;
    }

    pos_s =  &work->mpod_warhead->pos[loop];
    if (&pos_s->world == NULL){
	printf ("DANGER DANGER  Pointer NULL Access !!!!!!\n");
	return;
    }
    Fvec_from_Matrix( &from_vec, &pos_s->world );	//開始点を保存 
    
    if (pos_s->world.m[3][0]>X_BORDER){
	FVECTOR	tmppos = {X_BORDER, 0.0f, Z_BORDER, 1.0f};
	FVECTOR	tmpvec;
	float	dist;

	_sceVu0CopyVector(&tmpvec, (FVECTOR*)pos_s->world.m[2]);
	tmpvec.vy = 0.0f;
	_sceVu0Normalize(&tmpvec, &tmpvec);


	tmppos.vy = pos_s->world.m[3][1];



	//左タンクの安全地帯
	dist = CheckDistanceLineAndPoint( (FVECTOR*)pos_s->world.m[3],
				       &tmpvec, 
				       &tmppos);
	if ((1200.0f>dist)&&(dist>300.0f)){
#if 0
	    FVECTOR verts[2];
    
	    verts[0] =tmppos;
	    verts[1] =*(FVECTOR*)pos_s->world.m[3];
	    verts[0].vy = 600.0f;
	    NewLineView(verts, 1, 150, 150, 150);
#endif
	    if (pos_s->world.m[3][2]+
		pos_s->world.m[2][2]*((pos_s->world.m[3][0]-X_BORDER)/-pos_s->world.m[2][0])
		< Z_BORDER){
		SVECTOR tmprot = {0, +4, 0, 0};
		DG_SetPos(&pos_s->world);
		DG_RotatePos(&tmprot);
		DG_GetPos(&pos_s->world);
		

	    }
	    if (pos_s->world.m[3][2]+
		pos_s->world.m[2][2]*((pos_s->world.m[3][0]-X_BORDER)/-pos_s->world.m[2][0])
		> Z_BORDER){
		SVECTOR tmprot = {0, -4, 0, 0};
		DG_SetPos(&pos_s->world);
		DG_RotatePos(&tmprot);
		DG_GetPos(&pos_s->world);
	    }
	}


	tmppos.vz = Z_BORDER_R;
	//右タンクの安全地帯
	dist = CheckDistanceLineAndPoint( (FVECTOR*)pos_s->world.m[3],
				       &tmpvec, 
				       &tmppos);
	if ((1200.0f>dist)&&(dist>300.0f)){
#if 0
	    FVECTOR verts[2];
    
	    verts[0] =tmppos;
	    verts[1] =*(FVECTOR*)pos_s->world.m[3];
	    verts[0].vy = 600.0f;
	    NewLineView(verts, 1, 150, 150, 150);
#endif
	    if (pos_s->world.m[3][2]+
		pos_s->world.m[2][2]*((pos_s->world.m[3][0]-X_BORDER)/-pos_s->world.m[2][0])
		< Z_BORDER_R){
		SVECTOR tmprot = {0, +4, 0, 0};
		DG_SetPos(&pos_s->world);
		DG_RotatePos(&tmprot);
		DG_GetPos(&pos_s->world);
		

	    }
	    if (pos_s->world.m[3][2]+
		pos_s->world.m[2][2]*((pos_s->world.m[3][0]-X_BORDER)/-pos_s->world.m[2][0])
		> Z_BORDER_R){
		SVECTOR tmprot = {0, -4, 0, 0};
		DG_SetPos(&pos_s->world);
		DG_RotatePos(&tmprot);
		DG_GetPos(&pos_s->world);
	    }
	}
    }




    if (((Work*)GET_HAR_WORK)->har_mode_l1==H_MOD_L1_PROGRAM_MOVE){ // ==機銃フェイズのみ
	if (fabs(from_vec.vx) < 20000.0f){	//====近付いたら減速 橋を突き抜けたりするから
	    max_speed = max_speed + (dist*fabs(from_vec.vx)/20000.0f);
	    speed.vz += (dist*fabs(from_vec.vx)/20000.0f);
	}
	else {
	    max_speed = max_speed + (dist*1.0f);
	    speed.vz += (dist*1.0f);
	}
    }




    speed.vz += RP_TIMER * 7.0f / 5.0f;

    if (speed.vz >max_speed){
	speed.vz =max_speed;
    }

    DG_SetPos( &pos_s->world );

    if (RP_TIMER == 0){
	if ( (GET_HAR_WORK!=NULL)&&
	     (((Work*)GET_HAR_WORK)->har_mode_l1==H_MOD_L1_PROGRAM_MOVE) ){
	    randrot.vx = RAND(11)-5;
	    randrot.vy = RAND(11)-5;
	}
	else {
	    randrot.vx = RAND(61)-30;
    	    randrot.vy = RAND(61)-30;
	    //	    randrot.vx = 0;
	    //	    randrot.vy = 0;
	}
	randrot.vz = 0;
	DG_RotatePos( &randrot );
    }

    DG_MovePos( &speed );
    DG_GetPos( &pos_s->world );
    
    {	// 先端で当たりをとる	
	FMATRIX tmpmat;
	FVECTOR tmpvec = {0.0f, 0.0f, 600.0f, 1.0f};
	DG_MovePos( &tmpvec );
	DG_GetPos( &tmpmat );
	Fvec_from_Matrix( &to_vec, &tmpmat );	//終了点を保存 
    }
    
    set_posvec(loop, work, &pos_s->world );	//エフェクとの位置と方向ベクトル設定
    
    GM_MoveTarget2Map( &work->mp_tgt_body[loop], &pos_s->world, work->mp_tgt_body[loop].map);
    //    GM_MoveTarget2( &work->mp_tgt_body[loop], &pos_s->world);
    
    
    if ( (((Work*)GET_HAR_WORK)->har_mode_l1!=H_MOD_L1_PROGRAM_MOVE)&&
	 (to_vec.vy < -3000.0f)&&(to_vec.vx < -2900.0f)){
	FVECTOR	point;
	Fvec_from_Matrix( &point, &pos_s->world);
	
	RP_MODE = HAR_MPMOD_BOMED;	//爆発フラグを立てる
	//煙エフェクト
	NewExplosionSmoke( &point );
	
	//振動
	if (!GM_CheckGameStatus(STATE_PLAY_DEMO)){
	    NewShakeCamera2(0, 30, 10, &point);	//カメラ振動
	    call_pad_vibration(VIB_MP_EXP);		//パッド振動
	}
	pos_s->color.vw = 0;
	work->mp_tgt_body[loop].class |= TARGET_SKIP;
	
	if (RAND(2)){
	    GM_SeSetMode (SD_E_M38BOM07, &point, GM_SEMODE_BOMB);	//SE小爆発
	}
	else {
	    GM_SeSetMode (SD_E_M38BOM08, &point, GM_SEMODE_BOMB);	//SE小爆発
	}
	
	if  (GM_PlayerStatus & PLAYER_ROLLING){
	    printf ("Rolling !! \n");
	}
	else if ( (fabs(GM_PlayerPosX-point.vx)<2500.0f) && (fabs(GM_PlayerPosZ-point.vz)<2500.0f) &&
		  (HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &point, &GM_PlayerFindPos,
					  HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE )) ){
	    printf ("Behind Wall !! \n");
	}
	else {
	    GM_MoveTarget (&work->mp_target[loop], &point);
	    GM_PutTarget (&work->mp_target[loop] );
	}
	RP_TIMER = 0;
    }
    else if ( HZX_OnlineHazardCheck(0, &from_vec, &to_vec, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE_ONLY,
				    (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE),
				    (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE)) ){
	FVECTOR	point, direction, normal;
	HZX_HZD 	seg;
	int		tmp;
	
	RP_MODE = HAR_MPMOD_BOMED;	//爆発フラグを立てる
	HZX_GetOnlineHazard( &seg, &tmp );	//ハザードを取得
	HZX_GetOnlinePoint( &point );	//接触点を取得
	HZX_GetOnlineVector( &direction );	//弾の方向ベクトルを取得
	HZX_GetNormal (&normal, &seg); 	//ハザードの法線ベクトルを取得
	
	//E3専用？
	direction.vy = 0.0f;
	SAT_Minus_FVECTOR(&normal, &DG_ZeroVector, &direction);
	
	Fvec_from_Matrix( &point, &pos_s->world);
	
	//煙エフェクト
	NewRunningSmoke( &point, &normal, &direction );
	NewExplosionSmoke( &point );
	
	//振動
	if (!GM_CheckGameStatus(STATE_PLAY_DEMO)){
	    if (fabs(point.vx) < 10000.0f){
		NewShakeCamera2(0, 30, 10, &point);	//カメラ振動
		call_pad_vibration(VIB_MP_EXP);		//パッド振動
	    }
	}
	
	pos_s->color.vw = 0;
	work->mp_tgt_body[loop].class |= TARGET_SKIP;
	
	if (RAND(2)){
	    GM_SeSetMode (SD_E_M38BOM07, &point, GM_SEMODE_BOMB);	//SE小爆発
	}
	else {
	    GM_SeSetMode (SD_E_M38BOM08, &point, GM_SEMODE_BOMB);	//SE小爆発
	}
	
	if  (GM_PlayerStatus & PLAYER_ROLLING){
	    printf ("Rolling !! \n");
	}
	else if ( (fabs(GM_PlayerPosX-point.vx)<2500.0f) && (fabs(GM_PlayerPosZ-point.vz)<2500.0f) &&
		  (HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &point, &GM_PlayerFindPos,
					  HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE )) &&
		  (HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &point,
					  (FVECTOR*)GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world.m[3],
					  HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE )) ){
	    printf ("Behind wall !! \n");
	}
	else {
	    GM_MoveTarget (&work->mp_target[loop], &point);
	    GM_PutTarget (&work->mp_target[loop] );
	    
	}
	RP_TIMER = 0;
#if 0
	NewTargetView2(&work->mp_target[loop],
		       255, 50, 50);
#endif
    }	
    
    {
	int count;
	
	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    count = 1500;
	    break;
	case ST_LEV_EASY:
	    count = 1300;
	    break;
	case ST_LEV_NORMAL:
	    count = 1100;
	    break;

	case ST_LEV_UPNORM:
	    count = 1050;
	    break;

	case GM_LEVEL_HARD:
	    count = 1000;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    count = 950;
	    break;
	default:
	    count = 1100;
	    break;
	}
	
	if (RP_TIMER > count){
	    RP_MODE = HAR_MPMOD_BOMED;
	    work->mpod_warhead->pos[loop].color.vw = 0;
	    work->mp_tgt_body[loop].class |= TARGET_SKIP;
	    //煙エフェクト
	    NewExplosionSmoke( &from_vec );
	    
	    //振動
	    if (!GM_CheckGameStatus(STATE_PLAY_DEMO)){
		if (fabs(from_vec.vx) < 10000.0f){
		    NewShakeCamera2(0, 30, 10, &from_vec);	//カメラ振動
		    call_pad_vibration(VIB_MP_EXP);		//パッド振動
		}
	    }
	}
	
    }
    
    RP_TIMER += TIME_BASE;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mp_bomed_act						*/
/*	引数:	Work	*work							*/
/*	説明:	爆発したロケットの末路						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mp_bomed_act(Mpod_Work *work, short loop)
{

    //    if ( work->mis_mode == HAR_MPMOD_EQUIP){
    work->mp_fire_flg[loop]++;
    RP_MODE = HAR_MPMOD_EQUIP;
    work->mpod_warhead->pos[loop].color.vw = 0;
    work->mp_tgt_body[loop].class |= TARGET_SKIP;
	//    }

}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mp_equip_act						*/
/*	引数:	Work	*work							*/
/*	説明:	発射前のロケット						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mp_equip_act(Mpod_Work *work, short loop)
{
    OBJECT	*tmp;
    FVECTOR		shift;
    FVECTOR		from_vec, to_vec;
    int			fire_speed;
    int			shoot_count;


    DG_COMDL_POS	*pos_s;
    pos_s =  &work->mpod_warhead->pos[loop];

    tmp = work->oya_control->object;
	
    DG_SetPos( &tmp->objs->world);
    Fvec_from_Matrix( &from_vec, &pos_s->world );	//開始点を保存 

    shift.vw = 0.0f;
    get_mp_shift(loop, &shift, work->which);	//ミサイルポッドSHIFT値を取得
    shift.vz -= 700.0f;

    DG_MovePos( &shift );
    DG_GetPos( &pos_s->world );

    Fvec_from_Matrix( &to_vec, &pos_s->world );	//終了点を保存 



    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	fire_speed = 30;
	shoot_count = 10;
	break;
    case ST_LEV_EASY:
	fire_speed = 25;
	shoot_count = 13;
	break;
    case ST_LEV_NORMAL:
	fire_speed = 20;
	shoot_count = 20;
	break;

    case ST_LEV_UPNORM:
	fire_speed = 17;
	shoot_count = 25;
	break;

    case GM_LEVEL_HARD:
	fire_speed = 14;
	shoot_count = 30;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	fire_speed = 11;
	if ( (GET_HAR_WORK!=NULL)&&	//機銃突っ込みフェイズの射撃数
	     (((Work*)GET_HAR_WORK)->har_mode_l1==H_MOD_L1_HOVER_MP) ){
	    shoot_count = 40;
	}
	else {	//大きくオーバーして次がミサイルポッドフェイズだと危ない
	    shoot_count = 13;
	}
	break;
    default:
	fire_speed = 25;
	shoot_count = 20;
	break;
    }

    if (SHOOT_COUNT>shoot_count){
	work->mis_mode = HAR_MPMOD_EQUIP;
    }

    // *****ロケット発射
    if ((work->mis_mode == HAR_MPMOD_FIRE)&&(work->mp_end_flg[loop]==OFF)&& (SHOOT_COUNT<=shoot_count) ){
	if ( ((work->mp_fire_flg[loop]==0)&&(ALL_COUNT > loop*fire_speed*5))||
	     ((work->mp_fire_flg[loop]==1)&&(ALL_COUNT > (loop+MPOD_MISSILE_NUM)*fire_speed*5)
	      &&(ALL_COUNT < MPOD_MISSILE_NUM*2*fire_speed*5)) ){
	
	    if ((work->break_flag == OFF)||(RAND(4)==0)){
		int rgba0, rgba1;

		SHOOT_COUNT++;
		if (work->mp_fire_flg[loop]==1){
		    work->mp_end_flg[loop]=ON;
		}
		printf("Mis fire:%d \n",loop);

		SetRGBA_CharR(((Work *)GET_HAR_WORK)->missile_burn_col, &rgba0);
		SetRGBA_CharR(&((Work *)GET_HAR_WORK)->missile_burn_col[4], &rgba1);

		RP_MODE = HAR_MPMOD_FIRE;
		RP_TIMER = 0;
		work->mp_fire_flg[loop] = 0;
		set_posvec(loop, work, &pos_s->world );	//エフェクとの位置と方向ベクトル設定
		NewHarrierMissileFire(&work->position[loop], &work->vector[loop], &work->mp_fire_flg[loop], rgba0, rgba1);
		NewHarrierPodSmoke(
		    &work->position[loop], 
		    &work->vector[loop],
		    &work->oya_control->object->objs->world);
		
		pos_s->color.vx = pos_s->color.vy = pos_s->color.vz = 128;
		pos_s->color.vw = 128;
		
		//printf ("Fire! %d\n", loop);
		
		GM_SeSetMode (SD_E_MISFIRS2, &work->position[loop], GM_SEMODE_BOMB);//SEロケット弾発射	    
	    }
	    else {
		RP_TIMER = 0;
		RP_MODE = HAR_MPMOD_BOMED;	//爆発フラグを立てる
		NewMissileExplosion((FVECTOR*)pos_s->world.m[3], 350.0f);
		work->mp_fire_flg[loop] = 1;
		if (RAND(3)==0){
		    NewCrashDebris((FVECTOR*)pos_s->world.m[3]);
		}
	    }
	    return;
	}
    }

}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void set_mpod							*/
/*	引数:	Work	*work							*/
/*	説明:	挙動関連全般制御						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_mpod(Mpod_Work *work){
    FVECTOR	tmpvec = {work->which*1950.0f, -220.0f, 680.0f, 1.0f};

    Work *pwork;
    pwork  = (Work*)GET_HAR_WORK;


    //  *****非表示親にひきずられます
    work->mpod_body.objs->flag |= pwork->body.objs->flag & DG_FLAG_INVISIBLE;

    if (work->which == 1){		// **右のポッド
	if (pwork->missilepod_damage & POD_RIGHT_DAMAGE){
	    work->mpod_body.objs->objs[0].flag |= DG_FLAG_INVISIBLE;
	    return;
	}
    }
    else if (work->which == -1){	// **左のポッド
	if (pwork->missilepod_damage & POD_LEFT_DAMAGE){
	    work->mpod_body.objs->objs[0].flag |= DG_FLAG_INVISIBLE;
	    return;
	}
    }

    DG_SetPos(&pwork->body.objs->objs[0].world);
    DG_MovePos(&tmpvec);
    DG_GetPos(&work->mpod_body.objs->world);
    DG_GetPos(&work->mpod_body.objs->objs[0].world);

}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	挙動関連全般制御						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Mp_Act(Mpod_Work *work)
{
    short	loop;



    set_mpod(work);	// ミサイルポッドの表示


    if (work->mis_mode == HAR_MPMOD_FIRE){
	ALL_COUNT += TIME_BASE;
    }

    for (loop=0; loop < MPOD_MISSILE_NUM; loop++){

	switch (RP_MODE){

	case HAR_MPMOD_EQUIP:	//装備状態
	    mp_equip_act(work, loop);
	    break;

	case HAR_MPMOD_FIRE:	//発射
	    mp_fire_act(work, loop);
	    break;

	case HAR_MPMOD_FLYING:	//飛行
	    mp_flying_act(work, loop);
	    break;

	case HAR_MPMOD_BOMED:	//ちゃくだん
	    mp_bomed_act(work, loop);
	    break;
	}
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int ReceiveSignal						*/
/*	引数:	Work	*work							*/
/*	    :	int	signal							*/
/*	    :	int	value							*/
/*	説明:	シグナルを受信する						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int Mp_ReceiveSignal(void *work, int signal, int value){

    if (signal == GV_SIGNAL_KILL){
	((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
	GV_DestroyActor( work);
	return 1;
    }
    else if ( (signal & HAR_SIGNAL_MPOD_MASK) == HAR_SIGNAL_MPOD ){
	((Mpod_Work*)work)->mis_mode = ((signal) & (~HAR_SIGNAL_MPOD));

	if (((Mpod_Work*)work)->mis_mode == HAR_MPMOD_FIRE){
	    int loop;
	    ALL_COUNT = 0;
	    SHOOT_COUNT = 0;
	    for (loop=0; loop < MPOD_MISSILE_NUM; loop++){
		(((Mpod_Work*)work)->mp_timer[loop]) = 0;
		((Mpod_Work*)work)->mp_fire_flg[loop] = 0;
		((Mpod_Work*)work)->mp_end_flg[loop]=OFF;
	    }
	}

	return 1;
    }

    return 0;
}



/**************************<-------local function------>*************************/
/*	名前:	void *Targ_MPCallBack						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:	胴体当たりのコールバック					*/
/********************************************************************************/
void *Targ_MPCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Mpod_Work	*work ;
    short	loop = def->name;	    

    work = ( Mpod_Work * )ptr ;
    if ( off->class & TARGET_POWER ) {
	if( !(def->weapon_type & WP_COLDSPRAY) && 
	    !(def->weapon_type & WP_STUNGRENADE) ){

	if (RP_MODE == HAR_MPMOD_FLYING){
	    RP_MODE = HAR_MPMOD_BOMED;	//爆発フラグを立てる
	    NewExplosionSmoke( (FVECTOR*)work->mpod_warhead->pos[loop].world.m[3]) ;
	    work->mpod_warhead->pos[loop].color.vw = 0;
	    work->mp_tgt_body[loop].class |= TARGET_SKIP;

	    if (RAND(2)){
		GM_SeSetMode (SD_E_M38BOM07, (FVECTOR*)work->mpod_warhead->pos[loop].world.m[3], GM_SEMODE_BOMB);	//SE小爆発
	    }
	    else {
		GM_SeSetMode (SD_E_M38BOM08, (FVECTOR*)work->mpod_warhead->pos[loop].world.m[3], GM_SEMODE_BOMB);	//SE小爆発
	    }
	}
	}
    }
    return 0;
}


#define	MPTARGET_SETUP( _target, _x, _y, _z )\
{\
    targ_size.vx = (_x);						\
    targ_size.vy = (_y);						\
    targ_size.vz = (_z);						\
    GM_SetTarget( (_target), flag,map, ENEMY_SIDE, 			\
		  &targ_size, &DG_ZeroVector );				\
    GM_SetTargetCallBack( (_target), (void *)Targ_MPCallBack, work );	\
    GM_SetTargetWeaponType( (_target) ,0);			       	\
    GM_PutTarget( (_target) );						\
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int Mp_GetResources(Mpod_Work *work, Work *oya_work, int where, char which)
{

    work->oya_control = &oya_work->control;	//被装備オブジェクト登録
    work->which = which;
    if (work->which == MPOD_RIGHT){
	GET_MPOD_WORK_R = (void*)work;
    }
    if (work->which == MPOD_LEFT){
	GET_MPOD_WORK_L = (void*)work;
    }

    {
	short	loop;
	int	flag, map;
	FVECTOR	targ_size;
	FVECTOR	size = {900.0f, 800.0f, 900.0f, 1.0f};
	FVECTOR	force = {250.0f, 0.0f, 0.0f, 0.0f};

	GM_InitObject(&work->mpod_body,
		  GV_StrCode("hri_mspod"  ),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_FOGPARAM|DG_FLAG_IRREACTION);
	DG_SetFogParamObjs (work->mpod_body.objs, FOG_NEAR, FOG_FAR);    

	/*防御ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK
	       |TARGET_ROTATE|TARGET_THROUGH
	       |TARGET_POWER);
	map = where;

	for (loop=0; loop<MPOD_MISSILE_NUM; loop++){
	    MPTARGET_SETUP ( &work->mp_tgt_body[loop],
			   350.0f, 350.0f, 800.0f);
//			   60.0f, 60.0f, 700.0f);
	    work->mp_tgt_body[loop].name = loop;



	    if (GM_GameLevel >= GM_LEVEL_EXTREME){	//エクストリーム攻撃範囲広い
		size.vx = 1200.0f;
		size.vz = 1100.0f;
	    }
	    /*攻撃ターゲット設定*/
	    GM_SetTarget( &work->mp_target[loop],
			  TARGET_OFFENSE,
			  map, PLAYER_SIDE,
			  &size, &DG_ZeroVector ) ;
	    GM_SetTargetWeaponType( &work->mp_target[loop], WP_GRENADE);
	{
	    int damage = 12;
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	damage = 17;
	break;
    case ST_LEV_EASY:
	damage = 18;
	break;
    case ST_LEV_NORMAL:
	damage = 22;
	break;

    case ST_LEV_UPNORM:
	damage = 26;
	break;

    case GM_LEVEL_HARD:
	damage = 30;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	damage = 50;
	break;
    }
	    GM_SetPowerTarget( &work->mp_target[loop],
			       &work->mp_pow_tgt,
			       POWER_ONCE,
			       50,
			       12,
			       damage,
			       &force );
	}
	    GM_SetTargetCallBack( &work->mp_target[loop],
				  (void *)dumfunc,
				  NULL);


	}
    }


    {
	DG_DEF		*def;
	DG_COMDL_POS	*pos_s;
	short		loop;

	//表示するモデルをセット
	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("hri_mspod_warhead_cm"), 'k' ) );

#ifdef DEBUG_MODE
	if ( def == NULL ) {
	    printf( "moel not found in har_claster in har_claster\n" ) ;
	    ASSERT( 0 ) ;	    
	}
#endif

	work->mpod_warhead = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, MPOD_MISSILE_NUM, 0 );

#ifdef DEBUG_MODE
	if ( work->mpod_warhead == NULL ) {
	    printf( "warning : model make objs failed\n") ;
	}
#endif

	DG_QueueComdlObjs( work->mpod_warhead );
	GM_GroupObject(work->mpod_warhead, where);

	//コモデル ミサイルポッド ミサイル初期化
	pos_s = work->mpod_warhead->pos;
	for (loop=0; loop < MPOD_MISSILE_NUM; loop++, pos_s++){
	    pos_s->world = oya_work->body.objs->world;
	    pos_s->color.vx = pos_s->color.vy = pos_s->color.vz = 128;
	    pos_s->color.vw = 0;
	}
    }

    GV_SetActorSignalFunc( work, Mp_ReceiveSignal );	//シグナル受信関数設定

    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewHarMpod						*/
/*	引数:	Work	*oya_work	親のワークへのポインタ			*/
/*		int	where							*/
/*	説明:	ミサイルを表示							*/
/********************************************************************************/
void *NewHarMpod(Work *oya_work, int where, char which)
{
    Mpod_Work *work ;
    work = (Mpod_Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Mpod_Work ), 255 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Mp_Act,Mp_Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!Mp_GetResources(work, oya_work, where, which)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}





