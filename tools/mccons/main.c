/*
	メモリーカードをコンソールでいじる。
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

static int mounted_flag = 0;

/* read write buffer */

static char file_buffer[ FILE_BUFFER_SIZE ] __attribute__((aligned(16)));

#define Do( a )		({ int r; if( ( r = a ) < 0 ){ printf( "err %d\n", r ); for( ;; ); }; sceMcSync( 0, 0, &r ); r; })

/* ---------------------------------------------------------------------- */
/*
	各コマンド
*/

/* サブルーチン */

static void get_name( char *dest, char *dev, char *src )
{
	if( strncmp( src, dev, strlen( dev ) ) != 0 ){
		if( strchr( src, ':' ) != NULL ){
			printf( "set device '%s'\n", dev );
			return;
		}
		sprintf( dest, "%s%s", dev, src );
	} else {
		strcpy( dest, src );
	}
}

static char *get_file_base( char *file )
{
	char *p;
	if( ( p = strrchr( file, '/' ) ) == NULL ){
		char *pp;
		if( ( pp = strrchr( file, ':' ) ) != NULL ){
			return pp + 1;
		} else {
			return file;
		}
	}
	return p + 1;
}

static char *get_mode_str( int mode )
{
	static char modestr[ 16 ];

	if( mode & SCE_STM_FLNK ){
		modestr[ 0 ] = 'l';
	} else if( mode & SCE_STM_FREG ){
		modestr[ 0 ] = '-';
	} else if( mode & SCE_STM_FDIR ){
		modestr[ 0 ] = 'd';
	} else {
		modestr[ 0 ] = '?';
	}

	{
		static const char acs[] = "rwxrwxrwx";
		int i;
		int mask;
		mask = SCE_STM_RUSR;
		for( i = 0; i < 3 * 3; i++ ){
			int c;
			if( mode & mask ){
				c = acs[ i ];
			} else {
				c = '-';
			}
			modestr[ 1 + i ] = c;
			mask >>= 1;
		}
	}
	modestr[ 10 ] = '\0';

	return modestr;
}

static char *get_time_str( char *timedata )
{
	static unsigned char timestr[ 32 ];
	unsigned char *t;

	t = ( unsigned char * )timedata;

	sprintf( timestr, "%04d/%02d/%02d %02d:%02d:%02d"
			 , t[ 7 ] * 256 + t[ 6 ], t[ 5 ], t[ 4 ], t[ 3 ], t[ 2 ], t[ 1 ] );
	return timestr;
}

/* ---------------------------------------------------------------------- */
/*
	各コマンド
*/

static char info_help[] = "info\n";

static int info_cmd( int argc, char *argv )
{
	int type, free, format;
	int result;

	result = Do( sceMcGetInfo( 0, 0, &type, &free, &format ) );
	if( result == 0 ){
		printf( "same card\n" );
	} else if( result == -1 ){
		printf( "new formatted card\n" );
	} else if( result == -2 ){
		printf( "new unformat card\n" );
	} else {
		printf( "Error %d\n", result );
	}

	switch( type ){
	  case 0:
		printf( "no card\n" );
		break;
	  case 1:
		printf( "PlayStation Card\n" );
		break;
	  case 2:
		printf( "PlayStation2 Card\n" );
		break;
	  case 3:
		printf( "PocketStation\n" );
		break;
	}
	printf( "Free = %d\n", free );

	return 0;
}

static char format_help[] = "format\n";

static int format_cmd( int argc, char *argv[] )
{
	int res;
	res = Do( sceMcFormat( 0, 0 ) );

	return res;
}

static char unformat_help[] = "unformat\n";

static int unformat_cmd( int argc, char *argv[] )
{
	int res;
	res = Do( sceMcUnformat( 0, 0 ) );

	return res;
}

static char ls_help[] = "ls [dir]\n";

static sceMcTblGetDir table __attribute__((aligned(64)));

static int ls_cmd( int argc, char *argv[] )
{
	char path[ 64 ];
	int result;
	int start;

	if( argc < 2 ){
		sprintf( path, "./*" );
	} else {
		if( argv[ 1 ][ strlen( argv[ 1 ] ) - 1 ] == '/' ){
			sprintf( path, "%s*", argv[ 1 ] );
		} else {
			sprintf( path, "%s/*", argv[ 1 ] );
		}
	}

	printf( "%s:\n", path );

	start = 0;
	for( ;; ){
		result = Do( sceMcGetDir( 0, 0, path, start, 1, &table ) );
		start = 1;
		if( result < 0 ){
			return result;
		}
		if( result == 0 ){
			break;
		}

		printf( "%c%c%c%c%c%c%c%c %8d %s %s\n"
				, ( table.AttrFile & sceMcFileAttrReadable ) ? 'r' : '-'
				, ( table.AttrFile & sceMcFileAttrWriteable ) ? 'w' : '-'
				, ( table.AttrFile & sceMcFileAttrExecutable ) ? 'x' : '-'
				, ( table.AttrFile & sceMcFileAttrDupProhibit ) ? 'c' : '-'
				, ( table.AttrFile & sceMcFileAttrSubdir ) ? 'd' : '-'
				, ( table.AttrFile & sceMcFileAttrClosed ) ? 'e' : '-'
				, ( table.AttrFile & sceMcFileAttrPDAExec ) ? 'p' : '-'
				, ( table.AttrFile & sceMcFileAttrPS1 ) ? '1' : '-'
				, table.FileSizeByte
				, get_time_str( ( char * )&table._Modify )
				, table.EntryName );
	}
	return 0;
}

static char chdir_help[] = "chdir dir\n";

static int chdir_cmd( int argc, char *argv[] )
{
	int result;

	if( argc < 2 ){
		printf( "chdir <dir>\n" );
		return -1;
	}

	result = Do( sceMcChdir( 0, 0, argv[ 1 ], NULL ) );

	return result;
}

static char mkdir_help[] = "mkdir <dir>\n";

static int mkdir_cmd( int argc, char *argv[] )
{
	int result;

	if( argc < 2 ){
		printf( "mkdir <dir>\n" );
		return -1;
	}

	result = Do( sceMcMkdir( 0, 0, argv[ 1 ] ) );

	return result;
}

static char rm_help[] = "rm <file>\n";

static int rm_cmd( int argc, char *argv[] )
{
	int result;
	if( argc < 2 ){
		printf( "mkdir <dir>\n" );
		return -1;
	}

	result = Do( sceMcDelete( 0, 0, argv[ 1 ] ) );

	return result;
}

static char rename_help[] = "rename <file1> <file2>";

static int rename_cmd( int argc, char *argv[] )
{
	int result;
	if( argc < 3 ){
		printf( "%s\n", rename_help );
		return -1;
	}

	result = Do( sceMcRename( 0, 0, argv[ 1 ], argv[ 2 ] ) );

	return result;
	
}


static char put_help[] = "put <local> [mcfile]\n";

static int put_cmd( int argc, char *argv[] )
{
	char hostfilename[ 256 ];
	char *targetfilename;
	int hfd, tfd;
	int size;

	if( argc < 2 ){
		printf( "%s", put_help );
		return -1;
	}
	sprintf( hostfilename, "host0:%s", argv[ 1 ] );
	if( argc < 3 ){
		targetfilename = argv[ 1 ];
	} else {
		targetfilename = argv[ 2 ];
	}

	printf( "put %s -> %s\n", hostfilename, targetfilename );

	// open
	hfd = sceOpen( hostfilename, SCE_RDONLY );
	if( hfd < 0 ){
		return hfd;
	}
	// サイズの取得
	size = sceLseek( hfd, 0, SEEK_END );
	sceLseek( hfd, 0, SEEK_SET );
	
	tfd = Do( sceMcOpen( 0, 0, targetfilename, SCE_WRONLY | SCE_CREAT ) );
	if( tfd < 0 ){
		sceClose( hfd );
		return tfd;
	}

	while( size > 0 ){
		int len;
		len = ( size > FILE_BUFFER_SIZE ) ? FILE_BUFFER_SIZE : size;
		sceRead( hfd, file_buffer, len );
		if( Do( sceMcWrite( tfd, file_buffer, len ) ) < len ){
			sceClose( hfd );
			Do( sceMcClose( tfd ) );
			return len;
		}
		size -= len;
	}

	Do( sceMcFlush( tfd ) );
	Do( sceMcClose( tfd ) );
	sceClose( hfd );

	return 0;
}

static char get_help[] = "get <mcfile> [local]\n";

static int get_cmd( int argc, char *argv[] )
{
	char hostfilename[ 256 ];
	char *targetfilename;
	int hfd, tfd;
	int size;

	if( argc < 2 ){
		printf( "%s", put_help );
		return -1;
	}
	targetfilename = argv[ 1 ];
	if( argc < 3 ){
		sprintf( hostfilename, "host0:%s", argv[ 1 ] );
	} else {
		sprintf( hostfilename, "host0:%s", argv[ 2 ] );
	}

	printf( "get %s -> %s\n", targetfilename, hostfilename );

	tfd = Do( sceMcOpen( 0, 0, targetfilename, SCE_RDONLY ) );
	if( tfd < 0 ){
		return tfd;
	}

	// サイズの取得
	size = Do( sceMcSeek( tfd, 0, SEEK_END ) );
	if( size < 0 ){
		Do( sceMcClose( tfd ) );
		return size;
	}
	Do( sceMcSeek( tfd, 0, SEEK_SET ) );

	// open
	hfd = sceOpen( hostfilename, SCE_WRONLY | SCE_CREAT );
	if( hfd < 0 ){
		Do( sceMcClose( tfd ) );
		return hfd;
	}

	while( size > 0 ){
		int len;
		int r;
		len = ( size > FILE_BUFFER_SIZE ) ? FILE_BUFFER_SIZE : size;
		if( ( r = Do( sceMcRead( tfd, file_buffer, len ) ) ) < len ){
			printf( "READ %d\n", r );
			sceClose( hfd );
			Do( sceMcClose( tfd ) );
			return len;
		}
		sceWrite( hfd, file_buffer, len );

		size -= len;
	}

	Do( sceMcClose( tfd ) );
	sceClose( hfd );

	return 0;
}



/* ---------------------------------------------------------------------- */
/*
	main thread
*/

typedef struct {
	char *key;
	int ( *func )( int argc, char *argv[] );
	char *help;
} COMMAND_TABLE;

static int help_cmd( int argc, char *argv[] );

static COMMAND_TABLE command_table[] = {
	{ "info", info_cmd, info_help },
	{ "format", format_cmd, format_help },
	{ "unformat", unformat_cmd, unformat_help },
	{ "ls", ls_cmd, ls_help },
	{ "rm", rm_cmd, rm_help },
	{ "cd", chdir_cmd, chdir_help },
	{ "mkdir", mkdir_cmd, mkdir_help },
	{ "rename", rename_cmd, rename_help },
	{ "put", put_cmd, put_help },
	{ "get", get_cmd, get_help },
	{ NULL, NULL, NULL }
};

static int help_cmd( int argc, char *argv[] )
{
	COMMAND_TABLE *cp;

	for( cp = command_table; cp->key != NULL; cp ++ ){
		printf( cp->help );
	}
	return 0;
}

static void do_command( char *linebuf )
{
	static char arg_buffer[ 256 ];
	static char *arg_ptr[ 16 ];

	char *s, *p;
	int ap = 0;

	p = arg_buffer;
	s = linebuf;

	for( ap = 0; ap < 16 && *s != '\0'; ap++ ){
		arg_ptr[ ap ] = p;
		while( !isspace( *s ) && *s != '\0' ){
			if( *s == '\"' ){
				s++;
				while( *s != '\"' ){
					*( p++ ) = *( s++ );
				}
				s++;
				continue;
			}
			*( p++ ) = *( s++ );
		}
		*( p++ ) = '\0';
		while( isspace( *s ) && *s != '\0' ){
			s++;
		}
	}

	{
		COMMAND_TABLE *cp;

		for( cp = command_table; cp->key != NULL; cp++ ){
			if( strcmp( cp->key, arg_ptr[ 0 ] ) == 0 ){
				int r;
				r = ( *cp->func )( ap, arg_ptr );
				if( r != 0 ){
					printf( "Error %d\n", r );
				}
				return;
			}
		}
	}
	printf( "no command '%s'\n", arg_ptr[ 0 ] );
}

static void get_line( char *buffer, int max )
{
	int i;
	for( i = 0; i < max - 1; i++ ){
		int c;
		c = getchar();
		if( c == 0x0a || c == 0x0d ){
			break;
		}
		buffer[ i ] = c;
	}
	buffer[ i ] = '\0';
}

static void mainthread( void )
{
	static char line_buffer[ 256 ];

	printf( "\n\n" );
	printf( "This program must go on Emacs Shell Window.\n" );
	printf( "port 0 only.\n" );
	printf( "\n" );

	Do( sceMcInit() );

	for( ;; ){
		printf( "ok.\n" );
		get_line( line_buffer, 256 );
		do_command( line_buffer );
	}
}

static void putfile( char *filename )
{
	int result;

	printf( "PUT FILE %s...\n", filename );

	Do( sceMcInit() );

	{
		int type, free, format;
		result = Do( sceMcGetInfo( 0, 0, &type, &free, &format ) );
		if( result == 0 ){
			printf( "same card\n" );
		} else if( result == -1 ){
			printf( "new formatted card\n" );
		} else if( result == -2 ){
			printf( "new unformat card\n" );
		} else {
			printf( "Error %d\n", result );
		}

		switch( type ){
		  case 0:
			printf( "no card\n" );
			break;
		  case 1:
			printf( "PlayStation Card\n" );
			break;
		  case 2:
			printf( "PlayStation2 Card\n" );
			break;
		  case 3:
			printf( "PocketStation\n" );
			break;
		}
		printf( "Free = %d\n", free );

		if( result == -2 ){
			printf( "FORMATING...\n" );
			format_cmd( 0, NULL );
		}
	}

	{
		char *argv[ 2 ] = {
			"put", filename
		};
		put_cmd( 2, argv );
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

	if( argc < 2 ){
		mainthread();
	} else {
		putfile( argv[ 1 ] );
	}
	return 0;
}
