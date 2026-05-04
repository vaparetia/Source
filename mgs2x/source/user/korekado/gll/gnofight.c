/*
	gnofight.c
	ゲノラ戦闘思考処理

	2002/07/04 Y.Korekado
	$Id: gnofight.c,v 1.1.1.3 2002/11/19 11:44:13 Yoshizawa1 Exp $
*/
/*----- Think3 --------------------------------------------------------*/
static void Think3_GNO_FightStandWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	if ( GLL_ACHOO_TIME > 0 ) {
		work->think2 = TH2_ACHOO ;
		work->think3 = TH3_READY ;
		work->count3 = 0 ;
		return ;
	}

	if ( GLL_GAME_STATUS & GLL_GS_TUB_DORP ) {
		if ( work->clear_proc ) {
printf("gno special game clear proc call!!" ) ;
			GCL_ExecProc( work->clear_proc, NULL ) ;
			work->clear_proc = 0 ;
		}
		work->think2 = TH2_ACHOO ;
		work->think3 = TH3_READY ;
		work->count3 = 0 ;
		return ;
	}

	if ( !(work->tmp1 % 3) ) {
		if ( work->count3 > COUNT_VMODE(300) ) {
			work->think2 = TH2_ATTACK ;
			work->think3 = TH3_PUNCH ;
			work->count3 = 0 ;
			return ;
		}
	} else {
//		if ( work->count3 > 180 ) {
		if ( work->count3 > 2 ) {
			work->think2 = TH2_ATTACK ;
			work->think3 = TH3_PUNCH ;
			work->count3 = 0 ;
			return ;
		}
	}

	work->count3 ++ ;
}

static void Think3_GNO_FightAtkPunch( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	if ( work->count3 == 0 ) {
		int	n ;

		n = work->tmp1 % 3 ;
		if ( n == 0 ) {
			if ( GM_PlayerPosition.vx > npc->ctrl->mov.vx  ) {
				npc->action.pad = SP_PUNCH_L ;
				work->tmp2 = 1 ;
			} else {
				npc->action.pad = SP_PUNCH_R ;
				work->tmp2 = 0 ;
			}
		} else {
#if 1
			if ( KR_RandU(2)%2 ) {
				npc->action.pad = SP_PUNCH_R ;
				work->tmp2 = 0 ;
			} else {
				npc->action.pad = SP_PUNCH_L ;
				work->tmp2 = 1 ;
			}
#else
			if ( work->tmp2 ) {
				npc->action.pad = SP_PUNCH_R ;
				work->tmp2 = 0 ;
			} else {
				npc->action.pad = SP_PUNCH_L ;
				work->tmp2 = 1 ;
			}
#endif
		}
		work->tmp1 ++ ;
	}

	if ( GLL_GAME_STATUS & GLL_GS_TUB_DORP ) {
		if ( work->clear_proc ) {
printf("gno special game clear proc call!!" ) ;
			GCL_ExecProc( work->clear_proc, NULL ) ;
			work->clear_proc = 0 ;
		}
		work->think2 = TH2_ACHOO ;
		work->think3 = TH3_READY ;
		work->count3 = 0 ;
		return ;
	}

#if 0
	if ( GLL_ACHOO_TIME > 0 ) {
		work->think2 = TH2_ACHOO ;
		work->think3 = TH3_READY ;
		work->count3 = 0 ;
		return ;
	}
#endif

	if ( npc->action.act_end == 1 ) {
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_GNO_FightAchooReady( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	npc->action.pad = SP_ACHOO ;

	if ( GLL_GAME_STATUS & GLL_GS_TUB_DORP ) {
		if ( work->clear_proc ) {
printf("gno special game clear proc call!!" ) ;
			GCL_ExecProc( work->clear_proc, NULL ) ;
			work->clear_proc = 0 ;
		}
	}

	if ( npc->action.act_end == 1 ) {
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

/*----- Think2 --------------------------------------------------------*/
static void Think2_GNO_FightStnad( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_GNO_FightStandWait( work ) ;
		break ;
	}
}

static void Think2_GNO_FightAttack( Work *work )
{
	switch( work->think3 ) {
		case  TH3_PUNCH :
			Think3_GNO_FightAtkPunch( work ) ;
		break ;
	}
}

static void Think2_GNO_FightAchoo( Work *work )
{
	switch( work->think3 ) {
		case  TH3_READY :
			Think3_GNO_FightAchooReady( work ) ;
		break ;
	}
}

/*----- Think1 --------------------------------------------------------*/
static void Think1_Fight_GNO( Work *work )
{
	switch( work->think2 ) {
		case  TH2_STAND :
			Think2_GNO_FightStnad( work ) ;
		break ;
		case  TH2_ATTACK :
			Think2_GNO_FightAttack( work ) ;
		break ;
		case  TH2_ACHOO :
			Think2_GNO_FightAchoo( work ) ;
		break ;
	}

//	if ( FightDamageCheck( work ) ) return ;
//	FightNoticeCheck( work ) ;
}
