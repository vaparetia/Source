/*
	clemode.c
	クリアリングモードチェンジ
	
	2000/08/20 Y.Korekado
	$Id: clemode.c,v 1.1.1.3 2002/11/19 11:43:59 Yoshizawa1 Exp $
*/
#define CLE_NPC_FIND (1) //02.10.21

static	void	ClearingMoveChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH) ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 物音は無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 跡は無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		/* 近ダンボールは無視 */
	}

	/* 他モードへ移項 */
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( NowDamage( entk->dam_entk ) ) {
printf("korekorekroe1\n");
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else if ( COM_SameGroupUnit( entk, entk->dam_entk ) ) {
			/* 同じチームなら起こす */
			entk->think2 = TH2_ENE_DAMAGE ;
			THK_EneDamageModeStart( entk ) ;
		}
	}

	/* 発見モードへ */
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
printf("1korekorekroe damage to enedamaege1\n");
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NEAR_TOUCH ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("2korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_BOX ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("3korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("4korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("5korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("6korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("7korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("8korekorekroe damage to enedamaege1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
printf("9korekorekroe damage to enedamaege1\n");
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
printf("korekorekroe [%d]\n",entk->npc_eyei.sight);
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

/* 仲間の他モード移行と同時発生でも優先されるものに注意 */
static	void	ClearingChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}

	/* 他モードへ移行 */
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		if ( Cle->iknow_flag & CLE_IKNOW_BLOOD ) {
			THK_TraceModeStartCleFast( entk ) ;
		} else {
			THK_TraceModeStartCle( entk ) ;
			SET_FLAG( Cle->iknow_flag, CLE_IKNOW_BLOOD ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_BOX ) {
		if ( COM_CheckBoxPos( &GM_PlayerPosition ) ) {
			entk->think2 = TH2_BOX ;
			THK_BoxModeClearingStart( entk ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		if ( Cle->noise_num > CLE_NOISE_ALERT_NUM ) {
			entk->think2 = TH2_DISCOVERY ; 
printf("9korekorekroe damage to enedamaege1\n");
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_NOISE ;
			THK_NoiseModeStartClearing( entk ) ;
			return ;
		}
	}

	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( NowDamage( entk->dam_entk ) ) {
			entk->think2 = TH2_DISCOVERY ; 
printf("1aaaaaaaaaaaaakorekorekroe1\n");
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else if ( COM_SameGroupUnit( entk, entk->dam_entk ) ) {
			/* 同じチームなら起こす */
			entk->think2 = TH2_ENE_DAMAGE ;
			THK_EneDamageModeStart( entk ) ;
		}
	}

	/* 発見モードへ */
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("noise aaaaaaaaaaaaakorekorekroe1\n");
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("2aaaaaaaaaaaaakorekorekroe1\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
printf("Clearing clearing detect region\n");
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER_CLEARING_INTRUDE ) ;
		}
		return ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("3aaaaaaaaaaaaakorekorekroe1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
printf("clearing detect hold up\n");
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->alert >= DEF_EYE_DELAY ) {
printf("clearing detect def eye delay\n");
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("noise m aaaaaaaaaaaaakorekorekroe1\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
printf("clearing detect sight in\n");
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	ClearingCompChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* トレースモードは無視 */
	}

	/* 他モードへ移項 */
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->avoid = 0 ;	/*	com->avoidlevelがのこっていなければ終了 */
		UNSET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStartClearing( entk ) ;
	}

//printf("[%d]entk->notice[%x]sight[%d]\n",entk->id, entk->notice,entk->pl_eyei.sight);
	/* 他モードへ移項 */
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
//		if ( entk->dam_entk->act->old_status & (ACT_STATUS_DAMAGE|ACT_STATUS_MASUI_SASARU) ) {
		if ( NowDamage( entk->dam_entk ) ) {
			entk->think2 = TH2_DISCOVERY ; 
printf("1kokokaaaa-----rekorekroe damage to enedamaege1\n");
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
#if 0
		} else if ( COM_SameGroupUnit( entk, entk->dam_entk ) ) {
			/* 同じチームなら起こす */
			entk->think2 = TH2_ENE_DAMAGE ;
			THK_EneDamageModeStart( entk ) ;
#endif
		}
	}

	/* 発見 */
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("1teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("2teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		return ;
	}
	if ( (entk->notice & ENE_NOTICE_BOX) && (GM_PlayerStatus & PLAYER_MOVE) ) {
		/* 動いた */
		entk->think2 = TH2_DISCOVERY ; 
printf("3teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("4teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("5teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("6teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}

	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("7teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("8teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("9teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}

#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	NoiseChange( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* トレースモードは無視 */
	}

	/* 他モードへ移項 */
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 寝ている奴は起こす */
//		if ( entk->dam_entk->act->old_status & (ACT_STATUS_DAMAGE|ACT_STATUS_MASUI_SASARU) ) {
		if ( NowDamage( entk->dam_entk ) ) {
printf("7korekorekroe damage to enedamaege1\n");
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else if ( COM_SameGroupUnit( entk, entk->dam_entk ) ) {
			/* 同じチームなら起こす */
			entk->think2 = TH2_ENE_DAMAGE ;
			THK_EneDamageModeStart( entk ) ;
		}
	}

	/* 発見 */
	if ( (entk->notice & ENE_NOTICE_BOX) && (GM_PlayerStatus & PLAYER_MOVE) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("10teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("11teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("12teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		return ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("13teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("14teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("15teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}

	/* 何回も物音聞こえた */
	if ( entk->alert > ENE_INDISTINCT_ALERT_LEVEL ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("16teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("17teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	TraceChange( entk )
ENETHINK	*entk ;
{
	/* 反応無し */
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}

	/* 他のモードへ */
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		if ( Cle->noise_num > CLE_NOISE_ALERT_NUM ) {
			entk->think2 = TH2_DISCOVERY ; 
printf("18teset teset\n");
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			if ( entk->notice & ENE_NOTICE_NOISE ) {
				HZX_ZONE_ADD zoneaddr ;
				R_INTRPT *r_intrpt ;
#ifdef GMDEF_NOISE_MAP_RENEW
				zoneaddr = HZX_GetAddress( GM_NoiseHzxID, &GM_NoisePosition, -1 ) ;
#else
				int	 hzx_id, zone ;
				hzx_id = HZX_GetHzxIDbyZone( GM_GetHzxGroupID(GM_NoiseMap), &GM_NoisePosition, &zone ) ;
				zoneaddr = HZX_GetAddress( hzx_id, &GM_NoisePosition, -1 ) ;
#endif
				r_intrpt = GM_GetRIntrptOne( zoneaddr ) ;
				if ( r_intrpt != NULL ) {
					if ( ((r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED)) ==
						  (ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) ) 
						&& !(r_intrpt->status & (ROOT_INTRPT_NOENE|ROOT_INTRPT_BREAK)) ) {
						ENE_TraceEnd( entk ) ;
						entk->think2 = TH2_NOISE ;
						THK_NoiseModeStartClearing( entk ) ;
					}
				}
			}
		}
	}

	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 寝ている奴は起こす */
//		if ( entk->dam_entk->act->old_status & (ACT_STATUS_DAMAGE|ACT_STATUS_MASUI_SASARU) ) {
		if ( NowDamage( entk->dam_entk ) ) {
printf("6korekorekroe damage to enedamaege1\n");
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else if ( COM_SameGroupUnit( entk, entk->dam_entk ) ) {
			/* 同じチームなら起こす */
			entk->think2 = TH2_ENE_DAMAGE ;
			THK_EneDamageModeStart( entk ) ;
		}
	}

	/* 発見 */
	if ( (entk->notice & ENE_NOTICE_BOX) && (GM_PlayerStatus & PLAYER_MOVE) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("19teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_DISCOVERY ; 
printf("20teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("21teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		return ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("22teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("23teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_DISCOVERY ; 
printf("24teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_DISCOVERY ; 
printf("25teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			ENE_TraceEnd( entk ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	BoxChange( entk )
ENETHINK	*entk ;
{
	/* 発見 */
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
//		if ( entk->dam_entk->act->old_status & (ACT_STATUS_DAMAGE|ACT_STATUS_MASUI_SASARU) ) {
		if ( NowDamage( entk->dam_entk ) ) {
printf("5korekorekroe damage to enedamaege1\n");
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		}
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		ENE_TraceEnd( entk ) ;
		entk->think2 = TH2_DISCOVERY ; 
printf("26teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("27teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("29teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("30teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("31teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("32teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	NoticeSupportChange( entk )
ENETHINK	*entk ;
{
	/* 反応無し */
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}

	/* 発見 */
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		/* 寝ている奴は起こす */
//		if ( entk->dam_entk->act->old_status & (ACT_STATUS_DAMAGE|ACT_STATUS_MASUI_SASARU) ) {
		if ( NowDamage( entk->dam_entk ) ) {
printf("2kokokaaaa-----rekorekroe damage to enedamaege1\n");
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else if ( COM_SameGroupUnit( entk, entk->dam_entk ) ) {
			/* 同じチームなら起こす */
			entk->think2 = TH2_ENE_DAMAGE ;
			THK_EneDamageModeStart( entk ) ;
		}
	}
	if ( (entk->notice & ENE_NOTICE_BOX) && (GM_PlayerStatus & PLAYER_MOVE) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("33teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("34teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("35teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		return ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("36teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("37teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("38teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("39teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	EneDamageChange( entk )
ENETHINK	*entk ;
{
	/* 他モードへ移項 */
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 無視 */
#if 0
		HZX_ZONE_ADD zoneaddr ;
		R_INTRPT *r_intrpt ;

#ifdef GMDEF_NOISE_MAP_RENEW
		zoneaddr = HZX_GetAddress( GM_NoiseHzxID, &GM_NoisePosition, -1 ) ;
#else
		int	 hzx_id, zone ;
		hzx_id = HZX_GetHzxIDbyZone( GM_GetHzxGroupID(GM_NoiseMap), &GM_NoisePosition, &zone ) ;
		zoneaddr = HZX_GetAddress( hzx_id, &GM_NoisePosition, -1 ) ;
#endif
		r_intrpt = GM_GetRIntrptOne( zoneaddr ) ;
		if ( r_intrpt != NULL ) {
			if ( ((r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED)) == 
				(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) ) 
				&& !(r_intrpt->status & (ROOT_INTRPT_NOENE|ROOT_INTRPT_BREAK)) ) {
				ENE_TraceEnd( entk ) ;
				entk->think2 = TH2_NOISE ;
				THK_NoiseModeStartClearing( entk ) ;
			}
		}
#endif
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeNearStart( entk ) ;
	}

	/* 発見モードへ */
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("40teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("41teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		return ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("42teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("43teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("44teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("45teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}

static	void	DamageChange( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		/* 無視 */
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		/* 物音は無視 */
	}
	if ( entk->notice & ENE_NOTICE_FOUND ) {
		/*オブジェモードは無視 */
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		/* 跡は無視 */
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
		/* 死体は無視 */
	}
	if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
		/* 近ダンボールは無視 */
	}

	/* 他のモードへ */
	if ( entk->notice & ENE_NOTICE_NOISE_L ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( NowDamage( entk->dam_entk ) ) {
printf("4korekorekroe damage to enedamaege1\n");
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		}
	}
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("46teset teset\n");
		THK_DiscoveryModeStartAvoidDelay( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( (entk->alert > 0) || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
//	if ( entk->notice & (ENE_NOTICE_REGION|ENE_NOTICE_LIGHT) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("47teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		return ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE_M ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("49teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("50teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		if ( ENE_CheckPlayerHidden() ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("51teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("52teset teset\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#ifdef CLE_NPC_FIND
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryNpcModeStart( entk ) ;
		}
	}
#endif
}
