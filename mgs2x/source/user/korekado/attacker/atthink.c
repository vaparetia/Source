/*
	atthink.c
	攻撃兵、思考ルーチン

	1997/07/07 Y.Korekado
	$Id: atthink.c,v 1.1.1.3 2002/11/19 11:43:59 Yoshizawa1 Exp $
	
*/
#include "atalert.c"

/*----- 思考ルーチン --------------------------------------------------*/
/*----- 低レベル思考モード --------------------------------------------*/
/*----- 高レベル思考モード --------------------------------------------*/
static	void	SneakModeCheack( entk )
ENETHINK	*entk ;
{
	if(	GM_AlertMode == ALERT_MODE_ALERT ){
		entk->act->CheckPad = CheckPad ;

		entk->think1 = TH1_ALERT ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		entk->act->move_s = MoveRun ;
		ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerPosition, &GM_PlayerAddress, NULL, &GM_PlayerHzxID ) ;
	}
}

static	void	AlertModeCheack( entk )
ENETHINK	*entk ;
{
	if(	GM_AlertMode == ALERT_MODE_AVOID ){
		entk->think1 = TH1_AVOID ;
		/* 注！！CheckPad関数を入れ替える */
		ENE_AttackerStartModeAvoid( entk ) ;
	}
}

static	void	AvoidModeCheack( entk )
ENETHINK	*entk ;
{
	if(	GM_AlertMode == ALERT_MODE_ALERT ){
		/* 注！！CheckPad関数を戻す */
		entk->act->CheckPad = CheckPad ;

		entk->think1 = TH1_ALERT ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		entk->act->move_s = MoveRun ;
		ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerPosition, &GM_PlayerAddress, NULL, &GM_PlayerHzxID ) ;
	}

	if ( GM_AlertMode == ALERT_MODE_SNEAK || GM_AlertMode == ALERT_MODE_SEARCH ){
		ENE_AttackerStartModeSneak( entk ) ;
		return ;
	}

}

/*----------------------------------------------------------------------------*/
	/*
		危険
	*/
static	void	Think1_Alert( entk )
ENETHINK		*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_CHASE :
			Think2_Chase( entk ) ;
		break ;
	    case TH2_MOVE :
			Think2_MoveDefensePoint( entk ) ;
		break ;
	    case TH2_ATTACK :
			Think2_Attack( entk ) ;
		break ;
	    case TH2_SEARCH :	/* 指定場所で警戒する */
			Think2_AlertSearch( entk ) ;
		break ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = MAX_ALERT_LEVEL ;
	}
}

/*----- 思考処理 --------------------------------------------------------*/
static void InitThinkParam( work )
Work	*work ;
{
	work->action.dir = -1 ;
	work->action.pad = 0 ;
}

static void Think( work )
Work	*work ;
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	InitThinkParam( work ) ;

	switch( entk->think1 ){
		case TH1_NORMAL :
			ENE_Attacker_Think1_Normal( entk ) ;
			SneakModeCheack( entk ) ;
		break ;
		case TH1_ALERT :
			Think1_Alert( entk ) ;
			AlertModeCheack( entk ) ;
		break ;
		case TH1_AVOID :
			ENE_Attacker_Think1_Avoid( entk ) ;
			AvoidModeCheack( entk ) ;
		break ;
	}

}
