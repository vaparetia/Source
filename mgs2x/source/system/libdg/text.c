//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	text.c
	テクスチャ管理ルーチン

	1999/07/07 K.Takabe
	$Id: text.c,v 1.1.1.3 2002/11/19 11:42:31 Yoshizawa1 Exp $

*/
/*

	----------------------------------------------------------------

	void DG_InitTextureCache( void )

		ロード時の初期化


	int DG_LoadInitTri( void *buf, int code )

		テクスチャの登録関数


	void DG_FreeTextureCache( void *buf )
	void	*buf ;		開放するＴＲＩアドレス

		任意テクスチャの開放（テクスチャ初期化をステージロードと非同期に行う場合に使用）


	----------------------------------------------------------------

	void DG_SetTextureTmpClut( int which )
	int		which ;		設定バッファ

		一時ＣＬＵＴイメージ情報の設定（システム内部で使用）


	void DG_ResetTextureTmpClut( void )

		一時ＣＬＵＴイメージ情報のクリア（システム内部で使用）

	----------------------------------------------------------------

	DG_TEX* DG_GetTexture( unsigned int code )
	unsigned int	code ;	検索テクスチャＩＤコード

		テクスチャ情報の取得（半透明用テクスチャ内からテクスチャを検索する）


	DG_TEX* DG_GetTexture2( unsigned int tri_code, unsigned int code )
	unsigned int	tri_code ;	検索対象ＴＲＩコード
	unsigned int	code ;		検索テクスチャＩＤコード

		テクスチャ情報の取得（任意ＴＲＩ内からテクスチャを検索）


	DG_TEX_PACKET* DG_GetTexturePacket( unsigned int code )
	unsigned int	code ;		取得するパケットのＴＲＩコード

		テクスチャ転送パケットのアドレスを取得


	DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code )
	unsigned int	code ;		取得するテクスチャリスト構造体のＴＲＩコード

		テクスチャリスト構造体のアドレスを取得

	----------------------------------------------------------------

	int	DG_SearchTriFromKms( DG_DEF *def )
	DG_DEF	*def ;		モデルデータ(*.kms)へのポインタ

		モデルに対応したＴＲＩ検索


	int	DG_SearchTriFromTex( DG_TEX *org )
	DG_TEX	*org ;		テクスチャ構造体へのポインタ

		テクスチャに対応したＴＲＩ検索

	----------------------------------------------------------------

	void *DG_MakeLoadImagePacket( void *tag_addr,
			int fmt, int width, int height,
			int vram_addr, int vram_width, void *tex_addr )
	void	*tag_addr ;		パケットを書き出すアドレス。内容はDG_LOADIMAGEと同じ構成になる
	int		fmt ;			転送カラーフォーマット指定(SCE_GS_PSM??)
	int		width ;			転送幅。上位１６ビットで転送先オフセットを指定可能
	int		height ;		転送高さ。上位１６ビットで転送先オフセットを指定可能
	int		vram_addr ;		転送先ベースアドレス（６４で割る前の値）
	int		vram_width ;	転送先ＶＲＡＭ幅（６４で割る前の値）
	void	*tex_addr ;		転送データアドレス

		テクスチャ転送パケットの作成
		width,heightの上位１６ビットで任意のオフセット位置から転送することも可能
		vram_addr,vram_widthはともに６４で割る前の値を指定すること
		戻り値にはパケットの次のアドレスが返す


	void *DG_MakeMoveImagePacket( void *tag_addr,
			int dst_addr, int dst_width, int dst_fmt, int dst_x, int dst_y,
			int src_addr, int src_width, int src_fmt, int src_x, int src_y,
			int width, int height, int dir )
	void	*tag_addr ;		パケットを書き出すアドレス。内容はDG_MOVEIMAGEと同じ構成になる
	int		dst_addr ;		転送先ベースアドレス（６４で割る前の値）
	int		dst_width ;		転送先ＶＲＡＭ幅（６４で割る前の値）
	int		dst_fmt ;		転送先カラーフォーマット指定(SCE_GS_PSM??)
	int		dst_x ;			転送先オフセット
	int		dst_y ;			転送先オフセット
	int		src_addr ;		転送元ベースアドレス（６４で割る前の値）
	int		src_width ;		転送元ＶＲＡＭ幅（６４で割る前の値）
	int		src_fmt ;		転送元カラーフォーマット指定(SCE_GS_PSM??)
	int		src_x ;			転送元オフセット
	int		src_y ;			転送元オフセット
	int		width ;			転送幅
	int		height ;		転送高さ
	int		dir ;			転送方向

		テクスチャ転送パケットの作成
		dst_addr,dst_width,src_addr,src_widthは６４で割る前の値を指定すること
		戻り値にはパケットの次のアドレスが返す


	void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex )
	int		*width ;	テクスチャ本来の幅を格納する変数へのポインタ（テクセル単位）
	int		*height ;	テクスチャ本来の高さを格納する変数へのポインタ（テクセル単位）
	int		*offset_x ;	テクスチャ本来のオフセットを格納する変数へのポインタ（テクセル単位）
	int		*offset_y ;	テクスチャ本来のオフセットを格納する変数へのポインタ（テクセル単位）
	DG_TEX	*tex ;		情報を取得するテクスチャ

		テクスチャのピクセル数を計算する
		（通常はバイリニアフィルタによる色の混合を避けるため0.5ピクセル内側を
		マッピングするようにしているため、この関数を用いないと正確なサイズなどがわからない）

	----------------------------------------------------------------

	DG_TEX_MOVEREPLACE *DG_MakeMoveReplacePacket( int tri_code, int dst_tex_id, int src_tex_id )
	int		tri_code ;		入れ替え対象ＴＲＩのコード
	int		dst_tex_id ;	入れ替え先テクスチャＩＤ
	int		src_tex_id ;	入れ替え元テクスチャＩＤ

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを生成する
		但し入れ替え元と入れ替え先は同じフォーマット、同じサイズでなければならない


	void DG_FreeMoveReplacePacket( DG_TEX_MOVEREPLACE *tex_replace )
	DG_TEX_MOVEREPLACE	*tex_replace ;	開放するパケット

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを開放する


	DG_TEX_LOADREPLACE *DG_MakeLoadReplacePacket( int tri_code, int dst_tex_id, void *texel, void *clut )

		データ再転送によるテクスチャ入れ替えパケットを生成する
		但し転送データは入れ替え先と同じフォーマット、サイズでなければならない


	void DG_FreeLoadReplacePacket( DG_TEX_LOADREPLACE *tex_replace )
	DG_TEX_LOADREPLACE	*tex_replace ;	開放するパケット

		データ再転送によるテクスチャ入れ替えパケットを開放する


	void DG_SetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace )
	DG_TEX_MOVEREPLACE	*tex_replace ;	入れ替えパケット

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを設定する


	void DG_ResetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace )
	DG_TEX_MOVEREPLACE	*tex_replace ;	入れ替えパケット

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを解除する


	void DG_SetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace )
	DG_TEX_LOADREPLACE	*tex_replace ;	入れ替えパケット

		データ再転送によるテクスチャ入れ替えパケットを設定する


	void DG_ResetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace )
	DG_TEX_LOADREPLACE	*tex_replace ;	入れ替えパケット

		データ再転送によるテクスチャ入れ替えパケットを解除する


	----------------------------------------------------------------
	----------------------------------------------------------------

*/

#include "BP_EndianSupport.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "libdg.cnf"
#include "libdg.h"
#include "def_dma.h"
#include "libfs.h"

#ifndef DESIGN_PREVIEW
//#define MAX_TEX_CACHE	(64)		/* 一度に読み込み可能なTRIファイル数 */
//#define MAX_TEX_CACHE	(80)		/* 一度に読み込み可能なTRIファイル数 */
#define MAX_TEX_CACHE	(128)		/* 一度に読み込み可能なTRIファイル数 */
#define MAX_TEX_BUFFER	(1024)		/* ゲーム中で使用できるテクスチャ枚数 */
#else
#define MAX_TEX_CACHE	(128)		/* 一度に読み込み可能なTRIファイル数 */
#define MAX_TEX_BUFFER	(4096)		/* ゲーム中で使用できるテクスチャ枚数 */
#endif

int				DG_MaxTextures = 0 ;
int				DG_MaxTexBuffer = 0 ;
DG_TEXTURE_LIST	*DG_TextureList[ MAX_TEX_CACHE ] ;
static int _tex_base = TEXTURE_TOP_PAGE() ;
int		DG_LastLoadTexture ;

#include "BP_Renderer.h"

DG_TEX			DG_DefaultTexture = {
	0.0f, 0.0f, 1.0f, 1.0f,
	0, 0, 0, /* BP_TextureHandle */ 0,
	{
		{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
		{ SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 ), SCE_GS_CLAMP_1 },
		{ 0, SCE_GS_TEX2_1 },
		{ 0, SCE_GS_TEX0_1 },
		{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
		{ 0, SCE_GS_NOP },
		{0,0,0,0},
		{0,0,0,0}
	}
};
/*
   DG_SystemTexture[0]:フレームバッファ０（512x512x32bit）
   DG_SystemTexture[1]:フレームバッファ１（512x512x32bit）
   DG_SystemTexture[2]:テクスチャバッファ（512x512x32bit）
   DG_SystemTexture[3]:テクスチャバッファ（512x512x16bit）
*/
DG_TEX			DG_SystemTexture[] = {
	{/* FrameBuffer0-32bit */
		0.0f, 0.0f, 1.0f, 1.0f,
		0, 0, 0, /* BP_TextureHandle */ 0,
		{
			{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( BUFFER_PAGE(0) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{ 0, SCE_GS_NOP },
			{0,0,0,0},
			{0,0,0,0}
		}
	},
	{/* FrameBuffer1-32bit */
		0.0f, 0.0f, 1.0f, 1.0f,
		0, 0, 0, /* BP_TextureHandle */ 0,
		{
			{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( BUFFER_PAGE(1) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{ 0, SCE_GS_NOP },
			{0,0,0,0},
			{0,0,0,0}
		}
	},
	{/* TextureBuffer-32bit */
		0.0f, 0.0f, 1.0f, 1.0f,
		0, 0, 0, /* BP_TextureHandle */ 0,
		{
			{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{ 0, SCE_GS_NOP },
			{0,0,0,0},
			{0,0,0,0}
		}
	},
	{/* Texture page 16bit */
		0.0f, 0.0f, 1.0f, 1.0f,
		0, 0, 0, /* BP_TextureHandle */ 0,
		{
			{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT16,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{ 0, SCE_GS_NOP },
			{0,0,0,0},
			{0,0,0,0}
		}
	},
};

// NOTE: Actual texture register data is fairly dummy data, this is just a placeholder to get it to use the correct BP textures!
DG_TEX BP_PreviousFrameTexture = 
{
   0.0f, 0.0f, 1.0f, 1.0f,
   kRT_PreviousFrameBuffer_NoMSAA, 0, DG_TEXFLAG_RENDERTARGET, /* BP_TextureHandle */ 0,
   { 
      { SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
      { SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
      { 0, SCE_GS_TEX2_1 },
      { SCE_GS_SET_TEX0( BUFFER_PAGE(0) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
      9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
      { SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
      { 0, SCE_GS_NOP },
      {0,0,0,0},
      {0,0,0,0}
   }
};

DG_TEX BP_CurrentFrameTexture = 
{
   0.0f, 0.0f, 1.0f, 1.0f,
   kRT_CurrentFrameBuffer, 0, DG_TEXFLAG_RENDERTARGET, /* BP_TextureHandle */ 0,
   {
      { SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 ), 0x0fffeeee },
      { SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
      { 0, SCE_GS_TEX2_1 },
      { SCE_GS_SET_TEX0( BUFFER_PAGE(0) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
      9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
      { SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
      { 0, SCE_GS_NOP },
      {0,0,0,0},
      {0,0,0,0}
   }
};

/* ---------------------------------------------------------------- */
static void LoadImageTriTextureData( DG_TEXTURE_LIST *tex_list, int init_flag );
static void DG_SetDefLoadTextureImage( DG_TEX_PACKET *tex, int base, int size, void *image, int clut_size, void *clut );
static void DG_SetNewTextureBuffer( int tri_code, DG_TEX *tex, u_long64 offset, int mode );

#include "BP_FileSupport.h"
#include "BP_BaseRenderer.h"

void BP_LoadTextureList(DG_TEXTURE_LIST* list, int doLoad)
{
   int i;

   for( i = 0; i < list->header->n_textures; ++i )
   {
      DG_TEX* pTexture = list->textures + i;
      
      if( doLoad )
      {
         BP_BindMGSTexture(BP_GetCurrentAssetCacheTag(), BP_GetCurrentLoadPath(), pTexture->tri_id, pTexture->tex_id, (unsigned int)pTexture);
      }

      pTexture->BP_TextureHandle = BP_GetTextureByMGSAddr((unsigned int)pTexture);
   }
}

void BP_DeleteTextureList(DG_TEXTURE_LIST* list)
{
   int i;

   for( i = 0; i < list->header->n_textures; ++i )
   {
      DG_TEX* pTexture = list->textures + i;
      pTexture->BP_TextureHandle = 0xdeadbeef;
   }
}
/* ---------------------------------------------------------------- */

static void EndianFixTriHeader( TRI_FILEHEADER *header )
{
   //KP_CTASSERT( sizeof( TRI_FILEHEADER ) == sizeof( int ) * 8 );

   BP_LE_SwapUInt_Inp( &( header->tex_offset 	      ) );
   BP_LE_SwapUInt_Inp( &( header->tex_size 		      ) );
   BP_LE_SwapUInt_Inp( &( header->clut_offset	      ) );
   BP_LE_SwapUInt_Inp( &( header->clut_size 		      ) );
   BP_LE_SwapUInt_Inp( &( header->n_textures 	      ) );
   BP_LE_SwapUInt_Inp( &( header->compress_flag       ) );
   BP_LE_SwapUInt_Inp( &( header->texel_addr 	      ) );
   BP_LE_SwapUInt_Inp( &( header->clut_addr 		      ) );
}

/* ---------------------------------------------------------------- */

static inline void EndianFixGSReg( DG_GSREG *pReg )
{
   BP_LE_SwapULongLong_Inp( &( pReg->data ) );
   BP_LE_SwapULongLong_Inp( &( pReg->reg ) );
}

/* ---------------------------------------------------------------- */

static inline void EndianFixFVECTOR( FVECTOR *pVec )
{
   BP_LE_SwapFloat_Inp( &( pVec->vx ) );
   BP_LE_SwapFloat_Inp( &( pVec->vy ) );
   BP_LE_SwapFloat_Inp( &( pVec->vz ) );
   BP_LE_SwapFloat_Inp( &( pVec->vw ) );
}

/* ---------------------------------------------------------------- */

static void EndianFixDGTex( DG_TEX *_tex, unsigned int const count )
{
   unsigned i;
   for ( i = 0; i < count; ++i )
   {
      DG_TEX *tex = _tex + i;

      BP_LE_SwapFloat_Inp( &( tex->u_offset ) );
      BP_LE_SwapFloat_Inp( &( tex->v_offset ) );
      BP_LE_SwapFloat_Inp( &( tex->u_scale ) );
      BP_LE_SwapFloat_Inp( &( tex->v_scale ) );
      BP_LE_SwapUInt_Inp( &( tex->tex_id ) );
      BP_LE_SwapUInt_Inp( &( tex->tri_id ) );
      BP_LE_SwapUInt_Inp( &( tex->BP_flag ) );
      // only keep the injected flag
      tex->BP_flag &= DG_TEXFLAG_INJECTED_TEX;
      BP_LE_SwapUInt_Inp( &( tex->BP_TextureHandle ) );

      BP_LE_SwapULongLong_Inp( &( tex->tex_trans.giftag.tag ) );
      BP_LE_SwapULongLong_Inp( &( tex->tex_trans.giftag.regs ) );

      EndianFixGSReg( &( tex->tex_trans.tex0 ) );
      EndianFixGSReg( &( tex->tex_trans.tex2 ) );
      EndianFixGSReg( &( tex->tex_trans.alpha ) );
      EndianFixGSReg( &( tex->tex_trans.clamp ) );

      EndianFixFVECTOR( &( tex->tex_trans.vec1 ) );
      EndianFixFVECTOR( &( tex->tex_trans.vec2 ) );
   }
}

/* ---------------------------------------------------------------- */

/**
	ロード時の初期化
*/
void DG_InitTextureCache( void )
{
	int i;
	for( i = 0; i < DG_MaxTextures; i++ )
   {
      BP_DeleteTextureList(DG_TextureList[i]);
		GV_Free( DG_TextureList[ i ] );
		DG_TextureList[ i ] = NULL;
	}
	DG_MaxTextures = 0 ;
	DG_MaxTexBuffer = 0 ;
	_tex_base = TEXTURE_TOP_PAGE() ;
}

	/*
		テクスチャの登録
	*/

void FixUVPullIn( DG_TEXTURE_LIST *list )
{
   TRI_FILEHEADER	*header = list->header;
   // TODO - convert these strcode calls to constants, when I have a working PC debug build
   // from a texture, the tri code is the folder name in the .xmlmeta and the tri name is in bp_flatlistassetmapping.txt
   if (list->code == GV_StrCode("node_menu")
      || list->code == GV_StrCode("node_menu_stage_n_select")
      || list->code == GV_StrCode("boss_opt")
      || list->code == GV_StrCode("node_name")
      || list->code == GV_StrCode("vr")
      || list->code == GV_StrCode("vr_window")
      || list->code == GV_StrCode("vr_mission_cleared"))
   {
      unsigned i;
      for ( i = 0; i < header->n_textures; ++i )
      {
         DG_TEX *tex = list->textures + i;
         int tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f;
         int th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f;
         int w = 1 << tw;
         int h = 1 << th;
         int width, height, offset_x, offset_y;

         /* This is how these were originally calculated.
         texture->u_offset = ( (float)src_tex->pix_x + 0.5F ) / tex_width ;
         texture->v_offset = ( (float)src_tex->pix_y + 0.5F ) / tex_height ;
         texture->u_scale = ( (float)src_tex->width - 1.0F ) / tex_width ;
         texture->v_scale = ( (float)src_tex->height - 1.0F ) / tex_height ;
         */

         // need to rework to get rid of the 0.5 and -1.0
         DG_GetTexelInfo(&width, &height, &offset_x, &offset_y, tex);

         tex->u_offset = ((float)offset_x)/w;
         tex->v_offset = ((float)offset_y)/h;
         tex->u_scale = ((float)width)/w;
         tex->v_scale = ((float)height)/h;

         tex->BP_flag |= DG_TEXFLAG_NO_UV_PULLIN;
      }
   }
}

static void FixupDgTex( DG_TEXTURE_LIST *list )
{
   if (list->code == 0x0014de14 /*GV_StrCode("title_menu")*/
      || list->code == 0x00899c71 /*GV_StrCode("menuicon")*/
      || list->code == 0x0099b405 /*GV_StrCode("node_menu_stage_n_title")*/)
   {
      unsigned i;
      TRI_FILEHEADER	*header = list->header;
      for ( i = 0; i < header->n_textures; ++i )
      {
         DG_TEX *tex = list->textures + i;

         if (tex->BP_flag & DG_TEXFLAG_INJECTED_TEX)
         {
            // rework the offsets and scales
            tex->u_offset = 0.0f;
            tex->v_offset = 0.0f;
            tex->u_scale = 1.0f;
            tex->v_scale = 1.0f;
            tex->BP_flag |= DG_TEXFLAG_NO_UV_PULLIN;
         }
      }
   }
   //For Transfarring
   if (list->code == 0x009deed4 /*GV_StrCode("save_load")*/
      || list->code == 0x0096a917 /*GV_StrCode("photo_save")*/) 
   {
      unsigned i;
      TRI_FILEHEADER	*header = list->header;
      for ( i = 0; i < header->n_textures; ++i )
      {
         DG_TEX *tex = list->textures + i;

         if (tex->BP_flag & DG_TEXFLAG_INJECTED_TEX)
         {
            // rework the offsets and scales
            tex->u_offset = 0.0f;
            tex->v_offset = 0.0f;
            tex->u_scale = 1.0f;
            tex->v_scale = 1.0f;
            tex->BP_flag |= DG_TEXFLAG_NO_UV_PULLIN;

            //No wrapping
            tex->BP_flag |= DG_TEXFLAG_UV_CLAMP;
         }
      }
   }
}

int DG_LoadInitTriBuf( void *buf, int code, int cache_mode, DG_TEXTURE_LIST *list )
{
	TRI_FILEHEADER	*header ;
	int					init_flag = 0 ;

	if ( code & GV_CACHEID_RESIDENT ){
		init_flag = 1 ;
	}
	ASSERT( DG_MaxTextures < MAX_TEX_CACHE );
	code &= 0xffffff ;
   printf( "Loadinit TRI addr=%08x code=%08x\n", buf, code );
//	list = &DG_TextureList[ DG_MaxTextures++ ];
	DG_TextureList[ DG_MaxTextures ++ ] = list;

	//memset( list, 0, sizeof( DG_TEXTURE_LIST ) );
	GV_ZeroMemory( list, sizeof( DG_TEXTURE_LIST ) );
	/* テンポラリのワークに一度全部読み込む */
	list->header = header = buf ;

	list->textures = (DG_TEX*)&header[1] ;
	list->code = code ;

   // Don't endian swap if we're just reinitializing the resident area again.
   if( cache_mode != GV_INIT_RESIDENT_AGAIN )
   {
      EndianFixTriHeader( header );
      EndianFixDGTex( list->textures, header->n_textures );
      FixUVPullIn( list );
      FixupDgTex( list );
   }

	list->texture_image = (void*)( (int)header->texel_addr + (int)buf ) ;
	list->clut_image = (void*)( (int)header->clut_addr + (int)buf ) ;
	list->tex_size = ( list->header->tex_size + list->header->clut_size ) * 64 * 4 ;

	LoadImageTriTextureData( list, init_flag );
	DG_LastLoadTexture = code ;

#ifdef DESIGN_PREVIEW
	printf("texture memory: %d %d\n", DG_MaxTextures, DG_MaxTexBuffer);
#endif

   BP_LoadTextureList(list, (cache_mode != GV_INIT_RESIDENT_AGAIN) ? 1 : 0 );

	return (1);
}

int DG_LoadInitTri( void *buf, int code, int cache_mode )
{
	DG_TEXTURE_LIST *list;

	list = GV_Malloc( sizeof( DG_TEXTURE_LIST ) );
	ASSERT( list != 0 );
	return DG_LoadInitTriBuf( buf, code, cache_mode, list );
}

	/*
		任意テクスチャの開放
	*/
void *DG_FreeTextureCache( void *buf )
{
	int		i ;
	void *res;
//	DG_TEXTURE_LIST	*tex_list ;
	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		if ( (int)( DG_TextureList[ i ]->header ) == (int)buf ){
			res = DG_TextureList[ i ];
			for ( ; i < ( DG_MaxTextures - 1 ) ; i++ ){
				DG_TextureList[ i ] = DG_TextureList[ i + 1 ] ;
			}
			DG_MaxTextures-- ;
			return res;
		}
	}
	return NULL;
}

/* ---------------------------------------------------------------- */
	/*
		一時ＣＬＵＴイメージ情報の設定
	*/
void DG_SetTextureTmpClut( int which )
{
	int		i ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;

	tex_list_p = DG_TextureList ;
	for ( i = DG_MaxTextures; i > 0 ; i--, tex_list_p++ ){
		/* 一時ＣＬＵＴイメージ使用チェック */
		tex_list = *tex_list_p;
		if ( tex_list->tmp_clut_image == NULL || tex_list->header->compress_flag & TRI_FLAG_STATIC ){
			tex_list->tex_packet[which].clut.dmatag1.addr = tex_list->clut_image ;
		} else {
			tex_list->tex_packet[which].clut.dmatag1.addr = tex_list->tmp_clut_image ;
		}
	}
	return ;
}

	/*
		一時ＣＬＵＴイメージ情報のクリア
	*/
void DG_ResetTextureTmpClut( void )
{
	int		i ;

	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		DG_TextureList[ i ]->tmp_clut_image = NULL ;
	}
	return ;
}

/* ---------------------------------------------------------------- */
	/*
		テクスチャ情報の取得
	*/
DG_TEX* DG_GetTexture( unsigned int code )
{
	int		i, j ;
	DG_TEX	*tex ;

	/* 半透明テクスチャ内から検索 */
	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		if ( DG_TextureList[ i ]->header->compress_flag & TRI_FLAG_TRANS ){
			tex = DG_TextureList[ i ]->textures ;
			for ( j = DG_TextureList[ i ]->header->n_textures ; j > 0 ; j--, tex++ ){
				if ( tex->tex_id == code ) return ( tex );
			}
		}
	}

	printf("search texture error (code=%08x)\n", code );
#ifdef DEBUG
	if( (code!=0)  &&  (FS_MediaType() == FS_MEDIA_DEV) ){
		ASSERT(0);
	}
#endif
	/* 不透明テクスチャ内から検索 */
	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		if ( !( DG_TextureList[ i ]->header->compress_flag & TRI_FLAG_TRANS ) ){
			tex = DG_TextureList[ i ]->textures ;
			for ( j = DG_TextureList[ i ]->header->n_textures ; j > 0 ; j--, tex++ ){
				if ( tex->tex_id == code ) return ( tex );
			}
		}
	}
	if ( DG_MaxTextures != 0 ) return DG_TextureList[ 0 ]->textures ;
	return ( NULL );
}

	/*
		テクスチャ情報の取得
	*/
DG_TEX* DG_GetTexture2( unsigned int tri_code, unsigned int code )
{
	int		i, j ;
	DG_TEX	*tex ;

	/* 指定ＴＲＩ内でのサーチ */
	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		if ( DG_TextureList[ i ]->code != tri_code ) continue ;
		tex = DG_TextureList[ i ]->textures ;
		for ( j = DG_TextureList[ i ]->header->n_textures ; j > 0 ; j--, tex++ ){
			if ( tex->tex_id == code ) return ( tex );
		}
	}
	/* 半透明ＴＲＩ内でのサーチ */
	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		if ( !( DG_TextureList[ i ]->header->compress_flag & TRI_FLAG_TRANS ) ) continue ;
		tex = DG_TextureList[ i ]->textures ;
		for ( j = DG_TextureList[ i ]->header->n_textures ; j > 0 ; j--, tex++ ){
			if ( tex->tex_id == code ) return ( tex );
		}
	}
	printf("search texture error (tri=%08x, code=%08x)\n", tri_code, code );
#ifdef DEBUG
	if( (code!=0)  &&  (FS_MediaType() == FS_MEDIA_DEV) ){
		ASSERT(0);
	}
#endif
	if ( DG_MaxTextures != 0 ) return DG_TextureList[ 0 ]->textures ;
	return ( NULL );
}

	/*
		テクスチャ転送パケットのアドレスを取得
	*/
DG_TEX_PACKET* DG_GetTexturePacket( unsigned int code )
{
	int		i ;
	DG_TEXTURE_LIST	**tex_list_p ;

	tex_list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0; i--, tex_list_p++ ){
		if ( ( *tex_list_p )->code == code ){
			return ( (void*)&( *tex_list_p )->tex_packet[DG_Clock] );
		}
	}
	return NULL ;
}

	/*
		テクスチャリスト構造体のアドレスを取得
	*/
DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code )
{
	int		i ;
	DG_TEXTURE_LIST	**tex_list_p ;

	tex_list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0; i--, tex_list_p++ ){
		if ( ( ( *tex_list_p )->code & 0x00ffffff ) == code ){
			return ( (void*)( *tex_list_p ) );
		}
	}
	printf("not found tri !!(%d)\n", code );
	return NULL ;
}

/* ---------------------------------------------------------------- */

	/*
		モデルに対応したＴＲＩ検索
	*/
int	DG_SearchTriFromKms( DG_DEF *def )
{
	return ( def->texture );
}

	/*
		テクスチャに対応したＴＲＩ検索
	*/
int	DG_SearchTriFromTex( DG_TEX *org )
{
	return ( org->tri_id );
}

/* ---------------------------------------------------------------- */
	/* 
		テクスチャ転送パケットの作成
		width,heightの上位１６ビットで任意のオフセット位置から転送することも可能
		vram_addr,vram_widthはともに６４で割る前の値を指定すること
	*/
void *DG_MakeLoadImagePacket( void *tag_addr, int fmt, int width, int height, int vram_addr, int vram_width, void *tex_addr )
{
	/* 使用例
		tag_mem = DG_LoadImagePacket( tag_mem,
			SCE_GS_PSMCT32, 256, 256, TEXTURE_TOP_ADDR(), 256, tex_image );
	*/
	DG_LOADIMAGE	*loadimage ;
	int			oft_x, oft_y, dma_size ;

	oft_x = width >> 16 ;
	oft_y = height >> 16 ;
	width &= 0xffff ;
	height &= 0xffff ;

	/* フォーマットからパラメータ決定 */
	switch ( fmt ){
	  case SCE_GS_PSMCT32:
		dma_size = width * height * 4 / 16 ;
		break ;
	  case SCE_GS_PSMCT16:
	  case SCE_GS_PSMCT16S:
		dma_size = width * height * 2 / 16 ;
		break ;
	  case SCE_GS_PSMT8:
	  case SCE_GS_PSMT8H:
		dma_size = width * height / 16 ;
		break ;
	  case SCE_GS_PSMT4:
	  case SCE_GS_PSMT4HL:
	  case SCE_GS_PSMT4HH:
		dma_size = width * height / 2 / 16 ;
		break ;
	  default:
		dma_size = width * height * 4 / 16 ;
		break ;
	}

	loadimage = (DG_LOADIMAGE*)tag_addr ;
	/* ＧＩＦ設定パケット転送用ＤＭＡタグ用意 */
	loadimage->dmatag0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 6 ) ;
	loadimage->dmatag0.addr = NULL ;
	loadimage->dmatag0.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	loadimage->dmatag0.vifcode[1] = SCE_VIF1_SET_DIRECT( 6 , 0 ) ;
	/* ＧＩＦ設定パケット用ＧＩＦタグ用意 */
	loadimage->giftag0.tag = SCE_GIF_SET_TAG( 4, 0, 0, 0, SCE_GIF_PACKED, 1 );
	loadimage->giftag0.regs = 0x0000000e ;
	/* イメージ転送用設定 */
	loadimage->bitbltbuf.reg = SCE_GS_BITBLTBUF ;
	loadimage->bitbltbuf.data = SCE_GS_SET_BITBLTBUF( 0, 0, 0, vram_addr/64, vram_width/64, fmt ) ;
	loadimage->trxpos.reg = SCE_GS_TRXPOS ;
	loadimage->trxpos.data = SCE_GS_SET_TRXPOS( 0, 0, oft_x, oft_y, 0 ) ;
	loadimage->trxreg.reg = SCE_GS_TRXREG ;
	loadimage->trxreg.data = SCE_GS_SET_TRXREG( width, height ) ;
	loadimage->trxdir.reg = SCE_GS_TRXDIR ;
	loadimage->trxdir.data = SCE_GS_SET_TRXDIR( 0 ) ;
	/* イメージ転送用ＧＩＦタグ用意 */
	loadimage->giftag1.tag = SCE_GIF_SET_TAG( dma_size, 1, 0, 0, SCE_GIF_IMAGE, 0 );
	loadimage->giftag1.regs = 0x00000000 ;
	/* イメージ転送用ＤＭＡタグ用意 */
	loadimage->dmatag1.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, dma_size ) ;
	loadimage->dmatag1.addr = tex_addr ;
	loadimage->dmatag1.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	if ( dma_size != 0 ){
		loadimage->dmatag1.vifcode[1] = SCE_VIF1_SET_DIRECT( dma_size , 0 ) ;
	} else {
		loadimage->dmatag1.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	}

	return ( &loadimage[1] );

}

	/*
		テクスチャ転送パケットの作成
		dst_addr,dst_width,src_addr,src_widthは６４で割る前の値を指定すること
	*/
void *DG_MakeMoveImagePacket( void *tag_addr,
							 int dst_addr, int dst_width, int dst_fmt, int dst_x, int dst_y,
							 int src_addr, int src_width, int src_fmt, int src_x, int src_y,
							 int width, int height, int dir )
{
	/* 使用例
		tag_mem = DG_MakeMoveImagePacket( tag_mem,
					ZBUFFER_PAGE(), 512, SCE_GS_PSMT8H, 0, 0,
					TEXTURE_PAGE(), 512, SCE_GS_PSMT8 , 0, 0,
					512, 256, 0 );
	*/
	DG_MOVEIMAGE	*moveimage ;

	moveimage = (DG_MOVEIMAGE*)tag_addr ;
	/* ＧＩＦ設定パケット転送用ＤＭＡタグ用意 */
	moveimage->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 5 ) ;
	moveimage->dmatag.addr = NULL ;
	moveimage->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	moveimage->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( 5 , 0 ) ;
	/* ＧＩＦ設定パケット用ＧＩＦタグ用意 */
	moveimage->giftag.tag = SCE_GIF_SET_TAG( 4, 1, 0, 0, SCE_GIF_PACKED, 1 );
	moveimage->giftag.regs = 0x0000000e ;
	/* イメージ転送用設定 */
	moveimage->bitbltbuf.reg = SCE_GS_BITBLTBUF ;
	moveimage->bitbltbuf.data = SCE_GS_SET_BITBLTBUF( src_addr/64, src_width/64, src_fmt,
													 dst_addr/64, dst_width/64, dst_fmt ) ;
	moveimage->trxpos.reg = SCE_GS_TRXPOS ;
	moveimage->trxpos.data = SCE_GS_SET_TRXPOS( src_x, src_y, dst_x, dst_y, dir ) ;
	moveimage->trxreg.reg = SCE_GS_TRXREG ;
	moveimage->trxreg.data = SCE_GS_SET_TRXREG( width, height ) ;
	moveimage->trxdir.reg = SCE_GS_TRXDIR ;
	moveimage->trxdir.data = SCE_GS_SET_TRXDIR( 2 ) ;

	return ( &moveimage[1] );

}

/* ---------------------------------------------------------------- */
	/*
		テクスチャのピクセル数を計算する
	*/
void DG_GetTexelInfo_NoUVPullIn( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex )
{
   int tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f;
   int th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f;
   int w = 1 << tw;
   int h = 1 << th;
   if (width != NULL)
   {
      // round to nearest int
      *width = (int)(w * tex->u_scale + 0.5f);
   }
   if (height != NULL)
   {
      *height = (int)(h * tex->v_scale + 0.5f);
   }
   if (offset_x != NULL)
   {
      *offset_x = (int)(w * tex->u_offset + 0.5f);
   }
   if (offset_y != NULL)
   {
      *offset_y = (int)(h * tex->v_offset + 0.5f);
   }
}

void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex )
{
   if (tex->BP_flag & DG_TEXFLAG_NO_UV_PULLIN)
   {
      DG_GetTexelInfo_NoUVPullIn(width, height, offset_x, offset_y, tex);
   }
   else
   {
      int		w, h, tw, th, x, y ;
      tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
      th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
      w = 1 << tw ;
      h = 1 << th ;
      x = DG_FTOI( (float)w * tex->u_scale + ( 1.0f + 0.1f/* 誤差吸収の為 */ ) ) ;
      y = DG_FTOI( (float)h * tex->v_scale + ( 1.0f + 0.1f/* 誤差吸収の為 */ ) ) ;
      if ( width != NULL ) *width = x ;
      if ( height != NULL ) *height = y ;
      x = DG_FTOI( (float)w * tex->u_offset - ( 0.5f - 0.1f/* 誤差吸収の為 */ ) ) ;
      y = DG_FTOI( (float)h * tex->v_offset - ( 0.5f - 0.1f/* 誤差吸収の為 */ ) ) ;
      if ( offset_x != NULL ) *offset_x = x ;
      if ( offset_y != NULL ) *offset_y = y ;
   }
}

void DG_GetTexelInfo_BP( int *width, int *height, int *offset_x, int *offset_y, DG_TEX_BP *tex )
{
   int		w, h, tw, th, x, y ;
   tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
   th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
   w = 1 << tw ;
   h = 1 << th ;
   x = DG_FTOI( (float)w * tex->u_scale + ( 1.0f + 0.1f/* 誤差吸収の為 */ ) ) ;
   y = DG_FTOI( (float)h * tex->v_scale + ( 1.0f + 0.1f/* 誤差吸収の為 */ ) ) ;
   if ( width != NULL ) *width = x ;
   if ( height != NULL ) *height = y ;
   x = DG_FTOI( (float)w * tex->u_offset - ( 0.5f - 0.1f/* 誤差吸収の為 */ ) ) ;
   y = DG_FTOI( (float)h * tex->v_offset - ( 0.5f - 0.1f/* 誤差吸収の為 */ ) ) ;
   if ( offset_x != NULL ) *offset_x = x ;
   if ( offset_y != NULL ) *offset_y = y ;
}

/* ---------------------------------------------------------------- */
	/*
		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを生成する
	*/
DG_TEX_MOVEREPLACE *DG_MakeMoveReplacePacket( int tri_code, int dst_tex_id, int src_tex_id )
{
	DG_TEX_MOVEREPLACE	*tex_replace ;
	DG_TEX				*dst_tex, *src_tex ;
	DG_TEXTURE_LIST		*tex_list ;
	int		dst_base, dst_width, dst_fmt, dst_x, dst_y, dst_w, dst_h, dst_offset ;
	int		src_base, src_width, src_fmt, src_x, src_y, src_offset ;

	if ( ( tex_list = DG_GetTextureList( tri_code ) ) == NULL ) return ( NULL );
	if ( ( dst_tex = DG_GetTexture2( tri_code, dst_tex_id ) ) == NULL ) return ( NULL );
	if ( ( src_tex = DG_GetTexture2( tri_code, src_tex_id ) ) == NULL ) return ( NULL );

	if ( ( tex_replace = GV_Malloc( sizeof(DG_TEX_MOVEREPLACE) ) ) == NULL ) return ( NULL );

	/* テクセルの移動パケット生成 */
	dst_base = dst_tex->tex_trans.tex0.data & 0x3fff ;
	dst_width = ( dst_tex->tex_trans.tex0.data >> 14 ) & 0x3f ;
	dst_fmt = ( dst_tex->tex_trans.tex0.data >> 20 ) & 0x3f ;
	src_base = src_tex->tex_trans.tex0.data & 0x3fff ;
	src_width = ( src_tex->tex_trans.tex0.data >> 14 ) & 0x3f ;
	src_fmt = ( src_tex->tex_trans.tex0.data >> 20 ) & 0x3f ;
	DG_GetTexelInfo( &dst_w, &dst_h, &dst_x, &dst_y, dst_tex );
	DG_GetTexelInfo( NULL, NULL, &src_x, &src_y, src_tex );
	//printf("texel dst %08x %d %d,x:%d y:%d w:%d h:%d\n",
	//	   dst_base, dst_width, dst_fmt, dst_x, dst_y, dst_w, dst_h );
	//printf("texel src %08x %d %d,x:%d y:%d\n",
	//	   src_base, src_width, src_fmt, src_x, src_y );
	DG_MakeMoveImagePacket( &tex_replace->texel_move,
						   dst_base*64, dst_width*64, dst_fmt, dst_x, dst_y,
						   src_base*64, src_width*64, src_fmt, src_x, src_y,
						   dst_w, dst_h, 0 );
	/* ＣＬＵＴの移動パケット生成 */
	if ( dst_fmt == SCE_GS_PSMT8 ){
		/* 256パレット時 */
		dst_w = 16 ;
		dst_h = 16 ;
	} else {
		/* 16パレット時 */
		dst_w = 8 ;
		dst_h = 2 ;
	}
	dst_base = ( dst_tex->tex_trans.tex0.data >> 37 ) & 0x3fff ;
	dst_width = 1 ;
	dst_fmt = ( dst_tex->tex_trans.tex0.data >> 51 ) & 0xf ;
	dst_offset = ( dst_tex->tex_trans.tex0.data >> 56 ) & 0x1f ;
	dst_x = ( dst_offset & 1 ) ? 8 : 0 ;
	dst_y = dst_offset & ~1 ;
	src_base = ( src_tex->tex_trans.tex0.data >> 37 ) & 0x3fff ;
	src_width = 1 ;
	src_fmt = ( src_tex->tex_trans.tex0.data >> 51 ) & 0xf ;
	src_offset = ( src_tex->tex_trans.tex0.data >> 56 ) & 0x1f ;
	src_x = ( src_offset & 1 ) ? 8 : 0 ;
	src_y = src_offset & ~1 ;
	//printf("clut dst %08x %d %d,x:%d y:%d w:%d h:%d\n",
	//	   dst_base, dst_width, dst_fmt, dst_x, dst_y, dst_w, dst_h );
	//printf("clut src %08x %d %d,x:%d y:%d\n",
	//	   src_base, src_width, src_fmt, src_x, src_y );
	DG_MakeMoveImagePacket( &tex_replace->clut_move,
						   dst_base*64, dst_width*64, dst_fmt, dst_x, dst_y,
						   src_base*64, src_width*64, src_fmt, src_x, src_y,
						   dst_w, dst_h, 0 );
	/* ＤＭＡ終端の書き込み */
	tex_replace->rettag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	tex_replace->rettag.addr = NULL ;
	tex_replace->rettag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	tex_replace->rettag.vifcode[1] = SCE_VIF1_SET_NOP( 0 );

	tex_replace->tex_list = tex_list ;
	tex_replace->prev = NULL ;
	tex_replace->next = NULL ;

	return ( tex_replace );
}

	/*
		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを開放する
	*/
void DG_FreeMoveReplacePacket( DG_TEX_MOVEREPLACE *tex_replace )
{
	GV_DelayedFree( tex_replace );
}

	/*
		データ再転送によるテクスチャ入れ替えパケットを生成する
	*/
DG_TEX_LOADREPLACE *DG_MakeLoadReplacePacket( int tri_code, int dst_tex_id, void *texel, void *clut )
{
	DG_TEX_LOADREPLACE	*tex_replace ;
	DG_TEX				*dst_tex ;
	DG_TEXTURE_LIST		*tex_list ;
	int		dst_base, dst_width, dst_fmt, dst_x, dst_y, dst_w, dst_h, dst_offset ;

	if ( ( tex_list = DG_GetTextureList( tri_code ) ) == NULL ) return ( NULL );
	if ( ( dst_tex = DG_GetTexture2( tri_code, dst_tex_id ) ) == NULL ) return ( NULL );

	if ( ( tex_replace = GV_Malloc( sizeof(DG_TEX_LOADREPLACE) ) ) == NULL ) return ( NULL );

	/* テクセルの転送パケット生成 */
	dst_base = dst_tex->tex_trans.tex0.data & 0x3fff ;
	dst_width = ( dst_tex->tex_trans.tex0.data >> 14 ) & 0x3f ;
	dst_fmt = ( dst_tex->tex_trans.tex0.data >> 20 ) & 0x3f ;
	DG_GetTexelInfo( &dst_w, &dst_h, &dst_x, &dst_y, dst_tex );
	DG_MakeLoadImagePacket( &tex_replace->texel_load, dst_fmt,
						   dst_w | ( dst_x << 16 ), dst_h | ( dst_y << 16 ),
						   dst_base*64, dst_width*64, texel );
	/* ＣＬＵＴの転送パケット生成 */
	if ( dst_fmt == SCE_GS_PSMT8 ){
		/* 256パレット時 */
		dst_w = 16 ;
		dst_h = 16 ;
	} else {
		/* 16パレット時 */
		dst_w = 8 ;
		dst_h = 2 ;
	}
	dst_base = ( dst_tex->tex_trans.tex0.data >> 37 ) & 0x3fff ;
	dst_width = 1 ;
	dst_fmt = ( dst_tex->tex_trans.tex0.data >> 51 ) & 0xf ;
	dst_offset = ( dst_tex->tex_trans.tex0.data >> 56 ) & 0x1f ;
	dst_x = ( dst_offset & 1 ) ? 8 : 0 ;
	dst_y = dst_offset & ~1 ;
	DG_MakeLoadImagePacket( &tex_replace->clut_load, dst_fmt,
						   dst_w | ( dst_x << 16 ), dst_h | ( dst_y << 16 ),
						   dst_base*64, dst_width*64, clut );
	/* ＤＭＡ終端の書き込み */
	tex_replace->rettag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	tex_replace->rettag.addr = NULL ;
	tex_replace->rettag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	tex_replace->rettag.vifcode[1] = SCE_VIF1_SET_NOP( 0 );

	tex_replace->tex_list = tex_list ;
	tex_replace->prev = NULL ;
	tex_replace->next = NULL ;

   // JM(AS) - Make sure that the tex_list doesn't have stale load_link data!!!!
   tex_list->load_link = NULL;

	return ( tex_replace );
}

	/*
		データ再転送によるテクスチャ入れ替えパケットを開放する
	*/
void DG_FreeLoadReplacePacket( DG_TEX_LOADREPLACE *tex_replace )
{
	GV_DelayedFree( tex_replace );
}

/* ---------------------------------------------------------------- */

	/*
		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを設定する
	*/
void DG_SetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace )
{
	DG_TEX_MOVEREPLACE	*current ;
	DG_TEXTURE_LIST		*tex_list ;

	if ( tex_replace == NULL ) return ;
	tex_list = tex_replace->tex_list ;

	/* リンクリストに接続 */
	if ( tex_list->move_link == NULL ){
		/* １個目 */
		tex_list->move_link = tex_replace ;
		tex_replace->prev = NULL ;
		tex_replace->next = NULL ;
	} else {
		/* ２個目以降 */
		current = tex_list->move_link ;
		while ( current->next != NULL ){
			current = current->next ;
		}
		current->next = tex_replace ;
		tex_replace->prev = current ;
		tex_replace->next = NULL ;
	}
}

	/*
		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを解除する
	*/
void DG_ResetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace )
{
	DG_TEX_MOVEREPLACE	*prev, *next ;
	DG_TEXTURE_LIST		*tex_list ;

	if ( tex_replace == NULL ) return ;
	tex_list = tex_replace->tex_list ;

	/* リンクの切り離し */
	prev = tex_replace->prev ;
	next = tex_replace->next ;
	if ( prev != NULL ){
		prev->next = next ;
	}
	if ( next != NULL ){
		next->prev = prev ;
	}
	/* ＴＲＩ管理構造体からのリンク更新 */
	if ( prev == NULL ){
		tex_list->move_link = next ;
	}
}

	/*
		データ再転送によるテクスチャ入れ替えパケットを設定する
	*/
void DG_SetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace )
{
	DG_TEX_LOADREPLACE	*current ;
	DG_TEXTURE_LIST		*tex_list ;

	if ( tex_replace == NULL ) return ;
	tex_list = tex_replace->tex_list ;

	/* リンクリストに接続 */
	if ( tex_list->load_link == NULL ){
		/* １個目 */
		tex_list->load_link = tex_replace ;
		tex_replace->prev = NULL ;
		tex_replace->next = NULL ;
	} else {
		/* ２個目以降 */
		current = tex_list->load_link ;
		while ( current->next != NULL ){
			current = current->next ;
		}
		current->next = tex_replace ;
		tex_replace->prev = current ;
		tex_replace->next = NULL ;
	}
}

	/*
		データ再転送によるテクスチャ入れ替えパケットを解除する
	*/
void DG_ResetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace )
{
	DG_TEX_LOADREPLACE	*prev, *next ;
	DG_TEXTURE_LIST		*tex_list ;

	if ( tex_replace == NULL ) return ;
	tex_list = tex_replace->tex_list ;

	/* リンクの切り離し */
	prev = tex_replace->prev ;
	next = tex_replace->next ;
	if ( prev != NULL ){
		prev->next = next ;
	}
	if ( next != NULL ){
		next->prev = prev ;
	}
	/* ＴＲＩ管理構造体からのリンク更新 */
	if ( prev == NULL ){
		tex_list->load_link = next ;
	}
}



/* ---------------------------------------------------------------- */
	/*
		ミップマップ情報生成
	*/
void DG_MakeMipmap( DG_MIPMAP *mipmap )
{
	int		i ;
	u_long64	bp, bw ;
	int		bpl[7], bwl[7] ;

	for ( i = 0 ; i <= mipmap->max_level ; i++ ){
		if ( mipmap->tex[i] == NULL ){
			mipmap->tex[i] = DG_GetTexture2( mipmap->tri_id, mipmap->tex_id[i] );
		}
		bp = mipmap->tex[i]->tex_trans.tex0.data & 0x3fff ;
		bw = ( mipmap->tex[i]->tex_trans.tex0.data >> 14 ) & 0x3f ;
		bpl[i] = (int)bp ;
		bwl[i] = (int)bw ;
	}
	mipmap->miptbp1.data = SCE_GS_SET_MIPTBP1( bpl[1], bwl[1], bpl[2], bwl[2], bpl[3], bwl[3] );
	mipmap->miptbp2.data = SCE_GS_SET_MIPTBP2( bpl[4], bwl[4], bpl[5], bwl[5], bpl[6], bwl[6] );
}
void DG_SetMipmapLodParam( DG_MIPMAP *mipmap, float screen )
{
	int		lod_k ;

	lod_k = DG_FTOI( -16.0f * logf( mipmap->z0 * screen ) / 0.69314718055994530941723212145818f/*logf( 2 )*/ ) ;
	lod_k <<= mipmap->lod_l ;
	lod_k &= 0xfff ;
	mipmap->tex1.data = SCE_GS_SET_TEX1( 0, mipmap->max_level, 1, 5, 0, mipmap->lod_l, lod_k );
}

/* ---------------------------------------------------------------- */
	/*
		テクスチャ情報作成
	*/
static void DG_SetNewTextureBuffer( int tri_code, DG_TEX *tex, u_long64 offset, int mode )
{
#if 0	/* この処理をデータ構築のテクスチャパック時に行うようにしたので必要なくなった */
	DG_TEX	*tex ;

	if ( DG_MaxTexBuffer >= MAX_TEX_BUFFER ){
		printf("texture buffer over!!\n");
	}
	//printf("tex num : %d\n", DG_MaxTexBuffer );
	tex = &DG_TexBuffer[ DG_MaxTexBuffer++ ] ;

	tex->tex0_base = tag->tex0_base + offset ;
	tex->tex1 = tag->tex1 ;
	tex->tex2 = tag->tex2 + offset ;
	tex->clamp = tag->clamp ;
	tex->alpha = tag->alpha ;
	tex->u_offset = tag->u_offset ;
	tex->v_offset = tag->v_offset ;
	tex->u_scale = tag->u_scale ;
	tex->v_scale = tag->v_scale ;
	tex->tex_id = tag->tex_id ;
	if ( mode ){
		tex->tri_id = tri_code | 0x80000000 ;	/* 半透明用の場合には最上位ビットを立てる */
	} else {
		tex->tri_id = tri_code ;
	}
	tex->tex_trans.giftag.tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7) ;
	tex->tex_trans.giftag.regs = 0x0fffeeee ;/* A+D */
	tex->tex_trans.clamp.data = tex->clamp ;
	tex->tex_trans.clamp.reg = SCE_GS_CLAMP_1 ;
	tex->tex_trans.tex2.data = tex->tex2 ;
	tex->tex_trans.tex2.reg = SCE_GS_TEX2_1 ;
	tex->tex_trans.tex0.data = tex->tex0_base ;
	tex->tex_trans.tex0.reg = SCE_GS_TEX0_1 ;
	tex->tex_trans.alpha.data = tex->alpha ;
	tex->tex_trans.alpha.reg = SCE_GS_ALPHA_1 ;
	tex->tex_trans.pad[0].data = 0 ;
	tex->tex_trans.pad[0].reg = SCE_GS_NOP ;
	if ( tag->pad & TRI_TEXTAG_FLAG_BMAP ){
		/* バンプマッピングテクスチャ時の設定（テクスチャシフト量パラメータ） */
		int		t_size ;
		t_size = 2 << ( ( tex->tex0_base >> 26 ) & 15 ) ;
		tex->tex_trans.vec1.vx = -1.0f / (float)t_size ;
		t_size = 2 << ( ( tex->tex0_base >> 30 ) & 15 ) ;
		tex->tex_trans.vec1.vy = -1.0f / (float)t_size ;
	} else {
		/* 環境マップテクスチャ（バンプマッピング以外）時の設定 */
		tex->tex_trans.vec1.vx = -0.4999f * tex->u_scale ;
		tex->tex_trans.vec1.vy = -0.4999f * tex->v_scale ;
		tex->tex_trans.vec1.vz = 1.0f ;
		tex->tex_trans.vec1.vw = 1.0f ;
		tex->tex_trans.vec2.vx = 0.5f * tex->u_scale + tex->u_offset ;
		tex->tex_trans.vec2.vy = 0.5f * tex->v_scale + tex->v_offset ;
		tex->tex_trans.vec2.vz = 0.0f ;
		tex->tex_trans.vec2.vw = 0.0f ;
	}
#else
	/* 実際の読み込みベースアドレスにあわせて補正する */
	tex->tex_trans.tex0.data = tex->tex_trans.tex0.data + offset ;
	tex->tex_trans.tex2.data = tex->tex_trans.tex2.data + offset ;
	/* モードチェック */
	if ( mode ){
		tex->tri_id = tri_code | 0x80000000 ;	/* 半透明用の場合には最上位ビットを立てる */
	} else {
		tex->tri_id = tri_code ;
	}
#endif
}


/* ---------------------------------------------------------------- */
	/*
		テクスチャの設定ルーチン
	*/
static void LoadImageTriTextureData( DG_TEXTURE_LIST *tex_list, int init_flag )
{
	int		j, flag, texture_base, mode, clut_page_size ;
	DG_TEX	*tex ;

	flag = tex_list->header->compress_flag ;
	/* ＣＬＵＴサイズがページ単位になってない可能性があるので */
	clut_page_size = ( tex_list->header->clut_size + 31 ) & ( ~31 ) ;

	if ( flag & TRI_FLAG_TRANS || 0){
		printf("load base: %08x\n", _tex_base );
		/* 半透明オブジェクト＆エフェクト用テクスチャ時 */
		texture_base = _tex_base / 64 ;
		_tex_base += ( tex_list->header->tex_size + clut_page_size ) * 64 ;
		printf("semi trans textures : last base %08x\n", _tex_base );
#ifndef HIGHRESO_FFI
		if ( _tex_base > 0x100000 ){
			printf("warning:texture over (%x:%d byte over)\n", _tex_base, ( _tex_base - 0x100000 )*4 );
			GV_ERROR( GV_ERROR_TEXTURE_OVER );
#ifdef DEBUG_MODE
#ifndef DESIGN_PREVIEW
			//for ( i = 0 ; i < 260*60*15 ; i++ ) WAIT_HSYNC( 1 );
#endif
#endif
#else
		if ( _tex_base > 0x0c0000 ){
			printf("warning:texture over (%x:%d byte over)\n", _tex_base, ( _tex_base - 0x0c0000 )*4 );
			GV_ERROR( GV_ERROR_TEXTURE_OVER );
#ifdef DEBUG_MODE
#ifndef DESIGN_PREVIEW
			//for ( i = 0 ; i < 260*60*15 ; i++ ) WAIT_HSYNC( 1 );
#endif
#endif
#endif
		}
		mode = 1 ;
	} else {
		printf("load base: %08x\n", TEXTURE_TOP_PAGE() );
		if ( ( ( tex_list->header->tex_size + tex_list->header->clut_size ) * 64 ) > 0x40000 ){
			printf("warning:texture over (%x:%d byte over)\n", TEXTURE_TOP_PAGE(),
				   ( ( tex_list->header->tex_size + clut_page_size ) * 64 - 0x040000 ) * 4 );
			GV_ERROR( GV_ERROR_TEXTURE_OVER );
#ifdef DEBUG_MODE
#ifndef DESIGN_PREVIEW
			//for ( i = 0 ; i < 260*60*15 ; i++ ) WAIT_HSYNC( 1 );
#endif
#endif
		}
		/* 通常オブジェクト用テクスチャ時 */
		texture_base = TEXTURE_TOP_PAGE() / 64 ;
		mode = 0 ;
	}

	tex_list->load_tex_base = texture_base ;
	tex_list->load_clut_base = texture_base + tex_list->header->tex_size ;
	tex_list->load_tex0 = ( (u_long64)tex_list->load_tex_base )
	  + ( ( (u_long64)tex_list->load_clut_base ) << 37 ) ;

	tex = tex_list->textures ;
	for ( j = tex_list->header->n_textures ; j > 0 ; j--, tex++ ){
		if ( init_flag == 0 ){
			DG_SetNewTextureBuffer( tex_list->code, tex, tex_list->load_tex0, mode );
		}
#ifdef DEBUG_MODE
		if ( flag & TRI_FLAG_TRANS ){/* テクスチャＩＤ重複チェック */
			int		i, j, count ;
			DG_TEX	*_tex ;

			count = 0 ;
			for ( i = 0 ; i < DG_MaxTextures ; i++ ){
				if ( !( DG_TextureList[ i ]->header->compress_flag & TRI_FLAG_TRANS ) ) continue ;
				_tex = DG_TextureList[ i ]->textures ;
				for ( j = DG_TextureList[ i ]->header->n_textures ; j > 0 ; j--, _tex++ ){
					if ( tex->tex_id == _tex->tex_id ) count++ ;
				}
			}
			if ( count != 1 ){
				printf("warning !!!: conflict texture id (%d)%d\n", tex->tex_id, count );
			}
		}
#endif
	}


	/* テクスチャ転送パケットの作成 */
	DG_SetDefLoadTextureImage( &tex_list->tex_packet[0],
							  texture_base, tex_list->header->tex_size, tex_list->texture_image,
							  tex_list->header->clut_size, tex_list->clut_image );
	DG_SetDefLoadTextureImage( &tex_list->tex_packet[1],
							  texture_base, tex_list->header->tex_size, tex_list->texture_image,
							  tex_list->header->clut_size, tex_list->clut_image );

	return ;
}

	/*
		テクスチャ転送パケットの作成
	*/
static void DG_SetDefLoadTextureImage( DG_TEX_PACKET *tex, int base, int size, void *image, int clut_size, void *clut_image )
{
#if 0
	static DG_TEX_PACKET	def_ldtex = {
		/* 第１イメージ転送 */
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 6 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_DIRECT(6,0)} },
		{ .NLOOP = 4, .EOP = 0, .PRE = 0, .PRIM = 0, .FLG = SCE_GIF_PACKED, .NREG = 1, .REGS0 = GS_REGS_AD },
		{ .DBP = 0, .DBW = 1, .DPSM = SCE_GS_PSMCT32 },SCE_GS_BITBLTBUF,
		{ .DSAX = 0, .DSAY = 0, .DIR = 0 },SCE_GS_TRXPOS,
		{ .RRW = 64, .RRH = 0 },SCE_GS_TRXREG,
		{ .XDR = 0 },SCE_GS_TRXDIR,
		{ .NLOOP = 0, .EOP = 0, .PRE = 0, .PRIM = 0, .FLG = SCE_GIF_IMAGE, .NREG = 0 },
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_DIRECT(0,0)} },
		/* 第２イメージ転送 */
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 6 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_DIRECT(6,0)} },
		{ .NLOOP = 4, .EOP = 0, .PRE = 0, .PRIM = 0, .FLG = SCE_GIF_PACKED, .NREG = 1, .REGS0 = GS_REGS_AD },
		{ .DBP = 0, .DBW = 1, .DPSM = SCE_GS_PSMCT32 },SCE_GS_BITBLTBUF,
		{ .DSAX = 0, .DSAY = 0, .DIR = 0 },SCE_GS_TRXPOS,
		{ .RRW = 64, .RRH = 0 },SCE_GS_TRXREG,
		{ .XDR = 0 },SCE_GS_TRXDIR,
		{ .NLOOP = 0, .EOP = 0, .PRE = 0, .PRIM = 0, .FLG = SCE_GIF_IMAGE, .NREG = 0 },
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_DIRECT(0,0)} },
		/* ＣＬＵＴイメージ転送 */
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 6 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_DIRECT(6,0)} },
		{ .NLOOP = 4, .EOP = 0, .PRE = 0, .PRIM = 0, .FLG = SCE_GIF_PACKED, .NREG = 1, .REGS0 = GS_REGS_AD },
		{ .DBP = 0, .DBW = 1, .DPSM = SCE_GS_PSMCT32 },SCE_GS_BITBLTBUF,
		{ .DSAX = 0, .DSAY = 0, .DIR = 0 },SCE_GS_TRXPOS,
		{ .RRW = 64, .RRH = 0 },SCE_GS_TRXREG,
		{ .XDR = 0 },SCE_GS_TRXDIR,
		{ .NLOOP = 0, .EOP = 1, .PRE = 0, .PRIM = 0, .FLG = SCE_GIF_IMAGE, .NREG = 0 },
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_DIRECT(0,0)} },
		/* パケット終端 */
		{ .qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ), .vifcode = {SCE_VIF1_SET_NOP(0),SCE_VIF1_SET_NOP(0)} },
	};
	int	size1, size2 ;

	*tex = def_ldtex ;

	/* サイズが大きい場合には転送パケットを２つに分ける */
	if ( size > 4094 ){
		printf("texture size over !!(%d)\n", size - 4094);
		size = 4094 ;
	}
	if ( size >= 2048 ){
		size1 = 2047 ;
		size2 = size - 2047 ;
		printf("extend trans %d %d %d\n", size1, size2, size);
	} else {
		size1 = size ;
		size2 = 0 ;
	}

	tex->bitbltbuf1.DBP = base ;
	tex->trxreg1.RRH = size1 ;
	tex->giftag1.NLOOP = size1 * 16 ;
	tex->dmatag1.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, size1 * 16 );
	tex->dmatag1.addr = image ;
	tex->dmatag1.vifcode[1] = SCE_VIF1_SET_DIRECT( size1 * 16, 0 );

	tex->bitbltbuf2.DBP = base + ( size1 & ~63 ) ;
	tex->trxpos2.DSAY = size1 & 63 ;
	tex->trxreg2.RRH = size2 ;
	tex->giftag3.NLOOP = size2 * 16 ;
	tex->dmatag3.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, size2 * 16 );
	tex->dmatag3.addr = (void*)( (int)image + size1 * 16 * 16 ) ;
	tex->dmatag3.vifcode[1] = SCE_VIF1_SET_DIRECT( size2 * 16, 0 );

	tex->bitbltbuf3.DBP = base + size ;
	tex->trxreg3.RRH = clut_size ;
	tex->giftag5.NLOOP = clut_size * 16 ;
	tex->dmatag5.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, clut_size * 16 );
	tex->dmatag5.addr = (void*) clut_image ;
	tex->dmatag5.vifcode[1] = SCE_VIF1_SET_DIRECT( clut_size * 16, 0 );

	if ( size1 == 0 ) tex->dmatag1.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	if ( size2 == 0 ) tex->dmatag3.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	if ( clut_size == 0 ) tex->dmatag5.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
#else
	int	size1, size2 ;

	/* サイズが大きい場合には転送パケットを２つに分ける */
	if ( size > 4094 ){
		printf("texture size over !!(%d)\n", size - 4094);
		size = 4094 ;
	}
	if ( size >= 2048 ){
		size1 = 2047 ;
		size2 = size - 2047 ;
		printf("extend trans %d %d %d\n", size1, size2, size);
	} else {
		size1 = size ;
		size2 = 0 ;
	}

	/* 第１イメージ転送 */
	DG_MakeLoadImagePacket( &tex->texel0, SCE_GS_PSMCT32,
						   64, size1,
						   base * 64, 64,
						   image );
	/* 第２イメージ転送 */
	DG_MakeLoadImagePacket( &tex->texel1, SCE_GS_PSMCT32,
						   64, size2 | ( (size1 & 31) << 16 ),
						   ( base + ( size1 & ~31 ) ) * 64, 64,
						   (void*)( (int)image + size1 * 16 * 16 ) );
	/* ＣＬＵＴイメージ転送 */
	DG_MakeLoadImagePacket( &tex->clut, SCE_GS_PSMCT32,
						   64, clut_size,
						   ( base + size ) * 64, 64,
						   clut_image );
	tex->texel0.giftag1.tag &= ~(SCE_GIF_SET_TAG( 0, 1, 0, 0, 0, 0 ));
	tex->texel1.giftag1.tag &= ~(SCE_GIF_SET_TAG( 0, 1, 0, 0, 0, 0 ));
	/* パケット終端 */
	tex->rettag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	tex->rettag.vifcode[0] = SCE_VIF1_SET_NOP(0) ;
	tex->rettag.vifcode[1] = SCE_VIF1_SET_NOP(0) ;
#endif
}

void BP_CopyDGTexToDGTexBP_Optional( DG_TEX_BP **ppDst, DG_TEX_BP *pDst, DG_TEX *pOrig )
{
   if ( pOrig )
   {
      *ppDst = pDst;
      BP_CopyDGTexToDGTexBP( pDst, pOrig );
   }
   else
   {
      *ppDst = NULL;
   }
}




