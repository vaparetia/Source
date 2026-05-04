//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dsegment.c
   シナリオ透明壁／透明床
   
   2000/01/28 M.Sonoyama
   $Id: dsegment.c,v 1.1.1.3 2002/11/19 11:50:42 Yoshizawa1 Exp $
   */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
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

#ifndef VIEW_MODE
#define		WAIT_MESSAGE
#endif

enum {
    MSG_KILL = 0,
    MSG_ON,
    MSG_OFF,
    MSG_MOVE,
    MSG_ROTATE,
	MSG_SHIFT,
    MAX_MSG
} ;

enum {
    SEGMENT = 0,
    FLOOR,
} ;

typedef struct {
    GV_ACT 		actor ;
    HZX_D_SEGMENT	*segment ;
    int			name ;
    int			type ;
    int			n ;
#ifdef VIEW_MODE
	int			map ;
#endif
} Work;

/*------------------------------------------------------------*/

static	void	GetIV( iv, ptr, n )
IVECTOR			*iv ;
int				*ptr ;
int n;
{
	int			*dst ;

	dst = &iv->vx ;
	while( -- n >= 0 ) *( dst ++ ) = *( ptr ++ ) ;
}

static	void	RotateSegment( seg, rot, mode )
HZX_D_SEGMENT		*seg ;
SVECTOR				*rot ;
int					mode ;
{
	IVECTOR			iv[ 1 ] ;

	switch( mode ) {
	case 0 :
		iv[ 0 ].vx 
			= ( seg->def[ 0 ].vx + seg->def[ 1 ].vx ) / 2 ;
		iv[ 0 ].vy 
			= ( seg->def[ 0 ].vy + seg->def[ 1 ].vy ) / 2 ;
		iv[ 0 ].vz 
			= ( seg->def[ 0 ].vz + seg->def[ 1 ].vz ) / 2 ;
		break ;
	case 1 :
		iv[ 0 ].vx = seg->def[ 0 ].vx ;
		iv[ 0 ].vy = seg->def[ 0 ].vy ;
		iv[ 0 ].vz = seg->def[ 0 ].vz ;
		break ;
	case 2 :
		iv[ 0 ].vx = seg->def[ 1 ].vx ;
		iv[ 0 ].vy = seg->def[ 1 ].vy ;
		iv[ 0 ].vz = seg->def[ 1 ].vz ;					
	}
	HZX_RotateDynamicSegment( seg, &iv[ 0 ], rot ) ;
}

static	void	RotateFloor( flr, rot, mode )
HZX_D_FLOOR		*flr ;
SVECTOR			*rot ;
int				mode ;
{
	IVECTOR		*v = NULL, iv ;
	int			n, p1, p2 ;

	n = flr->tag[ 0 ] ;

	switch( mode ) {
	case 0 :
		v = &iv ;
		if ( n == 3 ) {
			iv.vx = ( flr->def[ 0 ].vx + flr->def[ 1 ].vx + flr->def[ 2 ].vx ) / 3 ;
			iv.vy = ( flr->def[ 0 ].vy + flr->def[ 1 ].vy + flr->def[ 2 ].vy ) / 3 ;
			iv.vz = ( flr->def[ 0 ].vz + flr->def[ 1 ].vz + flr->def[ 2 ].vz ) / 3 ;
		} else {
			iv.vx = ( flr->def[ 0 ].vx + flr->def[ 1 ].vx 
					 + flr->def[ 2 ].vx + flr->def[ 3 ].vx ) / 4 ;
			iv.vy = ( flr->def[ 0 ].vy + flr->def[ 1 ].vy 
					 + flr->def[ 2 ].vy + flr->def[ 3 ].vy ) / 4 ;
			iv.vz = ( flr->def[ 0 ].vz + flr->def[ 1 ].vz 
					 + flr->def[ 2 ].vz + flr->def[ 3 ].vz ) / 4 ;
		}
		break ;
	case 1 :
	case 2 :
	case 3 :
	case 4 :
		v = &flr->def[ mode - 1 ] ;
		break ;
	case 5 :
	case 6 :
	case 7 :
	case 8 :
		v = &iv ;
		p1 = mode - 5 ; p2 = p1 + 1 ;
		if ( p2 == 4 ) p2 = 0 ;
		iv.vx = ( flr->def[ p1 ].vx + flr->def[ p2 ].vx ) / 2 ;
		iv.vy = ( flr->def[ p1 ].vy + flr->def[ p2 ].vy ) / 2 ;
		iv.vz = ( flr->def[ p1 ].vz + flr->def[ p2 ].vz ) / 2 ;
	}
	HZX_RotateDynamicFloor( flr, v, rot ) ;
}

/*------------------------------------------------------------*/

static 	void 	Act( work )
Work		*work ;
{
    GV_MSG	*msg ;
    HZX_D_SEGMENT	*seg ;
	HZX_D_FLOOR		*flr ;
	IVECTOR			iv[ 4 ] ;
	SVECTOR			rot ;
    int				n_msg, code, mode, *atr ;

#ifdef VIEW_MODE
	GM_SetCurrentMap( work->map ) ;
	if ( work->type == SEGMENT ) {
		HZX_ViewDynamicSegment( work->segment ) ;
	} else {
		HZX_ViewDynamicFloor( ( HZX_D_FLOOR * )work->segment, work->n ) ;
	}
#endif
    if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) == 0 ) {
		return ;
	}
    seg = work->segment ;
    if ( work->type == SEGMENT ) {
		atr = &seg->atr ; 
		flr = NULL ;
	} else {
		flr = ( HZX_D_FLOOR * )seg ;
		seg = NULL ;
		atr = &flr->atr ;
	}
    while ( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch ( code ) {
		case MSG_KILL :
			GV_DestroyActor( work ) ;
			return ;
		case MSG_ON :
			*( int * )GV_UNCACHE( atr ) = *atr
				= ( *atr ) & ~HZX_SEG_SKIP ; /* HZX_SEG_SKIP == HZX_FLOOR_SKIP */
			break ;
		case MSG_OFF :
			*( int * )GV_UNCACHE( atr ) = *atr
				= ( *atr ) | HZX_SEG_SKIP ; /* HZX_SEG_SKIP == HZX_FLOOR_SKIP */
			break ;
		case MSG_MOVE :
			if ( seg != NULL ) {
				GetIV( &iv[ 0 ], &msg->message[ 1 ], 4 ) ;
				GetIV( &iv[ 1 ], &msg->message[ 5 ], 4 ) ;
				HZX_MoveDynamicSegment( seg, &iv[ 0 ], &iv[ 1 ] ) ;
			} else {
				GetIV( &iv[ 0 ], &msg->message[ 1 ], 3 ) ;
				GetIV( &iv[ 1 ], &msg->message[ 4 ], 3 ) ;				
				GetIV( &iv[ 2 ], &msg->message[ 7 ], 3 ) ;				
				if ( work->n == 4 ) 
					GetIV( &iv[ 3 ], &msg->message[ 10 ], 3 ) ;
				HZX_MoveDynamicFloor( flr, &iv[ 0 ], &iv[ 1 ], &iv[ 2 ], &iv[ 3 ] ) ;
			}
			break ;
		case MSG_ROTATE :
			mode = msg->message[ 1 ] ;
			if ( seg != NULL ) {
				rot.vx = 0 ;
				rot.vy = msg->message[ 2 ] ;
				rot.vz = 0 ;
				RotateSegment( seg, &rot, mode ) ;
			} else {
				rot.vx = msg->message[ 2 ] ;
				rot.vy = msg->message[ 3 ] ;
				rot.vz = msg->message[ 4 ] ;
				RotateFloor( flr, &rot, mode ) ;
			}
			break ;
		case MSG_SHIFT :
			iv[ 0 ].vx = msg->message[ 1 ] ;
			iv[ 0 ].vy = msg->message[ 2 ] ;
			iv[ 0 ].vz = msg->message[ 3 ] ;
			if ( seg != NULL ) HZX_ShiftDynamicSegment( seg, &iv[ 0 ] ) ;
			else			   HZX_ShiftDynamicFloor( flr, &iv[ 0 ] ) ;
			break ;
		}
		msg ++ ;
    }
#ifdef WAIT_MESSAGE
	GV_WaitMessage( work, work->name ) ;
#endif
}

static	void	Die( work )
Work		*work ;
{
    if ( work->type == SEGMENT ) {
		HZX_RemoveDynamicSegment( work->segment ) ;
    } else {
		HZX_RemoveDynamicFloor( ( HZX_D_FLOOR * )work->segment ) ;
    }
}

static	int	GetResources( work, name, where, type )
Work		*work ;
int		name, where, type ;
{
    ALIGN16_PRE int ALIGN16_POST	pos[ 16 ], pos2[ 16 ] ; 
    int		*p ;
    u_int	atr, n, seNo, direct ;

    work->name = name ;
    work->type = type ;
    p = pos ;
    atr = 0 ;

	if ( GCL_GetOption( 'm' ) != NULL ) {
		where = GM_GetMapID( GCL_GetNextInt() ) ;
	} 
	seNo = GCL_GetOptionValue( 'N', 0 ) ;
	direct = ( GCL_GetOption( 'd' ) != NULL ) ? 1 : 0 ;

    if ( type == SEGMENT ) {
		GCL_GetOption( 'p' ) ;
		while( GCL_NextStr() != NULL ) {
			*( p ++ ) = GCL_GetNextInt() ;
		}
		if ( GCL_GetOption( 'a' ) != NULL ) atr = GCL_GetNextInt() ;
		/* defaultで、「壁こんこんならない」「スプレーつかない」「弾痕つかない」
		   「レーダー映らない」「敵ＩＫみない」属性を付加しておく */
		atr = atr | ( ( seNo & 0xf ) << 28 ) ;
		if ( !direct ) {
			atr |= HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_DISP_RADAR |
				   HZX_SEG_NO_KNOCK_SE | HZX_SEG_NO_SPRAY | HZX_SEG_NO_ENEMY_IK ;
		}

		work->segment 
			= HZX_AddDynamicSegment( GM_GetHzxGroupID( where ),
									( IVECTOR * )pos, ( IVECTOR * )( pos + 4 ),
									atr ) ;
    } else {
		GCL_GetOption( 'p' ) ;
		while( GCL_NextStr() != NULL ) {
			*( p ++ ) = GCL_GetNextInt() ;
		}
		memcpy( &pos2[ 12 ], &pos[ 9 ], sizeof( int ) * 3 ) ;
		memcpy( &pos2[ 8 ], &pos[ 6 ], sizeof( int ) * 3 ) ;
		memcpy( &pos2[ 4 ], &pos[ 3 ], sizeof( int ) * 3 ) ;
		memcpy( &pos2[ 0 ], &pos[ 0 ], sizeof( int ) * 3 ) ;
		if ( GCL_GetOption( 'a' ) != NULL ) atr = GCL_GetNextInt() ;
		GCL_GetOption( 'n' ) ;
		n = work->n = GCL_GetNextInt() ;
		atr = atr | ( ( seNo & 0xf ) << 28 ) ;
		work->segment 
			= ( HZX_D_SEGMENT * )HZX_AddDynamicFloor( GM_GetHzxGroupID( where ),	
													 ( IVECTOR * )pos2,	
													 ( IVECTOR * )( pos2 + 4 ),
													 ( IVECTOR * )( pos2 + 8 ),
													 ( IVECTOR * )( pos2 + 12 ),
													 n, atr ) ;
    }
#ifdef WAIT_MESSAGE
	GV_WaitMessage( work, name ) ;
#endif
	return 0 ;
}

/* 透明壁 */
void *NewDynamicSegment( name, where )
int	name, where ;
{
    Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where, SEGMENT ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;	    
		}
    }
#ifdef VIEW_MODE
	work->map = where ;
#endif
    return ( void * )work ;
}

/* 透明床 */
void *NewDynamicFloor( name, where )
int	name, where ;
{
    Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where, FLOOR ) < 0 ) {
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

