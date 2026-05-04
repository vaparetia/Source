//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_act.c
   瓶壊れ アクト

   1999/12/02 T. Morita
   $Id: wireframe.c,v 1.1.1.3 2002/11/19 11:45:52 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"


typedef struct work_t
{
    GV_ACT    actor ;

    int        n_prim ;
    DG_PRIM2 **prim   ;
} Work  ;


static void Die( Work *work )
{
    if ( work->prim )
    {
	int i ;

	for( i=0 ; i<work->n_prim ; i++ )
	    if ( work->prim[i] )
		GM_FreePrim2( work->prim[i] ) ;
	GV_Free( work->prim ) ;
    }
}

static void SetUVS( DG_PRIM2_UVRGB *u0, DG_PRIM2_UVRGB *u1, int f, int rgba )
{
    int r,g,b,a ;

    r = (rgba>> 0)&0xff ;
    g = (rgba>> 8)&0xff ;
    b = (rgba>>16)&0xff ;
    a = (rgba>>24)&0xff ;

    u0->q = u1->q = 4096 ;
    u0->f = u1->f = f ;
    u0->r = u1->r = r ;
    u0->g = u1->g = g ;
    u0->b = u1->b = b ;
    u0->a = u1->a = a ;
}

static void SetPOS( FVECTOR *p0, FVECTOR *p1, short *p )
{
    p0->vx = p[0] ;
    p0->vy = p[1] ;
    p0->vz = p[2] ;
    _sceVu0CopyVector( p1, p0 ) ;
}

static int GetResources( Work *work,
			 DG_OBJPACK *packs, int n_packs,
			 int rgba, FMATRIX *mtx )
{
#ifdef PSX2
    int i, j ;
    DG_PRIM2 *p ;
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;
    int last, count ;
    int n_prim, n_line ;

    work->n_prim = n_packs * 3 ;
    work->prim = GV_Malloc( sizeof(DG_PRIM2 *) * work->n_prim ) ;
    if ( work->prim == NULL )
	return -1 ;
    for ( i=0 ; i<work->n_prim ; i++ )
	work->prim[i] = NULL ;

    n_prim = 0 ;
    for ( i=0 ; i<n_packs ; i++ ) {
	n_line = 0 ;
	for ( j=0 ; j<(int)packs[i].n_verts ; j++ ){
	    if ( n_line < 3 ) {
			n_line = 60 ;
			if ( !(p = GM_MakePrim2( DG_PRIM2_LINE, 1, n_line )) ) {
				return -1 ;
			} else {
				work->prim[n_prim++] = p ;
				p->root = mtx ;
				u0 = p->uvrgb[0], u1 = p->uvrgb[1] ;
				p0 = p->pos  [0], p1 = p->pos  [1] ;
				last  = 0x8000 ;
				count = 0      ;
			}
	    }

	    if ( last & 0x8000 )
		last = 0x0fff ;
	    else
		last = (int)packs[i].norms[j*4+3] ;
	    if ( !(last & 0x8000) )
		count++ ;
	    else
		count = 0 ;
	    SetUVS( u0++, u1++, last, rgba ) ;
	    SetPOS( p0++, p1++, &packs[i].verts[j*4] ) ;
	    n_line-- ;

	    if ( count >= 2 )
	    {
		SetUVS( u0++, u1++, 0x0fff, rgba ) ;
		SetPOS( p0++, p1++, &packs[i].verts[j*4-8] ) ;
		n_line-- ;
		SetUVS( u0++, u1++, 0x8fff, rgba ) ;
		SetPOS( p0++, p1++, &packs[i].verts[j*4] ) ;
		n_line-- ;
	    }
	}

	/* 残りのライン頂点を非描画にする */
	while( n_line > 0 )
	{
	    SetUVS( u0++, u1++, 0x8fff, rgba ) ;
	    SetPOS( p0++, p1++, packs[i].verts ) ;
	    n_line-- ;
	}
    }
//printf( "NewDrawWireframe[%x] [%d/%d]\n", work, n_prim, work->n_prim ) ;
    return 0 ;
#else
	return -1 ;
#endif
}

void *NewDrawWirePacks( DG_OBJPACK *packs, int n_packs, FMATRIX *mtx )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, Die ) ;
        if( GetResources( work, packs, n_packs, 0x7f7f7f7f, mtx ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewDrawWireRoot( DG_OBJ *obj, FMATRIX *mtx )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, Die ) ;
        if ( GetResources( work,
			   obj->model->packs, obj->model->n_packs,
			   0x7f7f7f7f, mtx ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewDrawWireframe( DG_OBJ *obj )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, Die ) ;
        if ( GetResources( work,
			   obj->model->packs, obj->model->n_packs,
			   0x7f7f7f7f, &obj->world ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
