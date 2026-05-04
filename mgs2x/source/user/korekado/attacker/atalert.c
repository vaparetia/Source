/*----- 低レベル思考モード --------------------------------------------*/
static void Think3_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > 20 ) {
		entk->think3 = TH3_ATTACK_MGUN ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_AttackWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}
	entk->act->pad = SP_CAUT_STAND ;
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ZoneChaseEnemy( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &(entk->pl_eyei), &(entk->trgpoint) ) ;
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseEnemy( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &(entk->pl_eyei), &(entk->trgpoint) ) ;
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->ctrl->mov ), -1 ) ;
	}

	/* いつまでも直線じゃいられない */
	if ( ENE_DirectReachCheck( entk ) == 0 ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	
	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}


static void Think3_ZoneMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static	void	Think3_AttackMgun( entk )
ENETHINK	*entk ;
{
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
		entk->bullet = 0 ;
		entk->think3 = TH3_ATTACK_RELOAD ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( !(entk->count3 % 4) ) {
		if ( !(BP_PS2_rand()%3) ) {
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			return ;
		}
		entk->act->pad = SP_MGUN_SHOT ;
		entk->bullet ++ ;
	} else {
		entk->act->pad = SP_READYGUN ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_AttackReload( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) entk->act->pad = SP_RELOAD ;

	if ( entk->act->act_end ) {
		if ( BP_PS2_rand()%2 ) {
#if 0
			ENETHINK	*trgene ;
			int			rd ;

			rd = BP_PS2_rand()%Comm.enemy_num[UNIT_WATCHER] ;
			trgene = Comm.entk[UNIT_WATCHER][ rd ] ;
			/* 目標の敵をセット */
			ENE_SetEyeInfo( &(entk->pl_eyei), &(trgene->ctrl->mov), &(trgene->ctrl->addr), NULL ) ;
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
#else
			entk->think3 = TH3_ATTACK_SETUP ; 
#endif
		} else {
			entk->think3 = TH3_ATTACK_SETUP ; 
		}
		entk->count3 = 0 ;
//printf("id[%d] [%f] [%f] \n",entk->id, entk->ctrl->mov.vx, entk->ctrl->mov.vz ) ;
 		return ;
	}

	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Chase( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneChaseEnemy( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChaseEnemy( entk ) ;
		break ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}
}

static	void	Think2_MoveDefensePoint( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :
	    	Think3_ZoneMove( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :
	    	Think3_DirectMove( entk ) ;
		break ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}
}

static	void	Think2_Attack( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ATTACK_SETUP :
	    	Think3_ReadyGun( entk ) ;
		break ;
	    case TH3_ATTACK_MGUN :
	    	Think3_AttackMgun( entk ) ;
		break ;
	    case TH3_ATTACK_RELOAD :
	    	Think3_AttackReload( entk ) ;
		break ;
	}
}
static	void	Think2_AlertSearch( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
	    	Think3_AttackWatch( entk ) ;
		break ;
	}
}
