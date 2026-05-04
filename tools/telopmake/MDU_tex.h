/*
   MDU_tex.h

   テクスチャファイル関連ヘッダ

   by M.Sonoyama 1999 Aug.～
   $Id: MDU_tex.h,v 1.1 2002/07/12 04:10:57 usr01475 Exp $

   Konami Computer Entertainment Japan West
*/

/* グローバル定義 */
#define	MAX_TEXTURES	(1024)


/* 内部用、共通テクスチャフォーマット */

#ifndef _INC_CRT
typedef struct tagRGBQUAD {
  u_char	b, g, r, alpha ;
} RGBQUAD ;

typedef struct tagRGBTriple {
  u_char	b, g, r ;
} RGBTriple ;
#endif

typedef	struct	{
    u_int	dx, dy ;	
    u_int	w, h ;
    void	*data ;
} MDU_TexRect ;

typedef	struct	{
    u_char		name[ 32 ] ;
    u_int		id ;
    u_int		flag ;
    u_int		width, height ;
    u_int		n_colors ;
    MDU_TexRect		data ;
    MDU_TexRect		clut ;
} MDU_Tex ;

/*------------------------------------------------------------------------*/

/* TIM ファイルフォーマットヘッダ */
typedef	struct	{
    u_long		bnum ;
    u_short		dx, dy ;
    u_short		w, h ;
    u_short		data[ 0 ] ;
} TIMRECT ;

typedef	struct	{
    u_long		id ;
    u_long		flag ;
    TIMRECT		clut[ 0 ] ;
} TIMDATA ;

typedef	struct	{
    int			mode ;
    int			width ;
    int			height ;
    int			n_colors ;
    TIMRECT		*clut_rect ;
    TIMRECT		*data_rect ;
    u_short		*clut ;
    u_short		*data ;
    u_long		*clut32 ;
    u_short		zeroflag ;
    u_short		nonzeroflag ;
    TIMDATA		timdata[ 0 ] ;
} TIMIF ;

/* PICファイルフォーマットヘッダ */
typedef	struct	{
	unsigned long	magic ;
	unsigned long	version ;
	unsigned char	comment[80] ;
	unsigned long	id ;
	unsigned short	width, height ;
	unsigned long	ratio ;
	unsigned short	fields ;
	unsigned short	pad ;
	unsigned char	chained ;
	unsigned char	size ;
	unsigned char	type ;
	unsigned char	channel ;
} PIC_Header ;

/* PCXファイルフォーマットヘッダ */
#define	PCXFILE_STAMP	(12345)

typedef struct {
    u_char **pltnew;
    u_char *pixnew;
} PP;

typedef	struct	{
    u_short		stamp ;
    u_short		flag ;
    u_short		px, py ;
    u_short		cx, cy ;
    u_short		n_colors ;
    u_short		zeroflag ;
    u_short		nonzeroflag ;
} TIM_Appendix ;

typedef	struct	{
    unsigned char	flag ;
    unsigned char	version ;
    unsigned char	encoding ;
    unsigned char	bits_per_pixel ;
    unsigned short	min_x, min_y ;
    unsigned short	max_x, max_y ;
    unsigned short	dpi_x, dpi_y ;
    unsigned char	header_palette[ 48 ] ;
    unsigned char	reserved ;
    unsigned char	n_planes ;
    unsigned short	bytes_per_line ;
    unsigned short	header_palette_class ;
    unsigned short	screen_width, screen_height ;
    TIM_Appendix	appendix[ 0 ] ;
    unsigned char	pad[ 54 ] ;
} PCX_Header ;

extern unsigned char	Palette16[48] ;
extern unsigned char	BytesPerLine16 ;

/* BMPファイルフォーマットヘッダ */
#ifndef _INC_CRT

typedef u_short	WORD ;
typedef u_long	DWORD ;
typedef long	LONG ;

typedef struct tagBITMAPINFOHEADER { /* bmih */
   DWORD  biSize;
   LONG   biWidth;
   LONG   biHeight;
   WORD   biPlanes;
   WORD   biBitCount;
   DWORD  biCompression;
   DWORD  biSizeImage;
   LONG   biXPelsPerMeter;
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed;
   DWORD  biClrImportant;
} BITMAPINFOHEADER ;

typedef struct tagBITMAPINFO { /* bmi */
   BITMAPINFOHEADER bmiHeader ;
   RGBQUAD          bmiColors[1];
} BITMAPINFO;

typedef struct tagBITMAPINFO2 { /* bmi2 */
   BITMAPINFOHEADER bmiHeader ;
   RGBTriple        bmiColors[ 1 ] ;
} BITMAPINFO2 ;

typedef struct tagBITMAPFILEHEADER { /* bmfh */
  WORD	  padding ;
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
} BITMAPFILEHEADER;

#else
typedef struct tagBITMAPFILEHEADER_PAD { /* bmfh */
  WORD	  padding ;
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
} BITMAPFILEHEADER_PAD ;

typedef struct tagBITMAPHEADER_PAD {
  BITMAPFILEHEADER_PAD 	header ;
  BITMAPINFOHEADER	info ;
} BITMAPHEADER_PAD ;
#endif


typedef struct tagBITMAPHEADER {
  BITMAPFILEHEADER 	header ;
  BITMAPINFOHEADER	info ;
} BITMAPHEADER ;

/*------------------------------------------------------------------------*/

/* フラグ等定義 */
enum {
    MDU_TEX_FLAG_COLOR16 	= 0x0000,
    MDU_TEX_FLAG_COLOR256 	= 0x0001,
    MDU_TEX_FLAG_COLOR_FULL 	= 0x0002,
    MDU_TEX_FLAG_ALPHA_REG	= 0x0004,
    MDU_TEX_FLAG_CLUT_EXIST	= 0x0008,

    MDU_TEX_FLAG_BLACK_NOTTRANS	= 0x1000,	/* 黒は不透明に */
    MDU_TEX_FLAG_COLOR_TRANS	= 0x2000,	/* 半透明ＯＮ */
} ;

#define	MDU_TEX_FLAG_TRANS_HALF	(0x0000)
#define	MDU_TEX_FLAG_TRANS_ADD	(0x0010)
#define	MDU_TEX_FLAG_TRANS_SUB	(0x0020)
#define	MDU_TEX_FLAG_TRANS_BLD	(0x0030)
#define	MDU_TEX_FLAG_DECAL	(0x0100)

#define	MDU_TEX_FLAG_EMAP	(0x00010000)
#define	MDU_TEX_FLAG_BMAP	(0x00020000)
#define	MDU_TEX_FLAG_SMAP	(0x00040000)

#define	MDU_TEX_FLAG_ALL	(0xff070134)

/*------------------------------------------------------------------------*/

/* 外部関数宣言 */

/* MDU_texInit.c */
extern	void	MDU_SetAlphaTexture( MDU_Tex *, int ) ;
extern	void	MDU_SetMaskTexture( MDU_Tex *, char * ) ;
extern	void	MDU_SetMaskTextureEx( MDU_Tex *, int ) ;
extern	void	MDU_SetTransTexture( MDU_Tex *, char * ) ;
extern	void	MDU_TexInit( void ) ;
extern	void	MDU_ClearTexture( MDU_Tex * ) ;
extern	MDU_Tex	*MDU_FindTexture( u_int ) ;
extern	void	MDU_AllocTex( MDU_Tex *, u_int, u_int ) ;

/* MDU_texLoad.c */
extern	MDU_Tex	*MDU_LoadBmp( char * ) ;
extern	MDU_Tex	*MDU_LoadTim( char * ) ;
extern	MDU_Tex	*MDU_LoadPcx( char * ) ;
extern	MDU_Tex	*MDU_LoadPic( char * ) ;

/* MDU_texSave.c */
extern	void	MDU_SaveBmp( MDU_Tex *, char * ) ;
extern	void	MDU_SaveTim( MDU_Tex *, char * ) ;
extern	void	MDU_SavePcx( MDU_Tex *, char * ) ;

/* MDU_texCm2.c */
extern	void	MDU_ReplaceTexCm2( MDU_Tex **, int ) ;
extern	int	MDU_SaveCm2( char *, MDU_Tex **, int ) ;
extern	void	MDU_DumpCm2( char * ) ;

/* MDU_texExtMdl.c */
#ifdef _INC_P3D
extern	int		MDU_ExtractTextureFromMdl( HP3DMODEL ) ;
#endif
extern	void	MDU_SaveExtractTextures( char * ) ;
extern	void	MDU_CompTexID( int ) ;

/* 外部変数宣言 */



