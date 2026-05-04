/*
	wdmapack.c
	ＤＭＡパケット接続型オブジェクト管理ルーチン

	2002/02/15 K.Takabe
	$Id: wdmapack.c,v 1.43 2002/12/25 04:35:21 takaki Exp $

*/
/*

----------------------------------------------------------------

	●ＤＭＡＰＡＣＫオブジェクトのＸＢＯＸ拡張について

	ＸＢＯＸではＰＳ２形式のＤＭＡパケットは使用できない為、コールバックと
	自動描画２Ｄプリミティブ機能を追加しました。

	＜コールバックについて＞
	これは実際のＬＩＢＤＧ内の描画フェーズにコールバックを呼ぶことで本来のDirectXを
	使用した描画を行えるように拡張したものです。ただし、この機能を使用すると完全に
	XBOX専用になるのでどうしても使わなければならないものや、ＰＳ２版で行っていた
	特殊処理をXBOX用に移植する場合以外には使用しないでください（移植が困難になるため）
	設定はDG_SetDmapackCallback()関数で行ってください。

	＜自動描画２Ｄプリミティブ機能について＞
	ＰＳ２のＤＭＡパケットのように２Ｄの描画パケットを独自形式で記録し、
	ＬＩＢＤＧ内で自動的に描画を行う為の機能です。基本的な２Ｄ描画はこれを使用するように
	してください。

	・使用可能な描画の種類
	　　ポイント（未サポート）
	　　ライン（テクスチャ無し）
	　　３角形（テクスチャ無し）
	　　４角形（テクスチャ無し）
	　　テクスチャ無しスプライト
	　　テクスチャ付きスプライト
	　　回転スプライト
	　　ラインストリップ（未サポート）
	　　３角形ストリップ（未サポート）
	・使用可能な特殊パケット
	　　各種モード設定
	　　テクスチャ指定（DG_TEXを指定）
	　　アルファ指定（ＰＳ２形式で指定）
	　　指定したアドレスへのジャンプ
	　　終端
	今後必要に応じてバリエーションが増えるかも知れません。

	パケット自体は内部フォーマットの変更を考慮して全て関数を通して作成するようにしてください。
	パケット生成用関数は全て返値として次のアドレスを返すようになってます。


	・使い方
	初期化）
		dmapack = DG_MakeDmapack2( DMAPACK_FLAG, DMAPACK_PHASE, DMAPACK_PRIO );
		dmapack->autopacket = &work->local_buffer[0] ;
		work->last = &work->local_buffer[0] ;
		work->last = DG_SetDmapackEnd( work->last );
	メイン）
		work->last = &work->local_buffer[0] ;
		work->last = DG_SetDmapackTex( work->last, tex );
		work->last = DG_SetDmapackSprtTex( work->last, 0, 0, DRAW_WIDTH, DRAW_HEIGHT, 0,0f, 0.0f, 1.0f, 1.0f, tex )
		work->last = DG_SetDmapackEnd( work->last );

	autopacketメンバに渡すバッファワークのサイズは追加するパケット内容によって
	変化するのである程度余裕を持って指定するか、dmapack.h内に記述されている
	構造体のサイズを計算したサイズで確保するようにしてください。
	また、関数の詳細はdmapack.cの先頭を参照してください


----------------------------------------------------------------

extern void *DG_SetDmapackEnd( void *addr );
	void		*addr ;		書き込みアドレス
	終端コードの書き込み

extern void *DG_SetDmapackMode( void *addr, int mode );
	void		*addr ;		書き込みアドレス
	int			mode ;		モードフラグ
	モード指定パケットの書き込み（使用禁止）

extern void *DG_SetDmapackModeEnable( void *addr, int mode );
	void		*addr ;		書き込みアドレス
	int			mode ;		モードフラグ
	モード指定パケットの書き込み（有効化）

extern void *DG_SetDmapackModeDisable( void *addr, int mode );
	void		*addr ;		書き込みアドレス
	int			mode ;		モードフラグ
	モード指定パケットの書き込み（無効化）

extern void *DG_SetDmapackTex( void *addr, DG_TEX *tex );
	void		*addr ;		書き込みアドレス
	DG_TEX		*tex ;		テクスチャ指定
	テクスチャ設定パケットの書き込み

extern void *DG_SetDmapackTexLin( void *addr, DG_TEX_LIN *tex );
	void		*addr ;		書き込みアドレス
	DG_TEX_LIN	*tex ;		線形テクスチャ指定
	線形テクスチャ設定パケットの書き込み

extern void *DG_SetDmapackAlpha( void *addr, u_long alpha );
	void		*addr ;		書き込みアドレス
	u_long		alpha ;		アルファ設定
	アルファ設定パケットの書き込み

extern void *DG_SetDmapackWindow( void *addr, int x, int y, int w, int h );
	void		*addr ;		書き込みアドレス
	int			x ;			描画ウィンドウ座標（左上）
	int			y ;			描画ウィンドウ座標（左上）
	int			w ;			描画ウィンドウ幅
	int			h ;			描画ウィンドウ高さ
	描画する領域を画面座標単位で指定（指定した左上座標が０点になる）

extern void *DG_SetDmapackSetZ( void *addr, float z );
	void		*addr ;		書き込みアドレス
	float		z ;			２Ｄ書き込みＺ値（手前：１～奥：０）
	２Ｄを描画する際のＺ値を指定する

void *DG_SetDmapackUseFrameTex( void *addr, int page )
	void		*addr ;		書き込みアドレス
	int			page ;		ページ指定（０：描画中バッファ、１：表示中バッファ、２退避バッファ）
	フレームバッファをテクスチャとして設定

void *DG_SetDmapackBackupFrame( void *addr, int page )
	void		*addr ;		書き込みアドレス
	int			page ;		ページ指定（０で現在描画中のフレームバッファ、１で表示中のフレームバッファ）
							　0x02:バイリニアを用いて暈した画像を退避
							　0x04:フレームバッファをモノクロ化して退避
							但し、ページにサイズがDISPLAY_WIDTHxDISPLAY_HEIGHTのDirect3DSurface8のポインタを
							キャストして渡すことで任意のサーフェスに退避することも可能
	フレームバッファを退避バッファへ転送

extern void *DG_SetDmapackPacketCallback( void *addr, void *func, int param )
	void		*addr ;		書き込みアドレス
	void		*func ;		コールバック関数
	int			param ;		コールバックに渡されるパラメータ
	任意のパケット処理タイミングでコールバック関数を呼ぶ

extern void *DG_SetDmapackNext( void *addr, void *next );
	void		*addr ;		書き込みアドレス
	void		*next ;		次のパケットアドレス
	接続パケットの書き込み

extern void *DG_SetDmapackPoint( void *addr, float x0, float y0, int rgba );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba ;		色
	点パケットの書き込み

extern void *DG_SetDmapackLine( void *addr, float x0, float y0, int rgba0, float x1, float y1, int rgba1 );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba0 ;		色
	float		x1, y1 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba1 ;		色
	ラインパケットの書き込み

extern void *DG_SetDmapackTriangle( void *addr,
								   float x0, float y0, int rgba0,
								   float x1, float y1, int rgba1,
								   float x2, float y2, int rgba2 );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba0 ;		色
	float		x1, y1 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba1 ;		色
	float		x2, y2 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba2 ;		色
	三角形パケットの書き込み

extern void *DG_SetDmapackQuad( void *addr,
							   float x0, float y0, int rgba0,
							   float x1, float y1, int rgba1,
							   float x2, float y2, int rgba2,
							   float x3, float y3, int rgba3 );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba0 ;		色
	float		x1, y1 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba1 ;		色
	float		x2, y2 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba2 ;		色
	float		x3, y3 ;	座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba3 ;		色
	四角形パケットの書き込み
	但し、頂点は右もしくは左に１回転するように指定する

extern void *DG_SetDmapackBox( void *addr, float x0, float y0, float x1, float y1, int rgba );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	左上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		x1, y1 ;	右下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	int			rgba ;		色
	テクスチャ無しスプライトパケットの書き込み

extern void *DG_SetDmapackSprt( void *addr,
							   float x0, float y0, float u0, float v0,
							   float x1, float y1, float u1, float v1, int rgba );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	左上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u0, v0 ;	ＵＶ値（補正後ＵＶ値）
	float		x1, y1 ;	右下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u1, v1 ;	ＵＶ値（補正後ＵＶ値）
	int			rgba ;		色
	スプライトパケットの書き込み

extern void *DG_SetDmapackSprtTex( void *addr,
								  float x0, float y0, float u0, float v0,
								  float x1, float y1, float u1, float v1, int rgba, DG_TEX *tex );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	左上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u0, v0 ;	ＵＶ値（補正後ＵＶ値）
	float		x1, y1 ;	右下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u1, v1 ;	ＵＶ値（補正後ＵＶ値）
	int			rgba ;		色
	DG_TEX		*tex ;		描画に使用するテクスチャ
	スプライトパケットの書き込み（ＵＶ座標の補正機能付き）
	但しテクスチャは設定されないので別途テクスチャ設定パケットは登録しておく必要があるので注意

extern void *DG_SetDmapackRSprt( void *addr,
								float x0, float y0, float x1, float y1,
								float x2, float y2, float x3, float y3,
								float u0, float v0, float u1, float v1,
								int rgba );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	左上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u0, v0 ;	ＵＶ値（補正後ＵＶ値）
	float		x1, y1 ;	右上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u1, v1 ;	ＵＶ値（補正後ＵＶ値）
	float		x2, y2 ;	左下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u2, v2 ;	ＵＶ値（補正後ＵＶ値）
	float		x3, y3 ;	右下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u3, v3 ;	ＵＶ値（補正後ＵＶ値）
	int			rgba ;		色
	回転スプライトの書き込み

extern void *DG_SetDmapackRSprtTex( void *addr,
								   float x0, float y0, float x1, float y1,
								   float x2, float y2, float x3, float y3,
								   float u0, float v0, float u1, float v1,
								   int rgba, DG_TEX *tex );
	void		*addr ;		書き込みアドレス
	float		x0, y0 ;	左上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u0, v0 ;	ＵＶ値（補正後ＵＶ値）
	float		x1, y1 ;	右上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u1, v1 ;	ＵＶ値（補正後ＵＶ値）
	float		x2, y2 ;	左下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u2, v2 ;	ＵＶ値（補正後ＵＶ値）
	float		x3, y3 ;	右下座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u3, v3 ;	ＵＶ値（補正後ＵＶ値）
	DG_TEX		*tex ;		描画に使用するテクスチャ
	int			rgba ;		色
	回転スプライトの書き込み（ＵＶ座標の補正機能付き）

extern void *DG_SetDmapackLineStrip( void *addr, int num );
	void		*addr ;		書き込みアドレス
	int			num ;		含まれる頂点数
	ラインストリップヘッダーの書き込み
	この直後に頂点追加関数を使ってnumの分だけ頂点を追加する必要があるので注意

extern void *DG_SetDmapackTriangleStrip( void *addr, int num );
	void		*addr ;		書き込みアドレス
	int			num ;		含まれる頂点数
	トライアングルストリップヘッダーの書き込み
	この直後に頂点追加関数を使ってnumの分だけ頂点を追加する必要があるので注意

extern void *DG_SetDmapackVertex( void *addr, float x, float y, float u, float v, int rgba );
	void		*addr ;		書き込みアドレス
	float		x, y ;		左上座標（0~DRAW_WIDTH,0~DRAW_HEIGHT）
	float		u, v ;		ＵＶ値
	int			rgba ;		色
	ライン及びトライアングルストリップ用頂点追加書き込み

	

*/

#ifdef _XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "libdg.cnf"

//#include	"gameheader.h"

#include "dmapack.h"
#include	"def_dma.h"

#include	"shader.h"

#define	__X2W_TEST_DEBUG__	(TRUE)

extern LPDIRECT3DTEXTURE8	DG_GetFrontBufferTexture ;	// GetFrontBuffer用テクスチャ
extern LPDIRECT3DTEXTURE8	DG_BackBuffer2Texture ;		// BackBuffer2用テクスチャ
extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;
extern LPDIRECT3DSURFACE8	DG_BackBufferSurface[3] ;
extern LPDIRECT3DSURFACE8	DG_BackBufferDepthSurface ;
extern DWORD				DG_BackBufferRendWidth ;	// 描画に使用する範囲
extern DWORD				DG_BackBufferRendHeight ;
extern float				DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
extern float				DG_BackBufferHeightAdjustCoef ;	// RendHeight/Height
extern FVECTOR				DG_BackBufferTexScaleParam[2] ;// VertexShader用定数CV_TEX0_SCALE
extern BOOL					DG_RendToBackBuffer ;			// BackBuffer描画処理中フラグ
extern DWORD				DG_SkipRendBackBuffer2FrameCntr ;

/* ---------------------------------------------------------------- */

/* DG_VERTEX_DMAPACK2D頂点へのデータ設定(Warning対策) */
#define	setVertexDMAPack2DXY(dst_, x_, y_)	\
				{(dst_)->x = (short)(x_);(dst_)->y = (short)(y_);}
#define	setVertexDMAPack2DUV(dst_, u_, v_)	\
				{(dst_)->u = (short)(u_);(dst_)->v = (short)(v_);}
#define	setVertexDMAPack2DRGBA(dst_, rgba_)	\
				{(dst_)->rgba = (rgba_);}

/* DG_VERTEX_DMAPACK2D頂点へのデータ設定(VertexShader非対応版)(Warning対策) */
#define	setVertexDMAPack2DFVFXY(dst_, x_, y_)	\
				{(dst_)->x = (float)(x_);(dst_)->y = (float)(y_);(dst_)->z=0.0f;}
#define	setVertexDMAPack2DFVFUV(dst_, u_, v_)	\
				{(dst_)->u = (float)(u_);(dst_)->v = (float)(v_);}
#define	setVertexDMAPack2DFVFRGBA(dst_, rgba_)	\
				{(dst_)->rgba = (rgba_);}

static inline	void	SetScreenParamTransform(FVECTOR	*param)
{
	FMATRIX	mtx ;

	mtx = DG_UnitMatrix ;
	mtx.m[0][0] = param[1].vx ;
	mtx.m[1][1] = param[1].vy ;
	mtx.m[2][2] = 0.0f ;				// 入力Ｚ値を無視
	mtx.m[3][0] = param[0].vx ;
	mtx.m[3][1] = param[0].vy ;
	mtx.m[3][2] = param[0].vz ;

	DG_SetTransform(D3DTS_WORLD, &mtx) ;
}

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* RGBAからARGBへの変換 */
#define	RGBA2ARGB(_c)	((((_c) >> 8) & 0x00ffffff) | (((_c) & 0xff) << 24))

/* PS2形式からXBOX形式へ変換 */
#define EXCHG_COLOR(_c)	( ((_c)&0xff00ff00) | (((_c)&0xff)<<16) | (((_c)&0xff0000)>>16) )
//#define EXCHG_COLOR(_c)		(_c)

/* 定数α処理 */
static	inline	EXCHG_COSTALPHA(DWORD col)
{
	if( !DG_ConstAlpha.act ){ return(col) ; }
	else{ return((col & 0x00ffffff) | DG_ConstAlpha.alpha) ; }
}

/* ---------------------------------------------------------------- */

	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(4827819)	/* "dmapack" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE|DG_PLUGIN_FLAG_ENABLEMENU)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_FIRST|DG_PLUGIN_PHASE_NORMAL|DG_PLUGIN_PHASE_AFTER|DG_PLUGIN_PHASE_LAST)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(64)

/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	DmapackPlugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );

static void *DrawMode( DG_DMAPACK_PARAM *packet );
static void *DrawTex( DG_DMAPACK_TEX *packet );
static void *DrawTexLin( DG_DMAPACK_TEX *packet );
static void *DrawUserTex( DG_DMAPACK_TEX *packet );
static void *DrawAlpha( DG_DMAPACK_ALPHA *packet );
static void *DrawWindow( DG_DMAPACK_WINDOW *packet );
static void *DrawSetZ( DG_DMAPACK_PARAM *packet );
static void *DrawModeEnable( DG_DMAPACK_PARAM *packet );
static void *DrawModeDisable( DG_DMAPACK_PARAM *packet );
static void *DrawUseFrameTex( DG_DMAPACK_PARAM *packet );
static void *DrawBackupFrame( DG_DMAPACK_PARAM *packet );
static void *DrawBackupFrameRect( DG_DMAPACK_BACKUPFRAME_RECT *packet );
static void *DrawRendBackBuffer2Frame( DG_DMAPACK_PARAM *packet );
static void *DrawPacketCallback( DG_DMAPACK_CALLBACK *packet );
static void *DrawNext( DG_DMAPACK_NEXT *packet );
static void *DrawPoint( DG_DMAPACK_POINT *packet );
static void *DrawLine( DG_DMAPACK_LINE *packet );
static void *DrawLineF( DG_DMAPACK_LINE_F *packet );
static void *DrawTriangle( DG_DMAPACK_TRIANGLE *packet );
static void *DrawQuad( DG_DMAPACK_QUAD *packet );
static void *DrawBox( DG_DMAPACK_BOX *packet );
static void *DrawBoxF( DG_DMAPACK_BOX_F *packet );
static void *DrawSprt( DG_DMAPACK_SPRT *packet );
static void *DrawRSprt( DG_DMAPACK_RSPRT *packet );
static void *DrawLineStrip( DG_DMAPACK_LINESTRIP *packet );
static void *DrawTriangleStrip( DG_DMAPACK_TRIANGLESTRIP *packet );
static void *DrawDummy( void *packet ) ;

static void *DrawSprts( DG_DMAPACK_SPRT *packet );
static void *DrawLines( DG_DMAPACK_LINE *packet );
static void *DrawLinesF( DG_DMAPACK_LINE_F *packet );
static void *DrawTriangles( DG_DMAPACK_TRIANGLE *packet );
static void *DrawQuads( DG_DMAPACK_QUAD *packet );
static void *DrawBoxes( DG_DMAPACK_BOX *packet );
static void *DrawBoxesF( DG_DMAPACK_BOX_F *packet );
static void *DrawRSprts( DG_DMAPACK_RSPRT *packet );

	/*
		Vertex Shader 非対応版
	*/
static void *DrawModeNVS( DG_DMAPACK_PARAM *packet );
static void *DrawTexNVS( DG_DMAPACK_TEX *packet );
static void *DrawTexLinNVS( DG_DMAPACK_TEX *packet );
static void *DrawUserTexNVS( DG_DMAPACK_TEX *packet );
static void *DrawAlphaNVS( DG_DMAPACK_ALPHA *packet );
static void *DrawWindowNVS( DG_DMAPACK_WINDOW *packet );
static void *DrawSetZNVS( DG_DMAPACK_PARAM *packet );
static void *DrawModeEnableNVS( DG_DMAPACK_PARAM *packet );
static void *DrawModeDisableNVS( DG_DMAPACK_PARAM *packet );
static void *DrawUseFrameTexNVS( DG_DMAPACK_PARAM *packet );
static void *DrawBackupFrameNVS( DG_DMAPACK_PARAM *packet );
static void *DrawBackupFrameRectNVS( DG_DMAPACK_BACKUPFRAME_RECT *packet );
static void *DrawRendBackBuffer2FrameNVS( DG_DMAPACK_PARAM *packet );
static void *DrawPacketCallbackNVS( DG_DMAPACK_CALLBACK *packet );
static void *DrawNextNVS( DG_DMAPACK_NEXT *packet );
static void *DrawPointNVS( DG_DMAPACK_POINT *packet );
static void *DrawLineNVS( DG_DMAPACK_LINE *packet );
static void *DrawLineFNVS( DG_DMAPACK_LINE_F *packet );
static void *DrawTriangleNVS( DG_DMAPACK_TRIANGLE *packet );
static void *DrawQuadNVS( DG_DMAPACK_QUAD *packet );
static void *DrawBoxNVS( DG_DMAPACK_BOX *packet );
static void *DrawBoxFNVS( DG_DMAPACK_BOX_F *packet );
static void *DrawSprtNVS( DG_DMAPACK_SPRT *packet );
static void *DrawRSprtNVS( DG_DMAPACK_RSPRT *packet );
static void *DrawLineStripNVS( DG_DMAPACK_LINESTRIP *packet );
static void *DrawTriangleStripNVS( DG_DMAPACK_TRIANGLESTRIP *packet );
static void *DrawDummyNVS( void *packet ) ;

static void *DrawSprtsNVS( DG_DMAPACK_SPRT *packet );
static void *DrawLinesNVS( DG_DMAPACK_LINE *packet );
static void *DrawLinesFNVS( DG_DMAPACK_LINE_F *packet );
static void *DrawTrianglesNVS( DG_DMAPACK_TRIANGLE *packet );
static void *DrawQuadsNVS( DG_DMAPACK_QUAD *packet );
static void *DrawBoxesNVS( DG_DMAPACK_BOX *packet );
static void *DrawBoxesFNVS( DG_DMAPACK_BOX_F *packet );
static void *DrawRSprtsNVS( DG_DMAPACK_RSPRT *packet );


/* ---------------------------------------------------------------- */
	/*
		関数テーブル
	*/

#undef	DMAPACK_CMD_EQU
#define	DMAPACK_CMD_EQU(cmd_, func_)	(void *)func_,
static void	*(*_draw_func_tbl[])(void *) =
{
#include "dmapack_cmd.h"
	NULL,
} ;
#undef	DMAPACK_CMD_EQU

#undef	DMAPACK_CMD_EQU
#define	DMAPACK_CMD_EQU(cmd_, func_)	(void *)func_##NVS,
static void	*(*_draw_func_nvs_tbl[])(void *) =
{
#include "dmapack_cmd.h"
	NULL,
} ;
#undef	DMAPACK_CMD_EQU

/* ---------------------------------------------------------------- */
/*
	頂点シェーダー関連
*/
DG_VERTEXSHADER	DG_DmapackVertexShader[2] ;
DG_PIXELSHADER	DG_DmapackPixelShader[4] ;
extern unsigned char VERTEX_SHADER_dmapk_2d[];
extern unsigned char VERTEX_SHADER_dmapk_2d4[];
extern unsigned char	PIXEL_SHADER_mtex_gray[] ;
void DG_InitDmapackVertexShader(void)
{
	static DWORD dwObjDecl_00[] = {
		/* テクスチャ無し */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT2),		/* position */
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),		/* diffuse */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};
#ifdef _WINDOWS
	static DWORD dwObjDecl_01[] = {
		/* テクスチャ無し */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT2),		/* position */
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),		/* diffuse */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_SHORT2),		/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_SHORT2),		/* uv2 */
		D3DVSD_REG(10,  D3DVSDT_SHORT2),		/* uv3 */
		D3DVSD_END()
	};
#endif
	if( DG_CheckUseVertexShader() )
	{
		DG_MakeVertexShader( &DG_DmapackVertexShader[0], VSHT_dmapk_2d,  dwObjDecl_00 );
		DG_MakeVertexShader( &DG_DmapackVertexShader[1], VSHT_dmapk_2d4, dwObjDecl_01 );
	}

	DG_MakePixelShader( &DG_DmapackPixelShader[0], PSHT_mtex_gray );
}
/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
void DG_AddPluginDmapack( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &DmapackPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &DmapackPlugin );

	PluginStartFlag = 1 ;
	DG_PrivilegeMode = 0 ;

	/* 頂点シェーダー作成 */
	DG_InitDmapackVertexShader();
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginDmapack( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &DmapackPlugin );
		DG_FreePlugin( &DmapackPlugin );
	}
	PluginStartFlag = 0 ;
}

/* ---------------------------------------------------------------- */
/*
	デバッグ用ルーチン
*/
#ifdef DEBUG_MODE
#undef DG_MakeDmapack2
#undef DG_MakeDmapack
DG_DMAPACK* DG_MakeDmapack2( int flag, int phase, int priority );
DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
DG_DMAPACK* DG_MakeDmapack2_D( int flag, int phase, int priority, char *file )
{
	DG_DMAPACK	*dmapack ;
	dmapack = DG_MakeDmapack2( flag, phase, priority );
	if ( dmapack != NULL ){
		dmapack->fname = file ;
	}
	return ( dmapack );
}
DG_DMAPACK* DG_MakeDmapack_D( int flag, int phase, char *file )
{
	DG_DMAPACK	*dmapack ;
	dmapack = DG_MakeDmapack( flag, phase );
	if ( dmapack != NULL ){
		dmapack->fname = file ;
	}
	return ( dmapack );
}
#endif
/* ---------------------------------------------------------------- */
	/*
		オブジェクト作成
	*/
DG_DMAPACK* DG_MakeDmapack2( int flag, int phase, int priority )
{
	DG_DMAPACK	*dmapack ;

	flag |= DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3 ;
	if ( ( dmapack = GV_Malloc( sizeof(DG_DMAPACK) ) ) == NULL ){
		return ( NULL );
	}
	GV_ZeroMemory( dmapack, sizeof(DG_DMAPACK) );

	dmapack->flag = flag ;
	dmapack->phase = phase ;
	dmapack->priority = priority ;

	return ( dmapack );
}

DG_DMAPACK* DG_MakeDmapack( int flag, int phase )
{
	return DG_MakeDmapack2( flag, phase, 128 );
}

	/*
		オブジェクト開放
	*/
void DG_FreeDmapack( DG_DMAPACK *dmapack )
{
	GV_Free( dmapack );
}

	/*
		オブジェクト登録
	*/
int DG_QueueDmapack( DG_DMAPACK *dmapack )
{
	if ( PluginStartFlag == 0 ){
		return (-1);
	}
	//DG_QueueUserObject( DmapackPlugin.obj_buffer, dmapack );

	{
		int		n ;
		DG_OBJ_BUFFER	*obj_buff = DmapackPlugin.obj_buffer ;
		DG_DMAPACK	**objs ;

		if ( dmapack == NULL ) return (-1);

		/*
			プライオリティを考慮し、登録順番をくずさないようにキューに追加する
		*/
		if ( obj_buff->n_queue >= obj_buff->max_queue ){
			printf("que buffer over!!\n");
			return -1 ;
		}
		objs = (DG_DMAPACK**)&obj_buff->queue[ obj_buff->n_queue - 1 ] ;
		for ( n = obj_buff->n_queue - 1 ; n >= 0 ; objs--, n-- ){
			if ( (*objs)->priority > dmapack->priority ){
				objs[1] = objs[0] ;
			} else {
				break ;
			}
		}
		objs++ ;
		*objs = dmapack ;
		obj_buff->n_queue++ ;
#if 0
		for ( n = 0 ; n < 16 ; n++ ){
			printf("%08x ", obj_buff->queue[n] );
		}
		printf(" (%d)\n", obj_buff->n_queue);
#endif
	}
	return ( 0 );
}

	/*
		オブジェクト削除
	*/
void DG_DequeueDmapack( DG_DMAPACK *dmapack )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_DequeueUserObject( DmapackPlugin.obj_buffer, dmapack );
}

/* ---------------------------------------------------------------- */
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_DMAPACK	*dmapack, **que ;
	int			i, exec_flag = 0 ;
	int			invisible_flag, chanl_flag ;

	MARK( "dmapack.c" );
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;

	/* 描画効率にも関わるので実際に描画するものがあった場合のみ実行 */
	//DG_OpenDmaTask();

	/* 非表示チェック用フラグ設定 */
	invisible_flag = DG_DMAPACK_INVISIBLE0 << cp->chanl_num ;
	chanl_flag = ( cp->chanl_num != 4 ) ? DG_DMAPACK_NORMAL : DG_DMAPACK_MENU ;

	que = (DG_DMAPACK**)obj_buff->queue ;
	for ( ; i > 0 ; i-- ){
		dmapack = (*que++) ;
		if ( !( dmapack->flag & chanl_flag ) ) continue ;
		if ( dmapack->flag & invisible_flag ) continue ;
		if ( cp->chanl_num == 4 ){/* 特権モードはメニューチャンネルでのみ有効にする */
			if ( DG_PrivilegeMode && !( dmapack->flag & DG_DMAPACK_PRIVILEGE ) ) continue ;
		}
		if ( !( dmapack->phase & status ) ) continue ;

		if ( exec_flag == 0 ){
			/* スタティックプッシュバッファへの記録開始 */
			DG_OpenDmaTask();
		}
		exec_flag = 1 ;
		//printf("%p %s\n", dmapack, dmapack->fname );

		/* ＤＭＡパケットの変わりにコールバックでエミュレーションする */
		if ( dmapack->callback != NULL ){
			(*dmapack->callback)( dmapack->param );
		}
		/* 自動処理簡易２Ｄプリミティブ処理 */
		if ( dmapack->autopacket != NULL ){
			DG_ExecAuto2DPrim( dmapack->autopacket );
		}
		/* 本来のＤＭＡ処理はＸＢＯＸではなにもしない */

#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

	if ( exec_flag ){
		/* スタティックプッシュバッファへの記録終了 */
		DG_CloseDmaTask();
	}

}

/* ---------------------------------------------------------------- */
void DG_SetDmapackCallback( DG_DMAPACK *dmapack, void *callback, void *param )
{
	dmapack->callback = callback ;
	dmapack->param = param ;
}



/* ---------------------------------------------------------------- */
/* ＸＢＯＸ専用プリミティブ描画補助 */

static DWORD *DG_DrawVerticesDirectAddr ;
void *DG_DrawVerticesDirect( u_int type, int v_size, int count )
{
#ifdef _WINDOWS
	void	*ret_addr ;
	int		dword_size ;
	static BYTE	_draw_vertex_direct_dmybuff[0x100000] ;	// ダミー

	dword_size = ( (v_size * count ) + 3 ) & ~3 ;

	ret_addr = _draw_vertex_direct_dmybuff ;	// 応急処置

	return(ret_addr) ;
#else
	void	*ret_addr ;
	int		dword_size ;

	dword_size = ( v_size * count + 3 ) / 4 ;
	/* プッシュバッファアドレス取得 */
	IDirect3DDevice8_BeginPush( g_pd3dDevice, dword_size + 5, &DG_DrawVerticesDirectAddr );
	/* プリミティブ描画準備 */
	*DG_DrawVerticesDirectAddr++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
	*DG_DrawVerticesDirectAddr++ = type ;
	*DG_DrawVerticesDirectAddr++ = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG|D3DPUSH_INLINE_ARRAY, dword_size );
	/* 頂点データアドレスを記録し、頂点データ直後のアドレスを算出 */
	ret_addr = DG_DrawVerticesDirectAddr ;
	DG_DrawVerticesDirectAddr = (void*)( (DWORD*)DG_DrawVerticesDirectAddr + dword_size );
	/* この段階でプッシュバッファ終端データを書き込んでおく */
//	*DG_DrawVerticesDirectAddr++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
//	*DG_DrawVerticesDirectAddr++ = 0 ;
	return ( ret_addr );
#endif
}
void DG_DrawVerticesDirectEnd( void )
{
#ifndef _WINDOWS
	*DG_DrawVerticesDirectAddr++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
	*DG_DrawVerticesDirectAddr++ = 0 ;
	/* プッシュバッファアクセスの終了 */
	IDirect3DDevice8_EndPush( g_pd3dDevice, DG_DrawVerticesDirectAddr );
#endif
}

/* ---------------------------------------------------------------- */
#if 0
typedef struct _DG_VERTEX2D {
	int				x, y, z, rhw ;
	int				rgba ;
} DG_VERTEX2D ;
#define D3DFVF_VERTEX2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
typedef struct _DG_VERTEX2DTEX {
	float			x, y, z, rhw ;
	int				rgba ;
	float			tu0, tv0 ;//, tq0 ;
} DG_VERTEX2DTEX ;
#define D3DFVF_VERTEX2DTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#endif

#define NEXT_ADDR( _p )	( (void*)&(_p)[1] )

static DG_VERTEX_DMAPACK2D		*Vertex;
static DG_TEX				*last_tex = NULL ;
static FVECTOR tex_param_normal[2] = {
	{1/16384.0f,1/16384.0f,1,1},	/* テクスチャスケール */
	{0,0,0,0}					/* テクスチャオフセット */
};

static FVECTOR tex_param_lintex_old[2] = {
	{DRAW_WIDTH/16384.0f,DRAW_HEIGHT/16384.0f,1,1},	/* テクスチャスケール */
	{(DISPLAY_WIDTH-DRAW_WIDTH)/2.0f,(DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f,0,0}		/* テクスチャオフセット */
};
static FVECTOR tex_param_lintex[2] = {
	{
		((float)DRAW_WIDTH/(float)DISPLAY_WIDTH)/16384.0f,
		((float)DRAW_HEIGHT/(float)DISPLAY_HEIGHT)/16384.0f,
		1,1
	},	/* テクスチャスケール */
	{
		((float)(DISPLAY_WIDTH-DRAW_WIDTH)/2.0f)/(float)DISPLAY_WIDTH,
		((float)(DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f)/(float)DISPLAY_HEIGHT,
		0,0
	}		/* テクスチャオフセット */
};
/* ---------------------------------------------------------------- */
extern DG_VIEWPORT DG_CurrentViewport;
static float	draw_z = 1.0f ;
static FVECTOR	screen_param[2] = {
	{-1.0f,1.0f,1.0f,1.0f},
	{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT,0,0}
};
static DG_VIEWPORT dmapack_viewport ;

static void SetViewport( float x, float y, float w, float h )
{
	dmapack_viewport.lx = x ;
	dmapack_viewport.ly = y ;	
	dmapack_viewport.width  = w ;
	dmapack_viewport.height = h ;
	dmapack_viewport.min_z  = 0.0f ;
	dmapack_viewport.max_z  = 1.0f ;
	DG_SetViewport( &dmapack_viewport );

	/* 頂点シェーダー用座標変換パラメータを修正する */

	if( DG_RendToBackBuffer )
	{
		w /= DG_BackBufferWidthAdjustCoef ;		// 補正
		h /= DG_BackBufferHeightAdjustCoef ;
	}

#ifndef _WINDOWS
	screen_param[0].vx = (float)0 ;
	screen_param[0].vy = ((float)-DISPLAY_HEIGHT) * h ;
	screen_param[0].vz = draw_z ;
	screen_param[1].vx = 2.0f / ((float)DISPLAY_WIDTH * w) ;
	screen_param[1].vy = -2.0f / ((float)DISPLAY_HEIGHT * h) ;
#else
	if( DG_CheckDispVibrationEnable() )	// 画面振動(PAD振動代替処理)
	{
		extern float	DG_DispVibration1 ;
		extern float	DG_DispVibration2 ;

		screen_param[0].vx = -1.0f + DG_DispVibration1 ;
		screen_param[0].vy =  1.0f + DG_DispVibration2 ;
	}

	screen_param[0].vz = draw_z ;
	screen_param[1].vx = 2.0f / ((float)DISPLAY_WIDTH * w) ;
	screen_param[1].vy = -2.0f / ((float)DISPLAY_HEIGHT * h) ;
#endif
	if( DG_CheckUseVertexShader() )
	{
		DG_SetVertexShaderConstant(0, screen_param, 2);
	}
	else
	{
		SetScreenParamTransform(screen_param) ;
	}
}
/* ---------------------------------------------------------------- */

/* 自動処理簡易２Ｄプリミティブの描画処理 */
void DG_ExecAuto2DPrim( void *addr )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};
	DG_DMAPACK_TAG		*header = addr ;
	void	*(*func)(void *) ;
	void	**func_tbl ;

	if( !header )
	{
#ifdef DEBUG_MODE
		printf("[ WARNING ] DG_ExecAuto2DPrim(NULL)!!\n") ;
#endif
		return ;
	}

	DG_SetPixelShader( NULL );

	/* 描画モードの初期化 */
	DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
	DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	if( !DG_CheckUseVertexShader() )
	{
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}

	/* 頂点シェーダー関連初期化 */
	if( DG_CheckUseVertexShader() )
	{
		DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
		DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_normal, 2);
	}
	else
	{
		FMATRIX	texmtx ;

		texmtx = DG_UnitMatrix ;
		texmtx.m[0][0] = tex_param_normal[0].vx ;
		texmtx.m[1][1] = tex_param_normal[0].vy ;
		texmtx.m[2][0] = tex_param_normal[1].vx ;
		texmtx.m[2][1] = tex_param_normal[1].vy ;
		DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&texmtx) ;
	}

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;

	if( DG_CheckUseVertexShader() )
	{
		DG_SelectVertexShader( &DG_DmapackVertexShader[0] );
	}
	else
	{
		/* FVF設定 */
		DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D_FLAG) ;

		/* 光源計算無し */
		DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;

		/* Perspective設定 */
		{
			D3DXMATRIX	d3dx_mtx ;

			D3DXMatrixOrthoOffCenterLH(&d3dx_mtx, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f) ;

			DG_SetTransform(D3DTS_PROJECTION, (FMATRIX *)d3dx_mtx.m) ;
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix) ;
		}
	}

	/* ビューポートをそのチャンネルの設定に合わせる */
	draw_z = 1.0f ;	/* 初期描画Ｚ値の設定 */
	if( !DG_RendToBackBuffer )
	{
		SetViewport(DG_CurrentViewport.lx     / DG_BackBufferWidthAdjustCoef,
					DG_CurrentViewport.ly     / DG_BackBufferHeightAdjustCoef,
					DG_CurrentViewport.width  / DG_BackBufferWidthAdjustCoef,
					DG_CurrentViewport.height / DG_BackBufferHeightAdjustCoef );
	}
	else
	{
		SetViewport(DG_CurrentViewport.lx,
					DG_CurrentViewport.ly,
					DG_CurrentViewport.width, DG_CurrentViewport.height );
	}

	/* 関数テーブル選択 */
	if( DG_CheckUseVertexShader() )
	{
		func_tbl = (void **)_draw_func_tbl ;
	}
	else
	{
		func_tbl = (void **)_draw_func_nvs_tbl ;
	}

	/* 描画 */
	while ( header->cmd != DG_DMAPACK_CMD_END ){

		func = func_tbl[header->cmd] ;	/* 関数テーブル版 */

		header = func((void *)header) ;
		if ( header == NULL ) break ;
	}

	/* 他に影響しないように初期化を行う */
	DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

	/* ビューポートを元に戻す */
	DG_SetViewport( &DG_CurrentViewport );
}

/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
/* 自動処理簡易２Ｄプリミティブ設定ルーチン */
void *DG_SetDmapackEnd( void *addr )
{
	DG_DMAPACK_TAG *packet = addr ;
	packet->cmd = DG_DMAPACK_CMD_END ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackMode( void *addr, int mode )
{
	DG_DMAPACK_PARAM *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_MODE ;
	packet->param = mode ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackModeEnable( void *addr, int mode )
{
	DG_DMAPACK_PARAM *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_ENABLE ;
	packet->param = mode ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackModeDisable( void *addr, int mode )
{
	DG_DMAPACK_PARAM *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_DISABLE ;
	packet->param = mode ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackTex( void *addr, DG_TEX *tex )
{
	DG_DMAPACK_TEX *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_TEX ;
	packet->tex = tex ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackTexLin( void *addr, DG_TEX_LIN *tex )
{
	DG_DMAPACK_TEX *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_TEXLIN ;
	packet->tex = (DG_TEX*)tex ;

	return ( (void*)&packet[1] );
}

void *DG_SetDmapackUserTex( void *addr, DG_USERTEX *tex )
{
	DG_DMAPACK_TEX *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_USERTEX ;
	packet->tex = (DG_TEX*)tex ;

	return ( (void*)&packet[1] );
}

void *DG_SetDmapackAlpha( void *addr, u_long alpha )
{
	DG_DMAPACK_ALPHA *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_ALPHA ;
#if 0
	//packet->alpha = alpha & SCE_GS_ALPHA_MASK ;
	packet->alpha = alpha ;
#else
	*(u_long*)&packet->alpha0 = alpha ;
#endif
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackWindow( void *addr, int x, int y, int w, int h )
{
	DG_DMAPACK_WINDOW *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_WINDOW ;
	packet->x = x ;
	packet->y = y ;
	packet->w = w ;
	packet->h = h ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackSetZ( void *addr, float z )
{
	DG_DMAPACK_PARAM *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_SETZ ;
	*(float*)&packet->param = z ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackUseFrameTex( void *addr, int page )
{
	DG_DMAPACK_PARAM *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_USEFRAMETEX ;
	packet->param = page ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackBackupFrame( void *addr, int page )
{
	DG_DMAPACK_PARAM *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_BACKUPFRAME ;
	packet->param = page ;
	ASSERT( !(packet->param & 0xfffffff8) ) ;	// Surfaceの直接指定には別対応
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackBackupFrameRect( void *addr, int page, int x, int y, int w, int h )
{
	DG_DMAPACK_BACKUPFRAME_RECT *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_BACKUPFRAME_RECT ;
	packet->param = page ;
	packet->x = x ;
	packet->y = y ;
	packet->w = w ;
	packet->h = h ;
	ASSERT( !(packet->param & 0xfffffff8) ) ;	// Surfaceの直接指定には別対応
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackRendBackBuffer2Frame( void *addr )
{
	DG_DMAPACK_PARAM *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_RENDBACKBUFFER2FRAME ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackPacketCallback( void *addr, void *func, int param )
{
	DG_DMAPACK_CALLBACK *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_CALLBACK ;
	packet->func = func ;
	packet->param = param ;
	return ( (void*)&packet[1] );
}

#if !__SET_DMAPACK_INLINE__
void *DG_SetDmapackNext( void *addr, void *next )
{
	DG_DMAPACK_NEXT *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_NEXT ;
	packet->next = next ;
	return ( (void*)&packet[1] );
}
#endif

/* ---------------------------------------------------------------- */
void *DG_SetDmapackPoint( void *addr, float x0, float y0, int rgba )
{
	DG_DMAPACK_POINT *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_POINT ;
	packet->rgba = EXCHG_COLOR( rgba ) ;
	packet->x = (short)x0 ;
	packet->y = (short)y0 ;
	return ( (void*)&packet[1] );
}


void *DG_SetDmapackLine( void *addr, float x0, float y0, int rgba0, float x1, float y1, int rgba1 )
{
	DG_DMAPACK_LINE *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_LINE ;
	packet->rgba0 = EXCHG_COLOR( rgba0 ) ; ;
	packet->rgba1 = EXCHG_COLOR( rgba1 ) ;
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackLineF( void *addr, float x0, float y0, int rgba0, float x1, float y1, int rgba1 )
{
	DG_DMAPACK_LINE_F *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_LINE_F ;
	packet->rgba0 = EXCHG_COLOR( rgba0 ) ; ;
	packet->rgba1 = EXCHG_COLOR( rgba1 ) ;
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackTriangle( void *addr,
							float x0, float y0, int rgba0,
							float x1, float y1, int rgba1,
							float x2, float y2, int rgba2 )
{
	DG_DMAPACK_TRIANGLE *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_TRIANGLE ;
	packet->rgba0 = EXCHG_COLOR( rgba0 ) ;
	packet->rgba1 = EXCHG_COLOR( rgba1 ) ;
	packet->rgba2 = EXCHG_COLOR( rgba2 ) ;
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	packet->x2 = (short)x2 ;
	packet->y2 = (short)y2 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackQuad( void *addr,
						float x0, float y0, int rgba0,
						float x1, float y1, int rgba1,
						float x2, float y2, int rgba2,
						float x3, float y3, int rgba3 )
{
	DG_DMAPACK_QUAD *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_QUAD ;
	packet->rgba0 = EXCHG_COLOR( rgba0 ) ;
	packet->rgba1 = EXCHG_COLOR( rgba1 ) ;
	packet->rgba2 = EXCHG_COLOR( rgba2 ) ;
	packet->rgba3 = EXCHG_COLOR( rgba3 ) ;
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	packet->x2 = (short)x2 ;
	packet->y2 = (short)y2 ;
	packet->x3 = (short)x3 ;
	packet->y3 = (short)y3 ;
	return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
void *DG_SetDmapackBox( void *addr, float x0, float y0, float x1, float y1, int rgba )
{
	DG_DMAPACK_BOX *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_BOX ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackBoxF( void *addr, float x0, float y0, float x1, float y1, int rgba )
{
	DG_DMAPACK_BOX_F *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_BOX_F ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackSprt( void *addr,
						float x0, float y0, float u0, float v0,
						float x1, float y1, float u1, float v1, int rgba )
{
	DG_DMAPACK_SPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_SPRT ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
#if 0
	packet->u0 = ( u0 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v0 = ( v0 * tex->v_scale + tex->v_offset ) * 16384.0f ;
	packet->u1 = ( u1 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v1 = ( v1 * tex->v_scale + tex->v_offset ) * 16384.0f ;
#else
	packet->u0 = (short)(( u0 ) * 16384.0f) ;
	packet->v0 = (short)(( v0 ) * 16384.0f) ;
	packet->u1 = (short)(( u1 ) * 16384.0f) ;
	packet->v1 = (short)(( v1 ) * 16384.0f) ;
#endif
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackSprtTex( void *addr,
						float x0, float y0, float u0, float v0,
						float x1, float y1, float u1, float v1, int rgba, DG_TEX *tex )
{
	DG_DMAPACK_SPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_SPRT ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	packet->u0 = (short)(( u0 * tex->u_scale + tex->u_offset ) * 16384.0f) ;
	packet->v0 = (short)(( v0 * tex->v_scale + tex->v_offset ) * 16384.0f) ;
	packet->u1 = (short)(( u1 * tex->u_scale + tex->u_offset ) * 16384.0f) ;
	packet->v1 = (short)(( v1 * tex->v_scale + tex->v_offset ) * 16384.0f) ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackRSprt( void *addr,
						 float x0, float y0, float x1, float y1,
						 float x2, float y2, float x3, float y3,
						 float u0, float v0, float u1, float v1,
						 int rgba )
{
	DG_DMAPACK_RSPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_RSPRT ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	packet->x2 = (short)x2 ;
	packet->y2 = (short)y2 ;
	packet->x3 = (short)x3 ;
	packet->y3 = (short)y3 ;
	packet->u0 = (short)(( u0 ) * 16384.0f) ;
	packet->v0 = (short)(( v0 ) * 16384.0f) ;
	packet->u1 = (short)(( u1 ) * 16384.0f) ;
	packet->v1 = (short)(( v1 ) * 16384.0f) ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackRSprtTex( void *addr,
						 float x0, float y0, float x1, float y1,
						 float x2, float y2, float x3, float y3,
						 float u0, float v0, float u1, float v1,
						 int rgba, DG_TEX *tex )
{
	DG_DMAPACK_RSPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_RSPRT ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x0 = (short)x0 ;
	packet->y0 = (short)y0 ;
	packet->x1 = (short)x1 ;
	packet->y1 = (short)y1 ;
	packet->x2 = (short)x2 ;
	packet->y2 = (short)y2 ;
	packet->x3 = (short)x3 ;
	packet->y3 = (short)y3 ;
	packet->u0 = (short)(( u0 * tex->u_scale + tex->u_offset ) * 16384.0f) ;
	packet->v0 = (short)(( v0 * tex->v_scale + tex->v_offset ) * 16384.0f) ;
	packet->u1 = (short)(( u1 * tex->u_scale + tex->u_offset ) * 16384.0f) ;
	packet->v1 = (short)(( v1 * tex->v_scale + tex->v_offset ) * 16384.0f) ;
	return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
void *DG_SetDmapackLineStrip( void *addr, int num )
{
	DG_DMAPACK_LINESTRIP *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_LINESTRIP ;
	packet->v_count = num ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackTriangleStrip( void *addr, int num )
{
	DG_DMAPACK_TRIANGLESTRIP *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_TRIANGLESTRIP ;
	packet->v_count = num ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackVertex( void *addr, float x, float y, float u, float v, int rgba )
{
	DG_DMAPACK_VERTEX *packet = addr ;
	packet->rgba = EXCHG_COLOR( rgba );
	packet->x = (short)x ;
	packet->y = (short)y ;
	packet->u = (short)(u * 16384.0f) ;
	packet->v = (short)(v * 16384.0f) ;
	return ( (void*)&packet[1] );
}


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static void *DrawMode( DG_DMAPACK_PARAM *packet )
{
	/* フィルタモード切替チェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_BILINEAR ){
		DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
	} else {
		DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	}
	/* 半透明モード切替チェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_ALPHA ){
		DG_SetAlphaMode( 0 );
	}
	/* アルファテスト切り替えチェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_ALPHATEST ){
		DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	} else {
		DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	}
	return ( (void*)&packet[1] );
}

static void *DrawModeEnable( DG_DMAPACK_PARAM *packet )
{
	/* フィルタモード切替チェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_BILINEAR ){
		DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
	}
	/* 半透明モード切替チェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_ALPHA ){
		DG_SetAlphaMode( 0 );
	}
	/* アルファテスト切り替えチェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_ALPHATEST ){
		DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	}
	/* カラーモジュレートスケールのＯＦＦ */
	if ( packet->param & DG_DMAPACK_MODE_MODULATE1X ){
		DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	}
	/* デプステスト条件の反転 */
	if ( packet->param & DG_DMAPACK_MODE_ZTEST_REV ){
		DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	}
	/* Ｚバッファの上書き禁止 */
	if ( packet->param & DG_DMAPACK_MODE_NO_ZOVERWRITE ){
		DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	}
	/* デプステスト禁止 */
	if ( packet->param & DG_DMAPACK_MODE_NO_ZTEST ){
		DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS );
	}
	/* フレームバッファのマスクＯＮ */
	if ( packet->param & DG_DMAPACK_MODE_COLORMASK ){
		DG_SetRenderState( D3DRS_COLORWRITEENABLE , 0 );
	}
	/* テクスチャのアルファ無視（頂点アルファのみ参照） */
	if ( packet->param & DG_DMAPACK_MODE_NO_TEXALPHA ){
		DG_SetRenderState( D3DRS_TEXTUREFACTOR, 0xff000000 );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
	}
	/* テクスチャのWRAP指定 */
	if ( packet->param & DG_DMAPACK_MODE_WRAP ){
		DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	}
	return ( (void*)&packet[1] );
}

static void *DrawModeDisable( DG_DMAPACK_PARAM *packet )
{
	/* フィルタモード切替チェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_BILINEAR ){
		DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	}
	/* 半透明モード切替チェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_ALPHA ){
	}
	/* アルファテスト切り替えチェック */
	if ( packet->param & DG_DMAPACK_MODE_NO_ALPHATEST ){
		DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	}
	/* カラーモジュレートスケールのＯＮ */
	if ( packet->param & DG_DMAPACK_MODE_MODULATE1X ){
		DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	}
	/* デプステスト条件の反転 */
	if ( packet->param & DG_DMAPACK_MODE_ZTEST_REV ){
		DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
	}
	/* Ｚバッファの上書き禁止 */
	if ( packet->param & DG_DMAPACK_MODE_NO_ZOVERWRITE ){
		DG_SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	}
	/* デプステスト禁止 */
	if ( packet->param & DG_DMAPACK_MODE_NO_ZTEST ){
		DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
	}
	/* フレームバッファのマスクＯＮ */
	if ( packet->param & DG_DMAPACK_MODE_COLORMASK ){
#ifndef	_WINDOWS
		DG_SetRenderState( D3DRS_COLORWRITEENABLE , D3DCOLORWRITEENABLE_ALL );
#else
		DG_SetRenderState( D3DRS_COLORWRITEENABLE ,   D3DCOLORWRITEENABLE_RED
												| D3DCOLORWRITEENABLE_GREEN
												| D3DCOLORWRITEENABLE_BLUE
												| D3DCOLORWRITEENABLE_ALPHA) ;
#endif
	}
	/* テクスチャのアルファ無視（頂点アルファのみ参照） */
	if ( packet->param & DG_DMAPACK_MODE_NO_TEXALPHA ){
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	}
	/* テクスチャのWRAP指定 */
	if ( packet->param & DG_DMAPACK_MODE_WRAP ){
		DG_SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		DG_SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	}
	return ( (void*)&packet[1] );
}

static void *DrawTex( DG_DMAPACK_TEX *packet )
{
	/* テクスチャ設定 */
	last_tex = packet->tex ;
	if ( last_tex != NULL ){
		//DG_SetTexture( 0, &last_tex->tex_trans );	/* 頂点シェーダーのCV_TEX0_SCALEなどを破壊されると困るので */
		DG_SetTextureDirect( 0, last_tex->tex_trans.ptex );
		DG_SetAlphaMode( last_tex->tex_trans.alpha.data );
	} else {
		DG_SetTextureDirect( 0, NULL );
	}
	return ( (void*)&packet[1] );
}

static void *DrawTexLin( DG_DMAPACK_TEX *packet )
{
	DG_TEX_LIN	*tex ;

	/* テクスチャ設定 */
	tex = (DG_TEX_LIN*)packet->tex ;

	/* Imageが変わっていそうだったら変更 */
	if( DG_LinerTextureGetImageDirty(tex) )
	{
		DG_LinerTextureFlushImage(tex) ;
		DG_LinerTextureClearImageDirty(tex) ;
	}

	DG_SetLinerTexture(tex) ;

	/* 頂点シェーダーのパラメータも変更する */
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, &tex->tex_param, 1);

	return ( (void*)&packet[1] );
}

static void *DrawUserTex( DG_DMAPACK_TEX *packet )
{
	FVECTOR		tex_param = { 1,1,1,1 };
	DG_USERTEX	*tex ;

	/* テクスチャ設定 */
	tex = (DG_USERTEX*)packet->tex ;
	DG_SetUserTexture(tex) ;

	/* 頂点シェーダーのパラメータも変更する */
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, &tex->tex_param, 1);

	return ( (void*)&packet[1] );
}

static void *DrawAlpha( DG_DMAPACK_ALPHA *packet )
{
#if 0
	DG_SetAlphaMode( packet->alpha );
#else
	DG_SetAlphaMode( *(u_long*)&packet->alpha0 );
#endif
	return ( (void*)&packet[1] );
}

static void *DrawWindow( DG_DMAPACK_WINDOW *packet )
{
	float x, y, w, h ;

	if( !DG_RendToBackBuffer )
	{
		x = ((float)packet->x + ((float)(DISPLAY_WIDTH - DRAW_WIDTH) / 2.0f))
		  * (1.0f/(float)DISPLAY_WIDTH) ;
		y = ((float)packet->y + ((float)(DISPLAY_HEIGHT - DRAW_HEIGHT) / 2.0f))
		  * (1.0f/(float)DISPLAY_HEIGHT) ;
		w = (float)packet->w * (1.0f/(float)DISPLAY_WIDTH) ;
		h = (float)packet->h * (1.0f/(float)DISPLAY_HEIGHT) ;
	}
	else
	{
		x = (((float)packet->x + ((float)(DISPLAY_WIDTH - DRAW_WIDTH) / 2.0f))
	   	   * DG_BackBufferWidthAdjustCoef)/(float)DISPLAY_WIDTH ;
		y = (((float)packet->y + ((float)(DISPLAY_HEIGHT - DRAW_HEIGHT) / 2.0f))
		  * DG_BackBufferHeightAdjustCoef)/(float)DISPLAY_HEIGHT ;
		w = ((float)packet->w * DG_BackBufferWidthAdjustCoef)/(float)DISPLAY_WIDTH ;
		h = ((float)packet->h * DG_BackBufferHeightAdjustCoef)/(float)DISPLAY_HEIGHT ;
	}
	SetViewport(x, y, w, h) ;

	return ( (void*)&packet[1] );
}

static void *DrawSetZ( DG_DMAPACK_PARAM *packet )
{
	draw_z = *(float*)&packet->param ;
	/* 頂点シェーダー内の定数を変更することでそれ以降の２Ｄの描画Ｚを変更する */
	screen_param[0].vz = draw_z ;
	DG_SetVertexShaderConstant(0, screen_param, 2);
	return ( (void*)&packet[1] );
}

static void *DrawUseFrameTex( DG_DMAPACK_PARAM *packet )
{
	LPDIRECT3DTEXTURE8	lpTex ;
	int					page ;

	if ( packet->param != -1 ){
		switch ( packet->param ){
		  case 0:
			page = DG_CurrentBackBuffer ;
			break ;

		  case 1:
			page = DG_CurrentBackBuffer ^ 1 ;
			break ;

		  default:
			page = 2 ;
			break ;
		}
		lpTex = DG_SystemTexture[page].tex_trans.ptex ;
		DG_SetTextureDirect( 0, lpTex );
		DG_SetVertexShaderConstant(CV_TEX0_SCALE, DG_BackBufferTexScaleParam, 2);
	} else {
		DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_normal, 2);
	}
	return ( (void*)&packet[1] );
}

static void *DrawBackupFrame( DG_DMAPACK_PARAM *packet )
{
	int		mono_flag = 0, col = 0xffffffff ;
	static FVECTOR full_screen_param[2] = {
		{-1.0f, 1.0f,1.0f,1.0f},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT, 0.0f, 0.0f}
	};
	DG_VIEWPORT			viewport ;
	LPDIRECT3DTEXTURE8	lpTex ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	int					page ;

	if ( (packet->param & 0xfffffff8) )
	{

		ASSERT(0) ;		// Surface指定には対応できません
		return ( (void*)&packet[1] );
	}

	/*-- カレントのRenderTarget記憶 ----------------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	switch ( packet->param & 1 ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;

	  case 1:
		page = DG_CurrentBackBuffer ^ 1 ;
		break ;
	}
	lpTex = DG_SystemTexture[page].tex_trans.ptex ;

	/* 指定したバッファをテクスチャとして退避バッファへレンダリングする */
	DG_SetRenderTarget( DG_BackBufferSurface[2], NULL );
	if ( packet->param & 0x04 ){
		mono_flag = 1 ;
	}

	viewport.lx     = (float)(( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0)
					/ (float)DISPLAY_WIDTH ;
	viewport.ly     = (float)(( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0)
					/ (float)DISPLAY_HEIGHT ;
	viewport.width  = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	viewport.height = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;

	viewport.lx     *= DG_BackBufferWidthAdjustCoef ;
	viewport.ly     *= DG_BackBufferHeightAdjustCoef ;
	viewport.width  *= DG_BackBufferWidthAdjustCoef ;
	viewport.height *= DG_BackBufferHeightAdjustCoef ;
	
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );

	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );

	DG_SetTextureDirect( 0, lpTex );
#if FALSE
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_lintex, 2);
#else
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, DG_BackBufferTexScaleParam, 2);
#endif
	DG_SetVertexShaderConstant(0, full_screen_param, 2);
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 255 ) );
	DG_SetAlphaMode( 0 );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	if ( mono_flag ){
		DG_SetPixelShader( &DG_DmapackPixelShader[0] );
		col = EXCHG_COSTALPHA(0x80808080) ;
	}

	{
		DG_VERTEX_DMAPACK2D	Vertex[4] ;
		DWORD				sofs ;
		float	lu, lv, ru, rv ;
		float	ofs_lu, ofs_lv, ofs_ru, ofs_rv ;

#if TRUE
		ofs_lu = ((float)DRAW_WIDTH * 0.5f)/(float)DG_BackBufferRendWidth ;
		ofs_lv = ((float)DRAW_HEIGHT * 0.5f)/(float)DG_BackBufferRendHeight ;
		ofs_ru = ofs_lu ;
		ofs_rv = ofs_lv ;

		if( (packet->param & 2) )
		{
			ofs_lu += 0.5f ;
			ofs_lv += 0.5f ;
			ofs_ru -= 0.5f ;
			ofs_rv -= 0.5f ;
		}

		lu = DG_FRAME_U(ofs_lu) * 16384.0f ; 
		lv = DG_FRAME_V(ofs_lv) * 16384.0f ; 
		ru = DG_FRAME_U(DRAW_WIDTH  + ofs_ru) * 16384.0f ; 
		rv = DG_FRAME_V(DRAW_HEIGHT + ofs_rv) * 16384.0f ; 
#else
		#define	BKUPFRAME_OFS_L		(0.50f)
		#define	BKUPFRAME_OFS_R		(-0.50f)
		#define	BKUPFRAME_OFS_L2	(BKUPFRAME_OFS_L + 0.5f)
		#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R + 0.5f)
		#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R - 0.5f)

		if( !(packet->param & 2) )
		{
			lu = DG_FRAME_U(BKUPFRAME_OFS_L) * 16384.0f ; 
			lv = DG_FRAME_V(BKUPFRAME_OFS_L) * 16384.0f ; 
			ru = DG_FRAME_U(DRAW_WIDTH  + BKUPFRAME_OFS_R) * 16384.0f ; 
			rv = DG_FRAME_V(DRAW_HEIGHT + BKUPFRAME_OFS_R) * 16384.0f ; 
		}
		else
		{
			lu = DG_FRAME_U(BKUPFRAME_OFS_L2) * 16384.0f ; 
			lv = DG_FRAME_V(BKUPFRAME_OFS_L2) * 16384.0f ; 
			ru = DG_FRAME_U(DRAW_WIDTH  + BKUPFRAME_OFS_R2) * 16384.0f ; 
			rv = DG_FRAME_V(DRAW_HEIGHT + BKUPFRAME_OFS_R2) * 16384.0f ; 
		}
#endif

		setVertexDMAPack2DXY(  &Vertex[0], 0, 0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], col) ;
		setVertexDMAPack2DUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DXY(  &Vertex[1], DRAW_WIDTH, 0) ;
		setVertexDMAPack2DRGBA(&Vertex[1], col) ;
		setVertexDMAPack2DUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DXY(  &Vertex[2], 0, DRAW_HEIGHT) ;
		setVertexDMAPack2DRGBA(&Vertex[2], col) ;
		setVertexDMAPack2DUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DXY(  &Vertex[3], DRAW_WIDTH, DRAW_HEIGHT) ;
		setVertexDMAPack2DRGBA(&Vertex[3], col) ;
		setVertexDMAPack2DUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/* レンダリング対象を元に戻す */
	if ( mono_flag ){
		DG_SetPixelShader( NULL );
	}
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( cur_front_surface, cur_depth_surface );
	//DG_SetViewport( &DG_CurrentViewport );
	DG_SetViewport( &dmapack_viewport );
	DG_SetVertexShaderConstant(0, screen_param, 2);
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	if( cur_front_surface ){ IDirect3DSurface8_Release( cur_front_surface ) ; }
	if( cur_depth_surface ){ IDirect3DSurface8_Release( cur_depth_surface ) ; }
#endif

	return ( (void*)&packet[1] );
}


static void *DrawBackupFrameRect( DG_DMAPACK_BACKUPFRAME_RECT *packet )
{
	int		mono_flag = 0, col = 0xffffffff ;
	static FVECTOR full_screen_param[2] = {
		{-1.0f, 1.0f,1.0f,1.0f},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT, 0.0f, 0.0f}
	};
	DG_VIEWPORT			viewport ;
	LPDIRECT3DTEXTURE8	lpTex ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	int					page ;

	if ( (packet->param & 0xfffffff8) )
	{

		ASSERT(0) ;		// Surface指定には対応できません
		return ( (void*)&packet[1] );
	}

	/*-- カレントのRenderTarget記憶 ----------------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	switch ( packet->param & 1 ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;

	  case 1:
		page = DG_CurrentBackBuffer ^ 1 ;
		break ;
	}
	lpTex = DG_SystemTexture[page].tex_trans.ptex ;

	/* 指定したバッファをテクスチャとして退避バッファへレンダリングする */
	DG_SetRenderTarget( DG_BackBufferSurface[2], NULL );
	if ( packet->param & 0x04 ){
		mono_flag = 1 ;
	}

	viewport.lx     = (float)(( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0)
					/ (float)DISPLAY_WIDTH ;
	viewport.ly     = (float)(( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0)
					/ (float)DISPLAY_HEIGHT ;
	viewport.width  = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	viewport.height = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;

	viewport.lx     *= DG_BackBufferWidthAdjustCoef ;
	viewport.ly     *= DG_BackBufferHeightAdjustCoef ;
	viewport.width  *= DG_BackBufferWidthAdjustCoef ;
	viewport.height *= DG_BackBufferHeightAdjustCoef ;
	
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );

	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );

	DG_SetTextureDirect( 0, lpTex );
#if FALSE
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_lintex, 2);
#else
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, DG_BackBufferTexScaleParam, 2);
#endif
	DG_SetVertexShaderConstant(0, full_screen_param, 2);
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 255 ) );
	DG_SetAlphaMode( 0 );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	if ( mono_flag ){
		DG_SetPixelShader( &DG_DmapackPixelShader[0] );
		col = EXCHG_COSTALPHA(0x80808080) ;
	}

	{
		DG_VERTEX_DMAPACK2D	Vertex[4] ;
		DWORD				sofs ;
		float	lx, ly, rx, ry ;
		float	lu, lv, ru, rv ;
		float	ofs_lu, ofs_lv, ofs_ru, ofs_rv ;

#if TRUE
		lx = (float)packet->x ;
		ly = (float)packet->y ;
		rx = (float)(packet->x + packet->w) ;
		ry = (float)(packet->y + packet->h) ;

		ofs_lu = ((float)DRAW_WIDTH * 0.5f)/(float)DG_BackBufferRendWidth ;
		ofs_lv = ((float)DRAW_HEIGHT * 0.5f)/(float)DG_BackBufferRendHeight ;
		ofs_ru = ofs_lu ;
		ofs_rv = ofs_lv ;

		if( (packet->param & 2) )
		{
			ofs_lu += 0.5f ;
			ofs_lv += 0.5f ;
			ofs_ru -= 0.5f ;
			ofs_rv -= 0.5f ;
		}

		lu = DG_FRAME_U(lx + ofs_lu) * 16384.0f ; 
		lv = DG_FRAME_V(ly + ofs_lv) * 16384.0f ; 
		ru = DG_FRAME_U(rx + ofs_ru) * 16384.0f ; 
		rv = DG_FRAME_V(ry + ofs_rv) * 16384.0f ; 
#else
		#define	BKUPFRAME_OFS_L		(0.50f)
		#define	BKUPFRAME_OFS_R		(-0.50f)
		#define	BKUPFRAME_OFS_L2	(BKUPFRAME_OFS_L + 0.5f)
		//#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R + 0.5f)
		#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R - 0.5f)

		lx = (float)packet->x ;
		ly = (float)packet->y ;
		rx = (float)(packet->x + packet->w) ;
		ry = (float)(packet->y + packet->h) ;

		if( !(packet->param & 2) )
		{
			lu = DG_FRAME_U(lx + BKUPFRAME_OFS_L) * 16384.0f ; 
			lv = DG_FRAME_V(ly + BKUPFRAME_OFS_L) * 16384.0f ; 
			ru = DG_FRAME_U(rx + BKUPFRAME_OFS_R) * 16384.0f ; 
			rv = DG_FRAME_V(ry + BKUPFRAME_OFS_R) * 16384.0f ; 
		}
		else
		{
			lu = DG_FRAME_U(lx + BKUPFRAME_OFS_L2) * 16384.0f ; 
			lv = DG_FRAME_V(ly + BKUPFRAME_OFS_L2) * 16384.0f ; 
			ru = DG_FRAME_U(rx + BKUPFRAME_OFS_R2) * 16384.0f ; 
			rv = DG_FRAME_V(ry + BKUPFRAME_OFS_R2) * 16384.0f ; 
		}
#endif

		setVertexDMAPack2DXY(  &Vertex[0], lx, ly) ;
		setVertexDMAPack2DRGBA(&Vertex[0], col) ;
		setVertexDMAPack2DUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DXY(  &Vertex[1], rx, ly) ;
		setVertexDMAPack2DRGBA(&Vertex[1], col) ;
		setVertexDMAPack2DUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DXY(  &Vertex[2], lx, ry) ;
		setVertexDMAPack2DRGBA(&Vertex[2], col) ;
		setVertexDMAPack2DUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DXY(  &Vertex[3], rx, ry) ;
		setVertexDMAPack2DRGBA(&Vertex[3], col) ;
		setVertexDMAPack2DUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/* レンダリング対象を元に戻す */
	if ( mono_flag ){
		DG_SetPixelShader( NULL );
	}
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( cur_front_surface, cur_depth_surface );
	//DG_SetViewport( &DG_CurrentViewport );
	DG_SetViewport( &dmapack_viewport );
	DG_SetVertexShaderConstant(0, screen_param, 2);
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	if( cur_front_surface ){ IDirect3DSurface8_Release( cur_front_surface ) ; }
	if( cur_depth_surface ){ IDirect3DSurface8_Release( cur_depth_surface ) ; }
#endif

	return ( (void*)&packet[1] );
}

static void *DrawRendBackBuffer2Frame( DG_DMAPACK_PARAM *packet )
{
	/*
		バックバッファに描画されている物をFrameBuffer(Window内部バッファ)
		に複写します。
		これ以降の描画はFrameBuffer側にされます。
		多重呼び出しは無視されます。
	*/

	ASSERT( DG_SkipRendBackBuffer2FrameCntr ) ;

	if( DG_SkipRendBackBuffer2FrameCntr )
	{
		DG_SkipRendBackBuffer2FrameCntr-- ;
		if( !DG_SkipRendBackBuffer2FrameCntr )
		{
			DG_RendBackBuffer2Frame() ;

			/* 内部状態が変わる可能性があるので再設定 */
			SetViewport(dmapack_viewport.lx,
						dmapack_viewport.ly,
						dmapack_viewport.width,
						dmapack_viewport.height) ;

		}
	}

	return ( (void*)&packet[1] );
}

static void *DrawPacketCallback( DG_DMAPACK_CALLBACK *packet )
{
	void	(*func)( int param );
	func = (void*)packet->func ;
	(*func)( packet->param );
	return ( (void*)&packet[1] );
}

static void *DrawNext( DG_DMAPACK_NEXT *packet )
{
#if FALSE
	return ( packet->next );
#else
	packet = packet->next ;
	while( packet && (packet->header.cmd == DG_DMAPACK_CMD_NEXT) )
	{
		// DG_DMAPACK_CMD_NEXTは連続している事が多いので、あらかじめ飛ばす
		packet = packet->next ;
	}
	return(packet) ;
#endif
}


/* ---------------------------------------------------------------- */
/* 描画関数関連 */
static void *DrawPoint( DG_DMAPACK_POINT *packet )
{
	{
		char	strbuff[256] ;
		sprintf(strbuff, "DrawPoint()") ;
		dbgErrMessPuts(strbuff, S_OK) ;
	}

	/* 未サポート */
	return ( (void*)&packet[1] );
}

static void *DrawLine( DG_DMAPACK_LINE *packet )
{
	DG_VERTEX_DMAPACK2D Vertex[2] ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_LINE )
	{
		return(DrawLines((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 2, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, 1 );

	return ( (void*)&packet[1] );
}

static void *DrawLines( DG_DMAPACK_LINE *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTexture( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

		vtx_cntr += 2 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_LINE )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 2 ;			// 次もSPRITEなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr>>1 );

	return ( (void*)next_packet );
}

static void *DrawLineF( DG_DMAPACK_LINE_F *packet )
{
	DG_VERTEX_DMAPACK2D Vertex[2] ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_LINE_F )
	{
		return(DrawLinesF((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 2, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, 1 );

	return ( (void*)&packet[1] );
}

static void *DrawLinesF( DG_DMAPACK_LINE_F *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTexture( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

		vtx_cntr += 2 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_LINE_F )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 2 ;			// 次もSPRITEなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr>>1 );

	return ( (void*)next_packet );
}

static void *DrawTriangle( DG_DMAPACK_TRIANGLE *packet )
{
	DG_VERTEX_DMAPACK2D		Vertex[3] ;
	DWORD					sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_TRIANGLE )
	{
		return(DrawTriangles((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
	setVertexDMAPack2DXY(  &Vertex[2], packet->x2, packet->y2) ;
	setVertexDMAPack2DRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba2)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 3, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, 1) ;

	return ( (void*)&packet[1] );
}

static void *DrawTriangles( DG_DMAPACK_TRIANGLE *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D	*Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTexture( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
		setVertexDMAPack2DXY(  &Vertex[2], packet->x2, packet->y2) ;
		setVertexDMAPack2DRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba2)) ;

		vtx_cntr += 3 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_TRIANGLE )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 3 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, vtx_cntr/3) ;

	return ( (void*)&packet[1] );
}

static void *DrawQuad( DG_DMAPACK_QUAD *packet )
{
	DG_VERTEX_DMAPACK2D Vertex[4] ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_QUAD )
	{
		return(DrawQuads((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
	setVertexDMAPack2DXY(  &Vertex[2], packet->x3, packet->y3) ;
	setVertexDMAPack2DRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba3)) ;
	setVertexDMAPack2DXY(  &Vertex[3], packet->x2, packet->y2) ;
	setVertexDMAPack2DRGBA(&Vertex[3], EXCHG_COSTALPHA(packet->rgba2)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );

	return ( (void*)&packet[1] );
}

static void *DrawQuads( DG_DMAPACK_QUAD *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTexture( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
		setVertexDMAPack2DXY(  &Vertex[2], packet->x3, packet->y3) ;
		setVertexDMAPack2DRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba3)) ;
		setVertexDMAPack2DXY(  &Vertex[3], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[3], EXCHG_COSTALPHA(packet->rgba1)) ;
		setVertexDMAPack2DXY(  &Vertex[4], packet->x3, packet->y3) ;
		setVertexDMAPack2DRGBA(&Vertex[4], EXCHG_COSTALPHA(packet->rgba3)) ;
		setVertexDMAPack2DXY(  &Vertex[5], packet->x2, packet->y2) ;
		setVertexDMAPack2DRGBA(&Vertex[5], EXCHG_COSTALPHA(packet->rgba2)) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_QUAD )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, vtx_cntr/3 );

	return ( (void*)&packet[1] );
}

static void *DrawBox( DG_DMAPACK_BOX *packet )
{
	DG_VERTEX_DMAPACK2D Vertex[4] ;
	DWORD				sofs ;
	DWORD				rgba ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_BOX )
	{
		return(DrawBoxes((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;


	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DXY(  &Vertex[3], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[3], rgba) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );

	return ( (void*)&packet[1] );
}

static void *DrawBoxes( DG_DMAPACK_BOX *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;
	DWORD				rgba ;

	DG_SetTexture( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DXY(  &Vertex[2], packet->x0, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
		setVertexDMAPack2DXY(  &Vertex[5], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[5], rgba) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_BOX )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, vtx_cntr/3 );

	return ( (void*)&packet[1] );
}

static void *DrawBoxF( DG_DMAPACK_BOX_F *packet )
{
	DG_VERTEX_DMAPACK2D Vertex[4] ;
	DWORD				sofs ;
	DWORD				rgba ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_BOX_F )
	{
		return(DrawBoxesF((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;


	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DXY(  &Vertex[3], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[3], rgba) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );

	return ( (void*)&packet[1] );
}

static void *DrawBoxesF( DG_DMAPACK_BOX_F *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;
	DWORD				rgba ;

	DG_SetTexture( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DXY(  &Vertex[2], packet->x0, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
		setVertexDMAPack2DXY(  &Vertex[5], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[5], rgba) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_BOX_F )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, vtx_cntr/3 );

	return ( (void*)&packet[1] );
}

static void *DrawSprt( DG_DMAPACK_SPRT *packet )
{
	DG_VERTEX_DMAPACK2D		Vertex[4] ;
	DWORD					sofs ;
	DWORD					rgba ;
	DG_DMAPACK_TAG			*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_SPRT )
	{
		return(DrawSprts((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	/*-- 頂点設定( TRIANGLEx2 の6頂点版(Indexの方が高速かは未検証)) ----*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[0], packet->u0, packet->v0) ;

	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[1], packet->u1, packet->v0) ;

	setVertexDMAPack2DXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[2], packet->u0, packet->v1) ;

	setVertexDMAPack2DXY(  &Vertex[3], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[3], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[3], packet->u1, packet->v1) ;
	/*------------------------------------------------------------------*/

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;

	return ( (void*)next_packet );
}

static void *DrawSprts( DG_DMAPACK_SPRT *packet )
{
	DG_VERTEX_DMAPACK2D		*VertexRoot ;
	DG_VERTEX_DMAPACK2D		*Vertex ;
	DWORD					vtx_cntr ;
	DWORD					sofs ;
	DWORD					rgba ;
	DG_DMAPACK_TAG			*next_packet ;

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ

	Vertex = VertexRoot ;
	vtx_cntr = 0 ;
	while( TRUE )
	{
		/*-- 頂点設定( TRIANGLEx2 の6頂点版(Indexの方が高速かは未検証)) --*/
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[0], packet->u0, packet->v0) ;

		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[1], packet->u1, packet->v0) ;

		setVertexDMAPack2DXY(  &Vertex[2], packet->x0, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[2], packet->u0, packet->v1) ;

#if FALSE
		setVertexDMAPack2DXY(  &Vertex[3], packet->x1, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[3], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[3], packet->u1, packet->v0) ;

		setVertexDMAPack2DXY(  &Vertex[4], packet->x0, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[4], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[4], packet->u0, packet->v1) ;
#else
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
#endif

		setVertexDMAPack2DXY(  &Vertex[5], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[5], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[5], packet->u1, packet->v1) ;
		/*--------------------------------------------------------------*/

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;

		if( next_packet->cmd != DG_DMAPACK_CMD_SPRT )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次もSPRITEなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, vtx_cntr/3) ;

	return ( (void*)&packet[1] );
}

static void *DrawRSprt( DG_DMAPACK_RSPRT *packet )
{
	DG_VERTEX_DMAPACK2D		Vertex[4] ;
	DWORD					sofs ;
	DWORD					rgba ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_RSPRT )
	{
		return(DrawRSprts((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;

	setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[0], packet->u0, packet->v0) ;

	setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[1], packet->u1, packet->v0) ;

	setVertexDMAPack2DXY(  &Vertex[2], packet->x2, packet->y2) ;
	setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[2], packet->u0, packet->v1) ;

	setVertexDMAPack2DXY(  &Vertex[3], packet->x3, packet->y3) ;
	setVertexDMAPack2DRGBA(&Vertex[3], rgba) ;
	setVertexDMAPack2DUV(  &Vertex[3], packet->u1, packet->v1) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;

	return ( (void*)&packet[1] );
}

static void *DrawRSprts( DG_DMAPACK_RSPRT *packet )
{
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;
	DWORD					rgba ;

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[0], packet->u0, packet->v0) ;
		setVertexDMAPack2DXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[1], packet->u1, packet->v0) ;
		setVertexDMAPack2DXY(  &Vertex[2], packet->x2, packet->y2) ;
		setVertexDMAPack2DRGBA(&Vertex[2], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[2], packet->u0, packet->v1) ;
#if FALSE
		setVertexDMAPack2DXY(  &Vertex[3], packet->x1, packet->y1) ;
		setVertexDMAPack2DRGBA(&Vertex[3], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[3], packet->u1, packet->v0) ;
		setVertexDMAPack2DXY(  &Vertex[4], packet->x2, packet->y2) ;
		setVertexDMAPack2DRGBA(&Vertex[4], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[4], packet->u0, packet->v1) ;
#else
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
#endif
		setVertexDMAPack2DXY(  &Vertex[5], packet->x3, packet->y3) ;
		setVertexDMAPack2DRGBA(&Vertex[5], rgba) ;
		setVertexDMAPack2DUV(  &Vertex[5], packet->u1, packet->v1) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_RSPRT )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), vtx_cntr, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, vtx_cntr/3) ;

	return ( (void*)&packet[1] );
}

static void *DrawLineStrip( DG_DMAPACK_LINESTRIP *packet )
{
	DG_DMAPACK_VERTEX	*vertex ;
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D	*Vertex ;
	int					i ;
	int					v_count ;
	DWORD				sofs ;
	DG_VERTEX_DMAPACK2D	VertexBuff[32] ;

	v_count = packet->v_count ;
	if( v_count < 32 )
	{
		VertexRoot = VertexBuff ;
	}
	else if( v_count < (DG_GetWorkBufferSize()/sizeof(DG_VERTEX_DMAPACK2D)) )
	{
		VertexRoot = (DG_VERTEX_DMAPACK2D *)DG_GetWorkBuffer() ;
	}
	else
	{
		VertexRoot = NULL ;
	}

	ASSERT( VertexRoot ) ;
	if( VertexRoot )
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		Vertex = VertexRoot ;
		for ( i = 0 ; i < v_count ; i++ ){
			setVertexDMAPack2DXY(  Vertex, vertex->x, vertex->y) ;
			setVertexDMAPack2DRGBA(Vertex, EXCHG_COSTALPHA(vertex->rgba)) ;
			setVertexDMAPack2DUV(  Vertex, vertex->u, vertex->v) ;

			vertex++ ;
			Vertex++ ;
		}

		DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), v_count, &sofs) ;
		DG_DrawPrimitive(D3DPT_LINESTRIP, sofs, v_count - 1) ;
	}
	else
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		vertex += v_count ;
	}

	return ( (void*)vertex );
}

static void *DrawTriangleStrip( DG_DMAPACK_TRIANGLESTRIP *packet )
{
	DG_DMAPACK_VERTEX	*vertex ;
	DG_VERTEX_DMAPACK2D	*VertexRoot ;
	DG_VERTEX_DMAPACK2D	*Vertex ;
	int					i ;
	int					v_count ;
	DWORD				sofs ;
	DG_VERTEX_DMAPACK2D	VertexBuff[32] ;

	v_count = packet->v_count ;
	if( v_count < 32 )
	{
		VertexRoot = VertexBuff ;
	}
	else if( v_count < (DG_GetWorkBufferSize()/sizeof(DG_VERTEX_DMAPACK2D)) )
	{
		VertexRoot = (DG_VERTEX_DMAPACK2D *)DG_GetWorkBuffer() ;
	}
	else
	{
		VertexRoot = NULL ;
	}

	ASSERT( VertexRoot ) ;
	if( VertexRoot )
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		Vertex = VertexRoot ;
		for ( i = 0 ; i < v_count ; i++ ){
			setVertexDMAPack2DXY(  Vertex, vertex->x, vertex->y) ;
			setVertexDMAPack2DRGBA(Vertex, EXCHG_COSTALPHA(vertex->rgba)) ;
			setVertexDMAPack2DUV(  Vertex, vertex->u, vertex->v) ;

			vertex++ ;
			Vertex++ ;
		}

		DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D), v_count, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, v_count - 2) ;
	}
	else
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		vertex += v_count ;
	}

	return ( (void*)vertex );
}

static void *DrawDummy( void *packet )
{
	return(NULL) ;
}

/* ---------------------------------------------------------------- */
static void *DrawModeNVS( DG_DMAPACK_PARAM *packet )
{
	return(DrawMode(packet)) ;
}

static void *DrawModeEnableNVS( DG_DMAPACK_PARAM *packet )
{
	return(DrawModeEnable(packet)) ;
}

static void *DrawModeDisableNVS( DG_DMAPACK_PARAM *packet )
{
	return(DrawModeDisable(packet)) ;
	/* フィルタモード切替チェック */
}

static void *DrawTexNVS( DG_DMAPACK_TEX *packet )
{
	return(DrawTex(packet)) ;
}

static FMATRIX	_tex_lin_mtx =
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
} ;
static void *DrawTexLinNVS( DG_DMAPACK_TEX *packet )
{
	DG_TEX_LIN	*tex ;

	/* テクスチャ設定 */
	tex = (DG_TEX_LIN*)packet->tex ;

	/* Imageが変わっていそうだったら変更 */
	if( DG_LinerTextureGetImageDirty(tex) )
	{
		DG_LinerTextureFlushImage(tex) ;
		DG_LinerTextureClearImageDirty(tex) ;
	}

	DG_SetLinerTexture(tex) ;

	/* 頂点シェーダーのパラメータも変更する */
	_tex_lin_mtx.m[0][0] = tex->tex_param.vx ;
	_tex_lin_mtx.m[1][1] = tex->tex_param.vy ;

	DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&_tex_lin_mtx) ;

	return ( (void*)&packet[1] );
}

static void *DrawUserTexNVS( DG_DMAPACK_TEX *packet )
{
	DG_USERTEX	*tex ;

	/* テクスチャ設定 */
	tex = (DG_USERTEX*)packet->tex ;
	DG_SetUserTexture(tex) ;

	/* 頂点シェーダーのパラメータも変更する */
	_tex_lin_mtx.m[0][0] = tex->tex_param.vx ;
	_tex_lin_mtx.m[1][1] = tex->tex_param.vy ;

	DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&_tex_lin_mtx) ;

	return ( (void*)&packet[1] );
}

static void *DrawAlphaNVS( DG_DMAPACK_ALPHA *packet )
{
	return(DrawAlpha(packet)) ;
}

static void *DrawWindowNVS( DG_DMAPACK_WINDOW *packet )
{
	return(DrawWindow(packet)) ;
}

static void *DrawSetZNVS( DG_DMAPACK_PARAM *packet )
{
	draw_z = *(float*)&packet->param ;

	/* World Matrixを変更することにより、以降の描画Ｚを設定 */
	screen_param[0].vz = draw_z ;
	SetScreenParamTransform(screen_param) ;

	return ( (void*)&packet[1] );
}

static void *DrawUseFrameTexNVS( DG_DMAPACK_PARAM *packet )
{
	LPDIRECT3DTEXTURE8	lpTex ;
	int					page ;
	FMATRIX				texmtx ;

	if ( packet->param != -1 ){
		switch ( packet->param ){
		  case 0:
			page = DG_CurrentBackBuffer ;
			break ;

		  case 1:
			page = DG_CurrentBackBuffer ^ 1 ;
			break ;

		  default:
			page = 2 ;
			break ;
		}
		lpTex = DG_SystemTexture[page].tex_trans.ptex ;
		DG_SetTextureDirect( 0, lpTex );

		texmtx = DG_UnitMatrix ;
		texmtx.m[0][0] = DG_BackBufferTexScaleParam[0].vx ;
		texmtx.m[1][1] = DG_BackBufferTexScaleParam[0].vy ;
		texmtx.m[2][0] = DG_BackBufferTexScaleParam[1].vx ;
		texmtx.m[2][1] = DG_BackBufferTexScaleParam[1].vy ;
		DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&texmtx) ;
	} else {

		texmtx = DG_UnitMatrix ;
		texmtx.m[0][0] = tex_param_normal[0].vx ;
		texmtx.m[1][1] = tex_param_normal[0].vy ;
		texmtx.m[2][0] = tex_param_normal[1].vx ;
		texmtx.m[2][1] = tex_param_normal[1].vy ;
		DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&texmtx) ;
	}
	return ( (void*)&packet[1] );
}

static void *DrawBackupFrameNVS( DG_DMAPACK_PARAM *packet )
{
	int		mono_flag = 0, col = 0xffffffff ;
	static FVECTOR full_screen_param[2] = {
		{-1.0f,1.0f,1.0f,1.0f},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT, 0.0f, 0.0f}
	};
	DG_VIEWPORT			viewport ;
	LPDIRECT3DTEXTURE8	lpTex ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	int					page ;
	FMATRIX				texmtx ;

	if ( (packet->param & 0xfffffff8) )
	{

		ASSERT(0) ;		// Surface指定には対応できません
		return ( (void*)&packet[1] );
	}

	/*-- カレントのRenderTarget記憶 ----------------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	switch ( packet->param & 1 ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;

	  case 1:
		page = DG_CurrentBackBuffer ^ 1 ;
		break ;
	}
	lpTex = DG_SystemTexture[page].tex_trans.ptex ;

	/* 指定したバッファをテクスチャとして退避バッファへレンダリングする */
	DG_SetRenderTarget( DG_BackBufferSurface[2], NULL );
	if ( packet->param & 0x04 ){
		mono_flag = 1 ;
	}

	viewport.lx     = (float)(( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0)
					/ (float)DISPLAY_WIDTH ;
	viewport.ly     = (float)(( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0)
					/ (float)DISPLAY_HEIGHT ;
	viewport.width  = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	viewport.height = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;

	viewport.lx     *= DG_BackBufferWidthAdjustCoef ;
	viewport.ly     *= DG_BackBufferHeightAdjustCoef ;
	viewport.width  *= DG_BackBufferWidthAdjustCoef ;
	viewport.height *= DG_BackBufferHeightAdjustCoef ;
	
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );

	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );

	DG_SetTextureDirect( 0, lpTex );

	texmtx = DG_UnitMatrix ;
	texmtx.m[0][0] = DG_BackBufferTexScaleParam[0].vx ;
	texmtx.m[1][1] = DG_BackBufferTexScaleParam[0].vy ;
	texmtx.m[2][0] = DG_BackBufferTexScaleParam[1].vx ;
	texmtx.m[2][1] = DG_BackBufferTexScaleParam[1].vy ;
	DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&texmtx) ;
	SetScreenParamTransform(full_screen_param) ;
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 255 ) );
	DG_SetAlphaMode( 0 );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	if ( mono_flag ){
		if( DG_CheckPixelShaderUseable() )
		{
			/*-- Picxel Shader版 -------------------------------------------*/

			DG_SetPixelShader(&DG_DmapackPixelShader[0]) ;
			col = EXCHG_COSTALPHA(0x80808080) ;
			/*--------------------------------------------------------------*/
		}
		else
		{
			/*-- Texture Stage使用版 ---------------------------------------*/

#if TRUE	// 通常版

			col = EXCHG_COSTALPHA(0xffffffff) ;
#else		// 豪華版

			// 色空間変換係数設定(mtex_gray.psh参照)
			DG_SetRenderState(D3DRS_TEXTUREFACTOR,
					0x80000000
					| ((DWORD)(255.0f * ((0.30f+1.0f)*0.5f)) << 16)
					| ((DWORD)(255.0f * ((0.59f+1.0f)*0.5f)) << 8)
					| ((DWORD)(255.0f * ((0.11f+1.0f)*0.5f)) << 0)) ;

			/* 1st Stage: Color * 0.5f */
			DG_SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE) ;
			DG_SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE) ;
			DG_SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE) ;

			/* 2nd Stage: Current + 0.5f */
			DG_SetTextureDirect(1, NULL) ;
			DG_SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_ADD) ;
			DG_SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT) ;
			DG_SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE) ;

			DG_SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1) ;
			DG_SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT) ;
			DG_SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT) ;

			/* 3rd Stage: Grayscale */
			DG_SetTextureDirect(2, NULL) ;
			DG_SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DOTPRODUCT3) ;
			DG_SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT) ;
			DG_SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_TFACTOR) ;

			DG_SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1) ;
			DG_SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_CURRENT) ;

			col = EXCHG_COSTALPHA(0x80808080) ;
#endif
			/*--------------------------------------------------------------*/
		}
	}

	{
		DG_VERTEX_DMAPACK2D_FVF	Vertex[4] ;
		DWORD				sofs ;
		float	lu, lv, ru, rv ;
		float	ofs_lu, ofs_lv, ofs_ru, ofs_rv ;

#if TRUE
		ofs_lu = ((float)DRAW_WIDTH * 0.5f)/(float)DG_BackBufferRendWidth ;
		ofs_lv = ((float)DRAW_HEIGHT * 0.5f)/(float)DG_BackBufferRendHeight ;
		ofs_ru = ofs_lu ;
		ofs_rv = ofs_lv ;

		if( (packet->param & 2) )
		{
			ofs_lu += 0.5f ;
			ofs_lv += 0.5f ;
			ofs_ru -= 0.5f ;
			ofs_rv -= 0.5f ;
		}

		lu = DG_FRAME_U(ofs_lu) * 16384.0f ; 
		lv = DG_FRAME_V(ofs_lv) * 16384.0f ; 
		ru = DG_FRAME_U(DRAW_WIDTH  + ofs_ru) * 16384.0f ; 
		rv = DG_FRAME_V(DRAW_HEIGHT + ofs_rv) * 16384.0f ; 
#else
		#define	BKUPFRAME_OFS_L		(0.50f)
		#define	BKUPFRAME_OFS_R		(-0.50f)
		#define	BKUPFRAME_OFS_L2	(BKUPFRAME_OFS_L + 0.5f)
		//#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R + 0.5f)
		#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R - 0.5f)

		if( !(packet->param & 2) )
		{
			lu = DG_FRAME_U(BKUPFRAME_OFS_L) * 16384.0f ; 
			lv = DG_FRAME_V(BKUPFRAME_OFS_L) * 16384.0f ; 
			ru = DG_FRAME_U(DRAW_WIDTH  + BKUPFRAME_OFS_R) * 16384.0f ; 
			rv = DG_FRAME_V(DRAW_HEIGHT + BKUPFRAME_OFS_R) * 16384.0f ; 
		}
		else
		{
			lu = DG_FRAME_U(BKUPFRAME_OFS_L2) * 16384.0f ; 
			lv = DG_FRAME_V(BKUPFRAME_OFS_L2) * 16384.0f ; 
			ru = DG_FRAME_U(DRAW_WIDTH  + BKUPFRAME_OFS_R2) * 16384.0f ; 
			rv = DG_FRAME_V(DRAW_HEIGHT + BKUPFRAME_OFS_R2) * 16384.0f ; 
		}
#endif

		setVertexDMAPack2DFVFXY(  &Vertex[0], 0, 0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], DRAW_WIDTH, 0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], 0, DRAW_HEIGHT) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[3], DRAW_WIDTH, DRAW_HEIGHT) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/* レンダリング対象を元に戻す */
	if ( mono_flag ){
		if( DG_CheckPixelShaderUseable() )
		{
			DG_SetPixelShader( NULL );
		}
		else
		{
#if TRUE	// 通常版

#else		// 豪華版
			DG_InitTextureStageState(0) ;
			DG_InitTextureStageState(1) ;
			DG_InitTextureStageState(2) ;
#endif
		}
	}
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( cur_front_surface, cur_depth_surface );
	//DG_SetViewport( &DG_CurrentViewport );
	DG_SetViewport( &dmapack_viewport );
	SetScreenParamTransform(screen_param) ;
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	if( cur_front_surface ){ IDirect3DSurface8_Release( cur_front_surface ) ; }
	if( cur_depth_surface ){ IDirect3DSurface8_Release( cur_depth_surface ) ; }
#endif

	return ( (void*)&packet[1] );
}

static void *DrawBackupFrameRectNVS( DG_DMAPACK_BACKUPFRAME_RECT *packet )
{
	int		mono_flag = 0, col = 0xffffffff ;
	static FVECTOR full_screen_param[2] = {
		{-1.0f,1.0f,1.0f,1.0f},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT, 0.0f, 0.0f}
	};
	DG_VIEWPORT			viewport ;
	LPDIRECT3DTEXTURE8	lpTex ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	int					page ;
	FMATRIX				texmtx ;

	if ( (packet->param & 0xfffffff8) )
	{

		ASSERT(0) ;		// Surface指定には対応できません
		return ( (void*)&packet[1] );
	}

	/*-- カレントのRenderTarget記憶 ----------------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	switch ( packet->param & 1 ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;

	  case 1:
		page = DG_CurrentBackBuffer ^ 1 ;
		break ;
	}
	lpTex = DG_SystemTexture[page].tex_trans.ptex ;

	/* 指定したバッファをテクスチャとして退避バッファへレンダリングする */
	DG_SetRenderTarget( DG_BackBufferSurface[2], NULL );
	if ( packet->param & 0x04 ){
		mono_flag = 1 ;
	}

	viewport.lx     = (float)(( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0)
					/ (float)DISPLAY_WIDTH ;
	viewport.ly     = (float)(( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0)
					/ (float)DISPLAY_HEIGHT ;
	viewport.width  = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	viewport.height = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;

	viewport.lx     *= DG_BackBufferWidthAdjustCoef ;
	viewport.ly     *= DG_BackBufferHeightAdjustCoef ;
	viewport.width  *= DG_BackBufferWidthAdjustCoef ;
	viewport.height *= DG_BackBufferHeightAdjustCoef ;
	
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );

	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );

	DG_SetTextureDirect( 0, lpTex );

	texmtx = DG_UnitMatrix ;
	texmtx.m[0][0] = DG_BackBufferTexScaleParam[0].vx ;
	texmtx.m[1][1] = DG_BackBufferTexScaleParam[0].vy ;
	texmtx.m[2][0] = DG_BackBufferTexScaleParam[1].vx ;
	texmtx.m[2][1] = DG_BackBufferTexScaleParam[1].vy ;
	DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&texmtx) ;
	SetScreenParamTransform(full_screen_param) ;
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 255 ) );
	DG_SetAlphaMode( 0 );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	if ( mono_flag ){
		if( DG_CheckPixelShaderUseable() )
		{
			/*-- Picxel Shader版 -------------------------------------------*/

			DG_SetPixelShader(&DG_DmapackPixelShader[0]) ;
			col = EXCHG_COSTALPHA(0x80808080) ;
			/*--------------------------------------------------------------*/
		}
		else
		{
			/*-- Texture Stage使用版 ---------------------------------------*/

#if TRUE	// 通常版

			col = EXCHG_COSTALPHA(0xffffffff) ;
#else		// 豪華版

			// 色空間変換係数設定(mtex_gray.psh参照)
			DG_SetRenderState(D3DRS_TEXTUREFACTOR,
					0x80000000
					| ((DWORD)(255.0f * ((0.30f+1.0f)*0.5f)) << 16)
					| ((DWORD)(255.0f * ((0.59f+1.0f)*0.5f)) << 8)
					| ((DWORD)(255.0f * ((0.11f+1.0f)*0.5f)) << 0)) ;

			/* 1st Stage: Color * 0.5f */
			DG_SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE) ;
			DG_SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE) ;
			DG_SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE) ;

			/* 2nd Stage: Current + 0.5f */
			DG_SetTextureDirect(1, NULL) ;
			DG_SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_ADD) ;
			DG_SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT) ;
			DG_SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE) ;

			DG_SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1) ;
			DG_SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT) ;
			DG_SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT) ;

			/* 3rd Stage: Grayscale */
			DG_SetTextureDirect(2, NULL) ;
			DG_SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DOTPRODUCT3) ;
			DG_SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT) ;
			DG_SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_TFACTOR) ;

			DG_SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1) ;
			DG_SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_CURRENT) ;

			col = EXCHG_COSTALPHA(0x80808080) ;
#endif
			/*--------------------------------------------------------------*/
		}
	}

	{
		DG_VERTEX_DMAPACK2D_FVF	Vertex[4] ;
		DWORD				sofs ;
		float	lx, ly, rx, ry ;
		float	lu, lv, ru, rv ;
		float	ofs_lu, ofs_lv, ofs_ru, ofs_rv ;

#if TRUE
		lx = (float)packet->x ;
		ly = (float)packet->y ;
		rx = (float)(packet->x + packet->w) ;
		ry = (float)(packet->y + packet->h) ;

		ofs_lu = ((float)DRAW_WIDTH * 0.5f)/(float)DG_BackBufferRendWidth ;
		ofs_lv = ((float)DRAW_HEIGHT * 0.5f)/(float)DG_BackBufferRendHeight ;
		ofs_ru = ofs_lu ;
		ofs_rv = ofs_lv ;

		if( (packet->param & 2) )
		{
			ofs_lu += 0.5f ;
			ofs_lv += 0.5f ;
			ofs_ru -= 0.5f ;
			ofs_rv -= 0.5f ;
		}

		lu = DG_FRAME_U(lx + ofs_lu) * 16384.0f ; 
		lv = DG_FRAME_V(ly + ofs_lv) * 16384.0f ; 
		ru = DG_FRAME_U(rx + ofs_ru) * 16384.0f ; 
		rv = DG_FRAME_V(ry + ofs_rv) * 16384.0f ; 
#else
		#define	BKUPFRAME_OFS_L		(0.50f)
		#define	BKUPFRAME_OFS_R		(-0.50f)
		#define	BKUPFRAME_OFS_L2	(BKUPFRAME_OFS_L + 0.5f)
		//#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R + 0.5f)
		#define	BKUPFRAME_OFS_R2	(BKUPFRAME_OFS_R - 0.5f)

		lx = (float)packet->x ;
		ly = (float)packet->y ;
		rx = (float)(packet->x + packet->w) ;
		ry = (float)(packet->y + packet->h) ;

		if( !(packet->param & 2) )
		{
			lu = DG_FRAME_U(lx + BKUPFRAME_OFS_L) * 16384.0f ; 
			lv = DG_FRAME_V(ly + BKUPFRAME_OFS_L) * 16384.0f ; 
			ru = DG_FRAME_U(rx + BKUPFRAME_OFS_R) * 16384.0f ; 
			rv = DG_FRAME_V(ry + BKUPFRAME_OFS_R) * 16384.0f ; 
		}
		else
		{
			lu = DG_FRAME_U(lx + BKUPFRAME_OFS_L2) * 16384.0f ; 
			lv = DG_FRAME_V(ly + BKUPFRAME_OFS_L2) * 16384.0f ; 
			ru = DG_FRAME_U(rx + BKUPFRAME_OFS_R2) * 16384.0f ; 
			rv = DG_FRAME_V(ry + BKUPFRAME_OFS_R2) * 16384.0f ; 
		}
#endif
		setVertexDMAPack2DFVFXY(  &Vertex[0], lx, ly) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], rx, ly) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], lx, ry) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[3], rx, ry) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/* レンダリング対象を元に戻す */
	if ( mono_flag ){
		if( DG_CheckPixelShaderUseable() )
		{
			DG_SetPixelShader( NULL );
		}
		else
		{
#if TRUE	// 通常版

#else		// 豪華版
			DG_InitTextureStageState(0) ;
			DG_InitTextureStageState(1) ;
			DG_InitTextureStageState(2) ;
#endif
		}
	}
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( cur_front_surface, cur_depth_surface );
	//DG_SetViewport( &DG_CurrentViewport );
	DG_SetViewport( &dmapack_viewport );
	SetScreenParamTransform(screen_param) ;
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	if( cur_front_surface ){ IDirect3DSurface8_Release( cur_front_surface ) ; }
	if( cur_depth_surface ){ IDirect3DSurface8_Release( cur_depth_surface ) ; }
#endif

	return ( (void*)&packet[1] );
}

static void *DrawRendBackBuffer2FrameNVS( DG_DMAPACK_PARAM *packet )
{
	return( DrawRendBackBuffer2Frame(packet) ) ;
}

static void *DrawPacketCallbackNVS( DG_DMAPACK_CALLBACK *packet )
{
	return( DrawPacketCallback(packet) ) ;
}

static void *DrawNextNVS( DG_DMAPACK_NEXT *packet )
{
	return( DrawNext(packet) ) ;
}


/* ---------------------------------------------------------------- */
/* 描画関数関連 */
static void *DrawPointNVS( DG_DMAPACK_POINT *packet )
{
	/* 未サポート */
	return ( (void*)&packet[1] );
}

static void *DrawLineNVS( DG_DMAPACK_LINE *packet )
{
	DG_VERTEX_DMAPACK2D_FVF Vertex[2] ;
	DWORD					sofs ;
	DG_DMAPACK_TAG			*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_LINE )
	{
		return(DrawLinesNVS((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 2, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, 1 );

	return ( (void*)&packet[1] );
}

static void *DrawLinesNVS( DG_DMAPACK_LINE *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTextureNVS( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

		vtx_cntr += 2 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_LINE )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 2 ;			// 次もSPRITEなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr>>1 );

	return ( (void*)next_packet );
}

static void *DrawLineFNVS( DG_DMAPACK_LINE_F *packet )
{
	DG_VERTEX_DMAPACK2D_FVF Vertex[2] ;
	DWORD					sofs ;
	DG_DMAPACK_TAG			*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_LINE_F )
	{
		return(DrawLinesFNVS((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTexture( 0, NULL );

	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 2, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, 1 );

	return ( (void*)&packet[1] );
}

static void *DrawLinesFNVS( DG_DMAPACK_LINE_F *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTextureNVS( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;

		vtx_cntr += 2 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_LINE_F )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 2 ;			// 次もSPRITEなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr>>1 );

	return ( (void*)next_packet );
}

static void *DrawTriangleNVS( DG_DMAPACK_TRIANGLE *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	Vertex[3] ;
	DWORD					sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_TRIANGLE )
	{
		return(DrawTrianglesNVS((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTextureNVS( 0, NULL );

	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x2, packet->y2) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba2)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 3, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, 1) ;

	return ( (void*)&packet[1] );
}

static void *DrawTrianglesNVS( DG_DMAPACK_TRIANGLE *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF	*Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTextureNVS( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x2, packet->y2) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba2)) ;

		vtx_cntr += 3 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_TRIANGLE )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 3 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), vtx_cntr, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, vtx_cntr/3) ;

	return ( (void*)&packet[1] );
}

static void *DrawQuadNVS( DG_DMAPACK_QUAD *packet )
{
	DG_VERTEX_DMAPACK2D_FVF Vertex[4] ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_QUAD )
	{
		return(DrawQuadsNVS((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	DG_SetTextureNVS( 0, NULL );

	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x3, packet->y3) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba3)) ;
	setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x2, packet->y2) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[3], EXCHG_COSTALPHA(packet->rgba2)) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );

	return ( (void*)&packet[1] );
}

static void *DrawQuadsNVS( DG_DMAPACK_QUAD *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;

	DG_SetTextureNVS( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], EXCHG_COSTALPHA(packet->rgba0)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], EXCHG_COSTALPHA(packet->rgba1)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x3, packet->y3) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], EXCHG_COSTALPHA(packet->rgba3)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], EXCHG_COSTALPHA(packet->rgba1)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[4], packet->x3, packet->y3) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[4], EXCHG_COSTALPHA(packet->rgba3)) ;
		setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x2, packet->y2) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[5], EXCHG_COSTALPHA(packet->rgba2)) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_QUAD )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, vtx_cntr/3 );

	return ( (void*)&packet[1] );
}

static void *DrawBoxNVS( DG_DMAPACK_BOX *packet )
{
#if FALSE
	DG_VERTEX_DMAPACK2D_FVF Vertex[4] ;
#else
	DG_VERTEX_DMAPACK2D_FVF Vertex[6] ;
#endif
	DWORD				sofs ;
	DWORD				rgba ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

#if TRUE
	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_BOX )
	{
		return(DrawBoxesNVS((void *)packet)) ;	// 連結して描画
	}
#endif
	/*------------------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;


	DG_SetTextureNVS( 0, NULL );

#if FALSE
	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[3], rgba) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );
#else
	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
	Vertex[3] = Vertex[1] ;
	Vertex[4] = Vertex[2] ;
	setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[5], rgba) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 6, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, 2 );
#endif
	return ( (void*)&packet[1] );
}

static void *DrawBoxesNVS( DG_DMAPACK_BOX *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;
	DWORD				rgba ;

	DG_SetTextureNVS( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
		setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[5], rgba) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_BOX )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}
	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF),
							vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, vtx_cntr/3 ) ;

	return ( (void*)&packet[1] );
}

static void *DrawBoxFNVS( DG_DMAPACK_BOX_F *packet )
{
#if FALSE
	DG_VERTEX_DMAPACK2D_FVF Vertex[4] ;
#else
	DG_VERTEX_DMAPACK2D_FVF Vertex[6] ;
#endif
	DWORD				sofs ;
	DWORD				rgba ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

#if TRUE
	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_BOX_F )
	{
		return(DrawBoxesFNVS((void *)packet)) ;	// 連結して描画
	}
#endif
	/*------------------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;


	DG_SetTextureNVS( 0, NULL );

#if FALSE
	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[3], rgba) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );
#else
	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
	Vertex[3] = Vertex[1] ;
	Vertex[4] = Vertex[2] ;
	setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[5], rgba) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 6, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, 2 );
#endif
	return ( (void*)&packet[1] );
}

static void *DrawBoxesFNVS( DG_DMAPACK_BOX_F *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;
	DWORD				rgba ;

	DG_SetTextureNVS( 0, NULL );

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
		setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[5], rgba) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_BOX_F )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}
	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF),
							vtx_cntr, &sofs) ;
	DG_DrawPrimitive( D3DPT_TRIANGLELIST, sofs, vtx_cntr/3 ) ;

	return ( (void*)&packet[1] );
}

static void *DrawSprtNVS( DG_DMAPACK_SPRT *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	Vertex[4] ;
	DWORD					sofs ;
	DWORD					rgba ;
	DG_DMAPACK_TAG			*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_SPRT )
	{
		return(DrawSprtsNVS((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	/*-- 頂点設定 ------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;

	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[0], packet->u0, packet->v0) ;

	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[1], packet->u1, packet->v0) ;

	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[2], packet->u0, packet->v1) ;

	setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[3], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[3], packet->u1, packet->v1) ;
	/*------------------------------------------------------------------*/

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;

	return ( (void*)next_packet );
}

static void *DrawSprtsNVS( DG_DMAPACK_SPRT *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF	*Vertex ;
	DWORD					vtx_cntr ;
	DWORD					sofs ;
	DWORD					rgba ;
	DG_DMAPACK_TAG			*next_packet ;

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ

	Vertex = VertexRoot ;
	vtx_cntr = 0 ;
	while( TRUE )
	{
		/*-- 頂点設定( TRIANGLEx2 の6頂点版(Indexの方が高速かは未検証)) --*/
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[0], packet->u0, packet->v0) ;

		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[1], packet->u1, packet->v0) ;

		setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x0, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[2], packet->u0, packet->v1) ;
#if FALSE
		setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x1, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[3], packet->u1, packet->v0) ;

		setVertexDMAPack2DFVFXY(  &Vertex[4], packet->x0, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[4], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[4], packet->u0, packet->v1) ;
#else
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
#endif
		setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[5], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[5], packet->u1, packet->v1) ;
		/*--------------------------------------------------------------*/

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_SPRT )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次もSPRITEなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), vtx_cntr, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, vtx_cntr/3) ;

	return ( (void*)&packet[1] );
}

static void *DrawRSprtNVS( DG_DMAPACK_RSPRT *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	Vertex[4] ;
	DWORD					sofs ;
	DWORD					rgba ;
	DG_DMAPACK_TAG		*next_packet ;

	/*-- 連結判定 ------------------------------------------------------*/

	next_packet = (void*)&packet[1] ;
	if( next_packet->cmd == DG_DMAPACK_CMD_RSPRT )
	{
		return(DrawRSprtsNVS((void *)packet)) ;	// 連結して描画
	}
	/*------------------------------------------------------------------*/

	rgba = EXCHG_COSTALPHA(packet->rgba) ;

	setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[0], packet->u0, packet->v0) ;

	setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[1], packet->u1, packet->v0) ;

	setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x2, packet->y2) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[2], packet->u0, packet->v1) ;

	setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x3, packet->y3) ;
	setVertexDMAPack2DFVFRGBA(&Vertex[3], rgba) ;
	setVertexDMAPack2DFVFUV(  &Vertex[3], packet->u1, packet->v1) ;

	DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;

	return ( (void*)&packet[1] );
}

static void *DrawRSprtsNVS( DG_DMAPACK_RSPRT *packet )
{
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF *Vertex ;
	DWORD				vtx_cntr ;
	DWORD				sofs ;
	DG_DMAPACK_TAG		*next_packet ;
	DWORD					rgba ;

	VertexRoot = DG_GetWorkBuffer() ;	// 作業用メモリ
	Vertex     = VertexRoot ;
	vtx_cntr   = 0 ;
	while( TRUE )
	{
		rgba = EXCHG_COSTALPHA(packet->rgba) ;

		setVertexDMAPack2DFVFXY(  &Vertex[0], packet->x0, packet->y0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[0], packet->u0, packet->v0) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[1], packet->u1, packet->v0) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], packet->x2, packet->y2) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[2], packet->u0, packet->v1) ;
#if FALSE
		setVertexDMAPack2DFVFXY(  &Vertex[3], packet->x1, packet->y1) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[3], packet->u1, packet->v0) ;
		setVertexDMAPack2DFVFXY(  &Vertex[4], packet->x2, packet->y2) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[4], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[4], packet->u0, packet->v1) ;
#else
		Vertex[3] = Vertex[1] ;
		Vertex[4] = Vertex[2] ;
#endif
		setVertexDMAPack2DFVFXY(  &Vertex[5], packet->x3, packet->y3) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[5], rgba) ;
		setVertexDMAPack2DFVFUV(  &Vertex[5], packet->u1, packet->v1) ;

		vtx_cntr += 6 ;
		next_packet = (void*)&packet[1] ;
		if( next_packet->cmd != DG_DMAPACK_CMD_RSPRT )
		{
			break ;	// 終了
		}
		else
		{
			Vertex += 6 ;			// 次も同種Packetなので連続処理
			packet = (void *)next_packet ;
		}
	}

	DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), vtx_cntr, &sofs) ;
	DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, vtx_cntr/3) ;

	return ( (void*)&packet[1] );
}

static void *DrawLineStripNVS( DG_DMAPACK_LINESTRIP *packet )
{
	DG_DMAPACK_VERTEX		*vertex ;
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF	*Vertex ;
	int					i ;
	int					v_count ;
	DWORD				sofs ;
	DG_VERTEX_DMAPACK2D_FVF	VertexBuff[32] ;

	v_count = packet->v_count ;
	if( v_count < 32 )
	{
		VertexRoot = VertexBuff ;
	}
	else if( v_count < (DG_GetWorkBufferSize()/sizeof(DG_VERTEX_DMAPACK2D_FVF)) )
	{
		VertexRoot = (DG_VERTEX_DMAPACK2D_FVF *)DG_GetWorkBuffer() ;
	}
	else
	{
		VertexRoot = NULL ;
	}

	ASSERT( VertexRoot ) ;
	if( VertexRoot )
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		Vertex = VertexRoot ;
		for ( i = 0 ; i < v_count ; i++ ){
			setVertexDMAPack2DFVFXY(  Vertex, vertex->x, vertex->y) ;
			setVertexDMAPack2DFVFRGBA(Vertex, EXCHG_COSTALPHA(vertex->rgba)) ;
			setVertexDMAPack2DFVFUV(  Vertex, vertex->u, vertex->v) ;

			vertex++ ;
			Vertex++ ;
		}

		DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), v_count, &sofs) ;
		DG_DrawPrimitive(D3DPT_LINESTRIP, sofs, v_count - 1) ;
	}
	else
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		vertex += v_count ;
	}

	return ( (void*)vertex );
}

static void *DrawTriangleStripNVS( DG_DMAPACK_TRIANGLESTRIP *packet )
{
	DG_DMAPACK_VERTEX	*vertex ;
	DG_VERTEX_DMAPACK2D_FVF	*VertexRoot ;
	DG_VERTEX_DMAPACK2D_FVF	*Vertex ;
	int					i ;
	int					v_count ;
	DWORD				sofs ;
	DG_VERTEX_DMAPACK2D_FVF	VertexBuff[32] ;

	v_count = packet->v_count ;
	if( v_count < 32 )
	{
		VertexRoot = VertexBuff ;
	}
	else if( v_count < (DG_GetWorkBufferSize()/sizeof(DG_VERTEX_DMAPACK2D_FVF)) )
	{
		VertexRoot = (DG_VERTEX_DMAPACK2D_FVF *)DG_GetWorkBuffer() ;
	}
	else
	{
		VertexRoot = NULL ;
	}

	ASSERT( VertexRoot ) ;
	if( VertexRoot )
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		Vertex = VertexRoot ;
		for ( i = 0 ; i < v_count ; i++ ){
			setVertexDMAPack2DFVFXY(  Vertex, vertex->x, vertex->y) ;
			setVertexDMAPack2DFVFRGBA(Vertex, EXCHG_COSTALPHA(vertex->rgba)) ;
			setVertexDMAPack2DFVFUV(  Vertex, vertex->u, vertex->v) ;

			vertex++ ;
			Vertex++ ;
		}

		DG_SetDynamicVertexBuffer(VertexRoot, sizeof(DG_VERTEX_DMAPACK2D_FVF), v_count, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, v_count - 2) ;
	}
	else
	{
		vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
		vertex += v_count ;
	}

	return ( (void*)vertex );
}

static void *DrawDummyNVS( void *packet )
{
	return(NULL) ;
}

/*--------------------------------------------------------------------------*/
/*	DG_BackupFrame2Surface													*/
/*--------------------------------------------------------------------------*/

static void	_DG_BackupFrame2Surface(DG_BACKUPFRAME2SURFACE_PARAM *param) ;
static void	_DG_BackupFrame2SurfaceNVS(DG_BACKUPFRAME2SURFACE_PARAM *param) ;

void	DG_BackupFrame2Surface(DG_BACKUPFRAME2SURFACE_PARAM *param)
{
	if( DG_CheckUseVertexShader() )
	{
		_DG_BackupFrame2Surface(param) ;	// とりあえず対応？(多分いらない)
	}
	else
	{
		_DG_BackupFrame2SurfaceNVS(param) ;
	}
}

static void	_DG_BackupFrame2Surface(DG_BACKUPFRAME2SURFACE_PARAM *param)
{
	int		mono_flag = 0, col = 0xffffffff ;
	static FVECTOR full_screen_param[2] = {
		{-1.0f, 1.0f,1.0f,1.0f},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT, 0.0f, 0.0f}
	};
	DG_VIEWPORT			viewport ;
	LPDIRECT3DTEXTURE8	lpTex ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	int					page ;

	/*-- カレントのRenderTarget記憶 ----------------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	/*-- 転送元選択 ---------------------------------------------------------*/

	switch ( param->src_page ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;

	  case 1:
		page = DG_CurrentBackBuffer ^ 1 ;
		break ;

	  default :
		page = 2 ;
		break ;
	}
	lpTex = DG_SystemTexture[page].tex_trans.ptex ;
	/*-----------------------------------------------------------------------*/

	/*-- 転送先をRenderTargetとして指定 -------------------------------------*/

	DG_SetRenderTarget(param->dst_surface, NULL) ;
	/*-----------------------------------------------------------------------*/

	/*-- 描画領域の設定 -----------------------------------------------------*/

	viewport.lx     = param->dst_x ;
	viewport.ly     = param->dst_y ;
	viewport.width  = param->dst_w ;
	viewport.height = param->dst_h ;
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );
	/*-----------------------------------------------------------------------*/

	/*-- 描画環境設定 -------------------------------------------------------*/

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;
	DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;
	DG_SelectVertexShader( &DG_DmapackVertexShader[0] );
	/*-----------------------------------------------------------------------*/

	/*-- 描画 ---------------------------------------------------------------*/

	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );
	DG_SetTextureDirect( 0, lpTex );

	DG_SetVertexShaderConstant(CV_TEX0_SCALE, DG_BackBufferTexScaleParam, 2);
	DG_SetVertexShaderConstant(0, full_screen_param, 2);
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );

	{
		DG_VERTEX_DMAPACK2D	Vertex[4] ;
		DWORD				sofs ;
		float	lu, lv, ru, rv ;
		float	ofs_lu, ofs_lv, ofs_ru, ofs_rv ;

		ofs_lu = ((float)DRAW_WIDTH * 0.5f)/(float)DG_BackBufferRendWidth ;
		ofs_lv = ((float)DRAW_HEIGHT * 0.5f)/(float)DG_BackBufferRendHeight ;
		ofs_ru = ofs_lu ;
		ofs_rv = ofs_lv ;

		lu = DG_FRAME_U(ofs_lu) * 16384.0f ; 
		lv = DG_FRAME_V(ofs_lv) * 16384.0f ; 
		ru = DG_FRAME_U(DRAW_WIDTH  + ofs_ru) * 16384.0f ; 
		rv = DG_FRAME_V(DRAW_HEIGHT + ofs_rv) * 16384.0f ; 

		setVertexDMAPack2DXY(  &Vertex[0], -1.0f, -1.0f) ;
		setVertexDMAPack2DRGBA(&Vertex[0], 0xffffffff) ;
		setVertexDMAPack2DUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DXY(  &Vertex[1],  1.0f, -1.0f) ;
		setVertexDMAPack2DRGBA(&Vertex[1], 0xffffffff) ;
		setVertexDMAPack2DUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DXY(  &Vertex[2], -1.0f,  1.0f) ;
		setVertexDMAPack2DRGBA(&Vertex[2], 0xffffffff) ;
		setVertexDMAPack2DUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DXY(  &Vertex[3],  1.0f,  1.0f) ;
		setVertexDMAPack2DRGBA(&Vertex[3], 0xffffffff) ;
		setVertexDMAPack2DUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 終了処理 -----------------------------------------------------------*/

	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( cur_front_surface, cur_depth_surface );

	DG_SetViewport( &DG_CurrentViewport );

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	if( cur_front_surface ){ IDirect3DSurface8_Release( cur_front_surface ) ; }
	if( cur_depth_surface ){ IDirect3DSurface8_Release( cur_depth_surface ) ; }
#endif
	/*-----------------------------------------------------------------------*/
}

static void	_DG_BackupFrame2SurfaceNVS(DG_BACKUPFRAME2SURFACE_PARAM *param)
{
	static FVECTOR full_screen_param[2] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f,-1.0f, 1.0f, 0.0f}
	};
	DG_VIEWPORT			viewport ;
	LPDIRECT3DTEXTURE8	lpTex ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	int					page ;
	FMATRIX				texmtx ;

	/*-- カレントのRenderTarget記憶 ----------------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	/*-- 転送元選択 ---------------------------------------------------------*/

	switch ( param->src_page ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;

	  case 1:
		page = DG_CurrentBackBuffer ^ 1 ;
		break ;

	  default :
		page = 2 ;
		break ;
	}
	lpTex = DG_SystemTexture[page].tex_trans.ptex ;
	/*-----------------------------------------------------------------------*/

	/*-- 転送先をRenderTargetとして指定 -------------------------------------*/

	DG_SetRenderTarget(param->dst_surface, NULL) ;
	/*-----------------------------------------------------------------------*/

	/*-- 描画領域の設定 -----------------------------------------------------*/

	viewport.lx     = param->dst_x ;
	viewport.ly     = param->dst_y ;
	viewport.width  = param->dst_w ;
	viewport.height = param->dst_h ;
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );
	/*-----------------------------------------------------------------------*/

	/*-- 描画環境設定 -------------------------------------------------------*/

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;
	DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;

	/* FVF設定 */
	DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D_FLAG) ;

	/* Perspective設定 */
	{
		D3DXMATRIX	d3dx_mtx ;

		D3DXMatrixOrthoOffCenterLH(&d3dx_mtx, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f) ;

		DG_SetTransform(D3DTS_PROJECTION, (FMATRIX *)d3dx_mtx.m) ;
		DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix) ;
		DG_SetTransform(D3DTS_WORLD,      &DG_UnitMatrix) ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 描画 ---------------------------------------------------------------*/

	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );
	DG_SetTextureDirect( 0, lpTex );

	texmtx = DG_UnitMatrix ;
	texmtx.m[0][0] = DG_BackBufferTexScaleParam[0].vx ;
	texmtx.m[1][1] = DG_BackBufferTexScaleParam[0].vy ;
	texmtx.m[2][0] = DG_BackBufferTexScaleParam[1].vx ;
	texmtx.m[2][1] = DG_BackBufferTexScaleParam[1].vy ;
	DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&texmtx) ;

	SetScreenParamTransform(full_screen_param) ;
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );

	DG_SetAlphaMode( 0 );
	DG_SetPixelShader( NULL );

	DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

	{
		DG_VERTEX_DMAPACK2D_FVF	Vertex[4] ;
		DWORD				sofs ;
		float	lu, lv, ru, rv ;
		float	ofs_lu, ofs_lv, ofs_ru, ofs_rv ;

		ofs_lu = ((float)DRAW_WIDTH * 0.5f)/(float)DG_BackBufferRendWidth ;
		ofs_lv = ((float)DRAW_HEIGHT * 0.5f)/(float)DG_BackBufferRendHeight ;
		ofs_ru = ofs_lu ;
		ofs_rv = ofs_lv ;

		lu = DG_FRAME_U(ofs_lu) * 16384.0f ; 
		lv = DG_FRAME_V(ofs_lv) * 16384.0f ; 
		ru = DG_FRAME_U(DRAW_WIDTH  + ofs_ru) * 16384.0f ; 
		rv = DG_FRAME_V(DRAW_HEIGHT + ofs_rv) * 16384.0f ; 

		setVertexDMAPack2DFVFXY(  &Vertex[0], -1.0f, -1.0f) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], 0xffffffff) ;
		setVertexDMAPack2DFVFUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1],  1.0f, -1.0f) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], 0xffffffff) ;
		setVertexDMAPack2DFVFUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], -1.0f,  1.0f) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], 0xffffffff) ;
		setVertexDMAPack2DFVFUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[3],  1.0f,  1.0f) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], 0xffffffff) ;
		setVertexDMAPack2DFVFUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 終了処理 -----------------------------------------------------------*/

	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( cur_front_surface, cur_depth_surface );

	DG_SetViewport( &DG_CurrentViewport );

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	if( cur_front_surface ){ IDirect3DSurface8_Release( cur_front_surface ) ; }
	if( cur_depth_surface ){ IDirect3DSurface8_Release( cur_depth_surface ) ; }
#endif
	/*-----------------------------------------------------------------------*/
}
