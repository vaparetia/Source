//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    body_fire.c
    体燃え
    2001/04/25 Yuuta Kunibe	
    $Id: solidus_test_fire.c,v 1.1.1.3 2002/11/19 11:44:51 Yoshizawa1 Exp $
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



#define	POINT_NUM	(200)

extern void SolidusMantGetFirePoint( FVECTOR *new_pos, int num );


typedef	struct	{

    GV_ACT_EX	actor;

    FVECTOR	center[POINT_NUM];
    FVECTOR	pos[POINT_NUM];
    float	rad[40];

    float	add_rad;

    int		cnt;
    
} Work;




/* アクト関数 */
static void Act( Work *work )
{

    int i,j;
    int id;
    float	wave;
    FVECTOR	*pos;
    FVECTOR	*center;

    pos    = work->pos;
    center = work->center;
    
    for ( i = 0 ; i < 40 ; i++ ) {

	work->rad[i] += work->add_rad;
	if ( work->rad[i] > PI ) {
	    work->rad[i] -= TPI;
	}

	wave = 100.0f * vu0_Sin(work->rad[i]);

	for ( j = 0 ; j < 5 ; j++ ) {
	    DG_COPY_VEC( pos, center );
	    pos->vy += wave;
	    pos++;
	    center++;
	}

    }
	

    id = work->cnt%20*10;
    SolidusMantGetFirePoint( &work->pos[id], 10 );

    work->cnt++;

}




static void Die( Work *work )
{

}





static int GetResources( Work *work )
{

    int 	i;
    FVECTOR	*center;
    FVECTOR	vectmp;

    work->cnt = 0;

    center = work->center;
    
    for ( i = 0 ; i < POINT_NUM ; i++ ) {

	vectmp.vx = 100.0f * (float)( i%5 - 2 );
	vectmp.vy = 0.0f;
	vectmp.vz = 50.0f * (float)( i/5 + 1 );
	vectmp.vw = 1.0f;

	_sceVu0AddVector( center, &GM_PlayerPosition, &vectmp );

	center++;

	if (!(i%5)) {
	    work->rad[i/5] = (float)( (i/5)%20 ) / 20.0f * TPI;
	}

    }

    work->add_rad = TPI/30.0f;

    return 0;

}



void *NewSolidusTestPoint( void )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



