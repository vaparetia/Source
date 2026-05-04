/*
	text.c
	テクスチャ管理ルーチン

	1999/07/07 K.Takabe
	$Id: wtext.c,v 1.25 2003/01/03 09:25:22 takaki Exp $

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


#ifndef _XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#include <xgraphics.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
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
int		DG_LastLoadTexture ;

DG_TEX			DG_DefaultTexture = {
	0.0f, 0.0f, 1.0f, 1.0f,
	0, 0, 0, {0},
};
/*
   DG_SystemTexture[0]:フレームバッファ０（512x512x32bit）
   DG_SystemTexture[1]:フレームバッファ１（512x512x32bit）
   DG_SystemTexture[2]:テクスチャバッファ（512x512x32bit）
   DG_SystemTexture[3]:テクスチャバッファ（512x512x16bit）
*/
extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;		/* バックバッファテクスチャ */
DG_TEX			DG_SystemTexture[] = {
	{/* FrameBuffer0-32bit */
		(DISPLAY_WIDTH-DRAW_WIDTH)/2, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT,
		0, 0, 0, 0, {0},
		{
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( BUFFER_PAGE(0) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{0,0,0,0},
			{0,0,0,0},
			NULL,	/* xdgd.cにて起動時に設定される */
			{0,0,0}
		}
	},
	{/* FrameBuffer1-32bit */
		(DISPLAY_WIDTH-DRAW_WIDTH)/2, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT,
		0, 0, 0, 0, {0},
		{
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( BUFFER_PAGE(1) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{0,0,0,0},
			{0,0,0,0},
			NULL,	/* xdgd.cにて起動時に設定される */
			{0,0,0}
		}
	},
	{/* TextureBuffer-32bit *//* ＸＢＯＸではテクスチャ領域ではなく専用の退避バッファ */
		(DISPLAY_WIDTH-DRAW_WIDTH)/2, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT,
		0, 0, 0, 0, {0},
		{
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{0,0,0,0},
			{0,0,0,0},
			NULL,	/* xdgd.cにて起動時に設定される */
			{0,0,0}
		}
	},
	{/* Texture page 16bit *//* 現在はTextureBuffer-32bitと同じ */
		(DISPLAY_WIDTH-DRAW_WIDTH)/2, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT,
		0, 0, 0, 0, {0},
		{
			{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
			{ 0, SCE_GS_TEX2_1 },
			{ SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
							  9,9,0,0,0,0,0,0,0),SCE_GS_TEX0_1 },
			{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ), SCE_GS_ALPHA_1 },
			{0,0,0,0},
			{0,0,0,0},
			NULL,	/* xdgd.cにて起動時に設定される */
			{0,0,0}
		}
	},
};

/* ---------------------------------------------------------------- */
static void LoadImageTriTextureData( DG_TEXTURE_LIST *tex_list, int init_flag );
static void DG_SetDefLoadTextureImage( DG_TEX_PACKET *tex, int base, int size, void *image, int clut_size, void *clut );
static void DG_SetNewTextureBuffer( int tri_code, DG_TEX *tex, u_long offset, int mode );

static void DG_FreeTextureList( DG_TEXTURE_LIST	*tex_list );
static void DG_ClearSurfaceImageFF(LPDIRECT3DSURFACE8 surface) ;


/* ---------------------------------------------------------------- */

	/*
		ロード時の初期化
	*/
void DG_InitTextureCache( void )
{
	int i;
	for( i = 0; i < DG_MaxTextures; i++ ){
#if 0	/* 現状ではメモリを確保しているためきちんと開放処理を行うようにする */
		int j;
		for ( j = 0 ; j < DG_TextureList[ i ]->header->n_info ; j++ ){
			if ( DG_TextureList[ i ]->texpack[ j ].data != NULL ){
				DG_FreeLocalVideoMemory( DG_TextureList[ i ]->texpack[ j ].data );
			}
		}
#endif
#ifdef _DEBUG_MODE
printf("DG_FreeTextureList:(%d/%d)\n", i, DG_MaxTextures) ;
#endif
		DG_FreeTextureList( DG_TextureList[ i ] );
		GV_Free( DG_TextureList[ i ] );
		DG_TextureList[ i ] = NULL;
	}
	DG_MaxTextures = 0 ;
	DG_MaxTexBuffer = 0 ;
}

	/*
		テクスチャの登録
	*/
int DG_LoadInitTriBuf( void *buf, int code, DG_TEXTURE_LIST *list )
{
	TRI_FILEHEADER	*header ;
	int					init_flag = 0 ;

	if ( code & GV_CACHEID_RESIDENT ){
		init_flag = 1 ;
	}
	ASSERT( DG_MaxTextures < MAX_TEX_CACHE );
	code &= 0xffffff ;
#ifdef _DEBUG_MODE
printf( "Loadinit TRI addr=%08x code=%d\n", buf, code );
#endif
//	list = &DG_TextureList[ DG_MaxTextures++ ];
	DG_TextureList[ DG_MaxTextures ++ ] = list;

	//memset( list, 0, sizeof( DG_TEXTURE_LIST ) );
	GV_ZeroMemory( list, sizeof( DG_TEXTURE_LIST ) );
	/* テンポラリのワークに一度全部読み込む */
	list->header = header = buf ;
	list->d3dpalette = (void*)&list->texpack[header->n_info] ;

	list->textures = (DG_TEX*)&header[1] ;
	list->code = code ;
	list->flag = 0 ;

	list->texture_image = (void*)( (int)header->texel_addr + (int)buf ) ;
	list->clut_image = (void*)( (int)header->clut_addr + (int)buf ) ;
	//list->tex_size = ( list->header->tex_size + list->header->clut_size ) * 64 * 4 ;
	{
		XTI_TEXINFO	*info ;
		int			i ;
		info = (void*)( (int)list->header->info_addr + (int)buf );
		for ( i = 0 ; i < header->n_info ; i++, info++ ){
			list->texpack[ i ].texinfo = info ;
		}
	}

	LoadImageTriTextureData( list, init_flag );
	DG_LastLoadTexture = code ;

#ifdef DESIGN_PREVIEW
	printf("texture memory: %d %d\n", DG_MaxTextures, DG_MaxTexBuffer);
#endif
	return (1);
}

int DG_GetTextureListSize( void *buf )
{
	TRI_FILEHEADER	*header = (TRI_FILEHEADER*)buf ;
	int				size ;
	size = sizeof(DG_TEXTURE_LIST) ;
	size += sizeof(DG_TEXPACK) * header->n_info ;
	size += sizeof(D3DPalette) * header->n_clut ;
	return ( size );
}

int DG_LoadInitTri( void *buf, int code )
{
	DG_TEXTURE_LIST *list;

	//list = GV_Malloc( sizeof( DG_TEXTURE_LIST ) );
	list = GV_Malloc( DG_GetTextureListSize( buf ) );
	ASSERT( list != 0 );
#ifndef _WINDOWS
printf("list:0x%08X\n", list) ;
#endif
	return DG_LoadInitTriBuf( buf, code, list );
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
			DG_FreeTextureList(res) ;
			for ( ; i < ( DG_MaxTextures - 1 ) ; i++ ){
				DG_TextureList[ i ] = DG_TextureList[ i + 1 ] ;
			}
			DG_TextureList[ i ] = NULL ;
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
#if 0
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
#endif
	return ;
}

	/*
		一時ＣＬＵＴイメージ情報のクリア
	*/
void DG_ResetTextureTmpClut( void )
{
#if 0
	int		i ;

	for ( i = 0 ; i < DG_MaxTextures ; i++ ){
		DG_TextureList[ i ]->tmp_clut_image = NULL ;
	}
#endif
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

	printf("search texture error(%d)\n", code );
#if 0
#ifdef DEBUG
	if( (code!=0)  &&  (FS_MediaType() == FS_MEDIA_DEV) ){
		ASSERT(0);
	}
#endif
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
		if ( (unsigned int)DG_TextureList[ i ]->code != tri_code ) continue ;
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

	printf("search texture error(tri=%d,code=%d)\n", tri_code, code );
#if 0
#ifdef DEBUG
	if( (code!=0)  &&  (FS_MediaType() == FS_MEDIA_DEV) ){
		ASSERT(0);
	}
#endif
#endif
	if ( DG_MaxTextures != 0 ) return DG_TextureList[ 0 ]->textures ;
	return ( NULL );
}

	/*
		テクスチャ転送パケットのアドレスを取得
	*/
#if 0
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
#endif

	/*
		テクスチャリスト構造体のアドレスを取得
	*/
DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code )
{
	int		i ;
	DG_TEXTURE_LIST	**tex_list_p ;

	tex_list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0; i--, tex_list_p++ ){
		if ( ( (unsigned int)( *tex_list_p )->code & 0x00ffffff ) == code ){
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


/* ---------------------------------------------------------------- */
	/*
		テクスチャのピクセル数を計算する
	*/
void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex )
{
	int		w, h, tw, th, x, y ;
#ifdef _WINDOWS
	if( !tex ){ return ; }
#endif
	tw = (int)(( tex->tex_trans.tex0.data >> 26 ) & 0x0f) ;
	th = (int)(( tex->tex_trans.tex0.data >> 30 ) & 0x0f) ;
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

	if ( ( tex_list = DG_GetTextureList( tri_code ) ) == NULL ) return ( NULL );
	if ( ( dst_tex = DG_GetTexture2( tri_code, dst_tex_id ) ) == NULL ) return ( NULL );
	if ( ( src_tex = DG_GetTexture2( tri_code, src_tex_id ) ) == NULL ) return ( NULL );

	if ( ( tex_replace = GV_Malloc( sizeof(DG_TEX_MOVEREPLACE) ) ) == NULL ) return ( NULL );

	tex_replace->org_tex = dst_tex ;
	tex_replace->src_tex = src_tex ;
	tex_replace->org_ptex = dst_tex->tex_trans.ptex ;
	tex_replace->org_vec1 = dst_tex->tex_trans.vec1 ;
	tex_replace->org_vec2 = dst_tex->tex_trans.vec2 ;

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
	int		dst_base, dst_width, dst_fmt, dst_x, dst_y, dst_w, dst_h ;

	if ( ( tex_list = DG_GetTextureList( tri_code ) ) == NULL ) return ( NULL );
	if ( ( dst_tex = DG_GetTexture2( tri_code, dst_tex_id ) ) == NULL ) return ( NULL );

	if ( ( tex_replace = GV_Malloc( sizeof(DG_TEX_LOADREPLACE) ) ) == NULL ) return ( NULL );

	dst_base = (int)(dst_tex->tex_trans.tex0.data & 0x3fff) ;
	dst_width = (int)(( dst_tex->tex_trans.tex0.data >> 14 ) & 0x3f) ;
	dst_fmt = (int)(( dst_tex->tex_trans.tex0.data >> 20 ) & 0x3f) ;
	DG_GetTexelInfo( &dst_w, &dst_h, &dst_x, &dst_y, dst_tex );

	/* ＣＬＵＴの転送パケット生成 */
	if ( dst_fmt == SCE_GS_PSMT8 ){
		/* 256パレット時 */
	} else {
		/* 16パレット時 */
	}

	tex_replace->org_tex = dst_tex ;
	tex_replace->src_tex = NULL ;
	tex_replace->org_ptex = dst_tex->tex_trans.ptex ;
	tex_replace->org_vec1 = dst_tex->tex_trans.vec1 ;
	tex_replace->org_vec2 = dst_tex->tex_trans.vec2 ;
	tex_replace->texel = texel ;
	tex_replace->clut = clut ;
	tex_replace->pitch = ( dst_w + 31 ) & ~31 ;
	tex_replace->width = dst_w ;
	tex_replace->height = dst_h ;
	tex_replace->mode = 1 ;
	/* 置き換え用線形テクスチャの生成 */
	tex_replace->tex = DG_MakeLinerTexture( tex_replace->pitch, dst_h, DG_TEXLIN_FORMAT_A8R8G8B8 );
	if ( tex_replace->tex == NULL ){
		GV_Free( tex_replace );
		return ( NULL );
	}
	//DG_LinerTextureSetFullLoad(tex_replace->tex) ;
	tex_replace->tex_list = tex_list ;
	tex_replace->prev = NULL ;
	tex_replace->next = NULL ;

	return ( tex_replace );
}

	/*
		データ再転送によるテクスチャ入れ替えパケットを開放する
	*/
void DG_FreeLoadReplacePacket( DG_TEX_LOADREPLACE *tex_replace )
{
	DG_FreeLinerTexture( tex_replace->tex );
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

	/* テクスチャ入れ替えエミュレーションの為ここでパラメータ書き換え */
	tex_replace->org_tex->tex_trans.ptex = tex_replace->src_tex->tex_trans.ptex ;
	tex_replace->org_tex->tex_trans.vec1 = tex_replace->src_tex->tex_trans.vec1 ;
	tex_replace->org_tex->tex_trans.vec2 = tex_replace->src_tex->tex_trans.vec2 ;

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

	/* テクスチャ入れ替えエミュレーションの為ここでパラメータ書き換えを戻す */
	tex_replace->org_tex->tex_trans.ptex = tex_replace->org_ptex ;
	tex_replace->org_tex->tex_trans.vec1 = tex_replace->org_vec1 ;
	tex_replace->org_tex->tex_trans.vec2 = tex_replace->org_vec2 ;

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

	/* テクスチャ入れ替えエミュレーションの為ここでパラメータ書き換え */
	tex_replace->org_tex->tex_trans.ptex = tex_replace->tex->d3dtexture ;
#ifndef _WINDOWS
	tex_replace->org_tex->tex_trans.vec1.vx = (float)tex_replace->width ;
	tex_replace->org_tex->tex_trans.vec1.vy = (float)tex_replace->height ;
	tex_replace->org_tex->tex_trans.vec2.vx = 0.0f ;
	tex_replace->org_tex->tex_trans.vec2.vy = 0.0f ;

#else	// Windows版では以下の計算
	tex_replace->org_tex->tex_trans.vec1.vx = (16384.0f/32767.0f)
											* ((float)tex_replace->width
												/(float)tex_replace->tex->width)
											* tex_replace->tex->tex_param.vx ;
	tex_replace->org_tex->tex_trans.vec1.vy = (16384.0f/32767.0f)
											* ((float)tex_replace->height
												/(float)tex_replace->tex->height)
											* tex_replace->tex->tex_param.vy ;
	tex_replace->org_tex->tex_trans.vec2.vx = 0.0f ;
	tex_replace->org_tex->tex_trans.vec2.vy = 0.0f ;
#endif
	{/* ＰＳ２形式からＸＢＯＸ形式にテクスチャを変換 */
		int		x, y ;
		u_char	*src, *dst, *dst_top, *pal, *pal_top ;
		src = tex_replace->texel ;
		dst_top = tex_replace->tex->image ;
		pal_top = tex_replace->clut ;
		if ( tex_replace->mode == 0 ){
			/* 4bit */
			for ( y = 0 ; y < tex_replace->height ; y++ ){
				dst = dst_top ;
				for ( x = 0 ; x < tex_replace->width ; x+=2 ){
					pal = pal_top + ( *src & 0x0f ) * 4 ;
					dst[ 0 ] = pal[ 2 ] ;
					dst[ 1 ] = pal[ 1 ] ;
					dst[ 2 ] = pal[ 0 ] ;
					dst[ 3 ] = ( pal[ 3 ] > 127 ) ? 255 : pal[ 3 ] * 2 ;
					dst += 4 ;
					pal = pal_top + ( *src >> 4 ) * 4 ;
					dst[ 0 ] = pal[ 2 ] ;
					dst[ 1 ] = pal[ 1 ] ;
					dst[ 2 ] = pal[ 0 ] ;
					dst[ 3 ] = ( pal[ 3 ] > 127 ) ? 255 : pal[ 3 ] * 2 ;
					dst += 4 ;
					src++ ;
				}
				dst_top += tex_replace->pitch * 4 ;
			}
		} else {
			/* 8bit */
			for ( y = 0 ; y < tex_replace->height ; y++ ){
				dst = dst_top ;
				for ( x = 0 ; x < tex_replace->width ; x++ ){
					pal = pal_top + (*src) * 4 ;
					dst[ 0 ] = pal[ 2 ] ;
					dst[ 1 ] = pal[ 1 ] ;
					dst[ 2 ] = pal[ 0 ] ;
					dst[ 3 ] = ( pal[ 3 ] > 127 ) ? 255 : pal[ 3 ] * 2 ;
					dst += 4 ;
					src++ ;
				}
				dst_top += tex_replace->pitch * 4 ;
			}
		}
#ifdef _WINDOWS
		DG_LinerTextureSetImageDirty(tex_replace->tex) ;	// 内容変更適用フラグON
#endif
	}

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

	/* テクスチャ入れ替えエミュレーションの為ここでパラメータ書き換えを戻す */
	tex_replace->org_tex->tex_trans.ptex = tex_replace->org_ptex ;
	tex_replace->org_tex->tex_trans.vec1 = tex_replace->org_vec1 ;
	tex_replace->org_tex->tex_trans.vec2 = tex_replace->org_vec2 ;

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
	ＰＳ２のテクスチャ入れ替えをＸＢＯＸでエミュレーション
*/
/* テクスチャ入れ替えのためにテクスチャ情報を変更 */
DG_StartTextureReplaceEmulation( void )
{
}

/* 変更したテクスチャ情報を復元 */
DG_EndTextureReplaceEmulation( void )
{
}


/* ---------------------------------------------------------------- */
#if 0
	/*
		ミップマップ情報生成
	*/
void DG_MakeMipmap( DG_MIPMAP *mipmap )
{
#if 0
	int		i ;
	unsigned long	bp, bw ;
	int		bpl[7], bwl[7] ;

	for ( i = 0 ; i <= mipmap->max_level ; i++ ){
		if ( mipmap->tex[i] == NULL ){
			mipmap->tex[i] = DG_GetTexture2( mipmap->tri_id, mipmap->tex_id[i] );
		}
		bp = mipmap->tex[i]->tex_trans.tex0.data & 0x3fff ;
		bw = ( mipmap->tex[i]->tex_trans.tex0.data >> 14 ) & 0x3f ;
		bpl[i] = bp ;
		bwl[i] = bw ;
	}
	mipmap->miptbp1.data = SCE_GS_SET_MIPTBP1( bpl[1], bwl[1], bpl[2], bwl[2], bpl[3], bwl[3] );
	mipmap->miptbp2.data = SCE_GS_SET_MIPTBP2( bpl[4], bwl[4], bpl[5], bwl[5], bpl[6], bwl[6] );
#endif
}
void DG_SetMipmapLodParam( DG_MIPMAP *mipmap, float screen )
{
#if 0
	int		lod_k ;

	lod_k = DG_FTOI( -16.0f * logf( mipmap->z0 * screen ) / 0.69314718055994530941723212145818f/*logf( 2 )*/ ) ;
	lod_k <<= mipmap->lod_l ;
	lod_k &= 0xfff ;
	mipmap->tex1.data = SCE_GS_SET_TEX1( 0, mipmap->max_level, 1, 5, 0, mipmap->lod_l, lod_k );
#endif
}
#endif

/* ---------------------------------------------------------------- */
	/*
		テクスチャ情報作成
	*/
static void DG_SetNewTextureBuffer( int tri_code, DG_TEX *tex, u_long offset, int mode )
{
	/* 実際の読み込みベースアドレスにあわせて補正する */
	//tex->tex_trans.tex0.data = tex->tex_trans.tex0.data + offset ;
	//tex->tex_trans.tex2.data = tex->tex_trans.tex2.data + offset ;
	/* モードチェック */
	if ( mode ){
		tex->tri_id = tri_code | 0x80000000 ;	/* 半透明用の場合には最上位ビットを立てる */
	} else {
		tex->tri_id = tri_code ;
	}
	/* スケールなどの補正 */
//#ifdef DEBUG_MODE	// (Windows版では常に必要そうなので常に実行)
	switch ( tex->flag & TEXINFO_FLAG_MAPMASK ){
	  case TEXINFO_FLAG_EMAP:
	  case TEXINFO_FLAG_SMAP:
		break ;
	  case TEXINFO_FLAG_BMAP:
		{
			int		w, h ;
			DG_GetTexelInfo( &w, &h, NULL, NULL, tex );
#ifdef _WINDOWS
			tex->tex_trans.vec1.vx = tex->u_scale * (1.0f/32767.0f) ;	// 正規化
			tex->tex_trans.vec1.vy = tex->v_scale * (1.0f/32767.0f) ;
#else
			tex->tex_trans.vec1.vx = tex->u_scale ;
			tex->tex_trans.vec1.vy = tex->v_scale ;
#endif
			tex->tex_trans.vec1.vz = -3.0f / (float)w ;
			tex->tex_trans.vec1.vw = -3.0f / (float)h ;
			tex->tex_trans.vec2.vx = tex->u_offset ;
			tex->tex_trans.vec2.vy = tex->v_offset ;
			tex->tex_trans.vec2.vz = 0 ;
			tex->tex_trans.vec2.vw = 0 ;
		}
		break ;
	  default:
#ifdef _WINDOWS
		tex->tex_trans.vec1.vx = tex->u_scale * (1.0f/32767.0f) ;	// 正規化
		tex->tex_trans.vec1.vy = tex->v_scale * (1.0f/32767.0f) ;
		tex->tex_trans.vec1.vz = 1.0f ;
		tex->tex_trans.vec1.vw = 1.0f ;
		tex->tex_trans.vec2.vx = tex->u_offset ;
		tex->tex_trans.vec2.vy = tex->v_offset ;
		tex->tex_trans.vec2.vz = 0.0f ;
		tex->tex_trans.vec2.vw = 0.0f ;
#else
		tex->tex_trans.vec1.vx = tex->u_scale ;
		tex->tex_trans.vec1.vy = tex->v_scale ;
		tex->tex_trans.vec1.vz = 1 ;
		tex->tex_trans.vec1.vw = 1 ;
		tex->tex_trans.vec2.vx = tex->u_offset ;
		tex->tex_trans.vec2.vy = tex->v_offset ;
		tex->tex_trans.vec2.vz = 0 ;
		tex->tex_trans.vec2.vw = 0 ;
#endif
		break ;
	}
//#endif 
}


/* ---------------------------------------------------------------- */
/*		アンスィズル												*/
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/*		Image Buffer補正											*/
/* ---------------------------------------------------------------- */
static void XBoxImg2D3DImg(XTI_TEXINFO *tex_info, DG_TEXPACK *texpack)
{
	DWORD	tex_w, tex_h ;
	DWORD	size ;
	DWORD	src_size ;
	DWORD	xb_fmt ;

	/*-- フォーマット検査 ------------------------------------------*/

	xb_fmt = tex_info->format ;
	switch ( xb_fmt )
	{
	  case XTI_FORMAT_A8R8G8B8:
	  case XTI_FORMAT_A4R4G4B4:
	  case XTI_FORMAT_A1R5G5B5:
	  case XTI_FORMAT_PAL4 :		/* 4bitパレット */
	  case XTI_FORMAT_PAL8 :		/* 8bitパレット */
	  case XTI_FORMAT_PAL4AGBR :	/* 4bitパレット（ＰＳ２用） */
	  case XTI_FORMAT_PAL8AGBR :	/* 8bitパレット（ＰＳ２用） */
		break ;

	  default :	// 変換不要
		return ;
		break ;
	}
	/*--------------------------------------------------------------*/

	/*-- 作業領域確保 ----------------------------------------------*/
	/*--------------------------------------------------------------*/

	/*-- 作業領域解放 ----------------------------------------------*/
	/*--------------------------------------------------------------*/
}

/* ---------------------------------------------------------------- */
/*		DirectX8テクスチャ作成										*/
/* ---------------------------------------------------------------- */
#ifdef _WINDOWS
static void LoadImageTriCreateTexture(XTI_TEXINFO *tex_info, DG_TEXPACK *texpack)
{
	D3DFORMAT			fmt ;
	D3DFORMAT			texfmt ;
	DWORD				pitch ;
	DWORD				tex_w, tex_h ;
	DWORD				size ;
	DWORD				src_size ;
	DWORD				block ;
	RECT				src_rect ;
	RECT				dst_rect ;
	PALETTEENTRY		*plt ;
	PALETTEENTRY		plt_buff[256] ;

	ASSERT( tex_info ) ;
	ASSERT( texpack ) ;
	ASSERT( texpack->data != NULL ) ;
	ASSERT( (tex_info->width > 0) && (tex_info->height > 0) ) ;

	tex_w    = tex_info->width ;
	tex_h    = tex_info->height ;

	src_size = tex_info->use_size ;

    src_rect.left   = 0 ;
    src_rect.top    = 0 ;
    src_rect.right  = tex_w ;

	/*-- 元データ情報を取得 ----------------------------------------*/

	plt = NULL ;
	pitch = tex_w ;
	size  = tex_w * tex_h ;
	switch ( tex_info->format )
	{
	  case XTI_FORMAT_A8R8G8B8:
		fmt  = D3DFMT_A8R8G8B8 ;
		size <<= 2 ;

	    src_rect.bottom = src_size / (tex_w << 2) ;
		break ;

	  case XTI_FORMAT_A4R4G4B4:
		fmt = D3DFMT_A4R4G4B4 ;
		size <<= 1 ;

	    src_rect.bottom = src_size / (tex_w << 1) ;
		break ;

	  case XTI_FORMAT_A1R5G5B5:
		fmt = D3DFMT_A1R5G5B5 ;
		size <<= 1 ;

	    src_rect.bottom = src_size / (tex_w << 1) ;
		break ;

	  case XTI_FORMAT_DXT1:
		fmt = D3DFMT_DXT1 ;
		pitch <<= 1 ;

		block = ((tex_w + 3) >> 2) * ((tex_h + 3) >> 2) ;
		size = block << 3 ;

	    src_rect.bottom = ((src_size >> 3) / (tex_w >> 2)) << 2 ;
		break ;

	  case XTI_FORMAT_DXT3:
		fmt = D3DFMT_DXT3 ;
		pitch <<= 2 ;			// ?(適当数値)

		block = ((tex_w + 3) >> 2) * ((tex_h + 3) >> 2) ;
		size = block << 4 ;

	    src_rect.bottom = ((src_size >> 4) / (tex_w >> 2)) << 2 ;
		break ;

	  case XTI_FORMAT_DXT5:
		fmt = D3DFMT_DXT5 ;
		pitch <<= 2 ;			// ?(適当数値)

		block = ((tex_w + 3) >> 2) * ((tex_h + 3) >> 2) ;
		size = block << 4 ;

	    src_rect.bottom = ((src_size >> 4) / (tex_w >> 2)) << 2 ;
		break ;

	  case XTI_FORMAT_PAL4 :		/* 4bitパレット */
		fmt = D3DFMT_A8R8G8B8 ;
		pitch = 0 ;
		size  = 0 ;
		ASSERT(0) ;
		break ;

	  case XTI_FORMAT_PAL8 :		/* 8bitパレット */
		fmt = D3DFMT_P8 ;
	    src_rect.bottom = src_size / tex_w ;

#if TRUE	// Palette対処 暫定処置
		{
			int		i ;

			plt = plt_buff ;
			for(i=0; i<256; i++, plt++)
			{
				plt->peRed   = i ;
				plt->peGreen = i ;
				plt->peBlue  = i ;
				plt->peFlags = 0xff ;		// 多分α
			}
		}
#endif
		break ;

	  case XTI_FORMAT_PAL4AGBR :	/* 4bitパレット（ＰＳ２用） */
		fmt = D3DFMT_A8R8G8B8 ;
		pitch = 0 ;
		size  = 0 ;
		ASSERT(0) ;
		break ;

	  case XTI_FORMAT_PAL8AGBR :	/* 8bitパレット（ＰＳ２用） */
		fmt = D3DFMT_A8R8G8B8 ;
		pitch = 0 ;
		size  = 0 ;
		ASSERT(0) ;
		break ;

	  default :					// 未対応フォーマット
		fmt = D3DFMT_A8R8G8B8 ;
		pitch = 0 ;
		size  = 0 ;
		ASSERT(0) ;
		break ;
	}

	if( !pitch )
	{
		texpack->p_d3d_texture = NULL ;
		return ;
	}

	if( src_rect.bottom > (int)tex_h ){ src_rect.bottom = (int)tex_h ; }
	/*--------------------------------------------------------------*/

	/*-- Create Texture --------------------------------------------*/

#if TRUE	// Palette対処 暫定処置
	switch( fmt )
	{
	  case D3DFMT_P8 :
		texfmt = DG_SelectUsableTextureFormat(D3DFMT_A4R4G4B4) ;	// 強制変更
		plt    = plt_buff ;
		break ;

	  default :
		texfmt = DG_SelectUsableTextureFormat(fmt) ;		// 使用可能Formatへ変更
		break ;
	}
#else
	texfmt = DG_SelectUsableTextureFormat(fmt) ;		// 使用可能Formatへ変更
#endif
	DG_CreateTexture(tex_info->width, tex_info->height, tex_info->mip_level, 0,
			texfmt, D3DPOOL_MANAGED, &texpack->p_d3d_texture) ;

#ifdef DEBUG_MODE
	if( texfmt != fmt ){ printf("Texture Format Trans %d --> %d\n", fmt, texfmt) ; }
#endif

    dst_rect = src_rect ;	// もし、テクスチャサイズの変更が必要に
							// なった場合、パラメータ調整する事。
	/*--------------------------------------------------------------*/

	/*-- Image転送 -------------------------------------------------*/

	{
		int					max_lv ;
		int					mlv ;
		HRESULT				hr ;
		IDirect3DSurface8	*surface ;
		IDirect3DSurface8	*src_surface ;
		BYTE				*img_data ;
		D3DLOCKED_RECT		src_locked_rect ;

		img_data = texpack->data ;
		max_lv   = tex_info->mip_level ;
		for(mlv=0; mlv<max_lv; mlv++)
		{
			/*-- Surface取得 ---------------------------------------*/

			hr = IDirect3DTexture8_GetSurfaceLevel(texpack->p_d3d_texture,
												mlv, &surface) ;
			if( FAILED(hr) )
			{
				dbgErrMessPuts("", hr) ;
				break ;
			}
			/*------------------------------------------------------*/

			/*-- 転送元Surface取得 ---------------------------------*/

			if( mlv > 0 )
			{
				/*-- 上位Surfaceからの複製 -------------------------*/
				hr = IDirect3DTexture8_GetSurfaceLevel(texpack->p_d3d_texture,
													mlv-1, &src_surface) ;
				if( FAILED(hr) )
				{
					ASSERT(0) ;
					break ;
				}

				hr = IDirect3DSurface8_LockRect(src_surface, &src_locked_rect, NULL,
												D3DLOCK_NOSYSLOCK) ;
				if( FAILED(hr) )
				{
					ASSERT(0) ;
					IDirect3DSurface8_Release(src_surface) ;
					break ;
				}

				img_data = src_locked_rect.pBits ;
				pitch    = src_locked_rect.Pitch ;
				fmt      = texfmt ;
				plt      = NULL ;
				/*--------------------------------------------------*/
			}
			else
			{
				/*-- 最上位Mipmap ----------------------------------*/

				src_surface = NULL ;
				/*--------------------------------------------------*/
			}
			/*------------------------------------------------------*/

			/*-- Surfaceに転送 -------------------------------------*/

			hr = D3DXLoadSurfaceFromMemory(surface,
									NULL,		// CONST PALETTEENTRY*
									&dst_rect,	// CONST RECT* pDestRect,
									img_data,	// LPCVOID pSrcMemory,
									fmt,		// D3DFORMAT SrcFormat,
									pitch,		// UINT SrcPitch,
									plt,		// CONST PALETTEENTRY* pSrcPalette,
									&src_rect,	// CONST RECT* pSrcRect,
									D3DX_FILTER_TRIANGLE,	//DWORD Filter
									0);			//D3DCOLOR ColorKey
			if( FAILED(hr) )
			{
				ASSERT(0) ;
#ifdef DEBUG_MODE
				printf("[ WARNING ]D3DXLoadSurfaceFromMemory:(%d:%d,%d,%d,%d:%08X)-->\n"
						"\t(%d:%d,%d,%d,%d)\n",
										fmt,
										src_rect.left, src_rect.top,
										src_rect.right, src_rect.bottom,
										(DWORD)img_data,
										texfmt,
										dst_rect.left, dst_rect.top,
										dst_rect.right, dst_rect.bottom) ;
#endif

				/*-- 内容だけはClearしておく -----------------------*/

				DG_ClearSurfaceImageFF(surface) ;
				/*--------------------------------------------------*/
			}
			/*------------------------------------------------------*/

			/*-- 転送元解放処理 ------------------------------------*/

			if( src_surface )
			{
				IDirect3DSurface8_UnlockRect(src_surface) ;	// Lock解除
				IDirect3DSurface8_Release(src_surface) ;
			}
			/*------------------------------------------------------*/

			/*-- 転送先解放処理 ------------------------------------*/

			IDirect3DSurface8_Release(surface) ;
			/*------------------------------------------------------*/

			/*-- 次のMipMapサイズ計算 ------------------------------*/

			src_rect = dst_rect ;

			dst_rect.left   >>= 1 ;
			dst_rect.right  >>= 1 ;
			dst_rect.top    >>= 1 ;
			dst_rect.bottom >>= 1 ;
			/*------------------------------------------------------*/
		}
	}
	/*--------------------------------------------------------------*/

}

#endif /* _WINDOWS */


/* ---------------------------------------------------------------- */
	/*
		テクスチャの設定ルーチン
	*/
static void LoadImageTriTextureData( DG_TEXTURE_LIST *tex_list, int init_flag )
{
	int		j, flag, mode ;
	DG_TEX	*tex ;

	flag = tex_list->header->compress_flag ;

	if ( flag & TRI_FLAG_TRANS || 0){
		mode = 1 ;
	} else {
		mode = 0 ;
	}


	{/* DirectXテクスチャの作成 */
		XTI_TEXINFO		*tex_info ;
		DG_TEXPACK		*texpack ;
		int				i ;
		int				n_info ;

		n_info  = (int)tex_list->header->n_info ;
		texpack = tex_list->texpack ;
		for ( i = 0 ; i < n_info ; i++ ){
			void	*org_tex_addr, *org_clut_addr ;

			tex_info = texpack->texinfo ;
			org_tex_addr = (void*)( (int)tex_info->texel_addr + (int)tex_list->header );
			if ( tex_info->clut_addr != 0 ){
				org_clut_addr = (void*)( (int)tex_info->clut_addr + (int)tex_list->header );
			}
#ifdef DEBUG_MODE
			printf("load tex mem : %p %d(%d %d)\n",
				   org_tex_addr, tex_info->use_size, tex_info->width, tex_info->height  );
#endif
#if 1
			if ( tex_info->mip_level == 0 ) tex_info->mip_level = 1 ;
#endif
			texpack->data = org_tex_addr ;
			LoadImageTriCreateTexture(tex_info, texpack) ;	// テクスチャ作成

#ifdef DEBUG_MODE
			printf("alloc tex mem : %p %d(%d %d)\n",
				   texpack->data, tex_info->use_size, tex_info->width, tex_info->height  );
#endif
			texpack++ ;
		}
	}

	tex = tex_list->textures ;
	for ( j = tex_list->header->n_textures ; j > 0 ; j--, tex++ ){
		/* 生成したDirect3Dテクスチャを設定する */
		ASSERT( tex->texinfonum < (int)tex_list->header->n_info ) ;
		tex->tex_trans.ptex = tex_list->texpack[ tex->texinfonum ].p_d3d_texture ;

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
	return ;
}


/*--------------------------------------------------------------*/
/*	DG_CheckUsableTextureFormats	使用可能Texture検査			*/
/*--------------------------------------------------------------*/

typedef	struct	SelUsableTexFmt_
{
	D3DFORMAT	org ;
	D3DFORMAT	sel ;
} SelUsableTexFmt ;

typedef	struct	CheckUsableTexFmt_
{
	D3DFORMAT	fmt ;

	DWORD	flag ;
	#define	CTF_C32BIT		(1 << 0)	// 32bit級 Color
	#define	CTF_USEALPHA	(1 << 1)	// Alpha使用
	#define	CTF_HAL_ONLY	(1 << 2)	// HALのみで使用可能
} CheckUsableTexFmt ;

static	CheckUsableTexFmt	_check_texfmt_tbl[] =
{
	{ D3DFMT_DXT5,		CTF_C32BIT | CTF_USEALPHA | CTF_HAL_ONLY },
	{ D3DFMT_DXT4,		CTF_C32BIT | CTF_USEALPHA | CTF_HAL_ONLY },
	{ D3DFMT_DXT3,		CTF_C32BIT | CTF_USEALPHA | CTF_HAL_ONLY },
	{ D3DFMT_DXT2,		CTF_C32BIT | CTF_USEALPHA | CTF_HAL_ONLY },
	{ D3DFMT_DXT1,		CTF_C32BIT | CTF_USEALPHA | CTF_HAL_ONLY },

	{ D3DFMT_R8G8B8,		CTF_C32BIT },
	{ D3DFMT_A8R8G8B8,		CTF_C32BIT | CTF_USEALPHA },
	{ D3DFMT_X8R8G8B8,		CTF_C32BIT },
	{ D3DFMT_R5G6B5,		0 },
	{ D3DFMT_X1R5G5B5,		0 },
	{ D3DFMT_A1R5G5B5,		CTF_USEALPHA },
	{ D3DFMT_A4R4G4B4,		CTF_USEALPHA },
	{ D3DFMT_R3G3B2,		0 },
	{ D3DFMT_A8,			CTF_USEALPHA },
	{ D3DFMT_A8R3G3B2,		CTF_USEALPHA },
	{ D3DFMT_X4R4G4B4,		0 },
	{ D3DFMT_A2B10G10R10,	CTF_C32BIT | CTF_USEALPHA },
	{ D3DFMT_G16R16,		CTF_C32BIT },

	{ D3DFMT_A8P8,		CTF_USEALPHA | CTF_HAL_ONLY },
	{ D3DFMT_P8,		CTF_HAL_ONLY },

	{ D3DFMT_L8,		0 },
	{ D3DFMT_A8L8,		0 },
	{ D3DFMT_A4L4,		0 },

	{ D3DFMT_V8U8,			0 },
	{ D3DFMT_L6V5U5,		0 },
	{ D3DFMT_X8L8V8U8,		0 },
	{ D3DFMT_Q8W8V8U8,		0 },
	{ D3DFMT_V16U16,		0 },
	{ D3DFMT_W11V11U10,		0 },
	{ D3DFMT_A2W10V10U10,	0 },

	{ D3DFMT_UYVY,		0 },

#if FALSE
	{ D3DFMT_D16_LOCKABLE,	0 },
	{ D3DFMT_D32,			0 },
	{ D3DFMT_D15S1,			0 },
	{ D3DFMT_D24S8,			0 },
	{ D3DFMT_D16,			0 },
	{ D3DFMT_D24X8,			0 },
	{ D3DFMT_D24X4S4,		0 },
#endif
} ;
#define	MAX_CHECK_USEABLE_TEXFMT_NUM	(sizeof(_check_texfmt_tbl)/sizeof(_check_texfmt_tbl[0]))

static	D3DFORMAT	_sub_texfmt_c32a_tbl[] =	// 32bit, Alpha付き
{
	D3DFMT_A8R8G8B8,	// 32bit, c24 a8
	D3DFMT_A4R4G4B4,	// 16bit, c12, a4
	D3DFMT_A2B10G10R10,	// 32bit, c30, a2
	D3DFMT_A1R5G5B5,	// 16bit, c15, a1
	D3DFMT_A8R3G3B2,	// 16bit, c8, a8
	D3DFMT_R8G8B8,		// 24bit, c24
	D3DFMT_X8R8G8B8,	// 32bit, c24
	D3DFMT_R5G6B5,		// 16bit, c16
	D3DFMT_X1R5G5B5,	// 16bit, c15
	D3DFMT_X4R4G4B4,	// 16bit, c12
	D3DFMT_R3G3B2,		// 8bit,  c8

	D3DFMT_UNKNOWN,
} ;


static	D3DFORMAT	_sub_texfmt_c32_tbl[] =
{
	D3DFMT_R8G8B8,		// 24bit, c24
	D3DFMT_X8R8G8B8,	// 32bit, c24
	D3DFMT_A2B10G10R10,	// 32bit, c30, a2
	D3DFMT_A8R8G8B8,	// 32bit, c24 a8
	D3DFMT_R5G6B5,		// 16bit, c16
	D3DFMT_X1R5G5B5,	// 16bit, c15
	D3DFMT_A1R5G5B5,	// 16bit, c15, a1
	D3DFMT_A4R4G4B4,	// 16bit, c12, a4
	D3DFMT_X4R4G4B4,	// 16bit, c12
	D3DFMT_R3G3B2,		// 8bit,  c8
	D3DFMT_A8R3G3B2,	// 16bit, c8, a8

	D3DFMT_UNKNOWN,
} ;

static	D3DFORMAT	_sub_texfmt_c16a_tbl[] =	// 16bit, Alpha付き
{
	D3DFMT_A4R4G4B4,	// 16bit, c12, a4
	D3DFMT_A1R5G5B5,	// 16bit, c15, a1
	D3DFMT_A8R3G3B2,	// 16bit, c8, a8
	D3DFMT_A8R8G8B8,	// 32bit, c24 a8
	D3DFMT_A2B10G10R10,	// 32bit, c30, a2

	D3DFMT_R5G6B5,		// 16bit, c16
	D3DFMT_X1R5G5B5,	// 16bit, c15
	D3DFMT_X4R4G4B4,	// 16bit, c12
	D3DFMT_R8G8B8,		// 24bit, c24
	D3DFMT_X8R8G8B8,	// 32bit, c24
	D3DFMT_R3G3B2,		// 8bit,  c8

	D3DFMT_UNKNOWN,
} ;

static	D3DFORMAT	_sub_texfmt_c16_tbl[] =	// 16bit
{
	D3DFMT_R5G6B5,		// 16bit, c16
	D3DFMT_X1R5G5B5,	// 16bit, c15
	D3DFMT_X4R4G4B4,	// 16bit, c12
	D3DFMT_A1R5G5B5,	// 16bit, c15, a1
	D3DFMT_A4R4G4B4,	// 16bit, c12, a4
	D3DFMT_A8R3G3B2,	// 16bit, c8, a8
	D3DFMT_A8R8G8B8,	// 32bit, c24 a8
	D3DFMT_A2B10G10R10,	// 32bit, c30, a2
	D3DFMT_R8G8B8,		// 24bit, c24
	D3DFMT_X8R8G8B8,	// 32bit, c24
	D3DFMT_R3G3B2,		// 8bit,  c8

	D3DFMT_UNKNOWN,
} ;


static	SelUsableTexFmt	_sel_usable_texfmt_tbl[MAX_CHECK_USEABLE_TEXFMT_NUM] ;

int			DG_CheckUsableTextureFormats(void)
{
	int					i ;
	SelUsableTexFmt		*tbl ;
	CheckUsableTexFmt	*chktbl ;
	D3DFORMAT			fmt ;
	D3DFORMAT			*subtbl ;
	D3DFORMAT			sub_fmt ;
	HRESULT				hr ;
	DWORD				flag ;
	BOOL				dev_type_hal ;

	ErrorLogPrintf("/*-- CheckUsableTextureFormats --*/\n") ;	// Log

	dev_type_hal = (DG_WinApp.d3d_cap.DeviceType == D3DDEVTYPE_HAL) ;

	/*-- 使用可能判定 ------------------------------------------*/

	tbl    = _sel_usable_texfmt_tbl ;
	chktbl = _check_texfmt_tbl ;
	for(i=MAX_CHECK_USEABLE_TEXFMT_NUM; i>0; i--, tbl++, chktbl++)
	{
		fmt  = chktbl->fmt ;
		flag = chktbl->flag ;
		tbl->org = fmt ;

		if( !dev_type_hal && (flag & CTF_HAL_ONLY) )
		{
			hr = D3DERR_NOTAVAILABLE ;	// HALじゃないとダメ
		}
		else
		{
			hr = IDirect3D8_CheckDeviceFormat(g_pD3D,
									DG_WinApp.d3d_cap.AdapterOrdinal,
									DG_WinApp.d3d_cap.DeviceType,
									DG_WinApp.d3dpp.BackBufferFormat,
 									0,
									D3DRTYPE_TEXTURE,
									fmt) ;
		}

		if( !FAILED(hr) )
		{
			tbl->sel = fmt ;	// 使用可能
			continue ;
		}

		/*-- 代替フォーマット候補テーブル選択 ------------------*/

		if( flag & CTF_C32BIT )
		{
			if( flag & CTF_USEALPHA ){ subtbl = _sub_texfmt_c32a_tbl ; }
			else{ subtbl = _sub_texfmt_c32_tbl ; }
		}
		else
		{
			if( flag & CTF_USEALPHA ){ subtbl = _sub_texfmt_c16a_tbl ; }
			else{ subtbl = _sub_texfmt_c16_tbl ; }
		}
		/*------------------------------------------------------*/

		/*-- 代替候補検査 --------------------------------------*/

		tbl->sel = D3DFMT_A8R8G8B8 ;	// 安全策
		while( (sub_fmt = *subtbl) != D3DFMT_UNKNOWN )
		{
			if( fmt != sub_fmt )	// 元テクスチャ形式は使用できない...
			{
				hr = IDirect3D8_CheckDeviceFormat(g_pD3D,
								DG_WinApp.d3d_cap.AdapterOrdinal, 	// 使用中のアダプタ番号
								DG_WinApp.d3d_cap.DeviceType,
								DG_WinApp.d3dpp.BackBufferFormat,	// Display Format
 								0,
								D3DRTYPE_TEXTURE,					// Texture使用指定
								sub_fmt) ;							// 検査するFormat

				if( !FAILED(hr) )
				{
					ErrorLogPrintf("\tFormat %d --> %d\n", fmt, sub_fmt) ;	// Log

					tbl->sel = sub_fmt ;	// 代替フォーマット使用可能
					break ;
				}
			}
			subtbl++ ;
		}
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/

	return(1) ;
}

/*--------------------------------------------------------------*/
/*	DG_SelectUsableTextureFormat	使用可能Textureに変換		*/
/*--------------------------------------------------------------*/
D3DFORMAT	DG_SelectUsableTextureFormat(D3DFORMAT original)
{
	int				i ;
	SelUsableTexFmt	*tbl ;
	D3DFORMAT		fmt ;

	fmt = D3DFMT_UNKNOWN  ;
	tbl = _sel_usable_texfmt_tbl ;
	for(i=MAX_CHECK_USEABLE_TEXFMT_NUM; i>0; i--, tbl++)
	{
		if( tbl->org == original )
		{
			fmt = tbl->sel ;
			break ;				// 検索終了
		}
	}
	return(fmt) ;
}


/*--------------------------------------------------------------*/
/*	DG_FreeTextureList		DG_TEXTURE_LISTの解放処理			*/
/*--------------------------------------------------------------*/
static void DG_FreeTextureList( DG_TEXTURE_LIST	*tex_list )
{
	int	i ;
	DG_TEXPACK *texpack ;

	DG_ExeBufferSync(FALSE) ;	// 安全策

	texpack = tex_list->texpack ;
#ifdef _DEBUG_MODE
printf("tex_list:0x%08X\n", tex_list) ;
printf("tex_list->header:0x%08X\n", tex_list->header) ;
#endif
	for(i=tex_list->header->n_info; i>0; i--, texpack++)
	{
		if( texpack->p_d3d_texture )
		{
			DG_ReleaseD3DTexture(texpack->p_d3d_texture) ;	// Texture解放
			texpack->p_d3d_texture = NULL ;
		}
	}
}

/*--------------------------------------------------------------*/
/*	DG_ReleaseTexture		テクスチャの解放処理				*/
/*--------------------------------------------------------------*/
void DG_ReleaseTexture(void)
{
	DG_InitTextureCache() ;	// やっている事は同じ
}

/*--------------------------------------------------------------*/
/*	DG_ClearSurfaceImage	サーフェイスイメージのClear			*/
/*--------------------------------------------------------------*/
void DG_ClearSurfaceImage(LPDIRECT3DSURFACE8 surface)
{
	D3DSURFACE_DESC	desc ;
	D3DLOCKED_RECT	lrect ;
	HRESULT			hr ;
	extern	DWORD	GetSurfaceSize(D3DSURFACE_DESC *) ;	// wimage.c


	/*-- Surface情報取得 -------------------------------------------*/

	hr = IDirect3DSurface8_GetDesc(surface, &desc) ;
	ASSERT( !FAILED(hr) ) ;
	if( FAILED(hr) ){ return ; }
	/*--------------------------------------------------------------*/

	/*-- Zero Clear ------------------------------------------------*/

	if( desc.Usage & D3DUSAGE_RENDERTARGET )
	{
		LPDIRECT3DSURFACE8	cur_front_surface ;
		LPDIRECT3DSURFACE8	cur_depth_surface ;

		/*-- RenderTargetの場合は描画機能を使用する ----------------*/

		IDirect3DDevice8_GetRenderTarget(g_pd3dDevice, &cur_front_surface) ;
		IDirect3DDevice8_GetDepthStencilSurface(g_pd3dDevice, &cur_depth_surface) ;

		IDirect3DDevice8_SetRenderTarget(g_pd3dDevice, surface, NULL) ;
		IDirect3DDevice8_Clear(g_pd3dDevice, 0, NULL, D3DCLEAR_TARGET, 0x00000000,
						0.0f, 0) ;

		IDirect3DDevice8_SetRenderTarget(g_pd3dDevice,
								cur_front_surface, cur_depth_surface) ;
		if( cur_front_surface ){ IDirect3DSurface8_Release(cur_front_surface) ; }
		if( cur_depth_surface ){ IDirect3DSurface8_Release(cur_depth_surface) ; }
		/*----------------------------------------------------------*/
	}
	else
	{
		/*-- Memoryを直接Zero Clear --------------------------------*/

		hr = IDirect3DSurface8_LockRect(surface,
							&lrect, NULL, D3DLOCK_NOSYSLOCK) ;

		ASSERT( !FAILED(hr) ) ;
		if( !FAILED(hr) )
		{
			DWORD	size ;

			size = GetSurfaceSize(&desc) ;	// desc.Sizeが信用できないので手動計算

			ZeroMemory(lrect.pBits, size) ;	// Zero Clear
			IDirect3DSurface8_UnlockRect(surface) ;
		}
		/*----------------------------------------------------------*/
	}
	/*--------------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	DG_ClearSurfaceImageFF	サーフェイスイメージのClear			*/
/*--------------------------------------------------------------*/
static	void DG_ClearSurfaceImageFF(LPDIRECT3DSURFACE8 surface)
{
	D3DSURFACE_DESC	desc ;
	D3DLOCKED_RECT	lrect ;
	HRESULT			hr ;
	extern	DWORD	GetSurfaceSize(D3DSURFACE_DESC *) ;	// wimage.c


	/*-- Surface情報取得 -------------------------------------------*/

	hr = IDirect3DSurface8_GetDesc(surface, &desc) ;
	ASSERT( !FAILED(hr) ) ;
	if( FAILED(hr) ){ return ; }
	/*--------------------------------------------------------------*/

	/*-- Zero Clear ------------------------------------------------*/

	if( desc.Usage & D3DUSAGE_RENDERTARGET )
	{
		LPDIRECT3DSURFACE8	cur_front_surface ;
		LPDIRECT3DSURFACE8	cur_depth_surface ;

		/*-- RenderTargetの場合は描画機能を使用する ----------------*/

		IDirect3DDevice8_GetRenderTarget(g_pd3dDevice, &cur_front_surface) ;
		IDirect3DDevice8_GetDepthStencilSurface(g_pd3dDevice, &cur_depth_surface) ;

		IDirect3DDevice8_SetRenderTarget(g_pd3dDevice, surface, NULL) ;
		IDirect3DDevice8_Clear(g_pd3dDevice, 0, NULL, D3DCLEAR_TARGET, 0xffffffff,
						0.0f, 0) ;

		IDirect3DDevice8_SetRenderTarget(g_pd3dDevice,
								cur_front_surface, cur_depth_surface) ;
		if( cur_front_surface ){ IDirect3DSurface8_Release(cur_front_surface) ; }
		if( cur_depth_surface ){ IDirect3DSurface8_Release(cur_depth_surface) ; }
		/*----------------------------------------------------------*/
	}
	else
	{
		/*-- Memoryを直接Zero Clear --------------------------------*/

		hr = IDirect3DSurface8_LockRect(surface,
							&lrect, NULL, D3DLOCK_NOSYSLOCK) ;

		ASSERT( !FAILED(hr) ) ;
		if( !FAILED(hr) )
		{
			DWORD	size ;

			size = GetSurfaceSize(&desc) ;	// desc.Sizeが信用できないので手動計算

			memset(lrect.pBits, 0xff, size) ;
			IDirect3DSurface8_UnlockRect(surface) ;
		}
		/*----------------------------------------------------------*/
	}
	/*--------------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	DG_ClearTextureImage	テクスチャイメージのClear			*/
/*--------------------------------------------------------------*/
void DG_ClearTextureImage(LPDIRECT3DTEXTURE8 tex)
{
	int					i ;
	int					mip_lv ;
	LPDIRECT3DSURFACE8	surface ;
	HRESULT				hr ;

	mip_lv = IDirect3DBaseTexture8_GetLevelCount(tex) ;
	for(i=0; i<mip_lv; i++)
	{
		hr = IDirect3DTexture8_GetSurfaceLevel(tex, i, &surface) ;
		ASSERT( !FAILED(hr) ) ;

		if( FAILED(hr) ){ continue ; }

		if( surface )
		{
			DG_ClearSurfaceImage(surface) ;
			IDirect3DSurface8_Release(surface) ;
		}
	}
}

/*--------------------------------------------------------------------------*/
/*	AjustTextureWidth	テクスチャサイズ補正(2^nに成る様)					*/
/*--------------------------------------------------------------------------*/
static DWORD AjustTextureWidth(DWORD width)
{
	int		shift ;
	DWORD	bit ;

	if( !width ){ return(0) ; }

	bit = width ;
	for(shift=0; shift<31; shift++, bit>>=1)
	{
		if( bit == 1 ){ break ; }
	}
	bit = (1 << shift) ;

	if( (width & (bit - 1)) ){ bit <<= 1 ; }

	ASSERT( bit >= width ) ;
	return( bit ) ;
}

/*--------------------------------------------------------------------------*/
/*	DG_CreateUserTexture													*/
/*			User Texture作成												*/
/*--------------------------------------------------------------------------*/
HRESULT DG_CreateUserTexture(D3DFORMAT fmt, DWORD w, DWORD h, DWORD flag,
									DG_USERTEX *usertex)
{
	HRESULT				hr ;
	DWORD				tex_w, tex_h ;
	LPDIRECT3DTEXTURE8	tex ;
	DWORD				usage ;
	BOOL				done ;
	DWORD				pool ;

	ZeroMemory(usertex, sizeof(DG_USERTEX)) ;

	/*-- テクスチャサイズ補正 ----------------------------------------------*/

	tex_w = AjustTextureWidth(w) ;
	tex_h = AjustTextureWidth(h) ;
	/*----------------------------------------------------------------------*/

	/*-- テクスチャ作成 ----------------------------------------------------*/

	usage = 0 ;
	pool  = D3DPOOL_MANAGED ;

	if( flag & DG_CLEATE_USERTEX_RENDERTARGET )
	{
		usage |= D3DUSAGE_RENDERTARGET ;
		pool   = D3DPOOL_DEFAULT ;		// MANAGEDは使用できない
	}

	done = FALSE ;
	while( !done )
	{
		hr = IDirect3DDevice8_CreateTexture( g_pd3dDevice,
					tex_w, tex_h, 1, usage,
					fmt, pool, &tex) ;

		switch( hr )
		{
		  case D3DERR_OUTOFVIDEOMEMORY :	// メモリ不足
		  case E_OUTOFMEMORY :				// メモリ不足
			if( (tex_w <= 1) && (tex_h <= 1) )
			{
				done = TRUE ;	// これ以上使用メモリ削減は無理なので終了
			}
			else
			{
				if( tex_w > tex_h ){ tex_w >>= 1 ; }
				else{ tex_h >>= 1 ; }
			}
			break ;

		  default :
			done = TRUE ;	// その他の状態なら終了
			break ;
		}
	}

	if( FAILED(hr) ){ ASSERT(0) ; return(hr) ; }
	/*----------------------------------------------------------------------*/

	/*-- Surface取得 -------------------------------------------------------*/

	hr = IDirect3DTexture8_GetSurfaceLevel(tex, 0, &usertex->surface) ;
	if( FAILED(hr) )
	{
		DG_ReleaseD3DTexture(tex) ;
		ASSERT(0) ;
		return(hr) ;				// 異常終了
	}
	/*----------------------------------------------------------------------*/

	/*-- パラメータ設定 ----------------------------------------------------*/

	usertex->texture = tex ;
	usertex->format  = fmt ;
	usertex->width   = (u_short)w ;
	usertex->height  = (u_short)h ;

	usertex->img_width  = (u_short)tex_w ;
	usertex->img_height = (u_short)tex_h ;

	if( w > tex_w ){ usertex->texcoord_scale_w = 1.0f ; }
	else{ usertex->texcoord_scale_w = (float)w/(float)tex_w ; }
	if( h > tex_h ){ usertex->texcoord_scale_h = 1.0f ; }
	else{ usertex->texcoord_scale_h = (float)h/(float)tex_h ; }

	usertex->tex_param.vx = usertex->texcoord_scale_w * (1.0f/16384.0f) ;
	usertex->tex_param.vy = usertex->texcoord_scale_h * (1.0f/16384.0f) ;

	usertex->flag = flag ;
	/*----------------------------------------------------------------------*/

	return(S_OK) ;
}

/*--------------------------------------------------------------------------*/
/*	DG_ReleaseUserTexture													*/
/*			User Texture解放												*/
/*--------------------------------------------------------------------------*/
void DG_ReleaseUserTexture(DG_USERTEX *usertex)
{
	DG_ExeBufferSync(FALSE) ;	//安全策

	if( usertex->surface )
	{
		IDirect3DSurface8_Release(usertex->surface) ;
		usertex->surface = NULL ;
	}

	if( usertex->texture )
	{
		DG_ReleaseD3DTexture(usertex->texture) ;
		usertex->texture = NULL ;
	}
}

/*--------------------------------------------------------------------------*/
/*	DG_ResetDeviceRelease_UserTexture										*/
/*			ResetDevice処理の為のUserTexture解放処理						*/
/*--------------------------------------------------------------------------*/
void DG_ResetDeviceRelease_UserTexture(DG_USERTEX *usertex)
{
	DG_ReleaseUserTexture(usertex) ;
}


/*--------------------------------------------------------------------------*/
/*	DG_ResetDeviceCreate_UserTexture										*/
/*			ResetDevice処理の為のUserTexture再作成処理						*/
/*--------------------------------------------------------------------------*/
HRESULT DG_ResetDeviceCreate_UserTexture(DG_USERTEX *usertex)
{
	HRESULT	hr ;

	hr = DG_CreateUserTexture(usertex->format,
							usertex->width, usertex->height,
							usertex->flag,
							usertex) ;
	return(hr) ;
}

