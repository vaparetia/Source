/*
	generate.c
	$Id: generate.c,v 1.2 2001/05/01 08:44:04 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

extern int verbose_mode;

#define PRINTF( fmt, arg... )	if( verbose_mode ) printf( fmt, ## arg )

/* ---------------------------------------------------------------------- */
/*
	MPEG2 ストリーム解析
*/

#define MPEG2_PACK_CODE		0x000001BA
#define MPEG2_SYSTEM_CODE	0x000001BB
#define MPEG2_END_CODE		0x000001B9

/* ---------------------------------------------------------------------- */
/*
	ビット単位入力
*/

typedef struct {
	int bit;
	FILE *fp;
	unsigned char buf;
} BitFile;

static void init_BitFile( BitFile *bf, FILE *fp )
{
	bf->fp = fp;
	bf->bit = 0;
}

static void seek_BitFile( BitFile *bf, int offset, int mode )
{
	fseek( bf->fp, offset, mode );
	bf->bit = 0;
}

static int ftell_BitFile( BitFile *bf )
{
	return ftell( bf->fp );
}

static int get_bit( BitFile *bf, int len )
{
	int res = 0;

	for( ;; ){
		if( bf->bit > 0 ){
			int l, mask;
			l = ( len < bf->bit ) ? len : bf->bit;	// min
			mask = ( bf->buf & ( ( ( 1 << l ) - 1 ) ) << ( bf->bit - l ) );
			res = ( res << l );
			res = res | ( ( bf->buf & mask ) >> ( bf->bit - l ) );
			bf->bit -= l;
			len -= l;
		}
		if( len <= 0 ) break;
		if( bf->bit == 0 ){
			bf->buf = fgetc( bf->fp );
			bf->bit = 8;
		}
	}
	return res;
}

/* ---------------------------------------------------------------------- */
/*
	各ブロックデータの解析。
	関数終了時にはファイルポインタはそのブロックの最後。
	先頭のCODEは読み込まれている。
*/

#define MARKER( bf, a, v ) \
	if( get_bit( bf, (a) ) != (v) ) printf( "LINE %d:Error\n", __LINE__ )

static void analyze_pack_header( BitFile *bf, int code )
{
	int flag, scr1, scr2, rate, staf;
	/*
		scr は42Bit
	*/

	flag = get_bit( bf, 2 );

	scr1 = get_bit( bf, 3 );
	MARKER( bf, 1, 1 );
	scr1 = ( scr1 << 7 ) | get_bit( bf, 7 );	// 10 Bit
	scr2 = get_bit( bf, 8 );
	MARKER( bf, 1, 1 );
	scr2 = ( scr2 << 15 ) | get_bit( bf, 15 );
	MARKER( bf, 1, 1 );
	scr2 = ( scr2 << 9 ) | get_bit( bf, 9 );
	MARKER( bf, 1, 1 );

	rate = get_bit( bf, 22 );
	MARKER( bf, 2, 3 );

	staf = get_bit( bf, 8 );

	PRINTF( "PACK %08X %X %08X_%08X %X %X\n", code, flag, scr1, scr2, rate, staf );

	seek_BitFile( bf, ( staf & 0x7 ), SEEK_CUR );
}

static void analyze_system_header( BitFile *bf, int code )
{
	int hlen, rate, achl, flag, vchl, resv;
	int pos;

	hlen = get_bit( bf, 16 );

	pos = ftell_BitFile( bf );

	MARKER( bf, 1, 1 );
	rate = get_bit( bf, 22 );
	MARKER( bf, 1, 1 );

	achl = get_bit( bf, 6 );
	flag = get_bit( bf, 5 );
	vchl = get_bit( bf, 5 );
	resv = get_bit( bf, 8 );

	PRINTF( "SYS  %08X %X %X %X %X %X %X\n"
			, code, hlen, rate, achl, flag, vchl, resv );

	while( ftell_BitFile( bf ) < pos + hlen ){
		int id, f, scale, size;

		id = get_bit( bf, 8 );
		f = get_bit( bf, 2 );
		scale = get_bit( bf, 1 );
		size = get_bit( bf, 13 );
		PRINTF( "    %X %X %X %X\n", id, f, scale, size );
	}

	seek_BitFile( bf, pos + hlen, SEEK_SET );
}

static int analyze_pes_header( BitFile *bf, int code )
{
	int id, plen, mpeg, flag, pesh, pts1, pts2, dts1, dts2;
	int pos;

	id = ( code & 0xFF );
	code = ( code >> 8 );
	plen = get_bit( bf, 16 );
	pos = ftell_BitFile( bf );

	if( id == 0xBE ){
		PRINTF( "PADDING size %d\n", plen );
		pts1 = 1; pts2 = -2;
		dts1 = 1; dts2 = -2;
	} else if( id == 0xE0 ){
		mpeg = get_bit( bf, 2 );
		flag = get_bit( bf, 14 );
		pesh = get_bit( bf, 8 );

		if( ( flag & 0xC0 ) == 0x80 ){
			/* PTS ONLY */
			MARKER( bf, 4, 2 );
			pts1 = get_bit( bf, 1 );
			pts2 = get_bit( bf, 2 );
			MARKER( bf, 1, 1 );
			pts2 = ( pts2 << 15 ) | get_bit( bf, 15 );
			MARKER( bf, 1, 1 );
			pts2 = ( pts2 << 15 ) | get_bit( bf, 15 );
			MARKER( bf, 1, 1 );

			dts1 = 1; dts2 = -1;
		} else if( ( flag & 0xC0 ) == 0xC0 ){
			/* PTS & DTS */
			MARKER( bf, 4, 3 );
			pts1 = get_bit( bf, 1 );
			pts2 = get_bit( bf, 2 );
			MARKER( bf, 1, 1 );
			pts2 = ( pts2 << 15 ) | get_bit( bf, 15 );
			MARKER( bf, 1, 1 );
			pts2 = ( pts2 << 15 ) | get_bit( bf, 15 );
			MARKER( bf, 1, 1 );

			MARKER( bf, 4, 1 );
			dts1 = get_bit( bf, 1 );
			dts2 = get_bit( bf, 2 );
			MARKER( bf, 1, 1 );
			dts2 = ( dts2 << 15 ) | get_bit( bf, 15 );
			MARKER( bf, 1, 1 );
			dts2 = ( dts2 << 15 ) | get_bit( bf, 15 );
			MARKER( bf, 1, 1 );
		} else {
			pts1 = 1; pts2 = -1;
			dts1 = 1; dts2 = -1;
		}
#if 1
		if( pts2 > 0 ){
			if( dts2 > 0 ){
				PRINTF( "FRAME %f %f\n"
						, ( pts2 / 90000.0 ) * 30.0
						, ( dts2 / 90000.0 ) * 30.0 );
			} else {
				PRINTF( "FRAME %f\n", ( pts2 / 90000.0 ) * 30.0 );
			}
		}
		PRINTF( "PES %X %X %X %X %X %X %02X%08X %02X%08X\n"
				, code, id, plen, mpeg, flag, pesh, pts1, pts2, dts1, dts2 );
#endif
	} else {
		pts1 = 1; pts2 = -1;
		dts1 = 1; dts2 = -1;
		printf( "Wrong Packet %02X\n", id );
	}
	
	seek_BitFile( bf, pos + plen, SEEK_SET );

	if( pts2 > 0 ){
		if( dts2 > 0 ){
			return dts2;
		}
	}
	return pts2;
}


/* ---------------------------------------------------------------------- */
/*
	出力ルーチン
	tick は 90KHzで来る。
	optionにはPSS中のサイズを入れる。
*/

typedef struct {
	int type;
	int size;
	int time;
	int option;
} STREAM_HEADER;

#define NORM16( a )	( ( (a) + 15 ) & ~15 )

#define ONE_READ_SIZE	1024*512

static void output_chank( FILE *in, int start, int end, int tick, FILE *out )
{
	STREAM_HEADER head;
	int size, pad;
	
	static char buffer[ ONE_READ_SIZE ];

	size = end - start;

	head.type = 0;
	head.size = NORM16( size ) + sizeof( STREAM_HEADER );
	head.time = ( int )( tick / 90000.0 * 300.0 );
	head.option = size;

	pad = NORM16( size ) - size;

	fwrite( &head, sizeof( STREAM_HEADER ), 1, out );
	fseek( in, start, SEEK_SET );

	while( size > 0 ){
		int l;
		l = ( size > ONE_READ_SIZE ) ? ONE_READ_SIZE : size;

		fread( buffer, sizeof( char ), l, in );
		if( fwrite( buffer, sizeof( char ), l, out ) < l ){
			printf( "Write Error\n" );
			exit( 1 );
		}
		size -= l;
	}
	if( pad > 0 ){
		memset( buffer, 0, pad );
		fwrite( buffer, sizeof( char ), pad, out );
	}
}

/* ---------------------------------------------------------------------- */
/*
	メインルーチン
*/

void mkstream( FILE *fp, FILE *out )
{
	BitFile *bf;
	BitFile bitfile;
	int code;
	int pos, prev;

	bf = &bitfile;

	init_BitFile( bf, fp );

	pos = ftell_BitFile( bf );
	prev = -1;

	for( ;; ){
		int now;

		now = ftell_BitFile( bf );
		code = get_bit( bf, 32 );
		switch( code ){
		  case MPEG2_PACK_CODE:
			analyze_pack_header( bf, code );
			break;
		  case MPEG2_SYSTEM_CODE:
			analyze_system_header( bf, code );
			break;
		  case MPEG2_END_CODE:
			PRINTF( "END CODE %X\n", code );
			goto END;
			break;
		  default:
			{
				int tick, back;

				tick = analyze_pes_header( bf, code );
				back = ftell_BitFile( bf );
				if( prev < 0 ){
					prev = pos;
					continue;
				}
				if( tick >= 0 ){
					output_chank( fp, pos, now, prev, out );
					pos = now;
					prev = tick;
					seek_BitFile( bf, back, SEEK_SET );
				}
			}
			break;
		}
	}
END:
	{
		int now;
		now = ftell_BitFile( bf );
		output_chank( fp, pos, now, prev, out );
	}

	return;
}

