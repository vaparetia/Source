//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   break_obj.c
   ライト壊れ

   1999/11/26      T. Morita
   1999/01/25 r1.8 T. Morita
   1999/01/30 r2.0 T. Morita
   $Id: brk_light.c,v 1.1.1.3 2002/11/19 11:45:34 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_light.h"
#include "../include/util.h"



static void Die( Work *work )
{
    if ( work->dust )
        GM_FreePrim2( work->dust ) ;
    if ( work->glas )
        GM_FreePrim2( work->glas ) ;
}

static int ActDust( DG_PRIM2 *p )
{
    int i, flag = 0 ;
    DG_PRIM2_UVRGB *uvs ;
    FVECTOR *cur, *prv ;

    prv = p->pos  [p->buffer_clock] ;
    DG_SwitchBuffPrim2( p );
    uvs = p->uvrgb[p->buffer_clock] ;
    cur = p->pos  [p->buffer_clock] ;
    for ( i=BRK_N_DUST+BRK_N_LITR ; --i>=0 ; flag |= *(int*)&cur[1].vw, prv+=2, cur+=2, uvs+=2 )
        if ( (*(int*)&cur[1].vw = *(int*)&prv[1].vw) > 0 )
            if ( --(*(int*)&cur[1].vw) < 128 )
            {
		_sceVu0CopyVectorXYZ( &cur[1], &prv[0] ) ;
		if ( i < BRK_N_DUST )
		    cur[0].vy = prv[0].vy + (cur[0].vw = prv[0].vw-BRK_GRAVITY*0.5f);
		else
		    cur[0].vy = prv[0].vy - BRK_GRAVITY*4 ;
		uvs[1].a = *(int*)&cur[1].vw * 2 ;
            }
    return flag ;
}

static int ActGlass( GLAS *glas, DG_PRIM2 *p )
{
    int i, flag=0, *alpha ;
    FVECTOR        *pos ;
    DG_PRIM2_UVRGB *uvs ;
    FMATRIX  mtx ;
    static FVECTOR shape[] = { {  0.0f, 20.0f, 0.0f, 1.0f},
			       {-20.0f,-30.0f, 0.0f, 1.0f},
			       {  5.0f,-10.0f, 0.0f, 1.0f} } ;
    static FVECTOR shape1[] = { { 0.0f, 6.0f, 0.0f, 1.0f},
			       {-10.0f,-5.0f, 0.0f, 1.0f},
			       { 4.0f,-8.0f, 0.0f, 1.0f} } ;

    DG_SwitchBuffPrim2( p );
    uvs = p->uvrgb[p->buffer_clock] ;
    pos = p->pos  [p->buffer_clock] ;
    for ( i=BRK_N_GLAS ; --i>=0 ;  )
    {
	uvs[0].a = uvs[1].a = uvs[2].a = 0 ;
        if ( *(alpha = (int*)&glas->pos.vw) > 0 )
            if ( --(*alpha) < 128 )
            {
		glas->rot_x = (glas->rot_x + glas->rot_vx)&4095 ;
		glas->rot_y = (glas->rot_y + glas->rot_vy)&4095 ;
		_sceVu0RotMatrixX( &mtx, &DG_UnitMatrix, (float)M_PI*2/4096.0f*glas->rot_x ) ;
		_sceVu0RotMatrixY( &mtx, &mtx          , (float)M_PI*2/4096.0f*glas->rot_y ) ;
		glas->pos_v.vy -= (i&3 ? BRK_GRAVITY*0.33f : BRK_GRAVITY) ;
		mtx.m[W][X] = glas->pos.vx += glas->pos_v.vx ;
		mtx.m[W][Y] = glas->pos.vy += glas->pos_v.vy ;
		mtx.m[W][Z] = glas->pos.vz += glas->pos_v.vz ;
		DG_SetPos( &mtx ) ;
		DG_PutVector( (i&3 ? shape1 : shape), pos, BRK_N_GLAS_VERTS ) ;
		_sceVu0MulMatrix( &mtx, &DG_Chanls->eye, &mtx ) ;

		uvs[0].r = uvs[0].g = uvs[0].b =
		uvs[1].r = uvs[1].g = uvs[1].b = 
		uvs[2].r = uvs[2].g = uvs[2].b = (short)(128 * mtx.m[1][Y] * mtx.m[1][Y]) ;
		uvs[0].a = uvs[1].a = uvs[2].a = uvs[0].g + *alpha/2 ;
            }

	glas++ ;
	pos+=BRK_N_GLAS_VERTS ;
	uvs+=BRK_N_GLAS_VERTS ;
	flag|=*alpha ;
    }

    return flag ;
}

static void Act( Work *work )
{
    int flag ;

    flag  = ActDust ( work->dust ) ;
    flag |= ActGlass( work->glas_prof, work->glas ) ;

    if ( !flag )
        GV_DestroyActor( work ) ;
}

static void InitPrimUVRGB( DG_PRIM2_UVRGB *u, int i, int n_verts, u_int rgba, int vtx_f )
{
    int j ;

    for ( ; --i>=0 ; )
        for ( j=0 ; j<n_verts ; j++, u++ )
        {
	    u->f = j<vtx_f ? 0x8fff : 0x0fff ;
            u->r = (rgba >>  0) & 0xff ;
            u->g = (rgba >>  8) & 0xff ;
            u->b = (rgba >> 16) & 0xff ;
            u->a = (rgba >> 24) & 0xff ;
        }
}

static int GetResources( Work *work, FVECTOR *pos, FVECTOR *dir, int map )
{
    int i, j ;
    GLAS *glas ;
    FVECTOR *p0, *p1 ;

    /* 外から作られるのでマップの設定を変える */
    GM_CurrentMap = map ;

    if ( !dir )
	return -1 ;
    work->vel = *dir ;

    if ( !(work->dust = GM_MakePrim2( DG_PRIM2_LINE| DG_PRIM2_ALPHA | DG_PRIM2_NOMSAA, BRK_N_DUST+BRK_N_LITR, 2 ) ) )
        return -1 ;
    InitPrimUVRGB( work->dust->uvrgb[0], BRK_N_DUST+BRK_N_LITR, 2, 0x00807040, 1 ) ;
    InitPrimUVRGB( work->dust->uvrgb[1], BRK_N_DUST+BRK_N_LITR, 2, 0x00807040, 1 ) ;
    DG_SetPrim2Alpha( work->dust, SCE_GS_SET_ALPHA(0,2,0,1,0) ) ;
    work->dust->root = &work->root ;

    if ( !(work->glas = GM_MakePrim2( DG_PRIM2_POLY| DG_PRIM2_ALPHA | DG_PRIM2_NOMSAA, BRK_N_GLAS, BRK_N_GLAS_VERTS ) ) )
        return -1 ;
    InitPrimUVRGB( work->glas->uvrgb[0], BRK_N_GLAS, BRK_N_GLAS_VERTS, 0x80807040, 2 ) ;
    InitPrimUVRGB( work->glas->uvrgb[1], BRK_N_GLAS, BRK_N_GLAS_VERTS, 0x80807040, 2 ) ;
    DG_SetPrim2Alpha( work->glas, SCE_GS_SET_ALPHA(0,2,0,1,0) ) ;
    work->glas->root = &work->root ;

    work->root = DG_UnitMatrix ;
    work->root.m[3][X] = pos->vx ;
    work->root.m[3][Y] = pos->vy ;
    work->root.m[3][Z] = pos->vz ;

    /* ガラス埃のPrimの初期化 */
    p0 = work->dust->pos[0] ;
    p1 = work->dust->pos[1] ;
    for ( i=BRK_N_DUST+BRK_N_LITR ; --i>=0 ; p0+=2, p1+=2 )
    {
        p0[0].vx = p0[1].vx = p1[0].vx = p1[1].vx = BRK_SIZE*4*frnd() ;
	p0[0].vz = p0[1].vz = p1[0].vz = p1[1].vz = BRK_SIZE*4*frnd() ;
	p0[0].vy = p0[1].vy = p1[0].vy = p1[1].vy = BRK_SIZE*4*frnd() ;
	p0[0].vw = p1[0].vw = 0 ;       
	*(int*)&p0[1].vw = *(int*)&p1[1].vw = 20 + i*10 ;
    }

    /* ガラス破片の Prim の初期化 */
    p0 = work->glas->pos[0] ;
    p1 = work->glas->pos[1] ;
    for ( i=BRK_N_GLAS ; --i>=0 ; )
	for ( j=BRK_N_GLAS_VERTS ; --j>=0 ; p0++, p1++ )
	    p0->vx = p0->vy = p0->vz = p1->vx = p1->vy = p1->vz = 0.0f ;
    /* ガラス破片の位置の初期化 */
    for ( i=BRK_N_GLAS, glas=work->glas_prof ; --i>=0 ; glas++ )
    {
	glas->pos.vx = BRK_SIZE*frnd() ;
	glas->pos.vy = BRK_SIZE*frnd() ;
	glas->pos.vz = BRK_SIZE*frnd() ;
	*(int*)&glas->pos.vw = 128 ;

	glas->pos_v.vx = glas->pos.vx*0.5f + work->vel.vx*0.025f*0.0f ;
	glas->pos_v.vy = glas->pos.vy*0.5f ;
	glas->pos_v.vz = glas->pos.vz*0.5f + work->vel.vz*0.025f*0.0f ;
	glas->pos_v.vw = 0 ;
	glas->rot_x  = irnd() & 4095 ;
	glas->rot_y  = irnd() & 4095 ;
	glas->rot_vx = (irnd()&511) - 256 ;
	glas->rot_vy = (irnd()&511) - 256 ;
    }

    if ( HZX_NearHazardCheck( GM_GetHzxGroupID( map ), pos,
			      BRK_HZX_SPHERE,
			      HZX_CHK_ALL,
			      HZX_SEG_ALL,
			      BRK_HZX_SPHERE ) )
    {
	HZX_SEG wall[2] ;
	float dx, dz, dd ;
	int atrs[2] ;

	HZX_GetNearHazard( wall, atrs ) ;

	work->ddx = dx = wall[0].p2.x - wall[0].p1.x ;
	work->ddz = dz = wall[0].p2.z - wall[0].p1.z ;
	work->dd = dd = fpu_Sqrt( dx*dx + dz*dz ) ;
	work->cs = dx/dd ;
	work->sn = dz/dd ;
	if ( pos->vx*dz > pos->vz*dx )
	    work->dx =  dx, work->dz =  dz ;
	else
	    work->dx = -dx, work->dz = -dz ;
    }
    else
	work->dx = work->dz = 0 ;

    return 0 ;
}


void *NewBreakLight( FVECTOR *pos, FVECTOR *dir, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, pos, dir, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
