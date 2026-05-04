/*
   texconv.c : texture converter for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Oct. ¡Á
   $Id: texconv.c,v 1.2 2001/01/19 08:12:34 usr02011 Exp $
 
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>

#include	"MDU_tex.h"
#include	"MDU_util.h"

static	void	Usage( void )
{
    fprintf( stderr, "texconv : texture converter\n" ) ;
    fprintf( stderr, "usage : texconv [-ldir] file[bmp or pic] [file2 ...]\n" ) ;
    fprintf( stderr, "\t\t -ldir : output directory\n" ) ;
    exit( -1 ) ;
}

int	main( int argc, char **argv )
{
    MDU_Tex	*tex ;
    u_char	*ext, *name, *tail, dir[ 256 ], full[ 256 ] ;

    if ( argc < 2 ) Usage() ;
    if ( argv[ 1 ][ 0 ] == '-' &&
	 argv[ 1 ][ 1 ] == 'l' ) {
	strcpy( dir, &argv[ 1 ][ 2 ] ) ;
	strcat( dir, "/" ) ;
	argc -= 2 ;
	argv += 2 ;
    } else {
	strcpy( dir, "./" ) ;
	argc -- ;
	argv ++ ;
    }

    while( -- argc >= 0 ) {
	name = *( argv ++ ) ;
	ext = MDU_GetExtension( name ) ;
	if ( !strcmp( ext, "bmp" ) ) {
	    tex = MDU_LoadBmp( name ) ;
	} else if ( !strcmp( ext, "pic" ) ) {
	    tex = MDU_LoadPic( name ) ;
	} else if ( !strcmp( ext, "pcx" ) ) {
	    tex = MDU_LoadPcx( name ) ;
	} else {
	    continue ;
	}
	if ( tex == NULL ) {
	    printf( "%s : load error\n", name ) ;
	    exit( -1 ) ;
	}
	tail = MDU_TailName( name ) ;
	MDU_SetTransTexture( tex, tail ) ;
	MDU_SetMaskTexture( tex, tail ) ;
	MDU_ChangeExtension( name, "bmp" ) ;
	strcpy( full, dir ) ;
	strcat( full, tail ) ;
//	printf( "%s\n", full ) ;
	MDU_SaveBmp( tex, full ) ;
	MDU_ClearTexture( tex ) ;
    }
    exit( 0 ) ;
}
