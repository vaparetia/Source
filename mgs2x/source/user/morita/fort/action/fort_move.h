/*
   fort_move.h
   フォーチュン 移動行動処理関数群

   2000/01/13 T.Morita
   $Id: fort_move.h,v 1.1.1.3 2002/11/19 11:46:08 Yoshizawa1 Exp $
*/



/*

  Moving ACTs

*/


/* 歩き */

static void FRT_ActWalk( Work *work )
{
    ASSERT( work->trgt_pos ) ;/* 普通はここに来ないが来ちゃった時のため */
    if ( FaceAtoB( work, work->trgt_pos, &work->control.mov ) < FRT_WALK_DIST )
    {
	CalcTargetPosAdj( work, work->trgt_pos, 8.0f ) ;
	work->body.step->vx = work->body.step->vz = 0.0f ;
	work->act++ ;
    }
}

static void FRT_ActWalkStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, norm_walk, 0, FRT_BODY_ALL, 150 ) ;
    ChangeFlag( work, FRT_F_NONE, FRT_F_FACETO_ALL ) ;
    work->act++ ;
}



/* 蟹歩き */

static void FRT_ActSideStepWalk( Work *work )
{
    ASSERT( work->trgt_pos ) ;/* 普通はここに来ないが来ちゃった時のため */

    if ( (work->body.step->vx < 0.0f && work->control.mov.vx<-2600.0f) ||/* 行き過ぎ防止 */
	 (work->body.step->vx > 0.0f && work->control.mov.vx> 2600.0f) ||
	 SideAtoB( work, work->trgt_pos, &work->control.mov ) < FRT_WALK_DIST )
    {
	CalcTargetPosAdj( work, work->trgt_pos, 8.0f ) ;
	work->body.step->vx = work->body.step->vz = 0.0f ;
	work->act++ ;
    }
}

static void FRT_ActSideStepWalkStart( Work *work )
{
    if ( work->control.mov.vx > work->trgt_pos->vx )
	GM_ConfigObjectAction( &work->body, 0, norm_side_r, 0, FRT_BODY_ALL, 150 ) ;
    else
	GM_ConfigObjectAction( &work->body, 0, norm_side_l, 0, FRT_BODY_ALL, 150 ) ;
    ChangeFlag( work, FRT_F_FACETO_ALL, FRT_F_NONE ) ;
    work->act++ ;
}


/* 方向転換 */

static void FRT_ActTurnToRot( Work *work, int rot )
{
    int motion ;

    rot -= work->control.turn.vy ;
    rot &= 4095 ;
    rot -= rot>2048 ? 4096 : 0 ;
    if ( rot/512 )
    {
	if ( rot > 0 )
	    motion = norm_turn_l ;
	else
	    motion = norm_turn_r ;

	//GM_SeSetMode( SD_E_TURN0001, &work->control.mov, GM_SEMODE_BOMB ) ;

	GM_ConfigObjectAction( &work->body, 0, motion, 0, FRT_BODY_ALL, 150 ) ;
	ChangeFlag( work, FRT_F_NONE, FRT_F_FACETO_ALL ) ;
	work->act++ ;
    }
    else
	work->act += 2 ;/* 回転モーションをすっとばす */
}

static void FRT_ActTurnToWalkStart( Work *work )
{
    FVECTOR d ;
    int rot ;

    _sceVu0SubVector( &d, work->trgt_pos, &work->control.mov ) ;
    rot = (int)(2048.0f / M_PI * atan2f( d.vx, d.vz )) ;

    FRT_ActTurnToRot( work, rot ) ;
}

static void FRT_ActTurnToStopStart( Work *work )
{
    FVECTOR d ;
    int rot ;

    _sceVu0SubVector( &d, FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ), &work->control.mov ) ;
    rot = (int)(2048.0f / M_PI * atan2f( d.vx, d.vz )) ;

    FRT_ActTurnToRot( work, rot ) ;
}

static void FRT_ActTurnToStop( Work *work )
{
    ASSERT( work->trgt_pos ) ;/* 普通はここに来ないが来ちゃった時のため */

    /* movを固定 */
    work->control.mov.vx = work->trgt_pos->vx ;
    work->control.mov.vz = work->trgt_pos->vz ;

    if ( GM_CheckObject_IsEnd( &work->body, work->flag&FRT_F_OVERLAYED ? 1 : 0 ) )
	work->act++ ;
}
