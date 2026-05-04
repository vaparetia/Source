//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  electric_floor.c
  電撃床エフェクト呼び出し
  2001/06/01 Yuuta Kunibe	
  $Id: solidus_damage_spark.c,v 1.1.1.3 2002/11/19 11:44:49 Yoshizawa1 Exp $
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

#define	LIFE			(20)


extern void *NewSpritePlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to,
			      float width, float noise_plasma, FVECTOR *color, int flag, int life );

extern void *NewSpritePlasma2( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color, int life );

extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count );



typedef	struct	{

    GV_ACT_EX	actor;    
    int		name;
    int		code[2];

    OBJECT	*body;

    FVECTOR	color;

    int		flag;
    
    int		cnt;
    int		cnt1;
    int		cnt2;
    int		cnt3;
    int		cnt4;

    GV_MSG	*msg;

    int		kill_flag;
    
} Work;




/* アクト関数 */
static void Act( Work *work )
{

    FVECTOR	vectmp;
    float	width;



    /* 体の周りにバチバチプラズマ */
    if ( work->cnt4 > 0 ) {

	if ( !(work->cnt4%3) ) {	    

	    width = 20.0f + frnd()*10.0f;

	    switch ( irnd()%6 ) {
	    case 0:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[14].world.m[2], 10.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[14].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[0].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[15].world.m[3],
				  width,
				  &work->color,
				  4 );
		break;
	    case 1:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[18].world.m[2], 10.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[18].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[0].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[19].world.m[3],
				  width,
				  &work->color,
				  4 );
		break;
	    case 2:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[4].world.m[0], 10.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[4].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[11].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[6].world.m[3],
				  width,
				  &work->color,
				  4 );
		break;
	    case 3:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[8].world.m[0], 80.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[8].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[11].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[10].world.m[3],
				  width,
				  &work->color,
				  4 );
		break;
	    case 4:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[2], 80.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[0].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[3].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    case 5:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[2], -10.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[0].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[7].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    }

	    
	    switch ( irnd()%6 ) {
	    case 0:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[14].world.m[2], -10.0f );//-200.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[14].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[0].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[15].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    case 1:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[18].world.m[2], 10.0f );//-200.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[18].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[0].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[19].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    case 2:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[4].world.m[0], 10.0f );//150.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[4].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[12].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[6].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    case 3:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[8].world.m[0], -10.0f );//-150.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[8].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[12].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[10].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    case 4:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[2], -50.0f );//-250.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[12].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[13].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    case 5:
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[2], -50.0f );//-250.0f );
		_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)work->body->objs->objs[2].world.m[3] );
		NewSpritePlasma2( (FVECTOR *)work->body->objs->objs[12].world.m[3],
				  &vectmp,
				  (FVECTOR *)work->body->objs->objs[17].world.m[3],
				  width,
				  &work->color,
		                  4 );
		break;
	    }

	}

	work->cnt4--;	

    }
    else {
	GV_DestroyActor(work);
    }


}


static void Die(Work *work )
{
}


static void InitWork( Work *work, OBJECT *body )
{

    work->body = body;
    
    work->cnt  = 30;
    work->cnt1 = 15 + irnd()%10;
    work->cnt2 = 15 + irnd()%10;
    work->cnt3 = 2 + irnd()%3;
    work->cnt4 = LIFE;

    work->color.vx = 128.0f;
    work->color.vy = 80.0f;
    work->color.vz = 16.0f;
    work->color.vw = 24.0f;

}


void *NewSolidusDamageSpark( OBJECT *body )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
	    InitWork( work, body );
	}

	return (void *)work ;

}



