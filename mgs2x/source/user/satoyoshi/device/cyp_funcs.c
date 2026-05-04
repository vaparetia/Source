/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cyp_funcs.c,v 1.1.1.3 2002/11/19 11:48:14 Yoshizawa1 Exp $			*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern void *NewPadVibration( char *script, int type );
/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#ifdef KP_XBOX	// サイファ音は遮蔽計算なしにする
#define	GM_SeSetFromVolCurve( a, b, c ) GM_SeSetFromVolCurveAddr( a, b, c, GM_INVALID_ADDR )
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif

static void red_bikkuri_set(Work *work){
	if ( !GM_CheckGameStatus( STATE_GAMEOVER ) && !( GM_VRStatus & GM_VR_CLEAR ) ) {
	    GM_SeSetMode( SD_E_BIKKRI01 , work->eye.eyepos,
			  GM_SEMODE_BOMB );
	    CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_RED_AT_MIN);
	    COM_DetectVibration( );
	}
}




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	CYP_Destroy						*/
/*	引数:	TARGET	*off   						       	*/
/*	説明:	サイファー壊れちゃった						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYP_Destroy(Work *work){


    work->control.step = DG_ZeroVector;
    work->control.mov.vx = 1000000.0f;
    work->control.mov.vz = 1000000.0f;
    SET_COLOR_CYP(0, 0, 0);
    //printf ("#####			CYP DEST\n");

    if (work->zidouset == OFF){
	printf ("#####			Cypher アクター終了\n");
	if ( COM_CypherResurrectionPermit() ){
	    printf ("#####	Cypher復活回数が指定されています\n");
	    printf ("#####	zidousetモードではないので復活できません\n");
	    printf ("#####	復活させたい場合は gcl で -zidouset オプションを指定して下さい\n");
	}
	work->r_trg.class |= TARGET_DEAD;
	GV_DestroyActor(work) ;
	return;
    }

    if ( COM_CypherResurrectionPermit() ){

	//	work->cyp_call_flag = OFF;
	//	work->noreturn = OFF;

	work->route = work->zido_root[0];

	printf ("Rout %d\n", work->route);

	ENE_InitRouteNavi( &work->rnavi, work->route, 0 );	//ルートナビ初期化
	CYP_InitTracePoint( work );
	//	CYPSetPointAction( work );

	/*ルートポイントに配置*/
	work->trg.pos.vx = work->control.mov.vx = work->rnavi.nodes[ 0 ].vx ;
	work->trg.pos.vy = work->control.mov.vy = work->rnavi.nodes[ 0 ].vy ;
	work->trg.pos.vz = work->control.mov.vz = work->rnavi.nodes[ 0 ].vz ;
	work->nowpos.vx = work->control.mov.vx ;
	work->nowpos.vy = work->control.mov.vy ;
	work->nowpos.vz = work->control.mov.vz ;

	printf("MOV: %f %f\n",work->control.mov.vx, work->control.mov.vz);
	printf("NOD: %f %f\n",work->rnavi.nodes[ 0 ].vx, work->rnavi.nodes[ 0 ].vz); 
	
	G1_STEP = CYP_NORMAL;
	work->ACTION_LAYER = EYE_LAYER_Normal;
	
	work->rot_body.vx = work->rot_body.vy = work->rot_body.vz = 0;
	
	work->mokuhyo_flg = BK2;
	work->route = work->zido_root[0];

	work->b_power.vital = CYP_BODY_VITAL;
	work->h_power.vital = CYP_HEAD_VITAL;
	
	work->kyodo_timer=0;
	work->damage_timer=0;
	work->tuiraku_timer=0;
	work->act_end_time=0;
	work->gun_count=0;
	work->chaff_time=0;
	work->dmg_flag=0;
	work->fire_flag=0;
	work->status = CYP_ACTIVE;
	work->body.objs->flag &= ~DG_FLAG_INVISIBLE;
	work->hom.status &= ~HOMING_SKIP;
	unset_alltrg_skip(work);
	work->r_trg.class |= TARGET_LOCKON;
	work->rctrl.flag |= RADAR_VISIBLE;
	work->ACTION_LAYER = EYE_LAYER_Normal;
	work->go_home_flag = OFF;
	printf ("####			Cypher 復活\n");
    }
    else {
	work->body.objs->flag |= DG_FLAG_INVISIBLE;
	work->hom.status |= HOMING_SKIP;
	set_alltrg_skip(work);
	work->r_trg.class &= ~TARGET_LOCKON;
	work->rctrl.flag &= ~RADAR_VISIBLE;
	G1_STEP = CYP_WAIT4RESURECT;
	work->ACTION_LAYER = EYE_LAYER_BREAK;
	//	printf ("#####			Cypher 今は我慢\n");
    }

    if (work->smokep != NULL){
	GV_DestroyOtherActor(work->smokep);
	work->smokep = NULL;
    }


    //	printf ("#####			END CYP DEST\n");

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	CheckMessage						*/
/*	引数:	Work *work   						       	*/
/*	説明:	メッセージを受信する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
    int n_msg, code  ;
    n_msg = work->control.n_msg ;
    msg = work->control.msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ] ;

printf  ("Get Mess Code %d\n", code);

	switch( code ) {


	case CYP_MSG_ROUTE_WARP4ZIDOUST:
	{
	    int now_node;
	    work->mokuhyo_flg=GO2;
	    work->noreturn = ON;
	    
	    work->route = msg->message[1];
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			work->route += COM_GetRootOfset( ) ;
		}
	    now_node = msg->message[2];
	    ENE_InitRouteNavi( &work->rnavi, work->route, now_node );	//ルートナビ初期化
	    CYP_InitTracePoint( work );
	    CYPSetPointAction( work );
	    /*ルートポイントに配置*/
	    work->trg.pos.vx = work->control.mov.vx = work->rnavi.nodes[ now_node ].vx ;
	    work->trg.pos.vy = work->control.mov.vy = work->rnavi.nodes[ now_node ].vy ;
	    work->trg.pos.vz = work->control.mov.vz = work->rnavi.nodes[ now_node ].vz ;
	    work->nowpos.vx = work->control.mov.vx ;
	    work->nowpos.vy = work->control.mov.vy ;
	    work->nowpos.vz = work->control.mov.vz ;
	    work->mode = MOVE_MODE ;
	}
	    break;

	case MSG_ROUTE_CHANGE :

	    work->route = msg->message[1] ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			work->route += COM_GetRootOfset( ) ;
		}
	    ENE_InitRouteNavi( &work->rnavi, work->route, msg->message[2]);
	    //	printf("b\n");
	    CYPSetPointAction( work );
	    work->trg.pos = work->rnavi.nodes[msg->message[2]] ;
	    work->trg.map = work->rnavi.mapbit[msg->message[2]] ;
	    work->trg.addr = HZX_GetAddress( work->trg.map,
					     &work->trg.pos, -1 ) ;
	    CYP_InitTracePoint( work );
	    work->mode = MOVE_MODE ;

#ifdef DEBUG_MODE
    printf (" ************** Route Change !! ***************\n");
    printf (" Route : %d\n",work->route);
    printf (" Node  : %d\n",msg->message[2]);
#endif
	    break ;

	case ENE_MSG_POINT_ACTION_START :
	    /*移動許可*/
	    work->status = CYP_ACTIVE ;
	    G1_STEP = CYP_NORMAL;
	    work->ACTION_LAYER = EYE_LAYER_Normal;
	    work->body.objs->flag &= ~DG_FLAG_INVISIBLE;
	    work->hom.status &= ~HOMING_SKIP;
	    unset_alltrg_skip(work);
	    work->r_trg.class |= TARGET_LOCKON;
	    work->rctrl.flag |= RADAR_VISIBLE;
	    break ;

	case CYP_MSG_SET_UNREAL :
	    work->unreal = ON;
	    work->control.step = DG_ZeroVector;
	    work->body.objs->flag |= DG_FLAG_INVISIBLE;
	    work->hom.status |= HOMING_SKIP;
	    set_alltrg_skip(work);
	    work->r_trg.class &= ~TARGET_LOCKON;
	    work->rctrl.flag &= ~RADAR_VISIBLE;


	    break ;

	case CYP_MSG_UNSET_UNREAL :
	    work->unreal = OFF;
	    work->body.objs->flag &= ~DG_FLAG_INVISIBLE;
	    work->hom.status &= ~HOMING_SKIP;
	    unset_alltrg_skip(work);
	    work->r_trg.class |= TARGET_LOCKON;
	    work->rctrl.flag |= RADAR_VISIBLE;


	    break ;

	case CYP_ZIDOU_POINT_CALL :
	    work->cyp_call_flag = ON;
	    work->noreturn = ON;
	    work->status = CYP_ACTIVE ;
	    work->body.objs->flag &= ~DG_FLAG_INVISIBLE;
	    work->hom.status &= ~HOMING_SKIP;
	    unset_alltrg_skip(work);
	    work->r_trg.class |= TARGET_LOCKON;
	    work->rctrl.flag |= RADAR_VISIBLE;

printf ("Cypher call!!\n");
	    break;

	case CYP_BIKKURI :
	    GM_SeSetMode( SD_E_BIKKRI01 , work->eye.eyepos,
			  GM_SEMODE_BOMB ) ;
	    CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_RED_AT_MIN);
	    break;

	case CYP_KILL :
	    GV_DestroyActor(work) ;
	    break;

	case CYP_FORCE_NORMAL_MODE :
	    G1_STEP = CYP_DEMO ;
	    break;
	}





	msg++ ;
    }
    

}



static u_char tmp_vib01[4];
static u_char tmp_vib02[4];

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CypherBrake						*/
/*	引数:	Work		*work						*/
/*		short		type						*/
/*	説明:	CYPHER破壊処理							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CypherBrake( Work *work, short type){

    if ( work->brake_proc_id != NULL ){
	GCL_ARGS	args;
	args.argc = 1;
	args.argv = &work->name;
	GCL_ExecProc(work->brake_proc_id, &args);

	printf ("cyp*********************call proc \n");
    }

    if (work->is_player_atack == ON){
	GM_MecaKillCount ++;
	if ( GM_MecaKillCount > GM_MAX_RESULT_COUNT ) GM_MecaKillCount = GM_MAX_RESULT_COUNT ;
    }

    //  ************爆発エフェクトとＳＥ
    if (work->type == NORMAL_TYPE){
printf ("break\n");
	NewCypherExplosion(&work->control.mov, 1000.0f, 0x00);
    }
    else {
printf ("break\n");
	NewCypherExplosion(&work->control.mov, 1000.0f, 0x02);
    }

    {	// 振動をつける
	FVECTOR shift;
	int distance;
	SAT_Minus_FVECTOR(&shift, &GM_PlayerFindPos, &work->control.mov);
	distance = (int)_FVecLen3( &shift);

	printf ("VIB %d\n", distance);
	if (distance < 20000){
	    int loop;
	    
	    tmp_vib01[0] = 1;
	    tmp_vib01[1] = 15;
	    tmp_vib02[0] = 255-(distance/80);
	    tmp_vib02[1] = 30;

	    tmp_vib01[2] = tmp_vib01[3] = 0;
	    for (loop=0; loop<4; loop++){
	    printf ("%d :",tmp_vib01[loop]);
	}
	printf ("\n");

	printf ("VIB02: ");
	for (loop=0; loop<4; loop++){
	    printf ("%d :",tmp_vib02[loop]);
	}
	printf ("\n");

	    NewPadVibration(tmp_vib01, 1);
	    NewPadVibration(tmp_vib02, 2);
	}
    }

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
#if 0
		extern void	C4MAN_Bomb( int name ) ;
		C4MAN_Bomb( work->name ) ;
#else
		extern void	C4MAN_Drop( int name ) ;
		C4MAN_Drop( work->name ) ;
#endif
	}

    GM_SeSetMode( SD_W_MINEEXP1, &work->control.mov, GM_SEMODE_BOMB );
    
    GM_SetNoise( NOISE_S ,&work->control.mov, GM_CurrentStageMap );

    work->act_end_time = TIME_BASE * 3;
}


extern void	NewSpark( FMATRIX * );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CallSpark							*/
/*	引数:	FVECTOR *pos							*/
/*	引数:	FVECTOR *force							*/
/*	説明:	火花を呼ぶ	       						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CallSpark(FVECTOR *pos ,FVECTOR *force, Work* work) {
    SVECTOR	rot ;
    FMATRIX	w ;

    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;

}




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetFaceDir							*/
/*	引数:	Work *work   						       	*/
/*	説明:	カメラの向きを設定する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetFaceDir(Work *work){
    FMATRIX	tmpmat;
    FVECTOR	trans;

    trans.vx = 0.0f;
    trans.vy = -270.0f;
    trans.vz = 500.0f;

    if (work->control.turn.vx < -700){
	work->control.turn.vx = -700;
    }
    if (work->control.turn.vx > 700){
	work->control.turn.vx = 700;
    }

    work->eye.rot = work->control.rot ;


#if 0
    {
	work->rotvy[19] = work->eye.rot.vy;
	{
	    short	loop;
	    for (loop=1; loop<20; loop++){
		work->rotvy[loop-1] = work->rotvy[loop];
	    }
	}    
	
	if (G1_STEP == CYP_ALERT){
	    if (work->type == GUN_TYPE){
		work->eye.rot.vy = work->rotvy[0];
	    }
	}
    }
#endif


    DG_SetPos( &BODYWORLD(&work->body, CAMERA_PARTS));

    if (work->type == GUN_TYPE){
	DG_MovePos( &trans );
    }
    DG_GetPos( &tmpmat ) ;
    GV_MatToVec(&tmpmat,&work->camera);

}
