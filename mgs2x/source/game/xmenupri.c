//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------

/*
	xmenupri.c

	メニュー系プリミティブ表示管理デーモン

	1999/08/18 K.Uehara ( original K.Takabe )
	2000/09/13 K.Takabe : new font support
	2002/02/18 K.Takabe : xbox version
	$Id: xmenupri.c,v 1.5 2002/11/23 11:01:05 Yoshizawa1 Exp $
*/

#if 0 //BP_XBOX
#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include	"gameheader.h"
#include	"menuprim.h"
#include	"dmapack.h"

#include	"def_dma.h"

#include "BP_BaseRenderer.h"

#define	MAX_MENU_PRIMS	(2048)

#define N_PRIMS	(MAX_MENU_PRIMS)
#define N_PACKETS	(1)

#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)

#ifndef PAL
#define CLUT_BASE	(0x100000/64-32)			/* NTSC時ＣＬＵＴ領域（ＶＲＡＭに常駐させる） */
#else
#define CLUT_BASE	(TEXTURE_TOP_PAGE()/64)		/* PAL時ＣＬＵＴ領域 */
#endif
#define CLUT_WIDTH	(64/64)

/* 表示関連設定 */
#define	MOJI_X_SPACE	(0)	/* 文字同士の隙間 */
#define	MOJI_Y_SPACE	(1)	/* 文字同士の隙間 */
#define MOJI_WIDTH		(8)	/* 文字幅（未使用） */
#define MOJI_HEIGHT		(8)	/* 文字高さ（未使用） */

#define TRI_CODE		(3678773)	/* "menu" */
#define TEXTURE0		(6852885)	/* "font_alp_ovl" */
#define TEXTURE1		(13364753)	/* "font_mini_alp_ovl" */
#define TEXTURE2		(9005379)	/* "debug_font_alp_ovl" */
#define ICON_TEXTURE0	(11717998)	/* "cd_err_alp_ovl" */

/* ------------------------------------------------------------ */
/*
	プリミティブ管理デーモン
*/


#include "plugin.h"

int GM_DebugPrint_Off = 0;	// これ以降DEBUGPRINTをOFFにする。

/* ---------------------------------------------------------------------- */
/*
	DMAPACK version
*/

/* 描画環境初期化パケット構造体 */
typedef struct {
	sceGifTag gif;
	DG_GSREG alpha;
	DG_GSREG tex2;
	DG_GSREG tex0;
	DG_GSREG clamp;
	DG_GSREG texa;
	DG_GSREG test;
} INIT_PACK;

/* フォントテクスチャ情報記録用構造体 */
typedef struct {
	int		x, y ;			/* テクスチャ配置座標 */
	int		w, h ;			/* テクスチャサイズ */
	int		x_space ;		/*  */
	int		y_space ;		/*  */
	int		width ;			/* フォントの幅 */
	int		height ;		/* フォントの高さ */
	char	*pitch_table ;	/* 文字ピッチテーブルへのポインタ */
	DG_TEX	tex ;
	float	u_scale ;		/* テクセル値からＵＶ値への変換パラメータ */
	float	v_scale ;		/* テクセル値からＵＶ値への変換パラメータ */
	DG_GSREG	tex2 ;		/*  */
	DG_GSREG	tex0 ;		/*  */
	DG_GSREG	prim ;
} FONT_INFO ;

/* アイコンテクスチャ情報記録用構造体 */
typedef struct {
	int		tex_width ;		/* テクスチャ幅 */
	int		tex_height ;	/* テクスチャの高さ */
	int		offset_x ;		/* 配置オフセット */
	int		offset_y ;		/* 配置オフセット */
	int		x_num ;			/* アイコン分割数 */
	int		y_num ;			/* アイコン分割数 */
	DG_TEX	tex ;
	int		pad[ 1 ];
	DG_GSREG	tex2 ;		/*  */
	DG_GSREG	tex0 ;		/*  */
	DG_GSREG	prim ;
} ICON_INFO ;

/* フォント毎の描画環境記録用 */
typedef struct {
	DG_PRIM_RGBA	color;
	int				x, y, flag;
	int				xtop;
} MENU_PRINT_WORK;

/* テクスチャ初期化構造体 */
typedef struct {
	DG_GSREG	tex2 ;
	DG_GSREG	tex0 ;
	DG_GSREG	rgba ;
	DG_GSREG	prim ;
} MENU_SPRT_INIT;

/* スプライト設定構造体 */
typedef struct {
	DG_PRIM_UV uv1;
	DG_PRIM_XY xy1;
	DG_PRIM_UV uv2;
	DG_PRIM_XY xy2;
} MENU_SPRT;

/* MENU_Printf処理実行アクター用ワーク */
typedef	ALIGN16_DECL(struct) _xmenupri_Work	{
	GV_ACT_EX		actor ;

	DG_DMAPACK		*current_dmapack ;
	DG_DMAPACK		*dmapack;

	//INIT_PACK		setup;
	//FONT_INFO		font_info[3] ;
	//ICON_INFO		icon_info[1] ;
	FONT_INFO		*font_info ;
	ICON_INFO		*icon_info ;

	void			*last;
	int				size;
	int				open_count;
	int				prim_buffer_size ;	/* プリミティブ用バッファのサイズ */
   float          safeZoneTrim;

	ALIGN16_PRE MENU_PRINT_WORK	print_work[ 3 ] ALIGN16_POST;

	u_long128		*buffer[ 2 ];
	//u_long128		buffer[ 2 ][ MAX_MENU_PRIM_SIZE / sizeof( u_long128 ) ];
} Work  ;

/* ワークの実体 */
static ALIGN16_PRE Work menuprimwork ALIGN16_POST;		/* 常駐させるためワークはスタティックに確保 */
static INIT_PACK	local_setup = {{0}};
static DG_DMAPACK	local_dmapack = {0};
static u_long128	local_buffer[ 2 ][ MAX_MENU_PRIM_SIZE / sizeof( u_long128 ) ] = {{0}};
static FONT_INFO	font_info[3] ;
static ICON_INFO	icon_info[1] ;

static DG_TEX	font_tex[3] = {{0}};
static DG_TEX	icon_tex[1] = {{0}};

/* パケットバッファ先頭のDMAタグ */
typedef struct {
	DG_DMATAG setup;
	DG_DMATAG top;
	u_long128  data[ 0 ];
} PACKET_TOP;

/* 各フォントデータのピッチデータ */
char	font_pitch0[96] = {
	/*  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  */
	 6, 7, 9,11,11,12,12, 6, 9, 9,11,12, 7,12, 7,11,11,11,11,11,11,11,11,11,11,11, 7, 7,10,12,10,12,
	/*  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \\  ]  ^  _  */
	12,12,12,12,12,12,12,12,12, 6,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12, 9,10, 9, 9,12,
	/*  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~     */
	11,11,11,11,11,11,11,11,11, 6,10,11, 6,12,11,11,11,11,11,11,11,11,11,11,11,11,11, 9, 8, 9,12, 8,
};
char	font_pitch1[96] = {
	/*  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  */
	 9, 6, 9,10,10,10,10, 6, 9, 9, 9,10, 5,10, 5, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 5, 5,10,10,10,10,
	/*  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \\  ]  ^  _  */
	10,10,10,10,10,10,10,10,10, 5, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10, 9,10, 9, 9,10,
	/*  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~     */	
	10,10,10,10,10,10,10,10,10, 5, 7, 9, 5,10,10,10,10,10,10,10,10,10,10,10,10,10,10, 9, 8, 9,10, 6,
};
char	font_pitch2[96] = {
	/*  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  */
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	/*  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \\  ]  ^  _  */
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	/*  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~     */
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

/* ---------------------------------------------------------------------- */
static void InitPacket( Work *work, void *pack );
/* ---------------------------------------------------------------------- */
/*
	DMAのセット
*/

#if 0
static inline void set_dma_size( void *packet, int size )
{
	PACKET_TOP *top = packet;
	top->top.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, size );
	top->top.vifcode[1] = SCE_VIF1_SET_DIRECT( size, 0 ) ;
}
#endif

/* ---------------------------------------------------------------------- */
/*
	ACT 
*/

static void Act( Work *work )
{
	DG_DMAPACK *dmapack ;

	dmapack = work->dmapack ;
	work->last = dmapack->autopacket ;

	work->last = DG_SetDmapackMode( work->last, DG_DMAPACK_MODE_NO_BILINEAR );
	DG_SetDmapackEnd( work->last );

	work->open_count = 0;

	GM_DebugPrint_Off = 0;
}

/* ---------------------------------------------------------------------- */
/*
	Initialize
*/

static void InitPacket( Work *work, void *pack )
{
}

static void InitSetup( INIT_PACK *setup )
{
#if 0
	setup->alpha.reg = SCE_GS_ALPHA_1;
	setup->alpha.data = SCE_GS_SET_ALPHA( 0,1,0,1,128 );

	setup->tex2.reg = SCE_GS_TEX2_1;
	setup->tex2.data = SCE_GS_SET_TEX2(SCE_GS_PSMT8,0,0,0,0,2) ;

	setup->tex0.reg = SCE_GS_TEX0_1;
	setup->tex0.data = SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH,
										 SCE_GS_PSMT4HL, 8, 8, 1, 0,
										 CLUT_BASE, SCE_GS_PSMCT16, 0, 0, 1 );

	setup->clamp.reg = SCE_GS_CLAMP_1;
	setup->clamp.data = SCE_GS_SET_CLAMP( 1, 1, 0, 0, 1024, 1024 );

	setup->texa.reg = SCE_GS_TEXA;
	setup->texa.data = SCE_GS_SET_TEXA( 128, 1, 128 );

	setup->test.reg = SCE_GS_TEST_1 ;
	setup->test.data = SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 1 );/* Ｚ値を更新せず、Ｚは無視するようにする */
#endif
}

static int GetResources( Work *work, int dmapack_flag, int dmapack_phase, int dmapack_prio )
{
	DG_DMAPACK *pack;
	int		i ;

	/*
		メニュー用チャンネルにプリミティブを生成
	*/

	pack = work->dmapack ;
	if ( pack == NULL ){
		pack = work->dmapack = DG_MakeDmapack2( dmapack_flag, dmapack_phase, dmapack_prio );
		if ( pack == NULL ) return ( -1 );
	}
	DG_QueueDmapack( pack );

	/*
		パケット領域の確保
		(32K * 2)
	*/

	//pack->packet[ 0 ] = work->buffer[ 0 ];
	//pack->packet[ 1 ] = work->buffer[ 1 ];
	pack->autopacket = work->buffer[ 0 ];

	/*
		セットアップパケットの初期化
	*/

	//InitSetup( &work->setup );

	/* その他ワークの初期化 */

	work->open_count = 0;

	/*
		描画環境の設定
	*/
	for ( i = 0 ; i < 3 ; i++ ){
		work->print_work[ i ].color.r = 200 ;
		work->print_work[ i ].color.g = 200 ;
		work->print_work[ i ].color.b = 200 ;
		//work->print_work[ i ].color.a = 128 ;
	}

#if 0
	/*
		フォント固有情報の設定
	*/
	/* 標準フォント */
	tex = DG_GetTexture2( TRI_CODE, TEXTURE0 );
	//tex = &font_tex[ 0 ] ;
	DG_GetTexelInfo( &work->font_info[0].w, &work->font_info[0].h,
					  &work->font_info[0].x, &work->font_info[0].y, tex );
	work->font_info[0].tex = tex ;
	work->font_info[0].u_scale = 1.0f / work->font_info[0].w ;
	work->font_info[0].v_scale = 1.0f / work->font_info[0].h ;
   work->font_info[0].x_space = 0 ;
	work->font_info[0].y_space = 1 ;
	work->font_info[0].width = 12 ;
	work->font_info[0].height = 8*2 ;
	work->font_info[0].pitch_table = font_pitch0 ;
	clut_offset = ( ( tex->tex_trans.tex0.data >> 37 ) & 0x3fff  ) - tri_clut_base ;
	work->font_info[0].tex0.reg = SCE_GS_TEX0_1 ;
	work->font_info[0].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
	work->font_info[0].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
													CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 0 );
	work->font_info[0].tex2.reg = SCE_GS_TEX2_1 ;
	work->font_info[0].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 4 );
	work->font_info[0].prim.reg = SCE_GS_PRIM ;
	work->font_info[0].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );

	/* ミニフォント */
	tex = DG_GetTexture2( TRI_CODE, TEXTURE1 );
	//tex = &font_tex[ 1 ] ;
	DG_GetTexelInfo( &work->font_info[1].w, &work->font_info[1].h,
					  &work->font_info[1].x, &work->font_info[1].y, tex );
	work->font_info[1].tex = tex ;
	work->font_info[1].u_scale = 1.0f / work->font_info[1].w ;
	work->font_info[1].v_scale = 1.0f / work->font_info[1].h ;
	work->font_info[1].x_space = 1 ;
	work->font_info[1].y_space = 1 ;
	work->font_info[1].width = 10 ;
	work->font_info[1].height = 7*2 ;
	work->font_info[1].pitch_table = font_pitch1 ;
	clut_offset = ( ( tex->tex_trans.tex0.data >> 37 ) & 0x3fff  ) - tri_clut_base ;
	work->font_info[1].tex0.reg = SCE_GS_TEX0_1 ;
	work->font_info[1].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
	work->font_info[1].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
													CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 0 );
	work->font_info[1].tex2.reg = SCE_GS_TEX2_1 ;
	work->font_info[1].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 4 );
	work->font_info[1].prim.reg = SCE_GS_PRIM ;
	work->font_info[1].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );

	/* デバッグ用フォント */
	//tex = DG_GetTexture2( TRI_CODE, TEXTURE2 );
	tex = &font_tex[ 2 ] ;
	DG_GetTexelInfo( &work->font_info[2].w, &work->font_info[2].h,
					  &work->font_info[2].x, &work->font_info[2].y, tex );
	work->font_info[2].tex = tex ;
	work->font_info[2].u_scale = 1.0f / work->font_info[2].w ;
	work->font_info[2].v_scale = 1.0f / work->font_info[2].h ;
	work->font_info[2].x_space = 1 ;
	work->font_info[2].y_space = 1 ;
	work->font_info[2].width = 8 ;
	work->font_info[2].height = 16 ;
	work->font_info[2].pitch_table = font_pitch2 ;
	clut_offset = ( ( tex->tex_trans.tex0.data >> 37 ) & 0x3fff  ) - tri_clut_base ;
	work->font_info[2].tex0.reg = SCE_GS_TEX0_1 ;
	work->font_info[2].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
	work->font_info[2].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
													CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 0 );
	work->font_info[2].tex2.reg = SCE_GS_TEX2_1 ;
	work->font_info[2].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 4 );
	work->font_info[2].prim.reg = SCE_GS_PRIM ;
	work->font_info[2].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );

	//printf("%d %d %d %d\n", work->font_info[0].x, work->font_info[0].y, work->font_info[0].w, work->font_info[0].h );
	//printf("%d %d %d %d\n", work->font_info[1].x, work->font_info[1].y, work->font_info[1].w, work->font_info[1].h );
	//printf("%d %d %d %d\n", work->font_info[2].x, work->font_info[2].y, work->font_info[2].w, work->font_info[2].h );

	/* アイコン関連の設定 */
	tex = DG_GetTexture2( TRI_CODE, ICON_TEXTURE0 );
	//tex = &icon_tex[ 0 ] ;
	DG_GetTexelInfo( &work->icon_info[0].tex_width, &work->icon_info[0].tex_height,
					  &work->icon_info[0].offset_x, &work->icon_info[0].offset_y, tex );
	work->font_info[0].tex = tex ;
	work->icon_info[0].x_num = 2 ;
	work->icon_info[0].y_num = 1 ;
	work->icon_info[0].tex0.reg = SCE_GS_TEX0_1 ;
	work->icon_info[0].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
	work->icon_info[0].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
													CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 0 );
	work->icon_info[0].tex2.reg = SCE_GS_TEX2_1 ;
	work->icon_info[0].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE+clut_offset, SCE_GS_PSMCT32, 0, 0, 4 );
	work->icon_info[0].prim.reg = SCE_GS_PRIM ;
	work->icon_info[0].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
#else
	work->font_info = font_info ;
	work->icon_info = icon_info ;
#endif

	/*
		その他設定
	*/
	work->current_dmapack = work->dmapack ;
	//work->current_prim = &work->menu_prim ;

	Act( work );

	return 0;
}



/* ---------------------------------------------------------------------- */
/*
	スタートアップ
*/
void *GM_StartMenuPrimManager( void )
{
	Work		*work ;
	int			dmapack_flag, dmapack_phase, dmapack_prio ;

	work = &menuprimwork;
	GV_ZeroMemory( work, sizeof( menuprimwork ) );
	work->dmapack = &local_dmapack ;
	work->buffer[ 0 ] = local_buffer[ 0 ] ;
	work->buffer[ 1 ] = local_buffer[ 1 ] ;
	work->prim_buffer_size = MAX_MENU_PRIM_SIZE ;
   work->safeZoneTrim = 0.0f;

	/* DG_DMAPACKの初期化 */
	dmapack_flag = DG_DMAPACK_MENU|DG_DMAPACK_PRIVILEGE ;	/* 特権オブジェクト */
	dmapack_phase = DG_DMAPACK_PHASE_AFTER ;				/*  */
	dmapack_prio = 250 ;									/* プライオリティは結構高めに */
	work->dmapack->flag = dmapack_flag ;
	work->dmapack->phase = dmapack_phase ;
	work->dmapack->priority = dmapack_prio ;

	if( ((GV_ACT*)&work->actor)->prev != NULL ){
		return NULL;
	}

   font_tex[0] = *( DG_GetTexture2( TRI_CODE, TEXTURE0 ) );
   font_tex[1] = *( DG_GetTexture2( TRI_CODE, TEXTURE1 ) );
   font_tex[2] = *( DG_GetTexture2( TRI_CODE, TEXTURE2 ) );
   icon_tex[0] = *( DG_GetTexture2( TRI_CODE, ICON_TEXTURE0 ) );

   BP_TextureSetResidentTexture(font_tex[0].BP_TextureHandle);
   BP_TextureSetResidentTexture(font_tex[1].BP_TextureHandle);
   BP_TextureSetResidentTexture(font_tex[2].BP_TextureHandle);
   BP_TextureSetResidentTexture(icon_tex[0].BP_TextureHandle);

	/* 描画初期化パケット生成 */
	InitSetup( &local_setup );

	GV_InsertActorPriority( GV_ACTOR_DAEMON, work, 0xE0 );
	GV_SetActorFreeFunc( work, NULL );
	GV_SetActor( work, Act, NULL );

	if ( GetResources( work, dmapack_flag, dmapack_phase, dmapack_prio ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------------- */
/*
	外部関数からの呼出しインターフェース
*/

void MENU_SetSafeZoneTrim(float safeZoneTrim)
{
   Work *work = &menuprimwork;
   work->safeZoneTrim = safeZoneTrim;
}

void *___MENU_OpenPrim( void *work_ptr )
{
	Work	*work = work_ptr ;

	if ( work_ptr == NULL ) return ( NULL );

   {
      float const ooSafeZone = 1.0f - work->safeZoneTrim;
      work->last = DG_SetDmapackViewMapping(work->last, work->safeZoneTrim*DRAW_WIDTH, work->safeZoneTrim*DRAW_HEIGHT, ooSafeZone*DRAW_WIDTH, ooSafeZone*DRAW_HEIGHT);
   }

	work->open_count ++;

	return ( work->last );
}
void *MENU_OpenPrim( void )
{
	return ( ___MENU_OpenPrim( &menuprimwork ) );
}

void ___MENU_ClosePrim( void *work_ptr, void *last )
{
	Work		*work = work_ptr ;

	if ( work_ptr == NULL ) return ;

	work->open_count --;
	if( work->open_count > 0 ){
		return;
	}

	work->last = last ;
   work->last = DG_SetDmapackViewMapping(work->last, 0.0f, 0.0f, DRAW_WIDTH, DRAW_HEIGHT);
	DG_SetDmapackEnd( work->last );
}
void MENU_ClosePrim( void *last )
{
	___MENU_ClosePrim( &menuprimwork, last );
}

/* ------------------------------------------------------------ */
/*
	画面へのprintf
*/

void ___MENU_Color( void *work_ptr, int which, int r, int g, int b, int a )
{
	Work		*work = work_ptr ;
	//MENU_PRINT_WORK *work = &print_work[ which ];
	MENU_PRINT_WORK *pw ;

	if ( work_ptr == NULL ) return ;
	pw = &work->print_work[ which ];
	pw->color.r = r ;
	pw->color.g = g ;
	pw->color.b = b ;

	pw->color.a = a;
}
void __MENU_Color( int which, int r, int g, int b, int a )
{
	___MENU_Color( &menuprimwork, which, r, g, b, a );
}

void ___MENU_ResetColor( void *work_ptr, int which )
{
	___MENU_Color( work_ptr, which, 192, 192, 192, 0 );
}
void __MENU_ResetColor( int which )
{
	___MENU_ResetColor( &menuprimwork, which );
}

void ___MENU_Locate( void *work_ptr, int which, int x, int y, int flag )
{
	Work		*work = work_ptr ;
	//MENU_PRINT_WORK *work = &print_work[ which ];
	MENU_PRINT_WORK *pw ;

	if ( work_ptr == NULL ) return ;
	pw = &work->print_work[ which ];
	pw->xtop = pw->x = x ;
	pw->y = y ;
	pw->flag = flag;
}
void __MENU_Locate( int which, int x, int y, int flag )
{
	___MENU_Locate( &menuprimwork, which, x, y, flag );
}

void ___MENU_SetAlphaMode( void *work_ptr, long64 alpha )
{
	Work		*work = work_ptr ;
	void		*prim ;

	if ( work_ptr == NULL ) return ;
	prim = ___MENU_OpenPrim( work );
	prim = DG_SetDmapackAlpha( prim, alpha );
	//p->value = alpha;
	___MENU_ClosePrim( work, prim );
}
void __MENU_SetAlphaMode( long64 alpha )
{
	___MENU_SetAlphaMode( &menuprimwork, alpha );
}

static int get_length( FONT_INFO *font_info, char *str )
{
	int		len = 0 ;
	char	c ;
	while ( ( c = *str++ ) != '\0' ){
		if( c == '\n' ) break ;
		if ( c >= ' ' && c < ( ' ' + 96 ) ){
			c -= ' ' ;
			len += font_info->pitch_table[ c ]*TARGET_ASPECT_X ;
		} else {
			len += font_info->pitch_table[ 0 ]*TARGET_ASPECT_X ;
		}
	}
	return ( len );
}

static int trim2( int xw, int flag )
{
	if( xw > 0 ){
		switch( flag & 3 ){
		  case MENU_MODE_LEFT:
		  default:
			return 0;
		  case MENU_MODE_CENTER:
			return ( -xw / 2 );
		  case MENU_MODE_RIGHT:
			return ( -xw );
		}
	}
	return 0 ;
}

void ___MENU_Print( void *work_ptr, int which, char *buffer )
{
	Work			*work = work_ptr ;
	int				x, y, w, h, u, v, n ;
	unsigned char	*string ;
	int				c;

	int wsum;
	MENU_PRINT_WORK *pw;
	FONT_INFO	*font_info ;
	DG_DMAPACK_COLOR		rgba ;
	void		*prim ;
	float		x0, y0, x1, y1, u0, v0, u1, v1 ;

	if ( work_ptr == NULL ) return ;

	/* モード、フラグによるフォント設定 */
	font_info = &work->font_info[ which ] ;

	string = ( unsigned char * )buffer;

	pw = &( work->print_work[ which ] );

	x = pw->x; y = pw->y;
	w = 0 ;
	h = font_info->height ;

	/* パケットを取得 */
	/* 初期化パケット */
	prim = ___MENU_OpenPrim( work );

	prim = DG_SetDmapackTex( prim, &font_info->tex );

   rgba = DG_RGBATODMARGBA( pw->color );
//	rgba = *(int*)&pw->color ;
	if( pw->color.a == 0 ){
      rgba.a |= 0x80;
	}

	/* 文字数分パケットを生成 */

	n = 0;
	wsum = 0;

	for( string = buffer; *string != 0; string ++ ){
		/*  */
		if ( wsum == 0 ){
			x += trim2( get_length( font_info, string ), pw->flag );
		}
		/* テクスチャＵＶ値決定 */
		c = *string ;
		if( c == '\n' ){
			pw->y = y + h + MOJI_Y_SPACE ;
			wsum = 0;
			x = pw->xtop;
			y = pw->y ;
			continue;
		}
		if( c >= 0x80 ){
			w = ( c - 0x80 );
			x += w + MOJI_X_SPACE;
			wsum += w + MOJI_X_SPACE;
			continue;
		}

		if ( c >= ' ' && c < ( ' ' + 96 ) ){
			c -= ' ' ;
			w = font_info->pitch_table[ c ];//*ASPECT_X() ;
			u = ( c & 31 ) * font_info->width ;
			v = ( c / 32 ) * font_info->height ;
		} else {
			w = font_info->pitch_table[ 0 ] ;
			u = 0 ;
			v = 0 ;
		}
		x0 = x ;
		y0 = y ;
		x1 = x + (w*TARGET_ASPECT_X) - 1 ;
		y1 = y + h - 1 ;

		u0 = ( 0.5f + u ) * font_info->u_scale ; 
		v0 = ( 0.5f + v ) * font_info->v_scale ;
		u1 = ( -0.5f + ( u + w ) ) * font_info->u_scale ;
		v1 = ( -0.5f + ( v + h ) ) * font_info->v_scale ;
		prim = DG_SetDmapackSprtTex( prim,
										  x0, y0, u0, v0, x1, y1, u1, v1, rgba, &font_info->tex );

		x += (w + MOJI_X_SPACE)*TARGET_ASPECT_X;
		wsum += (w + MOJI_X_SPACE);
		n++;
	}

	if( pw->flag > 0 && wsum > 0 ){
		pw->y = y + h + MOJI_Y_SPACE ;
	} else {
		pw->x = x ;
	}

	___MENU_ClosePrim( work, prim );
}
void __MENU_Print( int which, char *buffer )
{
#ifdef DEBUG_MODE
	if( which == 2 && ( ! GM_DebugModeEnable || GM_DebugPrint_Off ) ){
		return;
	}
#endif
	___MENU_Print( &menuprimwork, which, buffer );
}

void ___MENU_Printf( void *work_ptr, int which, char *fmt, ... )
{
	Work			*work = work_ptr ;
	va_list			args ;
	char			buffer[ 256 ];

	if ( work_ptr == NULL ) return ;

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );

	___MENU_Print( work, which, buffer );

	va_end( args );
}
void __MENU_Printf( int which, char *fmt, ... )
{
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );

	__MENU_Print( which, buffer );

	va_end( args );
}

/* ---------------------------------------------------------------- */
	/*
		文字と同じ感覚でアイコンを表示する
	*/

#if 0
static void put_icon( MENU_PRIM *prim, ICON_INFO *icon_info, int x, int y, int ptn, int col )
{
#if 0
	MENU_SPRT_INIT *init;
	sceGifTag *gstag;
	MENU_SPRT *sprt ;
	int			w, h, u, v ;

	MENU_NEW_TAG( gstag, prim );
	MENU_NEW_PRIM( init, prim );

	/* 初期化パケット生成 */
	DG_SET_GIFTAG( gstag, .FLG = SCE_GIF_PACKED, .NREG = 1, .NLOOP = 4
				   , .REGS0 = GS_REGS_AD );
	init->tex2 = icon_info->tex2 ;
	init->tex0 = icon_info->tex0 ;
	init->prim = icon_info->prim ;
	init->rgba.reg = SCE_GS_RGBAQ ;
	init->rgba.data = col ;

	/* 頂点、ＵＶの設定パケットタグ */
	MENU_NEW_TAG( gstag, prim );
	DG_SET_GIFTAG( gstag, .FLG = SCE_GIF_REGLIST, .NREG = 4, .NLOOP = 1
				   , .REGS0 = GS_REGS_UV
				   , .REGS1 = GS_REGS_XYZ2
				   , .REGS2 = GS_REGS_UV
				   , .REGS3 = GS_REGS_XYZ2 );

	MENU_NEW_PRIM( sprt, prim );

	x = DG_POS_X( x );
	y = DG_POS_X( y );
	w = ( icon_info->tex_width / icon_info->x_num ) << 4 ;
	h = ( icon_info->tex_height / icon_info->y_num ) << 4 ;
	u = icon_info->offset_x * 16 + w * ( ptn % icon_info->x_num ) + 8 ;
	v = icon_info->offset_y * 16 + h * ( ptn / icon_info->x_num ) + 8 ;
	DG_SET_XY1( sprt, x, y );
	DG_SET_XY2( sprt, x + w, y + h );
	DG_SET_UV1( sprt, u, v );
	DG_SET_UV2( sprt, u + w, v + h );
#endif
}
#endif

/*
	Undraw時に無理矢理表示バッファにアイコンを表示する
*/

void MENU_PutIconUndraw( int type, int x, int y, int ptn, DG_DMAPACK_COLOR col, int flag )
{
	// 現状、非対応。
}

void MENU_PutIcon( int type, int x, int y, int ptn, DG_DMAPACK_COLOR col, int flag )
{
	void *prim;
	Work *work;
	float w, h, u, v;
	ICON_INFO	*icon_info ;

	if( DG_UnDrawFrameCount > 0 ){
		MENU_PutIconUndraw( type, x, y, ptn, col, flag );
		return;
	}

#if FALSE	// Old
	/* モード、フラグによるフォント設定 */
	icon_info = &menuprimwork.icon_info[ 0 ] ;

	/* パケットを取得 */
	/* 初期化パケット */
	prim = MENU_OpenPrim();

	put_icon( prim, icon_info, x, y, ptn, col );

	MENU_ClosePrim();
//#endif
#else
	work = &menuprimwork;
	icon_info = work->icon_info;
	if( icon_info == NULL ){
		return;
	}
	prim = ___MENU_OpenPrim( work );
	prim = DG_SetDmapackTex( prim, &icon_info->tex );
	w = ( icon_info->tex_width / icon_info->x_num ) ;
	h = ( icon_info->tex_height / icon_info->y_num ) ;
	u = 0 + w * ( ptn % icon_info->x_num ) + 0.5F ;
	v = 0 + h * ( ptn / icon_info->x_num ) + 0.5F ;
	prim = DG_SetDmapackSprtTex( prim, x, y
								, u * 1.0F / icon_info->tex_width
								, v * 1.0F / icon_info->tex_height
								, x + w, y + h
								, ( u + w ) * 1.0F / icon_info->tex_width
								, ( v + h ) * 1.0F / icon_info->tex_height
								, col, &icon_info->tex );
	___MENU_ClosePrim( work, prim );
#endif
}

/* ---------------------------------------------------------------- */
	/*
		メニュー文字プリミティブ内に任意のサイズのＧＩＦパケット領域を確保する
	*/
void* MENU_PutPacket( int size )
{
	void		*prim ;
	void		*packet_addr ;

	prim = MENU_OpenPrim();

	packet_addr = prim ;
	prim = (void*)( (char*)prim + size );

	MENU_ClosePrim( prim );

	return ( packet_addr );
}


/* ---------------------------------------------------------------------- */
/*
	ユーザー独自使用型MENU_Printf
*/
static void Die( Work *work )
{
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	if ( work->buffer[ 0 ] != NULL ){
		GV_DelayedFree( work->buffer[ 0 ] );
	}
}
void *NewMenuPrintManager( int buffer_size, int dmapack_flag, int dmapack_phase, int dmapack_prio )
{
	Work		*work ;
	u_long128	*buffer ;

	work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT, sizeof(Work), 0 );
	if ( work != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		buffer = GV_Malloc( buffer_size * 2 );
		work->buffer[ 0 ] = buffer ;
		work->buffer[ 1 ] = buffer + buffer_size / sizeof(u_long128) ;
		work->prim_buffer_size = buffer_size ;
		if ( buffer == NULL || GetResources( work, dmapack_flag, dmapack_phase, dmapack_prio ) < 0 ){
			GV_DestroyActor( work );
			return ( NULL );
		}
	}
	return ( work );

}

/* ---------------------------------------------------------------------- */
void ___MENU_PrintMini( void *work_ptr, int which, char *buffer )
{
#if 0
	Work			*work = work_ptr ;
	int				x, y, w, h, u, v, n ;
	unsigned char	*string ;
	int				c;

	MENU_PRIM *prim;
	MENU_SPRT_INIT *init;
	sceGifTag *gstag;
	MENU_SPRT *sprt, *sptop;
	int wsum;
	MENU_PRINT_WORK *pw;
	FONT_INFO	*font_info ;

	if ( work_ptr == NULL ) return ;

	/* モード、フラグによるフォント設定 */
	font_info = &work->font_info[ which ] ;

	string = ( unsigned char * )buffer;

	pw = &( work->print_work[ which ] );

	x = pw->x; y = pw->y;
	w = 0 ;
	h = font_info->height << 4 ;

	/* パケットを取得 */
	/* 初期化パケット */
	prim = ___MENU_OpenPrim( work );

	MENU_NEW_TAG( gstag, prim );
	MENU_NEW_PRIM( init, prim );

	DG_SET_GIFTAG( gstag, .FLG = SCE_GIF_PACKED, .NREG = 1, .NLOOP = 4
				   , .REGS0 = GS_REGS_AD );
	init->tex2 = font_info->tex2 ;
	init->tex0 = font_info->tex0 ;
	init->prim = font_info->prim ;
	init->rgba.reg = SCE_GS_RGBAQ ;
	DG_COPY64( &init->rgba.data, &pw->color );
	if( pw->color.a == 0 ){
		init->rgba.data |= 128 << 24 ;
	}

	/* 文字数分パケットを生成 */

	MENU_NEW_TAG( gstag, prim );
	DG_SET_GIFTAG( gstag, .FLG = SCE_GIF_REGLIST, .NREG = 4
				   , .REGS0 = GS_REGS_UV
				   , .REGS1 = GS_REGS_XYZ2
				   , .REGS2 = GS_REGS_UV
				   , .REGS3 = GS_REGS_XYZ2 );

	n = 0;
	wsum = 0;
	sprt = sptop = prim->ptr;

	for( string = buffer; *string != 0; string ++ ){
		/* テクスチャＵＶ値決定 */
		c = *string ;
		if( c == '\n' ){
			pw->y = y + h/2 + MOJI_Y_SPACE * 16 ;
			if( pw->flag > 0 && wsum > 0 ){
				trim( sptop, sprt, wsum, pw->flag );
			}
			sptop = prim->ptr;
			wsum = 0;
			x = pw->xtop;
			y = pw->y ;
			continue;
		}
		MENU_NEW_PRIM( sprt, prim );
		if ( c >= ' ' && c < ( ' ' + 96 ) ){
			c -= ' ' ;
			w = font_info->pitch_table[ c ] * 16 ;
			u = ( font_info->x + ( c & 31 ) * font_info->width ) * 16 + 8;
			v = ( font_info->y + ( c / 32 ) * font_info->height ) * 16 + 8;
		} else {
			w = font_info->pitch_table[ 0 ] * 16 ;
			u = font_info->x * 16 ;
			v = font_info->y * 16 ;
		}
		DG_SET_XY1( sprt, x, y );
		DG_SET_XY2( sprt, x + w, y + h/2 );

		DG_SET_UV1( sprt, u, v );
		DG_SET_UV2( sprt, u + w, v + h );

		x += w + MOJI_X_SPACE * 16 ;
		wsum += w + MOJI_X_SPACE * 16 ;

		n++;
	}
	gstag->NLOOP = n;
	if( pw->flag > 0 && wsum > 0 ){
		trim( sptop, sprt, wsum, pw->flag );
		pw->y = y + h/2 + MOJI_Y_SPACE * 16 ;
	} else {
		pw->x = x ;
	}

	___MENU_ClosePrim( work );
#endif
	Work			*work = work_ptr ;
	int				x, y, w, h, u, v, n ;
	unsigned char	*string ;
	int				c;

	int wsum;
	MENU_PRINT_WORK *pw;
	FONT_INFO	*font_info ;
	DG_DMAPACK_COLOR		rgba ;
	void		*prim ;
	float		x0, y0, x1, y1, u0, v0, u1, v1 ;

	if ( work_ptr == NULL ) return ;

	/* モード、フラグによるフォント設定 */
	font_info = &work->font_info[ which ] ;

	string = ( unsigned char * )buffer;

	pw = &( work->print_work[ which ] );

	x = pw->x; y = pw->y;
	w = 0 ;
	h = font_info->height ;

	/* パケットを取得 */
	/* 初期化パケット */
	prim = ___MENU_OpenPrim( work );

	prim = DG_SetDmapackTex( prim, &font_info->tex );

	rgba = DG_RGBATODMARGBA( pw->color );
	if( pw->color.a == 0 ){
		rgba.a |= 0x80;
	}

	/* 文字数分パケットを生成 */

	n = 0;
	wsum = 0;

	for( string = buffer; *string != 0; string ++ ){
		/*  */
		if ( wsum == 0 ){
			x += trim2( get_length( font_info, string ), pw->flag );
		}
		/* テクスチャＵＶ値決定 */
		c = *string ;
		if( c == '\n' ){
			pw->y = y + h/2 + MOJI_Y_SPACE ;
			wsum = 0;
			x = pw->xtop;
			y = pw->y ;
			continue;
		}
		if ( c >= ' ' && c < ( ' ' + 96 ) ){
			c -= ' ' ;
			w = (int)((float)font_info->pitch_table[ c ]*TARGET_ASPECT_X) ;
			u = ( c & 31 ) * font_info->width ;
			v = ( c / 32 ) * font_info->height ;
		} else {
			w = (int)((float)font_info->pitch_table[ 0 ]*TARGET_ASPECT_X) ;
			u = 0 ;
			v = 0 ;
		}
		x0 = x ;
		y0 = y ;
		x1 = x + w - 1 ;
		y1 = y + h/2 - 1 ;

		u0 = ( 0.5f + u ) * font_info->u_scale ; 
		v0 = ( 0.5f + v ) * font_info->v_scale ;
		u1 = ( -0.5f + ( u + font_info->pitch_table[ c ] ) ) * font_info->u_scale ;
		v1 = ( -0.5f + ( v + h ) ) * font_info->v_scale ;
		prim = DG_SetDmapackSprtTex( prim,
										  x0, y0, u0, v0, x1, y1, u1, v1, rgba, &font_info->tex );

		x += w + MOJI_X_SPACE ;
		wsum += w + MOJI_X_SPACE ;

		n++;
	}

	if( pw->flag > 0 && wsum > 0 ){
		pw->y = y + h + MOJI_Y_SPACE ;
	} else {
		pw->x = x ;
	}

	___MENU_ClosePrim( work, prim );
}







void MENU_Printf( char *fmt, ... )
{
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );

	__MENU_Print( 0, buffer );

	va_end( args );
}
void MENU_S_Printf( char *fmt, ... )
{
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );

	__MENU_Print( 1, buffer );

	va_end( args );
}
void DEBUG_Printf( char *fmt, ... )
{
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );

	__MENU_Print( 2, buffer );

	va_end( args );
}


/* 常駐テクスチャの再イニシャライズに対応するため */
void GM_ResetMenuPrimManager( void )
{
	DG_TEX		*tex ;
	/*
		フォント固有情報の設定
	*/
	/* 標準フォント */
	//tex = DG_GetTexture2( TRI_CODE, TEXTURE0 );
	tex = &font_tex[ 0 ] ;
	if( tex )
	{
		DG_GetTexelInfo( &font_info[0].w, &font_info[0].h,
						  &font_info[0].x, &font_info[0].y, tex );
		font_info[0].tex = *tex ;
		font_info[0].u_scale = 1.0f / font_info[0].w ;
		font_info[0].v_scale = 1.0f / font_info[0].h ;
		font_info[0].x_space = 0 ;
		font_info[0].y_space = 1 ;
		font_info[0].width = 12 ;
		font_info[0].height = 8*2 ;
		font_info[0].pitch_table = font_pitch0 ;
		//clut_offset = ( ( tex->tex_trans.tex0.data >> 37 ) & 0x3fff  ) - tri_clut_base ;
		font_info[0].tex0.reg = SCE_GS_TEX0_1 ;
		font_info[0].tex0.data = tex->tex_trans.tex0.data ;
#if 0
		font_info[0].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
		font_info[0].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
														CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 0 );
#endif
		font_info[0].tex2.reg = SCE_GS_TEX2_1 ;
		font_info[0].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 4 );
		font_info[0].prim.reg = SCE_GS_PRIM ;
		font_info[0].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
	}

	/* ミニフォント */
	//tex = DG_GetTexture2( TRI_CODE, TEXTURE1 );
	tex = &font_tex[ 1 ] ;
	if( tex )
	{
		DG_GetTexelInfo( &font_info[1].w, &font_info[1].h,
						  &font_info[1].x, &font_info[1].y, tex );
		font_info[1].tex = *tex ;
		font_info[1].u_scale = 1.0f / font_info[1].w ;
		font_info[1].v_scale = 1.0f / font_info[1].h ;
		font_info[1].x_space = 1 ;
		font_info[1].y_space = 1 ;
		font_info[1].width = 10 ;
		font_info[1].height = 7*2 ;
		font_info[1].pitch_table = font_pitch1 ;
		//clut_offset = ( ( tex->tex_trans.tex0.data >> 37 ) & 0x3fff  ) - tri_clut_base ;
		font_info[1].tex0.reg = SCE_GS_TEX0_1 ;
		font_info[1].tex0.data = tex->tex_trans.tex0.data ;
#if 0
		font_info[1].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
		font_info[1].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
														CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 0 );
#endif
		font_info[1].tex2.reg = SCE_GS_TEX2_1 ;
		font_info[1].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 4 );
		font_info[1].prim.reg = SCE_GS_PRIM ;
		font_info[1].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );

		/* デバッグ用フォント */
		//tex = DG_GetTexture2( TRI_CODE, TEXTURE2 );
		tex = &font_tex[ 2 ] ;
		DG_GetTexelInfo( &font_info[2].w, &font_info[2].h,
						  &font_info[2].x, &font_info[2].y, tex );
		font_info[2].tex = *tex ;
		font_info[2].u_scale = 1.0f / font_info[2].w ;
		font_info[2].v_scale = 1.0f / font_info[2].h ;
		font_info[2].x_space = 1 ;
		font_info[2].y_space = 1 ;
		font_info[2].width = 8 ;
		font_info[2].height = 16 ;
		font_info[2].pitch_table = font_pitch2 ;
		//clut_offset = ( ( tex->tex_trans.tex0.data >> 37 ) & 0x3fff  ) - tri_clut_base ;
		font_info[2].tex0.reg = SCE_GS_TEX0_1 ;
		font_info[2].tex0.data = tex->tex_trans.tex0.data ;
#if 0
		font_info[2].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
		font_info[2].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
														CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 0 );
#endif
		font_info[2].tex2.reg = SCE_GS_TEX2_1 ;
		font_info[2].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 4 );
		font_info[2].prim.reg = SCE_GS_PRIM ;
		font_info[2].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );

		//printf("%d %d %d %d\n", font_info[0].x, font_info[0].y, font_info[0].w, font_info[0].h );
		//printf("%d %d %d %d\n", font_info[1].x, font_info[1].y, font_info[1].w, font_info[1].h );
		//printf("%d %d %d %d\n", font_info[2].x, font_info[2].y, font_info[2].w, font_info[2].h );
	}

	/* アイコン関連の設定 */
	//tex = DG_GetTexture2( TRI_CODE, ICON_TEXTURE0 );
	tex = &icon_tex[ 0 ] ;
#if FALSE	// Old
<<<<<<< xmenupri.c
	if( tex ) {
		DG_GetTexelInfo( &icon_info[0].tex_width, &icon_info[0].tex_height,
						  &icon_info[0].offset_x, &icon_info[0].offset_y, tex );
		icon_info[0].tex = tex ;
		icon_info[0].x_num = 2 ;
		icon_info[0].y_num = 1 ;
		icon_info[0].tex0.reg = SCE_GS_TEX0_1 ;
		icon_info[1].tex0.data = tex->tex_trans.tex0.data ;
#else
	if( tex ) {
      DG_GetTexelInfo( &icon_info[0].tex_width, &icon_info[0].tex_height,
						  &icon_info[0].offset_x, &icon_info[0].offset_y, tex );
printf( "ICON %d %d %d %d\n", icon_info[0].tex_width, icon_info[0].tex_height,
						  icon_info[0].offset_x, icon_info[0].offset_y );
		icon_info[0].tex = *tex ;
		icon_info[0].x_num = 2 ;
		icon_info[0].y_num = 1 ;
		icon_info[0].tex0.reg = SCE_GS_TEX0_1 ;
		icon_info[0].tex0.data = tex->tex_trans.tex0.data ;
#endif
#if 0
		icon_info[0].tex0.data = tex->tex_trans.tex0.data & ( (u_long64)0x0f << 56 ) ;
		icon_info[0].tex0.data |= SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HL, 9, 8, 1, 0,
														CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 0 );
#endif
		icon_info[0].tex2.reg = SCE_GS_TEX2_1 ;
		icon_info[0].tex2.data = SCE_GS_SET_TEX2( SCE_GS_PSMT8, CLUT_BASE/*+clut_offset*/, SCE_GS_PSMCT32, 0, 0, 4 );
		icon_info[0].prim.reg = SCE_GS_PRIM ;
		icon_info[0].prim.data = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
	}
}
