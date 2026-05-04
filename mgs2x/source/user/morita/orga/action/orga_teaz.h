/*
   orga_move.h
   オルガ 冷やかし行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_teaz.h,v 1.1.1.3 2002/11/19 11:46:23 Yoshizawa1 Exp $
*/

static void ORG_ActTease1Start( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, tease1, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_ENB_AVOID, ORGA_F_FACETO_ALL ) ;

    work->voice     = ORGA_VO_HIDETEASE1 ;
    work->voice_tim = 10                 ;

    work->act++ ;
}

static void ORG_ActTease1( Work *work )
{
    static FVECTOR up_pos = { 0.0f, 1000.0f, 0.0f, 0.0f } ;

    if ( work->body.m_ctrl->mt3_ctrl[0].play_time >= 5*5 )
    {
	if ( work->avoid_flg )
	{
	    work->act = ORG_ActionAbort ;
	    AN_HeadMark( &BODYWORLD( &work->body, HUMAN21_ATAMA ), 0 ) ;
	    work->voice = ORGA_VO_FOUND, work->voice_tim = 4 ;
	}
	else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0,  98*5 ) ||
		  MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 113*5 ) ||
		  MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 148*5 ) )
	    _sceVu0AddVector( &work->weap_usp, BODYPOS(&work->body, HUMAN21_MIGI_TE), &up_pos ),
		work->weap_usp.vw = 1.0f ;
	else
	    ORG_ActMotionOnceLayer0( work ) ;
    }    
}

static void ORG_ActTease2Start( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, tease2, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

static void ORG_ActTeaseSpeakFastSquatRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_squat_s_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_STOP_STILL, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActTeaseSpeakFastSquatLeftStart( Work *work )
{
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    ORG_ActTeaseSpeakFastSquatRightStart( work ) ;
}

static void ORG_ActTeaseSpeakFastRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_STOP_STILL, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActTeaseSpeakFastLeftStart( Work *work )
{
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    ORG_ActTeaseSpeakFastRightStart( work ) ;
}

static void ORG_ActTeaseSpeakFast( Work *work )
{
    if ( work->body.m_ctrl->mt3_ctrl[0].play_time >= 19*5 )
	work->act++ ;
}

static void ORG_ActTeaseStillFast( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_fast, 19*5, ORGA_BODY_ALL, 0 ) ;
    work->control.step.vx = work->control.step.vy = work->control.step.vz = 0.0f ;
    if ( !(work->flag & ORGA_F_STOP_STILL) )
    {
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_NVR_RESET ) ;
	work->act_flg = 0 ;
	work->act++ ;
    }
}


static void ORG_ActTeaseSpeakSlowSquatRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_squat_s_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_STOP_STILL, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActTeaseSpeakSlowSquatLeftStart( Work *work )
{
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    ORG_ActTeaseSpeakSlowSquatRightStart( work ) ;
}

static void ORG_ActTeaseSpeakSlowRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_STOP_STILL, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActTeaseSpeakSlowLeftStart( Work *work )
{
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    ORG_ActTeaseSpeakSlowRightStart( work ) ;
}
static void ORG_ActTeaseSpeakSlow( Work *work )
{
    if ( work->body.m_ctrl->mt3_ctrl[0].play_time >= 29*5 )
	work->act++ ;
}
static void ORG_ActTeaseStillSlow( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_slow, 29*5, ORGA_BODY_ALL, 0 ) ;
    work->control.step.vx = work->control.step.vy = work->control.step.vz = 0.0f ;
    if ( !(work->flag & ORGA_F_STOP_STILL) )
    {
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_NVR_RESET ) ;
	work->act_flg = 0 ;
	work->act++ ;
    }
}


/* 人生語り用アクト */
static void ORG_ActTeaseSpeakStream( Work *work )
{
    ORG_RecogStartStream( work ) ;
    work->act++ ;
}
static int ORG_TeaseStopStream( Work *work, int proc_id )
{
    if ( !work->str_hdl ||
	 ((GV_PadDataDirect[ 0 ].release & PAD_DEMO_CANCEL) && (work->flag & ORGA_F_DEMO_SKIP)) )
    {
	work->act_flg = 0 ;
	work->flag &= ~(ORGA_F_DEMO_SKIP|ORGA_F_DEMO_MOVIE) ;
	work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH ;

	if ( proc_id )
	    GCL_ExecProc( proc_id, NULL ) ;/*語り 最初プロック実行*/
	work->voice_vox += 0x1000 ;
	work->voice_vox &= 0xf000 ;
	work->act = ORG_ActionAbort ;
	return 1 ;
    }
    return 0 ;
}
static void ORG_ActTeaseStillStreamFirst( Work *work )
{
    ORG_TeaseStopStream( work, work->procs[ORGA_P_BIBLIO_FST1] ) ;
}
static void ORG_ActTeaseStillStreamWait( Work *work )
{
    if ( !ORG_TeaseStopStream( work, work->procs[ORGA_P_BIBLIO_FST1] ) )
	if ( --work->wait_sec <=0  )
	    work->act++ ;
}
static void ORG_ActTeaseMotionOnce( Work *work )
{
    if ( !ORG_TeaseStopStream( work, work->procs[ORGA_P_BIBLIO_FST1] ) )
	if ( GM_CheckObject_IsEnd( &work->body, (work->flag&ORGA_F_OVERLAYED ? 1 : 0) ) )
	    work->act++ ;
}


static void ORG_ActTeaseSpeakStreamLast( Work *work )
{
    if ( work->body.m_ctrl->mt3_ctrl[0].play_time >= 29*5 )
    {
	work->body.flag |= OBJECT_MOTIONSTEP_THROUGH ;
	ORG_ActTeaseSpeakStream( work ) ;
    }
}
static void ORG_ActTeaseStillStreamLast( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_slow, 29*5, ORGA_BODY_ALL, 0 ) ;
    work->control.step.vx = work->control.step.vy = work->control.step.vz = 0.0f ;

    if ( ORG_TeaseStopStream( work, work->procs[ORGA_P_BIBLIO_LST1] ) )
	if ( !work->str_hdl )
	{
      int pan;
      float bp_angle;
      pan = GM_SeGetPan( &ORGA_PLY_CAMERAPOS, GM_SEMODE_BOMB, &bp_angle );
	    GM_SeSet3D( pan, GM_MAX_VOL, SD_A_THUNDR01, bp_angle ) ;
	    GM_SeSetMode( SD_A_THUNDR01, &ORGA_PLY_CAMERAPOS, GM_SEMODE_BOMB ) ;
	}
}

/* 

   マガジンなげの反応

*/
static void ORG_ActOnceMoreMagazineStart( Work *work )
{
    if ( work->act_flg & ORGA_F_MAGAZIN_ATTK )
    {
	work->wait_sec = 240-1 ;
	work->act++ ;
    }
    else
	work->act = ORG_ActionAbort ;
}
static void ORG_ActEndOfMagazineStart( Work *work )
{
    work->trgt_nse.vw = -1.0f ;
    work->act++ ;
}
