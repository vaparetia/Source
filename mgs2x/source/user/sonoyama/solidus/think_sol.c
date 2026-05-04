/*
   think_sol.c
   ソリダス／考える

   2001/03/27	M.Sonoyama
   $Id: think_sol.c,v 1.1.1.3 2002/11/19 11:51:07 Yoshizawa1 Exp $
*/

/* 近距離思考 */
static	void	NearThinkAct( Work *work )
{
	NearThink	*nt ;

	nt = &work->near_think ;
	
	/* 向きが違う */
	if ( ( work->len_diff_player < 1800.0F + 300.0F ) &&
		 ( GV_DiffDirAbs( GM_PlayerControl->turn.vy, work->control.turn.vy + 2048 ) > 256 ) ) {
		nt->rot_diff_count ++ ;
		if ( nt->rot_diff_count > 4 ) {
			SetFlag( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ;
			if ( GV_Time & 1 ) SetFlag( work, SOL_FLAG_BLADE_ATTACK_W ) ;
		}
	} else {
		nt->rot_diff_count = 0 ;
	}

	/* 間合い内 */
	if ( work->maai_count > 0 ) {	
		if ( ++ nt->maai_count > 8 && !PL_SlashNow() ) {
			SetFlag( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ;		
			if ( GV_Time % 4 == 0 ) SetFlag( work, SOL_FLAG_BLADE_ATTACK_W ) ;
		}
	} else {
		nt->maai_count = 0 ;
	}

	/* ３ｍ以内で5秒以上 */
	if ( work->len_diff_player < 3000.0F ) {
		if ( ++ nt->middle_count > ( 300 / TIME_BASE ) * 5 ) {
			SetFlag( work, SOL_FLAG_FAST_AWAY ) ;
		}
	} else {
		nt->middle_count = 0 ;
	}

	/* プレイヤー位置が低い */
	if ( GM_CheckPlayerStatus( PLAYER_GROUND | PLAYER_DOWNED ) ) {
		int			time ;
		ResetFlag( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ;
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			++ nt->ground_count ;
			time = DIRECT_TICK( 120 ) ;
			if ( !GM_CheckPlayerStatus( PLAYER_DOWNED ) && 
				 !GM_CheckPlayerStatus( PLAYER_INVINCIBLE ) ) {
				time = nt->ground_count - 1 ;
			}
		} else {
			++ nt->ground_count ;
			if ( !GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ) {
				time = DIRECT_TICK( 60 ) ;
			} else {
				time = DIRECT_TICK( 120 ) ;
			}
		}
		if ( nt->ground_count > time ) {
			if ( work->len_diff_player > 3000.0F ) SetFlag( work, SOL_FLAG_FAST_AWAY ) ;	
			else								   SetFlag( work, SOL_FLAG_BLADE_ATTACK_ENABLE |
														    SOL_FLAG_BLADE_ATTACK_W ) ;
		}
	} else {
		nt->ground_count = 0 ;
	}
}

/* 遠距離思考 */
static	void	FarThinkAct( Work *work ) 
{
	FarThink	*ft ;	
	int			waitcount ;
	float		farlen ;

	if ( work->maai_count > 0 ) return ;

	ft = &work->far_think ;
	/* ４ｍ前後 */
	if ( work->mode == SOL_MODE_BLADE_ONLY ) {
		farlen = 4000.0F ;
		waitcount = 8 ;
	} else {
		/* 空振り蛇手を出すため、遠くにしてみる */
		farlen = 6500.0F ;
		waitcount = 16 ;
	}
	if ( work->len_diff_player > 2200.0F && 	
		 work->len_diff_player < farlen &&
		 !GM_CheckPlayerStatus( PLAYER_DOWNED | PLAYER_GROUND | PLAYER_DAMAGED ) ) {
		if ( ++ ft->liftup_count > waitcount ) {
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				SetFlag( work, SOL_FLAG_ELBOW_ENABLE ) ;
			} else {
				SetFlag( work, SOL_FLAG_LIFTUP_ENABLE ) ;
			}
		}
	} else {
		if ( ft->liftup_count > 0 ) ft->liftup_count = 0 ;
	}

	/* 結構離れている */
//	if ( work->len_diff_player > 10000.0F && 
//		 !GM_CheckPlayerStatus( PLAYER_DOWNED ) ) {
	if ( work->len_diff_player > 8000.0F ) {
		/* 0.90Fは、attack_sol.cにもあるので注意 */
		if ( ++ ft->missile_count > 0 && CheckLife( work, 0.90F ) &&	
			 GV_Time - ft->last_missile_time > 10 * 300 / TIME_BASE ) {
			SetFlag( work, SOL_FLAG_MISSILE_ATTACK_ENABLE ) ;
		}
	}
}
