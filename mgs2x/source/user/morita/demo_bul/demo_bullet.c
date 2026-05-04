//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  demo_bullet.c
  デモ用銃弾

   2000/04/18 T.Morita
   $Id: demo_bullet.c,v 1.1.1.3 2002/11/19 11:45:55 Yoshizawa1 Exp $
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


#include "demo_bullet.h"

#if DEBUG_MODE
int DEMO_BUL_Number = 6 ;
#endif

static void Die( Work *work )
{
#if DEBUG_MODE
    DEMO_BUL_Number = 6 ;
#endif

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
    FVECTOR *nxt_p, *prv_p ;
    FMATRIX mtx ;
    static FVECTOR  v[] = {
	{ 5.0f, 0.0f, 0.0f, 1.0f },{-5.0f, 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 5.0f, 1.0f },{ 0.0f, 0.0f,-5.0f, 1.0f },
    } ;

    /*弾丸の移動及び回転*/
    if ( (work->rot_y += work->rot_vy) > (float)M_PI )
        work->rot_y -= 2.0f*(float)M_PI ;
    _sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, work->rot_y ) ;
    _sceVu0MulMatrix( &mtx, &work->world, &mtx ) ;
    _sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
    TransMatrix( &mtx, &work->pos ) ;

    if ( work->objs )
	work->objs->world = mtx ;

    /*後の空気*/
    DG_SwitchBuffPrim2( work->trail ) ;
    i = work->trail->buffer_clock ;
    nxt_u = work->trail->uvrgb[i  ] ;
    prv_u = work->trail->uvrgb[i^1] ;
    nxt_p = work->trail->pos[i  ] ;
    prv_p = work->trail->pos[i^1] ;
    for ( i=DEMO_BUL_N_TRAIL ; --i>=0 ;  )
    {
	if ( work->alpha[i] < 0 )
	    work->alpha[i]++ ;
	else if ( work->alpha[i] > 0 )
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
	    _sceVu0ScaleVector( &work->vel[i], &work->vel[i], /*0.125f*/0.30f ) ;
	    work->size[i]  =  1 ;
	    work->alpha[i] = 64 ;
	}
	nxt_u[i].a = work->alpha[i]>0 ? work->alpha[i]/2 : 0 ;
	nxt_u[i].w = work->size[i] * Cos( work->rot[i] ) / 4096 /* /2 */;
	nxt_u[i].h = work->size[i] * Sin( work->rot[i] ) / 4096 /* /2 */;
    }
#if 0

    if ( work->flag >=0 )
	if ( !(--work->flag) )
	{
	    void *NewDemoBulletCall( int id, float rot_vy, float speed,
				     FVECTOR *pos, FVECTOR *pos_v, int flag ) ;

	    GV_SetActorChild( work,
			      NewDemoBulletCall( 0, work->rot_vy * 180.0f / (float)M_PI,
						 work->speed, &work->from, &work->to, 0 ) ) ;
	}
#endif
}

static void DEMO_BUL_InitCosine()
{
    int i ;

    for ( i=0 ; i<64 ; i++ )
	DEMO_BUL_Cos[i] = (int)(DEMO_BUL_SIZE * cosf( i*M_PI/32.0f )) ;
}

static void DEMO_BUL_InitPacket( DG_PRIM2 *p, int tex_id, int alpha, int i )
{
    DG_PRIM2_UVRGBWH *u0, *u1 ;     /* スプライト用 */
    DG_TEX *t ;
    FVECTOR *p0, *p1 ;

    DG_ConfigPrim2Tex( p, t=DG_GetTexture( tex_id ) ) ;
    DG_SetPrim2Alpha( p, alpha ) ;

    u0 = p->uvrgb[0] ;
    u1 = p->uvrgb[1] ;
    p0 = p->pos  [0] ;
    p1 = p->pos  [1] ;
    while( --i>=0 )
    {
#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
        u0->u0 = u1->u0 = FTOI12( 0.0F * t->u_scale + t->u_offset ) ;/* 左上 */
        u0->v0 = u1->v0 = FTOI12( 0.0F * t->v_scale + t->v_offset ) ;/* 左上 */
        u0->u1 = u1->u1 = FTOI12( 1.0F * t->u_scale + t->u_offset ) ;/* 右下 */
        u0->v1 = u1->v1 = FTOI12( 1.0F * t->v_scale + t->v_offset ) ;/* 右下 */
        u0->q0 = u1->q0 = 4096 ;
        u0->q1 = u1->q1 = 4096 ;
        u0->f0 = u1->f0 = 0x0fff ;
        u0->f1 = u1->f1 = 0x0fff ;

        u0->w = u1->w = 200 ;
        u0->h = u1->h = 200 ;

        u0->r = u1->r = DEMO_BUL_RED   ;
        u0->g = u1->g = DEMO_BUL_GREEN ;
        u0->b = u1->b = DEMO_BUL_BLUE  ;
        u0->a = u1->a = 0 ;
	u0++, u1++ ;

	_sceVu0CopyVector( p0, &DG_ZeroVector ) ;
	_sceVu0CopyVector( p1, &DG_ZeroVector ) ;
	p0++, p1++ ;
    }
}

static int DEMO_BUL_InitPrimitive( Work *work )
{
    int i ;

    if ( !(work->trail = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				       DEMO_BUL_N_TRAIL, 1 )) )
	PERROR( "Cannot create DG_PRIM2 (May be no memroy) :: NewDemoBullet\n" ) ;

    DEMO_BUL_InitPacket( work->trail,
			 GV_StrCode( "bombpowder1_msk" ),
			 SCE_GS_SET_ALPHA( 0,2,0,1,0 ),
			 DEMO_BUL_N_TRAIL ) ;
    for ( i=DEMO_BUL_N_TRAIL ; --i>=0 ; )
	work->alpha[i] = -(i/4) ;
    return  0 ;
}

static int GetResourcesCall( Work *work, int id, float rot_vy, float speed,
			     FVECTOR *pos, FVECTOR *pos_v, int flag )
{
    DG_DEF  *def ;
    FMATRIX  mtx ;
    extern void BIG_VectoMat( FMATRIX *, FVECTOR *,FVECTOR *, int ) ;
    extern void UTL_VectoMat( FMATRIX *, FVECTOR *,FVECTOR *, int ) ;

#if 0
    if ( DEMO_BUL_Number == 6 )
	flag = 1, work->flag = 8 ;
    else if ( DEMO_BUL_Number > 0 )
	flag = 0, work->flag = 8 ;
    else
	flag = 0, work->flag = -1 ;
    DEMO_BUL_Number-- ;
#endif
printf( "%.1f p(%.1f %.1f %.1f) v(%.1f %.1f %.1f) : NewDemoBullet\n",
speed, pos->vx,pos->vy,pos->vz,
 pos_v->vx,pos_v->vy,pos_v->vz ) ;
    if ( flag )
    {
	if ( !id )
	    PERROR( "No model ID (-m option missing) :: NewDemoBullet\n" ) ;
	if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	    PERROR( "No KMS-FILE in data.cnf :: NewDemoBullet\n" ) ;
	if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Cannot create DG_OBJS(Maybe no memory) :: NewDemoBullet\n" ) ;
	DG_QueueObjs( work->objs ) ;
    }

    work->speed  = speed ;
    work->rot_vy = rot_vy / 180.0f * (float)M_PI ;
    work->from   = *pos   ;
    work->to     = *pos_v ;
    work->pos    = *pos   ;
    _sceVu0SubVector( &work->pos_v, pos_v, pos ) ;
    _sceVu0Normalize( &work->pos_v, &work->pos_v ) ;
    BIG_VectoMat( &work->world, &work->pos_v, &DG_ZeroVector, 0x0012 ) ;
    _sceVu0RotMatrixX( &mtx, &DG_UnitMatrix, M_PI ) ;
    _sceVu0MulMatrix( &work->world, &work->world, &mtx ) ;
    _sceVu0ScaleVector( &work->pos_v, &work->pos_v, speed ) ;

    DEMO_BUL_InitCosine() ;
    if ( DEMO_BUL_InitPrimitive( work ) )
	return -1 ;
    return 0 ;
}


static int GetResources( Work *work )
{
    int      buf[3] ;
    FVECTOR  pos, pos_v ;

    if ( GCL_GetOption( 'd' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &pos_v ) ;
    }
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
    }
    if ( GetResourcesCall( work,
			   GCL_GetOptionValue( 'm', 0 ),
			  (float) GCL_GetOptionValue( 'r', 0 ),
			  (float) GCL_GetOptionValue( 's', 0 ),
			   &pos, &pos_v,
			   GCL_GetOptionValue( 'f', 0 )  ) )
	PERROR( "Cannot get resources :: NewDemoBullet\n" ) ;

    return 0 ;
}

void *NewDemoBulletCall( int id, float rot_vy, float speed,
			 FVECTOR *pos, FVECTOR *pos_v, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResourcesCall( work, id, rot_vy, speed, pos, pos_v, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewDemoBullet( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResources( work ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
