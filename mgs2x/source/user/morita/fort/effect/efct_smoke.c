//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_smoke.c
  フォーチュン戦 煙

  2001/02/08 T.Morita
  $Id: efct_smoke.c,v 1.1.1.3 2002/11/19 11:46:11 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"


#define SMOKE_SIZE   300.0f/*500.0f*/
#define SMOKE_SPEED  30.0f
#define SMOKE_N_PRIM 16

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    DG_PRIM2 *dust1 ;
    DG_PRIM2 *dust2 ;
    FVECTOR   vel[SMOKE_N_PRIM]  ;
    short     tics[SMOKE_N_PRIM] ;

    int       n_prim ;
    int       where ;
    FVECTOR   pos ;
    FVECTOR   dir ;
    FMATRIX  *world ;

    HZD_BOX  *hzd ;
} Work ;



#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_GRAVITY      4.0f
#define BRK_BOUNCE       1.08f
#define BRK_BOUNCE_WALL  1.08f
#define MAX_ALPHA        10


static void Act( Work *work )
{
    static FVECTOR    ofst = { SMOKE_SIZE/10.0f, SMOKE_SIZE/10.0f, SMOKE_SIZE/10.0f, 0.0f }  ;
    DG_PRIM2         *d1, *d2 ;
    FVECTOR          *nxt_p1, *prv_p1, *nxt_p2 ;
    DG_PRIM2_UVRGBWH *nxt_u1, *prv_u1, *nxt_u2 ;
    int   i ;

    d1 = work->dust1 ;
    d2 = work->dust2 ;
    prv_p1 = d1->pos  [d1->buffer_clock] ;
    prv_u1 = d1->uvrgb[d1->buffer_clock] ;
    DG_SwitchBuffPrim2( d2 ) ;
    DG_SwitchBuffPrim2( d1 ) ;
    nxt_p1 = d1->pos  [d1->buffer_clock] ;
    nxt_u1 = d1->uvrgb[d1->buffer_clock] ;
    nxt_p2 = d2->pos  [d2->buffer_clock] ;
    nxt_u2 = d2->uvrgb[d2->buffer_clock] ;

    for ( i=SMOKE_N_PRIM ; --i>=0 ; )
    {
	if ( work->vel[i].vw > 0.0f )
	    work->vel[i].vw -= 1.0f ;
	else if ( (nxt_u1[i].a = prv_u1[i].a) /* > (MAX_ALPHA-16)*/ )
	{
	    float a, d, r ;

	    if ( work->tics[i]++ > 30 )
		if ( !(work->tics[i] & (( 0xffff >> (nxt_u1[i].a /*+16-MAX_ALPHA*/)) & 0x3f)) )
		    nxt_u1[i].a-- ;

	    work->vel[i].vx *= 0.95f ;
	    work->vel[i].vz *= 0.95f ;
	    if ( work->vel[i].vy < 0.0f )
		work->vel[i].vy += BRK_GRAVITY ;
	    else
		work->vel[i].vy  = BRK_GRAVITY ;

	    d = sceVu0Sqrt( work->vel[i].vx*work->vel[i].vx + work->vel[i].vz*work->vel[i].vz ) ;
#define ANGtoRAD(_a) ((_a)*(float)M_PI/128.0f)
#define RADIUS(_d)   (SMOKE_SIZE*(work->tics[i]>9 ? 10 : work->tics[i]+1)/10.0f)
	    r = RADIUS( d ) ;
	    a = ANGtoRAD( work->tics[i] & 0xff )- d*0.05f ;
	    nxt_u1[i].w = (short)(r * sinf( a + i )) ;
	    nxt_u1[i].h = (short)(r * cosf( a + i )) ;
	    nxt_u2[i].w = (short)(r * sinf(-a + i )) ;
	    nxt_u2[i].h = (short)(r * cosf(-a + i )) ;

	    _sceVu0AddVector( &nxt_p1[i], &prv_p1[i], &work->vel[i] ) ;
	}
	else
	{
	    nxt_u1[i].a = MAX_ALPHA ;
	    nxt_u1[i].w = nxt_u1[i].h = nxt_u2[i].w = nxt_u2[i].h = 0 ;
	    if ( work->world )
		_sceVu0ApplyMatrix( &nxt_p1[i], work->world, &work->pos ) ;
	    else
		_sceVu0CopyVector( &nxt_p1[i], &work->pos ) ;
	    _sceVu0CopyVector( &work->vel[i], &work->dir ) ;
	    work->vel[i].vx += frnd()*(SMOKE_SPEED * 0.3f) ;
	    work->vel[i].vy += frnd()*(SMOKE_SPEED * 0.3f) ;
	    work->vel[i].vz += frnd()*(SMOKE_SPEED * 0.3f) ;
	    work->vel[i].vw = 0.0f ;

	    work->tics[i] = 0 ;
	}
	_sceVu0AddVector( &nxt_p2[i], &nxt_p1[i], &ofst ) ;
	nxt_u2[i].a = nxt_u1[i].a ;
    }
}

static void Die( Work *work )
{
    if ( work->dust1 )
	GM_FreePrim2( work->dust1 ) ;
    if ( work->dust2 )
	GM_FreePrim2( work->dust2 ) ;
}

static int GetResources( Work *work, FMATRIX *world, FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd )
{
    int   i ;
    float d ;

    work->hzd  = hzd  ;
    if ( !(work->dust1 = BRK_UTL_MakeRSPRTWH( SMOKE_N_PRIM,
#if 0
					      GV_StrCode( "w11c2_smoke1_alp" ),
					      SCE_GS_SET_ALPHA(2,0,0,1,0),
#else
					      GV_StrCode( "bombgas6_alp" ),
					      SCE_GS_SET_ALPHA(0,1,0,1,0),
#endif
					      0.0f,
					      0x007f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortSmoke\n" ) ;
    if ( !(work->dust2 = BRK_UTL_MakeRSPRTWH( SMOKE_N_PRIM,
#if 0
					      GV_StrCode( "w11c2_smoke2_alp" ),
					      SCE_GS_SET_ALPHA(2,0,0,1,0),
#else
					      GV_StrCode( "bombgas6_alp" ),
					      SCE_GS_SET_ALPHA(0,1,0,1,0),
#endif
					      0.0f,
					      0x007f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortSmoke\n" ) ;

    work->world = world ; 
    d = SMOKE_SPEED / sceVu0Sqrt( dir->vx*dir->vx + dir->vz*dir->vz ) ;
    dir->vx *= d ;
    dir->vz *= d ;
    _sceVu0CopyVector( &work->dir, dir ) ;
    _sceVu0CopyVector( &work->pos, pos ) ;
    for ( i=SMOKE_N_PRIM ; --i>=0 ; )
	work->vel[i].vw = (float)(i*32 + (irnd() & 15)) ;
    return 0 ;
}

void *NewFortSmoke( FMATRIX *world, FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, world, pos, dir, hzd ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
