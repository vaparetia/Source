/*
   orga_stll.h
   オルガ 静止行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_stll.h,v 1.1.1.3 2002/11/19 11:46:23 Yoshizawa1 Exp $
*/

/* 現在では,転がり移動での攻撃時にしか 出ていない。 */
void ORG_ActSquatStillStart( Work *work )
{
    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_NONE ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_squat, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}
void ORG_ActStandStillStart( Work *work )
{
    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_NONE ) ;
    GM_ConfigObjectAction( &work->body, 0, stll_idle, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act++ ;
}

/* 静止状態のスタート（リセットを掛ける） */
void ORG_ActStillStart( Work *work )
{
    /* 立つと危険な場合は,立たない,ビハインドしない */
    if ( work->flag & ORGA_F_DANGR_STAND )
	work->stll_mtn |= ORGA_F_IS_SQUAT, work->stll_mtn &= ~ORGA_F_IS_BEHIND ;

    ResetAll( work,
	      ORGA_F_NONE,
	      ORGA_F_ENB_SAFE| ORGA_F_NOMISS_SHOT| ORGA_F_NVR_DAMAGED| ORGA_F_ENB_ROUTE ) ;
    GM_ConfigObjectAction( &work->body, 0, work->stll_mtn, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;

    /*弾が空なので オフェンスにする（弾が空の状態で移動なし） */
    if ( work->weap_blt <=0 )
	ChangeFlag( work, ORGA_F_OFFENCE,
		    ORGA_F_FACETO_AIM2| ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM| ORGA_F_MODIST ) ;
    else if ( !(work->flag & ORGA_F_MODIST) )
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_AIM2| ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM ) ;

    /* ターゲット位置に位置補正を掛ける */
    CalcTargetPosAdj( work, work->trgt_pos, 5.0f ) ;

    /* ビハインドだったら, ハイドの壁に背を向ける */
    if ( work->stll_mtn & ORGA_F_IS_BEHIND )
    {
	if ( work->hide_spot )
	    work->control.turn.vy = work->hide_spot->turn ;
	else
	    work->control.turn.vy = 1024*3 ;
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_FACETO_ALL ) ;
    }
    else if ( work->trgt_aim )
	FaceAtoB( work, work->trgt_aim, &work->control.mov ) ;
    else
	FaceAtoB( work, ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ), &work->control.mov ) ;

    work->act++ ;
}


/* 中断アクション（避けなどでアクションを中断する場合に呼ばれる。） */
void ORG_ActAbortStart( Work *work )
{
    if ( !(work->flag & ORGA_F_UNRECOG_AIM) )
	work->rage++ ;
    ORG_ActStillStart( work ) ;
}



/* 突発性を考慮した 完全な行動リセット 必ず移動モードから始まる */
static void ORG_ActResetStart( Work *work )
{
    static void (*ORG_ActionFaceAndRunForReset[])( Work * ) = {
	ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
	ORG_ActRunStart, ORG_ActRunOrDash, ORG_ActStillStart, ORG_ActThink, NULL
    } ;
    printf( "HEY!! You why you RESET ME!!!!!!!!!!!!!!!%x\n", work->act_flg ) ;

    FullResetAll( work,
		  ORGA_F_ENB_ROUTE| ORGA_F_OFFENCE,
		  ORGA_F_ENB_SAFE| ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM|
		  ORGA_F_NVR_DAMAGED| ORGA_F_FACETO_ALL| ORGA_F_ENB_ROUTE|
		  ORGA_F_MODIST ) ;
    GM_ConfigObjectAction( &work->body, 0, work->stll_mtn, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ORG_SearchHidePlace( work, work->stll_mtn ) ;
    work->act = ORG_ActionFaceAndRunForReset ;
}


/* 完全な行動リセットを行ない ORGA_F_STOP_STILL フラグ解除まで止まる
   (orga_msg.cで work->act に代入される) */
static void ORG_ActStopStillStart( Work *work )
{
    FullResetAll( work,
		  ORGA_F_STOP_STILL| ORGA_F_ENB_ROUTE,
		  ORGA_F_ENB_SAFE| ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM|
		  ORGA_F_NVR_DAMAGED| ORGA_F_FACETO_ALL| ORGA_F_MODIST|
		  ORGA_F_OFFENCE ) ;
    GM_ConfigObjectAction( &work->body, 0, work->stll_mtn, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ORG_SearchHidePlace( work, work->stll_mtn ) ;
    work->act++ ;
}

static void ORG_ActStopStill( Work *work )
{
    if ( !(work->flag & ORGA_F_STOP_STILL) )
	work->act++ ;
}
