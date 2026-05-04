//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xdmapack.c
	ＤＭＡパケット接続型オブジェクト管理ルーチン

	2002/02/15 K.Takabe
	$Id: xdmapack.c,v 1.38 2002/08/22 08:28:52 usr02774 Exp $

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

extern void *DG_SetDmapackAlpha( void *addr, u_long64 alpha );
	void		*addr ;		書き込みアドレス
	u_long64		alpha ;		アルファ設定
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

#ifdef KP_XBOX
#include <xtl.h>
#elif !defined(PSX2)
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

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_Debug.h"

#if BP_VITA
__thread
#endif
char *BP_CurrentDmaPackPtr = NULL;

/* ---------------------------------------------------------------- */
/* PS2形式からXBOX形式へ変換 */
//#define EXCHG_COLOR(_c)	( ((_c)&0xff00ff00) | (((_c)&0xff)<<16) | (((_c)&0xff0000)>>16) )
#define EXCHG_COLOR(_c)	(_c)
#define DMAPACK_COLOR_TO_INT(_c) (((unsigned) _c.r) | ((unsigned) _c.g << 8 ) | ((unsigned) _c.b << 16 ) | ((unsigned) _c.a << 24 ))

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
static void *DrawTexDyn( DG_DMAPACK_TEXTURE_DYNAMIC *packet );
static void *DrawTexDyn01( DG_DMAPACK_TEXTURE_DYNAMIC *packet );
static void *DrawTexLin( DG_DMAPACK_TEX *packet );
static void *DrawAlpha( DG_DMAPACK_ALPHA *packet );
static void *DrawWindow( DG_DMAPACK_WINDOW *packet );
static void *DrawViewmapping( void *packet_addr );
static void *DrawSetZ( DG_DMAPACK_PARAM *packet );
static void *DrawModeEnable( DG_DMAPACK_PARAM *packet );
static void *DrawModeDisable( DG_DMAPACK_PARAM *packet );
static void *DrawUseFrameTex( DG_DMAPACK_PARAM *packet );
static void *DrawBackupFrame( DG_DMAPACK_PARAM *packet );
static void *DrawPacketCallback( DG_DMAPACK_CALLBACK *packet );
static void *DrawNext( DG_DMAPACK_NEXT *packet );
static void *DrawEnd( void *packet );
static void *DrawPoint( DG_DMAPACK_POINT *packet );
static void *DrawLine( DG_DMAPACK_LINE *packet );
static void *DrawLine_F( DG_DMAPACK_LINE_F *packet );
static void *DrawTriangle( DG_DMAPACK_TRIANGLE *packet );
static void *DrawQuad( DG_DMAPACK_QUAD *packet );
static void *DrawQuad_F( DG_DMAPACK_QUAD_F *packet );
static void *DrawBox( DG_DMAPACK_BOX *packet );
static void *DrawBox_F( DG_DMAPACK_BOX_F *packet );
static void *DrawSprt( DG_DMAPACK_SPRT *packet );
static void *DrawRSprt( DG_DMAPACK_RSPRT *packet );
static void *DrawSprt_F( DG_DMAPACK_SPRT_F *packet );
static void *DrawRSprt_F( DG_DMAPACK_RSPRT_F *packet );
static void *DrawLineStrip( DG_DMAPACK_LINESTRIP *packet );
static void *DrawLineStrip_F( DG_DMAPACK_LINESTRIP_F *packet );
static void *DrawTriangleStrip( DG_DMAPACK_TRIANGLESTRIP *packet );

/* ---------------------------------------------------------------- */
/*
	頂点シェーダー関連
*/

#if 0//BP_TODO
DG_VERTEXSHADER	DG_DmapackVertexShader[2] ;
DG_VERTEXFORMAT	DG_DmapackVertexFormat[1] ;
DG_PIXELSHADER	DG_DmapackPixelShader[4] ;
extern unsigned char VERTEX_SHADER_dmapk_2d[];
extern unsigned char VERTEX_SHADER_dmapk_2d4[];
extern unsigned char	PIXEL_SHADER_mtex_gray[] ;
#endif

void DG_InitDmapackVertexShader(void)
{
#if 0 //BP_RENDER
	static DWORD dwObjDecl_00[] = {
		/* テクスチャ無し */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT2),		/* position */
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_PBYTE4),		/* diffuse *//* ＰＳ２と同じになるように（ＸＢＯＸ拡張） */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};

	DG_MakeVertexFormat( &DG_DmapackVertexFormat[0], dwObjDecl_00 );
	DG_MakeVertexShader( &DG_DmapackVertexShader[0], VERTEX_SHADER_dmapk_2d, dwObjDecl_00 );
	DG_MakeVertexShader( &DG_DmapackVertexShader[1], VERTEX_SHADER_dmapk_2d4, dwObjDecl_00 );

	DG_MakePixelShader( &DG_DmapackPixelShader[0], PIXEL_SHADER_mtex_gray );
#endif
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
#if BP_DMAPACK_DEBUG_INFO
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
      dmapack->BP_LabelMask = kRL_DmaPack;
	}
	return ( dmapack );
}
DG_DMAPACK* DG_MakeDmapack_D( int flag, int phase, char *file )
{
	DG_DMAPACK	*dmapack ;
	dmapack = DG_MakeDmapack( flag, phase );
	if ( dmapack != NULL ){
		dmapack->fname = file ;
      dmapack->BP_LabelMask = kRL_DmaPack;
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
         //printf("DG_QueueDmapack: queue buffer over!!\n");
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
int BP_last_normal_dmapack_prio = -1;

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_DMAPACK	*dmapack, **que ;
	int			i, size, exec_flag = 0 ;
	int			invisible_flag, chanl_flag ;
	DG_DMATAG	*tag ;
   int BP_bHasUpdatedViewportInfo = 0;

	MARK( "dmapack.c" );
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;

   BP_RB_PushRegionMarker(kProfileColor_DmaPack, "Dmapack");
   BP_Debug_PushCPUMarker( "Dmapack" );

#if BP_VITA
   // reset the priority, in case there's only one dmapack in the after chain
   if ((cp->chanl_num != 4) && (status & DG_DMAPACK_PHASE_AFTER))
   {
      BP_last_normal_dmapack_prio = 0;
   }
#endif

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

#if 0 //BP_XBOX
		if ( exec_flag == 0 ){
			/* スタティックプッシュバッファへの記録開始 */
			DG_OpenDmaTask();
		}
		exec_flag = 1 ;
#endif

      {
		   if ( dmapack->BP_renderCallback != NULL )
         {
#if BP_DMAPACK_DEBUG_INFO
            if( dmapack->fname )
               BP_RB_AddLabel(dmapack->fname, dmapack->BP_LabelMask);
#endif
			   dmapack->BP_renderCallback(dmapack->BP_callbackParam);

            if( !BP_bHasUpdatedViewportInfo )
            {
               BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
               BP_bHasUpdatedViewportInfo = 1;
            }
		   }
         else
         {
            char * pDmaPackData;

            ASSERT(BP_CurrentDmaPackPtr == NULL);
            
            pDmaPackData = BP_CurrentDmaPackPtr = BP_RB_GetCurrentPtr();

            //printf("%p %s\n", dmapack, dmapack->fname );
            if ( gAS_UsedBufferSceneNOP )
            {
   		      /* ＤＭＡパケットの変わりにコールバックでエミュレーションする */
   		      if ( dmapack->buildAutoPacketCallback != NULL ){
   			      (*dmapack->buildAutoPacketCallback)( dmapack->buildAutoPacketCallbackParam );
   		      }         
               /* 自動処理簡易２Ｄプリミティブ処理 */
               if ( dmapack->autopacket != NULL )
               {
   			      DG_ExecAuto2DPrim( dmapack->autopacket );
               }
            }
            else
            {
               // copy the autopacket results to the dmapack stream
               if (dmapack->autopacket)
               {
                  memcpy(BP_CurrentDmaPackPtr, dmapack->autopacket, dmapack->autopacketSize);
                  BP_CurrentDmaPackPtr += dmapack->autopacketSize;
               }
            }
            

            // Add command if any data was written.
            if( BP_CurrentDmaPackPtr > pDmaPackData )
            {
               if( !BP_bHasUpdatedViewportInfo )
               {
                  BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
                  BP_bHasUpdatedViewportInfo = 1;
               }

               BP_RB_SetCurrentPtr(BP_CurrentDmaPackPtr);

#if BP_DMAPACK_DEBUG_INFO
               if( dmapack->fname )
                  BP_RB_AddLabel(dmapack->fname, dmapack->BP_LabelMask);
#endif
               {
#if BP_VITA
                  // This forces the scene up to highres around the time DoF or Glare would be rendered
                  // Some of the UI is done via dmapacks in the main scene, rather than the menu scene,
                  // and without this would render in the lowres translucent buffer
                  // We only do this for channel 0 in MGS2, triggering it in channel 1
                  // does strange things in w16a when you go into the bathroom
                  if ((cp->chanl_num == 0) &&
                      (status & DG_PLUGIN_PHASE_AFTER))
                  {
                     if ((dmapack->priority > 128) && (BP_last_normal_dmapack_prio <= 128))
                        BP_RB_AddCommand(kCmd_PostFx_Offscreen, NULL);
                     BP_last_normal_dmapack_prio = dmapack->priority;
                  }
#endif                      
                  SBP_RenderDmaPack* pPacket = (SBP_RenderDmaPack*)BP_RB_Alloc(sizeof(SBP_RenderDmaPack));
                  pPacket->dmapack = dmapack;
                  pPacket->data = pDmaPackData;
                  BP_RB_AddCommand(kCmd_DmaPack_AutoPacket, (char*)pPacket);
               }
            }
            else
            {
               BP_RB_AddLabel("Skipped DMA Pack", dmapack->BP_LabelMask);
               if( dmapack->fname )
                  BP_RB_AddLabel(dmapack->fname, dmapack->BP_LabelMask);
            }

            BP_CurrentDmaPackPtr = NULL;
         }

      }
		/* 本来のＤＭＡ処理はＸＢＯＸではなにもしない */

#if 0 //BP_XBOX
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

#if 0 //BP_XBOX
	if ( exec_flag ){
		/* スタティックプッシュバッファへの記録終了 */
		DG_CloseDmaTask();
	}
#endif

   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();
}

/* ---------------------------------------------------------------- */
void DG_SetDmapackCallback( DG_DMAPACK *dmapack, void *callback, void *param )
{
	dmapack->buildAutoPacketCallback = callback ;
	dmapack->buildAutoPacketCallbackParam = param ;
}



/* ---------------------------------------------------------------- */
/* ＸＢＯＸ専用プリミティブ描画補助 */
#if 0 //BP_RENDER

#ifdef KP_XBOX //BP
static DWORD *DG_DrawVerticesDirectAddr ;
#endif
void *DG_DrawVerticesDirect( u_int type, int v_size, int count )
{
	void	*ret_addr ;
	int		dword_size ;

	dword_size = ( v_size * count + 3 ) / 4 ;
	/* プッシュバッファアドレス取得 */
	IDirect3DDevice9_BeginPush( g_pd3dDevice, dword_size + 5, &DG_DrawVerticesDirectAddr );
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
}
void DG_DrawVerticesDirectEnd( void )
{
	*DG_DrawVerticesDirectAddr++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
	*DG_DrawVerticesDirectAddr++ = 0 ;
	/* プッシュバッファアクセスの終了 */
	IDirect3DDevice9_EndPush( g_pd3dDevice, DG_DrawVerticesDirectAddr );
}
#endif

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

#if 0//BP_TODO
static DG_VERTEX_DMAPACK2D		*Vertex;
#endif
static DG_TEX				*last_tex = NULL ;
static FVECTOR tex_param_normal[2] = {
	{1/16384.0f,1/16384.0f,1,1},	/* テクスチャスケール */
	{0,0,0,0}					/* テクスチャオフセット */
};
static FVECTOR tex_param_lintex[2] = {
	{DRAW_WIDTH/16384.0f,DRAW_HEIGHT/16384.0f,1,1},	/* テクスチャスケール */
	{(DISPLAY_WIDTH-DRAW_WIDTH)/2.0f,(DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f,0,0}		/* テクスチャオフセット */
};
/* ---------------------------------------------------------------- */
#if 0//BP_TODO
extern D3DVIEWPORT9 DG_CurrentViewport;
static float	draw_z = 1.0f ;
static FVECTOR	screen_param[2] = {
	{0,-DRAW_HEIGHT,1.0f,0},
	{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT,1,1}
};
static D3DVIEWPORT9 dmapack_viewport ;

static void SetViewport( int x, int y, int w, int h )
{

	ZeroMemory( &dmapack_viewport, sizeof(D3DVIEWPORT9) );
	dmapack_viewport.X = ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + x ; /* ターゲットサーフェスの左上座標 */
	dmapack_viewport.Y = ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + y ;	
	dmapack_viewport.Width = w ;
	dmapack_viewport.Height = h ;
	dmapack_viewport.MinZ = 0.0f; /* クリップボリュームの最小値 */
	dmapack_viewport.MaxZ = 1.0f; /* クリップボリュームの最大値 */
	IDirect3DDevice9_SetViewport( g_pd3dDevice, &dmapack_viewport );
	//printf("dmapack viewport: %d %d %d %d\n",
	//	   dmapack_viewport.X, dmapack_viewport.Y, dmapack_viewport.Width, dmapack_viewport.Height );

	/* 頂点シェーダー用座標変換パラメータを修正する */
	screen_param[0].vx = (float)0 ;
	screen_param[0].vy = (float)-h ;
	screen_param[0].vz = draw_z ;
	screen_param[1].vx = 2.0f / w ;
	screen_param[1].vy = -2.0f / h ;
	DG_SetVertexShaderConstant(0, screen_param, 2);
}
#endif

/* ---------------------------------------------------------------- */
/* 自動処理簡易２Ｄプリミティブの描画処理 */
void DG_ExecAuto2DPrim( void *addr )
{
   DG_DMAPACK_TAG		*header = addr ;

#if 0 //BP_XBOX
   static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};

	DG_SetPixelShader( NULL );

	/* 描画モードの初期化 */
	DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
	DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	/* 頂点シェーダー関連初期化 */
	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_normal, 2);
	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_DmapackVertexShader[0] );
	DG_LoadVertexShader( &DG_DmapackVertexShader[1] );
	DG_SetVertexStream( NULL );	/* 頂点ストリームは使用しない */
	DG_SelectVertexShader( &DG_DmapackVertexShader[0], &DG_DmapackVertexFormat[0] );

	/* ビューポートをそのチャンネルの設定に合わせる */
	dmapack_viewport = DG_CurrentViewport ;
	draw_z = 1.0f ;	/* 初期描画Ｚ値の設定 */

   SetViewport( DG_CurrentViewport.X - ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2,
				DG_CurrentViewport.Y - ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2,
				DG_CurrentViewport.Width, DG_CurrentViewport.Height );
#endif

	while ( header != NULL )
   {
		switch ( header->cmd ){	/* 最終的には関数テーブルによるジャンプに置き換わる予定 */
		  case DG_DMAPACK_CMD_POINT:
			header = DrawPoint( header );
			break ;
		  case DG_DMAPACK_CMD_LINE:
			header = DrawLine( header );
			break ;
        case DG_DMAPACK_CMD_LINE_F:
         header = DrawLine_F( header );
         break ;
		  case DG_DMAPACK_CMD_TRIANGLE:
			header = DrawTriangle( header );
			break ;
		  case DG_DMAPACK_CMD_QUAD:
			header = DrawQuad( header );
			break ;
        case DG_DMAPACK_CMD_QUAD_F:
           header = DrawQuad_F( header );
           break ;
		  case DG_DMAPACK_CMD_BOX:
			header = DrawBox( header );
			break ;
        case DG_DMAPACK_CMD_BOX_F:
           header = DrawBox_F( header );
           break;
		  case DG_DMAPACK_CMD_SPRT:
			header = DrawSprt( header );
			break ;
		  case DG_DMAPACK_CMD_RSPRT:
			header = DrawRSprt( header );
			break ;
        case DG_DMAPACK_CMD_SPRT_F:
           header = DrawSprt_F( header );
           break ;
        case DG_DMAPACK_CMD_RSPRT_F:
           header = DrawRSprt_F( header );
           break ;

		  case DG_DMAPACK_CMD_LINESTRIP:
			header = DrawLineStrip( header );
			break ;
        case DG_DMAPACK_CMD_LINESTRIP_F:
         header = DrawLineStrip_F( header );
         break ;
		  case DG_DMAPACK_CMD_TRIANGLESTRIP:
			header = DrawTriangleStrip( header );
			break ;

		  case DG_DMAPACK_CMD_MODE:
			header = DrawMode( header );
			break ;
		  case DG_DMAPACK_CMD_TEX:
			header = DrawTex( header );
			break ;
        case DG_DMAPACK_CMD_TEXDYN:
         header = DrawTexDyn( header );
         break ;
        case DG_DMAPACK_CMD_TEXDYN01:
         header = DrawTexDyn01( header );
         break ;
		  case DG_DMAPACK_CMD_TEXLIN:
			header = DrawTexLin( header );
			break ;
		  case DG_DMAPACK_CMD_ALPHA:
			header = DrawAlpha( header );
			break ;
		  case DG_DMAPACK_CMD_WINDOW:
			header = DrawWindow( header );
			break ;

        case DG_DMAPACK_CMD_VIEWMAPPING:
         header = DrawViewmapping(header);
         break;

		  case DG_DMAPACK_CMD_SETZ:
			header = DrawSetZ( header );
			break ;
		  case DG_DMAPACK_CMD_ENABLE:
			header = DrawModeEnable( header );
			break ;
		  case DG_DMAPACK_CMD_DISABLE:
			header = DrawModeDisable( header );
			break ;

		  case DG_DMAPACK_CMD_USEFRAMETEX:
			header = DrawUseFrameTex( header );
			break ;
		  case DG_DMAPACK_CMD_BACKUPFRAME:
			header = DrawBackupFrame( header );
			break ;

		  case DG_DMAPACK_CMD_CALLBACK:
			header = DrawPacketCallback( header );
			break ;

		  case DG_DMAPACK_CMD_NEXT:
			header = DrawNext( header );
			break ;

        case DG_DMAPACK_CMD_END:
        header = DrawEnd(header);
        break;

		  default:
			printf("dmapack : packet error (%02x)\n", header->cmd );
			header = NULL ;
         BP_BREAK;
			break ;
		}
	}

#if 0 //BP_XBOX
	/* 他に影響しないように初期化を行う */
	DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
   DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

	/* ビューポートを元に戻す */
	IDirect3DDevice9_SetViewport( g_pd3dDevice, &DG_CurrentViewport );
#endif
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

void *DG_SetDmapackTextureDynamic( void *addr, DG_TEX *tex, unsigned int bp_texture )
{
   DG_DMAPACK_TEXTURE_DYNAMIC *packet = addr ;
   packet->header.cmd = DG_DMAPACK_CMD_TEXDYN;
   packet->tex = tex ;
   packet->bp_tex = bp_texture;
   return ( (void*)&packet[1] );
}

void *DG_SetDmapackTextureDynamic01( void *addr, DG_TEX *tex, unsigned int bp_texture )
{
   DG_DMAPACK_TEXTURE_DYNAMIC *packet = addr ;
   packet->header.cmd = DG_DMAPACK_CMD_TEXDYN01;
   packet->tex = tex ;
   packet->bp_tex = bp_texture;
   return ( (void*)&packet[1] );
}

void *DG_SetDmapackTexLin( void *addr, DG_TEX_LIN *tex )
{
	DG_DMAPACK_TEX *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_TEXLIN;
	packet->tex = (DG_TEX*)tex ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackAlpha( void *addr, u_long64 alpha )
{
	DG_DMAPACK_ALPHA *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_ALPHA ;
#if 0
	//packet->alpha = alpha & SCE_GS_ALPHA_MASK ;
	packet->alpha = alpha ;
#else
   packet->alpha0 = alpha & 0xffffffff ;
   packet->alpha1 = ( alpha >> 32 ) & 0xffffffff ;
#endif
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackWindow( void *addr, int x, int y, int w, int h )
{
	DG_DMAPACK_WINDOW *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_WINDOW ;
	packet->x = x;
	packet->y = y;
	packet->w = w;
	packet->h = h;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackViewMapping( void *buffer, float x0, float y0, float x1, float y1 )
{
   DG_DMAPACK_VIEWMAPPING		*packet = buffer ;
   packet->header.cmd = DG_DMAPACK_CMD_VIEWMAPPING ;
   packet->x0 = x0;
   packet->y0 = y0;
   packet->x1 = x1;
   packet->y1 = y1;
   return ( &packet[1] );
}

void *DG_SetDmapackSetZ( void *addr, float z )
{
	DG_DMAPACK_PARAM *packet = addr ;
	FVECTOR		vec ;
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
	return ( (void*)&packet[1] );
}

void *DG_DmapackSetCallback( void *buffer, void (*callback)(void*), int dataSize )
{
   DG_DMAPACK_CALLBACK* packet = buffer;
   packet->header.cmd = DG_DMAPACK_CMD_CALLBACK;
   packet->callback = callback;
   packet->dataSize = dataSize;

   return ((char*)buffer) + sizeof(DG_DMAPACK_CALLBACK) + dataSize;
}

void *DG_SetDmapackNext( void *addr, void *next )
{
	DG_DMAPACK_NEXT *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_NEXT ;
	packet->next = next ;
	return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
void *DG_SetDmapackPoint( void *addr, float x0, float y0, DG_DMAPACK_COLOR rgba )
{
	DG_DMAPACK_POINT *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_POINT ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba ) ;
	packet->x = x0 ;
	packet->y = y0 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackLine( void *addr, float x0, float y0, DG_DMAPACK_COLOR rgba0, float x1, float y1, DG_DMAPACK_COLOR rgba1 )
{
	DG_DMAPACK_LINE *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_LINE ;
	packet->rgba0 = DMAPACK_COLOR_TO_INT( rgba0 ) ;
	packet->rgba1 = DMAPACK_COLOR_TO_INT( rgba1 ) ;
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackLine_F( void *addr, float x0, float y0, DG_DMAPACK_COLOR rgba0, float x1, float y1, DG_DMAPACK_COLOR rgba1 )
{
   DG_DMAPACK_LINE_F *packet = addr ;
   packet->header.cmd = DG_DMAPACK_CMD_LINE_F ;
   packet->rgba0 = DMAPACK_COLOR_TO_INT( rgba0 ) ;
   packet->rgba1 = DMAPACK_COLOR_TO_INT( rgba1 ) ;
   packet->x0 = x0 ;
   packet->y0 = y0 ;
   packet->x1 = x1 ;
   packet->y1 = y1 ;
   return ( (void*)&packet[1] );
}

void *DG_SetDmapackTriangle( void *addr,
							float x0, float y0, DG_DMAPACK_COLOR rgba0,
							float x1, float y1, DG_DMAPACK_COLOR rgba1,
							float x2, float y2, DG_DMAPACK_COLOR rgba2 )
{
	DG_DMAPACK_TRIANGLE *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_TRIANGLE ;
	packet->rgba0 = DMAPACK_COLOR_TO_INT( rgba0 ) ;
	packet->rgba1 = DMAPACK_COLOR_TO_INT( rgba1 ) ;
	packet->rgba2 = DMAPACK_COLOR_TO_INT( rgba2 ) ;
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	packet->x2 = x2 ;
	packet->y2 = y2 ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackQuad( void *addr,
						float x0, float y0, DG_DMAPACK_COLOR rgba0,
						float x1, float y1, DG_DMAPACK_COLOR rgba1,
						float x2, float y2, DG_DMAPACK_COLOR rgba2,
						float x3, float y3, DG_DMAPACK_COLOR rgba3 )
{
	DG_DMAPACK_QUAD *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_QUAD ;
	packet->rgba0 = DMAPACK_COLOR_TO_INT( rgba0 ) ;
	packet->rgba1 = DMAPACK_COLOR_TO_INT( rgba1 ) ;
	packet->rgba2 = DMAPACK_COLOR_TO_INT( rgba2 ) ;
	packet->rgba3 = DMAPACK_COLOR_TO_INT( rgba3 ) ;
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	packet->x2 = x2 ;
	packet->y2 = y2 ;
	packet->x3 = x3 ;
	packet->y3 = y3 ;
	return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
void *DG_SetDmapackQuad_F( void *addr,
                        float x0, float y0, DG_DMAPACK_COLOR rgba0,
                        float x1, float y1, DG_DMAPACK_COLOR rgba1,
                        float x2, float y2, DG_DMAPACK_COLOR rgba2,
                        float x3, float y3, DG_DMAPACK_COLOR rgba3 )
{
   DG_DMAPACK_QUAD_F *packet = addr ;
   packet->header.cmd = DG_DMAPACK_CMD_QUAD_F ;
   packet->rgba0 = DMAPACK_COLOR_TO_INT( rgba0 ) ;
   packet->rgba1 = DMAPACK_COLOR_TO_INT( rgba1 ) ;
   packet->rgba2 = DMAPACK_COLOR_TO_INT( rgba2 ) ;
   packet->rgba3 = DMAPACK_COLOR_TO_INT( rgba3 ) ;
   packet->x0 = x0 ;
   packet->y0 = y0 ;
   packet->x1 = x1 ;
   packet->y1 = y1 ;
   packet->x2 = x2 ;
   packet->y2 = y2 ;
   packet->x3 = x3 ;
   packet->y3 = y3 ;
   return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
void *DG_SetDmapackBox( void *addr, float x0, float y0, float x1, float y1, DG_DMAPACK_COLOR rgba )
{
	DG_DMAPACK_BOX *packet = addr ;
	packet->header.cmd = DG_DMAPACK_CMD_BOX ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
void *DG_SetDmapackBox_F( void *addr, float x0, float y0, float x1, float y1, DG_DMAPACK_COLOR rgba )
{
   DG_DMAPACK_BOX_F *packet = addr;
   packet->header.cmd = DG_DMAPACK_CMD_BOX_F;
   packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
   packet->x0 = x0;
   packet->y0 = y0;
   packet->x1 = x1;
   packet->y1 = y1;
   return (void *) &packet[1];
}

void *DG_SetDmapackSprt( void *addr,
						float x0, float y0, float u0, float v0,
						float x1, float y1, float u1, float v1, DG_DMAPACK_COLOR rgba )
{
	DG_DMAPACK_SPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_SPRT ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
#if 0
	packet->u0 = ( u0 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v0 = ( v0 * tex->v_scale + tex->v_offset ) * 16384.0f ;
	packet->u1 = ( u1 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v1 = ( v1 * tex->v_scale + tex->v_offset ) * 16384.0f ;
#else
	packet->u0 = ( u0 ) * 16384.0f ;
	packet->v0 = ( v0 ) * 16384.0f ;
	packet->u1 = ( u1 ) * 16384.0f ;
	packet->v1 = ( v1 ) * 16384.0f ;
#endif
 	return ( (void*)&packet[1] );
}

void *DG_SetDmapackSprt_F( void *addr,
                        float x0, float y0, float u0, float v0,
                        float x1, float y1, float u1, float v1, DG_DMAPACK_COLOR rgba, unsigned int sprFlags )
{
   DG_DMAPACK_SPRT_F *packet = addr ;

   packet->header.cmd = DG_DMAPACK_CMD_SPRT_F ;
   packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
   packet->x0 = x0 ;
   packet->y0 = y0 ;
   packet->x1 = x1 ;
   packet->y1 = y1 ;
   packet->u0 = ( u0 ) * 16384.0f ;
   packet->v0 = ( v0 ) * 16384.0f ;
   packet->u1 = ( u1 ) * 16384.0f ;
   packet->v1 = ( v1 ) * 16384.0f ;
   packet->flags = sprFlags;
   return ( (void*)&packet[1] );
}

void *DG_SetDmapackSprtTex( void *addr,
						float x0, float y0, float u0, float v0,
						float x1, float y1, float u1, float v1, DG_DMAPACK_COLOR rgba, DG_TEX *tex )
{
	DG_DMAPACK_SPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_SPRT ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	packet->u0 = ( u0 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v0 = ( v0 * tex->v_scale + tex->v_offset ) * 16384.0f ;
	packet->u1 = ( u1 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v1 = ( v1 * tex->v_scale + tex->v_offset ) * 16384.0f ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackRSprt( void *addr,
						 float x0, float y0, float x1, float y1,
						 float x2, float y2, float x3, float y3,
						 float u0, float v0, float u1, float v1,
						 DG_DMAPACK_COLOR rgba )
{
	DG_DMAPACK_RSPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_RSPRT ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	packet->x2 = x2 ;
	packet->y2 = y2 ;
	packet->x3 = x3 ;
	packet->y3 = y3 ;
	packet->u0 = ( u0 ) * 16384.0f ;
	packet->v0 = ( v0 ) * 16384.0f ;
	packet->u1 = ( u1 ) * 16384.0f ;
	packet->v1 = ( v1 ) * 16384.0f ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackRSprt_F( void *addr,
                         float x0, float y0, float x1, float y1,
                         float x2, float y2, float x3, float y3,
                         float u0, float v0, float u1, float v1,
                         DG_DMAPACK_COLOR rgba )
{
   DG_DMAPACK_RSPRT_F *packet = addr ;

   packet->header.cmd = DG_DMAPACK_CMD_RSPRT_F ;
   packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
   packet->x0 = x0 ;
   packet->y0 = y0 ;
   packet->x1 = x1 ;
   packet->y1 = y1 ;
   packet->x2 = x2 ;
   packet->y2 = y2 ;
   packet->x3 = x3 ;
   packet->y3 = y3 ;
   packet->u0 = ( u0 ) * 16384.0f ;
   packet->v0 = ( v0 ) * 16384.0f ;
   packet->u1 = ( u1 ) * 16384.0f ;
   packet->v1 = ( v1 ) * 16384.0f ;
   return ( (void*)&packet[1] );
}

void *DG_SetDmapackRSprtTex( void *addr,
						 float x0, float y0, float x1, float y1,
						 float x2, float y2, float x3, float y3,
						 float u0, float v0, float u1, float v1,
						 DG_DMAPACK_COLOR rgba, DG_TEX *tex )
{
	DG_DMAPACK_RSPRT *packet = addr ;

	packet->header.cmd = DG_DMAPACK_CMD_RSPRT ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
	packet->x0 = x0 ;
	packet->y0 = y0 ;
	packet->x1 = x1 ;
	packet->y1 = y1 ;
	packet->x2 = x2 ;
	packet->y2 = y2 ;
	packet->x3 = x3 ;
	packet->y3 = y3 ;
	packet->u0 = ( u0 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v0 = ( v0 * tex->v_scale + tex->v_offset ) * 16384.0f ;
	packet->u1 = ( u1 * tex->u_scale + tex->u_offset ) * 16384.0f ;
	packet->v1 = ( v1 * tex->v_scale + tex->v_offset ) * 16384.0f ;
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

void *DG_SetDmapackLineStrip_F( void *addr, int num )
{
   DG_DMAPACK_LINESTRIP *packet = addr ;
   packet->header.cmd = DG_DMAPACK_CMD_LINESTRIP_F ;
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

void *DG_SetDmapackVertex( void *addr, float x, float y, float u, float v, DG_DMAPACK_COLOR rgba )
{
	DG_DMAPACK_VERTEX *packet = addr ;
	packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
	packet->x = x ;
	packet->y = y ;
	packet->u = u * 16384.0f ;
	packet->v = v * 16384.0f ;
	return ( (void*)&packet[1] );
}

void *DG_SetDmapackVertex_F( void *addr, float x, float y, float u, float v, DG_DMAPACK_COLOR rgba )
{
   DG_DMAPACK_VERTEX_F *packet = addr ;
   packet->rgba = DMAPACK_COLOR_TO_INT( rgba );
   packet->x = x ;
   packet->y = y ;
   packet->u = u ;
   packet->v = v ;
   return ( (void*)&packet[1] );
}

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static void *DrawMode( DG_DMAPACK_PARAM *packet )
{
#if 0 //BP_RENDER
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
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_PARAM));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_PARAM);
#endif
   return ( (void*)&packet[1] );
}

static void *DrawModeEnable( DG_DMAPACK_PARAM *packet )
{
#if 0 //BP_RENDER

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
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_PARAM));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_PARAM);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawModeDisable( DG_DMAPACK_PARAM *packet )
{
#if 0 //BP_RENDER

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
		DG_SetRenderState( D3DRS_COLORWRITEENABLE , D3DCOLORWRITEENABLE_ALL );
	}
	/* テクスチャのアルファ無視（頂点アルファのみ参照） */
	if ( packet->param & DG_DMAPACK_MODE_NO_TEXALPHA ){
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	}
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_PARAM));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_PARAM);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawTex( DG_DMAPACK_TEX *packet )
{
#if 0 //BP_XBOX
	/* テクスチャ設定 */
	last_tex = packet->tex ;
	if ( last_tex != NULL ){
		//DG_SetTexture( 0, &last_tex->tex_trans );	/* 頂点シェーダーのCV_TEX0_SCALEなどを破壊されると困るので */
		DG_SetTextureDirect( 0, last_tex->tex_trans.ptex );
		DG_SetPalette( 0, last_tex->tex_trans.ppal );
		DG_SetAlphaMode( last_tex->tex_trans.alpha.data );
	} else {
		DG_SetTextureDirect( 0, NULL );
	}
#else
   DG_DMAPACK_TEXTURE_BP* pDestPacket = (DG_DMAPACK_TEXTURE_BP*)BP_CurrentDmaPackPtr;
   pDestPacket->header = packet->header;
   if( packet->tex )
   {
      pDestPacket->hasvalidTex = 1;
      pDestPacket->tex = *packet->tex;
   }
   else
   {
      pDestPacket->hasvalidTex = 0;
   }
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TEXTURE_BP);
#endif

	return ( (void*)&packet[1] );
}

static void *DrawTexDyn( DG_DMAPACK_TEXTURE_DYNAMIC *packet )
{
   DG_DMAPACK_TEXTURE_DYNAMIC_BP* pDestPacket = (DG_DMAPACK_TEXTURE_DYNAMIC_BP*)BP_CurrentDmaPackPtr;
   pDestPacket->header = packet->header;
   pDestPacket->tex = *packet->tex;
   pDestPacket->bp_tex = packet->bp_tex;
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TEXTURE_DYNAMIC_BP);

   return ( (void*)&packet[1] );
}

static void *DrawTexDyn01( DG_DMAPACK_TEXTURE_DYNAMIC *packet )
{
   DG_DMAPACK_TEXTURE_DYNAMIC01_BP* pDestPacket = (DG_DMAPACK_TEXTURE_DYNAMIC01_BP*)BP_CurrentDmaPackPtr;
   pDestPacket->header = packet->header;
   pDestPacket->bp_tex = packet->bp_tex;
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TEXTURE_DYNAMIC01_BP);

   return ( (void*)&packet[1] );
}

static void *DrawTexLin( DG_DMAPACK_TEX *packet )
{
#if 0 //BP_XBOX
	FVECTOR		tex_param[2] = {
		{1,1,1,1},
		{0,0,0,0}
	};
	DG_TEX_LIN	*tex ;
	/* テクスチャ設定 */
	tex = (DG_TEX_LIN*)packet->tex ;
	//last_tex = packet->tex ;
	DG_SetLinerTexture( tex );
	/* 頂点シェーダーのパラメータも変更する */
	tex_param[0].vx = (float)tex->width / 16384.0f ;
	tex_param[0].vy = (float)tex->height / 16384.0f ;
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param, 2);
	/*  */
	//DG_SetAlphaMode( last_tex->tex_trans.alpha.data );
#else
   DG_DMAPACK_TEXTURE_LINEAR_BP* pDestPacket = (DG_DMAPACK_TEXTURE_LINEAR_BP*)BP_CurrentDmaPackPtr;
   pDestPacket->header = packet->header;
   if( packet->tex )
   {
      pDestPacket->hasvalidTex = 1;
      pDestPacket->tex = *(DG_TEX_LIN*)packet->tex;
   }
   else
   {
      pDestPacket->hasvalidTex = 0;
   }
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TEXTURE_LINEAR_BP);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawAlpha( DG_DMAPACK_ALPHA *packet )
{
#if 0 //BP_XBOX
#if 0
	DG_SetAlphaMode( packet->alpha );
#else
	DG_SetAlphaMode( *(u_long64*)&packet->alpha0 );
#endif
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_ALPHA));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_ALPHA);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawWindow( DG_DMAPACK_WINDOW *packet )
{
#if 0 //BP_XBOX
	SetViewport( packet->x, packet->y, packet->w, packet->h );
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_WINDOW));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_WINDOW);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawViewmapping( void *packet_addr )
{
   DG_DMAPACK_VIEWMAPPING	*packet = packet_addr ;
   
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_VIEWMAPPING));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_VIEWMAPPING);
   
   return ( &packet[1] );
}

static void *DrawSetZ( DG_DMAPACK_PARAM *packet )
{
#if 0 //BP_XBOX
	draw_z = *(float*)&packet->param ;
	/* 頂点シェーダー内の定数を変更することでそれ以降の２Ｄの描画Ｚを変更する */
	screen_param[0].vz = draw_z ;
	DG_SetVertexShaderConstant(0, screen_param, 2);
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_PARAM));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_PARAM);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawUseFrameTex( DG_DMAPACK_PARAM *packet )
{
#if 0 //BP_XBOX
	int		page ;
	extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;		/* バックバッファテクスチャ */
	if ( packet->param != -1 ){
		switch ( packet->param ){
		  case 0:
			page = DG_CurrentBackBuffer ;
			break ;
		  case 1:
			page = 1 - DG_CurrentBackBuffer ;
			break ;
		  default:
			page = 2 ;
			break ;
		}
		last_tex = &DG_SystemTexture[ page ] ;
		DG_SetTextureDirect( 0, last_tex->tex_trans.ptex );
		//DG_SetTextureDirect( 0 , DG_BackBufferTexture[page] );
		DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_lintex, 2);
	} else {
		DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_normal, 2);
	}
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_PARAM));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_PARAM);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawBackupFrame( DG_DMAPACK_PARAM *packet )
{
#if 0 //BP_XBOX
	int		page, mono_flag = 0, col = 0xffffffff ;
	extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;		/* バックバッファテクスチャ */
	extern LPDIRECT3DSURFACE8	DG_BackBufferSurface[3] ;		/* バックバッファサーフェス */
	extern LPDIRECT3DSURFACE8	DG_DepthBufferSurface ;			/* Ｚバッファサーフェス */
	extern D3DVIEWPORT9			DG_CurrentViewport ;
	static FVECTOR full_screen_param[2] = {
		{0,-DRAW_HEIGHT,1.0f,0},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT,1,1}
	};
	D3DVIEWPORT9		d3dview ;

	switch ( packet->param & 1 ){
	  case 0:
		page = DG_CurrentBackBuffer ;
		break ;
	  case 1:
		page = 1 - DG_CurrentBackBuffer ;
		break ;
	}
	if ( ( packet->param & 0xfffffffc ) < 0x100 ){
		/* 指定したバッファをテクスチャとして退避バッファへレンダリングする */
		DG_SetRenderTarget( DG_BackBufferSurface[ 2 ], NULL );
		if ( packet->param & 0x04 ){
			mono_flag = 1 ;
		}
	} else {
		/* パラメータをサーフェスへのポインタと見なして設定 */
		void	*addr = (void*)( packet->param & 0xfffffffc );
		DG_SetRenderTarget( addr, NULL );
	}
	ZeroMemory( &d3dview, sizeof(D3DVIEWPORT9) );
	d3dview.X = ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0 ; /* ターゲットサーフェスの左上座標 */
	d3dview.Y = ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0 ;	
	d3dview.Width = DRAW_WIDTH ;
	d3dview.Height = DRAW_HEIGHT ;
	d3dview.MinZ = 0.0f; /* クリップボリュームの最小値 */
	d3dview.MaxZ = 1.0f; /* クリップボリュームの最大値 */
	IDirect3DDevice9_SetViewport( g_pd3dDevice, &d3dview );
	//IDirect3DDevice9_SetViewport( g_pd3dDevice, &DG_CurrentViewport );
	DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );
	DG_SetTextureDirect( 0, DG_SystemTexture[ page ].tex_trans.ptex );
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_lintex, 2);
	DG_SetVertexShaderConstant(0, full_screen_param, 2);
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 255 ) );
	DG_SetAlphaMode( 0 );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	if ( mono_flag ){
		DG_SetPixelShader( &DG_DmapackPixelShader[0] );
		col = 0x80808080 ;
	}

#if 0
	{
		Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 4 );
		Vertex[0].x = 0 ;
		Vertex[0].y = 0 ;
		Vertex[0].rgba = col ;
		Vertex[0].u = DG_FRAME_U(0.5f) * 16384.0f ;
		Vertex[0].v = DG_FRAME_V(0.5f) * 16384.0f ;
		Vertex[1].x = 640 ;
		Vertex[1].y = 0 ;
		Vertex[1].rgba = col ;
		Vertex[1].u = DG_FRAME_U(640.0f-0.5f) * 16384.0f ;
		Vertex[1].v = DG_FRAME_V(0.5f) * 16384.0f ;
		Vertex[2].x = 0 ;
		Vertex[2].y = 448 ;
		Vertex[2].rgba = col ;
		Vertex[2].u = DG_FRAME_U(0.5f) * 16384.0f ;
		Vertex[2].v = DG_FRAME_V(448.0f-0.5f) * 16384.0f ;
		Vertex[3].x = 640 ;
		Vertex[3].y = 448 ;
		Vertex[3].rgba = col ;
		Vertex[3].u = DG_FRAME_U(640.0f-0.5f) * 16384.0f ;
		Vertex[3].v = DG_FRAME_V(448.0f-0.5f) * 16384.0f ;
		if ( packet->param & 2 ){
			Vertex[0].u = DG_FRAME_U(0.5f+0.5f) * 16384.0f ;
			Vertex[0].v = DG_FRAME_V(0.5f+0.5f) * 16384.0f ;
			Vertex[1].u = DG_FRAME_U(640.0f-0.5f+0.5f) * 16384.0f ;
			Vertex[1].v = DG_FRAME_V(0.5f+0.5f) * 16384.0f ;
			Vertex[2].u = DG_FRAME_U(0.5f+0.5f) * 16384.0f ;
			Vertex[2].v = DG_FRAME_V(448.0f-0.5f+0.5f) * 16384.0f ;
			Vertex[3].u = DG_FRAME_U(640.0f-0.5f+0.5f) * 16384.0f ;
			Vertex[3].v = DG_FRAME_V(448.0f-0.5f+0.5f) * 16384.0f ;
		}
		DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, 4 );
	}
#else
	{
		Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), 4 );
		Vertex[0].x = 0 ;
		Vertex[0].y = 0 ;
		Vertex[0].rgba = col ;
		Vertex[0].u = DG_FRAME_U(0.5f) * 16384.0f ;
		Vertex[0].v = DG_FRAME_V(0.5f) * 16384.0f ;
		Vertex[1].x = 640 ;
		Vertex[1].y = 0 ;
		Vertex[1].rgba = col ;
		Vertex[1].u = DG_FRAME_U(640.0f-0.5f) * 16384.0f ;
		Vertex[1].v = DG_FRAME_V(0.5f) * 16384.0f ;
		Vertex[2].x = 0 ;
		Vertex[2].y = 448 ;
		Vertex[2].rgba = col ;
		Vertex[2].u = DG_FRAME_U(0.5f) * 16384.0f ;
		Vertex[2].v = DG_FRAME_V(448.0f-0.5f) * 16384.0f ;
		Vertex[3].x = 640 ;
		Vertex[3].y = 448 ;
		Vertex[3].rgba = col ;
		Vertex[3].u = DG_FRAME_U(640.0f-0.5f) * 16384.0f ;
		Vertex[3].v = DG_FRAME_V(448.0f-0.5f) * 16384.0f ;
		if ( packet->param & 2 ){
			Vertex[0].u = DG_FRAME_U(0.5f+0.5f) * 16384.0f ;
			Vertex[0].v = DG_FRAME_V(0.5f+0.5f) * 16384.0f ;
			Vertex[1].u = DG_FRAME_U(640.0f-0.5f+0.5f) * 16384.0f ;
			Vertex[1].v = DG_FRAME_V(0.5f+0.5f) * 16384.0f ;
			Vertex[2].u = DG_FRAME_U(0.5f+0.5f) * 16384.0f ;
			Vertex[2].v = DG_FRAME_V(448.0f-0.5f+0.5f) * 16384.0f ;
			Vertex[3].u = DG_FRAME_U(640.0f-0.5f+0.5f) * 16384.0f ;
			Vertex[3].v = DG_FRAME_V(448.0f-0.5f+0.5f) * 16384.0f ;
		}
		DG_DrawVerticesDirectEnd();
	}
#endif
	/* レンダリング対称を元に戻す */
	if ( mono_flag ){
		DG_SetPixelShader( NULL );
	}
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
	DG_SetRenderTarget( DG_BackBufferSurface[ DG_CurrentBackBuffer ], DG_DepthBufferSurface );
	//IDirect3DDevice9_SetViewport( g_pd3dDevice, &DG_CurrentViewport );
	IDirect3DDevice9_SetViewport( g_pd3dDevice, &dmapack_viewport );
	DG_SetVertexShaderConstant(0, screen_param, 2);
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_PARAM));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_PARAM);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawPacketCallback( DG_DMAPACK_CALLBACK *packet )
{
   int size = sizeof(DG_DMAPACK_CALLBACK) + packet->dataSize;
   memcpy(BP_CurrentDmaPackPtr, packet, size);
   BP_CurrentDmaPackPtr += size;

   return (char*)packet + size;
}

static void *DrawNext( DG_DMAPACK_NEXT *packet )
{
	return ( packet->next );
}

static void *DrawEnd( void * packet )
{
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_TAG));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TAG);

   return NULL;
}

/* ---------------------------------------------------------------- */
/* 描画関数関連 */
static void *DrawPoint( DG_DMAPACK_POINT *packet )
{
	/* 未サポート */
	return ( (void*)&packet[1] );
}

static void *DrawLine( DG_DMAPACK_LINE *packet )
{
#if 0 //BP_XBOX
	DG_SetTexture( 0, NULL );

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 2 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba1 ;

	DG_DrawVertices( D3DPT_LINELIST, 0, 2 );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_LINELIST, sizeof(DG_VERTEX_DMAPACK2D), 2 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba1 ;
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_LINE));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_LINE);

#endif
	return ( (void*)&packet[1] );
}

static void *DrawLine_F( DG_DMAPACK_LINE_F *packet )
{
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_LINE_F));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_LINE_F);

   return ( (void*)&packet[1] );
}

static void *DrawTriangle( DG_DMAPACK_TRIANGLE *packet )
{
#if 0 //BP_XBOX
	DG_SetTexture( 0, NULL );

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 3 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba1 ;
	Vertex[2].x = packet->x2 ;
	Vertex[2].y = packet->y2 ;
	Vertex[2].rgba = packet->rgba2 ;

	DG_DrawVertices( D3DPT_TRIANGLELIST, 0, 3 );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLELIST, sizeof(DG_VERTEX_DMAPACK2D), 3 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba1 ;
	Vertex[2].x = packet->x2 ;
	Vertex[2].y = packet->y2 ;
	Vertex[2].rgba = packet->rgba2 ;
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_TRIANGLE));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TRIANGLE);
#endif
	return ( (void*)&packet[1] );
}

static void *DrawQuad( DG_DMAPACK_QUAD *packet )
{
#if 0 //BP_XBOX
	DG_SetTexture( 0, NULL );

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba1 ;
	Vertex[2].x = packet->x3 ;
	Vertex[2].y = packet->y3 ;
	Vertex[2].rgba = packet->rgba3 ;
	Vertex[3].x = packet->x2 ;
	Vertex[3].y = packet->y2 ;
	Vertex[3].rgba = packet->rgba2 ;

	DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, 4 );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba1 ;
	Vertex[2].x = packet->x3 ;
	Vertex[2].y = packet->y3 ;
	Vertex[2].rgba = packet->rgba3 ;
	Vertex[3].x = packet->x2 ;
	Vertex[3].y = packet->y2 ;
	Vertex[3].rgba = packet->rgba2 ;
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_QUAD));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_QUAD);

#endif
   return ( (void*)&packet[1] );
}

static void *DrawQuad_F( DG_DMAPACK_QUAD_F *packet )
{
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_QUAD_F));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_QUAD_F);
   return (void *) &packet[1];
}

static void *DrawBox( DG_DMAPACK_BOX *packet )
{
#if 0 //BP_XBOX

	DG_SetTexture( 0, NULL );

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y0 ;
	Vertex[1].rgba = packet->rgba ;
	Vertex[2].x = packet->x0 ;
	Vertex[2].y = packet->y1 ;
	Vertex[2].rgba = packet->rgba ;
	Vertex[3].x = packet->x1 ;
	Vertex[3].y = packet->y1 ;
	Vertex[3].rgba = packet->rgba ;

	DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, 4 );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y0 ;
	Vertex[1].rgba = packet->rgba ;
	Vertex[2].x = packet->x0 ;
	Vertex[2].y = packet->y1 ;
	Vertex[2].rgba = packet->rgba ;
	Vertex[3].x = packet->x1 ;
	Vertex[3].y = packet->y1 ;
	Vertex[3].rgba = packet->rgba ;
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_BOX));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_BOX);

#endif

	return ( (void*)&packet[1] );
}

static void *DrawBox_F( DG_DMAPACK_BOX_F *packet )
{
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_BOX_F));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_BOX_F);
   return (void *) &packet[1];
}

static void *DrawSprt( DG_DMAPACK_SPRT *packet )
{
#if 0 //BP_XBOX

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba ;
	Vertex[0].u = packet->u0 ;
	Vertex[0].v = packet->v0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y0 ;
	Vertex[1].rgba = packet->rgba ;
	Vertex[1].u = packet->u1 ;
	Vertex[1].v = packet->v0 ;
	Vertex[2].x = packet->x0 ;
	Vertex[2].y = packet->y1 ;
	Vertex[2].rgba = packet->rgba ;
	Vertex[2].u = packet->u0 ;
	Vertex[2].v = packet->v1 ;
	Vertex[3].x = packet->x1 ;
	Vertex[3].y = packet->y1 ;
	Vertex[3].rgba = packet->rgba ;
	Vertex[3].u = packet->u1 ;
	Vertex[3].v = packet->v1 ;

	DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, 4 );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba ;
	Vertex[0].u = packet->u0 ;
	Vertex[0].v = packet->v0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y0 ;
	Vertex[1].rgba = packet->rgba ;
	Vertex[1].u = packet->u1 ;
	Vertex[1].v = packet->v0 ;
	Vertex[2].x = packet->x0 ;
	Vertex[2].y = packet->y1 ;
	Vertex[2].rgba = packet->rgba ;
	Vertex[2].u = packet->u0 ;
	Vertex[2].v = packet->v1 ;
	Vertex[3].x = packet->x1 ;
	Vertex[3].y = packet->y1 ;
	Vertex[3].rgba = packet->rgba ;
	Vertex[3].u = packet->u1 ;
	Vertex[3].v = packet->v1 ;
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_SPRT));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_SPRT);

#endif

	return ( (void*)&packet[1] );
}

static void *DrawRSprt( DG_DMAPACK_RSPRT *packet )
{
#if 0 //BP_XBOX

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba ;
	Vertex[0].u = packet->u0 ;
	Vertex[0].v = packet->v0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba ;
	Vertex[1].u = packet->u1 ;
	Vertex[1].v = packet->v0 ;
	Vertex[2].x = packet->x2 ;
	Vertex[2].y = packet->y2 ;
	Vertex[2].rgba = packet->rgba ;
	Vertex[2].u = packet->u0 ;
	Vertex[2].v = packet->v1 ;
	Vertex[3].x = packet->x3 ;
	Vertex[3].y = packet->y3 ;
	Vertex[3].rgba = packet->rgba ;
	Vertex[3].u = packet->u1 ;
	Vertex[3].v = packet->v1 ;

	DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, 4 );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), 4 );
	Vertex[0].x = packet->x0 ;
	Vertex[0].y = packet->y0 ;
	Vertex[0].rgba = packet->rgba ;
	Vertex[0].u = packet->u0 ;
	Vertex[0].v = packet->v0 ;
	Vertex[1].x = packet->x1 ;
	Vertex[1].y = packet->y1 ;
	Vertex[1].rgba = packet->rgba ;
	Vertex[1].u = packet->u1 ;
	Vertex[1].v = packet->v0 ;
	Vertex[2].x = packet->x2 ;
	Vertex[2].y = packet->y2 ;
	Vertex[2].rgba = packet->rgba ;
	Vertex[2].u = packet->u0 ;
	Vertex[2].v = packet->v1 ;
	Vertex[3].x = packet->x3 ;
	Vertex[3].y = packet->y3 ;
	Vertex[3].rgba = packet->rgba ;
	Vertex[3].u = packet->u1 ;
	Vertex[3].v = packet->v1 ;
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_RSPRT));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_RSPRT);
#endif

	return ( (void*)&packet[1] );
}

static void *DrawSprt_F( DG_DMAPACK_SPRT_F *packet )
{
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_SPRT_F));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_SPRT_F);
   return ( (void*)&packet[1] );
}

static void *DrawRSprt_F( DG_DMAPACK_RSPRT_F *packet )
{
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_RSPRT_F));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_RSPRT_F);
   return ( (void*)&packet[1] );
}

static void *DrawLineStrip( DG_DMAPACK_LINESTRIP *packet )
{
	DG_DMAPACK_VERTEX	*vertex ;
	int					i ;

	vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;

#if 0 //BP_XBOX
#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), packet->v_count );
	for ( i = 0 ; i < packet->v_count ; i++ ){
		Vertex->x = vertex->x ;
		Vertex->y = vertex->y ;
		Vertex->u = vertex->u ;
		Vertex->v = vertex->v ;
		Vertex->rgba = vertex->rgba ;
		vertex++ ;
		Vertex++ ;
	}
	DG_DrawVertices( D3DPT_LINESTRIP, 0, packet->v_count );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_LINESTRIP, sizeof(DG_VERTEX_DMAPACK2D), packet->v_count );
	for ( i = 0 ; i < packet->v_count ; i++ ){
		Vertex->x = vertex->x ;
		Vertex->y = vertex->y ;
		Vertex->u = vertex->u ;
		Vertex->v = vertex->v ;
		Vertex->rgba = vertex->rgba ;
		vertex++ ;
		Vertex++ ;
	}
	DG_DrawVerticesDirectEnd();
#endif

#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_LINESTRIP));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_LINESTRIP);

   for ( i = 0; i < packet->v_count; i++, vertex++ )
   {
      memcpy(BP_CurrentDmaPackPtr, vertex, sizeof(DG_DMAPACK_VERTEX));
      BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_VERTEX);
   }

#endif
	return ( (void*)vertex );
}

static void *DrawLineStrip_F( DG_DMAPACK_LINESTRIP_F *packet )
{
   DG_DMAPACK_VERTEX_F	*vertex ;
   int					i ;

   vertex = (DG_DMAPACK_VERTEX_F*)&packet[1] ;

   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_LINESTRIP_F));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_LINESTRIP_F);

   for ( i = 0; i < packet->v_count; i++, vertex++ )
   {
      memcpy(BP_CurrentDmaPackPtr, vertex, sizeof(DG_DMAPACK_VERTEX_F));
      BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_VERTEX_F);
   }

   return ( (void*)vertex );
}

static void *DrawTriangleStrip( DG_DMAPACK_TRIANGLESTRIP *packet )
{
	DG_DMAPACK_VERTEX	*vertex ;
	int					i ;

	vertex = (DG_DMAPACK_VERTEX*)&packet[1] ;
#if 0 //BP_XBOX

#if 0
	Vertex = DG_NewDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), packet->v_count );
	for ( i = 0 ; i < packet->v_count ; i++ ){
		Vertex->x = vertex->x ;
		Vertex->y = vertex->y ;
		Vertex->u = vertex->u ;
		Vertex->v = vertex->v ;
		Vertex->rgba = vertex->rgba ;
		vertex++ ;
		Vertex++ ;
	}
	DG_DrawVertices( D3DPT_TRIANGLESTRIP, 0, packet->v_count );
#else
	Vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), packet->v_count );
	for ( i = 0 ; i < packet->v_count ; i++ ){
		Vertex->x = vertex->x ;
		Vertex->y = vertex->y ;
		Vertex->u = vertex->u ;
		Vertex->v = vertex->v ;
		Vertex->rgba = vertex->rgba ;
		vertex++ ;
		Vertex++ ;
	}
	DG_DrawVerticesDirectEnd();
#endif
#else
   memcpy(BP_CurrentDmaPackPtr, packet, sizeof(DG_DMAPACK_TRIANGLESTRIP));
   BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_TRIANGLESTRIP);

   for ( i = 0; i < packet->v_count; i++, vertex++ )
   {
      memcpy(BP_CurrentDmaPackPtr, vertex, sizeof(DG_DMAPACK_VERTEX));
      BP_CurrentDmaPackPtr += sizeof(DG_DMAPACK_VERTEX);
   }
#endif
	return ( (void*)vertex );
}

