//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_lgt.c 
   オルガ ダイナミックハザード

   1999/12/26 T.Morita
   $Id: orga_hzx.c,v 1.1.1.3 2002/11/19 11:46:26 Yoshizawa1 Exp $
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

#include "../../include/util.h"


typedef struct lgtwork_t
{
    GV_ACT          actor  ;

    HZX_D_SEGMENT **segs   ;
    int             n_segs ;
    HZX_D_FLOOR   **flrs   ;
    int             n_flrs ;
} Work ;

void HZX_MoveDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *p1, IVECTOR *p2 ) ;
HZX_D_SEGMENT *HZX_AddDynamicSegment( HZX_GROUP_ID id, IVECTOR *p1,  IVECTOR *p2, u_int atr ) ;
void HZX_MoveDynamicFloor( HZX_D_FLOOR *flr, IVECTOR *p1, IVECTOR *p2, IVECTOR *p3, IVECTOR *p4 ) ;
HZX_D_FLOOR   *HZX_AddDynamicFloor( HZX_GROUP_ID id,
				    IVECTOR *p1, IVECTOR *p2, IVECTOR *p3, IVECTOR *p4,
				    int n, u_int atr ) ;
static void Die( Work *work )
{
    int i ;
    HZX_D_SEGMENT **s ;
    HZX_D_FLOOR   **f ;
    //void HZX_RemoveDynamicSegment( HZX_D_SEGMENT *seg ) ;
    //void HZX_RemoveDynamicFloor( HZX_D_FLOOR *flr ) ;

    for ( i=work->n_segs, s=work->segs ; --i>=0 ; )
	HZX_RemoveDynamicSegment( *s++ ) ;
    for ( i=work->n_flrs, f=work->flrs ; --i>=0 ; )
	HZX_RemoveDynamicFloor( *f++ ) ;
}

static void Act( Work *work )
{
}

static int GetResources( Work *work, int name, int where )
{
    int     i, j, buf[3] ;
    char   *c ;
    DG_DEF *def[30] ;
    HZX_D_SEGMENT **s ;
    HZX_D_FLOOR   **f ;

    if ( GCL_GetOption( 'm' ) )
	for ( i=0 ; (c=GCL_NextStr()) ; i++ )
	{
	    def[i] = GV_GetCache( GV_CacheID( GCL_GetInt( c ), 'k') ) ;
	    work->n_segs += 4 ;
	    work->n_flrs += 1 ;
	}
    else
	return -1 ;

    work->segs = s = GV_Malloc( sizeof(HZX_D_SEGMENT *) * work->n_segs ) ;
    work->flrs = f = GV_Malloc( sizeof(HZX_D_FLOOR   *) * work->n_flrs ) ;
    if ( GCL_GetOption( 'p' ) )
	for ( i=0 ; (c=GCL_NextStr()) ; i++ )
	{
	    FVECTOR t_size, t_pos ;
	    FVECTOR uv = { def[i]->ux, def[i]->uy, def[i]->uz, 0 } ;
	    FVECTOR lv = { def[i]->lx, def[i]->ly, def[i]->lz, 0 } ;
	    FMATRIX m =DG_UnitMatrix ;
	    static TARGET t[30] ;
	    static POWER_TARGET p[30] ;

	    GCL_GetIV( c, buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&m.m[W] ) ;

	    _sceVu0ApplyMatrix( &uv, &m, &uv ) ;
	    _sceVu0ApplyMatrix( &lv, &m, &lv ) ;
	    _sceVu0AddVector  ( &t_pos, &uv, &lv ) ;
	    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
	    _sceVu0AddVector  ( &t_pos, &t_pos, (FVECTOR *)&m.m[W] ) ;
	    t_size.vx = fpu_Abs(uv.vx - lv.vx)*0.5f ;
	    t_size.vy = fpu_Abs(uv.vy - lv.vy)*0.5f ;
	    t_size.vz = fpu_Abs(uv.vz - lv.vz)*0.5f ;
	    for ( j=4 ; --j>=0 ; )
	    {
		IVECTOR p1, p2 ;
		p1.vx = (int)(t_pos.vx + (j==1 ? -t_size.vx :  t_size.vx) ) ;
		p1.vz = (int)(t_pos.vz + (j==0 ? -t_size.vz :  t_size.vz) ) ;
		p1.vy = (int)(t_pos.vy - t_size.vy ) ;
		p1.vw = (int)(t_size.vy*2 ) ;
		p2.vx = (int)(t_pos.vx + (j==3 ?  t_size.vx : -t_size.vx) ) ;
		p2.vz = (int)(t_pos.vz + (j==2 ?  t_size.vz : -t_size.vz) ) ;
		p2.vy = (int)(t_pos.vy - t_size.vy ) ; 
		p2.vw = (int)(t_size.vy*2 ) ;
		*s++ =HZX_AddDynamicSegment( GM_GetHzxGroupID( where ), &p1, &p2, 0 ) ;
	    }
	    GM_SetTarget( &t[i], TARGET_DEFENSE|TARGET_POWER, where, BOTH_SIDE, &t_size, &t_pos ) ;
	    GM_SetPowerTarget( &t[i], &p[i], POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
	    GM_PutTarget( &t[i] ) ;
	    //NewTargetView( &t[i],  200, 50, 32 ) ;
	}

    return 0 ;
}


void *NewPutDynamicHzxBox( int name, int where )
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
