// -------------------------------------------------------------------------
//	HUF.C - ハフマン符号化
//
//	Copyright (C) 1997 M.Furuichi
//
//	compiled : Turbo C++ Ver.2.00
//
//	note )
//		・Compact 以上の model でのコンパイル要
//
//		・８ Bit の頻度表を出力するかわりに４ Bit の符号長表を
//		出力するように改良したハフマン符号プログラム
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

//	ハフマン符号の定数

#define	HUFFBIT		8
#define TABLENO		(1U << HUFFBIT)
#define HUFNIL		(-1)

//	短縮した型定義 ---------------------------------------------------------

#ifndef ulong
#define	ulong	unsigned long
#endif

#ifndef uint
#define	uint	unsigned short int
#endif

#ifndef uchar
#define	uchar	unsigned char
#endif

#ifndef ushort
#define	ushort	unsigned short int
#endif

//	構造体の定義 ===========================================================

typedef struct tree_node
{
	unsigned int count;
	int	up;
	int child_0;
	int child_1;
} NODE;

typedef struct code
{
	unsigned int code;	// ハフマン記号データ
	int code_bits;		// 有効なビット数
} CODE;

typedef struct huff
{
	int		bitno;
	int		tableno;
	ulong *	freq;
	CODE *	code;
	NODE *	node;
} HUFF;

//	グローバル変数 ---------------------------------------------------------

static ushort len_16cnt[17];

// =========================================================================
//	bitio
// =========================================================================

#define rightbits(n,x) ((x)&((1U<<(n))-1U))

FILE *infile, *outfile;

unsigned bitbuf = 0;
unsigned long outcount = 0;
int getcount = 0, putcount = 8;

unsigned getbit( void )
{
	if( --getcount >= 0 )
		return (bitbuf >> getcount) & 1U;

	getcount = 7;
	bitbuf = getc( infile );
	return ( bitbuf >> 7 ) & 1U;
}

unsigned getbits( int n )
{
	unsigned x = 0;
	while( n > getcount )
	{
		n -= getcount;
		x |= rightbits( getcount, bitbuf) << n;
		bitbuf = getc( infile );
		getcount = 8;
	}
	getcount -= n;
	return x | rightbits( n, bitbuf >> getcount);
}

void putbit( unsigned bit )
{
	putcount--;

	if( bit != 0 )
		bitbuf |= ( 1 << putcount );

	if( putcount == 0 )
	{
		if( putc( bitbuf, outfile) == EOF )
		{
			printf("Can't write\n");
			exit(0);
		}
		bitbuf = 0;
		putcount = 8;
		outcount++;
	}
}

void putbits( int n, unsigned x)
{
	if( n > 16 )
		printf("Error : too big bits\n");

	while( n >= putcount )
	{
		n -= putcount;
		bitbuf |= rightbits( putcount, x >> n);
		if( putc( bitbuf, outfile) == EOF )
		{
			printf("Can't write\n");
			exit(0);
		}
		bitbuf = 0U;
		putcount = 8;
		outcount++;
	}
	putcount -= n;
	bitbuf |= rightbits( n, x) << putcount;
}

// =========================================================================
//	δ符号を扱うための関数
// =========================================================================

//	整数用対数 log2 num を返す filt=2^(log2 num) ---------------------------

int I_log(int num, int *filt)
{
	int i = 0;
	(*filt) = 1;

	for( i = 0; (*filt) <= num; i++ )
		*filt = (*filt) << 1;

	return i;
}

// filt 通りのbit幅で codeを符号化 -----------------------------------------

void fit_code( int code, int filt)
{
	int bit;

	filt >>= 1;
	while( filt > 0 )
	{
		bit = ((code & filt) == 0) ? 0 : 1;
		putbit( bit );
		filt >>= 1;
	}
}

//	δ符号化 ---------------------------------------------------------------

void put_gamma( int code )
{
    int i, ret;
    int leng;

    leng = I_log( code, &ret);

    for( i = 1; i < leng; i++ )
		putbit(0);

	fit_code( code, ret);
}

//	δ復号化 ---------------------------------------------------------------

int get_gamma(void)
{
    int i, leng = 0, code = 1, bit;

	while( !getbit() )
		leng++;

	for( i = 0 ; i < leng ; i++ )
	{
		bit = getbit();
		code = (code << 1) + bit;
    }
    return code;
}

// =========================================================================
//	huffman encoding functions
// =========================================================================

//	ハフマン木を作る時だけに使用する変数
static int g_Node_size;

//	huffman 法用のメモリの確保 ---------------------------------------------

static HUFF * huff_create( int bitno, int tableno )
{
	static HUFF *h;

	//	構造体用のメモリを確保
	if( ( h = (HUFF*)calloc( 1, sizeof( HUFF ))) == NULL )
	{
		printf( "Error allocating huff structure\n" );	exit(0);
	}

	h->bitno	= bitno;
	h->tableno	= tableno;

	//	各作業用メモリを確保
	if(( h->node = (NODE*)calloc( h->tableno * 2, sizeof( NODE ) )) == NULL)
	{
		printf( "Error allocating nodes array\n" );	exit(0);
	}
	if(( h->code = (CODE*)calloc( h->tableno, sizeof( CODE ) ) ) == NULL )
	{
		printf( "Error allocating codes array\n" );	exit(0);
	}
	if(( h->freq = (ulong*)calloc( h->tableno, sizeof( ulong ) ) ) == NULL )
	{
		printf( "Error allocating freq array\n" );	exit(0);
	}
	return h;
}

//	huffman 法で使用したメモリの解放 ---------------------------------------

static void huff_release( HUFF *h )
{
	if( h )
	{
		if( h->node )	free( (char *)h->node );
		if( h->code )	free( (char *)h->code );
		if( h->freq )	free( (char *)h->freq );
		free( (char *)h );
	}
}

//	huffman 法で使用するデータ域の初期化 -----------------------------------

static void huff_init( HUFF *h )
{
	uint	i;

	//	huffman に関連する全ての変数を初期化します
	for( i = 0 ; i < h->tableno ; i++ )
	{
		h->freq[ i ]			= 0;
		h->code[ i ].code_bits	= h->code[ i ].code		= 0;
	}
	for( i = 0 ; i < h->tableno * 2 ; i++ )
	{
		h->node[ i ].count = 0;
		h->node[ i ].child_0 = h->node[ i ].child_1 = h->node[ i ].up = HUFNIL;
	}
}

//	頻度テーブルを３２ビットから８ビットにスケールダウンする ---------------

static void huff_scale_count( HUFF *h )
{
	uint	i;
	ulong	max_count;

	max_count = 0;

	for( i = 0 ; i < h->tableno ; i++ )
		if( h->freq[ i ] > max_count )
			max_count = h->freq[ i ];

	if( max_count == 0 )
	{
		h->freq[ 0 ] = 1;
		max_count = 1;
	}

	max_count = max_count / (1 << h->bitno);
	max_count = max_count + 1;

	for( i = 0 ; i < h->tableno ; i++ )
	{
		h->node[ i ].count = (unsigned int)( h->freq[ i ] / max_count );
		if( h->node[ i ].count == 0 && h->freq[ i ] != 0 )
			h->node[ i ].count = 1;
	}
}

//	出現頻度の少ない葉を見つける -------------------------------------------

static int huff_search_least( NODE *node )
{
	int i, least = HUFNIL;

	for( i = 0; i < g_Node_size ; i++ )
	{
		if( node[ i ].up == HUFNIL && node[ i ].count > 0 )
		{
			if( least == HUFNIL || node[ least ].count > node[ i ].count )
				least = i;
		}
	}
	if( least == HUFNIL )
		return -1;

    return least;
}

//	ハフマン木を作る -------------------------------------------------------

static int huff_build_tree( HUFF *h )
{
	int left, right, root, i;

	g_Node_size = h->tableno;

	left = huff_search_least( h->node );

	while( left >= 0 )
	{
		h->node[ left ].up				= g_Node_size;
		h->node[ g_Node_size ].up		= HUFNIL;
		h->node[ g_Node_size ].child_0	= left;
		h->node[ g_Node_size ].child_1	= HUFNIL;

		right = huff_search_least( h->node );
		if( right < 0 )
		{
			root = left;	// １番頻度の多いnodeがrootとなる
			if( h->node[ root ].child_0 == HUFNIL
			&&	h->node[ root ].child_1 == HUFNIL )
			{
				root = g_Node_size;	// 木がnodeのみの時(null file)
				g_Node_size++;		// rootと葉だけの木を作る
			}
			return root;
		}
		h->node[ g_Node_size ].child_1 = right;
		h->node[ g_Node_size ].count
			= h->node[ left ].count + h->node[ right ].count;
		h->node[ right ].up = g_Node_size;
		g_Node_size++;
		left = huff_search_least( h->node );
	}
	// ここには絶対来ない
	return HUFNIL;
}

//	ハフマン木をたどって符号長をカウントする -------------------------------

static void huff_count_len( HUFF *h, int i)
{
	static int depth = 0;

	if( i < h->tableno )
	{
		h->code[ i ].code_bits = depth;
		len_16cnt[ depth ]++;
	}
	else
	{
		depth++;
		huff_count_len( h, h->node[i].child_0 );
		huff_count_len( h, h->node[i].child_1 );
		depth--;
	}
}

//	符号長をカウントする ---------------------------------------------------

static void huff_make_len( HUFF *h, int root )
{
	int i;

	//	１６種類のカウンタを初期化
	for( i = 0 ; i <= 16 ; i++ ) len_16cnt[ i ] = 0;

	//	全ての葉にたいしてカウントを行う
	huff_count_len( h, root );
}

//	符号長からハフマン符号を生成 -------------------------------------------

static void huff_make_code( HUFF *h )
{
	int		i, j;
	uint	start[ 18 ];

	for( i = 0 ; i < 18 ; i++ )	start[ i ] = 0;

	start[ 1 ] = 0;
	for( i = 1 ; i <= 16 ; i++ )
		start[ i + 1 ] = ( start[ i ] + len_16cnt[ i ] ) << 1;

	for( i = 0 ; i < 18 ; i++ )
	{
		for( j = 0 ; j < h->tableno ; j++ )
			if( h->code[ j ].code_bits == i )
				h->code[ j ].code = start[ i ]++;
	}
}

//	指定された文字を符号にして出力 -----------------------------------------

static void huff_encode( short int c ,HUFF* p_huff )
{
	putbits( p_huff->code[ c ].code_bits, p_huff->code[ c ].code);
}

//	符号語を読込んで文字に復元 ---------------------------------------------

static int huff_decode( HUFF* p_huff , int root)
{
	int node;

	node = root;				// 木の幹

	do
	{
		if( getbit() )
			node = p_huff->node[ node ].child_1;
		else
			node = p_huff->node[ node ].child_0;
	}
	while( node >= p_huff->tableno );

	return node;
}

//	符号長を連長＋δ符号で圧縮して出力 -------------------------------------

static void huff_output_counts( HUFF *h )
{
	int i, pc, c;

	pc = 0;
	for( i = 0 ; i < h->tableno ; i++ )
	{
		c = h->code[ i ].code_bits;
		if( c ^ pc )
		{
			putbit(1);
			put_gamma( c ^pc);
		}
		else
		{
			putbit(0);
		}
		pc = c;
	}
}

//	圧縮された符号長を復元し符号語を復元する -------------------------------

static void huff_input_counts( HUFF *h )
{
	int i, pc;

	pc = 0;
	for( i = 0 ; i < h->tableno ; i++ )
	{
		if( getbit() )	h->code[ i ].code_bits = get_gamma() ^ pc;
		else			h->code[ i ].code_bits = pc;
		pc = h->code[ i ].code_bits;
	}
}

//	符号長表から符号語をつくる ---------------------------------------------

static void huff_restore_code( HUFF *h )
{
	int i, j, pc, root, node;

	for( i = 0 ; i <= 16 ; i++ )	len_16cnt[ i ] = 0;

	for( i = 0 ; i < h->tableno ; i++ )
	{
		if( h->code[ i ].code_bits > 16 )
		{
			printf("符号長データが壊れています。\n");	exit(0);
		}
		len_16cnt[ h->code[ i ].code_bits ]++;
	}
	huff_make_code( h );
}

//	符号語から復号用のハフマン木をつくる -----------------------------------

int huff_restore_tree( HUFF *h )
{
	unsigned int	i, j;
	unsigned int	root_pos, next_free;

	root_pos = h->tableno;
	next_free = h->tableno +1;
	h->node[ root_pos ].child_0 = HUFNIL;
	h->node[ root_pos ].child_1 = HUFNIL;
	h->node[ next_free ].child_0 = HUFNIL;
	h->node[ next_free ].child_1 = HUFNIL;

	for( i = 0 ; i < h->tableno ; i++ )
	{
		root_pos = h->tableno;
		for( j = 0 ; j < h->code[ i ].code_bits ; j++ )
		{
			if( h->code[ i ].code & (1 << h->code[ i ].code_bits-j-1) )
			{
				if( j+1 == h->code[ i ].code_bits )
				{
					h->node[ root_pos ].child_1 = i;
					break;
				}

				if( h->node[ root_pos ].child_1 != HUFNIL )
					root_pos = h->node[ root_pos ].child_1;
				else
				{
					h->node[ root_pos ].child_1 = next_free;
					h->node[ next_free ].child_0 = HUFNIL;
					h->node[ next_free ].child_1 = HUFNIL;
					root_pos = next_free;
					next_free++;
				}
			}
			else
			{
				if( j+1 == h->code[ i ].code_bits )
				{
					h->node[ root_pos ].child_0 = i;
					break;
				}

				if( h->node[ root_pos ].child_0 != HUFNIL )
				{
					root_pos = h->node[ root_pos ].child_0;
				}
				else
				{
					h->node[ root_pos ].child_0 = next_free;
					h->node[ next_free ].child_0 = HUFNIL;
					h->node[ next_free ].child_1 = HUFNIL;
					root_pos = next_free;
					next_free++;
				}
			}
		}
	}
	return h->tableno;
}

void encode( void )
{
	int c;
	HUFF *h;
	int root;
	int	i;

	h = huff_create( HUFFBIT, TABLENO );
	huff_init( h );

	while( ( c = getc( infile )) != EOF )
		h->freq[ c ]++;

	fseek( infile, 0L, 0);

	huff_scale_count( h );
	root = huff_build_tree( h );

	// ハフマン木の長さをうまく処理してカウント
	huff_make_len( h, root );
	huff_make_code( h );				// 簡易ハフマン符号を生成

#ifdef DEBUG
	for( i = 0 ; i < h->tableno ; i++ )
	{
		if( !h->code[i].code_bits )continue;
		printf("%d(%d):",i,h->code[i].code_bits );
		for( c = 0 ; c < h->code[i].code_bits ; c++ )
			printf("%d", h->code[i].code >> (h->code[i].code_bits - c -1) & 1 );
		printf("\n");
	}
#endif

	huff_output_counts( h );

	while( ( c = getc( infile )) != EOF )
		huff_encode( c, h );

	putbits( 7, 0);
	huff_release( h );
}

void decode( unsigned long count )
{
	ulong l;
	HUFF *h;
	int root, node;
	int	i,j;

	h = huff_create( HUFFBIT, TABLENO );
	huff_init( h );
	huff_input_counts( h );
	huff_restore_code( h );

#ifdef DEBUG
	for( i = 0 ; i < h->tableno ; i++ )
	{
		if( !h->code[i].code_bits )continue;
		printf("%d(%d):",i,h->code[i].code_bits );
		for( j = 0 ; j < h->code[i].code_bits ; j++ )
			printf("%d", h->code[i].code >> (h->code[i].code_bits - j -1) & 1 );
		printf("\n");
	}
#endif

	root = huff_restore_tree( h );

	for( l = 0 ; l < count ; l++ )
	{
		node = huff_decode( h, root);
		putc( node, outfile);
	}
	huff_release( h );
	printf("%12lu\n", count);
}

int main( int argc, char **argv)
{
	int c;
	unsigned long size;

	if(argc != 4 ||
		((c = *argv[1]) != 'E' && c != 'e' && c != 'D' && c != 'd'))
	{
		printf(
			"\n<< HUF >> Ver.1.00 Copyright (C) 1997 M.Furuichi"
			"\nUsage : huf [e|d] (infile) (outfile)"
			"\n  e   compress"
			"\n  d   uncompress\n");
		exit(0);
	}
	if( (infile = fopen( argv[2], "rb")) == NULL )
	{
		printf("Can't open infile %s", argv[2]);
		exit(0);
	}
	if( (outfile = fopen( argv[3], "wb")) == NULL )
	{
		printf("Can't open outfile %s", argv[3]);
		exit(0);
	}
	if(c == 'E' || c == 'e')
	{
		fseek( infile, 0L, SEEK_END);
		size = ftell( infile );
		fwrite( &size, sizeof size, 1, outfile);
		rewind( infile );
		encode();
	}
	else
	{
		fread( &size, sizeof size, 1, infile);
		decode( size );
	}
	fclose( infile );
	fclose( outfile );
	return EXIT_SUCCESS;
}
