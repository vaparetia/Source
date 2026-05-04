/*
  thk_afraid.h
  舟虫用 思考処理

  2001/06/14 T.Morita Revised
  $Id: thk_slater.h,v 1.1.1.3 2002/11/19 11:46:05 Yoshizawa1 Exp $
*/





static int EMA_ThinkSetHideZone( Work *work )
{
    int      addr ;
    float    radius ;
    int EMA_GetNearestHide( Work *work, float *min ) ;
    int idx ;

    /* 再生＆補間スピードを元に戻す */
    MT_SetMotionSpeed( work->body.m_ctrl, -1.0f ) ;
    //work->control.interp = 0 ;

    if ( (idx = EMA_GetNearestHide( work, &radius )) >= 0 )
	if ( radius > 1000.0f*1000.0f )
	{
	    addr  = (work->hide_g[idx] << 16) & 0xffff0000 ;
	    addr |= (work->hide_a[idx] << 8 ) & 0x0000ff00 ;
	    addr |= (work->hide_a[idx] << 0 ) & 0x000000ff ;

	    if ( EMA_SetNaviTarget( work, addr, &work->hide[idx] ) )
		return 1 ;
	}	
    return 0 ;
}

/*

  舟虫が近くにいるかどうか

*/
static int EMA_ThinkCheckSlater( Work *work )
{
#if 1
    /* コールドスプレーがあるから こっちでなきゃ駄目 */
    extern float GetEmmSsltrNearLen2( FVECTOR *pos ) ;

    if ( GetEmmSsltrNearLen2( &work->control.mov ) < 1000.0f*1000.0f )
#else
    if ( work->stage == EMA_STG_W31B )
	if ( work->control.mov.vx > -1550.0f   &&
	     work->control.mov.vx <  3700.0f   &&
	     work->control.mov.vy > -5000.0f   &&
	     work->control.mov.vy <     0.0f   &&
	     work->control.mov.vz > -253160.0f &&
	     work->control.mov.vz < -250000.0f )
#endif
	{
	    *work->ext_trg = 0 ;

	    if ( EMA_ThinkSetHideZone( work ) )
	    {
		printf( "Emma : I hate SLATER!!!\n" ) ;
		return 1 ;
	    }
	}
    return 0 ;
}

static void EMA_ThinkEscapeFromSlater( Work *work )
{
    int id ; 

    work->npc.action.pad = PAL_DISLIKE ;
    EMA_SetFlag( EMA_F_RUN_AWAY ) ;

    id = (GV_Time & 1 ? EMA_VC451 :           
	  irnd()  & 1 ? EMA_VC461 : EMA_VC471) ;
    EMA_UtilStartStreamIdx( work, id ) ;
}

static void EMA_ThinkSlaterMove( Work *work )
{
#if 0
    FVECTOR v   ;

    /* 他人に当りそうになったら止める */
    if ( EMA_NearEneControl( work, 600.0f, &v ) )
    {
	work->npc.action.pad = PAL_IDLE ;
	EMA_ResetFlag( EMA_F_RUN_AWAY ) ;
    }
    else if ( GM_Navi( &work->navigate, &work->navitrg, 500 ) )
#endif

    if ( EMA_Navi( work ) )
    {
	 /* nvtrgに向ってゾーン移動 */
	work->npc.action.pad = PAL_IDLE ;
	EMA_ResetFlag( EMA_F_RUN_AWAY ) ;
	EMA_SetAdjustPosition( work, &work->navitrg.pos, 20.0f ) ;
    }
    else
    {
	/* 移動方向指定 */
	work->npc.action.dir = work->navigate.next_dir ;
	//work->npc.action.pad = RAI_WALK_CORPS ;
	work->npc.action.pad = AFRAID_WALK ;
    }
}
