/*
	wcaleact.c
	巡回兵、危険モードアクション

	1997/07/29 Y.Korekado
	$Id: spaleact.c,v 1.1.1.3 2002/11/19 11:44:24 Yoshizawa1 Exp $
	
*/
/*-------------------------------------------------------------------*/
static	void ActKokeru( ACTION *, int ) ;
static	void ActMgunShot( ACTION *, int ) ;
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

static	void ActReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
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

static	void ActNearAtk( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
//		AT_SetAction( act, 0, EM_ak_attack_near_kick_l, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_SetOffenseTarget( act, 0 ) ;
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

static	void ActMgunShot( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		int mot ;
		mot = ENE_ReadyGunMotion(act) ;
		AT_SetAction( act, 0, mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_Shoot( &(BODYWORLD( act->body, BODY21_RHAND )), act ) ;
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
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActReload( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;

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
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActKeri( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_SetOffenseTarget( act, 0 ) ;
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

/*--------------------------------------------------------------------*/
static int	SupportAlertCheckPad( act )
ACTION	*act ;
{
	int	mot ;

	if ( act->pad == SP_NONE ) return 0 ;

	switch ( act->pad ) {
		case SP_DOWNBACK :
			AT_SetModeFromPad( act, ActKokeru, EM_dam_out, act->pad ) ;
		break ;
		case SP_READYGUN :
			AT_SetModeFromPad( act, ActReadyGun, ENE_ReadyGunMotion(act), act->pad ) ;
		break ;
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_NEARATK :
			AT_SetModeFromPad( act, ActNearAtk, EM_ak_attack_near_kick_l, act->pad ) ;
		break ;
		case SP_MGUN_SHOT :
			AT_SetModeFromPad( act, ActMgunShot, ENE_ReadyGunMotion(act), act->pad ) ;
		break ;
		case SP_RELOAD :
			mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			AT_SetModeFromPad( act, ActReload, mot, act->pad ) ;
		break ;
		case SP_MEDICATION :
			AT_SetModeFromPad( act, ENE_ActMedication, EM_legl_morph_legl, act->pad ) ;
		break ;
		case SP_KERI :
			AT_SetModeFromPad( act, ActKeri, EM_shl_nom_attack_near, act->pad ) ;
		break ;
	}
	return 1 ;
}
