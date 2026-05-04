/*
	font.c
		フォント描画ルーチン
	1999/12/20	K.Uehara
	$Id: font.c,v 1.2 2002/04/05 07:29:44 usr01475 Exp $
*/

/*
	FONT.Hとともに外部ツールに埋め込めるようにする。
	外部ツールから呼び出す場合はCHECK_ONLYマクロをコンパイル時に定義すると,
	描画処理は行なわない。
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define FALSE	0
#define TRUE	1

#ifdef PSX2
#include "libgv.h"
#else
#define ASSERT( a )
#endif

#include "font.h"

/* 外部から設定されるリファレンスデータ */
static int font_ref_num = 0;

/* フォントデータへのポインタ */

int font_han_max_offset = 0;

static unsigned int *hantable = NULL;
static unsigned char *handata = NULL;
static unsigned char *zendata0;	
static unsigned char *zendata[ MAX_FONT_TYPE ] = { NULL };
static unsigned char *rubi_data = NULL;

static int font_line_len = FONT_LINE_LEN;

static inline int getcode( int code )
{
	return ( code & ~( TOP_KINSOKU_MASK | BACK_KINSOKU_MASK ) );
}

static inline char *getnextchar( int *code, unsigned char *m )
{
	int a;

	if( *m == 0x1f ){
		a = 0x8000 | ( int )( *( m + 1 ) + 0x7F );
		m += 2;
	} else if( *m < 0x80 ){
		a = 0x8000 | ( int )*m;
		m ++;
	} else {
		a = ( ( int )*m << 8 ) | ( int )*( m + 1 );
		m += 2;
	}
	*code = a;
	return m;
}

/* --------------------------------------------------- */
/*
	フォントデータ関連
		フォントのコーディングに依存する
*/

void font_resident_rubi_set( char *rubitop )
{
	rubi_data = rubitop;
}

void font_set_top_addr( int type, void *addr )
{
	zendata[ type ] = addr;
	if( type == FONT_TYPE_GCL ){
		zendata[ FONT_TYPE_GCL2 ] = addr + FONT_ONE_SIZE * ( 1024 - 4 );
	}
}

static unsigned char *get_zen_font_data( int code )
{
	int pos;
	char *top;

	code = getcode( code );

	top = zendata0;

	switch( code >> 8 ){
	  case 0x81:	// 平仮名
		pos = code - 0x8101 + FONT_HIRA_TOP;
		break;
	  case 0x82:	// カタカナ
		pos = code - 0x8201 + FONT_KATA_TOP;
		break;
	  case 0x83:	// 記号
		pos = code - 0x8301 + FONT_KIGOU_TOP;
		break;
	  default:		// 漢字 : コードによってフォントのメモリ位置が違う
		{
			int type;
			type = FONT_TYPE( code );
			pos = code - FONT_CODE_TOP( type );
			pos = pos - ( pos / 256 );
			top = zendata[ type ];
		}
	}
	return top + pos * FONT_ONE_SIZE;
}

void font_resident_load_set( char *fonttop )
{
	// 常駐フォントの初期化
	unsigned int *table;

	table = ( int * )fonttop;
	hantable = ( unsigned int * )( fonttop + sizeof( int ) * 2 );
	handata = ( unsigned char * )( fonttop + ( table[ 0 ] & 0x03FFFFFF ) );
	zendata0 = ( unsigned char * )( fonttop + table[ 1 ] );
	zendata[ FONT_TYPE_RESIDENT ] = zendata0 + FONT_ZEN_TOP + FONT_ONE_SIZE;
	font_han_max_offset = ( table[ 0 ] >> 26 );

	memset( get_zen_font_data( SP_SPACE ), 0, FONT_ONE_SIZE );
}

static unsigned int get_hantable( int code )
{
	if( code > 0 ){
#if 0
		if( code > 0x80 ){
			code -= ( 0xa1 - 0x7f );
		}
#endif
		code -= 0x20;
	}
	return hantable[ code ];
}

static inline unsigned char *get_han_top( int hancode )
{
	return handata + ( hancode & 0x000FFFFF );
}

static inline int get_han_width( int hancode )
{
	return ( hancode >> 20 ) & 0x3F;
}

static inline int get_han_offset( int hancode )
{
	return ( hancode >> 26 ) & 0x3F;
}

static inline int get_font_width( int code )
{
	if( code < 0x8100 ){
		/* 半角 */
		int han;
		han = get_hantable( code & 0xFF );
		return get_han_width( han );
	}
	return FONT_SIZE_H;
}

/* --------------------------------------------------- */
/*
	描画関数の実体
*/

#define CONV2TO4HI( c ) ( unsigned char )((((int)(c)&0xc0)>>6)|(((int)(c)&0x30)))
#define CONV2TO4LO( c ) ( unsigned char )((((int)(c)&0x0c)>>2)|(((int)(c)&0x03)<<4))

static void put_Tdot( unsigned char *vtop, int x, int y, int row, int color )
{
	unsigned char data;
	int i, j;

	data = 0x33 | color;

	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 4; j += 2 ){
			*( vtop + ( x + j ) / 2 + ( y + i ) * row ) |= data;
		}
	}
}

static int put_zenkaku_4bpp( FONT_VRAMINFO *out, int x, int y, int code, int flag )
{
#ifndef CHECK_ONLY
	int i;
	int row;
	unsigned char *vram;
	unsigned char color;
	unsigned char *p;

	vram = out->vramtop;
	row = out->row;

	color = ( unsigned char )( ( out->color << 6 ) | ( out->color << 2 ) );
	if( flag != FALSE ){
		int ytop;

//		ytop = ( out->rubi_display_flag ) ? ( y - 6 ) : ( y - 2 );
		ytop = out->l_skip / 2 + 2;
		if( ytop > 4 ){
			put_Tdot( vram, x + 12 - 2, y - ytop, row, color );
		}
	}

	vram = vram + ( x / 2 ) + y * row;

	p = get_zen_font_data( code );

	if( x % 2 == 0 ){
		for( i = FONT_SIZE_H; i > 0; i-- ){
			int j;
			for( j = FONT_SIZE_W / 4; j > 0; j-- ){
				*( vram ++ ) = CONV2TO4HI( *p ) | color;
				*( vram ++ ) = CONV2TO4LO( *p ) | color;
				p++;
			}
			vram += row - FONT_SIZE_W / 2;
		}
	} else {
		int j, cn, cp;
		unsigned char d;
		unsigned char *v;

		color = ( unsigned char )( out->color << 2 );
		cn = 4;
		d = *p;
		for( i = 0; i < FONT_SIZE_H; i++ ){
			cp = ( 1 - x % 2 ) * 4;
			v = vram;
			for( j = 0; j < FONT_SIZE_W; j++ ){
				unsigned char data;
				data = color  | ( unsigned char )( d >> 6 );
				if( --cn == 0 ){
					d = *( ++ p );
					cn = 4;
				} else {
					d = d << 2;
				}
				*v |= data << ( 4 - cp );
				if( cp == 0 ){
					v ++;
					cp = 4;
				} else {
					cp = 0;
				}
			}
			vram += row;
		}
	}
#endif
	return FONT_SIZE_W;
}

static int put_hankaku_4bpp( FONT_VRAMINFO *out, int x, int y, int code, int flag )
{
	unsigned char *vtop, *vram, *v;
	unsigned char *p, d;
	unsigned char color;
	unsigned int han;
	int i, j, width, height, row;
	int cn, cp;

	row = out->row;
	vtop = ( unsigned char * )out->vramtop;
	vram = vtop + ( x / 2 ) + y * row;

	code = code & 0xFF;
	if( (code > 0) && (code < 0xff) ){
		han = get_hantable( code );
		p = get_han_top( han );
		width = get_han_width( han ) + 1;
		height = FONT_SIZE_H;
#ifndef CHECK_ONLY
		color = ( unsigned char )( ( out->color << 2 ) | ( out->color << 6 ) );
		if( flag != FALSE ){
			int w;
			int ytop;
			w = ( width + 1 ) / 2 - 2;
//			ytop = ( out->rubi_display_flag ) ? ( y - 8 ) : ( y - 4 );

			ytop = out->l_skip / 2 + 2;
			if( ytop > 4 ){
				put_Tdot( vtop, x + w, y - ytop, row, color );
			}
		}

		vram += row * get_han_offset( han );
		v = vram;
	
		cn = 4;
		d = *( p ++ );
		// color = ( unsigned char )out->color << 2;

		for( i = 0; i < height; i++ ){
			cp = ( 1 - x % 2 ) * 4;
			for( j = 0; j < width; j++ ){
				unsigned char data;
				data = ( unsigned char )( d >> 6 );
				if( --cn == 0 ){
					d = *( p ++ );
					cn = 4;
				} else {
					d = d << 2;
				}
				/* 前の色が残ると困るので、
				   描画ピクセルに該当する部分を
				   一旦 0 にする。 2001/08/22 Y.Kira */
				*v &= 0xf0 >> ( 4 - cp );
				*v |= ( color | data ) << ( 4 - cp );
				if( cp == 0 ){
					v ++;
					cp = 4;
				} else {
					cp = 0;
				}
			}
			v = ( vram += row );
		}
#endif
	} else {
		/* ---- */
		if( code == 0 ){
			int xx, w;
			unsigned char cc;

#ifndef CHECK_ONLY
			cc = ( unsigned char )( 0x33 | ( out->color << 6 ) | ( out->color << 2 ) );
			xx = x % 2;
			for( i = 0; i < 2; i++ ){
				w = font_line_len;
				v = vram + row * ( ( FONT_SIZE_H / 2 ) + i - 2 );
				if( xx > 0 ){
					*v |= ( cc >> ( xx * 4 ) );
					v ++;
					w -= 2 - xx;
				}
				for( ; w > 1; w -= 2 ){
					*v |= cc;
					v ++;
				}
				if( w > 0 ){
					xx = ( 2 - w ) * 4;
					*v |= ( cc << ( 4 - xx ) );
					v ++;
				}
			}
#endif
			width = font_line_len;
		} else {
			return 0;
		}
	}
	return width;
}

/* ---------------------------------------------------------------------- */
/*
	ルビ処理
*/

static void set_rubi_left_pos( FONT_DRAWINFO *dr, int xmax, int x, int y )
{
	dr->rubi_left_x = x;
	dr->rubi_left_y = y;
	dr->rubi_orikaeshi_xmax = xmax;
}

static void set_rubi_orikaeshi( FONT_DRAWINFO *dr, int xmax )
{
	dr->rubi_orikaeshi_xmax = xmax;
}

static inline int rubi_get_bit( char *datatop, int pos )
{
	return ( datatop[ pos / 4 ] & ( 3 << ( ( 3 - ( pos % 4 ) ) * 2 ) ) ) ? 1 : 0;
}

static inline int rubi_get_pattern( char *datatop, int width, int x, int y )
{
	/* x, yに対応するデータを返す */
	/* xはオーバーランする可能性がある */

	static unsigned char table[ 5 ] = { 0, 1, 2, 3, 3 };
	int count;
	int pos;

	pos = x + y * width;

	count =	rubi_get_bit( datatop, pos ) + rubi_get_bit( datatop, pos + width );
	if( x < width ){
		count += rubi_get_bit( datatop, pos + 1 ) + rubi_get_bit( datatop, pos + width + 1 );
	}
	return table[ count ];
}

static int put_rubi( FONT_VRAMINFO *out, int x, int y, int code )
{
	/*
		24*24フォントを12*12フォントに縮小して表示する
	*/
	unsigned char *vram, *v;
	unsigned char *p;
	unsigned char color;
	int i, j, width, height, row;
	int cn, cp;

	row = out->row;
	vram = ( unsigned char * )out->vramtop + ( x / 2 ) + y * row;

	if( ( code & 0xFF00 ) == 0x8000 ){
		/* 半角フォント */
		unsigned int han;

		code = code & 0xFF;

		han = get_hantable( code );
		p = get_han_top( han );
		width = get_han_width( han ) + 1;
		height = FONT_SIZE_H;

		vram += row * get_han_offset( han );
	} else {
		/* 全角フォント */
		p = get_zen_font_data( code );
		width = FONT_SIZE_W;
		height = FONT_SIZE_H;
	}
#ifndef CHECK_ONLY
	v = vram;
	
	cn = 4;
	color = ( unsigned char )out->color << 2;

	for( i = 0; i < height; i += 2 ){
		cp = ( 1 - x % 2 ) * 4;
		for( j = 0; j < width; j += 2 ){
			unsigned char data;
			data = rubi_get_pattern( p, width, j, i );
			*v |= ( color | data ) << ( 4 - cp );
			if( cp == 0 ){
				v ++;
				cp = 4;
			} else {
				cp = 0;
			}
		}
		v = ( vram += row );
	}
#endif

	return ( width + 1 ) / 2;
}

static int get_rubi_width( int *count, unsigned char *string )
{
	unsigned char *m;
	int n, w;

	w = 0;
	n = 0;
	m = string;

	for( ;; ){
		int code;
		m = getnextchar( &code, m );
		code = getcode( code );

		if( code == CODE( '}' ) ){
			break;
		}

		w += ( get_font_width( code ) + 1 ) / 2;
		n++;
	}
	*count = n;
	return w;
}

static int draw_rubi_string( FONT_DRAWINFO *out, int x, int y, unsigned char *string )
{
	int width;
	int count;
	int basewidth, basey;
	int dx, top;
	unsigned char *m;

	width = get_rubi_width( &count, string );
	/* 縦表示座標と基本幅の算出 */
	if( y > out->rubi_left_y ){
		if( out->rubi_orikaeshi_xmax - out->rubi_left_x <= x / 2 ){
			basey = y;
			basewidth = x;
			out->rubi_left_x = 0;
		} else {
			basewidth = out->rubi_orikaeshi_xmax - out->rubi_left_x;
			basey = out->rubi_left_y;
		}
	} else {
		basewidth = x - out->rubi_left_x;
		basey = y;
	}
	/* 文字間隔の算出 */
	dx = 1;
	count = count - 1;
	if( count > 0 && width - basewidth < 0 ){
		dx = ( basewidth - width ) / count;
		if( dx > 4 ) dx = 4;
		if( dx < 1 ) dx = 1;
	}
	width = width + dx * count;

	/* 横表示位置の算出 */
	top = out->rubi_left_x + ( basewidth - width ) / 2;
	if( top + width > out->rubi_orikaeshi_xmax ){
		top = out->rubi_orikaeshi_xmax - width;
	}
	if( top < 0 ) top = 0;

	/* 描画 */
	x = top;

	for( m = string; ; ){
		int code;
		m = getnextchar( &code, m );
		code = getcode( code );

		if( code == CODE( '}' ) ){
			break;
		}
		x += put_rubi( out->outinfo, x, basey - FONT_SIZE_H / 2, code ) + dx;
	}

	return 0;
}

/* --------------------------------------------------- */
/*
	FONT_VRAMINFO の設定
*/

void font_set_vraminfo( FONT_VRAMINFO *vinfo
						, void *vramtop, int width, int height
						, int c_skip, int l_skip, int flag )
{
	int w, h;

	vinfo->vramtop = vramtop;
	vinfo->row = width * 4 / 8;
	vinfo->height = height;
	vinfo->c_skip = c_skip;
	vinfo->l_skip = l_skip;
	vinfo->flag = flag;

	w = width / ( FONT_SIZE_W + c_skip );
	h = height / ( FONT_SIZE_H + l_skip );

	vinfo->width = w * ( FONT_SIZE_W + c_skip ) - c_skip;
	vinfo->height = height;
	ASSERT( height >= h * ( FONT_SIZE_H + l_skip ) );
#ifdef ENGLISH
	vinfo->width = width;
	vinfo->flag |= FONT_NO_KINSOKU;
#endif
#ifdef CHECK_ONLY
	vinfo->width = width;
	vinfo->flag |= FONT_NO_KINSOKU;
#endif

	if( !( flag & FONT_NO_ORIKAESHI ) ){
		vinfo->c_width = w;
	} else {
		vinfo->c_width = w + 1;
	}
	vinfo->c_height = h + 1;

	if( l_skip >= RUBI_SIZE_H ){
		vinfo->rubi_display_flag = 1;
	} else {
		vinfo->rubi_display_flag = 0;
	}

	vinfo->color = 0;
	vinfo->max_width = 0;
	vinfo->y_step = FONT_SIZE_H + l_skip;

	vinfo->han_func = put_hankaku_4bpp;
	vinfo->zen_func = put_zenkaku_4bpp;
	vinfo->rubi_func = draw_rubi_string;
}

/* --------------------------------------------------- */
/*
	4BitClutの生成
*/

void font_set_clut4( u_int *clut_buf, int no, int fg, int bg )
{
	u_int *top;
	struct {
		unsigned char r, g, b, a;
	} *fc, *bc;
	int r, g, b;

	fc = ( typeof( fc ) )&fg;
	bc = ( typeof( bc ) )&bg;

	ASSERT( no < 4 );
	top = clut_buf + no * 4;

	top[ 0 ] = bg;

	r = ( fc->r * 1 + bc->r * 2 ) / 3;
	g = ( fc->g * 1 + bc->g * 2 ) / 3;
	b = ( fc->b * 1 + bc->b * 2 ) / 3;
	top[ 1 ] = FONT_RGB( r, g, b );

	r = ( fc->r * 5 + bc->r * 3 ) / 8;
	g = ( fc->g * 5 + bc->g * 3 ) / 8;
	b = ( fc->b * 5 + bc->b * 3 ) / 8;
	top[ 2 ] = FONT_RGB( r, g, b );

	top[ 3 ] = fg;
}

/* --------------------------------------------------- */
/*
	draw_string の補助関数
*/

static int draw_font( FONT_DRAWINFO *dr, int x, int y, int code )
{
	FONT_VRAMINFO *out;
	int dx, flag;

	out = dr->outinfo;
	flag = dr->put_Tdot_flag;

	if( IS_HANKAKU( code ) ){
		dx = ( *out->han_func )( out, x, y, code, flag );
	} else {
		dx = ( *out->zen_func )( out, x, y, code, flag );
	}

	return dx + out->c_skip;
}

static int draw_rubi( FONT_DRAWINFO *dr, int x, int y, unsigned char *str )
{
	if( dr->outinfo->rubi_display_flag == 0 ){
		return 0;
	}
	return ( *dr->outinfo->rubi_func )( dr, x, y, str );
}

/* --------------------------------------------------- */

int font_draw_string( FONT_DRAWINFO *work, char *string )
{
	FONT_VRAMINFO *outinfo;
	unsigned char *m;
	int x, y;
	int dx;
	int result = 0;
	int orikaeshi_flag = 0;

	outinfo = work->outinfo;
	x = work->xtop;
	y = work->ytop + outinfo->l_skip;

	m = ( unsigned char * )string;
	outinfo->max_width = 0;
	dx = 0;

	for( ;; ){
		char *next;
		int mdata, code;

		if( *m == '\0' ){
			break;
		}
		next = getnextchar( &mdata, m );
		code = getcode( mdata );
		/* 特殊コマンド文字解析 */
		if( mdata < CODE( 0x20 ) ){
			/* コントロールコード */
			m = next;
		} else {
			if( code == CODE( '#' ) ){
				int d;
				/* # の後は制御文字 */
				m = getnextchar( &d, next );
				d = getcode( d );
				switch( d ){
				  case CODE( 'R' ):
					m = getnextchar( &d, m );
					ASSERT( getcode( d ) == CODE( '{' ) );
				  case CODE( '{' ):
					/* ルビ表示 */
					work->block_flag = TRUE;
					if( outinfo->rubi_display_flag ){
						work->rubi_flag = TRUE;
						set_rubi_left_pos( work, outinfo->width, x, y );
					}
					break;
				  case CODE( 'T' ):
					/* 強調点 */
					m = getnextchar( &d, m );
					ASSERT( getcode( d ) == CODE( '{' ) );
					work->block_flag = TRUE;
					work->put_Tdot_flag = TRUE;
					break;
				  case CODE( '-' ):
				  case 0x8312:
				  case 0x8313:
					/* 長棒 ( x1.5倍程度 ) */
					font_line_len = FONT_LINE_LEN;
					code = 0;
					goto DRAW;
				  case CODE( '_' ):
					font_line_len = FONT_SIZE_W;
					code = 0;
					goto DRAW;
				  case CODE( 'N' ):
					/* 改行 */
					mdata = code = CODE( '\n' );
					break;
				  case CODE( 0x8302 ):	// ○
				  case CODE( 0x8303 ):	// △
				  case CODE( 0x8304 ):	// □
				  case CODE( 0x8305 ):	// ×
					// パッドコンフィグによって変える
					mdata = code = d;
					break;
				  case CODE( '1' ):
					// 指定数値に変換
					if( font_ref_num < 10 ){
						mdata = code = CODE( '0' + font_ref_num );
					} else {
						mdata = code = CODE( '*' );
					}
					goto DRAW;
				  default:
					/* コード変換が必要なものはコード変換を行なう */
//					mdata = code = code_convert( mdata );
					break;
				}
			} else if( work->block_flag ){
				/* ブロックの終了判定 */
				if( IS_BLOCK_END_CODE( code ) ){
					if( work->rubi_flag && code != CODE( '}' ) ){
						// 、。, で区切られたうしろ
						draw_rubi( work, x, y, next );
					}
					work->rubi_flag = FALSE;
					do {
						m = getnextchar( &code, m );
						code = getcode( code );
					} while( code != CODE( '}' ) );
					next = getnextchar( &code, m );
					if( code == CODE( '#' ) ){
						/* skip ( old style ) */
						m = next;
					}
					work->put_Tdot_flag = FALSE;
					work->block_flag = FALSE;
					/* 描画処理を飛ばす */
					dx = 0;
					goto NEXT;
				}
			} else if( code == CODE( '|' ) ){
				/* 改行 */
				m = next;
				mdata = code = CODE( '\n' );
			}
		}

		/* 特殊文字実行 */
		if( mdata < CODE( 0x20 ) ){
			switch( mdata ){
			  case CODE( '\n' ):
				/* 改行 */
				if( work->cr_flag == FALSE ){
					goto DO_CR;
				} else {
					// 直前が改行されていたら、改行は行なわない
					work->cr_flag = FALSE;
					orikaeshi_flag = 0;
				}
				break;
			}
			continue;
		}

		/* ここから通常文字描画 */
		work->prev_char = m;
		m = getnextchar( &mdata, m );
		code = getcode( mdata );
DRAW:
		/* 描画処理 */
		dx = draw_font( work, x, y, code );
NEXT:
		/* 禁則処理とスキップ処理 */
		{
			char *nn;
			int next_width;
			int next_mdata, next_code;
			int buf_width;
			nn = getnextchar( &next_mdata, m );
			next_code = getcode( next_mdata );
			next_width = get_font_width( next_code );

			if( outinfo->flag & FONT_NO_KINSOKU ){
				buf_width = outinfo->width;
			} else {
				buf_width = outinfo->width - FONT_SIZE_W;
			}

			/*
				ここで、
					mdata : 描画した文字
					next_mdata  : 次に描画する文字
			*/

			if( next_width > 0
				&& ( x + dx + next_width + outinfo->c_skip - 1 ) >= buf_width ){
				int no_orikaeshi = 0;

				// 折り返し処理が必要？
				if( ! ( outinfo->flag & FONT_NO_KINSOKU ) ){
					/* 禁則処理開始 */
					if( mdata & BACK_KINSOKU_MASK ){
						/* すでに書いてしまった禁則文字を消して巻き戻し */
						draw_font( work, x, y, SP_SPACE );
						m = work->prev_char;
						dx = 0;
					} else if( next_mdata & TOP_KINSOKU_MASK ){
						if( work->block_flag && IS_BLOCK_END_CODE( next_code ) ){
							/* ブロックの終端 */
							goto NO_KINSOKU;
						} else {
							/* 先頭禁則 */
							int nn_mdata;
							getnextchar( &nn_mdata, nn );
							if( code == SP_JMARU || !( nn_mdata & TOP_KINSOKU_MASK ) ){
								// 禁則が重なった場合は禁則処理を行なわないが、
								// 「。」の時だけは禁則処理を行なう。
								dx += draw_font( work, x + dx, y, next_code );
								m = nn;
								no_orikaeshi = 1;
							}
						}
					}
				}
				if( no_orikaeshi == 0 ){
					orikaeshi_flag = 1;
				}
				if( work->rubi_flag ){
					set_rubi_orikaeshi( work, x + dx );
				}
DO_CR:
				// 改行処理
				x += dx;
				if( outinfo->max_width < x ){
					outinfo->max_width = x;
				}
				x = work->xtop;
				y = y + outinfo->y_step;
				work->yc ++;
				work->cr_flag = TRUE;

				if( y + FONT_SIZE_H + DOWN_MARGINE > outinfo->height
					 || work->yc >= outinfo->c_height ){
					if( *m != '\0' && *m != '\n' ){
						goto OVER;
					} else {
						goto OK;
					}
				}
			} else {
NO_KINSOKU:
				/* 改行の必要なし */
				x += dx;
				work->cr_flag = FALSE;
				if( orikaeshi_flag ){
					result |= FONT_RESULT_ORIKAESHI;
				}
			}
		}
	}
	/* 終了時処理 */
	if( outinfo->max_width < x ){
		outinfo->max_width = x;
	}
OK:
	return result;
OVER:
	return result | FONT_RESULT_OVER;
}

/* ----------------------------------------------------------- */

void font_open_drawinfo( FONT_DRAWINFO *draw, FONT_VRAMINFO *vram )
{
	memset( draw, 0, sizeof( FONT_DRAWINFO ) );

	draw->outinfo = vram;
}

void font_set_locate( FONT_DRAWINFO *draw, int x, int y )
{
	draw->xtop = x;
	draw->ytop = y;
}

void font_set_color( FONT_DRAWINFO *draw, int color )
{
	draw->color = color;
	draw->outinfo->color = color;
}

/* ----------------------------------------------------------- */

void font_set_refnum( int refnum )
{
	font_ref_num = refnum;
}

#ifdef CHECK_ONLY

/* ---------------------------------------------------------------------- */
/*
	ツールで使う時のチェック用

	font_resident_load_set( char *fonttop )
	で常駐用フォントを設定。
	ENCRIPT済みの文字列に対してこの関数を呼び出すことで,
	表示がうまくかどうかを算出する。

	返り値は
	0x00000000		OK
	0x00000001		YOVER
*/

int font_draw_area_check( char *mesg, int width, int height, int c_skip, int l_skip, int flag )
{
	FONT_VRAMINFO vinfo;
	FONT_DRAWINFO drawinfo;
	int res;

	font_set_vraminfo( &vinfo, NULL, width, height, c_skip, l_skip, flag );
	font_open_drawinfo( &drawinfo, &vinfo );

	return font_draw_string( &drawinfo, mesg );
}

#endif
