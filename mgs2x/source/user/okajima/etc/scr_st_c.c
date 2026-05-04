//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
/*
	scr_st_c.c
	スクリーンショット呼び出しキャラ
	1999/10/18 S.Okajima
	$Id: scr_st_c.c,v 1.1.1.3 2002/11/19 11:47:42 Yoshizawa1 Exp $
*/

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

#include	"gameheader.h"
#include	"libmt.h"

typedef	struct	{
   GV_ACT_EX		actor ;
   int			count;
   int			which;
   int			key;
} Work ;

#ifdef DEBUG_MODE

extern void SaveScreenShot( unsigned char *filename );
extern void DG_SaveScreen( char *filename );

static	void Act( Work *work )
{
	unsigned char filename[0x100];


//	if ( GV_PadData[0].press & PAD_AL ){
	if ( GV_PadDataDirect[work->which&1].press & work->key ){
//		sprintf( filename, "host0:/u/develop/mj002/preview/capture/capture.raw" );
		sprintf( filename, "host0:shot%02x.raw",work->count++ );
//		SaveScreenShot( filename );
		DG_SaveScreen( filename );
	}
}

static	void Die( Work *work )
{
}

static	int GetResources( Work *work )
{

	work->which = 0;
	if( GCL_GetOption( 'w' ) != NULL ){
		work->which = GCL_GetNextInt();
	}
	work->key = PAD_AL;
	if( GCL_GetOption( 'k' ) != NULL ){
		work->key = GCL_GetNextInt();
	}


	work->count=0;

	return 0;
}

#endif // DEBUG

void *NewScreenShotControl( void )
{
	Work		*work = NULL;

	OPERATOR() ;

#ifndef DEBUG_MODE
	printf("スクリーンショットはデバグモードのみで有効です。\n");
	//BP_PS2 HANGUP();
   //BP: Note that in a shipping build (I.e. DEBUG_MODE not defined) this HANGUP would have done nothing!
   //There is instances in the game (i.e. after completing Nikita VR mission wp64a where this actually occurs!)
   // Ignoring this hangup call makes the game function properly.
#else

//	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_DAEMON2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
#endif
	return (void *)work ;
}
