//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xbox_dmy.c
	未移植プログラム用ダミーキャラクタ

	2002/02/01 K.Takabe
	$Id: xbox_dmy.c,v 1.1.1.3 2002/11/19 11:51:13 Yoshizawa1 Exp $

*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libdg.h"
#include	"libmt.h"
#include	"gameheader.h"


typedef struct {
	GV_ACT_EX	actor ;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}
/* ---------------------------------------------------------------- */
static void *NewTakabeXBoxDummy( int actor_level )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( actor_level, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
#if 0
/* crosfade.c */
void *NewCrossFadeEffect( int time )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void *NewCrossFadeEffectCustom( int time, int capture_interval, int bright_time, int alpha_time, int flag )
{
	if ( bright_time == 0 && alpha_time == 0 && flag == 0 ){
		extern void *NewFrameSkipEffect( int time, int interval );
		return NewFrameSkipEffect( time, capture_interval );
	}
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
#endif
/* ---------------------------------------------------------------- */
#if 0
/* f_focus.c */
void *NewFarFocusEffectSet( int name, int where )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER ) );
}
void *NewFarFocusEffect( int name, int max_plane, int var_near, int var_far )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER ) );
}
#endif
/* ---------------------------------------------------------------- */
#if 0
/* frm_skip.c */
void *NewFrameSkipEffect( int time, int interval )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
#endif
/* ---------------------------------------------------------------- */
#if 0
/* grayclut.c */
void *NewGrayClut( int time, u_int add_col )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void *NewContrastClut( int time, SVECTOR *scale, SVECTOR *bias )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
#endif
/* ---------------------------------------------------------------- */
#if 0
/* ir_mode.c */
void *NewIRMode( void )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void *NewThermalMode( void )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
#endif
/* ---------------------------------------------------------------- */
#if 0
/* mini_scn.c */
void *NewReduceScreenEffect( void )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void *NewReduceScreenEffectEx( int phase, int prio )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void TAKABE_SetReduceScreenSize( void *work_ptr, int x1, int y1, int x2, int y2, int color )
{
}
void TAKABE_SetReduceScreenSizeF( void *work_ptr, float x1, float y1, float x2, float y2, int color )
{
}
#endif
/* ---------------------------------------------------------------- */
#if 0
/* n_focus.c */
void *NewNearFocusEffectSet( int name, int where )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER ) );
}
void *NewNearFocusEffect( int name, int max_plane, int var_near, int var_far )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER ) );
}
#endif
/* ---------------------------------------------------------------- */
/* raster.c */
#if 0
void *NewRasterEffect( int alpha, int color )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void *NewRasterEffectChanl( int alpha, int color, int chanl )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_AFTER2 ) );
}
void *NewRasterEffectChanlForCodec( int alpha, int color, int chanl )
{
	return ( NewTakabeXBoxDummy( GV_ACTOR_MANAGER ) );
}
#endif


