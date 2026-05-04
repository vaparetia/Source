/*
   fort_fire.h
   フォーチュン 撃ちアクトセット
   2000/01/13 T.Morita
   $Id: fort_fire.h,v 1.1.1.3 2002/11/19 11:46:07 Yoshizawa1 Exp $
*/


/*

  ほとんどすべての撃ち（特殊モーション以外）はここを通る

*/
void FRT_ActFireP( Work *work )
{
    work->wait_sec-- ;

    /* 必ず下の積荷に当たらないようにするため */
    if ( work->wait_sec <= 0 )
	FRT_AIM_CheckAimSpot( work ) ;

    if ( work->wait_sec <= 0 &&
	 (work->control.turn.vy - work->control.rot.vy)/64 == 0 )/* 狙い先を向くまで待つ */
	work->act++ ;
}
void FRT_ActFire( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, work->flag&FRT_F_OVERLAYED ? 1 : 0 ) )
    {
	//FRT_AIM_CheckAimSpot( work ) ;/* 必ず下の積荷に当たらないようにするため */

	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )/* 主観撃ちは一発めは必ず外す */
	    work->flag |= FRT_F_ENB_WATCHATK ;

	work->lgt_on = TIME_BASE ;
	work->act++ ;
    }
}


/*

 立ち撃ち

 */
static void FRT_ActStandFirePStart( Work *work )
{
    FaceAimAt( work ) ;
    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    FaceEyeAt( work, work->trgt_aim ) ;

    if ( work->flag & (FRT_F_DONT_SHOOT|FRT_F_DEMO_MOVIE) || work->weap_blt<=0 )
	work->act = FRT_ActionAbort ;
    else
    {
	FRT_SendLightOffMessage( work ) ;

	GM_ConfigObjectAction( &work->body, 0, fire_p, 0, FRT_BODY_ALL, work->wait_sec ) ;
	ChangeFlag( work,
		    (FRT_F_ENB_AIM   | FRT_F_FACETO_HEAD  |
		     FRT_F_FACETO_AIM| FRT_F_ENB_ENERGY_SE),
		    FRT_F_NONE ) ;
	work->act++ ;
    }
}

static void FRT_ActStandFireStart( Work *work )
{
    if ( work->flag & (FRT_F_DONT_SHOOT| FRT_F_DEMO_MOVIE) || work->weap_blt<=0 )
	work->act = FRT_ActionAbort ;
    else
    {
	_sceVu0CopyVector( &work->weap_lnr, FRT_RecogAimPos( work ) ) ;
	work->weap_lnr.vw = 1.0f ;
	work->shot_cnt++ ;
	GM_ConfigObjectAction( &work->body, 0, fire, 0, FRT_BODY_ALL, 2 ) ;
	ChangeFlag( work, FRT_F_FACETO_HEAD| FRT_F_FACETO_AIM, FRT_F_NONE ) ;
	work->act++ ;
    }
}



/*

  リロード

 */
static void FRT_ActReloading( Work *work )
{
    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 29*5 ) )
	work->weap_amo = -4 ; /* show drop cartridge */
    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 30*5 ) )
	work->weap_amo = -3 ; /* show insert cartridge */
#if 0
    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 67*5 ) )
    {
	if ( work->flag & FRT_F_DEMO_MOVIE )
	    GM_SeSetMode( SD_W_EQUIP03, &work->control.mov, GM_SEMODE_BOMB ) ; 
    }
#endif
    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 68*5 ) )
	work->weap_amo = -2 ; /* hide cartridge */
    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 80*5 ) )
    {
	work->weap_blt = FRT_WEAPON_BULLET ; /* blowback release */
#if 0
	if ( work->flag & FRT_F_DEMO_MOVIE )
	    GM_SeSetMode( SD_E_MAGAZI01, &work->control.mov, GM_SEMODE_BOMB ) ; 
#endif
    }
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	FaceEyeAt( work, work->trgt_aim ) ;/* デフォルトに戻す */
	ChangeFlag( work, FRT_F_NONE, FRT_F_FACETO_ALL ) ;
	work->act++ ;
    }
}

static void FRT_ActStandReloadStart( Work *work )
{
    ChangeFlag( work, FRT_F_NONE, FRT_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_reload, 0, FRT_BODY_ALL, 30 ) ;
    work->act++ ;
}


/*

  構える

 */
static void FRT_ActPrepareStart( Work *work )
{
    FaceAimAt( work ) ;
    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    //ChangeFlag( work, FRT_F_NONE, FRT_F_FACETO_ALL ) ;
    ChangeFlag( work, FRT_F_FACETO_HEAD| FRT_F_FACETO_AIM, FRT_F_NONE ) ;

    GM_ConfigObjectAction( &work->body, 0, fire_prepare, 0, FRT_BODY_ALL, 30 ) ;
    
    //GM_SeSetMode( SD_E_KAMAE001, &work->control.mov, GM_SEMODE_BOMB ) ;
    work->act++ ;
}
