/*
   fort_stll.h
   フォーチュン 静止行動処理関数群

   2000/01/13 T.Morita
   $Id: fort_stll.h,v 1.1.1.3 2002/11/19 11:46:08 Yoshizawa1 Exp $
*/

/* 現在では,転がり移動での攻撃時にしか 出ていない。 */
void FRT_ActStandStillStart( Work *work )
{
    ChangeFlag( work, FRT_F_NOMISS_SHOT, FRT_F_NONE ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_idle, 0, FRT_BODY_ALL, 150 ) ;
    work->act++ ;
}

/* 静止状態のスタート（リセットを掛ける） */
void FRT_ActStillStart( Work *work )
{
    ResetAll( work, FRT_F_NONE, FRT_F_NOMISS_SHOT| FRT_F_NVR_DAMAGED ) ;

    GM_ConfigObjectAction( &work->body, 0, work->stll_mtn, 0, FRT_BODY_ALL, 150 ) ;

    /*弾が空なので オフェンスにする（弾が空の状態で移動なし） */
    //if ( work->weap_blt <=0 )
	//ChangeFlag( work, FRT_F_OFFENCE, FRT_F_FACETO_AIM| FRT_F_ENB_AIM ) ;
    //else
	ChangeFlag( work, FRT_F_NONE, FRT_F_ENB_AIM ) ;

    /* ターゲット位置に位置補正を掛ける */
    CalcTargetPosAdj( work, work->trgt_pos, 5.0f ) ;

    FaceAtoB( work, FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ), &work->control.mov ) ;

    work->act++ ;
}


/* 中断アクション（避けなどでアクションを中断する場合に呼ばれる。） */
void FRT_ActAbortStart( Work *work )
{
    FRT_ActStillStart( work ) ;
}


static void FRT_CopyCurrentPos( Work *work )
{
    /* 突発性なためターゲット位置を現在位置に強制的に変える */
    _sceVu0CopyVector( work->trgt_pos = &work->trgt_frc, &work->control.mov ) ;

    if ( work->trgt_frc.vx < -2500.0f )
	work->trgt_frc.vx = -2500.0f ;
    if ( work->trgt_frc.vx >  2500.0f )
	work->trgt_frc.vx =  2500.0f ;
    if ( work->trgt_frc.vz < -8000.0f )
	work->trgt_frc.vz = -8000.0f ;
    if ( work->trgt_frc.vz > -4000.0f )
	work->trgt_frc.vz = -4000.0f ;
}

/* 突発性を考慮した 完全な行動リセット 必ず攻撃モードから始まる */
static void FRT_ActResetStart( Work *work )
{
    ResetAll( work, FRT_F_OFFENCE, FRT_F_ENB_AIM| FRT_F_NVR_DAMAGED| FRT_F_FACETO_ALL ) ;

    GM_ConfigObjectAction( &work->body, 0, work->stll_mtn, 0, FRT_BODY_ALL, 150 ) ;

    FRT_CopyCurrentPos( work ) ;

    work->act = FRT_ActionAbort ;
}


/* 完全な行動リセットを行ない FRT_F_STOP_STILL フラグ解除まで止まる
   (fort_msg.cで work->act に代入される) */
static void FRT_ActStopStillStart( Work *work )
{
    ResetAll( work, FRT_F_STOP_STILL, FRT_F_ENB_AIM| FRT_F_NVR_DAMAGED| FRT_F_FACETO_ALL ) ;
    GM_ConfigObjectAction( &work->body, 0, work->stll_mtn, 0, FRT_BODY_ALL, 150 ) ;

    FRT_CopyCurrentPos( work ) ;

    work->act++ ;
}

static void FRT_ActStopStill( Work *work )
{
    if ( !(work->flag & FRT_F_STOP_STILL) )
	work->act++ ;
}
