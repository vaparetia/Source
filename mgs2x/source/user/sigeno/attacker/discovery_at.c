/***
	discovery_at.c
	何かおきてびっくり

	2000/06/13 K.Sigeno
	$Id: discovery_at.c,v 1.1.1.3 2002/11/19 11:49:03 Yoshizawa1 Exp $
***/
/*首絞め仲間にビックリ*/
static void Think3_Discovery_Hang( entk ) 
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;

	if(entk->count3 == 0){
//		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		entk->act->pad = SP_BIKKURI ;
	}

#if 0
//hang
	/*中断処理*/
	if((entk->count3 > (AT_THK_RATE*5))&&( entk->at_com->com_sight == 0)){
		entk->act->act_end = 1;
	}
#endif
	if (entk->act->act_end) {
		if(
		( GM_PlayerStatus & PLAYER_ENEMY_HANG)
		&&(!(entk->act->bodyp.type & ENE_TYPE_SHIELD))

		&&
		(entk->at_com->com_sight> 1)
		&&
		(
		(at_thk->dis_rank == 0)
		||
		((at_thk->front_st & ACT_STATUS_CAPTURE)
		&&(at_thk->dis_rank == 1))
		)
		){
/*ストップサイン*/
			GoStopSign(entk);
		}else {
/*通常攻撃*/
			GoNearAttack(entk);
		}
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++;
}

static void Think3_Discovery_Death( entk ) 
ENETHINK	*entk ;
{
	FVECTOR sub ;
	int dir ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;

	if(entk->count3 == 0){
//printf("BIKKURI!!!!!!\n");
		entk->act->pad = SP_BIKKURI ;
	}
#if 0
//hang
	/*中断処理*/
	if((entk->count3 > (AT_THK_RATE*5))&&( entk->at_com->com_sight == 0)){
		entk->act->act_end = 1;
	}
#endif
	if (entk->act->act_end) {
		if(ENE_AlertGameLevel >= AT_ESC_LEVEL){
			/*逃げレベル*/
//			GoDanger(entk); /*安地優先*/
			GoEscape(entk); /*後方へ逃げる*/
		}else {
			/*ひるまない*/
			GoNearAttack(entk);
		}
		entk->count3 = 0 ;
		return ;
	}

	_sceVu0SubVector( &sub, &entk->at_com->tmptrg, &entk->ctrl->mov ) ;
	dir = GV_VecDir2( &sub );

	entk->act->dir = dir ;
	entk->count3 ++;
}

static void Think3_Abs_Escape( entk ) 
ENETHINK	*entk ;
{
//	FVECTOR sub ;
//	int dir ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;

	entk->at_com->surprised = 240 ;
	at_thk->surprised = 240 ;

	if(entk->count3 == 0){
		entk->act->pad = SP_BIKKURI ;
	}
	if (entk->act->act_end) {
		at_thk->escaddr = GetFarZone(GM_PlayerAddress ,10,entk);
		entk->think2 = TH2_CHASE;
		entk->think3 = TH3_ESC_ZONECHASE;
		at_thk->at_status |= AT_ST_ABS_ESCAPE ;
		entk->count3 = 0 ;
		return ;
	}

#if 0
/*体の向き　あとで考えよう*/
	_sceVu0SubVector( &sub, &entk->at_com->tmptrg, &entk->ctrl->mov ) ;
	dir = GV_VecDir2( &sub );
	entk->act->dir = dir ;
#endif

	entk->count3 ++;
}

static void Think3_Discovery_Player( entk ) 
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->pad = SP_DISCOVERY ;
	if (entk->act->act_end) {
		GoNearAttack(entk);
		entk->act->pad = SP_READYGUN;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++;
}

/*スティンガー発射目撃*/
static void Think3_Discovery_Stinger( entk ) 
ENETHINK	*entk ;
{
//	FVECTOR sub ;
//	int dir ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;
	if(entk->count3 == 0){
		entk->at_com->surprised = DIRECT_TICK(180) ;
//		entk->act->pad = SP_BIKKURI ;
		entk->act->pad = SP_BIKKURI_HIDE ;
	}
	if (entk->act->act_end) {
		/*何を見て隠れていたかにより行動分岐*/
		switch(at_thk->th3_buf){
			case AT_FOUND_STINGER :
				GoEasyAttackLie(entk,DIRECT_TICK(60));
			break ;
			case AT_FOUND_NIKITA :
//				GoEasyAttackLie(entk,DIRECT_TICK(120));
				GoDogeNikita(entk);
			break ;
			default :
				GoEasyAttackLie(entk,DIRECT_TICK(30));
			break ;
		}
		return ;
	}

#if 0
/*体の向き　あとで考えよう*/
	_sceVu0SubVector( &sub, &entk->at_com->tmptrg, &entk->ctrl->mov ) ;
	dir = GV_VecDir2( &sub );
	entk->act->dir = dir ;
#else
	entk->act->dir = entk->pl_eyei.dir ;
#endif
	entk->count3 ++;
}



/*ストップサインから通常攻撃へ*/
static void Think3_StopSign( entk ) 
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);

	entk->act->dir = entk->ctrl->turn.vy ;
	if(entk->count3 == 0){
		entk->act->pad = SP_STOPSIGN ;
		if(entk->com->enemys.group[entk->g_id]->
			unit[entk->u_id]->enemy_num>0){
			COM_SetSpeak( EV_STOP_1, entk );
		}
	}
	if(
	(entk->act->act_end) 
	){
		/*通常攻撃に*/
		GoNearAttack(entk);
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++;
}

static	void	Think2_Discovery( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_DISCOVERY_HANG : /*首絞め発見*/
			Think3_Discovery_Hang( entk ) ;
		break;
		case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
			break ;
		case TH3_DISCOVERY_DEATH :
			Think3_Discovery_Death( entk ) ;
			break ;
		case TH3_ABS_ESCAPE :
			Think3_Abs_Escape( entk ) ;
			break ;
		case TH3_DISCOVERY_STINGER :
			Think3_Discovery_Stinger( entk ) ; 
			break ;
		case TH3_FOUND_PL :
			Think3_Discovery_Player( entk ) ; 
			break ;
		default :
			printf("TH2 DISCOVERY ERR %d\n",entk->id);
			Think3_Discovery_Hang( entk ) ;
	}
}
