//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_act.c
   瓶壊れ アクト

   1999/12/02 T. Morita
   $Id: circle.c,v 1.1.1.3 2002/11/19 11:45:52 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"


typedef struct work_t
{
    GV_ACT    actor ;

    int       life  ;
    DG_PRIM2 *prim  ;
} Work  ;


static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
}

static void Act( Work *work )
{
    if ( work->life > 0 )
	if ( --work->life == 0 )
	    GV_DestroyActor( work ) ;
}

static void SetUVS( DG_PRIM2_UVRGB *u0, DG_PRIM2_UVRGB *u1, int f, int rgba )
{
    int r,g,b,a ;

    r = (rgba>> 0)&0xff ;
    g = (rgba>> 8)&0xff ;
    b = (rgba>>16)&0xff ;
    a = (rgba>>24)&0xff ;

    u0->q = u1->q = 4096 ;
    u0->f = u1->f = f ;
    u0->r = u1->r = r ;
    u0->g = u1->g = g ;
    u0->b = u1->b = b ;
    u0->a = u1->a = a ;
}

#define N_VERTS 64


static int GetResources( Work *work,
			 FMATRIX *root,
			 FVECTOR *pos,
			 float radius, int rgba, int life )
{
    int i ;
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;
    FVECTOR rad = { 0, 0, radius, 1.0f } ;
    FMATRIX mat ;

    if ( !(work->prim = GM_MakePrim2( DG_PRIM2_LINE, 1, N_VERTS )) )
	return -1 ;
    work->prim->root = root ;
    u0 = work->prim->uvrgb[0] ;
    u1 = work->prim->uvrgb[1] ;
    p0 = work->prim->pos  [0] ;
    p1 = work->prim->pos  [1] ;
    for( i=0 ; i<N_VERTS ; i++ )
    {
	_sceVu0RotMatrixY( &mat, &DG_UnitMatrix, ((float)i - N_VERTS/2.0f)/N_VERTS*4.0f*M_PI ) ;
	_sceVu0ApplyMatrix( p0, &mat, &rad ) ;
	_sceVu0AddVector( p0, p0, pos ) ;
	_sceVu0CopyVector( p1++, p0++ ) ;
	SetUVS( u0++, u1++, (i ? 0x0fff : 0x8fff), rgba ) ;
    }
    work->life = life ;

    return 0 ;
}

void *NewDrawCircleLife( FMATRIX *root, FVECTOR *pos, float radius,
		     int rbga, int life )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if ( GetResources( work, root, pos, radius, rbga, life ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewDrawCircle( FMATRIX *root, FVECTOR *pos, float radius, int rbga )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, Die ) ;
        if ( GetResources( work, root, pos, radius, rbga, -1 ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
