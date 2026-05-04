//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   utl_lzh.c 
   
   2000/01/21 T.Morita
   $Id: utl_lzh.c,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libgv.h"


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
    u_int   bitbuf   ;  /* BIT IO の ビットバッファ */
    short   getcount ;  /* BIT IO の 残りビット数   */
    int     size     ;  /* 解凍サイズ */
    u_char *in       ;  /* 解凍元現在 */
    u_char *in_head  ;  /* 解凍元先頭 */
    u_char *out      ;  /* 解凍先現在 */
    u_char *out_head ;  /* 解凍先先頭 */
    u_char  text[DICSIZ] ;/* LZSS辞書 */

    /* stream時に使用 */
    int     state    ;  /* 開始ステータス */
    int     s_pos    ;  /* 現在の位置 */
    u_short d_pos    ;  /* 辞書の位置 */
    int     r_pos    ;  /* 辞書参照位置   */
    int     r_num    ;  /* 辞書参照数     */
    int     line     ;  /* デコードループ */
    int     c_root   ;  /* 文字圧縮ハフマン木の根   */
    int     p_root   ;  /* 符合長圧縮ハフマン木の根 */
} LZSH ;


// =========================================================================
//	bitio
// =========================================================================

#define rightbits(n,x) ((x)&((1U<<(n))-1U))

static unsigned getbit( LZSH *lz )
{
    if( --lz->getcount >= 0 )
	return (lz->bitbuf >> lz->getcount) & 1U ;

    lz->getcount = 7 ;
    lz->bitbuf = *lz->in++ ;
    return ( lz->bitbuf >> 7 ) & 1U ;
}

static u_int getbits( int n, LZSH *lz )
{
    u_int x = 0 ;

    while( n > lz->getcount )
    {
	n -= lz->getcount ;
	x |= rightbits( lz->getcount, lz->bitbuf) << n ;
	lz->bitbuf = *lz->in++ ;
	lz->getcount = 8 ;
    }
    lz->getcount -= n ;
    return x | rightbits( n, lz->bitbuf >> lz->getcount) ;
}


// =========================================================================
//	δ符号を扱うための関数
// =========================================================================


/*	一致列長 読込み */

static int get_gamma( LZSH *lz )
{
    int i=0, code=1 ;

    while( !getbit( lz ) )
	i++ ;
    while( --i>=0 )
	code = (code << 1) + getbit( lz ) ;

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
	//bzero( h, size ) ;
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

static int huff_decode( HUFF* h, int root, LZSH *lz )
{
    int node;

    node = root; /* 木の幹*/
    do
	node = getbit( lz ) ? h->node[node].child_1 : h->node[node].child_0 ;
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

    return h->tableno ;
}

/*	圧縮された符号長を復元し符号語を復元する -------------------------------*/

static int huff_input_counts( LZSH *lz )
{
    int     i, j ;
    int     pc, root, flag, len ;
    u_short xorbuf[childN] ;
    u_int   len_16cnt[17] ;

    pc = 0 ;
    for( i = 0 ; i < lz->h_head->tableno ; i++ )
    {
	if( getbit( lz ) )
	    lz->h_head->code[ i ].code_bits = get_gamma( lz ) ^ pc ;
	else
	    lz->h_head->code[ i ].code_bits = pc ;
	pc = lz->h_head->code[ i ].code_bits ;
    }
    for( i = 0 ; i <= 16 ; i++ )
	len_16cnt[ i ] = 0 ;
    for( i = 0 ; i < lz->h_head->tableno ; i++ )
    {
	if( lz->h_head->code[ i ].code_bits > 16 )
	    return -1 ;
	len_16cnt[lz->h_head->code[i].code_bits]++ ;
    }
    huff_make_code( lz->h_head, len_16cnt ) ;
    root = restore_tree( lz->h_head ) ;

    for( i = 0 ; i < lz->h_char->tableno ; i++ )
	xorbuf[ i ] = 0 ;

    flag = getbit( lz ) ;
    for( i = 0 ; i < lz->h_char->tableno ; i+=len )
    {
	len = get_gamma( lz ) ;
	if( flag )
	    for( j = 0; j < len ; j++ )
		xorbuf[ i + j ] = huff_decode( lz->h_head, root, lz ) ;
	else
	    for( j = 0; j < len ; j++ )
		xorbuf[ i + j ] = 0 ;
	flag ^= 1 ;
    }

    for( i=0 ; i <= 16 ; i++ )
	len_16cnt[i] = 0 ;
    pc = 0 ;
    for( i = 0 ; i < lz->h_char->tableno ; i++ )
    {
	lz->h_char->code[ i ].code_bits = xorbuf[ i ] ^ pc ;
	if ( (pc = lz->h_char->code[ i ].code_bits) > 16 )
	    return -1 ;
	len_16cnt[pc]++ ;
    }
    huff_make_code( lz->h_char, len_16cnt ) ;

    for( i = 0 ; i <= 16 ; i++ )
	len_16cnt[i] = 0;
    pc = 0 ;
    for( i=0 ; i < lz->h_ptr->tableno ; i++ )
    {
	if( getbit( lz ) )
	    lz->h_ptr->code[i].code_bits = get_gamma( lz ) ^ pc ;
	else
	    lz->h_ptr->code[i].code_bits = pc ;
	if ( (pc = lz->h_ptr->code[i].code_bits) > 16 )
	    return -1 ;
	len_16cnt[pc]++ ;
    }
    huff_make_code( lz->h_ptr, len_16cnt ) ;

    return 0 ;
}

/*

  ここまでが 解凍用 静的関数

*/



void UTL_LzshDecodeRewindOut( LZSH *l )
{
    l->out = l->out_head ;
}
void UTL_LzshDecodeRewindIn( LZSH *l )
{
    int i ;

    l->bitbuf = l->getcount = 0 ;
    l->d_pos  = l->s_pos    = 0 ;
    l->state  = 0 ;
    for( i=DICSIZ ; --i>=0 ; )
	l->text[i] = 0 ;
    l->in = l->in_head ;
}
void UTL_LzshDecodeRewind( LZSH *l )
{
    UTL_LzshDecodeRewindIn ( l ) ;
    UTL_LzshDecodeRewindOut( l ) ;
}
void UTL_LzshDecodeChangeIn( LZSH *l, u_char *in )
{
#ifdef DEBUG
    ASSERT( in != NULL ) ;
#endif
    l->in_head = in+4 ;
    UTL_LzshDecodeRewindIn( l ) ;
}

void UTL_LzshDecodeFree( LZSH *l )
{
    if ( l )
    {
	huff_release( l->h_head ) ;
	huff_release( l->h_ptr  ) ;
	huff_release( l->h_char ) ;
	GV_Free( l ) ;
    }
}

LZSH *UTL_LzshDecodeInit( u_char *in, u_char *out, int size )
{
    LZSH *l ;

#ifdef DEBUG
    ASSERT( in != NULL ) ;
#endif
    if ( (l = GV_Malloc( sizeof(LZSH) )) )
    {
#if 1 //BP
	memset( (void*)l, 0, sizeof(LZSH) ) ;
#else
	bzero( (void*)l, sizeof(LZSH) ) ;  /* 位置,辞書,bitioを初期化 */
#endif
	l->h_head = huff_create( HUFFBIT, headN   ) ;
	l->h_char = huff_create( HUFFBIT, childN  ) ;
	l->h_ptr  = huff_create( HUFFBIT, parentN ) ;
	if ( !l->h_head || !l->h_char || !l->h_ptr )
	    UTL_LzshDecodeFree( l ), l = NULL ;
	else
	{
	    l->size     = size ? size : *(int*)in ;
	    l->in_head  = l->in  = in+4 ; /* サイズが入っているのを飛ばす */
	    l->out_head = l->out = out  ;

	    huff_init( l->h_head ) ;
	    huff_init( l->h_char ) ;
	    huff_init( l->h_ptr  ) ;
	}
    }
    return l ;
}

#pragma optimize( "", off )

int UTL_LzshDecode( u_char *in, u_char *out )
{
    int i, j, l, pos ;
    int  node, c_root, p_root ;
    LZSH *lz ;

#ifdef DEBUG
    ASSERT( in != NULL ) ;
#endif
    if ( (lz = UTL_LzshDecodeInit( in, out, 0 )) )
    {
	pos = 0 ;
	while( (l = getbits( 8, lz )| (getbits( 8, lz ) << 8)) )
	{
	    if ( huff_input_counts( lz ) )
		return -1 ;
	    c_root = restore_tree( lz->h_char ) ;
	    p_root = restore_tree( lz->h_ptr  ) ;

	    while( --l>=0 )
	    {
		node = huff_decode( lz->h_char, c_root, lz ) ;
		if( node < D256 )
		{
		    lz->text[pos] = node ;
		    *lz->out++    = node ;
		    pos = DIC(pos+1) ;
		}
		else
		{
		    if ( (i = huff_decode( lz->h_ptr, p_root, lz )) )
			i = (1U << (i-1)) + getbits( i-1, lz ) ;
		    i = DIC(pos - i - 1) ;
		    for ( j=node-D256+THRESHOLD ; --j>=0 ; )
		    {
			*lz->out++    = lz->text[i] ;
			lz->text[pos] = lz->text[i] ;
			pos = DIC(pos+1) ;
			i   = DIC(i  +1) ;
		    }
		}
	    }
	}
	UTL_LzshDecodeFree( lz ) ;
    }

    return 0 ;
}


int UTL_LzshStreamDecode( LZSH *lz )
{
    int i, node ;

    lz->s_pos = 0 ;
    switch ( lz->state )
    {
    default:
	while( (lz->line = getbits( 8, lz )| (getbits( 8, lz ) << 8)) )
	{
	    if ( huff_input_counts( lz ) )
		return -1 ;
	    lz->c_root = restore_tree( lz->h_char ) ;
	    lz->p_root = restore_tree( lz->h_ptr  ) ;

	    while( --(lz->line) >= 0 )
	    {
		node = huff_decode( lz->h_char, lz->c_root, lz ) ;
		if( node < D256 )
		{
		    *lz->out++          = node ;
		    lz->text[lz->d_pos] = node ;
		    lz->d_pos = DIC(lz->d_pos+1) ;
		    if ( ++lz->s_pos >= lz->size && lz->line>0 )
			return (lz->state = 1) ;
		case 1:
			;
		}
		else
		{
		    if ( (i = huff_decode( lz->h_ptr, lz->p_root, lz )) )
			i = (1U << (i-1)) + getbits( i-1, lz ) ;
		    lz->r_pos = DIC(lz->d_pos - i - 1) ;
		    for ( lz->r_num=node-D256+THRESHOLD ; --(lz->r_num)>=0 ; )
		    {
			*lz->out++          = lz->text[lz->r_pos] ;
		        lz->text[lz->d_pos] = lz->text[lz->r_pos] ;
			lz->d_pos = DIC(lz->d_pos+1) ;
			lz->r_pos = DIC(lz->r_pos+1) ;
			if ( ++lz->s_pos >= lz->size && lz->line>0 )
			    return (lz->state = 2) ;
		    case 2:
			;
		    }
		}
	    }
	}
    }
    return (lz->state = 0) ;
}
#pragma optimize( "", on )
