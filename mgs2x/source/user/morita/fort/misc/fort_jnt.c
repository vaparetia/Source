//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_jnt.c 
   2点間のプリミティブ埋め

   2001/01/16 T.Morita
   $Id: fort_jnt.c,v 1.1.1.3 2002/11/19 11:46:17 Yoshizawa1 Exp $
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


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   pos[4] ;
    FMATRIX  *world1 ;   
    FMATRIX  *world2 ;   
    int       where  ;
    DG_PRIM2 *prim   ;
} Work ;

static void CalcNormalFromVertex( FVECTOR *norm, FVECTOR *v00, FVECTOR *v10, FVECTOR *v20 )
{
    FVECTOR a0, a1, v ;

    _sceVu0SubVector( &a0, v10, v00 ) ;
    _sceVu0SubVector( &a1, v20, v10 ) ;
    _sceVu0OuterProduct( &v, &a0, &a1 ) ;
    _sceVu0Normalize( norm, &v ) ;
    norm->vw = 0.0f ;
}

static void PosColor( FVECTOR *dir, IVECTOR *col, FMATRIX *lgt )
{
    FVECTOR c ;

    _sceVu0ApplyMatrix( &c, &lgt[0], dir ) ;
#if 0
    c.vx = c.vx>0.0f ? c.vx : -c.vx ;
    c.vy = c.vy>0.0f ? c.vy : -c.vy ;
    c.vz = c.vz>0.0f ? c.vz : -c.vz ;
#else
    c.vx = c.vx>0.0f ? c.vx : 0.0f ;
    c.vy = c.vy>0.0f ? c.vy : 0.0f ;
    c.vz = c.vz>0.0f ? c.vz : 0.0f ;
#endif
    c.vw = 1.0f ;

    _sceVu0ApplyMatrix( &c, &lgt[1], &c ) ;
    col->vx += (int)c.vx ;
    col->vy += (int)c.vy ;
    col->vz += (int)c.vz ;

    col->vx = col->vx>255 ? 255 : col->vx ;
    col->vy = col->vy>255 ? 255 : col->vy ;
    col->vz = col->vz>255 ? 255 : col->vz ;
}


static void Act( Work *work )
{
    FVECTOR        *pos ;
    DG_PRIM2_UVRGB *uvs ;
    DG_PRIM2 *p = work->prim ;
    FVECTOR   dir1, dir2 ;
    IVECTOR   col ;
    FMATRIX   lgt[2] ;

    DG_SwitchBuffPrim2( p ) ;
    pos = p->pos  [p->buffer_clock] ;
    uvs = p->uvrgb[p->buffer_clock] ;

    if ( work->world1 )
    {
	_sceVu0ApplyMatrix( pos+0, work->world1, &work->pos[0] ) ;
	_sceVu0ApplyMatrix( pos+1, work->world1, &work->pos[1] ) ;
    }    
    if ( work->world2 )
    {
	_sceVu0ApplyMatrix( pos+2, work->world2, &work->pos[2] ) ;
	_sceVu0ApplyMatrix( pos+3, work->world2, &work->pos[3] ) ;
    }

    CalcNormalFromVertex( &dir1, pos+0, pos+2, pos+1 ) ;
    _sceVu0ScaleVector( &dir2, &dir1, 1.0f ) ;

    DG_GetLightMatrix( pos, lgt ) ;
    col.vx = col.vy = col.vz = 0 ;
    PosColor( &dir1, &col, lgt ) ;
    PosColor( &dir2, &col, lgt ) ;
    uvs[0].r = uvs[1].r = col.vx ;
    uvs[0].g = uvs[1].g = col.vy ;
    uvs[0].b = uvs[1].b = col.vz ;
    uvs[0].a = uvs[1].a = 128    ;

//printf( "(%.3f,%.3f,%.3f)(%.3f,%.3f,%.3f)\t",dir1.vx, dir1.vy, dir1.vz,dir2.vx, dir2.vy, dir2.vz ) ;
//printf( "C%d %d %d\t",uvs[0].r,uvs[0].g,uvs[0].b ) ;

    DG_GetLightMatrix( pos+2, lgt ) ;
    col.vx = col.vy = col.vz = 0 ;
    PosColor( &dir1, &col, lgt ) ;
    PosColor( &dir2, &col, lgt ) ;
    uvs[2].r = uvs[3].r = col.vx ;
    uvs[2].g = uvs[3].g = col.vy ;
    uvs[2].b = uvs[3].b = col.vz ;
    uvs[2].a = uvs[3].a = 128    ;

//printf( "C%d %d %d\n",uvs[2].r,uvs[2].g,uvs[2].b ) ;

}

static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
}



#define _CopyVector(_p1,_p2,_p3,_p4) (_sceVu0CopyVector((_p1),(_p4)),\
				      (_p1)->vw = 1.0f,              \
				      _sceVu0CopyVector((_p2),(_p1)),\
				      _sceVu0CopyVector((_p3),(_p1)) )

static int GetResources( Work *work, int id, int where, 
			 FMATRIX *world1, FVECTOR *pos1,
			 FMATRIX *world2, FVECTOR *pos2 )
{
    FVECTOR *p0, *p1 ;

    if ( !(work->prim = BRK_UTL_MakePOLYFOG( 1, id,
					     SCE_GS_SET_ALPHA(0,1,0,1,0),
					     0x007f7f7f )) )
        PERROR( "No Prim(no memory) : NewFortJointParts\n" ) ;
    p0 = work->prim->pos[0] ;
    p1 = work->prim->pos[1] ;

    work->where  = where ;
    work->world1 = world1 ;
    work->world2 = world2 ;
    _CopyVector( &work->pos[0], p0+0,p1+0, pos1+0 ) ;
    _CopyVector( &work->pos[1], p0+1,p1+1, pos1+1 ) ;
    _CopyVector( &work->pos[2], p0+2,p1+2, pos2+0 ) ;
    _CopyVector( &work->pos[3], p0+3,p1+3, pos2+1 ) ;

    return 0 ;
}

void *NewFortJointParts( int id, int where,
			 FMATRIX *world1, FVECTOR *pos1,
			 FMATRIX *world2, FVECTOR *pos2  )
{
    Work *work ;

    work = (Work *)GV_NewActorPrio( GV_ACTOR_USER, sizeof(Work), 0x10 ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, id, where, world1, pos1, world2, pos2 ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
