/*
	rescue.c
	行動不能者を救助に向かう
	2000/10/01 K.Sigeno
	$Id: rescue.c,v 1.1.1.3 2002/11/19 11:49:04 Yoshizawa1 Exp $
*/

static void Think3_ZoneChaseRescue( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
	entk->act->dir = entk->trgpoint.dir ;
	entk->act->pad = SP_MOVE_RUN ;
	at_thk->at_status &= (~AT_ST_SQUAT);
/*ゾーン追跡*/
/*ゾーン追跡＆終了判定*/
	if(ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),entk->ctrl, entk->count3 ))
	{
		if(entk->count3 == 0 ) entk->act->pad = SP_READYGUN ;
		/*ゾーン内の座標に近づく*/
		entk->think3 = TH3_DIRECT_POS ;
		entk->count3 = 0 ;
		return ;
	}

	/*通常歩き*/
	entk->act->move_s = MoveRun ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseRescue( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->dir = entk->trgpoint.dir ;
	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
	}
#endif

	if ( ENE_DirectTrace( &(entk->trgpoint), 
//		&(entk->znavi->flore_pos), 50 ) < 0 ) {
		&(entk->znavi->flore_pos), 250 ) < 0 ) {
		/*到着後*/
		/*目標座標に移動*/
#if 0
		entk->act->mot_speed_correct = 0.0F;
		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;
#endif
		/*目標修正*/
		entk->act->pad = SP_READYGUN ;
#if 1
		entk->think3 = TH3_WAKE ; 
#else
		if(at_thk->th2_buf >= 0){
			SetReturnThink( entk ) ;
		}else {
			entk->think2 = TH2_CHASE ; 
			entk->think3 = TH3_WAIT_CHASE ; 
		}
#endif
		entk->count3 = 0 ;
		return ;
	}
	entk->act->move_s = MoveRun ;
	entk->act->tmp_dir = entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void	Think3_Wake(entk)
ENETHINK	*entk ;
{
//	FVECTOR	aimpos,e_pos ;
//	float	trglen ;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->dir = entk->ctrl->turn.vy ;
	if(entk->count3 == 0){
printf("SP_STOMP_WAKE CALED !!!!!!!!\n");
		entk->act->pad = SP_STOMP_WAKE ;
		entk->act->the_target = &entk->buddy->act->bodyp.deftrg ;
		at_thk->at_tmptime = 0;
	}

	if(entk->act->act_end==1){
		/*踏み終わり時*/
printf("SP_STOMP_WAKE ACT END !!!!!!!!\n");
#if 1

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_WAIT_CHASE ;
		entk->count3 = 0;
#endif
		return ;
	}
	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/

/*** TH3_ZONE_POS, TH3_DIRECT_POS ***/
static void	Think2_Rescue( entk ) 
ENETHINK	*entk ;
{
extern int CheckAnesAT(ENETHINK *) ;

	if(entk->buddy != NULL ){
		if(!CheckAnesAT(entk->buddy)){
			/*寝てる人になにかあった*/
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		ENE_SetTrgpEyei(&(entk->npc_eyei),&(entk->trgpoint));
	}
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_WAIT_CHASE ;
			entk->count3 = 0;
			return ;
		}
	}
	switch ( entk->think3 ) {

	    case TH3_ZONE_POS :
			/*静止して攻撃*/
			Think3_ZoneChaseRescue( entk ) ;
		break;
		case TH3_DIRECT_POS :
			/*静止*/
			Think3_DirectChaseRescue( entk ) ;
		break;
		case TH3_WAKE :
			Think3_Wake(entk);
		break;
	}
}

