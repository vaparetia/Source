/********************************************************************************/
/*	cypheryp.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cyp_alert.c,v 1.1.1.3 2002/11/19 11:48:14 Yoshizawa1 Exp $			*/
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

#ifdef KP_XBOX	// サイファ音は遮蔽計算なしにする
#define	GM_SeSetFromVolCurve( a, b, c ) GM_SeSetFromVolCurveAddr( a, b, c, GM_INVALID_ADDR )
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int CYP_Evasion_Check		**  **		*/
/*	引数:	Work	*work							*/
/*	説明:							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CYP_Evasion_Check(Work *work){
    if(GM_AlertMode==ALERT_MODE_AVOID){   
	if (work->ACTION_LAYER > EYE_LAYER_EVASION){
	    SV_G1_STEP(work, CYP_EVASION);
	    work->ACTION_LAYER = EYE_LAYER_EVASION;
	    return 1;
	}
    }
    return 0;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Alert_Check		** EYE_LAYER_ALERT **		*/
/*	引数:	Work	*work							*/
/*	説明:	危険モードチェック						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CYP_Alert_Check(Work *work){
   
    if ( CHECK_PC_FLG(EYECK_PC_IN_SIGHT) &&
	 !CHECK_PC_FLG(EYECK_PC_IN_UNIFORM) ){
	   if (work->ACTION_LAYER > EYE_LAYER_PC_ALERT){
	       SV_G1_STEP(work, CYP_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_PC_ALERT;
	       return 1;
	   }
    }
    if ( (GM_AlertMode == ALERT_MODE_ALERT) && 
	 ( ((!(COM_AlertStatus()&COM_ALERT_NPC_DETECT))&&(!(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT))) || 
	   (COM_AlertStatus()&COM_ALERT_PLAYER_DETECT) )
	){
	   if (work->ACTION_LAYER > EYE_LAYER_PC_ALERT){
	       SV_G1_STEP(work, CYP_ALERT);
	       SV_G2_STEP(work, SUB_MOD_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_PC_ALERT;
	       return 1;
	   }
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Alert_Check		** EYE_LAYER_ALERT **		*/
/*	引数:	Work	*work							*/
/*	説明:	危険モードチェック						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CYP_Emma_Alert_Check(Work *work){
 
    if ( CHECK_PC_FLG(EYECK_PC_EMMA_IN_SIGHT) &&
	 !CHECK_PC_FLG(EYECK_PC_IN_SIGHT) ){
	   if (work->ACTION_LAYER > EYE_LAYER_EMMA_ALERT){
	       SV_G1_STEP(work, CYP_EMMA_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_EMMA_ALERT;
	       return 1;
	   }
    }

    if ( GM_AlertMode == ALERT_MODE_ALERT ){
	   if (work->ACTION_LAYER > EYE_LAYER_EMMA_ALERT){
	       SV_G1_STEP(work, CYP_EMMA_ALERT);
	       work->ACTION_LAYER = EYE_LAYER_EMMA_ALERT;
	       return 1;
	   }
    }
    return 0;
}

#define CHECK_FUNC_NUM	(1)

SV_FUNCLIST Check_Funk[CHECK_FUNC_NUM]= 
{
    CYP_Alert_Check,
};


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cam_sub_warn						*/
/*	引数:	Work	*work							*/
/*	説明:	赤びっくり発動待ち						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_sub_warn(Work *work){

    if( work->discv_time == 0 ){
	if (GM_AlertMode != ALERT_MODE_ALERT){
	    GM_SeSetMode( SD_E_CAMFND01 , work->eye.eyepos,		//カシャ
			  GM_SEMODE_BOMB );
	}
    }

    if( work->discv_time > SV_DISCV_DELAY ){		//通報
	COM_SetPureSecurityAlertLevel(ALERT_LEVEL_MAX);//危険レベルの維持
	SV_G2_STEP(work, SUB_MOD_ALERT);
    }

    
    /*びっくりセット*/
    if( ((SV_HMARK_DELAY+TIME_BASE) > work->discv_time)&&(work->discv_time >= SV_HMARK_DELAY) ){
	if ( GM_AlertMode != ALERT_MODE_ALERT ){
	    red_bikkuri_set(work);
	}
	COM_SetFlameFlag( CMFLAG_DETECT );
    }
    
    /*目標物へ向く*/

    //    printf ("CK: %p %p\n",work->eye.eyepos, work->eye.trgpos);

    Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
		  &work->control.turn);
    if (work->control.turn.vx > 3000){
	work->control.turn.vx -= 4096;
    }

    work->discv_time += TIME_BASE;

    // ****************プレイヤの位置******************
    {	
	TracePlayer3D(work, work->eye.trgpos);
    }
}

extern void AN_MazzleMeca( FMATRIX*, FVECTOR*);
extern void AN_CartridgeMeca_E( FMATRIX*, FVECTOR*);

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_warn							*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_gun_fire(Work *work){
    
    if( work->gun_count >= 800 ){ 
	// *************ガンは通報の代わりに発砲
	if ((work->gun_count/500)%4==0){	
	    extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
	    int type ;
	    FMATRIX tmpmat;
	    FVECTOR trans = {0.0f, -280.0f, 0.0f};
	    FVECTOR shift = {20.0f, -275.0f, 85.0f};
	    SVECTOR tmprot = {-1024, 0, 0};
	    
	    //	弾と軌跡発生
	    type = (BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_NO_PLAYER);
	    DG_SetPos( &work->body.objs->objs[CAMERA_PARTS].world) ;
	    DG_MovePos(&trans );

	    tmprot.vx += RAND(41)-20;
	    tmprot.vy += RAND(41)-20;


	    DG_RotatePos(&tmprot );
	    DG_GetPos( &tmpmat ) ;

	    if ((work->gun_count/500)%8==0){
		NewBullet( &tmpmat,type, PLAYER_SIDE, 25, 5, 20000, 1000, WP_m4 );
	    }

	    GM_SeSetMode( SD_E_FAMAS03 ,(FVECTOR*)tmpmat.m[3],GM_SEMODE_BOMB ) ;
	    
	    trans.vy = -650.0f;
	    DG_MovePos(&trans );
	    DG_GetPos( &tmpmat ) ;
	    AN_MazzleMeca (&tmpmat, &DG_ZeroVector);	// まずるふらっしゅ
	    
	    //メカカートリッジ
	    DG_SetPos( &work->body.objs->objs[1].world) ;
	    DG_MovePos(&shift );
	    DG_RotatePos(&tmprot);
	    DG_GetPos( &tmpmat ) ;
	    //	    AN_CartridgeMeca_E( &tmpmat , &DG_ZeroVector);	
	    work->fire_flag = 1;
	}
	work->gun_count -= TIME_BASE*100;
    }
    
    if (work->gun_count < 800){
	if (work->gun_count <= 0){
	    work->gun_count = (30+(irnd()&10))*2000;
	    work->gun_count += 50+(irnd()&25)*5;
	}
	work->gun_count-=TIME_BASE;
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cam_sub_alert						*/
/*	引数:	Work	*work							*/
/*	説明:	赤びっくり発動 警戒						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_sub_alert(Work *work){

    if(work->alert_time <=  SV_ALERT_TIME)
    {
	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,	// 目標物へ向く
		      &work->control.turn);
	if (work->control.turn.vx > 3000){
	    work->control.turn.vx -= 4096;
	}

	COM_SetPureSecurityAlertLevel(ALERT_LEVEL_MAX);//危険レベルの維持

	if (work->type == GUN_TYPE){	// 撃て！
	    if (work->alert_time == 0){
		cyp_gun_fire(work);
	    }
	}

	if (GM_AlertMode!=ALERT_MODE_ALERT){
	    red_bikkuri_set(work);
	}
	SV_G3_STEP(work, 0);
    }
    else if(GM_AlertMode==ALERT_MODE_ALERT){
	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,	// 目標物へ向く
		      &work->control.turn);
	if (work->control.turn.vx > 3000){
	    work->control.turn.vx -= 4096;
	}
	SV_G3_STEP(work, 0);
    }
    else {
	SV_G1_STEP(work, CYP_EVASION);
	work->ACTION_LAYER = EYE_LAYER_EVASION;
    }
    // ****************プレイヤの位置******************
    {	
	int	temp_int;	//マップ番号
	FVECTOR	tmppos;
	COM_GetPlayerLastPos(&tmppos, &temp_int);	//最後に見た地点
	TracePlayer3D(work, &tmppos);
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_evasion_control					*/
/*	引数:	Work	*work							*/
/*	説明:	回避モード管理							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void cyp_evasion_control(Work *work){
    if(GM_AlertMode==ALERT_MODE_AVOID){

	if ( (work->control.step.vx > 1.0f)||
	     (work->control.step.vy > 1.0f)||
	     (work->control.step.vz > 1.0f) ){
	}
	else {
	    DecayStep(&work->control, 0.9f);

	    switch (G3_STEP){// ****************ステップ分岐****************
	    case 0:
		work->serch_timer = (100+RAND(100))*5;
		SV_G3_STEP(work, 1);
	    case 1:
		work->serch_timer -= TIME_BASE;
		if (work->serch_timer <= 0){
		    if (RAND(2)==0){
			SV_G3_STEP(work, 2);
		    }
		    else {
			SV_G3_STEP(work, 3);
		    }
		    work->serch_timer = 0;
		}
		break;
	    case 2:
	    {
		SVECTOR tempturn;
		Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
			      &tempturn);
		work->control.turn.vy += TIME_BASE*2*6;
		work->control.turn.vx = tempturn.vx;
		if (work->control.turn.vx > 3000){
		    work->control.turn.vx -= 4096;
		}
		work->serch_timer += TIME_BASE;
		if (work->serch_timer >= 68*5){
		    SV_G3_STEP(work, 0);		
		}
	    }
	    break;
	    case 3:
	    {
		SVECTOR tempturn;
		Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
			      &tempturn);
		work->control.turn.vy -= TIME_BASE*2*6;
		work->control.turn.vx = tempturn.vx;
		if (work->control.turn.vx > 3000){
		    work->control.turn.vx -= 4096;
		}
		work->serch_timer += TIME_BASE;
		if (work->serch_timer >= 68*5){
		    SV_G3_STEP(work, 0);		
		}
	    }
	    break;
	    }
	}
    }
    else {
	SV_G1_STEP(work, CYP_NORMAL);
	work->ACTION_LAYER = EYE_LAYER_Normal;
    }

    // ****************プレイヤの位置******************
    {	
	int	temp_int;	//マップ番号
	FVECTOR	tmppos;
	COM_GetPlayerLastPos(&tmppos, &temp_int);	//最後に見た地点
	TracePlayer3D(work, &tmppos);
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_alert_control						*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void cyp_alert_control(Work *work){
    int tmp;
    static FVECTOR tmpvec;
    work->eye.trgpos = &tmpvec;
    COM_GetPlayerLastPos(work->eye.trgpos, &tmp);

    switch (G2_STEP)// ****************ステップ分岐****************
    {
    case SUB_MOD_WARN:	// *****赤びっくり発動待ち
	cyp_sub_warn(work);
	break;
	
    case SUB_MOD_ALERT:	// *****赤びっくり発動 危険

	if ( CHECK_PC_FLG(EYECK_PC_IN_SIGHT) &&
	     (!CHECK_PC_FLG(EYECK_PC_IN_UNIFORM)) ){
	    work->alert_time = 0;
	}
	else if (CHECK_PC_FLG(EYECK_PC_NO_AVOID)){
	    work->alert_time += TIME_BASE;
	    if (COM_GetCommander()->security_alert < ALERT_LEVEL_MAX/4){
		COM_SetPureSecurityAlertLevel(16+(ALERT_LEVEL_MAX/4));//危険レベルの維持
	    }
	}
	else {
	    work->alert_time += TIME_BASE;
	}

	cyp_sub_alert(work);

	//	work->eye.length = work->eye_len_alert;
	break;	
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_emma_alert_control					*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void cyp_emma_alert_control(Work *work){
    work->eye.trgpos = EMA_CommandGetPosition() ;

    switch (G2_STEP)// ****************ステップ分岐****************
    {
    case SUB_MOD_WARN:	// *****赤びっくり発動待ち
	cyp_sub_warn(work);
	break;
	
    case SUB_MOD_ALERT:	// *****赤びっくり発動 危険

	if ( CHECK_PC_FLG(EYECK_PC_EMMA_IN_SIGHT) ){
	    work->alert_time = 0;
	}
	else {
	    work->alert_time += TIME_BASE;
	}

	cyp_sub_alert(work);
	break;	
    }
}









