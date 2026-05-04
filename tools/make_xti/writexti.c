/*
	writexti.c
	XTI形式での書きだし

	2002.04.11 K.Takabe
	$Id: writexti.c,v 1.4 2002/08/06 09:46:37 usr02774 Exp $

*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "loadinc.h"

extern int		MaxTexpack ;
extern TEX_INFO	TexpackList[ 16 ] ;
extern int				MaxClutEntry ;
extern unsigned int	ClutEntry[ 256 * 4096 ];

static void InitSwizzleParam( int width, int height, int dpp );
static int SwizzleU( int num );
static int SwizzleV( int num );
static int SwizzleW( int num );

void WriteXboxTextureImageFile( char *filename, int option_flag )
{
	TRI_FILEHEADER	header ;
	TEX_INFO	*tpk_info ;
	DG_TEX		*tex ;
	int			total_n_textures, texel_offset, header_size ;
	int			i, j, num ;
	FILE		*fp ;

	total_n_textures = 0 ;
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		total_n_textures += TexpackList[ i ].n_textures ;
	}
	printf("texpack:%d tex:%d\n", MaxTexpack, total_n_textures );

	/* ヘッダー情報構築 */
	memset( &header, 0, sizeof( TRI_FILEHEADER ) );
	header.n_textures = total_n_textures ;
	header.n_info = MaxTexpack ;
	header.compress_flag = option_flag ;
	header.n_clut = MaxClutEntry ;
	/* データの先頭が8qword境界に揃うようにする */
	header_size = sizeof(TRI_FILEHEADER) ;
	header_size += sizeof( DG_TEX ) * header.n_textures ;
	header.info_addr = (void*)header_size ;
	header_size += sizeof( XTI_TEXINFO ) * MaxTexpack ;
	header_size = ( header_size + 127 ) & 0xffff80 ;
	/* CLUT */
	header.clut_addr = (void*)header_size ;
	header_size += 4 * 256 * MaxClutEntry ;
	header_size = ( header_size + 127 ) & 0xffff80 ;

	if ( ( fp = fopen( filename, "wb" ) ) == NULL ) return ;
	fwrite( &header, sizeof( TRI_FILEHEADER ), 1, fp );

	/* テクスチャ情報の書き出し */
	num = 0 ;
	while ( ( tpk_info = GetTexpackInfo( num ) ) != NULL ){
		fwrite( tpk_info->textures, sizeof( DG_TEX ), tpk_info->n_textures, fp );
		num++ ;
	}

	/* 各テクスチャの適切サイズを算出 */
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		int		max_y ;
		tpk_info = &TexpackList[ i ] ;
		if ( tpk_info->mip_level == 1 ){
			switch ( tpk_info->mode ){
			case TEXPACK_FORMAT_A8R8G8B8:
			case TEXPACK_FORMAT_A8G8G8R8:
				tpk_info->image_size = tpk_info->width * tpk_info->height * 4 ;
				break ;
			case TEXPACK_FORMAT_A4R4G4B4:
			case TEXPACK_FORMAT_A1R5G5B5:
			case TEXPACK_FORMAT_A1G5G5R5:
			case TEXPACK_FORMAT_V8U8:
				tpk_info->image_size = tpk_info->width * tpk_info->height * 2 ;
				break ;
			case TEXPACK_FORMAT_PAL4:
			case TEXPACK_FORMAT_PAL4AGBR:
				tpk_info->image_size = tpk_info->width * tpk_info->height / 2 ;
				break ;
			case TEXPACK_FORMAT_PAL8AGBR:
				tpk_info->image_size = tpk_info->width * tpk_info->height ;
				break ;
			case TEXPACK_FORMAT_PAL8:
				tpk_info->image_size = tpk_info->width * tpk_info->height ;
				{/* XBOX形式への変換（スィズル化） */
					int		x, y ;
					int		offset, y_offset, x_offset ;
					unsigned char	*src, *dst, *mem ;

					InitSwizzleParam( tpk_info->width, tpk_info->height, 1 );

					src = mem = malloc( tpk_info->width * tpk_info->height * 1 );
					memcpy( src, tpk_info->image_data, tpk_info->width * tpk_info->height * 1 );
					dst = tpk_info->image_data ;
					for ( y = 0 ; y < tpk_info->height ; y++ ){
						y_offset = SwizzleV( y ) | SwizzleW( 0 );
						for ( x = 0 ; x < tpk_info->width ; x++ ){
							x_offset = SwizzleU( x );
							offset = x_offset | y_offset ;
							dst[ offset ] = *src++ ;
						}
					}
					free( mem );
				}
				break ;
			case TEXPACK_FORMAT_DXT1:
				max_y = ( tpk_info->max_y + 3 ) & ~3 ;
				tpk_info->image_size = tpk_info->width * max_y / 2 ;
				//tpk_info->image_size = tpk_info->width * tpk_info->height / 2 ;
				break ;
			case TEXPACK_FORMAT_DXT3:
			case TEXPACK_FORMAT_DXT5:
				max_y = ( tpk_info->max_y + 3 ) & ~3 ;
				tpk_info->image_size = tpk_info->width * max_y ;
				break ;
			}
		} else {
			/* ミップマップテクスチャの場合 */
		}
		/* 128bytes境界に揃える */
		tpk_info->image_size = ( tpk_info->image_size + 127 ) & ~0x7f ;
	}

	/* グローバルテクスチャ情報の書きだし */
	texel_offset = header_size ;
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		XTI_TEXINFO	texinfo ;
		tpk_info = &TexpackList[ i ] ;
		texinfo.format = tpk_info->mode ;
		texinfo.flag = 0 ;
		texinfo.width = tpk_info->width ;
		texinfo.height = tpk_info->height ;
		texinfo.mip_level = tpk_info->mip_level ;
		texinfo.pad = 0 ;
		texinfo.use_size = tpk_info->image_size ;
		texinfo.texel_addr = (void*)texel_offset ;
		if ( MaxClutEntry == 0 ){
			texinfo.clut_addr = (void*)0 ;
		} else {
			texinfo.clut_addr = (void*)header.clut_addr ;
		}
		texel_offset += tpk_info->image_size ;
		fwrite( &texinfo, sizeof(texinfo), 1, fp );
	}

	/* CLUTデータの書きだし */
	if ( MaxClutEntry != 0 ){
		/* データの先頭が8qword境界に揃うようにする */
		for ( i = (int)header.clut_addr - ftell( fp ) ; i > 0 ; i-- ){
			fputc( 0, fp );
		}
		fwrite( ClutEntry, 4 * 256, MaxClutEntry, fp );
	}

	/* データの先頭が8qword境界に揃うようにする */
	for ( i = header_size - ftell( fp ) ; i > 0 ; i-- ){
		fputc( 0, fp );
	}


	/* テクセル情報の書きだし */
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		int		texel_size ;
		tpk_info = &TexpackList[ i ] ;
		printf("data offset (%d): %08x  size = %d\n", i, ftell( fp ), tpk_info->image_size );
		fwrite( tpk_info->image_data, 1, tpk_info->image_size, fp );
	}

	fclose( fp );

}


/* ---------------------------------------------------------------- */
/* スィズルテクスチャ生成関連 */
static unsigned int	Swizzle_Width ;
static unsigned int	Swizzle_Height ;
static unsigned int	Swizzle_Depth ;
static unsigned int	Swizzle_MaskU ;
static unsigned int	Swizzle_MaskV ;
static unsigned int	Swizzle_MaskW ;
static void InitSwizzleParam( int width, int height, int depth )
{
	unsigned int		i, j, k ;

	Swizzle_Width = width ;
	Swizzle_Height = height ;
	Swizzle_Depth = depth ;
	Swizzle_MaskU = 0 ;
	Swizzle_MaskV = 0 ;
	Swizzle_MaskW = 0 ;

	i = j = 1 ;
	do {
		k = 0 ;
		if ( i < width ){
			Swizzle_MaskU |= j ;
			k = ( j <<= 1 );
		}
		if ( i < height ){
			Swizzle_MaskV |= j ;
			k = ( j <<= 1 );
		}
		if ( i < depth ){
			Swizzle_MaskW |= j ;
			k = ( j <<= 1 );
		}
		i <<= 1 ;
	} while ( k );

}

static int SwizzleU( int num )
{
	unsigned int		r, i ;
	r = 0 ;
	for ( i = 1 ; i <= Swizzle_MaskU ; i <<= 1 ){
		if ( Swizzle_MaskU & i ){
			r |= num & i ;
		} else {
			num <<= 1 ;
		}
	}
	return ( r );
}

static int SwizzleV( int num )
{
	int		r, i ;
	r = 0 ;
	for ( i = 1 ; i <= Swizzle_MaskV ; i <<= 1 ){
		if ( Swizzle_MaskV & i ){
			r |= num & i ;
		} else {
			num <<= 1 ;
		}
	}
	return ( r );
}

static int SwizzleW( int num )
{
	int		r, i ;
	r = 0 ;
	for ( i = 1 ; i <= Swizzle_MaskW ; i <<= 1 ){
		if ( Swizzle_MaskW & i ){
			r |= num & i ;
		} else {
			num <<= 1 ;
		}
	}
	return ( r );
}


