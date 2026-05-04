//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  demo_bullet.c
  デモ用銃弾

  2000/04/18 T.Morita
  $Id: efct_bullet.c,v 1.1.1.3 2002/11/19 11:46:09 Yoshizawa1 Exp $
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

#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define DEMO_BUL_RED    128
#define DEMO_BUL_GREEN  128
#define DEMO_BUL_BLUE   128

#define DEMO_BUL_N_TRAIL (64*4)
#define DEMO_BUL_SIZE     4096

#define DEMO_BUL_AGE     1


typedef struct work_t
{
    GV_ACT_EX actor  ;

    FMATRIX   world  ;
    FVECTOR   pos    ; /* 弾丸位置     */
    FVECTOR   pos_v  ; /* 弾丸速度     */
    FVECTOR   avoid  ;
    float     rot_y,  rot_vy ;/*弾丸ヨー回転速度*/

    float     speed  ;
    int       flag   ;

    FVECTOR   vel[DEMO_BUL_N_TRAIL] ;
    char      rot  [DEMO_BUL_N_TRAIL] ;
    char      size [DEMO_BUL_N_TRAIL] ;
    char      alpha[DEMO_BUL_N_TRAIL] ;
    int       life ;

    DG_OBJS  *objs   ;
    DG_PRIM2 *trail  ;
} Work ;


static void Die( Work *work )
{
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    if ( work->trail )
	GM_FreePrim2( work->trail ) ;
}


static short DEMO_BUL_Cos[64] ;
#define Cos(s) DEMO_BUL_Cos[ (s)    &63]
#define Sin(s) DEMO_BUL_Cos[((s)+32)&63]


static void Act( Work *work )
{
    int i ;
    DG_PRIM2_UVRGBWH *nxt_u, *prv_u ;
    FVECTOR          *nxt_p, *prv_p ;
    FMATRIX mtx ;
    FVECTOR a ;
    float   d ;
    static FVECTOR  v[] = {
	{ 5.0f, 0.0f, 0.0f, 1.0f },{-5.0f, 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 5.0f, 1.0f },{ 0.0f, 0.0f,-5.0f, 1.0f },
    } ;

    /*弾がそれる処理*/
    a.vx = work->pos.vx - work->avoid.vx ;
    a.vz = work->pos.vz - work->avoid.vz ;
    d   = a.vx*a.vx + a.vz*a.vz ;
    if ( d < 2450.0f*2450.0f )
    {
	d = work->speed / sceVu0Sqrt( d ) ;
	a.vx *= d ;
	a.vz *= d ;
	work->pos_v.vx += work->pos_v.vz * (a.vx + work->pos_v.vx>0.0f ? -1.0f : 1.0f)*0.01f ;
	work->pos_v.vz -= work->pos_v.vx * (a.vx + work->pos_v.vx>0.0f ? -1.0f : 1.0f)*0.01f ;
	_sceVu0Normalize( &work->pos_v, &work->pos_v ) ;
	BIG_VectoMat( &work->world, &work->pos_v, &DG_ZeroVector, 0x0012 ) ;
	_sceVu0ScaleVector( &work->pos_v, &work->pos_v, work->speed ) ;
    }

    /*弾丸の移動及び回転*/
    if ( (work->rot_y += work->rot_vy) > (float)M_PI )
        work->rot_y -= 2.0f*(float)M_PI ;
    _sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, work->rot_y ) ;
    _sceVu0MulMatrix( &mtx, &work->world, &mtx ) ;
    _sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx.m[W], &work->pos ) ;

    if ( work->objs )
	_sceVu0CopyMatrix( &work->objs->world, &mtx ) ;

    /*後の空気*/
    DG_SwitchBuffPrim2( work->trail ) ;
    i = work->trail->buffer_clock ;
    nxt_u = work->trail->uvrgb[i  ] ;
    prv_u = work->trail->uvrgb[i^1] ;
    nxt_p = work->trail->pos[i  ] ;
    prv_p = work->trail->pos[i^1] ;
    for ( i=DEMO_BUL_N_TRAIL ; --i>=0 ;  )
    {
	if ( work->alpha[i] )
	{
	    work->rot[i]++ ;
	    work->alpha[i]-- ;
	    work->size[i]++ ;
	    _sceVu0AddVector( &nxt_p[i], &prv_p[i], &work->vel[i]  ) ;
	}
	else
	{
	    /*発生位置を45°の幅で回転させ分散させる*/
	    work->rot[i] = (irnd()&31)-16 ;
	    nxt_p[i] = v[i&3] ;
	    nxt_p[i].vx = (v[i&3].vx*Cos( work->rot[i] ) - v[i&3].vz*Sin( work->rot[i] ))/4096 ;
	    nxt_p[i].vz = (v[i&3].vx*Sin( work->rot[i] ) + v[i&3].vz*Cos( work->rot[i] ))/4096 ;
	    _sceVu0ApplyMatrix( &nxt_p[i], &mtx, &nxt_p[i] ) ;
	    _sceVu0SubVector( &work->vel[i], &nxt_p[i], &work->pos ) ;
	    _sceVu0ScaleVector( &work->vel[i], &work->vel[i], 0.125f ) ;
	    work->size[i]  = 1 ;
	    work->alpha[i] = 32 ;
	}
	nxt_u[i].a = work->alpha[i]/2 ;
	nxt_u[i].w = work->size[i] * Cos( work->rot[i] ) / 4096/2 ;
	nxt_u[i].h = work->size[i] * Sin( work->rot[i] ) / 4096/2 ;
    }

    if ( work->life )
	work->life-- ;
    else
	GV_DestroyActor( work ) ;
}

static void DEMO_BUL_InitCosine()
{
    int i ;

    for ( i=0 ; i<64 ; i++ )
	DEMO_BUL_Cos[i] = (short)(DEMO_BUL_SIZE * cosf( i*M_PI/32.0f )) ;
}

static int GetResourcesCall( Work *work, int id, float rot_vy, float speed,
			     FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid, int life )
{
    DG_DEF  *def ;
    FMATRIX  mtx ;
    int      i   ;
    extern void BIG_VectoMat( FMATRIX *, FVECTOR *,FVECTOR *, int ) ;
    extern void UTL_VectoMat( FMATRIX *, FVECTOR *,FVECTOR *, int ) ;

    if ( id )
    {
	if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	    PERROR( "No KMS-FILE<%d> in data.cnf : NewFortBulletCalled\n", id ) ;
	if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Cannot create DG_OBJS(Maybe no memory) :: NewFortBulletCalled\n" ) ;
	DG_QueueObjs( work->objs ) ;
    }

    work->life = life ;
    work->speed  = speed ;
    work->rot_vy = rot_vy / 180.0f * (float)M_PI ;

    _sceVu0CopyVector( &work->avoid, avoid ) ;
    _sceVu0CopyVector( &work->pos, pos ) ;
    work->pos.vw = 1.0f ;
    _sceVu0Normalize( &work->pos_v, pos_v ) ;
    BIG_VectoMat( &work->world, &work->pos_v, &DG_ZeroVector, 0x0012 ) ;
    _sceVu0RotMatrixX( &mtx, &DG_UnitMatrix, M_PI ) ;
    _sceVu0MulMatrix( &work->world, &work->world, &mtx ) ;
    _sceVu0ScaleVector( &work->pos_v, &work->pos_v, speed ) ;

    DEMO_BUL_InitCosine() ;

    if ( !(work->trail = BRK_UTL_MakeRSPRTWH( DEMO_BUL_N_TRAIL,
					      GV_StrCode( "bombpowder1_msk" ),
					      SCE_GS_SET_ALPHA(0,2,0,1,0),
					      0.0f,
					      0x007f7f7f )) )
	PERROR( "Cannot create DG_PRIM2 (May be no memroy) :: NewFortBulletCalled\n" ) ;

    for ( i=DEMO_BUL_N_TRAIL ; --i>=0 ; )
    {
	_sceVu0CopyVector( &work->trail->pos[0][i], &work->pos ) ;
	_sceVu0CopyVector( &work->trail->pos[1][i], &work->pos ) ;
	work->alpha[i] = (i/4) ;
    }

    return 0 ;
}

void *NewFortBulletCalled( int id, float rot_vy, float speed,
			 FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid, int life )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResourcesCall( work, id, rot_vy, speed, pos, pos_v, avoid, life ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
