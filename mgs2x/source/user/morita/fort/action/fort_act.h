/*
   fort_act.h
   フォーチュン 行動処理関数群

   2000/01/05 T.Morita
   $Id: fort_act.h,v 1.1.1.3 2002/11/19 11:46:07 Yoshizawa1 Exp $
*/

/***

  Subroutines for Acting as for <work->act>

  ***/
static float FaceAtoB( Work *work, FVECTOR *a, FVECTOR *b )
{
    FVECTOR d ;

    _sceVu0SubVector( &d, a, b ) ;
    work->control.turn.vy = (short)(2048.0f / M_PI * atan2f( d.vx, d.vz )) ;

    return d.vx*d.vx+d.vz*d.vz ;
}
static float SideAtoB( Work *work, FVECTOR *a, FVECTOR *b )
{
    FVECTOR d ;

    _sceVu0SubVector( &d, a, b ) ;
    work->control.turn.vy = (short)(2048.0f / M_PI * atan2f( d.vx, d.vz )) +
	(d.vx<0.0f ? 1024 : -1024) ;

    return d.vx*d.vx ;
}

static inline void FaceEyeAt( Work *work, FVECTOR *a )
{
    work->trgt_eye = a ;
}

static void FaceAimAt( Work *work )
{
    if ( work->flag & FRT_F_AIMMING_AUTO )
    {
	if ( GM_GameLevel >= GM_LEVEL_NORMAL &&
	     work->shot_cnt & 1 &&
	     (work->act_phase & 0xff00) >= 0x0200 )/* ノーマルで半分の確率で外す */
	    work->trgt_mis_pos.vw = 0.0f ;
	else
	    work->trgt_mis_pos.vw = 1000.0f*frnd() ;
	work->trgt_aim = &work->trgt_mis_pos ;
	work->shot_tim = work->wait_sec*3/4 ;

	if ( work->trgt_typ & FRT_AIM_DontDestinate )
	{
	    FVECTOR *raiden, *fortune ;

	    raiden  = FRT_MiscPlayerPos( HUMAN21_KUBI ) ;
	    fortune = BODYPOS( &work->body, HUMAN21_MIGI_TE ) ;

	    if ( FRT_AIM_GetAimSpot( &work->trgt_aim_pos,
				     work->act_phase, work->trgt_typ,
				     fortune,
				     work->flag & FRT_F_IGNORE_PLYER ? NULL : raiden ) )
		work->trgt_aim = &work->trgt_aim_pos, work->shot_tim = -1 ;
	}
	else if ( work->flag & FRT_F_NOMISS_SHOT )
	{
	    work->trgt_mis_pos.vw = 0.0f ;/* 必ず外さない */

	    work->shot_tim = work->wait_sec*4/5 ;
	    if ( GM_GameLevel <= GM_LEVEL_EASY )
		work->shot_tim -= TIME_BASE*2 ;
	    else if ( GM_GameLevel == GM_LEVEL_NORMAL )
		work->shot_tim -= TIME_BASE ;
	    if ( work->shot_tim < 0 )
		work->shot_tim = 0 ;
	}
	else if ( work->flag & FRT_F_UNRECOG_POS )
	{
	    work->shot_tim = work->wait_sec/2 ;
	    work->trgt_mis_pos.vw = 1000.0f*frnd() ;
	}
    }
    FaceEyeAt( work, work->trgt_aim ) ;

#if DEBUG_MODE
    //printf( "AIM trgt_aim%x DONTSHOOT%x\n", work->trgt_aim, work->flag & FRT_F_DONT_SHOOT ) ;
    if ( work->trgt_aim )
	printf( "trgt_aim(%.1f %.1f %.1f)\n",
		work->trgt_aim->vx,
		work->trgt_aim->vy,
		work->trgt_aim->vz ) ;
#endif
}



static inline void CalcTargetPosAdj( Work *work, FVECTOR *trgt_pos, float frames )
{
    ASSERT( trgt_pos ) ;

    _sceVu0SubVector( &work->pos_adj, trgt_pos, &work->control.mov ) ;
    _sceVu0ScaleVector( &work->pos_adj, &work->pos_adj, 1.0f/frames ) ;
    work->pos_adj.vw = frames ;
}


static inline void ResetAll( Work *work, int set, int reset )
{
    /* 反転ビットを元に戻す */
    work->body.m_ctrl->flag &= ~(MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2) ;

    /* オーバーレイを切る */
    if ( work->flag & FRT_F_OVERLAYED )
	GM_ConfigObjectAction( &work->body, 1, -1, 0, FRT_BODY_ALL, 30 ) ;
    /* 再生速度をデフォルトに */
    //MT_SetMotionSpeed( work->body.m_ctrl, -1 ) ;

    work->weap_amo    = -2    ; /* hide cartridge  */
    work->voice_tim   =  0    ;
    if ( work->lgt_on < -1 ) /* ライトが消えていたら 点ける */
	work->lgt_on = 1 ;

    reset |= (FRT_F_OVERLAYED  | FRT_F_NVR_RESET    |
	      FRT_F_DONT_SHOOT | FRT_F_AIMMING_AUTO | FRT_F_IGNORE_PLYER) ;
    ChangeFlag( work, set, reset ) ;
}





/***

  Act Routines <work->act>

  ***/
static inline void FRT_ActMotionOnce( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, work->flag&FRT_F_OVERLAYED ? 1 : 0 ) )
	work->act++ ;
}
