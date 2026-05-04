/*
	wcavoact.c
	巡回兵、回避モードアクション

	1999/07/29 Y.Korekado
	$Id: spavoact.c,v 1.1.1.3 2002/11/19 11:44:24 Yoshizawa1 Exp $
	
*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
static void ActReload( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->magg = 1 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->magg = 1 ;
			return ;
		}
	}

	if ( time == COUNT_VMODE(19) ) act->sw->magg = 4 ;
	if ( time == COUNT_VMODE(66) ) act->sw->magg = 2 ;
	if ( time == COUNT_VMODE(101) ) act->sw->magg = 1 ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->sw->magg = 1 ;
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
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

/*--------------------------------------------------------------------*/
static int	SupportAvoidCheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_RELOAD :
			act->keep_mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActReload ) ;
			return 1 ;
		break ;
		case SP_READYGUN :
			act->keep_mot = ENE_ReadyGunMotion(act) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActReadyGun ) ;
			return 1 ;
		break ;
	}

	return 0 ;
}
