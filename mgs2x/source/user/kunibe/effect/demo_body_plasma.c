//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  electric_floor.c
  電撃床エフェクト呼び出し
  2001/06/01 Yuuta Kunibe	
  $Id: demo_body_plasma.c,v 1.1.1.3 2002/11/19 11:44:36 Yoshizawa1 Exp $
*/

#ifdef PSX2
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define COLOR			(255)
#define ALPHA			(128)

#define N_PRIMS			(2)
#define N_VERTS 		(64)
#define CENTER_NUM 		( N_PRIMS * N_VERTS / 2 )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	WAVE_HEIGHT		(50.0f)//(100.0f)
#define SPEED			(20.0f)//(50.0f)

/* 電撃床情報 ->> エリア変更あり */
#define	FLOOR_MAX_X		(9000.0f)
#define	FLOOR_MIN_X		(4000.0f)
#define	FLOOR_MAX_Z		(-239500.0f)
#define	FLOOR_MIN_Z		(-242000.0f)

#define	PANNEL_WX		(750.0f)
#define	PANNEL_WZ		(350.0f)

#define	BOMB_DETECT_SPEED2	(5.0f*5.0f)



extern void *NewSpritePlasma2( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color, int life );



enum {
    PLASMA_MODE_ACTIVE	= 0x00,
    PLASMA_MODE_SLEEP	= 0x01,
    PLASMA_MODE_DEATH	= 0x02,
};

enum {
    MESG_ACTIVE	= 0x00,
    MESG_SLEEP	= 0x01,
    MESG_KILL	= 0x02,
};

typedef	struct	{

    GV_ACT_EX	actor;    
    int		name;
    int		code[2];

    OBJECT	*body;

    int		mode;

    FVECTOR	color;
    int		count;

    GV_MSG	*msg;

    int		kill_flag;
    
} Work;




static void RecieveMessage( Work* work )
{
    
    GV_MSG	*msg;
    int 	n_msg;


    if ( work->name == 0 ) {
	return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 


    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case MESG_ACTIVE:		
	    work->mode = PLASMA_MODE_ACTIVE;
	    break;
	case MESG_SLEEP:
	    work->mode = PLASMA_MODE_SLEEP;
	    break;
	case MESG_KILL:
	    work->mode = PLASMA_MODE_DEATH;
	    break;
	}
	msg++;

    }

}



static void MakeBodyPlasma( Work *work, int from, int middle, int to, float shift )
{

    FVECTOR		vectmp;
    float		width;

    width = 10.0f + frnd()*2.5f;

    _sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[middle].world.m[2], shift );
    _sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[middle].world.m[3] );
    NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[from].world.m[3],
		      &vectmp,
		      (FVECTOR *)work->body->objs->objs[to].world.m[3],
		      width,
		      &work->color,
		      4 );
}


/* アクト関数 */
static void Act( Work *work )
{

    int 	from;
    int 	middle;
    int		to;

    float	shift;



    /* メッセージ取得 */
    RecieveMessage( work );
    

#if 0    /* debug用mode変更 */
    if ( GV_PadData[1].press & PAD_R1 ) {
	if ( work->mode == PLASMA_MODE_SLEEP ) {
	    work->mode = PLASMA_MODE_ACTIVE;
	}
	else {
	    work->mode = PLASMA_MODE_SLEEP;
	}
    }
    else if ( GV_PadData[1].press & PAD_L1 ) {
	work->mode = PLASMA_MODE_DEATH;
    }
#endif    


    switch ( work->mode ) {

    case PLASMA_MODE_ACTIVE:
	if ( !(work->count%3) ) {

	    switch ( irnd()%6 ) {
	    case 0:
		from   = 0;
		middle = 14;
		to     = 15;
		shift = 10.0f;
		break;
	    case 1:
		from   = 0;
		middle = 18;
		to     = 19;
		shift = 10.0f;
		break;
	    case 2:
		from   = 11;
		middle = 4;
		to     = 6;
		shift = 10.0f;
		break;
	    case 3:
		from   = 11;
		middle = 8;
		to     = 10;
		shift = 80.0f;
		break;
	    case 4:
		from   = 0;
		middle = 2;
		to     = 3;
		shift = 80.0f;
		break;
	    default:
		from   = 2;
		middle = 0;
		to     = 7;
		shift = -10.0f;
		break;

	    }

	    MakeBodyPlasma( work, from, middle, to, shift );

	    
	    switch ( irnd()%6 ) {
	    case 0:
		from   = 0;
		middle = 14;
		to     = 15;
		shift = -10.0f;
		break;
	    case 1:
		from   = 18;
		middle = 0;
		to     = 19;
		shift = 10.0f;
		break;
	    case 2:
		from   = 12;
		middle = 4;
		to     = 6;
		shift = 10.0f;
		break;
	    case 3:
		from   = 12;
		middle = 8;
		to     = 10;
		shift = -10.0f;
		break;
	    case 4:
		from   = 12;
		middle = 2;
		to     = 13;
		shift = -50.0f;
		break;
	    default:
		from   = 12;
		middle = 2;
		to     = 17;
		shift = -50.0f;
		break;
	    }

	    MakeBodyPlasma( work, from, middle, to, shift );

	}

	work->count++;	
	break;

    case PLASMA_MODE_SLEEP:
	work->count = 0;
	break;

    case PLASMA_MODE_DEATH:
	GV_DestroyActor(work);
	break;
    }


}


static void Die(Work *work )
{
}


static void InitWork( Work *work, OBJECT *body, int color )
{

    work->body  = body;    
    work->count = 30;

    work->mode = PLASMA_MODE_SLEEP;
    
    work->color.vx = (float)( (color>>24)&0xff );
    work->color.vy = (float)( (color>>16)&0xff );
    work->color.vz = (float)( (color>>8)&0xff );
    work->color.vw = (float)( (color)&0xff );

}



void *NewDemoBodyPlasma( int name, OBJECT *body, int color )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
	    work->name = name;
	    InitWork( work, body, color );
	}

	return (void *)work ;

}



