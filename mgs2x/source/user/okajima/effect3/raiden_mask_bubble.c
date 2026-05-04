//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	raiden_mask_bubble.c
	ライデン水中マスクの泡

	2001/06/06 S.Okajima
	$Id: raiden_mask_bubble.c,v 1.1.1.3 2002/11/19 11:47:34 Yoshizawa1 Exp $
*/

#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"../effect2/bubble.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"


#define STRIP_WIDTH_BASE (0.20f)
#define STRIP_WIDTH_SIN  (0.05f)

#define INTERVAL  (60*5)

/*----------------------------------------------------------------*/
extern void *NewBubbleMany( FVECTOR *center, int life, int col );
extern float GM_WaterLevel;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;

	int			col;
	int			on_flag;

	EFTCONTROL	control;
	FMATRIX		dummy_world;
	FMATRIX		*world;

	FVECTOR		pos[2];

	int			count;
	int			cycle;
} Work ;

/* ---------------------------------------------------------------- */
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->on_flag = 0;
			break;
		  case REQ_ON:
			work->on_flag = 1;
			break;
		  case REQ_KILL:
		  case -1:
			GV_DestroyActor( work ) ;
			break;
		  default:
			  break;
		}
		msg--;
	}
}


static FVECTOR Shift[2] = {
	{ 130.0f,-45.0f, 100.0f, 1.0f },
	{-130.0f,-45.0f, 100.0f, 1.0f }
};
/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	CheckMesgParam( work );

	if( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE )
	 && !GM_CheckGameStatus( STATE_DEMO ) ){
		return;
	}

	if( work->name ){
		DM_EftControlMatrix( &work->control, &work->dummy_world ) ;
	}

	if( work->world->m[3][1] > GM_WaterLevel+1000.0f ) return;

	DG_SetPos( work->world );
	DG_PutVector( Shift, work->pos, 2 );
	if( work->count%work->cycle==0 ){
		NewBubbleMany( &work->pos[0], work->cycle, work->col );
		NewBubbleMany( &work->pos[1], work->cycle, work->col );
	}

	work->count++;
}

static void Die( Work *work )
{
	if( work->name ){
		DM_EftControlDelList( &work->control );
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	work->count = 0;
	work->on_flag = 1;

	return 0 ;
}

void *NewRaidenMaskBubble( FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->cycle = INTERVAL;
		work->name = 0;
		work->world = world;
		work->col = ((BUBBLE_COL_R<<24)|(BUBBLE_COL_G<<16)|(BUBBLE_COL_B<<8)|(BUBBLE_ALPHA));

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewRaidenMaskBubbleScn( int name, int where )
{
	Work	*work ;
	CONTROL	*ctrl;
	OBJECT	*object;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GCL_GetOption( 'n' ) != NULL ){
			ctrl = GM_SearchWhere( GCL_GetNextInt() );
			if( ctrl!=NULL ){
				object = (OBJECT *)(ctrl + 1);
				if( object!=NULL ){
					if( object->objs->n_models < HUMAN21_ATAMA ){
printf("ERR:NewRaidenMaskBubbleScn:a:\n");
						return NULL;
					}else{
						work->world = &object->objs->objs[HUMAN21_ATAMA].world;
						if( object==NULL ){
printf("ERR:NewRaidenMaskBubbleScn:b:\n");
							return NULL;
						}
					}
				}else{
printf("ERR:NewRaidenMaskBubbleScn:c:\n");
					return NULL;
				}
			}else{
printf("ERR:NewRaidenMaskBubbleScn:d:\n");
				return NULL;
			}
		}else{
printf("ERR:NewRaidenMaskBubbleScn:e:\n");
			return NULL;
		}

		work->cycle = INTERVAL;
		work->name = 0;
		work->col = ((BUBBLE_COL_R<<24)|(BUBBLE_COL_G<<16)|(BUBBLE_COL_B<<8)|(BUBBLE_ALPHA));

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewRaidenMaskBubbleDemo( int name, int col, int cycle )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->cycle = DIRECT_TICK( cycle );
		work->name = name;
		work->world = &work->dummy_world;
		DM_EftControlAddList( name, &work->control );
		work->col = col;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

