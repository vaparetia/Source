//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bgmanage.c
	ＢＧＭマネージャー

	2000/10/06 Y.Korekado
	$Id: bgmanage.c,v 1.1.1.3 2002/11/19 11:44:01 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
void	*NewBGMManager( name, where )

シナリオオプション
----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"define.h"

//#define BGM_CUT (1)
//#define JOUCHU_CUT (1)
//#define SIREN_CUT (1)
#define JIK_BGM_CONTINUE (1)

#ifdef JOUCHU_CUT
static int JOUCHU_CUT_TIME ;
#endif
/*----------------------------------------------------------------*/
#define SDMAN_BGM_PLAY_SNEAK	SNG_PLAY_05	/* 潜入曲 */
#define SDMAN_BGM_PLAY_DETECT	SNG_PLAY_01	/* 発見～回避曲 */
#define SDMAN_BGM_PLAY_ALERT	SNG_PLAY_02	/* 危険～回避曲 */
#define SDMAN_BGM_PLAY_AVOID	SNG_PLAY_03	/* 回避～危険曲 */
#define SDMAN_BGM_PLAY_SEARCH	SNG_PLAY_04	/* 警戒曲 */

#define FADE_0 	0xff000014	/* */

extern int	GM_GetSngCode( void );

enum {
	BGM_MODE_SNEAK,
	BGM_MODE_DETECT,
	BGM_MODE_ALERT,
	BGM_MODE_AVOID,
	BGM_MODE_SEARCH,
	BGM_MODE_VR_CLEAR
} ;
enum {
	BGM_MODE2_START,
	BGM_MODE2_START2,
	BGM_MODE2_IDLE,
	BGM_MODE2_END,
};

/* message */
enum {
	SD_SNEAK_AUTO,	/* 潜入曲のオートフェーダ */
	SD_SNEAK_NO_FADEOUT,	/* 潜入でもフェードアウトなし */
} ;
/*----------------------------------------------------------------*/
#define MAX_TRACK	(0x20)
#define MAX_LINE	(MAX_TRACK/4)


typedef	struct	{
	short	pan ;
	short	vol ;
} BGM_COMP ;

typedef	struct _bgmmanage_Work	{
	GV_ACT	actor ;

	int		name ;
	int		bgm_mode ;
	int		bgm_mode2 ;
	u_int	count2 ;
	u_int	sys_count ;
	int		current_bgm ;	/* 鳴っている曲 */
	int		current_at ;	/* 掛っているオートフェーダー */
	int		mess_at ;		/* メッセージオートフェーダー */

	int		status ;
	BGM_COMP	bgmcomp[MAX_TRACK] ;
} Work ;

/* status */
#define BGM_SNEAK_OTHER_CONTROL	0x00000001	/* 他のキャラクタが潜入モードの音を管理 */
#define BGM_SYUKAN_ON			0x00000002	/* 主観時BGM下げる */
#define BGM_PAUSE_ON			0x00000004	/* ポーズ時BGM下げる */
#define BGM_MIC_ON				0x00000008	/* マイク使用時BGM下げる */
#define BGM_SNEAK_NO_FADEOUT	0x00000010	/* 潜入モードでもフェードアウトなし */
#define BGM_END_FADEOUT			0x00000020	/* ステージ移動時フェードアウト */

static	void 	*fader_work[ MAX_TRACK ] ;
static	int	 	fader_area_num[ MAX_TRACK ] ;

/*----------------------------------------------------------------*/
static void BGM_GM_SdSet( int code )
{
	printf("BGM_GM_SdSet: code [%x]\n",code) ;
	GM_SdSet( code ) ;
}
/* BGMフェーダー関連 */
void BGM_SetFaderWork( int track, void *work )
{
	if( fader_work[ track ] == NULL ) {
		fader_work[ track ] = work ;
	}
	fader_area_num[ track ] ++ ;
}
void BGM_UnsetFaderWork( int track )
{
	fader_area_num[ track ] -- ;
	if( fader_area_num[ track ] == 0 ) {
		fader_work[ track ] = NULL ;
	}
}
void *BGM_GetFaderWork( int track )
{
	return fader_work[ track ] ;
}
int BGM_GetFaderAreaNum( int track )
{
	return fader_area_num[ track ] ;
}
/*----------------------------------------------------------------*/
/*
制御コード 0xFB000000 - 0xFB1F3F3F

0xFBiippvv ・・・ ii=トラック番号(0x00 - 0x1F)
           ・・・ pp=パン位置(左:0x00 - 0x1F:中央:0x20 - 右:0x3F)
           ・・・ vv=音量(0-0x3F)
*/

static	void	InitBgmComp( work )
Work		*work ;
{
	int i ;
	
	for( i=0; i<MAX_TRACK; i++ ) {
		work->bgmcomp[i].pan = 0x20 ;
		work->bgmcomp[i].vol = 0x3f ;
	}
}

static void BGM_System( work )
Work	*work ;
{
#ifdef BGMMANE_VOL_CTRL
	/* 指向性マイク */
	if ( ((GM_AlertMode == ALERT_MODE_SNEAK) || (GM_AlertMode == ALERT_MODE_SEARCH) )
			&& !(GM_GameStatus & GM_STATUS_DETECT) ) {
		if( PL_GetPlayerWeapon( ) == WP_Mic ) {
printf("kroe1\n");
			if ( !(work->status & BGM_MIC_ON) ) {
				BGM_GM_SdSet( SNG_MIC_ON );
//				BGM_GM_SdSet( SNG_FOUTP_SS );
				work->status |= BGM_MIC_ON ;
			}
		} else {
			if ( work->status & BGM_MIC_ON ) {
				BGM_GM_SdSet( SNG_MIC_OFF );
//				BGM_GM_SdSet( SNG_FIN_S ) ;
				work->status &= ~BGM_MIC_ON ;
			}
		}
	} else {
		if ( work->status & BGM_MIC_ON ) {
			BGM_GM_SdSet( SNG_MIC_OFF );
//			BGM_GM_SdSet( SNG_FIN_S ) ;
			work->status &= ~BGM_MIC_ON ;
		}
	}

	if( GM_PlayerStatus & PLAYER_WATCH ) {
		if ( !(work->status & BGM_SYUKAN_ON) ) {
			BGM_GM_SdSet( SNG_SYUKAN_ON );
			work->status |= BGM_SYUKAN_ON ;
//printf(" BGM SHUKAN ON\n");
		}
	} else {
		if ( work->status & BGM_SYUKAN_ON ) {
			BGM_GM_SdSet( SNG_SYUKAN_OFF );
			work->status &= ~BGM_SYUKAN_ON ;
//printf(" BGM SHUKAN OFF\n");
		}
	}
#endif

#ifdef DEBUG_MODE
{
	static int	Xcur=0, Ycur=0, repeat=0 ;
	int i, press1, status1, now_t, now_p ;
	short	*p ;

	if ( GM_Debug2PMode == GM_DEBUG_MODE_BGM_COMPOSE ) {
		press1 = GV_PadData[ 1 ].press ;
		status1 = GV_PadData[ 1 ].status ;

		if( press1 & PAD_R ){
			if ( ++Xcur >= MAX_LINE ) Xcur = 0 ;
		}
		if( press1 & PAD_L ){
			if ( --Xcur < 0 ) Xcur = MAX_LINE - 1  ;
		}
		if( press1 & PAD_D ){
			if ( ++Ycur > 7 ) Ycur = 0 ;
		}
		if( press1 & PAD_U ){
			if ( --Ycur < 0 ) Ycur = 7 ;
		}
		
		now_t = Xcur + (MAX_LINE * (Ycur/2)) ;
		now_p = Ycur%2 ;
		p = &(work->bgmcomp[now_t].pan) ;
		p += now_p ;

		if( press1 & PAD_X ){
//GM_SeSet( work->bgmcomp[0].pan, work->bgmcomp[0].vol, GM_SEMODE_BOMB ) ;

			*p += 0x10 ;
			if( *p >= 0x3f ) *p = 0x3f ;
			GM_MixConvFader( now_t, work->bgmcomp[now_t].pan, work->bgmcomp[now_t].vol ) ;
		} else if( press1 & PAD_Y ){
			*p -= 0x10 ;
			if( *p < 0 ) *p = 0 ;
			GM_MixConvFader( now_t, work->bgmcomp[now_t].pan, work->bgmcomp[now_t].vol ) ;
		}

		if( status1 & PAD_A ){
			repeat ++ ;
			if ( repeat == 1 ||	( (repeat > 60) && !(repeat%2) ) ) {
				if( ++(*p) >= 0x3f ) *p = 0x3f ;
				GM_MixConvFader( now_t, work->bgmcomp[now_t].pan, work->bgmcomp[now_t].vol ) ;
			}
		} else if( status1 & PAD_B ){
			repeat ++ ;
			if ( repeat == 1 ||	( (repeat > 60) && !(repeat%2) ) ) {
				if( --(*p) < 0 ) *p = 0 ;
				GM_MixConvFader( now_t, work->bgmcomp[now_t].pan, work->bgmcomp[now_t].vol ) ;
			}
		} else {
			repeat = 0 ;
		}

		if( press1 & PAD_SEL ){
			InitBgmComp( work ) ;
			for( i=0; i<MAX_TRACK; i++ ) {
				work->bgmcomp[i].pan = 0x20 ;
				work->bgmcomp[i].vol = 0x3f ;
				GM_MixConvFader( i, work->bgmcomp[i].pan, work->bgmcomp[i].vol ) ;
			}
		}


		/* 表示 */
//		DEBUG_Color( 255, 255, 255, 128 ) ;
		MENU_Locate( 40 + ( 22 ), 15, 0 );
		DEBUG_Printf( "BGM COMPOSE" );
		for ( i=0; i<MAX_LINE; i++ ) {
			DEBUG_Locate( 40 + ( 48 * i), 25, 0 );
			DEBUG_Printf( "%xTRC", i );
			DEBUG_Locate( 40 + ( 48 * i), 35, 0 );
			DEBUG_Printf( "P=%2x", work->bgmcomp[i].pan );
			DEBUG_Locate( 40 + ( 48 * i), 45, 0 );
			DEBUG_Printf( "V=%2x", work->bgmcomp[i].vol );

			DEBUG_Locate( 40 + ( 48 * i), 65, 0 );
			DEBUG_Printf( "%dTRC", i+MAX_LINE );
			DEBUG_Locate( 40 + ( 48 * i), 75, 0 );
			DEBUG_Printf( "P=%2x", work->bgmcomp[i+MAX_LINE].pan );
			DEBUG_Locate( 40 + ( 48 * i), 85, 0 );
			DEBUG_Printf( "V=%2x", work->bgmcomp[i+MAX_LINE].vol );

			DEBUG_Locate( 40 + ( 48 * i), 105, 0 );
			DEBUG_Printf( "%dTRC", i+(MAX_LINE*2) );
			DEBUG_Locate( 40 + ( 48 * i), 115, 0 );
			DEBUG_Printf( "P=%2x", work->bgmcomp[i+(MAX_LINE*2)].pan );
			DEBUG_Locate( 40 + ( 48 * i), 125, 0 );
			DEBUG_Printf( "V=%2x", work->bgmcomp[i+(MAX_LINE*2)].vol );

			DEBUG_Locate( 40 + ( 48 * i), 145, 0 );
			DEBUG_Printf( "%dTRC", i+(MAX_LINE*3) );
			DEBUG_Locate( 40 + ( 48 * i), 155, 0 );
			DEBUG_Printf( "P=%2x", work->bgmcomp[i+(MAX_LINE*3)].pan );
			DEBUG_Locate( 40 + ( 48 * i), 165, 0 );
			DEBUG_Printf( "V=%2x", work->bgmcomp[i+(MAX_LINE*3)].vol );
		}
		/* カレントカーソル */
//		DEBUG_Color( 255, 64, 0, 128 ) ;
		if ( now_p == 0 ) {
			DEBUG_Locate( 40 + ( 48 * Xcur), 35+(Ycur*20), 0 );
			DEBUG_Printf( "P=%2x", work->bgmcomp[now_t]. pan );
		} else {
			DEBUG_Locate( 40 + ( 48 * Xcur), 45+((Ycur-1)*20), 0 );
			DEBUG_Printf( "V=%2x", work->bgmcomp[now_t].vol );
		}

	}
}
#endif
}

/*----------------------------------------------------------------*/
static	void	BgmChange( work, code )
Work	*work ;
int		code ;
{
#ifndef BGM_CUT
	BGM_GM_SdSet( code );
#endif
	work->current_bgm = code ;
	work->current_at = -1 ;
}

static	void	Sneak_Start( work )
Work		*work ;
{
#if 0
	if ( work->count2 == 6 ) {
		BGM_GM_SdSet( SNG_FIN_S ) ;
//		if ( work->status & BGM_SNEAK_OTHER_CONTROL ) BGM_GM_SdSet( FADE_0 ) ;
		BgmChange( work, SDMAN_BGM_PLAY_SNEAK ) ;
	}

	if ( work->count2 > 16 ) {
		BGM_GM_SdSet( SD_AUTO_FADER6 );
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;
		
		return ;
	}
#else
	if ( work->count2 > COUNT_VMODE(16) ) {
		BGM_GM_SdSet( SNG_FIN_S ) ;
//		if ( work->status & BGM_SNEAK_OTHER_CONTROL ) BGM_GM_SdSet( FADE_0 ) ;
		BgmChange( work, SDMAN_BGM_PLAY_SNEAK ) ;
		BGM_GM_SdSet( SD_AUTO_FADER1 );	//他のFADERとけんかしないか注意
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;
		
		return ;
	}
#endif
	
	work->count2++ ;
}

static	void	Sneak_Start2( work )
Work		*work ;
{
	if ( work->count2 > COUNT_VMODE(16) ) {
		BGM_GM_SdSet( SNG_FOUTP_SS );
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;
		
		return ;
	}
	
	work->count2++ ;
}

static	void	Sneak_Idle( work )
Work		*work ;
{
}

static	void	Detect_Start( work )
Work		*work ;
{

	if ( !(GM_Configuration & GM_CONFIG_END_IF_FOUND) ) {
		if ( work->count2 > COUNT_VMODE(4) ) {
			BGM_GM_SdSet( SNG_FIN_S ) ;
			BgmChange( work, SDMAN_BGM_PLAY_DETECT ) ;
			BGM_GM_SdSet( SD_AUTO_FADER4 ) ;
			work->bgm_mode2 = BGM_MODE2_IDLE ;
			work->count2 = 0 ;
			
			return ;
		}
	}

	work->count2++ ;
}
static	void	Detect_Idle( work )
Work		*work ;
{
}

static	void	Alert_Start( work )
Work		*work ;
{
	if ( work->count2 > COUNT_VMODE(4) ) {
		BGM_GM_SdSet( SNG_FIN_S ) ;
		BgmChange( work, SDMAN_BGM_PLAY_ALERT ) ;
		BGM_GM_SdSet( SD_AUTO_FADER2 ) ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;
		
		return ;
	}

	work->count2++ ;
}
static	void	Alert_Idle( work )
Work		*work ;
{
}

static	void	Avoid_Start( work )
Work		*work ;
{
	if ( work->count2 > COUNT_VMODE(4) ) {
		BGM_GM_SdSet( SNG_FIN_S ) ;
		BgmChange( work, SDMAN_BGM_PLAY_AVOID ) ;
		BGM_GM_SdSet( SD_AUTO_FADER3 ) ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;
		
		return ;
	}

	work->count2++ ;
}
static	void	Avoid_Idle( work )
Work		*work ;
{
}

static	void	Search_Start( work )
Work		*work ;
{
	if ( work->count2 > COUNT_VMODE(4) ) {
		BGM_GM_SdSet( SNG_FIN_S ) ;

		if ( GM_GameStatus & STATE_VR_ONLY ) {
			BgmChange( work, SDMAN_BGM_PLAY_SNEAK ) ;
		} else {
			BgmChange( work, SDMAN_BGM_PLAY_SEARCH ) ;
		}

		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;
//printf("korekorekorekorekore3\n");
		return ;
	}

	work->count2++ ;
}

static	void	Search_Start2( work )
Work		*work ;
{
	if ( work->count2 > COUNT_VMODE(16) ) {
		BGM_GM_SdSet( SNG_FOUTP_SS );
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;
		
		return ;
	}
	
	work->count2++ ;
}

static	void	Search_Idle( work )
Work		*work ;
{
}

static	void	VrClear_Idle( work )
Work		*work ;
{
}
/*----------------------------------------------------------------*/
static	void	BGM_ModeSneak( work )
Work		*work ;
{
	switch( work->bgm_mode2 ) {
		case BGM_MODE2_START :
			Sneak_Start( work ) ;
		break ;
		case BGM_MODE2_START2 :	/* 発見から潜入 */
			Sneak_Start2( work ) ;
		break ;
		case BGM_MODE2_IDLE :
			Sneak_Idle( work ) ;
		break ;
	}
}

static	void	BGM_ModeDetect( work )
Work		*work ;
{
	switch( work->bgm_mode2 ) {
		case BGM_MODE2_START :
			Detect_Start( work ) ;
		break ;
		case BGM_MODE2_IDLE :
			Detect_Idle( work ) ;
		break ;
	}
}

static	void	BGM_ModeAlert( work )
Work		*work ;
{
	switch( work->bgm_mode2 ) {
		case BGM_MODE2_START :
			Alert_Start( work ) ;
		break ;
		case BGM_MODE2_IDLE :
			Alert_Idle( work ) ;
		break ;
	}
}

static	void	BGM_ModeAvoid( work )
Work		*work ;
{
	switch( work->bgm_mode2 ) {
		case BGM_MODE2_START :
			Avoid_Start( work ) ;
		break ;
		case BGM_MODE2_IDLE :
			Avoid_Idle( work ) ;
		break ;
	}
}

static	void	BGM_ModeSearch( work )
Work		*work ;
{
	switch( work->bgm_mode2 ) {
		case BGM_MODE2_START :
			Search_Start( work ) ;
		break ;
		case BGM_MODE2_START2 :	/* 発見から警戒 */
			Search_Start2( work ) ;
		break ;
		case BGM_MODE2_IDLE :
			Search_Idle( work ) ;
		break ;
	}
}

static	void	BGM_ModeVrClear( work )
Work		*work ;
{
	switch( work->bgm_mode2 ) {
		case BGM_MODE2_IDLE :
			VrClear_Idle( work ) ;
		break ;
	}
}

/*----------------------------------------------------------------*/
static	void	ModeSneakCondition( work )
Work		*work ;
{
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		BGM_GM_SdSet( SNG_FOUTS_SS );
		work->bgm_mode = BGM_MODE_VR_CLEAR ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;

		return ;
	}

	if ( GM_GameStatus & GM_STATUS_DETECT ) {
//		if ( work->current_bgm == SDMAN_BGM_PLAY_DETECT ) {
		if ( GM_GetSngCode( ) == SDMAN_BGM_PLAY_DETECT ) {
			BGM_GM_SdSet( SD_AUTO_FADER4 );
		} else {
			BGM_GM_SdSet( SNG_FOUTP_SS );
		}

		work->bgm_mode = BGM_MODE_DETECT ;
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
#ifndef SIREN_CUT
		BGM_GM_SdSet( SD_S_SIREN06 ) ;	/* サイレン */
#endif
		work->bgm_mode = BGM_MODE_ALERT ;
//		if ( work->current_bgm != SDMAN_BGM_PLAY_ALERT && 
//			work->current_bgm != SDMAN_BGM_PLAY_AVOID ) {
		if ( GM_GetSngCode() != SDMAN_BGM_PLAY_ALERT && 
			GM_GetSngCode() != SDMAN_BGM_PLAY_AVOID ) {
			BGM_GM_SdSet( SNG_FOUTP_SS );
			work->bgm_mode2 = BGM_MODE2_START ;
		} else {
			BGM_GM_SdSet( SNG_FIN_S );
			work->bgm_mode2 = BGM_MODE2_IDLE ;
		}
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		work->bgm_mode = BGM_MODE_AVOID ;
//		if ( work->current_bgm != SDMAN_BGM_PLAY_ALERT && 
//			work->current_bgm != SDMAN_BGM_PLAY_AVOID ) {
		if ( GM_GetSngCode() != SDMAN_BGM_PLAY_ALERT && 
			GM_GetSngCode() != SDMAN_BGM_PLAY_AVOID ) {
			BGM_GM_SdSet( SNG_FOUTP_SS );
			work->bgm_mode2 = BGM_MODE2_START ;
		} else {
			BGM_GM_SdSet( SNG_FIN_S );
			work->bgm_mode2 = BGM_MODE2_IDLE ;
		}
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
		work->bgm_mode = BGM_MODE_SEARCH ;
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;
//printf("korekorekorekorekore2\n");

		return ;
	}
}

static	void	ModeDetectCondition( work )
Work		*work ;
{
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		BGM_GM_SdSet( SNG_FOUTS_SS );
		work->bgm_mode = BGM_MODE_VR_CLEAR ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;

		return ;
	}

	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
#ifndef SIREN_CUT
		BGM_GM_SdSet( SD_S_SIREN06 ) ;	/* サイレン */
#endif
		work->bgm_mode = BGM_MODE_ALERT ;
//		if ( work->current_bgm == SDMAN_BGM_PLAY_DETECT ) {
		if ( GM_GetSngCode( ) == SDMAN_BGM_PLAY_DETECT ) {
			BGM_GM_SdSet( SD_INTRO_SKIP ) ;
			BGM_GM_SdSet( SD_AUTO_FADER2 ) ;
			work->bgm_mode2 = BGM_MODE2_IDLE ;
		} else {
			work->bgm_mode2 = BGM_MODE2_START ;
		}
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		BGM_GM_SdSet( SD_INTRO_SKIP ) ;
		BGM_GM_SdSet( SD_AUTO_FADER3 ) ;
		work->bgm_mode = BGM_MODE_AVOID ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;

		return ;
	}
	if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
//		BGM_GM_SdSet( SD_AUTO_FADER5 );
		BGM_GM_SdSet( 0xFF000104 );
		BGM_GM_SdSet( SD_PLAY_FX );

		if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
			work->bgm_mode = BGM_MODE_SNEAK ;
		} else {
			work->bgm_mode = BGM_MODE_SEARCH ;
		}
		work->bgm_mode2 = BGM_MODE2_START2 ;
		work->count2 = 0 ;

		return ;
	}
}

static	void	ModeAlertCondition( work )
Work		*work ;
{
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		BGM_GM_SdSet( SNG_FOUTS_SS );
		work->bgm_mode = BGM_MODE_VR_CLEAR ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;

		return ;
	}

	if ( GM_AlertMode != ALERT_MODE_ALERT ) {
		if ( GM_GameStatus & GM_STATUS_DETECT ) {
			BGM_GM_SdSet( SD_AUTO_FADER4 ) ;
			work->bgm_mode = BGM_MODE_DETECT ;
			work->bgm_mode2 = BGM_MODE2_IDLE ;
			work->count2 = 0 ;

			return ;
		}
		if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
			BGM_GM_SdSet( SNG_FOUTS_SS );
			work->bgm_mode = BGM_MODE_SNEAK ;
			work->bgm_mode2 = BGM_MODE2_START ;
			work->count2 = 0 ;

			return ;
		}
		if ( GM_AlertMode == ALERT_MODE_AVOID ) {
			BGM_GM_SdSet( SD_AUTO_FADER3 ) ;
			work->bgm_mode = BGM_MODE_AVOID ;
			work->bgm_mode2 = BGM_MODE2_IDLE ;
			work->count2 = 0 ;

			return ;
		}
		if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
			BGM_GM_SdSet( SNG_FOUTP_M );
			work->bgm_mode = BGM_MODE_SEARCH ;
			work->bgm_mode2 = BGM_MODE2_START ;
			work->count2 = 0 ;

			return ;
		}
	}
}

static	void	ModeAvoidCondition( work )
Work		*work ;
{
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		BGM_GM_SdSet( SNG_FOUTS_SS );
		work->bgm_mode = BGM_MODE_VR_CLEAR ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;

		return ;
	}

	if ( GM_AlertMode != ALERT_MODE_AVOID ) {
		if ( GM_GameStatus & GM_STATUS_DETECT ) {
			BGM_GM_SdSet( SD_AUTO_FADER4 ) ;
			work->bgm_mode = BGM_MODE_DETECT ;
			work->bgm_mode2 = BGM_MODE2_IDLE ;
			work->count2 = 0 ;

			return ;
		}
		if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
			BGM_GM_SdSet( SNG_FOUTP_M );
			work->bgm_mode = BGM_MODE_SNEAK ;
			work->bgm_mode2 = BGM_MODE2_START ;
			work->count2 = 0 ;

			return ;
		}
		if ( GM_AlertMode == ALERT_MODE_ALERT ) {
#ifndef SIREN_CUT
			BGM_GM_SdSet( SD_S_SIREN06 ) ;	/* サイレン */
#endif
			BGM_GM_SdSet( SD_AUTO_FADER2 ) ;
			work->bgm_mode = BGM_MODE_ALERT ;
			work->bgm_mode2 = BGM_MODE2_IDLE ;
			work->count2 = 0 ;

			return ;
		}
		if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
			BGM_GM_SdSet( SNG_FOUTP_M );
			work->bgm_mode = BGM_MODE_SEARCH ;
			work->bgm_mode2 = BGM_MODE2_START ;
			work->count2 = 0 ;

			return ;
		}
	}
}

static	void	ModeSearchCondition( work )
Work		*work ;
{
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		BGM_GM_SdSet( SNG_FOUTS_SS );
		work->bgm_mode = BGM_MODE_VR_CLEAR ;
		work->bgm_mode2 = BGM_MODE2_IDLE ;
		work->count2 = 0 ;

		return ;
	}

	if ( GM_GameStatus & GM_STATUS_DETECT ) {
//		if ( work->current_bgm == SDMAN_BGM_PLAY_DETECT ) {
		if ( GM_GetSngCode( ) == SDMAN_BGM_PLAY_DETECT ) {
			BGM_GM_SdSet( SD_AUTO_FADER4 );
		} else {
			BGM_GM_SdSet( SNG_FOUTP_SS );
		}
		work->bgm_mode = BGM_MODE_DETECT ;
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
		work->bgm_mode = BGM_MODE_ALERT ;
//		if ( work->current_bgm != SDMAN_BGM_PLAY_ALERT && 
//			work->current_bgm != SDMAN_BGM_PLAY_AVOID ) {
		if ( (GM_GetSngCode( ) != SDMAN_BGM_PLAY_ALERT) && 
			(GM_GetSngCode( ) != SDMAN_BGM_PLAY_AVOID) ) {
			BGM_GM_SdSet( SNG_FOUTP_SS );
			work->bgm_mode2 = BGM_MODE2_START ;
		} else {
			BGM_GM_SdSet( SNG_FIN_S );
			work->bgm_mode2 = BGM_MODE2_IDLE ;
		}
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		work->bgm_mode = BGM_MODE_AVOID ;
//		if ( work->current_bgm != SDMAN_BGM_PLAY_ALERT && 
//			work->current_bgm != SDMAN_BGM_PLAY_AVOID ) {
		if ( (GM_GetSngCode( ) != SDMAN_BGM_PLAY_ALERT) && 
			(GM_GetSngCode( ) != SDMAN_BGM_PLAY_AVOID) ) {
			BGM_GM_SdSet( SNG_FOUTP_SS );
			work->bgm_mode2 = BGM_MODE2_START ;
		} else {
			BGM_GM_SdSet( SNG_FIN_S );
			work->bgm_mode2 = BGM_MODE2_IDLE ;
		}
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;

		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
//		BGM_GM_SdSet( SD_AUTO_FADER1 );
		work->bgm_mode = BGM_MODE_SNEAK ;
		work->bgm_mode2 = BGM_MODE2_START ;
		work->count2 = 0 ;

		return ;
	}
}

/*----------------------------------------------------------------*/
static	void	BGM_Manager( work )
Work		*work ;
{
	switch( work->bgm_mode ) {
		case BGM_MODE_SNEAK :
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbgmmanegaer:SNEAK!!\n");
			BGM_ModeSneak( work ) ;
			ModeSneakCondition( work ) ;
			break ;
		case BGM_MODE_DETECT :
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbgmmanegaer:DETECT!![%x]\n",GM_GetSngCode( ));
			BGM_ModeDetect( work ) ;
			ModeDetectCondition( work ) ;
			break ;
		case BGM_MODE_ALERT :
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbgmmanegaer:ALERT!![%x]\n",GM_GetSngCode( ));
			BGM_ModeAlert( work ) ;
			ModeAlertCondition( work ) ;
			break ;
		case BGM_MODE_AVOID :
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbgmmanegaer:AVOID!![%x]\n",GM_GetSngCode( ));
			BGM_ModeAvoid( work ) ;
			ModeAvoidCondition( work ) ;
			break ;
		case BGM_MODE_SEARCH :
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbgmmanegaer:SEARCH!!\n");
			BGM_ModeSearch( work ) ;
			ModeSearchCondition( work ) ;
			break ;

		case BGM_MODE_VR_CLEAR :
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbgmmanegaer:CLEAR!!\n");
			BGM_ModeVrClear( work ) ;
			break ;
	}
#ifdef DEBUG_MODE
if ( GM_Debug2PMode == GM_DEBUG_MODE_ENEMY_THINK ) {
//	DEBUG_Locate( 40 + ( 72 ), 230, 0 );
//	DEBUG_Printf( "TH1=%2d\n", work->bgm_mode );
//	DEBUG_Printf( "TH2=%2d\n", work->bgm_mode2 );
//	DEBUG_Printf( "CT2=%2d\n", work->count2 );
}
#endif


}

static void CheckMessage( work )
Work	*work ;
{
	GV_MSG *msg;
	int mes_num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
			case SD_SNEAK_AUTO :
				work->mess_at = msg->message[1] ;
printf("bgmanage.c: message sneak auto fader[0x%x]\n",work->mess_at ) ;
			break ;
			case SD_SNEAK_NO_FADEOUT :
				work->status |= BGM_SNEAK_NO_FADEOUT ;
printf("bgmanage.c: sneak no fade out\n" ) ;
			break ;
		}
		msg--;
	}
}

static void BGM_AutoFader( work )
Work	*work ;
{
//	if ( work->current_bgm == SDMAN_BGM_PLAY_SNEAK ) {
	if ( GM_GetSngCode( ) == SDMAN_BGM_PLAY_SNEAK ) {
		if ( work->current_at != work->mess_at ) {
			BGM_GM_SdSet( work->mess_at ) ;
			work->current_at = work->mess_at ;
printf("bgmanage.c: change sneak auto fader[0x%x]\n",work->mess_at ) ;
		}
	}
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{

	CheckMessage( work ) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( GM_GameStatus & STATE_VR_ONLY ) {
			if ( GM_VRStatus & GM_VR_IDLE ) return ;
		} else {
			if ( GM_VRStatus & GM_VR_WINDOW_OPEN ) return ;
		}
	}

	BGM_Manager( work ) ;
	BGM_AutoFader( work ) ;
	BGM_System( work ) ;

#ifdef JOUCHU_CUT
	if ( JOUCHU_CUT_TIME++ == 180 ) {
		GM_SdSet( SE_JOUCHUU_OFF2 ) ;
	}
#endif
}

static	void	Die( work )
Work		*work ;
{
//	if ( (GM_AlertMode == ALERT_MODE_SNEAK) ) {
	if ( GM_GetSngCode( ) == SDMAN_BGM_PLAY_SNEAK ) {
		/* フェードアウトポーズ状態で同じ曲番の違うデータを再生するとバグる*/
		if ( !(work->status & BGM_SNEAK_NO_FADEOUT) ) {
			printf( "BGM MANAGER FADE OUTS S !!!\n");
			if ( GM_GameStatus & STATE_VR_ONLY ) {
//				BGM_GM_SdSet( SNG_FOUTS_SS );
				BGM_GM_SdSet( SNG_STOP );
			} else {
				BGM_GM_SdSet( SNG_FOUTS_S );
			}
		}
#ifdef DEBUG_MODE
		 else {
			printf( "BGM MANAGER NOT FADE OUT !!!\n");
		}
#endif
	} else {
		if ( work->status & BGM_END_FADEOUT ) {
			printf( "BGM MANAGER BGM_END_FADEOUT FADE OUTS S !!!\n");
			if ( GM_GameStatus & STATE_VR_ONLY ) {
//				BGM_GM_SdSet( SNG_FOUTS_SS );
				BGM_GM_SdSet( SNG_STOP );
			} else {
				BGM_GM_SdSet( SNG_FOUTS_S );
			}
		}
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	int	i ;
	
	/* status */
	work->status = GCL_GetOptionValue( 's', 0 ) ;

	work->name = name ;
	work->count2 = 0 ;
	work->sys_count = 0 ;
	work->current_bgm = 0 ;
	work->current_at = -1 ;
	work->mess_at = -1 ;

#ifdef JOUCHU_CUT
	JOUCHU_CUT_TIME = 0 ;
#endif
#ifdef	BGM_CUT
	BGM_GM_SdSet( SNG_FOUTS_S );
#endif

	InitBgmComp( work ) ;

	/* フェーダー関連初期化 */
	for ( i=0; i<MAX_TRACK; i++ ) {
		fader_work[ i ] = NULL ;
		fader_area_num[ i ] = 0 ;
	}

	work->current_bgm = GM_GetSngCode( ) ;
printf("BGM MANAGER SET ALERT MODE [%d] CURRENT BGM[%x]\n",GM_AlertMode,work->current_bgm ) ;
	switch( GM_AlertMode ) {
		case ALERT_MODE_SNEAK :
			if ( work->current_bgm == SDMAN_BGM_PLAY_SNEAK ) {
				/* 鳴りっぱなし */
				work->bgm_mode = BGM_MODE_SNEAK ;
				work->bgm_mode2 = BGM_MODE2_START ;
			} else {
				if ( GM_GameStatus & STATE_VR_ONLY ) {
					/*いきなり鳴らすため何故かSNG_STOPをいれるとfad_inが無効になる*/
					BGM_GM_SdSet( SNG_STOP );
					work->bgm_mode = BGM_MODE_SNEAK ;
					work->bgm_mode2 = BGM_MODE2_START ;
					work->count2 = 15 ;	/*すぐに曲が鳴るように*/
				} else {
					work->bgm_mode = BGM_MODE_ALERT ;
					work->bgm_mode2 = BGM_MODE2_START ;
				}
			}
			break ;
		case ALERT_MODE_ALERT :
#ifdef JIK_BGM_CONTINUE
			if ( work->current_bgm == SDMAN_BGM_PLAY_DETECT ||
				 work->current_bgm == SDMAN_BGM_PLAY_ALERT ||
				 work->current_bgm == SDMAN_BGM_PLAY_AVOID ) {
				BGM_GM_SdSet( SD_AUTO_FADER2 ) ;
				work->bgm_mode = BGM_MODE_ALERT ;
				work->bgm_mode2 = BGM_MODE2_IDLE ;
			} else {
				if ( GM_GameStatus & STATE_VR_ONLY ) {
					/*いきなり鳴らすため何故かSNG_STOPをいれるとfad_inが無効になる*/
					BGM_GM_SdSet( SNG_STOP );
				}
				work->bgm_mode = BGM_MODE_ALERT ;
				work->bgm_mode2 = BGM_MODE2_START ;
			}
#else
			work->bgm_mode = BGM_MODE_ALERT ;
			work->bgm_mode2 = BGM_MODE2_START ;
#endif
			break ;
		case ALERT_MODE_AVOID :
#ifdef JIK_BGM_CONTINUE
			if ( work->current_bgm == SDMAN_BGM_PLAY_DETECT ||
				 work->current_bgm == SDMAN_BGM_PLAY_ALERT ||
				 work->current_bgm == SDMAN_BGM_PLAY_AVOID ) {
				BGM_GM_SdSet( SD_AUTO_FADER3 ) ;
				work->bgm_mode = BGM_MODE_AVOID ;
				work->bgm_mode2 = BGM_MODE2_IDLE ;
printf("avoid kore kore kore kore kore kroe kore [%d]\n",work->current_bgm );
			} else {
				work->bgm_mode = BGM_MODE_AVOID ;
				work->bgm_mode2 = BGM_MODE2_START ;
			}
#else
			work->bgm_mode = BGM_MODE_AVOID ;
			work->bgm_mode2 = BGM_MODE2_START ;
#endif
		break ;
		case ALERT_MODE_SEARCH :
			work->bgm_mode = BGM_MODE_ALERT ;
			work->bgm_mode2 = BGM_MODE2_START ;
		break ;

	}

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewBGMManager( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_PREV, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
