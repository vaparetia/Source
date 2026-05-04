//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_monitor.c
   制御室モニター壊れ

   1999/12/13 T. Morita
   $Id: brk_mon_ini.c,v 1.1.1.3 2002/11/19 11:45:36 Yoshizawa1 Exp $
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

#include "brk_monitor.h"


static u_char Table[][BRK_MON_N_TYPE] =
{
    /* R */
    { 0xf8|7, 0xf0|2, 0xf0|3, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf8|7, 0xf0|0, 0xf0|1, 0xf0|2, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf8|7, 0xf0|2, 0xf0|3, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
	 	 	      				            
    { 0xf0|2, 0xf0|0, 0xf0|3, 0xf0|1, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|3, 0xf0|0, 0xf0|0, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|1, 0xf0|3, 0xf0|0, 0xf0|2, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
	 	 	      				            
    { 0xf0|0, 0xf0|0, 0xf0|0, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|1, 0xf0|2, 0xf0|3, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|6, 0xf0|0, 0xf0|3, 0xf0|1, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
							            
    /* L */						            
    { 0xf8|7, 0xf0|2, 0xf0|3, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf8|7, 0xf0|0, 0xf0|1, 0xf0|2, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|1, 0xf0|2, 0xf0|3, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
	 	 	      				            
    { 0xf0|2, 0xf0|0, 0xf0|3, 0xf0|1, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|3, 0xf0|0, 0xf0|0, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|1, 0xf0|3, 0xf0|0, 0xf0|2, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
	 	 	      				            
    { 0xf0|6, 0xf0|0, 0xf0|0, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|1, 0xf0|2, 0xf0|3, 0xf0|0, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
    { 0xf0|3, 0xf0|0, 0xf0|3, 0xf0|1, 0xf8|4, 0x08|6, 0x08|5, 0xf8|7 },
} ;


void BRK_MON_MakeShape( FVECTOR *pos, FVECTOR *shape,
			int i, int j,
			float offset_h, float offset_v )
{
    FVECTOR horz, vert  ;

    offset_h += (i%3+1-j%2)/3.0f ;
    offset_h = offset_h<0.0f ? 0.0f : offset_h>1.0f ? 1.0f : offset_h ;
    _sceVu0SubVector( &horz, &shape[1], &shape[0] ) ;
    _sceVu0ScaleVector( &horz, &horz, offset_h ) ;
    offset_v += (i/3+1-j/2)/3.0f ;
    offset_v = offset_v<0.0f ? 0.0f : offset_v>1.0f ? 1.0f : offset_v ;
    _sceVu0SubVector( &vert, &shape[2], &shape[0] ) ;
    _sceVu0ScaleVector( &vert, &vert, offset_v ) ;
    _sceVu0AddVector( pos, &horz, &vert ) ;

    _sceVu0AddVector( pos, pos, &shape[0] ) ;
}




int BRK_MON_InitParam( Work *work, int name, int where )
{
    MON   *m ;
    int    i ;

    work->proc = GCL_GetOptionValue( 'p', 0 ) ;
    work->flag = GCL_GetOptionValue( 'f', 0 ) ;

    work->n_monitor = 18 ;
    m = work->monitor ;
    for ( i=BRK_MON_N_PRIMS ; --i>=0 ; m++ )
    {
	m->laster   = rnd()/3.0f ;
	m->table    = Table[i] ;
	if ( work->flag & (1<<(BRK_MON_N_PRIMS-1-i)) )
	    m->type = 4 ;
	else
	    m->type = m->table[0] & 0x7 ;
	m->type_old = -1 ;
	if ( (m->table[m->type] & 0x7) == m->type )
	    m->change = -256 ;/* 画面が変化しないものは動かない */
	else
	    m->change = irnd() & 511 ;

	/* 点ける方 */
	BRK_MON_SetRGB( work, m->type, i, 0, 1 ) ;
	BRK_MON_SetRGB( work, m->type, i, 1, 1 ) ;
	/* 捜査線を消す? */
	BRK_MON_LasterRGB( work, m->type, i ) ;
    }    
    
    for ( i=4 ; --i>=0 ; m++ )
    {
	_sceVu0AddVector( &work->center, &work->center,
			  &BRK_MOT_FrameL[i] );
	_sceVu0AddVector( &work->center, &work->center,
			  &BRK_MOT_FrameR[i] ) ;
    }
    _sceVu0ScaleVector( &work->center, &work->center, 1.0f/(2.0f*4.0f) ) ;

    return 0 ;
}




int BRK_MON_InitMonitor( Work *work, int name, int where )
{
    int      i, j ;
    FVECTOR *pos  ;
    int ids[] = { 6185975 ,/* w12b0_moni0.bmp */
		  6185976 ,/* w12b0_moni1.bmp */
		  6185977 ,/* w12b0_moni2.bmp */
		  6185978 ,/* w12b0_moni3.bmp */
		  10009554,/* w12b0_moni_brk0.bmp */
		  10009555,/* w12b0_moni_brk1.bmp */
		  10009556,/* w12b0_moni_brk2.bmp */
		  10009557,/* w12b0_moni_brk3.bmp */ } ;

    /* 画面の形を作る */
    pos  = (void*)SCRPAD_ADDR ;
    for ( i=BRK_MON_N_PRIMS/2 ; --i>=0 ; )
	for ( j=4 ; --j>=0 ; )
	    BRK_MON_MakeShape( pos++, BRK_MOT_FrameR, i, j, 0.0f, 0.0f ) ;
    for ( i=BRK_MON_N_PRIMS/2 ; --i>=0 ; )
	for ( j=4 ; --j>=0 ; )
	    BRK_MON_MakeShape( pos++, BRK_MOT_FrameL, i, j, 0.0f, 0.0f ) ;

    /* 画面用のプリミティブを作る */
    for ( i=BRK_MON_N_TYPE ; --i>=0 ; )
    {
	if ( !(work->prim[i] = BRK_UTL_MakePOLY( BRK_MON_N_PRIMS,
						 ids[i],
						 SCE_GS_SET_ALPHA(0,1,0,1,0),
						 0x007f7f7f )) )
	    PERROR( "No Prim(no memory) : NewPutMonitor\n" ) ;
	memcpy( work->prim[i]->pos[0],
		SCRPAD_ADDR,
		sizeof(FVECTOR)*BRK_MON_N_PRIMS*4 ) ;
	memcpy( work->prim[i]->pos[1],
		SCRPAD_ADDR,
		sizeof(FVECTOR)*BRK_MON_N_PRIMS*4 ) ;
    }
    work->prim[7]->raise = -200 ;

    return 0 ;
}


int BRK_MON_InitBack( Work *work, int name, int where )
{
    /* 画面用のプリミティブを作る */
    if ( !(work->back = BRK_UTL_MakePOLY( 2,
					  6185975,
					  SCE_GS_SET_ALPHA(0,1,0,1,0),
					  0x7f000000 )) )
	PERROR( "No Prim(no memory) : NewPutMonitor\n" ) ;

    memcpy( work->back->pos[0]+0, BRK_MOT_FrameL, sizeof(FVECTOR)*4 ) ;
    memcpy( work->back->pos[1]+0, BRK_MOT_FrameL, sizeof(FVECTOR)*4 ) ;
    memcpy( work->back->pos[0]+4, BRK_MOT_FrameR, sizeof(FVECTOR)*4 ) ;
    memcpy( work->back->pos[1]+4, BRK_MOT_FrameR, sizeof(FVECTOR)*4 ) ;

    return 0 ;
}


int BRK_MON_InitLaster( Work *work, int name, int where )
{
    int i, idx ;
    FVECTOR *pos ;

    /* 画面の形を作る */
    pos  = (void*)SCRPAD_ADDR ;
    idx = 0 ;
    for ( i=BRK_MON_N_PRIMS/2 ; --i>=0 ; )
    {
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameR, i, 0,  0.1f, 0.0f ) ;
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameR, i, 1, -0.1f, 0.0f ) ;
    }
    for ( i=BRK_MON_N_PRIMS/2 ; --i>=0 ; )
    {
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameL, i, 0,  0.1f, 0.0f ) ;
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameL, i, 1, -0.1f, 0.0f ) ;
    }

    /* 流れ用のプリミティブを作る */
    if ( !(work->line = BRK_UTL_MakeLINE( BRK_MON_N_PRIMS,
					  6185975,
					  SCE_GS_SET_ALPHA(0,1,0,1,0),
					  0x2f3f3f3f, 0x1f3f3f3f )) )
	PERROR( "No Prim(no memory) : NewPutMonitor\n" ) ;
    work->line->raise = 200 ;
    memcpy( work->line->pos[0],
	    SCRPAD_ADDR,
	    sizeof(FVECTOR)*BRK_MON_N_PRIMS*2 ) ;
    memcpy( work->line->pos[1],
	    SCRPAD_ADDR,
	    sizeof(FVECTOR)*BRK_MON_N_PRIMS*2 ) ;

    return 0 ;
}


void BRK_MON_LasterRGB( Work *work, int type, int i )
{
    int             clock ;
    DG_PRIM2_UVRGB *u     ;

    if ( type == 4 || type == 7 )
    {
	clock = work->line->buffer_clock ;
	u = work->line->uvrgb[clock] ;
	u += i*2 ;
	(u+0)->a = (u+1)->a = 0 ;
    }
}

void BRK_MON_SetRGB( Work *work, int type, int i, int clock, int on )
{
    int             alpha ;
    DG_PRIM2_UVRGB *u     ;

    if ( type >= 0 )
    {
	if ( clock < 0 )
	    clock = work->prim[type]->buffer_clock ;
	u = work->prim[type]->uvrgb[clock] ;
	u += i*4 ;
	alpha = type==4 || type==7 ? 128 : 64 * (1 + clock) ;
	(u+0)->a = (u+1)->a = (u+2)->a = (u+3)->a = alpha * on ;
	if ( (type == 5 || type == 6) && !on )
	{
	    type = type==5 ? 6 : 5 ;
	    clock = work->prim[type]->buffer_clock ;
	    u = work->prim[type]->uvrgb[clock] ;
	    u += i*4 ;
	    (u+0)->a = (u+1)->a = (u+2)->a = (u+3)->a = 0 ;
	}
	if ( type!=4 && type!=7 )
	{
	    clock = work->prim[7]->buffer_clock ;
	    u = work->prim[7]->uvrgb[clock] ;
	    u += i*4 ;
	    alpha = 128 * (1 - clock) ;
	    (u+0)->a = (u+1)->a = (u+2)->a = (u+3)->a = alpha ;
	}
    }
}

int BRK_MON_InitTarget( Work *work, int name, int where )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target   ;
    POWER_TARGET *p = &work->power    ;

    t_size.vx =  BRK_MOT_FrameR[0].vx - BRK_MOT_FrameL[0].vx ;
    t_size.vy =  BRK_MOT_FrameR[0].vy - BRK_MOT_FrameR[3].vy ;
    t_size.vz =  BRK_MOT_FrameR[1].vz - BRK_MOT_FrameR[2].vz ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;

    _sceVu0AddVector( &t_pos, &BRK_MOT_FrameR[0], &BRK_MOT_FrameR[1]) ;
    _sceVu0AddVector( &t_pos, &t_pos, &BRK_MOT_FrameR[2]) ;
    _sceVu0AddVector( &t_pos, &t_pos, &BRK_MOT_FrameR[3]) ;
    _sceVu0AddVector( &t_pos, &t_pos, &BRK_MOT_FrameL[0]) ;
    _sceVu0AddVector( &t_pos, &t_pos, &BRK_MOT_FrameL[1]) ;
    _sceVu0AddVector( &t_pos, &t_pos, &BRK_MOT_FrameL[2]) ;
    _sceVu0AddVector( &t_pos, &t_pos, &BRK_MOT_FrameL[3]) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 1.0f/8.0f ) ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_THROUGH,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_MON_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}
