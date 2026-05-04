//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sejimaku.c
   字幕付きＳＥ
   2001/10/30	M.Sonoyama
   $Id: sejimaku.c,v 1.1.1.3 2002/11/19 11:41:55 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "sejimaku.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

static	int			N_JimakuSe ;
static	JimakuSe	JimakuSeSet[ MAX_JIMAKUSE ] ;

typedef	struct	{
	GV_ACT		actor ;
	int			time ;
} Work ;

static	Work		*JimakuSeAct ;

static	void	Act( Work *work )
{
	if ( GM_StreamIsPlay() ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	if ( -- work->time <= 0 ) {
		GM_JimakuHide() ;
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( Work *work )
{
	if ( work == JimakuSeAct ) JimakuSeAct = NULL ;
}

static	JimakuSe	*SearchJimakuSe( int code )
{
	int		i ;
	
	for ( i = 0; i < N_JimakuSe; i ++ ) {
		if ( JimakuSeSet[ i ].code == code ) return &JimakuSeSet[ i ] ;
	}
	return NULL ;
}

static	void	*NewJimakuSe( int code )
{
	Work		*work ;
	JimakuSe	*this ;

	if ( GM_StreamIsPlay() ||
		 ( GM_Configuration & GM_CONFIG_CAPTION_OFF ) ||
		GM_CheckMenuStatus( MENU_CAPTION_OFF ) ) return NULL ;

	this = SearchJimakuSe( code ) ;
	if ( this == NULL ) return NULL ;
	
	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		/* 前のアクトが残っていたら殺す */
		if ( JimakuSeAct != NULL ) {
			GV_DestroyOtherActor( JimakuSeAct ) ;
			JimakuSeAct = NULL ;
		}
		GV_SetActor( &work->actor, Act, Die ) ;
		work->time = this->time ;
		GM_JimakuShow( 0, this->caption ) ;
		JimakuSeAct = work ;
	}
	return work ;
}

void	GM_JimakuSeSet( int p, int l, int n )
{
	GM_SeSet( p, l, n ) ;
	NewJimakuSe( n ) ;
}

void	GM_JimakuSeSetMode( int se, FVECTOR *pos, int mode )
{
	GM_SeSetMode( se, pos, mode ) ;
	NewJimakuSe( se ) ;
}

void	GM_JimakuSeSetEx( int se, FVECTOR *pos, int max_dis, int min_dis )
{
	GM_SeSetEx( se, pos, max_dis, min_dis ) ;
	NewJimakuSe( se ) ;
}


/*----------------------------------------------------------------*/

int	GM_COM_AddSeJimaku( void )
{
	JimakuSe	*this ;
	int			code, time ;
	char		*caption ;
	int			i ;

	if ( N_JimakuSe == MAX_JIMAKUSE ) {
		printf( "warning : jimakuse max over\n" ) ;
		return 0 ;
	}

	code = GCL_GetOptionValue( 's', 0 ) ;
	GCL_GetOption( 'r' ) ;
	caption = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString()) ;
	time = 0 ;
	for ( i = 0; i < sizeof( JimakuSeDefs ) / sizeof( JimakuSeDef ); i ++ ) {
		if ( JimakuSeDefs[ i ].code == code ) {
			time = JimakuSeDefs[ i ].time ;
			break ;
		}
	}
	if ( time == 0 ) {
		printf( "warning : se %d is not defined in sejimaku.h\n", code ) ;
		return 0 ;
	}

	this = &JimakuSeSet[ N_JimakuSe ] ;
	this->code = code ;
	this->time = ( int )( ( float )( time + 300 ) / ( 1000.0F / ( float )( 300 / TIME_BASE ) ) ) ;
	this->caption = caption ;
	N_JimakuSe ++ ;

	return 0 ;
}
