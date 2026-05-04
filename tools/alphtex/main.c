/*
	main.c
	新ライトデータ（.lt2）ダンププログラム

	1999/10/01 K.Takabe
	2000/05/24 T.Morita fully revised
	$Id: main.c,v 1.4 2001/11/30 10:15:03 usr04098 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "format.h"

int convert( char *fname )
{
    FILE *fp ;
    BITMAPHEADER header ;
    RGBQUAD *rgb, *c ;
    int i, n_color ;

    if ( !(fp=fopen( fname, "rb+" )) )
    {
	fprintf( stderr, "file open error<%s>\n", fname ) ;
	return -1 ;
    }

    /* ヘッダを読む */
    fread( &header.header.bfType, sizeof(BITMAPHEADER)-2, 1, fp ) ;
    n_color = (int)pow( 2.0f, (double)header.info.biBitCount ) ;

    /* パレットを読む */
    rgb = malloc( sizeof(RGBQUAD)*n_color ) ;
    fread( rgb, sizeof(RGBQUAD), n_color, fp ) ;

    /* パレットを変更 */
    for ( i=n_color, c=rgb ; --i>=0 ; c++ )
	c->r = c->g = c->b = c->alpha,
	    printf( "%3d %3d %3d %3d\n", c->r, c->g, c->b, c->alpha ) ;

    /* パレットを書き込む */
    fseek( fp, 54, SEEK_SET ) ;
    fwrite( rgb, sizeof(RGBQUAD), n_color, fp ) ;

    fclose( fp ) ;
}


int convert_msk( char *fname, int mode )
{
    FILE *fp ;
    BITMAPHEADER header ;
    RGBQUAD *rgb, *c ;
    int i, n_color ;

    if ( !(fp=fopen( fname, "rb+" )) )
    {
	fprintf( stderr, "file open error<%s>\n", fname ) ;
	return -1 ;
    }

    /* ヘッダを読む */
    fread( &header.header.bfType, sizeof(BITMAPHEADER)-2, 1, fp ) ;
    n_color = (int)pow( 2.0f, (double)header.info.biBitCount ) ;

    /* パレットを読む */
    rgb = malloc( sizeof(RGBQUAD)*n_color ) ;
    fread( rgb, sizeof(RGBQUAD), n_color, fp ) ;

    /* パレットを変更 */
    for ( i=n_color, c=rgb ; --i>=0 ; c++ )
    {
	int alpha ;

	if ( mode )
	    alpha = (c->r?1:0)+(c->g?1:0)+(c->b?1:0), alpha = (c->r + c->g + c->b)/(alpha? alpha : 1) ;
	else
	    (alpha = c->r>c->g ? c->r : c->g), (alpha = alpha>c->b ? alpha : c->b) ;
	c->alpha = alpha>128 ? 128 : alpha ;
	printf( "%3d %3d %3d %3d\n", c->r, c->g, c->b, c->alpha ) ;
    }

    /* パレットを書き込む */
    fseek( fp, 54, SEEK_SET ) ;
    fwrite( rgb, sizeof(RGBQUAD), n_color, fp ) ;

    fclose( fp ) ;
}



void		MDU_EndianLongForce( ptr, n )
u_long		*ptr ;
int		n ;
{
    u_long	val, byte0, byte1, byte2, byte3 ;

    if ( ptr == NULL ) return ;
    while ( -- n >= 0 ) {
	val = *ptr ;
	byte0 = ( 0x000000ff & val ) << 24 ;
	byte1 = ( 0x0000ff00 & val ) << 8 ;
	byte2 = ( 0x00ff0000 & val ) >> 8 ;
	byte3 = ( 0xff000000 & val ) >> 24 ;
	*( ptr ++ ) = byte0 | byte1 | byte2 | byte3 ;
    }
}

void		MDU_EndianShortForce( ptr, n )
u_short		*ptr ;
int		n ;
{
    u_short		val, byte0, byte1 ;

    if ( ptr == NULL ) return ;
    while ( -- n >= 0 ) {
	val = *ptr ;
	byte0 = ( 0x00ff & val ) << 8 ;
	byte1 = ( 0xff00 & val ) >> 8 ;
	*( ptr ++ ) = byte0 | byte1 ;
    }
}

int display( char *fname, int pallet_flg )
{
    FILE *fp ;
    RGBQUAD *rgb=NULL, *c ;
    int i, n_color ;

    if ( !(fp=fopen( fname, "rb" )) )
    {
	fprintf( stderr, "file open error<%s>\n", fname ) ;
	return -1 ;
    }

    if ( (fname = strchr( fname, '.' )) )
	if ( !strcmp( fname, ".bmp" ) )
	{
	    BITMAPHEADER header ;

	    fread( &header.header.bfType, sizeof(BITMAPHEADER)-2, 1, fp ) ;
	    n_color = (int)pow( 2.0f, (double)header.info.biBitCount ) ;
	    printf( "color %d\nbit %d\nwidth %d\nheight %d\n",
		    n_color, header.info.biBitCount, header.info.biWidth, header.info.biHeight ) ;
	    if ( header.info.biBitCount <= 16 && pallet_flg )
	    {
		rgb = malloc( sizeof(RGBQUAD)*n_color ) ;
		fread( rgb, sizeof(RGBQUAD), n_color, fp ) ;
	    }
	}
	else if ( !strcmp( fname, ".pic" ) )
	{
	    PIC_Header header ;

	    fread( &header, sizeof(PIC_Header), 1, fp ) ;
	    MDU_EndianLongForce( ( u_long * )&header, 2 ) ;
	    MDU_EndianLongForce( &header.id, 1 ) ;
	    MDU_EndianLongForce( &header.ratio, 1 ) ;
	    MDU_EndianShortForce( &header.width, 2 ) ;
	    MDU_EndianShortForce( &header.fields, 2 ) ;

	    n_color = (int)pow( 2.0f, (double)header.type ) ;
	    printf( "color %d\ntyp %d chaned %d chanel%d\nwidth %d\nheight %d\n",
		    n_color,
		    header.type, header.chained, header.channel, header.width, header.height ) ;
	}
    if ( rgb )
    {
	for ( i=n_color, c=rgb ; --i>=0 ; c++ )
	    printf( "%3d %3d %3d %3d\n", c->r, c->g, c->b, c->alpha ) ;
	free( rgb ) ;
    }
    fclose( fp ) ;
}


int main( int argc, char **argv )
{
    int	  n_files = 0 ;
    char *files[256], **f ;
    int	  mode = 0 ;
    int   ret ;

    for ( argv++ ; --argc > 0 ; argv++ )
    {
	if ( argv[0][0] == '-' )
	    switch ( argv[0][1] )
	    {
	    case 'm':
		if ( argv[0][2] == 'a' )
		    mode = 3 ;
		else
		    mode = 4 ;
		break ;
	    case 'p':
		mode = 2 ;
		break ;
	    case 't':
		mode = 1 ;
		break ;
	    default:/* 入力ファイル取得 */
		n_files = -1 ;
		break ;
	    }
	else if ( n_files < 256 )
	    files[n_files++] = argv[0] ;
    }

    /* エラーチェック */
    if ( n_files < 1 || n_files > 255 )
    {
	fprintf( stderr,
		 "Usage: alphtex (-options) <file(*.bmp)> \n"
		 "make alpha texture from BMP-format \n"
		 "\toptions -t: show texture info\n"
		 "\t        -ma: make mask alphabmp by avarage RGB\n"
		 "\t        -mx: make mask alphabmp by heighest RGB\n"
		 "\t        -p: show texture info and pallet\n\n" );
	return -1 ;
    }

    for ( f=files ; --n_files>=0 ; f++ )
	switch( mode )
	{
	case 4:
	    if ( (ret = convert_msk( *f, 0 )) )
		return ret ;
	    break ;
	case 3:
	    if ( (ret = convert_msk( *f, 1 )) )
		return ret ;
	    break ;
	case 2:
	    if ( (ret = display( *f, 1 )) )
		return ret ;
	    break ;
	case 1:
	    if ( (ret = display( *f, 0 )) )
		return ret ;
	    break ;
	default:
	    if ( (ret = convert( *f )) )
		return ret ;
	}
    return 0 ;
}
