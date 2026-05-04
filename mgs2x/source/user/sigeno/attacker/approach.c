/*
	approach.c
	威嚇後、接近攻撃
	2000/06/16 K.Sigeno
	$Id: approach.c,v 1.1.1.3 2002/11/19 11:48:58 Yoshizawa1 Exp $
*/

/*

  威嚇後、接近攻撃

*/

static void Think3_Approach(ENETHINK *entk )
{
/******
2000.05.02 at_tmptimeを使い、
方向転換後の移動開始に少し間を取る
SetMoveMode()とat_tmptimeを使う関数に注意
****/
//	int reach,rank_dis = 1000,aim_check = 0;
//	int rank_dis = 1000,intr;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	SetAimPosPlayer(entk,0);

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
			if( at_thk->zone_dis > entk->at_com->chasedis+1000)
			{
				entk->act->pad = SP_READYGUN;
				entk->count3 =0;
				entk->think2 =TH2_CHASE;
				entk->think3 =TH3_WAIT_CHASE;
				entk->act->dir = entk->ctrl->turn.vy ;
				return ;
			}
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;
	at_thk->at_status &= (~AT_ST_SQUAT);

	if(entk->count3 ==0){
		/*初期化*/
		at_thk->at_tmptime = 0;
	}

	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}
/*プレイヤを目標に設定*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}else {
		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
	}
	/*目標が近けりゃ攻撃 */
	if(
	(at_thk->in_sight>(AT_THK_RATE*2))
	&&(HZX_ReachTo( GM_PlayerAddress, entk->ctrl->addr ) <= HZX_INDIRECT_REACH)
	){
#if 1
		GoReturnAttack(entk ,AT_APPROACH_COUNT,TH2_CHASE,TH3_ZONE_POS);
#else
		SetAimPosPlayer(entk,0);
		at_thk->th2_buf = TH2_APPROACH ;
		at_thk->th3_buf = TH3_ZONE_POS ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_ZONE_POS ;
		entk->count3 = 0 ;
		GoPosChase( entk , &at_thk->subtrg) ;
#endif
		return ;
	}
	if((!CheckWaitStatus(entk)
	||( SIG_CheckRIntrpt(entk) )
	)){
		/*通常移行*/
		entk->at_com->watch_status &= ~AT_COM_WATCH_APPROACH ;
#if 0
		if(ThinkSubNear( entk )){
			return;
		}
#else
		GoChaseWait(entk);
		return;
#endif

	}

	/*ゾーン追跡*/
	ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;
	SetMoveMode(entk);
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}

/*後方待機状態*/
/*何が合っても動かない*/
static void Think3_ApproachWait( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->pad = SP_READYGUN;

	if(!CheckWaitStatus(entk)){
		/*通常移行*/
#if 0
		if(ThinkSubNear( entk )){
			return;
		}
#else
		GoChaseWait(entk);
		return;
#endif

	}

	if(at_thk->in_sight>(AT_THK_RATE*2)) StillShoot( entk ,AT_SHT_NO_RELOAD) ;

	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}


/*攻撃状態簡易版 */
/*呼び出し前に tmptimeに終了時間を設定すること */
/*終了後の戻りthink設定必要*/
static void Think3_ReturnAttack( entk )
ENETHINK	*entk ;
{

	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	if( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ){
		SetAimPosPlayer(entk,0);
	}

	if(!CheckWaitStatus(entk)){
		/*通常移行*/
#if 0
		if(ThinkSubNear( entk )){
			return;
		}
#else
		GoChaseWait(entk);
		return;
#endif

	}

	/*終了時間チェック*/
	if(entk->count3 > at_thk->at_tmptime)
	{
		SetReturnThink(entk);
		SIG_SetChasePos( entk , &at_thk->subtrg) ;
		at_thk->th2_buf = -1 ;
		at_thk->th3_buf = -1 ;

//		GoPosChase( entk , &at_thk->subtrg) ;
		return;
	}
	/*基本攻撃姿勢*/
	at_thk->at_status &= ~AT_ST_SQUAT;
	entk->act->pad = SP_READYGUN;
	/*銃方向 リロード無し*/
	StillShoot( entk,(AT_SHT_NO_RELOAD|AT_SHT_BLIND|AT_SHT_NO_RAND));
	entk->bullet = 0 ;
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}


/*----- 中レベル思考モード --------------------------------------------*/

/*威嚇*/

static void	Think2_Approach( entk ) 
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_APPROACH :
			/*静止して攻撃*/
	    	Think3_Approach( entk ) ;
		break;
		case TH3_WAIT_CHASE :
			/*静止*/
			Think3_ApproachWait( entk );
		break;
		case TH3_ATTACK_EASY:
			Think3_ReturnAttack(entk);
		break;
	}
}

