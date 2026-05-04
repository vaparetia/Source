/*
	wcthink.c
	見張り兵、思考ルーチン

	1999/07/07 Y.Korekado
	$Id: wcthink.c,v 1.1.1.3 2002/11/19 11:44:32 Yoshizawa1 Exp $
	
*/
/*----- 高レベルモード移行チェック --------------------------------------------------*/
static	void	SneakModeCheack( entk )
ENETHINK	*entk ;
{
	switch( GM_AlertMode ) {
		case ALERT_MODE_ALERT :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_WatcherStartModeAlert( entk ) ;
		break ;
		case ALERT_MODE_AVOID :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_WatcherStartModeAvoid( entk ) ;
		break ;
		case ALERT_MODE_SEARCH :
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
			ENE_WatcherStartModeAvoid( entk ) ;
		break ;
		case ALERT_MODE_SEARCH :
			ENE_WatcherStartModeSneak( entk ) ;
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
			ENE_WatcherStartModeAlert( entk ) ;
		break ;
		case ALERT_MODE_SEARCH :
			if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
			ENE_WatcherStartModeSneak( entk ) ;
		break ;
	}
	if ( ENE_DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_TRACE ) ENE_TraceEnd( entk ) ;
		ENE_EnemyStartModeDamage( entk ) ;
	}
}
static	void	DamageModeCheack( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_DAMAGE ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				ENE_WatcherStartModeSneakDamage( entk ) ;
				if ( COM_StageKind() & ENE_STAGE_NO_NOTICE ) {
					/* 潜入モードのみ */
					ENE_WatcherStartModeSneak( entk ) ;
				}
			break ;
			case ALERT_MODE_ALERT :
				ENE_WatcherStartModeAlert( entk ) ;
			break ;
			case ALERT_MODE_SEARCH :
				ENE_WatcherStartModeSneakDamage( entk ) ;
			break ;
			default :
				ENE_WatcherStartModeAvoidDamage( entk ) ;
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
	if ( entk->notice & ENE_NOTICE_RES ) {
		switch( GM_AlertMode ) {
			case ALERT_MODE_SNEAK :
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			case ALERT_MODE_ALERT :
				ENE_WatcherStartModeAlert( entk ) ;
			break ;
			case ALERT_MODE_SEARCH :
				ENE_WatcherStartModeSneak( entk ) ;
			break ;
			default :
				ENE_WatcherStartModeAvoid( entk ) ;
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
	work->action.actgun.gun_pad = 0 ;
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
			ENE_Watcher_Think1_Alert( entk ) ;
			AlertModeCheack( entk ) ;
		break ;
		case ENE_TH1_AVOID :
			ENE_Watcher_Think1_Avoid( entk ) ;
			AvoidModeCheack( entk ) ;
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
