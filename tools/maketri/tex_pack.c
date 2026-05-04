/*
    tex_pac.c
    テクスチャのＶＲＡＭベタ形式ファイルへの変換 [[VRAM texture format file conversion to solid]]
	1999.03.30 K.Takabe		1999.11.17 M.Matsuzaki
	$Id: tex_pack.c,v 1.4 2001/12/04 10:01:57 usr02774 Exp $

 */

/* 
   1999.11.17更新	（松崎）
   幾つかの4bitCLUTをまとめ,8bitCLUTとして同一ブロックへ
   一括して流し込むための処理を追加しております。

	コメント内での「ブロックセット」という呼び方を「ページ」に変更

   [[Update 1999.11.17 (Matsuzaki)
   Some 4bitCLUT together, 8bitCLUT we add a process for pouring into the same block as in bulk.

   In the comment "block set" call it a "page" to change]]
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "loadinc.h"

/* （松崎）1999.11.17変更点スイッチ [[(Matsuzaki) 1999.11.17 Switch changes]] */
#define PACK_4BIT_CLUT

static long32				TopBaseAddress = 0 ;
static long32				MaxWriteBase = 0 ;
static unsigned long32		VramImage[ 1024*1024 ] ;

/* ページのサイズテーブル [[Page size table]] */
static long32		block_size[4][2] = {{32,16},{16,16},{16,8},{8,8}};

/* ページを６４×３２の３２ビットイメージデータに変換する [[Convert to page a 64x32 32-bit image]] */
static void ConvertBlockSetToImageData( unsigned long32 *src, unsigned long32 *dst );

/* 8bitCLUT(IDTEX8)にパックした4bitCLUT(IDTEX4)を使用する為の
   TEX0のCBA値テーブル (松崎)
   [[8bitCLUT (IDTEX8) packed with 4bitCLUT (IDTEX4) TEX0 use of CBA for the table (Matsuzaki)]] */
#ifdef PACK_4BIT_CLUT
static int tex0csa_tbl[16] = {0,2,4,6,1,3,5,7,8,10,12,14,9,11,13,15};
#endif
/* ページ内における8ビットCLUTオフセット（ブロック単位）[[8-bit CLUT in the page offset (block unit)]] */
static short	clut_offset_table0[8] = {0,4,16,20,8,12,24,28};
/* ページ内における8ビットCLUTオフセット番号と [[8-bit CLUT in the page number and offset]] */
static short	clut_offset_table1[128] = {
	0x00,0x01,0x10,0x11,0x20,0x21,0x30,0x31,
	0x02,0x03,0x12,0x13,0x22,0x23,0x32,0x33,
	0x04,0x05,0x14,0x15,0x24,0x25,0x34,0x35,
	0x06,0x07,0x16,0x17,0x26,0x27,0x36,0x37,
	0x08,0x09,0x18,0x19,0x28,0x29,0x38,0x39,
	0x0a,0x0b,0x1a,0x1b,0x2a,0x2b,0x3a,0x3b,
	0x0c,0x0d,0x1c,0x1d,0x2c,0x2d,0x3c,0x3d,
	0x0e,0x0f,0x1e,0x1f,0x2e,0x2f,0x3e,0x3f,
	0x40,0x41,0x50,0x51,0x60,0x61,0x70,0x71,
	0x42,0x43,0x52,0x53,0x62,0x63,0x72,0x73,
	0x44,0x45,0x54,0x55,0x64,0x65,0x74,0x75,
	0x46,0x47,0x56,0x57,0x66,0x67,0x76,0x77,
	0x48,0x49,0x58,0x59,0x68,0x69,0x78,0x79,
	0x4a,0x4b,0x5a,0x5b,0x6a,0x6b,0x7a,0x7b,
	0x4c,0x4d,0x5c,0x5d,0x6c,0x6d,0x7c,0x7d,
	0x4e,0x4f,0x5e,0x5f,0x6e,0x6f,0x7e,0x7f
};

/* テクスチャを一つのＶＲＡＭベタデータに変換し、ファイルに保存する [[Solid texture data is converted into VRAM, and saved to file]] */
void WriteTextureRowImageFile( char *filename, int option_flag )
{
	TEX_INFO	*info ;
	DG_TEX		*tex ;
	int		dx, dy, width, height ;
	int		num, i, j, k, l, mode, block_num, offset ;
	unsigned long32	block_buffer[ 64 ], *d_addr, *s_addr ;
	int		clut_start_block, total_n_textures, clut_use_line ;

	TopBaseAddress = 0 ;
	MaxWriteBase = 0 ;
	memset( VramImage, 0, sizeof( long32 ) * 1024 * 1024 );

	/* イメージ部分の変換 [[Converting part of the image]] */
	total_n_textures = 0 ;
	num = 0 ;
	while ( ( info = GetTextureInfo( num ) ) != NULL ){
		total_n_textures += info->n_textures ;
		mode = info->mode & 3 ;
		/* 先頭ベースアドレス記憶 [[base memory start address]] */
		info->load_tex_base = TopBaseAddress ;
		/* ブロックのサイズを算出 [[Calculate the size of the block]] */
		dx = block_size[ mode ][ 0 ];
		dy = block_size[ mode ][ 1 ];
		/* テクスチャのブロック数を算出 [[Calculate the number of blocks of texture]] */
		width = info->width / dx ;
		height = info->height / dy ;

		/* テクスチャイメージデータの書き出し [[Export texture image data]] */
		for ( j = 0 ; j < height ; j++ ){
			for ( i = 0 ; i < width ; i++ ){
				switch ( mode ){
				case 0:
					/* １ブロック分のデータをバッファに転送 [[Transfer one block of data to the buffer] */
					offset = info->width / 8 ;
					d_addr = block_buffer ;
					s_addr = (unsigned long32*)info->image_data + i * dx / 8 + j * dy * offset ;
					for ( l = 0 ; l < 16 ; l++ ){
						for ( k = 0 ; k < 4 ; k++ ){
							*d_addr++ = s_addr[k] ;
						}
						s_addr += offset ;
					}
					/* そのブロックの該当ブロック番号を取得 [[Get the block number corresponding to the block]] */
					block_num = GetImageBlockNumber4bit( i * dx, j * dy, info->width ) ;
					/* ブロック内のデータを３２カラー形式に変換 [[32 color format data in the block]] */
					ConvertBlockImage4to32( block_buffer, &VramImage[ ( TopBaseAddress + block_num ) * 64 ] );
					break ;
				case 1:
					/* １ブロック分のデータをバッファに転送 */
					offset = info->width / 4 ;
					d_addr = block_buffer ;
					s_addr = (unsigned long32*)info->image_data + i * dx / 4 + j * dy * offset ;
					for ( l = 0 ; l < 16 ; l++ ){
						for ( k = 0 ; k < 4 ; k++ ){
							*d_addr++ = s_addr[k] ;
						}
						s_addr += offset ;
					}
					/* そのブロックの該当ブロック番号を取得 */
					block_num = GetImageBlockNumber8bit( i * dx, j * dy, info->width ) ;
					/* ブロック内のデータを３２カラー形式に変換 */
					ConvertBlockImage8to32( block_buffer, &VramImage[ ( TopBaseAddress + block_num ) * 64 ] );
					break ;
				case 2:
					/* １ブロック分のデータをバッファに転送 */
					offset = info->width / 2 ;
					d_addr = block_buffer ;
					s_addr = (unsigned long32*)info->image_data + i * dx / 2 + j * dy * offset ;
					for ( l = 0 ; l < 8 ; l++ ){
						for ( k = 0 ; k < 8 ; k++ ){
							*d_addr++ = s_addr[k] ;
						}
						s_addr += offset ;
					}
					/* そのブロックの該当ブロック番号を取得 */
					block_num = GetImageBlockNumber16bit( i * dx, j * dy, info->width ) ;
					/* ブロック内のデータを３２カラー形式に変換 */
					ConvertBlockImage16to32( block_buffer, &VramImage[ ( TopBaseAddress + block_num ) * 64 ] );
					break ;
				case 3:
					/* １ブロック分のデータをバッファに転送 */
					offset = info->width ;
					d_addr = block_buffer ;
					s_addr = (unsigned long32*)info->image_data + i * dx + j * dy * offset ;
					for ( l = 0 ; l < 8 ; l++ ){
						for ( k = 0 ; k < 8 ; k++ ){
							*d_addr++ = s_addr[k] ;
						}
						s_addr += offset ;
					}
					/* そのブロックの該当ブロック番号を取得 */
					block_num = GetImageBlockNumber32bit( i * dx, j * dy, info->width ) ;
					/* ブロック内のデータを３２カラー形式に変換 */
					ConvertBlockImage32to32( block_buffer, &VramImage[ ( TopBaseAddress + block_num ) * 64 ] );
					break ;
				}
				if ( ( ( info->max_y - 1 ) / dy ) == j )
				if ( ( TopBaseAddress + block_num ) > MaxWriteBase ) MaxWriteBase = TopBaseAddress + block_num ;
			}
		}
		/* 次回書き込みベースアドレス更新 [[Write the next base address update]] */
		TopBaseAddress = MaxWriteBase + 1 ;
		num++ ;
	}

	/* パレット情報の書き込み [[Write palette information]] */
	/*
		CLUTはページ単位で書き込みを行ない,転送サイズが極力小さくなるように
		上から横方向に1列ずつ並べていく方法に変更したので注意
      [[CLUT did a write-in pages, note the changes to the way we arranged horizontally from one row to be minimized on the transfer size]]
	 */
	{
		unsigned 	long32	*clut ;
		int 		writeaddr;
		int			n_256clut ;

		/* 念のためページ境界に揃える [[Page boundary aligned to make sure]] */
		TopBaseAddress = ( TopBaseAddress + 31 ) & (~31) ;
	
		/* ＣＬＵＴ部分の変換及びテクスチャ、ＣＬＵＴベースアドレスオフセットの設定
        [[CLUT area and texture transformation, CLUT base address offset setting]] */
		clut_start_block = TopBaseAddress;
		
		/* まず８ビットＣＬＵＴを配置 [[First, place the 8-bit CLUT]] */
		num = 0;
		n_256clut = 0 ;/* そのページ内における２５６色CLUT使用数 [[Page 256 in the number of colors used CLUT]] */
		while ( ( info = GetTextureInfo( num ) ) != NULL ){
			if((info->mode & 3)==1){
				clut = info->clut_data ;
				tex = info->textures ;
				for ( i = 0 ; i < info->n_textures ; i++ ){
					int		base_addr ;
					base_addr = TopBaseAddress + clut_offset_table0[ n_256clut ] ;
					ConvertBlockImageClut32( &clut[i*256], &VramImage[ base_addr * 64 ], 0, 16 );
					tex->tex_trans.tex0.data |= info->load_tex_base ;
 					tex->tex_trans.tex0.data |= 
						( (unsigned long64)( base_addr - clut_start_block) << 37 ) |	/* ベースアドレス [[Base address]] */
						( (unsigned long64)0 << 56 );										/* オフセット０固定 [[Fixed offset 0]] */
					tex->tex_trans.tex2.data |=
						( (unsigned long64)( base_addr - clut_start_block) << 37 ) |	/* ベースアドレス [[Base address]] */
						( (unsigned long64)0 << 56 );										/* オフセット０固定 [[Fixed offset 0]] */
					if ( ++n_256clut >= 8 ){
						TopBaseAddress += 32 ;
						n_256clut = 0 ;
					}
					tex++;
				}
			}
			num++ ;
		}

		num = 0;
		writeaddr = 0;
		while ( ( info = GetTextureInfo( num ) ) != NULL ){
			if((info->mode & 3)==0){
				clut = info->clut_data ;
				tex = info->textures ;
				for ( i = 0 ; i < info->n_textures ; i++ ){
					int		base_addr, clut_offset, tmp_offset ;

					/* すでに２５６色CLUTが書き込まれている場合にはスキップ [[256-color CLUT already been written when the skip]] */
					tmp_offset = clut_offset_table1[ writeaddr ] ;
					while ( ( tmp_offset >> 4 ) < n_256clut ){
						if ( ++writeaddr >= 128 ){
							TopBaseAddress += 32 ;
							writeaddr = 0 ;
							n_256clut = 0 ;
						}
						tmp_offset = clut_offset_table1[ writeaddr ] ;
					}

					base_addr = TopBaseAddress + clut_offset_table0[ tmp_offset >> 4 ] ;
					clut_offset = tmp_offset & 0x0f ;
					ConvertBlockImageClut32(&clut[i*16],
											&VramImage[ base_addr * 64 ], clut_offset, clut_offset+1 );
					tex->tex_trans.tex0.data |= info->load_tex_base ;
					tex->tex_trans.tex0.data |=								 	/* TEX0(通常使用 4bit) [[Normally used 4bit]] */
						( ( (unsigned long64)( base_addr - clut_start_block) << 37 ) |
						  ( (unsigned long64)( clut_offset ) << 56 ) );
  					tex->tex_trans.tex2.data |=								  		/* TEX2(ロード用 8bit化) [[For loading of 8bit]] */
						( ( (unsigned long64)( base_addr - clut_start_block) << 37 ) |
						  ( (unsigned long64)0 << 56 ) );
					if ( ++writeaddr >= 128 ){
						TopBaseAddress += 32 ;
						writeaddr = 0 ;
						n_256clut = 0 ;
					}
					tex++;
				}
			}
			num++ ;
		}
		/* 念のため書き込みアドレス値 補正 [[Write the address to make sure the correct]] */
		clut_use_line = TopBaseAddress - clut_start_block ;
		{
			int		line0, line1, line ;
			line0 = ( ( writeaddr + 7 ) / 8 ) * 2 ;
			line1 = ( ( n_256clut + 3 ) / 4 ) * 16 ;
			line = ( line0 > line1 ) ? (line0) : (line1) ;
			clut_use_line += line ;
			TopBaseAddress += ( line + 31 ) / 32 ;
		}
	}
	
	/* ページ境界に揃える [[Page boundary aligned]] */
	TopBaseAddress = ( TopBaseAddress + 31 ) & (~31) ;
	
	{/* ファイルへの書き出し [[Export to a file]] */
		TRI_FILEHEADER	header ;
		FILE	*fp ;
		int		header_size ;
		unsigned long32	buffer[ 64 * 32 ] ;	/* １ページ分のバッファ [[1 page buffer]] */

		/* ファイルヘッダーの書き出し [[Export of the file]] */
		memset( &header, 0, sizeof( TRI_FILEHEADER ) );
		header.tex_offset = 0 ;
		header.tex_size = clut_start_block ;
		header.clut_offset = clut_start_block ;
		//header.clut_size = TopBaseAddress - clut_start_block ;
		header.clut_size = clut_use_line ;
		header.n_textures = total_n_textures ;
		header.compress_flag = option_flag ;
		/* データの先頭が8qword境界に揃うようにする [[The beginning of the data to align the boundaries 8 qword]] */
		header_size = sizeof(TRI_FILEHEADER) ;
		header_size += sizeof( DG_TEX ) * header.n_textures ;
		header_size = ( header_size + 127 ) & 0xffff80 ;
		header.texel_addr = header_size ;
		header.clut_addr = header_size + clut_start_block * 64 * 4 ;

		if ( ( fp = fopen( filename, "wb" ) ) == NULL ) return ;
		fwrite( &header, sizeof( TRI_FILEHEADER ), 1, fp );

		/* テクスチャ情報の書き出し [[Export texture information]] */
		num = 0 ;
		while ( ( info = GetTextureInfo( num ) ) != NULL ){
			fwrite( info->textures, sizeof( DG_TEX ), info->n_textures, fp );
			num++ ;
		}
		/* データの先頭が8qword境界に揃うようにする [[The beginning of the data to align the boundaries 8 qword]] */
		for ( i = header_size - ftell( fp ) ; i > 0 ; i-- ){
			fputc( 0, fp );
		}

		/* テクスチャイメージブロックの書き出し [[Export texture image blocks]] */
		for ( i = 0 ; i < header.tex_size ; i += 32 ){
			ConvertBlockSetToImageData( &VramImage[ 64 * i ], buffer );
			fwrite( buffer, sizeof( long32 ), 64 * 32, fp );
		}

		/* ＣＬＵＴテクスチャブロックの書き出し [[Export texture CLUT blocks]] */
		if ( header.clut_size != 0 ){
			for ( i = 0 ; i < header.clut_size ; i += 32 ){
				ConvertBlockSetToImageData( &VramImage[ 64 * ( header.clut_offset + i ) ], buffer );
				fwrite( buffer, sizeof( long32 ), 64 * 32, fp );
			}
		}

		fclose( fp );

	}

}

/* ページを６４×３２の３２ビットイメージデータに変換する [[Convert to page a 64x32 32-bit image]] */
static void ConvertBlockSetToImageData( unsigned long32 *src, unsigned long32 *dst )
{
	int		i, j, k ;
	int		block_offset, height_offset, image_offset ;

	for ( j = 0 ; j < 32 ; j++ ){
		/* ブロック内Ｙ座標によるオフセットを計算 [Y coordinate offset is calculated by the block]] */
		height_offset = ( j & 7 ) * 8 ;
		for ( i = 0 ; i < 64 ; i+=8 ){
			/* データ格納先オフセットを計算 [[Calculate the offset where the data storage]] */
			image_offset = i + j * 64 ;
			/* ブロックによるオフセットを計算 [[Calculate the offset of the block]] */
			block_offset = GetImageBlockNumber32bit( i, j, 64 ) * 64 ;
			/* ８ピクセル分書き込み [[8 pixels of writing]] */
			for ( k = 0 ; k < 8 ; k++ ){
				dst[ image_offset + k ] = src[ block_offset + height_offset + k ] ;
			}
		}
	}
}
