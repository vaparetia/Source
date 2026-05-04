//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ximage.c
	イメージ関連管理ルーチン

	2002/03/20 K.Takabe
	$Id: ximage.c,v 1.4 2002/11/23 11:36:56 Yoshizawa1 Exp $
*/
/*

extern void DG_LoadImage( void *addr, int which, int x, int y, int w, int h, int mode );

	イメージをＶＲＡＭへ転送（未実装）


extern void DG_StoreImage( void *addr, int which, int x, int y, int w, int h, int mode );
	void	*addr ;		転送先アドレス
	int		which ;		フレームバッファ選択（0:現在描画中のバッファ 1:現在表示中バッファ）
	int		x ;			転送元座標
	int		y ;			転送元座標
	int		w ;			転送矩形サイズ
	int		h ;			転送矩形サイズ
	int		mode ;		取り込みモード（0:３２ビットカラー 1:１６ビットカラー）

	ＶＲＡＭからイメージを取り込む


extern DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format );
	int		width ;		テクスチャ幅
	int		height ;	テクスチャ高さ
	int		format ;	画像フォーマット
						（DG_TEXLIN_FORMAT_A8R8G8B8,DG_TEXLIN_FORMAT_A1R5G5B5,DG_TEXLIN_FORMAT_A4R4G4B4
						DG_TEXLIN_FORMAT_R5G6B5,DG_TEXLIN_FORMAT_L8,DG_TEXLIN_FORMAT_A8）

	線形テクスチャを生成（DG_DMAPACKでのみ使用可能）


extern void DG_FreeLinerTexture( DG_TEX_LIN *tex );
	DG_TEX_LIN		*tex ;	線形テクスチャ

	線形テクスチャを開放する


extern void DG_SetLinerTexture( DG_TEX_LIN *tex );

	線形テクスチャを設定（システムで使用）

*/

#ifdef KP_XBOX //BP

#ifdef KP_XBOX
#include <xtl.h>
//BP_RENDER #include <xgraphics.h>
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

extern void DG_DrawSync( void );	/* xdgd */
extern void				*DG_BackBufferAddr[3] ;
extern LPDIRECT3DSURFACE9	DG_BackBufferSurface[3] ;		/* バックバッファサーフェス */

/* ---------------------------------------------------------------- */

/* イメージをメインメモリからフレームバッファへ転送する（未実装） */
void DG_LoadImage( void *addr, int which, int x, int y, int w, int h, int mode )
{
}

/* イメージをメインメモリに転送する */
void DG_StoreImage( void *addr, int which, int x, int y, int w, int h, int mode )
{
#if 0 //BP_RENDER
	u_int	*frame, *top_addr ;
	int		i, j ;
	LPDIRECT3DSURFACE8	lpSurface ;
	D3DLOCKED_RECT		locked_rect ;

	/* 描画終了待ち */
	DG_DrawSync();
	printf("frame buffer capture : addr:%p page:%d x:%d y:%d w:%d h:%d mode;%d\n", addr, which, x, y, w, h, mode );

	/* バッファの選択 */
	switch ( which ){
	  case 0:
		lpSurface = DG_BackBufferSurface[ DG_CurrentBackBuffer ];
		frame = DG_BackBufferAddr[ DG_CurrentBackBuffer ];
		break ;
	  case 1:
		lpSurface = DG_BackBufferSurface[ 1 - DG_CurrentBackBuffer ];
		frame = DG_BackBufferAddr[ 1 - DG_CurrentBackBuffer ];
		break ;
	  case 2:
		lpSurface = DG_BackBufferSurface[ 2 ];
		frame = DG_BackBufferAddr[ 2 ];
		break ;
	}
	IDirect3DDevice8_BlockUntilVerticalBlank( g_pd3dDevice );	/* = VSync() */
	IDirect3DSurface8_LockRect( lpSurface, &locked_rect, NULL, D3DLOCK_TILED );
	frame = locked_rect.pBits ;
	/* 色を変換しながら転送 */
	if ( mode == 0 ){
		/* ３２ビットとして転送 */
		u_int		*dst ;
		frame += x + y * DISPLAY_WIDTH ;
		dst = addr ;
		if ( ( w & 15 ) == 0 && ( x & 3 ) == 0 ){
			/* バースト転送モード */
			for ( j = 0 ; j < h ; j++ ){
				for ( i = 0 ; i < w ; i+=16 ){
					__asm {
						mov		edx, frame
						mov		ecx, dst
						movaps	xmm0, [ edx + 00h ]
						movaps	xmm1, [ edx + 10h ]
						movaps	xmm2, [ edx + 20h ]
						movaps	xmm3, [ edx + 30h ]
						movaps	[ ecx + 00h ], xmm0
						movaps	[ ecx + 10h ], xmm1
						movaps	[ ecx + 20h ], xmm2
						movaps	[ ecx + 30h ], xmm3
					}
					*dst = *frame ;
					dst += 16 ;
					frame += 16 ;
				}
				frame += DISPLAY_WIDTH - w ;
			}
		} else {
			/* 通常転送モード */
			for ( j = 0 ; j < h ; j++ ){
				for ( i = 0 ; i < w ; i++ ){
					*dst = *frame ;
					dst++ ;
					frame++ ;
				}
				frame += DISPLAY_WIDTH - w ;
			}
		}
	} else {
		/* １６ビットに減色して転送 */
		u_short		*dst ;
		frame += x + y * DISPLAY_WIDTH ;
		dst = addr ;
		for ( j = 0 ; j < h ; j++ ){
			for ( i = 0 ; i < w ; i++ ){
				u_short	tmp ;
				/* 16bit化 */
				tmp = ( ( *frame >> 3 ) & 0x001f ) |
				  ( ( *frame >> 6 ) & 0x03e0 ) |
					( ( *frame >> 9 ) & 0x7c00 ) |
					  0x8000 ;
				*dst = tmp ;
				dst++ ;
				frame++ ;
			}
			frame += DISPLAY_WIDTH - w ;
		}
	}
	IDirect3DSurface8_UnlockRect( lpSurface );
#endif
}

/* 線形テクスチャを生成する */
DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format )
{
#if 0 //BP_RENDER
	int		image_size, palette_size, total_size, d3d_format, pitch ;
	void	*addr ;
	DG_TEX_LIN		*tex ;

	switch ( format ){
	  case DG_TEXLIN_FORMAT_A8R8G8B8:
		image_size = width * height * 4 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_A8R8G8B8 ;
		pitch = width * 4 ;
		break ;
	  case DG_TEXLIN_FORMAT_A1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_A1R5G5B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_A4R4G4B4:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_A4R4G4B4 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_P8:
	  case DG_TEXLIN_FORMAT_L8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		d3d_format = D3DFMT_LIN_L8 ;
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_A8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		d3d_format = D3DFMT_LIN_A8 ;
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_R5G6B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_R5G6B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_X1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_X1R5G5B5 ;
		pitch = width * 2 ;
		break ;
	}
	total_size = image_size + palette_size ;

	addr = DG_AllocLocalVideoMemoryAlign( total_size, 256 );
	if ( addr == NULL ){
		ASSERT( 0 );
		return ( NULL );
	}

	tex = GV_Malloc( sizeof(DG_TEX_LIN) );
	if ( tex == NULL ){
		ASSERT( 0 );
		DG_FreeLocalVideoMemory( addr );
		return ( NULL );
	}

	tex->width = width ;
	tex->height = height ;
	tex->format = format ;
	tex->flag = 0 ;
	tex->image = addr ;
	tex->palette = NULL ;
	XGSetTextureHeader( tex->width, tex->height, 1, 0, d3d_format, 0,
					   (LPDIRECT3DTEXTURE8*)&tex->d3dtexture, 0, pitch );
#ifndef KP_WINDOWS
	XGSetPaletteHeader( D3DPALETTE_256, &tex->d3dpalette, 0 );
#endif
	IDirect3DTexture8_Register( (LPDIRECT3DTEXTURE8*)&tex->d3dtexture, tex->image );
#if 0
	if ( tex->palette != NULL ) IDirect3DPalette8_Register( &tex->d3dpalette, tex->palette );

	if ( palette_size != 0 ) tex->palette = (void*)( (char*)addr + image_size ) ;
#endif

	return ( tex );
#else
   return NULL;
#endif
}

/* ユーザーの確保したメモリで線形テクスチャを作成する */
void DG_MakeLinerTexture2( DG_TEX_LIN *tex, int width, int height, int format, void *addr )
{
#if 0 //BP_RENDER
	int		image_size, palette_size, total_size, d3d_format, pitch ;

	switch ( format ){
	  case DG_TEXLIN_FORMAT_A8R8G8B8:
		image_size = width * height * 4 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_A8R8G8B8 ;
		pitch = width * 4 ;
		break ;
	  case DG_TEXLIN_FORMAT_A1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_A1R5G5B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_A4R4G4B4:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_A4R4G4B4 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_P8:
	  case DG_TEXLIN_FORMAT_L8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		d3d_format = D3DFMT_LIN_L8 ;
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_A8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		d3d_format = D3DFMT_LIN_A8 ;
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_R5G6B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_R5G6B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_X1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_LIN_X1R5G5B5 ;
		pitch = width * 2 ;
		break ;
	}
	total_size = image_size + palette_size ;

	tex->width = width ;
	tex->height = height ;
	tex->format = format ;
	tex->flag = 0 ;
	tex->image = addr ;
	tex->palette = NULL ;
	XGSetTextureHeader( tex->width, tex->height, 1, 0, d3d_format, 0,
					   (LPDIRECT3DTEXTURE8*)&tex->d3dtexture, 0, pitch );
#ifndef KP_WINDOWS
	XGSetPaletteHeader( D3DPALETTE_256, &tex->d3dpalette, 0 );
#endif
	IDirect3DTexture8_Register( (LPDIRECT3DTEXTURE8*)&tex->d3dtexture, tex->image );
#if 0
	if ( tex->palette != NULL ) IDirect3DPalette8_Register( &tex->d3dpalette, tex->palette );

	if ( palette_size != 0 ) tex->palette = (void*)( (char*)addr + image_size ) ;
#endif

#endif
}

/* 確保した線形テクスチャを開放する */
void DG_FreeLinerTexture( DG_TEX_LIN *tex )
{
	if ( tex == NULL ) return ;

	DG_DelayedFreeLocalVideoMemory( tex->image );
	GV_DelayedFree( tex );
	
}

/* ---------------------------------------------------------------- */
void DG_SetLinerTexture( DG_TEX_LIN *tex )
{
#if 0 //BP_RENDER

	LPDIRECT3DTEXTURE8		*texture ;
	int						format ;

	texture = (LPDIRECT3DTEXTURE8*)&tex->d3dtexture ;

	DG_SetTextureDirect( 0, texture );
#if 0
	if ( tex->format == DG_TEXLIN_FORMAT_P8 ){
		DG_SetPalette( 0, &tex->d3dpalette );
	}
#endif

#endif
}



#endif
