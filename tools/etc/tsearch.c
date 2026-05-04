/*
   tsearch.c : searching texture from mdl for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Nov. ¡Á
   $Id: tsearch.c,v 1.1 1999/11/02 08:40:29 usr02011 Exp $
 
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>
#include 	<SFDUNIX.h>
#include 	<METALGEAR.h>
#include	"MDU_util.h"
#include	"MDU_mdl.h"
#include	"MDU_tex.h"

static	void	Usage( void )
{
    fprintf( stderr, "tsearch : searching texture from mdl\n" ) ;
    fprintf( stderr, "usage : tsearch mdlfile(*.mdl) [-i id1 id2 ...]\n" ) ;
    exit( -1 ) ;
}

int	main( int argc, char **argv )
{
    int		n_ids, id[ 256 ], buf, i ;
    HP3DMODEL	hP3DModel ;

    if ( argc < 2 ) Usage() ;
    MDU_ParseOption( argc, argv ) ;

    n_ids = 0 ;
    if ( MDU_GetOption( 'i' ) != NULL ) {
	while( ( buf = MDU_GetNextInt() ) >= 0 ) {
	    id[ n_ids ] = buf ; n_ids ++ ;
	}
    }

    if ( ( hP3DModel = P3DReadFileModel( argv[ 1 ], NULL, 0 ) ) == NULL ) Usage() ;
    MDU_ExtractTextureFromMdl( hP3DModel ) ;

    if ( n_ids == 0 ) {
	MDU_CompTexID( -1 ) ;
    } else {
	for( i = 0; i < n_ids; i ++ ) MDU_CompTexID( id[ i ] ) ;
    }

    exit( 0 ) ;
}
