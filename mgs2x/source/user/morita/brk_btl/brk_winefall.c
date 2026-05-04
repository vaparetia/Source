//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_wine_drip.c
   瓶壊れ

   1999/12/02 T. Morita
   $Id: brk_winefall.c,v 1.1.1.3 2002/11/19 11:45:24 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"
// for DG-LIB
#include "../include/libdg_x.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define BRK_N_DRIP          4
#define BRK_N_STREAM        6
#define BRK_N_SPILL         4+1

#define BRK_DRIP_GRAVITY    1.0f
#define BRK_STREAM_GRAVITY  4

#define BRK_WIN_DRIPSTART   200
#define BRK_WIN_VANISHSTART 30

#define BRK_BTL_WINE_SPD    0.98f


typedef struct work_t
{
    GV_ACT    actor  ;

    FMATRIX   world  ;
    FMATRIX   lights[2] ;

    DG_PRIM2 *drip   ;
    DG_PRIM2 *spill  ;
    DG_COMDL *stream ;

    float     sx, sy, sa ;
    float     floor  ;
} Work ;



static void Die( Work *work )
{
    if ( work->drip )
	GM_FreePrim2( work->drip   ) ;
    if ( work->stream )
	DG_DequeueComdlObjs( work->stream ), DG_FreeComdl( work->stream ) ;
    if ( work->spill )
	GM_FreePrim2( work->spill ) ;
}

static void Act( Work *work )
{
    int     i    ;
    int     flag = 0 ;
    FMATRIX *mtx ;
    extern void *NewCrushDust( FVECTOR *, FVECTOR *, int ) ;/*取り敢えずこれを呼んでおく*/

    if ( work->sx > 0.0f )
    {
	if ( work->sy*1000.0f < work->floor )
	    work->sy += work->sa += 0.0125f ;
	else
	{
	    DG_VisiblePrim2( work->spill ) ;
	    if ( work->sx > 0.3f )
	    {
		mtx = &work->spill->as_world ;
		_sceVu0CopyVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&work->world.m[W] ) ;
		mtx->m[W][Y] -= work->floor ;
		if ( (GV_Time&0x3) == 0 )
		    NewCrushDust( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[Y], 2 ) ;
		_sceVu0ScaleVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[W],
				    1.0f/(work->spill->pos[0]->vw += 0.3f) ) ;
	    }
	    if ( (work->sx *= 0.95f) < 0.001f )
	    {
		DG_DequeueComdlObjs( work->stream ) ;
		DG_FreeComdl( work->stream ) ;
		work->stream = NULL ;
		work->sx = 0.0f ;
	    }
	}
	if ( work->stream )
	{
	    mtx = &work->stream->pos[0].world ;
	    GV_ZeroMemory( mtx, sizeof(FMATRIX) ) ;
	    mtx->m[Y][Y] = work->sy ;
	    mtx->m[X][X] = mtx->m[Z][Z] = work->sx ;
	    mtx->m[W][W] = 10.0f ;
	    _sceVu0MulMatrix( mtx, &work->world, mtx ) ;
	}
    }
    else
    {
	FVECTOR        *p = work->drip->pos[0]   ;
	DG_PRIM2_UVRGB *u = work->drip->uvrgb[0] ;

	flag = 1 ;
	for ( i=BRK_N_DRIP ; --i>=0 ; u+=2, p+=2 )
	    if ( (p+1)->vw > 0 )
	    {
		flag = 0 ;
		if ( ((p+1)->vw -= 1.0f) < BRK_WIN_DRIPSTART )
		{
		    (p+1)->vy  = p->vy ;
		    (p+0)->vy -= p->vw += BRK_DRIP_GRAVITY ;
		    u->a = DG_FTOI( (p+1)->vw );
		}
	    }
    }

    {
	FVECTOR  *v ;

        /* 優先防止用頂点をカメラに向けて進ませる  */
        v = work->spill->pos[0] ;
	v[4].vx = 50000.0f * DG_Chanls->eye.m[Z][X] + work->world.m[W][X] ;
	v[4].vz = 50000.0f * DG_Chanls->eye.m[Z][Z] + work->world.m[W][Z] ;
    }
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

static void InitSpillPrim( DG_PRIM2 *prim, int id, u_int rgba )
{
    DG_TEX *t ;
    DG_PRIM2_UVRGB *u = prim->uvrgb[0] ;

    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    DG_ConfigPrim2Tex( prim, t = DG_GetTexture( id ) ) ;
    u[0].u = u[2].u = FTOI12( 0.0F * t->u_scale + t->u_offset ) ;
    u[1].u = u[3].u = FTOI12( 1.0F * t->u_scale + t->u_offset ) ;
    u[0].v = u[1].v = FTOI12( 0.0F * t->v_scale + t->v_offset ) ;
    u[2].v = u[3].v = FTOI12( 1.0F * t->v_scale + t->v_offset ) ;
    u[0].q = u[1].q = u[2].q = u[3].q =  4096 ;
    u[0].f = u[1].f = u[4].f = 0x8fff ;
    u[2].f = u[3].f = 0x0fff ;
    u[0].r = u[1].r = u[2].r = u[3].r = (rgba >>  0) & 0xff ;
    u[0].g = u[1].g = u[2].g = u[3].g = (rgba >>  8) & 0xff ;
    u[0].b = u[1].b = u[2].b = u[3].b = (rgba >> 16) & 0xff ;
    u[0].a = u[1].a = u[2].a = u[3].a = (rgba >> 24) & 0xff ;
}

static void InitLinePrim( DG_PRIM2 *prim, int i, int n_verts, u_int rgba )
{
    int j ;
    DG_PRIM2_UVRGB *u = prim->uvrgb[0] ;

    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    for ( ; --i>=0 ; )
        for ( j=0 ; j<n_verts ; j++, u++ )
        {
            u->f = j ? 0x0fff : 0x8fff ;
            u->r =  (rgba >>  0) & 0xff ;
            u->g =  (rgba >>  8) & 0xff ;
            u->b =  (rgba >> 16) & 0xff ;
            u->a = ((rgba >> 24) & 0xff)*j/n_verts ;
        }
}

static int GetResources( Work *work, FMATRIX *world, int where )
{
    int i ;
    static FVECTOR spill_shape[] = {
	{-30, 0, -30, 1}, { 30, 0, -30, 1}, {-30, 0, 30, 1}, { 30, 0, 30, 1}, { 30, 0, 30, 1},
    } ;
    static short start_frm[] = {  0 + BRK_WIN_DRIPSTART,  5 + BRK_WIN_DRIPSTART,
				 20 + BRK_WIN_DRIPSTART, 45 + BRK_WIN_DRIPSTART } ;
    static short start_posx[] = { 15, 15, -14, 0 } ;
    DG_DEF *def ;

    if ( !(work->drip = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF,
				      BRK_N_DRIP, 2 )) )
        return -1 ;
    InitLinePrim( work->drip, BRK_N_DRIP, 2, 0x700a0040 ) ;
    work->drip->flag |= DG_PRIM_AS_CUSTOMWORLD;
    work->drip->as_world = *world ;

    if ( !(work->spill = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF,
				       1, BRK_N_SPILL )) )
        return -1 ;
    InitSpillPrim( work->spill, GV_StrCode("splash03_alp"), 0x400a0040 ) ;
    DG_InvisiblePrim2( work->spill ) ;
    work->spill->flag |= DG_PRIM_AS_CUSTOMWORLD;
    work->spill->as_world = *world ;
    work->spill->as_world.m[W][Y] = 9010.0f ;

    work->world        = *world ;
    work->floor = 9010.0f ;
    work->floor = world->m[W][Y] - work->floor ;

    work->sx = 1.0f ;
    work->sy = 0.0f ; /* Yの位置 */
    work->sa = 0.0f ; /* 加速度  */

    for ( i=BRK_N_DRIP ; --i>=0 ; )
    {
        work->drip->pos[0][i*2].vx = work->drip->pos[0][i*2+1].vx = start_posx[i] ;
        work->drip->pos[0][i*2].vz = work->drip->pos[0][i*2+1].vz = 150.0f ;
        work->drip->pos[0][i*2].vy = work->drip->pos[0][i*2+1].vy = 0     ;
        work->drip->pos[0][i*2].vw = 0 ;
        work->drip->pos[0][i*2+1].vw = start_frm[i] ;
    }

    memcpy( work->spill->pos[0], spill_shape, sizeof(FVECTOR)*BRK_N_SPILL ) ;

    if ( !(def = GV_GetCache( GV_CacheID( GV_StrCode( "eki_R" ), 'k' ) )) )
	return -1 ;
    if ( !(work->stream = DG_MakeComdl( def->models[0].packs, DG_COMDL_NOFOG|DG_COMDL_SEMITRANS, 1, 0 )) )
	return -1 ;
    DG_QueueComdlObjs( work->stream ) ;
    GM_GroupObject( work->stream, where ) ;
#if 0
    work->stream->pos[0].color.vx = 64 ;
    work->stream->pos[0].color.vy =  0 ;
    work->stream->pos[0].color.vz = 10 ;
    work->stream->pos[0].color.vw = 16 ;
#else
    work->stream->pos[0].color.vx = 64 ;
    work->stream->pos[0].color.vy = 64 ;
    work->stream->pos[0].color.vz = 64 ;
    work->stream->pos[0].color.vw = 128 ;
#endif
    GM_SeSetMode( SD_A_KOBORE01, (FVECTOR*)&world->m[3], GM_SEMODE_NORMAL ) ;

    return 0 ;
}

void *NewWineStreamFall( FMATRIX *world, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, world, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}


