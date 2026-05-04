//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dtrap.c
   動的トラップ（主としてテスト用）
   
   2000/08/07 M.Sonoyama
   $Id: dtrap.c,v 1.1.1.3 2002/11/19 11:50:42 Yoshizawa1 Exp $
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

#ifdef HZX_DTRP

enum {
    MSG_KILL = 0,
    MSG_ON,
    MSG_OFF,
    MAX_MSG
} ;

typedef struct {
    GV_ACT 			actor ;
	HZX_GROUP_ID	hzx_id ;
    HZX_D_TRP		*trap ;
	int				name ;
} Work;

/*------------------------------------------------------------*/


/*------------------------------------------------------------*/

static 	void 	Act( work )
Work		*work ;
{
    GV_MSG	*msg ;
    int				n_msg, code ;

//	NewBoundingBoxView( &work->trap->b1, &work->trap->b2, 232, 232, 32 ) ;

    if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) == 0 ) {
		return ;
	}

    while ( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch ( code ) {
		case MSG_KILL :
			work->trap->attribute |= HZX_TRP_ATR_NO_CHECK ;
			GV_DestroyActor( work ) ;
			return ;
		case MSG_ON :
			work->trap->attribute &= ~HZX_TRP_ATR_NO_CHECK ;
			break ;
		case MSG_OFF :
			work->trap->attribute |= HZX_TRP_ATR_NO_CHECK ;
			break ;
		}
		msg ++ ;
    }
	GV_WaitMessage( work, work->name ) ;
}

static	void	Die( work )
Work		*work ;
{
	if ( work->trap != NULL ) {
		HZX_RemoveDynamicTrap( work->hzx_id, work->trap ) ;
	}
}

static	inline	int	GetResources( Work *work, int name, int where )
{
	HZX_GROUP_ID	hzx_id ;
	float			b[ 6 ] ;
	int				atr, i ;

	if ( GCL_GetOption( 'm' ) == NULL ) {
		hzx_id = GM_GetHzxGroupID( where ) ;
	} else {
		hzx_id = GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt() ) ) ;
	}
	work->hzx_id = hzx_id ;
	work->name = name ;
	atr = GCL_GetOptionValue( 'a', 0 ) ;
	GCL_GetOption( 'b' ) ;
	for ( i = 0; i < 6; i ++ ) {
		b[ i ] = ( float )GCL_GetNextInt() ;
	}
	work->trap = HZX_AddDynamicTrap( hzx_id, name,
									 b[ 0 ], b[ 1 ], b[ 2 ], 
									 b[ 3 ], b[ 4 ], b[ 5 ], atr ) ;
	if ( work->trap == NULL ) return -1 ;
	return 0 ;
}
#endif

/* プログラム起動 */
void	*NewDynamicTrap( HZX_GROUP_ID hzx_id,
						 int		  name_id,
						 float		  minx, float miny, float minz,
						 float		  maxx, float maxy, float maxz,
						 int		  attribute )
{
#ifndef HZX_DTRP
	return ( void * )1 ;
#else
	Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_PREV, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		work->hzx_id = hzx_id ;
		work->name = name_id ;
		work->trap = HZX_AddDynamicTrap( hzx_id, name_id,
										 minx, miny, minz, maxx, maxy, maxz, attribute ) ;
		if ( work->trap == NULL ) {
			GV_DestroyActor( work ) ;
			return NULL ;	    
		}
    }
    return ( void * )work ;
#endif
}

/* シナリオ起動 */
void 	*NewDynamicTrapScn( name, where )
int	name, where ;
{
#ifndef HZX_DTRP
	return ( void * )1 ;
#else
    Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_PREV, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;	    
		}
    }
    return ( void * )work ;
#endif
}

/*--------------------------------------------------------------------------*/

