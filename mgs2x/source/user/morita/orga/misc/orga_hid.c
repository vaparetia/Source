//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_hid.c 
   オルガ ハイド生成プットオブジェ

   1999/12/18 T.Morita
   $Id: orga_hid.c,v 1.1.1.3 2002/11/19 11:46:26 Yoshizawa1 Exp $
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
#include "../include/orga.h"


typedef struct hidework_t
{
    GV_ACT    actor  ;
} HideWork ;



HIDE ORG_HideBranch[ORGA_N_HIDE_POOL] ;
HIDE ORG_HoloHide ;

static void Die( HideWork *work )
{
    HIDE *h ;

    for ( h=ORG_HideBranch ; h->objs ; h++ )
	if ( h->objs )
	    DG_DequeueObjs( h->objs ), DG_FreeObjs( h->objs );
}

static inline int ORG_HID_InitParam( HIDE *h, int id ) 
{
    FVECTOR *t_size, *t_pos ;
    FVECTOR uv = { h->objs->def->ux, h->objs->def->uy, h->objs->def->uz, 0 } ;
    FVECTOR lv = { h->objs->def->lx, h->objs->def->ly, h->objs->def->lz, 0 } ;

    _sceVu0ApplyMatrix( &uv, &h->objs->world, &uv ) ;
    _sceVu0ApplyMatrix( &lv, &h->objs->world, &lv ) ;

    t_size = &h->size   ;
    t_pos  = &h->center ;
    t_size->vx = fpu_Abs(uv.vx - lv.vx)*0.5f ;
    t_size->vy = fpu_Abs(uv.vy - lv.vy)*0.5f ;
    t_size->vz = fpu_Abs(uv.vz - lv.vz)*0.5f ;

    h->id     = id ;
    h->height = t_size->vy*2 ;
    h->turn  += 1024*3 ;

    _sceVu0AddVector  ( t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( t_pos, t_pos, 0.5f ) ;
    _sceVu0AddVector  ( t_pos, t_pos, (FVECTOR *)&h->objs->world.m[W] ) ;
    t_pos->vx += h->objs->def->tx ;
    t_pos->vy += h->objs->def->ty ;
    t_pos->vz += h->objs->def->tz ;

#if 0
    {
	TARGET *t  = &h->target ;
	GM_SetTarget( t, TARGET_DEFENSE, map, ENEMY_SIDE, &t_size, &DG_ZeroVector ) ;
	GM_MoveTarget( t, t_pos ) ;
	GM_PutTarget( t ) ;
	//NewTargetView( t,  200, 50, 32 ) ;
    }
#endif

    h->p[0][X] = t_pos->vx + t_size->vx ;
    h->p[0][Y] = t_pos->vz - t_size->vz ;
    h->p[1][X] = t_pos->vx + t_size->vx ;
    h->p[1][Y] = t_pos->vz + t_size->vz ;
    h->p[2][X] = t_pos->vx - t_size->vx ;
    h->p[2][Y] = t_pos->vz - t_size->vz ;
    h->p[3][X] = t_pos->vx - t_size->vx ;
    h->p[3][Y] = t_pos->vz + t_size->vz ;

    h->right.vx = t_pos->vx - t_size->vx - ORGA_HIDE_V_SPHERE ;
    h->right.vy = t_pos->vy ;
    h->left.vx  = t_pos->vx - t_size->vx - ORGA_HIDE_V_SPHERE ;
    h->left.vy  = t_pos->vy ;

    return 0 ;
}


static int GetResources( HideWork *work, int name, int where )
{
    int   i, buf[3] ;
    char *c ;
    HIDE *h ;
    LIT_DEF *lit_def = NULL ;
    DG_DEF  *def ;

    for ( h=ORG_HideBranch, i=ORGA_N_HIDE_POOL ; --i>=0 ; h++ )
	h->objs = NULL ;

    if ( (i = GCL_GetOptionValue( 'l', 0 )) != 0 )
        if ( !(lit_def = (LIT_DEF *)GV_GetCache( GV_CacheID( i, 'l' ) )) )
	    PERROR( "No lt2-ID!! :NewPutHideObject" ) ;
    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model-ID!! :NewPutHideObject" ) ;
    for( h=ORG_HideBranch ; (c=GCL_NextStr()) ; h++ )
    {
	if ( !(def = GV_GetCache( GV_CacheID( GCL_GetInt( c ), 'k' ) )) )
	    PERROR( "No KMS-data in data.cnf!! :NewPutHideObject" ) ;
	if ( !(h->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Cannot make DG_OBJS( maybe no memory )!! :NewPutHideObject" ) ;
	DG_QueueObjs( h->objs ) ;
	DG_MakePreshade( h->objs, lit_def ) ;
    }

    if ( GCL_GetOption( 'r' ) )
	for( h=ORG_HideBranch ; (c=GCL_NextStr()) ; h++ )
	{
	    i = GCL_GetInt( c ) & 0x0fff ;
	    i -= 4096 & (i<<1) ;
	    h->turn = i & 1023 ;
	    _sceVu0RotMatrixY( &h->objs->world, &DG_UnitMatrix,i*(float)M_PI/2048.0F ) ;
	}

    if ( GCL_GetOption( 'p' ) )
	for( h=ORG_HideBranch, i=0 ; (c=GCL_NextStr()) ; h++, i++ )
	{
	    GCL_GetIV( c, buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&h->objs->world.m[3] ) ;
	    h->objs->world.m[3][3] = 1.0f ;
	    ORG_HID_InitParam( h, i ) ;
	}

    return 0 ;
}


void *NewPutHideObject( int name, int where )
{
    HideWork *work ;

    work = (HideWork *)GV_NewActor( GV_ACTOR_USER, sizeof(HideWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
