//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    objs_test.c
    モデル表示テスト
    2000/03/15 Yuuta Kunibe	
    $Id: harrier_tank_break.c,v 1.1.1.3 2002/11/19 11:44:42 Yoshizawa1 Exp $
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


#define	INIT_X		(-385.0f)
#define	INIT_Y		(200.0f)
#define	INIT_Z		(-153800.0f)

#define OFFSET_X	(75.0f)
#define	OFFSET_Y	(775.0f)
#define	OFFSET_Z	(-100.0f)

static FVECTOR Tank_Offset = { OFFSET_X, OFFSET_Y, OFFSET_Z, 1.0f };

typedef struct {

	GV_ACT_EX	actor;

	DG_OBJS		*bottom;
	DG_OBJS		*top;
	
	FVECTOR		vec_bottom;
	FVECTOR		vec_top;

	SVECTOR		rot_bottom;
	SVECTOR		rot_top;
    
	FMATRIX		light[2];
	int			name;
	int 		map;
	int			count;

	int			*flag;
	int			step;

} Work ;


extern void *NewMissileExplosion( FVECTOR* center, float size );


static void Act(Work *work)
{

    FVECTOR	vectmp;
    
    GM_GroupObjs( work->bottom, GM_CurrentStageMap );
    GM_GroupObjs( work->top,    GM_CurrentStageMap );

	switch ( work->step ) {
	case 0:
		if ( *work->flag ) {
			work->step = 1;
		}
		break;
	case 1:		/* まだくっついている */
		DG_SetPos( &work->bottom->world );
		DG_RotatePos( &work->rot_bottom );
		DG_GetPos( &work->bottom->world );

		_sceVu0AddVector( (FVECTOR *)work->bottom->world.m[3], (FVECTOR *)work->bottom->world.m[3], &work->vec_bottom );
		work->vec_bottom.vx *= 0.98f;
		work->vec_bottom.vy -= 10.0f;
		work->vec_bottom.vz *= 0.98f;
	
		DG_COPY_MAT( &work->top->world, &work->bottom->world );
		DG_SetPos( &work->bottom->world );
		DG_PutVector( &Tank_Offset, (FVECTOR *)work->top->world.m[3], 1 );

		/* 分裂 */
		if ( ++work->count == 30 ) {
			DG_COPY_VEC( &work->vec_top, &work->vec_bottom );
			_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->top->world.m[2], 200.0f );
			_sceVu0AddVector( &work->vec_top, &work->vec_top, &vectmp );

			_sceVu0ScaleVector( &vectmp, (FVECTOR *)work->top->world.m[2],-100.0f );
			_sceVu0AddVector( &work->vec_bottom, &work->vec_bottom, &vectmp );

			//NewMissileExplosion( (FVECTOR *)work->bottom->world.m[3], 1000.0f );

			work->step = 2;
		}
		break;
	case 2:		/* 別々に落下 */
		DG_SetPos( &work->bottom->world );
		DG_RotatePos( &work->rot_bottom );
		DG_GetPos( &work->bottom->world );

		_sceVu0AddVector( (FVECTOR *)work->bottom->world.m[3], (FVECTOR *)work->bottom->world.m[3], &work->vec_bottom );
		work->vec_bottom.vx *= 0.98f;
		work->vec_bottom.vy -= 10.0f;
		work->vec_bottom.vz *= 0.98f;

		DG_SetPos( &work->top->world );
		DG_RotatePos( &work->rot_top );
		DG_GetPos( &work->top->world );

		_sceVu0AddVector( (FVECTOR *)work->top->world.m[3], (FVECTOR *)work->top->world.m[3], &work->vec_top );
		work->vec_top.vx *= 0.98f;
		work->vec_top.vy -= 10.0f;
		work->vec_top.vz *= 0.98f;

		if ( ++work->count >= 180 ) {
		    GV_DestroyActor( work );
		}
		break;
	}

	
	
    /*if ( ++work->count >= 60 ) {
	DG_COPY_VEC( (FVECTOR *)work->objs->world.m[3], &GM_PlayerPosition );
	work->count = 0;
    }*/

}

static void Die(Work *work)
{
	if ( work->bottom ) {
	    DG_DequeueObjs( work->bottom );
	    DG_FreeObjs( work->bottom );
	}
	if ( work->top ) {
	    DG_DequeueObjs( work->top );
	    DG_FreeObjs( work->top );
	}
}


static int InitObjs( Work* work )
{

	DG_DEF		*def;

	/* タンク下部初期化 */
	if ( ( def = (DG_DEF *)GV_GetCache( GV_CacheID( GV_StrCode("w25a_tank_brk1"), 'k' ) ) ) == NULL ) {
	    return 0;
	}

	if ( ( work->bottom = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 ) ) == NULL ) {
	    return 0;
	}

	if ( DG_QueueObjs( work->bottom ) < 0 ) {
	    return 0;
	}

	DG_COPY_MAT( &work->bottom->world, &DG_UnitMatrix );
	work->bottom->world.m[3][0] = INIT_X;
	work->bottom->world.m[3][1] = INIT_Y;
	work->bottom->world.m[3][2] = INIT_Z;

	GM_GroupObjs( work->bottom, GM_CurrentStageMap );



	/* タンク上部初期化 */
	if ( ( def = (DG_DEF *)GV_GetCache( GV_CacheID( GV_StrCode("w25a_tank_brk0"), 'k' ) ) ) == NULL ) {
	    return 0;
	}

	if ( ( work->top = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 ) ) == NULL ) {
	    return 0;
	}

	if ( DG_QueueObjs( work->top ) < 0 ) {
	    return 0;
	}

	DG_COPY_MAT( &work->top->world, &work->bottom->world );
	DG_SetPos( &work->bottom->world );
	DG_PutVector( &Tank_Offset, (FVECTOR *)work->top->world.m[3], 1 );
	
	GM_GroupObjs( work->top, GM_CurrentStageMap );
	
	
	/*DG_GetLightMatrix( &GM_PlayerPosition, light );
	DG_SetLightMatrix( objs, light );

	light[1].m[3][0] = (float)GET_COL_R(color);
	light[1].m[3][1] = (float)GET_COL_G(color);
	light[1].m[3][2] = (float)GET_COL_B(color);*/

	return 1;

}



static int GetResources( Work *work, int *flag )
{

    work->flag = flag;
	work->step  = 0;
	work->count = 0;

	work->rot_bottom.vx = irnd()%128;
	work->rot_bottom.vy = irnd()%128;
	work->rot_bottom.vz = irnd()%128;

	work->rot_top.vx = irnd()%128;
	work->rot_top.vy = irnd()%128;
	work->rot_top.vz = irnd()%128;

	work->vec_bottom.vx =-300.0f;
	work->vec_bottom.vy = 200.0f;
	work->vec_bottom.vz = 0.0f;
	

	if ( !( InitObjs( work ) ) ) return -1;
	return 0;
}



void *NewHarrierTankBreak( int *flag )
{

	Work *work = NULL;

	work = (Work*)GV_NewEffect( GV_ACTOR_USER, sizeof(Work) );
	if ( work ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, flag ) < 0 ) {
			GV_DestroyActor(work);
			return NULL ;
		}
	}

	return (void *)work ;
}
