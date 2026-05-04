/*
   main.c : scn2hzx main function
   
   1999/12/16 M.Sonoyama
   $Id: main.c,v 1.8 2001/08/09 11:49:55 usr02011 Exp $
   */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	<math.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include "Scene2HZX.h"
#include "fmt_hzx.h"

extern	HZD_DEF	*Scn2Hzx( SCENE2HZXHEADER * ) ;
extern	void	SaveHzx( HZD_DEF *, char * ) ;
extern	void	FreeHzxDef( HZD_DEF * ) ;


/* 使用法 */
void	Usage( void )
{
    printf( "\nscn2hzx : vrs-scn-file -> mgs2-hzx converter\n" ) ;
    printf( "usage : scn2hzx -i input(*.scn) -o output(*.hzx) [-k]\n" ) ;
    printf( "\t\t [-d outputdir]\n" ) ;
    printf( "\t<options>\n" ) ;
    printf( "\t -k : input file is old-scn-format\n" ) ;
    printf( "\t -l : dumphzx mode\n" ) ;
    exit ( -1 ) ;
}

/* メイン */
int 	main( int argc, char *argv[] )
{
    SCENE2HZXINFO   scene2HZXInfo ;	
    SCENE2HZXHEADER scene2HZXHeader ;
    HZD_DEF		*def, *def2 ;
    char		output[ 256 ] ;
	int			i ;

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
    /* コマンドライン解析 */
    if ( AnalyzeCommand( argc, argv, &scene2HZXInfo ) == 0 ) Usage() ;

    if ( scene2HZXInfo.DebugMode == 2 ) {
		//for ( i = 1; i < argc; i ++ ) {
			//fprintf( stdout, "%s\n", argv[ i ] ) ;
			//strcpy( scene2HZXInfo.szInputFileName, argv[ i ] ) ;
			//strcpy( scene2HZXInfo.szOutputFileName, argv[ i ] ) ;
			def = LoadHzx( scene2HZXInfo.szOutputFileName ) ;
			DumpHzx( def ) ;
			FreeHzxDef( def ) ;
		//}
		return 0 ;
    }

    /* シーンからオブジェクトを取り出す */
    if ( GetHZXObject( &scene2HZXInfo, &scene2HZXHeader ) == 0 ) {
		DestroyScene2HZXObject( &scene2HZXHeader ) ;
		printf( "Error!! Error!! Error!!\n" ) ;
		return 1 ;
    }
    /* ブロック分割を行う */
    if ( MakeHZXBlockObject( &scene2HZXHeader ) == 0 ) {
		DestroyScene2HZXObject( &scene2HZXHeader ) ;
		printf( "Error!! Error!! Error!!\n" ) ;
		return 1 ;
    }
	
    /* scn -> hzx 変換 */
    def = Scn2Hzx( &scene2HZXHeader ) ;
	if ( def == NULL ) {
		DestroyScene2HZXObject( &scene2HZXHeader ) ;
		printf( "Error!! Error!! Error!!\n" ) ;
		return 1 ;
	}

    if ( scene2HZXInfo.DebugMode == 1 ) DumpHzx( def ) ;
    if ( scene2HZXInfo.szOutputFileName[ 0 ] != '/' ) {
		strcpy( output, scene2HZXInfo.szOutputDirName ) ;
		strcat( output, "/" ) ;
		strcat( output, scene2HZXInfo.szOutputFileName ) ;
    } else {
		strcpy( output, scene2HZXInfo.szOutputFileName ) ;
    }

    /* 作成したオブジェクトを破棄する */
    DestroyScene2HZXObject( &scene2HZXHeader ) ;

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
    SaveHzx( def, output ) ;
//  def2 = LoadHzx( scene2HZXInfo.szOutputFileName ) ;
//  DumpHzx( def2 ) ;
    FreeHzxDef( def ) ;

    return 0 ;
}







