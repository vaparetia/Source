/*
   lzh.c 
   
   2000/01/21 T.Morita
   $Id: decode.c,v 1.1 2000/03/24 05:23:32 usr04098 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#define GV_Malloc malloc
#define GV_Free   free


//	LZSS 関連 --------------------------------------------------------------

#define THRESHOLD	3			/* しきい値         */

#define	F		( 256 + 2 )		/* 最大一致文字数   */
#define DICBIT		14			/* 辞書ビット数     */
#define DICSIZ		(1U << DICBIT)		/* 辞書のサイズ     */
#define NIL		DICSIZ			/* 未登録記号       */

#define	DIC(_s)		((_s) & (DICSIZ - 1))	/* 辞書のサイズに制限するマクロ */

//	huffman 関連 -----------------------------------------------------------

#define	CHAR_BIT8		8
#define	D256			256

//	ハフマン符号のテーブルサイズ
#define	childN			(F -THRESHOLD +1 + D256)
#define	parentN			16
#define headN			16

#define	HUFFBIT		8
#define HUFNIL		(-1)



/* 構造体の定義 */

typedef struct node_t
{
    u_int count   ;
    int   up      ;
    int   child_0 ;
    int   child_1 ;
} NODE ;

typedef struct code_t
{
    u_int code      ;  /* ハフマン記号データ */
    int   code_bits ;  /* 有効なビット数     */
} CODE ;

typedef struct huff_t
{
    int	   bitno   ;
    int	   tableno ;
    u_int *freq    ;
    CODE  *code    ;
    NODE  *node    ;
} HUFF ;

typedef struct handler_t
{
    HUFF   *h_ptr    ;	/* ポインタ長圧縮の構造体       */
    HUFF   *h_char   ;	/* 一致長／一致文字圧縮の構造体 */
    HUFF   *h_head   ;	/* 符号長テーブルの圧縮用       */
    u_int   bitbuf   ;
    int     getcount ;
    u_char *in       ;
    u_char  text[DICSIZ] ;/* 辞書 */
} HANDLER ;


static HUFF *h_ptr  ;	/* ポインタ長圧縮の構造体       */
static HUFF *h_char ;	/* 一致長／一致文字圧縮の構造体 */
static HUFF *h_head ;	/* 符号長テーブルの圧縮用       */



// =========================================================================
//	bitio
// =========================================================================

#define rightbits(n,x) ((x)&((1U<<(n))-1U))

static unsigned bitbuf ;
static int getcount ;

static unsigned getbit( u_char **in )
{
    if( --getcount >= 0 )
	return (bitbuf >> getcount) & 1U ;
    getcount = 7 ;
    bitbuf = *(*in)++ ;

    return ( bitbuf >> 7 ) & 1U ;
}

static u_int getbits( int n, u_char **in )
{
    u_int x = 0 ;

    while( n > getcount )
    {
	n -= getcount ;
	x |= rightbits( getcount, bitbuf) << n ;
	bitbuf = *(*in)++ ;
	getcount = 8 ;
    }
    getcount -= n ;

    return x | rightbits( n, bitbuf >> getcount) ;
}


// =========================================================================
//	δ符号を扱うための関数
// =========================================================================


/*	一致列長 読込み */

static int get_gamma( u_char **in )
{
    int i=0, code=1 ;

    while( !getbit( in ) )
	i++ ;
    while( --i>=0 )
	code = (code << 1) + getbit( in ) ;

    return code ;
}

// =========================================================================
//	huffman encoding functions
// =========================================================================


/*	huffman 法用のメモリの確保 */

static HUFF *huff_create( int bitno, int tableno )
{
    HUFF *h    ;
    int   size ;

    size = sizeof(HUFF) + tableno * ( 2 * sizeof(NODE) + sizeof(CODE) + sizeof(u_int) ) ;

    /*	構造体用のメモリを確保 */
    if( ( h = (HUFF*)GV_Malloc( size ) ) )
    {
	bzero( h, size ) ;
	h->bitno   = bitno   ;
	h->tableno = tableno ;

	h->node = (NODE  *)(h + 1) ;
	h->code = (CODE  *)(h->node +  tableno * 2) ;
	h->freq = (u_int *)(h->code +  tableno    ) ;
    }

    return h ;
}

/*	huffman 法で使用したメモリの解放 */

static void huff_release( HUFF *h )
{
    if ( h )
	GV_Free( h ) ;
}

/*	huffman 法で使用するデータ域の初期化 */

static void huff_init( HUFF *h )
{
    int	i ;

    for( i=h->tableno   ; --i>=0 ; )
    {
	h->freq[i] = 0 ;
	h->code[i].code_bits = h->code[i].code = 0 ;
    }
    for( i=h->tableno*2 ; --i>=0 ; )
    {
	h->node[i].count = 0 ;
	h->node[i].child_0 = h->node[i].child_1 = h->node[i].up = HUFNIL ;
    }
}


/*	符号長からハフマン符号を生成 */

static void huff_make_code( HUFF *h, u_int *len_16cnt )
{
    int	 i, j ;
    u_int start[18] ;

    start[0] = 0;
    start[1] = 0;
    for( i=1 ; i<=16 ; i++ )
	start[i+1] = ( start[i] + len_16cnt[i] ) << 1 ;

    for( i = 0 ; i < 18 ; i++ )
	for( j = 0 ; j < h->tableno ; j++ )
	    if( h->code[j].code_bits == i )
		h->code[j].code = start[i]++ ;
}


/*	符号語を読込んで文字に復元 */

static int huff_decode( HUFF* h, int root, u_char **in )
{
    int node;

    node = root; /* 木の幹*/
    do
	node = getbit( in ) ? h->node[node].child_1 : h->node[node].child_0 ;
    while( node >= h->tableno ) ;

    return node ;
}


/*	符号語から復号用のハフマン木をつくる */

static int restore_tree( HUFF *h )
{
    int i, j ;
    int root_pos, next_free ;

    root_pos  = h->tableno     ;
    next_free = h->tableno + 1 ;
    h->node[root_pos ].child_0 = HUFNIL ;
    h->node[root_pos ].child_1 = HUFNIL ;
    h->node[next_free].child_0 = HUFNIL ;
    h->node[next_free].child_1 = HUFNIL ;

    for( i=0 ; i<h->tableno ; i++ )
    {
	root_pos = h->tableno ;
	for( j=0 ; j<h->code[i].code_bits ; j++ )
	{
	    if ( h->code[i].code & (1 << (h->code[i].code_bits - j - 1) ) )
	    {
		if ( j+1 == h->code[i].code_bits )
		{
		    h->node[ root_pos ].child_1 = i ;
		    break ;
		}

		if ( h->node[root_pos].child_1 != HUFNIL )
		    root_pos = h->node[root_pos].child_1 ;
		else
		{
		    h->node[root_pos ].child_1 = next_free ;
		    h->node[next_free].child_0 = HUFNIL ;
		    h->node[next_free].child_1 = HUFNIL ;
		    root_pos = next_free++ ;
		}
	    }
	    else
	    {
		if ( j+1 == h->code[i].code_bits )
		{
		    h->node[root_pos].child_0 = i ;
		    break ;
		}

		if ( h->node[root_pos].child_0 != HUFNIL )
		    root_pos = h->node[root_pos].child_0 ;
		else
		{
		    h->node[root_pos ].child_0 = next_free ;
		    h->node[next_free].child_0 = HUFNIL ;
		    h->node[next_free].child_1 = HUFNIL ;
		    root_pos = next_free++ ;
		}
	    }
	}
    }

    return h->tableno ;
}

/*	圧縮された符号長を復元し符号語を復元する -------------------------------*/

static int huff_input_counts( u_char **in )
{
    int     i, j ;
    int     pc, root, flag, len ;
    u_short xorbuf[childN] ;
    u_int   len_16cnt[17] ;

    pc = 0 ;
    for( i = 0 ; i < h_head->tableno ; i++ )
    {
	if( getbit( in ) )
	    h_head->code[ i ].code_bits = get_gamma( in ) ^ pc ;
	else
	    h_head->code[ i ].code_bits = pc ;
	pc = h_head->code[ i ].code_bits ;
    }
    for( i = 0 ; i <= 16 ; i++ )
	len_16cnt[ i ] = 0 ;
    for( i = 0 ; i < h_head->tableno ; i++ )
    {
	if( h_head->code[ i ].code_bits > 16 )
	    return -1 ;
	len_16cnt[h_head->code[i].code_bits]++ ;
    }
    huff_make_code( h_head, len_16cnt ) ;
    root = restore_tree( h_head ) ;

    for( i = 0 ; i < h_char->tableno ; i++ )
	xorbuf[ i ] = 0 ;

    flag = getbit( in ) ;
    for( i=0 ; i < h_char->tableno ; i+=len )
    {
	len = get_gamma( in ) ;
	if( flag )
	    for( j = 0; j < len ; j++ )
		xorbuf[ i + j ] = huff_decode( h_head, root, in ) ;
	else
	    for( j = 0; j < len ; j++ )
		xorbuf[ i + j ] = 0 ;
	flag ^= 1 ;
    }

    pc = 0 ;
    for( i = 0 ; i < h_char->tableno ; i++ )
    {
	h_char->code[ i ].code_bits = xorbuf[ i ] ^ pc ;
	pc = h_char->code[ i ].code_bits ;
    }
    for( i=0 ; i <= 16 ; i++ )
	len_16cnt[i] = 0 ;

    for( i=0 ; i < h_char->tableno ; i++ )
    {
	if( h_char->code[ i ].code_bits > 16 )
	    return -1 ;
	len_16cnt[ h_char->code[ i ].code_bits ]++ ;
    }
    huff_make_code( h_char, len_16cnt ) ;

    pc = 0 ;
    for( i=0 ; i < h_ptr->tableno ; i++ )
    {
	if( getbit( in ) )
	    h_ptr->code[i].code_bits = get_gamma( in ) ^ pc ;
	else
	    h_ptr->code[i].code_bits = pc ;
	pc = h_ptr->code[i].code_bits ;
    }

    for( i = 0 ; i <= 16 ; i++ )
	len_16cnt[i] = 0;

    for( i = 0 ; i < h_ptr->tableno ; i++ )
    {
	if( h_ptr->code[ i ].code_bits > 16 )
	    return -1 ;
	len_16cnt[ h_ptr->code[ i ].code_bits ]++ ;
    }
    huff_make_code( h_ptr, len_16cnt ) ;

    return 0 ;
}





int UTL_LzhDecode( u_char *in, u_char *out )
{
    int i, j, pos ;
    static u_char text[DICSIZ] ;/*スタックが心配なので*/
    int  l ;
    int  node, c_root, p_root ;

    //in += 4 ; /* サイズが入っているのを飛ばす */

    bitbuf = getcount = 0 ;

    h_head = huff_create( HUFFBIT, headN   ) ;
    h_char = huff_create( HUFFBIT, childN  ) ;
    h_ptr  = huff_create( HUFFBIT, parentN ) ;
    if ( h_head && h_char && h_ptr )
    {
	huff_init( h_head );
	huff_init( h_char );
	huff_init( h_ptr );

	/* Initialize dictionary */
	for( i=DICSIZ ; --i>=0 ; )
	    text[i] = 0 ;

	pos = 0 ;
	while( (l = (getbits( 8, &in ) | (getbits( 8, &in ) ) << 8) ) )
	{
	    if ( huff_input_counts( &in ) )
		return -1 ;
	    c_root = restore_tree( h_char ) ;
	    p_root = restore_tree( h_ptr  ) ;

	    while( --l>=0 )
	    {
		node = huff_decode( h_char, c_root, &in ) ;
		if( node < D256 )
		{
		    *out++ = text[pos] = node ;
		    pos = DIC(pos+1) ;
		}
		else
		{
		    if ( (i = huff_decode( h_ptr, p_root, &in )) )
			i = (1U << (i-1)) + getbits( i-1, &in ) ;
		    i = DIC(pos - i - 1) ;
		    for ( j=node-D256+THRESHOLD ; --j>=0 ; )
		    {
			*out++ = text[pos] = text[i] ;
			pos = DIC(pos+1) ;
			i   = DIC(i  +1) ;
		    }
		}
	    }
	}
    }
    huff_release( h_head ) ;
    huff_release( h_ptr  ) ;
    huff_release( h_char ) ;

    printf( "END out%x\n", out ) ;

    return 0 ;
}


int main( int argc, char **argv)
{
    uint size;
    char *in, *out, *c ;
    FILE *infile ;

    if ( argc != 3 )
    {
	printf( "\n<< LZSH >>\nUsage : lzsh (infile) (outfile)\n" ) ;
	exit( 0 ) ;
    }
    if ( (infile = fopen( argv[1], "rb")) == NULL )
    {
	printf( "Can't open infile %s", argv[1] ) ;
	exit( 0 ) ;
    }

    {
	int i, j ;

	fread( &size, sizeof size, 1, infile ) ;
	fprintf( stderr, "%d\n", size ) ;
	in  = malloc( size ) ;
	out = malloc( size ) ;
	c   = in ;
	while( (*c++ = fgetc( infile )) != EOF ) ;
	fclose( infile ) ;

	for ( i=100 ; --i>=0 ; )
	{
	    printf( "out%x in%x end%x\n", out, in, out+size ) ;
	    UTL_LzhDecode( (u_char*)in, (u_char*)out ) ;
#if 1
	    for ( j=16*8, c=out ;--j>=0 ; c++ )
		printf( (j%16 ? j%4 ? "%02x" : "%02x " : "%02x\n"), (u_char)*c ) ;
#endif
	}

    }

    return EXIT_SUCCESS ;
}
