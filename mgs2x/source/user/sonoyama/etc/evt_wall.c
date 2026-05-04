//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   evt_wall.c
   イベント壁
   
   2000/04/25 M.Sonoyama
   $Id: evt_wall.c,v 1.1.1.3 2002/11/19 11:50:43 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#ifdef DEBUG_MODE
//#define		VIEW_MODE
#endif

enum {
    MSG_KILL = 0,
    MAX_MSG
} ;

typedef struct {
    GV_ACT 			actor ;
    HZX_D_SEGMENT	*segment ;
    int				name ;
	int				flag ;
#ifdef VIEW_MODE
	int				map ;
#endif
} Work;

/*------------------------------------------------------------*/

static 	void 	Act( work )
Work		*work ;
{
    GV_MSG			*msg ;
    HZX_D_SEGMENT	*seg ;
    int				n_msg, code, *atr ;

#ifdef VIEW_MODE
	GM_SetCurrentMap( work->map ) ;
	HZX_ViewDynamicSegment( work->segment ) ;
#endif
    n_msg = GV_ReceiveMessage( work->name, &msg ) ;
    seg = work->segment ;
	atr = &seg->atr ; 
    while ( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch ( code ) {
		case MSG_KILL :
			GV_DestroyActor( work ) ;
			return ;
		}
		msg ++ ;
    }
	if ( GM_CheckPlayerStatus( PLAYER_EVENT_ENABLE ) ) {
		if ( work->flag == 0 ) {
			*( int * )GV_UNCACHE( atr ) = ( *atr ) | HZX_SEG_SKIP ;
			work->flag = 1 ;
		}
	} else if ( work->flag == 1 ) {
		*( int * )GV_UNCACHE( atr ) = ( *atr ) & ~HZX_SEG_SKIP ;
		work->flag = 0 ;		
	}
}

static	void	Die( work )
Work		*work ;
{
	if ( work->segment != NULL ) {
		HZX_RemoveDynamicSegment( work->segment ) ;
	}
}

static	int	GetResources( work, name, where )
Work		*work ;
int		name, where ;
{
    ALIGN16_PRE int ALIGN16_POST	pos[ 8 ] ; 
    int		*p ;
    u_int	atr ;

    work->name = name ;
    p = pos ;
    atr = HZX_SEG_NO_ENEMY | HZX_SEG_NO_BULLET | HZX_SEG_NO_MISSILE | 
		  HZX_SEG_NO_C4 | HZX_SEG_NO_RECOIL | HZX_SEG_NO_HARITSUKI |
		  HZX_SEG_NO_DISP_RADAR | HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_SPRAY ;
	GCL_GetOption( 'p' ) ;
	while( GCL_NextStr() != NULL ) {
		*( p ++ ) = GCL_GetNextInt() ;
	}
	work->segment 
		= HZX_AddDynamicSegment( GM_GetHzxGroupID( where ),
								( IVECTOR * )pos, ( IVECTOR * )( pos + 4 ),
								atr ) ;
	if ( work->segment == NULL ) return -1 ;
	/* 最初はＯＦＦ */
	work->segment->atr |= HZX_SEG_SKIP ;
	return 0 ;
}

/* イベント壁 */
void *NewEventWall( name, where )
int	name, where ;
{
    Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;	    
		}
    }
#ifdef VIEW_MODE
	work->map = where ;
#endif
    return ( void * )work ;
}

/*--------------------------------------------------------------------------*/

