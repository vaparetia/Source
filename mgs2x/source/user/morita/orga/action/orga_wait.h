/*
   orga_wait.h
   オルガ 待ち行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_wait.h,v 1.1.1.3 2002/11/19 11:46:23 Yoshizawa1 Exp $
*/



static void ORG_ActFaceToPlayerStart( Work *work )
{
    FaceAtoB( work, ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ), &work->control.mov ) ;
    work->act++ ;
}
static void ORG_ActFaceToTargetStart( Work *work )
{
    if ( work->trgt_pos )
    {
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
	FaceAtoB( work, work->trgt_nxt ? work->trgt_nxt : work->trgt_pos, &work->control.mov ) ;
    }
    work->act++ ;
}
/*現在 使われていない
static void ORG_ActBehindToTargetStart( Work *work )
{
    if ( work->trgt_pos )
    {
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
	FaceAtoB( work, &work->control.mov, work->trgt_nxt ? work->trgt_nxt : work->trgt_pos ) ;
    }
    work->act++ ;
}
*/
static void ORG_ActBehindToWallWestStart( Work *work )
{
    work->control.turn.vy = work->hide_spot ? work->hide_spot->turn : 0 ;
    work->act++ ;
}
static void ORG_ActBehindToWallStart( Work *work )
{
    work->control.turn.vy = work->hide_spot ? work->hide_spot->turn : 3072 ;
    work->act++ ;
}
static void ORG_ActFaceToWallStart( Work *work )
{
    work->control.turn.vy = work->hide_spot ? work->hide_spot->turn-2048 : -2048 ;
    work->act++ ;
}
static void ORG_ActWaitFacing( Work *work )
{
    if ( (work->control.turn.vy - work->control.rot.vy)/64 == 0 )
	work->act++ ;
}



static void ORG_ActWait( Work *work )
{
    if ( --work->wait_sec <= 0 )
	work->act++ ;
}
static void ORG_ActWaitThreeSecStart( Work *work )
{
    work->wait_sec = 180-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitTwoSecStart( Work *work )
{
    work->wait_sec = 120-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitOneSecStart( Work *work )
{
    work->wait_sec = 60-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitHalfSecStart( Work *work )
{
    work->wait_sec = 30-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitQuaterSecStart( Work *work )
{
    work->wait_sec = 15-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitDeciSecStart( Work *work )
{
    work->wait_sec = 10-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitCentiSecStart( Work *work )
{
    work->wait_sec = 6-1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}
static void ORG_ActWaitZeroSecStart( Work *work )
{
    work->wait_sec = 1 ;
    ChangeFlag( work, ORGA_F_ENB_SAFE, ORGA_F_NONE ) ;
    work->act++ ;
}


static void ORG_ActQuestionStart( Work *work )
{
    work->head_mark = HMK2_TYPE_WHT_QE ; /* 白？マーク */
    if ( !work->voice_tim )
    {
	work->voice     = GV_Time&1 ? ORGA_VO_FINDTEASE2 : ORGA_VO_HIDETEASE0 ;
	work->voice_tim = 2 ;
    }
    work->act++ ;
}
