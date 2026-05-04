/*
	gllfight.c
	ゴルルゴン戦闘思考処理

	2002/06/24 Y.Korekado
	$Id: gllfight.c,v 1.1.1.3 2002/11/19 11:44:12 Yoshizawa1 Exp $
*/
/*-----  --------------------------------------------------------*/
#if 0
static int BookFound( Work *work )
{
	ENEFINDLIST	*efl ;
	ENEFIND *ef ;
	FVECTOR	vec, head ;
	int 	dis ;

	efl = &GM_EneFindList ;

	if( efl->start == NULL ) return 0 ;

	ef = efl->start ;
	while( ef != NULL ) {
//printf("EF ef->id[%d]  ef->type[%x]\n",ef->id, ef->type)  ;
		if( !(ef->type & (EF_TYPE_NO_FIND|EF_TYPE_FOUND)) ) {
			if ( ef->type & EF_TYPE_ADULT ) {
				_sceVu0SubVector(  &vec, &ef->pos, &GM_PlayerPosition ) ;
				dis = _FVecLen2( &vec ) ;
				if ( dis < 2000 ) {
					KR_FMatToFvec( &BODYWORLD(&work->body, HUMAN21_ATAMA), &head ) ;
#if 0
{
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = head ;
	lin[1] = ef->pos ;

	NewLineView(  &lin[0],1,0,0,255) ;
}
#endif
					if ( !HZX_OnlineHazardCheck( work->control.hzx_id, &ef->pos, &head,
								HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {
						/* 見えた！！ */
 						_sceVu0SubVector(  &vec, &ef->pos, &work->control.mov ) ;
						work->book_dir = _FVecDir2( &vec ) ;
						work->book_pos = ef->pos ;
						return 1 ;
					}
				}
			}
		}
		ef = ef->next ;
	}

	return 0 ;
}
#endif

static int GLL_FindCheck( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

#if 1
	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return -1 ;

	if ( work->find_flag & GLL_FIND_BOOK ) {
		SET_FLAG ( GllEyebeemFlag, GLL_EYEBEEM_ORDER_BOOK_STOP ) ;
		SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
		return TH3_SP_CLEAR ;
	}

	if ( work->find_flag & GLL_FIND_BUDY ) {
		SET_FLAG ( GllEyebeemFlag, GLL_EYEBEEM_ORDER_STOP ) ;
		SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
		return TH3_FIND ;
	}

	return -1 ;
#else
	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return TH3_FIRE ;

	if ( BookFound( work ) ) {
		SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
		return TH3_SP_CLEAR ;
	}

	if ( (PL_GetPlayerItem() == IT_Uniform) &&		/* ユニフォーム */
		 (PL_GetPlayerWeapon() == WP_Aks ) ) {
		SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
		return TH3_FIND ;
	}

	return TH3_FIRE ;
#endif
}

static int SonicCheck( Work *work )
{
	if ( GM_PlayerStatus & PLAYER_CAUTION ) return 0 ;
	if ( GM_PlayerStatus & PLAYER_BEHIND_ATTACK ) return 0 ;
	if ( GM_PlayerStatus & PLAYER_DOWNED ) return 0 ;
	
	return 1 ;
}
/*----- Think3 --------------------------------------------------------*/
static void Think3_GLL_FightStandWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

#if 1
	if ( work->damaged & GLL_DAM_TRGONE ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_TRACE ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->count3 > COUNT_VMODE(600) ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_TRACE ;
		work->count3 = 0 ;
		return ;
	}
#endif
	work->count3 ++ ;
}

static void Think3_GLL_FightAtkTrace( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
//		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_AT_GENORA );
		GM_SeSetMode( SD_E_GLLBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->damaged & GLL_DAM_REGENE ) {
		work->think2 = TH2_HIT ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->count3 > COUNT_VMODE( 60 ) ) {
		if ( SonicCheck( work ) ){
			work->think3 = TH3_SONIC ;
		} else {
			work->think3 = TH3_FIRE ;
		}
		work->count3 = 0 ;
		work->tmp_trgpos = GM_PlayerPosition ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_GLL_RegeneWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->damaged & GLL_DAM_REGENE ) {
		work->think2 = TH2_HIT ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->count3 > COUNT_VMODE(30) ) {
		if ( SonicCheck( work ) ){
			work->think3 = TH3_SONIC ;
		} else {
			work->think3 = TH3_FIRE ;
		}
		work->count3 = 0 ;
		work->tmp_trgpos = GM_PlayerPosition ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_GLL_FightAtkSonic( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		npc->action.pad = SP_SONIC ;
	}

	if ( npc->action.act_end == 1 ) {
		work->tmp_trgpos = GM_PlayerPosition ;
		work->think3 = TH3_COMBO ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_GLL_FightAttackCombo( Work *work )
{
	extern int	PL_KageshibariNow( void ) ;
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 > COUNT_VMODE(60) ) {
		if ( PL_KageshibariNow( ) ) {	/* プレイヤーの動きが止まってたら */
			work->tmp_trgpos = GM_PlayerPosition ;
			work->think3 = TH3_FIRE ;
			work->count3 = 0 ;
			return ;
		} else {
			work->tmp_trgpos = GM_PlayerPosition ;
			work->think3 = TH3_SEARCH ;
			work->count3 = 0 ;
			return ;
		}
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_GLL_FightAttackSearch( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->damaged & GLL_DAM_REGENE ) {
		work->think2 = TH2_HIT ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->count3 > COUNT_VMODE( 60 ) ) {
		if ( GM_PlayerStatus & (PLAYER_DAMAGED|PLAYER_DOWNED) ) {
			work->think2 = TH2_STAND ;
			work->think3 = TH3_WAIT ;
			work->count3 = 0 ;
			return ;
		}
	}

	if ( work->count3 > COUNT_VMODE( 240 ) ) {
		if ( work->sight == EYE_INFO_SIGHT_IN ) {
			work->tmp_trgpos = GM_PlayerPosition ;
			work->think3 = TH3_TRACE ;
			work->count3 = 0 ;
			return ;
		}
	}

	if ( work->count3 > COUNT_VMODE( 300 ) ) {
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->count3 < COUNT_VMODE( 180 ) ) {
		work->tmp_trgpos = GM_PlayerPosition ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_GLL_FightAtkFire( Work *work )
{
	NPCWORK	*npc ;
	int		tk3 ;

	npc = &work->npc ;


	if ( work->count3 == 0 ) {
		npc->action.pad = SP_FIRE ;
	}

	if ( npc->action.act_end == 1 ) {
		work->tmp_trgpos = GM_PlayerPosition ;
		work->think3 = TH3_SEARCH ;
		work->count3 = 0 ;
		return ;
	}

	if ((tk3 = GLL_FindCheck( work )) >= 0 ) {
		work->think3 = tk3 ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_GLL_HeartHitWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	npc->action.pad = SP_REGENE ;

	if ( !(work->damaged & GLL_DAM_REGENE) ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		work->tmp_trgpos = GM_PlayerPosition ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_GLL_FightFind( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
	npc->action.pad = SP_MYFRIEND ;

	if ( work->count3 == 0 ) {
		/*proc*/
		if ( work->gover_proc ) {
printf("gll special game over proc call!!" ) ;
			GCL_ExecProc( work->gover_proc, NULL ) ;
			work->gover_proc = 0 ;
		}
		work->find_flag = 0 ;
	}

	if ( work->count3 > COUNT_VMODE(800) ) {
		/*保険*/
		work->tmp_trgpos = GM_PlayerPosition ;
		work->think3 = TH3_SEARCH ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_GLL_FightSpClear( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
	npc->action.pad = SP_BOOK ;

	if ( work->count3 == 0 ) {
		if ( work->clear_proc ) {
printf("gll special game clear proc call!!" ) ;
			GCL_ExecProc( work->clear_proc, NULL ) ;
			work->clear_proc = 0 ;
		}
	}

//	if ( work->count3 > 60 ) {
//		work->tmp_trgpos = GM_PlayerPosition ;
//		work->think3 = TH3_SEARCH ;
//		work->count3 = 0 ;
//		return ;
//	}

	npc->action.dir = work->book_dir ;
	if ( work->count3 <COUNT_VMODE(30) ) {
//		SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
//		npc->nadj->aim_pos = work->book_pos ;
	}

	work->count3 ++ ;
}
/*----- Think2 --------------------------------------------------------*/
static void Think2_GLL_FightStnad( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_GLL_FightStandWait( work ) ;
		break ;
	}
}

static void Think2_GLL_FightAttack( Work *work )
{
	switch( work->think3 ) {
		case  TH3_TRACE :
			Think3_GLL_FightAtkTrace( work ) ;
		break ;
		case  TH3_WAIT :
			Think3_GLL_RegeneWait( work ) ;
		break ;
		case  TH3_SONIC :
			Think3_GLL_FightAtkSonic( work ) ;
		break ;
		case  TH3_COMBO :
			Think3_GLL_FightAttackCombo( work ) ;
		break ;
		case  TH3_SEARCH :
			Think3_GLL_FightAttackSearch( work ) ;
		break ;
		case  TH3_FIRE :
			Think3_GLL_FightAtkFire( work ) ;
		break ;
		case  TH3_FIND :
			Think3_GLL_FightFind( work ) ;
		break ;
		case  TH3_SP_CLEAR :
			Think3_GLL_FightSpClear( work ) ;
		break ;
	}
}

static void Think2_GLL_HeartHit( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_GLL_HeartHitWait( work ) ;
		break ;
	}
}
/*----- Think1 --------------------------------------------------------*/
static int GLLFightDamageCheck( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( work->damaged & GLL_DAM_HEART ) {
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}

	return 0 ;
}

static void Think1_Fight_GLL( Work *work )
{
	if ( GLLFightDamageCheck( work ) ) return ;
	switch( work->think2 ) {
		case  TH2_STAND :
			Think2_GLL_FightStnad( work ) ;
		break ;
		case  TH2_ATTACK :
			Think2_GLL_FightAttack( work ) ;
		break ;
		case  TH2_HIT :
			Think2_GLL_HeartHit( work ) ;
		break ;
	}
}
