/*
   orga_fire.h
   オルガ 撃ちアクトセット
   2000/01/13 T.Morita
   $Id: orga_fire.h,v 1.1.1.3 2002/11/19 11:46:22 Yoshizawa1 Exp $
*/



/*

  攻撃用汎用関数

 */

static inline void UnrecogAction( Work *work )
{
    work->head_mark = HMK2_TYPE_WHT_QE ; /* ？マーク */
#if 0
    work->voice =
	irnd() & 0x10 ? ORGA_VO_HIDETEASE0 :
	irnd() & 0x10 ? ORGA_VO_HIDETEASE1 : ORGA_VO_HIDETEASE2 ;
    work->voice_tim = 30 ;
#endif
}

static void FireRecogniton( Work *work, int peep )
{
    if ( (work->flag & ORGA_F_UNRECOG_AIM && !(work->act_flg & ORGA_F_MAGAZIN_ATTK)) ||
	 (work->act_flg & ORGA_F_MAGAZIN_ATTK) )
    {
	work->trgt_aim_pos = *ORG_RecogPlayerPos( HUMAN21_KUBI ) ;
	work->trgt_aim = &work->trgt_aim_pos ;
	if ( !peep ) 
	    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
	FaceEyeAt( work, work->trgt_aim ) ;

	work->wait_sec = 240 ;

	UnrecogAction( work ) ;
    }
    else
    {
	FaceAimAt( work ) ;
	if ( !peep ) 
	    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
	FaceEyeAt( work, work->trgt_aim ) ;
    }
    /*	printf( "FireRecog  AIM%x %f %f %f\n", work->trgt_aim,
		work->trgt_aim->vx, work->trgt_aim->vy, work->trgt_aim->vz  ) ;	*/
}




/*

  ほとんどすべての撃ち（特殊モーション以外）はここを通る

*/
void ORG_ActFireP( Work *work )
{
    static char dir[32] = {
	 124,  117,  105,   89,   70,   48,   24,    0,
 	   0,  -24,  -48,  -70,  -89, -105, -117, -124,
	-124, -117, -105,  -89,  -70,  -48,  -24,    0,
	   0,   24,   48,   70,   89,  105,  117,  124,
    } ;

    if ( (work->flag & ORGA_F_UNRECOG_AIM) || (work->act_flg & ORGA_F_MAGAZIN_ATTK) )
	work->trgt_aim_pos.vz += dir[(work->wait_sec>>2)&31] ;
    else if ( work->avoid_flg && !(work->flag & ORGA_F_NOMISS_SHOT) )
    {
	work->act = ORG_ActionAbort ;
	return  ;
    }
    if ( --work->wait_sec <= 0 )
	work->act++ ;
}

static void ORG_ActFireAimmingShot( Work *work )
{
#if 0
    printf( "May Be %.2f %.2f %.2f %.2f\n", work->trgt_aim->vx,work->trgt_aim->vy,work->trgt_aim->vz,
	    work->trgt_aim_pos.vw ) ;

    if ( work->trgt_aim_pos.vw != 0.0f  )
    {
	if ( !(work->flag & ORGA_F_NOMISS_SHOT) &&
	     (work->trgt_aim_pos.vw >= 1000.0f || work->trgt_aim_pos.vw == 1.0f )  )
	{
#if 0
	    _sceVu0AddVector( &work->trgt_aim_pos, 
			      ORG_RecogPlayerPos( HUMAN21_HIDARI_TSUMASAKI ),
			      ORG_RecogPlayerPos( HUMAN21_MIGI_TSUMASAKI   ) ) ;
	    _sceVu0ScaleVector( &work->trgt_aim_pos,  &work->trgt_aim_pos, 0.5f ) ;
	    work->trgt_aim_pos.vx += frnd() * ORGA_MISS_SHOT_RAD ;
	    work->trgt_aim_pos.vz += frnd() * ORGA_MISS_SHOT_RAD ;
#else
	    //work->trgt_aim_pos = GM_PlayerControl->mov ;
	    work->trgt_aim_pos = *ORG_RecogPlayerPos( HUMAN21_ATAMA ) ;
#endif
	}
	work->act++ ;
    }
    else
	work->trgt_aim_pos.vw = 1.0f ;
#else
    if ( !(work->flag & ORGA_F_UNRECOG_AIM) )
	FaceAimAt( work ) ;
    work->act++ ;
#endif
}


/*

  しゃがみ撃ち

 */
static void ORG_ActSquatFirePStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 1, fire_p, 0, ORGA_BODY_UPPER,
			   work->wait_sec*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM|
		ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActSquatFireStart( Work *work )
{
    if ( !(work->flag & ORGA_F_UNRECOG_AIM)   && 
	 !(work->aim_flg & ORGA_F_DONT_SHOOT) && work->weap_blt > 0 )
    {
	_sceVu0CopyVector( &work->weap_usp, ORG_RecogAimPos( work ) ) ;
	work->weap_usp.vw = 1.0f ;
	GM_ConfigObjectAction( &work->body, 1, fire, 0, ORGA_BODY_UPPER,
			       2*work->act_speed/ORGA_NORMAL_SPEED ) ;
	ChangeFlag( work,
		    ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		    ORGA_F_FACETO_ALL ) ;
	FireRecogniton( work, 0 ) ;
    }
    else
	work->rage++ ;
    work->act++ ;
}




/*

 立ち撃ち

 */
static void ORG_ActStandFirePStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_p, 0, ORGA_BODY_ALL,
			   work->wait_sec*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActStandFireStart( Work *work )
{
    if ( !(work->flag & ORGA_F_UNRECOG_AIM) && work->weap_blt > 0 )
    {
	_sceVu0CopyVector( &work->weap_usp, ORG_RecogAimPos( work ) ) ;
	work->weap_usp.vw = 1.0f ;
	GM_ConfigObjectAction( &work->body, 0, fire, 0, ORGA_BODY_ALL,
			       2*work->act_speed/ORGA_NORMAL_SPEED ) ;
	ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    }
    else
	work->rage++ ;
    work->act++ ;
}




/*

  覗き込み撃ち

*/
static void ORG_ActPeepingFirePStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_fire_p, 0, ORGA_BODY_ALL,
			   work->wait_sec/30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
    ChangeFlag( work,
		ORGA_F_ENB_AIM| ORGA_F_ENB_AVOID| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
}
static void ORG_ActSquatPeepingFirePStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_squat_p, 0, ORGA_BODY_ALL,
			   work->wait_sec/30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
    ChangeFlag( work,
		ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
}

static void ORG_ActPeepingFireStart( Work *work )
{
    if ( !(work->flag & ORGA_F_UNRECOG_AIM)   &&
	 !(work->aim_flg & ORGA_F_DONT_SHOOT) && work->weap_blt > 0 )
    {
	FireRecogniton( work, 0 ) ;
	_sceVu0CopyVector( &work->weap_usp, ORG_RecogAimPos( work ) ) ;
	work->weap_usp.vw = 1.0f ;
        GM_ConfigObjectAction( &work->body, 1, peep_r_fire, 0, ORGA_BODY_UPPER,
                               2*work->act_speed/ORGA_NORMAL_SPEED ) ;
        ChangeFlag( work,
		    ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		    ORGA_F_FACETO_ALL ) ;
    }
    else
	work->rage++ ;

    work->act++ ;
}
static void ORG_ActPeepingFire( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
        GM_ConfigObjectAction( &work->body, 1, -1, 0, ORGA_BODY_UPPER,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
        ChangeFlag( work, ORGA_F_NONE, ORGA_F_OVERLAYED ) ;
	work->act++ ;
    }
}





/*

  ビハインドから飛びだし撃ち

*/
static void ORG_ActBehindPeepingFireRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_fire_l_s, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID|ORGA_F_ENB_AIM ) ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
    work->act++ ;
}
static void ORG_ActBehindPeepingFireLeftStart( Work *work )
{
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    ORG_ActBehindPeepingFireRightStart( work ) ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}
static void ORG_ActBehindSquatPeepingFireRightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_fire_l_squat_s, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID|ORGA_F_ENB_AIM ) ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
    work->act++ ;
}
static void ORG_ActBehindSquatPeepingFireLeftStart( Work *work )
{
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    ORG_ActBehindSquatPeepingFireRightStart( work ) ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}

static void ORG_ActBehindPeepingFireERightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_fire_l_e, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->control.rot.vy = work->control.turn.vy -= /*work->body.m_ctrl->rot_correct*/2027 ;
    work->act++ ;
}
static void ORG_ActBehindPeepingFireELeftStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_fire_l_e, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->control.rot.vy = work->control.turn.vy += /*work->body.m_ctrl->rot_correct*/1875 ;
    work->act++ ;
}
static void ORG_ActBehindSquatPeepingFireERightStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_fire_l_squat_e, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->control.rot.vy = work->control.turn.vy -= /*work->body.m_ctrl->rot_correct*/2130 ;
    work->act++ ;
}
static void ORG_ActBehindSquatPeepingFireELeftStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, bhid_fire_l_squat_e, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->control.rot.vy = work->control.turn.vy += /*work->body.m_ctrl->rot_correct*/1744 ;
    work->act++ ;
}

static void ORG_ActBehindStandFirePRightStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_p, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
#if 0
    work->control.rot.vy = work->control.turn.vy += /*work->body.m_ctrl->rot_correct*/2027 ;
#endif
    work->act++ ;
}
static void ORG_ActBehindSquatFirePRightStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 1, fire_p, 0, ORGA_BODY_UPPER,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_squat, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_OVERLAYED| ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
#if 0
    work->control.rot.vy = work->control.turn.vy += /*work->body.m_ctrl->rot_correct*/2130 ;
#endif
    work->act++ ;
}

static void ORG_ActBehindStandFirePLeftStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_p, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
#if 0
    work->control.rot.vy = work->control.turn.vy -= /*work->body.m_ctrl->rot_correct*/1875 ;
#endif
    work->act++ ;
}
static void ORG_ActBehindSquatFirePLeftStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 1, fire_p, 0, ORGA_BODY_UPPER,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_squat, 0, ORGA_BODY_ALL,
			   0*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_OVERLAYED| ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
#if 0
    work->control.rot.vy = work->control.turn.vy -= /*work->body.m_ctrl->rot_correct*/1744 ;
#endif
    work->act++ ;
}


/*

  立ち覗き込み撃ち

  */
static void ORG_ActFastStandingFireStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_u_fast, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_ENB_AVOID| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActSlowStandingFireStart( Work *work )
{
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_u_slow, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_ENB_AVOID| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActStandingFireEStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, peep_u_e, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}







/*

  迷い撃ち

  */
static void ORG_ActUnrecogPeepingStandStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_lost, 0, ORGA_BODY_ALL,
			   work->wait_sec/30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActUnrecogPeepingSquatStart( Work *work )
{
    FireRecogniton( work, 1 ) ;
    GM_ConfigObjectAction( &work->body, 0, peep_r_squat_lost, 0, ORGA_BODY_ALL,
			   work->wait_sec/30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM, ORGA_F_NONE ) ;
    work->act++ ;
}
void ORG_ActUnrecogFireP( Work *work )
{
    extern void (*ORG_ActionFastZeroSecPeepingTripleFireLeft[])( Work * ) ;
    extern void (*ORG_ActionSquatFastZeroSecPeepingTripleFireLeft[])( Work * ) ;

    if ( !(work->flag & ORGA_F_UNRECOG_AIM) )
    {
	work->trgt_aim = ORG_RecogPlayerPos( HUMAN21_ATAMA ) ;
	FaceEyeAt( work, work->trgt_aim ) ;
	work->wait_sec = 60 ;
	work->act = work->stll_mtn & ORGA_F_IS_SQUAT ?
            &ORG_ActionSquatFastZeroSecPeepingTripleFireLeft[3] :
	    &ORG_ActionFastZeroSecPeepingTripleFireLeft[3]  ;
        //ChangeFlag( work, ORGA_F_FACETO_HEAD, ORGA_F_FACETO_ALL ) ;
    }
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 2*5 ) )
	UnrecogAction( work ) ;
    else if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
	work->act++ ;
}

/*

  エルード撃ち

*/
#if 0
static void ORG_ActErudeFire( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	if ( work->weap_blt <= 0 )
	    work->act++ ;
	else if ( GM_PlayerStatus & PLAYER_BEYOND )
	    work->act-- ;
	else
	    work->act++ ;
    }
}
#else

static void ORG_ActErudeFirePStart( Work *work )
{
    static FVECTOR offset = { -116.443649f-12.617148f, 0.0f, 87.511169f+22.488947f, 0.0f } ;

    work->voice     = GV_Time&1 ? ORGA_VO_ERUDE0 : ORGA_VO_ERUDE1 ;
    work->voice_tim = 120 ;
    work->trgt_aim = work->trgt_eye = &GM_PlayerControl->mov ;
    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    CalcCurrentPosAdj( work, &offset, 10.0f ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_anti_erude_p, 0, ORGA_BODY_ALL, ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_NOMISS_SHOT| ORGA_F_ENB_AIM| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActErudeFireStart( Work *work )
{
    _sceVu0CopyVector( &work->weap_usp, work->trgt_aim ) ;
    work->weap_usp.vw = 1.0f ;
    GM_ConfigObjectAction( &work->body, 0, fire_anti_erude, 0, ORGA_BODY_ALL,
			   work->wait_sec/30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
static void ORG_ActErudeFire( Work *work )
{
    static FVECTOR offset = { 116.443649f+12.617148f, 0.0f, -87.511169f-22.488947f, 0.0f } ;

    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	if ( GM_PlayerStatus & PLAYER_BEYOND && work->weap_blt > 0 )
	    work->act-- ;
	else
	{
	    work->voice_tim = 0 ; /* 止めたら音声は要らない */
	    CalcCurrentPosAdj( work, &offset, 30.0f ) ;
	    work->act++ ;
	}
    }
}

#endif






/*

  リロード

 */
#if 0
static void ORG_ActMotionOnceTest( Work *work )
{
    extern FVECTOR air_test ;
    extern FMATRIX air_inv ;
    extern FVECTOR air_pos ;
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	printf( "AirEnd%f %f %f\n", work->control.mov.vx, work->control.mov.vy, work->control.mov.vz ) ; 
	printf( "AirPOS%f %f %f = ", air_pos.vx,air_pos.vy,air_pos.vz ); 
	_sceVu0SubVector( &air_pos, &work->control.mov, &air_pos ) ;
	printf( "diff%f %f %f\n", air_pos.vx,air_pos.vy,air_pos.vz ); 
	_sceVu0SubVector( &air_test, &work->control.mov, &air_test ) ;
	_sceVu0ApplyMatrix( &air_test, &air_inv, &air_test ) ;
	printf( "AirOFFSET%f %f %f\n", air_test.vx, air_test.vy, air_test.vz ) ;
	work->act++ ;
    }
}
#endif

#if 0
static void (*ORG_ActionAirJump[])( Work * ) = {
    ORG_ActBehindToTargetStart, ORG_ActWaitFacing,
    ORG_ActAirJumpStart, ORG_ActMotionOnceLayer0 /*ORG_ActMotionOnceTest*/,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
#else
static void (*ORG_ActionAirJump[])( Work * ) = {
    ORG_ActFaceToWallStart, ORG_ActWaitFacing,
    ORG_ActAirJumpStart, ORG_ActMotionOnceLayer0 /*ORG_ActMotionOnceTest*/,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
#endif

static void ORG_ActReloading( Work *work )
{
    int layer = work->flag & ORGA_F_OVERLAYED ? 1 : 0 ;

    if ( work->avoid_flg )
	work->act = ORG_ActionAirJump ;
    else
    {
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, layer, 29*5 ) )
	    work->weap_amo = -4 ; /* show drop cartridge */
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, layer, 30*5 ) )
	    work->weap_amo = -3 ; /* show insert cartridge */
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, layer, 67*5 ) )
	{
	    if ( work->flag & ORGA_F_DEMO_MOVIE )
		GM_SeSetMode( ORGA_SE_USP_RELOAD, &work->control.mov, GM_SEMODE_BOMB ) ; 
	}
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, layer, 68*5 ) )
	    work->weap_amo = -2 ; /* hide cartridge */
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, layer, 80*5 ) )
	{
	    work->weap_blt = work->weap_max ;

	    if ( work->flag & ORGA_F_DEMO_MOVIE )
		GM_SeSetMode( ORGA_SE_USP_BLOWBK, &work->control.mov, GM_SEMODE_BOMB ) ; 
	}
	if ( GM_CheckObject_IsEnd( &work->body, layer ) )
	{
	    FaceEyeAt( work, work->trgt_aim ) ;/* デフォルトに戻す */
	    ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
	    work->act++ ;
	}
    }
}

static void ORG_ActStandReloadStart( Work *work )
{

#if 0
    FaceEyeAt( work, BODYPOS( &work->body, HUMAN21_MIGI_TE) ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_ENB_SAFE, ORGA_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_reload, 0, ORGA_BODY_ALL,
    			     30*work->act_speed/ORGA_NORMAL_SPEED ) ;
#else
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_reload, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
#endif
    work->act++ ;
}

static void ORG_ActSquatReloadStart( Work *work )
{
    FaceEyeAt( work, BODYPOS( &work->body, HUMAN21_MIGI_TE) ) ;
    ChangeFlag( work, ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_ENB_SAFE, ORGA_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 1, stll_reload, 0, ORGA_BODY_UPPER,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}




/*

  転がり移動撃ち

*/
static void ORG_ActSquatRollFirePStart( Work *work )
{
    ORG_ActSquatFirePStart( work ) ;
    ChangeFlag( work, ORGA_F_ENB_AVOID, ORGA_F_NONE ) ;
}
static void ORG_ActRollFireP( Work *work )
{
    if ( work->avoid_flg )
    {
	ChangeFlag( work, ORGA_F_NVR_DAMAGED, ORGA_F_NONE ) ;
	work->act = ORG_ActionAirJump ;
    }
    if ( --work->wait_sec <= 0 )
	work->act++ ;
}
static void ORG_ActSquatRollFireStart( Work *work )
{
    static void (*ORG_ActionRollReload[])( Work * ) = {
	ORG_ActSquatReloadStart, ORG_ActReloading, 
	ORG_ActRunStart, ORG_ActRunOrDash,
	ORG_ActStillStart, ORG_ActThink, NULL
    } ;
    if ( GM_IsGameOver() )
    {
	work->act = ORG_ActionAbort ;
	return ;
    }
    if ( work->avoid_flg )
    {
	ChangeFlag( work, ORGA_F_NVR_DAMAGED, ORGA_F_NONE ) ;
	work->act = ORG_ActionAirJump ;
    }
    else if ( work->weap_blt <= 0 )
	work->act = ORG_ActionRollReload ;
    else
	ORG_ActSquatFireStart( work ) ;
}
static void ORG_ActRollMotionOnce( Work *work )
{
    if ( work->avoid_flg )
    {
	ChangeFlag( work, ORGA_F_NVR_DAMAGED, ORGA_F_NONE ) ;
	work->act = ORG_ActionAirJump ;
    }
    else if ( GM_CheckObject_IsEnd( &work->body, 1 ) )
    {
	GM_ConfigObjectAction( &work->body, 1, -1, 0, ORGA_BODY_UPPER,
			       10*work->act_speed/ORGA_NORMAL_SPEED ) ;
	work->act++ ;
    }
}



/*

  ホロ撃ち開き

  */
static void ORG_ActHoloFireStart( Work *work )
{
    _sceVu0CopyVector( &work->weap_usp, ORG_RecogAimPos( work ) ) ;
    work->weap_usp.vw = 1.0f ;
    GM_ConfigObjectAction( &work->body, 0, fire, 0, ORGA_BODY_ALL,
			   2*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

static void ORG_ActAimToCutHoloStart( Work *work )
{
    FaceAtoB( work, &ORG_ShootHoloStr1, &work->control.mov ) ;
    work->trgt_aim = &ORG_ShootHoloStr1 ;
    FaceEyeAt( work, &ORG_ShootHoloStr1 ) ;

    if ( /*work->trgt_pos == &ORG_ShootHoloPos &&*/ work->procs[ORGA_P_HOLO_FIRE] )
	GCL_ExecProc( work->procs[ORGA_P_HOLO_FIRE], NULL ) ;

    GM_ConfigObjectAction( &work->body, 0, fire_p, 0, ORGA_BODY_ALL,
			   work->wait_sec*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;

    /* なくならないように補充 */
    work->weap_blt += 2 ;

    work->wait_sec = 60 ;
    work->act++ ;
}

static void ORG_ActAimToCutNextStart( Work *work )
{
    work->trgt_aim = &ORG_ShootHoloStr2 ;
    FaceEyeAt( work, &ORG_ShootHoloStr2 ) ;

    if ( work->misc_holo_stat )
	ORG_SendHoloMessage( 1 ) ;

    GM_ConfigObjectAction( &work->body, 0, fire_p, 0, ORGA_BODY_ALL,
			   work->wait_sec*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->wait_sec = 15 ;
    work->act++ ;
}


/*

  ホロ撃ち飛ばし（前側撃ち）

  */
static void ORG_ActBlowOffHoloStart( Work *work )
{
    if ( work->misc_holo_stat ? (*work->misc_holo_stat != ORGA_HOL_PLY_FREL &&
				 *work->misc_holo_stat != ORGA_HOL_PLY_FRER ) : 1 )
	work->act = ORG_ActionAbort ;
    else
    {
	FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
	FaceEyeAt( work, work->trgt_aim ) ;

	GM_ConfigObjectAction( &work->body, 1, fire_p, 0, ORGA_BODY_UPPER,
			       work->wait_sec*work->act_speed/ORGA_NORMAL_SPEED ) ;
	ChangeFlag( work,
		    ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM|
		    ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		    ORGA_F_FACETO_ALL ) ;

	work->wait_sec = 45 * 5 / TIME_BASE ;
	work->act++ ;
    }
}

static void ORG_ActBlowOffFireStart( Work *work )
{
    _sceVu0CopyVector( &work->weap_usp, work->trgt_aim ) ;
    work->weap_usp.vw = 1.0f ;
    GM_ConfigObjectAction( &work->body, 1, fire, 0, ORGA_BODY_UPPER,
			   2*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_OVERLAYED| ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}

static void ORG_ActBlowOffFire( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 1 ) )
    {
	/* 静止した時に変な方向へ向いてしまうから */
	work->trgt_aim = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;

	work->act++ ;
    }
}


/*

  スポットライト撃ち

  */
static void ORG_ActAimToSpotLightStart( Work *work )
{
    work->trgt_aim = &ORG_ShootSpotLgt ;
    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    FaceEyeAt( work, work->trgt_aim ) ;

    GM_ConfigObjectAction( &work->body, 0, fire_p, 0, ORGA_BODY_ALL,
			   work->wait_sec*work->act_speed/ORGA_NORMAL_SPEED ) ;

    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->wait_sec = 60 ;
    work->act++ ;
}

static void ORG_ActFireToSpotLightStart( Work *work )
{
    ORG_SendSpotLightFlareMessage() ;

    ORG_ActStandFireStart( work ) ;
}

static void ORG_ActFireToSpotLight( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	work->trgt_aim = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
	work->act++ ;
    }
}


/*

  グレネード投げ

 */
static void ORG_ActThroughGrenadeStart( Work *work )
{
    if ( work->procs[ORGA_P_GRENADE] )
	GCL_ExecProc( work->procs[ORGA_P_GRENADE], NULL ) ;

    work->wait_sec = 0 ;/* 一箇所にしか投げない */
    FireRecogniton( work, 0 ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_throw_l, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_ENB_AVOID  | ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    work->act++ ;
}

static void ORG_ActThroughGrenade( Work *work )
{
    FVECTOR d ;

    if ( (work->body.m_ctrl->mt3_ctrl[0].play_time < 16*69 ||
	  work->body.m_ctrl->mt3_ctrl[0].play_time > 16*280) &&
	 !(work->flag & ORGA_F_UNRECOG_AIM) && work->wait_sec > 0 )
    {
	work->act = ORG_ActionAbort ;
	work->wait_sec = 0 ;
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;
    }

    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 25*5 ) )
	work->weap_usp.vw = -6.0f ; /* USPを腰にしまう */
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 69*5 ) )
	work->weap_sgr.vw = -3.0f ; /* SGRを取り出す（右手） */
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 125*5 ) )
	//GM_SeSetMode( ORGA_SE_SGR_PIN, &work->control.mov, GM_SEMODE_BOMB )/*ピンの抜き音*/
	;
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 200*5 ) )
    {
	work->voice = work->wait_sec==2 ? ORGA_VO_THROUGH_SGR2 : 
		      work->wait_sec==1 ? ORGA_VO_THROUGH_SGR1 : ORGA_VO_THROUGH_SGR0 ;
	work->voice_tim = 4 ;
	ChangeFlag( work, ORGA_F_NVR_RESET, ORGA_F_NONE ) ;
    }
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 224*5 ) )
    {
	_sceVu0SubVector( &d, &work->control.mov, work->trgt_aim ) ;
	_sceVu0Normalize( &work->weap_sgr, &d ) ;
	_sceVu0ScaleVector( &work->weap_sgr, &work->weap_sgr, 900.0f ) ;
	_sceVu0AddVector( &work->weap_sgr, &work->weap_sgr, work->trgt_aim ) ;
	work->weap_sgr.vw = sceVu0Sqrt( d.vx*d.vx+d.vz*d.vz )/ 120.0f ;
    }
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 280*5 ) )
    {
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_NVR_RESET ) ;
	if ( --work->wait_sec >= 0 )
	{
	    if ( work->wait_sec == 0 )
		work->trgt_aim = work->trgt_eye = &GM_PlayerControl->mov ;
	    else
		work->trgt_aim = work->trgt_eye = &ORG_SnakeHidingPos[irnd()%ORG_N_SNAKE_HIDEPOS] ;
	    GM_ConfigObjectAction( &work->body, 0, fire_throw_l, 56*5, ORGA_BODY_ALL, 160 ) ;
	    CalcTargetPosAdj( work, work->trgt_pos, 12.0f ) ;
	    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
	}
    }
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 304*5 ) )
	MT_SetMotionSpeed( work->body.m_ctrl, -1 ),
	work->weap_usp.vw = -4.0f ; /* USPを右手で握る */
    else
	ORG_ActMotionOnceLayer0( work ) ;
}

static void ORG_ActUnrecogThroughGrenadeStart( Work *work )
{
    if ( work->procs[ORGA_P_GRENADE] )
	GCL_ExecProc( work->procs[ORGA_P_GRENADE], NULL ) ;

    work->wait_sec = 2 ;
    work->trgt_eye = work->trgt_aim = &ORG_SnakeHidingPos[irnd()%ORG_N_SNAKE_HIDEPOS] ;

    work->control.turn.vy = 3072-2048 ;
    GM_ConfigObjectAction( &work->body, 0, fire_throw_l, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    MT_SetMotionSpeed( work->body.m_ctrl, 8 ) ;
    FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    work->act++ ;
}




/*

  グレネード下投げ（スネークが,しゃがみ狙撃ポイントにいるとき投げる）

 */
static void ORG_ActThroughGrenadeLowStart( Work *work )
{
    if ( work->procs[ORGA_P_GRENADE] )
	GCL_ExecProc( work->procs[ORGA_P_GRENADE], NULL ) ;

    work->trgt_eye = work->trgt_aim = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_throw_down, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		ORGA_F_ENB_AVOID  | ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;

    work->weap_sgr.vw = -5.0f ; /* SGRを摩擦の低いのに設定する */

    work->act++ ;
}

static void ORG_ActThroughGrenadeToGap( Work *work )
{
    static FVECTOR throw_point_l = {  -10450, 12817, -17000, 19.0f } ;
    static FVECTOR throw_point_r = {  -10520, 12818, -16570, 19.0f } ;

    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 29*5 ) )
	work->weap_usp.vw = -6.0f ; /* USPを腰にしまう */
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 62*5 ) )
	work->weap_sgr.vw = -3.0f ; /* SGRを取り出す（右手） */
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 135*5 ) )
	work->voice = ORGA_VO_THROUGH_SGR1, work->voice_tim = 4 ;
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 145*5 ) )
	work->weap_sgr = (work->trgt_pos == &ORG_Ply_AttkPosL ? throw_point_l : throw_point_r) ;
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 202*5 ) )
	work->weap_usp.vw = -4.0f ; /* USPを右手で握る */
    else
	ORG_ActMotionOnceLayer0( work ) ;
}

/*
  ORG_SnakeFBTNArea にグレネードを投げる
*/
static void ORG_ActThroughGrenadeLow( Work *work )
{
    FVECTOR d ;

    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 29*5 ) )
	work->weap_usp.vw = -6.0f ; /* USPを腰にしまう */
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 62*5 ) )
	work->weap_sgr.vw = -3.0f ; /* SGRを取り出す（右手） */
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 135*5 ) )
	work->voice = ORGA_VO_THROUGH_SGR1, work->voice_tim = 4 ;
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 145*5 ) )
    {
	_sceVu0SubVector( &d, &work->control.mov, &ORG_SnakeFBTNArea ) ;
	_sceVu0Normalize( &work->weap_sgr, &d ) ;
	_sceVu0ScaleVector( &work->weap_sgr, &work->weap_sgr, 900.0f ) ;
	_sceVu0AddVector( &work->weap_sgr, &work->weap_sgr, &ORG_SnakeFBTNArea ) ;
	work->weap_sgr.vw = sceVu0Sqrt( d.vx*d.vx+d.vz*d.vz )/ 120.0f ;
    }
    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 202*5 ) )
	work->weap_usp.vw = -4.0f ; /* USPを右手で握る */
    else
	ORG_ActMotionOnceLayer0( work ) ;
}


/*

  低い隠れ場所から相手を見ずに撃つ（左右）

*/
static inline void BlindFireRecog( Work *work )
{
    if ( work->weap_blt == 1 && !(work->ply_hide & ORGA_F_PLYR_HIDING) &&
	 !(work->aim_flg & ORGA_F_DONT_SHOOT) )
	_sceVu0CopyVector( &work->weap_usp, ORG_RecogPlayerPos( HUMAN21_ONAKA ) ) ;
    else
    {
	_sceVu0ScaleVector( &work->weap_usp,
			    (FVECTOR*)&BODYWORLD(&work->body, HUMAN21_MIGI_TE).m[Y], -1000.0f ) ;
	_sceVu0AddVector( &work->weap_usp, &work->weap_usp, BODYPOS(&work->body, HUMAN21_MIGI_TE) ) ;
    }
    work->weap_usp.vw = 1.0f ;/* USP撃つ */
}
static void ORG_ActBlindFirePRightStart( Work *work )
{
    if ( work->procs[ORGA_P_BLIND_FIRE] )
	GCL_ExecProc( work->procs[ORGA_P_BLIND_FIRE], NULL ) ;/*プロック実行*/
    GM_ConfigObjectAction( &work->body, 0, fire_blind_r_p, 0, ORGA_BODY_ALL,
			   60*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_NVR_DAMAGED| ORGA_F_ENB_AIM| ORGA_F_NOMISS_SHOT,
		ORGA_F_ENB_AVOID| ORGA_F_FACETO_ALL ) ;
    /* 腰のX軸方向へ -130 だけ 補正を掛ける */
    _sceVu0ScaleVector( &work->pos_adj, (FVECTOR*)&BODYWORLD(&work->body, HUMAN21_KOSHI).m[X],-10.0f ) ;
    work->pos_adj.vw = 13.0f ;/* 位置補正を13フレーム行なう */
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
    work->act++ ;
}
static void ORG_ActBlindFirePLeftStart( Work *work )
{
    ORG_ActBlindFirePRightStart( work ) ;
    /* 腰のX軸方向へ 130 だけ 補正を掛ける */
    _sceVu0ScaleVector( &work->pos_adj, (FVECTOR*)&BODYWORLD(&work->body, HUMAN21_KOSHI).m[X], 10.0f ) ;
    work->pos_adj.vw = 13.0f ;/* 位置補正を13フレーム行なう */
    work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2 ;
    work->weap_usp.vw = -5.0f ;/* USPを左手に持つ */
}
static void ORG_ActBlindFireRightLeftStart( Work *work )
{
    BlindFireRecog( work ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_blind_r, 0, ORGA_BODY_ALL, 0 ) ;/*モーション補間なし*/
    work->act++ ;
}

static void ORG_ActBlindFirePUpStart( Work *work )
{
    if ( work->procs[ORGA_P_BLIND_FIRE] )
	GCL_ExecProc( work->procs[ORGA_P_BLIND_FIRE], NULL ) ;/*プロック実行*/
    GM_ConfigObjectAction( &work->body, 0, fire_blind_u_p, 0, ORGA_BODY_ALL,
			   60*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_NVR_DAMAGED| ORGA_F_ENB_AIM| ORGA_F_NOMISS_SHOT,
		ORGA_F_ENB_AVOID| ORGA_F_FACETO_ALL ) ;
    work->weap_usp.vw = -4.0f ;/* USPを右手に持つ */
    work->act++ ;
}
static void ORG_ActBlindFireUpStart( Work *work )
{
    BlindFireRecog( work ) ;
    GM_ConfigObjectAction( &work->body, 0, fire_blind_u, 0, ORGA_BODY_ALL, 0 ) ;/*モーション補間なし*/
    work->act++ ;
}

static void ORG_ActBlindFire( Work *work )
{
    if ( --work->wait_sec <= 0 )
    {
	if ( work->weap_blt <=0 /* || work->aim_flg & ORGA_F_DONT_SHOOT*/ )
	    work->act++ ;
	else if ( work->rage < 10 )
	    work->act -= 2 ;
	else
	    work->act++ ;
    }
}



/*

  スネークを狙い撃ち

*/
static void ORG_ActSnipeSnakeMoveStart( Work *work )
{
    if ( work->trgt_pos == &ORG_Ply_AttkGrnd )
    {
	if ( work->flag & ORGA_F_ENB_ROUTE )
	    work->act += 3 ;/* ORG_ActRunOrDash */
	else
	    work->act += 5 ;/* ORG_ActSnipeFireP */
	ChangeFlag( work,
		    ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1,
		    ORGA_F_ENB_AIM| ORGA_F_ENB_AVOID| ORGA_F_FACETO_ALL ) ;
    }
    else
    {
	ResetAll( work, ORGA_F_ENB_ROUTE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM ) ;
	work->trgt_pos = &ORG_Ply_AttkGrnd ;
	work->act++ ;
    }

    work->hide_spot = NULL ;/* ハイドはない */
    ChangeFlag( work,
		ORGA_F_OFFENCE| ORGA_F_STOP_STILL| ORGA_F_NOMISS_SHOT,
		ORGA_F_MODIST| ORGA_F_ENB_SAFE ) ;
}

static void ORG_ActSnipeFireP( Work *work )
{
    static FVECTOR pos ;

    if ( work->flag & ORGA_F_STOP_STILL )
    {
	if ( GM_PlayerStatus & PLAYER_DOWNED )
	    pos = GM_PlayerControl->mov ;
	if ( work->rage < 10 )
	    if ( --work->wait_sec <= 0 ||
		 (pos.vz > GM_PlayerControl->mov.vz + 50.0f && !(GM_PlayerStatus & PLAYER_DOWNED)) )
		work->act -= 3 ;
    }
    else
	work->act++ ;
}

static void ORG_ActSnipeFireStart( Work *work )
{
    work->wait_sec = 180 ;
    work->weap_usp.vw = -8.0f ;/* スナイプファイヤー用のUSPコマンド */
    GM_ConfigObjectAction( &work->body, 0, fire, 0, ORGA_BODY_ALL,
			   2*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_FACETO_HEAD| ORGA_F_FACETO_AIM1, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
