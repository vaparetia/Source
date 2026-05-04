/*
   dar.c : file archiver for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Aug. ～
   $Id: dar.c,v 1.4 1999/10/13 10:46:23 usr02011 Exp $
 
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>

#include	"MDU_darFile.h"
#include	"MDU_util.h"

static	int	Mode ;

enum {
    MODE_NONE = 0,
    MODE_CREATE,
    MODE_APPEND,
    MODE_EXTRACT,
    MODE_DELETE,
    MAX_MODES 
} ;

static	void	Usage( void )
{
    printf( "dar : file archiver for MGS\n" ) ;
    printf( "usage : dar <options> archive_file file1 [file2 ...]\n" ) ;
    printf( "\t options :\n" ) ;
    printf( "\t\t c : create archive\n" ) ;
    printf( "\t\t x : extract archive\n" ) ;
    printf( "\t\t t : show files contained archive\n" ) ;
    printf( "\t\t a : append to archive\n" ) ;
    printf( "\t\t d : delete from archive\n" ) ;
    printf( "\t\t v : verbose mode\n" ) ;
    printf( "\t\t l<dir> : extract directory\n" ) ;
    exit( -1 ) ;
}

int	main( int argc, char **argv )
{
    char	*option, c, *d, dir[ 256 ] ;
    int		v, t ;

    if ( argc < 3 ) Usage() ;
    option = argv[ 1 ] ;
    t = v = 0 ;
    strcpy( dir, "./" ) ;
    while( *option != '\0' ) {
	c = *option ;
	if ( c == 'l' ) {
	    d = option + 1 ;
	    strcpy( dir, d ) ;
	    strcat( dir, "/" ) ;
	    break ;
	}
	else if ( c == 'c' ) Mode = MODE_CREATE ;
	else if ( c == 'x' ) Mode = MODE_EXTRACT ;
	else if ( c == 'a' ) Mode = MODE_APPEND ;
	else if ( c == 'd' ) Mode = MODE_DELETE ;
	else if ( c == 'v' ) v = 1 ;
	else if ( c == 't' ) {
	    Mode = MODE_EXTRACT ; t = 1 ;
	} 
	option ++ ;
    }
    MDU_DarInitFileAll() ;
    MDU_DarChangeMode( v, t ) ;
    switch( Mode ) {
    case MODE_CREATE :
	if ( argc < 4 ) Usage() ;
	MDU_DarCreateArchive( argv[ 2 ], argc - 3, argv + 3 ) ;
	break ;
    case MODE_EXTRACT :
	if ( argc < 4 ) {
	    MDU_DarExtractArchive( dir, argv[ 2 ] ) ;
	} else {
	    MDU_DarExtractArchive2( dir, argv[ 2 ], argc - 3, argv + 3 ) ;
	}
	break ;
    case MODE_APPEND :
	if ( argc < 4 ) Usage() ;
	MDU_DarAppendArchive( argv[ 2 ], argc - 3, argv + 3 ) ;
	break ;
    case MODE_DELETE :
	if ( argc < 4 ) Usage() ;
	MDU_DarDeleteArchive( argv[ 2 ], argc - 3, argv + 3 ) ;
	break ;
    default :
	Usage() ;
    }
    exit( 0 ) ;
}
