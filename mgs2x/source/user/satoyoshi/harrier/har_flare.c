/********************************************************************************/
/*	Har_main.c								*/
/*	ハリアメイン *NewHarrier						*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_flare.c,v 1.1.1.3 2002/11/19 11:48:21 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*******************************<Global function>********************************/
/*	名前:	void Har_flare_shoot						*/
/*	引数:	Work	*work							*/
/*	説明:	フレア射出:							*/
/********************************************************************************/
void Har_FlareShoot(Work *work, float size)
{
    short	loop;
    short	count = 0;
    // *****空きフレアをループで探す
    for(loop=0; loop<HAR_FLARE_NUM; loop++){
	if (work->flare_timer[loop] <= 0){
	    FVECTOR	tmpvec = {100.0f, 500.0f, -3300.0f, 0.0f };
	    //	    FVECTOR	speedvec = {500.0f, 750.0f, -50.0f, 0.0f };
	    FVECTOR	speedvec = {1000.0f, 3000.0f, -50.0f, 0.0f };
	    FMATRIX	tmpmat;
	    _sceVu0CopyMatrix(&tmpmat, &work->body.objs->world);

	    if (count){
		tmpvec.vx = -tmpvec.vx;
		speedvec.vx = -speedvec.vx;
	    }

	    // 発射位置の計算
	    DG_SetPos(&tmpmat);
	    DG_MovePos(&tmpvec);
	    DG_GetPos(&tmpmat);
	    _sceVu0CopyVector(&work->flare_pos[loop], (FVECTOR*)tmpmat.m[3]);

	    //発射速度の計算
	    if (count){
		speedvec.vx -= RAND(1500)*1.0f;
	    }
	    else {
		speedvec.vx += RAND(1500)*1.0f;
	    }

	    if (size > 10000.0f){
		speedvec.vy += RAND(1300)*3.0f;
		speedvec.vz += RAND(100)*3.0f;

		speedvec.vx = speedvec.vx *4.0f;
		speedvec.vy = speedvec.vy *3.0f;
	    }
	    else {
		speedvec.vy += RAND(1300)*1.0f;
		speedvec.vz += RAND(100)*1.0f;
	    }

	    _sceVu0CopyVector((FVECTOR*)tmpmat.m[3], &DG_ZeroVector);
	    DG_SetPos(&tmpmat);
	    DG_MovePos(&speedvec);
	    DG_GetPos(&tmpmat);
	    _sceVu0CopyVector(&speedvec, (FVECTOR*)tmpmat.m[3]);

	    work->flare_pos[loop].vw = 1.0f;

	    _sceVu0CopyVector( &work->flare_speed[loop], &work->control.step );
	    work->flare_speed[loop].vx += speedvec.vx;
	    work->flare_speed[loop].vy += speedvec.vy;
	    work->flare_speed[loop].vz += speedvec.vz;
	    work->flare_timer[loop] = 350*5;

	    NewHarrierFlare( &work->flare_pos[loop], size , work->flare_timer[loop]);

	    work->target_flare[loop].class &= ~TARGET_SKIP; 	//処理開始
	    work->target_flare[loop].class |= TARGET_LOCKON; 	//処理開始

	    printf (" Shoot frale # %d \n", loop);
	    count ++;
	    if (count == 1){
		GM_SeSetMode (SD_E_H_FLARE1, &work->control.mov, GM_SEMODE_BOMB);
	    }
	}
	if (count == 2){
	    return;
	}
    }
    // ****空きフレア無し
    printf (" Can't shoot frale, because there is no empty flare\n");
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void harCallBack_flare						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:	フレア当たりのコールバック					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void harCallBack_flare( TARGET	*off, TARGET *def, void	*ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;

    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {
	}
    }

} 

/*******************************<Global function>********************************/
/*	名前:	void Har_InitFlare						*/
/*	引数:	Work *work   						       	*/
/*	説明:	フレア関連のイニシャライズ		       			*/
/********************************************************************************/
void Har_InitFlare(Work *work)
{
    FVECTOR	targ_size= { 700.0f,700.0f,700.0f,0.0f } ;
    short	loop;
    
    for(loop=0; loop<HAR_FLARE_NUM; loop++){
	int		flag, map;
	
	// *****ターゲット設定 フレアは（当然）はロックオンされる
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_LOCKON|TARGET_ROTATE|TARGET_POWER|TARGET_LOCKON_HIGH_PRIO);
	map = work->control.map;
	
	GM_SetTarget( &work->target_flare[loop], flag,map, ENEMY_SIDE, 
		      &targ_size, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( &work->target_flare[loop], harCallBack_flare, work ) ;
	GM_SetTargetWeaponType(&work->target_flare[loop],0);
	GM_PutTarget( &work->target_flare[loop] );
	work->target_flare[loop].class |= TARGET_SKIP; 	//スキップ

    }
}

/*******************************<Global function>********************************/
/*	名前:	void Har_flare_move						*/
/*	引数:	Work	*work							*/
/*	説明:	フレア挙動:							*/
/********************************************************************************/
void Har_FlareMove(Work *work)
{
    short	loop;
    FVECTOR	tmpvec = { 0.0f, -80.0f, 0.0f, 0.0f };	//落下分

    // *****動作中フレアをループで探す
    for(loop=0; loop<HAR_FLARE_NUM; loop++){
	if (work->flare_timer[loop] > 0){
	    _sceVu0ScaleVector( &work->flare_speed[loop], &work->flare_speed[loop], 0.93f);
	    work->flare_speed[loop].vy = work->flare_speed[loop].vy * 0.95f;

	    _sceVu0AddVector( &work->flare_pos[loop], &work->flare_pos[loop], &tmpvec );
	    _sceVu0AddVector( &work->flare_pos[loop], &work->flare_pos[loop], &work->flare_speed[loop]);
	    work->flare_pos[loop].vw = 1.0f;
	    work->flare_timer[loop] -= 5;
	    GM_MoveTargetMap( &work->target_flare[loop], &work->flare_pos[loop], work->control.map);
	}
	else {	//スキップ
	    work->target_flare[loop].class |= TARGET_SKIP;
	    work->target_flare[loop].class &= ~TARGET_LOCKON;
	}
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void CheckTargetInSight						*/
/*	引数:	Work	*work							*/
/*	    :	int	dist							*/
/*	説明:	画面内チェック 							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CheckTargetInSight( TARGET *t )
{
    FMATRIX		tmpmat;
    FVECTOR		tmpvec;
    FVECTOR		max, min, offset ;
    
    if ( t->class & TARGET_ROTATE ) {
	_sceVu0ApplyMatrix( &offset, &t->world, &t->offset ) ;
	DG_COPY_MAT( &tmpmat, &t->world ) ;
    } else {
	DG_COPY_VEC( &offset, &t->offset ) ;
	DG_COPY_MAT( &tmpmat, &DG_UnitMatrix ) ;
    }
    _sceVu0AddVector( &tmpvec, &t->center, &offset ) ;
    GV_VecToMat( &tmpvec, &tmpmat ) ;
    DG_COPY_VEC( &max, &t->size ) ; max.vw = 1.0F ;
    _sceVu0SubVector( &min, &DG_ZeroVector, &max ) ; min.vw = 1.0F ;
    if ( DG_BoundCheck( &tmpmat, &max, &min ) & 0x01 ){
	return 0;
    }
    return 1 ;
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void check_stinger						*/
/*	引数:	Work	*work							*/
/*	説明:	スティンガーの発射を検知する					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void chech_stinger(Work *work, int dist){
    
    if (work->flare_shoot_num == 0){	// *****予約フレアなし
	
	if (dist > 330000){		// ****十分に距離がある
	    // スティンガー発射を検出
	    if (work->save_stinger_num != GM_WeaponNum(WP_Stinger)){
		if ( (PL_LockonTarget==&work->target_b_nzl[0])||
		     (PL_LockonTarget==&work->target_b_nzl[1])||
		     ( CheckTargetInSight(&work->target_b_nzl[0]) )||
		     ( CheckTargetInSight(&work->target_b_nzl[1]) ) ){
		    work->headmark_timer = 150;
		}
		work->save_stinger_num = GM_WeaponNum(WP_Stinger);
	    }
	    //フレア発射カウントダウン
	    if ((0 <= work->headmark_timer)&&(work->headmark_timer != 999)){
		work->headmark_timer -= 5;
	    }
	    
	    //フレア発射予約
	    if (work->headmark_timer < 0){
		//		work->headmark = HMK2_TYPE_RED_AT;	//! 赤 大
		
		
		//	       GM_SeSetMode( SD_V_SOLDM202 , &work->control.mov,
		//			     GM_SEMODE_BOMB ) ;
		if (RAND(2)){
		    GM_SeSet( GM_PAN_CENTER, 0x3f, SD_V_SOLDM201);   //どうした
		}
		else {
		    GM_SeSet( GM_PAN_CENTER, 0x3f, SD_V_SOLDM202);   //効かんな
		}
		// ****無敵になる
		set_targbodyall_skip(work, ON);
		set_hartarg_skip(work, ON);	       
		
		
		work->save_stinger_num = GM_WeaponNum(WP_Stinger);
		work->headmark_timer = 0;
		work->flare_shoot_num = 5;	//５個予約
	    }
	}
	else {
	    // スティンガー発射を検出
	    if (work->save_stinger_num != GM_WeaponNum(WP_Stinger)){
		work->save_stinger_num = GM_WeaponNum(WP_Stinger);
		work->headmark_timer = 100;
	    }
	    
	    //カウントダウン
	    if ((0 <= work->headmark_timer)&&(work->headmark_timer != 999)){
		work->headmark_timer -= 5;
	    }
	    
	    //
	    if (work->headmark_timer < 0){
		work->headmark_timer = 999;
		//	       GM_SeSetMode( SD_V_SOLDM101 , &work->control.mov,	// ****ぬぅっ
		//			     GM_SEMODE_BOMB ) ;
		//	       GM_SeSet( GM_PAN_CENTER, 0x3f, SD_V_SOLDM301);   //クソ
		// ターゲット元通り
		set_targbodyall_big(work);
		set_targbodyall_skip(work, OFF);
	    }
	    
	}
    }
    else {	// *****予約フレア射出
	work->headmark_timer += 5;
	if (work->headmark_timer > 45){
	    Har_FlareShoot(work, 13000.0f);	//フレア撃ち
	    
	    work->headmark_timer -= 45;
	    work->flare_shoot_num --;
	}
	
	if (work->flare_shoot_num == 0){
	    work->headmark_timer = 999;
	}
    }
}


