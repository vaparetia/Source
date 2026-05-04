/*--------------------------------------------------------------------
  
  hzx_conv2 : hzx → hzx2 変換コマンド

  使用方法 ：	hzx_conv2 -i <inputfile(*.hzx)> [-o <outputfile(*.hzx)]

  programed by M.Sonoyama 1999.Nov ～

  $Id: main.c,v 1.4 1999/12/15 23:32:27 usr02011 Exp $

----------------------------------------------------------------------*/

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	"MDU_util.h"
//#include	"MDU_mdl.h"
#include 	"fmt_hzx.h"

static	void	Usage( void )
{
    fprintf( stderr, "Usage: hzx_conv2 -i <inputfile(*.hzx)> [-o <outputfile(*.hzx)]\\\n" ) ;
    fprintf( stderr, "\t [-l outputdir] [-d]\n" ) ;
    exit( -1 ) ;
}

static	int	LoadHazard( name )
char		*name ;
{
    char	*ext ;
    int		id ; 

    ext = MDU_GetExtension( name ) ;
    if ( strcmp( ext, "hzm" ) == 0 ) {
	if ( ( id = MDU_LoadHzm( name ) ) < 0 ) return -1 ;
    } else if ( strcmp( ext, "hzx" ) == 0 ) {
	if ( ( id = MDU_LoadHzx( name ) ) < 0 ) return -1 ;
    } else {
	fprintf( stderr, "Unknown extension : %s : %s\n", ext, name ) ;
	return -1 ;
    }
    return id ;
}

int main( int argc, char **argv )
{
    int		n_output_files = 0, n_input_files = 0 ;
    char	*output_file[ 256 ], *input_file[ 256 ], outdir[ 256 ], full[ 256 ] ;
    int		mode = -1 ;
    int		i, div[ 3 ], d, id, dumpmode ;
    HZD_DEF	*def ;
    HZX_DEF	*xdef, *xdef2 ;

    /* 分割パラメータ初期化 */
    d = 0 ; dumpmode = 0 ;
    div[ 0 ] = div[ 1 ] = div[ 2 ] = 0 ;

    strcpy( outdir, "./" ) ;
    /* オプションの解析 */
    for ( argc --, argv ++ ; argc > 0 ; argc --, argv ++ ) {
	if ( argv[ 0 ][ 0 ] == '-' ) {
	    if ( argv[ 0 ][ 1 ] == 'o' ) {
		mode = 0 ; 	/* output */
	    } else if ( argv[ 0 ][ 1 ] == 'i' ) {
		mode = 1 ; 	/* input */
	    } else if ( argv[ 0 ][ 1 ] == 'l' ) {
		mode = 2 ;	/* out dir */
	    } else if ( argv[ 0 ][ 1 ] == 'd' ) {
		dumpmode = 1 ;
	    } else {
		mode = 255 ;
	    }
	} else {
	    switch ( mode ) {
	    case 0:/* 出力ファイル取得 */
		if ( n_output_files < 256 ) output_file[ n_output_files ] = argv[ 0 ] ;
		n_output_files ++ ;
		break ;
	    case 1:/* 入力ファイル取得 */
		if ( n_input_files < 256 ) input_file[ n_input_files ] = argv[ 0 ] ;
		n_input_files ++ ;
		break ;
	    case 2 :/* 出力ディレクトリ */
		strcpy( outdir, argv[ 0 ] ) ;
		strcat( outdir, "/" ) ;
		break ;		
	    default:
		break ;
	    }
	}
    }
    if ( n_input_files == 0 || n_input_files > 255 ) {
	Usage() ;
    }
    for ( i = n_output_files; i < n_input_files; i ++ ) {
	output_file[ i ] = input_file[ i ] ;
    }

    MDU_InitHzdCache() ;

    for ( i = 0; i < n_input_files; i ++ ) {
	printf( "load %s\n", input_file[ i ] ) ;
	id = LoadHazard( input_file[ i ] ) ;
//	if ( dumpmode ) MDU_DumpHzx( id ) ;
	def = MDU_HzdId2Def( id ) ;
	xdef = MDU_Hzx2Hzx2( def ) ;
	if ( dumpmode ) MDU_DumpHzx2( xdef ) ;
	strcpy( full, outdir ) ;
	strcat( full, output_file[ i ] ) ;
	MDU_ChangeExtension( full, "xzh" ) ;
	MDU_SaveHzx2( xdef, full ) ;
	xdef2 = MDU_LoadHzx2( full ) ;
//	MDU_DumpHzx2( xdef2 ) ;
    }
    return 0 ;
}

