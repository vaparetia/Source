/*
	dmapack.c
	ＤＭＡパケット接続型オブジェクト用ヘッダ

	2000/07/17 K.Takabe
	$Id: dmapack.h,v 1.12 2002/12/23 10:42:10 takaki Exp $

*/


#ifndef __DMAPACK_C__
#define __DMAPACK_C__

#ifdef __cplusplus
extern "C" {
#endif



/*----------------------------------------------------------------*/

#define	__SET_DMAPACK_INLINE__	(FALSE)
#define BP_DMAPACK_DEBUG_INFO 1

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* 制御属性 */
enum {
	DG_DMAPACK_NORMAL			= 0x0001,	/* 通常チャンネルで使用 */
	DG_DMAPACK_MENU				= 0x0002,	/* メニュー専用チャンネルで使用 */
	DG_DMAPACK_INVISIBLE0		= 0x0010,	/* チャンネル０で非表示 */
	DG_DMAPACK_INVISIBLE1		= 0x0020,	/* チャンネル１で非表示 */
	DG_DMAPACK_INVISIBLE2		= 0x0040,	/* チャンネル２で非表示 */
	DG_DMAPACK_INVISIBLE3		= 0x0080,	/* チャンネル３で非表示 */
	DG_DMAPACK_INVISIBLE		= 0x0030,	/* 全チャンネルで非表示 */
	DG_DMAPACK_INVISIBLEMENU	= 0x0100,	/* メニューチャンネルで非表示 */
	DG_DMAPACK_PRIVILEGE		= 0x8000,	/* 特権オブジェクトフラグ */
} ;
/* 描画フェーズ */
enum {
	DG_DMAPACK_PHASE_FIRST	= DG_PLUGIN_PHASE_FIRST,	/* 描画開始前 */
	DG_DMAPACK_PHASE_NORMAL	= DG_PLUGIN_PHASE_NORMAL,	/* 不透明モデル描画後 */
	DG_DMAPACK_PHASE_AFTER	= DG_PLUGIN_PHASE_AFTER,	/* 半透明モデル描画後 */
	DG_DMAPACK_PHASE_LAST	= DG_PLUGIN_PHASE_LAST,		/* 最終描画 */
} ;

typedef ALIGN16_DECL(struct) _dg_dmapack {
	int			flag ;					/* 制御フラグ */
	short		phase ;
	short		priority ;				/* 登録プライオリティ */
	void		*packet[2] ;			/* ＤＭＡパケット（CALLタグでDMAに接続される） */
#if 1 //BP_XBOX def KP_XBOX
	/* ＸＢＯＸでＤＭＡＰＡＣＫ相当を実現するためのコールバック機構 */
   //BP This callback is used to build the auto packets
   void		*buildAutoPacketCallbackParam;
   void		(*buildAutoPacketCallback)( void *param );

   void		*BP_callbackParam ;
   void		(*BP_renderCallback)( void *param );
   void *   (*BP_bufferCallback)( void *param, struct _dg_dmapack *pNewDmaPack );
	void		*autopacket ;
	int		autopacketSize;
#endif
#if BP_DMAPACK_DEBUG_INFO
	char		*fname ;
   unsigned int BP_LabelMask;
	int			_pad[2] ;
#endif
} DG_DMAPACK  ;


/* ＸＢＯＸ用自動処理用簡易２Ｄプリミティブ構造体 */
/*
	以下の形式でプリミティブを作成することでＤＭＡＰＡＣＫシステムがDirectXにて
	描画を行う機能。ＰＳ２とは互換性がないので注意
	主に
 */
#ifndef KP_WINDOWS
enum {
	DG_DMAPACK_CMD_POINT,				/* ポイント（未サポート） */
	DG_DMAPACK_CMD_LINE,				/* ライン */
   DG_DMAPACK_CMD_LINE_F, // BP_ENGINE - Added for xsprite_dma.c for radar sprites are using fractional pixels
	DG_DMAPACK_CMD_TRIANGLE,			/* トライアングル */
	DG_DMAPACK_CMD_QUAD,				/* 四角形 */
	DG_DMAPACK_CMD_BOX,					/* テクスチャ無しスプライト */
	DG_DMAPACK_CMD_SPRT,				/* テクスチャ付きスプライト */
	DG_DMAPACK_CMD_RSPRT,				/* 回転テクスチャ付きスプライト（四角形） */
   DG_DMAPACK_CMD_SPRT_F,
   DG_DMAPACK_CMD_RSPRT_F,
   DG_DMAPACK_CMD_BOX_F,
   DG_DMAPACK_CMD_QUAD_F,

	DG_DMAPACK_CMD_LINESTRIP,			/* ラインストリップ */
   DG_DMAPACK_CMD_LINESTRIP_F, // BP_ENGINE - Added for xsprite_dma.c for radar sprites are using fractional pixels
	DG_DMAPACK_CMD_TRIANGLESTRIP,		/* トライアングルストリップ */

	DG_DMAPACK_CMD_MODE,				/* 各種設定 *//* もう使ってはいけない */
	DG_DMAPACK_CMD_TEX,					/* テクスチャ設定 */
   DG_DMAPACK_CMD_TEXDYN,				/* テクスチャ設定 */
   DG_DMAPACK_CMD_TEXDYN01,			/* テクスチャ設定 */
	DG_DMAPACK_CMD_TEXLIN,				/* 線形テクスチャ設定 */
	DG_DMAPACK_CMD_ALPHA,				/* アルファ設定 */
	DG_DMAPACK_CMD_WINDOW,				/* 描画領域の設定 */
   DG_DMAPACK_CMD_VIEWMAPPING,
	DG_DMAPACK_CMD_SETZ,				/* 描画時のＺ値を設定 */
	DG_DMAPACK_CMD_ENABLE,				/* 各種設定 */
	DG_DMAPACK_CMD_DISABLE,				/* 各種設定 */

	DG_DMAPACK_CMD_USEFRAMETEX,			/* フレームバッファをテクスチャとして設定 */
	DG_DMAPACK_CMD_BACKUPFRAME,			/* 現在描画中のフレームバッファを退避 */

	DG_DMAPACK_CMD_CALLBACK,			/* コールバックを設定（移植した際に互換性がなくなるので注意！） */

	DG_DMAPACK_CMD_NEXT,				/* 指定アドレスのパケットへジャンプ */

	DG_DMAPACK_CMD_END,					/* パケットの終了 */
};
#else	// for Windows

#undef	DMAPACK_CMD_EQU
#define	DMAPACK_CMD_EQU(cmd_, func_)	DG_DMAPACK_CMD_ ## cmd_,
enum
{
#include "dmapack_cmd.h"
} ;
#undef	DMAPACK_CMD_EQU
#endif

enum {
	DG_DMAPACK_MODE_NO_BILINEAR			= 0x0001,		/* バイリニア禁止 */
	DG_DMAPACK_MODE_NO_ALPHA			= 0x0002,		/* アルファ禁止 */
	DG_DMAPACK_MODE_NO_ALPHATEST		= 0x0004,		/* アルファテスト禁止 */
	DG_DMAPACK_MODE_MODULATE1X			= 0x0008,		/* カラースケールの禁止（ブレンド精度向上のため） */
	DG_DMAPACK_MODE_ZTEST_REV			= 0x0010,		/* デプステスト条件の反転 */
	DG_DMAPACK_MODE_NO_ZOVERWRITE		= 0x0020,		/* Ｚバッファの上書き禁止 */
	DG_DMAPACK_MODE_NO_ZTEST			= 0x0040,		/* デプステスト禁止 */
	DG_DMAPACK_MODE_COLORMASK			= 0x0080,		/* フレームバッファのマスクＯＮ */
	DG_DMAPACK_MODE_NO_TEXALPHA			= 0x0100,		/* テクスチャのアルファ無視（頂点アルファのみ参照） */
#ifdef KP_WINDOWS
	DG_DMAPACK_MODE_WRAP				= 0x0200,		/* テクスチャのWRAP指定 */
#endif
};

typedef struct {
#if BPE_IS_ENDIAN_BIG()
   unsigned char a;
   unsigned char b;
   unsigned char g;
   unsigned char r;
#else
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
#endif
} DG_DMAPACK_COLOR;

static inline DG_DMAPACK_COLOR DG_MakeDMAPackColor( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
   DG_DMAPACK_COLOR c;

   // Not using c = { r, g, b, a } because the order of these elements changes depending on endianness
   c.r = r;
   c.g = g;
   c.b = b;
   c.a = a;

   return c;
}

static inline DG_DMAPACK_COLOR DG_MakeDmaPackColorFromInt( unsigned int i )
{
   DG_DMAPACK_COLOR ret;

   *( (unsigned int *)(&ret) ) = i;
   return ret;
}

#define DG_RGBATODMARGBA(_r) DG_MakeDMAPackColor(_r.r,_r.g,_r.b,_r.a)
#define DG_SCERGBATODMARGBA(_r) DG_MakeDMAPackColor(_r.R,_r.G,_r.B,_r.A)

/* ---------------------------------------------------------------- */
typedef struct {
	unsigned char	cmd ;	/* 認識ＩＤ */
	unsigned char	pad[3] ;
} DG_DMAPACK_TAG ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	int				param ;
} DG_DMAPACK_PARAM ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	DG_TEX			*tex ;	
} DG_DMAPACK_TEX ;

typedef struct {
   DG_DMAPACK_TAG	header ;
   int            hasvalidTex;
   DG_TEX			tex ;	
} DG_DMAPACK_TEXTURE_BP ;

typedef struct {
   DG_DMAPACK_TAG	header ;
   int            hasvalidTex;
   DG_TEX_LIN	   tex ;	
} DG_DMAPACK_TEXTURE_LINEAR_BP ;

typedef struct {
   DG_DMAPACK_TAG	header;
   DG_TEX			*tex;
   unsigned int   bp_tex;
} DG_DMAPACK_TEXTURE_DYNAMIC;

typedef struct {
   DG_DMAPACK_TAG	header;
   DG_TEX			tex;
   unsigned int   bp_tex;
} DG_DMAPACK_TEXTURE_DYNAMIC_BP;

typedef struct {
   DG_DMAPACK_TAG	header;
   unsigned int   bp_tex;
} DG_DMAPACK_TEXTURE_DYNAMIC01_BP;

typedef struct {
	DG_DMAPACK_TAG	header ;
#if 0
	u_long64			alpha ;
#else
	u_int			alpha0, alpha1 ;
#endif
} DG_DMAPACK_ALPHA ;

typedef struct {
	DG_DMAPACK_TAG	header ;
   void		      (*callback)(void * data);
   int            dataSize;
} DG_DMAPACK_CALLBACK ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	void			*next ;
} DG_DMAPACK_NEXT ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned short	x, y ;
	unsigned short	w, h ;
} DG_DMAPACK_WINDOW ;

typedef struct 
{
   DG_DMAPACK_TAG	header;
   float		x0, y0, x1, y1;
} DG_DMAPACK_VIEWMAPPING;

typedef struct {
	DG_DMAPACK_TAG	header ;
	int				param ;
	unsigned short	x, y ;
	unsigned short	w, h ;
} DG_DMAPACK_BACKUPFRAME_RECT ;

/* ---------------------------------------------------------------- */
typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba ;
	signed short	x, y ;
} DG_DMAPACK_POINT ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba0 ;
	signed short	x0, y0 ;
	unsigned int	rgba1 ;
	signed short	x1, y1 ;
} DG_DMAPACK_LINE ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba0 ;
	float			x0, y0 ;
	unsigned int	rgba1 ;
	float			x1, y1 ;
} DG_DMAPACK_LINE_F ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba0 ;
	signed short	x0, y0 ;
	unsigned int	rgba1 ;
	signed short	x1, y1 ;
	unsigned int	rgba2 ;
	signed short	x2, y2 ;
} DG_DMAPACK_TRIANGLE ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba0 ;
	signed short	x0, y0 ;
	unsigned int	rgba1 ;
	signed short	x1, y1 ;
	unsigned int	rgba2 ;
	signed short	x2, y2 ;
	unsigned int	rgba3 ;
	signed short	x3, y3 ;
} DG_DMAPACK_QUAD ;

typedef struct {
   DG_DMAPACK_TAG	header ;
   unsigned int	rgba0 ;
   float	x0, y0 ;
   unsigned int	rgba1 ;
   float	x1, y1 ;
   unsigned int	rgba2 ;
   float	x2, y2 ;
   unsigned int	rgba3 ;
   float	x3, y3 ;
} DG_DMAPACK_QUAD_F ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba ;
	signed short	x0, y0 ;
	signed short	x1, y1 ;
} DG_DMAPACK_BOX ;

typedef struct
{
   DG_DMAPACK_TAG header;
   unsigned int rgba;
   float	x0, y0;
   float	x1, y1;
}
DG_DMAPACK_BOX_F;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba ;
	signed short	u0, v0 ;
   signed short	x0, y0 ;
   signed short	u1, v1 ;
	signed short	x1, y1 ;
} DG_DMAPACK_SPRT ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba ;
	signed short	u0, v0 ;
	signed short	u1, v1 ;
	signed short	x0, y0 ;
	signed short	x1, y1 ;
	signed short	x2, y2 ;
	signed short	x3, y3 ;
} DG_DMAPACK_RSPRT ;

typedef struct {
   DG_DMAPACK_TAG header;
   unsigned int rgba;
   float u0, v0;
   float u1, v1;
   float x0, y0;
   float x1, y1;
   unsigned int flags;
} DG_DMAPACK_SPRT_F;

typedef struct {
   DG_DMAPACK_TAG header;
   unsigned int	rgba ;
   float u0, v0;
   float u1, v1;
   float x0, y0;
   float x1, y1;
   float x2, y2;
   float x3, y3;
} DG_DMAPACK_RSPRT_F;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	v_count ;
} DG_DMAPACK_LINESTRIP ;

typedef struct {
   DG_DMAPACK_TAG	header ;
   unsigned int	v_count ;
} DG_DMAPACK_LINESTRIP_F ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	v_count ;
} DG_DMAPACK_TRIANGLESTRIP ;

typedef struct {
	DG_DMAPACK_TAG	header ;
	unsigned int	rgba ;
	signed short	u, v ;
	signed short	x, y ;
} DG_DMAPACK_VERTEX ;

typedef struct {
   DG_DMAPACK_TAG	header ;
   unsigned int	rgba ;
   float          u, v ;
   float          x, y ;
} DG_DMAPACK_VERTEX_F ;

/* パケット生成関数郡 */
extern void *DG_SetDmapackEnd( void *addr );
extern void *DG_SetDmapackMode( void *addr, int mode );
extern void *DG_SetDmapackModeEnable( void *addr, int mode );
extern void *DG_SetDmapackModeDisable( void *addr, int mode );
extern void *DG_SetDmapackTex( void *addr, DG_TEX *tex );
extern void *DG_SetDmapackTextureDynamic( void *buffer, DG_TEX *tex, unsigned int bp_texture );
extern void *DG_SetDmapackTextureDynamic01( void *buffer, DG_TEX *tex, unsigned int bp_texture );

extern void *DG_SetDmapackTexLin( void *addr, DG_TEX_LIN *tex );
#ifdef KP_WINDOWS
extern void *DG_SetDmapackUserTex( void *addr, DG_USERTEX *tex );
#endif
extern void *DG_SetDmapackAlpha( void *addr, u_long64 alpha );
extern void *DG_SetDmapackWindow( void *addr, int x, int y, int w, int h );
extern void *DG_SetDmapackViewMapping( void *buffer, float x0, float y0, float x1, float y1 );
extern void *DG_SetDmapackSetZ( void *addr, float z );
extern void *DG_SetDmapackUseFrameTex( void *addr, int page );
extern void *DG_SetDmapackBackupFrame( void *addr, int page );
extern void *DG_SetDmapackBackupFrameRect(void *addr,int page,int x,int y,int w,int h);
extern void *DG_SetDmapackRendBackBuffer2Frame( void *addr );
#if __SET_DMAPACK_INLINE__
extern void *DG_SetDmapackNext( void *addr, void *next );
#endif
extern void *DG_SetDmapackPoint( void *addr, float x0, float y0, DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackLine( void *addr, float x0, float y0, DG_DMAPACK_COLOR rgba0, float x1, float y1, DG_DMAPACK_COLOR rgba1 );
extern void *DG_SetDmapackLine_F( void *addr, float x0, float y0, DG_DMAPACK_COLOR rgba0, float x1, float y1, DG_DMAPACK_COLOR rgba1 );
extern void *DG_SetDmapackTriangle( void *addr,
								   float x0, float y0, DG_DMAPACK_COLOR rgba0,
								   float x1, float y1, DG_DMAPACK_COLOR rgba1,
								   float x2, float y2, DG_DMAPACK_COLOR rgba2 );
extern void *DG_SetDmapackQuad( void *addr,
							   float x0, float y0, DG_DMAPACK_COLOR rgba0,
							   float x1, float y1, DG_DMAPACK_COLOR rgba1,
							   float x2, float y2, DG_DMAPACK_COLOR rgba2,
							   float x3, float y3, DG_DMAPACK_COLOR rgba3 );
extern void *DG_SetDmapackQuad_F( void *addr,
                               float x0, float y0, DG_DMAPACK_COLOR rgba0,
                               float x1, float y1, DG_DMAPACK_COLOR rgba1,
                               float x2, float y2, DG_DMAPACK_COLOR rgba2,
                               float x3, float y3, DG_DMAPACK_COLOR rgba3 );
extern void *DG_SetDmapackBox( void *addr, float x0, float y0, float x1, float y1, DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackBox_F( void *addr, float x0, float y0, float x1, float y1, DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackSprt( void *addr,
							   float x0, float y0, float u0, float v0,
							   float x1, float y1, float u1, float v1, DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackSprt_F( void *addr,
                               float x0, float y0, float u0, float v0,
                               float x1, float y1, float u1, float v1, DG_DMAPACK_COLOR rgba, unsigned int sprFlags );
extern void *DG_SetDmapackSprtTex( void *addr,
								  float x0, float y0, float u0, float v0,
								  float x1, float y1, float u1, float v1, DG_DMAPACK_COLOR rgba, DG_TEX *tex );
extern void *DG_SetDmapackRSprt( void *addr,
								float x0, float y0, float x1, float y1,
								float x2, float y2, float x3, float y3,
								float u0, float v0, float u1, float v1,
								DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackRSprt_F( void *addr,
                                float x0, float y0, float x1, float y1,
                                float x2, float y2, float x3, float y3,
                                float u0, float v0, float u1, float v1,
                                DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackRSprtTex( void *addr,
								   float x0, float y0, float x1, float y1,
								   float x2, float y2, float x3, float y3,
								   float u0, float v0, float u1, float v1,
								   DG_DMAPACK_COLOR rgba, DG_TEX *tex );
extern void *DG_SetDmapackLineStrip( void *addr, int num );
extern void *DG_SetDmapackLineStrip_F( void *addr, int num );
extern void *DG_SetDmapackTriangleStrip( void *addr, int num );
extern void *DG_SetDmapackVertex( void *addr, float x, float y, float u, float v, DG_DMAPACK_COLOR rgba );
extern void *DG_SetDmapackVertex_F( void *addr, float x, float y, float u, float v, DG_DMAPACK_COLOR rgba );
extern void *DG_DmapackSetCallback( void *buffer, void (*callback)(void*), int dataSize );

#if 1 //BP_XBOX def KP_XBOX

#define DG_FRAME_U( _u )	( (float)(_u) * ( 1.0f / DRAW_WIDTH ) )
#define DG_FRAME_V( _v )	( (float)(_v) * ( 1.0f / DRAW_HEIGHT ) )

/* inline */
#if __SET_DMAPACK_INLINE__

static inline void *DG_SetDmapackNext( void *addr, void *next )
{
	DG_DMAPACK_NEXT *packet = (DG_DMAPACK_NEXT *)addr ;
	packet->header.cmd = DG_DMAPACK_CMD_NEXT ;
	packet->next = next ;
	return ( (void*)&packet[1] );
}
#endif

#endif

#ifdef KP_XBOX
/* DMAPACK用頂点シェーダー用頂点フォーマット */
typedef struct {
	signed short	x, y ;
	unsigned int	rgba ;
	signed short	u, v ;
} DG_VERTEX_DMAPACK2D ;

typedef struct {
	signed short	x, y ;
	unsigned int	rgba ;
	signed short	u0, v0 ;
	signed short	u1, v1 ;
	signed short	u2, v2 ;
	signed short	u3, v3 ;
} DG_VERTEX_DMAPACK2D4 ;

/* DMAPACK用頂点シェーダー用頂点フォーマット(Vertex Shader非対応版) */
typedef struct {
	float			x, y, z ;
	D3DCOLOR		rgba ;
	float			u, v ;
} DG_VERTEX_DMAPACK2D_FVF ;
#define	D3DFVF_DG_VERTEX_DMAPACK2D_FLAG	( D3DFVF_XYZ		\
										| D3DFVF_DIFFUSE	\
										| D3DFVF_TEX1		\
										| D3DFVF_TEXCOORDSIZE2(0))

typedef struct {
	float			x, y, z ;
	D3DCOLOR		rgba ;
	float			u0, v0 ;
	float			u1, v1 ;
} DG_VERTEX_DMAPACK2D2_FVF ;
#define	D3DFVF_DG_VERTEX_DMAPACK2D2_FLAG	( D3DFVF_XYZ		\
											| D3DFVF_DIFFUSE	\
											| D3DFVF_TEX2		\
											| D3DFVF_TEXCOORDSIZE2(1)	\
											| D3DFVF_TEXCOORDSIZE2(0))

typedef struct {
	float			x, y, z ;
	D3DCOLOR		rgba ;
	float			u0, v0 ;
	float			u1, v1 ;
	float			u2, v2 ;
	float			u3, v3 ;
} DG_VERTEX_DMAPACK2D4_FVF ;
#define	D3DFVF_DG_VERTEX_DMAPACK2D4_FLAG	( D3DFVF_XYZ		\
											| D3DFVF_DIFFUSE	\
											| D3DFVF_TEX4		\
											| D3DFVF_TEXCOORDSIZE2(0)	\
											| D3DFVF_TEXCOORDSIZE2(1)	\
											| D3DFVF_TEXCOORDSIZE2(2)	\
											| D3DFVF_TEXCOORDSIZE2(3))


typedef	struct	DG_BACKUPFRAME2SURFACE_PPARAM_
{
	int	src_page ;	// 転送元Page(0:描画中フレーム, 1:表示中フレーム, Other:退避フレーム)

#if 0 //BP
	LPDIRECT3DSURFACE8	dst_surface ;	// 転送先
#endif

	float	dst_x ;	// 転送先領域(0〜1で正規化)
	float	dst_y ;
	float	dst_w ;
	float	dst_h ;
} DG_BACKUPFRAME2SURFACE_PARAM ;
#endif



/*----------------------------------------------------------------*/
/* dmapack.c */
extern void DG_AddPluginDmapack( void );
extern void DG_DeletePluginDmapack( void );
#if !BP_DMAPACK_DEBUG_INFO 
extern DG_DMAPACK* DG_MakeDmapack2( int flag, int phase, int priority );
extern DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
#else
extern DG_DMAPACK* DG_MakeDmapack2_D( int flag, int phase, int priority, char *file );
extern DG_DMAPACK* DG_MakeDmapack_D( int flag, int phase, char *file );
#define DG_MakeDmapack2( flag, phase, priority ) DG_MakeDmapack2_D( flag, phase, priority, __FILE__ )
#define DG_MakeDmapack( flag, phase ) DG_MakeDmapack_D( flag, phase, __FILE__ )
#endif
extern void DG_FreeDmapack( DG_DMAPACK *dmapack );
extern int DG_QueueDmapack( DG_DMAPACK * ) ;
extern void DG_DequeueDmapack( DG_DMAPACK * ) ;

#if 1 //BP_XBOX def KP_XBOX
extern void DG_ExecAuto2DPrim( void *addr );
extern void DG_SetDmapackCallback( DG_DMAPACK *dmapack, void *callback, void *param );
//BP_XBOX extern void	DG_BackupFrame2Surface(DG_BACKUPFRAME2SURFACE_PARAM *param) ;
#endif

static inline void DG_AS_DMAPackSetCallbacks( DG_DMAPACK *pDmaPack, void (*BP_renderCallback)( void *param ), void * (*BP_bufferCallback)( void *param, struct _dg_dmapack *pNewDmaPack ) )
{
   pDmaPack->BP_renderCallback = BP_renderCallback;
   pDmaPack->BP_bufferCallback = BP_bufferCallback;
}

#/*----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
