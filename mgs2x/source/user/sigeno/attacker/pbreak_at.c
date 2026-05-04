/*
	pbreak_at.c
	負傷中行動
	2001/07/06 K.Sigeno
	$Id: pbreak_at.c,v 1.1.1.3 2002/11/19 11:49:04 Yoshizawa1 Exp $
*/
/*----- 中レベル思考モード --------------------------------------------*/

/*治療開始条件チェック*/
static int CheckMedication ( entk )
ENETHINK *entk ;
{
//	||(entk->act->bodyp.type & ENE_TYPE_SHIELD)
//	||(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
	/**両腕 両足は別処理に回っているものとして除外*/
	/*目標視界外なので治療*/
	if( ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) 
	||(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
	){
		return 1 ;
	}
	/*右手 攻撃不能なので即治療*/
	if ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) {
		return 1 ;
	}
	/*左手 弾切れなら治療*/
	/*ショットガンは片手撃ち不能なので即治療*/
	if( entk->act->bodyp.pbreak & PBREAK_ARM_L ) {
		if(
		( entk->bullet >= entk->max_bullet )
		||(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
		){
			return 1 ;
		}
	}
	/* 片足 */
#if 0
	if ( entk->act->bodyp.pbreak & PBREAK_LEGS ) {
	}
#endif
	return 0 ;
}

static void Think3_PB_Attack( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN_PB ;
	entk->act->dir = entk->pl_eyei.dir ;
	SetAbsAimPosPlayer(entk);
	if((entk->count3 > 3000)
	||(CheckMedication(entk))
	){
		GoMedication(entk) ;
		return ;
	}
printf("NOW BULLET [%d]\n",entk->bullet) ;
printf("NOW MAX BULLET [%d]\n",entk->max_bullet) ;
	if ( entk->bullet >= entk->max_bullet ) {
		/*Goreload*/
printf("PB_Attack GO 2 RELOAD!!!!\n");
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
		entk->think2 = TH2_PBREAK ; 
		entk->think3 = TH3_ATTACK_RELOAD ;
		entk->count3 = 0 ;
		return ;
	}
	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ){
printf("RandShoot!\n");
		RandShoot( entk );
	}
#if 0
	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		SetPBreakThink( entk ) ;
		return ;
	}

	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
		!(entk->act->bodyp.pbreak & PBREAK_LEGS) && /* 両足が無事 */
		!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
		entk->think3 = TH3_ATTACK_NEAR ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(20) ) {
		if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
			entk->think3 = TH3_ATTACK_MGUN ;
			entk->count3 = 0 ;
			return ;
		}
		SetPBreakThink( entk ) ;
	}

#endif
	entk->count3 ++ ;
}
static	void	Think3_Medical( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_MEDICATION ;
	}
	if ( entk->act->act_end ) {
		GoNearAttack(entk);
		return ;
	}
	entk->act->dir = entk->ctrl->turn.vy ;
	entk->count3 ++ ;
}


static	void Think3_RelaodPB( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	entk->act->dir = entk->ctrl->turn.vy ;

	at_thk->at_status &= ~AT_ST_SQUAT ;
	entk->act->pad = SP_RELOAD ;
	if ( entk->act->act_end ) {
		GoPbreak(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}

static	void	Think2_PBreak( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_ATTACK_NEAR :
			Think3_PB_Attack( entk ) ;
			break ;
		case TH3_MEDICATION :
			Think3_Medical( entk ) ;
			break ;
		case TH3_ATTACK_RELOAD :
			Think3_RelaodPB( entk ) ;
			break ;
#if 0
	    case TH3_WATCH :
	    	Think3_PBAttackWatch( entk ) ;
		break ;
	    case TH3_ATTACK_SETUP :
	    	Think3_PB_ReadyGun( entk ) ;
		break ;
	    case TH3_ATTACK_MGUN :
	    	Think3_AttackMgun( entk ) ;
		break ;
	    case TH3_ATTACK_RELOAD :
	    	Think3_AttackReload( entk ) ;
		break ;
	    case TH3_ATTACK_NEAR :
	    	Think3_AttackNear( entk ) ;
		break ;
	    case TH3_MEDICAL :
			Think3_Medical( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA :
			Think3_MoveWaitArea( entk ) ;
		break ;
	    case TH3_MORTALLY :
		    Think3_Mortally( entk ) ;
		break ;
#endif
	}
}

