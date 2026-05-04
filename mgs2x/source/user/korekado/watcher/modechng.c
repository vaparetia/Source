#define HOLD_AVOID_ALERT (1)

static	void	DiscoveryModeChange( ENETHINK *entk )
{
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
#if 0	//警備兵なし
		if ( entk->notice & ENE_NOTICE_NOISE_M ) {
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		}
#endif
		return ;
	}
	/* 段ボール等で利用 */
	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
		if ( entk->think1 == ENE_TH1_AVOID ) {
			THK_DiscoveryModeStartAvoid( entk ) ;
		} else {
			THK_DiscoveryModeStart( entk ) ;
		}
		return ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		if ( entk->think1 == ENE_TH1_AVOID ) {
			THK_DiscoveryModeStartAvoid( entk ) ;
		} else {
			THK_DiscoveryModeStart( entk ) ;
		}
		return ;
	}
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
}

static	void	RadioModeChange( ENETHINK *entk )
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
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}

#ifdef HOLD_AVOID_ALERT
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	InvestigatedModeChange( ENETHINK *entk )
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
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}

#ifdef HOLD_AVOID_ALERT
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	TouchModeChange( ENETHINK *entk )
{
	/* 他のモードへ */
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
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	NoiseModeChange( ENETHINK *entk )
{
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 内部で処理 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
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
//	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) {
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif


	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	FoundModeChange( ENETHINK *entk )
{
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
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	
	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	TraceModeChange( ENETHINK *entk )
{
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
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
//	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) {
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}


static	void	IndistinctModeChange( ENETHINK *entk )
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
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 無視 */
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

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
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	AccidentRepoModeChange( ENETHINK *entk )
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
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	CorpsModeChange( ENETHINK *entk )
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
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	EneDamageModeChange( ENETHINK *entk )
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

	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	BoxModeChange( ENETHINK *entk )
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 内部で処理 */
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

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

	if ( entk->alert > ENE_INDISTINCT_ALERT_LEVEL ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	}
}

static	void	DamageModeChange( ENETHINK *entk )
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
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 無視 */
	}
#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
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
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}
