/*
	hstgthink.c
	人質思考処理

	2001/03/22 Y.Korekado
	$Id: hstgthink.c,v 1.1.1.3 2002/11/19 11:44:18 Yoshizawa1 Exp $
*/
#define SMP_DEBUG_THINK (1)
/*-----  --------------------------------------------------------*/
enum {	// work->think1
	TH1_NORMAL, TH1_NOISE, TH1_DAMAGE
} ;
enum {	// work->think2
	TH2_SIT,TH2_HSTG_DAMAGE, TH2_MM, TH2_ATTACK
} ;
enum {	// work->think3
	TH3_IDLE, TH3_WAIT, TH3_STRAIN, TH3_JITABATA, TH3_TERROR,
	TH3_SLEEP, TH3_SILENT, TH3_PEE, TH3_SOPPO, TH3_STRAIGHT_FOOT,
	TH3_FOOT_END, TH3_NO_EIMS, TH3_MAIL
} ;
/*-----  --------------------------------------------------------*/
static void CallVox( Work *work, int vox )
{
	work->strmhandler = HSTG_CallVox( vox ) ;
	work->strm = vox ;
}

static void AimReversePos( Work *work, FVECTOR *pos )
{
	FVECTOR pos_head, vec ;
	
	KR_FMatToFvec( &BODYWORLD( &work->body, HUMAN21_ATAMA ), &pos_head ) ;
	_sceVu0SubVector(  &vec, &pos_head, pos ) ;
	_sceVu0AddVector(  &work->npc.nadj->aim_pos, &pos_head, &vec ) ;
}

static FVECTOR foot_sft = { 0.0f,0.0f,200.0f } ;
static int StrainCheck( Work *work )
{
	if ( work->status & HSTG_STATUS_OL ) {
		if ( GM_CheckPlayerStatus(PLAYER_INTRUDE|PLAYER_WATCH) ) {
			FVECTOR	pos_west ;
			float f ;

			DG_SetPos2( &work->control.mov, &work->control.rot );
			DG_PutVector( &foot_sft, &pos_west, 1 ) ;

			f = KR_InnerProductInCamera( &pos_west ) ;
//AN_Test_Eye2( &pos_west, 1 );
			if ( GM_CheckPlayerStatus(PLAYER_GROUND) ) {
				if ( f > HOST_SE_COS_30 ) return 1 ;
			} else {
				if ( f > HOST_SE_COS_10 ) return 1 ;
			}
		}
	} else {
		if ( work->pl_dis < 2000 ) return 1 ;
	}

	return 0 ;
}

static int StrainOffCheck( Work *work )
{
	if ( work->status & HSTG_STATUS_OL ) {
		FVECTOR	pos_west ;
		float f ;
		DG_SetPos2( &work->control.mov, &work->control.rot );
		DG_PutVector( &foot_sft, &pos_west, 1 ) ;

		f = KR_InnerProductInCamera( &pos_west ) ;
//AN_Test_Eye2( &pos_west, 1 );
		if ( GM_CheckPlayerStatus(PLAYER_GROUND) ) {
			if ( f < HOST_SE_COS_45 ) return 1 ;
		} else {
			if ( f < HOST_SE_COS_15 ) return 1 ;
		}
	} else {
		if ( work->pl_dis > 4000 ) return 1 ;
	}

	return 0 ;
}

#define	CORP_AREA	(350.0f)
#define	CORP_AREA_H	(1250.0f)
static int AttackCheck( Work *work )
{
	FVECTOR *pos ;
	float	f ;

	if ( !(work->status & HSTG_STATUS_FOOT) ) return 0 ;

	pos = &work->on_corp ;

	f = pos->vx - GM_PlayerPosition.vx ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return 0 ;
	f = pos->vz - GM_PlayerPosition.vz ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return 0 ;
	f = pos->vy - GM_PlayerPosition.vy ;
	if ( f < -CORP_AREA_H || f > 0.0f ) return 0 ;

	return 1 ;
}

/*----- Think3 --------------------------------------------------------*/
static void Think3_SitIdle( Work *work )
{
	if ( AttackCheck( work ) ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_STRAIGHT_FOOT ;
		work->count3 = 0 ;
		return ;
	}
	if ( StrainCheck( work ) ) {
		work->think3 = TH3_STRAIN ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_Strain( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	npc->action.pad = PAD_STRAIN ;
	if ( AttackCheck( work ) ) {
		work->think2 = TH2_ATTACK ;
		work->think3 = TH3_STRAIGHT_FOOT ;
		work->count3 = 0 ;
		return ;
	}

	if ( work->count3 > COUNT_VMODE(180) ) {
		if ( StrainOffCheck( work ) ) {
			work->think3 = TH3_IDLE ;
			work->count3 = 0 ;
			return ;
		}
	}

	work->count3 ++ ;
}

/* 攻撃 */
static void Think3_StraightFoot( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	npc->action.pad = PAD_STRAIGHT_FOOT ;
	if ( work->count3 == 0 ) {
	}

	if ( work->count3 > COUNT_VMODE(180) ) {
		work->think3 = TH3_FOOT_END ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_StraightFootEnd( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	npc->action.pad = PAD_FOOT_END ;
	if ( work->count3 == 0 ) {
	}

	if ( npc->action.act_end == 1 ) {
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_NoiseSurprise( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		NPC_CallHeadMark( npc, HMK2_TYPE_WHT_AT  ) ;
		if ( work->status & HSTG_STATUS_MAIL ) {
			SET_FLAG ( work->status, HSTG_STATUS_MAIL_STOP ) ;
		}
	}

	if(GM_VRStatus & GM_VR_SNAKETALES) {
		if ( work->status & HSTG_STATUS_OL ) {
			npc->action.pad = PAD_STRAIN ;
		}
	}

	if ( work->status & (HSTG_STATUS_JITABATA|HSTG_STATUS_JITABATA2) ) {
		npc->action.pad = PAD_SITFAST ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	if ( !(work->status & HSTG_STATUS_SOPPO) ) {
		npc->nadj->aim_pos = GM_PlayerPosition ;
	}

	if ( work->count3 > COUNT_VMODE( 120 ) ) {
		if ( work->status & (HSTG_STATUS_JITABATA|HSTG_STATUS_JITABATA2) ) {
			work->think3 = TH3_JITABATA ;
		} else if( work->status & (HSTG_STATUS_NO_EIMS|HSTG_STATUS_NO_EIMS2) ) {
			work->think3 = TH3_NO_EIMS ;
		} else if( work->status & (HSTG_STATUS_JENIFA|HSTG_STATUS_DARE|HSTG_STATUS_NOEIMS_OL) ) {
			work->think3 = TH3_NO_EIMS ;
		} else if( work->status & (HSTG_STATUS_RINDA|HSTG_STATUS_CYNDI) ) {
			work->think3 = TH3_NO_EIMS ;
		} else if ( work->status & HSTG_STATUS_PEE ) {
			work->think3 = TH3_PEE ;
		} else if ( work->status & HSTG_STATUS_SOPPO ) {
			work->think3 = TH3_SOPPO ;
		} else if ( work->status & HSTG_STATUS_MAIL ) {
			work->think3 = TH3_MAIL ;
		} else {
			work->think3 = TH3_SILENT ;
		}
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_Jitabata( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		int	vox=0 ;
		
		if ( work->status & (HSTG_STATUS_JITABATA) ) {
			if ( work->status & HSTG_STATUS_OL ) {
//				vox = (work->quest_time%2)?VOX_WOM_HOKANO_2:VOX_WOM_PANIC_2 ;
				vox = VOX_WOM_PANIC_2 ;/* 音声が変なので */
			} else if ( work->status & HSTG_STATUS_WOMEN ) {
				vox = (work->quest_time%2)?VOX_WOM_HOKANO_1:VOX_WOM_PANIC_1 ;
			} else {
				vox = VOX_MAN_PANIC_1 ;
			}
		} else {
			if ( work->status & HSTG_STATUS_OL ) {
				vox = (work->quest_time%2)?VOX_WOM_HOKANO_1:VOX_WOM_PANIC_1 ;
			} else if ( work->status & HSTG_STATUS_WOMEN ) {
				vox = (work->quest_time%2)?VOX_WOM_HOKANO_2:VOX_WOM_PANIC_2 ;
			} else {
				vox = VOX_MAN_PANIC_2 ;
			}
		}
		CallVox( work, vox ) ;
	}

	if ( work->count3 == COUNT_VMODE( 180 ) ) {
		work->quest_time ++ ;
	}

	if ( work->pl_dis > 2000 ) {
//	if ( npc->action.act_end == 1 ) {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	npc->action.pad = PAD_JITABATA ;

//	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
//	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_Silent( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE  ) ;
	}

	if(GM_VRStatus & GM_VR_SNAKETALES) {
		if ( work->status & HSTG_STATUS_OL ) {
			npc->action.pad = PAD_STRAIN ;
		}
	}

//	if ( work->count3 > COUNT_VMODE( 180 ) ) {
	if ( work->pl_dis > 2000 ) {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_Mail( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		GM_SeSetMode( SD_A_KCEJ_TEL, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
	}
	if ( work->count3 == COUNT_VMODE(90) ) {
		CallVox( work, VOX_MEC_SENDMAIL ) ;
	}

	if ( work->count3 == COUNT_VMODE( 180 ) ) {
		NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE  ) ;
	}

	if ( work->count3 > COUNT_VMODE( 180 ) ) {
		if ( work->pl_dis > 2000 ) {
			work->think1 = TH1_NORMAL ;
			work->think2 = TH2_SIT ;
			work->think3 = TH3_IDLE ;
			work->count3 = 0 ;
			return ;
		}
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_NoEims( Work *work )
{
	NPCWORK	*npc ;
	int status ;

	npc = &work->npc ;

	if(GM_VRStatus & GM_VR_SNAKETALES) {
		if ( work->status & HSTG_STATUS_OL ) {
			npc->action.pad = PAD_STRAIN ;
		} else {
			npc->action.pad = PAD_SITFAST ;
		}
	} else {
		npc->action.pad = PAD_SITFAST ;
	}

	if ( work->count3 == 0 ) {
		int vox = 0 ;
		NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE  ) ;
		if ( work->status & (HSTG_STATUS_NO_EIMS) ) {
			switch( work->quest_time ) {
				case 0:
					vox = VOX_MAN_NO_EIMS_1 ;
				break ;
				case 1:
					vox = VOX_MAN_NO_1 ;
				break ;
				default :
					vox = VOX_MAN_SAY_NO_1 ;
				break ;
			}
		} else if ( work->status & (HSTG_STATUS_JENIFA) ) {
			vox = VOX_WOM_JENEFA ;
		} else if ( work->status & (HSTG_STATUS_RINDA) ) {
			switch( work->quest_time ) {
				case 0:
					vox = VOX_WOM_RINDA ;
				break ;
				default :
					vox = VOX_WOM_YORUNA_2 ;
				break ;
			}
		} else if ( work->status & (HSTG_STATUS_CYNDI) ) {
			switch( work->quest_time ) {
				case 0:
					vox = VOX_WOM_CINDY ;
				break ;
				default :
					vox = VOX_WOM_SAWARUNA_2 ;
				break ;
			}
		} else if ( work->status & (HSTG_STATUS_DARE) ) {
			switch( work->quest_time ) {
				case 0:
					vox = VOX_WOM_JENEFA ;
				break ;
				case 1:
					vox = VOX_WOM_RINDA ;
				break ;
				case 2:
					vox = VOX_WOM_CINDY ;
				break ;
				default :
					vox = VOX_WOM_JENEFA + KR_RandU( 3 ) ;
				break ;
			}
		} else if ( work->status & (HSTG_STATUS_NOEIMS_OL) ) {
			switch( work->quest_time ) {
				case 0:
					vox = VOX_WOM_JENEFA ;
				break ;
				case 1:
					vox = VOX_WOM_YORUNA_2 ;
				break ;
				case 2:
					vox = VOX_WOM_SAWARUNA_2 ;
				break ;
				default :
					vox = (work->quest_time%2)?VOX_WOM_YORUNA_2:VOX_WOM_SAWARUNA_2 ;
				break ;
			}
		} else {
			switch( work->quest_time ) {
				case 0:
					vox = VOX_MAN_NO_EIMS_2 ;
				break ;
				case 1:
					vox = VOX_MAN_NO_2 ;
				break ;
				default :
					vox = VOX_MAN_SAY_NO_2 ;
				break ;
			}
		}
		CallVox( work, vox ) ;
	}

	status = GM_StreamStatus ( work->strmhandler ) ;

	/* これは時間で戻る */
	if ( work->count3 > COUNT_VMODE( 200 ) ) {
//	if ( work->pl_dis > 2000 ) {
		work->quest_time ++ ;
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;
	if ( status == GM_STREAM_STATE_PLAY || status == GM_STREAM_STATE_READ_END ) {
		SET_FLAG( npc->nadj->adj_status, NPC_ADJ_KUBIFURI ) ;
	}

	work->count3 ++ ;
}

static void Think3_Pee( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		extern void *NewBloodBioDead( FVECTOR *pos, int life, float size, int , int map ) ;
		GV_SetActorChild( work, 
			NewBloodBioDead( &work->control.mov, 1000, 500.0, 1, work->control.map ) ) ;
		GM_SeSetMode( SD_A_OMORASI1, &npc->ctrl->mov, GM_SEMODE_NORMAL ) ;
	}

//	if ( work->count3 > COUNT_VMODE( 180 ) ) {
	if ( work->pl_dis > 2000 ) {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	npc->action.pad = PAD_TERROR ;

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}

static void Think3_Soppo( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

//	if ( work->count3 > COUNT_VMODE( 180 ) ) {
	if ( work->pl_dis > 2000 ) {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	AimReversePos( work, &GM_PlayerPosition ) ;

	work->count3 ++ ;
}

static void Think3_DamageWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( npc->action.status & NPC_ACT_STATUS_FAINT ) {
		work->think3 = TH3_SLEEP ;
		work->count3 = 0 ;
		return ;
	}

	if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {
		work->think3 = TH3_TERROR ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think3_SleepWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think3_Terror( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 > COUNT_VMODE( 300 ) ) {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_SIT ;
		work->think3 = TH3_IDLE ;
		work->count3 = 0 ;
		return ;
	}

	npc->action.pad = PAD_TERROR ;

	SET_FLAG( npc->nadj->adj_status, NPC_ADJ_ON ) ;
	npc->nadj->aim_pos = GM_PlayerPosition ;

	work->count3 ++ ;
}
/*----- Think2 --------------------------------------------------------*/
static void Think2_Sit( Work *work )
{
	switch( work->think3 ) {
		case  TH3_IDLE :
			Think3_SitIdle( work ) ;
		break ;
		case  TH3_STRAIN :
			Think3_Strain( work ) ;
		break ;
	}
}

static void Think2_Attack( Work *work )
{
	switch( work->think3 ) {
		case  TH3_STRAIGHT_FOOT :
			Think3_StraightFoot( work ) ;
		break ;
		case  TH3_FOOT_END :
			Think3_StraightFootEnd( work ) ;
		break ;
	}
}

static void Think2_MM( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_NoiseSurprise( work ) ;
		break ;
		case  TH3_JITABATA :
			Think3_Jitabata( work ) ;
		break ;
		case  TH3_SILENT :
			Think3_Silent( work ) ;
		break ;
		case  TH3_NO_EIMS :
			Think3_NoEims( work ) ;
		break ;
		case  TH3_PEE :
			Think3_Pee( work ) ;
		break ;
		case  TH3_SOPPO :
			Think3_Soppo( work ) ;
		break ;
		case  TH3_MAIL :
			Think3_Mail( work ) ;
		break ;
	}
}

static void Think2_HstgDamage( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_DamageWait( work ) ;
		break ;
		case  TH3_SLEEP :
			Think3_SleepWait( work ) ;
		break ;
		case  TH3_TERROR :
			Think3_Terror( work ) ;
		break ;
	}
}
/*----- Think1 --------------------------------------------------------*/
static int DamageCheck( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( npc->action.status & NPC_ACT_STATUS_DAMAGE ) {
		GM_StreamStop( work->strmhandler ) ;
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_HSTG_DAMAGE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}
	return 0 ;
}

static void NoticeCheck( Work *work )
{
	if ( work->notice & HSTG_NOTICE_NOISE ) {
		HSTG_ProcCallReaction( work->control.name ) ;
		work->think1 = TH1_NOISE ;
		work->think2 = TH2_MM ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return ;
	}
}

static void Think1_Normal( Work *work )
{
	if ( DamageCheck( work ) ) return ;

	switch( work->think2 ) {
		case  TH2_SIT :
			Think2_Sit( work ) ;
		break ;
		case  TH2_ATTACK :
			Think2_Attack( work ) ;
		break ;
	}

	NoticeCheck( work ) ;
}

static void Think1_Noise( Work *work )
{
	switch( work->think2 ) {
		case  TH2_MM :
			Think2_MM( work ) ;
		break ;
	}
}

static void Think1_Damage( Work *work )
{
	switch( work->think2 ) {
		case  TH2_HSTG_DAMAGE :
			Think2_HstgDamage( work ) ;
		break ;
	}
}

/*----- 思考処理メイン --------------------------------------------------------*/
static void Think( Work *work )
{
	switch( work->think1 ) {
		case  TH1_NORMAL :
			Think1_Normal( work ) ;
		break ;

		case  TH1_NOISE :
			Think1_Noise( work ) ;
		break ;

		case  TH1_DAMAGE :
			Think1_Damage( work ) ;
//printf("work->id=%d\n",work->id ) ;
		break ;
	}

#ifdef SMP_DEBUG_THINK

if ( work->id == 200 ) {
	DEBUG_Locate( 40, 30, 0 );
	DEBUG_Printf( "TH1=%2d\n", work->think1 );
	DEBUG_Printf( "TH2=%2d\n", work->think2 );
	DEBUG_Printf( "TH3=%2d\n", work->think3 );
	DEBUG_Printf( "CT3=%2d\n", work->count3 );
	DEBUG_Printf( "FAINT%2d\n", work->npc.action.faint );
	DEBUG_Printf( "FCOUNT%2d\n", work->npc.action.faint_count );
	DEBUG_Printf( "LIFE%2d\n", work->npc.action.life );
}
#endif
}
/*----- --------------------------------------------------------*/
static void StartThink( Work *work )
{
	NPC_SetCheckPad( &work->npc, HstgCheckPad ) ;
	NPC_SetCheckDamage( &work->npc, HstgCheckDamage ) ;

	work->think1 = TH1_NORMAL ;
	work->think2 = TH2_SIT ;
	work->think3 = TH3_IDLE ;
	work->count3 = 0 ;
}
