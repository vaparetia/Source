//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breath_demo.c
	息：デモバージョン

	2001/06/15 S.Okajima
	$Id: breath_demo.c,v 1.1.1.3 2002/11/19 11:47:29 Yoshizawa1 Exp $
*/



#ifdef PSX2	///
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
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define STRIP_WIDTH_BASE (0.20f)
#define STRIP_WIDTH_SIN  (0.05f)


/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			put_flag;
	int			cnt_count;
	int			cnt_count_max;
	FMATRIX		*world;
	FVECTOR 	shift;
} Work ;

/* ---------------------------------------------------------------- */
enum {
	REQ_PUT=0,
	REQ_CONT,
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
		  case REQ_PUT:
			work->put_flag = 1;
			break;
		  case REQ_CONT:
			work->cnt_count     = 0;
			work->cnt_count_max = DIRECT_TICK( msg->message[1] );
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			  break;
		}
		msg--;
	}
}

static int OK_Dummy_1 = 1;

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	CheckMesgParam( work );

	if( work->cnt_count_max > 0 ){
		work->cnt_count--;
		if( work->cnt_count < 0){
			work->cnt_count = work->cnt_count_max;
			work->put_flag = 1;
		}
	}

	if( work->put_flag ){
		extern void  *NewBreath( FMATRIX *world, FVECTOR *move, int *sw );
		NewBreath( work->world, &work->shift, &OK_Dummy_1 );
	}

	work->put_flag = 0;
}

static void Die( Work *work )
{
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	work->put_flag = 0;
	work->cnt_count_max = 0;
	return 0 ;
}


void *NewBreathDemo( int name, FMATRIX *world, FVECTOR *shift )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->world = world;
		DG_COPY_VEC( &work->shift, shift );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

