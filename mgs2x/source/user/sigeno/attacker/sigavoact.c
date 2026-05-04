/*
	sigavoact.c
	攻撃兵、回避モード固有アクション

	2000/02/04 K.SIgeno
	$Id: sigavoact.c,v 1.1.1.3 2002/11/19 11:49:04 Yoshizawa1 Exp $
*/
/*-------------------------------------------------------------------*/


extern void AT_ActMoveRun(ACTION * ,int);


static	void ActKokeru( ACTION *, int ) ;
static	void ActDiscovery( ACTION *, int ) ;
//static	void ActMgunShot( ACTION *, int ) ;

/*-------------------------------------------------------------------*/
static	void ActKokeru( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_dam_out, 0, MOTION_MASK_FULL, 
		ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
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
		AT_SetAction( act, 0, ENE_DiscoveryMotion(act), 0, MOTION_MASK_FULL,
		ACT_INTERP_DEF ) ;
		ENE_SetHeadMark( act, BODY21_HEAD, HEADMARK_BR ) ;
	}
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	act->ctrl->step = DG_ZeroVector ;
}

void AT_ActMoveAvoid( act, time )
ACTION	*act ;
int		time ;
{
	float speed = 0.0F;
	AT_SetActStatus( act,(ACT_STATUS_STAND|ACT_STATUS_MOVE));
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	
//	if (( time == 0 )||(act->motion_num != act->keep_mot)) {
	if (( time == 0 )||(act->c_motion_num[0] != act->keep_mot)) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
	}

	if( act->pad != act->keep_pad ){
		 act->CheckPad( act );
		return ;
	}

	if(speed != 0.0F ){
		act->ctrl->step.vx += speed * _RsinF( (int)act->dir ) ;
		act->ctrl->step.vz += speed * _RcosF( (int)act->dir ) ;
	}

}

/*--------------------------------------------------------------------*/
static int	AttackerAvoidCheckPad( act )
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
#if 0
		case SP_READYGUN :
			act->keep_mot = EM_kamae_gun_high ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActKeepMotion ) ;
			return 1 ;
		break ;
#endif
		case SP_CAUT_STAND :
			if(act->bodyp.type & ENE_TYPE_SHIELD){
				act->keep_mot = EM_shl_nom_idle ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_cle_signe_idle ;
			}else {
				act->keep_mot = EM_cle_stand02 ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActKeepMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_GOSIGN :
			if(act->bodyp.type & ENE_TYPE_SHIELD){
				act->keep_mot = EM_shl_nom_signe_go ;
			}else if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_cle_signe_go ;
			}else {
				act->keep_mot = EM_cle_signe_go ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_PEEP_L :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_cle_look_l ;
			}else {
				act->keep_mot = EM_cle_look_l ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_PEEP_R :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_ak_nom_cle_look_r ;
			}else {
				act->keep_mot = EM_cle_look_r ;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_CLE_PEEP_D :
			act->keep_mot = EM_cle_look_d ;
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActOneTimeMotion ) ;
			return 1 ;
		break ;
		case SP_MV_RUN :
			if(act->bodyp.type & ENE_TYPE_HITECH){
				act->keep_mot = EM_htc_m4_nom_run_atk;
			}else {
				act->keep_mot = EM_run_atk;
			}
			act->keep_pad = act->pad ;
			AT_SetMode( act, AT_ActMoveRun) ;
			return 1 ;
		break ;

	}
	return 0 ;
}


