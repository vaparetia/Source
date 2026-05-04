/*
	speak.c
	敵兵台詞

	2000/04/24 Y.Korekado
	$Id: speak.c,v 1.1.1.3 2002/11/19 11:44:10 Yoshizawa1 Exp $
	
*/

#define VOX_ONLY_ONE	1		// VOXを必ず1つだけ使用する

#ifdef VOX_ONLY_ONE
static int StrmNextCode = -1;		// 次に呼び出すSDコード
static int StrmNextVox = -1;		// 次に呼び出すVOXコード
#endif

#define	BLANKTIME	COUNT_VMODE(30)

static	int			BlankTime ;
static	int			SetEneVoice ;
static	ENETHINK	*SetEnethink ;
static	ENETHINK	*StrmUseEnethink ;		/* ストリーミング使用者 */
static	int			StrmCurrentCode ;		/* ストリーミングコード */
static	int			StrmCurrentHandler ;	/* ストリーミングハンドラ */
static	int			StrmPlayNum ;			/* ストリーミング再生番号 */
static	int			StrmPlayLv ;			/* ストリーミング再生番号 */
static	int			StrmPlayCount ;			/* ストリーミング再生カウント */
//static	int			StrmStatus ;			/* ストリーミング再生ステータス */

static	int 		StrmPlayBuff[4] ;
static	int 		StrmCurrentPlay ;
static	int 		StrmCurrentCaution ;

#if 1
	#define MAX_GHQAREA		(16)
	#define MAX_GHQAREA_VOICE	(4)
	#define MAX_CAUTION_VOICE	(16)
#else
	#define MAX_GHQAREA		(4)
	#define MAX_GHQAREA_VOICE	(4)
	#define MAX_CAUTION_VOICE	(8)
#endif

typedef	struct	{
	int	num ;
	int	strm_code[ MAX_GHQAREA_VOICE ] ;
}GHQ_AREA_VOICE ;

GHQ_AREA_VOICE	GHQAreaVoice[ MAX_GHQAREA ] ;
int		GHQCautionVoice[ MAX_CAUTION_VOICE ] ;
int		GHQConfirmVoice ;
int		GHQMecaConfirmVoice ;

int		StrmCode[ MAX_VOICE_NUM ] ;
int		StrmTimeTable[ MAX_VOICE_NUM ] ;

/* ルートに貼る音声 */
#define MAX_ROUTE_VOICE	(16)
typedef	struct	{
	short	route ;
	short	point ;
	int		strm_code ;
	int		strm_time ;
} ROUTE_VOICE ;
ROUTE_VOICE	RouteVoice[ MAX_ROUTE_VOICE ] ;

typedef	struct	{
	char	area ;
	char	dummy[3] ;
	short	route ;
	short	point ;
	int		strm_code ;
	int		strm_time ;
} CLEARING_VOICE ;
CLEARING_VOICE	ClearingVoice[ MAX_ROUTE_VOICE ] ;

#define PLAREA0	(3813612)	//GV_StrCode( "PLAREA0" )
#define PLAREA1	(3813613)	//GV_StrCode( "PLAREA1" )
#define PLAREA2	(3813614)	//GV_StrCode( "PLAREA2" )
#define PLAREA3	(3813615)	//GV_StrCode( "PLAREA3" )
#define PLAREA4	(3813616)	//GV_StrCode( "PLAREA4" )
#define PLAREA5	(3813617)	//GV_StrCode( "PLAREA5" )
#define PLAREA6	(3813618)	//GV_StrCode( "PLAREA6" )
#define PLAREA7	(3813619)	//GV_StrCode( "PLAREA7" )
#define PLAREA8	(3813620)	//GV_StrCode( "PLAREA8" )
#define PLAREA9	(3813621)	//GV_StrCode( "PLAREA9" )
#define PLAREA10	(4595159)	//GV_StrCode( "PLAREA10" )
#define PLAREA11	(4595160)	//GV_StrCode( "PLAREA11" )
#define PLAREA12	(4595161)	//GV_StrCode( "PLAREA12" )
#define PLAREA13	(4595162)	//GV_StrCode( "PLAREA13" )
#define PLAREA14	(4595163)	//GV_StrCode( "PLAREA14" )
#define PLAREA15	(4595164)	//GV_StrCode( "PLAREA15" )
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static int GetStrmTime( int no  )
{
	return StrmTimeTable[ no ] ;
}
/*------------------------------------------------------------*/
void	COM_SetSpeak( v, entk )
int			v ;
ENETHINK	*entk ;
{
	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return ; 

	if ( BlankTime <= 0 ) {
		SetEneVoice = v ;
		SetEnethink = entk ;
		BlankTime = BLANKTIME ;
	}
}
/*---危険モード-------------------------------------------------------*/
static	int	GetPlayerGHQArea( void )
{
printf(" select GetPlayerGHQArea !!\n");

//printf("PLAREA0 num[%d]  !!\n",GHQAreaVoice[0].num);
	if ( GHQAreaVoice[0].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA0 ) ) return 0 ;
//printf("PLAREA1 num[%d]  !!\n",GHQAreaVoice[0].num);
	if ( GHQAreaVoice[1].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA1 ) ) return 1 ;
//printf("PLAREA2 num[%d]  !!\n",GHQAreaVoice[0].num);
	if ( GHQAreaVoice[2].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA2 ) ) return 2 ;
//printf("PLAREA3 num[%d]  !!\n",GHQAreaVoice[0].num);
	if ( GHQAreaVoice[3].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA3 ) ) return 3 ;

	if ( GHQAreaVoice[4].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA4 ) ) return 4 ;
	if ( GHQAreaVoice[5].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA5 ) ) return 5 ;
	if ( GHQAreaVoice[6].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA6 ) ) return 6 ;
	if ( GHQAreaVoice[7].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA7 ) ) return 7 ;

	if ( GHQAreaVoice[8].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA8 ) ) return 8 ;
	if ( GHQAreaVoice[9].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA9 ) ) return 9 ;
	if ( GHQAreaVoice[10].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA10) ) return 10 ;
	if ( GHQAreaVoice[11].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA11) ) return 11 ;

	if ( GHQAreaVoice[12].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA12 ) ) return 8 ;
	if ( GHQAreaVoice[13].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA13 ) ) return 9 ;
	if ( GHQAreaVoice[14].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA14) ) return 10 ;
	if ( GHQAreaVoice[15].num && HZX_CheckInsideAllTrap( &GM_PlayerPosition,PLAREA15) ) return 11 ;

	return -1 ;
}

static	int	GetGHQCode( int no )
{
	int	area, num ;

	area = no/MAX_GHQAREA_VOICE ;
	num = no%MAX_GHQAREA_VOICE ;

	return GHQAreaVoice[area].strm_code[num] ;
}

static	int	GetGHQAreaVoice( void )
{
	int area, r, ev_rad ;

	area = GetPlayerGHQArea( ) ;

printf(" GHQ Area Voice [%d]\n",area ) ;

	if ( area < 0 ) return EV_RAD_GHQ_ORDER1 ;

	r = KR_RandU( GHQAreaVoice[area].num ) ;

printf(" r [%d] num[%x]\n",r,GHQAreaVoice[area].num ) ;

	ev_rad = (area*MAX_GHQAREA_VOICE) + r ;
	ev_rad |= (EV_RAD_LEVEL_1|STRM_GHQ_ALERT) ;

printf(" GHQ Area num [%d] code[%x]\n",r,ev_rad ) ;
	return ev_rad ;
}

/*---ルートヴォイス-------------------------------------------------------*/
static	int	GetRouteVoiceCode( int no )
{
	return RouteVoice[ no ].strm_code ;
}
static	int	GetRouteVoiceTime( int no )
{
	return RouteVoice[ no ].strm_time ;
}

int COM_CallRouteVoice( int route, int point, int v_flag, ENETHINK *entk )
{
	int i, time ;
	ROUTE_VOICE	*route_voice ;

printf( " Call Route Voice Call route[%d] point[%d] \n",route, point ) ;

	i = 0 ;
	route_voice = &RouteVoice[0] ;
	while( route_voice->strm_code > 0 ) {
		if( route_voice->route == route ) {
			if( route_voice->point == point ) {
				time =  COM_SetRadio( v_flag|i, entk ) ;
				if ( time < 0 ) time = LV1_STRM_LENGTH ;
printf("COM_CallRouteVoice time[%d]\n",time ) ;
				return time ;
			}
		}
		route_voice ++ ;
		i++ ;
	}

	return -1 ;
}

static	int	GetClearingVoiceCode( int no )
{
	return ClearingVoice[ no ].strm_code ;
}
static	int	GetClearingVoiceTime( int no )
{
	return ClearingVoice[ no ].strm_time ;
}
int COM_CallClearingVoice( int area, int route, int point, int v_flag, ENETHINK *entk )
{
	int i ;
	CLEARING_VOICE	*crealing_voice ;

//printf( " Call Clearing Voice Call area [%d] route[%d] point[%d] \n",area, route, point ) ;

	i = 0 ;
	crealing_voice = &ClearingVoice[0] ;
	while( crealing_voice->strm_code > 0 ) {
		if( crealing_voice->area == area ) {
			if( crealing_voice->route == route ) {
				if( crealing_voice->point == point ) {
printf( " Call Clearing Voice Call area [%d] route[%d] point[%d] \n",area, route, point ) ;
					return COM_SetRadio( v_flag|i, entk ) ;
				}
			}
		}
		crealing_voice ++ ;
		i++ ;
	}

	return -1 ;
}
/*---警戒モード-------------------------------------------------------*/

static	int	GetGHQCautionCode( int no )
{
	return GHQCautionVoice[ no ] ;
}

static	int	GetGHQCautionVoice( void )
{
	int ev_rad ;
	
	ev_rad = STRM_GHQ_CAUTION|StrmCurrentCaution ;
	return ev_rad ;
}

/*------------------------------------------------------------*/
static	void	SetStrmBuff( int v )
{
	switch( v ){
		case EV_RAD_FIND_PL_FIRST :
			StrmCurrentPlay = 1 ;
//			StrmPlayBuff[ 1 ] = EV_RAD_GHQ_REINFORCE1 ;
			StrmPlayBuff[ 0 ] = GetGHQAreaVoice( ) ;
		break ;
		case EV_RAD_FIND_PL :
		case EV_RAD_FIND_PL_AT :
			StrmCurrentPlay = 1 ;
//			StrmPlayBuff[ 1 ] = EV_RAD_GHQ_REINFORCE2 ;
			StrmPlayBuff[ 0 ] = GetGHQAreaVoice( ) ;
		break ;
		case EV_RAD_FIND_PL_UNEXP :
			StrmCurrentPlay = 1 ;
//			StrmPlayBuff[ 1 ] = EV_RAD_GHQ_REINFORCE3 ;
			StrmPlayBuff[ 0 ] = GetGHQAreaVoice( ) ;
		break ;

		case EV_RAD_NO_ACCIDENT :
//			StrmCurrentPlay = 1 ;
//			StrmPlayBuff[ 0 ] = EV_RAD_GHQ_REINFORCE2 ;
		break ;

		case EV_RAD_END_AVOID :
		case EV_RAD_DAMAGE :
		case EV_RAD_HOLDUP :
		case EV_RAD_DAMAGE_GUN :
		case EV_RAD_ENE_DAMAGE :
		case EV_RAD_FIND :
		case EV_RAD_FIND_RAD :
			StrmCurrentPlay = 1 ;
//			StrmPlayBuff[ 1 ] = EV_RAD_GHQ_ORDER2 ;
			StrmPlayBuff[ 0 ] = GetGHQCautionVoice( ) ;
//			StrmPlayBuff[ 0 ] = EV_RAD_ROGER ;
		break ;
		case EV_RAD_CORP :
		case EV_RAD_CORP_AT :
			StrmCurrentPlay = 2 ;
			/* ステージで復活できる人数以上 */
//02.07.06	if ( Comm.res_count < Comm.max_res_num ) {
			if ( (Comm.res_count < Comm.max_res_num) &&
				 (Comm.res_count_in_alert < Comm.max_res_in_alert) ) {
				StrmPlayBuff[ 0 ] = GetGHQCautionVoice( ) ;
			} else {
				StrmPlayBuff[ 0 ] = EV_RAD_GHQ_NO_BRINGIN ;
			}
			StrmPlayBuff[ 1 ] = EV_RAD_ONEMORE_PL ;
		break ;

		case EV_RAD_ATK_RETURN :
			StrmCurrentPlay = 1 ;
			StrmPlayBuff[ 0 ] = GetGHQCautionVoice( ) ;
		break ;

		case EV_SLEEP_RAD_MUDAASI :
			StrmCurrentPlay = 1 ;
			StrmPlayBuff[ 0 ] = EV_RAD_GHQ_SLEEPCALL ;
		break ;
		case EV_RAD_GHQ_CONFIRM :
			if ( !(GM_Configuration & GM_CONFIG_STORY_TANKER) ) {
				StrmCurrentPlay = 1 ;
				StrmPlayBuff[ 0 ] = EV_RAD_GHQ_INVETIGATED ;
			}
		break ;
		case EV_RAD_GHQ_CONFIRM2 :
		break ;
		default :
		break ;
	}
}

#define MAN_KIND	4
#define STRM_STOP	0xff00000a
/*
	無線機コール

	Lv1 探索モードへ
	Lv2 危険モードへ

*/

/* 音声の長さを返す。 -1ならストリーム使っちゃ駄目 */
int		COM_SetRadio( int v, ENETHINK *entk )
{
	int	id_type, ev_rad, n, lv, str_status ;


	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return -1 ; 

	if ( COM_StageKind() & ENE_STAGE_NO_STRM ) return -1 ; 

	if ( COM_StageKind() & ENE_STAGE_NO_COMMANDER ) {
		if ( v & STRM_COMMANDER ) {
			if ( !(v & STRM_NO_COMM_USE) ) return -1 ;
		}
	}

	if ( entk != NULL ) {
		if ( entk->act->bodyp.type & ENE_TYPES_NO_STRM ) return -1 ;
		/* アクシデントビットを立てていればクリア */
		COM_UnsetAccident( entk->uniq_id ) ;
	}
//v=EV_RAD_FIND_PL_FIRST ;
printf("enemy speak.c: input vol [0x%8x]\n",v);
	/* 無線レベル */
	lv = (v&0xf0000000) >> 28 ;

printf("StrmPlayCount[%d] StrmPlayLv[%d] Lv[%d]\n",StrmPlayCount, StrmPlayLv, lv);
	/* ストリーミング使用中 */
	if( StrmPlayCount > 0 ) {
		str_status = GM_StreamStatus( StrmCurrentHandler ) ;
		if ( (str_status == GM_STREAM_STATE_PLAY) || (str_status == GM_STREAM_STATE_READ_END) ) {
			if(  StrmPlayLv >= lv ) return -1 ;	/* 今無線しちゃだめ */
		} else {
			GM_StreamStop( StrmCurrentHandler ) ;
		}
	}

	StrmUseEnethink = entk ;
	StrmPlayLv = lv ;
	n = v & 0xfff ;

	if( v&STRM_NO_CHARA ) {
		n += OTHER_VOICE_HEAD ;
printf(" Stream Voice No Chara [%x]\n",n ) ;
	}

	if ( StrmUseEnethink != NULL 
		&& !(v&STRM_ROUTE) && !(v&STRM_CLEARING) ) {	/* ＩＤ毎に音声振り分け */
//		id_type = (entk->id%MAN_KIND) * VOICE_SET_NUM ;
		id_type = entk->voice_chara * VOICE_SET_NUM ;
		StrmPlayNum = n + id_type ;
printf("StrmPlayNum = %d \n",StrmPlayNum) ;
		/* 訛り対応 */
		if ( entk->act->bodyp.type & ENE_TYPE_HITECH ) {
			/* 訛り無し */
			StrmPlayNum += (CHARACTER_VOICE_NUM*4) ;
printf("  namari------ StrmPlayNum = %d \n",StrmPlayNum) ;
		}
	} else {							/* 司令官の声 */
		StrmPlayNum = n ;
	}
	/* 音声の長さセット */
	if ( v&STRM_TIME_TABLE ) {
		StrmPlayCount = GetStrmTime( StrmPlayNum ) ;
	} else if( v&STRM_ROUTE ) {
		StrmPlayCount = GetRouteVoiceTime( StrmPlayNum ) ;
	} else if( v&STRM_CLEARING ) {
		StrmPlayCount = GetClearingVoiceTime( StrmPlayNum ) ;
	} else {
		/* 音声の長さ分入れないとWリードしてしまう
			体験版後、要修正 */
		StrmPlayCount = (StrmPlayLv == 1 )?LV1_STRM_LENGTH:LV2_STRM_LENGTH ;
	}

//printf(" change vol [0x%8x]\n",sd );

	if( v&STRM_GHQ_ALERT ) {
printf("speak.c:  GetGHQCode [%d] area[%d] num[%d]",
		StrmPlayNum,StrmPlayNum/MAX_GHQAREA_VOICE, StrmPlayNum%MAX_GHQAREA_VOICE );
		ev_rad = GetGHQCode( StrmPlayNum ) ;
	} else if( v&STRM_GHQ_CAUTION ) {
printf("speak.c:  GetGHQCautionCode [%d] ",StrmPlayNum );
		ev_rad = GetGHQCautionCode( StrmPlayNum ) ;
		if ( ev_rad == 0 ) {
			printf("Errrrrrrrrrrrrrrr No GHQ Caution Voice !!! \n") ;
			ev_rad = StrmCode[ (EV_RAD_GHQ_ORDER2&0xfff) + OTHER_VOICE_HEAD ] ;
		}
	} else if( v&STRM_GHQ_CONFIRM ) {
printf("speak.c:  GHQConfirmVoice [%d] ",GHQConfirmVoice );
		ev_rad = GHQConfirmVoice ;
		if ( ev_rad == 0 ) {
			printf("Errrrrrrrrrrrrrrr No GHQ GHQConfirmVoice Voice !!! \n") ;
			ev_rad = StrmCode[ (EV_RAD_GHQ_QUESTION6&0xfff) + OTHER_VOICE_HEAD ] ;
		}
	} else if( v&STRM_GHQ_MECA_CONFIRM ) {
printf("speak.c:  GHQMecaConfirmVoice [%d] ",GHQMecaConfirmVoice );
		ev_rad = GHQMecaConfirmVoice ;
		if ( ev_rad == 0 ) {
			printf("No GHQ GHQMecaConfirmVoice Voice !!! \n") ;
			ev_rad = StrmCode[ (EV_RAD_GHQ_INVETIGATED&0xfff) + OTHER_VOICE_HEAD ] ;
		}
	} else if( v&STRM_ROUTE ) {
		ev_rad = GetRouteVoiceCode( StrmPlayNum ) ;
printf("speak.c:  GetRouteVoiceCode [%d] ",StrmPlayNum );
	} else if( v&STRM_CLEARING ) {
		ev_rad = GetClearingVoiceCode( StrmPlayNum ) ;
printf("speak.c:  GetClearingVoiceCode [%d] ",StrmPlayNum );
	} else {
printf("speak.c:  StrmPlayNum [%d] ",StrmPlayNum );
		ev_rad = StrmCode[ StrmPlayNum ] ;
	}
#ifdef VOX_ONLY_ONE
	StrmNextCode = v;
	StrmNextVox = ev_rad;
#else
	StrmCurrentCode = v ;

	/* 敵兵システムは必ず１つのチャンネルしか使用しない */
	str_status = GM_StreamStatus( StrmCurrentHandler ) ;
	if ( str_status == GM_STREAM_STATE_PLAY || str_status == GM_STREAM_STATE_READ_END ) {
		GM_StreamStop( StrmCurrentHandler ) ;
	}

//	StrmCurrentHandler = GM_VoxStream( ev_rad, 0 ) ;
	StrmCurrentHandler = GM_VoxStream( ev_rad, GM_STREAM_FLAG_3D ) ;
	if ( StrmCurrentCode & STRM_FADER_ON && StrmUseEnethink != NULL ) {
		int	vol, pan ;
		GM_SeGetVolPan( &StrmUseEnethink->ctrl->mov, GM_SEMODE_BOMB, &vol, &pan ) ;
		GM_VoxStreamSetParam( StrmCurrentHandler, &StrmUseEnethink->ctrl->mov, StrmUseEnethink->ctrl->addr, vol, pan ) ;
	}
#endif
printf("speak.c: call vol [0x%8x]\n",ev_rad );

	if ( !(v&STRM_ONSELF) ) {
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_CODEC01 ) ; /*開始ノイズ */
	}

	/* 続く台詞があればセット */
	SetStrmBuff( v ) ;

	return StrmPlayCount ;
}

/* 音声ストップ */
void	COM_StopRadio( ENETHINK *entk )
{
	if( ((StrmUseEnethink == entk)&&!(StrmCurrentCode&STRM_NO_CANCEL) ) 
		|| entk == NULL ) {

		if ( StrmCurrentCode & STRM_CANCEL_THROUGH ) {	/* キャンセルスルーなら */
			/* 音声だけ止めて後の処理は続行する */
			GM_StreamStop( StrmCurrentHandler ) ;
			StrmUseEnethink = NULL ;
			return ;
		}

		if ( entk != NULL ) {
			if ( !(StrmCurrentCode & STRM_ONSELF) ) {	/* 独り言じゃなかったら */
				if ( (entk->status & ENE_STATUS_TALK_SLEEP) ) {
					COM_SetAccident( entk->uniq_id, &entk->last_radio_pos, entk->last_radio_map,
					ACCIDENT_REPO_DELAY_TIME, ENE_ACCIDENT_REPORT ) ;
				} else { 
					COM_SetAccident( entk->uniq_id, &entk->ctrl->mov, entk->ctrl->hzx_id,
					ACCIDENT_DELAY_TIME, ENE_ACCIDENT_RADIO ) ;
				}
			}
		}

		GM_StreamStop( StrmCurrentHandler ) ;
		StrmNextVox = -1;
		StrmNextCode = -1;
		StrmUseEnethink = NULL ;
		StrmPlayNum = -1 ;
		StrmPlayLv = -1 ;
		StrmCurrentPlay = 0 ;

		return ;
	}

	if ( entk != NULL ) {
		if ( GM_AlertMode != ALERT_MODE_ALERT && GM_AlertMode != ALERT_MODE_AVOID ) {
			if ( (entk->status & ENE_STATUS_REPORT) ) {
#ifdef PAL
				if ( GM_GameLevel == GM_LEVEL_E_EXTREME ) {
					COM_SetAccident( entk->uniq_id, &entk->last_radio_pos, entk->last_radio_map,
					(ACCIDENT_REPO_DELAY_TIME/2), ENE_ACCIDENT_REPORT ) ;
				} else {
					COM_SetAccident( entk->uniq_id, &entk->last_radio_pos, entk->last_radio_map,
					ACCIDENT_REPO_DELAY_TIME, ENE_ACCIDENT_REPORT ) ;
				}
#else
				COM_SetAccident( entk->uniq_id, &entk->last_radio_pos, entk->last_radio_map,
				ACCIDENT_REPO_DELAY_TIME, ENE_ACCIDENT_REPORT ) ;
#endif
			}
		}
	}
}

/* 音声ストップしてもアクシデントにならない */
void	COM_StopRadioNoAccident( ENETHINK *entk )
{
	COM_StopRadio( entk ) ;
	COM_UnsetAccident( entk->uniq_id ) ;
}

int	COM_GetStrmCount( )
{
	return StrmPlayCount ;
}

int	COM_GetStrmLv( )
{
	return StrmPlayLv ;
}

void	COM_InitSpeak( )
{
	BlankTime = 0 ;
	SetEneVoice = 0 ;
	SetEnethink = NULL ;
	StrmUseEnethink = NULL ;
	StrmPlayNum = -1 ;
	StrmPlayLv = -1 ;
	StrmPlayCount = 0 ;
	StrmCurrentCode = 0 ;
#ifdef VOX_ONLY_ONE
	StrmNextCode = -1;
	StrmNextVox = -1;
	StrmCurrentHandler = 0;
#endif
}

/*------------------------------------------------------------*/
static	void	SpeakManage()
{
	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) {
//		if ( com->stage_kind & ENE_STAGE_GOVER_STOPSTR ) {
//			GM_StreamStop( StrmCurrentHandler ) ;/* 字幕消す */
//		}
	}

	if ( BlankTime == BLANKTIME ) {
		GM_SeSetMode( SetEneVoice+(SetEnethink->voice_chara), &SetEnethink->ctrl->mov, GM_SEMODE_BOMB ) ;
	}
	
	if ( --BlankTime < 0 ) BlankTime = 0 ;
	if ( StrmPlayCount > 0 ) {
		if ( --StrmPlayCount == 0 ) {
			if ( StrmCurrentCode & STRM_RAD_NOISE ) {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_CODEC01 ) ; /*終了ノイズ */
			}
			StrmUseEnethink = NULL ;
			StrmPlayNum = -1 ;
			StrmPlayLv = -1 ;
			if ( StrmCurrentPlay > 0 ) {
				StrmCurrentPlay-- ;
				COM_SetRadio( StrmPlayBuff[ StrmCurrentPlay ], NULL ) ;
			}
		} else {
			if ( StrmCurrentCode & STRM_FADER_ON && StrmUseEnethink != NULL ) {
				int	vol, pan ;
            float bp_angle;
				GM_SeGetVolPan( &StrmUseEnethink->ctrl->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
//				GM_VoxStreamSetPan( StrmCurrentHandler, vol, pan ) ;
				GM_VoxStreamSetParam( StrmCurrentHandler, &StrmUseEnethink->ctrl->mov,
									  StrmUseEnethink->ctrl->addr, vol, pan, bp_angle ) ;
//printf(" strm :vol=[%x] pan=[%x] \n",vol,pan ) ;
			}
		}
	}
#ifdef VOX_ONLY_ONE
	if( StrmNextVox >= 0 ){
		int status;
		status = GM_StreamStatus( StrmCurrentHandler );
		if( status == GM_STREAM_STATE_END ){
			StrmCurrentCode = StrmNextCode;
//			StrmCurrentHandler = GM_VoxStream( StrmNextVox, 0 ) ;
			StrmCurrentHandler = GM_VoxStream( StrmNextVox, GM_STREAM_FLAG_3D ) ;
			if ( StrmCurrentCode & STRM_FADER_ON && StrmUseEnethink != NULL ) {
				int	vol, pan ;
            float bp_angle;
				GM_SeGetVolPan( &StrmUseEnethink->ctrl->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
//				GM_VoxStreamSetPan( StrmCurrentHandler, vol, pan ) ;
				GM_VoxStreamSetParam( StrmCurrentHandler, &StrmUseEnethink->ctrl->mov,
									  StrmUseEnethink->ctrl->addr, vol, pan, bp_angle ) ;
			}
			StrmNextVox = -1;
			StrmNextCode = -1;
		} else {
			GM_StreamStop( StrmCurrentHandler );
		}
	}
#endif
}

/*------------------------------------------------------------*/
