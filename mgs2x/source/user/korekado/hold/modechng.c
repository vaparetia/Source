static	void	DiscoveryModeChange( ENETHINK *entk )
{
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		DiscoveryStart( entk ) ;
		return ;
	}
	/* 段ボール等で利用 */
	if ( entk->alert >= DEF_EYE_DELAY ) {
		DiscoveryStart( entk ) ;
		return ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		DiscoveryStart( entk ) ;
		return ;
	}
}

static	void	TravelModeChange( ENETHINK *entk )
{
	/* 優先度の低いものから判定していく */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
	if ( entk->alert > 0 || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}

static	void	TouchModeChange( ENETHINK *entk )
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
		entk->think2 = TH2_FOUND ;
		THK_FoundModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

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
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		entk->think2 = TH2_FOUND ;
		THK_FoundModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

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
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
	
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
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			ENE_TraceEnd( entk ) ;
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

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

	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

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
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 無視 */
	}

	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

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
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 無視 */
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 内部で処理 */
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
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
		DiscoveryStart( entk ) ;
	}
}

static	void	LookDownModeChange( ENETHINK *entk )
{
	/* 優先度の低いものから判定していく */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 下除きこみ中はなし！！*/
//		entk->think2 = TH2_NOISE ;
//		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 下除きこみ中はなし！！*/
//		entk->think2 = TH2_TRACE ;
//		THK_TraceModeStart( entk ) ;
	}
	if ( entk->alert > 0 || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		/* 下除きこみ中はなし！！*/
//		entk->think2 = TH2_INDISTINCT ; 
//		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( !(entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD) ) {
			entk->think2 = TH2_ENE_DAMAGE ; 
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		/* 下除きこみ中はなし！！*/
//		entk->think2 = TH2_BOX ;
//		THK_BoxModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		/* 下除きこみ中はなし！！*/
//		entk->think2 = TH2_HOLDUP ;
//		THK_HoldUpModeStart( entk ) ;
	}

	/* 発見 */
	DiscoveryModeChange( entk ) ;
}
