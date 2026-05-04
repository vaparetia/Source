/*
	threat.c
	障害物をはさんでの打ち合い状態
	2000/06/12 K.Sigeno
	$Id: threat.c,v 1.1.1.3 2002/11/19 11:49:05 Yoshizawa1 Exp $
*/

/*
	角の向こうにいるのを察知
	威嚇を行う
*/

static void Think3_Threat(ENETHINK *entk )
{
	int fl_atr ;
//	SVECTOR rgb;
	FVECTOR		tmptrg,sub;
	AT_THK *at_thk;
//	R_INTRPT	*r_intrpt ;
	int dir ;
	at_thk = (AT_THK *) entk->character ;

#if 0
/*DEBUG TRAP*/
if(entk->count3 ==0){
printf("Threat next [%x]\n",entk->znavi->next_addr);
}
	entk->znavi->next_addr = HZX_NO_ZONE ;
#endif

#if 0
	{
		SVECTOR	rgb;
		tmptrg = entk->znavi->flore_pos ;
		tmptrg.vy += 50.0F ;
		rgb.vz = 127;
		PosBox(&tmptrg ,25.0F ,&rgb );
	}
#endif
	/*ドア対策 途中にドアがあれば止まらず侵攻*/
	if(((entk->count3%(AT_THK_RATE*10))==0)||(entk->at_com->Pl_StayTime ==0)){
		if(CheckThreatCondition(entk->ctrl->addr,entk->com->plzone_in_zone[0])){
			/*自動ドア発見*/
			entk->act->dir = entk->ctrl->turn.vy ;
			entk->act->pad = SP_READYGUN;
			SetBerserkTime(entk,(AT_THK_RATE*30)) ;
//			GoNearAttack(entk) ;
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return  ;
		}else {
		}
	}
	/****************************************/
	if((entk->count3% (AT_THK_RATE*10)) ==0){
#if 0
		if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
			at_thk->at_status &= ~AT_ST_LOW_SQUAT ;
			at_thk->at_status &= ~AT_ST_SQUAT ;
		}else 
#endif
		if(
		(GM_PlayerStatus & PLAYER_GROUND )){
			/*足元警戒*/
			tmptrg = entk->znavi->flore_pos ;
			tmptrg.vy += 50.0F ;
			if(!(HZX_OnlineHazardCheck( entk->ctrl->hzx_id,
			&tmptrg, &GM_PlayerFindPos,
		HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
		HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
		HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) ) {
				/*足元ピンチなので*/
//				if(entk->act->bodyp.type & (ENE_TYPE_SHOTGUN|ENE_TYPE_HITECH)){
				if(1){
					/*下段撃ちのできない人たち*/
					at_thk->at_status &= ~AT_ST_LOW_SQUAT ;
					at_thk->at_status &= ~AT_ST_SQUAT ;
					SetBerserkTime(entk,(AT_THK_RATE*30)) ;
				}else {
					at_thk->at_status |= AT_ST_SQUAT ;
					at_thk->at_status |= AT_ST_LOW_SQUAT ;
				}
			}else {
				at_thk->at_status &= ~AT_ST_LOW_SQUAT ;
				at_thk->at_status &= ~AT_ST_SQUAT ;
			}
		}else if(GM_PlayerStatus &(PLAYER_SQUAT|PLAYER_CB_BOX)){
			/*しゃがみ位置から視界チェック*/
			at_thk->at_status &= ~AT_ST_LOW_SQUAT ;
			tmptrg = entk->znavi->flore_pos ;
			tmptrg.vy += 750.0F ;
			if(!(HZX_OnlineHazardCheck( entk->ctrl->hzx_id,
			&tmptrg, &GM_PlayerFindPos,
			HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
			HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
			HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) ) {
				at_thk->at_status |= AT_ST_SQUAT ;
			}else {
				at_thk->at_status &= ~AT_ST_SQUAT ;
			}
		}else {
			at_thk->at_status &= ~AT_ST_LOW_SQUAT ;
			if(
			(ENE_AlertGameLevel >= AT_SQUAT_LEVEL )
			&&(!(entk->act->bodyp.type & ENE_TYPE_SHOTGUN))
			){
				if((irnd()>>8)%2){
					at_thk->at_status |= AT_ST_SQUAT ;
				}else {
					at_thk->at_status &= ~AT_ST_SQUAT ;
				}
			}else {
				at_thk->at_status &= ~AT_ST_SQUAT ;
			}
		}
	}
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		at_thk->at_status &= ~AT_ST_LOW_SQUAT ;
		at_thk->at_status &= ~AT_ST_SQUAT ;
	}
	entk->act->dir = entk->ctrl->turn.vy ;

	if( entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		/*ショットガンはたちっぱなし*/
		entk->act->pad = SP_READYGUN;
	}else if(entk->act->bodyp.type & ENE_TYPE_HITECH){
		/*ハイテク兵は伏せなし*/
		if( at_thk->at_status & (AT_ST_LOW_SQUAT|AT_ST_SQUAT) ){
			entk->act->pad = SP_SQUATGUN;
		}else {
			entk->act->pad = SP_READYGUN;
		}
	}else {
		if( at_thk->at_status & AT_ST_LOW_SQUAT ){
			entk->act->pad = SP_INTRUDE_FIRE ;
		}else if( at_thk->at_status & AT_ST_SQUAT ){
			entk->act->pad = SP_SQUATGUN;
		}else {
			entk->act->pad = SP_READYGUN;
		}
	}




#if 0
	if((at_thk->dis_rank != 0)&&
	(at_thk->sight_time < entk->at_com->shoot_delay)
	){
		if(entk->act->pad == SP_READYGUN){
//			if(SIG_CheckFrontSeg(entk->ctrl,GUN_HAZARD_DIS)){
			entk->act->pad = SP_WAIT ;
			entk->status2 &= ~ENE_STATUS2_AIM_GUN ;
		}
	}
#endif


	/*接近モード*/
	if(( at_thk->dis_rank>0)&&(CheckWaitStatus(entk)))
	{
		GoApproachWait( entk );
		return ;
	}


	/*ビヨンドモードに逃げられた*/
	if((entk->at_com->watch_status & AT_COM_WATCH_BEYOND)
	&&(GM_PlayerPosition.vy < (entk->ctrl->mov.vy-BEYOND_DIS))
	){
		/*見失ってない*/
		if((entk->at_com->alert_time%(AT_THK_RATE*3)==0)&&(at_thk->dis_rank==0)){
//			SIG_AT_VoiceCall(entk,SD_V_ATKO03,AT_V_NIGETAZO) ;
			entk->think2 = TH2_CHASE;
			entk->think3 = TH3_CHASE_BEYOND ;
			entk->count3 = 0 ;
			return ;
		}
	}

	/*フロア属性チェック*/
	fl_atr = CheckFlrAtr(entk);
	/*目標位置セット*/
	if(
	(entk->count3 == 0)
	||(entk->at_com->Pl_StayTime < (AT_THK_RATE*2))
	){
#if 1
		/*角での威嚇としゃがみ威嚇を区別する*/
		if(at_thk->at_status & AT_ST_FEEL_LOW){ 
			/*しゃがみ隠れに対する威嚇*/
			at_thk->subtrg = GM_PlayerPosition ;
			at_thk->subtrg.vy = GM_PlayerControl->levels[0] ;
		}else {
			/*威嚇目標を見えてるゾーン近くに設定*/
			if(CheckStandOnZone(entk,GM_PlayerAddress,&at_thk->subtrg)){
				CheckThreatPosZone(entk,&at_thk->subtrg) ;
			}
		}
#else
		/*威嚇目標を自分近くに設定*/
		if(CheckStandOnZone(entk,GM_PlayerAddress,&at_thk->subtrg)){
			CheckThreatPos( entk,&at_thk->subtrg) ;
		}
#endif

		if(GM_PlayerStatus &(PLAYER_SQUAT|PLAYER_GROUND|PLAYER_CB_BOX)){
			at_thk->subtrg.vy += 500.0F;
		}else {
			at_thk->subtrg.vy += 1500.0F;
		}

	}
	if(
	((at_thk->in_sight > 0)&&(!( at_thk->at_status & AT_ST_LOW_SQUAT )) )
	||(GM_PlayerStatus & PLAYER_LOCKER)
	){
		at_thk->subtrg = GM_PlayerFindPos ;
	}
	entk->act->aim_pos = tmptrg = at_thk->subtrg ;
	_sceVu0SubVector( &sub, &at_thk->subtrg,&entk->ctrl->mov );
	dir = GV_VecDir2( &sub );

	{
	/*手ぶれ*/
		int		range1,range2 ;
		float	trg_dis,shot_rnd = 1000.0F;

		trg_dis = GV_VecLen3F2( &tmptrg, &entk->ctrl->mov );
		shot_rnd *= (trg_dis/10000.0F);
		range1 = (int) shot_rnd ;
		range2 = (int) shot_rnd/2 ;
		tmptrg.vx += (float) ((BP_PS2_rand()%range1)-range2) ;
		tmptrg.vy += (float) ((BP_PS2_rand()%range1)-range2) ;
		tmptrg.vz += (float) ((BP_PS2_rand()%range1)-range2) ;
	}

	


	/* 目標が見えなくてもチーム内の誰かが捕捉中なら */
	/* あわてて追跡しない */
/*06.01*/
//	if(at_thk->dis_rank==0){
	if(at_thk->dis_rank <= 1){
/*前衛兵士*/
#if 0
		if(
		( at_thk->zone_dis > (entk->at_com->chasedis+1000))
		||(!(at_thk->at_status & AT_ST_FEEL))
		||(at_thk->in_sight > (AT_THK_RATE*5))
		){
/*プレイヤが出てきたら通常攻撃に移行している*/
			if(
			(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )
			&&(at_thk->zone_dis < (5000))
			){
				GoIntGrd(entk);
				return ;
			}
			GoNearAttack(entk);
		}
#else
		/*逃げたら追う*/
		/*出てきてもしばらくはthreat維持*/
		if(
//		(ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress)))&&
		(!(at_thk->at_status & AT_ST_FEEL))
		&&(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
#if 0
		&&
		(!
		((entk->at_com->watch_status & AT_COM_WATCH_COVER ))
		&&(HZX_ReachTo(entk->ctrl->addr,entk->at_com->cov_addr)<= HZX_DIRECT_REACH)
		)
#endif
		){
			if(((entk->at_com->watch_status & AT_COM_WATCH_COVER ))
			&&(HZX_ReachTo(entk->ctrl->addr,entk->at_com->cov_addr)<= HZX_DIRECT_REACH)){
			}else {
				if(ThinkSubNear( entk )){
					if(GM_AlertLevel!=ALERT_LEVEL_MAX){
						SIG_AT_VoiceCall(entk,SD_V_ATKO03,AT_V_NIGETAZO) ;
					}
					return;
				}
			}
		}
		if(((entk->at_com->watch_status & AT_COM_WATCH_COVER ))
		&&(HZX_ReachTo(entk->ctrl->addr,entk->at_com->cov_addr)
		<= HZX_DIRECT_REACH)){
		}else {
			if(at_thk->in_sight > (AT_THK_RATE*10))
			{
				GoNearAttack(entk);
			}
		}
#endif

	}else if(
	/*後衛*/
	(!(at_thk->at_status & AT_ST_FEEL))
	){
/*FEEL終了判定*/
		if(
		( (entk->at_com->com_sight <= entk->at_com->siege_num)
		&&(at_thk->dis_rank <= entk->at_com->siege_num) )
		||( at_thk->zone_dis > entk->at_com->chasedis+2000 ) 
		)
		{
//printf("THREAT LINE [%d]\n",__LINE__);
			if(ThinkSubNear( entk )){
//printf("THREAT LINE [%d]\n",__LINE__);
				if(entk->count3 >DIRECT_TICK(60)){
					SIG_AT_VoiceCall(entk,SD_V_ATKO03,AT_V_NIGETAZO) ;
				}
//printf("THREAT LINE [%d]\n",__LINE__);
				return;
			}
		}
	}


	/*射撃目標を向かせる*/
	entk->act->dir = dir;
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;


	/*膠着状態が長いので*/
	if(
	( entk->at_com->Pl_StayTime >= AT_GRD_COUNT )
	&&(entk->count3 > (AT_THK_RATE*20))
	&&(at_thk->dis_rank == 0)
	&&( entk->bullet >= entk->max_bullet )
	){
		if(
		(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
		&&(entk->act->keep_pad != SP_READYGUN) 
		){
			/*ショットガン兵はタイミングが特種*/
//printf("THREAT LINE [%d]\n",__LINE__);
		}else {
			if(
//			(entk->act->bodyp.type & ENE_TYPE_SHIELD)
			0
/*盾兵にグレネード。*/
			)
			{
				/*プレイヤを目標に設定*/
				if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
					ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
				}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
					ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
				}
				if(SIG_CheckRIntrpt(entk)){
					/*接近不能なら*/
					/*グレネード*/
					entk->bullet = 0 ;
//printf("THREAT LINE [%d]\n",__LINE__);
					GoGrdHigh(entk);
				}else {
					/*接近可能なら*/
					/*突撃*/
//printf("THREAT LINE [%d]\n",__LINE__);
					SetBerserkTime(entk,(AT_THK_RATE*30)) ;
				}
			}else {
			/*グレネード*/
				entk->bullet = 0 ;
				GoGrdHigh(entk);
//printf("THREAT LINE [%d]\n",__LINE__);
			}
//printf("THREAT LINE [%d]\n",__LINE__);
			return ;
		}
	}
	if( entk->bullet >= entk->max_bullet )
	{
//printf("THREAT LINE [%d]\n",__LINE__);
		if(
		(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
		&&(entk->act->keep_pad != SP_READYGUN) 
		){
		} else {
			if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
				GoGrdHigh(entk);
				return ;
			}
			/*その場でリロード*/
			GoReload( entk );
//printf("THREAT LINE [%d]\n",__LINE__);
			return ;
		}
	}

	/*威嚇射撃*/
	if ( entk->act->act_end ) {
		entk->act->pad = SP_READYGUN;
		entk->count3 = 0 ;
		return ;
	}
#if 1
//	if(at_thk->in_sight > entk->at_com->shoot_delay)
	if(entk->count3 > entk->at_com->shoot_delay){
		/*ショットガンは照準ぶらし無し*/
		if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
			/*ショットガンは連射不可*/
			if(entk->act->keep_pad != SP_SHOOTGUN) {
				StillShoot(entk,AT_SHT_BLIND|AT_SHT_NO_RELOAD); 
//printf("THREAT LINE [%d]\n",__LINE__);
			}
		}else {
			if( at_thk->at_status & AT_ST_LOW_SQUAT ){
#if 1
				RandShoot(entk );
#else
/*2001.08.15*/
				ThreatShot(entk,&tmptrg);
#endif
			}else {
				ThreatShot(entk,&tmptrg);
			}
		}
	}else {
	}

	/*ビックリすることが起きた*/
	if(CheckDeathBed(entk) ){
		return ;
	}
#endif

	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/

/*威嚇*/

static void	Think2_Threat( entk ) 
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_THREAT :
			Think3_Threat(entk) ;
		break;
	}
}

