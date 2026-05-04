/*
   MDU_texSave.c

   テクスチャセーブ関数

   by M.Sonoyama 1999 Aug.～
   $Id: MDU_texSave.c,v 1.12 2002/12/26 07:08:11 usr01363 Exp $

   Konami Computer Entertainment Japan West   
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<sys/types.h>

#include	"MDU_util.h"
#include	"MDU_tex.h"

static	int		N_Colors, Mode ;
static	int		Width, Height ;

/*-------------------------------------------------------------------*/

/* ビットマップファイルセーブ */
static	void	BmpWriteHeader( tex, fp )
MDU_Tex		*tex ;
FILE		*fp ;
{
    // 必ずBITMAPFILEHEADERの最初にpaddingが入った形式を用いること
    // paddingがないとBITMAPFILEHEADERがもともと14Byteという半端なサイズなので、
    // 書き込みがずれてしまう。
    BITMAPHEADER_PAD	header ;
    int			size, rw, bit ;
    
    Width = tex->width ;
    Height = tex->height ;
    
    if ( Mode == 0 ) {
	bit = 4 ;
	rw = ( ( ( Width + 3 ) / 4 * 4 / 2 ) + 3 ) / 4 * 4 ;
	size = 54 + 16 * 4 + rw * Height ;
	N_Colors = 16 ;
    } else {
	bit = 8 ;
	rw = ( Width + 3 ) / 4 * 4 ;
	size = 54 + 256 * 4 + rw * Height ;
	N_Colors = 256 ;
    }
    
    header.header.bfType = 19778 ;
    header.header.bfSize = size ;  
    header.header.bfReserved1 = 0 ;
    header.header.bfReserved2 = 0 ;
    header.header.bfOffBits = 54 + N_Colors * 4 ;  
    
    header.info.biSize = 40 ;
    header.info.biWidth = Width ;
    header.info.biHeight = Height ;
    header.info.biPlanes = 1 ;
    header.info.biBitCount = bit ;
    header.info.biCompression = 0 ;
    header.info.biSizeImage = header.header.bfSize - header.header.bfOffBits ;

//    header.info.biXPelsPerMeter = 0 ;
    header.info.biXPelsPerMeter = tex->flag & MDU_TEX_FLAG_ALL ;
    header.info.biYPelsPerMeter = 0 ;
    header.info.biClrUsed = 0 ;
    header.info.biClrImportant = 0 ;  
    
    MDU_EndianShort( &header.header.bfType, 1 ) ;
    MDU_EndianLong( &header.header.bfSize, 1 ) ;  
    MDU_EndianLong( &header.header.bfOffBits, 1 ) ;  
    MDU_EndianLong( &header.info.biSize, 3 ) ;  
    MDU_EndianShort( &header.info.biPlanes, 2 ) ;
    MDU_EndianLong( &header.info.biCompression, 6 ) ;  
    
    fwrite( &( header.header.bfType ), sizeof( BITMAPHEADER ) - 2, 1, fp ) ;
}

static	void	BmpWriteClutData( tex, fp )
MDU_Tex		*tex ;
FILE		*fp ;
{
    int		i, n_colors ;
    RGBQUAD2	rgb, *src ;
    
    fseek( fp, 54, SEEK_SET ) ;
    n_colors = tex->n_colors ;
    src = ( RGBQUAD2 * )tex->clut.data ;
    for ( i = 0; i < N_Colors; i ++ ) {
	if ( i >= n_colors ) {
	    rgb.r = rgb.g = rgb.b = rgb.alpha = 0 ;
	} else {
	    rgb = *( src ++ ) ;
	}
	fwrite( &rgb, sizeof( RGBQUAD2 ), 1, fp ) ;
    }
}

static	void	BmpWritePixData( tex, fp )
MDU_Tex		*tex ;
FILE		*fp ;
{
    int		i, j, pad, p ;
    u_int	*src, c1, c2 ;
    char	c ;

    c = 0 ; pad = 0 ;
    if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
	pad = 4 - ( tex->width / 2 ) % 4 ;
	if ( pad == 4 ) pad = 0 ;	
    } else if ( Mode == MDU_TEX_FLAG_COLOR256 ) {
	pad = 4 - tex->width  % 4 ;
	if ( pad == 4 ) pad = 0 ;	
    }

    fseek( fp, 54 + N_Colors * 4, SEEK_SET ) ;
    src = ( u_int * )tex->data.data ;

    src += Width * ( Height - 1 ) ;
    for ( j = 0; j < Height; j ++ ) {
	for ( i = 0; i < Width; i ++ ) {
	    c1 = *( src ++ ) ;
	    if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
		c2 = *( src ++ ) ; i ++ ;
		c1 = ( ( c1 & 0xf ) << 4 ) | ( c2 & 0xf ) ;
	    }
	    putc( c1, fp ) ;
	}
	src -= Width * 2 ;
	p = pad ;
	while ( -- p >= 0 ) putc( c, fp ) ;
    } 
}

void	MDU_SaveBmp( tex, name )
MDU_Tex	*tex ;
char	*name ;
{
    FILE		*fp ;

    fp = fopen( name, "wb" ) ;
    if ( fp == NULL ) return ;
    if ( tex == NULL ) return ;
    Mode = tex->flag & MDU_TEX_FLAG_COLOR256 ;
    BmpWriteHeader( tex, fp ) ;
    BmpWriteClutData( tex, fp ) ;
    BmpWritePixData( tex, fp ) ;
    fclose( fp ) ;
}

/*-------------------------------------------------------------------*/

/* ＴＩＭファイルのセーブ */

static	void	*EndianRect2( data, mode )
TIMRECT		*data ;
int		mode ;
{
    void		*next ;
	
    next = (void *)data + data->bnum ;
    if ( mode == 2 ) {
	MDU_EndianShort( data->data, data->w * data->h ) ;
    }
    MDU_EndianLong( &( data->bnum ), 1 ) ;
    MDU_EndianShort( &( data->dx ), 4 ) ;
    return next ;
}

static	TIMRECT	*TimSetTimRectPixel( data, mode )
MDU_TexRect	*data ;
int		mode ;
{
    TIMRECT	*rect ;
    int		size ;
    u_int	*src ;
    u_char	*dst, c1, c2 ;
    int		i ;

    size = data->w * data->h / 2 * sizeof( u_short ) + sizeof( TIMRECT ) ;
    rect = ( TIMRECT * )MDU_Alloc( size ) ;
    if ( rect == NULL ) return NULL ;
    rect->bnum = size ;
    rect->w = data->w / 2 ;
    rect->h = data->h ;
    src = data->data ;
    dst = ( u_char * )rect->data ;
    for ( i = 0; i < data->w * data->h; i ++ ) {
	if ( mode == MDU_TEX_FLAG_COLOR16 ) {
	    c1 = *( src ++ ) ;
	    c2 = *( src ++ ) ;
	    c1 = ( ( c2 & 0xf ) << 4 ) | ( c1 & 0xf ) ;
	} else {
	    c1 = *( src ++ ) ;
	}
	*( dst ++ ) = c1 ;
    }
    return rect ;
}

static	TIMRECT	*TimSetTimRectClut( data, flag )
MDU_TexRect	*data ;
int		flag ;
{
    TIMRECT	*rect ;
    int		size ;
    RGBQUAD2	*src ;
    u_short	*dst, col ;
    u_char	r, g, b ;
    int		i ;

    if ( data->data == NULL ) return NULL ;
    size = data->w * data->h * sizeof( u_short ) + sizeof( TIMRECT ) ;
    rect = ( TIMRECT * )MDU_Alloc( size ) ;
    if ( rect == NULL ) return NULL ;
    rect->bnum = size ;
    rect->w = data->w ;
    rect->h = data->h ;
    src = ( RGBQUAD2 * )data->data ;
    dst = ( u_short * )rect->data ;
    for ( i = 0; i < data->w * data->h; i ++ ) {
	r = ( u_char )( ( float )src->r * 31.0F / 255.0F ) & 0x1f ;
	g = ( u_char )( ( float )src->g * 31.0F / 255.0F ) & 0x1f ;
	b = ( u_char )( ( float )src->b * 31.0F / 255.0F ) & 0x1f ;
	col = r | ( g << 5 ) | ( b << 10 ) ;
	if ( r == 0 && g == 0 && b == 0 ) {
	    if ( flag & MDU_TEX_FLAG_BLACK_NOTTRANS ) col |= 0x80 ;
	} else {
	    if ( flag & MDU_TEX_FLAG_COLOR_TRANS ) col |= 0x80 ;
	}
	*( dst ++ ) = col ;
	src ++ ;
    }
    return rect ;
}

void		MDU_SaveTim( tex, name )
MDU_Tex		*tex ;
char		*name ;
{
    FILE	*fp ;
    TIMRECT	*data, *clut ;
    int		size, mode ;
    u_long	ul[ 2 ], flag ;

    ul[ 0 ] = 0x10 ;
    ul[ 1 ] = flag = tex->flag & 0xff00013f ;
    mode = flag & 0x3 ;

    data = TimSetTimRectPixel( &( tex->data ), mode ) ;
    clut = TimSetTimRectClut( &( tex->clut ), flag ) ;

    if ( ( fp = fopen( name, "wb" ) ) == NULL ) return ;

    MDU_EndianLong( ul, 2 ) ;
    fwrite( ul, sizeof( u_long ), 2, fp ) ;
    size = clut->bnum ;
    EndianRect2( clut, 2 ) ;
    fwrite( clut, size, 1, fp ) ;
    size = data->bnum ;
    EndianRect2( data, mode ) ;
    fwrite( data, size, 1, fp ) ;
    fclose( fp ) ;
    MDU_Free( data ) ;
    MDU_Free( clut ) ;
}

/*-------------------------------------------------------------------*/

/* ＰＣＸファイルのセーブ */

static	void	BZero( to, size )
char		*to ;
int		size ;
{
    while ( -- size >= 0 ) *( to ++ ) = 0 ;
}

/*----------------------------------------------------------------*/

static	void	PcxWriteHeader( fp, tex )
FILE		*fp ;
MDU_Tex		*tex ;
{
    PCX_Header	header ;
    int		w, h ;
    
    w = tex->width ;
    h = tex->height ;
    Mode = tex->flag & MDU_TEX_FLAG_COLOR256 ;

    if ( Mode == MDU_TEX_FLAG_COLOR256 ) {
	header.flag = 10 ;
	header.version = 5 ;
	header.encoding = 1 ;
	header.bits_per_pixel = 8 ;
	header.min_x = 0 ;	header.min_y = 0 ;
	header.max_x = w - 1 ;	header.max_y = h - 1 ;
	header.dpi_x = 0x0640 ;	header.dpi_y = 0x04b0 ;
	BZero( header.header_palette, 48 ) ;
	header.reserved = 0 ;
	header.n_planes = 1 ;
	header.bytes_per_line = w ;
	header.header_palette_class = 1 ;
	header.screen_width = 0x280 ;
	header.screen_height = 0x1e0 ;
	BZero( header.pad, 54 ) ;
	header.appendix[ 0 ].stamp = PCXFILE_STAMP ;
	header.appendix[ 0 ].flag = tex->flag ;
	header.appendix[ 0 ].px = tex->data.dx ;
	header.appendix[ 0 ].py = tex->data.dy ;
	header.appendix[ 0 ].cx = tex->clut.dx ;
	header.appendix[ 0 ].cy = tex->clut.dy ;
	header.appendix[ 0 ].n_colors = tex->n_colors ;
	
	MDU_EndianShort( &( header.min_x ), 6 ) ;
	MDU_EndianShort( &( header.bytes_per_line ), 4 ) ;
	MDU_EndianShort( header.appendix, 9 ) ;
	fwrite( &header, sizeof( PCX_Header ), 1, fp ) ;
    } else {
	header.flag = 10 ;
	header.version = 5 ;
	header.encoding = 1 ;
	header.bits_per_pixel = 1 ;
	header.min_x = 0 ;	header.min_y = 0 ;
	header.max_x = w - 1 ;	header.max_y = h - 1 ;
	header.dpi_x = 0x0640 ;	header.dpi_y = 0x04b0 ;
	{
	    RGBQUAD2		*src ;
	    unsigned char	r, g, b ;
	    int		i ;
	    
	    src = ( RGBQUAD2 * )tex->clut.data ;
	    for ( i = 0 ; i < 16 ; i ++ ) {
		r = src->r ;
		g = src->g ;
		b = src->b ;
		header.header_palette[ i * 3 ] = r ;
		header.header_palette[ i * 3 + 1 ] = g ;
		header.header_palette[ i * 3 + 2 ] = b ;
		src ++ ;
	    }
	}
	header.reserved = 0 ;
	header.n_planes = 4 ;
	w = ( w + 7 ) / 8 ;
	if ( w % 2 ) w ++ ;
	header.bytes_per_line = w ;
	header.header_palette_class = 1 ;
	header.screen_width = 0x280 ;
	header.screen_height = 0x1e0 ;
	BZero( header.pad, 54 ) ;
	
	header.appendix[ 0 ].stamp = PCXFILE_STAMP ;
	header.appendix[ 0 ].flag = tex->flag ;
	header.appendix[ 0 ].px = tex->data.dx ;
	header.appendix[ 0 ].py = tex->data.dy ;
	header.appendix[ 0 ].cx = tex->clut.dx ;
	header.appendix[ 0 ].cy = tex->clut.dy ;
	header.appendix[ 0 ].n_colors = tex->n_colors ;
	MDU_EndianShort( &( header.min_x ), 6 ) ;
	MDU_EndianShort( &( header.bytes_per_line ), 4 ) ;
	MDU_EndianShort( header.appendix, 9 ) ;
	fwrite( &header, sizeof( PCX_Header ), 1, fp ) ;
    }
}

/*----------------------------------------------------------------*/

u_int		*PcxGet16ModeTexData( tex )
MDU_Tex		*tex ;
{
    u_int		*src, *dst ;
    u_char		*bits, code, d ;
    u_char		bytes_per_line ;
    int			w, h ;
    int			i, j, s, ri ;
    
    
    w = tex->width ;
    h = tex->height ;
    bytes_per_line = ( w + 7 ) / 8 ;
    if ( bytes_per_line % 2 ) bytes_per_line ++ ;
    dst = ( u_int *)MDU_Alloc( sizeof( u_int ) * bytes_per_line * 4 * h ) ;
    bits = ( u_char * )MDU_Alloc( sizeof( u_char ) * bytes_per_line * 8 * 4 * h ) ;
    src = ( u_int * )( tex->data.data ) ;
    d = 0x0f ;
    for ( j = 0; j < h ; j ++ ) {
	s = j * bytes_per_line * 8 * 4 ;
	for ( i = 0; i < bytes_per_line * 8; i ++ ) {
	    if ( i >= w ) code = 0 ;
	    else code = *( src ++ ) ;
	    if ( i % 2 ) ri = i + 1;
	    else ri = i - 1 ;
	    bits[ s + i + bytes_per_line * 8 * 3 ] = code / 8 ;
	    if ( code >= 8 ) code -= 8 ;
	    bits[ s + i + bytes_per_line * 8 * 2 ] = code / 4 ;
	    if ( code >= 4 ) code -= 4 ;	   
	    bits[ s + i + bytes_per_line * 8 * 1 ] = code / 2 ;
	    if ( code >= 2 ) code -= 2 ;
	    bits[ s + i ] = code ;
	}
    }
    for ( i = 0 ; i < bytes_per_line * 4 * 8 * h; i += 8 ) {
	*( dst ++ ) = bits[ i ] * 128 
	            + bits[ i + 1 ] * 64
		    + bits[ i + 2 ] * 32
	            + bits[ i + 3 ] * 16
		    + bits[ i + 4 ] * 8
		    + bits[ i + 5 ] * 4
		    + bits[ i + 6 ] * 2
		    + bits[ i + 7 ] ;
    }
    dst -= bytes_per_line * 4 * h ;
    return dst ;
}

static	void	PcxWriteData8( fp, tex )
FILE		*fp ;
MDU_Tex		*tex ;
{
    {
	u_int		*src ;
	u_char		col, new_col ;
	int		i, j, len ;
	int		w, h ;
	
	if ( Mode == MDU_TEX_FLAG_COLOR256 ) {
	    src = ( u_int * )( tex->data.data ) ;
	    w = tex->width ;
	    h = tex->height ;
	} else {
	    src = PcxGet16ModeTexData( tex ) ;
	    w = ( tex->width + 7 ) / 8 ;
	    if ( w % 2 ) w ++ ;
	    w *= 4 ;
	    h = tex->height ;
	}
	
	for ( i = h ; i > 0 ; -- i ) {
	    col = ( u_char )( *src ) ;
	    len = 0 ;
	    for ( j = w ; j > 0 ; -- j ) {
		new_col = ( u_char )( *( src ++ ) ) ;
		if ( new_col != col || len == 63 ) {
		    if ( col < 192 && len == 1 ) {
			putc( col, fp ) ;
		    } else {
			putc( len + 192, fp ) ;
			putc( col, fp ) ;
		    }
		    col = new_col ;
		    len = 0 ;
		}
		len ++ ;
	    }
	    if ( len > 0 ) {
		if ( col < 192 && len == 1 ) {
		    putc( col, fp ) ;
		} else {
		    putc( len + 192, fp ) ;
		    putc( col, fp ) ;
		}
	    }
	}
    }
    
    if ( Mode == MDU_TEX_FLAG_COLOR256 ) {
	unsigned char	r, g, b, a ;
	int		i ;
	RGBQUAD2		*src, code ;
	
	putc( 0x0c, fp ) ; 
	
	src = ( RGBQUAD2 * )tex->clut.data ;
	for ( i = 256 ; i > 0 ; -- i ) {
	    code = *( src ++ ) ;
	    r = code.r ;
	    g = code.g ;
	    b = code.b ;
	    a = code.alpha ;
	    putc( r, fp ) ;
	    putc( g, fp ) ;
	    putc( b, fp ) ;
	}
    }
}

void		MDU_SavePcx( tex, name )
MDU_Tex		*tex ;
char		*name ;
{
    FILE		*fp ;
    
    if ( ( fp = fopen( name, "wb" ) ) != NULL ) {
	PcxWriteHeader( fp, tex ) ;
	PcxWriteData8( fp, tex ) ;
	fclose( fp ) ;
    }
}
