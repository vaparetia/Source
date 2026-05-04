//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	Har_sound.c								*/
/*	ハリア音関連								*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id: har_sdmng.c,v 1.1.1.3 2002/11/19 11:48:24 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>
#endif

#include "libmt.h"
#include "utl_dma.h"
#include "gameheader.h"
#include "libutl.h"
#ifdef KP_XBOX
#include "har_sdmng.h"
#endif

#define HARRIER_TARBIN		0x10
#define HARRIER_HOBARING	0x11
#define HARRIER_DOPPLER		0x12
#define KASACKA_WING		0x13
#define KASACKA_TARBIN		0x14

typedef struct {
    GV_ACT_EX		actor ;
    DG_OBJS		*obj ;
	int            toggle ;
}Work;

/********************************************************************************/
/*	Program									*/
/********************************************************************************/
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Work *work){
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void sound_control						*/
/*	引数:	Work	*work							*/
/*	説明:	サウンドコントロール						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Act_BGM_Man( Work *work )
{
    if (GM_MenuStatus & MENU_MENU_OPEN){
		if ( work->toggle == 1 ) {
			GM_MixConvFader( HARRIER_TARBIN, GM_PAN_CENTER, 0 ) ;
			GM_MixConvFader( HARRIER_HOBARING, GM_PAN_CENTER, 0 ) ;
			GM_MixConvFader( HARRIER_DOPPLER, GM_PAN_CENTER, 0 ) ;
			GM_MixConvFader( KASACKA_WING, GM_PAN_CENTER, 0 ) ;
			GM_MixConvFader( KASACKA_TARBIN, GM_PAN_CENTER, 0 ) ;
			work->toggle = 0 ;
		}
    } else if ( work->toggle == 0 ) {
		work->toggle = 1 ;
#ifdef KP_XBOX
		GM_MixConvFader( HARRIER_TARBIN, GM_PAN_CENTER, GM_MAX_VOL ) ;
		GM_MixConvFader( HARRIER_HOBARING, GM_PAN_CENTER, GM_MAX_VOL ) ;
		GM_MixConvFader( HARRIER_DOPPLER, GM_PAN_CENTER, GM_MAX_VOL ) ;
		GM_MixConvFader( KASACKA_WING, GM_PAN_CENTER, GM_MAX_VOL ) ;
		GM_MixConvFader( KASACKA_TARBIN, GM_PAN_CENTER, GM_MAX_VOL ) ;
#endif	
	}
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void sound_control						*/
/*	引数:	Work	*work							*/
/*	説明:	サウンドコントロール						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources( Work *work ){
	work->toggle = 1 ;
    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewHarBGM_Manager						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewHarBGM_Manager( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), 30 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act_BGM_Man,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work )){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

#ifdef KP_XBOX

#if 1 //BP

SD_3D_SNG_TRACK* HAR_GetSngTrackTable( void )
{
   BP_SOUND_TODO_BREAK;
   return 0;
}

#else

static SD_3D_SNG_TRACK_TABLE	sng_track_table;
static int	nRefer = 0;

SD_3D_SNG_TRACK* HAR_GetSngTrackTable( void )
{
	if( nRefer == 0 ) {	// 初起動
		DSI3DL2BUFFER ds3db;
		int h[ 5 ];
		int i;

		ZeroMemory( &ds3db, sizeof( ds3db ));

		// ３Ｄ音バッファテーブルのセットアップ
		ZeroMemory( &sng_track_table, sizeof( sng_track_table ) );
		// ハンドルの取得
		h[ 0 ] = sng_track_table[ TR_HARRIER_TARBIN ].handle = sd_3d_get_handle();
		h[ 1 ] = sng_track_table[ TR_HARRIER_HOBARING ].handle = sd_3d_get_handle();
		h[ 2 ] = sng_track_table[ TR_HARRIER_DOPPLER ].handle = sd_3d_get_handle();
		h[ 3 ] = sng_track_table[ TR_KASACKA_WING ].handle = sd_3d_get_handle();
		h[ 4 ] = sng_track_table[ TR_KASACKA_TARBIN ].handle = sd_3d_get_handle();

		sd_3d_set_sng_track_table( SNG_PLAY_01, sng_track_table );

		// もともとのフェーダは最大にしておく
		GM_MixConvFader( TR_HARRIER_TARBIN, GM_PAN_CENTER, 0x3f );
		GM_MixConvFader( TR_HARRIER_HOBARING, GM_PAN_CENTER, 0x3f );
		GM_MixConvFader( TR_HARRIER_DOPPLER, GM_PAN_CENTER, 0x3f );
		GM_MixConvFader( TR_KASACKA_WING, GM_PAN_CENTER, 0x3f );
		GM_MixConvFader( TR_KASACKA_TARBIN, GM_PAN_CENTER, 0x3f );

		// 遮蔽設定をリセットしボリュームを下げておく
		for ( i = 0 ; i < 5 ; i++ ) {
			sd_3dsrc_seti3dl2( h[ i ], &ds3db );
			sd_3dsrc_setvol( h[ i ], DSBVOLUME_MIN );
		}
	}
	++nRefer;
	return sng_track_table;
}

void HAR_ReleaseSngTrackTable( void )
{
	if( --nRefer == 0 ) {
		// 完全開放
		// ハンドルの開放
		sd_3d_release_handle( sng_track_table[ TR_HARRIER_TARBIN ].handle, 0);
		sd_3d_release_handle( sng_track_table[ TR_HARRIER_HOBARING ].handle, 0);
		sd_3d_release_handle( sng_track_table[ TR_HARRIER_DOPPLER ].handle, 0);
		sd_3d_release_handle( sng_track_table[ TR_KASACKA_WING ].handle, 0);
		sd_3d_release_handle( sng_track_table[ TR_KASACKA_TARBIN ].handle, 0);
		sd_3d_release_sng_track_table( sng_track_table );
	}
}

#endif //BP

#endif //KP_XBOX
