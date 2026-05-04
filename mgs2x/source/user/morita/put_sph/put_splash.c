//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_splash.c
   飛沫出し

   1999/12/13 T. Morita
   $Id: put_splash.c,v 1.1.1.3 2002/11/19 11:46:32 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

typedef struct work_t
{
    GV_ACT    actor  ;
    DG_OBJS  *objs   ;
} Work ;

static void Die( Work *work )
{
    DG_FreeObjs( work->objs );
}

static void Act( Work *work )
{
}

static int GetResources( Work *work, int name, int where )
{
    int      i, id, buf[3] ;
    FMATRIX *mtx ;
    extern void *NewBodySplash( DG_OBJS *objs, CONTROL *control, int model_id ) ;

    if ( (id = GCL_GetOptionValue( 'm', 0 )) )
	work->objs = DG_MakeObjs( GV_GetCache(GV_CacheID( id, 'k' )), DG_FLAG_ONEPIECE, 0 ) ;
    else
	return -1 ;

    mtx = &work->objs->world ;
    if ( GCL_GetOption( 's' ) != NULL )
    {
	mtx->m[0][0] *= GCL_GetNextInt()/100.0f ;
	mtx->m[1][1] *= GCL_GetNextInt()/100.0f ;
	mtx->m[2][2] *= GCL_GetNextInt()/100.0f ;
    }
    if ( GCL_GetOption( 'r' ) != NULL )
    {
	int r ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixX( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixY( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixZ( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    }
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&mtx->m[3] ) ;
	mtx->m[3][3] = 1.0f ;
    }
    for ( i=work->objs->n_models ; --i>=0 ; )
	work->objs->objs[i].world = work->objs->world ;

    if ( !NewBodySplash( work->objs, NULL, id ) )
	return -1 ;
    return 0 ;
}


void *NewPutSplashObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
