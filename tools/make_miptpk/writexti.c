/*
	writexti.c
	XTI形式での書きだし

	2002.04.11 K.Takabe
	$Id: writexti.c,v 1.1 2002/06/19 05:48:55 usr02774 Exp $

*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "loadinc.h"

int				LocalMaxTexpack ;
extern int		MaxTexpack ;
extern TEX_INFO	TexpackList[ 16 ] ;

/* ミップマップを実現するためにレベル順にソートを行なう */
static void SortTexInfo( void )
{
	int		i, j ;
	TEX_INFO	tmp_texinfo ;
	for ( i = MaxTexpack ; i > 1 ; i-- ){
		for ( j = 0 ; j < ( i - 1 ) ; j++ ){
			if ( TexpackList[ j ].image_size < TexpackList[ j + 1 ].image_size ){
				tmp_texinfo = TexpackList[ j ] ;
				TexpackList[ j ] = TexpackList[ j + 1 ] ;
				TexpackList[ j + 1 ] = tmp_texinfo ;
			}
		}
	}
	/* DG_TEXの参照テクスチャ番号を補正する */
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		DG_TEX		*tex ;
		tex = TexpackList[ i ].textures ;
		for ( j = 0 ; j < TexpackList[ i ].n_textures ; j++, tex++ ){
			tex->texinfonum = i ;
		}
	}
}

void WriteXboxTextureImageFile( char *filename, int option_flag )
{
	TRI_FILEHEADER	header ;
	TEX_INFO	*tpk_info ;
	DG_TEX		*tex ;
	int			total_n_textures, texel_offset, header_size, total_image_size ;
	int			i, num ;
	FILE		*fp ;

	/* ミップマップのためにソートを行なう */
	SortTexInfo();
	LocalMaxTexpack = MaxTexpack ;
	MaxTexpack = 1 ;

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
	/* データの先頭が8qword境界に揃うようにする */
	header_size = sizeof(TRI_FILEHEADER) ;
	header_size += sizeof( DG_TEX ) * header.n_textures ;
	header.info_addr = (void*)header_size ;
	header_size += sizeof( XTI_TEXINFO ) * MaxTexpack ;
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
	total_image_size = 0 ;
	for ( i = 0 ; i < LocalMaxTexpack ; i++ ){
		int		max_y ;
		tpk_info = &TexpackList[ i ] ;
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
		case TEXPACK_FORMAT_PAL8:
		case TEXPACK_FORMAT_PAL8AGBR:
			tpk_info->image_size = tpk_info->width * tpk_info->height ;
			break ;
		case TEXPACK_FORMAT_DXT1:
			max_y = ( tpk_info->max_y + 3 ) & ~3 ;
			//tpk_info->image_size = tpk_info->width * max_y / 2 ;
			tpk_info->image_size = tpk_info->width * tpk_info->height / 2 ;
			break ;
		case TEXPACK_FORMAT_DXT3:
		case TEXPACK_FORMAT_DXT5:
			max_y = ( tpk_info->max_y + 3 ) & ~3 ;
			//tpk_info->image_size = tpk_info->width * max_y ;
			tpk_info->image_size = tpk_info->width * tpk_info->height ;
			break ;
		}
		/* 128bytes境界に揃える */
		//tpk_info->image_size = ( tpk_info->image_size + 127 ) & ~0x7f ;
		total_image_size += tpk_info->image_size ;
	}
	total_image_size = ( total_image_size + 127 ) & ~0x7f ;

	/* グローバルテクスチャ情報の書きだし */
	texel_offset = header_size ;
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		XTI_TEXINFO	texinfo ;
		tpk_info = &TexpackList[ i ] ;
		texinfo.format = tpk_info->mode ;
		texinfo.flag = 0 ;
		texinfo.width = tpk_info->width ;
		texinfo.height = tpk_info->height ;
		//texinfo.mip_level = tpk_info->mip_level ;
		texinfo.mip_level = LocalMaxTexpack ;
		texinfo.pad = 0 ;
		//texinfo.use_size = tpk_info->image_size ;
		texinfo.use_size = total_image_size ;
		texinfo.texel_addr = (void*)texel_offset ;
		texinfo.clut_addr = (void*)0 ;
		//texel_offset += tpk_info->image_size ;
		texel_offset += total_image_size ;
		fwrite( &texinfo, sizeof(texinfo), 1, fp );
	}

	/* データの先頭が8qword境界に揃うようにする */
	for ( i = header_size - ftell( fp ) ; i > 0 ; i-- ){
		fputc( 0, fp );
	}

	/* テクセル情報の書きだし */
	for ( i = 0 ; i < LocalMaxTexpack ; i++ ){
		int		texel_size ;
		tpk_info = &TexpackList[ i ] ;
		printf("data offset (%d): %08x\n", i, ftell( fp ) );
		fwrite( tpk_info->image_data, 1, tpk_info->image_size, fp );
	}

	fclose( fp );

}

