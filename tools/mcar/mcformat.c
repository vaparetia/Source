/*
 * 	mcformat.c
 *	メモリーカードをフォーマット
 *
 *				Y.YANO 2001.10.24
 */

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libmc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Do( a )		({ int r; if( ( r = a ) < 0 ){ printf( "err at'Do()'%d\n", r ); for( ;; ); }; sceMcSync( 0, 0, &r ); r; })


static char format_help[] = "\nmcformat [command]\n\t-f: Format\n\t-uf:Unformat\n";

static int format_cmd( int argc, char *argv[] )
{
	int res ;
	
	res = Do( sceMcGetInfo( 0, 0, 0, 0, 0 ) );
	if( res < -1 ){
		printf( "GetInfo err!(%d)\n", res );
		if( res == -2 ){
			printf( "This MEMORYCARD(PS2) WAS Unformatted !!\n" );
		}else if( res < -10 ){
			printf( "Please insert MEMORYCARD(PS2) in PORT 1.\n" );
		}
		return (-1);
	}

	if( argc > 2 ){
	    printf( "%s\n", format_help );
	    return -1;
	}

	if( argc == 1 ){
	    // フォーマット
		printf( "Format......\n" );
	    res = Do( sceMcFormat( 0, 0 ) );
	    if( res < 0 ){
			printf("Format err!(%d)\n", res );
			return (-1);
	    }
		printf( "Format OK.\n" );
		
	}else if( argc == 2 ){
	    	char *str;
		if( argv[ 1 ][ 0 ] == '-' ){
		    str = argv[ 1 ] + 1;
		}else{
		    str = argv[ 1 ];
		}

		if( strcmp( str , "uf" ) == 0 ){
			// アンフォーマット
			printf( "Unformat......\n" );
			res = Do( sceMcUnformat( 0, 0 ) );
			if( res < 0 ){
				printf("Format err!(%d)\n", res );
				return (-1);
			}
			printf( "Unformat OK.\n" );

		}else if( strcmp( str , "f" ) == 0 ){
			// フォーマット
			printf( "Format......\n" );
			res = Do( sceMcFormat( 0, 0 ) );
			if( res < 0 ){
				printf("Format err!(%d)\n", res );
				return (-1);
			}
			printf( "Format OK.\n" );
			
		}else{
			// ヘルプ表示
			printf( "%s\n", format_help );
			return (-1);
		}
			
	}else{
		printf( "command err!\n" );
		return (-1);
	}
					
	return (0);
}

/* ---------------------------------------------------------------------- */
/*
  Boot
*/

#define SYSTEMPATH "host0:/usr/local/sce/iop/modules/"

int main( int argc, char **argv )
{
	sceDevVif0Reset();
	sceDevVif1Reset();
	sceDevVu0Reset();
	sceDevVu1Reset();
	sceGsResetPath();

	EnableCache( INST_CACHE | DATA_CACHE );
	sceDmaReset( 1 );

	sceSifInitRpc( 0 );

	sceSifLoadModule( SYSTEMPATH "sio2man.irx", NULL, 0 );
	sceSifLoadModule( SYSTEMPATH "mcman.irx", NULL, 0 );
	sceSifLoadModule( SYSTEMPATH "mcserv.irx", NULL, 0 );

	Do( sceMcInit() );

	format_cmd( argc, argv );
	
	return 0;
}
