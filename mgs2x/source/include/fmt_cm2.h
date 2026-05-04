/*
	fmt_cm2.h
	テクスチャデータ .cm2 フォーマット

	1999/07/07 K.Takabe
	$Id: fmt_cm2.h,v 1.1.1.3 2002/11/19 11:41:58 Yoshizawa1 Exp $

*/

#ifndef __FMTCM2_H__
#define __FMTCM2_H__

#include "mgs_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0 //BP
#define long64 long
#define long32 int
#endif

#define OLD_CM2_MAGIC	(0)/* ???? */
#define NEW_CM2_MAGIC	(19990907)

typedef struct _tex_data{
	unsigned short	strcode ;
	unsigned short	n_colors ;
	unsigned short	width ;
	unsigned short	height ;
	unsigned short	pix_x, pix_y ;
	unsigned short	clut_x, clut_y ;
	long32		clut_addr ;
} TexData ;

typedef struct _tex_data2{
	unsigned int	strcode ;
	unsigned int	flag ;
	unsigned int	n_colors ;
	unsigned short	width ;
	unsigned short	height ;
	unsigned short	pix_x, pix_y ;
	unsigned short	clut_x, clut_y ;
	void			*clut_addr ;
} TexData2 ;


typedef struct _cm2_fileheader{
	long32		magic ;
	long32		width, height ;
	long32		dx, dy ;
	long32		n_textures ;
	long32		mode ;
	TexData	*texdatas ;
	void	*pixdata ;
} CM2_FILEHEADER ;

typedef struct _tex_tag {
	u_long64		tex0_base ;	/* TEX0基本設定値 */
	u_long64		tex1 ;		/* TEX1設定値（現在未使用） */
	u_long64		tex2 ;		/* TEX2設定値（現在未使用） */
	u_long64		clamp ;		/* CLAMP設定値 */
	u_long64		alpha ;		/* ALPHA設定値 */
	float				u_offset ;	/* 使用テクスチャの開始Ｕ座標 */
	float				v_offset ;	/* 使用テクスチャの開始Ｖ座標 */
	float				u_scale ;	/* 使用テクスチャのスケール補正値 */
	float				v_scale ;	/* 使用テクスチャのスケール補正値 */
	u_long32		tex_id ;	/* テクスチャＩＤ */
	u_long32		pad ;		/* リザーブ */
} TEXTAG ;


typedef struct _tri_fileheader {
	u_long32		tex_offset ;	/* テクスチャデータ開始オフセット（ブロック単位） */
	u_long32		tex_size ;		/* テクスチャデータサイズ（ブロック単位） */
	u_long32		clut_offset ;	/* ＣＬＵＴデータ開始オフセット（ブロック単位） */
	u_long32		clut_size ;		/* ＣＬＵＴデータサイズ（ブロック単位） */
	u_long32		n_textures ;	/* テクスチャデータ数 */
	u_long32		compress_flag ;	/* 圧縮フラグ */
	u_long32		texel_addr ;	/* テクセルデータへのポインタ（現在未使用） */
	u_long32		clut_addr ;		/* ＣＬＵＴデータへのポインタ（現在未使用） */
} TRI_FILEHEADER ;

enum {
	TRI_FLAG_COMPRESS	= 0x00001,		/* 圧縮フラグ */
	TRI_FLAG_TRANS		= 0x00002,		/* 半透明テクスチャフラグ */
	TRI_FLAG_STATIC		= 0x00004,		/* スタティックパレットフラグ */
	TRI_FLAG_LATTERDRAW	= 0x00008,		/* 描画フェーズ制御フラグ */
};

enum {
	TRI_TEXTAG_FLAG_EMAP	= 0x00000001,
	TRI_TEXTAG_FLAG_SMAP	= 0x00000002,
	TRI_TEXTAG_FLAG_BMAP	= 0x00000004,
};



#ifdef __cplusplus
};
#endif

#endif
