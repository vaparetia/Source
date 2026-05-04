/*
  thk_player.h
  思考処理

  2001/02/14 T.Morita Revised
  $Id: thk_player.h,v 1.1.1.3 2002/11/19 11:46:05 Yoshizawa1 Exp $
*/




/*

  角度差

*/
void EMA_ThinkResetDiff( Work *work, int move )
{
    int turn ;
    FVECTOR diff, aim ;
    FMATRIX world ;

    if ( move )
	turn = GM_PlayerControl->rot.vy ;
    else
    {
	EMA_ThinkDestinatePos( work, &diff, &aim, &world, 5 ) ;
	turn = RAD2ANG(atan2f( diff.vx, diff.vz )) ;
    }

    work->diff_turn = EMA_RecalcDir( work->control.rot.vy - turn ) ;
    work->pl_turn   = GM_PlayerControl->rot.vy ;
    work->prev_turn = work->control.rot.vy ;
}

static inline void EMA_ThinkDirectionDiff( Work *work )
{
    /* 向きの変化 */
    if ( *work->ext_trg == 0 )
	work->diff_turn = 0 ;
    else if ( !GM_CheckPlayerStatus( PLAYER_WATCH ) )/*主観時は,移動しない*/
    {
	/*if ( work->prv_trg != *work->ext_trg )*/
	if ( !work->prv_trg )
	{
	    EMA_ThinkResetDiff( work, 0 ) ;
	    EMA_SetFlag( EMA_F_FIRST_LINK ) ;
	}

	work->diff_turn += EMA_RecalcDir( work->control.rot.vy -
					  work->prev_turn ) ;
	work->diff_turn -= EMA_RecalcDir( GM_PlayerControl->rot.vy -
					  work->pl_turn ) ;
	work->prev_turn = work->control.rot.vy     ;
	work->pl_turn   = GM_PlayerControl->rot.vy ;
    }
}


static inline void EMA_ThinkAngerAtPlayer( Work *work )
{
    int id ;

    return ;/* ちょっと怒らない */

    if ( EMA_Flag( EMA_F_HURT_BY_PLY ) && !EMA_Flag( EMA_F_ATTACKED_BY ) )
    {
	id = (irnd()&1 ? EMA_VC281 :
	      irnd()&2 ? EMA_VC261 :
	                 EMA_VC251) ;
	EMA_UtilStartStreamIdx( work, id ) ;
	EMA_ResetFlag( EMA_F_HURT_BY_PLY ) ;
    }
    else if ( work->voice_code == work->voice[EMA_VC251] ||
	      work->voice_code == work->voice[EMA_VC261] ||
	      work->voice_code == work->voice[EMA_VC281] )
	EMA_ResetFlag( EMA_F_NON_FACING ) ;
    else
	EMA_SetFlag( EMA_F_NON_FACING ) ;
}


static void Vibration( Work *work )
{
    float dis ;
    int   vib ;

    if ( PL_GetPlayerItem() == IT_VibSensor || !EMA_Flag(EMA_F_ENB_VIBRATE) )
	return ;

#define EMA_VIB_NORMAL 80
    if ( work->tic == 0 )
	if ( work->vib_rate > 0 )
	    work->vib_rate-- ;
    if ( GM_AlertMode == ALERT_MODE_ALERT )
	work->vib_rate = EMA_VIB_NORMAL - 28 ;
    if ( GM_AlertMode == ALERT_MODE_AVOID )
	work->vib_rate = EMA_VIB_NORMAL - 32 ;
    if ( GM_AlertMode == ALERT_MODE_SEARCH )
	work->vib_rate = EMA_VIB_NORMAL - 40 ;
    vib = EMA_VIB_NORMAL - work->vib_rate ;
    vib = vib<28 ? 28 : vib>EMA_VIB_NORMAL ? EMA_VIB_NORMAL : vib ;

    dis = GV_VecLen3F2( &GM_PlayerControl->mov, &work->control.mov ) ;
    if ( dis < 10000 )
	if ( fpu_Abs(GV_Time - work->vib_time) > vib )
	{
	    static u_char HeartBeat1[] = { 120, 6, 0, 0 } ;
	    static u_char HeartBeat2[] = { 128, 6, 0, 0 } ;
	    static u_char HeartBeat3[] = { 144, 6, 0, 0 } ;
	    u_char *pad ;

	    pad = HeartBeat1 ;
	    if ( GM_AlertMode == ALERT_MODE_AVOID ||
		 GM_AlertMode == ALERT_MODE_SEARCH )
		pad = HeartBeat2 ;
	    if ( GM_AlertMode == ALERT_MODE_ALERT )
		pad = HeartBeat3 ;

	    NewPadVibration( pad, 2|VAR_FLAG_FORCE ) ;
	    work->vib_time = GV_Time ;
	}
}


/*

  頭がメリコムかどうかの処理
  メリ込んだら向きを変える

 */
static void EMA_ThinkCheckHead( Work *work )
{
    int   i ;
    int   dir[4], n_dir, ang ;
    int   min ;
    float d ;
    FVECTOR v ;

    n_dir = 0 ;
    min   = 4096 ;
    for ( i=work->control.n_touches ; --i>=0 ; )
    {
	_sceVu0Normalize( &v, &work->control.vecs[i] ) ;
	d = _sceVu0InnerProduct( &v, work->body.objs->world.m[Z] ) ;
	if ( d > 0.8122f )
	{
	    ang = 2048.0f / M_PI * atan2f( work->control.vecs[i].vx,
					   work->control.vecs[i].vz ) ;
	    dir[n_dir+0] = ang - 406 - work->control.turn.vy ;
	    dir[n_dir+1] = ang + 406 - work->control.turn.vy ;
	    dir[n_dir+0] = EMA_RecalcDir( dir[n_dir+0] ) ;
	    dir[n_dir+1] = EMA_RecalcDir( dir[n_dir+1] ) ;
	    if ( fpu_Abs( min ) > fpu_Abs( dir[n_dir+0] ) )
		min = dir[n_dir+0] ;
	    if ( fpu_Abs( min ) > fpu_Abs( dir[n_dir+1] ) )
		min = dir[n_dir+1] ;

	    n_dir += 2 ;
	}
    }

    if ( min && min < 4096 )
	work->control.turn.vy += min ;
}

/*

  プレーヤーに見つめられているかどうかの処理
  
 */
static void EMA_ThinkPlayerStare( Work *work )
{
     int dir ;

     if ( work->stare_cnt > 10 )
     {
	  dir  = work->pl_dir - work->control.turn.vy ;
	  dir &= 4095 ;
	  dir -= dir>2048 ? 4096 : 0 ;
	  if ( !(dir / 800) )
	       EMA_ResetSetFlag( EMA_F_NON_FACING, EMA_F_ENB_EYESIGHT ) ;
     }
     if ( EMA_Flag( EMA_F_ENB_EYESIGHT ) )
	  _sceVu0CopyVector( &work->eye_pos,
			     (FVECTOR *)DG_Chanl(0)->eye.m[W] ) ;
}


/*

  プレーヤーに見つめられているかどうかの処理
  
 */
static void EMA_ThinkPlayerStareSquat( Work *work )
{
    int id ;

    if ( work->stare_cnt < 80 && work->stare_cnt > 25 &&
	 (irnd() & 0x011) == 0 )
    {
	id = (irnd()&0x010) ? EMA_VC351 : EMA_VC361 ;

	if ( !work->cold_stare && id )
	    if ( EMA_UtilStartStreamIdx( work, id ) >= 0 )
	    {
		work->stare_cnt = 81 ; 
		work->voice_chk |= 1<<id ;
	    }
    }
}


/*

  プレーヤーに押された時の処理

*/
static void EMA_ThinkPlayerPushed( Work *work )
{
    FVECTOR  v   ;
    float    diff ;
    int      dir ;
    int      rot ;
    CONTROL *det ;

    if ( EMA_Flag( EMA_F_ATTACKED_BY ) )
	return ;

    switch( work->npc.action.pad )
    {
    case DAMG_DOWN:	case DAMG_FALL:
    case DAMG_SLEEP:    case DAMG_DOWN_OVER:
    case DAMG_OVER:     case DAMG_TUMBLE:
    case DAMG_DROPOFF:  case SNIPE_DOWN_FALL:
	return ;
    case DAMG_WAKE:
	if ( work->time < 35 )
	    return ;
    case RAI_GETUP:
	if ( work->time < 14 )
	    return ;
    }

    /* 周りの物のへの距離を計る */
    det = EMA_NearEneControl( work, (*work->ext_trg ? 150.0f : 350.0f), &v ) ;
    /* 主観の場合 */
    if ( det == NULL )
	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
	{
	    v.vx = DG_Chanls->eye.m[W][X] - work->control.mov.vx ;
	    v.vz = DG_Chanls->eye.m[W][Z] - work->control.mov.vz ;
	    v.vy = 0.0f ;
#if 0
	    if ( v.vx*v.vx + v.vz*v.vz < 300.0f*300.0f )
		det = GM_PlayerControl ;

	    else
#endif
	    {
		/* 顔にはメリ込まない様に */
		if ( GM_CheckPlayerStatus( PLAYER_SQUAT ) )
		{
		    v.vx = DG_Chanls->eye.m[W][X] -
			BODYPOS( &work->body, HUMAN21_ATAMA)->vx ;
		    v.vz = DG_Chanls->eye.m[W][Z] -
			BODYPOS( &work->body, HUMAN21_ATAMA)->vz ;
		    v.vy = 0.0f ;
		    if ( v.vx*v.vx + v.vz*v.vz < 300.0f*300.0f )
			det = GM_PlayerControl ;
		}

		/* マイクの時だけ特別にチェックする */
		if ( PL_GetPlayerWeapon() == WP_Mic )
		{
#define MIC_LENGTH 800.0f
		    diff = DG_Chanls->eye.m[W][Y]+ MIC_LENGTH * DG_Chanls->eye.m[Z][Y]
			- work->control.mov.vy ;
		    if ( diff < 500.0f && diff > -500.0f )
		    {
			v.vx += MIC_LENGTH * DG_Chanls->eye.m[Z][X] ;
			v.vz += MIC_LENGTH * DG_Chanls->eye.m[Z][Z] ;
			if ( v.vx*v.vx + v.vz*v.vz < 400.0f*400.0f )
			    det = GM_PlayerControl ;
		    }
		}
	    }
	}

    /* 匍匐の場合 */
    if ( det == NULL )
	if ( GM_CheckPlayerStatus( PLAYER_GROUND ) )
	{
	    v.vx = BODYPOS(GM_PlayerBody,HUMAN21_KUBI)->vx -
		work->control.mov.vx ;
	    v.vz = BODYPOS(GM_PlayerBody,HUMAN21_KUBI)->vz -
		work->control.mov.vz ;
	    v.vy = 0.0f ;
	    if ( v.vx*v.vx + v.vz*v.vz < 300.0f*300.0f )
		det = GM_PlayerControl ;
	}

    /* ふっ飛び処理 */
    if ( det != NULL )
    {
	if ( work->ext_trg )
	    *work->ext_trg = 0 ;    
	if ( (work->npc.action.set_pad == PAL_PUSHED_LF ||
	      work->npc.action.set_pad == PAL_PUSHED_LB) && work->time >= 10 )
	    work->npc.action.pad = DAMG_TUMBLE ;
	else if ( (work->npc.action.set_pad != PAL_PUSHED_LF &&
		   work->npc.action.set_pad != PAL_PUSHED_LB) )
	{
	    /* 手のIKと顔を向けない */
	    EMA_ResetSetFlag( EMA_F_ENB_ARM_IK, EMA_F_NON_FACING ) ;

	    _sceVu0Normalize( &v, &v ) ;
	    _sceVu0ScaleVector( &work->vel_adjust, &v, -60.0f ) ;
	    work->vel_adjust.vw = work->vel_adjust.vy = 0.0f ;

	    EMA_ActLayerMotion( work, -1, 0, BODY_MASK|ARMS_MASK ) ;

	    rot = 2048.0f * atan2f( v.vx, v.vz ) / M_PI ;
	    dir = rot - work->control.turn.vy ;
	    dir &= 4095 ;
	    work->control.turn.vy = rot ;
	    if ( dir > 1024 && dir < 3072 )
	    {
		work->npc.action.pad = PAL_PUSHED_LB ;
		work->control.turn.vy += 2048 ;
	    }
	    else
		work->npc.action.pad = PAL_PUSHED_LF ;
	    //work->voice_id  = SD_V_EMADWN01 ;
	    //work->voice_tim = 1 ;
	}
    }
}


static inline void EMA_ThinkPlayerTrigger( Work *work )
{
    int id ;

    /* トリガーが変化したらタイマーを０にする */
    if ( *work->ext_trg != work->prv_trg )
	work->time = 0 ;

    EMA_ThinkAngerAtPlayer( work ) ;

    /* 舟虫対策 */
    if ( EMA_Flag( EMA_F_RUN_AWAY ) )
    {
#if 1
	if ( work->npc.action.set_pad == PAL_DISLIKE ||
	     work->npc.action.set_pad == AFRAID_WALK )
#else
	if ( work->npc.action.set_pad == DAMG_IDLE ||
	     work->npc.action.set_pad == RAI_WALK_CORPS )
#endif
	    PLY_EE_ResetGraspModeAndTrigger() ;
	else
	    EMA_ResetFlag( EMA_F_RUN_AWAY ) ;
    }

    /* 音声を止める 立ったり／危険だったら */
    if ( (*work->ext_trg ||
	  GM_AlertMode == ALERT_MODE_ALERT ||
	  GM_AlertMode == ALERT_MODE_AVOID ) )
	if ( work->voice_code == work->voice[EMA_VC351] ||
	     work->voice_code == work->voice[EMA_VC361] ||
	     work->voice_code == work->voice[EMA_VC291] ||
	     work->voice_code == work->voice[EMA_VC301] )
	    EMA_UtilStopStream( work ) ;

    switch( *work->ext_trg )
    {
    case 0:
	/* IKおよび顔追尾をＯＦＦにする */
	EMA_ResetSetFlag( EMA_F_FIRST_WALL|EMA_F_ENB_ARM_IK,
			  EMA_F_NON_FACING ) ;
	/* プレーヤーの顔もリセットする */
	work->ply_stare_cnt =  -EMA_NONSTARE_CNT ;
	/*壁向き限度の設定*/
	work->wall_rot = 0.82f ;

#if 0
	if ( work->prv_trg )
	    work->npc.action.pad = RAI_FREE ;
#endif

	if ( GM_AlertMode != ALERT_MODE_ALERT &&
	     GM_AlertMode != ALERT_MODE_AVOID )
	    switch( work->npc.action.set_pad )
	    {
	    case PAL_PUSHED_LB :
	    case PAL_PUSHED_LF :
	    case DAMG_WAKE:
	    case RAI_GETUP:
	    case RAI_FREE:
		if ( work->npc.action.act_end )
		    work->npc.action.pad = PAL_IDLE ;
		break ;

	    case RAI_WALK:
		if ( EMA_Flag( EMA_F_IS_COMMING ) )
		{
		    EMA_ThinkComeInMove( work, PAL_IDLE, RAI_WALK ) ;
		    if ( !work->prv_trg )
			if ( !EMA_ThinkCheckOutSideEmma( work ) )
			    if ( work->npc.action.act_end )
			    {
				EMA_ResetFlag( EMA_F_IS_COMMING ) ;
				work->npc.action.pad = PAL_IDLE ;
			    }
		}
		else
		    work->npc.action.pad = PAL_IDLE ;
		break ;

	    case PAL_SITDOWN:
		if ( work->npc.action.act_end )
		    work->npc.action.pad = PAL_SQUAT_IDLE ;
		break ;

	    case PAL_EROKICK:
#if 0
		if ( !work->book )
		    work->npc.action.pad = PAL_IDLE ;
		else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 46*5 ) )
		    EMA_UtilStartStreamIdx( work, EMA_VC411 ) ;/* へんたい！*/
		else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 96*5 ) )
		    EMA_UtilBullet( work, &work->book_pos ) ;
		else if ( work->npc.action.act_end )
		{
		    work->book->flag |= GM_BMB_FLAG_DESTROY ;
		    work->npc.action.pad = PAL_IDLE ;
		}
#endif
		break ;

	    case PAL_SQUAT_IDLE:
		/* プレーヤーに見つめられているかどうか */
		EMA_ThinkPlayerStare( work ) ;

		/* 座っている時の見つめられている時 */
		EMA_ThinkPlayerStareSquat( work ) ;

		/* 舟虫が近くにいるか */
		if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
		    if ( EMA_ThinkCheckSlater( work ) )	/*think/thk_slater.h*/
		    {
			work->npc.action.pad = RAI_GETUP ;
			break ;
		    }
#if 0
		/* エロ本がある */
		if ( work->book )
		{
		    work->npc.action.pad = RAI_GETUP ;
		    break ;
		}
#endif
		EMA_RecoverVitality( work, EMA_VITAL_UP ) ;
		break ;

	    case PAL_IDLE:
		/* プレーヤーに見つめられているかどうか */
		//EMA_ThinkPlayerStare( work ) ;

		NPC_ReSetSlopeRotX( &work->control ) ;
		work->shadow_flg = 1 ;

		/* 外にいるかどうか */
		if ( work->time > 3 )
		{
		    if ( EMA_ThinkComeInside( work ) )	/*think/thk_comein.h*/
		    {
			work->npc.action.pad = RAI_WALK ;
			break ;
		    }
		}

		/* プレーヤーの頭が壁にメリ込む？ */
		EMA_ThinkCheckHead( work ) ;

		/* 舟虫が近くにいるか */
		if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
		    if ( EMA_ThinkCheckSlater( work ) )	/*think/thk_slater.h*/
		    {
			EMA_ThinkEscapeFromSlater( work ) ;
			break ;
		    }

#if 0
		/* エロ本がある */
		if ( work->book )
		{
		    float x, z ;

		    x = work->book_pos.vx - work->control.mov.vx ;
		    z = work->book_pos.vz - work->control.mov.vz ;

		    work->control.turn.vy = RAD2ANG( atan2f( x, z ) ) ;
		    work->npc.action.pad = PAL_EROKICK ;
		    break ;
		}
#endif

		if ( (work->time > 100*2 ||
		      work->npc.action.life < work->vital_max/4) && 
		     work->time > 10 &&  /*必ず少しは立っている(ロード対策) */
		     work->control.level[0]->p3.h > 30000.0f )
		{
		    if ( work->npc.action.life < work->vital_max/4 )
		    {
			id = irnd()&0x011 ? EMA_VC291 : EMA_VC301 ;
			/*else
			id = irnd()&0x011 ? EMA_VC351 : EMA_VC361 ;*/
			if ( !work->cold_stare )
			    if ( EMA_UtilStartStreamIdx( work, id ) >= 0 )
				work->voice_chk |= 1<<id ;
		    }
		    work->npc.action.pad = PAL_SITDOWN ;
		}
		break ;

	    case PAL_DISLIKE:
		EMA_ThinkSlaterMove( work ) ;	/*think/thk_slater.h*/
		if ( work->npc.action.act_end )
		{
		    if ( EMA_Flag( EMA_F_RUN_AWAY ) )
			work->npc.action.pad = AFRAID_WALK ;
		    else
			work->npc.action.pad = PAL_IDLE ;
		}
		break ;

	    case DAMG_IDLE:
		if ( EMA_Flag( EMA_F_RUN_AWAY ) )
		{
		    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
			work->npc.action.pad = RAI_WALK_CORPS ;
		}
		else
		    work->npc.action.pad = PAL_IDLE ;
		break ;

	    case AFRAID_WALK:
	    case RAI_WALK_CORPS:
		if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) &&
		      EMA_Flag( EMA_F_RUN_AWAY ) )
		    EMA_ThinkSlaterMove( work ) ;	/*think/thk_slater.h*/
		else
		    work->npc.action.pad = PAL_IDLE ;
		break ;

	    case DAMG_TUMBLE:
		if ( work->npc.action.act_end )
		    work->npc.action.pad = DAMG_WAKE ;
		break ;

	    default:
		work->npc.action.pad = PAL_IDLE ;
		break ;
	    }
	else
	    switch( work->npc.action.set_pad )
	    {
	    case PAL_PUSHED_LB :
	    case PAL_PUSHED_LF :
	    case DAMG_WAKE:
	    case RAI_GETUP:
	    case RAI_FREE:
		if ( work->npc.action.act_end )
		    work->npc.action.pad = AFRAID_IDLE ;
		break ;

	    case DAMG_TUMBLE:
		if ( work->npc.action.act_end )
		    work->npc.action.pad = DAMG_WAKE ;
		break ;

	    case AFRAID_WALK:
		EMA_ThinkAfraidMove( work ) ;	/* think/thk_rai_afraid.h */
		break ;

	    default:
		/* プレーヤーの頭が壁にメリ込む？ */
		EMA_ThinkCheckHead( work ) ;

		EMA_ThinkAfraidIdle( work ) ;	/* think/thk_rai_afraid.h */
		break ;
	    }
	break ;

    case 1:
	switch( work->npc.action.set_pad )
	{
#if 0 /* とりあえずREACH FREE は 無し ウザイので */
	case PAL_IDLE:
	    work->npc.action.pad = RAI_REACH ;
	    break ;
#endif
	case DAMG_TUMBLE:
	    if ( work->npc.action.act_end )
		work->npc.action.pad = DAMG_WAKE ;
	    break ;

	case PAL_SQUAT_IDLE:
	    work->npc.action.pad = RAI_GETUP ;
	    break ;
	case PAL_PUSHED_LB :
	case PAL_PUSHED_LF :
	case RAI_REACH:
	case RAI_GETUP:
	case DAMG_WAKE:
	    if ( work->npc.action.act_end )
		work->npc.action.pad = RAI_IDLE ;
	    break ;

	case PAL_SITDOWN:
	    if ( work->npc.action.time > 44*TIME_BASE/5 )
	    {
		if ( work->npc.action.act_end )
		    work->npc.action.pad = PAL_SQUAT_IDLE ;
		break ;
	    }

	case RAI_IDLE:
	    /* どんなことがあっても水平に戻す */
	    NPC_ReSetSlopeRotX( &work->control ) ;
	    work->shadow_flg = 1 ;

	    EMA_SetFlag( EMA_F_ENB_ARM_IK ) ;
	    Vibration( work ) ;

	default:
	    if ( work->mar_mtn == MAR(EMA_BASE,RAI_IDLE) )
		EMA_SetNaviTarget( work,
				   GM_PlayerControl->addr,
				   &GM_PlayerControl->mov ) ;

	    /* IKおよび顔追尾をＯＮにする */
	    //EMA_ResetSetFlag( EMA_F_NON_FACING, EMA_F_ENB_ARM_IK ) ;
	    EMA_ResetFlag( EMA_F_NON_FACING ) ;
	    EMA_ThinkRaiIdle( work ) ;	/* think/thk_rai_idle.h */
	}
	break ;

#if 0
	/* 歩き出したら見つめない */
	work->ply_stare_cnt = 0 ;
	/* 振動 */
	Vibration( work ) ;
	if ( work->npc.action.set_pad == PAL_PUSHED_LB ||
	     work->npc.action.set_pad == PAL_PUSHED_LF )
	{
	    if ( work->npc.action.act_end )
		work->npc.action.pad = PAL_IDLE ;
	    break ;
	}
	/* 舟虫が近くにいるか */
	if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
	    if ( EMA_ThinkCheckSlater( work ) )	/* think/thk_slater.h */
	    {
		EMA_ThinkEscapeFromSlater( work )
		break ;
	    }
	work->npc.action.pad = RAI_WALK ;
	EMA_ThinkRaiMove( work ) ; 	/* think/thk_rai_move.h */

	break ;
#endif

    case 2:
    case 3:
	/* 歩き出したら見つめない */
	work->ply_stare_cnt = 0 ;

	/* どんなことがあっても水平に戻す */
	NPC_ReSetSlopeRotX( &work->control ) ;
	work->shadow_flg = 1 ;

	/* 自動的に入る行為が中断された時のためにフラグをねかす */
	EMA_ResetFlag( EMA_F_IS_COMMING ) ;

	/* 振動 */
	Vibration( work ) ;
	if ( work->npc.action.set_pad == PAL_PUSHED_LB ||
	     work->npc.action.set_pad == PAL_PUSHED_LF )
	{
	    if ( work->npc.action.act_end )
		work->npc.action.pad = PAL_IDLE ;
	    break ;
	}
	/* 舟虫が近くにいるか */
	if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
	    if ( EMA_ThinkCheckSlater( work ) )	/* think/thk_slater.h */
	    {
		EMA_ThinkEscapeFromSlater( work ) ;
		break ;
	    }
	work->npc.action.pad = *work->ext_trg==2 ? RAI_WALK : RAI_RUN ;
	EMA_ThinkRaiMove( work ) ; 	/* think/thk_rai_move.h */
#if 0
	if ( GM_CheckPlayerStatus( PLAYER_ON_CORPSE ) )
	    work->npc.action.pad = RAI_WALK_CORPS ;
#endif
	break ;

    case 4:
	/* どんなことがあっても水平に戻す */
	NPC_ReSetSlopeRotX( &work->control ) ;
	work->shadow_flg = 1 ;

	/* 歩き出したら見つめない */
	work->ply_stare_cnt = 0 ;
	/* 振動 */
	Vibration( work ) ;
	EMA_RecoverVitality( work, EMA_VITAL_UP ) ;
	work->npc.action.pad = RAI_SQUAT_IDLE ;
	break ;
    }

    /* プレーヤーに押されたか？ */
    EMA_ThinkPlayerPushed( work ) ;

    /* プレーヤーとの角度相対関係(くるりん防止用の情報) */
    EMA_ThinkDirectionDiff( work ) ;

    if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )/*主観時は,移動しない*/
	work->prv_trg = 0 ;
    else
	work->prv_trg = *work->ext_trg ;


    /* エマの行動活動停止 */
    if ( EMA_Flag(EMA_F_NON_MOVE) )
	work->npc.action.pad = PAL_IDLE ;
}
