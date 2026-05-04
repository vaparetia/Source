/*
	rtthink.c
	ルート兵、思考ルーチン

	2000/12/20 Y.Korekado
	$Id: rtthink.c,v 1.1.1.3 2002/11/19 11:44:17 Yoshizawa1 Exp $
*/
#define TH1_DISCOVERY	(100)
enum {
	TH2_TRAVEL, TH2_TRACE, TH2_ENE_DAMAGE, TH2_NOISE, TH2_BOX, 
	TH2_HOLDUP,	TH2_INDISTINCT, TH2_TOUCH, TH2_DISCOVERY, TH2_RETURN,
	TH2_FOUND, TH2_END
} ;
enum {
	TH3_RETURN_WAIT, TH3_RETURN_TRAVEL, TH3_GO_NEXT_END,TH3_GO_NEXT, TH3_POINT_ACTION,
	TH3_ZONE_MOVE, TH3_DIRECT_MOVE, TH3_JUST_MOVE, TH3_DISCOVERY_POSE, TH3_ATTACK_SETUP,
	TH3_READY, TH3_SCOT_MOVE
} ;
enum {
	SP_NONE,
	SP_STANDPOSE,
	SP_DISCOVERY,
	SP_READYGUN,
	SP_GOTO_END,	/* 体の向きを変えながら直進 */
	SP_SLEEP,
	SP_SCOTMOVE,
} ;

#define WC_SP_ZZZ (3)
/*----- --------------------------------------------*/
static	void	ReturnPositionStart( ENETHINK * ) ;
static	void	DiscoveryStart( ENETHINK	*entk ) ;
static	void	RootStart( ENETHINK	*entk, int mode ) ;
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

static void ActScotMove( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_walk, 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !(act->CheckPad( act )) ) {
			SetMode( act, ENE_ActMove ) ;
		}
		return ;
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

/*	歩く	*/
void aENE_ActGoToEnd( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	float	speed ;

	ctrl = act->ctrl ;

	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_MOVE  ) ;

	if ( act->CheckDamage( act ) ) {
		act->ctrl->turn.vz = 0 ;
		return ;
	}

	if ( act->dir < 0 ) {
		SetMode( act, ENE_ActStandStill ) ;
		act->ctrl->turn.vz = 0 ;
		return ;
	}

	if ( act->pad != SP_GOTO_END ) {
		if ( act->CheckPad( act ) ) 	return ;
		SetMode( act, ENE_ActStandStill ) ;
		ctrl->turn.vy  = act->dir ;
		act->ctrl->turn.vz = 0 ;
		return ;
	}

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_walk, 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
		act->ctrl->interp = 15 ;
	}


	speed = STEP_VMODE(50.0F) - ( time * STEP_VMODE(8.0F) ) ;
	if ( speed < STEP_VMODE(5.0F) ) speed = STEP_VMODE(5.0F) ;
	ctrl->turn.vy  = act->body_dir ;

	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
	ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;

	ENE_Incline( act->ctrl ) ;
	act->ctrl->turn.vz /= 2 ;
	act->ctrl->turn.vz = 0 ;
	
//printf(" step x= %f step z = %f \n",ctrl->step.vx,ctrl->step.vz);
//printf(" act turn=%d rot=%d \n",ctrl->turn.vy,ctrl->rot.vy);
}

/*----------------------------------------------------------------------------*/

static int	CheckEneCheckPad( act )
ACTION	*act ;
{

	if ( act->pad == SP_NONE ) return 0 ;

	switch ( act->pad ) {
		case SP_STANDPOSE :
			AT_SetModeFromPad( act, ActStandPose, USHOLD_idle_2, act->pad ) ;
		break ;
		case SP_DISCOVERY :
			AT_SetModeFromPad( act, ActDiscovery, EM_find_ply, act->pad ) ;
		break ;
		case SP_READYGUN :
			AT_SetModeFromPad( act, ENE_ActReadyGun, ENE_ReadyGunMotion(act), act->pad ) ;
		break ;
		case SP_GOTO_END :
			/* 歩き続ける */
			ENE_SetActionPBreak( act, 0, EM_walk, 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
			AT_SetMode( act, aENE_ActGoToEnd ) ;
		break ;
		case SP_SLEEP :
			AT_SetModeFromPad( act, ENE_ActZzz, EM_zzz, act->pad ) ;
		break ;
		case SP_SCOTMOVE :
			AT_SetModeFromPad( act, ActScotMove, EM_walk, act->pad ) ;
		break ;
	}
	return 1 ;
}

/*----------------------------------------------------------------------------*/
static	void	DetectCheck( entk )
ENETHINK	*entk ;
{
	Work *work ;
	
	work = entk->w ;

	if ( *(work->flag) & HLD_STATUS_SCOT ) return ;

	if ( *(work->flag) & HLD_STATUS_DETECT_DELAY ) {
		DiscoveryStart( entk ) ;
	}
}

static void SetHoldFlag( Work *work, int flag )
{
	if ( flag == HLD_STATUS_NORMAL_DETECT ) {
		if ( *(work->flag) & HLD_STATUS_SCOT ) {
			flag = HLD_STATUS_SCOT_DETECT ;
		}
	}

	SET_FLAG ( *work->flag, flag ) ;
}
/*----------------------------------------------------------------------------*/
static int RouteCheck( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;
	
	rnavi = entk->rnavi ;
	if ( rnavi->next_route == rnavi->c_route ) return 0 ;

	printf("ene[%d] Change Root [%d]->[%d]\n",entk->id, rnavi->c_route, rnavi->next_route ) ;
	ENE_ChangeRoute( entk,rnavi->next_route ) ;
	
	if ( entk->rnavi->chang_node < 0 ) {
		ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	} else {
		entk->rnavi->next_node = entk->rnavi->chang_node ;
		ENE_SetTrgpNode( entk->rnavi, &(entk->trgpoint) ) ;
	}
	entk->think1 = ENE_TH1_SNEAK ; 
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;

	return  1 ;
}

/*----------------------------------------------------------------------------*/
static void Think3_ReturnWait( entk )
ENETHINK		*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_RETURN_TRAVEL ; 
		entk->count3 = 0 ;
	}

	entk->count3 ++ ;
}

static void Think3_ReturnTravel( entk )
ENETHINK		*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
		entk->act->move_s = MoveWalk ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_GoNext( entk )
ENETHINK		*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		if ( RouteCheck( entk ) ) return ;

		entk->rnavi->p_action = entk->rnavi->pa_action[(int)entk->rnavi->next_node] ;
		entk->rnavi->p_dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
		entk->rnavi->p_acttime = entk->rnavi->pa_time[(int)entk->rnavi->next_node] ;
		entk->think3 = TH3_POINT_ACTION ; 
		entk->count3 = 0 ;

		if ( entk->ctrl->turn.vy != entk->rnavi->p_dir ) {
			entk->act->dir = entk->rnavi->p_dir ;
			entk->trgpoint.tmp_dirbuff = entk->rnavi->p_dir ;
			entk->act->body_dir = 4095 & GV_NearExp4P( entk->ctrl->turn.vy, entk->trgpoint.tmp_dirbuff ) ;
			entk->think3 = TH3_GO_NEXT_END ; 
		}
		
		return ;
	}

	if ( *(((ROOTENEWORK *)entk->character)->flag) & HLD_STATUS_SCOT ) {
		/* 方向転換は悠々と */
//		entk->act->ctrl->interp = ACT_CTRL_INTERP_SLOWLY ;
//		entk->act->ctrl->interp = 60 ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}



static void Think3_GoNextEnd( entk )
ENETHINK	*entk ;
{
	int flag ;

	flag = 0 ;
	if ( !(_PosInRangeXZ( &entk->ctrl->mov, &(entk->trgpoint.pos), 500 )) ) {
		flag = 1 ;
	}
	if( entk->count3 > COUNT_VMODE(60) ) {
		flag = 1 ;
	}
	if( entk->count3 > COUNT_VMODE(10) && GV_DiffDirAbs( entk->ctrl->rot.vy, entk->trgpoint.tmp_dirbuff ) < 64 ) {
		flag = 1 ;
	}
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 50 ) < 0 ) {
		flag = 1 ;
	}

	if ( flag ) {
		entk->act->dir = entk->rnavi->p_dir ;
		entk->think3 = TH3_POINT_ACTION ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->pad = SP_GOTO_END ;

	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
	entk->act->aim_dir = entk->rnavi->p_dir ;

	entk->act->dir = entk->trgpoint.dir ;
	entk->act->body_dir = entk->trgpoint.tmp_dirbuff ;

//printf("dir[%d] bodydir[%d] \n",entk->act->dir,entk->act->body_dir);

	entk->count3 ++ ;
}

static void Think3_PointAction( entk )
ENETHINK		*entk ;
{
	switch ( entk->rnavi->p_action ) {
		case WC_SP_ZZZ:
			entk->act->pad = SP_SLEEP ;
		break ;
	}

	if ( entk->rnavi->p_acttime <= 0) {
		if ( RouteCheck( entk ) ) return ;
		if ( entk->rnavi->n_nodes > 1 ) {
			ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
			if ( *(((ROOTENEWORK *)entk->character)->flag) & HLD_STATUS_SCOT ) {
				entk->think3 = TH3_SCOT_MOVE ; 
			} else {
				entk->think3 = TH3_GO_NEXT ; 
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->rnavi->p_acttime -- ;
	entk->count3 ++ ;
}

static void Think3_ScotMove( entk )
ENETHINK		*entk ;
{
	entk->act->pad = SP_SCOTMOVE ;
	
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		if ( RouteCheck( entk ) ) return ;

		entk->rnavi->p_action = entk->rnavi->pa_action[(int)entk->rnavi->next_node] ;
		entk->rnavi->p_dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
		entk->rnavi->p_acttime = entk->rnavi->pa_time[(int)entk->rnavi->next_node] ;
		entk->think3 = TH3_POINT_ACTION ; 
		entk->count3 = 0 ;

		if ( entk->ctrl->turn.vy != entk->rnavi->p_dir ) {
			entk->act->dir = entk->rnavi->p_dir ;
			entk->trgpoint.tmp_dirbuff = entk->rnavi->p_dir ;
			entk->act->body_dir = 4095 & GV_NearExp4P( entk->ctrl->turn.vy, entk->trgpoint.tmp_dirbuff ) ;
			entk->think3 = TH3_GO_NEXT_END ; 
		}
		
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;

	if ( entk->count3 > COUNT_VMODE(8) ) {
		entk->think3 = TH3_GO_NEXT ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
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

//	SetHoldFlag( entk->w, HLD_STATUS_ACTIVE_END ) ;
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
/*	巡回	*/
static	void	Think2_Travel( entk )
ENETHINK		*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_WAIT :		    /* 指定位置に移動 */
	    	Think3_ReturnWait( entk ) ;
		break ;
	    case TH3_RETURN_TRAVEL :		    /* 指定位置に移動 */
	    	Think3_ReturnTravel( entk ) ;
		break ;
	    case TH3_GO_NEXT_END :				/* 次の巡回ポイントへ移動 */
			Think3_GoNextEnd( entk ) ;
		break ;
	    case TH3_GO_NEXT :					/* 次の巡回ポイントへ移動 */
			Think3_GoNext( entk ) ;
		break ;
	    case TH3_SCOT_MOVE :					/* 次の巡回ポイントへ移動 */
			Think3_ScotMove( entk ) ;
		break ;
	    case TH3_POINT_ACTION :				/* ポイントアクション */
	    	Think3_PointAction( entk ) ;
		break;
	}
}

static	void	Think2_Discovery( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_DISCOVERY_POSE :
	    	Think3_DiscoveryPose( entk ) ;
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
#include "modechng.c"
static	void	TravelModeCheck( entk )
ENETHINK	*entk ;
{
	TravelModeChange( entk ) ;
}

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

#if 0//no use
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
	    case TH2_TRAVEL :		/* 巡回 */
			Think2_Travel( entk ) ;
			TravelModeCheck( entk ) ;
		break ;

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
		break ;
	    case TH2_END :		/* 終了 */
	    	Think2_End( entk ) ;
		break ;
	}
}

static	void	Check_Discovery( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_DISCOVERY :		/* 発見 */
			Think2_Discovery( entk ) ;
		break ;
	}
}

/*----- 高レベルモード移行チェック --------------------------------------------------*/
static int ScotDamageCheck( entk )
ENETHINK	*entk ;
{
	TARGET_PARTS	*parts ;
	CAPTURE_TARGET	*cap ;
	TARGET			*def, *child ;
	int i, n ;

	/* パワーターゲット */
	def = &(entk->act->bodyp.deftrg) ;

	/* 子ターゲットチェック */
	for ( i = 1; i < MAX_TARGET_PARTS_LEVELS; i ++ ) {
		parts = KR_GetTargetPart( def, i ) ;
		if ( parts == NULL || ( parts->flag & TARGET_SKIP ) ) continue ;
		n = parts->n_parts ;
		child = ( TARGET * )parts->parts ;
		while( -- n >= 0 ) {
			if ( child->damaged & TARGET_POWER ) {
				return 1 ;
			}
			child ++ ;
		}
	}

	/* 親ターゲットチェック */
	if ( def->damaged & (TARGET_POWER) ) {
		return 1 ;
	}

	/* スタンダメージ */
	/* キャプチャーターゲット */
	cap = &(entk->act->bodyp.capture) ;
    if ( cap->capture != NULL ) { /* 捕まった */
		return 1 ;
	}

	return 0 ;
}

static	void	SneakModeCheack( entk )
ENETHINK	*entk ;
{
	ROOTENEWORK *rtwork ;

	switch( GM_AlertMode ) {
	}

	rtwork = (ROOTENEWORK *)entk->character ;
	if ( *(rtwork->flag) & HLD_STATUS_SCOT ) {
		if ( ScotDamageCheck( entk ) ) {
			DiscoveryStart( entk ) ;
		}
	} else {
		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
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
//			RootStart( entk, ENE_NOTICE_NOISE ) ;
//			ENE_WatcherResurrectionMode( entk ) ;
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

		case TH1_DISCOVERY :
			Check_Discovery( entk ) ;
		break ;

		case ENE_TH1_RESURRECT :
//			ENE_WatcherResurrect( entk ) ;
//			RessModeCheck( entk ) ;
		break ;
	}
}

/*----- 思考処理初期化 --------------------------------------------------------*/
static	void	ReturnPositionStart( entk )
ENETHINK	*entk ;
{
#if 0
	entk->think2 = TH2_RETURN ;
	entk->think3 = TH3_ZONE_MOVE ;
#else
	entk->think1 = ENE_TH1_SNEAK ; 
	entk->think2 = TH2_TRAVEL ;
	entk->think3 = TH3_RETURN_WAIT ; 
#endif

	entk->sense.status = RADAR_COLOR_BLUE ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->act->CheckPad = CheckEneCheckPad ;

//	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->def_pos, entk->ctrl->hzx_id ) ;
	ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
}

static	void	DiscoveryStart( entk )
ENETHINK	*entk ;
{
	entk->think1 = TH1_DISCOVERY ;
	entk->think2 = TH2_DISCOVERY ;
	entk->think3 = TH3_DISCOVERY_POSE ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->act->CheckPad = CheckEneCheckPad ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
}

static	void	RootStart( entk, mode )
ENETHINK	*entk ;
int			mode ;
{
	entk->sense.status = RADAR_COLOR_BLUE ;

	entk->think1 = ENE_TH1_SNEAK ; 

	entk->think2 = TH2_TRAVEL ;
	entk->think3 = TH3_RETURN_WAIT ; 

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->act->CheckPad = CheckEneCheckPad ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
/*
	if( mode == ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	} else {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
*/
}
