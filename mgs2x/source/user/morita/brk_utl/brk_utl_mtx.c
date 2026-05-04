/*
   brk_utl_ini.c
   壊れ用 汎用初期化ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_mtx.c,v 1.1.1.3 2002/11/19 11:45:51 Yoshizawa1 Exp $
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

#include "brk_utl.h"

FMATRIX BRK_UTL_MatrixTable[16][16] ;

int BRK_UTL_InitMatrixTable()
{
    int      x, z ;
    FMATRIX *m ;

#if 1
    for ( z=16 ; --z>=0 ; )
	for ( x=16 ; --x>=0 ; )
	{
	    m = &BRK_UTL_MatrixTable[x][z] ;
	    _sceVu0RotMatrixX( m, &DG_UnitMatrix, (float)(2.0f * (float)M_PI * x /16 - (float)M_PI) ) ;
	    _sceVu0RotMatrixY( m, m             , (float)(2.0f * (float)M_PI * z /16 - (float)M_PI) ) ;
	}
#else
    static int flag  = 1 ;

    if ( flag )
	for ( z=16 ; --z>=0 ; )
	    for ( x=16 ; --x>=0 ; )
	    {
		m = &BRK_UTL_MatrixTable[x][z] ;
		_sceVu0RotMatrixX( m, &DG_UnitMatrix, (float)M_PI * ( (float)x/8.0f - 1.0f ) ) ;
		_sceVu0RotMatrixY( m, m             , (float)M_PI * ( (float)z/8.0f - 1.0f ) ) ;
	    }
    flag = 0 ;
#endif

    return 0 ;
}
