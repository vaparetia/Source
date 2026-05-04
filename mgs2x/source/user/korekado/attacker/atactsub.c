/*
	wcactsub.c
	見張り兵、固有アクション

	1997/07/07 Y.Korekado
	$Id: atactsub.c,v 1.1.1.3 2002/11/19 11:43:58 Yoshizawa1 Exp $
	
*/
/*-------------------------------------------------------------------*/
static	void ActKokeru( ACTION *, int ) ;
static	void ActDiscovery( ACTION *, int ) ;
static	void ActMgunShot( ACTION *, int ) ;

/*-------------------------------------------------------------------*/
static	void ActKokeru( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_dam_out, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
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
}

static	void ActDiscovery( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, ENE_DiscoveryMotion(act), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
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
	act->ctrl->step = DG_ZeroVector ;
}

static	void ActMgunShot( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		int mot ;
		mot = ENE_ReadyGunMotion(act) ;
		AT_SetAction( act, 0, mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
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
	act->ctrl->step = DG_ZeroVector ;
}

