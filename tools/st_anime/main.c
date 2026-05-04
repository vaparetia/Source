/*
	ストリーム頂点アニメ

	XSI→ANI

	T.Morita  Feb 21 2000
	$Id: main.c,v 1.10 2002/06/13 01:33:55 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __MAIN_FILE__

#include "alltypes.h"
#include "allvars.h"


char *find_extention( char *name )
{
    char *c = NULL ;

    while( (name=strchr( name, '.' )) )
	c = ++name ;
    return c ;
}

int open_file_with_ext( FILE **fp, char *src, char *ext, char *mode )
{
    char name[256] ;
    char *c ;

    strcpy( name, src ) ;
    if ( (c = find_extention( name )) )
	*c = '\0', strcat( name, ext ) ;
    else
	strcat( name, "." ), strcat( name, ext ) ;

    if ( (*fp = fopen( name, mode )) == NULL )
    {
	fprintf( stderr, "File open error<%s>!!\n", name ) ;
	return 1 ;
    }
    return 0 ;
}


void init_and_option( int argc, char **argv, char **xsi_name, char **cv2_name )
{
    char *ext ;

    *xsi_name = NULL ;
    *cv2_name = NULL ;
    if ( argc < 2 )
	fprintf( stderr, "Usage: st_anime in.xsi mdl.cv2 [<out.anm>] [-n] [-s <scale>] \n"
		         "options\n"
		         "\t-n make up normals.\n"
		         "\t-s <scale> set scaling of the model. Default 100.\n" ) ;

    yyin = stdin ;
    flag = FLG_VERTEX ;
    for( ; --argc>0 ; argv++ )
	if ( argv[0][0] == '-' )
	    switch( argv[0][1] )
	    {
	    case 'r':
		skip_frame = atoi( *++argv ) ;
		argc-- ;
		break ;
	    case 'n':
		flag |= FLG_NORMAL ;
		break ;
	    case 's':
		scaling = atoi( *++argv ) ;
		argc-- ;
		break ;
	    }
	else if ( (ext = find_extention( *argv )) )
	{
	    if ( !strcmp( ext, "xsi" ) )
		open_file_with_ext( &yyin, *xsi_name = *argv, "xsi", "r" ) ;
	    if ( !strcmp( ext, "anm" ) )
		*xsi_name = *argv ;
	    if ( !strcmp( ext, "cvx" ) )
		*cv2_name = *argv ;
	    if ( !strcmp( ext, "cvy" ) )
		*cv2_name = *argv ;
	    if ( !strcmp( ext, "cv2" ) )
		*cv2_name = *argv ;
	}
	else if ( yyin == stdin )
	    open_file_with_ext( &yyin, *xsi_name = *argv, "xsi", "r" ) ;
	else if ( !*xsi_name )
	    *xsi_name = *argv ;
	else
	    *cv2_name = *argv ;
}

int main( int argc, char **argv )
{
    char *xsi_name, *cv2_name ;
    CV2_OBJS *c   ;
    extern CV2_OBJS *MDU_LoadCv2( char *name ) ;

    init_and_option( argc, argv+1, &xsi_name, &cv2_name ) ;

    if ( cv2_name )
    {
	/* load CV2 */
	if ( !(c = MDU_LoadCv2( cv2_name )) )
	{
	    fprintf( stderr, "No Cv2-file<%s>!!\n", cv2_name ) ;
	    return 1 ;
	}

	if ( xsi_name )
	{
	    if ( !yyparse() )
	    {
		if ( make_datas() < 0 )
		    return 1 ;
		if ( output_anm( xsi_name, c ) )
		    return 2 ;
	    }
	}
	else
	    printf( "Err!! : Specify Xsi-File\n" ) ;

    }
    else
	printf( "Err!! : Specify Cv2-File\n" ) ;


    return 0 ;
}
