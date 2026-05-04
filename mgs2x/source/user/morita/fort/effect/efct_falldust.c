//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_falldust.c
  フォーチュン戦 上から振る砂埃

  2001/02/08 T.Morita
  $Id: efct_falldust.c,v 1.1.1.3 2002/11/19 11:46:09 Yoshizawa1 Exp $
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

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define DUST_SIZE   500.0f
#define DUST_SPEED  50.0f
#define DUST_N_DUST 8
#define DUST_N_PRIM (DUST_N_DUST*2)
#define DUST_VEL_R  0.95f
#define DUST_GRAVITY2      4.0f
#define DUST_GRAVITY      1.0f
#define DUST_BOUNCE       1.08f
#define DUST_BOUNCE_WALL  1.08f

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   vel[DUST_N_DUST] ;
    float     gravity[DUST_N_DUST] ;
    short     tic[DUST_N_DUST] ;
    float     size   ;

    DG_PRIM2 *dust   ;
    int       n_dust ;
    int       latency ;
} Work ;



static void Act( Work *work )
{
    DG_PRIM2       *d ;
    FVECTOR        *nxt_p, *prv_p ;
    DG_PRIM2_UVRGB *nxt_u, *prv_u ;
    FVECTOR        *vel ;
    float          *gravity ;
    int             i, flag=0 ;
    short          *tic ;

    if ( work->latency > 0 )
    {
	work->latency-- ;
	return  ;
    }

    d     = work->dust ;
    prv_p = d->pos  [d->buffer_clock] ;
    prv_u = d->uvrgb[d->buffer_clock] ;
    DG_SwitchBuffPrim2( d ) ;
    nxt_p = d->pos  [d->buffer_clock] ;
    nxt_u = d->uvrgb[d->buffer_clock] ;
    vel     = work->vel     ;
    gravity = work->gravity ;
    tic     = work->tic     ;

    for ( i=work->n_dust ; --i>=0 ; )
    {
	if ( vel->vw > 0.0f )
	    vel->vw -= 1.0f ;
	else if ( (nxt_u[0].a = prv_u[0].a) )
	{
	    if ( ++*tic > 40 )
//		if ( !(*tic & (0x03 >> (i&1))) )
		if ( !(*tic & 0x07) )
		    nxt_u[0].a-- ;

	    vel->vy -= *gravity ;
	    if ( vel->vy < -50.0f )
		vel->vy = -50.0f ;
	    nxt_p[1].vx = nxt_p[0].vx = prv_p[0].vx + vel->vx * 0.1f  ;
	    nxt_p[1].vz = nxt_p[0].vz = prv_p[0].vz + vel->vz * 0.1f  ;
	    nxt_p[3].vx = nxt_p[2].vx = prv_p[2].vx - vel->vx * 0.1f  ;
	    nxt_p[3].vz = nxt_p[2].vz = prv_p[2].vz - vel->vz * 0.1f  ;
	    nxt_p[2].vy = nxt_p[0].vy = prv_p[0].vy + vel->vy         ;
	    nxt_p[3].vy = nxt_p[1].vy = prv_p[1].vy + vel->vy * 0.01f ;
	}

	flag += nxt_u[0].a + (int)vel->vw ;
	nxt_u[1].a = nxt_u[2].a = nxt_u[3].a = nxt_u[0].a ;

	nxt_p+=4, prv_p+=4 ;
	nxt_u+=4, prv_u+=4 ;
	vel++ ;
	tic++ ;
	gravity++ ;
    }

    if ( !flag )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->dust )
	GM_FreePrim2( work->dust ) ;
}

static int GetResources( Work *work, FVECTOR *pos, int n_dust, float size, int alpha,
			 float gravity, int latency )
{
    int     i ;
    FVECTOR p ;
    float   a ;

    work->size    = size    ;
    work->latency = latency ;
    work->n_dust = n_dust > DUST_N_DUST ? DUST_N_DUST : n_dust ;
    if ( !(work->dust = BRK_UTL_MakePOLY( work->n_dust,
					  GV_StrCode( "bombgas3_alp" ),
					  SCE_GS_SET_ALPHA(0,2,0,1,0),
					  (16<<24)|0x7f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortSideDust\n" ) ;

    for ( i=work->n_dust ; --i>=0 ; )
    {
	a = M_PI*frnd() ;

	p.vx = pos->vx + frnd() * DUST_N_DUST ;
	p.vy = pos->vy ;
	p.vz = pos->vz + frnd() * DUST_N_DUST ;
	p.vw = 1.0f ;
	work->vel[i].vx = 100.0f*cosf( a ) ;
	work->vel[i].vy = 0.0f ;
	work->vel[i].vz = 100.0f*sinf( a ) ;
	work->vel[i].vw = 0.0f ;
	work->gravity[i] = gravity + frnd()*0.1f ;

	_sceVu0AddVector( &work->dust->pos[0][i*4+0], &p, &work->vel[i] ) ;
	_sceVu0AddVector( &work->dust->pos[1][i*4+0], &p, &work->vel[i] ) ;
	_sceVu0AddVector( &work->dust->pos[0][i*4+1], &p, &work->vel[i] ) ;
	_sceVu0AddVector( &work->dust->pos[1][i*4+1], &p, &work->vel[i] ) ;
	_sceVu0SubVector( &work->dust->pos[0][i*4+2], &p, &work->vel[i] ) ;
	_sceVu0SubVector( &work->dust->pos[1][i*4+2], &p, &work->vel[i] ) ;
	_sceVu0SubVector( &work->dust->pos[0][i*4+3], &p, &work->vel[i] ) ;
	_sceVu0SubVector( &work->dust->pos[1][i*4+3], &p, &work->vel[i] ) ;
	work->vel[i].vw = (float)(irnd() & 7) ;
	work->tic[i] = 0 ;
    }

    return 0 ;
}

void *NewFortFallDust( FVECTOR *pos, int n_dust, float gravity, int latency )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, pos, n_dust, DUST_SIZE, 16, gravity, latency ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
