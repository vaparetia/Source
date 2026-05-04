/********************************************************************************/
/*	cam_alert.c								*/
/*	監視カメラ アラートモード処理						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cam_alert.c,v 1.1.1.3 2002/11/19 11:48:13 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/
/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern void COM_SetPureSecurityAlertLevel( int level );
/********************************************************************************/
/*	define      								*/
/********************************************************************************/
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cam_sub_warn						*/
/*	引数:	Work	*work							*/
/*	説明:	赤びっくり発動前 危険モードサブ処理				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cam_sub_warn(Work *work){

    if( work->discv_time == 0 ){
	if (GM_AlertMode != ALERT_MODE_ALERT){
	    GM_SeSetMode( SD_E_CAMFND01 , work->eye.eyepos,		//カシャ
			  GM_SEMODE_BOMB );
	}
    }

    if( work->discv_time > SV_DISCV_DELAY ){		//通報
	if ( GM_AlertMode != ALERT_MODE_ALERT ){
	    if (work->camera_type != GUN){
		COM_SetPureSecurityAlertLevel(ALERT_LEVEL_MAX);
	    }
	}
	SV_G2_STEP(work, SUB_MOD_ALERT);
    }
    
    /*びっくりセット*/
    if( ((SV_HMARK_DELAY+TIME_BASE) > work->discv_time)&&(work->discv_time >= SV_HMARK_DELAY) ){
	if ( GM_AlertMode != ALERT_MODE_ALERT ){
	    red_bikkuri_set(work);
	}
	if (work->camera_type != GUN){
	    COM_SetFlameFlag( CMFLAG_DETECT );
	}
    }
    
    /*目標物へ向く*/
    Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
		  &work->ctrl.turn);

    work->discv_time += TIME_BASE;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cam_sub_evasion						*/
/*	引数:	Work	*work							*/
/*	説明:	回避モード							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActEvasion(Work *work){
    if(GM_AlertMode==ALERT_MODE_AVOID){
	//回避中 プレイヤ探索
	SV_ActSearch(work) ;
    }
    else {
	SV_G1_STEP(work, MOD_NORMAL);
	work->ACTION_LAYER = EYE_LAYER_Normal;
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cam_sub_alert						*/
/*	引数:	Work	*work							*/
/*	説明:	赤びっくり発動後 危険モード中サブ処理				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cam_sub_alert(Work *work){

    //work->eye.eyepos

    if(work->alert_time <=  SV_ALERT_TIME)
    {
	Dir_from_2Vec(&work->camera_pos,work->eye.trgpos,	// 目標物へ向く
		      &work->ctrl.turn);
	COM_SetPureSecurityAlertLevel(ALERT_LEVEL_MAX);
	//	COM_SetSecurityAlert(work->eye.trgpos,GM_PlayerMap);//危険レベルの維持
	if (GM_AlertMode!=ALERT_MODE_ALERT){
	    red_bikkuri_set(work);
	}
    }
    else if(GM_AlertMode==ALERT_MODE_ALERT){
	Dir_from_2Vec(&work->camera_pos,work->eye.trgpos,	// 目標物へ向く
		      &work->ctrl.turn);
    }
    else {
	SV_G1_STEP(work, MOD_EVASION);
	work->ACTION_LAYER = EYE_LAYER_EVASION;
    }
}

extern void	AN_MazzleMeca( FMATRIX *world , FVECTOR *shift );
extern void AN_CartridgeMeca_E( FMATRIX *world , FVECTOR *shift );

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void gun_sub_alert						*/
/*	引数:	Work	*work							*/
/*	説明:	赤びっくり発動後 危険モード中サブ処理 （ガンカメ）		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void gun_sub_alert(Work *work){

    SET_COLOR_CAM(127 ,30 ,30 ,127, &work->rgba);

    if(work->alert_time <=  SV_ALERT_TIME)
    {
	Dir_from_2Vec(&work->camera_pos,work->eye.trgpos,	// 目標物へ向く
		      &work->ctrl.turn);
	//	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,	// 目標物へ向く
	//		      &work->ctrl.turn);

	if (CHECK_PC_FLG(EYECK_PC_EMMA_IN_SIGHT)||CHECK_PC_FLG(EYECK_PC_IN_SIGHT)||CHECK_PC_FLG(EYECK_NIKITA_IN_SIGHT)){
	if( work->gun_count >= 100 ){ 
	    
	    FVECTOR trans = {0.0f, 230.0f, 640.0f};
	    SVECTOR tmprot = {-1024, 0, 0};			    
	    DG_SetPos( &work->body.objs->objs[1].world) ;
	    DG_MovePos(&trans );
	    DG_RotatePos(&tmprot );
	    DG_GetPos( &work->mazzle_mat ) ;
	    
	    
	    // *************ガンカメは通報の代わりに発砲
	    if ((work->gun_count/100)%4==0){	
		extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
		int type ;
		
		FMATRIX tmpmat;
		FVECTOR trans = {0.0f, 230.0f, 0.0f};
		FVECTOR shift = {-20.0f, 230.0f, 85.0f};
		SVECTOR tmprot = {-1024, 0, 0};
		
		AN_MazzleMeca (&work->mazzle_mat, &DG_ZeroVector);	// まずるふらっしゅ
		
		//メカカートリッジ
		DG_SetPos( &work->body.objs->objs[1].world) ;
		DG_MovePos(&shift );
		DG_RotatePos(&tmprot);
		DG_GetPos( &tmpmat ) ;
		AN_CartridgeMeca_E( &tmpmat , &DG_ZeroVector);	
		
		//	弾と軌跡発生
		type = (BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_NO_PLAYER);
		DG_SetPos( &work->body.objs->objs[1].world) ;
		DG_MovePos(&trans );
		DG_RotatePos(&tmprot );
		DG_GetPos( &tmpmat ) ;
		NewBullet( &tmpmat,type, PLAYER_SIDE, 25, 5, 20000, 1000, WP_m4 ) ;
		GM_SeSetMode( SD_E_FAMAS03 ,(FVECTOR*)tmpmat.m[3],GM_SEMODE_BOMB ) ;
		
		work->fire_flag = 1;
	    }
	    work->gun_count -= 100;
	}
	
	if (work->gun_count < 100){
	    if (work->gun_count == 0){
		work->gun_count = (6+(irnd()&10))*400;
		work->gun_count += 10+(irnd()&25);
	    }
	    work->gun_count--;
	}
	}

    }
    else if( (GM_AlertMode==ALERT_MODE_ALERT)||(GM_AlertMode==ALERT_MODE_AVOID) )
    {
	//回避中 プレイヤ探索
	SV_ActSearch(work) ;
    }
    else {
	SV_G1_STEP(work, MOD_NORMAL);
	work->ACTION_LAYER = EYE_LAYER_Normal;
    }



}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActAlert						*/
/*	引数:	Work	*work							*/
/*	説明:	危険モード 全体処理						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActAlert(Work *work){
    int tmp;
    static FVECTOR tmpvec;
    work->eye.trgpos = &tmpvec;
    COM_GetPlayerLastPos(work->eye.trgpos, &tmp);

    switch (G2_STEP)// ****************ステップ分岐****************
    {
    case SUB_MOD_WARN:	// *****赤びっくり発動待ち
	cam_sub_warn(work);
	break;
	
    case SUB_MOD_ALERT:	// *****赤びっくり発動 危険

	if ( CHECK_PC_FLG(EYECK_PC_IN_SIGHT) &&
	     (!CHECK_PC_FLG(EYECK_PC_IN_UNIFORM)) ){
	    work->alert_time = 0;
	}
	else if (CHECK_PC_FLG(EYECK_PC_NO_AVOID)){
	    work->alert_time = 0;
	}
	else {
	    work->alert_time += TIME_BASE;
	}

	if (work->camera_type == GUN){
	    gun_sub_alert(work);
	}
	else {
	    cam_sub_alert(work);
	}
	work->eye.length = work->eye_len_alert;
	break;	
    }
}

extern FVECTOR *EMA_CommandGetPosition();
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_EmmaActAlert						*/
/*	引数:	Work	*work							*/
/*	説明:	エマ発見 危険モード 全体処理					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_EmmaActAlert(Work *work){
     work->eye.trgpos = EMA_CommandGetPosition() ;

    switch (G2_STEP)// ****************ステップ分岐****************
    {
    case SUB_MOD_WARN:	// *****赤びっくり発動待ち
	cam_sub_warn(work);
	break;
	
    case SUB_MOD_ALERT:	// *****赤びっくり発動 危険

	if ( CHECK_PC_FLG(EYECK_PC_EMMA_IN_SIGHT)){
	    work->alert_time = 0;
	}
	else {
	    work->alert_time += TIME_BASE;
	}

	if (work->camera_type == GUN){
	    gun_sub_alert(work);
	}
	else {
	    cam_sub_alert(work);
	}
	work->eye.length = work->eye_len_alert;
	break;	
    }
}


extern int		GM_DestroyNikita( void );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActNikitaShoot						*/
/*	引数:	Work	*work							*/
/*	説明:	ニキータ発見 危険モード全体処理 （ガンカメ）			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActNikitaShoot(Work *work){

    work->eye.trgpos = &GM_NikitaPosition[0];
    work->eye.length = work->eye_len_alert;

    if ( CHECK_PC_FLG(EYECK_NIKITA_IN_SIGHT) ){
	work->alert_time = 0;

	work->nikita_count += TIME_BASE;

	printf("NT: %d\n", work->nikita_count);

	// *****ニキータを破壊
	if (work->nikita_count > work->nikita_deadend){
	    GM_DestroyNikita();
	    work->nikita_count = 0;
	}
    }
    else {
	work->alert_time += TIME_BASE;
	if (work->nikita_count>0){
	    work->nikita_count -= TIME_BASE*4;
	}
    }
    gun_sub_alert(work);
}

