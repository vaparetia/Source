/*
   dumpkm3.c : km3_file dumper for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Nov. ¡Á
   $Id: dumpkm3.c,v 1.1 1999/12/15 23:32:25 usr02011 Exp $
 
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>

#include	"MDU_util.h"
#include	"MDU_mdl.h"
#include	"MDU_tex.h"

static	void	Usage( void )
{
    fprintf( stderr, "dumpkm3 : km3_file dumper\n" ) ;
    fprintf( stderr, "usage : dumpkms mdlfile(*.km3)\n" ) ;
    exit( -1 ) ;
}

int	main( int argc, char **argv )
{
    KM3_DEF	*sdef ;

    if ( argc < 2 ) Usage() ;
    sdef = MDU_LoadKm3( argv[ 1 ] ) ;
    if ( sdef == NULL ) Usage() ;
    MDU_DumpKm3( sdef ) ;
    exit( 0 ) ;
}
