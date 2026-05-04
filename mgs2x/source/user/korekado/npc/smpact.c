/*
	smpthink.c
	NPCサンプル思考処理

	2001/02/09 Y.Korekado
	$Id: smpact.c,v 1.1.1.3 2002/11/19 11:44:23 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/
/* アクション */
enum {
	PAD_NONE,
	PAD_SQUAT,	/* しゃがむ */
	PAD_SLIDING,	/* 滑り込み */
	PAD_SUMMER,		/* サマーソルト */
	PAD_FAT_RUN,		/* */
	PAD_FAT_READY,		/* */
	PAD_FAT_SHOT,		/* */
	PAD_WAKEUP,		/* */

} ;
/*-------------------------------------------------------------*/
static void Attack( NPCWORK *npc )
{
	Work	*work ;
	TARGET	*off ;
	POWER_TARGET	*power ;
	FVECTOR			v ;
	static FVECTOR Size = { 500.0F,500.0F,500.0F } ;
	static FVECTOR Force = { 0.0F,-100.0F,100.0F } ;

	work = (Work *)npc->character ;

	off = &( work->attack ) ;
	power = &( work->power ) ;

	GM_SetTarget( off, TARGET_OFFENSE , 0, PLAYER_SIDE, &Size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, WP_KICK ) ; 
	GM_MoveTarget3( off, &( BODYWORLD(npc->body, 16 ) ) ) ;
	DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot ) ;
    DG_RotVector( &Force, &v, 1 ) ;

	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, 20, &v ) ;
	GM_PutTarget( off ) ;
}

/*-------------------------------------------------------------*/
static void ActSummerSolt( NPCWORK *npc, int time ) ;
static void ActFatRun( NPCWORK *npc, int time ) ;
//static void ActMutekiOneTime( NPCWORK *npc, int time ) ;
static void ActGunStand( NPCWORK *npc, int time ) ;
static void ActGunShoot( NPCWORK *npc, int time ) ;

static void ActDamage( NPCWORK *npc, int time ) ;
static void ActFall( NPCWORK *npc, int time ) ;
static void ActDown( NPCWORK *npc, int time ) ;
static void ActWakeup( NPCWORK *npc, int time ) ;


/*-------------------------------------------------------------*/
static void ActSummerSolt( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( time > COUNT_VMODE(10) && time < COUNT_VMODE(20) ) Attack( npc ) ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static	void ActFatRun( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;

	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE  ) ;

	if ( time == 0 ) {
		Work	*work ;
		
		work = (Work *)npc->character ;
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_MOVE ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*work->speed ) ;
	}

	Attack( npc ) ;

	if ( npc->CheckDamage( npc ) ) {
		ctrl->turn.vz = 0 ;
		return ;
	}
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir < 0 ) {
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		ctrl->turn.vz = 0 ;
		return ;
	}

	/* 方向転換時の体の傾き */
	NPC_Incline( ctrl ) ;
	/* 進行方向 */
	ctrl->turn.vy = act->dir ;

#if 0
	if ( speed != 0.0F ) {
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
		ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
	}
#endif
}

/* ループモーション */
static void ActSquat( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

#if 0
	NPC_SetActMotionEX( npc, 1,
	npc->base_mar, NPC_MOT_MOVE, MOTION_MASK_UPPER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 1 ].flag, MT3_ACTIVE ) ;
#endif
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

#if 0//NO USE
static void ActMutekiOneTime( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}

	if ( npc->CheckPad( npc ) ) 	return ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}
#endif

static void ActGunStand( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_STAND ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActGunShoot( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_STAND ) ;
{
		extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
		SVECTOR	rot ;
		FMATRIX	w ;
		DG_SetPos( &BODYWORLD( npc->body, HUMAN21_MIGI_TE ) ) ;
//		rot.vx = KR_RandS( 64 )  ;
//		rot.vy = KR_RandS( 64 )  ;

		rot.vx = KR_RandS( 128 )  ;
		rot.vy = KR_RandS( 128 )  ;
		rot.vz = 0 ;
		DG_RotatePos( &rot ) ;
		DG_GetPos( &w ) ;
		NewBullet( &w, BUL_TYPE_VISIBLE|BUL_TYPE_GREEN,PLAYER_SIDE,25,5,20000,1000,WP_Famas ) ;
}

	}

	if ( npc->CheckDamage( npc ) ) return ;
	if ( npc->CheckPad( npc ) ) return ;

	if ( time == 4 ) {
		NPC_SetActMode( npc, ActGunStand ) ;
		return ;
	}
}

static void ActWakeup( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;

	if ( time == 0 ) {
		int mot ;

		mot = SMP_MOT_WAKEUP_F ;
		if ( act->faint <= 0 ) {
			mot = SMP_MOT_WAKEUP_SLOW_F ;
			act->faint = SMP_FAINT ;
		}
		NPC_SetActMotion( npc, npc->damage_mar, mot ) ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
}

/*-------------------------------------------------------------*/
static int	SampleCheckPad( NPCWORK *npc )
{
	NPCACT	*act ;

	act = &npc->action ;
	if ( act->pad == PAD_NONE ) return 0 ;

	switch ( act->pad ) {
		case PAD_SQUAT :
//			NPC_SetModeFromPad( npc, NPC_ActLoopMotion, npc->base_mar, SMP_MOT_SQUAT, act->pad ) ;
			NPC_SetModeFromPad( npc, ActSquat, npc->base_mar, SMP_MOT_SQUAT, act->pad ) ;
		break ;
		case PAD_SLIDING :
			NPC_SetModeFromPad( npc, NPC_ActOneTimeMotion, npc->base_mar, SMP_MOT_SLIDING, act->pad ) ;
		break ;
		case PAD_SUMMER :
			NPC_SetModeFromPad( npc, ActSummerSolt, npc->base_mar, SMP_MOT_SUMMER, act->pad ) ;
		break ;

		case PAD_FAT_RUN :
			NPC_SetModeFromPad( npc, ActFatRun, npc->base_mar, SMP_MOT_SUMMER, act->pad ) ;
		break ;
		case PAD_FAT_READY :
			NPC_SetModeFromPad( npc, ActGunStand, npc->base_mar, NPC_MOT_STAND, act->pad ) ;
		break ;
		case PAD_FAT_SHOT :
			NPC_SetModeFromPad( npc, ActGunShoot, npc->base_mar, NPC_MOT_STAND, act->pad ) ;
		break ;
		case PAD_WAKEUP :
			NPC_SetModeFromPad( npc, ActWakeup, npc->damage_mar, SMP_MOT_WAKEUP_F, act->pad ) ;
		break ;
	}
	return 1 ;
}

/*----- ダメージアクション -----------------------------------------------------*/
static void ActDamage( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
}

static void ActFall( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		/* モーション遅いので再生スピードUP */
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*2.0f ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		if ( act->faint > 0 ) {
			NPC_SetActMode( npc, ActDown ) ;
		} else {
			NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
			NPC_CallHeadMark( npc, HMK2_TYPE_PIYO  ) ;
			NPC_SetActMode( npc, NPC_ActFaint ) ;
		}
		return ;
	}
}

static void ActDown( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->damage_mar, SMP_MOT_DOWN_F ) ;
	}

	if ( time > COUNT_VMODE(60) ) {
		NPC_SetActMode( npc, ActWakeup ) ;
		return ;
	}
}

static void ActSleepDown( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetFaintCount( npc, act->sleep_max, NPC_FAINT_ZZZ ) ;
		NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
		NPC_SetActMode( npc, NPC_ActFaint ) ;
		return ;
	}
}

#define SMP_PUNCH_DAM (1)
#define SMP_KICK_DAM (3)
#define SMP_THROW_DAM (5)
/*----------------------------------------------------------------*/
static	int	SmpCheckDamage( NPCWORK	*npc )
{
	TARGET	*def ;
	NPCACT	*act ;
	NPCTARGET	*trg ;
	CAPTURE_TARGET	*cap ;
	long64	weapon ;
	int		dam_child_num ;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

	if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
		trg->dam_trg = trg->def_child + dam_child_num ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageCaptureFlagClear( npc ) ;
		printf(" child damage [%d]/[%d]\n",dam_child_num,npc->target.child_trg_num ) ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( weapon & WP_M92 ) {
			NPC_SetNeedl( npc->body,trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			NPC_SetModeFromPad( npc, ActSleepDown, npc->damage_mar, SMP_MOT_DAM_KICK, act->pad ) ;
		} else {
			NPC_SetModeFromPad( npc, ActDamage, npc->damage_mar, SMP_MOT_DAM_PUNCH_R, act->pad ) ;
		}

		act->life -= 10 ;
		return 1 ;
	}


	if ( TARGET_POWER & def->damaged ) {
		printf("smp damage weapon type [%lx]  \n",weapon);

		weapon = def->weapon_type ;
		NPC_DamageCaptureFlagClear( npc ) ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( weapon & WP_PUNCHR ) {
			act->faint -= SMP_PUNCH_DAM ;
			if ( act->faint > 0 ) 
				NPC_SetModeFromPad( npc,ActDamage, npc->damage_mar, SMP_MOT_DAM_PUNCH_R, act->pad ) ;
			else
				NPC_SetModeFromPad( npc,ActFall, npc->damage_mar, SMP_MOT_DAM_KICK, act->pad ) ;
		} else if ( weapon & WP_PUNCHL ) {
			act->faint -= SMP_PUNCH_DAM ;
			if ( act->faint > 0 ) 
				NPC_SetModeFromPad( npc,ActDamage, npc->damage_mar, SMP_MOT_DAM_PUNCH_L, act->pad ) ;
			else
				NPC_SetModeFromPad( npc,ActFall, npc->damage_mar, SMP_MOT_DAM_KICK, act->pad ) ;
		} else if ( weapon & WP_KICK ) {
			act->faint -= SMP_KICK_DAM ;
			NPC_SetModeFromPad( npc, ActFall, npc->damage_mar, SMP_MOT_DAM_KICK, act->pad ) ;
		} else if ( weapon & WP_M92 ) {
			NPC_SetNeedl( npc->body, HUMAN21_MUNE, &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			NPC_SetModeFromPad( npc, ActSleepDown, npc->damage_mar, SMP_MOT_DAM_KICK, act->pad ) ;
		}
		act->life -= 10 ;
		return 1 ;
	}

	if ( TARGET_CAPTURE & def->damaged ) {
		NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
		cap = trg->capture ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( cap->flag & CAPTURE_HANG ) {	/* 首絞め */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;
			NPC_SetModeFromPad( npc, NPC_ActHang, npc->capture_mar, NPC_MOT_HANG_TIE, act->pad ) ;
		} else if ( cap->flag & CAPTURE_THROW ) {	/* 投げ */
			act->faint -= SMP_THROW_DAM ;
			NPC_SetModeFromPad( npc, NPC_ActThrow, npc->capture_mar, NPC_MOT_THROW, act->pad ) ;
		}
		cap->flag = 0 ;
		act->life -= 10 ;
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
	NPC_ActStatusCheck( npc ) ;

	NPC_Gravitation( npc ) ;
}

