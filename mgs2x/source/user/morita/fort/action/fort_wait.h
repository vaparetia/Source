/*
   fort_wait.h
   フォーチュン 待ち行動処理関数群

   2000/01/13 T.Morita
   $Id: fort_wait.h,v 1.1.1.3 2002/11/19 11:46:08 Yoshizawa1 Exp $
*/



static void FRT_ActFaceToPlayerStart( Work *work )
{
    FaceAtoB( work, FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ), &work->control.mov ) ;
    work->act++ ;
}
static void FRT_ActFaceToTargetStart( Work *work )
{
    ASSERT( work->trgt_pos ) ;

    ChangeFlag( work, FRT_F_NONE, FRT_F_FACETO_ALL ) ;
    FaceAtoB( work, work->trgt_pos, &work->control.mov ) ;
    work->act++ ;
}
static void FRT_ActWaitFacing( Work *work )
{
    if ( (work->control.turn.vy - work->control.rot.vy)/64 == 0 )
	work->act++ ;
}

static void FRT_ActUnrecogWait( Work *work )
{
    if ( --work->wait_sec <= 0 || !(work->flag & FRT_F_UNRECOG_POS) )
	work->act++ ;
}


static void FRT_ActWaitWhileDemo( Work *work )
{
    if ( !(work->flag & FRT_F_DEMO_MOVIE) )
	work->act++ ;
}

static void FRT_ActWait( Work *work )
{
    if ( --work->wait_sec <= 0 )
	work->act++ ;
}
static void FRT_ActWaitThreeSecStart( Work *work )
{
    work->wait_sec = 180*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitTwoSecStart( Work *work )
{
    work->wait_sec = 120*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitOneSecStart( Work *work )
{
    work->wait_sec = 60*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitHalfSecStart( Work *work )
{
    work->wait_sec = 30*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitQuaterSecStart( Work *work )
{
    work->wait_sec = 15*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitDeciSecStart( Work *work )
{
    work->wait_sec = 10*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitCentiSecStart( Work *work )
{
    work->wait_sec = 6*5/TIME_BASE-1 ;
    work->act++ ;
}
static void FRT_ActWaitZeroSecStart( Work *work )
{
    work->wait_sec = 1 ;
    work->act++ ;
}


static void FRT_ActQuestionStart( Work *work )
{
    work->head_mark = HMK2_TYPE_WHT_QE ; /* 白？マーク */
    if ( !work->voice_tim )
    {
	work->voice     = GV_Time&1 ? FRT_VO_FINDTEASE2 : FRT_VO_HIDETEASE0 ;
	work->voice_tim = 2 ;
    }
    work->act++ ;
}
