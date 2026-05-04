/*
   orga_act.h
   オルガ 覗き行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_peep.h,v 1.1.1.3 2002/11/19 11:46:22 Yoshizawa1 Exp $
*/


/*

  立ちビハインド覗き込み

*/
static void ORG_ActBehindFastPeepingRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActBehindFastPeepingLeftStart( Work *work )
{
    ORG_ActBehindFastPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActBehindSlowPeepingRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_s_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActBehindSlowPeepingLeftStart( Work *work )
{
    ORG_ActBehindSlowPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActBehindPeepingPStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_p, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
static void ORG_ActBehindPeepingEStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_e, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}


/*

  しゃがんでビハインド覗き込み

*/
static void ORG_ActBehindSquatFastPeepingRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_squat_s_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActBehindSquatFastPeepingLeftStart( Work *work )
{
    ORG_ActBehindSquatFastPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActBehindSquatSlowPeepingRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_squat_s_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActBehindSquatSlowPeepingLeftStart( Work *work )
{
    ORG_ActBehindSquatSlowPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActBehindSquatPeepingPStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_squat_p, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
static void ORG_ActBehindSquatPeepingEStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_look_l_squat_e, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}


/*

  立ち覗き込み

*/
static void ORG_ActFastPeepingRightStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_s_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
}
static void ORG_ActFastPeepingLeftStart( Work *work )
{
    ORG_ActFastPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}
static void ORG_ActSlowPeepingRightStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_s_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
}
static void ORG_ActSlowPeepingLeftStart( Work *work )
{
    ORG_ActSlowPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}
static void ORG_ActFastPeepingNoshootRightStart( Work *work )
{
    ORG_ActFastPeepingRightStart( work ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_idle, 0, ORGA_BODY_BOTH_ARMS,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED, ORGA_F_FACETO_ALL ) ;
}
static void ORG_ActFastPeepingNoshootLeftStart( Work *work )
{
    ORG_ActFastPeepingNoshootRightStart( work ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED, ORGA_F_FACETO_ALL ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActSlowPeepingNoshootRightStart( Work *work )
{
    ORG_ActSlowPeepingRightStart( work ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED, ORGA_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_idle, 0, ORGA_BODY_BOTH_ARMS,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
}
static void ORG_ActSlowPeepingNoshootLeftStart( Work *work )
{
    ORG_ActSlowPeepingNoshootRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActPeepingPStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, peep_r_fire_p, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
static void ORG_ActPeepingEStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, peep_r_e, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}


/*

  しゃがみ覗き込み

*/
static void ORG_ActSquatFastPeepingRightStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_squat_s_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
}
static void ORG_ActSquatFastPeepingLeftStart( Work *work )
{
    ORG_ActSquatFastPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}
static void ORG_ActSquatSlowPeepingRightStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_squat_s_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
}
static void ORG_ActSquatSlowPeepingLeftStart( Work *work )
{
    ORG_ActSquatSlowPeepingRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}
static void ORG_ActSquatPeepingPStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, peep_r_squat_p, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
static void ORG_ActSquatPeepingEStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, peep_r_squat_e, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
static void ORG_ActSquatFastPeepingNoshootRightStart( Work *work )
{
    ORG_ActSquatFastPeepingRightStart( work ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED, ORGA_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_idle, 0, ORGA_BODY_BOTH_ARMS,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
}
static void ORG_ActSquatFastPeepingNoshootLeftStart( Work *work )
{
    ORG_ActSquatFastPeepingNoshootRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}
static void ORG_ActSquatSlowPeepingNoshootRightStart( Work *work )
{
    ORG_ActSquatSlowPeepingRightStart( work ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED, ORGA_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_idle, 0, ORGA_BODY_BOTH_ARMS,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
}
static void ORG_ActSquatSlowPeepingNoshootLeftStart( Work *work )
{
    ORG_ActSquatSlowPeepingNoshootRightStart( work ) ;
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
}



/*

  中腰で覗き込み（低い障害物の上から覗き込む）

*/
static void ORG_ActHalfStandPeepingFastStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_u_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_idle, 0, ORGA_BODY_BOTH_ARMS,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1| ORGA_F_ENB_AVOID, 
		ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActHalfStandPeepingSlowStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_u_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_idle, 0, ORGA_BODY_BOTH_ARMS,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1| ORGA_F_ENB_AVOID,
		ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActHalfStandPeepingEStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, peep_u_e, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}


#if 0
static void ORG_ActHalfStandPeepingStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_idle, 0, ORGA_BODY_ALL,
			   work->wait_sec = 120*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->wait_sec /= 2 ;
}
static void ORG_ActHalfStandPeeping( Work *work )
{
    if ( work->wait_sec <= 0 )
	work->act++ ;
}
#endif


