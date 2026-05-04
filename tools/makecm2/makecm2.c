/* 
   makecm2 : texture archiver for MGS2 on linux
   
   by M.Sonoyama 1999 Sep. ¡Á
   $Id: makecm2.c,v 1.6 2001/05/11 06:00:56 usr02011 Exp $
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>

#include	"MDU_util.h"
#include	"MDU_tex.h"

static	MDU_Tex	*Texs[ 1024 ] ;
static	int	N_Texs, Mode, Size ;

static	void	Usage( void )
{
    printf( "makecm2 : texture archiver for MGS2\n" ) ;
    printf( "\tusage : makecm2 -out archive_file_root \\\n" ) ;
    printf( "\t\t\t -in file1 [file2 ...]\n" ) ;
    exit( -1 ) ;
}

static	int	LoadTex( name )
char		*name ;
{
    MDU_Tex	*tex ;
    char       	*ext ;
    int		size ;

    size = MDU_GetFileSize( name ) ;
    if ( size < 0 ) return -1 ;

    tex = NULL ;
    ext = MDU_GetExtension( name ) ;
    if ( !strcmp( ext, "bmp" ) ) {
		tex = MDU_LoadBmp( name ) ;  
    } else if ( !strcmp( ext, "tim" ) ) {
		tex = MDU_LoadTim( name ) ;
    } else if ( !strcmp( ext, "pcx" ) ) {
		tex = MDU_LoadPcx( name ) ;
    } else if ( !strcmp( ext, "pic" ) ) {
		tex = MDU_LoadPic( name ) ;
    } else {
		printf( "unknown extension : %s\n", ext ) ;
    }
    
    if ( tex != NULL ) {
		//printf( "%s id %d\n", name, tex->id ) ;
		if ( ( tex->flag & MDU_TEX_FLAG_COLOR256 ) == Mode ) {
			Size += size ;
			Texs[ N_Texs ] = tex ;
			N_Texs ++ ;
		}
    } else {
		return -1 ;
	}

	return 0 ;
}

int	main( int argc, char **argv )
{
    char       	*out, *in, *ext ;
    char	outfile[ 256 ] ;
    int		n, i ;

    MDU_TexInit() ;

    MDU_ParseOption( argc, argv ) ;

    if ( MDU_GetOption( 'o' ) == NULL ) Usage() ;
    out = MDU_GetNextValue() ;

    if ( MDU_GetOption( 'd' ) != NULL ) {
		MDU_DumpCm2( out ) ;
		exit( 0 ) ;
    }
#if 0    
    if ( MDU_GetOption( 'm' ) == NULL ) Usage() ;
    Mode = MDU_GetNextInt() ;
    printf( "mode %d\n", Mode ) ;
    if ( Mode == 16 ) Mode = MDU_TEX_FLAG_COLOR16 ;
    else	      Mode = MDU_TEX_FLAG_COLOR256 ;
#endif
    
	//    MDU_GetRoot( out, outfile ) ;
	//    out = outfile + strlen( outfile ) ;

    ext = strrchr( out, '.' ) ;
    if ( ext != NULL && !strcmp( ext, ".cm2" ) ) {
		*ext = '\0' ;
		strcpy( outfile, out ) ;
    } else {
		strcpy( outfile, out ) ;	
    }
    out = outfile + strlen( outfile ) ;

    /* £±£¶¿§ */
    N_Texs = 0 ;
    Size = 0 ;
    n = 1 ;
    Mode = MDU_TEX_FLAG_COLOR16 ;
    if ( MDU_GetOption( 'i' ) == NULL ) Usage() ;
    while( ( in = MDU_GetNextValue() ) != NULL ) {
		if ( LoadTex( in ) < 0 ) exit( -1 ) ;
		if ( Size > 300000 ) {
			out[ 0 ] = '1' ;
			out[ 1 ] = '6' ;
			out[ 2 ] = '_' ;
			out[ 3 ] = '0' + n ;
			strcpy( out + 4, ".cm2\0" ) ;
			MDU_ReplaceTexCm2( Texs, N_Texs ) ;
			printf( "%s\n", outfile ) ;
			MDU_SaveCm2( outfile, Texs, N_Texs ) ;	
			n ++ ;
			for ( i = 0; i < N_Texs; i ++ ) {
				MDU_ClearTexture( Texs[ i ] ) ;
			}
			N_Texs = 0 ;
			Size = 0 ;
		}
    }
    if ( Size > 0 ) {
		out[ 0 ] = '1' ;
		out[ 1 ] = '6' ;
		out[ 2 ] = '_' ;
		out[ 3 ] = '0' + n ;
		strcpy( out + 4, ".cm2\0" ) ;
		MDU_ReplaceTexCm2( Texs, N_Texs ) ;
		printf( "%s\n", outfile ) ;
		MDU_SaveCm2( outfile, Texs, N_Texs ) ;	
		for ( i = 0; i < N_Texs; i ++ ) {
			MDU_ClearTexture( Texs[ i ] ) ;
		}
    }

    /* £²£µ£¶¿§ */
    N_Texs = 0 ;
    Size = 0 ;
    n = 1 ;
    Mode = MDU_TEX_FLAG_COLOR256 ;
    if ( MDU_GetOption( 'i' ) == NULL ) Usage() ;
    while( ( in = MDU_GetNextValue() ) != NULL ) {
		if ( LoadTex( in ) < 0 ) exit( -1 ) ;
		if ( Size > 300000 ) {
			out[ 0 ] = '2' ;
			out[ 1 ] = '5' ;
			out[ 2 ] = '6' ;
			out[ 3 ] = '_' ;
			out[ 4 ] = '0' + n ;
			strcpy( out + 5, ".cm2\0" ) ;
			MDU_ReplaceTexCm2( Texs, N_Texs ) ;
			printf( "%s\n", outfile ) ;
			MDU_SaveCm2( outfile, Texs, N_Texs ) ;	
			n ++ ;
			for ( i = 0; i < N_Texs; i ++ ) {
				MDU_ClearTexture( Texs[ i ] ) ;
			}
			N_Texs = 0 ;
			Size = 0 ;
		}
    }
    if ( Size > 0 ) {
		out[ 0 ] = '2' ;
		out[ 1 ] = '5' ;
		out[ 2 ] = '6' ;
		out[ 3 ] = '_' ;
		out[ 4 ] = '0' + n ;
		strcpy( out + 5, ".cm2\0" ) ;
		MDU_ReplaceTexCm2( Texs, N_Texs ) ;
		printf( "%s\n", outfile ) ;
		MDU_SaveCm2( outfile, Texs, N_Texs ) ;	
		for ( i = 0; i < N_Texs; i ++ ) {
			MDU_ClearTexture( Texs[ i ] ) ;
		}
    }

    exit( 0 ) ;
}
