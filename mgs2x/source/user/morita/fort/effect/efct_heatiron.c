//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_heatiron.c
  フォーチュン戦 熱く灼熱化した鉄の部分（貫通鉄箱）

  2001/02/08 T.Morita
  $Id: efct_heatiron.c,v 1.1.1.3 2002/11/19 11:46:10 Yoshizawa1 Exp $
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

#define HEAT_N_PRIM 9

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    DG_PRIM2 *heat  ;
    int       n_verts ;
} Work ;


static void Act( Work *work )
{
    DG_PRIM2       *d ;
    DG_PRIM2_UVRGB *nxt_u, *prv_u ;
    int             i, flag ;

    d = work->heat ;
    prv_u = d->uvrgb[d->buffer_clock] ;
    DG_SwitchBuffPrim2( d ) ;
    nxt_u = d->uvrgb[d->buffer_clock] ;

    flag = 0 ;
    for ( i=work->n_verts ; --i>=0 ; flag += nxt_u[i].a )
	if ( (nxt_u[i].a = prv_u[i].a) )
	    if ( !(GV_Time & 1) )
		nxt_u[i].a-- ;
    if ( !flag )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->heat )
	GM_FreePrim2( work->heat ) ;
}

DG_PRIM2* MakePOLYfromMDL( DG_MDLPACK *mdlpack, DG_OBJ *parent, DG_OBJ *current,
			   int tex_id, u_long64 alpha, int rgba )
{
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;
    DG_PRIM2 *p ;
    DG_TEX   *t ;
    int       i,  r,g,b,a ;
    short    *u ;
    SVECTOR  *v, *n ;
    FMATRIX  *world ;

    r = (rgba>> 0)&0xff ;
    g = (rgba>> 8)&0xff ;
    b = (rgba>>16)&0xff ;
    a = (rgba>>24)&0xff ;

    if ( !(p = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
			     1, mdlpack->n_verts )) )
	return NULL ;
    DG_ConfigPrim2Tex( p, t=DG_GetTexture( tex_id ) ) ;
    if ( alpha )
	DG_SetPrim2Alpha( p, alpha ) ;

    u0 = p->uvrgb[0], u1 = p->uvrgb[1] ;
    p0 = p->pos[0]  , p1 = p->pos[1]   ;

#ifdef PSX2 ///////////////////////Hottoki  T.Morita
    u = (short   *)mdlpack->uvs[0] ;
    v = (SVECTOR *)mdlpack->verts  ;
    n = (SVECTOR *)mdlpack->norms  ;
    for( i=mdlpack->n_verts ; --i>=0 ; ) {
		u0->u = u1->u = (short)((float)u[0]*t->u_scale + t->u_offset * 4096.0f ) ;
		u0->v = u1->v = (short)((float)u[1]*t->v_scale + t->v_offset * 4096.0f ) ;
		u0->q = u1->q = 4096 ;
		u0->f = u1->f = n->pad ;
		u0->r = u1->r = r ;
		u0->g = u1->g = g ;
		u0->b = u1->b = b ;
		u0->a = u1->a = a ;
		
		vu0_SV0toFV( v, p1 ) ;
		if ( v->pad >= 4095 ) {
			world = &current->world ;
		} else {
			_sceVu0AddVector( p1, p1, &current->trans ) ;
			world = &parent->world ;
		}
		p1->vw = 1.0f ;
		_sceVu0ApplyMatrix( p1, world , p1 ) ;
		_sceVu0CopyVector( p0, p1 ) ;

		v++, n++, u+=2 ;
		u0++, u1++, p0++, p1++ ;
    }
#else
	DG_InvisiblePrim2( p ) ;
#endif

    return p ;
}

static int GetResources( Work *work, DG_OBJS *objs )
{
    DG_MDL     *mdl  = objs->objs[2].model ;
    DG_MDLPACK *pack = mdl->packs+2 ;

    if ( !(work->heat = MakePOLYfromMDL( pack,
					 &objs->objs[mdl->parent],
					 &objs->objs[2],
					 GV_StrCode( "w11_ibx_danmen_burned_add" ),
					 SCE_GS_SET_ALPHA(0,2,0,1,0),
					 0x7f7f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortHeatIron\n" ) ;
    work->n_verts = pack->n_verts ;

    return 0 ;
}

void *NewFortHeatIron( DG_OBJS *objs )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, objs ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
