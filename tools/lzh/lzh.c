//==========================================================================
//	lzsh.c
//
//	Copyright (C) 1997 M.Furuichi
//
//	ハッシュ法と線形リスと構造による LZSS 圧縮プログラム
//	+ 符号長から作るハフマン符号化
//
//	注意：
//	このプログラムを作者の了承無く研究目的以外で使用する事を禁止致します。
//
//	[compiler]
//	Turbo C++  Version 1.01 Copyright (c) 1990 Borland International 
//	※　コンパクトモデルでビルドする必要があります
//==========================================================================

#include <stdio.h>
#include <stdlib.h>


//	LZSS 関連 --------------------------------------------------------------

#define BREAKCNT	256			//	最大検索回数

#define THRESHOLD	3			//	しきい値

#define	F		( 256 + 2 )		//	最大一致文字数
#define DICBIT		14			//	辞書ビット数
#define DICSIZ		(1U << DICBIT)		//	辞書のサイズ
#define HASHBIT		14			//	ハッシュ表のビット数
#define HASHSIZE	(1U << HASHBIT)		//	ハッシュ表のサイズ
#define NIL		DICSIZ			//	未登録記号

#define	DIC(s)		(s & ( DICSIZ - 1))	//	辞書のサイズに制限するマクロ

#define	HASH(a,b,c)	((((a << (HASHBIT -8)) + b<<((HASHBIT -8)/2)) ^ c ) & (HASHSIZE - 1))
						//	３文字から求めるハッシュ関数


//	huffman 関連 -----------------------------------------------------------

#define	CHAR_BIT8		8
#define	D128			128
#define	D256			256
#define UCHAR_MAX		255

//	ＬＺＳＳの出力を溜めるバッファサイズ
#define	HUFFBUFSIZE		(16*1024)	// 1M-1 のハフマン符号用バッファ

//	ハフマン符号のテーブルサイズ
#define	childN			(F -THRESHOLD +1 + D256)
#define	parentN			16
#define headN			16

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

static int		breaksize = BREAKCNT;	//	検索の最大回数
static ushort	len_16cnt[ 17 ];

static uchar *	g_hufbuf;
static ulong	g_hufsize;

static HUFF *	h_ptr;					//	ポインタ長圧縮の構造体
static HUFF *	h_char;					//	一致長／一致文字圧縮の構造体
static HUFF *	h_head;					//	符号長テーブルの圧縮用

static uchar *	g_text;					//	テキストバッファへのポインタ
static int *	g_next;					//	次位置バッファへのポインタ
static int *	g_index;				//	ハッシュ表へのポインタ
static int		g_matchlen = 0, g_matchpos = 0;	//	一致情報
static int		g_remainder;			//	メモリ内の入力文字数
static uint		g_pos;					//	圧縮位置
static uint *	g_cnt;


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
    bitbuf = getc( infile ) ;
    return ( bitbuf >> 7 ) & 1U;
}

uint getbits(int n)
{
#if 1
    uint x = 0 ;

    while( n > getcount )
    {
	n -= getcount ;
	x |= rightbits( getcount, bitbuf ) << n ;
	bitbuf = getc( infile ) ;
	getcount = 8 ;
    }
    getcount -= n ;
    return x | rightbits( n, bitbuf >> getcount) ;
#else
    uint x=bitbuf, i ;

    for ( i=(n-getcount+7)/8 ; --i>=0 ; x<<=8 )
	x |= (bitbuf = getc( infile )) ;
    return (x<<getcount) >> (32-n) ;
#endif
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
	{
		printf("n > 16\n");
	}
	if( n == 0 )
	{
		printf("n == 0\n");
	}
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

//	一致記号数の符号化 -----------------------------------------------------

void put_gamma( int code )
{
    int i, ret;
    int leng;

    leng = I_log( code, &ret);

    for( i = 1; i < leng; i++ )
		putbit(0);

	fit_code( code, ret);
}

//	一致列長 読込み --------------------------------------------------------

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
		if( h->node )	free( h->node );
		if( h->code )	free( h->code );
		if( h->freq )	free( h->freq );
		free( h );
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
	int	i, least = HUFNIL;

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
	int	left, right, root, i;

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
    int i, j;
    int start[ 18 ];

    for( i=18 ; --i>=0 ; )
	start[i] = 0 ;

    start[1] = 0;
    for( i = 1 ; i <= 16 ; i++ )
	start[i+1] = ( start[i] + len_16cnt[i] ) << 1;

    for( i = 0 ; i < 18 ; i++ )
	for( j = 0 ; j < h->tableno ; j++ )
	    if( h->code[j].code_bits == i )
		h->code[j].code = start[i]++ ;
}

//	指定された文字を符号にして出力 -----------------------------------------

static void huff_encode( short int c ,HUFF* p_huff )
{
    putbits( p_huff->code[ c ].code_bits, p_huff->code[ c ].code);
}

//	符号語を読込んで文字に復元 ---------------------------------------------

static int huff_decode( HUFF* h , int root)
{
    int node;

    node = root ;				/* 木の幹 */
    do
	node = getbit() ? h->node[node].child_1 : h->node[node].child_0 ;
    while( node >= h->tableno ) ;

    return node ;
}

//	符号長を連長＋δ符号で圧縮して出力 -------------------------------------

static void huff_output_counts( HUFF *h )
{
    int i, pc, c;

    pc = 0;
    for( i=0 ; i < h->tableno ; i++ )
    {
	c = h->code[ i ].code_bits;
	if( c ^ pc )
	{
	    putbit(1);
	    put_gamma( c ^pc);
	}
	else
	    putbit(0);
	pc = c;
    }
}

//	出現頻度表の符号化 -----------------------------------------------------
//	試行錯誤した為ソースがわかりにくい。ここはもう少しまとめる必要がある。

static void huff_output_counts2( void )
{
	int i, pc, c, root, skip, len, j, flag;
	ushort	xorbuf[ childN ];

	pc = 0;
	for( i = 0 ; i < h_char->tableno ; i++ )
	{
		xorbuf[ i ] = h_char->code[ i ].code_bits ^ pc;
		pc = h_char->code[ i ].code_bits;

//		printf("%x,",h_char->code[ i ].code_bits);

		if( xorbuf[ i ] )
			h_head->freq[ xorbuf[ i ] ]++;
	}

	huff_scale_count( h_head );
	root = huff_build_tree( h_head );
	huff_make_len( h_head, root );
	huff_make_code( h_head );				// 簡易ハフマン符号を生成

	huff_output_counts( h_head );

	flag = xorbuf[ 0 ];
	if( flag )	putbit(1);
	else		putbit(0);

	for( i = 0 ; i < h_char->tableno ; )
	{
		if( flag )
		{
//			printf("D");
			len = 0;
			for( j = i; j < h_char->tableno ; j++ )
			{
				c = xorbuf[ j ];
				if( !c ) break;
				len++;
			}

//			printf("%d->", len);
			put_gamma( len );

			for( j = 0; j < len ; j++ )
			{
				c = xorbuf[ i + j ];
				putbits( h_head->code[ c ].code_bits, h_head->code[ c ].code);
			}
			i += len;
			flag = 0;
		}
		else
		{
//			printf("S");
			skip = 0;
			for( j = i; j < h_char->tableno ; j++ )
			{
				c = xorbuf[ j ];
				if( !c )
				{
					skip++;
				}
				else
				{
					break;
				}
			}
//			printf("%d->",skip);
			put_gamma( skip );
			i += skip;
			flag = 1;
		}
	}

	huff_output_counts( h_ptr );
}

//	圧縮された符号長を復元し符号語を復元する -------------------------------

static void huff_input_counts( void )
{
	int i, j, pc, root, node, flag, skip, len;
	ushort	xorbuf[ childN ];

	pc = 0;
	for( i = 0 ; i < h_head->tableno ; i++ )
	{
		if( getbit() )	h_head->code[ i ].code_bits = get_gamma() ^ pc;
		else			h_head->code[ i ].code_bits = pc;
		pc = h_head->code[ i ].code_bits;
	}
	for( i = 0 ; i <= 16 ; i++ )	len_16cnt[ i ] = 0;
	for( i = 0 ; i < h_head->tableno ; i++ )
	{
		if( h_head->code[ i ].code_bits > 16 )
		{
			printf("符号長データが壊れています。\n");	exit(0);
		}
		len_16cnt[ h_head->code[ i ].code_bits ]++;
	}
	huff_make_code( h_head );
	root = restore_tree( h_head );

	for( i = 0 ; i < h_char->tableno ; i++ )
		xorbuf[ i ] = 0;

	flag = getbit();
	for( i = 0 ; i < h_char->tableno ; )
	{
		if( flag )
		{
//			printf("D");
			len = get_gamma();
//			printf("%d->", len);

			for( j = 0; j < len ; j++ )
			{
				xorbuf[ i + j ] = huff_decode( h_head, root);
			}
			i += len;
			flag = 0;
		}
		else
		{
//			printf("S");
			skip = get_gamma();
//			printf("%d->",skip);

			for( j = 0; j < len ; j++ )
			{
				xorbuf[ i + j ] = 0;
			}
			i += skip;
			flag = 1;
		}
	}

	pc = 0;
	for( i = 0 ; i < h_char->tableno ; i++ )
	{
		h_char->code[ i ].code_bits = xorbuf[ i ] ^ pc;
		pc = h_char->code[ i ].code_bits;

//		printf("%x,",h_char->code[ i ].code_bits);
	}


//	for( i = 0 ; i < h_ptr->tableno ; i++ )
//		h_ptr->code[ i ].code_bits = huff_decode( h_head, root);

	for( i = 0 ; i <= 16 ; i++ )	len_16cnt[ i ] = 0;

	for( i = 0 ; i < h_char->tableno ; i++ )
	{
		if( h_char->code[ i ].code_bits > 16 )
		{
			printf("符号長データが壊れています。\n");	exit(0);
		}
		len_16cnt[ h_char->code[ i ].code_bits ]++;
	}
	huff_make_code( h_char );

	pc = 0;
	for( i = 0 ; i < h_ptr->tableno ; i++ )
	{
		if( getbit() )	h_ptr->code[ i ].code_bits = get_gamma() ^ pc;
		else			h_ptr->code[ i ].code_bits = pc;
		pc = h_ptr->code[ i ].code_bits;
	}

	for( i = 0 ; i <= 16 ; i++ )	len_16cnt[ i ] = 0;

	for( i = 0 ; i < h_ptr->tableno ; i++ )
	{
		if( h_ptr->code[ i ].code_bits > 16 )
		{
			printf("符号長データが壊れています。\n");	exit(0);
		}
		len_16cnt[ h_ptr->code[ i ].code_bits ]++;
	}
	huff_make_code( h_ptr );
}

//	符号語から復号用のハフマン木をつくる -----------------------------------

int restore_tree( HUFF *h )
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

// =========================================================================
//	ここからは LZH のテクニック
// =========================================================================

//	親データ出力関数 親データ(>UCHAR_MAX) ----------------------------------

static void encode_p( unsigned short int p, HUFF * p_huff )
{
    unsigned short int q = p;
    unsigned short int c;

    for( c = 0 ; q ; c++ )
	q >>= 1;

    huff_encode( c, p_huff);

    if( c > 1 )
	putbits( c - 1 , p & (0xFFFFU >> (17 - c)));
}

//	ブロツク移動関数 -------------------------------------------------------

static void send_block(void)
{
    unsigned short int i,k;
    unsigned short int flags;
    unsigned short int pos;
    int		root;

    putbits( 8, g_hufsize & 0xff);
    putbits( 8, g_hufsize >> 8 );

    huff_scale_count( h_char );
    root = huff_build_tree( h_char );
    huff_make_len( h_char, root );
    huff_make_code( h_char );				/* 簡易ハフマン符号を生成 */

    huff_scale_count( h_ptr );
    root = huff_build_tree( h_ptr );
    huff_make_len( h_ptr, root );
    huff_make_code( h_ptr );				/* 簡易ハフマン符号を生成 */
    huff_output_counts2( );

    pos = 0;

    for( i = 0 ; i < g_hufsize ; i++)
    {
	if(i % CHAR_BIT8 == 0)
	    flags = g_hufbuf[pos++];
	else
	    flags<<=1;

	if( flags & D128 )
	{
	    huff_encode( g_hufbuf[pos] + D256 , h_char );
	    pos++;
	    k = g_hufbuf[pos] << CHAR_BIT8;
	    pos++;
	    k += g_hufbuf[pos];
	    pos++;
	    encode_p( k , h_ptr );
	}
	else
	{
	    huff_encode( g_hufbuf[pos] ,h_char );
	    pos++;
	}
    }
    huff_init( h_head );
    huff_init( h_char );
    huff_init( h_ptr );
}

//	ハフマン法圧縮終了関数 -------------------------------------------------

static void huf_encode_end( void )
{
	send_block();
	putbits( 16, 0);
	putbits( CHAR_BIT8 - 1, 0);  /* 残留ビット出力 */
	huff_release( h_head );
	//huff_release( h_ptr );
	huff_release( h_char );
	free( (char *) g_hufbuf );
}

//	LZSS で圧縮されたデータをハフマンバッファへ出力 ------------------------
//	モード(0:初期化 1:次呼出)
static void output( unsigned short int c, unsigned short int p, int mode)
{
	static unsigned short int pos1;
	static unsigned short int pos2;
	static unsigned short int mask;

	// ●初期化 mode
	if( mode == 0 )
	{
		pos2 = mask = 0;
		g_hufsize = 0;
		return;
	}

	//	flag(ひだりづめ)位置の更新
	mask >>= 1;

	//	もしマスクがあふれたのなら
	if( mask == 0 )
	{
		//	マスク情報の再設定
		mask = D128;				//	128d = 1000,0000 b

		//	ハフマンバッファがいっぱいなら
		if( pos2 >= HUFFBUFSIZE - 3 * CHAR_BIT8 )
		{
			send_block();
			g_hufsize = 0;
			pos2 = 0;		//	ハフマンバッファの最初の位置に戻す
		}
		pos1 = pos2;			//	LZSSフラグ位置をセット
		pos2++;				//	データ格納位置をフラグ分ずらす
		g_hufbuf[ pos1 ] = 0;		//	フラグを０で初期化
	}

	g_hufbuf[ pos2++ ] = (unsigned char)c;	//	入力文字（長さ）
	h_char->freq[ c ]++;			//	未圧縮文字及び一致長のカウント
	g_hufsize++;

	//	LZSSで圧縮した文字列なら
	if( c >= D256 )
	{
		g_hufbuf[ pos1 ] |= mask;				//	マスクのセット
		g_hufbuf[ pos2++ ] = (unsigned char)(p >> CHAR_BIT8);	//	Pos High Bit
		g_hufbuf[ pos2++ ] = (unsigned char)p;			//	Pos Low Bit

		//	位置情報の圧縮情報の作成
		for( c = 0 ; p ; c++ )
			p >>= 1;

		h_ptr->freq[ c ]++;		//	huffで位置情報の省略をするためのカウントをする
	}
}

//**************************************************************************
//	ハッシュ法と線形リストを用いたLZSS式圧縮 Copyright (C) 1997 M.Furuichi
//**************************************************************************

static int fblockread(uchar *p, int n, FILE *f)
{
	n = fread( p, 1, n, f);
	return n;
}

static void allocate_memory(void)
{
	g_cnt	= (uint*)	malloc( HASHSIZE * sizeof(uint) );
	g_text	= (uchar*)	malloc( ( DICSIZ * 2 + F ) * sizeof(uchar) );
	g_index	= (int*)	malloc( HASHSIZE * sizeof(int) );
	g_next	= (int*)	malloc( DICSIZ * sizeof(int) );
	if( g_next == NULL )
	{
		printf("Out of memory.\n");
		exit(0);
	}
}

static void init( void )
{
	register uint i;
	allocate_memory();
	for( i = 0 ; i < HASHSIZE ; i++ )
	{
		g_cnt[ i ] = 0;
		g_index[ i ] = NIL;
	}
	for( i = 0 ; i < DICSIZ ; i++ )	g_next[ i ] = NIL;

	for( i = 0 ; i < 2 * DICSIZ + F ; i++ )
		g_text[ i ] = 0;

	g_matchpos = g_matchlen = 0;
}

static void _insert_( int r )
{
    register int i, k;

    k = HASH( g_text[ r ] , g_text[ r + 1 ] , g_text[ r + 2 ] );
    if( g_cnt[ k ] < DICSIZ )
	g_cnt[ k ]++;

    g_next[ DIC(r) ] = g_index[ k ];
    g_index[ k ] = DIC(r);
}

static void search( int r )
{
    register int i, j, k ,cnt;

    i = HASH( g_text[ r ] , g_text[ r + 1 ] , g_text[ r + 2 ]);
    cnt = g_cnt[ i ];
    i = g_index[ i ];

    for( k = 0 ; k < cnt ; k++ )
    {
	if( k >= breaksize )
	{
	    break;
	}
	j = 0;
	/*	現在のブロック( DICSIZ ～ g_pos )との比較 */
	if( i < DIC(g_pos) )
	{
	    while( j < g_remainder && j < F )
	    {
		if( *(DICSIZ + g_text + i +j ) != *(g_text + r +j ) )
		    break;
		j++;
	    }
	}
	else
	{
	    while( j < g_remainder && j < F )
	    {
		if( *(g_text + i +j ) != *(g_text + r +j ) )
		    break;
		j++;
	    }
	}

	if( j > g_matchlen )
	{
	    g_matchlen = j;
	    g_matchpos = i;
	}

	if( g_matchlen >= F )	break;
	i = g_next[ i ];
    }
}

static void delete( ushort r )
{
    register ushort k;
    r = DIC( r );
    k = HASH( g_text[ r ] , g_text[ r + 1 ] , g_text[ r + 2 ]);
    if ( g_cnt[ k ] > 0 )
	g_cnt[ k ]--;
}

static void insert( void )
{
    register int n ;
    static int cnt = DICSIZ ;

    if( cnt )
	cnt-- ;
    else
	delete( g_pos ) ;

    _insert_( g_pos ) ;		/*	辞書に現在の g_pos の文字列を挿入 */

    g_remainder-- ;

    /*	この処理は LHA の物を真似させていただきました
	バッファがいっぱいなら今の辞書を前へずらして、続きを補充*/
    if( ++g_pos == DICSIZ * 2 )
    {
	memmove( &g_text[0], &g_text[ DICSIZ ], DICSIZ + F );
	n = fblockread( &g_text[ DICSIZ + F ], DICSIZ, infile);
	g_remainder += n;
	g_pos = DICSIZ;
	putc('.', stderr);	/*	バッファの移動毎に出力*/
    }
}

void encode( void )
{
    register int	last_matchlen;
    register int	last_matchpos;

    g_hufbuf = (uchar *)malloc( HUFFBUFSIZE );
    if( g_hufbuf == NULL )
    {
	printf( "Out of memory for g_hufbuf (%d)\n", HUFFBUFSIZE);
	exit(0);
    }

    h_char = huff_create( HUFFBIT, childN );
    huff_init( h_char );

    h_ptr = huff_create( HUFFBIT, parentN );
    huff_init( h_ptr );

    h_head = huff_create( HUFFBIT, headN );
    huff_init( h_head );

    output( 0, 0, 0); /* 初期化 */

    init();

    g_remainder = fblockread( &g_text[ DICSIZ ], DICSIZ + F, infile);

    g_pos = DICSIZ;

    while( g_remainder > 0 )
    {
	/*	一つ前の検索状況を保存 */
	last_matchlen = g_matchlen;
	last_matchpos = g_matchpos;
	
	/*	新たに文字を登録し、検索する */
	insert();
	g_matchpos = g_matchlen = 0;
	search( g_pos );
	if( g_matchlen > g_remainder )
	    g_matchlen = g_remainder;
	
	/*	圧縮／非圧縮の場合分け*/
	if( g_matchlen > last_matchlen || last_matchlen < THRESHOLD )
	{
	    /*	非圧縮*/
	    output( g_text[ g_pos - 1 ], 0, 1);
	}
	else
	{
	    /*	圧縮*/
	    output( last_matchlen + (UCHAR_MAX + 1 - THRESHOLD),
		    (g_pos - last_matchpos - 2) & (DICSIZ - 1), 1);

	    /*	出力した分だけ補充 */
	    while( --last_matchlen > 0 )
	    {
		insert();
	    }

	    /*	現在の位置にある文字列を検索 */
	    g_matchpos = g_matchlen = 0;
	    search( g_pos );
	    if( g_matchlen > g_remainder )
		g_matchlen = g_remainder;
	}
    }
    huf_encode_end();
}

void decode( unsigned long count )
{
    int   i, j, l, pos ;
    uchar text[DICSIZ] ;
    int	  node, c_root, p_root ;

    h_head = huff_create( HUFFBIT, headN   ) ;
    h_char = huff_create( HUFFBIT, childN  ) ;
    h_ptr  = huff_create( HUFFBIT, parentN ) ;
    huff_init( h_head ) ;
    huff_init( h_char ) ;
    huff_init( h_ptr  ) ;

    /*	辞書の初期化 */
    for( i=DICSIZ ; --i>=0 ; )
	text[ i ] = 0 ;

    pos = 0 ;
    while( (l = getbits(8)|(getbits(8) << 8)) )
    {
	huff_input_counts() ;
	c_root = restore_tree( h_char ) ;
	p_root = restore_tree( h_ptr  ) ;

	while( --l>=0 )
	{
	    node = huff_decode( h_char, c_root) ;
	    if( node < D256 )
	    {
		putc( text[pos]=node, outfile ) ;
		pos = DIC(pos+1)  ;
	    }
	    else
	    {
		if ( (i = huff_decode( h_ptr, p_root)) )
		    i = (1U << ( i - 1)) + getbits( i - 1 );
		i = DIC( pos - i - 1 );

		for ( j=node-D256+THRESHOLD ; --j>=0 ; )
		{
		    putc( text[pos]=text[i], outfile ) ;
		    pos = DIC(pos+1) ;
		    i   = DIC( i +1) ;
		}
	    }
	}
    }
    huff_release( h_head );
    huff_release( h_ptr );
    huff_release( h_char );

    printf("%12lu\n", count);
}

int main( int argc, char **argv)
{
    int c;
    unsigned long size;

    if( argc > 1 )
	c = *argv[1] ;
    if( argc != 4 || (
	c != 'E' && c != 'e' &&
	c != 'X' && c != 'x' &&
	c != 'D' && c != 'd'))
    {
	printf(
	    "\n<< LZSH >> Ver.1.00 Copyright (C) 1997 M.Furuichi"
	    "\nUsage : lzsh [e|d] (infile) (outfile)"
	    "\n  e   compress"
	    "\n  x   compress (eXtra)"
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
    if( c == 'E' || c == 'e' || c == 'x' || c == 'X' )
    {
	if( c == 'x' || c == 'X' )
	    breaksize = DICSIZ;

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
