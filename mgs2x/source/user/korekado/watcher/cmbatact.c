/*
	cmbatact.c

	$Id: cmbatact.c,v 1.1.1.3 2002/11/19 11:44:29 Yoshizawa1 Exp $
	
*/
/*-------------------------------------------------------------------*/
static	void ActKokeru( ACTION *, int ) ;
/*-------------------------------------------------------------------*/
static	void ActKokeru( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_dam_bomb_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}

#define AROUND_BRANK	COUNT_VMODE(270)
static void ActAround( act, time )
ACTION	*act ;
int		time ;
{
	/* 注！！キープモーションは移動しない */
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;

	/* turn.vyにセットされてから */
	if ( (time%AROUND_BRANK) == 0 ) {
		act->aim_dir = act->ctrl->turn.vy  ;
	} else if ( (time%AROUND_BRANK) == AROUND_BRANK/3 ) {
		act->aim_dir = act->ctrl->turn.vy + 512 ;
	} else if ( (time%AROUND_BRANK) == AROUND_BRANK*2/3 ) {
		act->aim_dir = act->ctrl->turn.vy - 512 ;
	}

	if ( (time%AROUND_BRANK) == AROUND_BRANK-1 ) {
		act->act_end = 1 ;
	}
}

static	void ActRunSide( act, time )
ACTION	*act ;
int		time ;
{
//	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*4.0f ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}
/*--------------------------------------------------------------------*/
static int	WatcherAlertCheckPad( act )
ACTION	*act ;
{
	int	mot ;

	if ( act->pad == SP_NONE ) return 0 ;

	switch ( act->pad ) {
		case SP_DOWNBACK :
			AT_SetModeFromPad( act, ActKokeru, EM_dam_out, act->pad ) ;
		break ;
		case SP_READYGUN :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActReadyGun, mot, act->pad ) ;
		break ;
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_NEARATK :
			AT_SetModeFromPad( act, ENE_ActNearAtk, EM_ak_attack_near_kick_l, act->pad ) ;
		break ;
		case SP_MGUN_SHOT :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActMgunShot, mot, act->pad ) ;
		break ;
		case SP_RELOAD :
			mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			AT_SetModeFromPad( act, ENE_ActReload, mot, act->pad ) ;
		break ;
		case SP_NSIGHT_ON :
			AT_SetModeFromPad( act, ENE_ActNSight_ON, EM_gogle_on, act->pad ) ;
		break ;
		case SP_MEDICATION :
			AT_SetModeFromPad( act, ENE_ActMedication, EM_legl_morph_legl, act->pad ) ;
		break ;
		case SP_KERI :
			AT_SetModeFromPad( act, ENE_ActKeri, EM_shl_nom_attack_near, act->pad ) ;
		break ;
		case SP_HANG_WATCH :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_AROUND :
			AT_SetModeFromPad( act, ActAround, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_RUN_R :
			AT_SetModeFromPad( act, ActRunSide, EM_cle_kaniwalk_r, act->pad ) ;
		break ;
		case SP_RUN_L :
			AT_SetModeFromPad( act, ActRunSide, EM_cle_kaniwalk_l, act->pad ) ;
		break ;
	}
	return 1 ;
}
