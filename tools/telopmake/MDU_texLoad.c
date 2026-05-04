/*
   MDU_texLoad.c
   
   テクスチャロード関数
   
   by M.Sonoyama 1999 Aug.～
   $Id: MDU_texLoad.c,v 1.1 2002/07/12 04:10:58 usr01475 Exp $
   
   Konami Computer Entertainment Japan West   
   */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>

#include	"MDU_util.h"
#include	"MDU_tex.h"

static	RGBTriple	*Rgb ;
static	u_char		*Index ;
static	int		N_Colors, Mode, Offx, Reverse ;
static	int		Width, Height, Size ;
static	RGBQUAD		Colors[ 256 ] ;

/*-------------------------------------------------------------------*/

/* ビットマップファイルロード */

/* ヘッダ読み込み */
static	void	BmpReadHeader( bmphead, fp )
BITMAPHEADER	*bmphead ;
FILE		*fp ;
{
    fread( &( bmphead->header.bfType ), sizeof( BITMAPHEADER ) - 2, 1, fp ) ; 
    MDU_EndianShort( &( bmphead->header.bfType ), 1 ) ;
    MDU_EndianLong( &( bmphead->header.bfSize ), 1 ) ;
    MDU_EndianLong( &( bmphead->header.bfOffBits ), 1 ) ;
    MDU_EndianLong( &( bmphead->info.biSize ), 3 ) ;
    MDU_EndianShort( &( bmphead->info.biPlanes ), 2 ) ;
    MDU_EndianLong( &( bmphead->info.biCompression ), 6 ) ;
#if 0
    printf( "%d\n", bmphead->header.bfType ) ;
    printf( "%d\n", bmphead->header.bfSize ) ;
    printf( "%d\n", bmphead->header.bfOffBits ) ;

    printf( "%d\n", bmphead->info.biSize ) ;
    printf( "%d\n", bmphead->info.biWidth ) ;
    printf( "%d\n", bmphead->info.biHeight ) ;
    printf( "%d\n", bmphead->info.biPlanes ) ;
    printf( "%d\n", bmphead->info.biBitCount ) ;
    printf( "%d\n", bmphead->info.biCompression ) ;
    printf( "%d\n", bmphead->info.biSizeImage ) ;
    printf( "%d\n", bmphead->info.biClrUsed ) ;
#endif
    if ( bmphead->info.biHeight < 0 ) {
		Reverse = 1 ; bmphead->info.biHeight = -bmphead->info.biHeight ;
    } else {
		Reverse = 0 ;
    }
}

static	int	BmpCheckColor4( header, fp )
BITMAPHEADER	*header ;
FILE		*fp ;
{
    RGBTriple	*rgb ;
    RGBQUAD	*colors ;
    int		i, w, pad, size ;
    u_char	*index, *rgbpad, code ;

    Mode = MDU_TEX_FLAG_COLOR16 ;
	//    if ( header->info.biWidth % 4 ) {
		//	printf( "wrong width %ld\n", header->info.biWidth ) ;
		//	return -1 ;
		//    }
    colors = Colors ;
//    fseek( fp, header->header.bfOffBits - 4 * 16, SEEK_SET ) ;
    fseek( fp, sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) - 2, SEEK_SET ) ;
//printf( "%x\n", ftell( fp ) ) ;
//printf( "%x\n", sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) - 2 ) ;
    fread( colors, sizeof( RGBQUAD ), 16, fp ) ;
#if 0
	{
		int		i ;

		for ( i = 0; i < 16; i ++ ) {
			printf( "[%d] %d %d %d\n", i, colors[ i ].r, colors[ i ].g, colors[ i ].b ) ;
		}
	}
#endif
    N_Colors = 16 ;
	size = header->info.biSizeImage ;
	if ( size == 0 ) {
		size = ( ( ( header->info.biWidth / 2 ) + 3 ) / 4 * 4 ) * header->info.biHeight ; 
//printf( "size 0 -> %d\n", size ) ;
	}
	rgb = ( RGBTriple * )MDU_Alloc( size ) ;
    if ( rgb == NULL ) return -1 ;
    Rgb = rgb ;
    index = ( u_char * )MDU_Alloc( header->info.biWidth * header->info.biHeight ) ;
    if ( index == NULL ) return -1 ;
    Index = index ;
    if ( Reverse == 0 ) {
		index = Index + header->info.biWidth * ( header->info.biHeight - 1 ) ;
    }
    w = 0 ; 
    pad = 4 - ( header->info.biWidth / 2 ) % 4 ;
    if ( pad == 4 ) pad = 0 ;
    rgbpad = ( u_char * )rgb ;
//    fseek( fp, header->header.bfOffBits, SEEK_SET ) ;
    fseek( fp, header->header.bfOffBits, SEEK_SET ) ;
    fread( rgbpad, sizeof( u_char ), size, fp ) ;
    if ( pad == 4 ) pad = 0 ;
    for ( i = 0; i < header->info.biWidth * header->info.biHeight / 2; i ++ ) {
		code = *( rgbpad ++ ) ;
		index[ 1 ] = code & 0x0f ;
		index[ 0 ] = ( code & 0xf0 ) >> 4 ;
//printf( "%2d%2d", index[ 0 ], index[ 1 ] ) ;
		index += 2 ;
		if ( ++ w == header->info.biWidth / 2 ) {
			rgbpad += pad ;
			if ( Reverse == 0 ) index -= header->info.biWidth * 2 ;
			w = 0 ;
//printf( "\n" ) ;
		}
    }
    return 0 ;
}

static	int	BmpCheckColor8( header, fp )
BITMAPHEADER 	*header ;
FILE		*fp ;
{
    RGBTriple	*rgb ;
    RGBQUAD	*colors ;
    int		i, w, pad, size ;
    u_char	*index, *rgbpad ;

    Mode = MDU_TEX_FLAG_COLOR256 ;
    colors = Colors ;

    fseek( fp, header->header.bfOffBits - sizeof( RGBQUAD ) * 256, SEEK_SET ) ;
    fread( colors, sizeof( RGBQUAD ), 256, fp ) ;
    N_Colors = 256 ;
	size = header->info.biSizeImage ;
	if ( size == 0 ) {
		size = ( ( header->info.biWidth + 3 ) / 4 * 4 ) * header->info.biHeight ; 
	}
	rgb = ( RGBTriple * )MDU_Alloc( size ) ;
    if ( rgb == NULL ) return -1 ;
    Rgb = rgb ;
    index = ( u_char * )MDU_Alloc( header->info.biWidth * header->info.biHeight ) ;
    if ( index == NULL ) return -1 ;
    Index = index ;
    if ( Reverse == 0 ) {
		index = Index + header->info.biWidth * ( header->info.biHeight - 1 ) ;
    }
    w = 0 ; 
    pad = 4 - ( header->info.biWidth ) % 4 ;
    if ( pad == 4 ) pad = 0 ;
    rgbpad = ( u_char * )rgb ;
    fseek( fp, header->header.bfOffBits, SEEK_SET ) ;
    fread( rgbpad, sizeof( u_char ), size, fp ) ;
    if ( pad == 4 ) pad = 0 ;
    for ( i = 0; i < header->info.biWidth * header->info.biHeight; i ++ ) {
		*( index ++ ) = *( rgbpad ++ ) ;
		if ( ++ w == header->info.biWidth ) {
			rgbpad += pad ;
			if ( Reverse == 0 ) index -= header->info.biWidth * 2 ;
			w = 0 ;
		}
    }
    return 0 ;
}

static	int	BmpCheckColor24( header, fp )
BITMAPHEADER 	*header ;
FILE		*fp ;
{
    RGBTriple	*rgb, *color ;
    RGBQUAD	*colors ;
    int		size, i, j, c, w, pad ;
    u_char	*index, *rgbpad ;

    fseek( fp, header->header.bfOffBits, SEEK_SET ) ;
    size = header->info.biSizeImage ;
    if ( size == 0 ) size = header->header.bfSize - ftell( fp ) ;
    rgb = ( RGBTriple * )MDU_Alloc( size ) ;
    if ( rgb == NULL ) return -1 ;
    fread( rgb, sizeof( u_char ), size, fp ) ;
    Rgb = rgb ;
    size = header->info.biWidth * header->info.biHeight ;
    index = ( u_char * )MDU_Alloc( size ) ;
    if ( index == NULL ) return -1 ;
    Index = index ;
    if ( Reverse == 0 ) {
		index = Index + header->info.biWidth * ( header->info.biHeight - 1 ) ;
    }
    N_Colors = 0 ;
    rgb = Rgb ; colors = Colors ;
    w = 0 ;
    pad = 4 - header->info.biWidth * 3 % 4 ;
    if ( pad == 4 ) pad = 0 ;
    for ( i = 0; i < size; i ++ ) {
		c = 0 ;
		color = rgb ; 
		for ( j = 0; j < N_Colors; j ++ ) {
			if ( color->r == Colors[ j ].r &&
				color->g == Colors[ j ].g &&
				color->b == Colors[ j ].b ) {
				*( index ++ ) = j ; c = 1 ;
				break ;
			}
		}
		if ( c == 0 ) {
			if ( N_Colors == 256 ) {
				printf( "too many colors ( over 256 )\n" ) ;
				return -1 ;
			}
			Colors[ N_Colors ].r = color->r ;
			Colors[ N_Colors ].g = color->g ;
			Colors[ N_Colors ].b = color->b ;
			Colors[ N_Colors ].alpha = 128 ; /* 注意せよ */
			*( index ++ ) = N_Colors ;
			N_Colors ++ ;
		}
		rgb ++ ;
		if ( ++ w == header->info.biWidth ) {
			rgbpad = ( u_char * )rgb ; rgbpad += pad ;
			rgb = ( RGBTriple * )rgbpad ;
			if ( Reverse == 0 ) index -= header->info.biWidth * 2 ;
			w = 0 ;
		}
    }
    printf( "%d colors used\n", N_Colors ) ;
    Mode = ( N_Colors <= 16 ) ? MDU_TEX_FLAG_COLOR16 : MDU_TEX_FLAG_COLOR256 ;
    return 0 ;
}

static	int	BmpCheckColor( header, fp )
BITMAPHEADER	*header ;
FILE		*fp ;
{
    int	bitmode ;

    bitmode = header->info.biBitCount ;
    switch( bitmode ) {
    case 4 :
		if ( BmpCheckColor4( header, fp ) < 0 ) return -1 ;
		break ;
    case 8 :
		if ( BmpCheckColor8( header, fp ) < 0 ) return -1 ;
		break ;
    case 24 :
		if ( BmpCheckColor24( header, fp ) < 0 ) return -1 ; 
		break ;
    default :
		fprintf( stderr, "invalid bit mode %d\n", bitmode ) ;
		return -1 ;
    }
    return 0 ;
}

static	void	BmpSetPixelAndClut( tex, header )
MDU_Tex		*tex ;
BITMAPHEADER	*header ;
{
    RGBQUAD	*rgb ;
    u_int	*dst ;
    u_char	*src ;
    u_char	r, g, b, a ;
    int		i, j ;
    int		w, h ;    

    rgb = ( RGBQUAD * )tex->clut.data ;
    for ( i = 0; i < N_Colors; i ++, rgb ++ ) {
		r = Colors[ i ].r ; 
		g = Colors[ i ].g ;
		b = Colors[ i ].b ;
		a = Colors[ i ].alpha ;
		rgb->r = r ;
		rgb->g = g ;
		rgb->b = b ;
		rgb->alpha = a ;
    }
	
    w = header->info.biWidth ;
    h = header->info.biHeight ;
    dst = tex->data.data ;
    src = Index ;

    for ( j = 0; j < h; j ++ ) {
		for ( i = 0; i < w + Offx ; i ++ ) {
			if ( i >= w ) *( dst ++ ) = 0 ;
			else *( dst ++ ) = *( src ++ ) ;
		}
    }
}

MDU_Tex		*MDU_LoadBmp( name )
char		*name ;
{
    MDU_Tex		*tex ;
    FILE		*fp ;
    BITMAPHEADER	bmphead ;

    tex = NULL ; Rgb = NULL ; Index = NULL ; fp = NULL ;
    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) goto bmp_load_error ;
    BmpReadHeader( &bmphead, fp ) ;
    if ( BmpCheckColor( &bmphead, fp ) < 0 ) goto bmp_load_error ;
    if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
		Offx = bmphead.info.biWidth % 2 ;
    } else {
		Offx = 4 - bmphead.info.biWidth % 4 ;
		if ( Offx == 4 ) Offx = 0 ;
    }
    tex = MDU_FindTexture( 0 ) ;
    if ( tex == NULL ) goto bmp_load_error ;
    MDU_AllocTex( tex, bmphead.info.biWidth + Offx, 
				 bmphead.info.biHeight ) ;
	//    MDU_AllocTex( tex, bmphead.info.biWidth,
					   //		 bmphead.info.biHeight ) ;
    tex->id = MDU_GetStrCode( name ) ;
    tex->flag = bmphead.info.biXPelsPerMeter & MDU_TEX_FLAG_ALL ;
    tex->flag &= ~MDU_TEX_FLAG_COLOR256 ;
    tex->flag |= MDU_TEX_FLAG_CLUT_EXIST | Mode ;
    tex->data.dx = tex->data.dy = 0 ;
    tex->clut.dx = tex->clut.dy = 0 ;
    tex->n_colors = N_Colors ;
    if ( ( tex->flag & Mode ) == MDU_TEX_FLAG_COLOR256 ) {
		tex->data.w = tex->width ;
		tex->data.h = tex->height ;
    } else {
		tex->data.w = tex->width / 2 ;
		tex->data.h = tex->height ;
    }
    tex->clut.w = tex->n_colors ;
    tex->clut.h = 1 ;
    BmpSetPixelAndClut( tex, &bmphead ) ;
    fclose( fp ) ;
    if ( Rgb != NULL ) MDU_Free( Rgb ) ;
    if ( Index != NULL ) MDU_Free( Index ) ;
    return tex ;

bmp_load_error :    
	printf( "bmp load error\n" ) ;
    if ( fp != NULL ) fclose( fp ) ;
    if ( Rgb != NULL ) MDU_Free( Rgb ) ;
    if ( Index != NULL ) MDU_Free( Index ) ;
    if ( tex != NULL ) MDU_ClearTexture( tex ) ;
    return NULL ;
}

/*-------------------------------------------------------------------*/

/* ＴＩＭファイルのロード */

static	int	Tim_N_Colors[] = { 16, 256, 65536, 16777216 } ;

static	void	*EndianRect( data, mode )
TIMRECT		*data ;
int		mode ;
{
    MDU_EndianLong( &( data->bnum ), 1 ) ;
    MDU_EndianShort( &( data->dx ), 4 ) ;
    if ( mode == 2 ) {
		MDU_EndianShort( data->data, data->w * data->h ) ;
    }
    return (void *)data + data->bnum ;
}

static	int	ExpandWidth( mode, width )
int		mode, width ;
{
    switch ( mode ) {
    case 0 :
		return width * 4 ;
    case 1 :
		return width * 2 ;
    case 2 :
		return width ;
    case 3 :
		return width * 2 / 3 ;
    }
    return width ;
}

static	void	TimSetPixel( tex, data )
MDU_Tex		*tex ;
TIMRECT		*data ;
{
    u_int	*dst ;
    u_char	*src, c, c1, c2 ;
    int		i ;

    tex->data.dx = data->dx * 2 ;
    tex->data.dy = data->dy ;
    tex->data.w = data->w * 2 ;
    tex->data.h = data->h ;
    
    src = ( u_char * )data->data ;
    dst = tex->data.data ;
    for ( i = 0; i < data->w * data->h * 2; i ++ ) {
		if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
			c = *( src ++ ) ;
			c1 = c & 0x0f ; c2 = ( c & 0xf0 ) >> 4 ;
			*( dst ++ ) = c1 ; *( dst ++ ) = c2 ;
		} else if ( Mode == MDU_TEX_FLAG_COLOR256 ) {
			*( dst ++ ) = *( src ++ ) ;
		}
    }
}

static	void	TimSetClut( tex, clut )
MDU_Tex		*tex ;
TIMRECT		*clut ;
{
    RGBQUAD	*dst ;
    u_short	*src, c ;
    u_char	r, g, b ;
    float	t ;
    int		i ;

    tex->clut.dx = clut->dx ;
    tex->clut.dy = clut->dy ;
    tex->clut.w = clut->w ;
    tex->clut.h = clut->h ;
    
    tex->flag &= ~( MDU_TEX_FLAG_BLACK_NOTTRANS | MDU_TEX_FLAG_COLOR_TRANS ) ;

    t = 255.0F / 31.0F ;
    src = clut->data ;
    dst = ( RGBQUAD * )tex->clut.data ;
    for ( i = 0; i < clut->w * clut->h; i ++ ) {
		c = *( src ++ ) ;
		r = ( u_char )( ( float )( c & 0x1f ) * t ) ;
		g = ( u_char )( ( float )( ( c >> 5 ) & 0x1f ) * t ) ;
		b = ( u_char )( ( float )( ( c >> 10 ) & 0x1f ) * t ) ;
		if ( c & 0x80 ) {
			if ( r == 0 && g == 0 && b == 0 ) {
				tex->flag |= MDU_TEX_FLAG_BLACK_NOTTRANS ;
				r = g = b = 1 ;
			} else {
				tex->flag |= MDU_TEX_FLAG_COLOR_TRANS ;
			}
		}
		dst->r = r ; dst->g = g ; dst->b = b ;
		dst->alpha = 128 ;		/* とりあえず */
		dst ++ ;
    }	
}

MDU_Tex		*MDU_LoadTim( name )
char		*name ;
{
    FILE	*fp ;
    MDU_Tex	*tex ;
    TIMDATA	*timdata ;
    TIMRECT	*clut, *data ;
    int		size, mode ;

    fp = NULL ;
    tex = MDU_FindTexture( 0 ) ;
    if ( tex == NULL ) goto tim_load_error ;
    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) goto tim_load_error ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) ;
    fseek( fp, 0, SEEK_SET ) ;
    timdata = ( TIMDATA * )MDU_Alloc( size ) ;
    if ( timdata == NULL ) goto tim_load_error ;
    fread( timdata, size, 1, fp ) ;
    fclose( fp ) ;
    MDU_EndianLong( ( u_long * )timdata, 2 ) ;
    mode = timdata->flag & 7 ;
    clut = NULL ; 
    data = timdata->clut ;
    if ( timdata->flag & MDU_TEX_FLAG_CLUT_EXIST ) {
		clut = data ;
		data = EndianRect( clut, 2 ) ;
    }
    EndianRect( data, mode ) ;
    MDU_AllocTex( tex, ExpandWidth( mode, data->w ), data->h ) ;
    tex->id = MDU_GetStrCode( name ) ;
    tex->flag = timdata->flag & 0x3f ;
    tex->n_colors = Tim_N_Colors[ mode ] ;
    Mode = mode & 3 ;
    if ( mode >= 2 ) printf( "full color is not yet\n" ) ;

    TimSetPixel( tex, data ) ;
    TimSetClut( tex, clut ) ;
    MDU_Free( timdata ) ;
    return tex ;

	tim_load_error :    
    printf( "tim load error\n" ) ;
    if ( fp != NULL ) fclose( fp ) ;
    if ( tex != NULL ) MDU_ClearTexture( tex ) ;
    return NULL ;
}

/*-------------------------------------------------------------------*/

/* ＰＣＸファイルのロード */

static	TIM_Appendix	AppendixInfo ;
u_char	Palette16[ 48 ] ;
u_char	BytesPerLine16 ;

static	int	PcxReadHeader( fp )
FILE		*fp ;
{
    PCX_Header	header ;
    
    fread( &header, sizeof( PCX_Header ), 1, fp ) ;
    MDU_EndianShort( &( header.min_x ), 6 ) ;
    MDU_EndianShort( &( header.bytes_per_line ), 4 ) ;
    MDU_EndianShort( header.appendix, 9 ) ;
    Width = header.max_x - header.min_x + 1 ;
    Height = header.max_y - header.min_y + 1 ;
    bcopy( header.appendix, &AppendixInfo, sizeof( TIM_Appendix ) ) ;
    if ( header.n_planes == 1 && header.bits_per_pixel == 8 ) {
		Mode = MDU_TEX_FLAG_COLOR256 ;
		return 0 ;
    } else {
		if ( header.n_planes == 4 && header.bits_per_pixel == 1 ) {
			int	i ;
	    
			Mode = MDU_TEX_FLAG_COLOR16 ;
			for ( i = 0; i < 48; i ++ ) 
				Palette16[ i ] = header.header_palette[ i ] ;
			BytesPerLine16 = header.bytes_per_line ;
			return 0 ;
		} else {
			fputs( "illegal mode \n", stderr ) ;
			return -1 ;
		}
    }    
    return 0 ;
}

static	void	PcxGetColor( buf, tex )
void		*buf ;
MDU_Tex		*tex ;
{
    int		i ;
    u_char	*src ;
    RGBQUAD	*rgb ;

    rgb = ( RGBQUAD * )tex->clut.data ;
    if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
		for ( i = 0; i < 16; i ++ ) {
			rgb->r = Palette16[ i * 3 ] ;
			rgb->g = Palette16[ i * 3 + 1 ] ;
			rgb->b = Palette16[ i * 3 + 2 ] ;
			rgb->alpha = 128 ;
			rgb ++ ;
		}
    } else {
		src = buf + Size - 3 * 256 ;
		for ( i = 0; i < 256; i ++ ) {
			rgb->r = *( src ++ ) ;
			rgb->g = *( src ++ ) ;
			rgb->b = *( src ++ ) ;
			rgb->alpha = 128 ;
			rgb ++ ;
		}
    }
}

static	void	PcxGetPixel( buf, tex )
void		*buf ;
MDU_Tex		*tex ;
{
    int		w, h ;
    u_char	*src, *bits, c1, c2, data, code ;
    u_int	*dst ;
    int		i, j, s, len ;

    src = ( u_char * )buf ;
    dst = ( u_int * )tex->data.data ;
    w = Width ; h = Height ;

    if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
		i = BytesPerLine16 * h * 4;
		bits = ( u_char * )MDU_Alloc( sizeof( u_char ) * i * 8 ) ;
		if ( bits == NULL ) return ;
		do {
			code = *( src ++ ) ;
			if ( code <= 192 ) {
				len = 1 ;
			} else {
				len = code - 192 ;
				code = *( src ++ ) ;
			}
			i -= len ;
			while ( -- len >= 0 ) {
				data = code ;
				for ( j = 0; j < 8; j++ ) {
					*( bits ++ ) = ( data & 0x80 ) / 128 ;
					data = data << 1 ;
				}
			}
		} while ( i > 0 ) ;
		if ( i < 0 ) {
			fputs( "total error ?\n", stderr ) ;
			return ;
		}
		bits -= BytesPerLine16 * h * 32 ;
		if ( w % 2 ) w ++ ;
		for ( i = 0; i < h; i ++ ) {	
			s = i * ( BytesPerLine16 * 8 * 4 ) ;
			for ( j = s; j < s + w; j += 2 ) {
				c1 = bits[ j ] * 1
					+ bits[ j + BytesPerLine16 * 8 ] * 2
						+ bits[ j + BytesPerLine16 * 16 ] * 4
							+ bits[ j + BytesPerLine16 * 24 ] * 8 ;
				*( dst ++ ) = c1 ;
				c2 = bits[ j + 1 ] * 1
					+ bits[ j + 1 + BytesPerLine16 * 8 ] * 2
						+ bits[ j + 1 + BytesPerLine16 * 16 ] * 4
							+ bits[ j + 1 + BytesPerLine16 * 24 ] * 8 ;
				*( dst ++ ) = c2 ;
			}
		}
		MDU_Free( bits ) ;
    } else {
		i = w * h ;
		do {
			code = *( src ++ ) ;
			if ( code <= 192 ) {
				len = 1 ;
			} else {
				len = code - 192 ;
				code = *( src ++ ) ;
			}
			i -= len ;
			while ( -- len >= 0 ) {
				*( dst ++ ) = code ;
			}
		} while ( i > 0 ) ;
		if ( i < 0 ) {
			fputs( "total error ?\n", stderr ) ;
			return ;
		}	
    }
}

MDU_Tex		*MDU_LoadPcx( name )
char		*name ;
{
    FILE	*fp ;
    MDU_Tex	*tex ;
    void	*buf ;
    int		where ;

    fp = NULL ; tex = NULL ; buf = NULL ;
    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) goto pcx_load_error ;
    tex = MDU_FindTexture( 0 ) ;
    if ( tex == NULL ) goto pcx_load_error ;
    if ( PcxReadHeader( fp ) < 0 ) goto pcx_load_error ;
    where = ftell( fp ) ;
    fseek( fp, 0, SEEK_END ) ;
    Size = ftell( fp ) - where ;
    fseek( fp, where, SEEK_SET ) ;
    buf = ( u_char * )MDU_Alloc( sizeof( u_char ) * Size ) ;
    if ( buf == NULL ) goto pcx_load_error ;
    fread( buf, Size, 1, fp ) ;
    fclose( fp ) ;

    MDU_AllocTex( tex, Width, Height ) ;
    tex->id = MDU_GetStrCode( name ) ;
    PcxGetColor( buf, tex ) ;
    PcxGetPixel( buf, tex ) ;
    if ( AppendixInfo.stamp == PCXFILE_STAMP ) {
		if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
			tex->flag = ( 0x70 & AppendixInfo.flag ) | 0x08 ;
			tex->n_colors = 16 ;
			tex->data.w = Width / 2 ;
		} else {
			tex->flag = ( 0x70 & AppendixInfo.flag ) | 0x09 ;
			tex->n_colors = 256 ;
			tex->data.w = Width ;
		}
		tex->data.dx = AppendixInfo.px ;
		tex->data.dy = AppendixInfo.py ;
		tex->data.h = Height ;
		tex->clut.dx = AppendixInfo.cx ;
		tex->clut.dx = AppendixInfo.cy ;
		tex->clut.w = tex->n_colors ;
		tex->clut.h = 1 ;
    } else {
		if ( Mode == MDU_TEX_FLAG_COLOR16 ) {
			tex->flag = 0x08 ;
			tex->n_colors = 16 ;
			tex->data.w = Width / 2 ;
		} else {
			tex->flag = 0x09 ;
			tex->n_colors = 256 ;
			tex->data.w = Width ;
		}
		tex->data.dx = 0 ;
		tex->data.dy = 0 ;
		tex->data.h = Height ;
		tex->clut.dx = 0 ;
		tex->clut.dx = 0 ;
		tex->clut.w = tex->n_colors ;
		tex->clut.h = 1 ;
    }
    MDU_Free( buf ) ;
    return tex ;

	pcx_load_error :    
    if ( fp != NULL ) fclose( fp ) ;
    if ( tex != NULL ) MDU_ClearTexture( tex ) ;
    if ( buf != NULL ) MDU_Free( buf ) ;
    return NULL ;    
}

/*-------------------------------------------------------------------*/

/* ＰＩＣファイルのロード */
static	u_char	EncodeType ;
static	u_char	PacketChained ;
static	u_char	PacketChannel ;
static	u_char	*Alpha ;

static	int	PicReadHeader( fp )
FILE		*fp ;
{
    PIC_Header	header ;
    
    fread( &header, sizeof( PIC_Header ), 1, fp ) ;
    /* pic は BIG_ENDIAN FORMAT */
#ifdef IS_LITTLE_ENDIAN
    MDU_EndianLongForce( ( u_long * )&header, 2 ) ;
    MDU_EndianLongForce( &header.id, 1 ) ;
    MDU_EndianLongForce( &header.ratio, 1 ) ;
    MDU_EndianShortForce( &header.width, 2 ) ;
    MDU_EndianShortForce( &header.fields, 2 ) ;
#endif
#if 1
    printf( "magic %ld\n", header.magic ) ;
    printf( "id %ld\n", header.id ) ;
    printf( "ratio %ld\n", header.ratio ) ;
    printf( "w, h %d %d\n", header.width, header.height ) ;
    printf( "type %d\n", header.type ) ;
    printf( "chained %d\n", header.chained ) ;
    printf( "fields %d\n", header.fields ) ;
    printf( "channel %d\n", header.channel ) ;
#endif
    if ( header.magic != 1400960564 ) return -1 ;
    if ( header.id != 1346978644 ) return -1 ;
	//    if ( header.ratio != 1065353216 ) return -1 ;
    if ( header.type != 0 && header.type != 2 ) return -1 ;
    if ( header.fields != 3 ) return -1 ;
	//    if ( header.channel != 224 ) return -1 ;
    PacketChannel = header.channel ;
    
    Width = header.width ;
    Height = header.height ;
    EncodeType = header.type ;
    PacketChained = header.chained ;
    return 1 ;
}

static void	*PicGetBuf( fp )
FILE		*fp ;
{
    int		where, size ;
    void	*buf ;
    
    where = ftell( fp ) ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) - where ;
    fseek( fp, where, SEEK_SET ) ;
    buf = MDU_Alloc( size + 1 ) ;
    if ( buf == NULL ) exit(0) ;
    fread( buf, size, 1, fp ) ;
    fclose( fp ) ;
    return buf ;
}

static u_char	*PicMeltEncode( buf )
void		*buf ;
{
    u_char		*melt, *dst, *src, tmp ;
    int			length, i, j, count ;
    u_int		len, w, h ;
    u_char		code, r, g, b ;
    u_char		*alpha, *adst ;
	    
    w = Width ; h = Height ;
    length = w * h ;
    melt = ( u_char * )MDU_Alloc( sizeof( u_char ) * length * 3 + 1 ) ;
    if ( melt == NULL ) return NULL ;
    Alpha = alpha = ( u_char * )MDU_Alloc( sizeof( u_char ) * length * 3 + 1 ) ;
    if ( alpha == NULL ) return NULL ;
    for ( i = 0; i < length; i ++ ) ( *alpha ++ ) = 128 ; /* 初期設定 */
    adst = alpha = Alpha ;

    dst = melt ; src = ( u_char * )buf ;
    
    count = 0 ;
    if ( PacketChained ) {
		for ( i = 0 ; i < 4; i ++ ) {
			tmp = *( src ++ ) ;
			count ++ ;
		}
    }
    if ( !EncodeType ) {
		if ( PacketChannel == 224 ) {
			for ( i = 0; i < h; i ++ ) {
				j = w * 3 ;
				while ( j > 0 ) {
					*( dst ++ ) = *( src ++ ) ;
					count ++ ;
					j -- ;
				}
				if ( PacketChained ) {
					j = w ;
					while ( j > 0 ) {
						tmp = *( src ++ ) ;
						count ++ ;
						j -- ;
					}
				}
			}
			return melt ;
		} else if ( PacketChannel == 240 ) {
			for ( i = 0; i < h; i ++ ) {
				j = w ;
				while( j > 0 ) {
					*( dst ++ ) = *( src ++ ) ;
					*( dst ++ ) = *( src ++ ) ;
					*( dst ++ ) = *( src ++ ) ;
					*( adst ++ ) = *( src ++ ) ;
					j -- ;
				}
			}
			return melt ;
		}
    }
    for ( i = 0; i < h; i ++ ) {
		length = w ;
		while ( length > 0 ) {
			code = *( src ++ ) ;
			count ++ ;
			if ( code < 128 ) {
				len = code + 1 ;
				length -= len ;
				while ( len > 0 ) {
					*( dst ++ ) = *src ; src ++ ;
					*( dst ++ ) = *src ; src ++ ;
					*( dst ++ ) = *src ; src ++ ;
					count += 3 ;
					len -- ;
				}
			} else {
				if ( code == 128 ) {
					len = *( src ++ ) * 256 ;
					count ++ ;
					len += *( src ++ ) ;
					count ++ ;
				}
				else len = code - 127 ;
				r = *( src ++ ) ;
				g = *( src ++ ) ;
				b = *( src ++ ) ;
				count += 3 ;
				length -= len ;
				while ( len > 0 ) {
					*( dst ++ ) = r ;
					*( dst ++ ) = g ;
					*( dst ++ ) = b ;
					len -- ;
				}
			}
			if ( length < 0 ) {printf("%d\n", length) ; return NULL;}
		}
		/*	Alpha Channel 値を取得	*/
		if ( PacketChained ) {
			unsigned char 	code ;
			int			a_len, ll ;
	    
			//	    alpha = ( unsigned char * )MDU_Alloc( sizeof( unsigned char ) * w ) ;
			//	    if ( alpha == NULL ) return NULL ;
			a_len = w ;
			while ( a_len > 0 ) {
				code = *( src ++ ) ;
				count ++ ;
				if ( code < 128 ) {
					ll = code + 1 ;
					a_len -= ll ;
					while ( ll > 0 ) {
						*( adst ++ ) = *( src ++ ) ;
						count ++ ;
						ll -- ;
					}
				} else {
					if ( code == 128 ) {
						ll= *( src ++ ) * 256 ;
						count ++ ;
						ll += *( src ++ ) ;
						count ++ ;
					}
					else ll = code - 127 ;
		    
					a_len -= ll ;
					tmp = *( src ++ ) ;
					count ++ ;
					while ( ll > 0 ) {
						*( adst ++ ) = tmp ;
						ll -- ;
					}
				}
				if ( a_len < 0 ) {printf("?????\n") ; return NULL;}	    
			}
			//	    MDU_Free( alpha ) ;
		}
    }
    MDU_Free( buf ) ;
    return melt ;
}	

static int	PicSetData( tex, buf, name )
MDU_Tex		*tex ;
u_char		*buf ;
char		*name ;
{
    RGBQUAD	color, *clut ;
    u_int	*data ;
    int		c, i, j, length, width, w ;
    int		alpha16 ;
    u_char	*alpha, alp ;

    alpha16 = 0 ;
	check_color_start :
    width = Width ;
    if ( Width % 2 ) Width += 1 ;
    w = 0 ;
    length = Width * Height ;
    clut = ( RGBQUAD * )tex->clut.data ;
    data = ( u_int * )tex->data.data ;
    N_Colors = 0 ;
    alpha = Alpha ;

    for ( i = 0; i < length; i ++ ) {
		c = 1 ;
		color.r = buf[ i * 3 ] ;
		color.g = buf[ i * 3 + 1 ] ;
		color.b = buf[ i * 3 + 2 ] ;
		//	color.r = color.g = color.b = 128 ;
		if ( PacketChained ) {	
			if ( MDU_FindString( name, "_alp" ) >= 0 ) {
				if ( alpha16 == 1 ) {
					alp = *( alpha ++ ) ;
					alp >>= 4 ; alp &= 0xf ;
					alp = alp * 128 / 15 ;
				} else {
					alp = *( alpha ++ ) ;
					if ( MDU_FindString( name, "_alpx2" ) < 0 ) {
						alp = alp * 128 / 255 ;
					}
				}
			} else {
				alp = 128 ;
			}
			color.alpha = alp ;
		} else {
			color.alpha = 128 ;
		}
		for ( j = 0; j < N_Colors; j ++ ) {
			if ( color.r == Colors[ j ].r
				&& color.g == Colors[ j ].g
				&& color.b == Colors[ j ].b 
				&& color.alpha == Colors[ j ].alpha ) { 
				c = 0 ;
				*( data ++ ) = j ;
				break ;
			} 
		}
		if ( c ) {
			if ( N_Colors >= 256 ) {
				if ( MDU_FindString( name, "_alp" ) >= 0 &&
					alpha16 == 0 ) {
					/* アルファ１６段階モードで変換しなおす */
					printf( "%s : alpha -> 16 levels\n", name ) ;
					alpha16 = 1 ;
					goto check_color_start ;
				}
				printf( "%s : too many colors. \n", name ) ;
				return -1 ;
			}
			clut->r = Colors[ N_Colors ].r = color.r ;
			clut->g = Colors[ N_Colors ].g = color.g ;
			clut->b = Colors[ N_Colors ].b = color.b ;
			clut->alpha = Colors[ N_Colors ].alpha = color.alpha ;
			*( data ++ ) = N_Colors ;
			clut ++ ;
			N_Colors ++ ;
		}
		if ( ++ w == width ) {
			if ( Width % 2 ) {
				/* パディングする */
				*data = *( data - 1 ) ;
				data ++ ; i ++ ;
			}
			w = 0 ;
		}
    }
#if 0
    {
		int	i ;
		RGBQUAD	*c ;

		c = ( RGBQUAD * )tex->clut.data ;
		i = 256 ;
		while( -- i >= 0 ) {
			printf( "%x %d %d %d %d\n", c, c->r, c->g, c->b, c->alpha ) ;
			c ++ ;
		}
    }
#endif
    if ( N_Colors > 16 ) { 
		if ( N_Colors != 256 ) {
			for ( i = N_Colors; i < 256 ; i ++ ) {
				clut->r = 0;
				clut->g = 0;
				clut->b = 0;
				clut->alpha = 0;
				clut ++ ;
			}
		}
		Mode = MDU_TEX_FLAG_COLOR256 ;
		tex->n_colors = 256 ;
		tex->data.w = Width ;
    } else {
		if ( N_Colors != 16 ) {
			for ( i = N_Colors; i < 16 ; i ++ ) {
				clut->r = 0;
				clut->g = 0;
				clut->b = 0;
				clut->alpha = 0;
				clut ++ ;
			}
		}
		Mode = MDU_TEX_FLAG_COLOR16 ;
		tex->n_colors = 16 ;
		tex->data.w = Width / 2 ;
    }
    tex->flag = MDU_TEX_FLAG_CLUT_EXIST | Mode ;
    tex->width = Width ; tex->height = Height ;
    tex->data.dx = tex->data.dy = 0 ;
    tex->clut.dx = tex->data.dy = 0 ;
    tex->data.h = Height ;
    tex->clut.w = tex->n_colors ;
    tex->clut.h = 1 ;
#if 0
    {
		int	i, j ;
		u_int	*data ;
		RGBQUAD	*clut ;

		data = ( u_int * )tex->data.data ;
		for ( j = 0; j < Height; j ++ ) {
			for ( i = 0; i < Width; i ++ ) {
				printf( "%d ", data[ i + j * Width ] ) ;
			}
			printf( "\n" ) ;
		}
		clut = ( RGBQUAD * )tex->clut.data ;
		for ( i = 0; i < 16; i ++ ) {	
			printf( "%d %d %d\n", clut->r, clut->g, clut->b ) ;
			clut ++ ;
		}
    }
#endif
    return 0 ;
}

MDU_Tex		*MDU_LoadPic( name )
char		*name ;
{
    FILE		*fp ;
    MDU_Tex		*tex ;
    void		*buf ;

    fp = NULL ;
    tex = NULL ;
    buf = NULL ;
    Alpha = NULL ;
    if ( ( fp = fopen( name, "rb" ) ) == NULL ) goto pic_load_error ;
    if ( PicReadHeader( fp ) != 1 ) goto pic_load_error ;
	//    if ( Width % 2 ) {
		//	printf( "texture width error.\n" ) ;
		//	goto pic_load_error ;
		//    }
    tex = MDU_FindTexture( 0 ) ;
    if ( tex == NULL ) goto pic_load_error ;
    MDU_AllocTex( tex, Width + Width % 2, Height ) ;
    tex->id = MDU_GetStrCode( name ) ;
    buf = PicGetBuf( fp ) ;
    if ( buf == NULL ) goto pic_load_error ;
    buf = ( void * )PicMeltEncode( buf ) ;
    if ( buf == NULL ) goto pic_load_error ;
    if ( PicSetData( tex, buf, name ) < 0 ) goto pic_load_error ;
    MDU_Free( buf ) ;
    if ( Alpha != NULL ) MDU_Free( Alpha ) ;
    return tex ;

pic_load_error :
    if ( fp != NULL ) fclose( fp ) ;
    if ( buf != NULL ) MDU_Free( buf ) ;
    if ( tex != NULL ) MDU_ClearTexture( tex ) ;
    if ( Alpha != NULL ) MDU_Free( Alpha ) ;
    return NULL ;
}
