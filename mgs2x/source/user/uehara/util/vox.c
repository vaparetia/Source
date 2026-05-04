//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vox.c
		シナリオからストリーミング音声を呼び出す
	2000/10/02	K.Uehara
	$Id: vox.c,v 1.1.1.3 2002/11/19 11:51:38 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#include <sifdev.h>
#endif

#include "gameheader.h"
#include "libfs.h"
#include "strctrl.h"
#include "jimaku.h"

int NewStreamSet( void )
{
	int flag, pos;
	int handler;

	flag = 0;

	pos = GCL_GetNextInt();

	if( GCL_GetOption( 'w' ) != NULL ){
		flag |= GM_STREAM_PLAY_WAIT;
	}
	if( GCL_GetOption( 'p' ) != NULL ){
		int proc;
		proc = GCL_GetNextInt();
		flag |= GM_STREAM_FLAG_PROC( proc );
	}
	if( GCL_GetOption( 'c' ) != NULL ){
		int channel;
		channel = GCL_GetNextInt();
		flag |= GM_STREAM_FLAG_CHANNEL( channel );
	}
	if( GCL_GetOption( 'm' ) != NULL ){
		flag |= GM_STREAM_PAUSE_MENU;
	}

	if( GCL_GetOption( 'i' ) != NULL ){
		handler = GM_IpuStream( pos, flag );
	} else {
		handler = GM_VoxStream( pos, flag );
	}

	if( GCL_GetOption( 'a' ) != NULL ){
		int proc;
		proc = GCL_GetNextInt();
		GM_StreamSetActionProc( handler, proc );
	}

	return handler;
}

int NewStreamStart( void )
{
	int handler;

	handler = GCL_GetNextInt();

	GM_StreamStart( handler );

	return 0;
}

int NewStreamStatus( void )
{
	int handler;

	handler = GCL_GetNextInt();

	return GM_StreamStatus( handler );
}

int NewStreamStop( void )
{
	int handler;

	handler = GCL_GetNextInt();

	GM_StreamStop( handler );

	return 0;
}

int NewStreamStopAll( void )
{
	/* ゲームオーバーロゴが出た後は無効 */
	if ( GM_GameOverFlag & GM_OVERFLAG_LOGO_START ) return 0 ;

	GM_StreamStopAll();

	return 0;
}

int NewStreamIsPlay( void )
{
	return GM_StreamIsPlay();
}

extern float bp_pan64_to_angle( const int p );

int NewStreamSetPan( void )
{
	int handler, pan, vol;
   float bp_angle;

	handler = GCL_GetNextInt();

	pan = GCL_GetOptionValue( 'p', 0x20 );
	vol = GCL_GetOptionValue( 'v', 0x3F );

   //BP - looks like this command only has a non-center pan when it's used for the event
   //where you listen to Johnny with the directional mic.  Which means luckily that the pan
   //won't be behind the player.
   bp_angle = bp_pan64_to_angle( pan );

	GM_VoxStreamSetPan( handler, vol, pan, bp_angle);

	return 0;
}

int NewStreamPauseControl( void )
{
	int ch = 0;
	int flag = -1;

	if( GCL_GetOption( 'c' ) == NULL ){
		HANGUP();
		return -1;
	}
	ch = GCL_GetNextInt();

	if( GCL_GetOption( 'o' ) != NULL ){
		flag = 1;
	}
	if( GCL_GetOption( 'F' ) != NULL ){
		flag = 0;
	}
	if( flag < 0 ){
		HANGUP();
		return -1;
	}

	GM_StreamSdPauseControl( ch, flag );

	return 0;
}

int NewJimakuControl( void )
{
	if( GCL_GetOption( 'h' ) != NULL ){
		GM_JimakuHide();
	}
	if( GCL_GetOption( 'y' ) != NULL ){
		GM_JimakuSetPosY( GCL_GetNextInt() );
	}
	if( GCL_GetOption( 'd' ) != NULL ){
		GM_JimakuDisable();
	}

	
	return 0;
}
