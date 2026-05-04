/*
	spthink.c
	見張り兵、思考ルーチン

	1999/08/30 Y.Korekado
	$Id: spthink.c,v 1.1.1.3 2002/11/19 11:44:25 Yoshizawa1 Exp $
	
*/

#define SPP_ALERT_WATCHER	(1)
/*----- 高レベルモード移行チェック --------------------------------------------------*/
static	void	SneakModeCheack( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_SupportStartModeAlert( entk ) ;
		break ;
		case ALERT_MODE_AVOID :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_SupportStartModeAvoid( entk ) ;
		break ;
		case ALERT_MODE_SEARCH :
			ENE_SupportStartModeCaution( entk ) ;
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}
static	void	AlertModeCheack( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			ENE_WatcherStartModeSneak( entk ) ;
		break ;
		case ALERT_MODE_AVOID :
			ENE_SupportStartModeAvoid( entk ) ;
		break ;
		case ALERT_MODE_SEARCH :
			ENE_SupportStartModeCaution( entk ) ;
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
	}
}
static	void	AvoidModeCheack( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_WatcherStartModeSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_SupportStartModeAlert( entk ) ;
		break ;
		case ALERT_MODE_SEARCH :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_SupportStartModeCaution( entk ) ;
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}

static	void	SearchModeCheack( entk )
ENETHINK	*entk ;
{
#if 1
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			ENE_WatcherStartModeSneak( entk ) ;
		break ;
		case ALERT_MODE_ALERT :
			ENE_SupportStartModeAlert( entk ) ;
			entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
		break ;
		case ALERT_MODE_AVOID :
			ENE_SupportStartModeAvoid( entk ) ;
			entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
		break ;
	}
#else
	/* 潜入モードへはそれぞれのタイミングで戻る */
	if ( entk->count1 > 60*180 ) {
		ENE_WatcherStartModeSneak( entk ) ;
		entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
	}
	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			ENE_SupportStartModeAlert( entk ) ;
			entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
		break ;
		case ALERT_MODE_AVOID :
			ENE_SupportStartModeAvoid( entk ) ;
			entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
		break ;
	}
#endif
	if ( ENE_DamageCheck( entk ) ) {
		ENE_EnemyStartModeDamage( entk ) ;
		entk->bd_eyei.flag |= EYE_INFO_FLAG_SKIP ;
	}
}

static	void	DamageModeCheack( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
				ENE_SupportStartModeAlert( entk ) ;
			break ;
			case ALERT_MODE_SEARCH :
				ENE_SupportStartModeCaution( entk ) ;
				ENE_SupportStartModeCautionDamage( entk ) ;
			break ;
			default :
				ENE_SupportStartModeAvoidDamage( entk ) ;
			break ;
		}


		if ( ENE_DamageCheck( entk ) ) {
			ENE_EnemyStartModeDamage( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_RES ) {
		ENE_WatcherResurrectionMode( entk ) ;
	}
}

static	void	RessModeCheck( entk )
ENETHINK	*entk ;
{
	if ( !(entk->notice & ENE_NOTICE_RES) ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
				ENE_SupportStartModeAlert( entk ) ;
//				ENE_WatcherStartModeAlert( entk ) ;
			break ;
			case ALERT_MODE_SEARCH :
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			default :
				ENE_SupportStartModeAvoid( entk ) ;
			break ;
		}
	}
}
/*----- 思考処理 --------------------------------------------------------*/
static void InitThinkParam( work )
Work	*work ;
{
	work->action.dir = -1 ;
	work->action.body_dir = -1 ;
	work->action.pad = 0 ;
	work->enethink.status2 = 0 ;
	work->enethink.thk_status = 0 ;
}

static void Think( work )
Work	*work ;
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	InitThinkParam( work ) ;

	switch( entk->think1 ){
		case ENE_TH1_SNEAK :
			ENE_Watcher_Think1_Sneak( entk ) ;
			SneakModeCheack( entk ) ;
		break ;
		case ENE_TH1_ALERT :
			ENE_Support_Think1_Alert( entk ) ;
			AlertModeCheack( entk ) ;
		break ;
		case ENE_TH1_AVOID :
			ENE_Support_Think1_Avoid( entk ) ;
			AvoidModeCheack( entk ) ;
		break ;
		case ENE_TH1_SEARCH :
			ENE_Support_Think1_Caution( entk ) ;
			SearchModeCheack( entk ) ;
		break ;
		case ENE_TH1_DAMAGE :
			ENE_Enemy_Think1_Damage( entk ) ;
			DamageModeCheack( entk ) ;
		break ;
		case ENE_TH1_RESURRECT :
			ENE_WatcherResurrect( entk ) ;
			RessModeCheck( entk ) ;
		break ;
	}

}
