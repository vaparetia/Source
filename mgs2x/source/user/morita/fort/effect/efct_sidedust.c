//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_sidedust.c
  フォーチュン戦 横に出る埃

  2001/02/08 T.Morita
  $Id: efct_sidedust.c,v 1.1.1.3 2002/11/19 11:46:11 Yoshizawa1 Exp $
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


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define DUST_SIZE   500.0f
#define DUST_SPEED  50.0f
#define DUST_N_DUST 8
#define DUST_N_PRIM (DUST_N_DUST*2)
#define DUST_VEL_R  0.95f
#define DUST_GRAVITY      4.0f
#define DUST_BOUNCE       1.08f
#define DUST_BOUNCE_WALL  1.08f

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   vel[DUST_N_DUST] ;
    short     tic[DUST_N_DUST] ;
    float     size   ;

    DG_PRIM2 *dust   ;
    int       n_dust ;
    int       alpha  ;

    HZD_BOX  *hzd    ;
} Work ;




static void Act( Work *work )
{
    static FVECTOR bounce = { DUST_BOUNCE_WALL, DUST_BOUNCE, DUST_BOUNCE_WALL, 0 } ;
    FVECTOR  size = { work->size, work->size, work->size, 0 } ;
    FVECTOR  ofst = { work->size/10.0f, work->size/10.0f, work->size/10.0f, 0.0f }  ;
    DG_PRIM2         *d ;
    FVECTOR          *nxt_p, *prv_p ;
    DG_PRIM2_UVRGBWH *nxt_u, *prv_u ;
    FVECTOR          *vel ;
    int     i, flag=0 ;
    short  *tic ;

    d     = work->dust ;
    prv_p = d->pos  [d->buffer_clock] ;
    prv_u = d->uvrgb[d->buffer_clock] ;
    DG_SwitchBuffPrim2( d ) ;
    nxt_p = d->pos  [d->buffer_clock] ;
    nxt_u = d->uvrgb[d->buffer_clock] ;
    vel   = work->vel ;
    tic   = work->tic ;

    for ( i=work->n_dust ; --i>=0 ; )
    {
	nxt_u[0].a = prv_u[0].a ;
	nxt_u[1].a = prv_u[1].a ;
	if ( vel->vw > 0.0f )
	    vel->vw -= 1.0f ;
	else if ( nxt_u[0].a || nxt_u[1].a )
	{
	    float a, r ;

	    if ( ++*tic > 30 )
	    {
		
		if ( !( (*tic + 0 ) & (((1<<(work->alpha - nxt_u[0].a))-1) & 0xf)) )
		    nxt_u[0].a-- ;
		if ( !( (*tic + 20) & (((1<<(work->alpha - nxt_u[1].a))-1) & 0xf)) )
		    nxt_u[1].a-- ;
	    }

#define ANGtoRAD(_a) ((_a)*(float)M_PI/128.0f)
#if 0
//#define RADIUS(_d)   (work->size*((d) - DUST_SPEED)/DUST_SPEED)
	    d = sceVu0Sqrt( vel->vx*vel->vx + vel->vz*vel->vz ) ;
	    vel->vy  = RADIUS( d )/2 ;
	    d = sceVu0Sqrt( vel->vx*vel->vx + vel->vz*vel->vz ) ;
	    vel->vy -= RADIUS( d )/2 ;
	    vel->vy *= DUST_VEL_R ;
#endif

	    r = work->size * (*tic>9 ? 10 : *tic+1) / 10.0f ;
	    a = ANGtoRAD( *tic & 0xff ) - r*0.05f ;

	    vel->vx *= DUST_VEL_R ;
	    vel->vz *= DUST_VEL_R ;
	    vel->vy  = *tic>9 ? DUST_GRAVITY : r*0.01f ;
	    //if ( vel->vy < work->size*0.1f )
		//vel->vy = work->size*0.1f ;

	    nxt_u[0].w = (short)(r * sinf( a + i)) ;
	    nxt_u[0].h = (short)(r * cosf( a + i)) ;
	    nxt_u[1].w = (short)(r * sinf(-a + i)) ;
	    nxt_u[1].h = (short)(r * cosf(-a + i)) ;

	    _sceVu0CopyVector( &nxt_p[0], &prv_p[0] ) ;
	    if ( work->hzd ? !BRK_CheckHazard( work->hzd, &nxt_p[0], vel, &bounce, &size ) : 1 )
		_sceVu0AddVector( &nxt_p[0], &nxt_p[0], vel ) ;
	    _sceVu0AddVector( &nxt_p[1], &nxt_p[0], &ofst ) ;
	}

	flag += (int)(nxt_u[0].a + prv_u[0].a + vel->vw) ;
	nxt_p+=2, prv_p+=2 ;
	nxt_u+=2, prv_u+=2 ;
	vel++ ;
	tic++ ;
    }

    if ( !flag )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->dust )
	GM_FreePrim2( work->dust ) ;
}

static int GetResources( Work *work, FVECTOR *pos, FVECTOR *dir, int n_dust, HZD_BOX *hzd, float size, int alpha )
{
    int     i ;
    float   d, scale ;
    FVECTOR dust_pos ;

    work->hzd    = hzd   ;
    work->size   = size  ;
    work->alpha  = alpha ;
    work->n_dust = n_dust > DUST_N_DUST ? DUST_N_DUST : n_dust ;
    if ( !(work->dust = BRK_UTL_MakeRSPRTWH( work->n_dust*2,
					     GV_StrCode( "bombgas6_alp" ),
					     SCE_GS_SET_ALPHA(0,1,0,1,0),
					     0.0f,
					     (alpha<<24)|0x7f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortSideDust\n" ) ;

    scale = work->n_dust/2 - 0.5f*(1 - (work->n_dust & 1)) ;
    d = DUST_SPEED / sceVu0Sqrt( dir->vx*dir->vx + dir->vz*dir->vz ) ;
    for ( i=work->n_dust ; --i>=0 ; )
    {
	_sceVu0CopyVector( &dust_pos, pos ) ;
	dust_pos.vx += 3*d*dir->vz * (scale - i) ;
	dust_pos.vz -= 3*d*dir->vx * (scale - i) ;
	_sceVu0CopyVector( &work->dust->pos[0][i*2+0], &dust_pos ) ;
	_sceVu0CopyVector( &work->dust->pos[1][i*2+0], &dust_pos ) ;
	_sceVu0CopyVector( &work->dust->pos[0][i*2+1], &dust_pos ) ;
	_sceVu0CopyVector( &work->dust->pos[1][i*2+1], &dust_pos ) ;
	_sceVu0CopyVector( &work->vel[i], dir ) ;
	work->vel[i].vx = d*dir->vx + frnd()*(DUST_SPEED * 0.1f) + 0.05f*(scale - i)*d*dir->vz ;
	work->vel[i].vy =   dir->vy + frnd()*(DUST_SPEED * 0.1f) ;
	work->vel[i].vz = d*dir->vz + frnd()*(DUST_SPEED * 0.1f) - 0.05f*(scale - i)*d*dir->vx ;
	work->vel[i].vw = (float)(irnd() & 7) ;
	work->tic[i] = 0 ;
    }
    return 0 ;
}

void *NewFortSideDust( FVECTOR *pos, FVECTOR *dir, int n_dust )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, pos, dir, n_dust, NULL, DUST_SIZE, 8 ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
void *NewFortSideDustSize( FVECTOR *pos, FVECTOR *dir, int n_dust, float size )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, pos, dir, n_dust, NULL, size, 16 ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
