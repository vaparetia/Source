/*
 * 	mcget.c
 *	メモリーカード(PS2)の中身を1ファイルにしてローカルに保存
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

#define FILE_BUFFER_SIZE (32*1024)
#define MAX_FILE_NUM	(1024)
#define FILEPATH_LEN	(254)


/* read write buffer */

static char file_buffer[ FILE_BUFFER_SIZE ] __attribute__((aligned(16)));

#define Do( a )		({ int r; if( ( r = a ) < 0 ){ printf( "err at'Do()'%d\n", r ); for( ;; ); }; sceMcSync( 0, 0, &r ); r; })


static char getall_help[] = "getall [local]\n";

static int gall_header_wr( int hfd, char *base , char *filename )
{
    int len;
	char buf[ FILEPATH_LEN ];

	sprintf( buf, "%s/%s", base, filename );
    len = strlen( buf ) + 1;
    sceWrite( hfd, &len, sizeof( char ) );
    sceWrite( hfd, buf, sizeof( char ) * len );
	printf("len = %d, Path =%s\n",len,buf);

    return 0;
}

static int gall_reflexive( int hfd, char *base )
{
    int i;
    int result;
    char base_buf[ FILEPATH_LEN ];
    sceMcTblGetDir table[ MAX_FILE_NUM ] __attribute__((aligned(64)));
    sceMcTblGetDir *tbl;
    int len;

	tbl = table;
	len = strlen( base );
	
	// 情報ファイル取得
    sprintf( base_buf, "%s/*", base );
    result = Do( sceMcGetDir( 0, 0, base_buf, 0, MAX_FILE_NUM, tbl ) );
    if( result < 0 ){
		switch( result ){
		  case (-2):
			printf( "new unformat card\n" );
			return (-1);
		  default:
			printf( "Err!!(%d)\n" , result );
			return (-1);
		}
    }
    printf( "files number = %d(%s)\n" , result, base_buf );
    
    for( i = 0; i < result; i++ ){

		// まず排除
		if( strcmp( tbl->EntryName, ".") == 0 ){
			tbl ++;
			continue;
		}
		if( strcmp( tbl->EntryName, "..") == 0 ){
			tbl ++;
			continue;
		}

		//printf("%s\n", tbl->EntryName);
		
		// ヘッダ部分書き込み
		gall_header_wr( hfd, base , tbl->EntryName );

		if( tbl->AttrFile & sceMcFileAttrSubdir ){
			/* ディレクトリのとき */
			int r;
			char buf[ FILEPATH_LEN ];
			
			// 情報ファイル部分の書き込み
			sceWrite( hfd, tbl, sizeof( sceMcTblGetDir ) );
			printf( "info file size(DIR) = %d\n", sizeof( sceMcTblGetDir ) );
			
			// デイレクトリもぐる
			sprintf( buf, "%s/%s", base, tbl->EntryName );
			sprintf( base_buf, "./%s", tbl->EntryName );
			printf("%s\n",base_buf);
			r = Do( sceMcChdir( 0, 0, base_buf, 0 ) );
			if( r < 0 ){
				printf( "ERR chdir(%d)\n", r );
			}

			// 再帰
			gall_reflexive( hfd, buf );
			
			// ディレクトリ戻る
			r = Do( sceMcChdir( 0, 0, "..", 0 ) );/*いっこ上へ*/

		}else{
			/* ファイルのとき */
			int tfd;
			unsigned size;
	    
			// サイズの取得
			size = tbl->FileSizeByte;
			if( size < 0 ){
				return size;
			}
			printf( "size = %d\n", size );

			// メモリーカードのファイルオープン
			tfd = Do( sceMcOpen( 0, 0, tbl->EntryName, SCE_RDONLY ) );
			if( tfd < 0 ){
				printf( "Err Mc fileopen(%d)\n", tfd );
				return tfd;
			}

			// 情報ファイル部分の書き込み
			sceWrite( hfd, tbl, sizeof( sceMcTblGetDir ) );
			printf( "info file size = %d\n", sizeof( sceMcTblGetDir ) );
	    
			// データファイル部分の書き込み
			while( size > 0 ){
				int len;
				int r;
				len = ( size > FILE_BUFFER_SIZE ) ? FILE_BUFFER_SIZE : size;
				if( ( r = Do( sceMcRead( tfd, file_buffer, len ) ) ) < len ){
					printf( "READ ERR%d\n", r );
					sceClose( hfd );
					Do( sceMcClose( tfd ) );
					return len;
				}
				sceWrite( hfd , file_buffer, len );
		
				size -= len;
			}

			// メモリーカードのファイルクローズ
			Do( sceMcClose( tfd ) );
			
		}/*if*/
		
		tbl ++;
		
    }/*for*/
	
    return 0;
}

static int getall_cmd( int argc, char *argv[] )
{
	char hostfilename[ 256 ];

	Do( sceMcGetInfo( 0, 0, 0, 0, 0 ) );

	if( argc == 1 ){
	    sprintf( hostfilename , "host0:mcdata" );
	}else if( argc == 2 ){
	    sprintf( hostfilename, "host0:%s", argv[ 1 ] );
	}else{
	    printf( "%s", getall_help );
	    return -1;
	}
	printf( "ALL MemoryCard data ---> HDD ( %s )...\n", hostfilename );
	
	{
	    int hfd;
	    char _base[ FILEPATH_LEN ] = "";/*ファイルパス*/
	    char *base;
	    
	    base = _base;

		// host0: ファイルオープン
	    hfd = sceOpen( hostfilename, SCE_WRONLY | SCE_CREAT | SCE_TRUNC );
	    if( hfd < 0 ){
			sceClose( hfd );
			return hfd;
	    }
	    
	    // 再帰でチェック&書き込み
	    gall_reflexive( hfd, base );
	    
	    // host0: ファイルクローズ
	    sceClose( hfd );

	    return 0;
	}
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

	getall_cmd( argc, argv );

	return 0;
}
