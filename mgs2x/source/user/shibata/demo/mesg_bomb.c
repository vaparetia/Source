//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mesg_bomb.c
	デモ用メッセージ起動エフェクトコールキャラ
	
	2000/02/07 T.Shibata
	
	$Id: mesg_bomb.c,v 1.1.1.3 2002/11/19 11:48:32 Yoshizawa1 Exp $

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
	FMATRIX		world;
} Work;

enum {
	BOMB_EFT = 0,
	FLYING_SMK,
	BOMB_GAS_EFT,
	CRUSH_FORCE,
	REC,
	SPARK,
	BLOOD_MIST,
};

//Ｃ４爆発
extern void *NewBombEffect( FVECTOR *pos, int mode );
//壁這う煙
extern void *NewFlyingSmoke( FVECTOR *pos, FVECTOR *vector, int life );
//中煙
extern void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot );
//ライン飛び散り
extern void *NewCrushWithForce( FVECTOR *pos );
//跳弾
extern void *NewSpark( FMATRIX *world );
//火花
extern void *NewTs_Spark( FVECTOR *center, FVECTOR *vec, float pow );
//血煙
extern void *AN_Blood_Mist( FVECTOR *pos );

static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	FVECTOR		fvtemp;
	SVECTOR		rot;
	float		force;
	
	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case BOMB_EFT:
			NewBombEffect( (FVECTOR*)work->world.m[3], msg->message[1] );
			break;
		  case FLYING_SMK:
			force = (float)msg->message[2];
			_sceVu0ScaleVector( &fvtemp, (FVECTOR*)work->world.m[2], force );
			NewFlyingSmoke( (FVECTOR*)work->world.m[3], &fvtemp, msg->message[1] );			
			break;
		  case BOMB_GAS_EFT:
			TS_VecToRot( &rot, (FVECTOR*)work->world.m[2] );
			NewBombGasEffect( (FVECTOR*)work->world.m[3], &rot );
			break;
		  case CRUSH_FORCE:
			NewCrushWithForce( (FVECTOR*)work->world.m[3] );
			break;
		  case REC:
			NewSpark( &work->world );
			break;
		  case SPARK:
			NewTs_Spark( (FVECTOR*)work->world.m[3], (FVECTOR*)work->world.m[2], (float)msg->message[1] );
			break;
		  case BLOOD_MIST:
			AN_Blood_Mist( (FVECTOR*)work->world.m[3] );
			break;
		  default:
			printf("mesg_bomb.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	GV_WaitMessage( work, work->name );
}

static void Die( Work *work )
{
}

void *NewMesgBomb1( int name, FMATRIX *world )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ));

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		DG_COPY_MAT( &work->world, world );
	}

	return (void *)work ;
}
