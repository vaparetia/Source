/*
	hstgpre.c
	人質思考前処理

	2001/03/22 Y.Korekado
	$Id: hstgpre.c,v 1.1.1.3 2002/11/19 11:44:18 Yoshizawa1 Exp $
*/
enum {
	SMP_MESSAGE_MODE_CHANGE,
} ;

static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
	int n_msg, code ;
	CONTROL	*ctrl ;

	ctrl = &work->control ;

    n_msg = ctrl->n_msg ;
	msg = ctrl->msg ;

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case SMP_MESSAGE_MODE_CHANGE :
//				work->mode = msg->message[ 1 ] ;
//				if ( work->mode == SMP_MODE_BOMB ) {
//					work->bomb_zone = msg->message[ 2 ] ;
//				}
			break ;
			default :
			break ;
		}
		msg++ ;
	}
}

static	void	InfoCheck( Work *work )
{
	FVECTOR	vec ;
	NPCWORK	*npc ;

	npc = &work->npc ;

	_sceVu0SubVector(  &vec, &GM_PlayerPosition, &npc->ctrl->mov ) ;
	work->pl_dis = _FVecLen3( &vec ) ;	/* プレイヤーとの距離 */
	work->pl_dir = _FVecDir2( &vec ) ;	/* プレイヤーへの方向 */

}

#define NOISE_MM_SURPRISE	(2000)
static	void	Noise( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

#if 1
	if( !GM_CheckGameStatus( STATE_BIG_SNORE ) ) {
		if ( HSTG_MicQuestion( work ) ) {
			work->notice |= HSTG_NOTICE_NOISE ;
		}
	}
#else
	switch ( HSTG_NoiseCheck( ) ) {
		case NOISE_SS :
			dis = _FVecTrgDis( &npc->ctrl->mov, &GM_NoisePosition ) ;
			if ( dis < NOISE_MM_SURPRISE ) work->notice |= HSTG_NOTICE_NOISE ;
			break ;
	}
#endif
}

/*----------------------------------------------------------------*/
static void PreProcess( Work *work )
{
	work->notice = 0 ;

	CheckMessage( work ) ;

	InfoCheck( work ) ;		/* 周りの情報チェック */
	Noise( work ) ;
}

#define	CORP_AREA	(350.0f)
#define	CORP_AREA_H	(1250.0f)
static void PlayerOnCorp( FVECTOR *pos )
{
	float	f ;

	f = pos->vx - GM_PlayerPosition.vx ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return ;
	f = pos->vz - GM_PlayerPosition.vz ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return ;
	f = pos->vy - GM_PlayerPosition.vy ;
	if ( f < -CORP_AREA_H || f > 0.0f ) return ;

	/* 壁チェックすると反応なくなる */
//	if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, pos, &GM_PlayerPosition,
//			HZX_CHK_ALL, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return ;

	GM_PlayerStatus |= PLAYER_ON_CORPSE ;
}

static char mail_seed[] = {
	/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),/*,COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),-1,
	/*COUNT_VMODE*/(24),/*COUNT_VMODE*/(24),-1,
	/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),-1,
	/*COUNT_VMODE*/(12),-1,
	/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(24),/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),/*COUNT_VMODE*/(12),-2
} ;
static void AfterProcess( Work *work )
{
	NPCWORK	*npc ;
	int		hear_beat ;

	npc = &work->npc ;
	/* 顔の向き */
	npc->action.face_dir = npc->ctrl->rot.vy ;
	
	/* ライフゲージ更新 */

	/* 心臓の鼓動 */
	hear_beat = ( npc->action.status & NPC_ACT_STATUS_TERROR ) ? 6 : 1 ;

	if ( (++work->heart_count) >= (work->heart_int/hear_beat) ) {
		int	se ;

		work->heart_count = 0 ;

		if ( work->status & HSTG_STATUS_RIC ) {
			se = SD_A_PMHEART1 ;
		} else {
			se = work->id%4 ;
			se += SD_A_AHEART01 ;
		}
		
		if( !GM_CheckGameStatus( STATE_BIG_SNORE ) ) {
			if ( !(work->status & HSTG_STATUS_NO_HEART) ) {
				GM_SeSetMode( se, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
			}
		}
	}
	if ( work->status & HSTG_STATUS_RIC ) {
//		if ( work->heart_count == 2 ) {
		if ( !(GM_PlayTime%(COUNT_VMODE(12))) ) {
			/* ペースメーカー */
			if( !GM_CheckGameStatus( STATE_BIG_SNORE ) ) {
				GM_SeSetMode( SD_A_PMHIGH01, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
//printf("ric hear beat call!!!!\n");
			}
		}
	}
	if ( (work->status & (HSTG_STATUS_MAIL|HSTG_STATUS_MAIL_NOCALL)) && !(work->status & HSTG_STATUS_MAIL_STOP) ) {
		if ( work->mail_time <= 0 ) {
			if( !GM_CheckGameStatus( STATE_BIG_SNORE ) ) {
				if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {
					GM_SeSetMode( SD_A_MAILTYPE, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
				}
			}
			work->mail_time = COUNT_VMODE(mail_seed[ (int)(work->mail_seed++) ]) ;
			if ( work->mail_time == -1 ) {
				work->mail_time = 40 + 20*KR_RandU( 8 );
				work->mail_time = COUNT_VMODE(work->mail_time) ;
			} else if ( work->mail_time <= -2 ) {
				work->mail_time = 40 + 20*KR_RandU( 8 );
				work->mail_time = COUNT_VMODE(work->mail_time) ;
				work->mail_seed =  0;
			}
		}
		work->mail_time -- ;
	}

	/* 死亡 */
	if ( npc->action.status & NPC_ACT_STATUS_DEATH ) {
		SET_FLAG( work->status, HSTG_STATUS_DIE ) ;
	}
	if ( npc->action.status & NPC_ACT_STATUS_MUST_DIE ) {
		SET_FLAG( work->status, HSTG_STATUS_MUST_DIE ) ;
	}

	PlayerOnCorp( &work->on_corp ) ;

	/* ストリーミング制御 */
	{
		int stream_status ;
		stream_status = GM_StreamStatus ( work->strmhandler ) ;
		if ( stream_status == GM_STREAM_STATE_PLAY || stream_status == GM_STREAM_STATE_READ_END ) {
			int	vol, pan ;
         float bp_angle;
			if ( work->strm == VOX_MEC_SENDMAIL ) {
				GM_SeGetVolPan( &npc->ctrl->mov, GM_SEMODE_MIC, &vol, &pan, &bp_angle ) ;
				vol /= 2 ;
				if ( vol >= 0x18 ) vol = 0x18 ;
			} else {
				GM_SeGetVolPan( &npc->ctrl->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
			}
//			GM_VoxStreamSetPan( work->strmhandler, vol, pan ) ;
			GM_VoxStreamSetParam( work->strmhandler, &npc->ctrl->mov,
								  npc->ctrl->addr, vol, pan, bp_angle ) ;
		}
	}

	if ( --work->se_interval < 0 )  work->se_interval = 0 ;
}
