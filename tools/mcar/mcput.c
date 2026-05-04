/*
 * 	mcput.c
 *	mcget でローカルに保存したファイルを再びメモリーカード上に戻す
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
struct {
	unsigned char Resv2,Sec,Min,Hour;
	unsigned char Day,Month;
	unsigned short Year;
} _Create;
struct {
	unsigned char Resv2,Sec,Min,Hour;
	unsigned char Day,Month;
	unsigned short Year;
} _Modify;
/* read write buffer */

static char file_buffer[ FILE_BUFFER_SIZE ] __attribute__((aligned(16)));

#define Do( a )		({ int r; if( ( r = a ) < 0 ){ printf( "err at'Do()'%d\n", r ); for( ;; ); }; sceMcSync( 0, 0, &r ); r; })


static char putall_help[] = "putall <local>\n";

static int putall_cmd( int argc, char *argv[] )
{
	char hostfilename[ 256 ];

	Do( sceMcGetInfo( 0, 0, 0, 0, 0 ) );

	if( argc == 2 ){
	    sprintf( hostfilename, "host0:%s", argv[ 1 ] );
	}else{
	    printf( "%s", putall_help );
	    return -1;
	}
	printf( "ALL MemoryCard data ---> MemoryCard ( %s )...\n", hostfilename );
	
	{
	    int hfd;
	    int res;
		sceMcTblGetDir DirModify_buf;/*ディレクトリ更新日時記憶用*/
		sprintf(DirModify_buf.EntryName, "" );

		// host0:ファイルオープン
	    hfd = sceOpen( hostfilename, SCE_RDONLY );
	    if( hfd < 0 ){
			sceClose( hfd );
			return hfd;
	    }

	    // フォーマット
	    res = Do( sceMcFormat( 0, 0 ) );
	    if( res < 0 ){
		printf("Format err!(%d)\n", res );
	    }
	    
	    // チェック&書き込み
		while( 1 ){
			int end_check;
			char len;
			char filepath[ FILEPATH_LEN ];
			sceMcTblGetDir info_data __attribute__((aligned (64)));
			char *p_info;
			int result;
			
			// ファイルパス名文字列長チェック
			end_check = sceRead( hfd, &len, sizeof(char) );
			printf("len = %d, end = %d\n",len,end_check);
			if( end_check <= 0 ){
				printf( " File end or Read err\n" );
				break;
			}

			// ファイルパス名チェック
			end_check = sceRead( hfd, filepath, sizeof(char) * len );
			printf("end2 = %d\n",end_check);

			// 情報ファイルチェック
			sceRead( hfd, &info_data, sizeof(sceMcTblGetDir) );
			printf( "create = %d%d, modify = %d%d\n", info_data.CMonth,info_data.CDay,info_data.MMonth,info_data.MDay );
			if( info_data.AttrFile & sceMcFileAttrSubdir ){
				/*ディレクトリのとき*/
				int r;
				printf("dirname = %s\n",filepath);
				r = Do( sceMcMkdir( 0, 0, filepath ) );
				if( r < 0 ){
					printf( "MC mkdir err!(%d)\n", r );
					sceClose( hfd );
					return (-1);
				}

				// 以前のディレクトリの更新日時を書き込む
				if( strcmp(DirModify_buf.EntryName,"") != 0){
					char *tbl;
					char buf[ MAX_FILE_NUM ];
					sprintf( buf, "/%s", DirModify_buf.EntryName );
					tbl = (char *)(&DirModify_buf);
					result = Do( sceMcSetFileInfo( 0, 0,
												   buf,
												   tbl,
												   sceMcFileInfoModify ) );
					if( result < 0 ){
						printf( "MC set file info ERR!(%d)\n", result );
						sceClose( hfd );
						return (-1);
					}
				}
				
				// 更新日時記録
				DirModify_buf = info_data;
				
			}else{
				/*ファイルのとき*/
				int tfd;
				int size;
				
				// メモリーカードのファイルオープン
				printf("filename = %s(%s)\n",filepath,info_data.EntryName);
				tfd = Do( sceMcOpen( 0, 0, filepath, SCE_WRONLY | SCE_CREAT ) );
				if( tfd < 0 ){
					printf( "MC file open err!(%d)\n", tfd );
					sceClose( hfd );
					return (-1);
				}

				// ファイルサイズの取得
				size = info_data.FileSizeByte;

				// HDDから読み込み
				while( size > 0 ){
					int len;
					len = ( size > FILE_BUFFER_SIZE ) ? FILE_BUFFER_SIZE : size;
					sceRead( hfd, file_buffer, len );
					if( Do( sceMcWrite( tfd, file_buffer, len ) ) < len ){
						printf( "MC write err!(%d)\n", len );
						sceClose( hfd );
						Do( sceMcClose( tfd ) );
						return (-1);
					}
					size -= len;
				}
				
				// メモリーカードのファイルクローズ
				Do( sceMcClose( tfd ) );
						
			}/*if*/

			// 情報ファイルの書き込み
			p_info = (char *)(&info_data);
			result = Do( sceMcSetFileInfo( 0, 0, filepath, p_info,
										   sceMcFileInfoCreate |
										   sceMcFileInfoModify |
										   sceMcFileInfoAttr			) );
			if( result < 0 ){
				printf( "MC ser filr info ERR!(%d)\n", result );
				sceClose( hfd );
				return (-1);
			}
			
		}/*while*/
	    
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

	putall_cmd( argc, argv );
	
	return 0;
}
