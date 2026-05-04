//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_elev.c 
   昇降機

   2001/01/16 T.Morita
   $Id: elv_shadow.c,v 1.1.1.3 2002/11/19 11:46:30 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../include/util.h"
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT_EX  actor  ;
    FMATRIX   *elev   ;
    float      cur_y  ;
    
    FVECTOR   *top    ;
    DG_PRIM2  *shadow_h ;
    DG_PRIM2  *shadow_v ;
    DG_PRIM2  *shade  ;
} Work ;

#define OFFT_X      0.0f
#define OFFT_Y -45550.0f
#define OFFT_Z -12248.0f

#if 0

FVECTOR ShadowShape_h[] = {
    {-2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f       +OFFT_Z, 1.0f},
    { 2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f       +OFFT_Z, 1.0f},
    {-2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f+450.0f+OFFT_Z, 1.0f},
    { 2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f+450.0f+OFFT_Z, 1.0f},
} ;

FVECTOR ShadowShape_v[] = {
    { 2100.0f-360.0f+OFFT_X, 552.0f+OFFT_Y, 450.0f -1952.0f+OFFT_Z, 1.0f},
    { 2100.0f       +OFFT_X, 552.0f+OFFT_Y, -1952.0f+OFFT_Z, 1.0f},
    { 2100.0f-360.0f+OFFT_X, 552.0f+OFFT_Y, 450.0f+ 2052.0f+OFFT_Z, 1.0f},
    { 2100.0f       +OFFT_X, 552.0f+OFFT_Y, 2052.0f+OFFT_Z, 1.0f},
} ;

#else

FVECTOR ShadowShape_h[] = {
    {-2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f       +OFFT_Z, 1.0f},
    { 2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f       +OFFT_Z, 1.0f},
    {-2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f+450.0f+OFFT_Z, 1.0f},
    { 2050.0f+OFFT_X, 552.0f+OFFT_Y, -1752.0f+450.0f+OFFT_Z, 1.0f},
} ;

FVECTOR ShadowShape_v[] = {
    { 2100.0f-360.0f+OFFT_X, 552.0f+OFFT_Y, 450.0f -1952.0f+OFFT_Z, 1.0f},
    { 2100.0f       +OFFT_X, 552.0f+OFFT_Y, -1952.0f+OFFT_Z, 1.0f},
    { 2100.0f-360.0f+OFFT_X, 552.0f+OFFT_Y, 450.0f+ 2052.0f+OFFT_Z, 1.0f},
    { 2100.0f       +OFFT_X, 552.0f+OFFT_Y, 2052.0f+OFFT_Z, 1.0f},
} ;

#endif

FVECTOR ShadeShape[] = {
    {-2020.0f+OFFT_X, 555.0f+OFFT_Y, -1752.0f+OFFT_Z, 1.0f},
    { 2020.0f+OFFT_X, 555.0f+OFFT_Y, -1752.0f+OFFT_Z, 1.0f},
    {-2020.0f+OFFT_X, 555.0f+OFFT_Y,  1752.0f+OFFT_Z, 1.0f},
    { 2020.0f+OFFT_X, 555.0f+OFFT_Y,  1752.0f+OFFT_Z, 1.0f},
} ;


static void Act( Work *work )
{
    int  i     ;
    int  alpha ;
    DG_PRIM2_UVRGB *u ;
    float y = work->elev->m[W][Y] ;
    float t ;

    t = (work->top->vy - y)/1000.0f ;
    if ( t < 1.0f )
    {
	alpha = (int)(96.0f * (1.0f - t)) ;
	DG_SwitchBuffPrim2( work->shadow_v ) ;
	u = work->shadow_v->uvrgb[work->shadow_v->buffer_clock] ;
	for( i=4 ; --i>=0 ; u++ )
	    u->a  = alpha ;
	DG_SwitchBuffPrim2( work->shadow_h ) ;
	u = work->shadow_h->uvrgb[work->shadow_h->buffer_clock] ;
	for( i=4 ; --i>=0 ; u++ )
	    u->a  = alpha ;
    }

    if ( t < 12.2f )
    {
	if ( t == 0.0f )
	    DG_InvisiblePrim2( work->shade ) ;
	else
	{
	    DG_VisiblePrim2( work->shade ) ;
	    DG_SwitchBuffPrim2( work->shade ) ;
	    u = work->shade->uvrgb[work->shade->buffer_clock] ;
	    alpha = (int)(64.0f * (t>1.0f ? 1.0f : t)) ;
	    for( i=4 ; --i>=0 ; u++ )
		u->a  = alpha ;
	}
    }
}

static void Die( Work *work )
{
    /* 影を解放 */
    if ( work->shadow_v )
	GM_FreePrim2( work->shadow_v ) ;
    if ( work->shadow_h )
	GM_FreePrim2( work->shadow_h ) ;

    if ( work->shade )
	GM_FreePrim2( work->shade  ) ;
}

static int GetResourcesCalled( Work *work, FMATRIX *elev, FVECTOR *top )
{
    work->top  = top  ;
    work->elev = elev ;
    if ( !(work->shadow_v = BRK_UTL_MakePOLY( 1,
					      8268169/*w12c_sdw05f_alp.pic*/,
					      SCE_GS_SET_ALPHA(0,1,0,1,0),
					      0x007f7f7f )) )
	PERROR( "Fail make Prim(no memory) : NewPutElevator\n" ) ;
    work->shadow_v->root = elev ;
    memcpy( work->shadow_v->pos[0], ShadowShape_v, sizeof(FVECTOR)*4 ) ;
    memcpy( work->shadow_v->pos[1], ShadowShape_v, sizeof(FVECTOR)*4 ) ;

    if ( !(work->shadow_h = BRK_UTL_MakePOLY( 1,
					      7219593/*w12c_sdw05e_alp.pic*/,
					      SCE_GS_SET_ALPHA(0,1,0,1,0),
					      0x007f7f7f )) )
	PERROR( "Fail make Prim(no memory) : NewPutElevator\n" ) ;
    work->shadow_h->root = elev ;
    memcpy( work->shadow_h->pos[0], ShadowShape_h, sizeof(FVECTOR)*4 ) ;
    memcpy( work->shadow_h->pos[1], ShadowShape_h, sizeof(FVECTOR)*4 ) ;

    if ( !(work->shade = BRK_UTL_MakePOLY( 1, 0,
					   SCE_GS_SET_ALPHA(0,1,0,1,0),
					   0x000f0f0f )) )
	PERROR( "Fail make Prim(no memory) : NewPutElevator\n" ) ;
    work->shade->root = elev ;
    memcpy( work->shade->pos[0], ShadeShape, sizeof(FVECTOR)*4 ) ;
    memcpy( work->shade->pos[1], ShadeShape, sizeof(FVECTOR)*4 ) ;

    return 0 ;
}

void *NewPutElevatorShadow( FMATRIX *elev, FVECTOR *top )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, elev, top ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
