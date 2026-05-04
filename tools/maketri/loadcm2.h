/*
	$Id: loadcm2.h,v 1.1 2000/01/12 11:54:16 usr10478 Exp $
 */
/*                                              */
/* ＣＭ２形式テクスチャ読み込みユーティリティー [[CM2 Texture format import utility]] */
/*                                              */
/* 1999.3.29 K.Takabe                           */
/*                                              */



#ifndef __FMTCM2_H__
#define __FMTCM2_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 読み込みテクスチャ情報 [[Loading texture information]] */
typedef struct _tex_info {
	int				image_size ;		/* テクセルデータのバイト数 [[Texture data bytes]] */
	int				clut_size ;			/* ＣＬＵＴのサイズ（バイト数）[[CLUT size (bytes)]] */
	int				load_tex_base ;		/* ＶＲＡＭに読み込まれた際のベースアドレス [[When loaded into the base of VRAM]] */
	int				load_clut_base ;	/* ＶＲＡＭに読み込まれた際のベースアドレス [[When loaded into the base of VRAM]] */
	int				width, height ;		/* テクスチャの幅、高さ[[Texture width, height]] */
	int				w_exp, h_exp ;		/* テクスチャの幅、高さ（指数形式＝２＾ｎ）The width of the texture, height (exponential format = 2 ^ n) */
	int				max_x, max_y ;		/* 最大使用座標 [[Maximum coordinate]] */
	int				mode ;				/* テクスチャモード [[Texture mode]] */
	int				n_textures ;
	DG_TEX			*textures ;			/* 各テクスチャの情報 [[Information for each texture]] */
	void			*image_data ;		/* テクセルデータへのポインタ [[Texture data pointer]] */
	void			*clut_data ;		/* ＣＬＵＴデータへのポインタ [[CLUT data pointer]] */
} TEX_INFO ;


/* === ＣＭ２形式ファイル読み込みルーチン [[Format file read routine]] === */
/* テクスチャデータを読み込む [[Read texture data]] */
int LoadCm2File( char *name );
/* テクスチャ情報の取得 [[Get texture info]] */
TEX_INFO *GetTextureInfo( int num );
/* 読み込んだテクスチャデータを破棄する [[Discard the texture data read]] */
void ClearTextureData( void );
/* テクスチャプレビューデータを得る [[Texture data obtained preview]] */
void GetTextureImage( int num, int *width, int *height, void **image_data );

/* === ＶＲＡＭデータ形式変換ユーティリティー [[VRAM data format conversion utility]] === */
/* ３２ビットカラーモードアドレス変換 [[32-bit color mode address translation]] */
int GetImageBlockNumber32bit( int x, int y, int width );
/* ３２ビットカラーイメージ＝＞３２ビットカラーイメージ変換 [[convert 32-bit color image => 32-bit color images]] */
void ConvertBlockImage32to32( void *src, void *dst );
/* １６ビットカラーモードアドレス変換 */
int GetImageBlockNumber16bit( int x, int y, int width );
/* １６ビットカラーイメージ＝＞３２ビットカラーイメージ変換 */
void ConvertBlockImage16to32( void *src, void *dst );
/* ８ビットインデックスモードアドレス変換 */
int GetImageBlockNumber8bit( int x, int y, int width );
/* ８ビットインデックスイメージ＝＞３２ビットカラーイメージ変換 */
void ConvertBlockImage8to32( void *src, void *dst );
/* ４ビットインデックスモードアドレス変換 */
int GetImageBlockNumber4bit( int x, int y, int width );
/* ４ビットインデックスイメージ＝＞３２ビットカラーイメージ変換 */
void ConvertBlockImage4to32( void *src, void *dst );
/* ３２ビットＣＬＵＴ＝＞３２ビットブロックイメージ変換 [[Convert Bit 32 CLUT => 32 bit block image]] */
void ConvertBlockImageClut32( void *src, void *dst, int offset, int num );

/* === ＶＲＡＭベタ形式変換ルーチン [[VRAM solid (beta?) format conversion routines]] === */
/* テクスチャを一つのＶＲＡＭベタデータに変換し、ファイルに保存する [[Solid texture data is converted into VRAM, and saved to file]] */
void WriteTextureRowImageFile( char *filename, int option_flag );


/* 1999/11/10 まつざきの変更点 はじまり[[Begin changes by Matsu Miyazaki]] */

/* 1999/11/10 まつざきの変更点 おわり[[End changes by Matsu Miyazaki]] */

#ifdef __cplusplus
};
#endif

#endif
