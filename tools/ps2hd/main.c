#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <kernel.h>
//#include <timerold.h>
#include <dirent.h>

#define FILE_BUFFER_SIZE (32*1024)

static int mounted_flag = 0;

/* read write buffer */

static char file_buffer[ FILE_BUFFER_SIZE ] __attribute__((aligned(16)));

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

static char *get_size_str( int size )
{
	static char buf[ 64 ];
	if ( size >= 0x00200000){
		sprintf(buf, "%4dGB", size/0x00200000);
	} else {
		sprintf(buf, "%4dMB", size/0x00000800);
	}
	return buf;
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

/* 時間計測用 */

static struct SysClock clk;

static void time_set( void )
{
	GetSystemTime( &clk );
}

static int time_end( void )
{
	struct SysClock end;
	int sec1, usec1;
	int sec2, usec2;
	int sec, usec;

	GetSystemTime( &end );

	SysClock2USec( &clk, &sec1, &usec1 );
	SysClock2USec( &end, &sec2, &usec2 );

	sec = sec2 - sec1;
	usec = usec2 - usec1;
	if( usec < 0 ){
		sec --;
		usec += 1000000;
	}
	return sec * 100 + usec / 10000;
}

/* ---------------------------------------------------------------------- */
/*
	format
*/

static char allformat_help[] = \
	"allformat\n"
	"  hdd0:をformatします\n";

static int allformat_cmd( int argc, char *argv[] )
{
	printf( "format hdd0:\n" );
	return format( "hdd0:", NULL, NULL, 0 );
}

/* ---------------------------------------------------------------------- */
/*
	pdir
*/

static char pdir_help[] = \
	"pdir\n"
	"  hdd0:のパーティション情報を表示します\n";

static int pdir_cmd( int argc, char *argv[] )
{
    int i, r, n, fd;
    char desc[128];
    char size[16];
    char type[16];
    struct sce_dirent dirbuf;
    struct FsType {
		char   *str;
		u_short type;
    } fstype[] = {
		{"EMPTY",    0x0000},
		{"PFS",      0x0100},
		{"EXT2",     0x0083},
		{"EXT2SWAP", 0x0082},
    };

    dirbuf.d_stat.st_private[0] = (int)desc;

    if ((fd = dopen( "hdd0:" )) < 0) {
		printf("cannot open device %s, %d\n", "hdd0:", fd);
		return fd;
    }
    printf("No      Size       Fs sub Id               desc\n");

    n = 0;
    while ((r = dread(fd, &dirbuf)) > 0) {
		if (dirbuf.d_stat.st_size >= 0x00200000){
			sprintf(size, "%4dGB", dirbuf.d_stat.st_size/0x00200000);
		} else {
			sprintf(size, "%4dMB", dirbuf.d_stat.st_size/0x00000800);
		}

		strcpy(type, "UNKNOWN");
		for (i=0; i<sizeof(fstype)/sizeof(struct FsType); i++){
			if (dirbuf.d_stat.st_mode == fstype[i].type){
				strcpy(type, fstype[i].str);
			}
		}

		printf("%03d:%c %s %8s  %2d %-16s %s\n", n++,
			   (dirbuf.d_stat.st_attr & 0x01) ? 'S' : 'M',
			   size, type, dirbuf.d_stat.st_private[1], dirbuf.d_name, desc);
//	printf( " CREATE %s\n", get_time_str( dirbuf.d_stat.st_ctime ) );
//	printf( " ACCESS %s\n", get_time_str( dirbuf.d_stat.st_atime ) );
//	printf( " MODIFY %s\n", get_time_str( dirbuf.d_stat.st_mtime ) );
    }
    dclose(fd);
	if( r < 0 ) return r;

	{
		int maxsector, sectornum;
		maxsector = devctl( "hdd0:", HDIOC_MAXSECTOR, NULL, 0, NULL, 0 );
		sectornum = devctl( "hdd0:", HDIOC_TOTALSECTOR, NULL, 0, NULL, 0 );

		printf( "maxsector %X sectornum %X\n", maxsector, sectornum );
		printf( "TOTAL SIZE = %dGB\n", ( sectornum / 2 / 1024 / 1024 ) );
		printf( "MAX PART SIZE = %dMB\n", maxsector / 2 / 1024 );
		printf( "partsize >= 128M\n" );
	}

	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	pinfo
*/

static char pinfo_help[] =
	"pinfo <パーティション名>\n"
	"  パーティション情報の表示\n";

static int pinfo_cmd( int argc, char *argv[] )
{
	char buf[ 128 ];
	struct sce_stat stat;
	int err;

	if( argc < 1 ){
		return 1;
	}
	get_name( buf, "hdd0:", argv[ 1 ] );
	printf( "status %s\n", buf );

	err = getstat( buf, &stat );
	if( err != 0 ){
		return err;
	}

	printf( "SIZE %s\n", get_size_str( stat.st_size ) );
//	printf( "MODE %s\n", get_mode_str( stat.st_mode ) );
	printf( "MODE %08X\n", stat.st_mode );

	printf( "ATTR %08X\n", stat.st_attr );
	printf( "size %8X%08X\n", stat.st_hisize, stat.st_size );

	printf( "CREATE %s\n", get_time_str( stat.st_ctime ) );
	printf( "ACCESS %s\n", get_time_str( stat.st_atime ) );
	printf( "MODIFY %s\n", get_time_str( stat.st_mtime ) );

	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	create
*/

static char create_help[] = \
	"create <パーティション名>,read_pass,full_pass,<SIZE>,PFS,name\n"
	"  パーティションを作成\n";

static int create_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];
	int fd;

	if( argc < 2 ){
		return 1;
	}

	get_name( buf, "hdd0:", argv[ 1 ] );
	printf( "open %s\n", buf );
	fd = open( buf, O_CREAT | O_RDWR );
	if( fd < 0 ){
		return fd;
	}

	close( fd );
	return 0;
}

static char addsub_help[] = \
	"addsub <パーティション名>,full_pass <size>\n";

static int addsub_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];
	int fd;
	int r;

	if( argc < 3 ){
		return 1;
	}

	get_name( buf, "hdd0:", argv[ 1 ] );
	fd = open( buf, O_RDWR );
	if( fd < 0 ){
		return fd;
	}

	r = ioctl2( fd, HIOCADDSUB, argv[ 2 ], strlen( argv[ 2 ] ), NULL, 0 );
	close( fd );

	return r;
}

/* ---------------------------------------------------------------------- */
/*
	remove
*/

static char remove_help[] = \
	"remove <パーティション名>,full_pass\n"
	"  パーティションの削除\n";

static int remove_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];

	if( argc < 2 ){
		return 1;
	}
	get_name( buf, "hdd0:", argv[ 1 ] );
	printf( "remove %s\n", buf );
	return remove( buf );
}

/* ---------------------------------------------------------------------- */
/*
	format
*/

static char format_help[] =
	"format <パーティション名>,full_pass [zonesize]\n"
	"  パーティションのフォーマット\n";

static int format_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];
	int zonesize = 8192;

	if( argc < 2 ){
		return 1;
	}
	get_name( buf, "hdd0:", argv[ 1 ] );
	if( argc == 3 ){
		zonesize = atoi( argv[ 2 ] );
	}
	printf( "format pfs: %s %d\n", buf, zonesize );
	return format( "pfs:", buf, &zonesize, sizeof( zonesize ) );
}

/* ---------------------------------------------------------------------- */
/*
	mount
*/

static char mount_help[] =
	"mount <パーティション名>,pass\n"
	"  パーティションをpfs0:にマウント\n";

static int mount_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];
	int flag;
	int r;

	if( argc < 2 ){
		return 1;
	}
	if( mounted_flag ){
		printf( "already mounted\n" );
		return 0;
	}
	flag = SCE_MT_RDWR;
	if( argc == 3 ){
		flag = atoi( argv[ 2 ] );
	}

	get_name( buf, "hdd0:", argv[ 1 ] );
	printf( "mount pfs0: %s %x\n", buf, flag );
	r = mount( "pfs0:", buf, flag, NULL, 0 );
	if( r == 0 ){
		mounted_flag = 1;
	}
	return r;
}

/* ---------------------------------------------------------------------- */
/*
	umount
*/

static char umount_help[] = \
	"umount\n"
	"  pfs0:に割り当てられているパーティションをunmount\n";

static int umount_cmd( int argc, char *argv[] )
{
	int r;
	if( !mounted_flag ){
		printf( "no mounted\n" );
		return 0;
	}
	r = umount( "pfs0:" );
	if( r == 0 ){
		mounted_flag = 0;
	}
	return r;
}

/* ---------------------------------------------------------------------- */
/*
	dir
*/

static char dir_help[] =
	"dir[ls]\n"
	"  カレントのディレクトリ一覧を見る\n";

static int dir_cmd( int argc, char *argv[] )
{
	int fd, r;
	struct sce_dirent dirbuf;

	if( ( fd = dopen( "pfs0:." ) ) < 0 ){
		printf( "can't open dir .\n" );
		return fd;
	}
	while( ( r = dread( fd, &dirbuf ) ) > 0 ){
		int mode;
		int attr;

		mode = dirbuf.d_stat.st_mode;
		attr = dirbuf.d_stat.st_attr;

		{
			unsigned char *t;

			t = dirbuf.d_stat.st_mtime;
			printf( "%s %12d %s %c %s\n"
					, get_mode_str( mode ), dirbuf.d_stat.st_size
					, get_time_str( dirbuf.d_stat.st_mtime )
					, ( attr & 1 ) ? 'd' : '-'
					, dirbuf.d_name );
		}
	}
	dclose( fd );
	{
		int zonesize;
		int freezone;
		zonesize = devctl( "pfs0:", PDIOC_ZONESZ, NULL, 0, NULL, 0 );
		freezone = devctl( "pfs0:", PDIOC_ZONEFREE, NULL, 0, NULL, 0 );
		printf( "FreeZone = %d ZoneSize = %d FreeSize = %dK\n"
				, freezone, zonesize
				, ( zonesize / 1024 ) * freezone );
	}
	return r;
}

/* ---------------------------------------------------------------------- */
/*
	cd remove
*/

static char cd_help[] =
	"cd <ディレクトリ>\n"
	"  カレントディレクトリ変更\n";

static int cd_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];

	if( argc < 2 ){
		return 1;
	}

	get_name( buf, "pfs0:", argv[ 1 ] );
	printf( "cd %s\n", buf );
	return chdir( buf );
}

static char mkdir_help[] =
	"mkdir <ディレクトリ>\n"
	"  ディレクトリ作成\n";

static int mkdir_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];
	int mask;

	if( argc < 2 ){
		return 1;
	}
	mask = 0777;
	if( argc == 3 ){
		mask = strtol( argv[ 1 ], NULL, 0 );
	}

	get_name( buf, "pfs0:", argv[ 1 ] );
	printf( "mkdir %s %o\n", buf, mask );
	return mkdir( buf, mask );
}

static char rmdir_help[] =
	"rmdir <ディレクトリ>\n"
	"  ディレクトリ削除\n";

static int rmdir_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];

	if( argc < 2 ){
		return 1;
	}

	get_name( buf, "pfs0:", argv[ 1 ] );
	printf( "cd %s\n", buf );
	return rmdir( buf );
}

static char delete_help[] =
	"delete <ファイル名>\n"
	"  ファイル削除\n";

static int delete_cmd( int argc, char *argv[] )
{
	char buf[ 256 ];

	if( argc < 2 ){
		return 1;
	}

	get_name( buf, "pfs0:", argv[ 1 ] );
	printf( "delete %s\n", buf );
	return remove( buf );
}

/* ---------------------------------------------------------------------- */
/*
	put, get
*/

static int copy_file( char *to, char *from )
{
	int infd, outfd, size, s;

	infd = open( from, O_RDONLY, 0 );
	if( infd < 0 ){
		printf( "read open error\n" );
		return infd;
	}
	size = lseek( infd, 0, SEEK_END );
	if( size < 0 ){
		printf( "seek error\n" );
		return size;
	}
	lseek( infd, 0, SEEK_SET );
	outfd = open( to, O_WRONLY | O_CREAT, 0777 );

	time_set();

	s = size;
	while( s > 0 ){
		int l;
		int r;
		l = ( s > FILE_BUFFER_SIZE ? FILE_BUFFER_SIZE : s );
		r = read( infd, file_buffer, l );
		if( r != l ){
			printf( "read err\n" );
			return r;
		}
		r = write( outfd, file_buffer, l );
		if( r != l ){
			printf( "write_err\n" );
			return r;
		}
		s -= l;
	}
	close( infd );
	close( outfd );

	{
		int t;

		t = time_end();
		if( t > 0 ){
			printf( "size = %d time = %dmsec rate %dKbyte/sec\n"
					, size, t, ( size / 1024 * 100 ) / t );
		}
	}

	return 0;
}

static char get_help[] =
	"get <リモートファイル> [ローカルファイル]\n"
	"  HDユニット上のファイルを取得\n";

static int get_cmd( int argc, char *argv[] )
{
	char from[ 256 ];
	char to[ 256 ];

	if( argc < 2 ){
		return 1;
	}
	get_name( from, "pfs0:", argv[ 1 ] );
	if( argc == 3 ){
		get_name( to, "host1:", argv[ 2 ] );
	} else {
		get_name( to, "host1:", get_file_base( from ) );
	}

	printf( "get %s %s\n", from, to );

	return copy_file( to, from );
}

static char put_help[] =
	"put <ローカルファイル> [リモートファイル]\n"
	"  開発機上のファイルを取得\n";

static int put_cmd( int argc, char *argv[] )
{
	char from[ 256 ];
	char to[ 256 ];

	if( argc < 2 ){
		return 1;
	}
	get_name( from, "host1:", argv[ 1 ] );
	if( argc == 3 ){
		get_name( to, "pfs0:", argv[ 2 ] );
	} else {
		get_name( to, "pfs0:", get_file_base( from ) );
	}

	printf( "put %s %s\n", from, to );

	return copy_file( to, from );
}

static char copy_help[] =
	"copy <リモートファイル> <リモートファイル>\n"
	"  開発機上のファイルを転送\n";

static int copy_cmd( int argc, char *argv[] )
{
	char from[ 256 ];
	char to[ 256 ];

	if( argc != 3 ){
		return 1;
	}
	get_name( from, "pfs0:", argv[ 1 ] );
	get_name( to, "pfs0:", argv[ 2 ] );

	printf( "copy %s %s\n", from, to );

	return copy_file( to, from );
}

static char rename_help[] =
	"rename <リモートファイル> <リモートファイル>\n"
	"  開発機上のファイルの名前の変更\n";

static int rename_cmd( int argc, char *argv[] )
{
	char from[ 256 ];
	char to[ 256 ];

	if( argc != 3 ){
		return 1;
	}
	get_name( from, "pfs0:", argv[ 1 ] );
	get_name( to, "pfs0:", argv[ 2 ] );

	printf( "rename %s %s\n", from, to );

	return rename( from, to );
}

static char info_help[] =
	"info <ファイル>\n"
	"  ファイル情報の表示\n";

static int info_cmd( int argc, char *argv[] )
{
	struct sce_stat stat;
	char buf[ 128 ];
	int r;

	if( argc < 2 ){
		return 1;
	}
	get_name( buf, "pfs0:", argv[ 1 ] );
	r = getstat( buf, &stat );
	if( r < 0 ){
		return r;
	}
	printf( "%s %12d %c %s\n"
		, get_mode_str( stat.st_mode ), stat.st_size
		, ( stat.st_attr & 1 ) ? 'd' : '-'
		, buf );
	printf( "CREATE %s\n", get_time_str( stat.st_ctime ) );
	printf( "ACCESS %s\n", get_time_str( stat.st_atime ) );
	printf( "MODIFY %s\n", get_time_str( stat.st_mtime ) );

	{
		int fd;
		char key[ 128 ], value[ 128 ];
		int attr[ 2 ] = { ( int )key, ( int )attr };
		
		fd = open( buf, O_RDONLY, 0 );
		if( fd < 0 ){
			return fd;
		}

		while( ( r = ioctl2( fd, PIOCATTRREAD, NULL, 0, &attr , 0) ) > 0 ){
			printf( "%s/%s\n", key, value );
		}
		if( r < 0 ){
			return r;
		}
		close( fd );
	}

	return 0;
}

static char sync_help[] =
	"sync\n"
	"  sync発行\n";

static int sync_cmd( int argc, char *argv[] )
{
	return devctl( "hdd0:", HDIOC_FLUSH, NULL, 0, NULL, 0 );
}

static char chmod_help[] =
	"chmod <ファイル名> <アクセスフラグ>\n"
	"  ファイルのステータスを変更\n";

static int chmod_cmd( int argc, char *argv[] )
{
	char buf[ 128 ];
	int mode;
	struct sce_stat stat;

	if( argc < 3 ){
		return 1;
	}
	get_name( buf, "pfs0:", argv[ 1 ] );
	mode = strtol( argv[ 2 ], NULL, 8 );

	stat.st_mode = ( stat.st_mode & ~SCE_STM_RWXUGO ) | mode;
	return chstat( buf, &stat, SCE_CST_MODE );
}

/* ---------------------------------------------------------------------- */
/*
	bench
*/

static char bench_help[] =
	"bench\n"
	"  ベンチマーク実行\n";

// 64Mのファイルを読んで書いて時間をはかる

#define BENCH_SIZE	(64*1024*1024)

static void show_time( char *mes, int size )
{
	int t;

	t = time_end();
	if( t > 0 ){
		printf( "%s : size = %d time = %dmsec rate %dKbyte/sec\n"
				, mes, size, t, ( size / 1024 * 100 ) / t );
	} else {
		printf( "%s time = 0\n", mes );
	}
}

static int bench_cmd( int argc, char *argv[] )
{
	static char file[] = "pfs0:bench.dat";
	int fd;
	int size;

	memset( file_buffer, 0, FILE_BUFFER_SIZE );

	printf( "normal write check\n" );

	// ファイルを通常作成
	fd = open( file, O_WRONLY | O_CREAT | O_TRUNC, 0777 );
	if( fd < 0 ){
		return fd;
	}
	size = BENCH_SIZE;
	time_set();
	while( size > 0 ){
		int l;
		int r;
		l = ( size > FILE_BUFFER_SIZE ? FILE_BUFFER_SIZE : size );
		r = write( fd, file_buffer, l );
		if( r != l ) return r;
		size -= l;
	}
	show_time( "Normal Write", BENCH_SIZE );
	close( fd );

	remove( file );

	// ファイルを領域確保して作成

	printf( "alloc write check\n" );
	fd = open( file, O_WRONLY | O_CREAT | O_TRUNC, 0777 );
	if( fd < 0 ){
		return fd;
	}
	size = BENCH_SIZE;
	time_set();
	ioctl2( fd, PIOCALLOC, &size, sizeof( int ), NULL, 0 );
	show_time( "ALLOC TIME", BENCH_SIZE );
	
	time_set();
	while( size > 0 ){
		int l;
		int r;
		l = ( size > FILE_BUFFER_SIZE ? FILE_BUFFER_SIZE : size );
		r = write( fd, file_buffer, l );
		if( r != l ) return r;
		size -= l;
	}
	show_time( "ALLOC Write", BENCH_SIZE );
	close( fd );

	// リード
	printf( "read check\n" );
	fd = open( file, O_RDONLY, 0777 );
	if( fd < 0 ){
		return fd;
	}
	size = BENCH_SIZE;
	time_set();
	while( size > 0 ){
		int l;
		int r;
		l = ( size > FILE_BUFFER_SIZE ? FILE_BUFFER_SIZE : size );
		r = read( fd, file_buffer, l );
		if( r != l ) return r;
		size -= l;
	}
	show_time( "Normal Read", BENCH_SIZE );
	close( fd );

	remove( file );

	return 0;
}

static int test_cmd( int argc, char *argv[] )
{
	struct SysClock time;
	unsigned long long t;
	int s;

	GetSystemTime( &time );

	t = ( ( unsigned long long )time.hi << 32 ) | ( time.low );

	s = t / ( 36894000 );

	printf( "TIME = %d ( %d )\n", s, time.hi );

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
	{ "allformat", allformat_cmd, allformat_help },
	{ "pdir", pdir_cmd, pdir_help },
	{ "pinfo", pinfo_cmd, pinfo_help },
	{ "create", create_cmd, create_help },
	{ "addsub", addsub_cmd, addsub_help },
	{ "remove", remove_cmd, remove_help },
	{ "format", format_cmd, format_help },
	{ "mount", mount_cmd, mount_help },
	{ "umount", umount_cmd, umount_help },
	{ "dir", dir_cmd, dir_help },
	{ "ls", dir_cmd, dir_help },
	{ "cd", cd_cmd, cd_help },
	{ "get", get_cmd, get_help },
	{ "put", put_cmd, put_help },
	{ "mkdir", mkdir_cmd, mkdir_help },
	{ "rmdir", rmdir_cmd, rmdir_help },
	{ "copy", copy_cmd, copy_help },
	{ "rename", rename_cmd, rename_help },
	{ "delete", delete_cmd, delete_help },
	{ "info", info_cmd, info_help },
	{ "bench", bench_cmd, bench_help },
	{ "sync", sync_cmd, sync_help },
	{ "chmod", chmod_cmd, chmod_help },
	{ "help", help_cmd, "help\n" },
	{ "test", test_cmd, "test\n" },
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
	printf( "\n" );

	for( ;; ){
		printf( "ok.\n" );
		get_line( line_buffer, 256 );
		do_command( line_buffer );
	}
}

/* ---------------------------------------------------------------------- */
/*
	Boot
*/

int start(int argc, char **argv)
{
    struct ThreadParam th;
    char hddarg[] = "-o" "\0" "4";
    char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "24";

//    LoadStartModule("host1:./modules/dev9.irx", 0, NULL, NULL);
//    LoadStartModule("host1:./modules/atad.irx", 0, NULL, NULL);
//    LoadStartModule("host1:./modules/hdd.irx", 5, hddarg, NULL);
//    LoadStartModule("host1:./modules/pfs.irx", 18, pfsarg, NULL);
    LoadStartModule("host1:/usr/local/sce/iop/modules/dev9.irx", 0, NULL, NULL);
    LoadStartModule("host1:/usr/local/sce/iop/modules/atad.irx", 0, NULL, NULL);
    LoadStartModule("host1:/usr/local/sce/iop/modules/hdd.irx", 5, hddarg, NULL);
    LoadStartModule("host1:/usr/local/sce/iop/modules/pfs.irx", 18, pfsarg, NULL);

    th.attr = TH_C;
    th.entry = mainthread;
    th.initPriority = 30;
    th.stackSize = 0x800 << 2;
    StartThread(CreateThread(&th), 0);
    while(1) DelayThread(1000*1000);

    return 0;
}
