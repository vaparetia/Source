/*
   orga_thk_attk.h
   オルガ 思考関数 攻撃モード

   2000/01/05 T.Morita
   $Id: orga_thk_attk.h,v 1.1.1.3 2002/11/19 11:46:23 Yoshizawa1 Exp $
*/

static int ORG_ActThinkToAttack( Work *work, void (***list)( Work * ) )
{
    int lvl = work->flag & ORGA_F_LEVEL_MSK ;

#if DEBUG_MODE
    if ( !ORG_DebugAttackAction( work, list ) )
	return 0 ;
#endif

    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;/* 安全な位置にいるのか */
    work->stll_tim++ ; /* 思考論理時間を進める */

    /* ここから反射的行動 */
    /* 弾なしの場合はリロードをする */
    if ( work->weap_blt <= 0 )
	ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM ),
	    *list = ORG_ActListReload[ (int)work->stll_mtn ] ;

    /* ２回に１回は,静止コマンド（ガチャガチャ行為防止） */
    else if ( work->stll_tim & 1 && 0 )
	*list = ORG_ActListWait[(irnd()&3)+2] ;/* とりあえず,ここには来ない */

    else if ( CHECKACT( work ) )
    {
	/* マガジンの落ちた場所を見て 少し見回す */
	if ( work->act_flg & ORGA_F_MAGAZIN_ATTK )
	{
	    *list = ORG_ActionMagazineThrown ;
	    ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
	}

	/* 人生を語る カットインデモ */
	else if ( work->act_flg & ORGA_F_BIBLIO_ATTK )
	{
	    /*マルチウェイト表示*/
	    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	    DG_InvisibleObjs( work->body.objs ) ;

	    switch( work->voice_vox >> 12 )
	    {
	    case 0:
		if ( work->procs[ORGA_P_BIBLIO_FST0] )
		    GCL_ExecProc( work->procs[ORGA_P_BIBLIO_FST0], NULL ) ;/*語り最初プロック実行*/
		*list = ORG_ActionTeaseSpeakStreamFirst ;
		break ;
	    case (ORGA_N_STREAM-1):
		if ( work->procs[ORGA_P_BIBLIO_LST0] )
		    GCL_ExecProc( work->procs[ORGA_P_BIBLIO_LST0], NULL ) ;/*語り最後プロック実行*/
		*list = ORG_ActionTeaseSpeakStreamLast ;
	    }
	    ChangeFlag( work, ORGA_F_NVR_DAMAGED, ORGA_F_OFFENCE| ORGA_F_FACETO_ALL ) ;
	}



	/* アホなスネークがエルードしているから撃ちにいく */
	else if ( work->act_flg & ORGA_F_ERUDE_ATTK )
	{
	    if ( work->procs[ORGA_P_ANTI_ERUDE] )
		GCL_ExecProc( work->procs[ORGA_P_ANTI_ERUDE], NULL ) ;/*プロック実行*/
	    *list = ORG_ActionErudeKiller ;
	    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_ENB_SAFE| ORGA_F_OFFENCE ) ;
	}



#if 0
	/* 禁止区域で這っている（匍匐のみ） */
	else if ( work->act_flg & ORGA_F_GOUND_ATTK )
	{
	    if ( work->procs[ORGA_P_MACHINGUN] )
		GCL_ExecProc( work->procs[ORGA_P_MACHINGUN], NULL ) ;/*プロック実行*/
	    *list = ORG_ActionSquatFastZeroSecPeepingQuadpleFireLeft ;
	    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_ENB_SAFE| ORGA_F_ENB_AVOID ) ;
	}



	/* 禁止区域に入っている */
	else if ( work->act_flg & ORGA_F_STAND_ATTK )
	{
	    *list = ORG_ActionThroughGrenadeLow ;
#if 0
	    if ( !(work->stll_tim & 3) )
		*list = ORG_ActionThroughGrenadeLow ;
	    else if ( GM_PlayerControl->mov.vz > -13750.0f )
		work->stll_mtn = 0,
		    *list = ORG_ActionFastZeroSecPeepingQuadpleFireLeft
		    /*ORG_ActionLongTermPeepingFireLeft*/ ;
	    else 
		work->stll_mtn = ORGA_F_IS_SQUAT,
		    *list = ORG_ActionSquatFastZeroSecPeepingQuadpleFireLeft
		    /*ORG_ActionLongTermSquatPeepingFireLeft*/  ;
	    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_ENB_SAFE| ORGA_F_ENB_AVOID ) ;
#endif
	}
#endif


	/* ホロの影になって攻撃 */
	else if ( work->act_flg & ORGA_F_HOLO_HIDE_ATTK )
	{
	    work->stll_mtn = 0 ;

	    if ( work->stll_tim >= ORGA_HOLO_THINKTIME )
	    {
		/*移動ホロきり攻撃のため（ここのみの例外措置） trgt_posとORGA_F_ENB_ROUTE を設定する*/
		work->hide_spot = &work->hide_pool[0] ;
		work->trgt_pos = &work->hide_spot->right ;
		ORG_HOL_GetStringPos( &work->trgt_aim_pos ) ;
		work->flag |= ORGA_F_DANGR_RIGHT ;
		*list = ORG_ActionAimmingHoloDash ;
		//ChangeFlag( work, ORGA_F_ENB_ROUTE|ORGA_F_DMG_HOROATTK, ORGA_F_OFFENCE ) ;
		ChangeFlag( work, ORGA_F_ENB_ROUTE|ORGA_F_DMG_HOROATTK, ORGA_F_NONE ) ;
	    }
	    else if ( !(work->stll_tim & 3) )
		*list = ORG_ActionStandThroughGrenade ;
	    else if ( work->trgt_pos == &ORG_Ply_AttkHoloL )
	    {
		*list = GV_Time&1 ?
		    ORG_ActionStandQuaterSecDoubleFire : ORG_ActionStandQuaterSecTripleFire ;
#if 0
		if ( work->ply_locate>=0x170 )
		    *list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
#endif
	    }
	    else
	    {
		*list = ORG_ActionSlowHalfSecPeepingTripleFireRight ;
#if 0
		/* はみ出てたら移動 */
		if ( work->ply_locate>=0x170 )
		{
		    float z = ORG_GetHidePosZ( &work->hide_pool[2],
					       ORG_RecogPlayerPosNoHide( ORGA_GET_PLY_CONTROL ),
					       -ORGA_HIDE_H_SPHERE,
					       work->stll_mtn ) ;
		    if ( (int)((work->control.mov.vz - z)/400.0f) )
			*list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
		}
		else
		    *list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
#endif
	    }
	}



	/* 投光器をまぶしくさせてスネークを困らせる */
	else if ( work->act_flg & ORGA_F_SPOTLGT_ATTK )
	{
	    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_ENB_SAFE ) ;
	    work->stll_mtn = 0 ;

	    if ( GM_CheckPlayerStatus( PLAYER_GROUND|PLAYER_SQUAT ) &&
		 !(((GM_PlayerControl->turn.vy & 4095) - 3072)/512) &&
		 GM_PlayerControl->mov.vx >=  -9000.0f &&
		 GM_PlayerControl->mov.vx <=  -7500.0f &&
		 GM_PlayerControl->mov.vz >= -17300.0f &&
		 GM_PlayerControl->mov.vz <= -16400.0f )/* すき間にいる */
	    {
		if ( work->trgt_pos == &ORG_Ply_AttkLightR )
		{
		    ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
		    *list = ORG_ActionStandQuaterSecTripleFire ;
		}
		else
		    *list = ORG_ActionStandThroughGrenade ;
	    }
	    /* ライトまぶしいフェーズに突入 */
	    else if ( ORG_RecogCheckSpotLight( work ) == 1 )
	    {
		if ( work->procs[ORGA_P_FLARED_LIGHT] )
		    GCL_ExecProc( work->procs[ORGA_P_FLARED_LIGHT], NULL ) ;/*プロック実行*/
		*list = ORG_ActionBreakSpotLight ;
	    }
	    /* ライトの向き変え要請があった */
	    else if ( work->act_flg & ORGA_F_CHNGLGT_ATTK )
	    {
		printf( "I'm Going to Change Dir\n" ) ;
		work->act_flg &= ~ORGA_F_CHNGLGT_ATTK ;
		*list = ORG_ActionBreakSpotLight ;
	    }
	    else 
	    {
		if ( work->stll_tim > 3 )
		{
		    ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
		    *list = ORG_ActionStandThroughGrenade ;
		}
		else
		    *list = ORG_ActionStandQuaterSecTripleFire ;
	    }
	}


	/* すき間からスネークがしゃがんでいる */
	else if ( work->act_flg & ORGA_F_GAP_ATTK )
	{
	    float z ;
	    static float box[] = { -10550,-16500,   -10550,-16500,
				   -10550,-17300,   -13500,-17500 } ;

	    /* 立ち場所を再計算 */
	    if ( work->trgt_pos == &ORG_Ply_AttkPosL )
		z = ORG_CalcNewHidePosZ( &box[0], &box[2],
					 &GM_PlayerControl->mov,
					 ORGA_HIDE_SQUAT_SPHERE,
					 work->trgt_pos->vx ) ;
	    else
		z = ORG_CalcNewHidePosZ( &box[4], &box[6],
					 &GM_PlayerControl->mov,
					 -ORGA_HIDE_SQUAT_SPHERE,
					 work->trgt_pos->vx ) ;

	    work->stll_mtn = ORGA_F_IS_SQUAT ;

	    /* 隠れ場所から撃つのは初めて？ならオルガは,どう足掻いても
               見つけられない */
	    if ( !(work->flag & ORGA_F_DMG_FARHIDE) && work->rage < 4 )
	    {
		if ( work->procs[ORGA_P_SNAKE_HIDE] )
		    GCL_ExecProc( work->procs[ORGA_P_SNAKE_HIDE], NULL ) ;/*プロック実行*/
		work->rage++ ;
		*list = work->trgt_pos == &ORG_Ply_AttkPosL ?
		    ORG_ActionUnrecogSquatPeepLeft :
		    ORG_ActionUnrecogSquatPeepRight ;
		//if ( work->stll_tim >= ORGA_STILL_THINKTIME )
		//    ChangeFlag( work, ORGA_F_OFFENCE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM ) ;
	    }

	    /* 隠れ場所から撃つのは初めてじゃないから攻撃する(グレネー
               ド投げ)*/
	    else if ( GM_PlayerStatus & PLAYER_SQUAT ||
		      work->stll_tim & 1 )
	    {
		*list = ORG_ActionSnakeHideThroughGrenade ;
		work->stll_tim = ORGA_STILL_THINKTIME-2 ;
		work->ply_hide &= ORGA_F_PLYR_HIDCLR ;/*これをしないと投げまくるから*/
	    }
	    /* 隠れ場所から撃つのは初めてじゃないから攻撃する (しゃがみ撃ち)*/
	    else
	    {
		ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_ENB_AVOID ) ;
		*list = work->trgt_pos == &ORG_Ply_AttkPosL ?
		    ORG_ActListPeepingFireLeft[3]  :
		    ORG_ActListPeepingFireRight[3] ;
	    }
	    printf ( "trgtz %.1f z%.1f (%d %d %d)\n",
		     work->trgt_pos->vz-200.0f, z,
		     work->trgt_pos->vz-200.0f < z,
		     work->trgt_pos->vz+200.0f > z,
		     work->stll_tim >= ORGA_STILL_THINKTIME ) ;
	    if ( work->trgt_pos->vz-200.0f < z ||
		 work->trgt_pos->vz+200.0f > z ||
		 work->stll_tim >= ORGA_STILL_THINKTIME )
	    {
		ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
	    }
	}


	/* 体力が減ったからホロを風になびかせるため撃ちにいく */
	else if ( work->act_flg & ORGA_F_HOLO_ATTK )
	{
	    *list = ORG_ActionOpenHolo ;
	    ChangeFlag( work, ORGA_F_NOMISS_SHOT,
			ORGA_F_ENB_SAFE| ORGA_F_OFFENCE ) ;
	}

	/* 投光器フェーズに移行するために ホロ飛ばし */
	else if ( work->act_flg & ORGA_F_HOLO_BLOW_ATTK )
	{
	    *list = ORG_ActionBlowHolo ;
	    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_OFFENCE ) ;
	}

	/* ライフが１／８のプロック */
	else if ( work->act_flg & ORGA_F_HALF_ATTK )
	{
	    if ( work->procs[ORGA_P_LIFE_HALF] )
		GCL_ExecProc( work->procs[ORGA_P_LIFE_HALF], NULL ) ;/*プロック実行*/
	    if ( work->flag & ORGA_F_DANGR_LEFT )
		*list = ORG_ActionTeaseSpeakFastLeft ;
	    else
		*list = ORG_ActionTeaseSpeakFastRight ;
	}

	/* ライフが１／４のプロック */
	else if ( work->act_flg & ORGA_F_QUAD_ATTK )
	{
	    if ( work->procs[ORGA_P_LIFE_QUAD] )
		GCL_ExecProc( work->procs[ORGA_P_LIFE_QUAD], NULL ) ;/*プロック実行*/
	    if ( work->flag & ORGA_F_DANGR_LEFT )
#if 0
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionTeaseSpeakFastSquatLeft  : ORG_ActionTeaseSpeakFastLeft  ;
#else
		*list = ORG_ActionTeaseSpeakFastLeft ;
#endif
	    else
#if 0
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionTeaseSpeakFastSquatRight : ORG_ActionTeaseSpeakFastRight ; 
#else
		*list = ORG_ActionTeaseSpeakFastRight ;
#endif
	}

	else if ( work->act_flg & ORGA_F_EIGHT_ATTK )
	{
	    if ( work->procs[ORGA_P_LIFE_EIGHT] )
		GCL_ExecProc( work->procs[ORGA_P_LIFE_EIGHT], NULL ) ;/*プロック実行*/
	    if ( work->flag & ORGA_F_DANGR_LEFT )
#if 0
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionTeaseSpeakSlowSquatLeft  : ORG_ActionTeaseSpeakSlowLeft  ;
#else
		*list = ORG_ActionTeaseSpeakSlowLeft ;
#endif
	    else
#if 0
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionTeaseSpeakSlowSquatRight : ORG_ActionTeaseSpeakSlowRight ;
#else
		*list = ORG_ActionTeaseSpeakSlowRight ;
#endif
	}
	else if ( work->act_flg & ORGA_F_THIRD_ATTK )
	{
printf( "ORGA_P_LIFE_THIRD%d\n", work->procs[ORGA_P_LIFE_THIRD] ) ;
	    if ( work->procs[ORGA_P_LIFE_THIRD] )
		GCL_ExecProc( work->procs[ORGA_P_LIFE_THIRD], NULL ) ;/*プロック実行*/
	    if ( work->flag & ORGA_F_DANGR_LEFT )
#if 0
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionTeaseSpeakSlowSquatLeft  : ORG_ActionTeaseSpeakSlowLeft  ;
#else
		*list = ORG_ActionTeaseSpeakSlowLeft ;
#endif
	    else
#if 0
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionTeaseSpeakSlowSquatRight : ORG_ActionTeaseSpeakSlowRight ;
#else
		*list = ORG_ActionTeaseSpeakSlowRight ;
#endif
	}

	else if ( work->act_flg & ORGA_F_CHNGLGT_ATTK )
	{
	    *list = ORG_ActionAbort ;
	    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_SAFE| ORGA_F_OFFENCE ) ;
	    work->act_flg &= ~ORGA_F_CHNGLGT_ATTK ;
	}

    }

    /* 特別な場所に来ているから取り敢えず移動する（ちょっと保険掛けすぎかな？） */
    else if ( work->trgt_pos == &ORG_Ply_AttkGrnd || work->trgt_pos == &ORG_Ply_AttkStnd  ||
	      work->trgt_pos == &ORG_ShootHoloPos || work->trgt_pos == &ORG_ShootHoloPos2 ||
	      work->trgt_pos == &ORG_Ply_AttkPosL || work->trgt_pos == &ORG_Ply_AttkPosR  ||
	      work->trgt_pos == &ORG_Ply_AttkHoloL|| work->trgt_pos == &ORG_Ply_AttkHoloR ||
	      work->trgt_pos == &ORG_ShootHoloRun1|| work->trgt_pos == &ORG_ShootHoloRun2 ||
	      work->trgt_pos == &ORG_ShootErudePos )
	*list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;

    /* 撃てなかったら取り敢えず移動する */
    else if ( work->aim_flg & ORGA_F_DONT_SHOOT )
	*list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;

    /* スネークから丸見えなので移動する */
    else if ( work->exposed >= 20 && !(work->flag & ORGA_F_UNRECOG_AIM) )
	*list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;

    /* 見失った時の固定シーケンス */
    else if ( !(work->flag & ORGA_F_UNRECOG_AIM) &&
	        work->flag & ORGA_F_UNRECOG_POS && 
	      (work->ply_hide & ORGA_F_PLYR_HIDTIM) > 2 )
    {
	/* ビハインド時の攻撃アクション(飛びだし撃ち) */
	if ( work->stll_mtn & ORGA_F_IS_BEHIND )
	{
	    printf( "YATTOKAI\n",  work->stll_mtn & ORGA_F_IS_BEHIND ) ;

	    ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
	    if ( work->flag & ORGA_F_DANGR_LEFT )
		*list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionSquatBehindPeepingFireLeft  : ORG_ActionBehindPeepingFireLeft  ;
	    else if ( work->flag & ORGA_F_DANGR_RIGHT )
		*list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionSquatBehindPeepingFireRight : ORG_ActionBehindPeepingFireRight ;
	    //work->stll_tim = 0 ;
	}

	/* まず迷う */
	work->stll_mtn &= ~ORGA_F_IS_BEHIND ;
	if ( work->stll_tim == ORGA_STILL_THINKTIME*3 )
	{
	    work->stll_tim = 0 ;
	    *list = ORG_ActionAbort ;
	    work->flag |= ORGA_F_UNRECOG_AIM ;
	    work->exposed = 0 ;
	}
	else if ( work->stll_tim == ORGA_STILL_THINKTIME*3-1 )
	    *list = ORG_ActionStandThroughGrenade ;
	else if ( work->weap_blt < 4 )
	{
	    ChangeFlag( work,
			ORGA_F_ENB_SAFE,
			ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM ) ;
	    *list = ORG_ActListReload[ (int)work->stll_mtn ] ;
	}
	else if ( work->weap_blt < 10 )
	{
	    work->stll_tim = ORGA_STILL_THINKTIME*3-3 ;
	    if ( work->flag & ORGA_F_DANGR_LEFT )
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionLongTermSquatPeepingFireLeft  : ORG_ActionLongTermPeepingFireLeft  ;
	    else
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionLongTermSquatPeepingFireRight : ORG_ActionLongTermPeepingFireRight ;
	}
	else
	{
	    if ( work->flag & ORGA_F_DANGR_LEFT )
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionSquatSlowHalfSecPeepingTripleFireLeft :
		    ORG_ActionSlowHalfSecPeepingTripleFireLeft      ;
	    else
		*list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionSquatSlowHalfSecPeepingTripleFireRight :
		    ORG_ActionSlowHalfSecPeepingTripleFireRight      ;
	}
    }

    /* 迷った時の固定シーケンス */
    else if ( work->flag & ORGA_F_UNRECOG_AIM )
    {
	work->stll_mtn &= ~ORGA_F_IS_BEHIND ;

	/* からかう */
	if ( work->ply_stop > 3 &&                             /* 6秒以上静止している */
	     (work->hide_spot ? work->hide_spot->id==1 : 0) && /* 右前の低い障害物にいる */
	     lvl < ORGA_F_LEVEL_HARD && work->weap_blt >= 3 && /* ノーマル以下で 3発以上弾がある */
	     (work->voice_vox >> 12) >= ORGA_N_STREAM )        /* 人生語りの最中でない */
	{
	    if ( work->procs[ORGA_P_TEASE] )
		GCL_ExecProc( work->procs[ORGA_P_TEASE], NULL ) ;/*プロック実行*/
	    //if ( work->weap_blt >= 3 )
		*list = ORG_ActionTeasing1 ;
	    //*list = ORG_ActionTeasing2 ;
	    work->ply_stop = 0 ;
	}

	else if ( GM_GameLevel>=GM_LEVEL_HARD ? work->stll_tim > ORGA_STILL_THINKTIME/2 :
	                                        work->stll_tim > ORGA_STILL_THINKTIME )
	{
	    if ( work->procs[ORGA_P_GRENADE] )
		GCL_ExecProc( work->procs[ORGA_P_GRENADE], NULL ) ;
	    *list = ORG_ActionUnrecogThroughGrenade ;
	}
	else if ( work->flag & ORGA_F_DANGR_STAND )
	    *list = ORG_ActionUnrecog ;
	else if ( work->flag & ORGA_F_DANGR_LEFT )
	    *list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		ORG_ActionUnrecogSquatPeepLeft  : ORG_ActionUnrecogPeepLeft  ;
	else
	    *list = work->stll_mtn & ORGA_F_IS_SQUAT ?
		ORG_ActionUnrecogSquatPeepRight : ORG_ActionUnrecogPeepRight ;

	if ( work->stll_tim > ORGA_STILL_THINKTIME*2 )
	    work->stll_tim = 0 ;
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
    }

    /* ここまでが反射的行動 */
    else
    {
	if ( lvl >= ORGA_F_LEVEL_NORM )
	{
	    /* 隠れずに止まったままので NOMISSショット */
	    if ( work->ply_stop > 3 && !(work->ply_hide & ORGA_F_PLYR_HIDING) )
		ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_OFFENCE ) ;

	    /* 主観で構えたまま待っているので NOMISSショット */
	    else if ( work->ply_stop > 3 && ((GM_PlayerStatus & (PLAYER_HOLD| PLAYER_WATCH)) || PL_SubjectMove) )//BP_CAMERA - added PL_SubjectMove (which we use for 1st person movement mode) so Ogla can be damaged
		ChangeFlag( work,
			    ORGA_F_NOMISS_SHOT| ORGA_F_LEVEL_HARD,
			    ORGA_F_ENB_AVOID  | ORGA_F_LEVEL_MSK |ORGA_F_OFFENCE ) ;
	}

	/* ビハインド時の攻撃アクション(飛びだし撃ち) */
	if ( work->stll_mtn & ORGA_F_IS_BEHIND )
	{
	    ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
	    if ( work->flag & ORGA_F_DANGR_LEFT )
		*list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionSquatBehindPeepingFireLeft  : ORG_ActionBehindPeepingFireLeft  ;
	    else if ( work->flag & ORGA_F_DANGR_RIGHT )
		*list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
		    ORG_ActionSquatBehindPeepingFireRight : ORG_ActionBehindPeepingFireRight ;
	    //work->stll_tim = 0 ;
	}

	/* 少し怒っているので避けずに覗き込み撃ち */
	else if ( work->rage )
	{
	    if ( GM_GameLevel > GM_LEVEL_NORMAL || work->rage>1 )
	    {
		if ( lvl > ORGA_F_LEVEL_NORM || !(work->rage&1) )
		    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_NONE ) ;
		if ( work->flag & ORGA_F_DANGR_LEFT )
		    *list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
			ORG_ActionSquatNoFearPeepingFireLeft  : ORG_ActionNoFearPeepingFireLeft  ;
		else if ( work->flag & ORGA_F_DANGR_RIGHT )
		    *list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
			ORG_ActionSquatNoFearPeepingFireRight : ORG_ActionNoFearPeepingFireRight ;
	    }
	    else/* NORMAL EASYのみクリーンショット 避けずに2秒狙う */
	    {
		ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_NONE ) ;
		if ( work->flag & ORGA_F_DANGR_LEFT )
		    *list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
			ORG_ActionSquatCleanShotPeepingLeft : ORG_ActionCleanShotPeepingLeft   ;
		else if ( work->flag & ORGA_F_DANGR_RIGHT )
		    *list++ = work->stll_mtn & ORGA_F_IS_SQUAT ?
			ORG_ActionSquatCleanShotPeepingRight : ORG_ActionCleanShotPeepingRight ;
	    }
	    work->rage-- ;
	}
	else
	{

	    /* 見ないで銃だけ出して攻撃アクション(めくら撃ち) */
	    if ( work->weap_blt >= 4 )
	    {
		if ( work->flag & ORGA_F_DANGR_STAND )
		    *list++ = ORG_ActionBlindFireUp ;
#if 0
		else if ( !work->stll_mtn && ( work->hide_spot ? work->hide_spot->id==1 : 0 ) )
		{
		    if ( work->flag & ORGA_F_DANGR_LEFT )
			*list++ = ORG_ActionBlindFireLeft ;
		    else
			*list++ = ORG_ActionBlindFireRight ;
		}
#endif
	    }

	    /* スネークの現在の位置が分からないので迷う */
	    //if ( work->flag & ORGA_F_AIMED_NEAR )
	    {
		int idx = (lvl>>2)*2 ;

		/*低い場所だったらどの覗き込みもやる可能性があって*/
		if ( work->flag & ORGA_F_DANGR_STAND )
		{
		    *list++ = ORG_ActListStandingFire[idx+0] ;
		    *list++ = ORG_ActListStandingFire[idx+1] ;
		    if ( work->flag & ORGA_F_DANGR_LEFT )
			*list++ = ORG_ActListPeepingFireLeft[idx+0],
			*list++ = ORG_ActListPeepingFireLeft[idx+1] ;
		    else if ( work->flag & ORGA_F_DANGR_RIGHT )
			*list++ = ORG_ActListPeepingFireRight[idx+0],
			*list++ = ORG_ActListPeepingFireRight[idx+1] ;
		    idx += 8 ;
		}
		else
		    idx += work->stll_mtn & ORGA_F_IS_SQUAT ? 8 : 0 ;
		if ( work->flag & ORGA_F_DANGR_LEFT )
		    *list++ = ORG_ActListPeepingFireLeft[idx+0],
		    *list++ = ORG_ActListPeepingFireLeft[idx+1] ;
		else if ( work->flag & ORGA_F_DANGR_RIGHT )
		    *list++ = ORG_ActListPeepingFireRight[idx+0],
		    *list++ = ORG_ActListPeepingFireRight[idx+1] ;
		//work->stll_tim = ORGA_STILL_THINKTIME+1 ;
	    }
	}
	*list = NULL ;

	/*攻撃モードの変化*/
	if ( work->stll_tim > ORGA_STILL_THINKTIME )
	{
	    work->stll_tim = 0 ;
	    /* ノーマル以上は,狙われている場合は,のぞき込まない */
	    if ( irnd() & 0x3 || work->scared > 0 )
		ChangeFlag( work, ORGA_F_NONE  , ORGA_F_OFFENCE ) ;
	    else if ( !work->rage && 
		 (lvl >= ORGA_F_LEVEL_NORM ? !(work->flag & ORGA_F_AIMED_NEAR) : irnd() & 0x3) )
		ChangeFlag( work, ORGA_F_MODIST, ORGA_F_NONE    ) ;
	    work->scared++ ;
	}
	work->aim_flg &= ~ORGA_F_DONT_SHOOT ;
	work->avoid_flg = 0 ;

	return 1 ;
    }
    work->aim_flg &= ~ORGA_F_DONT_SHOOT ;
    work->avoid_flg = 0 ;

    return  0 ;
}
