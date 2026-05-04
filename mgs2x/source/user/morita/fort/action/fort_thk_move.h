/*
   fort_thk_move.h
   フォーチュン 思考関数 移動モード

   2000/01/05 T.Morita
   $Id: fort_thk_move.h,v 1.1.1.3 2002/11/19 11:46:08 Yoshizawa1 Exp $
*/









/*

  新しいハイドをランダムに検索する。
  この関数は,このファイル以外からも呼ばれている。
  
*/
static void FRT_NextTargetPlace( Work *work )
{
    if ( !(work->flag & (FRT_F_AM_ON_LEFT| FRT_F_AM_ON_RIGHT) ) )
	work->trgt_pos = &FRT_LeftPos, work->flag |= FRT_F_AM_ON_LEFT ;
    else if ( work->flag & FRT_F_AM_ON_LEFT )
	work->trgt_pos = &FRT_RightPos, ChangeFlag( work, FRT_F_AM_ON_RIGHT, FRT_F_AM_ON_LEFT ) ;
    else if ( work->flag & FRT_F_AM_ON_RIGHT )
	work->trgt_pos = &FRT_LeftPos , ChangeFlag( work, FRT_F_AM_ON_LEFT, FRT_F_AM_ON_RIGHT ) ;
}

static void FRT_ActThinkToMove( Work *work )
{
    int stll_old = work->stll_mtn ;

    printf( "Fort: Move\n" ) ;

    ChangeFlag( work, FRT_F_MOVED_ONCE|FRT_F_OFFENCE, FRT_F_NONE ) ;/* 攻撃モードへ */

    work->act = FRT_ActionWalk ;
    work->stll_mtn = stll_idle ;
    work->stll_tim = 0 ;

    if ( work->act_flg & FRT_F_HIDING_ATTK )
    {
	printf( "Fort: Change <trgt_pos> to HidePos\n" ) ;
	work->trgt_pos = &FRT_HidingPos ;
	return ;
    }

    switch( work->act_phase & 0xff00 )
    {
    case 0x0000:
    {
	static FVECTOR shoot_pos[] = { 
	    {-800.0f,-45000.0f,-8000.0f}, { 0.0f,-45000.0f,-8000.0f},
	    { 800.0f,-45000.0f,-8000.0f}, { 0.0f,-45000.0f,-8000.0f},
	} ;

	work->trgt_pos = &shoot_pos[(int)work->stll_tim&3] ;
	break ;
    }

    case 0x0400:
    {
	FRT_Phase_00Pos.vx = -300.0f ;
	work->trgt_pos = &FRT_Phase_00Pos ;
	break ;
    }

    default:
	if ( work->act_phase >= 0x0c00 )
	{
	    work->trgt_pos = &FRT_Phase_0cPos ;
	    if ( fpu_Abs(work->trgt_pos->vz - work->control.mov.vz) < 500.0f )
		work->act = FRT_ActionSideStepWalk ;
	}
	else
	{
	    float d ;
	    static int voice[]={
		SD_V_FORTUN11,  //フォーチュン「早く私を殺して」//fortun11 789
		SD_V_FORTUN12,  //フォーチュン「何してるの？～」//fortun12 790
		SD_V_FORTUN13,  //フォーチュン「殺してみなさ～」//fortun13 791
		SD_V_FORTUN14,  //フォーチュン「貴方に私の悲～」//fortun14 792
		SD_V_FORTUN15,  //フォーチュン「当たってないわ」//fortun15 793
		SD_V_FORTUN16,  //フォーチュン「当たらないわ」//fortun16 794
	    } ;
	    work->trgt_pos = &FRT_Phase_00Pos ;

	    d =  FRT_PLY_PLAYERPOS.vx - work->control.mov.vx ;
	    d =  d>1000.0f ? 1000.0f : d<-1000.0f ? -1000.0f : d ;
	    d += work->control.mov.vx ;
	    d =  d>2500.0f ? 2500.0f : d<-2500.0f ? -2500.0f : d ;
	    work->trgt_pos->vx = d ;

	    if ( (int)((d - work->control.mov.vx) * 100.0f) )
	    {
		if ( fpu_Abs(work->trgt_pos->vz - work->control.mov.vz) < 500.0f )
		    work->act = FRT_ActionSideStepWalk ;

		/* 「私を殺して」て語り */
		if ( work->voice_cnt < sizeof(voice)/sizeof(int) )
		{
		    work->voice = voice[work->voice_cnt++] ;
		    work->voice_tim = 5 ;
		}
	    }
	    else
	    {
		work->act = FRT_ActionAbort ;
		work->stll_mtn = stll_old ;
		ChangeFlag( work, FRT_F_NONE, FRT_F_MOVED_ONCE ) ;/* 移動してない */
	    }
	}
	break ;

    }
}
