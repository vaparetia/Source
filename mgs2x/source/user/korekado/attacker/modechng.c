/*
	modechng.c
	アタッカーモードチェンジ
	
	1999/07/26 Y.Korekado
	$Id: modechng.c,v 1.1.1.3 2002/11/19 11:44:01 Yoshizawa1 Exp $
*/

static	void	DiscoveryModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
		if ( entk->notice & ENE_NOTICE_NOISE_M ) {
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		}
	}
	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		return ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		return ;
	}

	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
}

static	void	TravelModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		if ( entk->think1 != ENE_TH1_AVOID ) {
			entk->think2 = TH2_ACCIDENT ;
			THK_AccidentModeStart( entk ) ;
		}
	}

	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			entk->think2 = TH2_FOUND ;
			THK_FoundModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	AccidentModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 確認しに来た敵兵が死んでいたら */
		if( entk->corp_id == entk->accident_id ) {
//			COM_UnsetAccident( entk->accident_id ) ;
			COM_FreeReservAccident( entk->accident_id ) ;
		}
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 確認しに来た敵兵がダメージを受けていたら */
		if( entk->dam_entk->uniq_id == entk->accident_id ) {
//			COM_UnsetAccident( entk->accident_id ) ;
			COM_FreeReservAccident( entk->accident_id ) ;
		}
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}


	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	TouchModeChange( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	if ( entk->notice & ENE_NOTICE_TOUCH ) {
		/* 内部でチェック */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			entk->think2 = TH2_FOUND ;
			THK_FoundModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
//	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) {
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}


	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	NoiseModeChange( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 内部で処理 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		entk->think2 = TH2_FOUND ;
		THK_FoundModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
//	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) {
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	FoundModeChange( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
//	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) {
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	TraceModeChange( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			/* 無視 */
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	IndistinctModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_BLURR ) {
		/* 朧げに見えていない時のみ物音チェック */
		if ( entk->notice & ENE_NOTICE_NOISE ) {
			entk->think2 = TH2_NOISE ;
			THK_NoiseModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			/* 無視 */
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		/* 無視 */
	}

	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}
	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	UniformModeChange( ENETHINK *entk )
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		/* 無視 */
	}

	if ( entk->notice & ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	CorpsModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			/* 無視 */
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	EneDamageModeChange( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			/* 無視 */
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 無視 */
	}
	if ( entk->notice & (ENE_NOTICE_BOX) ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	BoxModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			/* 無視 */
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 無視 */
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 内部で処理 */
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	HoldUpModeChange( ENETHINK *entk )
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		/* 無視 */
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 無視 */
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	DamageModeChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		if ( entk->think1 == ENE_TH1_AVOID ) {
			/* 発見するだけ */
			THK_FoundModeFastStart( entk ) ;
		} else {
			/* 無視 */
		}
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 無視 */
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 無視 */
	}

	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

