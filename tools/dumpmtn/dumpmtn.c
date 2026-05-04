/*
  DumpMtn files

  Originated from Takabe
  2000/06/08 revised by T. Morita

  $Id: dumpmtn.c,v 1.1 2000/06/08 06:22:04 usr04098 Exp $
*/
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "quat.h"
#include "motion.h"


int main( int argc, char **argv )
{
    MTN_MOTION *motion ;
    int		i, last_frm ;
    FVECTOR     start, end ;
    char       *c ;
    int         flag ;

    if ( argc == 1 )
	fprintf( stderr, "Usage: dumpmtn [-d] files\n"
	    "\t-d: dump the MTN files with frame\n" );

    for( argv++ ; --argc>0 ; argv++ )
    {
	if ( argv[0][0] == '-' )
	{
	    flag = argv[0][1] ;
	}
	else
	{
	    if ( !( motion = MTN_OpenMTNFile( *argv )) )
	    {
		printf( "<%s>file open error!!\n", *argv ) ;
		return -1 ;
	    }

	    last_frm = motion->header.motion_length ;
	    switch( flag )
	    {
	    case 'd':
		for ( i=0 ; i<last_frm ; i++ )
		{
		    MTN_GetMotionData( motion, i, &start, NULL, NULL ) ;
		    printf( "{ %3d, %4f,%4f,%4f },\n",
			    i,
			    start.vx,
			    start.vy,
			    start.vz ) ;
		}
		break ;
	    default:
		MTN_GetMotionData( motion,        0, &start, NULL, NULL ) ;
		MTN_GetMotionData( motion, last_frm-1, &end,   NULL, NULL ) ;
		MTN_FreeMotion( motion );
		c = rindex( *argv, '/' ) ;
		printf( "{ %3d, %4f,%4f,%4f },\t/* %s */\n",
			last_frm,
			end.vx-start.vx,
			end.vy-start.vy,
			end.vz-start.vz,
			c ? c+1 : *argv ) ;
	    }
	}
    }
    return ( 0 );
}

