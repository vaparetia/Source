/*
	$Id: loadtpk.h,v 1.1 2002/04/16 05:24:31 usr02774 Exp $
 */
/*                                              */
/* ＣＭ２形式テクスチャ読み込みユーティリティー */
/*                                              */
/* 1999.3.29 K.Takabe                           */
/*                                              */



#ifndef __FMTCM2_H__
#define __FMTCM2_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 読み込みテクスチャ情報 */
typedef struct _tex_info {
	int				image_size ;		/* テクセルデータのバイト数 */
	int				clut_size ;			/* ＣＬＵＴのサイズ（バイト数） */
	int				load_tex_base ;		/* ＶＲＡＭに読み込まれた際のベースアドレス */
	int				load_clut_base ;	/* ＶＲＡＭに読み込まれた際のベースアドレス */
	int				width, height ;		/* テクスチャの幅、高さ */
	int				w_exp, h_exp ;		/* テクスチャの幅、高さ（指数形式＝２＾ｎ） */
	int				max_x, max_y ;		/* 最大使用座標 */
	int				mode ;				/* テクスチャモード */
	int				mip_level ;			/* 細大ミップマップレベル */
	int				n_textures ;
	DG_TEX			*textures ;			/* 各テクスチャの情報 */
	void			*image_data ;		/* テクセルデータへのポインタ */
	void			*clut_data ;		/* ＣＬＵＴデータへのポインタ */
} TEX_INFO ;


/* === ＣＭ２形式ファイル読み込みルーチン === */
/* テクスチャデータを読み込む */
int LoadCm2File( char *name );
/* テクスチャ情報の取得 */
TEX_INFO *GetTextureInfo( int num );
/* 読み込んだテクスチャデータを破棄する */
void ClearTextureData( void );
/* テクスチャプレビューデータを得る */
void GetTextureImage( int num, int *width, int *height, void **image_data );

/* === ＶＲＡＭデータ形式変換ユーティリティー === */
/* ３２ビットカラーモードアドレス変換 */
int GetImageBlockNumber32bit( int x, int y, int width );
/* ３２ビットカラーイメージ＝＞３２ビットカラーイメージ変換 */
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
/* ３２ビットＣＬＵＴ＝＞３２ビットブロックイメージ変換 */
void ConvertBlockImageClut32( void *src, void *dst, int offset, int num );

/* === ＶＲＡＭベタ形式変換ルーチン === */
/* テクスチャを一つのＶＲＡＭベタデータに変換し、ファイルに保存する */
void WriteTextureRowImageFile( char *filename, int option_flag );


/* 1999/11/10 まつざきの変更点 はじまり */

/* 1999/11/10 まつざきの変更点 おわり */

#ifdef __cplusplus
};
#endif

#endif
