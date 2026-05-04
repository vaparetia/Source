//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_claster_exp.c
    クラスター爆発管理エフェクト
    2001/07/07 Yuuta Kunibe	
    $Id: bridge_exp.c,v 1.1.1.3 2002/11/19 11:44:34 Yoshizawa1 Exp $
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



#define	UPPER_FLOOR_HEIGHT	(-1000.0f)
#define	LOWER_FLOOR_HEIGHT	(-4000.0f)

#define	STAGE_CENTER_X		(0.0f)	
#define	STAGE_WIDTH_X		(3000.0f)

#define	STAGE_MAX_Z		(-137000.0f)
#define	STAGE_MIN_Z		(-164000.0f)
#define	STAGE_WIDTH_Z		(STAGE_MIN_Z-STAGE_MAX_Z)

#define	HOLE_MAX_X		(1500.0f)
#define	HOLE_MIN_X		(-1500.0f)
#define	HOLE_MAX_Z		(-148500.0f)
#define	HOLE_MIN_Z		(-152000.0f)

#define	STAGE_PIPE_X		(5200.0f)
#define	STAGE_PIPE_Y		(-2800.0f)

#define	RANDOM_Z		(1000.0f)

#define	PANNEL1_Z1		(-152650.0f)
#define	PANNEL1_Z2		(-153850.0f)
#define	PANNEL1_Z3		(-155050.0f)

#define	PANNEL2_Z1		(-152950.0f)
#define	PANNEL2_Z2		(-154750.0f)

#define	TANK_Z			(-153800.0f)

#define	EXP_COUNT		(60)
#define	LIFE			(300)



/*--- クラスター爆発単体エフェクト ---*/
extern void *NewBridgeSmoke( FVECTOR *position );
extern void *NewMissileExplosion( FVECTOR* center, float size );
extern void *NewBombKasu( FVECTOR *pos, SVECTOR *pole_rot );
extern void GM_SeSetEx( int, FVECTOR*, int, int );



typedef	struct	{

    GV_ACT_EX	actor;
    int		count;

    int		name;
    int 	map;
    GV_MSG	*msg;
    int		code;

	int		se_count;
	
    FVECTOR	hit_pos;
    
} Work;



static int BridgeRecieveMessage( Work* work )
{
    
    GV_MSG*	msg;
    int 	n_msg;


    if ( work->name == 0 ) {
		return 0;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &work->msg ); 

    msg = work->msg;

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:
	    return 1;
	    break;
	default:
	    return 0;
	    break;
	}
	msg++;
    }

    return 0;
    
}



static void NewBridgeExplosionUnit( Work *work )
{

    FVECTOR	pos;
    SVECTOR	rot;
    float	now_z;


	/* カウンターからパラメータ算出 */
	now_z = STAGE_MAX_Z + STAGE_WIDTH_Z * ( 0.5f + frnd()*0.5f );

	/* パラメータから爆発位置を算出し爆発呼び出し */
	pos.vx = STAGE_CENTER_X + STAGE_WIDTH_X * frnd();
	pos.vz = now_z + frnd()*RANDOM_Z;
	pos.vw = 1.0f;
		
	/* ステージ両端の踊り場で爆発 */
	if ( pos.vz > -137500.0f || pos.vz < -162000.0f ) {
	    pos.vy = 500.0f;
	}	    
	/* ステージ中心の穴に着弾したら下の階で爆発 */
	else if ( pos.vz < HOLE_MAX_Z && pos.vz > HOLE_MIN_Z && pos.vx < HOLE_MAX_X && pos.vx > HOLE_MIN_X ) {
	    pos.vy = LOWER_FLOOR_HEIGHT;
	}
	/* 通常ステージ面で爆発 */
	else {
	    pos.vy = UPPER_FLOOR_HEIGHT;
	}

	NewBridgeSmoke( &pos );


	/* 岡嶋さん爆発核呼び出し */
	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 0;
	pos.vy += 500.0f + frnd()*500.0f;
	NewBombKasu( &pos, &rot );

	if ( work->count == 0 || work->se_count == 0 ) {
		if ( ( irnd()>>2 )%2 ) {
			GM_SeSetMode( SD_W_EXPLOS02, &pos, GM_SEMODE_BOMB );
		}
		else {
			GM_SeSetMode( SD_W_EXPLOS01, &pos, GM_SEMODE_BOMB );
		}
		NewPadVibration2( GV_StrCode( "explosion" ), 0 );
	}


	if ( ++work->se_count >= 4 ) {
		work->se_count = 0;
	}
	
}


/* アクト関数 */
static void Act( Work *work )
{


    /*{
	GV_MSG	msg;

	if ( GV_PadData[1].press & PAD_R2 ) {
	    work->code = 0;
	    msg.address = work->name;
	    msg.message = &work->code;
	    msg.message_len = 1;
	    GV_SendMessage( &msg );
	}

    }*/

    

    
    if ( work->count < DIRECT_TICK( 180 ) ) {
		NewBridgeExplosionUnit( work );
    }

    work->count++;

    if ( BridgeRecieveMessage( work ) ) {
	    GM_SeSetMode( SD_W_EXPLOS02, (FVECTOR *)DG_Chanls[0].eye.m[3], GM_SEMODE_BOMB );	
		NewPadVibration2( GV_StrCode( "explosion" ), 0 );
		GV_DestroyActor( work );
    }   
    
}


static void Die( Work *work )
{
}



static int GetResources( Work *work )
{

    work->count = 0;
	work->se_count = 0;
    
    return 0;

}



void *NewBridgeExplosion( void )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->map  = 0;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work;

}


void *NewBridgeExplosionScn( int name, int map )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map  = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work;

}


