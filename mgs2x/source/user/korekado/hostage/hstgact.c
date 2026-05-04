/*
	hstgact.c
	人質思考処理

	2001/03/22 Y.Korekado
	$Id: hstgact.c,v 1.1.1.3 2002/11/19 11:44:17 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/
/* アクション */
enum {
	PAD_NONE,
	PAD_STRAIN,	/* 緊張する */
	PAD_JITABATA,	/* 暴れる */
	PAD_TERROR,	/* 恐怖 */
	PAD_STRAIGHT_FOOT,	/* 足伸ばす */
	PAD_FOOT_END,	/* 足引っ込める */
	PAD_SITFAST,	/* 補完早い*/
} ;
/*-------------------------------------------------------------*/
static void ActSit( NPCWORK *npc, int time ) ;
static void ActSitFast( NPCWORK *npc, int time ) ;
static void ActStrain( NPCWORK *npc, int time ) ;
static void ActJitabata( NPCWORK *npc, int time ) ;
static void ActStraightFoot( NPCWORK *npc, int time ) ;
static void ActStraightFootIdle( NPCWORK *npc, int time ) ;
static void ActStraightFootEnd( NPCWORK *npc, int time ) ;
/*-------------------------------------------------------------*/
#define	ADJ_PIKU_TIME	COUNT_VMODE(6)


/*----------------------------------------------------------------*/
/*
*NewBlood( FMATRIX *, FVECTOR *, FVECTOR *force, int mode, int white );
[force]:吹き出しベクトル
[mode ]:０：通常、 １：大量出血
[white]:0:赤 1:白
*/
#define	NPC_BLOOD_NORMAL	(0)
#define	NPC_BLOOD_MUCH		(1)
void NPC_Blood( NPCWORK *npc, int mode )
{
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	FMATRIX	*world ;
	TARGET	*dam ;
	NPCTARGET	*npctrg ;

	npctrg = &npc->target ;
	dam = npctrg->dam_trg ;
	world = &(BODYWORLD( npc->body, npctrg->dam_obj )) ;
//printf("damtrg force =  [%f] [%f] [%f] \n",dam->power->force.vx,dam->power->force.vy,dam->power->force.vz) ;
	GV_SetActorChild( npc->character, NewBlood( world, &dam->center, &npctrg->force, mode, 0 ) ) ;
}

static	int SetOffenseTarget( NPCWORK *npc, int n )
{
	Work			*work ;
	TARGET			*off ;
	POWER_TARGET	*power ;
	FVECTOR			force ;
	static FVECTOR Size = { 300.0F,300.0F,300.0F } ;

	work = (Work *)npc->character ;

	off = &( work->attack ) ;
	power = &( work->power ) ;

	force.vy = 0.0f ;
	force.vx = 100.0f * _RsinF( (int)GM_PlayerControl->turn.vy ) ;
	force.vz = 100.0f * _RcosF( (int)GM_PlayerControl->turn.vy ) ;

	GM_SetTarget( off, TARGET_OFFENSE , 0, PLAYER_SIDE, &Size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_TUMBLE ) ; 
	GM_MoveTarget3( off, &( BODYWORLD(npc->body, 16 ) ) ) ;

	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, 0, &force ) ;
	GM_PutTarget( off ) ;

//NewTargetView( off, 34, 184, 200 ) ;

	return 0 ;
}

/*----------------------------------------------------------------*/
static void ActSit( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;


	if ( time == 0 ) {
		NPC_SetActMotionEX( npc, LAYER_BASE,
			npc->base_mar, 0, MOTION_MASK_FULL, 300, (float)TIME_BASE ) ;
//		NPC_SetActMotion( npc, npc->base_mar, 0 ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( npc->CheckPad( npc ) ) return ;

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActSitFast( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, 0 ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActStrain( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;

	if ( time == 0 ) {
		Work *w ;
		
		w = (Work *)npc->character ;
		if ( w->status & HSTG_STATUS_OL ) {
			NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		} else {
			NPC_SetActMotionEX( npc, LAYER_BASE,
				act->set_mar, act->set_mot, MOTION_MASK_FULL, 150, (float)TIME_BASE ) ;
		}
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActJitabata( NPCWORK *npc, int time )
{
	NPCACT	*act ;
	int 	mot_leng, mot_time ;

	act = &npc->action ;

	if ( time == 0 ) {
		GM_SetNoise( NOISE_MM , &npc->ctrl->mov, npc->ctrl->hzx_id ) ;
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
//		GM_SeSetMode( SD_A_HOSTMAFR, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	mot_leng = (int)(npc->body->m_ctrl->mt3_ctrl->motion_total_time / npc->body->m_ctrl->mt3_ctrl->motion_time_base) ;
	mot_time = time%mot_leng ;

	if ( mot_time == COUNT_VMODE(100) || mot_time == COUNT_VMODE(110) || 
		 mot_time == COUNT_VMODE(120) || mot_time == COUNT_VMODE(130) ||
		 mot_time == COUNT_VMODE(140) ) {
//		GM_SeSetMode( SD_A_TABLEGG1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( GM_CheckObject_PlayEnd( npc->body, 0 ) ) {
//		GM_SeSetMode( SD_A_HOSTMAFR, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;

		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
			act->act_end = 1 ;
			NPC_SetActMode( npc, ActSit ) ;
			return ;
		}
	}


	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActTerror( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;

	NPC_ActStatus( act, NPC_ACT_STATUS_TERROR ) ;

#if 1
	if ( time == 0 ) {
//		GM_SetNoise( NOISE_MM , &npc->ctrl->mov, npc->ctrl->hzx_id ) ;
	}

	if ( (time%4) == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, HSTG_MOT_SLEEP ) ;
	}
	if ( (time%4) == 2 ) {
		NPC_SetActMotion( npc, act->set_mar, HSTG_MOT_DAM ) ;
	}
#else
	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}
#endif

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActStraightFoot( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, HSTG_MOT_2_STRAIGHT_FOOT ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}

	if ( time == COUNT_VMODE(5) ) {
		SetOffenseTarget( npc, 0 ) ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 3 ;
		NPC_SetActMode( npc, ActStraightFootIdle ) ;
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActStraightFootIdle( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, HSTG_MOT_STRAIGHT_FOOT ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActStraightFootEnd( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, HSTG_MOT_BACK_FOOT ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActSit ) ;
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}


/*-------------------------------------------------------------*/
static int	HstgCheckPad( NPCWORK *npc )
{
	NPCACT	*act ;

	act = &npc->action ;
	if ( act->pad == PAD_NONE ) return 0 ;

	switch ( act->pad ) {
		case PAD_SITFAST :
			NPC_SetModeFromPad( npc, ActSitFast, npc->base_mar, HSTG_MOT_SIT, act->pad ) ;
		break ;
		case PAD_STRAIN :
			NPC_SetModeFromPad( npc, ActStrain, npc->base_mar, HSTG_MOT_STRAIN, act->pad ) ;
		break ;
		case PAD_JITABATA :
			NPC_SetModeFromPad( npc, ActJitabata, npc->base_mar, HSTG_MOT_STRUGGLE, act->pad ) ;
		break ;
		case PAD_TERROR :
			NPC_SetModeFromPad( npc, ActTerror, npc->base_mar, HSTG_MOT_SLEEP_IDLE, act->pad ) ;
		break ;
		case PAD_STRAIGHT_FOOT :
			NPC_SetModeFromPad( npc, ActStraightFoot, npc->base_mar, HSTG_MOT_2_STRAIGHT_FOOT, act->pad ) ;
		break ;
		case PAD_FOOT_END :
			NPC_SetModeFromPad( npc, ActStraightFootEnd, npc->base_mar, HSTG_MOT_BACK_FOOT, act->pad ) ;
		break ;
	}
	return 1 ;
}


/*----- ダメージアクション -----------------------------------------------------*/
static void ActDamage( NPCWORK *npc, int time ) ;
static void ActSleep( NPCWORK *npc, int time ) ;
static void ActSleepIdle( NPCWORK *npc, int time ) ;
static void ActWakeUp( NPCWORK *npc, int time ) ;
static void ActDeath( NPCWORK *npc, int time ) ;
/*-------------------------------------------------------------*/
#define SMP_PUNCH_DAM (1)
#define SMP_KICK_DAM (3)
#define SMP_THROW_DAM (5)
#define SMP_ONEDOWN_DAM (100)
#define FAINT_DAM (300)
/*----------------------------------------------------------------*/
static	int	HstgSleepCheckDamage( NPCWORK	*npc )
{
	TARGET	*def ;
	NPCACT	*act ;
	NPCTARGET	*trg ;
	long64	weapon ;
	int		dam_child_num ;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

	if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		trg->weapon_type = trg->dam_trg->weapon_type ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageFlagClear( npc ) ;
		printf(" child damage [%d]/[%d]\n",dam_child_num,npc->target.child_trg_num ) ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( weapon & WP_M92 ) {
			NPC_SetNeedlV( npc->body,trg->dam_obj, &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_ZZZ ) ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
		} else if ( weapon & (WP_BULLET) ) {
			if ( dam_child_num < 2 ) {
				act->life = 0 ;
			} else {
				act->life -= 10 ;
			}
			if ( act->life <= 0 ) {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
				return 1 ;
			} else {
				act->adj_piku_time = ADJ_PIKU_TIME ;
				act->faint_count -= FAINT_DAM ;
			}
		} else if ( weapon & (WP_PUNCHR|WP_PUNCHL ) ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			}
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		} else if ( weapon & WP_KICK ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				GM_SeSetMode( SD_P_NKTPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		} else {
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		}
		return 0 ;
	}


	if ( TARGET_POWER & def->damaged ) {
		printf("npc damage weapon type [%lx]  \n",weapon);

		weapon = def->weapon_type ;
		NPC_DamageFlagClear( npc ) ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( weapon & (WP_PUNCHR|WP_PUNCHL ) ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			}
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		} else if ( weapon & WP_KICK ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				GM_SeSetMode( SD_P_NKTPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
		} else if ( weapon & WP_COLDSPRAY ) {
			if ( act->faint_count > COUNT_VMODE(100) ) {
				act->faint_count -= COUNT_VMODE(100) ;
			}
			act->adj_piku_time = ADJ_PIKU_TIME ;
		} else if ( weapon & WP_M92 ) {
			NPC_SetNeedlV( npc->body, HUMAN21_MUNE, &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_ZZZ ) ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
		} else if ( weapon & (WP_THROWG) ) {
			act->adj_piku_time = ADJ_PIKU_TIME ;
		} else if ( weapon & (WP_BULLET) ) {
			act->life -= 10 ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->faint_count -= FAINT_DAM ;
			if ( act->life <= 0 ) {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
				return 1 ;
			}
		} else if ( weapon & (WP_BLAST) ) {
			act->life = 0 ;
			NPC_Blood( npc, NPC_BLOOD_MUCH ) ;
			NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			return 1 ;
		}
		return 0 ;
	}

	return 0 ;
}
/*-------------------------------------------------------------*/
static void ActDamage( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;

	if ( act->life <= 0 ) {
		NPC_ActStatus( act, NPC_ACT_STATUS_MUST_DIE  ) ;
	}

	if ( time == 0 ) {
		Work *w ;
		int se ;
		
		w = (Work *)npc->character ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

		if ( w->se_interval <= 0 ) {
			GM_SetNoise( NOISE_MM , &npc->ctrl->mov, npc->ctrl->hzx_id ) ;
			if ( npc->target.weapon_type & WP_COLDSPRAY ) w->se_interval = 120 ;

			se = 0 ;
			if ( w->status & HSTG_STATUS_OL ) {
				if ( act->life <= 0 ) {
					se = SD_A_HOSTFOU2 ;
				} else {
					se = SD_A_HOSTFDM2  ;
				}
			} else if ( w->status & HSTG_STATUS_WOMEN ) {
				if ( act->life <= 0 ) {
					se = SD_A_HOSTFOUT ;
				} else {
					se = SD_A_HOSTFDMG  ;
				}
			} else {
				if ( act->life <= 0 ) {
					se = (w->id%2)?SD_A_HOSTMOUT:SD_A_HOSTMOU2 ;
				} else {
					se = (w->id%2)?SD_A_HOSTMDMG:SD_A_HOSTMDM2 ;
				}
			}
			GM_SeSetMode( se , &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
		if ( act->life <= 0 ) {
			KR_KillCount() ;
		}
	}

	if ( act->life > 0 ) {
		if ( npc->CheckDamage( npc ) ) return ;
	} else {
		NPC_ActStatus( act, NPC_ACT_STATUS_DEATH  ) ;
	}
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		if ( act->life <= 0 ) {
			NPC_SetActMode( npc, ActDeath ) ;
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 0 ].flag, MT3_SLEEP ) ;
		} else if ( act->faint <= 0 ) {
			NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
			NPC_SetActMode( npc, ActSleep ) ;
		} else {
			act->act_end = 1 ;
			NPC_SetActMode( npc, ActSit ) ;
		}
		return ;
	}
}

static void ActSleep( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		if ( act->life > 0 ) {
			if ( act->faint_mode == NPC_FAINT_ZZZ ) {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_ZZZ ) ;
				NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
			} else {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
				NPC_CallHeadMark( npc, HMK2_TYPE_PIYO  ) ;
			}
			NPC_SetActMode( npc, ActSleepIdle ) ;
		} else {
			NPC_SetActMode( npc, ActDeath ) ;
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 0 ].flag, MT3_SLEEP ) ;
		}
		return ;
	}
}

static void ActSleepIdle( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, (NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_FAINT)  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, HSTG_MOT_SLEEP_IDLE ) ;
	}

	if ( HstgSleepCheckDamage( npc ) ) {
		if ( act->life <= 0 ) {
			NPC_CallHeadMark( npc, HMK2_TYPE_KILL  ) ;
		}
		return ;
	}


	if ( act->faint_mode == NPC_FAINT_ZZZ ) {
		if ( !(time%COUNT_VMODE(120)) ) {
			Work *w ;
			int se ;
			
			w = (Work *)npc->character ;
			se = 0 ;
			if ( w->status & HSTG_STATUS_OL ) {
				se = SD_A_HOSTFSL2  ;
			} else if ( w->status & HSTG_STATUS_WOMEN ) {
				se = SD_A_HOSTFSLP  ;
			} else {
				se = (w->id%2)?SD_A_HOSTMSLP:SD_A_HOSTMSL2 ;
			}
			GM_SeSetMode( se , &npc->ctrl->mov, GM_SEMODE_REAL ) ;
		}
	}

	{
		Work *w ;
		w = (Work *)npc->character ;
		if ( w->status & HSTG_STATUS_OL ) {
			int p_time ;
			
			p_time = KR_MotionTime( &npc->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
			if ( p_time > COUNT_VMODE(300) || p_time < COUNT_VMODE(20) ) {
				NPC_ActStatus( act, NPC_ACT_STATUS_OPEN_SKIRT ) ;
			}
		}
	}

	if ( act->life > 0 ) {
		if ( act->faint_count <= 0 ) {
			NPC_SetActMode( npc, ActWakeUp ) ;
			return ;
		}
	}
}

static void ActWakeUp( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	if ( time == 0 ) {
		extern void SearchAndFallAttachment_called(OBJECT *,int ,int ,int ) ;

		SearchAndFallAttachment_called( npc->body, 0, COUNT_VMODE(30), 16 ) ;
		NPC_CallHeadMark( npc, HMK2_TYPE_KILL  ) ;
		NPC_SetActMotion( npc, npc->base_mar, HSTG_MOT_WAKE_UP ) ;
		/* 眠り、気絶から覚める時は 気絶値最大にする */
		act->faint = SMP_FAINT ;
	}

//	if ( npc->CheckDamage( npc ) ) return ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActSit ) ;
		return ;
	}
}

static void ActDeath( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_DEATH  ) ;

	if ( time == 0 ) {
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 0 ].flag, MT3_SLEEP ) ;
//		NPC_SetActMotion( npc, npc->base_mar, HSTG_MOT_SLEEP_IDLE ) ;
	}

//	if ( npc->CheckDamage( npc ) ) return ;

//	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
//	}
}

static	int	HstgCheckDamage( NPCWORK	*npc )
{
	TARGET	*def ;
	NPCACT	*act ;
	NPCTARGET	*trg ;
	long64	weapon ;
	int		dam_child_num ;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

	if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		trg->weapon_type = trg->dam_trg->weapon_type ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageFlagClear( npc ) ;
		printf(" child damage [%d]/[%d]\n",dam_child_num,npc->target.child_trg_num ) ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( weapon & WP_M92 ) {
			NPC_SetNeedlV( npc->body,trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_SLEEP, act->pad ) ;
			act->faint_mode = NPC_FAINT_ZZZ ;
		} else if ( weapon & (WP_BULLET) ) {
			if ( dam_child_num < 2 ) {
				act->life = 0 ;
			} else {
				act->life -= 10 ;
			}
#ifdef DEBUG_MODE
{
	Work *w ;
	
	w = (Work *)npc->character ;
	printf(" dam id [%d] \n",w->id ) ;
}
#endif
			if ( act->life <= 0 ) {
				NPC_Blood( npc, NPC_BLOOD_MUCH ) ;
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_OUT, act->pad ) ;
			} else {
				NPC_Blood( npc, NPC_BLOOD_NORMAL ) ;
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
		} else if ( weapon & (WP_PUNCHR|WP_PUNCHL ) ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->faint -= SMP_PUNCH_DAM*2 ;
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->faint -= SMP_PUNCH_DAM*3 ;
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->faint -= SMP_PUNCH_DAM ;
			}
			if ( act->faint <= 0 ) {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
				NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			} else {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
			NPC_ActStatus( act, NPC_ACT_STATUS_PUNCH_DAMAGE  ) ;
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->faint -= SMP_PUNCH_DAM ;
		} else if ( weapon & WP_KICK ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				act->faint -= SMP_ONEDOWN_DAM ;
				GM_SeSetMode( SD_P_NKTPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				act->faint -= SMP_KICK_DAM ;
				GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
			if ( act->faint <= 0 ) {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
				NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			} else {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
			NPC_ActStatus( act, NPC_ACT_STATUS_PUNCH_DAMAGE  ) ;
		} else {
			if ( act->life <= 0 ) {
				NPC_Blood( npc, NPC_BLOOD_MUCH ) ;
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_OUT, act->pad ) ;
			} else {
				NPC_Blood( npc, NPC_BLOOD_NORMAL ) ;
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
		}
		return 1 ;
	}

	if ( TARGET_POWER & def->damaged ) {

		trg->dam_trg = def ;
		trg->dam_obj = HUMAN21_MUNE ;
		trg->weapon_type = def->weapon_type ;

		weapon = def->weapon_type ;
		NPC_DamageFlagClear( npc ) ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		printf("npc damage weapon type [%lx]  \n",weapon);
		if ( weapon & (WP_PUNCHR|WP_PUNCHL ) ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->faint -= SMP_PUNCH_DAM*2 ;
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->faint -= SMP_PUNCH_DAM*3 ;
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->faint -= SMP_PUNCH_DAM ;
			}

			if ( act->faint <= 0 ) {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
				NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			} else {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
			NPC_ActStatus( act, NPC_ACT_STATUS_PUNCH_DAMAGE  ) ;
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->faint -= SMP_PUNCH_DAM ;
			if ( act->faint <= 0 ) {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
				NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			} else {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
			NPC_ActStatus( act, NPC_ACT_STATUS_PUNCH_DAMAGE  ) ;
		} else if ( weapon & WP_KICK ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				GM_SeSetMode( SD_P_NKTPNC01, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
			act->faint -= SMP_KICK_DAM ;
			if ( act->faint <= 0 ) {
				NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
				NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			} else {
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
			GM_SeSetMode( SD_P_KICK02, &npc->ctrl->mov, GM_SEMODE_NORMAL ) ;
			NPC_ActStatus( act, NPC_ACT_STATUS_PUNCH_DAMAGE  ) ;
		} else if ( weapon & WP_M92 ) {
			NPC_SetNeedlV( npc->body, HUMAN21_MUNE, &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, HSTG_MOT_SLEEP, act->pad ) ;
			act->faint_mode = NPC_FAINT_ZZZ ;
		} else if ( weapon & (WP_BULLET) ) {
			act->life -= 10 ;
			if ( act->life <= 0 ) {
				NPC_Blood( npc, NPC_BLOOD_MUCH ) ;
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_OUT, act->pad ) ;
			} else {
				NPC_Blood( npc, NPC_BLOOD_NORMAL ) ;
				NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
			}
		} else if ( weapon & WP_BLAST ) {
			act->life = 0 ;
			NPC_Blood( npc, NPC_BLOOD_MUCH ) ;
			NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_OUT, act->pad ) ;
		} else if ( weapon & (WP_THROWG) ) {
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else {
			NPC_SetModeFromPad( npc, ActDamage, npc->base_mar, HSTG_MOT_DAM, act->pad ) ;
		}
		return 1 ;
	}

	return 0 ;
}
/*----------------------------------------------------------------*/
static void Action( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

//	NPC_ActInit( npc ) ;
	NPC_Action( npc ) ;
	
	if ( work->status & HSTG_STATUS_NO_TARGET ) {
		NPC_ActStatus( &npc->action, NPC_ACT_STATUS_TRG_OFF  ) ;
	}
 	NPC_ActStatusCheck( npc ) ;	/* 首振り実験 */

	NPC_Gravitation( npc ) ;
}

