/*
  geo_main.c
  SCN2GEO Converter
   
  2002.11.6 T.Morita

  $Id: geo_main.c,v 1.10 2002/12/02 06:32:06 usr04098 Exp $



   +------------+ 各ファイルの依存構造です。
   |    main    |
   |   +--------+
   |   |  file  |
   +---+------+ |
   |  convert | | geo_coreが主にGEOを扱うところで,P3Dはscnファイルを
   |          | | 扱うためのものです。
   |     +----+ | 
   |     |poly| |
   +---+-+-+--+-+ 実際のコンバートの流れは,geo_convertでやられています。
   |mem|core|P3D| P3Dをうまく使い分けるため geo_poly を間に入れました。
   +---+----+---+
   
   ここでは,
   GEOM
   GEO_BLOCK  基本的にはGEOMはブロックを使わなければＮＧ
   GEO_GROUP  BLOCKを使う
   GEO_DEF    GEO_CHANKにより同時にさまざまなファイルを持つ
   を扱うためのライブラリである


   +---------------+
   |GEO_DEF        |
   | +-------------+
   | |GEO_CHANK[0] |
   | +-------------+
   | |    ...      |
   | +-------------+
   | |GEO_CHANK[n] |
   +-+-------------+

chank 0  グループチャンク(システム予約のチャンク)
   +---------------+
   |GEO_GROUP[0]   |
   +---------------+
   |     ...       |
   +---------------+
   |GEO_GROUP[n]   | max 32
   +---------------+
   +---------------+
   | GEO_RADIX0    |
   |   GROUP       |
   | +-------------+
   | |GEO_BLOCK[0] |
   | +-------------+
   |     ...       |
   | +-------------+
   | |GEO_BLOCK[n] |
   | +-------------+
   | |GEOM         |
   | |     リンク  |
   +-+-------------+

  */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 

#include "scn2geo.h"

char    g_input [256] = { '\0' } ;
char    g_output[256] = { '\0' } ;
FVECTOR g_division = { 2000.0f, 4000.0f, 2000.0f } ;

enum 
{
    GEO_TOOL_M_NONE      = 0,
    GEO_TOOL_M_CONVERT   = 1,
    GEO_TOOL_M_DUMP      = 2,
    GEO_TOOL_MODE_MASK   = 0x0000000f,
    GEO_TOOL_GOT_INFILE  = 0x00000010,
    GEO_TOOL_GOT_OUTFILE = 0x00000020,
    GEO_TOOL_GOT_OUTDIR  = 0x00000040,
} ;

/* 使用法 */
void	usage( void )
{
    static int shown = 0 ;

    if ( shown == 0 ) {
	printf( "\nusage : scn2geo input(*.scn) [-o output(*.geo)] [-d x y z] [-l outdir]\n"
		"\tvrs-scn-file => mgs2-geo converter\n"
		"\t<options>\n"
		"\t -o : output file name\n"
		"\t -d : set division\n"
		"\t -l : output directory name\n" ) ;
	printf( "usage : scn2geo input(*.geo)\n"
		"\tdump geo-file\n\n"  ) ;
	shown = 1 ;
    }
}

int get_option( int argc, char *argv[] )
{
    int i ;
    int flag = 0 ;
    char dir[256] ;

    for( i=1 ; i<argc ; i++ ){
		if ( argv[i][0] == '-' ){
			switch( argv[i][1] ){
			case 'l':
				if ( ++i < argc ){
					strcpy( dir, argv[i] ) ;
					flag |= GEO_TOOL_GOT_OUTDIR ;
				}
				break ;
			case 'o':
				if ( ++i < argc ){
					strcpy( g_output, argv[i] ) ;
					flag |= GEO_TOOL_GOT_OUTFILE ;
				}
				break ;
			case 'd':
				if ( ++i < argc ) g_division.vx = atoi( argv[i] ) ;
				if ( ++i < argc ) g_division.vy = atoi( argv[i] ) ;
				if ( ++i < argc ) g_division.vz = atoi( argv[i] ) ;
				break ;
			default:
				usage() ;
				printf( "Invalid option<%c> found.\n", argv[i][1] ) ;
			}
		} else {
			if ( (flag & GEO_TOOL_MODE_MASK) == 0 ) {
				char *ext, *nxt ;

				for( ext=argv[i] ; ext ; ext=nxt ) {
					if ( !(nxt = strchr( ++ext, '.' )) ) break ;
				}
				if ( ext != argv[i] && !strchr( ext, '/' ) ){
					if ( strcmp( ext, "scn" ) == 0 ){
						strcpy( g_input , argv[i] ) ;
						flag |= GEO_TOOL_M_CONVERT|GEO_TOOL_GOT_INFILE ;
					}
					if ( strcmp( ext, "geo" ) == 0 ){
						strcpy( g_input , argv[i] ) ;
						flag |= GEO_TOOL_M_DUMP| GEO_TOOL_GOT_INFILE ;
					}
					if ( strcmp( ext, "jeo" ) == 0 ){
						strcpy( g_input , argv[i] ) ;
						flag |= GEO_TOOL_M_DUMP| GEO_TOOL_GOT_INFILE ;
					}
					if ( !(flag & GEO_TOOL_GOT_INFILE) ){
						usage() ;
						printf( "Invalid file-extension for input file<%s>.\n", ext ) ;
					}
					if ( !(flag & GEO_TOOL_GOT_OUTFILE) ) {
						strcpy( g_output, g_input ) ;
						for( ext=g_output ; ext ; ext=nxt ) {
							if ( !(nxt = strchr( ++ext, '.' )) ) break ;
						}
						strcpy( ext, "geo" ) ;
					}
				} else {
					usage() ;
					printf( "There is no file-extension for input file<%s>.\n", argv[i] ) ;
				}
			}
		}
    }

    if ( flag & GEO_TOOL_GOT_OUTDIR ){
		if ( dir[strlen( dir )-1] != '/' ) {
			strcat( dir, "/" ) ;
		}
		strcat( dir, g_output ) ;
		strcpy( g_output, dir ) ;
    }

    if ( (flag & GEO_TOOL_MODE_MASK) == GEO_TOOL_M_NONE ) {
		usage() ;
		printf( "No input file!!!!!!\n" ) ;
    }

    return flag ;
}

/* メイン */
int 	main( int argc, char *argv[] )
{
    int        error = 1 ;


    /* 初期化 */
#ifdef _WINDOWS
    AttachSfdWin( ( HINSTANCE )GetModuleHandle( "Scene2HZX.exe" ) ) ;
#endif
#ifdef _UNIX
    AttachSfdUnix() ;
#endif
#ifdef _LINUX
    AttachSfdLinux() ;
#endif

    /* メモリ管理システムを上げておく */
    mem_startup() ;

    switch( get_option( argc, argv ) & GEO_TOOL_MODE_MASK ) {
    case GEO_TOOL_M_CONVERT:
    {
		HP3DMODEL  model ;
		GEO_DEF   *def   ;

		model = file_scn_load( g_input ) ;
		if ( model == NULL ) {
			printf( "reading error scn-file\n" ) ;
			break ;
		}
		def = convert_geodef( model ) ;
		if ( def == NULL ) {
			printf( "convert error\n" ) ;
			break ;
		}
		if ( file_geo_save( def, g_output ) < 0 ){
			error = 0 ;
		}
		break ;
    }
    case GEO_TOOL_M_DUMP:
    {
		GEO_DEF   *def   ;
	
		def = file_geo_load( g_input ) ;
		if ( def == NULL ) {
			printf( "file open error for dump\n" ) ;
			break ;
		}
		file_dump_radix( def ) ;
		file_dump_geo( def ) ;
		break ;
    }
    case GEO_TOOL_M_NONE:
    default:
		break ;
    }

    /* メモリ管理システムは最後に落す */
    mem_shutdown() ;


    /* 終了 */
#ifdef _WINDOWS
    DetachSfdWin() ;
#endif
#ifdef _UNIX
    DetachSfdUnix() ;
#endif
#ifdef _LINUX
    DetachSfdLinux() ;
#endif

    return error ;
}
