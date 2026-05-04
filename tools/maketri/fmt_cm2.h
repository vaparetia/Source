/*
	$Id: fmt_cm2.h,v 1.7 2001/02/07 05:15:39 usr02774 Exp $
 */
/*                                              */
/* ＣＭ２形式テクスチャフォーマット構造体 [[CM2 texture format structure]] */
/*                                              */
/*                                              */
/*                                              */


#ifndef __LOADCM2_H__
#define __LOADCM2_H__

#ifdef __cplusplus
extern "C" {
#endif


#define OLD_CM2_MAGIC	(0)/* ???? */
#define NEW_CM2_MAGIC	(19990907)

/* 各テクスチャ毎の設定フラグ [[Setting of each flag for each texture]] */
#define CM2_TEX_COLOR_MASK	(0x00000003)	/* テクスチャカラーモード設定値マスク */
#define CM2_TEX_USE_AMODE	(0x00000004)	/* αブレンドモード直接指定フラグ */
#define CM2_TEX_BLEND_MODE	(0x00000030)	/* αブレンドモード設定値マスク */
#define CM2_TEX_DECAL		(0x00000100)	/* 強制シェーディング禁止フラグ */
#define CM2_TEX_FLAG_EMAP	(0x00010000)	/* 環境マッピングフラグ [[Environment mapping flag]] */
#define CM2_TEX_FLAG_BMAP	(0x00020000)	/* バンプマッピングフラグ [[Bump mapping flag]] */
#define CM2_TEX_FLAG_SMAP	(0x00040000)	/* アニメ影マップフラグ [Animated shadow map flag]] */
#define CM2_TEX_AMODE_MASK	(0xff000000)	/* αブレンドモード直接指定設定値マスク */
#define CM2_TEX_FREE_BIT	(0x00f7fec8)	/* 空ビット [[Empty bit]] */

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
	long32		clut_addr ;
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

#if 0
typedef struct _dg_tex {
	unsigned long64		tex0_base ;	/* TEX0基本設定値 */
	unsigned long64		tex1 ;		/* TEX1設定値（現在未使用） */
	unsigned long64		tex2 ;		/* TEX2設定値（現在未使用） */
	unsigned long64		clamp ;		/* CLAMP設定値 */
	unsigned long64		alpha ;		/* ALPHA設定値 */
	float				u_offset ;	/* 使用テクスチャの開始Ｕ座標 */
	float				v_offset ;	/* 使用テクスチャの開始Ｖ座標 */
	float				u_scale ;	/* 使用テクスチャのスケール補正値 */
	float				v_scale ;	/* 使用テクスチャのスケール補正値 */
	unsigned long32		tex_id ;	/* テクスチャＩＤ */
	unsigned long32		pad ;		/* リザーブ */
} DG_TEX ;

#else
	/*
		テクスチャパラメータＶＵ１転送パケット [[VU1 texture parameter packets transferred]]
	*/
typedef struct _dg_tex_trans {
	DG_GIFTAG	giftag ;	/* 描画設定ＧＩＦタグ [[GIF tag drawing set]] */
	DG_GSREG	clamp ;		/* 描画設定プリミティブ（clamp）[[Primitive drawing set]] */
	DG_GSREG	tex2 ;		/* 描画設定プリミティブ（tex2） */
	DG_GSREG	tex0 ;		/* 描画設定プリミティブ（tex0） */
	DG_GSREG	alpha ;		/* 描画設定プリミティブ（alpha） */
	DG_GSREG	pad[1] ;	/* 描画設定プリミティブ（リザーブ）[[Reserved]] */
	FVECTOR		vec1 ;		/* パラメータ格納用 [[For parameter storage]] */
	FVECTOR		vec2 ;		/* パラメータ格納用 */
} DG_TEX_TRANS ;

	/*
		新テクスチャ情報 [[New texture information]]
	*/
typedef struct _dg_tex {
	float				u_offset ;	/* 使用テクスチャの開始Ｕ座標 */
	float				v_offset ;	/* 使用テクスチャの開始Ｖ座標 */
	float				u_scale ;	/* 使用テクスチャのスケール補正値 [[Using the texture scale correction]] */
	float				v_scale ;	/* 使用テクスチャのスケール補正値 */
	unsigned int		tex_id ;	/* テクスチャＩＤ [[Texture ID]] */
	unsigned int		tri_id ;	/* テクスチャアーカイバＩＤ [[Texture Archiver ID]] */
	int					flag ;		/* テクスチャフラグ [[Texture flag]] */
	int					pad[1] ;	/*  */
	DG_TEX_TRANS		tex_trans ;/* テクスチャパラメータ転送パケット[[Texture parameters transmitted packet]] */
} DG_TEX ;

	/*
		転送最大サイズのテクスチャ転送パケット構造体 [[The maximum size of transmission packet structure texture transfer]]
	*/
typedef struct _dg_tex_packet {
	DG_DMATAG		dmatag0 ;			/* レジスタ設定値転送ＤＭＡタグ [[DMA register settings transfer Tag]] */
	sceGifTag		giftag0 ;			/* レジスタ設定 [[Register setting]] */
	sceGsBitbltbuf	bitbltbuf1 ;
	long			bitbltbuf1_addr ;
	sceGsTrxpos		trxpos1 ;
	long			trxpos1_addr ;
	sceGsTrxreg		trxreg1 ;
	long			trxreg1_addr ;
	sceGsTrxdir		trxdir1 ;
	long			trxdir1_addr ;
	sceGifTag		giftag1 ;			/* イメージ転送ＧＩＦタグ [[GIF image transfer tag]] */
	DG_DMATAG		dmatag1 ;			/* イメージ転送ＤＭＡタグ [[DMA image transfer tag]] */

	DG_DMATAG		dmatag2 ;			/* 第２イメージ転送ＧＩＦタグ転送ＤＭＡタグ */
	sceGifTag		giftag2 ;			/* レジスタ設定 */
	sceGsBitbltbuf	bitbltbuf2 ;
	long			bitbltbuf2_addr ;
	sceGsTrxpos		trxpos2 ;
	long			trxpos2_addr ;
	sceGsTrxreg		trxreg2 ;
	long			trxreg2_addr ;
	sceGsTrxdir		trxdir2 ;
	long			trxdir2_addr ;
	sceGifTag		giftag3 ;			/* 第２イメージ転送ＧＩＦタグ */
	DG_DMATAG		dmatag3 ;			/* 第２イメージ転送ＤＭＡタグ */

	DG_DMATAG		dmatag4 ;			/* 第３イメージ転送ＧＩＦタグ転送ＤＭＡタグ */
	sceGifTag		giftag4 ;			/* レジスタ設定 */
	sceGsBitbltbuf	bitbltbuf3 ;
	long			bitbltbuf3_addr ;
	sceGsTrxpos		trxpos3 ;
	long			trxpos3_addr ;
	sceGsTrxreg		trxreg3 ;
	long			trxreg3_addr ;
	sceGsTrxdir		trxdir3 ;
	long			trxdir3_addr ;
	sceGifTag		giftag5 ;			/* 第３イメージ転送ＧＩＦタグ */
	DG_DMATAG		dmatag5 ;			/* 第３イメージ転送ＤＭＡタグ */

	DG_DMATAG		dmatag6 ;			/* 転送終了ＤＭＡタグ [[DMA transfer end tag]] */
} DG_TEX_PACKET ;
#endif


typedef struct _tri_fileheader {
	unsigned long32		tex_offset ;	/* テクスチャデータ開始オフセット（ブロック単位）[[Texture data start offset (in blocks)]] */
	unsigned long32		tex_size ;		/* テクスチャデータサイズ（ブロック単位） [[Texture data size (in blocks)]] */
	unsigned long32		clut_offset ;	/* ＣＬＵＴデータ開始オフセット（ブロック単位）[[CLUT data start offset (in blocks)]] */
	unsigned long32		clut_size ;		/* ＣＬＵＴデータサイズ（ブロック単位）[[CLUT data size (in blocks)]] */
	unsigned long32		n_textures ;	/* テクスチャデータ数 */
	unsigned long32		compress_flag ;	/* 圧縮フラグ＆半透明テクスチャフラグ [[Compression flag and transparent texture flag]] */
	unsigned long32		texel_addr ;	/* テクセルデータへのポインタ（現在未使用）[[Pointer to texture data (currently unused)]] */
	unsigned long32		clut_addr ;		/* ＣＬＵＴデータへのポインタ（現在未使用）[[Pointer to CLUT data (currently unused)]] */
} TRI_FILEHEADER ;

enum {
	TRI_FLAG_COMPRESS	= 0x00001,		/* 圧縮フラグ [[Compression flag]] */
	TRI_FLAG_TRANS		= 0x00002,		/* 半透明テクスチャフラグ [[Translucent texture flag]] */
	TRI_FLAG_STATIC		= 0x00004,		/* スタティックパレットフラグ [[Static palette flag]] */
	TRI_FLAG_LATTERDRAW	= 0x00008,		/* 描画フェーズ制御フラグ [[Phase control drawing flag]] */
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
