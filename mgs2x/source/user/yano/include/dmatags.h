/*
    dmatags.h
	ＤＭＡタグやＧＩＦタグ関係
    2001/03/11 T.Shibata

    $Id: dmatags.h,v 1.1.1.3 2002/11/19 11:52:01 Yoshizawa1 Exp $
*/

#ifndef _DMATAGS_H_
#define _DMATAGS_H_

//汎用プリミティブ設定
typedef struct _set_draw_data {
	DG_GSREG	texflush;	//ウェイト
	DG_GSREG	test;		//テスト値
	DG_GSREG	tex0;		//テクスチャー
	DG_GSREG	clamp;		//クランプモード
	DG_GSREG	prim;		//プリミティブ設定
	DG_GSREG	alpha;		//アルファモード
} SET_DRAW_PRIM_DATA;

typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	SET_DRAW_PRIM_DATA	data;
} SET_DRAW_PRIM;

#if 0 /* user/shibata/util/dma_set.h と構造体がかぶったため */
//テクスチャーあり描画初期化
typedef struct _tex_draw_data {
	DG_GSREG	texflush;	//ウェイト
	DG_GSREG	test;		//テスト値
	DG_GSREG	tex0;		//テクスチャー
	DG_GSREG	clamp;		//クランプモード

	DG_GSREG	prim;		//プリミティブ設定
	DG_GSREG	rgbaq;		//カラー
	DG_GSREG	alpha;		//アルファモード
} TEX_DRAW_DATA;

typedef struct {
	DG_GIFTAG		giftag;		//gifタグ
	TEX_DRAW_DATA	data;
} TEX_DRAW;
#endif

//全画面一枚描画（コントラストやクリアーに使う）
typedef struct _screen_draw_data {
	DG_GSREG	texflush;	//ウェイト
	DG_GSREG	test;		//テスト値
	DG_GSREG	prim;		//プリミティブ設定
	DG_GSREG	alpha;		//アルファモード
	DG_GSREG	rgbaq;		//カラー
	DG_GSREG	xyz0;		//
	DG_GSREG	xyz1;		//
} SCREEN_DRAW_DATA;

typedef struct {
	DG_GIFTAG			giftag;		//gifタグ
	SCREEN_DRAW_DATA	data;
} SCREEN_DRAW;

//フレーム設定
typedef struct _frame_set {
	DG_GIFTAG		giftag;
	DG_GSREG		frame;
} FRAME_SET;

//Ｚバッファー設定
typedef struct _zbuff_set {
	DG_GIFTAG		giftag;
	DG_GSREG		zbuff;
} ZBUFF_SET;

//フラッシュ
typedef struct {
	DG_GIFTAG		giftag;		//gifタグ
	DG_GSREG		texflush;	//ウェイト
} TEX_FLUSH;

//頂点
typedef struct _gif_stq {
	float		s;
	float		t;
	float		q;
	float		pad;
} _GIF_STQ;

typedef struct _gif_uv {
	int			u;
	int			v;
	int			pad[2];
} _GIF_UV;

typedef struct _gif_zyz2 {
	int			x;
	int			y;
	int			z;
	short		flags;
	short		pad2;
} _GIF_XYZ2;

typedef struct _gif_rgbaq {
	int			r;
	int			g;
	int			b;
	int			a;
} _GIF_RGBAQ;

typedef struct _vart_st {
	_GIF_STQ	stq;
	_GIF_XYZ2	xyz;
} _VERT_ST;

typedef struct _vart_uv {
	_GIF_UV		uv;
	_GIF_XYZ2	xyz;
} _VERT_UV;

typedef struct _vart_uv_rgba {
	_GIF_RGBAQ	rgba;
	_GIF_UV		uv;
	_GIF_XYZ2	xyz;
} _VERT_UV_RGBA;

typedef struct _vart_xyz_rgba {
	_GIF_RGBAQ	rgba;
	_GIF_XYZ2	xyz;
} _VERT_XYZ_RGBA;

typedef struct _vart_sprt_uv {
	_GIF_RGBAQ	rgba;
	_GIF_UV		uv0;
	_GIF_XYZ2	xyz0;
	_GIF_UV		uv1;
	_GIF_XYZ2	xyz1;
} _VERT_SPRT_UV;


#define		DRAW_DIV	(16)

typedef struct _sprt_parts{
	DG_GSREG	uv0;
	DG_GSREG	xyz0;
	DG_GSREG	uv1;
	DG_GSREG	xyz1;
} SPRT_PARTS;

typedef	struct _screen_draw_div_data{
		DG_GSREG	alpha;
		DG_GSREG	tex0;
		DG_GSREG	test;
		DG_GSREG	prim;
		DG_GSREG	rgbaq;
		SPRT_PARTS	sprt[DRAW_DIV];
} SCR_DRAW_DIV_DATA;

typedef struct _scr_draw_div {
	DG_GIFTAG			giftag;
	SCR_DRAW_DIV_DATA	data;
} SCR_DRAW_DIV ;

#define		GIFREG_VERT_ST			(0x0052)
#define		GIFREG_VERT_UV			(0x0053)
#define		GIFREG_VERT_UV_RGBA		(0x0531)
#define		GIFREG_VERT_SPRT_UV		(0x53531)

//addr上にパケット作成、次のアドレスを返す
// DMA_TAG作成
extern void *InitDmaVifCodeTags( void *addr,
								 u_int dmatag, void *send_addr,
								 u_int vifcode0, u_int vifcode1 );

// TEX_FLUSH初期化(giftagのみ)
extern void *InitTexFlush( void *addr );
// TEX_FLUSH初期化(DMA_TAG付)
extern void *AddDmaTagsTexFlush( void *addr );

// FRAME_SET初期化(giftagのみ)
extern void *InitFrameSet( void *addr, u_long64 frame );
// FRAME_SET初期化(DMA_TAG付)
extern void *AddDmaTagsFrameSet( void *addr, u_long64 frame );

// SCREEN_DRAW初期化(giftagのみ)
extern void *InitScreenDraw( void *addr,
							 u_long64 prim, u_long64 alpha, u_long64 rgbaq );
// SCREEN_DRAW初期化(DMA_TAG付)
extern void *AddDmaTagsScreenDraw( void *addr,
								   u_long64 prim, u_long64 alpha, u_long64 rgbaq );

// SET_DRAW_PRIM初期化(giftagのみ)
extern void *InitSetDrawPrim( void *addr,
							  u_long64 prim, u_long64 alpha,
							  u_long64 tex, u_long64 clamp, int test );


// TEX_DRAW初期化(giftagのみ)
extern void *InitTexDraw( void *addr,
						  u_long64 prim, u_long64 alpha,
						  u_long64 rgbaq, u_long64 tex, u_long64 clamp, int test );
// TEX_DRAW初期化(DMA_TAG付)
extern void *AddDmaTagsTexDraw( void *addr,
								u_long64 prim, u_long64 alpha,
								u_long64 rgbaq, u_long64 tex, u_long64 clamp, int test );

// SCR_DRAW_DIV初期化(giftagのみ)
extern void *InitScrDrawDiv( void *addr,
							 u_long64 prim, u_long64 alpha, u_long64 rgbaq,
							 u_long64 tex, u_long64 test, int z );

// ZBUFF_SET初期化(giftagのみ)
extern void *InitZbuffSet( void *addr, u_long64 zbuff );

#if 0 /* DEF_TEXLIST が未対応 */
// テクスチャー転送パケット作成
extern void *MakeDmaPacketTexPack( void *_addr, DEF_TEXLIST *texlist );
#endif

// 先頭にgiftag付加＋_VERT_ST頂点列初期化
extern void *InitVertsDatas_VERT_ST( void *addr,
									 int n_verts, int z,
									 int gifregs  );

#if 0
#define MAX_U		((DRAW_WIDTH<<4)-(1<<3))
#define MAX_V		((DRAW_HEIGHT<<4)-(1<<3))
#define MIN_UV		(1<<3)
#define	MAX_X		((2048+DRAW_WIDTH/2)<<4)
#define	MAX_Y		((2048+DRAW_HEIGHT/2)<<4)
#define	MIN_X		((2048-DRAW_WIDTH/2)<<4)
#define	MIN_Y		((2048-DRAW_HEIGHT/2)<<4)
#define	CENTER_X	((2048)<<4)
#define	CENTER_Y	((2048)<<4)

#define	CENTER_U	(DRAW_WIDTH<<3)
#define	CENTER_V	(DRAW_HEIGHT<<3)
#endif

#define _FTOI4(_f)		((int)((_f)*16.0f))


#endif
