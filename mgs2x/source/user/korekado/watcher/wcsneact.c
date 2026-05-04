/*
	wcsneact.c
	巡回兵、潜入モードアクション

	1999/07/29 Y.Korekado
	$Id: wcsneact.c,v 1.1.1.3 2002/11/19 11:44:31 Yoshizawa1 Exp $
	
*/

static void ActHounyouIdle( ACTION *act, int time ) ;
static void ActHounyou( ACTION *act, int time ) ;
/*-------------------------------------------------------------------*/
/*	歩く  */
void ENE_ActGoToEnd( act, time )
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
		ENE_SetActionPBreak( act, 0, ENE_WalkMotion(act), 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
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
#if 1
static SVECTOR	glass_rot = {0,0,0,0} ;
static FVECTOR	glass_sft = {0.0f,0.0f,0.0f,0.0f} ;
#else
static SVECTOR	glass_rot = {-440,-120,635} ;
static FVECTOR	glass_sft = {-80.0f,-60.0f,-80.0f} ;
#endif
static void ActGlass( act, time )
ACTION	*act ;
int		time ;
{
	extern void *NewConnectObject( OBJECT *cnct_obj, int cnct_num, FVECTOR *shift, SVECTOR *rot, int model ) ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_glasses, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->tmp_item = NULL ;
	}

	if ( act->CheckDamage( act ) ) {
		if ( act->tmp_item )	KR_DestroyActor( &act->tmp_item ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->tmp_item )	KR_DestroyActor( &act->tmp_item ) ;
		if ( act->CheckPad( act ) ) {
			return ;
		}
		SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->keep_mot == EM_glasses ) {
		if ( time == COUNT_VMODE(18) ) {
		    GV_SetActorChild( act->w, 
		    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_HIDARI_TE, &glass_sft, &glass_rot, GV_StrCode("sougan") ))) ;
		}


		if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
			ENE_SetActionPBreak( act, 0, EM_glasses_p, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
			act->keep_mot = EM_glasses_p ;
		}
	} else {
		AT_SetActStatus( act, ACT_STATUS_GLASS ) ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActOneTimeMotion( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_EYE_CLOSE ) ;
	
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

	switch ( act->pad ) {
		case SP_AKUBI :
//			if ( time > COUNT_VMODE(30) && time < COUNT_VMODE(300) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		break ;
		case SP_SENOBI :
			if ( time > 0 && time < COUNT_VMODE(408) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		break ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
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

static	void ActGuruguru( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( time>COUNT_VMODE(20) && time<COUNT_VMODE(249) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;


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

static	void ActOneTimeMotion_Spere100( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;
	ENE_ActOneTimeMotion( act, time ) ;
}
static	void ActKeepMotion_spere100( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_SPHERE_100 ) ;
	ENE_ActKeepMotion( act, time ) ;
}

static void ActHounyou( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_EYE_CLOSE|ACT_STATUS_HOUNYOU ) ;
	
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

	if ( time > 0 && time < COUNT_VMODE(408) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;


	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 3 ;	/* モーションあと2個、続き有り */
		AT_SetMode( act, ActHounyouIdle ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActHounyouIdle( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_EYE_CLOSE|ACT_STATUS_HOUNYOU ) ;
	AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_toilet_idle, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->hounyou = 1 ;	/* 放尿開始 */
	}
	if ( act->CheckDamage( act ) ) {
		act->sw->hounyou = 0 ;	/* 放尿強制終了 */
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !(act->CheckPad( act )) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
			act->sw->hounyou = 0 ;	/* 放尿強制終了 */
		} else {
			act->sw->hounyou = 4 ;	/* 放尿自然終了 */
		}
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActHounyouEnd( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_EYE_CLOSE|ACT_STATUS_HOUNYOU ) ;
	
	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !(act->CheckPad( act )) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	if ( time < COUNT_VMODE(100) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActPeepLR( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_W25_SUPER_PEEP ) ;
	}

	if ( act->CheckDamage( act ) ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
			return ;
		}
	}

	if ( time > COUNT_VMODE(30) ) AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_LR ) ;


	if ( time == COUNT_VMODE(32) ) {
		MT_SetMotionSpeed( act->body->m_ctrl, 0.0f ) ;
	}

	if ( GM_AnotherPlayer == GM_ANOTHER_PLAYER_RAIDEN ) {
		if ( time == COUNT_VMODE(272) ) {
			MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE ) ;
		}
	} else {
		if ( time == COUNT_VMODE(152) ) {
			MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE ) ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActRollingLR( act, time )
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

	if ( (time > 16) && (time < 62) ) {
//		AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP);
		AT_SetActStatus( act,ACT_STATUS_HOMING_SKIP);
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
static int	WatcherSneakCheckPad( act )
ACTION	*act ;
{
	int	p, mot ;

	if ( act->pad == SP_NONE ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_AKUBI :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_akubi, act->pad ) ;
		break ;
		case SP_SENOBI :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_senobi, act->pad ) ;
		break ;
		case SP_ZZZ :
			mot = (act->bodyp.type & ENE_TYPE_HITECH)? EM_htc_ak_nom_zzz:EM_zzz ;
			AT_SetModeFromPad( act, ENE_ActZzz, mot, act->pad ) ;
		break ;
		case SP_ZZZ_NORINORI :
			AT_SetModeFromPad( act, ENE_ActZzz, EM_norinori_sleep, act->pad ) ;
		break ;
		case SP_PORIPORI :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_poripori, act->pad ) ;
		break ;
		case SP_OTTOTTO :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_ottotto, act->pad ) ;
		break ;
		case SP_NORINORI :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_nori_nori,act->pad ) ;
		break ;
		case SP_GLASS :
			AT_SetModeFromPad( act, ActGlass, EM_glasses, act->pad ) ;
		break ;
		case SP_GOTO_END :
			/* 歩き続ける */
			ENE_SetActionPBreak( act, 0, ENE_WalkMotion(act), 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
			AT_SetMode( act, ENE_ActGoToEnd ) ;
		break ;
		case SP_NSIGHT_OFF :
			AT_SetModeFromPad( act, ENE_ActNSight_OFF, EM_gogle_off, act->pad ) ;
		break ;
		case SP_FUNAYOI :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_break_seasick, act->pad ) ;
		break ;
		case SP_AROUND :
			AT_SetModeFromPad( act, ActAround, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_PINPOINT :
			AT_SetModeFromPad( act, ENE_ActPinpointMove, ENE_WalkMotion(act), act->pad ) ;
		break ;
		/* ステージ固有モーション keep */
		case SP_STAND_01 :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_watch_pose_1, act->pad ) ;
		break ;
		case SP_STAND_02 :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_watch_pose_2, act->pad ) ;
		break ;

		/* ステージ固有モーション keep */
		case SP_ONETIME_01 :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_watch_OneTime_1, act->pad ) ;
		break ;
		case SP_ONETIME_02 :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_watch_OneTime_2, act->pad ) ;
		break ;
		case SP_ONETIME_03 :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_watch_OneTime_3, act->pad ) ;
		break ;

		case SP_IJIIJI :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_ijiiji, act->pad ) ;
		break ;
		case SP_UNREAL :
			AT_SetModeFromPad( act, ENE_ActUnreal, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_KATAGURU :
			AT_SetModeFromPad( act, ActGuruguru, EM_ak_nom_guruguru, act->pad ) ;
		break ;
		case SP_CLEAR_SIGN :
			AT_SetModeFromPad( act, ActOneTimeMotion, EM_cle_signe_clear, act->pad ) ;
		break ;
		case SP_RADIO_CALL :
			AT_SetModeFromPad( act, ENE_ActSetRadio, EM_cle_call_team, act->pad ) ;
		break ;
		case SP_RADIO_RETURN :
			act->keep_mot = EM_call_team_l_3end ;
			act->keep_pad = act->pad ;
			act->time = MAX_VOL_TIME ;
			return 0 ;
		break ;
		case SP_HOUNYOU :
			AT_SetModeFromPad( act, ActHounyou, EM_toilet_ready, act->pad ) ;
		break ;
		case SP_HOUNYOU_END :
			AT_SetModeFromPad( act, ActHounyouEnd, EM_toilet_putback, act->pad ) ;
		break ;

		case SP_PEEP_R :
			if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_shl_nom_cle_look_r, act->pad ) ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_htc_ak_nom_cle_look_r, act->pad ) ;
			} else {
				AT_SetModeFromPad( act, ActPeepLR, EM_ak_nom_cle_look_r_max, act->pad ) ;
			}
		break ;
		case SP_PEEP_L :
			if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_shl_nom_cle_look_l, act->pad ) ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActPeepLR, EM_htc_ak_nom_cle_look_l, act->pad ) ;
			} else {
				AT_SetModeFromPad( act, ActPeepLR, EM_ak_nom_cle_look_l_max, act->pad ) ;
			}
		break ;
		case SP_ROLLING_R :
			AT_SetModeFromPad( act, ActRollingLR, EM_squat_roll_r, act->pad ) ;
		break ;
		case SP_ROLLING_L :
			AT_SetModeFromPad( act, ActRollingLR, EM_squat_roll_l, act->pad ) ;
		break ;



	}

	if ( act->pad & SP_STAGE_ONETIME ) {
		p = act->pad & 0xfff ;
	printf(" Stage One Time Action [%d]\n",p);
		if ( act->pad & SP_STAGE_SPHERE100 ) {
			AT_SetModeFromPadMar( act,ActOneTimeMotion_Spere100,p, act->pad, act->name_id->mot_stage ) ;
		} else {
			AT_SetModeFromPadMar( act,ENE_ActOneTimeMotion,p, act->pad, act->name_id->mot_stage ) ;
		}
	} else if ( act->pad & SP_STAGE_LOOP ) {
		p = act->pad & 0xfff ;
	printf(" Stage Loop Action [%d]\n",p);
		if ( act->pad & SP_STAGE_SPHERE100 ) {
			AT_SetModeFromPadMar( act, ActKeepMotion_spere100, p, act->pad, act->name_id->mot_stage ) ;
		} else {
			AT_SetModeFromPadMar( act, ENE_ActKeepMotion, p, act->pad, act->name_id->mot_stage ) ;
		}
	}

	return 1 ;
}

