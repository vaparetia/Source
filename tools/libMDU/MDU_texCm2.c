/*
   MDU_texCm2.c

   テクスチャＣＭ２関連関数

   by M.Sonoyama 1999 Aug.～
   $Id: MDU_texCm2.c,v 1.6 2002/08/26 11:22:09 usr01363 Exp $

   Konami Computer Entertainment Japan West   
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<sys/types.h>

#include	"MDU_util.h"
#include	"MDU_tex.h"

/*-------------------------------------------------------------------------*/

/* テクスチャ群を、2^n*2^m範囲に配置 */

#define MAX_PAGES	(16)
#define MAX_CLUTS       (256)
#define N_PAGES		(32767)
#define N_CLUTS		(32767)
#define YET             (0)
#define SETTLED         (1)

typedef	struct {
    int		x ;
    int		z ;
    int		w ;
    int		h ;
} RECT2 ;

typedef	struct {
    int		mode ;
    int		n_pages ;
    RECT2	r_p[ MAX_PAGES ] ;
    int		n_cluts ;
    RECT2        r_c[ MAX_CLUTS ] ;
} ELEM ;

static 	RECT2	Pages[ N_PAGES ] ;
static	RECT2	Cluts[ N_CLUTS ] ;
static	int	Mode ;
static	int	n_pages ;
static	int	n_cluts ;
static	int	M_JOU, WIDTH ;
static	int	N_JOU, HEIGHT ;

static	ELEM	Use ;

/*	テクスチャページの設定	*/
static	void	MakePages( texs, n_texs )
MDU_Tex		**texs ;
int		n_texs ;
{
    int		i, w, n, h ;

    w = 0 ; h = 0 ;
    for ( i = 0; i < n_texs; i ++ ) {
	if ( w < texs[ i ]->data.w ) w = texs[ i ]->data.w ;
	if ( h < texs[ i ]->data.h ) h = texs[ i ]->data.h ;
    }
    w = ( Mode == MDU_TEX_FLAG_COLOR16 ) ? w * 2 : w ;
    
    n = 0 ;
    while( 1 ) {
	w /= 2 ;
	if ( w == 0 ) break ;
	n ++ ;
    }
    if ( n < 7 ) n = 7 ;
    N_JOU = n ;
    
    w = 1 ; 
    while( -- n >= 0 ) {
	w *= 2 ;
    }
    WIDTH = w ;
    w = ( Mode == MDU_TEX_FLAG_COLOR16 ) ? w / 2 : w ;
    
    n = 0 ;
    while( 1 ) {
	h /= 2 ;
	if ( h == 0 ) break ;
	n ++ ;
    }
    if ( n < 7 ) n = 7 ;
    M_JOU = n ;
    
    h = 1 ; 
    while( -- n >= 0 ) {
	h *= 2 ;
    }
    HEIGHT = h ;
    
    for ( i = 0; i < N_PAGES; i ++ ) {
	Pages[ i ].x = -1 ;
	Pages[ i ].z = -1 ;
	Pages[ i ].w = -1 ;
	Pages[ i ].h = -1 ;
    }
    Use.n_pages = 1 ;
    n_pages = 1 ;
    Pages[ 0 ].x = Use.r_p[ 0 ].x = 0 ;
    Pages[ 0 ].z = Use.r_p[ 0 ].z = 0 ;
    Pages[ 0 ].w = Use.r_p[ 0 ].w = w ;
    Pages[ 0 ].h = Use.r_p[ 0 ].h = h ;
/*    fprintf( stderr, "Page %d %d\n", WIDTH, HEIGHT ) ; */
}

/*	最適ページの選択	*/
static	int	SelectBestPage( texs, n_texs, page, chk )
MDU_Tex		**texs ;
int		n_texs ;
RECT2		page ;
char		*chk ;
{
    int		best_w, best_h ;
    int		best_tex ;
    int		i ;
    int		w, h ;
    
    best_tex = -1 ;
    best_w = 0 ;
    best_h = 0 ;
    
    for ( i = 0; i < n_texs; i ++ ) {
	/* もし、すでに設定されていたら、とばす */
	if ( chk[ i ] == SETTLED ) {
	    continue ;
	}	
	w = texs[ i ]->data.w ;
	h = texs[ i ]->data.h ;
	/* 残り領域に入るテクスチャの中で、一番でかいのを選択 */
	if ( w <= page.w && h <= page.h ) {
	    /* 一番でかいのを選択 */
#if 0
	    if ( w > best_w ) {
		best_tex = i ;
		best_w = w ;
		best_h = h ;
	    } else if ( w == best_w ) {
		if ( h > best_h ) {
		    best_tex = i ;
		    best_h = h ;
		}
	    }
#else
	    if ( h > best_h ) {
		best_tex = i ;
		best_w = w ;
		best_h = h ;
	    } else if ( h == best_h ) {
		if ( w > best_w ) {
		    best_tex = i ;
		    best_w = w ;
		}
	    }
#endif
	} 
    }
    return best_tex ;
}

/*  VRAM 領域をテクスチャに割り当て、新たな空きページを作成  */
static	void	UsePage( texs, page, best_tex )
MDU_Tex		**texs ;
RECT2		page ;
int		best_tex ;
{
    int 	w, h ;
    RECT2	newpage[ 2 ] ;
    int		n = 0 ;
    int		i ;
    
    /* 割り当て結果を設定 */
    texs[ best_tex ]->data.dx = page.x ;
    texs[ best_tex ]->data.dy = page.z ;
    
    /* 新たなページを作成 */
    w = texs[ best_tex ]->data.w ;
    h = texs[ best_tex ]->data.h ; 
    
    if ( w < page.w ) {
	newpage[ n ].x = page.x + w ;
	newpage[ n ].z = page.z ;
	newpage[ n ].w = page.w - w ;
	newpage[ n ].h = h ;
	n ++ ;
    }
    if ( h < page.h ) {
	newpage[ n ].x = page.x ;
	newpage[ n ].z = page.z + h ;
	newpage[ n ].w = page.w ;
	newpage[ n ].h = page.h - h ;
	n ++ ;
    }
    
    /* 残りページと同じ大きさのテクスチャを割り当てた場合 */
    if ( n == 0 ) {
	for ( i = 0; i < n_pages; i ++ ) {
	    Pages[ i ] = Pages[ i + 1 ] ;
	}
	n_pages --;
	
	/* 残りページと高さが同じテクスチャを割り当てた場合 */
    } else if ( n == 1 ) {
	Pages[ 0 ] = newpage[ 0 ] ;
	
	/* それ以外の場合 */
    } else if ( n == 2 ) {
	for ( i = 0; i < n_pages - 1; i ++ ) {
	    Pages[ n_pages - i ] = Pages[ n_pages - i - 1 ] ;
	}
	n_pages ++ ;
	
	Pages[ 0 ] = newpage[ 0 ] ;
	Pages[ 1 ] = newpage[ 1 ] ;
    }
}

/*	テクスチャページの割り当て	*/
static	int	AllocPages( texs, n_texs )
MDU_Tex		**texs ;
int		n_texs ;
{
    int		i, j, c ;
    RECT2	page ;
    int		best_tex ;
    char	*chk ;
    
    /* 領域を割り当てたかどうかチェック用 */
    chk = ( char * )MDU_Alloc( n_texs ) ;
    if ( chk == NULL ) return 0 ;
    for ( i = 0; i < n_texs; i ++ ) {
	chk[ i ] = YET ;
    }
    
    /* 領域割り当て */
    c = n_texs ;
    while ( c > 0 ) {
	int	px, pz, pw, ph ;
	page = Pages[ 0 ] ;
	px = page.x ;
	pz = page.z ;
	pw = page.w ;
	ph = page.h ;
	
	/* 領域にはいるもののうちで一番でかいテクスチャを選択 */
	best_tex = SelectBestPage( texs, n_texs, page, chk ) ;
	
	/* 最適なテクスチャが発見された場合 */
	if ( best_tex != -1 ) {
	    /* 領域を割り当て、新たなページ作成 */ 
	    UsePage( texs, page, best_tex ) ;
	    chk[ best_tex ] = SETTLED ;
	    c -- ;
	    /* 割り当て可能なテクスチャがない場合 */
	} else {
	    /* 次のページを割り当て対象に */
	    for ( j = 0; j < n_pages; j ++ ) {
		Pages[ j ] = Pages[ j + 1 ] ;
	    }
	    n_pages -- ;
	}
	if ( n_pages == 0 ) {
	    for ( i = 0; i < n_texs; i ++ ) {
		chk[ i ] = YET ;
	    }
	    c = n_texs ;
#if 1
	    if ( HEIGHT < 1024 ) {
		M_JOU ++ ;
		HEIGHT *= 2 ;
	    } else {
		N_JOU ++ ;
		WIDTH *= 2 ;
	    }
#else
	    if ( WIDTH < 1024 ) {
		N_JOU ++ ;
		WIDTH *= 2 ;
		
	    } else {
		M_JOU ++ ;
		HEIGHT *= 2 ;
	    }
#endif
	    for ( i = 0; i < N_PAGES; i ++ ) {
		Pages[ i ].x = -1 ;
		Pages[ i ].z = -1 ;
		Pages[ i ].w = -1 ;
		Pages[ i ].h = -1 ;
	    }
	    Use.n_pages = 1 ;
	    n_pages = 1 ;
	    Pages[ 0 ].x = Use.r_p[ 0 ].x = 0 ;
	    Pages[ 0 ].z = Use.r_p[ 0 ].z = 0 ;
	    Pages[ 0 ].w = Use.r_p[ 0 ].w = ( Mode == 0 ) ? WIDTH / 2 : WIDTH ;
	    Pages[ 0 ].h = Use.r_p[ 0 ].h = HEIGHT ;
/*	    fprintf( stderr, "NewPage %d %d\n", WIDTH, HEIGHT ) ; */
	    if ( WIDTH > 1024 || HEIGHT > 1024 ) {
		fprintf( stderr, "cannot allocate texture.\n" ) ;
		return 0 ;
	    }
	}
    }
    MDU_Free( chk ) ;
    return 1 ;
}

void	MDU_ReplaceTexCm2( texs, n_texs )
MDU_Tex		**texs ;
int		n_texs ;
{
    int		c, i, mode ;
    
    /* 違うモードが混在していないかチェック */
    mode = texs[ 0 ]->flag & MDU_TEX_FLAG_COLOR256 ;
    for ( i = 0; i < n_texs; i ++ ) {
	if ( mode != ( texs[ i ]->flag & MDU_TEX_FLAG_COLOR256 ) ) {
	    fprintf( stderr, "another mode texture exists." ) ;
	    return ;
	}
    }
    Mode = mode ;
    
    /* VRAM 割り当て処理 */
    MakePages( texs, n_texs ) ;
    c = AllocPages( texs, n_texs ) ;
    if ( c == 0 ) return ;
    /* クラット配置はまた今度 */
    for ( i = 0; i < n_texs; i ++ ) {
	texs[ i ]->clut.dx = 0 ;
	texs[ i ]->clut.dy = i ;
    }
}

/*---------------------------------------------------------------------------*/

/* CM2ファイルでセーブ */

/* セーブは、MDU_ReplaceTexCm2を実行した直後に
   行うこと */

/*
   .cm2 ファイルフォーマット
   
   long		magic ;			マジックナンバー
   long		width, height ;		幅,高さ
   long		dx, dy ;		VRAM位置
   long		n_textures ;		テクスチャ数
   long		mode ;			１６色 = 0 ; ２５６色 = 1 ;
   TexData	*texdatas ;		テクスチャデータ列へのポインタ
   void		*pixdata ;		ピクセルデータ列へのポインタ
   
   ----------------------------------------
   
   [ TexDatas ]
   [ PixDatas ]
   [ ClutDatas ]
*/

typedef struct _TexData {
    long		strcode ;		/* テクスチャID */
    long		flag ;			/* フラグ */
    long		n_colors ;		/* 使用色数	*/
    short		width, height ;		/* 幅,高さ */
    short		pix_x, pix_y ;		/* ピクセルデータ位置 */
    short		clut_x, clut_y ;	/* クラットデータ位置 */
    long		*clut_addr ;		/* クラットデータへのポインタ */
} TexData ;

typedef	struct _Cm2Header {
    long		magic ;			
    long		width, height ;		
    long		dx, dy ;	
    long		n_textures ;		
    long		mode ;			
    TexData		*texdatas ;
    void		*pixdata ;
} Cm2Header ;

static	long	texdata_addr ;
static	long	pixdata_addr ;
static	long	clutdata_addr ;

static	void	WriteCS2Header( fp, texs, n_texs )
FILE		*fp ;
MDU_Tex		**texs ;
int		n_texs ;
{
    long		magic, width, height, n_textures, mode ;
    long		dx, dy ;
    
    magic = 19990907 ;	/* 作った日だったりして */
    width = WIDTH ; height = HEIGHT ;
    mode = texs[ 0 ]->flag & MDU_TEX_FLAG_COLOR256 ;
    dx = dy = 0 ;
    n_textures = n_texs ;
    MDU_FwriteEndianLong( fp, &magic, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &width, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &height, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &dx, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &dy, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &n_textures, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &mode, 1, 1 ) ;
    
    texdata_addr = sizeof( long ) * 7 + sizeof( TexData * ) + sizeof( long * ) ;
    pixdata_addr = texdata_addr + sizeof( TexData ) * n_textures ;
    MDU_FwriteEndianLong( fp, &texdata_addr, 1, 1 ) ;
    MDU_FwriteEndianLong( fp, &pixdata_addr, 1, 1 ) ;
}

static	void	WriteTexData( fp, texs, n_texs )
FILE		*fp ;
MDU_Tex		**texs ;
int		n_texs ;
{
    int		i, mode ;
    long		c_addr ;
    long		strcode, n_colors, flag ;
    short		pix_x, pix_y, clut_x, clut_y ;
    short		width, height ;
    
    mode = texs[ 0 ]->flag & MDU_TEX_FLAG_COLOR256 ;
    clutdata_addr = pixdata_addr + WIDTH * HEIGHT ;
    if ( mode == 0 ) clutdata_addr -= WIDTH * HEIGHT / 2 ;
    c_addr = clutdata_addr ;
    
    fseek( fp, texdata_addr, SEEK_SET ) ;
    for ( i = 0; i < n_texs; i ++ ) {
	strcode = ( long )texs[ i ]->id ;
	n_colors = ( long )texs[ i ]->n_colors ;
//printf( "%d : %x\n", texs[ i ]->id, texs[ i ]->flag ) ;
	flag = texs[ i ]->flag ;
	width = texs[ i ]->width ;
	height = texs[ i ]->height ;
	pix_x = texs[ i ]->data.dx ;
	pix_y = texs[ i ]->data.dy ;
	if ( mode == 0 ) pix_x *= 2 ;
	clut_x = texs[ i ]->clut.dx ;
	clut_y = texs[ i ]->clut.dy ;
	
	MDU_FwriteEndianLong( fp, &strcode, 1, 1 ) ;
	MDU_FwriteEndianLong( fp, &flag, 1, 1 ) ;
	MDU_FwriteEndianLong( fp, &n_colors, 1, 1 ) ;
	MDU_FwriteEndianShort( fp, &width, 1, 1 ) ;
	MDU_FwriteEndianShort( fp, &height, 1, 1 ) ;
	MDU_FwriteEndianShort( fp, &pix_x, 1, 1 ) ;
	MDU_FwriteEndianShort( fp, &pix_y, 1, 1 ) ;
	MDU_FwriteEndianShort( fp, &clut_x, 1, 1 ) ;
	MDU_FwriteEndianShort( fp, &clut_y, 1, 1 ) ;
	MDU_FwriteEndianLong( fp, &c_addr, 1, 1 ) ;
	c_addr += n_colors * sizeof( long ) ;
    }
}

static	int	WritePixData( fp, texs, n_texs )
FILE		*fp ;
MDU_Tex		**texs ;
int		n_texs ;
{
    u_int	*src ;
    u_char	*pixdata, *dst ;
    u_char	c1, c2 ;
    int		mode, size, pix_x, pix_y, pix_w, pix_h ;
    int		dst_w, dst_h ;
    int		i, j, k ;
    
    mode = texs[ 0 ]->flag & MDU_TEX_FLAG_COLOR256 ;
    dst_w = ( mode == 0 ) ? WIDTH / 2 : WIDTH ;
    dst_h = HEIGHT ;
    size = dst_w * dst_h ;
    pixdata = dst = ( u_char * )MDU_Alloc( sizeof( u_char ) * size ) ;
    if ( pixdata == NULL ) return -1 ;
    for ( i = 0; i < dst_w * dst_h ; i ++ ) dst[ i ] = 255 ;  
    for ( i = 0; i < n_texs; i ++ ) {
	src = ( u_int * )texs[ i ]->data.data ;
	pix_x = texs[ i ]->data.dx ;
	pix_y = texs[ i ]->data.dy ;
	pix_w = texs[ i ]->data.w ;
	pix_h = texs[ i ]->data.h ;
	for ( j = 0; j < pix_h; j ++ ) {
	    for ( k = 0; k < pix_w; k ++ ) {
		if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
		    c1 = src[ k * 2 + j * texs[ i ]->width ] ;
		    c2 = src[ k * 2 + j * texs[ i ]->width + 1 ] ;
		    c1 = ( ( c2 & 0xf ) << 4 ) | ( c1 & 0xf ) ;
		} else {
		    c1 = src[ k + j * pix_w ] ;
		}
		dst[ pix_x + k + ( pix_y + j ) * dst_w ] = c1 ;
	    }
	}
    }
    fseek( fp, pixdata_addr, SEEK_SET ) ;
    for ( i = 0; i < dst_w * dst_h ; i ++ ) {
	putc( *( dst ++ ), fp ) ;
    }
    return 0 ;
}

static	void	WriteClutData( fp, texs, n_texs )
FILE		*fp ;
MDU_Tex		**texs ;
int		n_texs ;
{
    int		i, j, n_colors ;
    RGBQUAD2	*clut ;

    fseek( fp, clutdata_addr, SEEK_SET ) ;
    for( i = 0; i < n_texs; i ++ ) {
	n_colors = texs[ i ]->n_colors ;
	clut = ( RGBQUAD2 * )texs[ i ]->clut.data ;
	for( j = 0; j < n_colors; j ++ ) {
	    putc( clut->r, fp ) ;
	    putc( clut->g, fp ) ;
	    putc( clut->b, fp ) ;
	    putc( clut->alpha, fp ) ;
	    clut ++ ;
	}
    }
}

int	MDU_SaveCm2( name, texs, n_texs )
char		*name ;
MDU_Tex		**texs ;
int		n_texs ;
{
    FILE	*fp ;
    
    if ( ( fp = fopen( name, "wb" ) ) != NULL ) {
	WriteCS2Header( fp, texs, n_texs ) ;
	WriteTexData( fp, texs, n_texs ) ;
	if ( WritePixData( fp, texs, n_texs ) ) return -1 ;
	WriteClutData( fp, texs, n_texs ) ;
	fclose( fp ) ;
	return 0 ;
    }
    return -1 ;
}

/*-------------------------------------------------------------------------*/

/* Cm2の中身をちょっと拝見 */
void	MDU_DumpCm2( name )
char		*name ;
{
    FILE	*fp ;
    int		size, i, j ;
    Cm2Header	*cm2 ;
    TexData	*td ;
    RGBQUAD2	*clut ;

    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) return ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) ;
    fseek( fp, 0, SEEK_SET ) ;
    cm2 = ( Cm2Header * )MDU_Alloc( size ) ;
    if ( cm2 == NULL ) return ;
    fread( cm2, size, 1, fp ) ;
    fclose( fp ) ;
    MDU_EndianLong( ( long * )cm2, sizeof( Cm2Header ) / sizeof( long ) ) ;
    printf( "magic	%ld\n", cm2->magic ) ;
    printf( "w, h	%ld %ld\n", cm2->width, cm2->height ) ;
    printf( "dx, dy	%ld %ld\n", cm2->dx, cm2->dy ) ;
    printf( "%ld 	textures\n", cm2->n_textures ) ;
    printf( "mode 	%ld\n", cm2->mode ) ;

    td = ( TexData * )( ( u_int )cm2->texdatas + ( u_int )cm2 ) ;
    for ( i = 0; i < cm2->n_textures; i ++ ) {
	MDU_EndianLong( ( long * )td, 3 ) ;
	MDU_EndianShort( ( short * )&td->width, 6 ) ;	
	printf( "id		%ld\n", td->strcode ) ;
	printf( "flag		%lx\n", td->flag ) ;
	printf( "n_colors	%ld\n", td->n_colors ) ;
	printf( "w, h		%d, %d\n", td->width, td->height ) ;
	printf( "px, py		%d, %d\n", td->pix_x, td->pix_y ) ;
	printf( "cx, cy		%d, %d\n", td->clut_x, td->clut_y ) ;

	td->clut_addr = ( long * )( ( u_int )td->clut_addr + ( u_int )cm2 ) ;
	clut = ( RGBQUAD2 * )td->clut_addr ;
	for ( j = 0; j < td->n_colors; j ++ ) {
	    printf( "[%d] %d %d %d %d\n", j, clut->r, clut->g, clut->b, clut->alpha ) ;
	    clut ++ ;
	}
	td ++ ;
    }
}

/*-------------------------------------------------------------------------*/

#if 0
typedef struct _TexData {
    long		strcode ;		/* テクスチャID */
    long		flag ;			/* フラグ */
    long		n_colors ;		/* 使用色数	*/
    short		width, height ;		/* 幅,高さ */
    short		pix_x, pix_y ;		/* ピクセルデータ位置 */
    short		clut_x, clut_y ;	/* クラットデータ位置 */
    long		*clut_addr ;		/* クラットデータへのポインタ */
} TexData ;
#endif

