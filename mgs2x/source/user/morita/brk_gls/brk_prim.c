//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_piece.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_prim.c,v 1.1.1.3 2002/11/19 11:45:30 Yoshizawa1 Exp $
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

#define BRK_PCE_SPREAD 30.0f
#define BRK_PCE_INACTIVE 0x8000

typedef struct
{
    FMATRIX  world ;
    FVECTOR  pos   ; /* 位置     */
    FVECTOR  pos_v ; /* 速度     */
    short    rot_x,  rot_y ;
    short    rot_vx, rot_vy ;

    FVECTOR *shape ;
    HZD_BOX *hzd   ;

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
    int          se_id   ;
} Work ;

#define BRK_PCE_NVERTS 10

typedef struct poly_t POLY ;
struct poly_t
{
    FVECTOR shape ;
    float   u,v ;
    int     rgba ;
} ;

static POLY BRK_PCE_Poly[][6] = {
    {
	{  { 35.0f, 5.0f,-50.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  5.0f, 5.0f, 50.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  {-35.0f, 5.0f,-50.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
	{  { 35.0f,-5.0f,-50.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  5.0f,-5.0f, 50.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  {-35.0f,-5.0f,-50.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
    },		        	 
    {		        	 
	{  { 30.0f, 5.0f,-30.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  3.0f, 5.0f, 30.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  {-15.0f, 5.0f,-30.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
	{  { 30.0f,-5.0f,-30.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  3.0f,-5.0f, 30.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  {-15.0f,-5.0f,-30.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
    },		        	 
    {		        	 
	{  { 10.0f, 5.0f,-10.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  3.0f, 5.0f, 20.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  {- 5.0f, 5.0f,-15.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
	{  { 10.0f,-5.0f,-10.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  3.0f,-5.0f, 20.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  {- 5.0f,-5.0f,-15.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
    },		        	 
    {		        	 
	{  { 10.0f, 5.0f,- 5.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  3.0f, 5.0f, 10.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  { -8.0f, 5.0f,-15.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
	{  { 10.0f,-5.0f,- 5.0f,1.0f},  0.2f,1.0f, 0x1f53564f },
	{  {  3.0f,-5.0f, 10.0f,1.0f},  0.5f,0.0f, 0x1f53564f },
	{  { -8.0f,-5.0f,-15.0f,1.0f},  0.8f,1.0f, 0x1f53564f },
    },
} ;

int BRK_PCE_Index[BRK_PCE_NVERTS] = {
    3,4,0,1,2,4,5,3,2,0
} ;

int BRK_PCE_nModels ;
int BRK_PCE_Time    ;



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
    int     i, j ;
    int     flag=0 ;
    PIECE  *p ;
    FVECTOR v, *pos ;
    DG_PRIM2_UVRGB *cur, *prv ;
    static FVECTOR Size    = { 10.0f, 10.0f, 10.0f, 0.0f } ;
    static FVECTOR Bounce  = { 1.48f, 1.48f, 1.48f, 0.0f } ;

    if ( !(work->n_piece & BRK_PCE_INACTIVE) )
    {
       //AR_PARTICLE_HALF
       if( !DG_SwitchBuffPrim2( work->prim ) )
       {
          return;
       }

	/* 既に解放されていたら消すために起動されている */
	if ( !work->piece )
	{
	    cur = work->prim->uvrgb[work->prim->buffer_clock] ;
	    prv = work->prim->uvrgb[work->prim->buffer_clock ^ 1] ;
	    if ( !prv->a )
		GV_DestroyActor( work ) ;
	    else
		for ( i=work->n_piece*BRK_PCE_NVERTS ; --i>=0 ; cur++ )
		    cur->a = prv->a - 1 ;
	}

	/* 普通に落下処理 */
	else
	{
	    pos = work->prim->pos[work->prim->buffer_clock] ;
	    
	    for ( i=work->n_piece, p=work->piece ; --i>=0 ; flag|=p->flag, p++ )
	    {
		if ( p->flag > 1 )
		    p->flag-- ;
		else if ( p->flag )
		{
		    /* 動きを更新 */
		    p->pos_v.vy -= BRK_PIECE_GRAVITY ;
		    p->rot_x = (p->rot_x + p->rot_vx) & 4095 ;
		    p->rot_y = (p->rot_y + p->rot_vy) & 4095 ;
		    RotateMatrixXY( p ) ;

		    /* 当たりチェック */
		    switch ( BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v,
					      &Bounce, &Size ) )
		    {
		    case 1:
		    case 3:
			if ( work->se_time && work->se_id )
			{
			    work->se_time = 0 ;
			    GM_SeSetMode( work->se_id, &p->pos,
					  GM_SEMODE_BOMB ) ;
			}

			_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
			_sceVu0ScaleVector( &v, &p->pos_v, 0.125f ) ;
			if ( !(int)v.vx && !(int)v.vy && !(int)v.vz )
			{
			    p->pos_v.vx = p->pos_v.vy = p->pos_v.vz = 0.0f ;
			    p->rot_vy = p->rot_vy * 15 /16 ;
			    if ( !p->rot_vx && !p->rot_vy )
				p->flag = 0 ;
			    else
				p->rot_vx = ( (p->rot_x>3072 ? 4096 :
					       p->rot_x>1024 ? 2048 :
					       0) - p->rot_x)/2 ;
			}
			break ;
		    case 2:
			p->pos_v.vx += BRK_PCE_SPREAD * 0.5f * frnd() ;
			p->pos_v.vz += BRK_PCE_SPREAD * 0.5f * frnd() ;
			break ;
		    case 0:
			_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
		    }
		    _sceVu0CopyVectorXYZ( (FVECTOR *)p->world.m[W], &p->pos ) ;
		}

		/* モデルの表示を更新 */
		for ( j=0 ; j<BRK_PCE_NVERTS ; j++ )
		    _sceVu0ApplyMatrix( pos++, &p->world,
					&BRK_PCE_Poly[p->type][BRK_PCE_Index[j]].shape ) ;
	    }

	    /* もう動かないので動きのためのメモリは,解放しておく(もう
               ２度とここには来ないために解放した後NULLを入れる) */
	    if ( !flag )
	    {
		work->n_piece |= BRK_PCE_INACTIVE ;
		GV_Free( work->piece ), work->piece = NULL ;
	    }
	}
    }

    /* 全体的に数が多く表示されていたら消す */
    else if ( BRK_PCE_nModels > 150 )
    {
	work->n_piece &= ~BRK_PCE_INACTIVE ;
	BRK_PCE_nModels -= work->n_piece ;
    }
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

static int GetResources( Work *work, HZD_BOX *hzd, FMATRIX *world,
			 FVECTOR *v0, FVECTOR *v1, FVECTOR *v2,
			 FVECTOR *hit, FVECTOR *frc,
			 int n, int tex_id, int se_id )
{
    DG_PRIM2 *prim ;
    DG_TEX   *t    ;
    int       i, j ;
    FVECTOR         v ;
    FVECTOR        *p0, *p1 ;
    DG_PRIM2_UVRGB *u0, *u1 ;
    PIECE *p ;
    float  scale ;
    FVECTOR shape[3], vr, hr ;

    if ( !(work->piece = GV_Malloc( sizeof(PIECE) * n )) )
	PERROR( "No memory (work->piece) : NewGlassDustPrim\n" ) ;

    prim = GM_MakePrim2( (DG_PRIM2_POLY|DG_PRIM2_SHADE|
			  DG_PRIM2_TEX|DG_PRIM2_ALPHA), n, BRK_PCE_NVERTS ) ;
    if ( !prim )
	PERROR( "No memory (work->prim) : NewGlassDustPrim\n" ) ;
    DG_ConfigPrim2Tex( prim, t = DG_GetTexture( tex_id ) ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ) ;
    prim->flag |= DG_PRIM2_INVISIBLE1 ;


    /* 位置を計算するための垂直＆水平ベクトル */
    _sceVu0ApplyMatrix( &shape[0], world, v0 ) ;
    _sceVu0ApplyMatrix( &shape[1], world, v1 ) ;
    _sceVu0ApplyMatrix( &shape[2], world, v2 ) ;
    _sceVu0SubVector( &vr, &shape[0], &shape[1] ) ;
    _sceVu0SubVector( &hr, &shape[2], &shape[1] ) ;
    /* 中心位置 */
    _sceVu0AddVector( &work->center, &hr, &vr ) ;
    _sceVu0ScaleVector( &work->center, &work->center, 0.5f ) ;
    _sceVu0AddVector( &work->center, &work->center, &shape[1] ) ;

    work->prim = prim ;
    work->n_piece = n ;
    p  = work->piece ;
    p0 = prim->pos[0]  , p1 = prim->pos[1]   ;
    u0 = prim->uvrgb[0], u1 = prim->uvrgb[1] ;
    for( j=n ; --j>=0 ; p++ )
    {
	p->hzd    = hzd ;
	p->rot_x  = 1024 ;
	p->rot_y  = irnd()&2047 ;
	p->rot_vx = irnd()& 511 ;
	p->rot_vy = irnd()& 511 ;

#if 0
	scale = BRK_PCE_SPREAD * frnd() ;
	_sceVu0ScaleVector( &p->pos_v, (FVECTOR *)&world->m[X], scale ) ;
	scale = BRK_PCE_SPREAD * frnd() ;
	_sceVu0ScaleVector( &v       , (FVECTOR *)&world->m[Y], scale ) ;
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;

	_sceVu0ScaleVector( &p->pos, &p->pos_v, 10.0f ) ;
	_sceVu0AddVector( &p->pos, &p->pos, hit ) ;
	p->pos.vw = 1.0f ;

	scale = (frc->vz>0.0f ? 1.0f : -1.0f) * BRK_PCE_SPREAD * (rnd()+1.0f) ;
	_sceVu0ScaleVector( &v       , (FVECTOR *)&world->m[Z], scale ) ;
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
	p->flag = 1 ;
#else

	scale = rnd() ;
	_sceVu0ScaleVector( &p->pos, &vr, scale ) ;
	scale = (1.0f - scale) * rnd() ;
	_sceVu0ScaleVector( &v     , &hr, scale ) ;
	_sceVu0AddVector( &p->pos, &p->pos, &v ) ;
	_sceVu0AddVector( &p->pos, &p->pos, &shape[1] ) ;

	_sceVu0SubVector( &v, &p->pos, hit ) ;
	scale = _sceVu0InnerProduct( &v, &v ) ;
	_sceVu0ScaleVector( &v, &v, 50.0f*50.0f/scale*rnd() ) ;
	scale = sceVu0Sqrt( scale )/50.0f ;

	p->flag = 1/* + scale*/ ;

	/* 破片速度の初期化 */
	scale = rnd()+0.5f /*-  scale/4.0f*/ ;
	scale = (frc->vz>0.0f ? 1.0f : -1.0f) * BRK_PCE_SPREAD * 2.0f* (scale > 0.0f ? scale : 0.0f) ;
	_sceVu0ScaleVector( &p->pos_v, (FVECTOR *)&world->m[Z], scale ) ;
	p->pos_v.vx += 5.0f*frnd() ;
	p->pos_v.vy += 5.0f*frnd() ;
	p->pos_v.vz += 5.0f*frnd() ;
#if 0
	scale = BRK_PCE_SPREAD * frnd() ;
	_sceVu0ScaleVector( &v, (FVECTOR *)&world->m[X], scale ) ;
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
	scale = BRK_PCE_SPREAD * frnd() ;
	_sceVu0ScaleVector( &v, (FVECTOR *)&world->m[Y], scale ) ;
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
#endif
	_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;

#endif
	p->type = j&3 ;

	RotateMatrixXY( p ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)p->world.m[W], &p->pos ) ;

	for ( i=0 ; i<BRK_PCE_NVERTS ; i++ )
	{
	    POLY *poly = &BRK_PCE_Poly[p->type][BRK_PCE_Index[i]] ;

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
    BRK_PCE_nModels += work->n_piece ;

    work->se_id = se_id > 0 ? se_id : 0 ;

    /* 10フレームいないは鳴らさない */
    if ( !BRK_PCE_Time || GM_StagePlayTime-BRK_PCE_Time > 15 )
    {
	BRK_PCE_Time = GM_StagePlayTime ;
	work->se_time = 30 ;
    }
    else
	work->se_time = 0 ;

    return 0 ;
}

void *NewGlassDustPrim( HZD_BOX *hzd, FMATRIX *world,
			FVECTOR *p0, FVECTOR *p1, FVECTOR *p2,
			FVECTOR *hit, FVECTOR *frc,
			int n_piece, int tex_id, int se_id )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	if( GetResources( work, hzd, world,
			  p0, p1, p2,
			  hit, frc, n_piece,
			  tex_id, se_id ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
