/*
	HDインストール用にデータを展開してもつ
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <unistd.h>
#include <fcntl.h>

/* ---------------------------------------------------------------------- */
/*
	ユーティリティ
*/

static int fd_in;

#define FS_SECTOR_SIZE	2048
#define FS_SECTOR_ALIGN( a )	( (((a)+FS_SECTOR_SIZE-1)/FS_SECTOR_SIZE)*FS_SECTOR_SIZE )

#ifdef WIN32
#include <io.h>

#define lseek _lseek
#define read _read
#define write _write
#define open _open
#define close _close

void * zcalloc(void *opaque, unsigned items, unsigned size)
{
   return malloc( items * size );
}
void zcfree(void * opaque, voidpf ptr)
{
   free( ptr );
}

int z_verbose = 1;
void z_error( char *m )
{
   printf( "%s\n", m );
   abort();
}

#endif

// CDのエミュレート
static void cdread( void *buffer, int sector, int size )
{
	int lsize;

	lseek( fd_in, sector * FS_SECTOR_SIZE, SEEK_SET );
	lsize = FS_SECTOR_ALIGN( size );
	read( fd_in, buffer, lsize );
}

// malloc

static void *Malloc( int size )
{
	void *ptr;
	ptr = malloc( size );
	if( ptr == NULL ){
		printf( "NO MEMORY\n" );
		exit( 1 );
	}
	return ptr;
}

static void Free( void *ptr )
{
	free( ptr );
}

// write

static int Write( int fd, void *buffer, int size )
{
	int s;
	s = write( fd, buffer, size );
	if( s < size ){
		printf( "Write Error\n" );
		exit( 1 );
	}
	return s;
}

// lseek

static int Lseek( int fd, int offset, int whence )
{
	return lseek( fd, offset, whence );
}

#define BIT_LEN		24

int get_strcode( char *str )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;

	p = ( unsigned char * )str;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	if( p[ 0 ] == 0xFF ){
		int code;
		memcpy( &code, &p[ 1 ], sizeof( int ) );
		return code;
	}

	while( ( c = *( p++ ) ) != '\0' ){
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	return ( int )id;
}

/* ---------------------------------------------------------------------- */
/*
	メイン関数
*/

static void *sector_work;	// セクタサイズ読み込み用のワーク

typedef struct {
	int seed;
	short version;
	short sector;
	short stagenum;
	short padding;
	int install_size;
} STAGE_TABLE_HEADER;

typedef struct {
	char name[ 8 ];
	int offset;
} STAGE_TABLE;

typedef struct {
	int id;
	int offset;
} DATACNF_TAG;

typedef struct {
	int tagnum;
	DATACNF_TAG tags[ 0 ];
} DATACNF;

#define ID_SECTION_END	0x7F000000
#define ID_NOCACHE		0x7F000001
#define ID_CACHE		0x7F000002
#define ID_RESIDENT		0x7F000003
#define ID_SOUND		0x7F000010
#define ID_BINARY		0x7F010000
#define ID_BLOCK( no )	( 0x7F000100 | (no) )

#define ID_COMPRESS		0x7E000000
#define ID_NOP			0x70000000

#define IS_SECTION_TAG( a )	( ( (a)&0xFF000000 ) == 0x7F000000 )
#define IS_SECTION_END( a )	( (a) == ID_SECTION_END )
#define IS_PRELOAD_TAG( a )	( ( (a) & ~0xFF ) != ID_BLOCK(0) )
#define IS_END_TAG( a )		( (a) == 0 )

#define IS_COMPRESSION_TAG( a )		( ( (a)&0xFF000000 ) == 0x7E000000 )
#define COMPRESSION_SIZE( a )		( (a)&0x00FFFFFF )

#define IS_COMMAND_TAG( a )	( ( (a)&0xF0000000 ) == 0x70000000 )


/* ---------------------------------------------------------------------- */
/*
	圧縮ステージを展開して出力する
*/

static int current_stage_id;

// 復号化ルーチン

static unsigned int header_seed;
static unsigned int seed;
static unsigned int _seed;

static void setup_decrypt( void *buffer )
{
	unsigned short *p = buffer;
	unsigned int s;

	s = *p ^ 0x9385;
	seed = ( s ) | ( ( s ^ 0x6576 ) << 16 );
	_seed = s * 278;
}

static void decrypt_buffer( void *buffer, int size )
{
	unsigned int *p = buffer;

	for( ; size > 0; size -= sizeof( int ) ){
		*p = *p ^ seed;
		seed = seed * 48828125 + _seed;
		p++;
	}
}

// 展開

#define READBUF_SIZE ( 1024 * 1024 )
#define OUTBUF_SIZE	( 1024 * 1024 )

static int output_compressed_section( int fd, int pos, int compress_size, int section_size )
{
	z_stream z;
	void *readbuf;
	void *outbuf;
	int first;
	int status;

	memset( &z, 0, sizeof( z_stream ) );

	readbuf = Malloc( READBUF_SIZE );
	outbuf = Malloc( OUTBUF_SIZE );

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	z.next_in = Z_NULL;
	z.avail_in = 0;

	if( ( status = inflateInit( &z ) ) != Z_OK ){
		printf( "%d %s\n", status, (z.msg) ? z.msg : "???" );
		exit( 1 );
	}

	z.next_out = outbuf;
	z.avail_out = OUTBUF_SIZE;

	first = 1;
	status = Z_OK;

	for( ;; ){
		if( z.avail_in == 0 ){
			int readsize;
			z.next_in = readbuf;
			readsize = ( compress_size > READBUF_SIZE ) ? READBUF_SIZE : compress_size;
			if( readsize > 0 ){
				cdread( readbuf, pos, FS_SECTOR_ALIGN( readsize ) );
				if( first ){
					setup_decrypt( readbuf );
					decrypt_buffer( readbuf, readsize );
					*( short * )readbuf += ( 0x9C78 - 0x9385 );
					first = 0;
				} else {
					decrypt_buffer( readbuf, readsize );
				}
				pos += FS_SECTOR_ALIGN( readsize ) / FS_SECTOR_SIZE;
				compress_size = compress_size - readsize;
			}
			z.avail_in = readsize;
		}
		status = inflate( &z, Z_NO_FLUSH );
		if( z.avail_out == 0 || status == Z_STREAM_END ){
			int write_size;
			write_size = OUTBUF_SIZE - z.avail_out;
			if( write_size > 0 ){
				Write( fd, outbuf, write_size );
			}
			z.next_out = outbuf;
			z.avail_out = OUTBUF_SIZE;
			if( status == Z_STREAM_END ){
				break;
			}
		}
		if( status != Z_OK ){
			/* error */
			printf( "Error %d %s\n", status, (z.msg) ? z.msg : "???" );
			exit( 1 );
		}
	}
	if( inflateEnd( &z ) != Z_OK ){
		printf( "inflateEnd: %s\n", (z.msg) ? z.msg : "???" );
		exit( 1 );
	}

	/* セクタアライメントにそろえる */

	{
		int align;
		align = FS_SECTOR_ALIGN( section_size ) - section_size;
		if( align > 0 ){
			decrypt_buffer( readbuf, align );
			Write( fd, readbuf, align );
		}
	}

	Free( readbuf );
	Free( outbuf );

	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	ステージ毎の変換関数
*/

static void convert_stage( int fd, int pos )
{
	DATACNF *cnf;
	DATACNF_TAG *tag;
	int tagnum;
	int secs;
	int top;

	/* datacnfの読み込み */
	cdread( sector_work, pos, FS_SECTOR_SIZE );
	seed = 0xA78925D9 + ( current_stage_id << 7 ) + current_stage_id + header_seed;
	tagnum = ( ( DATACNF * )sector_work )->tagnum ^ seed;
printf( "TAGNUM = %d\n", tagnum );
	secs = sizeof( DATACNF ) + sizeof( DATACNF_TAG ) * tagnum;
	secs = FS_SECTOR_ALIGN( secs );
	cnf = Malloc( secs );
	memcpy( cnf, sector_work, FS_SECTOR_SIZE );
	if( secs > FS_SECTOR_SIZE ){
		cdread( ( unsigned char * )cnf + FS_SECTOR_SIZE, pos + 1, secs - FS_SECTOR_SIZE );
	}

	seed = 0xA78925D9 + ( current_stage_id << 7 ) + current_stage_id + header_seed;
	_seed = 0x7A88FB59 + ( current_stage_id << 7 ) + current_stage_id;

	decrypt_buffer( cnf, sizeof( DATACNF ) + sizeof( DATACNF_TAG ) * tagnum );

	pos += secs / FS_SECTOR_SIZE;

	/* ダミーでdatacnfを書き出す */
	top = Lseek( fd, 0, SEEK_CUR );
	Write( fd, cnf, secs );

	for( tag = cnf->tags; ! IS_END_TAG( tag->id ); ){
		int section_size;
		int compress_size;
		
		if( ! IS_SECTION_TAG( tag->id ) || tag->id == ID_SECTION_END ){
			printf( "Wrong Tag %08X %08X!!\n", tag->id, tag->offset );
			exit( 1 );
		}
		section_size = tag->offset;
printf( "section id = %08X size = %08X\n", tag->id, section_size );
		if( tag->id == ID_BINARY ){
			// そのままコピー
			void *buffer;

			tag ++;
			if( !IS_COMPRESSION_TAG( tag->id ) ){
				printf( "Wrong Binary Tag %08X\n", tag->id );
				exit( 1 );
			}
			compress_size = FS_SECTOR_ALIGN( COMPRESSION_SIZE( tag->id ) );
printf( "OUTPUT BINARY %X %X\n", tag->id, compress_size );
			buffer = Malloc( compress_size );
			cdread( buffer, pos, compress_size );
			Write( fd, buffer, compress_size );
			Free( buffer );
			tag ++;

			pos += compress_size / FS_SECTOR_SIZE;
		} else {
			// 展開してコピー

			tag++;
			if( ! IS_COMPRESSION_TAG( tag->id ) ){
				void *buffer;
				int size;
				// 圧縮されていない
				// サウンドセクション

printf( "NO COMPRESS SECTION %X\n", section_size );
				size = FS_SECTOR_ALIGN( section_size );
				buffer = Malloc( size );
				cdread( buffer, pos, size );
				Write( fd, buffer, size );
				Free( buffer );
			} else {
				int compress_size;
				compress_size = COMPRESSION_SIZE( tag->id );

printf( "COMPRESS SECTION %08X %08X\n", tag->id, compress_size );
				output_compressed_section( fd, pos, compress_size, section_size );
				pos += FS_SECTOR_ALIGN( compress_size ) / FS_SECTOR_SIZE;

				// 圧縮タグをNOPにする
				tag->id = ID_NOP;
				tag ++;
			}
		}
		while( tag->id == ID_SECTION_END
			   || ( ! IS_COMMAND_TAG( tag->id ) && ! IS_END_TAG( tag->id ) ) ){
			tag++;
		}
	}

	{
		/* datacnfの再書き込み */
		int now;
		now = Lseek( fd, 0, SEEK_CUR );
		Lseek( fd, top, SEEK_SET );

		seed = 0xA78925D9 + ( current_stage_id << 7 ) + current_stage_id + header_seed;
		_seed = 0x7A88FB59 + ( current_stage_id << 7 ) + current_stage_id;
		decrypt_buffer( cnf, sizeof( DATACNF ) + sizeof( DATACNF_TAG ) * tagnum );

		Write( fd, cnf, secs );
		Lseek( fd, now, SEEK_SET );
	}
}


/* ---------------------------------------------------------------------- */
/*
	全体の変換関数
*/

static void convert( int fd, int pos )
{
	void *header_buffer;
	int secs;
	STAGE_TABLE_HEADER *header;
	STAGE_TABLE_HEADER h;
	STAGE_TABLE *stage_table;
	int i;
	int now;

	/* ヘッダ・ステージテーブルのロード */
	sector_work = Malloc( FS_SECTOR_SIZE );
	// 最初の1セクタを読み込む
	cdread( sector_work, pos, FS_SECTOR_SIZE );

	memcpy( &h, sector_work, sizeof( STAGE_TABLE_HEADER ) );
	seed = h.seed;
	header_seed = seed;
	_seed = h.seed ^ 0xF0F0;
	decrypt_buffer( ( unsigned char * )&h + 4, sizeof( STAGE_TABLE_HEADER ) - 4 );

	secs = h.sector * FS_SECTOR_SIZE;

printf( "HEADER SIZE = %d\n", secs );
	header_buffer = Malloc( secs );
	memcpy( header_buffer, sector_work, FS_SECTOR_SIZE );
	if( secs > FS_SECTOR_SIZE ){
		cdread( (unsigned char *) header_buffer + FS_SECTOR_SIZE, pos + 1, secs - FS_SECTOR_SIZE );
	}

	seed = h.seed;
	_seed = h.seed ^ 0xF0F0;
	decrypt_buffer( ( unsigned char * )header_buffer + 4
					, sizeof( STAGE_TABLE_HEADER ) - 4
					+ sizeof( STAGE_TABLE ) * h.stagenum );

	header = ( STAGE_TABLE_HEADER * )header_buffer;
	stage_table = ( STAGE_TABLE * )( header + 1 );

	/* ダミーデータを出力 */
	Write( fd, header_buffer, secs );

	for( i = 0; i < header->stagenum; i++ ){
		now = Lseek( fd, 0, SEEK_CUR );
printf( "NOW = %08X\n", now / 2048 );
printf( "CONVERT STAGE %s %X\n", stage_table[ i ].name, stage_table[ i ].offset );
		current_stage_id = get_strcode( stage_table[ i ].name );
		convert_stage( fd, stage_table[ i ].offset + pos );
		stage_table[ i ].offset = now / FS_SECTOR_SIZE;
	}
	now = Lseek( fd, 0, SEEK_CUR );
printf( "NOW = %08X\n", now / 2048 );

	/* ヘッダデータを再出力 */
	Lseek( fd, 0, SEEK_SET );
	seed = h.seed;
	_seed = h.seed ^ 0xF0F0;
	decrypt_buffer( ( unsigned char * )header_buffer + 4
					, sizeof( STAGE_TABLE_HEADER ) - 4
					+ sizeof( STAGE_TABLE ) * h.stagenum );
	Write( fd, header_buffer, secs );

	Free( header_buffer );
	Free( sector_work );

	Lseek( fd, 0, SEEK_END );
	now = Lseek( fd, 0, SEEK_CUR );
printf( "END = %08X\n", now / 2048 );
}

/* ---------------------------------------------------------------------- */
/*
	linux用ラッパ
*/

void convert_hd( char *infile, char *outfile )
{
	int fd_out;

	fd_in = open( infile, O_RDONLY );
	if( fd_in < 0 ){
		printf( "FILE %s not found\n", infile );
		return;
	}
	fd_out = open( outfile, O_WRONLY | O_CREAT | O_TRUNC );
	if( fd_out < 0 ){
		printf( "FILE %s not found\n", outfile );
		return;
	}

	convert( fd_out, 0 );

	close( fd_in );
	close( fd_out );
}
