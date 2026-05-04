//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	sdmanage.c
	サウンドマネージャー

	2000/01/04 Y.Korekado
	$Id: sdmanage.c,v 1.5 2002/11/23 12:16:40 Yoshizawa1 Exp $
*/
#endif
/*----------------------------------------------------------------
void	*NewSounddManager( name, where )

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

#include <libfs.h>

#include	"gameheader.h"

//#define BGM_CUT	(1)
#define	SE_CODE_PRINT_DEBUG (1)
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT	actor ;

	int		name ;
	int		status ;
	int		bgm_status ;
} Work ;

/* message */
enum {
	SD_CODE,
} ;

#define BGM_SYUKAN_ON			0x00000002	/* 主観時BGM下げる */
#define BGM_MIC_ON				0x00000008	/* マイク使用時BGM下げる */
/*----------------------------------------------------------------*/

#ifdef DEBUG_MODE
#include	"debugmenu.h"
extern int GM_SE_TYPE ;
#ifdef PSX2
/* 視界コントロール */
static GM_DEBUG_MENU se_type = {
 class:		"SE",
 menu:		"SE CENTER TYPE",
 max:		2,
 items:		( char *[] ){ "CAMERA", "PLAYER" },
 values:	( int [] ){ 0, 1 },
 target: 	NULL,
 mask:		1 ,
} ;
#else
static char *se_type_item[] = { "CAMERA", "PLAYER" } ;
static int se_type_value[] = { 0, 1 } ;

static GM_DEBUG_MENU se_type = {
	NULL, /* next */
	"SE", /* class */
	"SE CENTER TYPE", /* menu */
	se_type_item,/*items*/
	se_type_value, /* values */
	NULL,/*target*/
	1,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif
#endif

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
static void CheckMessage( work )
Work	*work ;
{
	GV_MSG *msg;
	int mes_num, code;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
			case SD_CODE :
			  	code = msg->message[1] ;
#ifdef BGM_CUT
				if ( code >= SNG_PLAY_01 && code <= SNG_PLAY_08 ) {
				} else {
					GM_SdSet( code );
				}
#else
				GM_SdSet( code );
#endif
printf("sdmanage.c: sd_code[0x%x]\n",code ) ;
			break ;
		}
		msg--;
	}
}
static void BGM_VolumeControl( work )
Work	*work ;
{
	/* 指向性マイク */
	if ( ((GM_AlertMode == ALERT_MODE_SNEAK) || (GM_AlertMode == ALERT_MODE_SEARCH) )
			&& !(GM_GameStatus & GM_STATUS_DETECT) ) {
		if( PL_GetPlayerWeapon( ) == WP_Mic ) {
			if ( !(work->bgm_status & BGM_MIC_ON) ) {
				GM_SdSet( SNG_MIC_ON );
				work->bgm_status |= BGM_MIC_ON ;
			}
		} else {
			if ( work->bgm_status & BGM_MIC_ON ) {
				GM_SdSet( SNG_MIC_OFF );
				work->bgm_status &= ~BGM_MIC_ON ;
			}
		}
	} else {
		if ( work->bgm_status & BGM_MIC_ON ) {
			GM_SdSet( SNG_MIC_OFF );
			work->bgm_status &= ~BGM_MIC_ON ;
		}
	}
	if( GM_PlayerStatus & PLAYER_WATCH ) {
		if ( !(work->bgm_status & BGM_SYUKAN_ON) ) {
			GM_SdSet( SNG_SYUKAN_ON );
			work->bgm_status |= BGM_SYUKAN_ON ;
//printf(" BGM SHUKAN ON\n");
		}
	} else {
		if ( work->bgm_status & BGM_SYUKAN_ON ) {
			GM_SdSet( SNG_SYUKAN_OFF );
			work->bgm_status &= ~BGM_SYUKAN_ON ;
//printf(" BGM SHUKAN OFF\n");
		}
	}
}
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	CheckMessage( work ) ;
	BGM_VolumeControl( work ) ;
}

static	void	Die( work )
Work		*work ;
{
//	GM_SdSet( SE_JOUCHUU_OFF ) ;
	/* 特殊擬音以外フェードアウト
		・エレベーター
	*/
	GM_SdSet( SE_JOUCHUU_OFF2 ) ;
	/* マイク音量ダウン解除 */
	GM_SdSet( SNG_MIC_OFF );
	/* 主観音量ダウン解除 */
	GM_SdSet( SNG_SYUKAN_OFF );
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	int		sd_num ;

printf( "SD MANAGER START !!!!!!!!!!!!!!!!!!!!!\n");

	GM_SdSet( SE_EXP_STOP );
	if( ( sd_num = GCL_GetOptionValue( 'p', -1 ) ) >= 0 ){
		printf( "load pak %x\n", sd_num );
		FS_LoadSoundPak( sd_num );
	}

	/* status */
	work->status = GCL_GetOptionValue( 's', 0 ) ;

	GM_SdLoadWait(  ) ;

	work->name = name ;
	work->bgm_status = 0 ;

#ifdef SE_CODE_PRINT_DEBUG
	GM_SdSet( 0xFF000000 );
#endif

#ifdef DEBUG_MODE
	GM_SE_TYPE = 0 ;
	se_type.target = &GM_SE_TYPE ;
	GM_AddDebugMenu( &se_type ) ; 	
#endif

#ifdef DEBUG_MODE
   //@02/08/12(月)@試しに鳴らしてみた
   //GM_SdLoadWait(  ) ;
   //GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_N_START1 );
#endif

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewSoundManager( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
