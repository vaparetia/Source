//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_elec.c
  フォーチュン戦 電撃びりびり（現在未使用）

  2001/02/08 T.Morita
  $Id: efct_elec.c,v 1.1.1.3 2002/11/19 11:46:09 Yoshizawa1 Exp $
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

#include "../../include/util.h"
#include "../../include/libdg_x.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"


#define ELEC_ALPHA    64
#define ELEC_N_LINE   32

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    float     pos[ELEC_N_LINE] ;
    u_int     idx[ELEC_N_LINE] ;

    DG_PRIM2 *prim ;
    FMATRIX  *world ;
    CV2_MDL  *c_mdl ;

    int       where ;
} Work ;



static int SearchVertex( Work *work, int src )
{
    int dst ;

    src >>= 16 ;
    dst = src + (irnd() & 3) + 1 ;
    if ( dst <= work->c_mdl->n_verts )
	dst -= work->c_mdl->n_verts ;
    dst <<= 16 ;

    return (src | dst) ;
}

static void Act( Work *work )
{
    FVECTOR        *prv_p, *nxt_p ;
    DG_PRIM2_UVRGB *prv_u, *nxt_u ;
    DG_PRIM2 *p = work->prim ;
    int       i ;
    FVECTOR   v ;

    prv_p = p->pos  [p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos  [p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;

    for ( i=ELEC_N_LINE ; --i>=0 ; )
    {
	nxt_u->a = prv_u->a ;

	if ( work->pos[i] > 1.0f )
	    if ( (work->pos[i] -= 1.0f) <= 1.0f )
		work->pos[i] = 1.0f, nxt_u->a = ELEC_ALPHA ;
	if ( nxt_u->a )
	{
	    FVECTOR *prv, *nxt ;

	    //nxt_u->a-- ;
	    if ( (work->pos[i] += 0.1f) >= 1.0f )
	    {
		work->pos[i] = 0.0f ;
		work->idx[i] = SearchVertex( work, work->idx[i] ) ;
	    }

	    prv = &work->c_mdl->verts[ work->idx[i]&0xffff ] ;
	    nxt = &work->c_mdl->verts[ work->idx[i] >> 16  ] ;

	    _sceVu0SubVector( &v, nxt, prv ) ;
	    _sceVu0ScaleVector( &v, &v, work->pos[i] ) ;
	    _sceVu0AddVector( &v, &v, prv ) ;
	    v.vw = 1.0f ;
	    _sceVu0ApplyMatrix( nxt_p, work->world, &v ) ;
	    _sceVu0CopyVector( nxt_p+1, prv_p ) ;
	}

	nxt_p += 2 ; prv_p += 2 ;
	nxt_u += 2 ; prv_u += 2 ;
    }
}

static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
}

static int GetResources( Work *work, DG_OBJS *objs, int cv2_id, int joint )
{
    int  i, idx ;
    CV2_DEF *c_def ;

    if ( !(c_def = (CV2_DEF*)GV_GetCache( GV_CacheID( cv2_id, 'c' ))) )
	PERROR( "Cant find CV2<%d> : NewFortBodyFlame\n", cv2_id ) ;
    if ( !(work->prim  = BRK_UTL_MakeLINE( ELEC_N_LINE,
					   0,
					   SCE_GS_SET_ALPHA(0,2,0,1,0),
					   0x004f7f7f,
					   0x004f7f7f )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;

    work->world = &objs->objs[joint].world ;
    work->c_mdl = &c_def->models[joint] ;

    for ( i=ELEC_N_LINE ; --i>=0 ; )
    {
	idx = irnd() % work->c_mdl->n_verts ;
	work->idx[i] = SearchVertex( work, idx << 16 ) ;
	work->pos[i] = i*3 ;
    }

    return 0 ;
}

void *NewFortElec( DG_OBJS *objs, int cv2_id, int joint )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, objs, cv2_id, joint ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
