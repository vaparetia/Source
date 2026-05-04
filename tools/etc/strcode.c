/*
   strcode.c : file strcode examiner for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Oct. ¡Á
   $Id: strcode.c,v 1.3 1999/10/29 03:54:41 usr02011 Exp $
 
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
    printf( "%x : %d\n", MDU_GetStrCodeNew( argv[ 1 ] ), MDU_GetStrCodeNew( argv[ 1 ] ) ) ;
    printf( "( old %x : %d )\n", MDU_GetStrCodeOld( argv[ 1 ] ), MDU_GetStrCodeOld( argv[ 1 ] ) ) ;
    exit( 0 ) ;
}
