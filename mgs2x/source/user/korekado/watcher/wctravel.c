/*----- 低レベル思考モード --------------------------------------------*/
static void Think3_ReturnTravel( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
//		entk->rnavi->next_node -- ;
		ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
		entk->act->move_s = MoveWalk ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_GoNext( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->act->dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
		entk->rnavi->p_acttime = entk->rnavi->pa_time[(int)entk->rnavi->next_node] ;
		entk->think3 = TH3_POINT_ACTION ; 
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_PointAction( entk )
ENETHINK	*entk ;
{
	if ( entk->rnavi->p_acttime <= 0) {
		ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
		entk->think3 = TH3_GO_NEXT ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->rnavi->p_acttime -- ;
	entk->count3 ++ ;
}

static void Think3_DiscoveryPose( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_DISCOVERY ;
	}

	if ( entk->act->act_end ) {
#if 1
		entk->think3 = TH3_RADIO_CALL ; 
		entk->count3 = 0 ;
#else
		entk->alert = MAX_ALERT_LEVEL ;
		entk->think1 = TH1_ALERT ; 
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		entk->act->move_s = MoveRun ;
#endif

		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_RadioCall( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL ;
	}

	if ( entk->act->act_end ) {
		entk->alert = MAX_ALERT_LEVEL ;
		entk->think1 = TH1_ALERT ; 
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		entk->act->move_s = MoveRun ;

		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_NoiseWatch( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		FVECTOR	vec ;
		int		dir ;

		sceVu0SubVector(  &vec, &(entk->trgpoint.pos), &(entk->ctrl->mov) ) ;
		dir = _FVecDir2( &vec ) ;
		entk->act->dir = dir ;
		/* 顔の上下 */
	}
	
	if ( entk->count3 == 6 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->count3 == 120 ) {
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_NoiseMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 6 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->count3 == 120 ) {
		SetThinkTravel( entk ) ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	TravelModeCheack( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		entk->think3 = TH3_DISCOVERY_POSE ; 
		entk->count3 = 0 ;
	}
	
	if ( entk->notice & ENE_NOTICE_NOISE ) {
#ifdef GMDEF_NOISE_MAP_RENEW
		ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
		ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_GetHzxGroupID(GM_NoiseMap) ) ;
#endif
		entk->think2 = TH2_NOISE ;
		entk->think3 = TH3_WATCH ;
		entk->count3 = 0 ;
	}
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		entk->think3 = TH3_DISCOVERY_POSE ; 
		entk->count3 = 0 ;
	}
#if 0
	if ( entk->notice & ENE_NOTICE_NOISE ) {
#ifdef GMDEF_NOISE_MAP_RENEW
		ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
		ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_GetHzxGroupID(GM_NoiseMap) ) ;
#endif
		entk->think2 = TH2_NOISE ;
		entk->think3 = TH3_WATCH ;
		entk->count3 = 0 ;
	}
#endif
}

static	void	Think2_Travel( entk )
ENETHINK	*entk ;
{

	switch ( entk->think3 ) {
	    case TH3_RETURN_TRAVEL :		    /* 指定位置に移動 */
	    	Think3_ReturnTravel( entk ) ;
		break ;
	    case TH3_GO_NEXT :					/* 次の巡回ポイントへ移動 */
			Think3_GoNext( entk ) ;
		break ;
	    case TH3_POINT_ACTION :				/* ポイントアクション */
	    	Think3_PointAction( entk ) ;
		break;
	}

	TravelModeCheack( entk ) ;
}


/*
	物音モード
	*/
static	void	Think2_Noise( entk )
ENETHINK	*entk ;
{

	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_NoiseWatch( entk ) ;
		break ;
	    case TH3_ZONETRACE :
			Think3_NoiseMove( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;
	}

	NoiseModeCheck( entk ) ;
}

static	void	Think2_Discovery( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_DISCOVERY_POSE :
	    	Think3_DiscoveryPose( entk ) ;
		break ;
	    case TH3_RADIO_CALL :
	    	Think3_RadioCall( entk ) ;
		break ;
	}
}


