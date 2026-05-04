//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_piece.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_build_pce.c,v 1.1.1.3 2002/11/19 11:45:25 Yoshizawa1 Exp $
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

#define BRK_BLD_SPREAD 30.0f
#define BRK_BLD_INACTIVE 0x8000

typedef struct
{
    FMATRIX  world ;
    FVECTOR  pos   ; /* 位置     */
    FVECTOR  pos_v ; /* 速度     */
    short    rot_x,  rot_y ;
    short    rot_vx, rot_vy ;

    FVECTOR *shape ;

    short    flag  ;
    short    type  ;
} PIECE ;

typedef struct work_t
{
    GV_ACT       actor  ;

    FVECTOR      center  ;
    DG_PRIM2    *prim    ;
    PIECE       *piece   ;
    short        n_piece ;
    short        se_time ;
    short        life    ;
} Work ;

#define BRK_BLD_NVERTS 3

typedef struct poly_t POLY ;
struct poly_t
{
    FVECTOR shape ;
    float   u,v ;
    int     rgba ;
} ;

#define SCALE 2.0f

static float BRK_BLD_Gravity[] = {
    4.0f, 3.0f, 2.0f, 1.0f
} ;

/*共有頂点*/
static POLY BRK_BLD_Poly[][6] = {
    {
	{{ 35.0f*SCALE, 5.0f*SCALE,-50.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  5.0f*SCALE, 5.0f*SCALE, 50.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{-35.0f*SCALE, 5.0f*SCALE,-50.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
	{{ 35.0f*SCALE,-5.0f*SCALE,-50.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  5.0f*SCALE,-5.0f*SCALE, 50.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{-35.0f*SCALE,-5.0f*SCALE,-50.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
   },		        	 				         	   
    {		        	 				         	   
	{{ 30.0f*SCALE, 5.0f*SCALE,-30.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  3.0f*SCALE, 5.0f*SCALE, 30.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{-15.0f*SCALE, 5.0f*SCALE,-30.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
	{{ 30.0f*SCALE,-5.0f*SCALE,-30.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  3.0f*SCALE,-5.0f*SCALE, 30.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{-15.0f*SCALE,-5.0f*SCALE,-30.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
   },		        	 				         	   
    {		        	 				         	   
	{{ 10.0f*SCALE, 5.0f*SCALE,-10.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  3.0f*SCALE, 5.0f*SCALE, 20.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{- 5.0f*SCALE, 5.0f*SCALE,-15.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
	{{ 10.0f*SCALE,-5.0f*SCALE,-10.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  3.0f*SCALE,-5.0f*SCALE, 20.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{- 5.0f*SCALE,-5.0f*SCALE,-15.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
   },		        	 				         	   
    {		        	 				         	   
	{{ 10.0f*SCALE, 5.0f*SCALE,- 5.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  3.0f*SCALE, 5.0f*SCALE, 10.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{ -8.0f*SCALE, 5.0f*SCALE,-15.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
	{{ 10.0f*SCALE,-5.0f*SCALE,- 5.0f*SCALE,1.0f},  0.2f,1.0f, 0x3f505456},
	{{  3.0f*SCALE,-5.0f*SCALE, 10.0f*SCALE,1.0f},  0.5f,0.0f, 0x3f505456},
	{{ -8.0f*SCALE,-5.0f*SCALE,-15.0f*SCALE,1.0f},  0.8f,1.0f, 0x3f505456},
   },
} ;

/* ポリゴンインデックス情報 */
int BRK_BLD_Index[] = {
#if 0
    3,4,0,1,2,4,5,3,2,0
#else
    0,1,2
#endif
} ;

int BRK_BLD_Time    ;



static void RotateMatrixXY( PIECE *p )
{
    int      r ;
    FMATRIX *out = &p->world ;

    r = p->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, &DG_UnitMatrix,
		       (r-4096*(r>>11))*(float)M_PI/2048.0f ) ;
    r = p->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out,
		       (r-4096*(r>>11))*(float)M_PI/2048.0f ) ;
}

static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    if ( work->piece )
	GV_Free( work->piece ) ;
}

static void Act( Work *work )
{
    int             i, j  ;
    PIECE          *p     ;
    FVECTOR        *pos   ;
    DG_PRIM2_UVRGB *uvs   ;
    float           alpha ;

    DG_SwitchBuffPrim2( work->prim ) ;

    /* 既に解放されていたら消すために起動されている */
    if ( work->life <= 0 )
	    GV_DestroyActor( work ) ;

    /* 普通に落下処理 */
    if ( work->life > 0 )
    {
	work->life-- ;

	uvs = work->prim->uvrgb[work->prim->buffer_clock] ;
	pos = work->prim->pos[work->prim->buffer_clock] ;

	for ( i=work->n_piece, p=work->piece ; --i>=0 ; p++ )
	{
	    if ( p->flag > 1 )
		p->flag-- ;
	    else if ( p->flag )
	    {
		/* 動きを更新 */
		p->pos_v.vy -= BRK_BLD_Gravity[p->type] ;
		p->rot_x = (p->rot_x + p->rot_vx) & 4095 ;
		p->rot_y = (p->rot_y + p->rot_vy) & 4095 ;
		RotateMatrixXY( p ) ;

		_sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.95f ) ;
		_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
		_sceVu0CopyVectorXYZ( (FVECTOR *)p->world.m[W], &p->pos ) ;
	    }

	    alpha = _sceVu0InnerProduct( (FVECTOR *)p->world.m[Z],
					 (FVECTOR *)DG_Chanls->eye.m[Z] ) ;
	    alpha = fpu_Abs( alpha ) * 80.0f ;
	    /* モデルの表示を更新 */
	    for ( j=0 ; j<BRK_BLD_NVERTS ; j++ )
	    {
		POLY *poly = &BRK_BLD_Poly[p->type][BRK_BLD_Index[j]] ;

		_sceVu0ApplyMatrix( pos, &p->world, &poly->shape ) ;
		uvs->a = (short)alpha ;

		pos++ ;
		uvs++ ;
	    }
	}
    }

}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

static int GetResources( Work *work, FVECTOR *pos, DG_DEF *def,
			 int n, int tex_id )
{
    DG_PRIM2 *prim ;
    DG_TEX   *t    ;
    int       i, j ;
    FVECTOR        *p0, *p1 ;
    DG_PRIM2_UVRGB *u0, *u1 ;
    PIECE *p ;
    static FVECTOR force = { -100.0f, 0.0f, 0.0f, 0.0f } ;

    if ( !(work->piece = GV_Malloc( sizeof(PIECE) * n )) )
	PERROR( "No memory (work->piece) : NewGlassDustPrim\n" ) ;

    prim = GM_MakePrim2( (DG_PRIM2_POLY|DG_PRIM2_SHADE|
			  DG_PRIM2_TEX|DG_PRIM2_ALPHA), n, BRK_BLD_NVERTS ) ;
    if ( !prim )
	PERROR( "No memory (work->prim) : NewGlassDustPrim\n" ) ;
    DG_ConfigPrim2Tex( prim, t = DG_GetTexture( tex_id ) ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ) ;

    /* 中心位置 */
    work->prim = prim ;
    work->n_piece = n ;
    p  = work->piece ;
    p0 = prim->pos[0]  , p1 = prim->pos[1]   ;
    u0 = prim->uvrgb[0], u1 = prim->uvrgb[1] ;
    for( j=n ; --j>=0 ; p++ )
    {
	p->flag = 1 ;
	p->type = j&3 ;

	p->rot_x  = 1024 ;
	p->rot_y  = irnd()&2047 ;
	p->rot_vx = irnd()& 127 ;
	p->rot_vy = irnd()& 127 ;

	p->pos.vx = def->ux*frnd() ;
	p->pos.vy = def->uy*frnd() ;
	p->pos.vz = def->uz*frnd() ;
	_sceVu0AddVector( &p->pos, &p->pos, pos ) ;
	_sceVu0ScaleVector( &p->pos_v, &force, rnd() ) ;
	RotateMatrixXY( p ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)p->world.m[W], &p->pos ) ;

	for ( i=0 ; i<BRK_BLD_NVERTS ; i++ )
	{
	    POLY *poly = &BRK_BLD_Poly[p->type][BRK_BLD_Index[i]] ;

	    u0->q = u1->q = 4096 ;
	    u0->f = u1->f = i>1 ? 0x0fff : 0x8fff ;
	    u0->r = u1->r = (poly->rgba >> 0) & 0xff ;
	    u0->g = u1->g = (poly->rgba >> 8) & 0xff ;
	    u0->b = u1->b = (poly->rgba >>16) & 0xff ;
	    u0->a = u1->a = (poly->rgba >>24) & 0xff ;
	    u0->u = u1->u = FTOI12( poly->u * t->u_scale + t->u_offset ) ;
	    u0->v = u1->v = FTOI12( poly->v * t->v_scale + t->v_offset ) ;
	    u0++, u1++ ;

	    _sceVu0ApplyMatrix( p0, &p->world, &poly->shape ) ;
	    _sceVu0CopyVector( p1++, p0++ ) ;
	}
    }
    work->life = 2*60* 5/TIME_BASE ;

    /* 10フレームいないは鳴らさない */
    if ( !BRK_BLD_Time || GM_StagePlayTime-BRK_BLD_Time > 15 )
    {
	BRK_BLD_Time = GM_StagePlayTime ;
	work->se_time = 10 ;
    }

    return 0 ;
}

void *NewBuildGlassPrim( FVECTOR *pos, DG_DEF *def, int n_piece, int tex_id )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	if( GetResources( work, pos, def, n_piece, tex_id ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
