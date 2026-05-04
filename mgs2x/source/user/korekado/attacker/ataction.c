/*
	wcaction.c
	見張り兵のアクションコントロール

	1997/07/07 Y.Korekado
	$Id: ataction.c,v 1.1.1.3 2002/11/19 11:43:58 Yoshizawa1 Exp $
	
*/
static	int	CheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_DOWNBACK :
			act->keep_mot = EM_dam_out ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActKokeru ) ;
			return 1 ;
		break ;
		case SP_DISCOVERY :
			act->keep_mot = ENE_DiscoveryMotion(act) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActDiscovery ) ;
			return 1 ;
		break ;
		case SP_READYGUN :
			act->keep_mot = ENE_ReadyGunMotion(act) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActKeepMotion ) ;
			return 1 ;
		break ;
		case SP_CAUT_STAND :
			act->keep_mot = ENE_CleStandMotion( act ) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActKeepMotion ) ;
			return 1 ;
		break ;
		case SP_MGUN_SHOT :
			act->keep_mot = ENE_ReadyGunMotion(act) ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ActMgunShot ) ;
			return 1 ;
		break ;
		case SP_RELOAD :
			act->keep_mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_GOSIGN :
			act->keep_mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_ak_cle_signe_go:EM_cle_signe_go ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_PEEP_L :
			act->keep_mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_ak_nom_cle_look_l:EM_cle_look_l ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_PEEP_R :
			act->keep_mot=(act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_ak_nom_cle_look_r:EM_cle_look_r;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
	}
	return 0 ;
}

/*--------------------------------------------------------------------*/
static	void	Action ( work )
Work	*work ;
{
	CONTROL		*ctrl ;

	ctrl = &work->control ;

	AT_Action( &work->action ) ;

	ctrl->height = work->body.height ;
	if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F ;
	ctrl->step.vy -= 64.0F ;
}
