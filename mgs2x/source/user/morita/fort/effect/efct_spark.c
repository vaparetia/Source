//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_spark.c
  フォーチュン戦 電気のショート（床に落ちると照り返す）

  2001/02/08 T.Morita
  $Id: efct_spark.c,v 1.1.1.3 2002/11/19 11:46:11 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    DG_PRIM2 *spark ;
    DG_PRIM2 *blur  ;
    FVECTOR  *vel   ;

    short      life ;
    short      n_prim ;
    int        where ;
    int        hzx_flag ;

    HZD_BOX   *hzd ;
} Work ;



#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_GRAVITY      4
#define BRK_BOUNCE       1.38f
#define BRK_BOUNCE_WALL  1.38f

#define MAX_LIFE    (10*60*5/TIME_BASE)
#define SPARK_SIZE   30.0f
#define BONBORI_SIZE 210.0f


static void MakeBright( FVECTOR *b_pos, DG_PRIM2_UVRGB *b_uvs, int alpha )
{
    int i ;
    static FVECTOR shape[4] = { {-1.0f, 0.0f, 1.0f,1.0f},
				{-1.0f, 0.0f,-1.0f,1.0f},
				{ 1.0f, 0.0f, 1.0f,1.0f},
				{ 1.0f, 0.0f,-1.0f,1.0f} } ;

    for ( i=4 ; --i>=0 ; )
    {
	_sceVu0ScaleVector( &b_pos[i], &shape[i], (float)alpha*BONBORI_SIZE/128.0f ) ;
	_sceVu0AddVector( &b_pos[i], &b_pos[i], &BRK_HZD_HitPos ) ;
	b_pos[i].vw = 1.0f ;
	b_uvs[i].a = alpha/4 ;
    }
}

static int MoveObject( Work *work, FVECTOR *vel, FVECTOR *pos,
		       FVECTOR *b_pos, DG_PRIM2_UVRGB *b_uvs, int alpha )
{
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    static FVECTOR size   = { SPARK_SIZE, SPARK_SIZE, SPARK_SIZE, 0 } ;
    HZD_BOX *get = NULL ;
    int flag = 0 ;

    vel->vy -= BRK_GRAVITY ;
    if ( !work->hzx_flag )
	flag = BRK_CheckHazard( work->hzd, pos, vel, &bounce, &size ) & 1 ;
    if ( flag )
    {
	vel->vx *= 0.9f ;
	vel->vz *= 0.9f ;
	if ( !(int)vel->vx && !(int)vel->vz && !(int)(vel->vy/BRK_GRAVITY) )
	    flag = 1 ;
	else
	    flag = 0 ;
    }
    else
	_sceVu0AddVector( pos, pos, vel ) ;
    if ( !work->hzx_flag )
	get = BRK_UTL_GetCenterHazard( work->hzd, pos, NULL, 500.0f, 0.0f ) ;
    if ( get )
	alpha = (BRK_HZD_HitDist<=0.0f ? alpha :
		 BRK_HZD_HitDist>=1.0f ? 0     :
		 (int)((1.0f - BRK_HZD_HitDist)*(float)alpha)) ;
    else
	alpha = 0 ;
    MakeBright( b_pos, b_uvs, alpha ) ;

    return flag ;
}


static void Act( Work *work )
{
    DG_PRIM2         *s, *b ;
    FVECTOR          *nxt_sp, *prv_sp, *nxt_bp, *prv_bp ;
    DG_PRIM2_UVRGB   *nxt_bu, *prv_bu ;
    DG_PRIM2_UVRGBWH *nxt_su, *prv_su ;
    FVECTOR          *vel ;
    int i, flag=0, alpha ;


    s = work->spark ;
    b = work->blur  ;
    prv_sp = s->pos[s->buffer_clock] ; prv_su = s->uvrgb[s->buffer_clock] ;
    prv_bp = b->pos[b->buffer_clock] ; prv_bu = b->uvrgb[b->buffer_clock] ;
    DG_SwitchBuffPrim2( s ) ;
    DG_SwitchBuffPrim2( b ) ;
    nxt_sp = s->pos[s->buffer_clock] ; nxt_su = s->uvrgb[s->buffer_clock] ;
    nxt_bp = b->pos[b->buffer_clock] ; nxt_bu = b->uvrgb[b->buffer_clock] ;
    vel = work->vel ;

    if ( work->hzx_flag > 0 )
	 work->hzx_flag-- ;
    for ( i=work->n_prim ; --i>=0 ; )
    {
	if ( vel->vw > 0.0f )
	{
	    flag++ ;
	    vel->vw -= 1.0f ;
	}
	else if ( vel->vw < 0.0f )
	{
	    flag += prv_su->a ;
	    if ( (nxt_su->a = prv_su->a) )
	    {
		_sceVu0CopyVector( nxt_sp, prv_sp ) ;
		_sceVu0CopyVector( nxt_bp, prv_bp ) ;
		alpha = prv_su->a - (irnd()& 15) ;
		alpha = alpha > 0 ? alpha : 0 ;
		nxt_su->a = alpha ;
		nxt_bu[0].a = nxt_bu[1].a = nxt_bu[2].a = nxt_bu[3].a = alpha / 4 ;
	    }
	}
	else
	{
	    flag++ ;
	    _sceVu0CopyVector( nxt_sp, prv_sp ) ;
	    nxt_su->a = (irnd() & 127) | 128 ;
	    if ( MoveObject( work, vel, nxt_sp, nxt_bp, nxt_bu, nxt_su->a ) )
		vel->vw = -1.0f ;
	}
	nxt_sp++,  prv_sp++  ;
	nxt_su++,  prv_su++  ;
	nxt_bp+=4, prv_bp+=4 ;
	nxt_bu+=4, prv_bu+=4 ;
	vel++ ;
    }

    if ( work->life > 0 )
	work->life-- ;
    if ( !flag || !work->life )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->spark )
	GM_FreePrim2( work->spark ) ;
    if ( work->blur  )
	GM_FreePrim2( work->blur  ) ;
    if ( work->vel )
	GV_Free( work->vel ) ;
}

static int GetResources( Work *work, FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd, int mode )
{
    int i ;
    static int n_prim[] = { 8, 16, 6, } ;
    static int color[] = {0x007f7f7f, 0x007f7f7f, 0x007f7f7f } ;
    float d ;

    work->hzx_flag = 15 ;
    work->life   = MAX_LIFE     ;
    work->hzd    = hzd          ;
    work->n_prim = n_prim[mode] ;
    if ( !(work->spark = BRK_UTL_MakeSPRTWH( work->n_prim,
					     GV_StrCode( "svc_bonbori" ),
					     SCE_GS_SET_ALPHA(0,2,0,1,0),
					     SPARK_SIZE,
					     color[mode] )) )
	PERROR( "No Prim(no memory) : NewFortSpark\n" ) ;
    if ( !(work->blur = BRK_UTL_MakePOLY( work->n_prim,
					  GV_StrCode( "drop01_alp2_mod1021" ),
					  SCE_GS_SET_ALPHA(1,2,0,1,0),
					  0x00ffffff )) )
	PERROR( "No Prim(no memory) : NewFortSpark\n" ) ;
    if ( !(work->vel = GV_Malloc( work->n_prim * sizeof(FVECTOR) ) ) )
	PERROR( "Cannot GV_Malloc(no memory) : NewFortSpark\n" ) ;

    d = sceVu0Sqrt( _sceVu0InnerProduct( dir, dir ) ) ;
    for ( i=work->n_prim ; --i>=0 ; )
    {
	_sceVu0CopyVector( &work->vel[i], dir ) ;
	switch( mode )
	{
	case 0:
	    work->vel[i].vx += frnd()*0.3f*d ;
	    work->vel[i].vy += frnd()*0.3f*d ;
	    work->vel[i].vz += frnd()*0.3f*d ;
	    work->vel[i].vw =  (float)(irnd()&7) ;
	    break ;
	case 1:
	    work->vel[i].vx *= 1.0f + frnd()*0.3f ;
	    work->vel[i].vy *= 1.0f + frnd()*0.3f ;
	    work->vel[i].vz *= 1.0f + frnd()*0.3f ;
	    work->vel[i].vw =  (float)(i*((irnd()&3)+1)) ;
	    break ;
	case 2:
	    work->vel[i].vx = frnd()*5.0f ;
	    work->vel[i].vy =  rnd()*5.0f ;
	    work->vel[i].vz = frnd()*5.0f ;
	    work->vel[i].vw =  (float)((i/4)*((irnd()&7)+8)) ;
	    break ;
	}
	_sceVu0CopyVector( &work->spark->pos[0][i], pos ) ;
	_sceVu0CopyVector( &work->spark->pos[1][i], pos ) ;
    }

    return 0 ;
}

void *NewFortSpark( FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd, int mode )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, pos, dir, hzd, mode  ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
