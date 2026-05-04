//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_blink.c
  点滅ボタン

  2001/06/12 T. Morita
  $Id: efct_monitor.c,v 1.1.1.3 2002/11/19 11:45:26 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


#define SPEED   0.005f
#define MARGINE_UV 0.01f
#define MARGINE_LV 0.06f
#define MARGINE_W 0.01f


typedef struct work_t
{
    GV_ACT_EX actor ;

    FVECTOR   shape[4] ;

    DG_PRIM2 *mon[4] ;
    DG_PRIM2 *line   ;

    int       name  ;
    float     laster ;
    int       count ;
    int       type  ;
} Work ;



static void Die( Work *work )
{
    int  i ;

    for( i=4 ;--i>=0 ; )
	if ( work->mon[i] )
	    GM_FreePrim2( work->mon[i] ) ;
    if ( work->line )
	GM_FreePrim2( work->line ) ;
}



void BRK_COM_MakeShape( FVECTOR *pos, FVECTOR *shape,
			float offset_h, float offset_v )
{
    FVECTOR horz, vert  ;

    offset_h = offset_h<0.0f ? 0.0f : offset_h>1.0f ? 1.0f : offset_h ;
    _sceVu0SubVector( &horz, &shape[1], &shape[0] ) ;
    _sceVu0ScaleVector( &horz, &horz, offset_h ) ;
    offset_v = offset_v<0.0f ? 0.0f : offset_v>1.0f ? 1.0f : offset_v ;
    _sceVu0SubVector( &vert, &shape[2], &shape[0] ) ;
    _sceVu0ScaleVector( &vert, &vert, offset_v ) ;
    _sceVu0AddVector( pos, &horz, &vert ) ;

    _sceVu0AddVector( pos, pos, &shape[0] ) ;
}

static inline void BRK_COM_ActLaster( Work *work )
{
    FVECTOR *pos ;

    DG_SwitchBuffPrim2( work->line ) ;
    pos = work->line->pos[work->line->buffer_clock] ;
    if ( (work->laster += SPEED) > (1.0f-MARGINE_LV) )
	work->laster = MARGINE_UV ;
    BRK_COM_MakeShape( pos+0, work->shape, 1.0f-MARGINE_W, work->laster ) ;
    BRK_COM_MakeShape( pos+1, work->shape, 0.0f+MARGINE_W, work->laster ) ;
}

static inline void BRK_COM_ActMonitor( Work *work )
{
    /* 画面ちらつき */
    DG_SwitchBuffPrim2( work->mon[work->type] ) ;
}

static void Act( Work *work )
{
    int     i   ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        if ( msg->message[0] == 0 )
	{
	    work->type = msg->message[1] ;
	    for ( i=4 ; --i>=0 ; )
		DG_InvisiblePrim2( work->mon[i] ) ;
	    DG_VisiblePrim2( work->mon[work->type] ) ;
	}
    BRK_COM_ActLaster( work ) ;
    BRK_COM_ActMonitor( work ) ;
}


static void SetPosition( DG_PRIM2 *p, FVECTOR *pos )
{
    int i ;
    DG_PRIM2_UVRGB *u0, *u1 ;

    u0 = p->uvrgb[0] ;
    u1 = p->uvrgb[1] ;
    for( i=4 ; --i>=0 ; )
    {
	_sceVu0CopyVector( &p->pos[0][i], &pos[i] ) ;
	_sceVu0CopyVector( &p->pos[1][i], &pos[i] ) ;
	p->pos[0][i].vw = 1.0f ;
	p->pos[1][i].vw = 1.0f ;
	u0[i].a = 128 ;
	u1[i].a = 80  ;
    }
}

/* コマンドでいじれるようにした */
static int ids[] = { 
    9697762, 10746338, 11794914, 12843490
} ;

int COM_SetPicIdBreakComputerMonitor( void )
{
    int idx ;

    idx = GCL_GetOptionValue( 'i', 0 ) ;
#ifdef DEBUG_MODE
    if ( idx >= 4 ) {
	printf( "efct_monitor.c : Error!!!! Too many index %d (must be <= 3)\n", idx ) ;
    }
#endif
    ids[idx&3] = GCL_GetOptionValue( 'D', 9697762 ) ;
    return 1 ;
}

static int GetResources( Work *work, int name, FVECTOR *pos, int start )
{
    int i ;

    for ( i=4 ; --i>=0 ; )
    {
	if ( !(work->mon[i] = BRK_UTL_MakePOLY( 1,
						ids[i],
						SCE_GS_SET_ALPHA(0,1,0,1,0),
						0x2f3f3f3f )) )
	    PERROR( "No Prim(no memory) : NewBreakComputerMonitor\n" ) ;
	SetPosition( work->mon[i], pos ) ;
	DG_InvisiblePrim2( work->mon[i] ) ;
    }
    work->type = start ;
    DG_VisiblePrim2( work->mon[work->type] ) ;

    /* 流れ用のプリミティブを作る */
    if ( !(work->line = BRK_UTL_MakeLINE( 1, 0,
					  SCE_GS_SET_ALPHA(0,1,0,1,0),
					  0x2f1f1f1f, 0x2f1f1f1f )) )
	PERROR( "No LinePrim(no memory) : NewBreakComputerMonitor\n" ) ;
    work->line->raise = 200 ;

    memcpy( work->shape, pos, sizeof(FVECTOR)*4 ) ;
    work->name = name ;
    return 0 ;
}

void *NewBreakComputerMonitor( int name, FVECTOR *pos, int start )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, name, pos, start ) < 0 )
	{
	    GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
