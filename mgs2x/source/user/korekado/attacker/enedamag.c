//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	damage.c
	敵兵共通ダメージモード
	
	1997/07/28 Y.Korekado
	$Id: enedamag.c,v 1.1.1.3 2002/11/19 11:44:00 Yoshizawa1 Exp $
	
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

//#include	"enedamag.h"

/*----- 思考状態 --------------------------------------------------*/
enum	{
	TH2_POWER_DAMAGE, TH2_MASUI_DOWN, TH2_THROW, TH2_HANG, TH2_MORTALLY,
	TH2_DEATH, TH2_GOOFY, TH2_EXIT
} ;
enum	{
	TH3_WAIT_MOTIONEND, TH3_WAIT, TH3_CHECK, TH3_GHOST
} ;

/*-----	定数定義	-----*/
	/*アクションパッド */
enum {
	SP_NONE,
	SP_MORTALLY,	/* 瀕死 */
	SP_RESURRECTION,		/* 奇跡の復活 */
	SP_DIE_ERASE
} ;

#include	"endamact.c"

/*----- --------------------------------------------*/
static	void	DownDamCheck( ENETHINK	*entk )
{
	ACTION *act ;
	
	act = entk->act ;
	if ( entk->act->status_status & (ACT_STST_DOWN_DAM) ) {
		if ( entk->status & ENE_STATUS_TALK_SLEEP ){
			if ( (act->time == 1) && (act->bodyp.life <= 0) ) {
				/* ストリーミングここで停止 */
				COM_StopRadio( entk ) ;
			}
		}
	}

	if( entk->receive & ENE_ORDER_CALL_COMM ) {
		COM_SetRadio( EV_SLEEP_RAD_MUDAASI, entk ) ;
	}
}

/*----- 低レベル思考モード --------------------------------------------*/
static	void	Think3_PowerDamageWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
//		printf(" damage wait [%d]\n",GV_Time ) ;
	}

	DownDamCheck( entk ) ;

//	if ( entk->act->act_end || (entk->act->status&ACT_STATUS_STANDSTILL) ) {
//	if ( entk->count3 != 0 ) {	/* 前のモーションのact_endを懸念 */
		if ( entk->act->act_end ) {
			if ( entk->act->bodyp.life <= 0 ) {
				entk->think2 = TH2_DEATH ;
				entk->think3 = TH3_GHOST ;
			} else {
				if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
					entk->think2 = TH2_MORTALLY ;
					entk->think3 = TH3_WAIT ;
				} else {
					if ( !(entk->act->bodyp.pbreak & PBREAK_ARMLEG) ) {
						if ( entk->act->act_end == 2 ) entk->mess_notice = MES_NOTICE_ZZZ ;
					}
					entk->notice |= ENE_NOTICE_DAMAGE ;
					entk->think3 = TH3_WAIT ;
				}

			}
			UNSET_FLAG( entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;/*報告フラグクリア*/
			entk->count3 = 0 ;
			return ;
		}
//	}

	if ( entk->act->status & ACT_STATUS_FAINT ) entk->mess_notice = MES_NOTICE_FAINT ;

	entk->count3 ++ ;
}

static	void	Think3_MasuiDownWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
//		printf(" damage wait [%d]\n",GV_Time ) ;
	}

	DownDamCheck( entk ) ;

//	if ( entk->act->act_end || (entk->act->status&ACT_STATUS_STANDSTILL) ) {
	if ( entk->act->act_end ) {
		if ( entk->act->bodyp.life <= 0 ) {
			entk->think2 = TH2_DEATH ;
			entk->think3 = TH3_GHOST ;
		} else {
			entk->notice |= ENE_NOTICE_DAMAGE ;
			if ( !(entk->act->bodyp.pbreak & PBREAK_ARMLEG) ) {
				entk->mess_notice = MES_NOTICE_ZZZ ;
			}
			entk->think3 = TH3_WAIT ;
		}
		UNSET_FLAG( entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;/*報告フラグクリア*/
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_ThrowWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
//		printf(" damage wait [%d]\n",GV_Time ) ;
	}

	DownDamCheck( entk ) ;

//	if ( entk->act->act_end || (entk->act->status&ACT_STATUS_STANDSTILL) ) {
	if ( entk->act->act_end ) {
		if ( entk->act->bodyp.life <= 0 ) {
			entk->think2 = TH2_DEATH ;
			entk->think3 = TH3_GHOST ;
		} else {
			/* 寝てたら書き換え */
			if ( !(entk->act->bodyp.pbreak & PBREAK_ARMLEG) ) {
				if ( entk->act->act_end == 2 ) entk->mess_notice = MES_NOTICE_ZZZ ;
			}
			entk->notice |= ENE_NOTICE_DAMAGE ;
			entk->think3 = TH3_WAIT ;
		}
		UNSET_FLAG( entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;/*報告フラグクリア*/
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->act->status & ACT_STATUS_FAINT ) entk->mess_notice = MES_NOTICE_FAINT ;

	entk->count3 ++ ;
}

static	void	Think3_HangWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
//		printf(" damage wait [%d]\n",GV_Time ) ;
	}

	DownDamCheck( entk ) ;

//	if ( entk->act->act_end || (entk->act->status&ACT_STATUS_STANDSTILL) ) {
	if ( entk->act->act_end ) {
//printf(" damage wait END[%d]\n",GV_Time ) ;
		if ( entk->act->bodyp.life <= 0 ) {
			entk->think2 = TH2_DEATH ;
			entk->think3 = TH3_GHOST ;
		} else {
			if ( !(entk->act->bodyp.pbreak & PBREAK_ARMLEG) ) {
				if ( entk->act->act_end == 2 ) entk->mess_notice = MES_NOTICE_ZZZ ;
			}
			entk->notice |= ENE_NOTICE_DAMAGE ;
			entk->think3 = TH3_WAIT ;
		}
		UNSET_FLAG( entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;/*報告フラグクリア*/
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->act->status & ACT_STATUS_FAINT ) entk->mess_notice = MES_NOTICE_FAINT ;

	entk->count3 ++ ;
}

static	void	Think3_GoofyWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
//		printf(" damage wait [%d]\n",GV_Time ) ;
		entk->c_notice = 0 ;
	}

	DownDamCheck( entk ) ;

	if ( entk->act->act_end ) {
		if ( entk->act->bodyp.life <= 0 ) {
			entk->think2 = TH2_DEATH ;
			entk->think3 = TH3_GHOST ;
		} else {
			if ( entk->act->bodyp.dammode == DAM_MODE_GOOFY ) {
				if ( !(entk->act->bodyp.pbreak & PBREAK_ARMLEG) ) {
					entk->mess_notice = MES_NOTICE_GOOFY ;
				}
			} if ( entk->act->bodyp.dammode == DAM_MODE_SMOKE ) {
				if ( !(entk->act->bodyp.pbreak & PBREAK_ARMLEG) ) {
					entk->mess_notice = MES_NOTICE_SMOKE ;
				}
			}

			entk->notice |= ENE_NOTICE_DAMAGE ;
			entk->think3 = TH3_WAIT ;
		}
		UNSET_FLAG( entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;/*報告フラグクリア*/
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_DeathWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		COM_UnitMemberDie( entk->g_id, entk->u_id ) ;
		ENE_ProcCall( &entk->death_proc ) ;
		entk->count3 ++ ;
	}

	if ( entk->act->status & ACT_STATUS_GHOST ) {
		if ( COM_ResurrectionPermit( entk ) ) {		/* 生き返り許可 */
			entk->think3 = TH3_WAIT ;
			entk->count3 = 0 ;
			UNSET_FLAG( entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;/*報告フラグクリア*/
			
			return ;
		}
	}

}

static	void	Think3_WaitRes( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->notice |= ENE_NOTICE_RES ;
	}

	entk->count3 ++ ;
}

static	void	Think3_WaitMortally( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_MORTALLY ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
//		entk->act->aim_pos = GM_PlayerPosition ;
//		entk->status2 |= ENE_STATUS2_AIM_FACE ;
	}

	if ( entk->act->act_end ) {
		entk->think2 = TH2_DEATH ;
		entk->think3 = TH3_GHOST ;
	}

	entk->count3 ++ ;
}

static	void	Think3_WaitExit( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 == 0 ) {
//		entk->act->pad = SP_HOLDUP_EXIT ;
	}

	if ( entk->act->act_end ) {
		entk->think2 = TH2_DEATH ;
		entk->think3 = TH3_GHOST ;
	}

	entk->count3 ++ ;
}

static	void	Think3_Wait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
	}
	
	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_PowerDamage( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT_MOTIONEND :
	    	Think3_PowerDamageWait( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_Wait( entk ) ;
	    break ;
	}
}

static	void	Think2_MasuiDown( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT_MOTIONEND :
	    	Think3_MasuiDownWait( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_Wait( entk ) ;
	    break ;
	}
}

static	void	Think2_Throw( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT_MOTIONEND :
	    	Think3_ThrowWait( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_Wait( entk ) ;
	    break ;
	}
}

static	void	Think2_Hang( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT_MOTIONEND :
	    	Think3_HangWait( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_Wait( entk ) ;
	    break ;
	}
}

static	void	Think2_Goofy( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT_MOTIONEND :
	    	Think3_GoofyWait( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_Wait( entk ) ;
	    break ;
	}
}

static	void	Think2_Death( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_GHOST :
	    	Think3_DeathWait( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_WaitRes( entk ) ;
	    break ;
	}
}

static	void	Think2_Mortally( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT :
	    	Think3_WaitMortally( entk ) ;
	    break ;
	}
}

static	void	Think2_Exit( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT_MOTIONEND :
	    	Think3_WaitExit( entk ) ;
	    break ;
	}
}
/*----- 高レベル思考モード --------------------------------------------*/
static void KillCheck( entk )
ENETHINK	*entk ;
{
	if( entk->receive & ENE_ORDER_DIE_ERASE 
		|| entk->ctrl->mov.vy < -400000.0f ) {
		entk->act->pad = SP_DIE_ERASE ;

		entk->think2 = TH2_DEATH ;
		entk->think3 = TH3_GHOST ;
		entk->count3 = 0 ;
	}
}

void	ENE_Enemy_Think1_Damage( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_POWER_DAMAGE :
			Think2_PowerDamage( entk ) ;
			KillCheck( entk ) ;
		break ;
	    case TH2_MASUI_DOWN :
			Think2_MasuiDown( entk ) ;
			KillCheck( entk ) ;
		break ;
	    case TH2_THROW :
			Think2_Throw( entk ) ;
			KillCheck( entk ) ;
		break ;
	    case TH2_HANG :
			Think2_Hang( entk ) ;
			KillCheck( entk ) ;
		break ;
	    case TH2_GOOFY :
			Think2_Goofy( entk ) ;
			KillCheck( entk ) ;
		break ;
	    case TH2_DEATH :
			Think2_Death( entk ) ;
		break ;
	    case TH2_MORTALLY :
			Think2_Mortally( entk ) ;
		break ;
	    case TH2_EXIT :
			Think2_Exit( entk ) ;
		break ;
	}
	if ( entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_DEATH) ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;
		UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		entk->c_notice = 0 ;
	}
	if ( entk->act->status & ACT_STATUS_DOWN ) {
		SET_FLAG( entk->iknow_flag, IKNOW_DOWNED ) ;
		UNSET_FLAG( entk->iknow_flag, IKNOW_CLEARIMG_DAMAGE ) ;
	}
	if( entk->iknow_flag & IKNOW_DETECT ) {
		/* 発見中フラグＯＮ */
		COM_SetFlameFlag( CMFLAG_DETECT ) ;
	}
}

/*----- スタートモード、ダメージ --------------------------------------------*/
void	ENE_EnemyStartModeDamage( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = EneEnemyDamageCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_DAMAGE ;
	switch ( entk->act->bodyp.dammode ) {
		case DAM_MODE_BULLET :
			entk->think2 = TH2_POWER_DAMAGE ;
		break ;
		case DAM_MODE_MASUI_DOWN :
			entk->think2 = TH2_MASUI_DOWN ;
			CLEAR_FLAG( entk->iknow_flag ) ;
		break ;
		case DAM_MODE_THROW :
			entk->think2 = TH2_THROW ;
		break ;
		case DAM_MODE_HANG :
			entk->think2 = TH2_HANG ;
			if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
				SET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;
			}
		break ;
		case DAM_MODE_GOOFY :
			entk->think2 = TH2_GOOFY ;
			CLEAR_FLAG( entk->iknow_flag ) ;
		break ;
		case DAM_MODE_SMOKE :
			entk->think2 = TH2_GOOFY ;
		break ;
		case DAM_MODE_EXIT :
			entk->think2 = TH2_EXIT ;
		break ;
		default :
			entk->think2 = TH2_POWER_DAMAGE ;
		break ;
	}


	entk->think3 = TH3_WAIT_MOTIONEND ;
	entk->mess_notice = MES_NOTICE_NONE ;
	UNSET_FLAG( entk->iknow_flag, IKNOW_DOWNED ) ;

	if ( GM_GameStatus & STATE_CLEARING ) {
		SET_FLAG( entk->iknow_flag, IKNOW_CLEARIMG_DAMAGE ) ;
	} else {
		UNSET_FLAG( entk->iknow_flag, IKNOW_CLEARIMG_DAMAGE ) ;
	}

	entk->count3 = 0 ;
	entk->avoid = 0 ;

	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}

void	ENE_EnemyStartModeDamageDeath( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = EneEnemyDamageCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_DAMAGE ;
	entk->think2 = TH2_DEATH ;
	entk->think3 = TH3_GHOST ;
	entk->count3 = 0 ;

	entk->mess_notice = MES_NOTICE_NONE ;
	entk->avoid = 0 ;
	UNSET_FLAG( entk->iknow_flag, IKNOW_DOWNED ) ;

	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}
