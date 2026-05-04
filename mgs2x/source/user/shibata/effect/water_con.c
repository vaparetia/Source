//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    water_con.c
	水面のコントロール
	2000/01/05 T.Shibata
	
	$Id: water_con.c,v 1.1.1.3 2002/11/19 11:48:42 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"

typedef	struct	{
	GV_ACT_EX	actor;
	int			name;
	float		diff;
	float		to_level;
	int			timer;
	int			count;
} Work;

enum {
	MSG_WATER_LEVEL_TIME = 0,	//高さと時間
	MSG_WATER_LEVEL_DIFF,		//高さと変化量
	MSG_WATER_LEVEL_LEVEL,		//高さ直代入
};

extern float GM_WaterLevel;
extern int DM_FrameSkip;

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case MSG_WATER_LEVEL_TIME:
			work->to_level = msg->message[1];
			work->count = DIRECT_TICK(msg->message[2]);
			work->diff = (work->to_level - GM_WaterLevel)/(float)work->count;
			work->timer = 0;
			break;
		  case MSG_WATER_LEVEL_DIFF:
			work->to_level = (float)msg->message[1];
			work->diff = (float)msg->message[2];
			work->count = abs((int)((work->to_level - GM_WaterLevel)/work->diff));
			work->timer = 0;
			break;
		  case MSG_WATER_LEVEL_LEVEL:
			GM_WaterLevel = (float)msg->message[1];
			work->timer = -1;
			break;
		  default:
			printf("water_con.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return 0;
}

static void Act( Work *work )
{
	CheckMesgParam( work );

	if(work->timer>=0){
		work->timer += 1+DM_FrameSkip;
		GM_WaterLevel += (float)(1+DM_FrameSkip)*work->diff;
		if(work->timer < work->count){
			work->timer = -1;
			GM_WaterLevel = work->to_level;
		}
	}
	
}

static void Die(Work *work)
{
}

static int GetResources( Work *work, int name, float init_level )
{
	work->name = name;
	GM_WaterLevel = init_level;
	
	return 0;
}

void *NewWaterLevelControl_Demo( int name, float init_level )
{
	Work		*work ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ));
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, init_level ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return (void *)work ;
}

int NewWaterLevelControl( void )
{
	GM_WaterLevel = (float)GCL_GetOptionValue( 'l', 0 );

	if ( GCL_GetOption( 's' ) != NULL ){
		GM_WaterBulletSE = GCL_GetNextInt() ;
	}

	return 0;
}
