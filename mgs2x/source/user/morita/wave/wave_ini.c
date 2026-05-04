//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	init.c
	    波 初期化関数群

	1999/11/10 T.Morita
	$Id: wave_ini.c,v 1.1.1.3 2002/11/19 11:46:40 Yoshizawa1 Exp $
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

#define _INIT_FILE_
#include "wave.h"



int WAV_GetOptionValue( Work *work, int col, int row )
{
    int buf[3] ;
    int i   ;
    char *c ;

    /* initialize wave parameter */
    work->wave0 = 0 ;
    work->speed = WAVE_SPEED ;
    work->center.vx = 5000.0f ;
    work->center.vz = 0.0f ;
    work->center.vy = -2000.0f ;

    work->big_alp[0] = 0 ;

    /* */
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &work->center ) ;
    } 
    work->speed  = GCL_GetOptionValue( 's', 0 ) ;
    work->n_big  = GCL_GetOptionValue( 'n', 0 ) ;


    i = 0 ;
    if ( GCL_GetOption( 'b' ) != NULL )
        for( ; (c=GCL_NextStr()) ; i++ )
	{
	    FVECTOR pos ;
	    int     rot ;

	    pos.vx = (float)GCL_GetInt( c ) ;
	    pos.vy = (float)GCL_GetInt( GCL_NextStr() ) ;
	    pos.vz = (float)GCL_GetInt( GCL_NextStr() ) ;
	    pos.vw = (float)GCL_GetInt( GCL_NextStr() ) ;
	    rot    = GCL_GetInt( GCL_NextStr() ) ;
	    work->big_alp[i] = GCL_GetInt( GCL_NextStr() ) ;
	    _sceVu0RotMatrixY( &work->big_mtx[i], &DG_UnitMatrix, (float)M_PI*2*rot/4096 ) ;
	    work->big_mtx[i].m[3][X]  = pos.vx         /pos.vw ;
	    work->big_mtx[i].m[3][Y]  = work->center.vy/pos.vw ;
	    work->big_mtx[i].m[3][Z]  = pos.vz         /pos.vw ;
	    work->big_mtx[i].m[W][W] /= pos.vw ;

	    /*
	    while( pos.vx < work->center.vx )
		pos.vx += WAVE_SIZE*WAVE_BASE ;
	    while( pos.vz < work->center.vz )
		pos.vz += WAVE_SIZE*WAVE_BASE ;
	    work->big_pos[i].vx = (int)(pos.vx-work->center.vx)%(int)(WAVE_SIZE*WAVE_BASE)/(WAVE_SIZE*WAVE_BASE/(col-1)) ;
	    work->big_pos[i].vy = pos.vy ;
	    work->big_pos[i].vz = (int)(pos.vz-work->center.vz)%(int)(WAVE_SIZE*WAVE_BASE)/(WAVE_SIZE*WAVE_BASE/(row-1)) ;
	    work->big_pos[i].pad= 0 ;
	    if ( !work->big_pos[i].vx )
		work->big_pos[i].vx = col-1 ;
	    if ( !work->big_pos[i].vz )
		work->big_pos[i].vz = col-1 ;
		*/
	}
    work->n_big = i ;
    while( --i>=0 )
	if ( GM_CheckGameStatus( STATE_DEMO ) )
	    work->big_cnt[i] = i * 1024 / work->n_big + (irnd() & 255) ;
	else
	    work->big_cnt[i] = 1024 + i * 1024 / work->n_big + (irnd() & 255) ;

    work->no_sound = 0 ;
    if ( GCL_GetOption( 'O' ) )
	work->no_sound = 1 ;

    if ( !GM_CheckGameStatus( STATE_DEMO ) )
	work->speed = WAVE_SPEED ;
    work->center.vx = -WAVE_SIZE*WAVE_BASE/2 ;
    work->center.vz =  WAVE_SIZE*WAVE_BASE/2 ;

    return 0 ;
}


static void init_model( MDL *dst, int col, int row, int flag )
{
    int    x, z ;
    short  *mp ;

    dst->flag = flag ; 
    dst->col  = col ;
    dst->row  = row ;
    if ( flag & MDL_INIT_VTX )
	dst->vtx[0]= WAV_MemAlloc( sizeof(short) * (col*row*2*4)*2 ),
	    dst->vtx[1]= dst->vtx[0] + (col*row*2*4) ;
    if ( flag & MDL_INIT_MAP )
	dst->map   = WAV_MemAlloc( sizeof(short) * (col*row*2  ) ) ;

    mp = &dst->map[2*row*col-2] ;
    for ( z=row ; --z>=0 ; )
	for ( x=col ; --x>=0 ; mp -= 2 )
	{
	    *(mp  ) = 4*( (col*2)*z + x*2 ) ;
	    *(mp+1) = 4*( *(mp)/4 - (col*2-1) ) ;
	    if ( z==row-1 )
		*(mp) = 4*( x*2 ) ;
	    if ( z==0     )
		*(mp+1) = *mp ;

	    dst->vtx[0][*mp+X] = dst->vtx[0][*(mp+1)+X] =
		dst->vtx[1][*mp+X] = dst->vtx[1][*(mp+1)+X] = 4096*WAVE_SIZE*x/col ;
	    dst->vtx[0][*mp+Z] = dst->vtx[0][*(mp+1)+Z] =
		dst->vtx[1][*mp+Z] = dst->vtx[1][*(mp+1)+Z] = 4096*WAVE_SIZE*z/row ;
	    dst->vtx[0][*mp+W] = dst->vtx[0][*(mp+1)+W] =
		dst->vtx[1][*mp+W] = dst->vtx[1][*(mp+1)+W] = 4096 ;
	}
}


int WAV_InitModelsAndPrims( Work *work )
{
    init_model( &work->m_wave,
		WAVE_WAVE_COL, WAVE_WAVE_ROW,
		MDL_INIT_VTX|MDL_INIT_MAP ) ;
    return 0 ;
}
