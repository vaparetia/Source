/*
	spcauact.c
	巡回兵、潜入モードアクション

	1999/07/29 Y.Korekado
	$Id: spcauact.c,v 1.1.1.3 2002/11/19 11:44:25 Yoshizawa1 Exp $
	
*/
/*-------------------------------------------------------------------*/
static void ActDiscovery( ACTION *, int ) ;
/*-------------------------------------------------------------------*/
static	void ActDiscovery( act, time )
ACTION	*act ;
int		time ;
{
//	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, ENE_DiscoveryMotion(act), 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->CheckPad( act ) ) {
		return ;
	}

	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActOutOfWay( act, time )
ACTION	*act ;
int		time ;
{
//	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->CheckPad( act ) ) {
		return ;
	}

	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActKeepMotion( act, time )
ACTION	*act ;
int		time ;
{
	/* 注！！キープモーションは移動しない */
//	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
//	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		if ( (act->keep_mot == EM_cle_stand02) ) {
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				ENE_SetMarActionPBreak( act, 0, EM_shl_nom_idle, 0, 
					MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar ) ;
			} else if (act->bodyp.type & ENE_TYPE_HITECH) {
				ENE_SetMarActionPBreak( act, 0, EM_htc_ak_cle_signe_idle, 0, 
					MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar ) ;
			} else {
				ENE_SetMarActionPBreak( act, 0, act->keep_mot, 0, 
					MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar  ) ;
			}
		} else {
			ENE_SetMarActionPBreak( act, 0, act->keep_mot, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar  ) ;
		}
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
static int	SupportSneakCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_DISCOVERY :
			AT_SetModeFromPad( act, ActDiscovery, ENE_DiscoveryMotion(act), act->pad ) ;
		break ;
		case SP_RADIOCALL :
			AT_SetModeFromPad( act, ENE_ActRadioCall, EM_cle_call_team, act->pad ) ;
		break ;
		case SP_READYGUN :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_ReadyGunMotion(act), act->pad ) ;
		break ;
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_OUTOF_WAY_R :
			AT_SetModeFromPad( act, ActOutOfWay, EM_cle_kaniwalk_r, act->pad ) ;
		break ;
		case SP_OUTOF_WAY_L :
			AT_SetModeFromPad( act, ActOutOfWay, EM_cle_kaniwalk_l, act->pad ) ;
		break ;
		case SP_NORMAL_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_NormalStandMotion(act), act->pad ) ;
		break ;
		case SP_UNREAL :
			AT_SetModeFromPad( act, ENE_ActUnreal, ENE_StandMotion(act), act->pad ) ;
		break ;
	}

	return 1 ;
}
