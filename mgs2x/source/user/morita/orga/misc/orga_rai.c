//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_rai.c 
   オルガ スポット雨

   2000/01/11 T.Morita
   $Id: orga_rai.c,v 1.1.1.3 2002/11/19 11:46:27 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define ORGA_RAI_VEL 400
#define ORGA_RAI_N_PACKS 64
#define ORGA_RAI_N_VERTS 2

#define ORGA_RAI_MAX_ALPH 128

#define ORG_RAI_PRIM_FLG  (DG_PRIM2_LINE        |\
			   DG_PRIM2_SHADE       |\
			   DG_PRIM2_TEX         |\
			   DG_PRIM2_ANTIALIASING|\
			   DG_PRIM2_ALPHA)

typedef	struct work_rai_t Work ;
struct work_rai_t
{
    GV_ACT    actor  ;
    DG_PRIM2 *rain   ;  /* 雨プリム */
    int       name   ;

    u_int     rgba   ;  /* スポットライトの色         */
    FVECTOR   dir    ;  /* スポットライトの向き       */
    FVECTOR   pos    ;  /* スポットライトの位置       */
    float     radius ;  /* スポットライトの有効範囲   */
    float     ratio  ;  /* スポットライトのコーン角度 */
} ;

static void Die( Work *work )
{
    GM_FreePrim2( work->rain ) ;
}

static inline float CheckInsideCorn( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
#ifndef BP_PSX2_ASM
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
#else
    FVECTOR v, l ;

    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    lqc2        vf3, 0(%2)

    vsub.xyz    vf1,vf2,vf1   /* _sceVu0SubVector( vf1, p, lp )      */

    vopmula.xyz ACC,vf1,vf3   /* _sceVu0OuterProduct( vf2, vf1, ld ) */
    vopmsub.xyz vf2,vf3,vf1

    vopmula.xyz ACC,vf2,vf3   /* _sceVu0OuterProduct( vf2, vf2, ld ) */
    vopmsub.xyz vf2,vf3,vf2

    sqc2        vf2, 0(%3)
    sqc2        vf1, 0(%4)
    " : : "r"(lp), "r"(p), "r"(ld), "r"(&l), "r"(&v) : "memory" ) ;

    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
#endif

}

#if 0
static void Act( Work *work )
{
    int   i ;
    float d, f ;
    FVECTOR  *cur, v, wind ;
    DG_PRIM2 *prim = work->rain ;
    DG_PRIM2_UVRGB *u ;
    extern  FVECTOR G_wind;
    int   r, g, b, a ;
    GV_MSG  *msg  ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        if ( !msg->message[0] )
        {
	    GV_DestroyActor( work ) ;
            return ;
        }

    _sceVu0ScaleVector( &wind, &G_wind, 1.0f ) ;

    r =  (work->rgba >>  0) & 0xff ;
    g =  (work->rgba >>  8) & 0xff ;
    b =  (work->rgba >> 16) & 0xff ;
    a =  (work->rgba >> 24) & 0xff ;

    DG_SwitchBuffPrim2( prim ) ;
    cur = prim->pos[prim->buffer_clock  ] ;
    u   = prim->uvrgb[prim->buffer_clock] ;
    for ( i=ORGA_RAI_N_PACKS ; --i>=0 ; cur+=2, u+=2 )
    {
	*(cur+1) = *(cur+0) ;
	_sceVu0AddVector( cur, cur, &G_wind ) ;
	cur->vy -= ORGA_RAI_VEL ; 
	_sceVu0SubVector( &v, cur, &work->pos ) ;
	d = sceVu0Sqrt( v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ) ;
	f = CheckInsideCorn( &work->pos, &work->dir, cur+1 ) ;
	if ( f < d*work->ratio )
	{
	    //f = CheckInsideCorn( &work->pos, &work->dir, cur+0 ) ;
	    //if ( f < d*work->ratio )
	    {
		u->a = (work->radius*0.5f   > d ? 1 : work->radius*0.5f/d  ) * a    ;
		u->a = (work->radius*0.5f*d > f ? 1 : work->radius*0.5f*d/f) * u->a ;
	    }
	}
	else
	{
	    u->a = 0 ;
	    //if ( (cur+0)->vy < work->dir.m[W][Y] )
	    {
		d = work->radius * rnd() ;

		(cur+0)->vx = (cur+1)->vx = work->dir.vx * d + work->pos.vx +
		    work->dir.vz * d * work->ratio * frnd() ;
		(cur+0)->vy = (cur+1)->vy = work->ratio  * d + work->pos.vy +
		    500*frnd() ;
		(cur+0)->vz = (cur+1)->vz = work->dir.vz * d + work->pos.vz -
		    work->dir.vx * d * work->ratio * frnd() ;
	    }
	}
    }
}
#else
static void Act( Work *work )
{
    int   i, a ;
    float d, f ;
    FVECTOR  *cur, v, wind ;
    DG_PRIM2 *prim = work->rain ;
    DG_PRIM2_UVRGB *uvs ;
    extern  FVECTOR G_wind;
    GV_MSG  *msg  ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        if ( !msg->message[0] )
        {
	    GV_DestroyActor( work ) ;
            return ;
        }

    _sceVu0CopyVector( &wind, &G_wind ) ;
    wind.vy -= ORGA_RAI_VEL ; 

    DG_SwitchBuffPrim2( prim ) ;
    cur = prim->pos[prim->buffer_clock  ] ;
    uvs = prim->uvrgb[prim->buffer_clock] ;
    for ( i=ORGA_RAI_N_PACKS ; --i>=0 ; cur+=2, uvs+=2 )
    {
	_sceVu0CopyVector( cur+1, cur ) ;
	_sceVu0AddVector( cur, cur, &wind ) ;
	_sceVu0SubVector( &v, cur, &work->pos ) ;
	d = sceVu0Sqrt( v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ) ;
	f = CheckInsideCorn( &work->pos, &work->dir, cur+1 ) ;
	if ( f < d*work->ratio )
	{
	    a = (int)( (ORGA_RAI_MAX_ALPH*1000.0f / d) * (500.0f / f) ) ;
	    (uvs+1)->a = (uvs+0)->a = (a>ORGA_RAI_MAX_ALPH ? ORGA_RAI_MAX_ALPH : a) ;
	}
	else
	{
	    d = work->radius * rnd() ;
	    f = rnd() ;
	    (uvs+1)->a = 0 ;
	    (uvs+0)->a = (int)(ORGA_RAI_MAX_ALPH*1000.0f / d) ;
#if 1
	    (cur+1)->vx = work->pos.vx + work->dir.vx*d +work->dir.vz*d * work->ratio*frnd() ;
	    (cur+1)->vy = work->pos.vy + work->ratio *d + 500*frnd() ;
	    (cur+1)->vz = work->pos.vz + work->dir.vz*d -work->dir.vx*d * work->ratio*frnd() ;
#else
	    _sceVu0ScaleVector( cur+1, &work->dir, d ) ;
	    _sceVu0AddVector( cur+1, cur+1, &work->pos ) ;
	    d *= work->ratio ;
	    (cur+1)->vx += d * (1.0f-f) ;
	    (cur+1)->vy += d *       f  * (1.0-work->dir.vy) ;
	    (cur+1)->vz += d *       f  *      work->dir.vy  ;
#endif
	    _sceVu0ScaleVector( cur, &wind, f ) ;
	    _sceVu0AddVector( cur, cur, cur+1 ) ;
	}
    }
}


#endif

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

static void InitLinePrim( DG_PRIM2 *prim, int id, int i, int n_verts, u_int rgba )
{
    int j ;
    DG_TEX *t ;
    DG_PRIM2_UVRGB *u0 = prim->uvrgb[0], *u1 = prim->uvrgb[1] ;

    DG_ConfigPrim2Tex( prim, t=DG_GetTexture( id ) ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA(0,2,0,1,0) ) ;
    for ( ; --i>=0 ; )
	for ( j=0 ; j<n_verts ; j++, u0++, u1++ )
	{
	    u0->f = u1->f = j ? 0x0fff : 0x8fff ;
	    u0->u = u1->u = FTOI12( (j ? 0.0f:1.0f) * t->u_scale + t->u_offset ) ;
	    u0->v = u1->v = FTOI12( (j ? 0.0f:1.0f) * t->v_scale + t->v_offset ) ;
	    u0->q = u1->q = 4096 ;
	    u0->r = u1->r = ((rgba >>  0) & 0xff) ;
	    u0->g = u1->g = ((rgba >>  8) & 0xff) ;
	    u0->b = u1->b = ((rgba >> 16) & 0xff) ;
	    u0->a = u1->a = ((rgba >> 24) & 0xff)*j/n_verts ;
	}
}

static int GetResources( Work *work, int name, int where )
{
    int     i, buf[3] ;
    FVECTOR *p0, *p1 ;

    if ( GCL_GetOption( 'f' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
    }
    if ( GCL_GetOption( 't' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->dir ) ;
    }
    _sceVu0SubVector( &work->dir, &work->dir, &work->pos ) ;
    _sceVu0Normalize( &work->dir, &work->dir ) ;

    work->rgba   = 0x40ffffa0 ;
    work->name   = name       ;
    work->ratio  = (float)(GCL_GetOptionValue( 'r', 50   )/100.0f) ;
    work->radius = (float)(GCL_GetOptionValue( 's', 6000 )       ) ;

    if ( !(work->rain = GM_MakePrim2( ORG_RAI_PRIM_FLG, ORGA_RAI_N_PACKS, ORGA_RAI_N_VERTS ) ) )
	return -1 ;
    InitLinePrim( work->rain, GV_StrCode("rain01_msk"), ORGA_RAI_N_PACKS, ORGA_RAI_N_VERTS, work->rgba ) ;

    p0 = work->rain->pos[0] ;
    p1 = work->rain->pos[1] ;
    for ( i=ORGA_RAI_N_PACKS ; --i>=0 ; p1+=2, p0+=2 )
    {
	float d = work->radius * rnd() ;
	p0->vx = (p0+1)->vx = p1->vx = (p1+1)->vx = work->dir.vx * d + work->pos.vx + work->dir.vz * d * work->ratio * frnd() ;
	p0->vy = (p0+1)->vy = p1->vy = (p1+1)->vy = work->ratio  * d + work->pos.vy + 500*frnd() ;
	p0->vz = (p0+1)->vz = p1->vz = (p1+1)->vz = work->dir.vz * d + work->pos.vz - work->dir.vx * d * work->ratio * frnd() ;
    }
    return 0 ;
}

void *NewSpotRain( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
