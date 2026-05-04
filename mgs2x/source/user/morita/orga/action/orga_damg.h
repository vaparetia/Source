/*
   orga_damg.h
   オルガ ダメージ行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_damg.h,v 1.1.1.3 2002/11/19 11:46:21 Yoshizawa1 Exp $
*/

static void ORG_ActDamage( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
#if 0
	/* やれた後の一言(やられ声の後) */
	if ( irnd()&0x100 )
	{
	    if ( !(work->flag & (ORGA_F_GOT_WORSE| ORGA_F_GOT_SICK)) )
		work->voice = irnd()&0x1000? ORGA_VO_SHOTTEASE1: ORGA_VO_UTTER1, work->voice_tim = 20 ;
	    else if ( work->flag&ORGA_F_GOT_SICK && !(work->flag&ORGA_F_GOT_WORSE) )
		work->voice = irnd()&0x1000? ORGA_VO_SHOTTEASE0: ORGA_VO_UTTER0, work->voice_tim = 20 ;
	}
#endif
	if ( work->flag & (ORGA_F_UNRECOG_AIM| ORGA_F_UNRECOG_POS) )
	{
	    work->head_mark = HMK2_TYPE_RED_AT ; /* ！マーク */
	    work->head_time = 30 ;/*この秒数だけ,ビックリした時以外のビックリマークは出ない*/
	}
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_UNRECOG_POS| ORGA_F_UNRECOG_AIM| ORGA_F_NVR_RESET ) ;
	work->trgt_hid = *ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ; 
	work->ply_hide &= ~ORGA_F_PLYR_HIDLNG ;
	work->ply_hide &=  ORGA_F_PLYR_HIDCLR ;
	work->act++ ;
    }
}

/*オルガのダメージ*/
void ORG_ActDamageStart( Work *work, int motion )
{
    static void (*ORG_ActionRunaway[])( Work * ) = {
	ORG_ActDamage, ORG_ActSquatStillStart,
	ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
	ORG_ActRunStart, ORG_ActRunOrDash,
	ORG_ActStillStart, ORG_ActThink, NULL
    } ;

    work->non_dmg = 0 ; /*ダメージを受けていない経過時間を０にする*/

    FullResetAll( work,
		  ORGA_F_NVR_DAMAGED| ORGA_F_ENB_ROUTE| ORGA_F_NVR_RESET,
		  ORGA_F_ENB_SAFE| ORGA_F_ENB_AVOID|
		  ORGA_F_ENB_AIM| ORGA_F_FACETO_ALL ) ;

    if ( motion < 0 )
	work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2, motion *= -1 ;
    GM_ConfigObjectAction( &work->body, 0, motion, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;

    if ( work->act_flg & ORGA_F_GAP_ATTK )
	ChangeFlag( work, ORGA_F_DMG_FARHIDE , ORGA_F_NONE ) ;/*覗き穴からから狙撃された*/
    else if ( work->act_flg & ORGA_F_HOLO_ATTK )
	ChangeFlag( work, ORGA_F_DMG_HOROFIRE, ORGA_F_NONE ) ;/*ホロ撃ちの時に撃たれた*/
    else if ( work->act_flg & ORGA_F_HOLO_HIDE_ATTK )
	ChangeFlag( work, ORGA_F_DMG_HOROATTK, ORGA_F_NONE ) ;/*ホロ攻撃の時に撃たれた*/

    if ( (work->flag&ORGA_F_LEVEL_MSK) >= ORGA_F_LEVEL_NORM )
	work->rage++ ;
    if ( work->procs[ORGA_P_DAMAGED] )
	GCL_ExecProc( work->procs[ORGA_P_DAMAGED], NULL ) ;
    ORG_SearchHidePlace( work, work->stll_mtn ) ;

    work->act = ORG_ActionRunaway ;
}


/*オルガのゲームオーバー*/
static void ORG_ActDamagePastOutStart( Work *work )
{
    GM_ConfigObjectAction( &work->body, 0, damg_pastout, 0, ORGA_BODY_ALL,
			   30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work, ORGA_F_NVR_DAMAGED|ORGA_F_NVR_RESET, ORGA_F_FACETO_ALL ) ;
    work->act++ ;
}
static void ORG_ActDamageOver( Work *work )
{
#if DEBUG_MODE
#if 0
    static void *piyo = NULL ;

    if ( !piyo )
	piyo = NewPiyori( &BODYWORLD( &work->body, HUMAN21_ATAMA ) ) ;

    /*復活の呪文（デバッグ用）*/
    if ( GV_PadData[0].press & PAD_STA )
	ORG_InitParam( work, work->name ), GV_DestroyActor( piyo ), piyo=NULL ;
#endif
#endif
}

void ORG_ActDamageOverStart( Work *work )
{
    static void (*ORG_ActionDamageOverToSleep[])( Work * ) = {
	ORG_ActMotionOnceLayer0, ORG_ActDamagePastOutStart, ORG_ActDamageOver, NULL
    } ;

    GM_SeSetMode( ORGA_VO_PASTOUT, &work->control.mov, GM_SEMODE_BOMB ) ;
    GM_ConfigObjectAction( &work->body, 0, damg_over, 0, ORGA_BODY_ALL,
			   80*work->act_speed/ORGA_NORMAL_SPEED ) ;
    work->act = ORG_ActionDamageOverToSleep ;
    if ( work->flag & ORGA_F_OVERLAYED )
	GM_ConfigObjectAction( &work->body, 1, -1, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    ChangeFlag( work,
		ORGA_F_NVR_DAMAGED| ORGA_F_ENB_ROUTE| ORGA_F_NVR_RESET,
		ORGA_F_ENB_SAFE| ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM|
		ORGA_F_OVERLAYED| ORGA_F_FACETO_ALL ) ;
}

