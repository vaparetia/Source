/*
	gmefight.c
	メカゲノラ戦闘思考処理

	2002/06/24 Y.Korekado
	$Id: gmefight.c,v 1.1.1.3 2002/11/19 11:44:13 Yoshizawa1 Exp $

 SD_A_MG_CBIKU //チビメカゲ、びっくり音
 SD_A_MG_CCHAK //チビメカゲ、着地
 SD_A_MG_CFIRE //チビメカゲ、ホーミング加速
 SD_A_MG_CJUMP //チビメカゲ、ジャンプ
 SD_A_MG_CPARA //チビメカゲ、落下傘開く
 SD_A_MG_CSHOT //首穴からチビメカゲ発射
 SD_A_MG_INCHA //チャフ苦しみ(4,8,12,16四択ﾗﾝﾀﾞﾑ)
 SD_A_MG_INFAL //チャフ首穴に入る
 SD_A_MG_KFALL //首落ち
 SD_A_MG_KGOAL //首ゴール
 SD_A_MG_KNEJI //首外れクルクル(8fps連)
 SD_A_MG_KSURE //首床摩擦(首１回転につき１回)
 SD_A_MG_KTORE //首外れ切る～落下開始
 SD_A_MG_KWALL //首壁当たり
*/
/*----- Think3 --------------------------------------------------------*/
static void Think3_FightStandWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

#if 0
	if ( work->count3 == 0 ) {
		NewGllMissile( (FVECTOR *)&work->body.objs->objs[11].world.m[3][0], 0 ) ;
	}
#else
	if ( work->head_flag & GLL_HEAD_LIT_BREAK ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_NO_SEARCH ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->sight == EYE_INFO_SIGHT_IN ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_TRACE ;
		work->count3 = 0 ;
		return ;
	}
	if ( work->damaged & GLL_DAM_TRGONE ) {
		work->tmp_trgpos = GM_PlayerPosition ;
		work->think3 = TH3_SEARCH ;
		work->count3 = 0 ;
		return ;
	}
#endif

	work->count3 ++ ;
}

static void Think3_FightStandSearch( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->sight == EYE_INFO_SIGHT_IN ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_TRACE ;
		work->count3 = 0 ;
		return ;
	}
	if ( work->count3 == 0 ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), WHT_AT_GENORA );
		GM_SeSetMode( SD_E_MGNBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}
	if ( work->count3 == COUNT_VMODE( 300 ) ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), WHT_QE_GENORA );
		GM_SeSetMode( SD_E_MGNBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}
	if ( work->count3 > COUNT_VMODE( 420 ) ) {
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_FightAtkTrace( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_AT_GENORA );
		GM_SeSetMode( SD_E_BIKKRI01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count3 > COUNT_VMODE( 60 ) ) {
		work->think3 = TH3_FIRE ;
		work->count3 = 0 ;
		work->tmp_trgpos = GM_PlayerPosition ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_FightAtkFire( Work *work )
{
	NPCWORK	*npc ;

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

	SET_FLAG ( work->status2, GLL_STATUS2_SL_TRG ) ;
	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_FightAttackSearch( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->sight == EYE_INFO_SIGHT_IN ) {
		work->tmp_trgpos = GM_PlayerPosition ;
		work->think3 = TH3_FIRE ;
		work->count3 = 0 ;
		return ;
	}
	if ( work->count3 == COUNT_VMODE( 300 ) ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), WHT_QE_GENORA );
		GM_SeSetMode( SD_E_MGNBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}
	if ( work->count3 > COUNT_VMODE( 420 ) ) {
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}
	if ( work->count3 < COUNT_VMODE( 300 ) ) {
		work->tmp_trgpos = GM_PlayerPosition ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

#define CHAFF_BEEM_TRG_MAX	(7)
static FVECTOR	ChaffBeemTrg[CHAFF_BEEM_TRG_MAX] = {
	{-7000.0, 0.0, -1000.0},
	{0.0, 0.0, -2000.0},
	{7000.0, 0.0, -1000.0},
	{-3000.0, 0.0, -6000.0},
	{3000.0, 0.0, -7000.0},
	{-6000.0, 0.0, -10000.0},
	{4000.0, 0.0, -11000.0},
} ;
static void Think3_FightAttackChaff( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;


	npc->action.pad = SP_CHAFFDAM ;
	if ( work->count3 == 0 ) {
		work->tmp_trgpos = ChaffBeemTrg[ KR_RandU(7) ] ;
	}

	if ( !(GM_GameStatus & STATE_CHAFF) ) {
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;

		return ;
	}

	if ( work->count3 >= COUNT_VMODE(120) ) {
		work->think3 = TH3_CHAFFBEEM ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_FightAttackChaffBeem( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		npc->action.pad = SP_CHAFFBEEM ;
	}

	if ( npc->action.act_end == 1 ) {
		npc->action.pad = SP_CHAFFDAM ;
		work->think3 = TH3_CHAFF ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_FightAttackNoSearch( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		if ( !(work->head_flag & GLL_HEAD_NO_LIT) ) {
			work->head_flag |= GLL_HEAD_NO_LIT ;
			AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), WHT_QE_GENORA );
		}
		work->tmp_trgpos = ChaffBeemTrg[ KR_RandU(7) ] ;
	}

	if ( work->count3 >= COUNT_VMODE(300) ) {
		work->think3 = TH3_NO_SEARCH_BEEM ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_FightAttackNoSearchBeem( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		npc->action.pad = SP_FIRE_F ;
	}

	if ( npc->action.act_end == 1 ) {
		work->think3 = TH3_NO_SEARCH ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = work->tmp_trgpos ;

	work->count3 ++ ;
}

static void Think3_GmeHeadReady( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		work->head_flag |= GLL_HEAD_OFF ;
		GM_SeSetMode( SD_E_MGNBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		work->tmp_count = 0 ;
	}

	if ( GLL_GAME_STATUS & GLL_GS_HEAD_REMOVE ) {
		if ( work->tmp_count == 0 ){
			work->tmp_count = 1 ;
			AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_QE_GENORA );
		}
	}

	work->count3 ++ ;
}

static void Think3_GmeHeadWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 > COUNT_VMODE(60) ) {
		work->think3 = TH3_BDMH ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static int rr=0 ;
static void Think3_GmeBikkuriDokkiriMecaHassin( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		npc->action.pad = SP_BDMH ;
		GM_SeSetMode( SD_E_MGNHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		work->tmp_count = 0 ;
		GllMissileInitID( ) ;
	}

	if ( npc->action.current_mot == GLL_MOT_BDMH_LOOP ) {
		if ( work->game_count == 0 ) {
			if ( !(work->tmp_count%(COUNT_VMODE(30))) ) {
				NewGllMissile( (FVECTOR *)&work->body.objs->objs[11].world.m[3][0], rr ) ;
				if ( ++rr >= 4 ) rr = 0 ;
			}
		} else {
			if ( !(work->tmp_count%(COUNT_VMODE(15))) ) {
				NewGllMissile( (FVECTOR *)&work->body.objs->objs[11].world.m[3][0], rr ) ;
				if ( ++rr >= 4 ) rr = 0 ;
			}
		}
		work->tmp_count ++ ;
	}

	if ( npc->action.act_end == 1 ) {
		work->game_count ++ ;
		work->think3 = TH3_REGENE ;
		work->count3 = 0 ;
		return ;
	}

//	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_GME_BDMH ) ;

	work->count3 ++ ;
}

static void Think3_GmeHeadRegene( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;
	if ( work->count3 == 0 ) {
		work->head_flag |= GLL_HEAD_NEXT ;
		UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_SPOT_BREAK ) ;
	}

	if ( work->head_flag & GLL_HEAD_REGENE ) {
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

/*----- Think2 --------------------------------------------------------*/
static void Think2_FightStnad( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_FightStandWait( work ) ;
		break ;
		case  TH3_SEARCH :
			Think3_FightStandSearch( work ) ;
		break ;
	}
	SET_FLAG ( work->status2, GLL_STATUS2_SEARCH ) ;
}

static void Think2_FightAttack( Work *work )
{
	switch( work->think3 ) {
		case  TH3_TRACE :
			Think3_FightAtkTrace( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_ALERT ) ;
		break ;
		case  TH3_FIRE :
			Think3_FightAtkFire( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_ALERT ) ;
		break ;
		case  TH3_SEARCH :
			Think3_FightAttackSearch( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_AVOID ) ;
		break ;
		case  TH3_CHAFF :
			Think3_FightAttackChaff( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_INVALID_CHAFF ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_CHAFF_CRAZY ) ;
		break ;
		case  TH3_CHAFFBEEM :
			Think3_FightAttackChaffBeem( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_INVALID_CHAFF ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_CHAFF_CRAZY ) ;
		break ;
		case  TH3_NO_SEARCH :
			Think3_FightAttackNoSearch( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_ALERT ) ;
		break ;
		case  TH3_NO_SEARCH_BEEM :
			Think3_FightAttackNoSearchBeem( work ) ;
			SET_FLAG ( work->status2, GLL_STATUS2_ALERT ) ;
		break ;
	}
}

static void Think2_GmeHead( Work *work )
{
	switch( work->think3 ) {
		case  TH3_READY :
			Think3_GmeHeadReady( work ) ;
		break ;

		case  TH3_WAIT :
			Think3_GmeHeadWait( work ) ;
		break ;
		case  TH3_BDMH :
			Think3_GmeBikkuriDokkiriMecaHassin( work ) ;
		break ;
		case  TH3_REGENE :
			Think3_GmeHeadRegene( work ) ;
		break ;
	}
	if ( !(work->head_flag & GLL_HEAD_REGENE) ) {
		SET_FLAG ( work->status2, GLL_STATUS2_NO_HEAD ) ;
		SET_FLAG ( work->status2, GLL_STATUS2_INVALID_CHAFF ) ;
	}
}
/*----- Think1 --------------------------------------------------------*/
static int GME_FightDamageCheck( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	if ( work->damaged & GLL_DAM_HIT_HEAD ) {
		work->think2 = TH2_HEAD ;
		work->think3 = TH3_READY ;
		work->count3 = 0 ;
		return 1 ;
	}

	if ( work->head_flag & GLL_HEAD_BREAK ) {
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}

	if ( work->damaged & GLL_DAM_CHAFF ) {
		if ( !(work->status2 & GLL_STATUS2_INVALID_CHAFF) ) {
			work->think2 = TH2_ATTACK ;
			work->think3 = TH3_CHAFF ;
			work->count3 = 0 ;
			return 1 ;

		}
	}

	return 0 ;
}

static void FightNoticeCheck( Work *work )
{
	if ( work->sight == EYE_INFO_SIGHT_IN ) {
//		work->think1 = TH1_DETECT ;
//		work->think2 = TH2_STAND ;
//		work->think3 = TH3_POINT_ACTION ;
//		work->count3 = 0 ;
	}
}

static void Think1_Fight_GME( Work *work )
{
	switch( work->think2 ) {
		case  TH2_STAND :
			Think2_FightStnad( work ) ;
		break ;
		case  TH2_ATTACK :
			Think2_FightAttack( work ) ;
		break ;
		case  TH2_HEAD :
			Think2_GmeHead( work ) ;
		break ;
	}

	if ( GME_FightDamageCheck( work ) ) return ;
	FightNoticeCheck( work ) ;
}
