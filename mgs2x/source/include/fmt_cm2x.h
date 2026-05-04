/*
	fmt_cm2.h
	テクスチャデータ .cm2 フォーマット

	1999/07/07 K.Takabe
	$Id: fmt_cm2x.h,v 1.1.1.3 2002/11/19 11:41:58 Yoshizawa1 Exp $

*/

#ifndef __FMTCM2_H__
#define __FMTCM2_H__

#ifdef __cplusplus
extern "C" {
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
	u_long64	         	tex0_base ;	/* TEX0基本設定値 */
	u_long64	         	tex1 ;		/* TEX1設定値（現在未使用） */
	u_long64	         	tex2 ;		/* TEX2設定値（現在未使用） */
	u_long64	         	clamp ;		/* CLAMP設定値 */
	u_long64	         	alpha ;		/* ALPHA設定値 */
	float				u_offset ;	/* 使用テクスチャの開始Ｕ座標 */
	float				v_offset ;	/* 使用テクスチャの開始Ｖ座標 */
	float				u_scale ;	/* 使用テクスチャのスケール補正値 */
	float				v_scale ;	/* 使用テクスチャのスケール補正値 */
	u_long32	         	tex_id ;	/* テクスチャＩＤ */
	u_long32	         	pad ;		/* リザーブ */
} TEXTAG ;


#if 0
/*
	ＸＢＯＸ用旧フォーマット
*/
typedef struct _tri_fileheader {
	u_long32		tex_offset ;	/* テクスチャデータ開始オフセット（ブロック単位） */
	u_long32		tex_size ;		/* テクスチャデータサイズ（ブロック単位） */
	u_long32		clut_offset ;	/* ＣＬＵＴデータ開始オフセット（ブロック単位） */
	u_long32		clut_size ;		/* ＣＬＵＴデータサイズ（ブロック単位） */
	u_long32		n_textures ;	/* テクスチャデータ数 */
	u_long32		compress_flag ;	/* 圧縮フラグ */
	//u_long32		texel_addr ;	/* テクセルデータへのポインタ（現在未使用） */
	u_long32		format ;		/* Xbox: テクスチャフォーマット */
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

// TRI_FILEHEADER.format メンバの値
enum {
	TRI_FORMAT_BITMAP,
	TRI_FORMAT_DDS,
};

#else
/*
	ＸＢＯＸ用新フォーマット
*/


/* グローバルテクスチャデータファイル（.tpk） */
typedef struct _texpack_info {
	unsigned int		id ;			/* 24bit strcode ID */
	unsigned int		flag ;			/* 各種フラグ */
	unsigned short		x, y ;			/* テクスチャ配置座標 */
	unsigned short		w, h ;			/* テクスチャサイズ */
	void				*clut_addr ;	/* パレットアドレス */
	unsigned int		pad[ 3 ];
} TEXPACK_INFO ;
typedef struct _texpack_header {
	int					file_type ;		/* ファイル認識用ＩＤ（現状未定＝０） */
	unsigned short		width ;			/* テクスチャ幅 */
	unsigned short		height ;		/* テクスチャ高さ */
	unsigned short		format ;		/* テクスチャフォーマット */
	unsigned short		flag ;			/* 各種フラグ */
	int					n_info ;		/* テクスチャ情報数 */
	TEXPACK_INFO		*info ;			/* テクスチャ情報へのオフセット */
	void				*texel_addr ;	/* テクセルデータへのオフセット */
	unsigned short		mip_level ;		/* 最大ミップマップレベル（通常０） */
	unsigned short		pad ;
	int					pad2 ;
} TEXPACK_HEADER ;

enum {/* TEXPACKフォーマット */
	/* ノーマルイメージ */
	TEXPACK_FORMAT_A8R8G8B8	= 0x0001,		/* ３２ビット無圧縮 */
	TEXPACK_FORMAT_A4R4G4B4	= 0x0002,		/* １６ビット無圧縮 */
	TEXPACK_FORMAT_A1R5G5B5	= 0x0003,		/* １６ビット無圧縮 */
	TEXPACK_FORMAT_A8G8G8R8	= 0x0004,		/* ３２ビット無圧縮（ＰＳ２用） */
	TEXPACK_FORMAT_A1G5G5R5	= 0x0005,		/* １６ビット無圧縮（ＰＳ２用） */
	/* DXT圧縮イメージ */
	TEXPACK_FORMAT_DXT1		= 0x0010,		/* 圧縮（１ビットアルファ） */
	TEXPACK_FORMAT_DXT3		= 0x0012,		/* 圧縮（１６段階アルファ付き） */
	TEXPACK_FORMAT_DXT5		= 0x0014,		/* 圧縮（８階調補間アルファ付き） */
	/* パレットインデックスイメージ */
	TEXPACK_FORMAT_PAL4		= 0x0020,		/* 4bitパレット */
	TEXPACK_FORMAT_PAL8		= 0x0021,		/* 8bitパレット */
	TEXPACK_FORMAT_PAL4AGBR	= 0x0022,		/* 4bitパレット（ＰＳ２用） */
	TEXPACK_FORMAT_PAL8AGBR	= 0x0023,		/* 8bitパレット（ＰＳ２用） */
	/* 特殊イメージ */
	TEXPACK_FORMAT_V8U8		= 0x0030,		/* ＵＶテクスチャ（バンプマップ用） */
};
enum {/* TEXPACK_INFO用フラグ */
	/* データフォーマット *//* 現在未使用(TEXPACK_FORMAT_*を指定すればいいかも) */
	TEXINFO_FLAG_FORMAT			= 0x000000ff,	/* 画像フォーマットマスク */
	/* 半透明タイプ */
	TEXINFO_FLAG_BLEND_NONE		= 0x00000000,	/* 不透明 */
	TEXINFO_FLAG_BLEND_HLF		= 0x00000100,	/* アルファ５０％半透明 */
	TEXINFO_FLAG_BLEND_ADD		= 0x00000200,	/* 加算半透明 */
	TEXINFO_FLAG_BLEND_SUB		= 0x00000300,	/* 減算半透明 */
	TEXINFO_FLAG_BLEND_AMODE	= 0x00000400,	/* アルファモード指定半透明 */
	TEXINFO_FLAG_BLEND_MASK		= 0x00000f00,	/* マスク */
	/* 各種特殊フラグ */
	TEXINFO_FLAG_DECAL			= 0x00001000,	/* シェーディング無効 */
	TEXINFO_FLAG_REP			= 0x00002000,	/* リピート指定（現在無効） */
	/* マッピング指定 */
	TEXINFO_FLAG_EMAP			= 0x00010000,	/* 環境マッピング用 */
	TEXINFO_FLAG_BMAP			= 0x00020000,	/* バンプマッピング用 */
	TEXINFO_FLAG_SMAP			= 0x00030000,	/* アニメシェーディング用 */
	TEXINFO_FLAG_MAPMASK		= 0x000f0000,	/* マッピングタイプマスク */
	/* その他 */
	TEXINFO_FLAG_AMODE_MASK		= 0xff000000,	/* PS2形式ブレンドパラメータ指定（BLEND_AMODE指定時のみ有効） */
	TEXINFO_FLAG_FREE			= 0x00f0c000,	/* 空きビット */
};


#if 0
#ifndef KP_XBOX
#ifndef u_long64
#define u_long64	unsigned long long
#endif
typedef struct { float vx, vy, vz, vw ; } FVECTOR ;

/* ＰＳ２ＧＳレジスタ（Ａ＋Ｄ）構造体 */
typedef ALIGN16_DECL(struct) _dg_gsreg {
	u_long64	data ;
	u_long64	reg ;
} DG_GSREG  ;

/* テクスチャパラメータ (PS2とは違う) */
typedef struct _dg_tex_trans {
	DG_GSREG alpha; // SCE_GS_SET_ALPHA の値が入る 
	DG_GSREG clamp;
	DG_GSREG tex2;
	DG_GSREG tex0;
	FVECTOR vec1; /* パラメータ格納用 */
	FVECTOR vec2; /* パラメータ格納用 */
	// XBOX追加 
	void		*ptex; // テクスチャ 
	int			pad[ 3 ];
} DG_TEX_TRANS;

/* テクスチャ情報 */
typedef struct _dg_tex {
    float               u_offset;   /* 使用テクスチャの開始Ｕ座標 */
    float               v_offset;   /* 使用テクスチャの開始Ｖ座標 */
    float               u_scale;    /* 使用テクスチャのスケール補正値 */
    float               v_scale;    /* 使用テクスチャのスケール補正値 */
    unsigned int        tex_id;     /* テクスチャＩＤ */
    unsigned int        tri_id;     /* テクスチャアーカイバＩＤ */
    int                 flag;       /* テクスチャフラグ */
	int					texinfonum ;/* xti内での使用XTI_TEXINFO番号 */
    DG_TEX_TRANS        tex_trans;  /* テクスチャパラメータ転送パケット */
} DG_TEX;
#endif
#endif

/* xtiファイル内に格納される各テクスチャについての情報 */
typedef struct _xti_texinfo {
	unsigned short		format ;		/* テクスチャフォーマット */
	unsigned short		flag ;			/* フラグ */
	unsigned short		width ;			/* テクスチャ幅 */
	unsigned short		height ;		/* テクスチャ高さ */
	unsigned short		mip_level ;		/* 最大ミップマップレベル */
	unsigned short		pad ;			/*  */
	unsigned int		use_size ;		/* 使用データサイズ */
	unsigned int		texel_addr ;	/* テクセルデータへのオフセット */
	unsigned int		clut_addr ;		/* パレットデータへのオフセット */
} XTI_TEXINFO ;

enum {/* 基本的にはTEXPACKに準ずる */
	/* ノーマルイメージ */
	XTI_FORMAT_A8R8G8B8	= 0x0001,		/* ３２ビット無圧縮 */
	XTI_FORMAT_A4R4G4B4	= 0x0002,		/* １６ビット無圧縮 */
	XTI_FORMAT_A1R5G5B5	= 0x0003,		/* １６ビット無圧縮 */
	XTI_FORMAT_A8G8G8R8	= 0x0004,		/* ３２ビット無圧縮（ＰＳ２用） */
	XTI_FORMAT_A1G5G5R5	= 0x0005,		/* １６ビット無圧縮（ＰＳ２用） */
	/* DXT圧縮イメージ */
	XTI_FORMAT_DXT1		= 0x0010,		/* 圧縮（１ビットアルファ） */
	XTI_FORMAT_DXT3		= 0x0012,		/* 圧縮（１６段階アルファ付き） */
	XTI_FORMAT_DXT5		= 0x0014,		/* 圧縮（８階調補間アルファ付き） */
	/* パレットインデックスイメージ */
	XTI_FORMAT_PAL4		= 0x0020,		/* 4bitパレット */
	XTI_FORMAT_PAL8		= 0x0021,		/* 8bitパレット */
	XTI_FORMAT_PAL4AGBR	= 0x0022,		/* 4bitパレット（ＰＳ２用） */
	XTI_FORMAT_PAL8AGBR	= 0x0023,		/* 8bitパレット（ＰＳ２用） */
	/* 特殊イメージ */
	XTI_FORMAT_V8U8		= 0x0030,		/* ＵＶテクスチャ（バンプマップ用） */
};


typedef struct _tri_fileheader {
#if 0
	u_long32		tex_offset ;	/* テクスチャデータ開始オフセット（ブロック単位） */
	u_long32		tex_size ;		/* テクスチャデータサイズ（ブロック単位） */
	u_long32		clut_offset ;	/* ＣＬＵＴデータ開始オフセット（ブロック単位） */
	u_long32		clut_size ;		/* ＣＬＵＴデータサイズ（ブロック単位） */
	u_long32		n_textures ;	/* テクスチャデータ数 */
	u_long32		compress_flag ;	/* 圧縮フラグ（現在は単なるフラグとして使用） */
	u_long32		texel_addr ;	/* テクセルデータへのポインタ（現在未使用） */
	u_long32		clut_addr ;		/* ＣＬＵＴデータへのポインタ（現在未使用） */
#else
	u_long32		file_type ;		/* ファイル識別ＩＤ（現在未使用） */
	u_long32		compress_flag ;	/* 圧縮フラグ（現在は単なるフラグとして使用） */
	u_long32		n_info ;		/* XTIテクスチャ情報数 */
	u_long32		info_addr ;		/* XTIテクスチャ情報へのオフセット */
	u_long32		n_textures ;	/* テクスチャデータ数 */
	u_long32		texel_addr ;	/* テクセルデータへのポインタ */
	u_long32		clut_addr ;		/* ＣＬＵＴデータへのポインタ */
	u_long32		n_clut ;		/* 格納ＣＬＵＴエントリ数 */
#endif
} TRI_FILEHEADER ;

enum {
	TRI_FLAG_COMPRESS	= 0x00001,		/* 圧縮フラグ */
	TRI_FLAG_TRANS		= 0x00002,		/* 半透明テクスチャフラグ */
	TRI_FLAG_STATIC		= 0x00004,		/* スタティックパレットフラグ */
	TRI_FLAG_LATTERDRAW	= 0x00008,		/* 描画フェーズ制御フラグ */
};

enum {
#if 0
	TRI_TEXTAG_FLAG_EMAP	= 0x00000001,
	TRI_TEXTAG_FLAG_SMAP	= 0x00000002,
	TRI_TEXTAG_FLAG_BMAP	= 0x00000004,
#else
	TRI_TEXTAG_FLAG_EMAP	= TEXINFO_FLAG_EMAP,
	TRI_TEXTAG_FLAG_SMAP	= TEXINFO_FLAG_SMAP,
	TRI_TEXTAG_FLAG_BMAP	= TEXINFO_FLAG_BMAP,
#endif
};

/*
	XBOX用テクスチャイメージファイル(XTI)フォーマットについて

	基本的にはＰＳ２システムとの互換性を考慮した形になっている
	MGS2システムでは２のｎ乗以外のテクスチャの使用を許可し、
	それらのテクスチャを大きな２のｎ乗サイズのテクスチャ内に
	複数配置することでデザイナ側での制限を減らしつつ、
	テクスチャサイズを最低限に抑えるように工夫している。
	ここではこの各テクスチャを配置した大きなテクスチャを
	グローバルテクスチャと表現することにする。

	また、PS2用テクスチャイメージファイル(TRI)は実際の描画での
	転送単位になっており、モデルなどの描画順の基準にもなっているため
	描画するモデルでの使用テクスチャ量によってはTRI及びXTI内に
	複数のグローバルテクスチャを持つことが出来るようになっている。


	(TRI)
	+---------------+
	|TRI_FILEHEADER |
	+---------------+
	|DG_TEX x n     |
	+---------------+
	|texel data     |
	+---------------+
	|clut data      |
	+---------------+

	(XTI)
	+---------------+
	|TRI_FILEHEADER |
	+---------------+
	|DG_TEX x n     |
	+---------------+
	|XTI_TEXINFO x m|
	+---------------+
	|texel data     |
	+---------------+
	|clut data      |
	+---------------+

	また、グローバルテクスチャはサイズを節約するためなるべく上方向に
	配置するようにし、未使用となる下領域はデータとして含めないようにする。


*/

#endif

#ifdef __cplusplus
};
#endif

#endif
