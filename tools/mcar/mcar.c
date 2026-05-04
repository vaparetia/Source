/*
 *	mcar.c
 *   	mcget で吸い出したPS2メモリーカードファイルの一覧表示
 *			Y.YANO 2001.10.24
 */

#include <stdio.h>
#include <string.h>

#define FILE_BUFFER_SIZE (32*1024)
#define MAX_FILENAME_LEN (128)

#define MODE_T  (0x0001)
#define MODE_TV (0x0002)

// PS2メモリーカード情報ファイル構造体
typedef struct {
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
    unsigned FileSizeByte;
    unsigned short AttrFile;
    unsigned short Reserve1;
    unsigned Reserve2;
    unsigned PdaAplNo;
    unsigned char EntryName[32];
} sceMcTblGetDir;
// 属性
#define Readable	0x0001
#define Writeable 	0x0002
#define Executable 	0x0004
#define DupProhibit 	0x0008
#define Subdir 		0x0020
#define Closed	 	0x0080
#define PDAExec 	0x0800
#define PS1	 	0x1000

/* Sub Routine Program */

/*---時間の文字列作成 (from /tool/mccons/main.c)---*/
static char *get_time_str( char *timedata )
{
	static unsigned char timestr[ 32 ];
	unsigned char *t;

	t = ( unsigned char * )timedata;

	sprintf( timestr, "%04d/%02d/%02d %02d:%02d:%02d"
			 , t[ 7 ] * 256 + t[ 6 ], t[ 5 ], t[ 4 ], t[ 3 ], t[ 2 ], t[ 1 ] );
	return timestr;
}

/*--- 情報を表示 ---*/
void print_info( sceMcTblGetDir *table )
{
    printf( "%c%c%c%c%c%c%c%c %8d %s   %s"
	    , ( table->AttrFile & Readable ) ? 'r' : '-'
	    , ( table->AttrFile & Writeable ) ? 'w' : '-'
	    , ( table->AttrFile & Executable ) ? 'x' : '-'
	    , ( table->AttrFile & DupProhibit ) ? 'c' : '-'
	    , ( table->AttrFile & Subdir ) ? 'd' : '-'
	    , ( table->AttrFile & Closed ) ? 'e' : '-'
	    , ( table->AttrFile & PDAExec ) ? 'p' : '-'
	    , ( table->AttrFile & PS1 ) ? '1' : '-'
	    , table->FileSizeByte
	    , get_time_str( ( char * )&table->_Modify )
	    , table->EntryName );
}

void sjis_to_euc( char *to, char *str )
{
	unsigned char *p, *s;

	p = ( unsigned char * )str;
	s = ( unsigned char * )to;
	while( *p != '\0' ){
		if( *p > 0x80 ){
			long h, l;
			h = *p;
			l = *( p + 1 );

			if( h >= 0xa0 + '@' ){
				h -= '@';
			}
			h = ( h - 0x81 ) * 2 + 1 + ' ' + 0x80;

			if( l >= 0x9f ){
				h ++;
				l = l - 0x9f + 0xa1;
			} else {
				if( l > 0x7f ){
					l --;
				}
				l = l - 0x40 + 0xa1;
			}
			*s = h;
			*( s + 1 ) = l;
			p += 2;
			s += 2;
		} else {
			p++;
			s++;
		}
	}
	*s = '\0';
}

void print_title( char *sjis_title )
{
    char euc_title[ 68 ];
    sjis_to_euc( euc_title , sjis_title );

    printf( "「%s」\n", euc_title );
    //printf( "「%s」\n", sjis_title );

    return;
}

/* Main Prigram */
int main( int argc, char **argv )
{
    FILE *fp;
    char filename[ MAX_FILENAME_LEN ];
    int mode_bit = 0;
    char title_name[ 68 ] = "" ;
    int num = 0;/*ファイルカウンタ*/

    if( argc ==2 ){

	mode_bit |= MODE_T;
	sprintf( filename, "%s", argv[ 1 ] );

    }else if( argc == 3 ){
	char *str;
	if( argv[ 1 ][ 0 ] == '-' ){
	    str = argv[ 1 ] + 1;
	}else{
	    str = argv[ 1 ];
	}

	if( strcmp( str, "t" ) == 0 ){
	    mode_bit |= MODE_T;
	}else if( strcmp( str, "tv" ) == 0 ){
	    mode_bit |= MODE_TV;
	    /*	if( strcmp( argv[ 1 ], "t" ) == 0 ){
		mode_bit |= MODE_T;
		}else if( strcmp( argv[ 1 ], "tv" ) == 0 ){
		mode_bit |= MODE_TV;*/
	}else{
	    printf( "\nmcar [command] <file>\n\t'command'\t-t   put only dierectory name\n\t\t\t-tv  put all file name\n\n" );
	    return (-1);
	}
	sprintf( filename, "%s", argv[ 2 ] );

    }else{

	printf( "\nmcar [command] <file>\n\t'command'\t-t   put only dierectory name\n\t\t\t-tv  put all file name\n\n" );
	return (-1);

    }

    // open
    fp = fopen( filename, "r" );
    if( fp == NULL ){
	printf("\nCannot file open!\n\n");
	return -1;
    }
    
    // ファイルの最後までループして、読みだし&print
    while( 1 ){
	int n;
	char len;
	char filepath[ MAX_FILENAME_LEN ];
	sceMcTblGetDir info_file;
	int size;
	static char file_buffer[ FILE_BUFFER_SIZE ] __attribute__((aligned(16)));
	
	// ファイルパス文字列長
	n = fread( &len, sizeof(char), 1, fp );
	if( n < 1 ){
	    // 最後のディレクトリのタイトル名表示
	    if( strcmp( title_name, "" ) != 0 ){/*何も入ってない時は行なわない*/
		print_title( title_name );
	    }
	    // ファイル数表示
	    printf( "\n総ディレクトリ数 は %d個 です。\n", num );
//	    printf( "\n\nFile end (%d)\n", n );
	    break;
	}

	// ファイルパス文字列
	n = fread( &filepath, sizeof(char), len, fp );
	if( n < len ){
	    printf( "fread Err!(%d)", n );
	    return (-1);
	}

	// 情報ファイル
	n = fread( &info_file, sizeof(sceMcTblGetDir), 1, fp );
	if( n < 1 ){
	    printf( "fread Err!(%d)", n );
	    return (-1);
	}

	// 表示
	if( info_file.AttrFile & Subdir ){
	    /* ディレクトリのとき */
	    // 一つ前のディレクトリのタイトル名表示
	    if( strcmp( title_name, "" ) != 0 ){/*何も入ってない時は行なわない*/
		print_title( title_name );
	    }
	    
	    // ディレクトリの情報表示
	    printf( "\n" );
	    print_info( &info_file );
	    printf( "/\n" );

	    num ++;/*ファイル数*/

	}else{
	    /* ファイルのとき */
	    if( mode_bit & MODE_TV ){
		//printf( " " ); 
		print_info( &info_file );
		printf( "\n" );
	    }
	}

	// データファイル
	size = info_file.FileSizeByte;
	while( size > 0 ){
	    int len;

	    len = ( size > FILE_BUFFER_SIZE ) ? FILE_BUFFER_SIZE :size;
	    n = fread( file_buffer, len, 1 , fp );
	    if( n < 1 ){
		printf( "fread Err!(%d)", n );
		return (-1);
	    }
	    size -= len;
	}

	// icon.sys からタイトル名取得
	if( strcmp( info_file.EntryName , "icon.sys" ) == 0 ){
	    strncpy( title_name, &file_buffer[ 192 ] , 68 );/*この時点ではSJIS*/
	}
	
    }/*while*/

    
    // close
    fclose( fp );

    return 0;
}
