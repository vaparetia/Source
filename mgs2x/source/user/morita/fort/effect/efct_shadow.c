//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   efct_shadow.c
   フォーチュン戦 影出し

   2000/12/15 T.Morita
   $Id: efct_shadow.c,v 1.1.1.3 2002/11/19 11:46:10 Yoshizawa1 Exp $
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
#include "../../include/util.h"
#include "../../include/libdg_x.h"
#include "../../brk_utl/brk_utl.x"

#define BRK_N_SHADOW 2

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_brk_t
{
    GV_ACT_EX    actor ;

    FVECTOR      ofst ;
    FVECTOR      size ;

    DG_PRIM2    *prim ;
    DG_OBJS     *objs ;
} BrkWork ;

extern FVECTOR  *FRT_MiscBulletPos() ;

static void Die( BrkWork *work )
{
    if ( work->prim )
	DG_FreePrim2( work->prim ) ;
}

static void CalcShape( BrkWork *work, FMATRIX *world, FVECTOR *shape, int i )
{
    while( --i>=0 )
    {
	shape[i].vx =  work->size.vx * (i&2 ? 1.0f : -1.0f) ;
	shape[i].vy =  work->size.vy * (i&4 ? 1.0f : -1.0f) ;
	shape[i].vz =  work->size.vz * (i&1 ? 1.0f : -1.0f) ;
	_sceVu0AddVector( &shape[i], &shape[i], &work->ofst ) ;
	shape[i].vw = 1.0f ;
	_sceVu0ApplyMatrix( &shape[i], world, &shape[i] ) ;
    }
}
static void SelectShadow( FVECTOR *bullet, FVECTOR *shape, int *fst, int *sec )
{
    int i, j ;
    float t, min=20000000.0f ;
    FVECTOR s, v ;

    for ( i=4 ; --i>=0 ; )
    {
	_sceVu0SubVector( &v, &shape[i], bullet ) ;
	_sceVu0Normalize( &v, &v ) ;
	for ( j=i ; --j>=0 ; )
	{
	    _sceVu0SubVector( &s, &shape[j], bullet ) ;
	    _sceVu0Normalize( &s, &s ) ;
	    t = _sceVu0InnerProduct( &s, &v ) ;
	    if ( t > min )
		min=t, *fst=i, *sec=j ;
	}
    }
}
static void CalcShadow( FVECTOR *bullet, FVECTOR *shape, FVECTOR *pos, DG_PRIM2_UVRGB *uvs )
{
    FVECTOR v ;
    float   d ;

    /* 1点めを計算 */
    _sceVu0CopyVector( pos, shape ) ;

    /* 距離によりαを決める */
    _sceVu0SubVector( &v, pos, bullet ) ;
    d = 128.0f * 3000.0f*3000.0f / (v.vx*v.vx + v.vz*v.vz) ;
    d = d>128.0f ? 128.0f : d ;
    (uvs+0)->a = (int)d ;
    (uvs+2)->a = 0 ;

    /* 2点めを計算 */
    _sceVu0Normalize( &v, &v ) ;
    _sceVu0ScaleVector( &v, &v, d*10.0f ) ;
    _sceVu0AddVector( pos+2, pos, &v ) ;
}

static void Act( BrkWork *work )
{
    if ( !work->objs )
	GV_DestroyActor( work ) ;
    else
    {
	FVECTOR         v   ;
	FMATRIX        *world = &work->objs->world ;
	FVECTOR        *pos ;
	DG_PRIM2_UVRGB *uvs ;
	FVECTOR         shape[8] ;
	FVECTOR        *bullet ;
	int i ;

	DG_SwitchBuffPrim2( work->prim ) ;
	pos = work->prim->pos  [ work->prim->buffer_clock ] ;
	uvs = work->prim->uvrgb[ work->prim->buffer_clock ] ;

	if ( (bullet = FRT_MiscBulletPos()) )
	{
	    _sceVu0SubVector( &v, (FVECTOR *)world->m[W], bullet ) ;
	    if ( (v.vx*v.vx + v.vz*v.vz) <= 3500.0f*3500.0f )
	    {
		int fst, sec ;

		CalcShape( work, world, shape, 8 ) ;
		SelectShadow( bullet, shape, &fst, &sec ) ;
		CalcShadow( bullet, &shape[fst  ], pos  , uvs   ) ; /* １つめ上段 */
		CalcShadow( bullet, &shape[fst+4], pos+1, uvs+1 ) ; /* １つめ下段 */
		CalcShadow( bullet, &shape[sec  ], pos+4, uvs+4 ) ; /* ２つめ上段 */
		CalcShadow( bullet, &shape[sec+4], pos+5, uvs+5 ) ; /* ２つめ下段 */
		return ;
	    }
	}
	for ( i=BRK_N_SHADOW*2 ; --i>=0 ; )
	    uvs[i].a = 0 ;
    }
}

static int GetResources( BrkWork *work, DG_OBJS *box )
{
    FVECTOR  ofst={ box->def->ux + box->def->lx,
		    box->def->uy + box->def->ly,
		    box->def->uz + box->def->lz } ;
    FVECTOR  size={ box->def->ux - box->def->lx,
		    box->def->uy - box->def->ly,
		    box->def->uz - box->def->lz } ;

    _sceVu0ScaleVector( &work->ofst, &ofst, 0.5f ) ;
    _sceVu0ScaleVector( &work->size, &size, 0.5f ) ;
    if ( !(work->prim = BRK_UTL_MakePOLY( BRK_N_SHADOW,
                                          GV_StrCode( "splash03_alp" ),
					  SCE_GS_SET_ALPHA(2,1,0,1,0),
                                          0x007f7f7f )) )
        PERROR( "Cannot make PRIM2 maybe no memory : NewFortObjectShadow\n" ) ;

    return 0 ;
}

void *NewFortObjectShadow( DG_OBJS *box )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_AFTER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if( GetResources( work, box ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
