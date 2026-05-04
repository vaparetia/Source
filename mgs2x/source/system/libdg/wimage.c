/*
	ximage.c
	イメージ関連管理ルーチン

	2002/03/20 K.Takabe
	$Id: wimage.c,v 1.21 2002/12/31 08:00:11 takaki Exp $

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

#ifdef _XBOX
#include <xtl.h>
#include <xgraphics.h>
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

extern LPDIRECT3DTEXTURE8 DG_ShadowTexture ;			// 影レンダリング用テクスチャ
extern LPDIRECT3DTEXTURE8 DG_ShadowDepthTexture ;		// 影Depthレンダリング用テクスチャ
extern LPDIRECT3DTEXTURE8 DG_GetFrontBufferTexture ;	// GetFrontBuffer用テクスチャ
extern LPDIRECT3DTEXTURE8 DG_BackBuffer2Texture ;		// BackBuffer2用テクスチャ

extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;
extern LPDIRECT3DSURFACE8	DG_BackBufferSurface[3] ;
extern LPDIRECT3DSURFACE8	DG_BackBufferDepthSurface ;

extern DWORD				DG_BackBufferRendWidth ;	// 描画に使用する範囲
extern DWORD				DG_BackBufferRendHeight ;

static DWORD AjustTextureWidth(DWORD width) ;

static	PALETTEENTRY	_tex_lin_a8_plt[0xff] ;

/* ---------------------------------------------------------------- */

typedef	struct D3DFMT_TO_PIXSIZE_
{
	D3DFORMAT	fmt ;		// Format
	DWORD		pixsize ;	// Pixelサイズ(BYTE)
} D3DFMT_TO_PIXSIZE ;

static	D3DFMT_TO_PIXSIZE	_d3dfmt2pixsize_tbl[] =
{
	{ D3DFMT_R8G8B8,		3	},
	{ D3DFMT_A8R8G8B8,		4	},
	{ D3DFMT_X8R8G8B8,		4	},
	{ D3DFMT_R5G6B5,		2	},
	{ D3DFMT_X1R5G5B5,		2	},
	{ D3DFMT_A1R5G5B5,		2	},
	{ D3DFMT_A4R4G4B4,		2	},
	{ D3DFMT_R3G3B2,		1	},
	{ D3DFMT_A8R3G3B2,		2	},
	{ D3DFMT_X4R4G4B4,		2	},
	{ D3DFMT_A2B10G10R10,	4	},
} ;
#define	D3DFMT_TO_PIXSIZE_TBL_SIZE	(sizeof(_d3dfmt2pixsize_tbl)/sizeof(_d3dfmt2pixsize_tbl[0]))

DWORD	GetSurfaceSize(D3DSURFACE_DESC *desc)
{
	int					i ;
	D3DFMT_TO_PIXSIZE	*tbl_ptr ;
	DWORD				size ;

	/*-- 使用FormatからPixel単位のByteSize計算 -----------------------------*/

	size    = 0 ;
	tbl_ptr = _d3dfmt2pixsize_tbl ;
	for(i=D3DFMT_TO_PIXSIZE_TBL_SIZE; i>0; i--, tbl_ptr++)
	{
		if( tbl_ptr->fmt == desc->Format )
		{
			size = tbl_ptr->pixsize ;
			break ;
		}
	}

	if( !size )
	{
		ASSERT(0) ;
		size = desc->Size ;	// 不安だけど、descの記述を信じる
	}
	else
	{
		size *= desc->Width * desc->Height ;

#ifdef DEBUG_MODE
		if( size != desc->Size )
		{
			printf("何故だか知りませんが、変なD3DSURFACE_DESCが帰ってきている様です。\n") ;
		}
#endif
	}
	/*----------------------------------------------------------------------*/

	return(size) ;
}

/* ---------------------------------------------------------------- */

/* イメージをメインメモリからフレームバッファへ転送する（未実装） */
void DG_LoadImage( void *addr, int which, int x, int y, int w, int h, int mode )
{
}

/* イメージをメインメモリに転送する */
static	void DG_StoreImage_BackBuffer( void *addr, int which, int x, int y, int w, int h, int mode ) ;
static	void DG_StoreImage_RendBuffer( void *addr, int which,
									int x, int y, int w, int h, int mode ) ;

void DG_StoreImage( void *addr, int which, int x, int y, int w, int h, int mode )
{
	/* 描画終了待ち */
	DG_ExeBufferSync(FALSE) ;

	/* 処理本体 */
	DG_StoreImagePacket(addr, which, x, y, w, h, mode) ;
}

void DG_StoreImagePacket( void *addr, int which, int x, int y, int w, int h, int mode )
{
#if __DG_STOREIMAGE_BY_RENDBUFFER_ONLY__
	DG_StoreImage_RendBuffer(addr, which, x, y, w, h, mode) ;
#else
	/* whichによって分岐 */
	switch( which )
	{
	  case 0 :
	  case 1 :
		DG_StoreImage_BackBuffer(addr, which, x, y, w, h, mode) ;
		break ;

	  default :
		DG_StoreImage_RendBuffer(addr, which, x, y, w, h, mode) ;
		break ;
	}
#endif
}

static	void DG_StoreImage_BackBuffer( void *addr, int which, int x, int y, int w, int h, int mode )
{
	LPDIRECT3DSURFACE8	lpSurface ;
	HRESULT				hr ;
	D3DSURFACE_DESC		src_desc ;
	LPDIRECT3DSURFACE8	tmp_surface ;
	LPDIRECT3DSURFACE8	dst_surface ;
	D3DFORMAT			dst_fmt ;
	D3DSURFACE_DESC		dst_desc ;

	printf("frame buffer capture(BackBuffer) : addr:%p x:%d y:%d w:%d h:%d mode:%d\n",
												addr, x, y, w, h, mode );

	/*-- BackBuffer数検査 ---------------------------------------------------*/

	if( (int)DG_WinApp.d3dpp.BackBufferCount <= which )
	{
		/* 違う方法で試みる */
		DG_StoreImage_RendBuffer(addr, which, x, y, w, h, mode) ;
		return ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- Surface取得 -------------------------------------------------------*/

	hr = IDirect3DDevice8_GetBackBuffer(g_pd3dDevice, which, D3DBACKBUFFER_TYPE_MONO,
									&lpSurface) ;
	ASSERT( !FAILED(hr) ) ;

	if( FAILED(hr) )
	{
		/* 違う方法で試みる */
		DG_StoreImage_RendBuffer(addr, which, x, y, w, h, mode) ;
		return ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 作業用Surface作成(BackBafferがLock可能とは限らないので) ------------*/

	hr = IDirect3DSurface8_GetDesc(lpSurface, &src_desc) ;
	ASSERT( !FAILED(hr) ) ;

#ifdef DEBUG_MODE
	printf("\tsource frame :\n") ;
	printf("\t\tFormat:%d\n", src_desc.Format) ;
	printf("\t\tType:  %d\n", src_desc.Type) ;
	printf("\t\tUsage: %d\n", src_desc.Usage) ;
	printf("\t\tPool:  %d\n", src_desc.Pool) ;
	printf("\t\tSize:  %d\n", src_desc.Size) ;
	printf("\t\tMultiSampleType:%d\n", src_desc.MultiSampleType) ;
	printf("\t\tWidth: %d\n", src_desc.Width) ;
	printf("\t\tHeight:%d\n", src_desc.Height) ;
#endif

	tmp_surface = NULL ;
	dst_surface = NULL ;

	hr = IDirect3DDevice8_CreateImageSurface(g_pd3dDevice,
										src_desc.Width, src_desc.Height,
										src_desc.Format,
										&tmp_surface) ;		// 作業用バッファ
	ASSERT( !FAILED(hr) ) ;
	if( !FAILED(hr) )
	{
		if( mode == 0 ){ dst_fmt = D3DFMT_A8R8G8B8 ; }
		else{ dst_fmt = D3DFMT_A1R5G5B5 ; }

		hr = IDirect3DDevice8_CreateImageSurface(g_pd3dDevice,
											w, h,
											dst_fmt,
											&dst_surface) ;		// 画像変換用バッファ
		ASSERT( !FAILED(hr) ) ;
	}
	/*-----------------------------------------------------------------------*/

	if( !FAILED(hr) )
	{
		RECT			src_rect ;
		RECT			dst_rect ;
		D3DLOCKED_RECT	lrect ;
		D3DLOCKED_RECT	dst_lrect ;
		//DWORD			size ;

		/*-- CopyRects ------------------------------------------------------*/

		src_rect.left   = 0 ;
		src_rect.top    = 0 ;
		src_rect.right  = src_desc.Width ;
		src_rect.bottom = src_desc.Height ;

		/* (注)新ドライバのGeForceでは安定しないので自力でコピー */
		hr = IDirect3DSurface8_LockRect(lpSurface, &lrect, NULL,
								  D3DLOCK_READONLY
								| D3DLOCK_NOSYSLOCK ) ;
		ASSERT( !FAILED(hr) ) ;
		if( !FAILED(hr) )
		{
			hr = IDirect3DSurface8_LockRect(tmp_surface, &dst_lrect, NULL,
									D3DLOCK_NOSYSLOCK) ;
			ASSERT( !FAILED(hr) ) ;
			if( !FAILED(hr) )
			{
#if FALSE
				size = GetSurfaceSize(&src_desc) ;
				memcpy(dst_lrect.pBits, lrect.pBits, size) ;
#else
				BYTE	*dst ;
				BYTE	*src ;
				INT		pitch ;
				int		y ;

				pitch = dst_lrect.Pitch ;
				if( pitch > lrect.Pitch ){ pitch = lrect.Pitch ; }

				dst = dst_lrect.pBits ;
				src = lrect.pBits ;

				for(y=src_desc.Height; y>0; y--)
				{
					memcpy(dst, src, pitch) ;

					dst += dst_lrect.Pitch ;
					src += lrect.Pitch ;
				}
#endif
				hr = IDirect3DSurface8_UnlockRect(tmp_surface) ;
				ASSERT( !FAILED(hr) ) ;
			}

			hr = IDirect3DSurface8_UnlockRect(lpSurface) ;
			ASSERT( !FAILED(hr) ) ;
		}
		/*-------------------------------------------------------------------*/

		if( !FAILED(hr) )
		{
			/*-- 転送 -------------------------------------------------------*/

			src_rect.left   = (x * src_desc.Width)/DRAW_WIDTH ;
			src_rect.right  = ((x + w) * src_desc.Width)/DRAW_WIDTH ;
			src_rect.top    = (y * src_desc.Height)/DRAW_HEIGHT ;
			src_rect.bottom = ((y + h) * src_desc.Height)/DRAW_HEIGHT ;

			dst_rect.left   = 0 ;
			dst_rect.right  = w ;
			dst_rect.top    = 0 ;
			dst_rect.bottom = h ;

			printf("\tload source : (%d,%d,%d,%d)-->(%d,%d,%d,%d)\n",
						src_rect.left, src_rect.top, src_rect.right, src_rect.bottom,
						dst_rect.left, dst_rect.top, dst_rect.right, dst_rect.bottom) ;


			hr = D3DXLoadSurfaceFromSurface(dst_surface,
							NULL,			// pDestPalette
							&dst_rect,		// pDestRect
							tmp_surface,	// pSrcSurface,
							NULL,			// pSrcPalette
							&src_rect,		// pSrcRect
							D3DX_DEFAULT,	// Filter,
							0) ;
			ASSERT( !FAILED(hr) ) ;

			/*---------------------------------------------------------------*/

			/*-- 結果格納 ---------------------------------------------------*/

			hr = IDirect3DSurface8_GetDesc(dst_surface, &dst_desc) ;
			ASSERT( !FAILED(hr) ) ;
#ifdef DEBUG_MODE
			printf("\tdest frame :\n") ;
			printf("\t\tFormat:%d\n", dst_desc.Format) ;
			printf("\t\tType:  %d\n", dst_desc.Type) ;
			printf("\t\tUsage: %d\n", dst_desc.Usage) ;
			printf("\t\tPool:  %d\n", dst_desc.Pool) ;
			printf("\t\tSize:  %d\n", dst_desc.Size) ;
			printf("\t\tMultiSampleType:%d\n", dst_desc.MultiSampleType) ;
			printf("\t\tWidth: %d\n", dst_desc.Width) ;
			printf("\t\tHeight:%d\n", dst_desc.Height) ;
#endif

			hr = IDirect3DSurface8_LockRect(dst_surface, &lrect, NULL,
									D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK) ;
			if( !FAILED(hr) )
			{
#if FALSE
				DWORD	size ;

				size = w * h ;
				if( mode == 0 ){ size <<= 2 ; }
				else{ size <<= 1 ; }

				if( size > dst_desc.Size ){ size = dst_desc.Size ; }

				memcpy(addr, lrect.pBits, size) ;
				IDirect3DSurface8_UnlockRect(dst_surface) ;
#else
				BYTE	*dst ;
				BYTE	*src ;
				INT		addr_pitch ;
				INT		pitch ;
				int		y ;

				addr_pitch = w ;
				if( mode == 0 ){ addr_pitch <<= 2 ; }
				else{ addr_pitch <<= 1 ; }

				pitch = addr_pitch ;
				if( pitch > lrect.Pitch ){ pitch = lrect.Pitch ; }

				dst = addr ;
				src = lrect.pBits ;

				for(y=h; y>0; y--)
				{
					memcpy(dst, src, pitch) ;

					dst += addr_pitch ;
					src += lrect.Pitch ;
				}
#endif
			}
			/*---------------------------------------------------------------*/
		}
	}
	/*-----------------------------------------------------------------------*/

	/*-- 解放 ---------------------------------------------------------------*/

	if( dst_surface ){ IDirect3DSurface8_Release(dst_surface) ; }
	if( tmp_surface ){ IDirect3DSurface8_Release(tmp_surface) ; }
	if( lpSurface ){ IDirect3DSurface8_Release(lpSurface) ; }
	/*-----------------------------------------------------------------------*/
}


static void DG_StoreImage_RendBuffer( void *addr, int which,
								int x, int y, int w, int h, int mode )
{
	LPDIRECT3DSURFACE8	lpSurface ;
	HRESULT				hr ;
	D3DSURFACE_DESC		src_desc ;
	LPDIRECT3DSURFACE8	tmp_surface ;
	LPDIRECT3DSURFACE8	dst_surface ;
	D3DFORMAT			dst_fmt ;
	D3DSURFACE_DESC		dst_desc ;

	printf("frame buffer capture : addr:%p page:%d x:%d y:%d w:%d h:%d mode:%d\n",
												addr, which, x, y, w, h, mode );

	/*-- Surface取得 -------------------------------------------------------*/

	printf("\tDG_CurrentBackBuffer:%d\n", DG_CurrentBackBuffer) ;

	switch ( which ){
	  case 0:
		lpSurface = DG_BackBufferSurface[DG_CurrentBackBuffer] ;
		break ;

	  case 1:
		lpSurface = DG_BackBufferSurface[DG_CurrentBackBuffer ^ 1] ;
		break ;

	  case 2:
		lpSurface = DG_BackBufferSurface[2] ;
		break ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 作業用Surface作成(BackBafferがLock可能とは限らないので) ------------*/

	hr = IDirect3DSurface8_GetDesc(lpSurface, &src_desc) ;
	ASSERT( !FAILED(hr) ) ;
	printf("\tsource frame : w:%d h:%d format:%d\n", src_desc.Width, src_desc.Height,
										src_desc.Format) ;

	tmp_surface = NULL ;
	dst_surface = NULL ;


	hr = IDirect3DDevice8_CreateImageSurface(g_pd3dDevice,
										src_desc.Width, src_desc.Height,
										src_desc.Format,
										&tmp_surface) ;		// 作業用バッファ
	ASSERT( !FAILED(hr) ) ;
	if( !FAILED(hr) )
	{
		if( mode == 0 ){ dst_fmt = D3DFMT_A8R8G8B8 ; }
		else{ dst_fmt = D3DFMT_A1R5G5B5 ; }

		hr = IDirect3DDevice8_CreateImageSurface(g_pd3dDevice,
											w, h,
											dst_fmt,
											&dst_surface) ;		// 画像変換用バッファ
		ASSERT( !FAILED(hr) ) ;
	}
	/*-----------------------------------------------------------------------*/

	if( !FAILED(hr) )
	{
		RECT			src_rect ;
		RECT			dst_rect ;
		D3DLOCKED_RECT	lrect ;
		float			coef ;

		/*-- CopyRects ------------------------------------------------------*/

		src_rect.left   = 0 ;
		src_rect.top    = 0 ;
		src_rect.right  = src_desc.Width-1 ;
		src_rect.bottom = src_desc.Height-1 ;
		hr = IDirect3DDevice8_CopyRects(g_pd3dDevice,
							lpSurface, &src_rect, 1, tmp_surface, NULL) ;
		ASSERT( !FAILED(hr) ) ;
		/*-------------------------------------------------------------------*/

		if( !FAILED(hr) )
		{
			/*-- 転送 -------------------------------------------------------*/

			if( DG_BackBufferRendWidth == DISPLAY_WIDTH )
			{
				src_rect.left   = x ;
				src_rect.right  = x + w ;
			}
			else
			{
				coef = (float)DG_BackBufferRendWidth * (1.0f/(float)DISPLAY_WIDTH) ;
				src_rect.left   = (int)((float)x * coef) ;
				src_rect.right  = (int)((float)(x + w) * coef) ;
			}

			if( DG_BackBufferRendHeight == DISPLAY_HEIGHT )
			{
				src_rect.top    = y ;
				src_rect.bottom = y + h ;
			}
			else
			{
				coef = (float)DG_BackBufferRendHeight * (1.0f/(float)DISPLAY_HEIGHT) ;
				src_rect.top    = (int)((float)y * coef) ;
				src_rect.bottom = (int)((float)(y + h) * coef) ;
			}


			dst_rect.left   = x ;
			dst_rect.top    = y ;
			dst_rect.right  = x + w ;
			dst_rect.bottom = y + h ;
			printf("\tload source : (%d,%d,%d,%d)-->(%d,%d,%d,%d)\n",
						src_rect.left, src_rect.top, src_rect.right, src_rect.bottom,
						x, y, w, h) ;


			hr = D3DXLoadSurfaceFromSurface(dst_surface,
							NULL,			// pDestPalette
							&dst_rect,		// pDestRect
							tmp_surface,	// pSrcSurface,
							NULL,			// pSrcPalette
							&src_rect,		// pSrcRect
							D3DX_DEFAULT,	// Filter,
							0) ;
			ASSERT( !FAILED(hr) ) ;

			/*---------------------------------------------------------------*/

			/*-- 結果格納 ---------------------------------------------------*/

			hr = IDirect3DSurface8_GetDesc(dst_surface, &dst_desc) ;
			ASSERT( !FAILED(hr) ) ;

			hr = IDirect3DSurface8_LockRect(dst_surface, &lrect, NULL,
											D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK) ;
			if( !FAILED(hr) )
			{
#if FALSE
				DWORD	size ;

				size = w * h ;
				if( mode == 0 ){ size <<= 2 ; }
				else{ size <<= 1 ; }

				if( size > dst_desc.Size ){ size = dst_desc.Size ; }

				memcpy(addr, lrect.pBits, size) ;
				IDirect3DSurface8_UnlockRect(dst_surface) ;
#else
				BYTE	*dst ;
				BYTE	*src ;
				INT		addr_pitch ;
				INT		pitch ;
				int		y ;

				addr_pitch = w ;
				if( mode == 0 ){ addr_pitch <<= 2 ; }
				else{ addr_pitch <<= 1 ; }

				pitch = addr_pitch ;
				if( pitch > lrect.Pitch ){ pitch = lrect.Pitch ; }

				dst = addr ;
				src = lrect.pBits ;

				for(y=h; y>0; y--)
				{
					memcpy(dst, src, pitch) ;

					dst += addr_pitch ;
					src += lrect.Pitch ;
				}
#endif
			}
			/*---------------------------------------------------------------*/
		}
	}
	/*-----------------------------------------------------------------------*/

	/*-- 解放 ---------------------------------------------------------------*/

	if( dst_surface ){ IDirect3DSurface8_Release(dst_surface) ; }
	if( tmp_surface ){ IDirect3DSurface8_Release(tmp_surface) ; }
	/*-----------------------------------------------------------------------*/
}

/* 線形テクスチャ生成初期化 */
void	DG_MakeLinerTextureInit(void)
{
	int				i ;
	PALETTEENTRY	*plt ;

	plt = _tex_lin_a8_plt ;
	for(i=0; i<256; i++, plt++)
	{
		plt->peRed   = 0xff ;
		plt->peGreen = 0xff ;
		plt->peBlue  = 0xff ;
		plt->peFlags = i ;		// 多分α
	}
}

/* 線形テクスチャを生成する */
DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format )
{
	int			image_size, palette_size, total_size, d3d_format, pitch ;
	void		*addr ;
	DG_TEX_LIN	*tex ;
	void		*palette_addr ;

	palette_addr = NULL ;
	switch ( format ){
	  case DG_TEXLIN_FORMAT_A8R8G8B8:
		image_size = width * height * 4 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_A8R8G8B8 ;
		pitch = width * 4 ;
		break ;
	  case DG_TEXLIN_FORMAT_A1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_A1R5G5B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_A4R4G4B4:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_A4R4G4B4 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_P8:
	  case DG_TEXLIN_FORMAT_L8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		d3d_format = D3DFMT_L8 ;
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_A8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		palette_addr = _tex_lin_a8_plt ;	// X-BOXと挙動を合わせる為
		d3d_format = D3DFMT_P8 ;			// X-BOXと挙動を合わせる為
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_R5G6B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_R5G6B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_X1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_X1R5G5B5 ;
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

	tex->width  = width ;
	tex->height = height ;
	tex->format = format ;
	tex->flag = 0 ;
	tex->image = addr ;
	tex->palette = palette_addr ;

	tex->image_size  = image_size ;
	tex->image_pitch = pitch ;

	tex->need_loadsurface = 0 ;

	tex->org_format = d3d_format ;
	switch ( d3d_format ){
	  case D3DFMT_P8 :
		d3d_format = D3DFMT_A4R4G4B4 ;	// Paletteには対応していない
		break ;
	}
	d3d_format = DG_SelectUsableTextureFormat(d3d_format) ;		// 使用可能Formatへ変更
	if( tex->org_format != d3d_format )
	{
		tex->need_loadsurface |= M_DGTEXLIN_NEED_FILTER ;	// 変換が必要
#ifdef DEFBUG_MODE
		printf("DG_MakeLinerTexture():Format:%d-->%d\n", tex->org_format, d3d_format) ;
#endif
	}

	tex->tex_width  = (u_short)AjustTextureWidth(width) ;
	tex->tex_height = (u_short)AjustTextureWidth(height) ;

	DG_CreateTexture(tex->tex_width, tex->tex_height, 1, 0,
			d3d_format, D3DPOOL_MANAGED, &tex->d3dtexture) ;

	tex->tex_param.x = (float)width/((float)tex->tex_width * 16384.0f) ;
	tex->tex_param.y = (float)height/((float)tex->tex_height * 16384.0f) ;
	tex->tex_param.z = 1.0f ;
	tex->tex_param.w = 1.0f ;

	if( (tex->tex_width != width) || (tex->tex_height != height) )
	{
		tex->need_loadsurface |= M_DGTEXLIN_NEED_SUBRECT ;	// 部分Copy
		//DG_ClearTextureImage(tex->d3dtexture) ;	// ゴミ対策
	}

	DG_LinerTextureSetImageDirty(tex) ;

	return ( tex );
}

/* ユーザーの確保したメモリで線形テクスチャを作成する */
void DG_MakeLinerTexture2( DG_TEX_LIN *tex, int width, int height, int format, void *addr )
{
	int		image_size, palette_size, total_size, d3d_format, pitch ;
	void		*palette_addr ;

	palette_addr = NULL ;
	switch ( format ){
	  case DG_TEXLIN_FORMAT_A8R8G8B8:
		image_size = width * height * 4 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_A8R8G8B8 ;
		pitch = width * 4 ;
		break ;
	  case DG_TEXLIN_FORMAT_A1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_A1R5G5B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_A4R4G4B4:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_A4R4G4B4 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_P8:
	  case DG_TEXLIN_FORMAT_L8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		d3d_format = D3DFMT_L8 ;
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_A8:
		image_size = width * height ;
		palette_size = 256 * 4 ;
		palette_addr = _tex_lin_a8_plt ;	// X-BOXと挙動を合わせる為
		d3d_format = D3DFMT_P8 ;			// X-BOXと挙動を合わせる為
		pitch = width ;
		break ;
	  case DG_TEXLIN_FORMAT_R5G6B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_R5G6B5 ;
		pitch = width * 2 ;
		break ;
	  case DG_TEXLIN_FORMAT_X1R5G5B5:
		image_size = width * height * 2 ;
		palette_size = 0 ;
		d3d_format = D3DFMT_X1R5G5B5 ;
		pitch = width * 2 ;
		break ;

	  default :
		ASSERT(0) ;
		return ;
		break ;
	}

	total_size = image_size + palette_size ;

	tex->width   = width ;
	tex->height  = height ;
	tex->format  = format ;
	tex->flag    = 0 ;
	tex->image   = addr ;
	tex->palette = palette_addr ;

	tex->image_size  = image_size ;
	tex->image_pitch = pitch ;

	tex->need_loadsurface = 0 ;

	tex->org_format = d3d_format ;
	switch ( d3d_format ){
	  case D3DFMT_P8 :
		d3d_format = D3DFMT_A4R4G4B4 ;	// Paletteには対応していない
		break ;
	}
	d3d_format = DG_SelectUsableTextureFormat(d3d_format) ;		// 使用可能Formatへ変更

	if( tex->org_format != d3d_format )
	{
		tex->need_loadsurface |= M_DGTEXLIN_NEED_FILTER ;	// 変換が必要
#ifdef DEFBUG_MODE
		printf("DG_MakeLinerTexture():Format:%d-->%d\n", tex->org_format, d3d_format) ;
#endif
	}

	tex->tex_width  = (u_short)AjustTextureWidth(width) ;
	tex->tex_height = (u_short)AjustTextureWidth(height) ;
	DG_CreateTexture(tex->tex_width, tex->tex_height, 1, 0,
			d3d_format, D3DPOOL_MANAGED, &tex->d3dtexture) ;

	tex->tex_param.x = (float)width/((float)tex->tex_width * 16384.0f) ;
	tex->tex_param.y = (float)height/((float)tex->tex_height * 16384.0f) ;
	tex->tex_param.z = 1.0f ;
	tex->tex_param.w = 1.0f ;

	if( (tex->tex_width != width) || (tex->tex_height != height) )
	{
		tex->need_loadsurface |= M_DGTEXLIN_NEED_SUBRECT ;	// 部分Copy
		//DG_ClearTextureImage(tex->d3dtexture) ;	// ゴミ対策
	}

	DG_LinerTextureSetImageDirty(tex) ;	// この時点でのimageが適用される様に
}

/* 確保した線形テクスチャを解放する */
void DG_FreeLinerTexture( DG_TEX_LIN *tex )
{
	if ( tex == NULL ) return ;

	DG_DelayedFreeLocalVideoMemory( tex->image );
	tex->image      = NULL ;
	tex->image_size = 0 ;

	if( tex->d3dtexture )
	{
		DG_ExeBufferSync(FALSE) ;	// 安全策

		DG_ReleaseD3DTexture((LPDIRECT3DTEXTURE8)tex->d3dtexture) ;
		tex->d3dtexture = NULL ;
	}

	GV_Free( tex );
}

void DG_FreeLinerTexture2( DG_TEX_LIN *tex )
{
	tex->image      = NULL ;
	tex->image_size = 0 ;

	if( tex->d3dtexture )
	{
		DG_ExeBufferSync(FALSE) ;	// 安全策

		DG_ReleaseD3DTexture((LPDIRECT3DTEXTURE8)tex->d3dtexture) ;
		tex->d3dtexture = NULL ;
	}
}

/* ---------------------------------------------------------------- */
void DG_SetLinerTexture( DG_TEX_LIN *tex )
{
	DG_SetTextureDirect( 0, tex->d3dtexture );
#if 0
	if ( tex->format == DG_TEXLIN_FORMAT_P8 ){
		DG_SetPalette( 0, &tex->d3dpalette );
	}
#endif
}

/* ---------------------------------------------------------------- */
void DG_SetUserTexture( DG_USERTEX *tex )
{
#if TRUE
	DG_SetTextureDirect( 0, tex->texture );
#else
	{
		static DWORD	cntr ;

		cntr++ ;
		if( cntr & 1 )
		{
			DG_SetTextureDirect( 0, tex->texture );
		}
		else
		{
			DG_SetTextureDirect( 0, NULL );
		}
	}
#endif
}


/*----------------------------------------------------------------------*/
/*	DG_LinerTextureFlushImage	imageをTexture SurfaceにCopy			*/
/*----------------------------------------------------------------------*/
void DG_LinerTextureFlushImage( DG_TEX_LIN *tex )
{
	HRESULT				hr ;
	IDirect3DSurface8	*surface ;
	DWORD				need_loadsurface ;

	if( !tex->d3dtexture ){ return ; }	// 若しもの場合の安全策

	/*-- Surface取得 ---------------------------------------------------*/

	hr = IDirect3DTexture8_GetSurfaceLevel(tex->d3dtexture,
									0, &surface) ;
	if( FAILED(hr) )
	{
		dbgErrMessPuts("IDirect3DTexture8_GetSurfaceLevel()", hr) ;
		ASSERT(0) ;
		return ;
	}
	/*------------------------------------------------------------------*/

	/*-- Image Load ----------------------------------------------------*/

	need_loadsurface = tex->need_loadsurface ;
	if( !need_loadsurface )
	{
		D3DLOCKED_RECT	locked_rect ;

		/*-- 単純にCopyでOK --------------------------------------------*/

#ifdef DEBUG_MODE
		//printf("DG_LinerTextureFlushImage:Copy(%d,%d)\n", tex->width, tex->height) ;
#endif
		hr = IDirect3DSurface8_LockRect(surface, &locked_rect, NULL, D3DLOCK_NOSYSLOCK) ;
		if( !FAILED(hr) )
		{
			memcpy(locked_rect.pBits, tex->image, tex->image_size) ;
			IDirect3DSurface8_UnlockRect(surface) ;
		}
		/*--------------------------------------------------------------*/
	}
	else
	{
		RECT	src_rect, dst_rect ;
		DWORD	filter ;

		/*-- バッファ内容を変更しながらLoad ----------------------------*/

#ifdef DEBUG_MODE
		//printf("DG_LinerTextureFlushImage:Load (%d,%d)/(%d,%d)\n",
		//			tex->width, tex->height,
		//			tex->tex_width, tex->tex_height) ;
#endif
		if( need_loadsurface & M_DGTEXLIN_NEED_FILTER )
		{
			filter = D3DX_DEFAULT ;	// Filtering
		}
		else
		{
			filter = D3DX_FILTER_POINT ;
		}

	    src_rect.left   = 0 ;
		src_rect.top    = 0 ;
		src_rect.right  = tex->width ;
		src_rect.bottom = tex->height ;

#if FALSE	// 今のところ要らない
		if( !(need_loadsurface & M_DGTEXLIN_NEED_FULLLOAD) )
		{
			dst_rect = src_rect ;
		}
		else
		{
		    dst_rect.left   = 0 ;
			dst_rect.top    = 0 ;
			dst_rect.right  = tex->tex_width ;
			dst_rect.bottom = tex->tex_height ;
		}
#else
		dst_rect = src_rect ;
#endif

		hr = D3DXLoadSurfaceFromMemory(surface,
							NULL,				// CONST PALETTEENTRY*
							&dst_rect,			// CONST RECT* pDestRect,
							tex->image,			// LPCVOID pSrcMemory,
							tex->org_format,	// D3DFORMAT SrcFormat,
							tex->image_pitch,	// UINT SrcPitch,
							tex->palette,		// CONST PALETTEENTRY* pSrcPalette,
							&src_rect,			// CONST RECT* pSrcRect,
							filter, 			// DWORD Filter,
							0);					// D3DCOLOR ColorKey
		if( FAILED(hr) ){ dbgErrMessPuts("FAILED:D3DXLoadSurfaceFromMemory", hr) ; }
		/*--------------------------------------------------------------*/
	}

	/*------------------------------------------------------------------*/

	/*-- Release -------------------------------------------------------*/

	IDirect3DSurface8_Release(surface) ;
	/*------------------------------------------------------------------*/
}

/*----------------------------------------------------------------------*/
/*	AjustTextureWidth	テクスチャサイズ補正(2^nに成る様)				*/
/*----------------------------------------------------------------------*/
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
