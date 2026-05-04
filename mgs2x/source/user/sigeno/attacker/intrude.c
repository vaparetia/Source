/*
	intrude.c
	イントルード時の対処
	2000/06/26 K.Sigeno
	$Id: intrude.c,v 1.1.1.3 2002/11/19 11:49:03 Yoshizawa1 Exp $
*/



static void GoIntrGrd(ENETHINK *entk){
		entk->think2 = TH2_INTRUDE ;
		entk->think3 = TH3_ATTACK_GRD ;
		entk->count3 = 0 ;
}

/*
認識時と見失い時両方行う。
*/

/*指定ポイントまで移動*/


/*76*/
static void Think3_IntrudeZoneChase( entk )
ENETHINK	*entk ;
{
extern void SigZoneView(int ,SVECTOR *, float) ; 

//	SVECTOR rgb ;
	AT_THK *at_thk;
//	int addr ;
	at_thk = (AT_THK *) entk->character ;


	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;

	if(entk->count3 == 0){
#if 0
		GetNearPosInZone( entk->com->plmap_in_zone[0],HZX_Zone1(entk->com->plzone_in_zone[0]),&GM_PlayerPosition,&at_thk->subtrg ) ;
		SIG_SetChasePos(entk ,&at_thk->subtrg ) ;
#else 
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#endif
	}

/*ゾーン追跡*/
/*ゾーン追跡＆終了判定*/
/**
printf("NEXT ADDR [%d] FLAG {%x] HZX_ID[%x]\n"
,HZX_Zone1(entk->znavi->next_addr)
,HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(entk->znavi->next_addr))->flag
,entk->ctrl->hzx_id
) ;
printf("PL ADDR [%d] FLAG {%x] HZX_ID[%x]\n"
,HZX_Zone1(GM_PlayerAddress)
,HZX_GetZone(GM_PlayerControl->hzx_id,HZX_Zone1(GM_PlayerAddress))->flag
GM_PlayerControl->hzx_id
) ;
****/
//	if(HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(entk->znavi->next_addr))->flag & HZX_ZONE_ZINTRPT){
	if(ENE_HZX_GetZone(entk->znavi->next_addr)->flag & HZX_ZONE_ZINTRPT){
		/**/
		GetNearPosInZone( entk->ctrl->hzx_id,
			HZX_Zone1(entk->ctrl->addr),
			&GM_PlayerPosition,&at_thk->subtrg ) ;
		SIG_SetChasePos(entk ,&at_thk->subtrg ) ;
		entk->think2 = TH2_INTRUDE ; 
		entk->think3 = TH3_DIRECT_POS ;
		entk->count3 = 0 ;
		return ;
	}

	if(
	(ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),entk->ctrl, entk->count3 )<0)
	||
	(ENE_HZX_GetZone(entk->znavi->next_addr)->flag & HZX_ZONE_ZINTRPT)
	){
		/*ゾーン内の座標に近づく*/
		entk->think2 = TH2_INTRUDE ; 
		entk->think3 = TH3_DIRECT_POS ;
		entk->count3 = 0 ;
		return ;
	}

	if(entk->count3%(AT_THK_RATE*3)==0){
		SetMoveMode(entk);
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
/*56座標追跡 ゾーン内座標*/
static	void	Think3_IntrudeDirectChase( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
//printf("Think3_IntrudeDirectChase\n");
	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,25.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,(float) entk->ctrl->r_sphere ,&rgb );
	}
#endif
//	entk->trgpoint.pos = entk->znavi->flore_pos ;

	if(entk->count3 == 0){
		GetNearPosInZone( entk->ctrl->hzx_id,HZX_Zone1(entk->ctrl->addr),&GM_PlayerPosition,&at_thk->subtrg ) ;
		SIG_SetChasePos(entk ,&at_thk->subtrg ) ;
	}

	/*r_sphere 以内に入れば到着*/
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos),
//	500 ) < 0 ) {
	(entk->ctrl->r_sphere+100) ) < 0 ) {
		/*到着後*/
		/*目標修正*/
/**/
		entk->act->mot_speed_correct = 0.0F;
//		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
//		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;

		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		entk->think2 = TH2_INTRUDE ;
		if(
		(entk->act->bodyp.type & ENE_TYPE_SHIELD) 
		||(entk->act->bodyp.type & ENE_TYPE_SHOTGUN) 
		||(entk->act->bodyp.type & ENE_TYPE_TNG_A) 
		){
			entk->think3 = TH3_ATTACK_GRD ;
//printf("DIRECT 2 TH3_ATTACK_GRD\n") ;
		}else {
			/*伏せ撃ち可能なのはマシンガン系装備者*/
//printf("DIRECT 2 ATTACK NEAR\n") ;
			entk->think3 = TH3_ATTACK_NEAR ;
		}
		entk->count3 = 0 ;
		return ;
	}

#if 0
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD) entk->act->pad = SP_MOVE_RUN ;
#endif


	SetMoveMode(entk);
	if( entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
	}
//	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_GrdAttack(entk)
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	if(entk->count3 == 10){
		FVECTOR tmptrg ;
		tmptrg = entk->znavi->flore_pos ;
		tmptrg.vy += 50.0F ;
		if((HZX_OnlineHazardCheck( entk->ctrl->hzx_id,
			&tmptrg, &GM_PlayerFindPos,HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
		HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) ) {
			entk->at_com->watch_status &= ~AT_COM_WATCH_INTRUDE ;
			entk->act->pad = SP_READYGUN;
//printf("GrdAttack2 chasewait\n");
			GoChaseWait(entk);
			return ;
		}
	}


	if(entk->count3 == 20){
//		if(entk->com->enemys.group[entk->g_id]->
//			unit[entk->u_id]->enemy_num>0){
//			COM_SetSpeak( EV_GRENADE_1, entk ) ;
//			COM_SetSpeak( EV_EAT_1, entk ) ;
//		}
		entk->act->pad = SP_GRD_LOW;
	}
	if(
	(entk->act->act_end)
//	||(!(GM_PlayerStatus & PLAYER_INTRUDE))
	||(!(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE))
	){
		if
//		(GM_PlayerStatus & PLAYER_INTRUDE)
		(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)
		{
			if(at_thk->dis_rank==0){
				/*移動不要*/
				if(
				(Sig_GetRoute(entk->ctrl->hzx_id,entk->ctrl->addr,GM_PlayerAddress) <= 1)
//				&&(entk->act->bodyp.type == ENE_TYPE_ATTACKER)
				)
				{
					if(
					(entk->act->bodyp.type & ENE_TYPE_SHIELD) 
					||(entk->act->bodyp.type & ENE_TYPE_SHOTGUN) 
					||(entk->act->bodyp.type & ENE_TYPE_TNG_A) 
					){
//printf("GRD 2 GRD!!!!\n");
						/*盾とショットガンはグレネード*/
						GoIntrGrd(entk);
					}else {
						/*低姿勢射撃*/
//printf("GO LOW SHOT!!!!\n");
						GoLowShot(entk);
						entk->bullet = 0 ;
						entk->count3 = 0 ;
					}
				}else {
//printf("GO ZONE CHASE!!!!\n");
					GoIntZoneChase(entk);
				}
			}else {
//printf("NOW NOT NO1!!!!GoIntWait\n");
				GoIntWait(entk);
			}
		}else {
		/*イントルードモード終了*/
//printf("GrdENd\n");
			entk->act->pad = SP_READYGUN;
			GoChaseWait(entk);
		}
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir;
	entk->count3++;
}

#if 1
/*25*/
static void Think3_LowShot(entk)
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status |= AT_ST_SQUAT ;


	if(entk->count3 ==10){
		FVECTOR tmptrg ;
		tmptrg = entk->znavi->flore_pos ;
		tmptrg.vy += 50.0F ;
		if((HZX_OnlineHazardCheck( entk->ctrl->hzx_id,
			&tmptrg, &GM_PlayerFindPos,HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
			HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) ) {
			entk->at_com->watch_status &= ~AT_COM_WATCH_INTRUDE ;
			entk->act->pad = SP_READYGUN;
			GoChaseWait(entk);
			return ;
		}
	}

	entk->act->pad = SP_INTRUDE_FIRE ;
	if( entk->bullet >= entk->max_bullet ){
		/*ATACK NEAR*/
		/*その場でリロード*/
		entk->bullet = 0 ;
		entk->think3 = TH3_ATTACK_RELOAD ;
		entk->count3 = 0 ;
		return ;
	}
//	printf("in sight is %d\n",at_thk->in_sight);
//	ThreatShot( entk , &GM_PlayerPosition ) ;
	SIG_BasicShoot( entk ,&GM_PlayerPosition ,0);

	/*イントルードモード終了*/
//	if(!(GM_PlayerStatus & PLAYER_INTRUDE)){
	if(!(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)){

		entk->act->pad = SP_READYGUN;
		GoChaseWait(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir;
	entk->count3++;
}
#endif

static void Think3_Wait( entk )
ENETHINK	*entk ;
{
	int near[6],zone_num ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	entk->act->pad = SP_READYGUN;
	entk->act->dir = entk->pl_eyei.dir ;
	if(
	(at_thk->dis_rank > 0)
//	&&(HZX_Zone1(entk->com->plzone_in_zone[0])
//		== HZX_Zone1(entk->ctrl->addr) )
	&&(Sig_GetRoute(entk->ctrl->hzx_id,entk->ctrl->addr,GM_PlayerAddress) <= 1)
	){
		/*投げる人のジャマにならないように下がる*/
		entk->think2 = TH2_CHASE;
//		at_thk->escaddr = HZX_NO_ZONE ;
		/*となりのゾーン判定*/
		zone_num = HZX_NearZones( entk->ctrl->hzx_id,
			HZX_Zone1(entk->ctrl->addr), near );
		at_thk->escaddr = 
			MapAddrSet(HZX_Zone1(near[((irnd()>>8)%zone_num)]),entk->ctrl->addr) ;
		entk->think3 = TH3_ESC_ZONECHASE;
		entk->count3 = 0 ;
//printf("Think3_Wait 2 Esc\n");
		return ;
	}else {
		/*前の人が遠ければ追いかける*/
		if( at_thk->dis_dif >= 3000){
			entk->think2 = TH2_CHASE;
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0;
			return ;
		}
	}
	entk->count3 ++ ;
}
static void Think3_GrdReload(entk)
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status |= AT_ST_SQUAT ;

	entk->act->pad = SP_RELOAD_SQUAT ;
//printf("Think3_GrdReload!!!\n");

	if(entk->count3 == 0){
	}
	if(
	(entk->act->act_end)
	){
//printf("GRD ACT END!!!!\n");
		if
//		(GM_PlayerStatus & PLAYER_INTRUDE)
		(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)
		{
			if(at_thk->dis_rank==0){
				/*移動不要*/
				if(
					Sig_GetRoute(entk->ctrl->hzx_id,entk->ctrl->addr,GM_PlayerAddress) <= 1
				)
				{
//printf("GO GRD ATTACK!!!!\n");
					entk->think3 = TH3_ATTACK_GRD ;
					entk->act->pad = SP_GRD_LOW;
					entk->count3 = 0 ;
				}else {
//printf("GO ZONE CHASE!!!!\n");
					GoIntZoneChase(entk);
				}
			}else {
//printf("NOW NOT NO1!!!!\n");
				GoIntWait(entk);
			}
		}else {
		/*イントルードモード終了*/
//printf("GrdENd\n");
			entk->act->pad = SP_READYGUN;
			GoChaseWait(entk);
		}
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir;
	entk->count3++;
}


/*----- 中レベル思考モード --------------------------------------------*/

/*威嚇*/

static void	Think2_Intrude( entk ) 
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	entk->iknow_flag &= ~IKNOW_HIDDEN_PLAYER ;

	switch ( entk->think3 ) {
	    case TH3_WAIT :
			Think3_Wait( entk ) ;
			if(
			(at_thk->dis_rank==0)
//			&&(GM_PlayerStatus & PLAYER_INTRUDE)
			&&(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)
			){
//printf("INTRUPT THINK LINE [%d]\n",__LINE__);
				GoIntZoneChase(entk);
			}
			if(!(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )){
//printf("INTRUPT THINK LINE [%d]\n",__LINE__);
				GoChaseWait(entk);
			}
		break;
		case TH3_INTRUDE_ZONE_POS :
			Think3_IntrudeZoneChase( entk ) ;
			if(!(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )){
//printf("INTRUPT THINK LINE [%d]\n",__LINE__);
				GoChaseWait(entk);
			}
		break;
		case TH3_DIRECT_POS :
			Think3_IntrudeDirectChase( entk ) ;
			if(!(entk->at_com->watch_status & AT_COM_WATCH_INTRUDE )){
//printf("INTRUPT THINK LINE [%d]\n",__LINE__);
				GoChaseWait(entk);
			}
		break;
		case TH3_ATTACK_GRD :
			Think3_GrdAttack(entk);
		break;
		case TH3_ATTACK_NEAR :
			Think3_LowShot(entk);
		break;
		case TH3_ATTACK_RELOAD :
			Think3_GrdReload(entk);
		break;
		default :
			printf("UNKNOWN INTRUDE THINK [%d][%d]!!!!!!\n",entk->think2,entk->think3);
		break ;
	}
}

