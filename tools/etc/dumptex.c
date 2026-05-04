/*
   dumptex.c : texture dump program for MGS on linux ( use libMDU )
   
   by M.Sonoyama 1999 Oct. ¡Á
   $Id: dumptex.c,v 1.3 2001/01/19 08:12:33 usr02011 Exp $
   
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

int	main( int argc, char **argv )
{
    MDU_Tex	*tex ;
    char	*ext ;

    ext = MDU_GetExtension( argv[ 1 ] ) ;
    if ( !strcmp( ext, "bmp" ) ) {
		tex = MDU_LoadBmp( argv[ 1 ] ) ;
    } else if ( !strcmp( ext, "pic" ) ) {
		tex = MDU_LoadPic( argv[ 1 ] ) ;
    } else if ( !strcmp( ext, "cm2" ) ) {
		MDU_DumpCm2( argv[ 1 ] ) ;
		exit( 0 ) ;
    } else {
		exit( 0 ) ;
    }
	//    MDU_SaveBmp( tex, "test.bmp" ) ;
    MDU_DumpTexture( tex ) ;
    exit( 0 ) ;
}
