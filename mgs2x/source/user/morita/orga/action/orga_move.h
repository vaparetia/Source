/*
   orga_move.h
   オルガ 移動行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_move.h,v 1.1.1.3 2002/11/19 11:46:22 Yoshizawa1 Exp $
*/



/*

  inline Subroutines

*/
/*この関数にバグあり,ビハインド移動がうまく動かず*/
static float inline CheckPosWhichSide( Work *work, FVECTOR *pos  )
{
    FVECTOR a, b ;
    int r = work->control.turn.vy & 0x0fff ;

    r -= 4096 & (r<<1) ;
    a.vx = sinf( r*(float)M_PI/2048.0F ) ;
    a.vz = cosf( r*(float)M_PI/2048.0F ) ;
    _sceVu0SubVector( &b, pos, &work->control.mov ) ;

    return  a.vx * b.vz - a.vz * b.vx ;
}

static int MoveToward( Work *work, int dist )
{
    float d     ;

    if ( !work->trgt_pos )/* 普通はここに来ないが来ちゃった時のため */
	return  0 ;
    else
    {
	d = VectorSquare( work->trgt_pos, &work->control.mov ) ;

	/* スピード調整（目的地に近いと遅くする） */
	if ( d < dist*2.0f )
	{
	    work->speed = 0.6666666f/(dist*2.0f-ORGA_STOP_DIST) ;
	    work->speed = work->speed * (d - dist*2.0f) + 1.0f ;
	    if ( work->speed < 0.6f )
		work->speed = 0.6f ;
	}

	/* 止まり半径まで来ているか */
	if ( d < ORGA_STOP_DIST )
	{
	    work->n_trgt_his = 0 ;
	    work->body.step->vx = work->body.step->vz = 0.0f ;
	    work->control.mov.vx = work->trgt_pos->vx ;
	    work->control.mov.vz = work->trgt_pos->vz ;
	    work->speed = 1.0f ;

	    return 0 ;
	}
	else if ( FaceAtoB( work, work->trgt_nxt ? work->trgt_nxt : work->trgt_pos,
			    &work->control.mov ) < dist )
	{
	    if ( work->trgt_nxt )
		return -2 ;
	    else if ( ORG_RecalcDir( work->control.turn.vy - work->control.rot.vy ) / 512 )
		return  0 ;
	}
    }

    return 1 ;
}











/*

  Moving ACTs

*/

static void ORG_ActTumbleStart( Work *work )
{
    static FVECTOR offset= { 2.0f, 32.0f, 5190.0f, 1.0f } ;

    CalcCurrentPosAdjToTrgt( work, &offset, 64.0f ) ;
    GM_ConfigObjectAction( &work->body, 0, norm_tumble, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NVR_DAMAGED, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActTumble( Work *work )
{
    if ( work->body.m_ctrl->mt3_ctrl[0].play_time >= 40*5 )
    {
	MoveToward( work, ORGA_RUN_DIST ) ;
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
	{
	    ChangeFlag( work, ORGA_F_NONE, ORGA_F_NVR_DAMAGED ) ;
	    work->act++ ;
	}
    }
}

/* スライディング */
static void ORG_ActSlidingStart( Work *work )
{
    static FVECTOR offset= { 0.000001f, 161.981079f, 3191.802612f, 1.0f } ;

    if ( work->procs[ORGA_P_SLIDING] )
	GCL_ExecProc( work->procs[ORGA_P_SLIDING], NULL ) ;

    CalcCurrentPosAdjToTrgt( work, &offset, 70.0f ) ;
    GM_ConfigObjectAction( &work->body, 0, norm_sliding, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NVR_DAMAGED, ORGA_F_FACETO_ALL ) ;

    work->act++ ;

    //GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;  /* debug code */
}
static void ORG_ActSliding( Work *work )
{
    if ( !(GV_Time & 7) )
	NewSplashParts_Demo( BODYPOS( &work->body, HUMAN21_HIDARI_KAKATO ), &DG_ZeroSVector, 100.0f ) ;
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_NVR_DAMAGED ) ;
	work->act++ ;
    }
}


/* 走り */
static void ORG_ActRunOrDash( Work *work )
{
    int d = MoveToward( work, ORGA_RUN_DIST ) ;

    if ( d == -1 )
    {
	CalcTargetPosAdj( work, work->trgt_pos, 8.0f ) ;
	work->body.step->vx = work->body.step->vz = 0.0f ;
	work->act++ ;
    }
    else if ( d == 0 )
	work->act++ ;
}

/* 飛び込み */
static void ORG_ActRunOrDashToTumble( Work *work )
{
    int d ;

    if ( work->hide_spot && work->hide_hist[0] )
	if ( work->hide_spot->id == work->hide_hist[0]->id )
	{
	    work->act += 4 ;
	    return ;
	}
    d = MoveToward( work, ORGA_TUMBLE_DIST ) ;
    if ( d == 1 )
	work->speed = 1.0f, work->act++ ;
    else if ( d == 0 || d == -1 )
	work->act += 4 ;
}
static void ORG_ActRunOrDashToSliding( Work *work )
{
    int d ;

    if ( work->hide_spot && work->hide_hist[0] )
	if ( work->hide_spot->id == work->hide_hist[0]->id )
	{
	    work->act += 3 ;
	    return ;
	}   
    d = MoveToward( work, ORGA_SLIDING_DIST ) ;
    if ( d == 1 )
	work->speed = 1.0f, work->act++ ;
    else if ( d == 0 )
	work->act += 3 ;
}
static void ORG_ActRunStart( Work *work )
{
    if ( work->flag & ORGA_F_GOT_WORSE ) /* 弱っている */
    {
	GM_ConfigObjectAction( &work->body, 0, pal_run /*pal_walk + work->vitality/16*/, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
	if ( !work->voice_tim && !(irnd() & 0x300) )
	{
	    work->voice = ORGA_VO_TIRED ;
	    work->voice_tim = 60 ;
	}
    }
    else
	GM_ConfigObjectAction( &work->body, 0, norm_run, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActDashStart( Work *work )
{
    if ( work->flag & ORGA_F_GOT_WORSE ) /* 弱っている */
	GM_ConfigObjectAction( &work->body, 0, pal_run /*pal_walk + work->vitality/16*/, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    else
	GM_ConfigObjectAction( &work->body, 0, norm_dash, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

/* かがみ走り */
static void ORG_ActCrowRunStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, norm_dash_lw, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_SAME_HIDE|ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

/* 撃ち走り */
static void ORG_ActAimmingDash( Work *work )
{
    int  d ;

    if ( work->wait_sec > 0 && work->weap_blt > 0 )
    {
	if ( --work->wait_sec == 0 )
	{
	    GM_ConfigObjectAction( &work->body, 1, fire_run, 0, ORGA_BODY_RIGHT_ARM,
				   5*work->act_speed/ORGA_NORMAL_SPEED ) ;
	    _sceVu0CopyVector( &work->weap_usp, work->trgt_aim ) ;
	    work->weap_usp.vw = 1.0f ;
	}
    }
    else if ( GM_CheckObject_IsEnd( &work->body, 1 ) )
    {
	work->wait_sec = 1 + (irnd()&16) ;

	GM_ConfigObjectAction( &work->body, 1, -1, 0, ORGA_BODY_RIGHT_ARM,
			       5*work->act_speed/ORGA_NORMAL_SPEED ) ;
    }

    d = MoveToward( work, ORGA_RUN_DIST ) ;
    if ( d == -1 )
    {
	CalcTargetPosAdj( work, work->trgt_pos, 8.0f ) ;
	work->body.step->vx = work->body.step->vz = 0.0f ;
    }
    if ( d == 0 || d == -1 )
    {
	work->act++ ;
	/* 静止した時に変な方向へ向いてしまうから */
	work->trgt_aim = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    }
}

static void ORG_ActAimmingDashWithRage( Work *work )
{
    if ( work->weap_blt > 0 )
	if ( --work->wait_sec <= 0 )
	{
	    GM_ConfigObjectAction( &work->body, 1, fire_run, 0, ORGA_BODY_RIGHT_ARM,
				   5*work->act_speed/ORGA_NORMAL_SPEED ) ;
	    _sceVu0CopyVector( &work->weap_usp, work->trgt_aim ) ;
	    work->weap_usp.vw = 1.0f ;
	    work->wait_sec = 5 + (irnd()&8) ;
	}
    ORG_ActRunOrDash( work ) ;
}

static void ORG_ActAimmingDashStart( Work *work )
{
    work->wait_sec = 15 ;
    GM_ConfigObjectAction( &work->body, 0, fire_run_p, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    FaceAimAt( work ) ;
    FaceEyeAt( work, work->trgt_aim ) ;
    ChangeFlag( work,
		ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM2 |ORGA_F_OVERLAYED,
		ORGA_F_ENB_AVOID  | ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

static void ORG_ActAimmingHoloDashStart( Work *work )
{
    work->wait_sec = 15 ;
    work->trgt_aim = &ORG_ShootHoloStr3 ;

    GM_ConfigObjectAction( &work->body, 0, fire_run_p, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    FaceEyeAt( work, work->trgt_aim ) ;
    ChangeFlag( work,
		ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM2 |ORGA_F_OVERLAYED,
		ORGA_F_ENB_AVOID  | ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}



/*
  ビハインド移動
*/
static void ORG_ActBehindWalkStart( Work *work )
{
    float f ;

    if ( !work->trgt_pos )
	work->act++ ;
    if ( (f=CheckPosWhichSide( work, work->trgt_pos )) > 0.0f )
    {
	printf( "Behind Walk Left%f rvs%d\n", f, work->body.m_ctrl->flag&MT_FLAG_REVERSAL1 ) ;
	GM_ConfigObjectAction( &work->body, 0, bhid_move_l, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    }
    else
    {
	printf( "Behind Walk Right%f rvs%d\n", f, work->body.m_ctrl->flag&MT_FLAG_REVERSAL1 ) ;
	GM_ConfigObjectAction( &work->body, 0, bhid_move_r, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    }
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_SAME_HIDE|ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActBehindWalk( Work *work )
{
    if ( !work->trgt_pos )
	work->act++ ;
    if ( !(int)(( work->trgt_pos->vz - work->control.mov.vz ) / 64) )
	work->act++ ;
}

/*
  転がり
 */
static void ORG_ActRollStart( Work *work )
{
    if ( !work->trgt_pos )
	work->act++ ;
    if ( work->procs[ORGA_P_SIDE_JUMP] )
	GCL_ExecProc( work->procs[ORGA_P_SIDE_JUMP], NULL ) ;/*プロック実行*/
    if ( CheckPosWhichSide( work, work->trgt_pos ) > 0.0f )
//printf( "Roll left!!!!!\n" ),
	work->weap_usp.vw = -5.0f,/* USPを左手に持つ */
	    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    GM_ConfigObjectAction( &work->body, 0, norm_roll_l, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}


/*

  宙返りアクトセット

 */
static void ORG_ActAirJumpStart( Work *work )
{
#if 0
    static FVECTOR ofst1 = { -184.780396, 340.999969, -2684.136475, 0.0f  } ;/*宙返りのオフセット*/
    static FVECTOR ofst2 = {  198.958313, 340.999969, -2683.041016, 0.0f  } ;
#else
    static FVECTOR ofst1 = {-2623.253667f, 281.447327f, -2.053219f, 0.0f } ;
    static FVECTOR ofst2 = { 2623.253667f, 281.447327f, -2.053219f, 0.0f } ;
#endif
    float d ;
    FMATRIX air_mtx ;

    if ( work->procs[ORGA_P_AIR_JUMP] )
	GCL_ExecProc( work->procs[ORGA_P_AIR_JUMP], NULL ) ;/*プロック実行*/

    work->voice = ORGA_VO_AIRJUMP, work->voice_tim = 4 ;
    GM_ConfigObjectAction( &work->body, 1, -1, 0, ORGA_BODY_UPPER,
			   10*work->act_speed/ORGA_NORMAL_SPEED ) ;
    GM_ConfigObjectAction( &work->body, 0, norm_special, 0, ORGA_BODY_ALL,
			   10*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;

    if ( work->trgt_pos )
    {
#if 0

	d = work->control.rot.vy*M_PI/2048.0f ;
	d += d<-M_PI/2 ? M_PI*2 : 0 ;
	_sceVu0RotMatrixY( &air_mtx, &DG_UnitMatrix,  d ) ;
	_sceVu0ApplyMatrix( &work->pos_adj, &air_mtx,
			    (work->hide_hist[0]->id==1 && work->hide_spot->id==0) ? &ofst1 : &ofst2 ) ;
	_sceVu0AddVector( &work->pos_adj, &work->pos_adj, &work->control.mov ) ;
	_sceVu0SubVector( &work->pos_adj, work->trgt_pos, &work->pos_adj ) ;
	_sceVu0ScaleVector( &work->pos_adj, &work->pos_adj, 1.0f/41.0f ) ; /*このモーションは 41 frm*/
	work->pos_adj.vw = 41.0f ;

#else
	d = work->control.rot.vy*M_PI/2048.0f ;
	d += d<-M_PI/2 ? M_PI*2 : 0 ;
	_sceVu0RotMatrixY( &air_mtx, &DG_UnitMatrix,  d ) ;
	_sceVu0ApplyMatrix( &work->pos_adj, &air_mtx,
			    work->body.m_ctrl->flag & MT_FLAG_REVERSAL1 ? &ofst1 : &ofst2 ) ;
	      //(work->hide_hist[0]->id==1 && work->hide_spot->id==0) ? &ofst1 : &ofst2 ) ;
	_sceVu0AddVector( &work->pos_adj, &work->pos_adj, &work->control.mov ) ;
	_sceVu0SubVector( &work->pos_adj, work->trgt_pos, &work->pos_adj ) ;
	_sceVu0ScaleVector( &work->pos_adj, &work->pos_adj, 1.0f/104.0f ) ; /*このモーションは 41 frm*/
	work->pos_adj.vw = 104.0f ;
#endif
    }

    //GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;  /* debug code */

    work->act++ ;
}



#if 0
/*
  転ぶ アクトおよびアクションセット
 */
static void ORG_ActTrippedStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, damg_tripped, 0, ORGA_BODY_ALL,
			   10*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActTripped( Work *work )
{
    if ( work->body.m_ctrl->mt3_ctrl->time == 11 )
	work->act++ ;
}
static void ORG_ActWakeUpStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, damg_wakeup, 0, ORGA_BODY_ALL,
			   10*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

void (*ORG_ActionTrippedDown[])( Work * ) = {
    ORG_ActTrippedStart, ORG_ActTripped,
    ORG_ActPastOutStart, ORG_ActWaitTwoSecStart, ORG_ActWait, ORG_ActMotionOnceLayer0,
    ORG_ActWakeUpStart, ORG_ActMotionOnceLayer0,
    ORG_ActRunStart, ORG_ActWalk,/*ORG_ActRunOrDash,*/
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
void (**ORG_ActionWakeUpAndRun)( Work * ) = &ORG_ActionTrippedDown[5] ;
#endif
