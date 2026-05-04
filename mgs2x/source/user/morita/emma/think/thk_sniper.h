/*
  thk_sniper.h
  思考処理

  2001/03/18 T.Morita
  $Id: thk_sniper.h,v 1.1.1.3 2002/11/19 11:46:05 Yoshizawa1 Exp $
*/

static int EMA_ThinkCheckWave( Work *work )
{
    if ( work->ground >= 3 )
    {
	work->npc.action.pad = SNIPE_SITDOWN/*SNIPE_SQUAT_IDLE*/ ;
	return 1 ;
    }
    if ( work->ground >= 8 )
    {
	printf( "work->ground%d\n", work->ground ) ;
	EMA_DamageGameOver( work, DAMG_DROPOFF ) ;
    }
    return 0 ;
}

static void EMA_ThinkOnTheRoute( Work *work )
{
    int  proc_id ;
    static FVECTOR DestGoDown = { 0.0f, -500.0f, 100.0f, 1.0f } ;
    static FVECTOR DestGoUp   = { 0.0f, 1000.0f, 460.0f, 1.0f } ;

    if ( EMA_GetDirFromRoute( work, 500.0f ) )
	if ( work->route )
	{
	    printf( "Emma: flag%d act%d time%d",
		    work->route->flag, work->route->act, work->route->time ) ;
	    if ( work->route->flag > 0 && work->route->flag < EMA_P_N_PROC )
	    {
		proc_id = work->proc[work->route->flag] ;
		if ( proc_id )
		    GCL_ExecProc( proc_id, NULL ) ;
	    }

	    switch( work->route->act )
	    {
	    case 1:
		work->control.skip_flag |=  CTRL_SKIP_SEG_CHECK ;/* 取り敢えず壁は抜けない */
		EMA_SetFlag( EMA_F_ENB_GO_DOWN|EMA_F_NON_DAMAGE ) ;
		break ;
	    case 2:
		//work->control.skip_flag |=  CTRL_SKIP_SEG_CHECK ;
		EMA_SetFlag( EMA_F_ENB_GO_UP|EMA_F_NON_DAMAGE ) ;
		break ;
	    }
	    work->stay_cnt = work->route->time ;

	    printf( " stay%d\n", work->stay_cnt ) ;
	}

    if ( work->stay_cnt )
	work->npc.action.pad = SNIPE_IDLE ;
    if ( EMA_Flag( EMA_F_ENB_GO_DOWN ) )/* 下がれるかどうか 10cm前を見る */
	if ( EMA_CheckDestFloor( work, &DestGoDown ) < -300.0f )
	{
	    work->npc.action.pad = SNIPE_GO_DOWN ;
	    EMA_ResetSetFlag( EMA_F_ENB_GO_DOWN, EMA_F_NON_DAMAGE ) ;
	}
    if ( EMA_Flag( EMA_F_ENB_GO_UP ) )/* 上がれるかどうか 50cm前を見る */
	if ( EMA_CheckDestFloor( work, &DestGoUp ) > 300.0f )
	{
	    work->npc.action.pad = SNIPE_GO_UP ;
	    EMA_ResetSetFlag( EMA_F_ENB_GO_UP, EMA_F_NON_DAMAGE ) ;
	}
}

static void EMA_ThinkStartMove( Work *work )
{
    if ( EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	work->npc.action.pad = SNIPE_WALK ;
    else
	work->npc.action.pad = SNIPE_RUN  ;
}

static int EMA_ThinkDamageVoice( Work *work )
{
    int id ;

    id = ( !(work->voice_chk & (1 << EMA_VC651)) ? EMA_VC651 :     /*「ちょっと！殺す気」        */
	   !(work->voice_chk & (1 << EMA_VC671)) ? EMA_VC671 :     /*「何なのよ、あなたサイテー」*/
	   !(work->voice_chk & (1 << EMA_VC681)) ? EMA_VC681 : 0) ;/*「もうイヤ」                */

    if ( id )
	if ( EMA_PFlag( EMA_F_NON_FACING ) )
	    if ( EMA_UtilStartStreamIdx( work, id ) >= 0 )
	    {
		work->voice_chk |= 1 << id ;
		EMA_ResetFlag( EMA_F_NON_FACING|EMA_F_HURT_BY_PLY ) ;
	    }
    return id ;
}

static inline void EMA_ThinkCheckStop( Work *work )
{
    FVECTOR diff  ;

    /* プレーヤの実際の速度を計る(壁を歩き続けるので..) */
    _sceVu0SubVector( &diff, &work->control.mov, &work->prev_pos ) ;

    if ( diff.vx*diff.vx + diff.vz*diff.vz < 10.0f )
	EMA_SetFlag( EMA_F_ENB_GO_UP ) ;

}

static inline void EMA_ThinkSniperEvent( Work *work )
{

#if 1
    switch( work->npc.action.set_pad )
    {
	//case SNIPE_SQUAT_IDLE:
    case SNIPE_IDLE:
    case SNIPE_STANDUP:

    case SNIPE_RUN:
    case SNIPE_WALK:
	if ( EMA_Flag( EMA_F_ENB_RECOVER ) && 
	     !work->non_damage && work->npc.action.life < work->vital_max/4 )
	{
	    printf( "Emma : Yeah!!! going to recover\n" ) ;
	    work->npc.action.pad = SNIPE_SITDOWN ;
	    return ;
	}
    }
#endif

    switch( work->npc.action.set_pad )
    {
    case SNIPE_SITDOWN:
    	EMA_SetFlag( EMA_F_NON_FACING ) ;
	if ( work->npc.action.act_end )
	    work->npc.action.pad = SNIPE_SQUAT_IDLE ;
	break ;

    case SNIPE_SITAFRAID:
    	EMA_SetFlag( EMA_F_NON_FACING ) ;
	if ( work->npc.action.act_end )
	    work->npc.action.pad = SNIPE_AFRAID ;
	break ;

    case SNIPE_AFRAID:
    case SNIPE_SQUAT_IDLE:
    	EMA_SetFlag( EMA_F_NON_FACING ) ;
	if ( work->stay_cnt > 0 ) /* 強制的に止まるカウント */
	    work->stay_cnt-- ;
	else if ( !EMA_ThinkCheckEnemy( work ) )
	{
	    /* 回復している */
	    if ( EMA_Flag( EMA_F_IS_RECOVER ) )
		break ;

	    /* 回復する */
	    if ( EMA_Flag( EMA_F_ENB_RECOVER ) &&
		 work->npc.action.life < work->vital_max/4 )
		EMA_SetFlag( EMA_F_IS_RECOVER ) ;

	    //if ( EMA_Flag(EMA_F_IS_ATTACKED) )
	    {
		if ( work->time > 5*10 )
		{
		    EMA_ResetFlag( EMA_F_NEAR_BY_PLY|EMA_F_IS_ATTACKED ) ;
		    work->npc.action.pad = SNIPE_STANDUP ;
		}
	    }

#if 0
	    /* 音声が終ったら */
	    else if ( work->voice_code != work->voice[EMA_VC651] &&
		      work->voice_code != work->voice[EMA_VC671] &&
		      work->voice_code != work->voice[EMA_VC681] )
	    {
		EMA_ResetFlag( EMA_F_NEAR_BY_PLY|EMA_F_IS_ATTACKED ) ;
		work->npc.action.pad = SNIPE_STANDUP ;
	    }
#endif

	}
	break ;

    case SNIPE_IDLE:
	if ( work->stay_cnt > 0 ) /* 強制的に止まるカウント */
	    work->stay_cnt-- ;
	else if ( !EMA_Flag( EMA_F_NON_FACING ) )
	{
	    if ( work->voice_code != work->voice[EMA_VC651] &&
		 work->voice_code != work->voice[EMA_VC671] &&
		 work->voice_code != work->voice[EMA_VC681] )
	    {
		EMA_SetFlag( EMA_F_NON_FACING ) ;
		EMA_ThinkStartMove( work ) ;
	    }
	}
	else if ( work->time > 5*10 && !EMA_ThinkCheckEnemy( work ) )
	    EMA_ThinkStartMove( work ) ;
	if ( EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	    if ( EMA_ThinkCheckWave( work ) )
		if ( work->time>5*10 )
		    work->time = 5*10 ;
	break ;

    case DAMG_SQUAT: /* ダメージモーションは,もう終っているから */
	if ( EMA_Flag( EMA_F_HURT_BY_PLY ) )
	{
	    if ( !EMA_Flag(EMA_F_IS_ATTACKED) )
		EMA_ThinkDamageVoice( work ) ;
	    work->npc.action.pad = SNIPE_SQUAT_IDLE ;
	}
	else
	    work->npc.action.pad = SNIPE_AFRAID ;
	break ;

    case DAMG_IDLE : /* ダメージモーションは,もう終っているから */
	if ( EMA_Flag( EMA_F_HURT_BY_PLY ) )
	{
	    if ( !EMA_Flag(EMA_F_IS_ATTACKED) )
		EMA_ThinkDamageVoice( work ) ;
	    work->npc.action.pad = SNIPE_IDLE ;
	}
	else
	    work->npc.action.pad = SNIPE_SITAFRAID ;
	break ;

    case SNIPE_GO_DOWN:
	EMA_SetFlag( EMA_F_NON_FACING|EMA_F_IS_ON_BRIDGE ) ;
	if ( work->npc.action.act_end )
	    work->npc.action.pad = SNIPE_WALK/*SNIPE_RUN*/ ;
	break ;

    case SNIPE_GO_UP:
	EMA_ResetSetFlag( EMA_F_IS_ON_BRIDGE, EMA_F_NON_FACING ) ;
	if ( work->npc.action.act_end )
	    work->npc.action.pad = SNIPE_RUN  ;
	break ;

    case DAMG_TUMBLE:
    	EMA_SetFlag( EMA_F_NON_FACING ) ;
	if ( work->npc.action.act_end )
	    work->npc.action.pad = DAMG_WAKE ;
	break ;

    case DAMG_WAKE:
    case SNIPE_DOWN_FALL:
    case SNIPE_STANDUP:
    	EMA_SetFlag( EMA_F_NON_FACING ) ;
	if ( work->npc.action.act_end )
	    EMA_ThinkStartMove( work ) ;
	break ;

    case SNIPE_RUN:
    case SNIPE_WALK:
    	EMA_SetFlag( EMA_F_NON_FACING ) ;
	if ( work->time > 4 )
	    EMA_ThinkCheckStop( work ) ;
	EMA_ThinkOnTheRoute( work ) ;

	if ( EMA_Flag( EMA_F_IS_ATTACKED|EMA_F_NEAR_BY_PLY ) )
	{
	    work->voice_id  = SD_V_EMASCR01 ;
	    work->voice_tim = 1 ;
	    work->npc.action.pad = SNIPE_SITAFRAID ;
	}

	/* 橋の上にいる */
	if ( EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	{
	    /* 波の振幅を見る */
	    EMA_ThinkCheckWave( work ) ;

	    /* フェイントしないカウンターはどんどん減らす */
	    if ( !work->tic && work->fake_cnt > 0  )
		work->fake_cnt-- ;

	    /* フェイントアクション(走る) */
	    if ( work->npc.action.set_pad == SNIPE_RUN )
	    {
		if ( !(GM_StagePlayTime&0x3f) )
		{
		    if ( !(irnd()&0x1000) )
			work->npc.action.pad = DAMG_TUMBLE ;/* こける */
		    else
			EMA_SetFlag( EMA_F_ENB_WALK ) ;
		}
	    }

	    /* フェイントアクション(発動) */
	    else if ( !EMA_Flag( EMA_F_ENB_GO_DOWN| EMA_F_ENB_GO_UP ) )
	    {
		if ( !work->fake_cnt )
		     if ( EMA_CheckScreen( work ) )
		     {
			  work->fake_cnt = 10*50 ;
			  if ( !(irnd()&0x1000) )
			       work->npc.action.pad = DAMG_TUMBLE ;/* こける */
			  else
			       EMA_SetFlag( EMA_F_ENB_RUN ) ;/* 走る */
		     }
	    }

	    /* 走り/歩きの変更 */
	    if ( work->npc.action.act_end )
	    {
		if ( EMA_Flag( EMA_F_ENB_WALK ) && work->npc.action.set_pad == SNIPE_RUN  )
		    work->npc.action.pad = SNIPE_WALK, EMA_ResetFlag( EMA_F_ENB_WALK ) ;
		if ( EMA_Flag( EMA_F_ENB_RUN  ) && work->npc.action.set_pad == SNIPE_WALK )
		    work->npc.action.pad = SNIPE_RUN , EMA_ResetFlag( EMA_F_ENB_RUN  ) ;
	    }
	}
	MT_SetMotionSpeed( work->body.m_ctrl, work->walk_speed*TIME_BASE ) ;

	break ;

    default:
	work->npc.action.pad = SNIPE_IDLE ;
	break ;
    }

    /* エマの行動活動停止 */
    if ( EMA_Flag(EMA_F_NON_MOVE) )
	work->npc.action.pad = SNIPE_IDLE ;
}
