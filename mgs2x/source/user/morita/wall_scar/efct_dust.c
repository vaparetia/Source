//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   efct_dust.c
   ほこり処理

   1999/11/09 T. Morita
   $Id $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#define WLLSCR_N_DUST 3
#define WLLSCR_GRAVITY 2
#define WLLSCR_MAX_SEC 60*4

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT   actor ;

    DG_PRIM2 *prim  ;
    float     vx,vz ;
    int       sec   ;
} Work ;


static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
}

static void Act( Work *work )
{
    int i, flag = 1, *alpha ;
    FVECTOR        *pos = work->prim->pos[0] ;
    DG_PRIM2_UVRGB *u   = work->prim->uvrgb[0] ;

    for ( i=WLLSCR_N_DUST ; --i>=0 ; pos+=2, u+=2 )
    {
	alpha = (int*)&(pos+1)->vw ;
	if ( --(*alpha) > 0 )
	{
	    flag = 0 ;
	    if ( *alpha < 128 )
	    {
		(pos+1)->vx = pos->vx ;	pos->vx += work->vx ;
		(pos+1)->vy = pos->vy ;	pos->vy += pos->vw -= WLLSCR_GRAVITY ;
		(pos+1)->vz = pos->vz ;	pos->vz += work->vz ;
		u->a = *alpha/2 ;
	    }
	}
    }
    if ( flag )
        GV_DestroyActor( work ) ;
}

static void InitLinePrim( DG_PRIM2_UVRGB *u, int i, int n_verts, u_int rgba )
{
    int j ;

    for ( ; --i>=0 ; )
        for ( j=0 ; j<n_verts ; j++, u++ )
        {
            u->f = j ? 0x0fff : 0x8fff ;
            u->r =  (rgba >>  0) & 0xff ;
            u->g =  (rgba >>  8) & 0xff ;
            u->b =  (rgba >> 16) & 0xff ;
            u->a = ((rgba >> 24) & 0xff)*j/n_verts ;
        }
}

static int GetResources( Work *work, FVECTOR *p, float vx, float vz )
{
    int i ;
    int alpha_start[]={ 5, 18, 60 } ;

    if ( !(work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF,
                                      WLLSCR_N_DUST, 2 )) )
        return -1 ;
    DG_SetPrim2Alpha( work->prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ) ;
    work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
    work->prim->as_world.m[3][X] = p->vx ;
    work->prim->as_world.m[3][Y] = p->vy ;
    work->prim->as_world.m[3][Z] = p->vz ;

    InitLinePrim( work->prim->uvrgb[0], WLLSCR_N_DUST, 2, 0x40404040 ) ;

    for ( i=WLLSCR_N_DUST ; --i>=0 ; )
    {
	*(int*)&work->prim->pos[0][i*2+1].vw = 128+alpha_start[i] ;
	work->prim->pos[0][i*2].vx = work->prim->pos[0][i*2].vy =
	    work->prim->pos[0][i*2].vz = work->prim->pos[0][i*2].vw =
	    work->prim->pos[0][i*2+1].vx = work->prim->pos[0][i*2+1].vy =
	    work->prim->pos[0][i*2+1].vz = 0 ;
    }
    work->vx = vx ;
    work->vz = vz ;
    work->sec = 0 ;

    return 0 ;
}


void *NewWallScarDust( FVECTOR *p, float vx, float vz )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, p, vx,vz ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
