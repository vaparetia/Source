/*
   id_check.c : id conflict checker for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Oct. ～
   $Id: id_check.c,v 1.4 2001/06/18 11:39:52 usr04098 Exp $
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
    printf( "id_check : id conflict checker for MGS\n" ) ;
    printf( "usage : id_check file1 file2 [file3 ...]\n" ) ;
    exit( -1 ) ;
}

int	main( int argc, char **argv )
{
    char	**name ;
    int		n_files, i, j, c ;
    long	id[ 4096 ], base ;
    int         err = 0 ;

    n_files = argc - 1 ;
    if ( n_files < 2 ) Usage() ;
    if ( n_files > 4096 ) {
	printf( "too many files\n" ) ;
	return -1 ;
    }

    name = argv + 1 ;
    for ( i = 0; i < n_files; i ++ ) {
	id[ i ] = MDU_GetStrCode( *name ) ;
	name++ ;
    }

    name = argv + 1 ;
    for ( i = 0; i < n_files; i ++ ) {
	base = id[ i ] ; 
	c = 0 ;
	if ( base < 0 ) continue ;
	for ( j = i; j < n_files; j ++ ) {
	    if ( base == id[ j ] ) {
		/* 名前が違うときのみ（拡張子は同じ） */
		if ( strcmp( *( name + i ), *( name + j ) ) && 
		     !strcmp( MDU_GetExtension( *( name + i ) ), 
			      MDU_GetExtension( *( name + j ) ) ) ) {
		    if ( c == 0 ) {
			fprintf( stderr, "conflict : %s %s \n",
				 *( name + i ), *( name + j ) ) ;
			err = 1 ;
			c = 1 ;
		    }
		}
		id[ j ] = -1 ;
	    }
	}
	if ( c ) printf( "\n" ) ;
    }    
    if ( err )
      return -2 ;

    name = argv + 1 ;
    for ( i = 0; i < n_files; i ++ ) {
	id[ i ] = MDU_GetStrCode( *name ) ;
	printf( "%-10s\t=> 0x%08x : %8d\n", *name, id[i], id[i] ) ;
	name++ ;
    }

    return 0 ;
}
