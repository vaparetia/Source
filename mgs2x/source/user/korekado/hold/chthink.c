/*
	chthink.c
	チェック兵、思考ルーチン

	2000/04/01 Y.Korekado
	$Id: chthink.c,v 1.1.1.3 2002/11/19 11:44:16 Yoshizawa1 Exp $
*/
enum {
	TH2_TRACE, TH2_DAMAGE, TH2_ENE_DAMAGE, TH2_NOISE, TH2_HOLDUP,
	TH2_INDISTINCT, TH2_TOUCH, TH2_DISCOVERY, TH2_RETURN, TH2_END,
	TH2_LOOKDOWN, TH2_FOUND, TH2_BOX
} ;
enum {
	TH3_ZONE_MOVE, TH3_DIRECT_MOVE, TH3_JUST_MOVE, TH3_DISCOVERY_POSE, TH3_ATTACK_SETUP,
	TH3_READY, TH3_LOOK, TH3_DISCOVERY_POSE_DOWN 
} ;
enum {
	SP_NONE,
	SP_STANDPOSE,
	SP_DISCOVERY,
	SP_READYGUN,
	SP_LOOKDOWN,
	SP_DISCOVERY_STOP,
} ;

/*----- --------------------------------------------*/
static	void	ReturnPositionStart( ENETHINK *entk ) ;
static	void	DiscoveryStart( ENETHINK *entk ) ;
static	void	ENE_CheckStart( ENETHINK *entk, int mode, int event, FVECTOR *trg_pos ) ;
extern	void	*AN_HeadMark( FMATRIX *world, int mark ) ;
/*----- --------------------------------------------*/
static void ActStandPose( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		act->c_motion_num[0] = act->keep_mot ;
		GM_ConfigObjectAction( act->body, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF );
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = 1 ;
			return ;
		}
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActDiscovery( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = 2 ;/* 見開き */
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = 1 ;
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActDiscoveryStop( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		MT3_CONTROL *mt3_ctrl ;

		/* モーションをそのポーズで止める */
		mt3_ctrl = &act->body->m_ctrl->mt3_ctrl[0] ;
		mt3_ctrl->flag |= MT3_SLEEP ;
//		mt3_ctrl->move->step = DG_ZeroVector ;

		act->sw->eye_anim = 2 ;/* 見開き */
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}
/*	そのままゲームオーバー */
/*
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = 1 ;
			return ;
		}
	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
*/
}

static void ActLookDown( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static int	CheckEneCheckPad( act )
ACTION	*act ;
{
	int	mot ;
	
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_STANDPOSE :
			AT_SetModeFromPad( act, ActStandPose, USHOLD_idle_2, act->pad ) ;
		break ;
		case SP_DISCOVERY :
			AT_SetModeFromPad( act, ActDiscovery, EM_find_ply, act->pad ) ;
		break ;
		case SP_READYGUN :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActReadyGun, mot, act->pad ) ;
		break ;
		case SP_LOOKDOWN :
			AT_SetModeFromPad( act, ActLookDown, EM_ijiiji, act->pad ) ;
		break ;
		case SP_DISCOVERY_STOP :
			AT_SetModeFromPad( act, ActDiscoveryStop, EM_find_ply, act->pad ) ;
		break ;
	}
	return 1 ;
}
/*----- --------------------------------------------*/
static void SetHoldFlag( Work *work, int flag )
{
	SET_FLAG ( *work->flag, flag ) ;
}
/*----- --------------------------------------------*/
static void Think3_ZoneMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_MOVE ;
		entk->count3 = 0 ;
		
		return ;
	}
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_DirectMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_JUST_MOVE ;
		entk->count3 = 0 ;
		/* 回転速度をholdeneにあわす */
		entk->ctrl->interp = 8 ;
		return ;
	}
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static void Think3_JustMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 50 ) < 0 ) {
		entk->think2 = TH2_END ;
		entk->think3 = TH3_READY ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static void Think3_LookDownReady( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 == 0 ) {
//		entk->act->dir = entk->trgpoint.dir ;
		entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->trgpoint.pos ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_LOOK ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}

static void Think3_LookDownLook( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_LOOKDOWN ;
	}

	if ( entk->act->act_end ) {
		entk->think2 = TH2_END ;
		entk->think3 = TH3_READY ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}

static void Think3_Ready( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_STANDPOSE ;
	entk->act->dir = entk->tmp_buff2[0] ;
	if ( entk->count3 > COUNT_VMODE(8) ) {
		entk->think3 = TH3_ALLEND ;
		entk->count3 = 0 ;
		return ;
	}
//printf(" turn.vy[%d] interp[%d] rot.vy[%d] \n",entk->ctrl->turn.vy,	entk->ctrl->interp ,entk->ctrl->rot.vy ) ;
	entk->count3 ++ ;
	
}

static void Think3_AllEnd( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_STANDPOSE ;
	entk->act->dir = entk->tmp_buff2[0] ;

	SetHoldFlag( entk->w, HLD_STATUS_ACTIVE_END ) ;
}

static void Think3_DiscoveryPose( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_DISCOVERY ;
		SetHoldFlag( entk->w, HLD_STATUS_NORMAL_DETECT ) ;
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_NORMAL ) ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_DiscoveryPoseDown( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_DISCOVERY_STOP ;
		SetHoldFlag( entk->w, HLD_STATUS_DOWN_DETECT ) ;
		COM_SetSpeak( EV_NOTICE_NOISE, entk ) ; /* んっ */
	}

	if ( entk->count3 == COUNT_VMODE(60*3) ) {
//		ENE_SetHeadMark2( entk->act, HMK2_TYPE_LONG_AT, ACT_HEADMARK2_NONE ) ;

		HeadMarkRun(&BODYWORLD( entk->act->body, HUMAN21_ATAMA ), 0 ) ;
		GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_NORMAL ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60*6) ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	entk->act->dir = entk->pl_eyei.dir ;
	

#if 1
if ( entk->count3 > COUNT_VMODE(300) ) {
	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		ReturnPositionStart( entk ) ;
		return ;
	}
}
#endif

	entk->count3 ++ ;
}


/*----- --------------------------------------------*/
static	void	Think2_Discovery( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_DISCOVERY_POSE :
	    	Think3_DiscoveryPose( entk ) ;
		break ;
	    case TH3_DISCOVERY_POSE_DOWN :
	    	Think3_DiscoveryPoseDown( entk ) ;
		break ;
	    case TH3_ATTACK_SETUP :
	    	Think3_ReadyGun( entk ) ;
			/* 発見中フラグＯＮ */
			COM_SetFlameFlag( CMFLAG_DETECT ) ;
		break ;
	}
}

static	void	Think2_ReturnPosition( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_MOVE :
	    	Think3_ZoneMove( entk ) ;
		break ;
	    case TH3_DIRECT_MOVE :
	    	Think3_DirectMove( entk ) ;
		break ;
	    case TH3_JUST_MOVE :
	    	Think3_JustMove( entk ) ;
		break ;
	}
}

static	void	Think2_LookDown( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_READY :
	    	Think3_LookDownReady( entk ) ;
		break ;
	    case TH3_LOOK :
	    	Think3_LookDownLook( entk ) ;
		break ;
	}
}

static	void	Think2_End( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_READY :
	    	Think3_Ready( entk ) ;
	    break ;
	    case TH3_ALLEND :
	    	Think3_AllEnd( entk ) ;
	    break ;
	}
}

/*----- --------------------------------------------*/
static	void	DetectCheck( entk )
ENETHINK	*entk ;
{
	Work *work ;
	
	work = entk->w ;
	if ( *work->flag & HLD_STATUS_DETECT_DELAY ) {
		DiscoveryStart( entk ) ;
	}
}

#include "modechng.c"
static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ReturnPositionStart( entk ) ;
	}

	TouchModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		ReturnPositionStart( entk ) ;
	}

	NoiseModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	FoundModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_FOUND) ) {
		ReturnPositionStart( entk ) ;
	}

	FoundModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ReturnPositionStart( entk ) ;
	}

	TraceModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		ReturnPositionStart( entk ) ;
	}

	IndistinctModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		ReturnPositionStart( entk ) ;
	}

	EneDamageModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		ReturnPositionStart( entk ) ;
	}

	BoxModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		ReturnPositionStart( entk ) ;
	}

	HoldUpModeChange( entk ) ;
	DetectCheck( entk ) ;
}

static	void	LookDownModeCheck( entk )
ENETHINK	*entk ;
{
	LookDownModeChange( entk ) ;
}

#if 0//No use
static	void	DiscoveryModeCheck( entk )
ENETHINK	*entk ;
{
}
#endif

/*----- --------------------------------------------*/
static	void	Check_Think1_Sneak( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_TOUCH :			/* タッチモード */
			THK_TouchMode( entk ) ;
			TouchModeCheck( entk ) ;
		break ;
	    case TH2_NOISE :			/* 物音モード */
			THK_NoiseMode( entk ) ;
			NoiseModeCheck( entk ) ;
		break ;
	    case TH2_FOUND :			/* 不審物発見モード */
			THK_FoundMode( entk ) ;
			FoundModeCheck( entk ) ;
		break ;
	    case TH2_TRACE :			/* 跡、追跡モード */
			THK_TraceMode( entk ) ;
			TraceModeCheck( entk ) ;
		break ;
	    case TH2_INDISTINCT :		/* 朧モード */
			THK_IndistinctMode( entk ) ;
			IndistinctModeCheck( entk ) ;
		break ;
	    case TH2_ENE_DAMAGE :		/* 味方ダメージ発見 */
			THK_EneDamage( entk ) ;
			EneDamageModeCheck( entk ) ;
		break ;
	    case TH2_BOX :				/* ダンボールモード */
			THK_BoxMode( entk ) ;
			BoxModeCheck( entk ) ;
		break ;
	    case TH2_HOLDUP :			/* ホールドアップモード */
			THK_HoldUpMode( entk ) ;
			HoldUpModeCheck( entk ) ;
		break ;


	    case TH2_DISCOVERY :		/* 発見 */
			Think2_Discovery( entk ) ;
		break ;
	    case TH2_RETURN :		/* 隊列に戻る */
	    	Think2_ReturnPosition( entk ) ;
	    	TravelModeChange( entk ) ;
			DetectCheck( entk ) ;
		break ;
	    case TH2_LOOKDOWN :		/* 下を覗く */
	    	Think2_LookDown( entk ) ;
			LookDownModeCheck( entk ) ;
		break ;

	    case TH2_END :		/* 終了 */
	    	Think2_End( entk ) ;
		break ;
	}
}


/*----- 高レベルモード移行チェック --------------------------------------------------*/
static	void	SneakModeCheack( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
	}
	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
	}
}
static	void	DamageModeCheack( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		entk->think1 = ENE_TH1_SNEAK ; 

		if ( ENE_PlayerHoldCheck( entk ) ) {
			entk->think2 = TH2_HOLDUP ; 
			THK_HoldUpModeStart( entk ) ;
		} else {
			entk->think2 = TH2_TOUCH ;
			THK_TouchModeStart( entk ) ;
		}

		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
	}



	if ( entk->notice & ENE_NOTICE_RES ) {
//		ENE_WatcherResurrectionMode( entk ) ;
	}
}

#if 0//no use
static	void	RessModeCheck( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_RES ) {
		switch( GM_AlertMode ) {
			default :
//				ENE_WatcherStartModeAvoid( entk ) ;
			break ;
		}
	}
}
#endif
/*----- 思考処理 --------------------------------------------------------*/
static void InitThinkParam( work )
Work	*work ;
{
	work->action.dir = -1 ;
	work->action.body_dir = -1 ;
	work->action.pad = 0 ;
	work->enethink.status2 = 0 ;
	work->enethink.thk_status = 0 ;
}

static void Think( work )
Work	*work ;
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	InitThinkParam( work ) ;

	switch( entk->think1 ){
		case ENE_TH1_SNEAK :
			Check_Think1_Sneak( entk ) ;
			SneakModeCheack( entk ) ;
		break ;
		case ENE_TH1_DAMAGE :
			ENE_Enemy_Think1_Damage( entk ) ;
			DamageModeCheack( entk ) ;
		break ;
		case ENE_TH1_RESURRECT :
//			ENE_WatcherResurrect( entk ) ;
//			RessModeCheck( entk ) ;
		break ;
	}

//printf("TH1[%d] TH2[%d] TH3[%d]\n",entk->think1,entk->think2,entk->think3 ) ;
}

/*----- 思考処理初期化 --------------------------------------------------------*/
static	void	ReturnPositionStart( ENETHINK *entk ) 
{
	entk->think2 = TH2_RETURN ;
	entk->think3 = TH3_ZONE_MOVE ;

	entk->sense.status = RADAR_COLOR_BLUE ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->act->CheckPad = CheckEneCheckPad ;

//	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->def_pos, entk->ctrl->hzx_id ) ;
	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit ) ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
}

static	void	DiscoveryStart( ENETHINK *entk )
{

printf("11111111kor[%d]??\n",entk->think2);
	if ( entk->think2 == TH2_LOOKDOWN ) {
		entk->think2 = TH2_DISCOVERY ;
		entk->think3 = TH3_DISCOVERY_POSE_DOWN ;
printf("2222222koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee??\n");
	} else {
		entk->think2 = TH2_DISCOVERY ;
		entk->think3 = TH3_DISCOVERY_POSE ;
printf("33333333koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee??\n");
	}

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->act->CheckPad = CheckEneCheckPad ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
}

static	void	ENE_CheckStart( ENETHINK *entk, int mode, int event, FVECTOR *trg_pos )
{
	CHKENEWORK	*chkwork ;
	CNCTOBJ 	*cnct ;

#if 0
	ENE_EnemyDown( entk->act ) ;
	entk->act->bodyp.dammode = DAM_MODE_MASUI_DOWN ;
	ENE_EnemyStartModeDamage( entk ) ;
#endif

	entk->think1 = ENE_TH1_SNEAK ; 

	if ( event == HLD_EVENT_LIGHTER ) {
		extern void *NewObjectFall( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec, 
			int model_name, float bound_ratio, int life );
		entk->act->CheckPad = CheckEneCheckPad ;
		entk->think2 = TH2_LOOKDOWN ;
		entk->think3 = TH3_READY ;
		entk->count3 = 0 ;
		ENE_SetTrgpPoint( &(entk->trgpoint), trg_pos, entk->ctrl->hzx_id ) ;
		NewObjectFall( trg_pos, &DG_ZeroSVector, &DG_ZeroVector, 
			GV_StrCode( "usp_amo" ), 0.2f, -1 );
		AT_SetType( entk->act, (ENE_TYPE_NO_HEAR) ) ;	/*耳聞こえない*/

		return ;
	}

	if( mode == ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	} else if( mode == HLD_EVENT_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	} else {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}

	chkwork = (CHKENEWORK *)entk->character ;
	cnct = chkwork->cnct ;
	CNCT_CnctObjNum( cnct, HUMAN21_MIGI_TE ) ;
	CNCT_CnctObjShift( cnct, &HOLD_WP_Shift[WP_POS_RIGHT_HAND] ) ;
	CNCT_CnctObjRot( cnct, &HOLD_WP_Rot[WP_POS_RIGHT_HAND] ) ;
}

