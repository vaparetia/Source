//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    objs_test.c
    モデル表示テスト
    2000/03/15 Yuuta Kunibe	
    $Id: harrier_stage_break.c,v 1.1.1.3 2002/11/19 11:44:42 Yoshizawa1 Exp $
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

#include "libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../../shibata/util/ts_util.h"


#define	WATER_LEVEL		(-40000.0f)

typedef struct {
	GV_ACT_EX	actor;
	DG_OBJS		*objs;
	FMATRIX		light[2];
	int			name;
    int 		map;
	int			count;
    FVECTOR		vec;
    SVECTOR		rot;

    int			step;
	int			*flag;
    
} Work ;



static void Act(Work *work)
{

    GM_GroupObjs( work->objs, GM_CurrentStageMap );

    switch ( work->step ) {
    case 0:
	if ( *work->flag ) {
	    work->step = 1;
	}
	break;
    case 1:
	DG_SetPos( &work->objs->world );
	DG_RotatePos( &work->rot );
	DG_GetPos( &work->objs->world );
    
	_sceVu0AddVector( (FVECTOR *)work->objs->world.m[3], (FVECTOR *)work->objs->world.m[3], &work->vec );
	work->vec.vx *= 0.98f;
	work->vec.vz *= 0.98f;
	work->vec.vy -= 10.0f;

	if ( work->objs->world.m[3][1] <= WATER_LEVEL ) {
		/* 水柱 */
	    /*extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense, int flag );
		NewWaterSurfaceMountain( (FVECTOR *)work->objs->world.m[3],
					 1000.0f + frnd()*300.0f, 3000.0f + frnd()*1000.0f, 0x01 );*/

		GV_DestroyActor( work );
	}
	break;
    }
    
}

static void Die(Work *work)
{
	if ( work->objs ) {
	    DG_DequeueObjs( work->objs );
	    DG_FreeObjs( work->objs );
	}
}


static DG_OBJS *InitObjs( Work* work, int objcode, FMATRIX *mat )
{

	DG_DEF		*def;
	DG_OBJS		*objs;


	def = (DG_DEF *)GV_GetCache( GV_CacheID( objcode, 'k' ) ) ;
	if ( !def ) {
	    return NULL;
	}

	objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if ( !objs ) {
	    return NULL;
	}

	if ( DG_QueueObjs( objs ) < 0 ) {
	    return NULL;
	}

	DG_COPY_MAT( &objs->world, mat );
	GM_GroupObjs( objs, GM_CurrentStageMap );	

	return objs;

}



static int GetResources( Work *work, int objcode, FMATRIX *mat, int *flag )
{

	FVECTOR	bound_max;
	FVECTOR	bound_min;

	
	work->count = 0;
	work->step  = 0;
	work->flag  = flag;
	
	work->objs = InitObjs( work, objcode, mat );
	if ( work->objs == NULL ) return -1;

	work->vec.vy = 300.0f + frnd()*60.0f;
	work->vec.vx = -250.0f+frnd()*150.0f;
	work->vec.vz = -50.0f + frnd()*100.0f;

	work->rot.vx = irnd()%32;
	work->rot.vy = irnd()%32;
	work->rot.vz = irnd()%32;

	bound_max.vx = 500.0f;
	bound_max.vy = 0.0f;
	bound_max.vz = 1000.0f;

	bound_min.vx =-500.0f;
	bound_min.vy = 0.0f;
	bound_min.vz =-1000.0f;

	/* 仮板炎 */
	//NewBoxFire( &work->objs->world, &bound_max, &bound_min, &work->flag );
	
	return 0;

}



void *NewHarrierStageObjs( int objcode, FMATRIX *mat, int *flag )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect( GV_ACTOR_USER, sizeof(Work) );
	if ( work ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, objcode, mat, flag ) < 0 ) {
			GV_DestroyActor(work);
			return NULL ;
		}
	}

	return (void *)work ;
}
