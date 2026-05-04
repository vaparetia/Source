/*
	atavoact.c
	攻撃兵、回避モード固有アクション

	1997/07/26 Y.Korekado
	$Id:
	
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

static void ActPeepD( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

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
	AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_D ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActPeepDuct( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

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
	AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_D ) ;

//	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	if ( time > COUNT_VMODE(90) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActPeepToilet( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( time > 40 ) {
		AT_SetActStatus( act, ACT_STATUS_SPHERE_300 ) ;
	}

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->keep_mot == EM_toilet_peep_u ) {
		/*プレイヤーにはLOCKER、NARROW , DARKAREAがたっている */
		/* 上覗き込み */
//		if ( (time > 84) &&  ( time < 276 ) ) {
		if ( (time > COUNT_VMODE(74)) &&  ( time < COUNT_VMODE(350) ) ) {
			AT_SetActStatus( act, ACT_STATUS_GUN_FREE  ) ;
		}
//		if ( (time > 160) &&  ( time < 250 ) ) {
		if ( (time > COUNT_VMODE(150)) &&  ( time < COUNT_VMODE(250) ) ) {
			AT_SetActStSt( act, ACT_STST_PEEP_TOILET ) ;
		}
	} else {
		/* 下覗き込み */
		if ( (time > COUNT_VMODE(118)) &&  ( time < COUNT_VMODE(195) ) ) {
			AT_SetActStSt( act, ACT_STST_PEEP_TOILET ) ;
		}
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
//	AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_D ) ;






	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActToiletKnock( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

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
//	AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_D ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActCleAround( act, time )
ACTION	*act ;
int		time ;
{
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

	if ( time > COUNT_VMODE(270) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActMukadeWait( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_IK_MUKADE ) ;

	/* 注！！キープモーションは移動しない */
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
#if 0
		/* ???頭か首をオーバーライドすると右手がしてくれない */
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		AT_SetAction( act, 1, EM_cau_mukade_idle, 0, MOTION_MASK_UPPER/*0x7fe*/, ACT_INTERP_DEF ) ;
printf(" base [%d] over[%d]\n",act->keep_mot,EM_cau_mukade_idle ) ;
		MT_ResetMotionData( act->body->m_ctrl, 1 ) ;
		SET_FLAG(act->body->m_ctrl->mt3_ctrl[ 1 ].flag, MT3_ACTIVE ) ;
#else
		AT_SetAction( act, 0, EM_cau_mukade_idle, 0, 0x7fc, ACT_INTERP_DEF ) ;
#endif

	}

	if ( act->CheckDamage( act ) ) {
		MT_ResetMotionData( act->body->m_ctrl, 1 ) ;
//		UNSET_FLAG(act->body->m_ctrl->mt3_ctrl[ 1 ].flag, MT3_ACTIVE ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		MT_ResetMotionData( act->body->m_ctrl, 1 ) ;
//		UNSET_FLAG(act->body->m_ctrl->mt3_ctrl[ 1 ].flag, MT3_ACTIVE ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActLockerOpen( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

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

/* １タイムモーション */
static	void ActPeepLR( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

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

	if ( time > COUNT_VMODE(30) ) AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_LR ) ;
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/* スタングレネード投げ */
static	void ActThrowStun( act, time )
ACTION	*act ;
int		time ;
{
	extern void *NewMagicGrd( FVECTOR *start ,FVECTOR *end ,
				float high ,int side ,int time ,short type) ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

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

	if ( time == COUNT_VMODE(208) ) {
		FVECTOR	start ;

		KR_FMatToFvec( &BODYWORLD( act->body, HUMAN21_HIDARI_TE), &start ) ;
		NewMagicGrd( &start , &act->aim_pos , 1000.0f, PLAYER_SIDE, 120, 2 ) ;
	}

	if ( time == COUNT_VMODE(216) ) {
		FVECTOR	start ;
		FVECTOR pos ;

		pos.vx  = 256.0f + act->aim_pos.vx ;
		pos.vy  = act->aim_pos.vy ;
		pos.vz  = 256.0f + act->aim_pos.vz ;
		KR_FMatToFvec( &BODYWORLD( act->body, HUMAN21_HIDARI_TE), &start ) ;
		NewMagicGrd( &start , &pos , 1000.0f, PLAYER_SIDE, 120, 2 ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/*--------------------------------------------------------------------*/
static int	AttackerAvoidCheckPad( act )
ACTION	*act ;
{
	int	mot ;
	
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_DOWNBACK :
			AT_SetModeFromPad( act, ActKokeru, EM_dam_out, act->pad ) ;
		break ;
		case SP_DISCOVERY :
			AT_SetModeFromPad( act, ActDiscovery, ENE_DiscoveryMotion(act), act->pad ) ;
		break ;
		case SP_READYGUN :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActKeepMotion, mot, act->pad ) ;
		break ;
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_MGUN_SHOT :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ActMgunShot, mot, act->pad ) ;
		break ;
		case SP_RELOAD :
			mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, mot, act->pad ) ;
		break ;
		case SP_CLE_GOSIGN :
			mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_ak_cle_signe_go:EM_cle_signe_go ;
			mot = (act->bodyp.type&ENE_TYPE_SHIELD)?EM_shl_nom_idle:mot ;
			AT_SetModeFromPad( act, ENE_ActCleSign, mot, act->pad ) ;
		break ;
		case SP_CLE_CLEAR_SIGN :
			mot = (act->bodyp.type&ENE_TYPE_SHIELD)?EM_shl_nom_idle:EM_cle_signe_clear ;
			AT_SetModeFromPad( act, ENE_ActCleSign, mot, act->pad ) ;
		break ;
		case SP_CLE_MOVE_SIGN :
			mot = (act->bodyp.type&ENE_TYPE_SHIELD)?EM_shl_nom_idle:EM_cle_signe_move ;
			AT_SetModeFromPad( act, ENE_ActCleSign, mot, act->pad ) ;
		break ;
		case SP_CLE_STOP_SIGN :
			mot = (act->bodyp.type&ENE_TYPE_SHIELD)?EM_shl_nom_idle:EM_cle_signe_stop ;
			AT_SetModeFromPad( act, ENE_ActCleSign, mot, act->pad ) ;
		break ;
		case SP_CLE_PEEP_L :
			if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_shl_nom_cle_look_l, act->pad ) ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_htc_ak_nom_cle_look_l, act->pad ) ;
			} else {
				AT_SetModeFromPad( act, ActPeepLR, EM_cle_look_l, act->pad ) ;
			}
		break ;
		case SP_CLE_PEEP_R :
			if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_shl_nom_cle_look_r, act->pad ) ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_htc_ak_nom_cle_look_r, act->pad ) ;
			} else {
				AT_SetModeFromPad( act, ActPeepLR, EM_cle_look_r, act->pad ) ;
			}
		break ;
		case SP_CLE_PEEP_D :
			if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
				AT_SetModeFromPad( act, ActPeepD, EM_shl_nom_cle_seach_r2l, act->pad ) ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActPeepD, EM_htc_ak_nom_cle_seach_r2l, act->pad ) ;
			} else {
				AT_SetModeFromPad( act, ActPeepD, EM_cle_look_d, act->pad ) ;
			}
		break ;
		case SP_PEEP_DUCT :
			AT_SetModeFromPad( act, ActPeepDuct, EM_squat_fire_intrude, act->pad ) ;
		break ;
		case SP_PEEP_TOILET_U :
			AT_SetModeFromPad( act, ActPeepToilet, EM_toilet_peep_u, act->pad ) ;
		break ;
		case SP_PEEP_TOILET_D :
			AT_SetModeFromPad( act, ActPeepToilet, EM_toilet_peep_d, act->pad ) ;
		break ;
		case SP_TOILET_KNOCK :
			AT_SetModeFromPad( act, ActToiletKnock, EM_toilet_knock, act->pad ) ;
		break ;
		case SP_CLE_AROUND :
			AT_SetModeFromPad( act, ActCleAround, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_MUKADE_WAIT :
			AT_SetModeFromPad( act, ActMukadeWait, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_LOCKER_OPEN :
			AT_SetModeFromPad( act, ActLockerOpen, EM_cle_locker_open, act->pad ) ;
		break ;
		case SP_PINPOINT :
			AT_SetModeFromPad( act, ENE_ActPinpointMove, ENE_WalkMotion(act), act->pad ) ;
		break ;
		case SP_RADIOCALL :
			AT_SetModeFromPad( act, ENE_ActSetRadio, EM_cle_call_team, act->pad ) ;
		break ;
		case SP_RADIOCALL_ATTACKER :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_ak_call_team_l, act->pad ) ;
		break ;
		case SP_RETURNRADIO :
			act->keep_mot = EM_call_team_l_3end ;
			act->keep_pad = act->pad ;
			act->time = MAX_VOL_TIME ;
			return 0 ;
		break ;

		case SP_THROW_STUN :
			AT_SetModeFromPad( act, ActThrowStun, EM_cle_gm_fire, act->pad ) ;
		break ;
		case SP_WB :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_NORMALSTAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_stand, act->pad ) ;
		break ;
	}

	return 1 ;
}

