//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_piece.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_piece.c,v 1.1.1.3 2002/11/19 11:45:30 Yoshizawa1 Exp $
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


// for box hzd
#include  "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define BRK_PIECE_GRAVITY 4
#define BRK_PVEL_R    0.88f 
#define BRK_VEL_R 16
#define BRK_ROT_R 16

#define BRK_PCE_SPREAD 50.0f
#define BRK_PCE_INACTIVE 0x8000000

typedef struct
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;

    DG_COMDL_POS *comdl ;
    HZD_BOX      *hzd   ;

    int           flag  ;
} PIECE ;

typedef struct work_t
{
    GV_ACT       actor  ;

    DG_COMDL    *piece_l ;
    DG_COMDL    *piece_s ;
    PIECE       *piece   ;
    int          n_piece ;
    int          index   ;
} Work ;

Work *Work_Piece = NULL ;

static void RotateMatrixXY( PIECE *p )
{
    int r ;
    FMATRIX *out = &p->comdl->world ;

    r = p->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, &DG_UnitMatrix,
		       (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = p->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out           ,
		       (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static void Die( Work *work )
{
    if ( work->piece_s )
	DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
	DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->piece )
	GV_Free( work->piece ) ;
    Work_Piece = NULL ;
}

static void Act( Work *work )
{
    int     i, flag=0 ;
    PIECE  *p ;
    FVECTOR v ;
    static FVECTOR Size    = { 10.0f, 10.0f, 10.0f, 0.0f } ;
    static FVECTOR Bounce  = { 1.48f, 1.48f, 1.48f, 0.0f } ;

    if ( !(work->n_piece & BRK_PCE_INACTIVE) )
    {
	for ( i=work->n_piece, p=work->piece ; --i>=0 ; flag|=p->flag, p++ )
	    if ( p->flag )
	    {
		p->pos_v.vy -= BRK_PIECE_GRAVITY ;
		p->rot_x += p->rot_vx ;
		p->rot_y += p->rot_vy ;
		RotateMatrixXY( p ) ;
		switch ( BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v,
					  &Bounce, &Size ) )
		{
		case 1:
		case 3:
		    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
		    _sceVu0ScaleVector( &v, &p->pos_v, 0.125f ) ;
		    if ( !(int)v.vx && !(int)v.vy && !(int)v.vz )
			p->rot_x = 0, p->flag = 0 ;
		    break ;
		case 0:
		    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
		}
		_sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W],
				      &p->pos ) ;
	    }
	if ( !flag )
	    work->n_piece |= BRK_PCE_INACTIVE ;
    }
}

static DG_COMDL *InitPiece( DG_DEF *def, int idx, int n, int where )
{
    DG_COMDL *cmdl ;

    cmdl = DG_MakeComdl( def->models[idx].packs, DG_COMDL_SEMITRANS, n, 0 ) ;
    if ( cmdl )
    {
	DG_QueueComdlObjs( cmdl ) ;
	GM_GroupObject( cmdl, where ) ;
	cmdl->flag |= DG_FLAG_INVISIBLE ;
    }
    return cmdl ;
}

static int GetResourcesCalled( Work *work, int where, int n_piece, int mdl_id )
{
    int     i   ;
    DG_DEF *def ;
    DG_COMDL_POS *pos ;

    work->n_piece = n_piece ;
    if ( !mdl_id )
	PERROR( "No -model option : NewPutGlassObject\n" ) ;
    if ( !(def = GV_GetCache( GV_CacheID( mdl_id, 'k' ) )) )
	PERROR( "No Model COMDL(%d) : NewPutGlassObject\n", mdl_id ) ;
    if ( !(work->piece = GV_Malloc( sizeof(PIECE) * work->n_piece )) )
	PERROR( "No memory (work->piece) : NewPutGlassObject\n" ) ;
    if ( !(work->piece_s = InitPiece( def, 0, work->n_piece/2, where )) )
	PERROR( "No memory for COMDL(work->piece_s) : NewPutGlassObject\n" ) ;
    if ( !(work->piece_l = InitPiece( def, 1, work->n_piece/2, where )) )
	PERROR( "No memory for COMDL(work->piece_l) : NewPutGlassObject\n" ) ;

    for ( i=0 ; i<work->n_piece ; i++ )
    {
	pos = (i&1 ? work->piece_s->pos : work->piece_l->pos) + i/2 ;
	work->piece[i].comdl = pos ;
	work->piece[i].flag  = 0   ;
        pos->world    = DG_UnitMatrix ;
        pos->color.vx = pos->color.vy = pos->color.vz = 64 ;
        pos->color.vw = 0 ;
    }
    work->n_piece |= BRK_PCE_INACTIVE ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    if ( GetResourcesCalled( work, where,
			     GCL_GetOptionValue( 'n', 20 ), 
			     GCL_GetOptionValue( 'm', 0 )) < 0 )
	return -1 ;
    return 0 ;
}

void *NewGlassDust( int name, int where )
{
    if ( Work_Piece )
	printf( "You Launch the Second NewGlassDust\n" ) ;
    else
    {
	Work_Piece = (Work *)GV_NewActorPrio( GV_ACTOR_USER,
					      sizeof(Work), 0x11 ) ;
	if( Work_Piece != NULL )
	{
	    GV_SetActor( &Work_Piece->actor, Act, Die ) ;
	    if( GetResources( Work_Piece, name, where ) < 0 )
	    {
		GV_DestroyActor( Work_Piece ) ;
		return NULL ;
	    }
	}
    }

    return Work_Piece ;
}

int BRK_PCE_StartActPieces( int hzd_id, FVECTOR *pos, FVECTOR *frc, int i )
{
    PIECE *p ;
    Work *work = Work_Piece ;

    if ( !work )
	return 0 ;

    work->n_piece &= ~BRK_PCE_INACTIVE ;
    while( --i>=0 )
    {
	if ( work->index >= work->n_piece )
	    work->index = 0 ;
	p = &work->piece[work->index++] ;
	p->hzd = BRK_InitHazard( hzd_id ) ;
	p->rot_x  = 0  ;
	p->rot_y  = irnd()&2047 ;
	p->rot_vx = 512 ;
	p->rot_vy = 0  ;
	p->pos.vx = pos->vx + (p->pos_v.vx = BRK_PCE_SPREAD*frnd()) ;
	p->pos.vy = pos->vy + (p->pos_v.vy = BRK_PCE_SPREAD*frnd()) ;
	p->pos.vz = pos->vz + (p->pos_v.vz = BRK_PCE_SPREAD*frnd()) ;
	p->pos_v.vx += frc->vx*0.005f ;
	p->pos_v.vz += frc->vz*0.005f ;
	p->comdl->color.vw = 64 ;
	RotateMatrixXY( p ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;
	p->flag = 1 ;
    }
    return work->index ;
}
