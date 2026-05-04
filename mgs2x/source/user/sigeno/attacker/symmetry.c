/*
	symmetry.c
	指定ポイントの対象位置で待機
	2000/07/26 K.Sigeno
	$Id: symmetry.c,v 1.1.1.3 2002/11/19 11:49:05 Yoshizawa1 Exp $
*/
static void SetPosChase(ENETHINK *entk )
{
	CONTROL	ctrl;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	/*目標地点セット*/
#ifdef BP_PS2
	(FVECTOR) entk->trgpoint.pos = entk->at_com->side_pos[entk->id] ;
	(FVECTOR) ctrl.mov =  entk->at_com->side_pos[entk->id] ;
#else
	entk->trgpoint.pos = entk->at_com->side_pos[entk->id] ;
	ctrl.mov =  entk->at_com->side_pos[entk->id] ;
#endif
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.addr = ctrl.addr ;
	entk->trgpoint.map = ctrl.map ;

}
static void GoSymPosChase(ENETHINK *entk ){
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

//	GoPosChase(entk ,&entk->at_com->cov_pos) ;
	GoPosChase(entk ,&entk->at_com->side_pos[entk->id]) ;
	entk->think2 = TH2_SYMMETRY ; 
#if 0
	at_thk->th2_buf = TH2_SYMMETRY ; 
	at_thk->th3_buf = TH3_SYM_ATTACK ;
#else
	/*到着後を通常攻撃に変更*/
	at_thk->th2_buf = TH2_THREAT ; 
	at_thk->th3_buf = TH3_THREAT ;
#endif
}


static void CheckSymmetry(ENETHINK *entk ){

	AT_THK		*at_thk ;
	at_thk = (AT_THK *) entk->character ;

	if(entk->id >1 ) return ;
	if(
//	(entk->at_com->watch_status & AT_COM_WATCH_COVER )
//	&&(entk->think2 !=TH2_SYMMETRY )
#if 0
	&&(entk->think2 == TH2_CHASE)
	&&(entk->think3 ==TH3_ZONE_CHASE)
#else
//	(at_thk->at_status & AT_ST_IN_COVER)
	(entk->at_com->watch_status & AT_COM_WATCH_COVER )
	&&(entk->think2 == TH2_ATTACK)
	&&(entk->think3 ==TH3_ATTACK_NEAR)
//	&&( entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT_HZD)
	&&( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN)
	&&(entk->ctrl->map == GM_PlayerMap )
#endif
	){
		if(GV_VecLen3F2( &entk->trgpoint.pos,
		&entk->at_com->side_pos[entk->id] ) > 1000.0F ){
			GoSymPosChase( entk );
		}
	}

	if(
	(!(entk->at_com->watch_status & AT_COM_WATCH_COVER ))
	&&(entk->think2 ==TH2_SYMMETRY )
	){
		/*SYMMETRY 強制終了*/
		GoChaseWait( entk ) ;
	}
}
static void Think3_SymAttack(ENETHINK *entk )
{
//	float dis;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;
	
	if(entk->count3 ==0){
#if 0
		if(GM_PlayerStatus &(PLAYER_SQUAT|PLAYER_CB_BOX)){
			at_thk->at_status |= AT_ST_SQUAT ;
		}else {
			at_thk->at_status &= ~AT_ST_SQUAT ;
		}
#else
		if((irnd()>>8)&1){
			at_thk->at_status |= AT_ST_SQUAT ;
		}else {
			at_thk->at_status &= ~AT_ST_SQUAT ;
		}
#endif
	}else if
	(
//	(entk->bullet == 5 )
	((entk->count3%((AT_THK_RATE*10)*2)) == 0 )
	&&((irnd()>>8)&1)
	){
		if(at_thk->at_status & AT_ST_SQUAT){
			at_thk->at_status &= ~AT_ST_SQUAT ;
		}else {
			at_thk->at_status |= AT_ST_SQUAT ;
		}
	}
	if(at_thk->at_status & AT_ST_SQUAT){
		entk->act->pad = SP_SQUATGUN;
	}else {
		entk->act->pad = SP_READYGUN ;
	}
	
	if(GV_VecLen3F2( &entk->znavi->flore_pos,
		&entk->at_com->side_pos[entk->id] ) > 1000.0F ){
		GoSymPosChase( entk );
		return ;
	}
	if(!(entk->at_com->watch_status & AT_COM_WATCH_COVER )){
		/*通常復帰*/
		GoNearAttack(entk);
		return ;
	}
	if( entk->bullet >= entk->max_bullet ){
		entk->bullet = 0 ;
		at_thk->at_status |= AT_ST_SQUAT ;
		entk->act->pad = SP_SQUATGUN;
		entk->think3 = TH3_SYM_RELOAD ;
		return ;
	}

	SymmRandShoot( entk );
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}
static	void	Think3_SymmReload( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

#if 0
	/*追跡が手薄なのでリロード中断*/
	if(
	&&( at_thk->zone_dis > (entk->at_com->chasedis+2000) ))
	)
	{
		entk->act->act_end = 1;
	}
#endif	
	/*しゃがみ状態*/
	entk->act->pad = SP_RELOAD_SQUAT ;

	if ( entk->act->act_end ) {
		/*しゃがんでいても見えるならしゃがみ攻撃*/
		if(
		( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
		){
			at_thk->at_status |= AT_ST_SQUAT ;
//			entk->act->pad = SP_SQUATGUN;
		}else {
			at_thk->at_status &= ~AT_ST_SQUAT ;
//			entk->act->pad = SP_SQUATGUN;
		}

		entk->think2 = TH2_SYMMETRY ; 
		entk->think3 = TH3_SYM_ATTACK ; 
		entk->count3 = 0 ;
//	GoNearAttack(entk);
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;
	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/

static void	Think2_Symmetry( entk ) 
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_ZONE_POS :
			if((entk->count3 % (AT_THK_RATE*10)) == 0) SetPosChase(entk);
			Think3_ZoneChasePos( entk ) ;
		break;
		case TH3_DIRECT_POS :
			if((entk->count3 % (AT_THK_RATE*10)) == 0) SetPosChase(entk);
			Think3_DirectChasePos( entk ) ;
		break;
		case TH3_SYM_ATTACK :
			/*静止して攻撃*/
			Think3_SymAttack( entk ) ;
		break;
		case TH3_SYM_RELOAD :
			/*静止して攻撃*/
			Think3_SymmReload( entk ) ;
		break;

	}
}

