//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_swing.c
   レードル揺れ

   1999/12/13 T. Morita
   $Id: brk_fragile.c,v 1.1.1.3 2002/11/19 11:45:47 Yoshizawa1 Exp $
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


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"


#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoDEG(_a) ((int)(_a)*180/32768)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)
#define BRK_GRAVITY 30

#define BRK_MAX_SPEED   -100.0f



typedef struct work_t  Work  ;
typedef struct piece_t PIECE ;

struct piece_t
{
    FVECTOR   pos   ; /* 位置     */
    short     rot_x,  rot_y  ;
    short     rot_vx, rot_vy ;

    float     pos_vy  ;
    float     gravity ;
    float     scale   ;

    FVECTOR  *shape ;
    short     alpha ;
    short     flag  ;
} ;

struct work_t
{
    GV_ACT    actor   ;

    DG_PRIM2 *frag    ;
    PIECE    *piece   ;
    short     n_piece ;
    short     life    ;
} ;


static FVECTOR BRK_Shape1[] = {
    {-5.0f, 0.0f, 50.0f, 1.0f },
    { 5.0f, 0.0f, 50.0f, 1.0f },
    {-5.0f, 0.0f,-50.0f, 1.0f },
    { 5.0f, 0.0f,-50.0f, 1.0f },
} ;

static FVECTOR BRK_Shape2[] = {
    { -5.0f, 0.0f, 10.0f, 1.0f },
    { 10.0f, 0.0f, 10.0f, 1.0f },
    {-10.0f, 0.0f,-10.0f, 1.0f },
    {  5.0f, 0.0f,-10.0f, 1.0f },
} ;


extern void AN_Test_Eye3( FVECTOR *mov ) ; /* for test */


static void MakeShape( PIECE *p, FVECTOR *pos, DG_PRIM2_UVRGB *uvs, FMATRIX *lights )
{
    FMATRIX mtx ;
    float   rgb ;
    int     i   ;

    mtx.m[X][Y] = mtx.m[X][Z] = 0.0f ;
    mtx.m[Y][X] = mtx.m[Y][Z] = 0.0f ;
    mtx.m[Z][X] = mtx.m[Z][Y] = 0.0f ;
    mtx.m[X][X] = mtx.m[Y][Y] = mtx.m[Z][Z] = p->scale ;
    _sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, ANGtoRAD(p->rot_y) ) ;
    _sceVu0RotMatrixX( &mtx, &mtx          , ANGtoRAD(p->rot_x) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx.m[W], &p->pos ) ;

    rgb = fpu_Abs( mtx.m[Y][Y] ) * 120.0f ;

    for( i=4 ; --i>=0 ; )
    {
	_sceVu0ApplyMatrix( &pos[i], &mtx, &p->shape[i] ) ;
	uvs[i].r = uvs[i].g = uvs[i].b = (int)rgb ;
	uvs[i].a = p->alpha ;
    }
}

static void Die( Work *work )
{
    if ( work->frag  )
	GM_FreePrim2( work->frag ) ;
    if ( work->piece )
	GV_Free( work->piece ) ;
}

static void Act( Work *work )
{
    int             i    ;
    FVECTOR        *pos  ;
    DG_PRIM2_UVRGB *uvs  ;
    PIECE          *p    ;
    FMATRIX         lights[2] ;
    int flag = 0 ;

    DG_SwitchBuffPrim2( work->frag ) ;
    pos = work->frag->pos  [work->frag->buffer_clock] ;
    uvs = work->frag->uvrgb[work->frag->buffer_clock] ;

    DG_GetLightMatrix( &work->piece[0].pos, lights ) ;

    p = work->piece ;
    for ( i=work->n_piece ; --i>=0 ; )
    {
	if ( p->flag > 0 )
	    if ( !--p->flag )
		p->alpha = 128 ;
	if ( p->alpha > 0 )
	{
	    /* 寿命がなくなったらαを下げる */
	    if ( work->life <= 0 )
		p->alpha-- ;

	    //AN_Test_Eye2( &p->pos, 2 ) ;

	    /* 位置回転 更新 */
	    p->rot_y  += p->rot_vy ;
	    p->rot_x  += p->rot_vx ;
	    if ( p->pos_vy > BRK_MAX_SPEED ) /* 速度は一定以上は上がらない */
		p->pos_vy -= p->gravity ;
	    p->pos.vy += p->pos_vy ;
	    MakeShape( p, pos, uvs, lights ) ;
	}
	flag |= p->alpha | p->flag ;
	p++ ;
	uvs+=4 ;
	pos+=4 ;
    }

    if ( work->life > 0 )
	work->life-- ;
    if ( !flag )
	GV_DestroyActor( work ) ;
}

static int GetResources( Work *work, FVECTOR *origine, int n_piece,
			 int life, float width, int interval )
{
    int             i    ;
    FVECTOR        *pos  ;
    DG_PRIM2_UVRGB *uvs  ;
    PIECE          *p    ;
    FMATRIX lights[2] ;

    if ( !(work->frag = BRK_UTL_MakePOLY( n_piece,
                                          GV_StrCode( "for_lnr_sling" ),
                                          SCE_GS_SET_ALPHA(0,1,0,1,0),
                                          0x00707f7f )) )
        PERROR( "No Prim(no memory) : NewFallFragile\n" ) ;

    if ( !(work->piece = GV_Malloc( sizeof(PIECE) * n_piece ) ) )
        PERROR( "No memory for <work->piece> : NewFallFragile\n" ) ;

    pos = work->frag->pos  [work->frag->buffer_clock] ;
    uvs = work->frag->uvrgb[work->frag->buffer_clock] ;

    DG_GetLightMatrix( origine, lights ) ;

    work->life    = life    ;
    work->n_piece = n_piece ;
    p = work->piece ;
    for ( i=n_piece ; --i>=0 ; )
    {
	p->shape   = i&1 ? BRK_Shape1 : BRK_Shape2 ;
	p->rot_x   = irnd() ;
	p->rot_y   = irnd() ;
	p->rot_vx  = irnd()&4095 ;
	p->rot_vy  = irnd()&4095 ;
	p->flag    = i * interval + (irnd() & 3) + 1 ;
	p->gravity = rnd()*BRK_GRAVITY + 1.0f ;
	p->scale   = frnd() ;
	p->alpha   = 0 ;

	_sceVu0CopyVector( &p->pos, origine ) ;
	p->pos_vy = 0.0f ;
	p->pos.vx += frnd() * width ;
	p->pos.vz += frnd() * width ;

	MakeShape( p, pos, uvs, lights ) ;

	uvs += 4 ;
	pos += 4 ;
	p++ ;
    }

    return 0 ;
}

void *NewFallFragile( FVECTOR *pos, int n_piece, int life, float width, int interval )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if ( GetResources( work, pos, n_piece, life, width, interval ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

