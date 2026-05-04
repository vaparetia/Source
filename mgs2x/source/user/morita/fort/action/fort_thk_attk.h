/*
   fort_thk_attk.h
   フォーチュン 思考関数 攻撃モード

   2000/01/05 T.Morita
   $Id: fort_thk_attk.h,v 1.1.1.3 2002/11/19 11:46:08 Yoshizawa1 Exp $
*/


static inline void NextPhase( Work *work, int phase )
{
    work->ply_time = 0 ;
    work->shot_cnt = 0 ;
    work->act_phase = phase ;

    work->flag &= ~FRT_F_ENB_WATCHATK ;/* 主観マジ狙いをフェーズが変更する毎に行なえるようにする */

    printf( "Phase Changed %x\n",phase  ) ;
}

static void FRT_ActThinkToAttack( Work *work )
{
    work->stll_tim++ ; /* 思考論理時間を進める */

    /* ここから反射的行動 */
    /* 弾なしの場合はリロードをする */
    if ( work->stll_mtn == stll_idle )
    {
	if ( work->stll_tim == 1 )
	    work->act = FRT_ActionTwoSecFaceWait ;
	else
	{
	    work->act = FRT_ActionPrepareWeapon ;
	    work->stll_mtn = stll_ready ;
	}
	ChangeFlag( work, FRT_F_NONE, FRT_F_MOVED_ONCE ) ;
	return ;
    }
    else if ( work->weap_blt <=0 )
    {
	ChangeFlag( work, FRT_F_NONE, FRT_F_ENB_AIM ) ;
	work->act = FRT_ActionStandReload ;
	return ;
    }
    else if ( work->flag & FRT_F_DEMO_MOVIE || GM_IsGameOver() )
    {
	work->act = FRT_ActionStandWhileDemo ;
	return ;
    }
    else if ( work->act_flg )
    {
	if ( work->act_flg & FRT_F_COUNTER_ATTK )
	{
	    work->act_flg &= ~FRT_F_COUNTER_ATTK ;/* フラグは下げておく */

	    work->trgt_aim = FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ;
	    work->act = FRT_ActionStandOneSecFire ;
	    if ( work->flag & FRT_F_COUNTER_SHOT )
		ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_NONE ) ;
	    else
		ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;
	    ChangeFlag( work, FRT_F_COUNTER_SHOT, FRT_F_NONE ) ;
	    return ;
	}

	else if ( work->act_flg & FRT_F_HIDING_ATTK )
	{
	    work->trgt_aim = FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ;
	    work->act = FRT_ActionStandOneSecFire ;
	    ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_NONE ) ;
	    if ( work->trgt_pos != &FRT_HidingPos )
	    {
#if 0
		work->trgt_typ = FRT_AIM_TypeWoodBox| FRT_AIM_TypeIronBox ;
		if ( !FRT_AIM_GetAimmingObjects( 0x0000, work->trgt_typ,
						 BODYPOS( &work->body, HUMAN21_MIGI_TE )) )
		    ChangeFlag( work, FRT_F_NONE, FRT_F_OFFENCE ) ;
#else
		work->trgt_typ = FRT_AIM_TypeWoodBox| FRT_AIM_TypeIronBox ;
		//work->act = FRT_ActionAbort ;
		ChangeFlag( work, FRT_F_NONE, FRT_F_OFFENCE ) ;
#endif
	    }
	    else
		ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;

	    return ;
	}

	else if ( work->act_flg & FRT_F_GASTANK_ATTK )
	{
	    ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_UNRECOG_POS ) ;
	    work->act = FRT_ActionStandZeroSecFire ;
	    work->trgt_typ = FRT_AIM_TypeGasCan ;
	    work->trgt_typ |= FRT_AIM_DontDestinate| FRT_AIM_NoPhase| FRT_AIM_NoOnlineCheck ; 
	    if ( FRT_AIM_GetAimmingObjects( 0x0cff, work->trgt_typ,
					    BODYPOS( &work->body, HUMAN21_MIGI_TE )) )
		return ;
	    else
		ChangeFlag( work, FRT_F_NOMORE_DRUM, FRT_F_NONE ) ;
	}

	else if ( work->act_flg & FRT_F_RAGING_ATTK )
	{
printf( "FRT_F_RAGING_ATTK\n" ) ;
	    ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;
	    work->act = FRT_ActionStandZeroSecFire ;
	    work->trgt_aim = FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ;
	    work->trgt_typ = FRT_AIM_TypeNothing ;
	    return ;
	}

	else if ( work->act_flg & FRT_F_STAND_ATTK )
	{
	    ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;
	    work->act = FRT_ActionStandZeroSecFire ;
	    work->trgt_aim = FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ;
	    return ;
	}
    }

#if 0
    if ( work->trgt_pos == &work->trgt_frc )
    {
	ChangeFlag( work, FRT_F_NONE, FRT_F_OFFENCE ) ;
	work->act = FRT_ActionAbort ;
	return ;
    }
#endif

    /* ここまでが反射的行動 */

    /*フェーズごとの処理*/
    switch( work->act_phase & 0xff00 )
    {
    case 0x0000:
	NextPhase( work, 0x0100 ) ;
	//ChangeFlag( work, FRT_F_NONE, FRT_F_OFFENCE ) ; /*デバッグコード 2/3 */
	//NextPhase( work, 0x0cff ) ;                     /*デバッグコード 2/3 */
	//NextPhase( work, 0x0800 ) ;                     /*デバッグコード 5/10*/

	/* 15秒以下で未だフォーチュンに対し撃っていない */
    case 0x0100:
	if ( GM_GameLevel>=GM_LEVEL_EXTREME /*&& work->ply_aim_cnt*/ )
	{
	    ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_NONE ) ;
	    work->act = FRT_ActionStandDoubleFire ;
	}
	else
	{
	    ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_OFFENCE ) ;
	    work->act = FRT_ActionStandTwoSecFire ;
	}
	work->trgt_typ = (FRT_AIM_TypeGasCan | FRT_AIM_TypeWoodBox|
			  FRT_AIM_TypeIronBox| FRT_AIM_NoPhase    )  ;
	if ( work->ply_time > 15 /*&& work->ply_aim_cnt*/ )
	    NextPhase( work, 0x0200 ) ;
	break ;

	/* 壁や天井の電灯を落す */
    case 0x0200:
	ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_NONE ) ;

	if ( GM_GameLevel>=GM_LEVEL_EXTREME )
	    work->act = FRT_ActionStandZeroSecFire ;
	else
	    work->act = FRT_ActionStandOneSecFire ;
	work->trgt_typ = (FRT_AIM_TypeHangLight | FRT_AIM_TypeWallLight|
			  FRT_AIM_NoPhase| FRT_AIM_DontDestinate )  ;
	if ( (work->act_phase & 0xff) >= 2 )
	    NextPhase( work, 0x0400 ) ;
	else
	    work->act_phase++ ;
	break ;

	/*前列壊す*/
    case 0x0400:
	ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_NONE ) ;
	work->act = FRT_ActionStandZeroSecFire ;
	work->trgt_typ = FRT_AIM_TypeWoodBox| FRT_AIM_TypeIronBox| FRT_AIM_DontDestinate ;
	if ( !FRT_AIM_GetAimmingObjects( 0x03ff, work->trgt_typ,
					 BODYPOS( &work->body, HUMAN21_MIGI_TE )) )
	    NextPhase( work, 0x0800 ) ;
	break ;

	/*フォークリフト壊す*/
    case 0x0800:
	ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_NONE ) ;
	if ( GM_GameLevel>=GM_LEVEL_EXTREME )
	    work->act = FRT_ActionStandDoubleFire ;
	else
	    work->act = FRT_ActionStandOneSecFire ;
	work->trgt_typ = FRT_AIM_TypeForkLift| FRT_AIM_DontDestinate| FRT_AIM_NoOnlineCheck ;
	if ( !FRT_AIM_GetAimmingObjects( work->act_phase, work->trgt_typ,
					 BODYPOS( &work->body, HUMAN21_MIGI_TE )) )
	{
	    work->voice = SD_V_FORTUN01 ;/* 嘲笑「あはははっ」 */
	    work->voice_tim = 5 ;
	    NextPhase( work, 0x0aff ) ;
	}
	break ;

	/*物を壊しつつライデンを追う*/
    case 0xa00:
	ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_UNRECOG_POS ) ;
	if ( GM_GameLevel>=GM_LEVEL_HARD )
	    ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;
	if ( GM_GameLevel <= GM_LEVEL_VERYEASY )
	    work->act = FRT_ActionStandTwoSecFire ;
	else if ( GM_GameLevel<=GM_LEVEL_HARD )
	    work->act = FRT_ActionStandOneSecFire ;
	else
	    work->act = FRT_ActionStandDoubleFire ;
	work->trgt_typ = (FRT_AIM_TypeGasCan | FRT_AIM_TypeWoodBox|
			  FRT_AIM_TypeIronBox| FRT_AIM_NoPhase ) ;
	if ( work->ply_time > 20 )
	{
	    ChangeFlag( work, FRT_F_NONE, FRT_F_OFFENCE ) ;
	    /* 昇降機 ランプ点灯 */
	    FRT_RecogExecProc( work, FRT_P_ELEVATOR_CUT ) ;
	    NextPhase( work, 0x0cff ) ;
	}
	break ;

	/*ライデンを本気で追う*/
    case 0x0c00:
	ChangeFlag( work, FRT_F_AIMMING_AUTO, FRT_F_UNRECOG_POS ) ;

	if ( GM_GameLevel>=GM_LEVEL_NORMAL )
	    ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;
	if ( GM_GameLevel<=GM_LEVEL_NORMAL )
	    work->act = FRT_ActionStandOneSecFire ;
	else if ( GM_GameLevel<=GM_LEVEL_HARD )
	    work->act = FRT_ActionStandZeroSecFire ;
	else
	    work->act = FRT_ActionStandDoubleFire ;

	work->trgt_typ = (FRT_AIM_TypeGasCan | FRT_AIM_TypeWoodBox| FRT_AIM_TypeContainer|
			  FRT_AIM_TypeIronBox| FRT_AIM_NoPhase) ;

#if DEBUG_MODE	
	{
	    extern int FRT_DbgAgingTest ;/* エイジグテスト用 */
	    if (  FRT_DbgAgingTest )
		break ;
	}
#endif

	if ( work->ply_time > 25 )
	{
	    ChangeFlag( work, FRT_F_NOMORE_DRUM|FRT_F_SHOOT_AT_ME|FRT_F_THROW_AT_ME, FRT_F_NONE ) ;

	    FRT_RecogExecProc( work, FRT_P_ELEVATOR_DOWN ) ;

	    NextPhase( work, 0x0d00 ) ;
	}
	break ;

	/*ライデンのいない方から外側の爆発物を壊す*/
    case 0x0d00:
	ChangeFlag( work, FRT_F_AIMMING_AUTO| FRT_F_UNRECOG_POS, FRT_F_NONE ) ;
	work->act = FRT_ActionStandZeroSecFire ;
	if ( FRT_PLY_PLAYERPOS.vx >0.0f )
	    work->trgt_typ = FRT_AIM_TypeExplosiv_R ;
	else
	    work->trgt_typ = FRT_AIM_TypeExplosiv_L ;
	work->trgt_typ |= FRT_AIM_NoPhase| FRT_AIM_DontDestinate| FRT_AIM_NoOnlineCheck ;

	if ( !FRT_AIM_GetAimmingObjects( work->act_phase, work->trgt_typ,
					 BODYPOS( &work->body, HUMAN21_MIGI_TE )) )
	    work->act = FRT_ActionAbort ;
	NextPhase( work, 0x0e00 ) ;
	break ;

	/*最後の爆発物を壊す*/
    case 0x0e00:
	ChangeFlag( work, FRT_F_AIMMING_AUTO| FRT_F_UNRECOG_POS, FRT_F_NONE ) ;
	work->act = FRT_ActionStandZeroSecFire ;
	work->trgt_typ = (FRT_AIM_TypeExplosiv_L | FRT_AIM_TypeExplosiv_R|
			  FRT_AIM_NoPhase| FRT_AIM_DontDestinate| FRT_AIM_NoOnlineCheck) ;
	if ( !FRT_AIM_GetAimmingObjects( work->act_phase, work->trgt_typ,
					 BODYPOS( &work->body, HUMAN21_MIGI_TE )) )
	    work->act = FRT_ActionAbort ;
	NextPhase( work, 0x0f08 ) ;
	break ;

	/*瓦礫落し*/
    case 0x0f00:
	ChangeFlag( work, FRT_F_AIMMING_AUTO| FRT_F_UNRECOG_POS, FRT_F_NONE ) ;
	work->act = FRT_ActionStandZeroSecFire ;
	work->trgt_typ = (FRT_AIM_NoPhase| FRT_AIM_TypeCeiling|
			  FRT_AIM_DontDestinate| FRT_AIM_NoOnlineCheck) ;

	work->voice = (work->shot_cnt==0 ? SD_V_FORTUN07/*「ふんっ」*/ :
		       work->shot_cnt==1 ? SD_V_FORTUN08/*「はぁっ」*/ :
		                           SD_V_FORTUN09/*「でぇえぃ」*/) ;
	work->voice_tim = 5 ;

	work->act_phase++ ;
	FRT_LastCieling++ ; /* 強制落し用当たらなかった場合 */
	FRT_AIM_GetAimmingObjects( work->act_phase, work->trgt_typ,
				   BODYPOS( &work->body, HUMAN21_MIGI_TE ) ) ;
	if ( (work->act_phase & 0x7) >= 4 )
	{
	    NextPhase( work, 0x1000 ) ;
	    work->act = FRT_ActionAbort ;
	}

	break ;

    case 0x1000:
	if ( !(work->act_phase & 0xff) )
	{
	    work->act_phase++ ;

	    FRT_RecogExecProc( work, FRT_P_GAME_END ) ;

#if DEBUG_MODE
	    FRT_AIM_DebugPrint() ;
#endif
	}
    default:
	work->act = FRT_ActionAbort ;
    }
}
