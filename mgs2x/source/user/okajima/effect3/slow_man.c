//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*スローパラメータ制御スローパラメータ制御スローパラメータ制御スローパラメータ制御

	slow_man.c
	スローパラメータ制御
	2000/08/24 S.Okajima
	$Id: slow_man.c,v 1.1.1.3 2002/11/19 11:47:35 Yoshizawa1 Exp $

*/


#ifdef PSX2 ///
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"

#include	"../etc/ok_util.h"

extern int DM_FrameSkip ;

extern float OK_slow_param;

typedef	struct	{
	GV_ACT_EX	actor;
	int		name;
	int		where;

	float	slow_param_before;
	float	slow_param;
	int		count;
	int		count_max;
} Work ;

enum {
	REQ_PARAM=0,
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
		  case REQ_PARAM:
			work->slow_param_before = work->slow_param;
			work->slow_param = (float)(msg->message[1]) * 0.01f;
			work->count_max  = work->count = msg->message[2];
			break;
		  default:
			break;
		}
		msg--;
	}
}


static void Act( Work *work )
{
	CheckMesgParam( work );

	if( work->count > 0 ){
		work->count -= DM_FrameSkip ;
		work->count--;
		if( work->count < 0 ) work->count = 0;
		OK_slow_param = work->slow_param_before
		   + (work->slow_param - work->slow_param_before) * (float)(work->count_max - work->count) / (float)work->count_max;
//printf("OK_slow_param:%f\n",OK_slow_param);
	}
}

static void Die( Work *work )
{
}

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	if ( GCL_GetOption( 's' ) != NULL ){
		OK_slow_param = (float)GCL_GetNextInt() * 0.01f ;
	}
}

static int GetResources( Work *work )
{
	work->slow_param = work->slow_param_before = OK_slow_param;
	work->count = 0;
	GetOptionValue( work ) ;
	return (0);
}

void *NewSlowParamMan( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->name  = name;
		work->where = where;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

//add shibata
static int GetResources_prog( Work *work )
{
	work->slow_param = work->slow_param_before = OK_slow_param;
	work->count = 0;
	return (0);
}

void *NewSlowParamMan_prog( int name, int param )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->name  = name;
		work->where = 0;
		OK_slow_param = (float)param * 0.01f;
		if ( GetResources_prog( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
