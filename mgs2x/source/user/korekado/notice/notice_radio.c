//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	radio.c
	notice/無線機関係
	
	2000/06/13 Y.Korekado
	$Id: radio.c,v 1.1.1.3 2002/11/19 11:44:19 Yoshizawa1 Exp $
*/

#if 0
必要パラメータ 
pad:
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */

think3:
	TH3_RADIOSET
	TH3_RADIOBREAK
	TH3_RADIOCALL
	TH3_MOVE_WAITAREA
	TH3_REPAIR
	TH3_END
	

	tmp_buff[0]		無線音声
	tmp_buff[1]		現在のNOTICEモード
#endif

#define NOTICE_IKNOW_DURESSED (ENE_NOTICE_HOLDUP|ENE_NOTICE_DAMAGE)

extern int ENE_UseShoulderRadio( ENETHINK *entk);


static int ToSearchCheck( int mode )
{
	if ( mode == ENE_NOTICE_ACCIDENT ) return 0 ;
	if ( mode == ENE_NOTICE_ACCDNT_REPO ) return 0 ;
	return 1 ;
}

static	void	SetThinkRadioSet( ENETHINK *entk, int radio, int notice )
{
	entk->tmp_buff[0] = radio ;
	entk->tmp_buff[1] = notice ;

	if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ||
			entk->iknow_flag & IKNOW_RADIO_BREAK ) {
		/* 両腕が壊れていたら */
		ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
		/* 仲間が見かけたら、代わりに無線連絡してもらえるように */
		SET_FLAG( entk->iknow_flag, IKNOW_HELP_ME ) ;
		entk->think3 = TH3_MOVE_WAITAREA ; 
		entk->act->move_s = MoveRun ;
		entk->count3 = 0 ;
		return ;
	} else if ( ENE_NoRadio( ) ) {
		/* 回避モードか警戒モードになったばかり */
		entk->think3 = TH3_NO_RADIO ; 
		entk->count3 = 0 ;

		return ;
	} else if ( entk->act->bodyp.type & ENE_TYPES_NO_STRM ) {
		/* 無線なし */
		entk->think3 = TH3_NO_RADIO ; 
		entk->count3 = 0 ;

		return ;
#if 0
	} else if ( ENE_PureAttacker( entk->act ) ) {
#else
/*02.10.04*/
	} else if ( ENE_UseShoulderRadio( entk ) ) {
		/*肩についた無線機使用*/
#endif
		/* 攻撃兵 */
		entk->think3 = TH3_RADIOCALL ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->thk_status |= THK_STATUS_RADIO ;
	entk->think3 = TH3_RADIOSET ; 
}

static	void	Think3_RadioSet( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

#if 1
	/* モーションが発動されずバッグった！、要原因究明 */
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL ;
	}
#else
	/* 保険としてならこっちを採用 */
	entk->act->pad = SP_RADIOCALL ;
#endif

	if ( entk->act->act_end == 3 ) {
		if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
			entk->think3 = TH3_RADIOBREAK ; 
		} else {
			entk->think3 = TH3_RADIOCALL ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_RadioCall( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->tmp_time = COM_SetRadio( entk->tmp_buff[0], entk ) ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV1_STRM_LENGTH ;
		}
		ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
	}

#if 0
	if ( ENE_PureAttacker( entk->act ) ) {
#else
/*02.10.04*/
	if ( ENE_UseShoulderRadio( entk ) ) {
		/*肩についた無線機使用*/
#endif
		entk->act->pad = SP_RADIOCALL_ATTACKER ;

		if ( entk->count3 > entk->tmp_time ) {
			entk->act->pad = SP_RETURNRADIO ;
			if ( ToSearchCheck( entk->tmp_buff[1] ) ) {
				COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
			}
			UNSET_FLAG( entk->c_notice, entk->tmp_buff[1] ) ;
			if ( entk->tmp_buff[1] & NOTICE_IKNOW_DURESSED ) {
				UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
			}
			entk->think3 = TH3_END ; 
			entk->count3 = 0 ;
			return ;
		}
	} else {
		if ( entk->count3 == entk->tmp_time ) {
			entk->act->pad = SP_RETURNRADIO ;
			if ( ToSearchCheck( entk->tmp_buff[1] ) ) {
				COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
			}
		}

		if ( entk->count3 < entk->tmp_time ) {
			if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
				COM_StopRadio( entk ) ;
				entk->think3 = TH3_RADIOBREAK ; 
				entk->count3 = 0 ;
				return ;
			}
		}

//printf("entk->sw.radio[%x]\n",entk->sw.radio);

		if ( entk->act->act_end == 1 ) {
			if ( ToSearchCheck( entk->tmp_buff[1] ) ) {
				COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
			}
			UNSET_FLAG( entk->c_notice, entk->tmp_buff[1] ) ;
			if ( entk->tmp_buff[1] & NOTICE_IKNOW_DURESSED ) {
				UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
			}
			entk->think3 = TH3_END ; 
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_RadioBreak( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL_BREAK ;
	}

	if ( entk->count3 == COUNT_VMODE(115) ) {
		SET_FLAG( entk->iknow_flag, IKNOW_RADIO_BREAK ) ;
	}

	if ( entk->act->act_end == 1 ) {
		ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
		entk->think3 = TH3_MOVE_WAITAREA ; 
		entk->act->move_s = MoveRun ;
		entk->count3 = 0 ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_MoveWaitArea( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			entk->think3 = TH3_MOVE_WAITAREA_DIRECT ;
			entk->count3 = 0 ;
		} else {
			if ( entk->tmp_buff[0] == 0 ) {	/* モードの変わり目で無線無し */
				UNSET_FLAG( entk->c_notice, entk->tmp_buff[1] ) ;
				CLEAR_FLAG( entk->iknow_flag ) ;
				if ( entk->tmp_buff[1] & NOTICE_IKNOW_DURESSED ) {
					UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
				}
				entk->think3 = TH3_END ; 
				entk->count3 = 0 ;
				return ;
			} else {
				entk->think3 = TH3_REPAIR ;
				entk->count3 = 0 ;
			}
		}
		
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_MoveWaitAreaDirect( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 250 ) < 0 ) {
		if ( entk->tmp_buff[0] == 0 ) {	/* モードの変わり目で無線無し */
			UNSET_FLAG( entk->c_notice, entk->tmp_buff[1] ) ;
			CLEAR_FLAG( entk->iknow_flag ) ;
			if ( entk->tmp_buff[1] & NOTICE_IKNOW_DURESSED ) {
				UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
			}
			entk->think3 = TH3_END ; 
			entk->count3 = 0 ;
			return ;
		} else {
			entk->think3 = TH3_REPAIR ;
			entk->count3 = 0 ;
		}
		
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

/* 修理＆連絡 */
static	void	Think3_Repair( entk )
ENETHINK	*entk ;
{
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		entk->act->pad = SP_UNREAL ;
	}

	if ( entk->count3 == 0 ) {
		UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;
		ENE_ClearPDamage( entk->act ) ;
		entk->tmp_time = COM_SetRadio( EV_RAD_FIND_RAD, entk ) ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV1_STRM_LENGTH ;
		}
	}
//printf("repair radio count[%d] \n",entk->count3 ) ;

	if ( entk->count3 > entk->tmp_time ) {
		if ( ToSearchCheck( entk->tmp_buff[1] ) ) {
			COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
		}
		UNSET_FLAG( entk->c_notice, entk->tmp_buff[1] ) ;
		CLEAR_FLAG( entk->iknow_flag ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_NoRadio( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QR ) ;
//		COM_SetSpeak( EV_NOTICE_NORADIO, entk ) ; /* くそっ */
	}

	if ( entk->count3 > COUNT_VMODE(120) ) {
		UNSET_FLAG( entk->c_notice, entk->tmp_buff[1] ) ;
		if ( entk->tmp_buff[1] & NOTICE_IKNOW_DURESSED ) {
			UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		}
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

