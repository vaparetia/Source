//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_plasma.c
  プラズマ

  2001/06/12 T. Morita
  $Id: efct_plasma.c,v 1.1.1.3 2002/11/19 11:45:26 Yoshizawa1 Exp $
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

#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

void *NewBreakComputerPlasma( FVECTOR *pos, FVECTOR *conf, int life ) ;


/* user/source/kunibe/effect */
extern void *NewSpritePlasma2( FVECTOR *from,   /* 開始点*/
			       FVECTOR *middle, /* 中継点*/
			       FVECTOR *to,	/* 終端点*/
			       float    width,  /* 幅*/    
			       FVECTOR *color,  /* 色ベクトル  r,g,b,a */
			       int      life ); /* 寿命 */ 
     /*
       このプラズマは生きている間
       fromとtoを参照しています。
       fromとtoを動かすことで動きを付けたりもできます。
       参照しないタイプがいい場合は知らせてください。
       フラグ分けします。

       あとこのプラズマは小さいエリア用に作成
       したものなのであまり大きいエリアに表示しないでください。
       スプライト数の関係でプラズマが切れてしまう場合があります。*/


typedef struct work_t
{
    GV_ACT_EX actor ;

    FVECTOR   from[3] ;
    FVECTOR   to  [3] ;

    FVECTOR   norm ;

    FVECTOR  *conf ;
    FVECTOR   pos  ;
    int       count ;
    int       life ;
} Work ;


static void CalcNormalFromVertex( FVECTOR *norm, FVECTOR *verts )
{
    FVECTOR a0, a1 ;

    _sceVu0SubVector( &a0, verts+1, verts   ) ;
    _sceVu0SubVector( &a1, verts+2, verts+1 ) ;
    _sceVu0OuterProduct( norm, &a0, &a1 ) ;
    _sceVu0Normalize( norm, norm ) ;
}

static void Act( Work *work )
{
    int i ;

    for ( i=3 ; --i>=0 ; )
    {
	work->to[i].vx   += frnd()*5.0f ;
	work->to[i].vz   += frnd()*5.0f ;
	work->from[i].vx += frnd()*5.0f ;
	work->from[i].vz += frnd()*5.0f ;
    }
    if ( work->life >=0 )
	if ( --work->life == 0 )
	{
	    if ( work->count )
		NewBreakComputerPlasma( &work->pos, work->conf, work->count ) ;
	    GV_DestroyActor( work ) ;
	}
}

static int GetResources( Work *work, FVECTOR *pos, FVECTOR *conf, int count )
{
    FVECTOR color = { 32, 64, 255, 128 } ;
    FVECTOR middle ;
    FVECTOR vel ;
    int     i ;
    FVECTOR norm ;

    _sceVu0CopyVector( &work->pos, pos ) ;

    CalcNormalFromVertex( &norm, conf ) ;
    _sceVu0ScaleVector( &norm, &norm, 50.0f ) ;
    _sceVu0AddVector( &middle, &norm, pos ) ;

    GM_SeSetMode( SD_A_SPARK01 , pos, GM_SEMODE_NORMAL ) ;

    middle.vw = 1.0f ;
    vel.vy = 0.0f ;
    vel.vw = 0.0f ;
    for ( i=3 ; --i>=0 ; )
    {
	vel.vx = frnd() * 400.0f ;
	vel.vz = frnd() * 400.0f ;

	_sceVu0AddVector( &work->from[i], pos, &vel ) ;
	_sceVu0SubVector( &work->to[i]  , pos, &vel ) ;

	GV_SetActorChild( work,
			  NewSpritePlasma2( &work->from[i],
					    &middle,
					    &work->to[i],
					    10.0f+rnd()*5.0f,
					    &color,
					    15 ) ) ;
    }
    work->conf  = conf  ;
    work->life  = 13    ;
    work->count = count-1 ;

    return 0 ;
}


void *NewBreakComputerPlasma( FVECTOR *pos, FVECTOR *conf, int life )
{
    Work *work ;

    if ( life )
    {
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
	if ( work != NULL )
	{
	    GV_SetActor( &work->actor, Act, NULL ) ;
	    GV_ActorEX( &work->actor ) ;
	    if ( GetResources( work, pos, conf, life ) < 0 )
	    {
		GV_DestroyActor( work ) ;
		return NULL ;
	    }
	}
	return work ;
    }
    return NULL ;
}
