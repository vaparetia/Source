//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xvshader.c
	頂点シェーダー管理ルーチン

	2002/03/19 K.Takabe
	$Id: xvshader.c,v 1.4 2002/11/23 11:36:56 Yoshizawa1 Exp $

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

//#include	"gameheader.h"

#include "dmapack.h"
#include	"def_dma.h"

#define MAX_VSHADER_SLOT	(136)	/* 読み込める頂点シェーダーの最大サイズ */

/* ---------------------------------------------------------------- */
static int		DG_VertexShaderLoadOffset ;
static int		DG_VertexShaderSafeAddr = 0 ;
static void		*DG_VertexShaderLastLoadShader = NULL ;
static int		DG_VertexShaderOverFlag = 0 ;
static	*current_faf = NULL ;
static DG_VERTEXSHADER	*DG_VertexShaderLink = NULL ;
/* ---------------------------------------------------------------- */
//void DG_InitVertexShader( void )
//{
//}
void DG_ResetVertexShader(void)
{
#if 0 //BP_RENDER
	/* 適当な固定機能シェーダで初期化 */
	/* なぜか初期化しておかないとマルチテクスチャＫＭＳ描画でおかしくなる */
	IDirect3DDevice8_SetVertexShader( g_pd3dDevice, D3DFVF_XYZ );
#endif

	/* シェーダーの設定が上で変更されているため初期化しておく */
	current_faf = NULL ;
}

void DG_MakeVertexShader( DG_VERTEXSHADER *shader, void *code, const DWORD* pDecl )
{
#if 0 //BP_RENDER
	/* 現状ではマイクロコードのサイズを取得する方法がわからないため一度ハンドルを生成する */
	if( FAILED(	IDirect3DDevice8_CreateVertexShader( g_pd3dDevice,  pDecl, code, &shader->handle, 0 ) ) ) {
		printf("can't create vertex shader \n");
		HANGUP();
	}
	IDirect3DDevice8_GetVertexShaderSize( g_pd3dDevice, shader->handle, &shader->size );
	IDirect3DDevice8_DeleteVertexShader( g_pd3dDevice, shader->handle );

	shader->prog_addr = code ;
#endif
}

/* 頂点シェーダープログラムのクリア */
void DG_ClearVertexShader( void )
{
	DG_VertexShaderLoadOffset = 0 ;
	DG_VertexShaderSafeAddr = 0 ;
	DG_VertexShaderLastLoadShader = NULL ;
	DG_VertexShaderOverFlag = 0 ;
	DG_VertexShaderLink = NULL ;
}

/* 頂点シェーダープログラムの追加読み込み */
int DG_LoadVertexShader( DG_VERTEXSHADER *shader )
{
#if 0 //BP_RENDER
	DG_VERTEXSHADER		*link ;
	/* 後で辿れるようにリンクを記録 */
	shader->next = DG_VertexShaderLink ;
	DG_VertexShaderLink = shader ;
	if ( ( DG_VertexShaderLoadOffset + shader->size ) <= MAX_VSHADER_SLOT ){
		/* 一度にシェーダーが読み込める場合 */
		//IDirect3DDevice8_LoadVertexShader( g_pd3dDevice, shader->handle, DG_VertexShaderLoadOffset );
		IDirect3DDevice8_LoadVertexShaderProgram( g_pd3dDevice, shader->prog_addr, DG_VertexShaderLoadOffset );
		shader->load_addr = DG_VertexShaderLoadOffset ;
		DG_VertexShaderLoadOffset += shader->size ;
		DG_VertexShaderSafeAddr = DG_VertexShaderLoadOffset ;
	} else {
		/* 一度に読み込めるサイズを超えた場合にはシェーダー選択時に動的読み込みを試みる */
		int		safe_addr ;
		safe_addr = MAX_VSHADER_SLOT - shader->size ;
		if ( DG_VertexShaderSafeAddr > safe_addr ){
			DG_VertexShaderSafeAddr = safe_addr ;
			/* リンクを辿ってアドレスが重なるもの全てを動的ロードの対象に変更 */
			for ( link = DG_VertexShaderLink ; link != NULL ; link = link->next ){
				if ( link->load_addr == -1 ) continue ;
				if ( ( link->load_addr + link->size ) > DG_VertexShaderSafeAddr ){
					DG_VertexShaderSafeAddr = link->load_addr ;
					link->load_addr = -1 ;		/* 動的ロード指定 */
				} else {
					/* 先頭から順番にロードしているので後は検索する必要なし */
					break ;
				}
			}
		}
		shader->load_addr = -1 ;		/* 動的ロード指定 */
		DG_VertexShaderLoadOffset = MAX_VSHADER_SLOT ;
		DG_VertexShaderOverFlag = 1 ;
	}
#endif
	return ( DG_VertexShaderLoadOffset );
}
int DG_LoadVertexShader2( int addr, DG_VERTEXSHADER *shader )
{
	DG_VertexShaderLoadOffset = addr ;
	return ( DG_LoadVertexShader( shader ) );
}

/* 頂点シェーダープログラムの選択 */
void DG_SelectVertexShader( DG_VERTEXSHADER *shader, DG_VERTEXFORMAT *vformat )
{
#if 0 //BP_RENDER

	void	*faf ;
	/* 頂点属性の取得 */
	faf = ( vformat != NULL ) ? &vformat->faf : NULL ;
	/* 多重設定を避けるためポインタでチェック */
	if ( current_faf == faf ){
		faf = NULL ;
	} else {
		current_faf = faf ;
	}
	if ( shader->load_addr != -1 ){
		/* ロード済みシェーダーがすべて読み込めていた場合 */
		//IDirect3DDevice8_SelectVertexShader( g_pd3dDevice, shader->handle, shader->load_addr );
		IDirect3DDevice8_SelectVertexShaderDirect( g_pd3dDevice, faf, shader->load_addr );
	} else {
		/* ロードしたシェーダーが一度に読み込めるサイズを超えている場合には動的にロード処理を行う */
		/* 安全領域を越えた場所に配置されたシェーダーは安全領域終了アドレスに一度読み込みを行う */
		if ( DG_VertexShaderLastLoadShader != shader->prog_addr ){
			IDirect3DDevice8_LoadVertexShaderProgram( g_pd3dDevice, shader->prog_addr, DG_VertexShaderSafeAddr );
			DG_VertexShaderLastLoadShader = shader->prog_addr ;
		}
		IDirect3DDevice8_LoadVertexShaderProgram( g_pd3dDevice, shader->prog_addr, DG_VertexShaderSafeAddr );
		IDirect3DDevice8_SelectVertexShaderDirect( g_pd3dDevice, faf, DG_VertexShaderSafeAddr );
	}
#endif
}


void DG_MakeVertexFormat( DG_VERTEXFORMAT *format, const DWORD* pDecl )
{
#if 0 //BP_RENDER
	int		i ;
	int		stream = 0, token, reg, type, size, tmp ;
	int		offset[8] ;
	/* 構造体初期化 */
	for ( i = 0 ; i < 16 ; i++ ){
#ifndef KP_WINDOWS
		format->faf.Input[i].StreamIndex = 0 ;
		format->faf.Input[i].Offset = 0 ;
		format->faf.Input[i].Format = D3DVSDT_NONE ;
		format->faf.Input[i].TessType = 0 ;
		format->faf.Input[i].TessSource = 0 ;
#endif
	}
	for ( i = 0 ; i < 8 ; i++ ){ offset[i] = 0 ; }
	//printf("make vertex format ...\n");
	/* フォーマットを解析して設定 *//* ＭＧＳ２で使用するもののみ */
	while ( ( token = ( *pDecl & D3DVSD_TOKENTYPEMASK ) >> D3DVSD_TOKENTYPESHIFT ) != D3DVSD_TOKEN_END ){
		switch ( token ){
		  case D3DVSD_TOKEN_NOP:
			break ;
		  case D3DVSD_TOKEN_STREAM:
			stream = ( *pDecl & D3DVSD_STREAMNUMBERMASK ) >> D3DVSD_STREAMNUMBERSHIFT ;
			break ;
		  case D3DVSD_TOKEN_STREAMDATA:
			reg = ( *pDecl & D3DVSD_VERTEXREGMASK ) >> D3DVSD_VERTEXREGSHIFT ;
			type = ( *pDecl & D3DVSD_DATATYPEMASK ) >> D3DVSD_DATATYPESHIFT ;
#ifndef KP_WINDOWS
			format->faf.Input[ reg ].StreamIndex = stream ;
			format->faf.Input[ reg ].Format = type ;
			format->faf.Input[ reg ].Offset = offset[ stream ] ;
#endif
			/* オフセットを計算（これは卑怯か？） */
			size = type >> 4 ;
			if ( size > 4 ) size - 4 ;
			size = size << ( type & 0x3 ) ;
			printf("%d %d %d\n", stream, reg, type );
			offset[ stream ] += size ;
			break ;
		  case D3DVSD_TOKEN_TESSELLATOR:
			break ;
		  case D3DVSD_TOKEN_CONSTMEM:
			break ;
		  case D3DVSD_TOKEN_EXT:
			break ;
		  case D3DVSD_TOKEN_END:
			break ;
		}
		pDecl++ ;
	}
#if 0
	for ( i = 0 ; i < 16 ; i++ ){
		printf("Input: %d %d %08x %d %d\n",
		format->faf.Input[i].StreamIndex,
		format->faf.Input[i].Offset,
		format->faf.Input[i].Format,
		format->faf.Input[i].TessType,
		format->faf.Input[i].TessSource );
	}
#endif

#endif
}


/* 指定した頂点フォーマットで入力ストリームの設定を行う */
void DG_SetVertexStream( DG_VERTEXFORMAT *format )
{
#if 0 //BP_RENDER
	extern DG_VERTEXSTREAM	DG_VertexStream ;
	void	*faf ;

	if ( format == NULL ){
		IDirect3DDevice8_SetVertexShaderInputDirect( g_pd3dDevice, NULL, 0, NULL );
		return ;
	} else {
		/* 頂点属性の取得 */
		//faf = ( vformat != NULL ) ? &vformat->faf : NULL ;
		faf = &format->faf ;
	}

	/* これを行うことで指定したフォーマットでのストリーム入力を行うことができる */
	/* 但し、事前に各ストリームへの頂点バッファの設定(DG_SetVertexBuffer())を済ませておく必要がある */
	IDirect3DDevice8_SetVertexShaderInputDirect( g_pd3dDevice,
												faf,
												DG_VertexStream.max_stream,
												DG_VertexStream.input );
#endif
}

#endif
