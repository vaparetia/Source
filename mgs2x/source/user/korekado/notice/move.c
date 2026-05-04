/*
	move.c
	notice/移動関係
	
	2000/10/7 Y.Korekado
	$Id: move.c,v 1.1.1.3 2002/11/19 11:44:19 Yoshizawa1 Exp $
*/

/*
必要パラメータ 
pad:
SP_PINPOINT
SP_LOCKER_OPEN

think3:
TH3_ZONE_MOVE
TH3_NEAR_MOVE
TH3_PINPOINT_MOVE
TH3_LOCKER_OPEN

tmp_buff[0] = 目標にどこまで近づくか
tmp_buff[1] = 1 ならダイレクトムーブまで行う

#define USE_DIRECT_MOVE	(1)
で
static	void	SetThinkMove( ENETHINK *entk, ENETHINKPROC proc, int dis )
static void Think3_MoveDirect( entk )
使用する
*/

/*-----  --------------------------------------------*/
static	void	SetThinkMove( ENETHINK *entk, ENETHINKPROC proc, int dis )
{
//	entk->thk_status |= THK_STATUS_MOVE ;
	entk->think3 = TH3_ZONE_MOVE ; 
	entk->tmp_proc[0] = proc ;
	
	if ( dis < 0 ) dis = 350 ;
	entk->tmp_buff[0] = dis ;
	entk->tmp_buff[1] = -1 ;
}
#ifdef USE_DIRECT_MOVE
static	void	SetThinkDirectMove( ENETHINK *entk, ENETHINKPROC proc, int dis, int directthink )
{
	SetThinkMove( entk, proc, dis ) ;
	entk->tmp_buff[1] = directthink ;
}
#endif

/*-----  --------------------------------------------*/
static	void	Think3_ZoneMove( entk )
ENETHINK	*entk ;
{
	int intrpt ;

	if ( (GM_AlertMode == ALERT_MODE_AVOID) && (entk->act->move_s == MoveCautionWalk) ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
#if 0
		int x ;
		x = ENE_GetGRot( entk->act->ctrl, 1000.0f ) ;
		/* 床の角度にあわせて顔をあげる */
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
		
#endif
	}

//	if( ENE_InRangeOnline( &entk->ctrl->mov, &(entk->trgpoint.pos), 
//											entk->tmp_buff[0], entk->ctrl->hzx_id ) ) {
	if( ENE_InRange( &entk->ctrl->mov, &(entk->trgpoint.pos), 
											entk->tmp_buff[0] ) ) {
		entk->tmp_proc[0]( entk ) ;
		entk->count3 = 0 ;

		return ;
	}

	intrpt = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ;
//printf("intrpt = [%d]\n", intrpt ) ;

	if (  intrpt < 0 ) {
		if ( (entk->tmp_buff[1] < 0) || (intrpt == -2) ) {
			entk->tmp_proc[0]( entk ) ;
		} else {
			entk->think3 = entk->tmp_buff[1] ;
		}

		entk->count3 = 0 ;
		
		return ;
	}else if ( intrpt == 1 ) {	/* R_INTRPT */
		R_INTRPT	*r_intrpt ;
		int			dir ;
		r_intrpt = GM_GetRIntrpt( entk->znavi->this_addr, entk->znavi->next_addr ) ;
		if ( r_intrpt != NULL ) {
#if 1	//11.19
printf("move.c:r_intrpt->status[%x]\n",r_intrpt->status);
printf("move.c:r_intrpt & status[%x]\n",
((r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED)) == (ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) ));
			if ( ( (r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED)) 
					== (ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) ) 
				&& !(r_intrpt->status & (ROOT_INTRPT_NOENE|ROOT_INTRPT_BREAK)) ) {
#else
			if ( r_intrpt->status & ROOT_INTRPT_CLOSE && r_intrpt->status & ROOT_INTRPT_HINGED ) {
#endif

printf("Locker Open!!\n");
				ENE_StandPosRintrpt( r_intrpt, MOT_RINTRPT_LOCKER_OPEN, 
							&entk->act->target_pos, &dir ) ;
				entk->act->aim_dir = dir ;
				entk->think3 = TH3_NEAR_MOVE ;
				entk->count3 = 0 ;

				return ;
			} else {
				entk->tmp_proc[0]( entk ) ;
				entk->count3 = 0 ;
				
				return ;
			}
		}
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

#ifdef USE_DIRECT_MOVE
static void Think3_MoveDirect( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		if ( entk->act->move_s == MoveCautionWalk )
			entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), entk->tmp_buff[0] ) < 0 ) {
		entk->tmp_proc[0]( entk ) ;
		entk->count3 = 0 ;
		return ;
	}

	/* 目的ゾーン内にＰＯＳがなければ終了 */
	if( !ENE_InZone( &entk->trgpoint.pos, entk->trgpoint.addr ) ) {
		entk->tmp_proc[0]( entk ) ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}
#endif

static void Think3_NearMove( entk )
ENETHINK	*entk ;
{
//	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	if( ENE_InRange( &entk->ctrl->mov, &entk->act->target_pos, 350 ) ) {
		entk->think3 = TH3_PINPOINT_MOVE ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->target_pos ) ;
	entk->count3 ++ ;
}

static void Think3_PinpointMove( entk )
ENETHINK		*entk ;
{
	entk->act->pad = SP_PINPOINT ;

//	if ( entk->act->act_end ) {
	if ( entk->act->act_end || entk->count3 > COUNT_VMODE(300) ) {
		entk->think3 = TH3_LOCKER_OPEN ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->act->aim_dir ;

	entk->count3 ++ ;

}

static void Think3_LockerOpen( entk )
ENETHINK		*entk ;
{
	if ( entk->count3 == 0 ) {
		R_INTRPT	*r_intrpt ;

		r_intrpt = GM_GetRIntrpt( entk->znavi->this_addr, entk->znavi->next_addr ) ;
		if ( r_intrpt != NULL ) {
			GM_RIntrptCallBack( r_intrpt, 100, 5, RINTRP_ORDER_OPEN ) ;
		}
		entk->act->pad = SP_LOCKER_OPEN ;
	}

	if ( entk->act->act_end ) {
		entk->tmp_proc[0]( entk ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;

}
