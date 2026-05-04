/*
	text.cpp
	テクスチャ管理ルーチン

	1999/07/07 K.Takabe
	$Id: text.cpp,v 1.1.1.3 2002/11/19 11:42:31 Yoshizawa1 Exp $

*/

/*

	----------------------------------------------------------------

	void DG_InitTextureCache(void)

		ロード時の初期化


	int DG_LoadInitTri(void *buf, int code)

		テクスチャの登録関数


	void DG_FreeTextureCache(void *buf)
	void	*buf;		開放するＴＲＩアドレス

		任意テクスチャの開放（テクスチャ初期化をステージロードと非同期に行う場合に使用）


	----------------------------------------------------------------

	void DG_SetTextureTmpClut(int which)
	int		which;		設定バッファ

		一時ＣＬＵＴイメージ情報の設定（システム内部で使用）


	void DG_ResetTextureTmpClut(void)

		一時ＣＬＵＴイメージ情報のクリア（システム内部で使用）

	----------------------------------------------------------------

	DG_TEX* DG_GetTexture(unsigned int code)
	unsigned int	code;	検索テクスチャＩＤコード

		テクスチャ情報の取得（半透明用テクスチャ内からテクスチャを検索する）


	DG_TEX* DG_GetTexture2(unsigned int tri_code, unsigned int code)
	unsigned int	tri_code;	検索対象ＴＲＩコード
	unsigned int	code;		検索テクスチャＩＤコード

		テクスチャ情報の取得（任意ＴＲＩ内からテクスチャを検索）


	DG_TEX_PACKET* DG_GetTexturePacket(unsigned int code)
	unsigned int	code;		取得するパケットのＴＲＩコード

		テクスチャ転送パケットのアドレスを取得


	DG_TEXTURE_LIST* DG_GetTextureList(unsigned int code)
	unsigned int	code;		取得するテクスチャリスト構造体のＴＲＩコード

		テクスチャリスト構造体のアドレスを取得

	----------------------------------------------------------------

	int	DG_SearchTriFromKms(DG_DEF *def)
	DG_DEF	*def;		モデルデータ(*.kms)へのポインタ

		モデルに対応したＴＲＩ検索


	int	DG_SearchTriFromTex(DG_TEX *org)
	DG_TEX	*org;		テクスチャ構造体へのポインタ

		テクスチャに対応したＴＲＩ検索

	----------------------------------------------------------------

	void *DG_MakeLoadImagePacket(void *tag_addr,
			int fmt, int width, int height,
			int vram_addr, int vram_width, void *tex_addr)
	void	*tag_addr;		パケットを書き出すアドレス。内容はDG_LOADIMAGEと同じ構成になる
	int		fmt;			転送カラーフォーマット指定(SCE_GS_PSM??)
	int		width;			転送幅。上位１６ビットで転送先オフセットを指定可能
	int		height;		転送高さ。上位１６ビットで転送先オフセットを指定可能
	int		vram_addr;		転送先ベースアドレス（６４で割る前の値）
	int		vram_width;	転送先ＶＲＡＭ幅（６４で割る前の値）
	void	*tex_addr;		転送データアドレス

		テクスチャ転送パケットの作成
		width,heightの上位１６ビットで任意のオフセット位置から転送することも可能
		vram_addr,vram_widthはともに６４で割る前の値を指定すること
		戻り値にはパケットの次のアドレスが返す


	void *DG_MakeMoveImagePacket(void *tag_addr,
			int dst_addr, int dst_width, int dst_fmt, int dst_x, int dst_y,
			int src_addr, int src_width, int src_fmt, int src_x, int src_y,
			int width, int height, int dir)
	void	*tag_addr;		パケットを書き出すアドレス。内容はDG_MOVEIMAGEと同じ構成になる
	int		dst_addr;		転送先ベースアドレス（６４で割る前の値）
	int		dst_width;		転送先ＶＲＡＭ幅（６４で割る前の値）
	int		dst_fmt;		転送先カラーフォーマット指定(SCE_GS_PSM??)
	int		dst_x;			転送先オフセット
	int		dst_y;			転送先オフセット
	int		src_addr;		転送元ベースアドレス（６４で割る前の値）
	int		src_width;		転送元ＶＲＡＭ幅（６４で割る前の値）
	int		src_fmt;		転送元カラーフォーマット指定(SCE_GS_PSM??)
	int		src_x;			転送元オフセット
	int		src_y;			転送元オフセット
	int		width;			転送幅
	int		height;		転送高さ
	int		dir;			転送方向

		テクスチャ転送パケットの作成
		dst_addr,dst_width,src_addr,src_widthは６４で割る前の値を指定すること
		戻り値にはパケットの次のアドレスが返す


	void DG_GetTexelInfo(int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex)
	int		*width;	テクスチャ本来の幅を格納する変数へのポインタ（テクセル単位）
	int		*height;	テクスチャ本来の高さを格納する変数へのポインタ（テクセル単位）
	int		*offset_x;	テクスチャ本来のオフセットを格納する変数へのポインタ（テクセル単位）
	int		*offset_y;	テクスチャ本来のオフセットを格納する変数へのポインタ（テクセル単位）
	DG_TEX	*tex;		情報を取得するテクスチャ

		テクスチャのピクセル数を計算する
		（通常はバイリニアフィルタによる色の混合を避けるため0.5ピクセル内側を
		マッピングするようにしているため、この関数を用いないと正確なサイズなどがわからない）

	----------------------------------------------------------------

	DG_TEX_MOVEREPLACE *DG_MakeMoveReplacePacket(int tri_code, int dst_tex_id, int src_tex_id)
	int		tri_code;		入れ替え対象ＴＲＩのコード
	int		dst_tex_id;	入れ替え先テクスチャＩＤ
	int		src_tex_id;	入れ替え元テクスチャＩＤ

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを生成する
		但し入れ替え元と入れ替え先は同じフォーマット、同じサイズでなければならない


	void DG_FreeMoveReplacePacket(DG_TEX_MOVEREPLACE *tex_replace)
	DG_TEX_MOVEREPLACE	*tex_replace;	開放するパケット

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを開放する


	DG_TEX_LOADREPLACE *DG_MakeLoadReplacePacket(int tri_code, int dst_tex_id, void *texel, void *clut)

		データ再転送によるテクスチャ入れ替えパケットを生成する
		但し転送データは入れ替え先と同じフォーマット、サイズでなければならない


	void DG_FreeLoadReplacePacket(DG_TEX_LOADREPLACE *tex_replace)
	DG_TEX_LOADREPLACE	*tex_replace;	開放するパケット

		データ再転送によるテクスチャ入れ替えパケットを開放する


	void DG_SetMoveReplaceTexture(DG_TEX_MOVEREPLACE *tex_replace)
	DG_TEX_MOVEREPLACE	*tex_replace;	入れ替えパケット

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを設定する


	void DG_ResetMoveReplaceTexture(DG_TEX_MOVEREPLACE *tex_replace)
	DG_TEX_MOVEREPLACE	*tex_replace;	入れ替えパケット

		ＶＲＡＭ内移動によるテクスチャ入れ替えパケットを解除する


	void DG_SetLoadReplaceTexture(DG_TEX_LOADREPLACE *tex_replace)
	DG_TEX_LOADREPLACE	*tex_replace;	入れ替えパケット

		データ再転送によるテクスチャ入れ替えパケットを設定する


	void DG_ResetLoadReplaceTexture(DG_TEX_LOADREPLACE *tex_replace)
	DG_TEX_LOADREPLACE	*tex_replace;	入れ替えパケット

		データ再転送によるテクスチャ入れ替えパケットを解除する


	----------------------------------------------------------------
	----------------------------------------------------------------

*/

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#include <xgraphics.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "libgv.h"
#include "libdg.h"
#include "libdg.cnf"

//------------------------------------------------------------------------------------

// とりあえずテクスチャがばらばらなので、枚数制限を増やしておく。
#if 0
#ifndef DESIGN_PREVIEW
#define MAX_TEX_CACHE	(64)		/* 一度に読み込み可能なTRIファイル数 */
#define MAX_TEX_BUFFER	(1024)		/* ゲーム中で使用できるテクスチャ枚数 */
#else
#define MAX_TEX_CACHE	(128)		/* 一度に読み込み可能なTRIファイル数 */
#define MAX_TEX_BUFFER	(4096)		/* ゲーム中で使用できるテクスチャ枚数 */
#endif
#endif // 0

//XBOX
#undef MAX_TEX_CACHE
#undef MAX_TEX_BUFFER
#define MAX_TEX_CACHE  (2048)
#define MAX_TEX_BUFFER (2048)
#define MAX_TEX_NUM		(2048)

int DG_MaxTextures = 0;
int DG_MaxTexBuffer = 0;
int DG_LastLoadTexture;
DG_TEXTURE_LIST	DG_TextureList[MAX_TEX_CACHE];
static int				DG_UseLocalTextures  = 0 ;
static DG_TEX			DG_LocalTextureList[ MAX_TEX_NUM ];

#define TEXTURE_FLAG_USEMALLOC		(0x0001)


// デフォルトのテクスチャ情報
static DG_TEX DG_DefaultTexture = {
	0.0f, 0.0f, // テクスチャ開始U座標/V座標
	1.0f, 1.0f, // スケール補正値
	0,          // tex_id
	0,          // tri_id
	0,          // flag
	0,          // pad
};
/*
   DG_SystemTexture[0]:フレームバッファ０（512x512x32bit）
   DG_SystemTexture[1]:フレームバッファ１（512x512x32bit）
   DG_SystemTexture[2]:テクスチャバッファ（512x512x32bit）
   DG_SystemTexture[3]:テクスチャバッファ（512x512x16bit）
*/

extern "C" LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;		/* バックバッファテクスチャ */
extern "C" {
DG_TEX			DG_SystemTexture[] = {
	{/* FrameBuffer0-32bit */
		(DISPLAY_WIDTH-DRAW_WIDTH)/2, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT,
		0, 0, 0, {0},
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
		0, 0, 0, {0},
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
		0, 0, 0, {0},
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
		0, 0, 0, {0},
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
}




#if 0
// テクスチャフラグ関連 (MDU_tex.h)
// DG_TEX.flagに入ってくる値
enum {
    MDU_TEX_FLAG_COLOR16    = 0x0000,
    MDU_TEX_FLAG_COLOR256   = 0x0001,
    MDU_TEX_FLAG_COLOR_FULL = 0x0002,
    MDU_TEX_FLAG_ALPHA_REG  = 0x0004, // アルファブレンドモード
    MDU_TEX_FLAG_CLUT_EXIST = 0x0008,

	MDU_TEX_FLAG_TRANS_HALF = 0x0000,
	MDU_TEX_FLAG_TRANS_ADD  = 0x0010,
	MDU_TEX_FLAG_TRANS_SUB  = 0x0020,
	MDU_TEX_FLAG_TRANS_BLD  = 0x0030,
	MDU_TEX_FLAG_DECAL      = 0x0100,

	MDU_TEX_FLAG_EMAP       = 0x00010000,
	MDU_TEX_FLAG_BMAP       = 0x00020000,
	MDU_TEX_FLAG_SMAP       = 0x00040000,

	MDU_TEX_FLAG_BLACK_NOTTRANS = 0x1000, // 黒は不透明に
	MDU_TEX_FLAG_COLOR_TRANS    = 0x2000, // 半透明ＯＮ
};
#endif

#define DG_TEX_COLOR_MASK (0x00000003) // テクスチャカラーモード設定値マスク
#define DG_TEX_USE_AMODE  (0x00000004) // アルファブレンドモード指定フラグ
#define DG_TEX_BLEND_MODE (0x00000030) // アルファブレンドモード設定値マスク
#define DG_TEX_DECAL      (0x00000100) // 強制シェーディング禁止フラグ
#define DG_TEX_FLAG_EMAP  (0x00010000) // 環境マッピングフラグ
#define DG_TEX_FLAG_BMAP  (0x00020000) // バンプマッピングフラグ
#define DG_TEX_FLAG_SMAP  (0x00040000) // アニメ影マップフラグ
#define DG_TEX_AMODE_MASK (0xff000000) // アルファブレンドモード直接指定設定値

#define DG_TEX_FREE_BIT   (0x00f7fec8)       // 空ビット
#define DG_TEX_FLAG_ALL   (~DG_TEX_FREE_BIT) // 使用ビット

// ロード時の初期化
void DG_InitTextureCache(void)
{
	DG_MaxTextures = 0;
	DG_MaxTexBuffer = 0;
	//_tex_base = TEXTURE_TOP_PAGE();
}

// Xboxの場合、自前でテクスチャを読み込む。
// ※ツール側で縦横2の整数乗サイズに揃えておく

#ifdef KP_XBOX
// BMPフォーマットのテクスチャを読み込む(24/32bit前提)
#include "fmt_bmp.h"
#include "fmt_dds.h"

static LPDIRECT3DTEXTURE8 LoadBMP(void *pImage, int nSize, DWORD *dwWidth, DWORD *dwHeight)
{
	BITMAPFILEHEADER *header;
	BITMAPINFOHEADER *info;
	D3DFORMAT format;
	LPDIRECT3DTEXTURE8 ptex;
	D3DLOCKED_RECT lock;

	header = (BITMAPFILEHEADER *)pImage;
	info = (BITMAPINFOHEADER *)(header + 1);

	*dwWidth = info->biWidth;
	*dwHeight = info->biHeight;

	if (info->biCompression) { // ランレングス圧縮
		ASSERT(0);
	}
	
	switch (info->biBitCount) {
	case 4:
	case 8:
		format = D3DFMT_P8; // TODO: パレット付きテクスチャ
		// CreatePaletteをしないと駄目。ptexのメンバに D3DPalette を追加。
		ASSERT(0);		
		break;
	case 16:
		format = D3DFMT_A1R5G5B5;
		break;
	case 24:
	case 32:
		format = D3DFMT_A8R8G8B8;
		break;
	default:
		printf("info->biBitCount = %d\n", info->biBitCount);
		ASSERT(0);
		break;
	}

	if (FAILED(g_pd3dDevice->CreateTexture(info->biWidth, info->biHeight,
										   1, // mipmap level
										   0, // usage
										   format,
										   D3DPOOL_MANAGED,
										   &ptex))) {
		printf("CreateTexture Failed\n");
		ASSERT(0);
	}

	ptex->LockRect(0, &lock, 0, 0);	
	if (info->biBitCount != 24) {
		// 16bit, 32bit はそのままでOK
		XGSwizzleRect(info + 1, lock.Pitch, NULL, lock.pBits,
					  info->biWidth, info->biHeight, NULL,
					  info->biBitCount / 8);
	} else {
		// 24bit image -> 32bit image 変換
		// ※BMPにパディングが入るような横幅には対応していないので注意。
		int i;
		unsigned char *new_image, *src, *dst;
		new_image = (unsigned char *)GV_Malloc(info->biWidth * info->biHeight * sizeof(DWORD));
		src = (unsigned char *)(info + 1);
		dst = new_image;
		if (new_image == NULL) ASSERT(0);
		for (i = info->biWidth * info->biHeight; i > 0; i--) {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = 0x80;   // アルファ固定
			dst += 4;
			src += 3;
		}
		XGSwizzleRect(new_image, lock.Pitch, NULL, lock.pBits,
					  info->biWidth, info->biHeight, NULL, 32 / 8);
		GV_Free(new_image);
	}
	ptex->UnlockRect(0);
	return ptex;
}

// DDSフォーマットのテクスチャを読み込む
static LPDIRECT3DTEXTURE8 LoadDDS(void *pImage, int nSize, DWORD *dwWidth, DWORD *dwHeight)
{
	LPDIRECT3DTEXTURE8 ptex;
	D3DLOCKED_RECT lock;
	DDS_HEADER *header;
	D3DFORMAT format;
	char *buf;

	printf("------------------LoadDDS\n");
		
	buf = (char *)pImage;
	header = (DDS_HEADER *)(buf + sizeof(DWORD));
	
	*dwWidth = header->dwWidth;
	*dwHeight = header->dwHeight;
	if (*dwWidth > 4096 || *dwHeight > 4096) {
		printf("LoadBMP: Texture Size Over\n");
		ASSERT(0);
	}

	// TODO:ツール側で Swizzle したテクスチャが吐き出せるように
	switch (header->ddspf.dwFlags) {
	case DDS_RGB:
		// さらにBitCountで分岐
		switch (header->ddspf.dwRGBBitCount) {
		case 16: format = D3DFMT_R5G6B5; break;
		case 32: format = D3DFMT_X8R8G8B8; break;
		default: ASSERT(0); break;
		}
		if (FAILED(g_pd3dDevice->CreateTexture(header->dwWidth, header->dwHeight,
											   1, // mipmap level
											   0, // usage
											   format,
											   D3DPOOL_MANAGED,
											   &ptex))) {
			printf("CreateTexture Failed\n");
			ASSERT(0);
		}
		ptex->LockRect(0, &lock, 0, 0);
		XGSwizzleRect(header + 1, lock.Pitch, NULL, lock.pBits,
					  header->dwWidth, header->dwHeight, NULL,
					  header->ddspf.dwRGBBitCount / 8);
		ptex->UnlockRect(0);
		break;

	case DDS_RGBA:
		printf("D3DFMT_R8G8B8A8\n");
		// さらにBitCountで分岐
		switch (header->ddspf.dwRGBBitCount) {
			// D3DFMT_A4R4G4B4 と分岐する場合には dwRBitMaskを見る
		case 16: format = D3DFMT_A1R5G5B5; break;
		case 32: format = D3DFMT_A8R8G8B8; break;
		default: ASSERT(0); break;
		}
		if (FAILED(g_pd3dDevice->CreateTexture(header->dwWidth, header->dwHeight,
											   1, // mipmap level
											   0, // usage
											   format,
											   D3DPOOL_MANAGED,
											   &ptex))) {
			printf("CreateTexture Failed\n");
			ASSERT(0);
		}
		ptex->LockRect(0, &lock, 0, 0);
		XGSwizzleRect(header + 1, lock.Pitch, NULL, lock.pBits,
					  header->dwWidth, header->dwHeight, NULL,
					  header->ddspf.dwRGBBitCount / 8);
		ptex->UnlockRect(0);
		break;

	case DDS_FOURCC:
		printf("D3DFMT_FOURCC\n");
		// さらに dwFourCC で分岐
		switch (header->ddspf.dwFourCC) {
		case MAKEFOURCC('D','X','T','1'): format = D3DFMT_DXT1; break;
		case MAKEFOURCC('D','X','T','2'): format = D3DFMT_DXT2; break;
		case MAKEFOURCC('D','X','T','3'): format = D3DFMT_DXT3; break;
		case MAKEFOURCC('D','X','T','4'): format = D3DFMT_DXT4; break;
		case MAKEFOURCC('D','X','T','5'): format = D3DFMT_DXT5; break;
		default: ASSERT(0); break;			
		}
		if (FAILED(g_pd3dDevice->CreateTexture(header->dwWidth, header->dwHeight,
											   1, // mipmap level
											   0, // usage
											   format,
											   D3DPOOL_MANAGED,
											   &ptex))) {
			printf("CreateTexture Failed\n");
			ASSERT(0);
		}
		ptex->LockRect(0, &lock, 0, 0);
		memcpy(lock.pBits, header + 1, nSize - (sizeof(DDS_HEADER) + 4));
		ptex->UnlockRect(0);
		break;
	default:
		// 未対応フォーマット
		printf("desc->ddpfPixelFormat.dwFlags = %d\n",
			   header->ddspf.dwFlags);
		printf("desc->ddpfPixelFormat.dwFourCC = %08x\n",
			   header->ddspf.dwFourCC);
		ASSERT(0);
		break;
	}
	return ptex;
}

typedef LPDIRECT3DTEXTURE8 (*TEX_LOAD_FUNC)(void *, int, DWORD *, DWORD *);

static TEX_LOAD_FUNC tex_load_func[] = {
	LoadBMP,
	LoadDDS,
};
#endif

// XBOX追加
// テクスチャファイル(PS2でTRIにあたるもの)を読み込む。
int DG_LoadInitXti(void *buf, int code)
{
	DG_TEXTURE_LIST *list;
	TRI_FILEHEADER *header;
	DWORD dwWidth, dwHeight;
	int init_flag = 0;

	ASSERT(DG_MaxTextures < MAX_TEX_CACHE); // 最大数チェック
	code &= 0xffffff;                       // コードは下位24bitが有効

	list = &DG_TextureList[DG_MaxTextures++];
printf( "DG_MaxTextures = %d\n", DG_MaxTextures );
	GV_ZeroMemory(list, sizeof(DG_TEXTURE_LIST));

	header = (TRI_FILEHEADER *)buf;
	list->header = header;
	list->code = code;
	list->flag = 0;
	list->textures = (DG_TEX *)&list->header[1];
#if 0
	{// XBOX  add takabe
		int			i ;
		DG_TEX		*dst ;
		DG_TEXX		*src ;
		list->textures = &DG_LocalTextureList[ DG_UseLocalTextures ];
		dst = &DG_LocalTextureList[ DG_UseLocalTextures ];
		src = (DG_TEXX*)&list->header[1];
		for ( i = 0 ; i < list->header->n_textures ; i++, dst++, src++, DG_UseLocalTextures++ ){
			dst->u_offset = src->u_offset ;
			dst->u_scale = src->u_scale ;
			dst->v_offset = src->v_offset ;
			dst->v_scale = src->v_scale ;
			dst->tex_id = src->tex_id ;
			dst->tri_id = src->tri_id ;
			dst->flag = src->flag ;
			dst->tex_trans.alpha.data = src->tex_trans.alpha.data ;
			dst->tex_trans.clamp.data = src->tex_trans.clamp.data ;
			dst->tex_trans.tex2.data = src->tex_trans.tex2.data ;
			dst->tex_trans.tex0.data = src->tex_trans.tex0.data ;
			dst->tex_trans.vec1 = src->tex_trans.vec1 ;
			dst->tex_trans.vec2 = src->tex_trans.vec2 ;
		}
	}
#endif
	
	DG_LastLoadTexture = code;

#ifdef KP_XBOX
	list->ptex = tex_load_func[header->format](
			(char *)header + header->tex_offset, header->tex_size,
			&dwWidth, &dwHeight);
#else
	{
		// メモリからファイルを読み込み
		HRESULT hr;
		void *buf;
		D3DXIMAGE_INFO info;
		
		buf = (char *)list->header + list->header->tex_offset;
		
		hr = D3DXCreateTextureFromFileInMemoryEx(
				g_pd3dDevice,
				buf,                    // データアドレス
				list->header->tex_size, // ファイルサイズ
				D3DX_DEFAULT,       // 幅(ファイルから取得)
				D3DX_DEFAULT,       // 高さ(ファイルから取得)
				1,                  // ミップマップレベル
				0,                  // レンダリングターゲットにするかどうか
				D3DFMT_UNKNOWN,     // テクスチャフォーマット
				D3DPOOL_DEFAULT,    // リソース配置位置
				D3DX_FILTER_LINEAR, // フィルタ
				D3DX_FILTER_LINEAR, // ミップマップフィルタ
				0xff000000,         // 透明色
				&info,              // D3DXIMAGE_INFO(返値)
				NULL,               // PALETTEENTRY(返値)
				&list->ptex);       // LPDIRECT3DTEXTURE8 (返値)
		if (FAILED(hr)) {
			DG_MaxTextures--;
			list->code = 0;
			printf("Texture Load Failed (%d) / %08x\n", code, hr);
			return 0;
		}
	}
#endif

	{
		int i;
		//DG_TEX *tex = (DG_TEX *)&list->header[1];
		DG_TEX	*tex = (DG_TEX*)list->textures ;

		for (i = 0; i < (int)list->header->n_textures; i++, tex++) {
			// DG_TEX_TRANS に LPDIRECT3DTEXTURE8 を設定
			tex->tex_trans.ptex = list->ptex;

			// tw, th の設定(PS2互換用)
			{
				unsigned int w, h, tw, th;

				tex->tex_trans.tex0.data = 0;
				w = 1;
				tw = 0;			
				while (w < dwWidth) {
					w <<= 1;
					tw++;
				} 
				h = 1;
				th = 0;
				while (h < dwHeight) {
					h <<= 1;
					th++;
				}
				tex->tex_trans.tex0.data |= ((u_long64)tw) << 26;
				tex->tex_trans.tex0.data |= ((u_long64)th) << 30;
			}

			// アルファブレンディングモード
			if (tex->flag & DG_TEX_USE_AMODE) {
				// SCE_GS_SET_ALPHA 直接指定
				int a, b, c, d, fix, flag;
				flag = (tex->flag >> 24) & 0x0f; // ここに2ビットずつ値が入っている
				a = (flag >> 30) & 0x03;
				b = (flag >> 28) & 0x03;
				c = (flag >> 26) & 0x03;
				d = (flag >> 24) & 0x03;
				fix = 0;
				tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA(a, b, c, d, fix);
			} else {
				switch ((tex->flag & DG_TEX_BLEND_MODE) >> 4) {
				  case 0:
					//printf("off\n");
					tex->tex_trans.alpha.data = 0; // 半透明モードOFF
					break;
				  case 1: // add
					//printf("add\n");
					tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA(0, 2, 0, 1, 0);
					break;
				  case 2: // sub
					//printf("sub\n");
					tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA(2, 0, 0, 1, 0);
					break;
				  case 3: // normal
					//printf("normal\n");
					tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
					break;
				}
			}
		}
	}
#if 0
	{   // ヘッダ用の領域をGV_Mallocで確保する
		TRI_FILEHEADER *new_header;
		int size;
		size = sizeof(TRI_FILEHEADER) + list->header->n_textures * sizeof(DG_TEX);
		//size = sizeof(TRI_FILEHEADER) + list->header->n_textures * sizeof(DG_TEXX);
		new_header = (TRI_FILEHEADER *)GV_Malloc(size);
		memcpy(new_header, list->header, size );
		//memcpy(new_header, list->header, sizeof(TRI_FILEHEADER) );
		printf(">>>>>>> %p %p %p %p %p\n", list->header, new_header, list->textures, &list->header[1], &new_header[1] );
		list->header = new_header;
		list->textures = (DG_TEX *)&list->header[1];
		//list->textures = (DG_TEX*) tmp_addr ;
	}
#endif
#if 0
	// ロードパラメータ確認用
	printf("tex_offset = %d\n", list->header->tex_offset);
	printf("n_textures = %d\n", list->header->n_textures);
	printf("tex_size   = %d\n", list->header->tex_size);

	tex = (DG_TEX *)(list->header + 1);
	for (i = 0; i < (int)list->header->n_textures; i ++, tex ++) {
		printf("offset = %f, %f\n", tex->u_offset, tex->v_offset);
		printf("scale  = %f, %f\n", tex->u_scale, tex->v_scale);
		printf("id     = %08x, %08x\n", tex->tri_id, tex->tex_id);
	}
#endif
	return 1;
}

// テスト用 : BMP/DDSファイルの単体読み込み
extern "C" extern int FS_FileSize; // 読み込まれたサイズ
int DG_LoadInitBmp(void *buf, int code)
{
	DG_TEXTURE_LIST *list;
	int init_flag = 0;
	DWORD dwWidth, dwHeight;
	
	ASSERT(DG_MaxTextures < MAX_TEX_CACHE); // 最大数チェック
	code &= 0xffffff;                       // コードは下位24bitが有効

	printf("######## LoadInitBmp code = %08x\n", code);
	
	list = &DG_TextureList[DG_MaxTextures ++];
	GV_ZeroMemory(list, sizeof(DG_TEXTURE_LIST));

#ifdef KP_XBOX
	char *c = (char *)buf;
	if (c[0] == 'B' && c[1] == 'M') {
		list->ptex = LoadBMP(buf, FS_FileSize, &dwWidth, &dwHeight);
	} else {
		list->ptex = LoadDDS(buf, FS_FileSize, &dwWidth, &dwHeight);		
	}
#else
	D3DXIMAGE_INFO image_info;
	{
		HRESULT hr;
		hr = D3DXCreateTextureFromFileInMemoryEx(
				g_pd3dDevice,
				buf,                // データアドレス
				FS_FileSize,        // ファイルサイズ
				D3DX_DEFAULT,       // 幅(ファイルから取得)
				D3DX_DEFAULT,       // 高さ(ファイルから取得)
				1,                  // ミップマップレベル
				0,                  // レンダリングターゲットにするかどうか
				D3DFMT_UNKNOWN,     // テクスチャフォーマット
				D3DPOOL_DEFAULT,    // リソース配置位置
				D3DX_FILTER_LINEAR, // フィルタ
				D3DX_FILTER_LINEAR, // ミップマップフィルタ
				0xff000000,         // 透明色
				&image_info,        // D3DXIMAGE_INFO(返値)
				NULL,               // PALETTEENTRY(返値)
				&list->ptex);       // LPDIRECT3DTEXTURE8 (返値)
		if (FAILED(hr)) {
			DG_MaxTextures --;
			list->code = 0;
			printf("Texture Load Failed (%d) / %08x\n", code, hr);
			ASSERT(0);
			return 0;
		}
	}
	dwWidth = image_info.Width;
	dwHeight = image_info.Height;
#endif

	{   // ヘッダ用の領域をGV_Mallocで確保する
		TRI_FILEHEADER *new_header;
		DG_TEX         *tex;
		int size;
		size = sizeof(TRI_FILEHEADER) + sizeof(DG_TEX);
		new_header = (TRI_FILEHEADER *)GV_Malloc(size);
		list->header = new_header;
		list->header->n_textures = 1;		
		list->textures = (DG_TEX *)&list->header[1];
		list->header->compress_flag |= TRI_FLAG_TRANS;
		list->code = code;
		list->flag = TEXTURE_FLAG_USEMALLOC ;	/* Malloc使用フラグ */
		tex = list->textures;
		tex->u_scale = tex->v_scale = 1.0f;
		tex->u_offset = tex->v_offset = 0.0f;
		tex->tex_id = code;
		tex->tri_id = code;
		tex->tex_trans.ptex = list->ptex;
		tex->tex_trans.alpha.data = 0;

		{
			// tw, th の設定
			unsigned int w, h, tw, th;

			tex->tex_trans.tex0.data = 0;
			w = 1;
			tw = 0;			
			while (w < dwWidth) {
				w <<= 1;
				tw++;
			} 
			h = 1;
			th = 0;
			while (h < dwHeight) {
				h <<= 1;
				th++;
			}
			tex->tex_trans.tex0.data |= tw << 26;
			tex->tex_trans.tex0.data |= th << 30;
		}
	}
	DG_LastLoadTexture = code;
	return 1;
}

// テクスチャの登録
int DG_LoadInitTri(void *buf, int code)
{
	// 未使用
	return 1;
}

// 任意テクスチャの開放
void DG_FreeTextureCache(void *buf)
{
#if 0
	int i;

	for (i = 0; i < DG_MaxTextures; i ++) {
		if ((int)(DG_TextureList[i].header) == (int)buf) {
			for (; i < DG_MaxTextures - 1; i ++) {
				DG_TextureList[i] = DG_TextureList[i + 1];
			}
			DG_MaxTextures --;
			return;
		}
	}
	return;
#else
	printf("DG_FreeTextureCache : ...\n");
	ASSERT(0);
	return;
#endif
}

// XBOX追加/全てのテクスチャを解放する。
//    プログラム終了前に必要。
//    DG_KillDaemon() から呼ばれる。
void DG_FreeTextureAll(void)
{
	int i;
	DG_TEXTURE_LIST *list;
	
	list = DG_TextureList;
	for (i = DG_MaxTextures; i > 0; i--, list++) {
		if (list->ptex != NULL) {
			list->ptex->Release();
			GV_Free(list->header);
		}
	}
	DG_MaxTextures = 0;
	DG_UseLocalTextures = 0 ;
}

// XBOX追加/テクスチャの解放
void DG_ReleaseTexture(void)
{
	int i;
	DG_TEXTURE_LIST *list;

	list = DG_TextureList;
	for (i = DG_MaxTextures; i > 0; i--, list++) {
		if (list->ptex != NULL) {
			list->ptex->Release();
			if ( list->flag & TEXTURE_FLAG_USEMALLOC ){
				GV_Free(list->header);
			}
		}
	}
	DG_MaxTextures = 0;
	DG_UseLocalTextures = 0 ;
}

	/*
		テクスチャリスト構造体のアドレスを取得
	*/
extern "C" DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code )
{
#if 0
	int		i ;
	DG_TEXTURE_LIST	**tex_list_p ;

	tex_list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0; i--, tex_list_p++ ){
		if ( ( ( *tex_list_p )->code & 0x00ffffff ) == code ){
			return ( (void*)( *tex_list_p ) );
		}
	}
#else
	int		i ;
//	for ( i = DG_MaxTextures ; i > 0; i-- ){
	for ( i = 0; i < DG_MaxTextures; i++ ){
		if ( ( DG_TextureList[i].code & 0x00ffffff ) == code ){
			return ( &DG_TextureList[i] );
		}
	}
#endif
	printf("not found tri !!(%d)\n", code );
	return NULL ;
}
// テクスチャ情報の取得
DG_TEX *DG_GetTexture(unsigned int code)
{
	int		i, j;
	DG_TEX	*tex;

	// 半透明テクスチャ内から検索
	for (i = 0; i < DG_MaxTextures; i ++) {
		if (DG_TextureList[i].header->compress_flag & TRI_FLAG_TRANS) {
			tex = (DG_TEX*)DG_TextureList[i].textures;
			if (tex == NULL) continue;
			for (j = DG_TextureList[i].header->n_textures; j > 0; j--, tex ++) {
				if (tex->tex_id == code) {
					return tex;
				}
			}
		}
	}

	// 不透明テクスチャ内から検索
	for (i = 0; i < DG_MaxTextures; i ++) {
		if (!(DG_TextureList[i].header->compress_flag & TRI_FLAG_TRANS)) {
			tex = (DG_TEX*)DG_TextureList[i].textures;
			if (tex == NULL) continue;
			for (j = DG_TextureList[i].header->n_textures; j > 0; j--, tex ++) {
				if (tex->tex_id == code) {
#if 0
					printf("%d %f %f %f %f %04x %p\n",
						   tex->tex_id,
						   tex->u_offset, tex->v_offset, tex->u_scale, tex->v_scale,
						   tex->tex_trans.alpha.data, tex->tex_trans.ptex );
#endif
					return tex;
				}
			}
		}
	}

	if (DG_MaxTextures != 0) return DG_TextureList[0].textures;
	return NULL;
}

// TODO: DG_GetTexture2
DG_TEX* DG_GetTexture2(unsigned int tri_code, unsigned int code)
{
	//return DG_GetTexture(code);
	DG_TEX		*tex ;
	int		i, j;

	// 指定ＴＲＩ内でのサーチ
	for (i = 0; i < DG_MaxTextures; i ++) {
		if (DG_TextureList[i].code == tri_code ){
			tex = (DG_TEX*)DG_TextureList[i].textures;
			for (j = DG_TextureList[i].header->n_textures; j > 0; j--, tex++ ){
				if (tex->tex_id == code) {
					return tex;
				}
			}
		}
	}
	// 半透明ＴＲＩ内でのサーチ
	for (i = 0; i < DG_MaxTextures; i ++) {
		if (DG_TextureList[i].header->compress_flag & TRI_FLAG_TRANS) {
			tex = (DG_TEX*)DG_TextureList[i].textures;
			if (tex == NULL) continue;
			for (j = DG_TextureList[i].header->n_textures; j > 0; j--, tex ++) {
				if (tex->tex_id == code) {
					return tex;
				}
			}
		}
	}
	printf("search texture error(tri=%d,code=%d)\n", tri_code, code);
	return DG_GetTexture(code);
	if (DG_MaxTextures != 0) return DG_TextureList[0].textures;
	return (NULL);
}

	/*
		テクスチャのピクセル数を計算する
	*/
extern "C" void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex )
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

// XBOX用
// code から LPDIRECT3DTEXTURE8 を検索
LPDIRECT3DTEXTURE8 DG_GetDxTexture(int code)
{
	int i;
	DG_TEXTURE_LIST *list;

	code &= 0xffffff;
	list = &DG_TextureList[0];

	//printf("code = %d\n", code);
	for (i = 0; i < DG_MaxTextures; i++, list++) {
		if (code == list->code) {
			return list->ptex;
		}
	}
	printf("texture not found\n");
	return NULL;
}

// モデルに対応したＴＲＩ検索
// ※ TODO: DG_SearchTriFromKms / ツール側で対応する必要がある
int	DG_SearchTriFromKms(DG_DEF *def)
{
	return def->texture;
}

// テクスチャに対応したＴＲＩ検索
int	DG_SearchTriFromTex(DG_TEX *org)
{
	return org->tri_id;
}

#ifndef KP_XBOX
#define DG_DATA_PATH "c:\\develop\\mgsx\\data\\"
#else
// 「.xbeのあるディレクトリ」が "d:\\" に見える。
#define DG_DATA_PATH "d:\\data\\"
#endif  // KP_XBOX

// 仮
// ファイルを読み込んで、読み込んだバッファを返す。
// バッファはGV_Mallocで確保する。
void *DG_LoadFile(char *name)
{
	void *buf;
	DWORD dwSize;
	HANDLE hFile;
	char full_path[256];

	strcpy(full_path, DG_DATA_PATH);
	strcat(full_path, name);

	// WindowsAPIを使ったファイル読み込み
	hFile = CreateFile(full_path, GENERIC_READ, FILE_SHARE_READ, NULL, 
					   OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile) {
		printf("file \"%s\" read failed\n", full_path);
		HANGUP();
    }

	dwSize = GetFileSize(hFile, NULL);
	buf = (void *)GV_Malloc(dwSize + 4);
	DWORD dwBytesRead;
	ReadFile(hFile, buf, dwSize, &dwBytesRead, NULL);
	
    CloseHandle(hFile);

	return buf;
}


