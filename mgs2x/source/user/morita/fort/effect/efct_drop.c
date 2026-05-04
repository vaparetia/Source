//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_drop.c
  フォーチュン戦 影出し

  2000/12/15 T.Morita
  $Id: efct_drop.c,v 1.1.1.3 2002/11/19 11:46:09 Yoshizawa1 Exp $
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

#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../../okajima/effect/lit_man.h"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;
#define N_SHADOW 4
typedef struct _Work
{
    GV_ACT_EX actor  ;

    DG_OBJS  *shadow[N_SHADOW] ;
    float     floor  ;
    FVECTOR **src    ;
    FVECTOR  *dst    ;

    FMATRIX  *world  ;
    FVECTOR   offset ;

    short       level ;
    short       mode  ;
} Work ;


/* 登録している光源の位置から最も近いものを取ってくる hang/fort_lgt_src.c */
extern void FRT_LGT_GetNearestLight( FVECTOR *center, FVECTOR ***src, FVECTOR **dst, float min ) ;
#define MAX_SRC_RAD 60000.0f



static void GetLightSrcDir( Work *work, FVECTOR *dir )
{
    FVECTOR *center = (FVECTOR *)work->world->m[W] ;

    if ( work->src )
	if ( *work->src == NULL )
	    FRT_LGT_GetNearestLight( center,
				     &work->src,
				     &work->dst,
				     MAX_SRC_RAD ) ;
    if ( work->src ? *work->src : 0 )
    {
	_sceVu0SubVector( dir, work->dst, *work->src ) ;
	_sceVu0Normalize( dir, dir ) ;
    }
    else
    {
	dir->vx = dir->vz = 0.0f ;
	dir->vy = 1.0f ;
    }
    dir->vw = 1.0f ;
}

static void Act( Work *work )
{
    FMATRIX drop_shadow ;
    FVECTOR v ;
    int i ;

    GetLightSrcDir( work, &v ) ;
    _sceVu0DropShadowMatrix( &drop_shadow, &v, 0.0f, 1.0f, 0.0f, 0/*平行光源*/ ) ;

    for( i=N_SHADOW ; --i>=0 ; )
	if ( work->shadow[i] )
	{
	    _sceVu0MulMatrix( &work->shadow[i]->world, &drop_shadow, work->world ) ;
	    _sceVu0ApplyMatrix( &v, &drop_shadow, &work->offset ) ;
	    _sceVu0AddVector( (FVECTOR *)work->shadow[i]->world.m[W],
			      (FVECTOR *)work->world->m[W],
			      &v ) ;

	    if ( work->mode == ':' )
		work->shadow[i]->world.m[W][Y] = work->floor + work->level ;
	    else
		work->shadow[i]->world.m[W][Y] = work->floor + i ;
	}
}

static void Die( Work *work )
{
    int i ;

    for ( i=N_SHADOW ; --i>=0 ; )
	if ( work->shadow[i] )
	    DG_DequeueObjs( work->shadow[i] ), DG_FreeObjs( work->shadow[i] ) ;
}

static int GetResources( Work *work,
			 DG_OBJS *objs, float floor,
			 char *shadow_name, int level )
{
    DG_DEF *def ;
    char  shadow[128], *c ;
    char *lvl[5] ;
    int   i ;
    int   n_lvl, mode  ;

    work->floor = floor + 1.0f ;
    work->world = &objs->world ;

    work->offset.vx = 0.0f ;
    work->offset.vy = -objs->def->models->ly ;
    work->offset.vz = 0.0f ;
    work->offset.vw = 0.0f ;

    FRT_LGT_GetNearestLight( (FVECTOR *)work->world->m[W],
			     &work->src,
			     &work->dst,
			     MAX_SRC_RAD ) ;

    // BP - WAS strcpy
    strncpy( shadow, shadow_name, sizeof( shadow ) );
    shadow[ sizeof( shadow ) - 1 ] = 0;

    lvl[n_lvl=0] = shadow ;
    mode = 0 ;
    for( c=shadow ; *c ; c++ )
	if ( *c == ':' || *c == ',' )
	    mode=*c, *c='\0', lvl[++n_lvl] = c+1 ;

    for( i=n_lvl ; i>=0 ; i-- )
    {
	if ( mode != ':' && i!=level )
	    continue ;
	if ( !(def = GV_GetCache( GV_CacheID( GV_StrCode( lvl[i] ), 'k' ) )) )
	    PERROR( "No Shadow KMS<%s> in data.cnf!! : NewFortDropShadow\n", lvl[i] ) ;
	if ( !(work->shadow[i] = DG_MakeObjs( def,
					      DG_FLAG_PAINT|DG_FLAG_ONEPIECE,
					      0 )) )
	    PERROR( "Can't make DG_OBJS(May be no memory) : NewFortDropShadow\n" ) ;
	DG_QueueObjs( work->shadow[i] ) ;
	DG_MakePreshade( work->shadow[i], GM_GetMap( GM_CurrentMap )->light ) ;
    }

    work->mode  = mode  ;
    work->level = level ;

    return 0 ;
}

void *NewFortDropShadow( DG_OBJS *objs, float floor,
			 char *shadow_name, int level )
{
    Work *work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, objs, floor, shadow_name, level ) < 0 ||
	     !objs )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return ( void * )work ;
}
