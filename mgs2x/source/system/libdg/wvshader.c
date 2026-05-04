/*
	xvshader.c
	頂点シェーダー管理ルーチン

	2002/03/19 K.Takabe
	$Id: wvshader.c,v 1.17 2002/11/22 13:08:24 takaki Exp $

*/

#include <xtl.h>
#include <xgraphics.h>

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

void DG_InitVertexShader( void )
{
	VShInfoInit() ;
}

void DG_ReleaseVertexShader( void )
{
	VShInfoDest() ;
}

void DG_ResetVertexShader(void)
{
	/* 適当な固定機能シェーダで初期化 */
	/* なぜか初期化しておかないとマルチテクスチャＫＭＳ描画でおかしくなる */
	DG_SetVertexShaderForce( D3DFVF_XYZ );

	/* シェーダーの設定が上で変更されているため初期化しておく */
	current_faf = NULL ;
}

void DG_MakeVertexShader( DG_VERTEXSHADER *shader, DWORD type, const DWORD* pDecl )
{
	HRESULT	hr ;
	VShInfo	*vsh_info ;
	void	*buffer ;
	DWORD	usage ;

	/*-- VertexShader使用判定 ----------------------------------*/

#if FALSE
	if( !DG_CheckUseVertexShader() ){ return ; }
#endif
	/*----------------------------------------------------------*/

	/*-- すでに初期化されていそうならしない --------------------*/

	if( shader->vsh_type != VSHT_NULL ){ return ; }
	/*----------------------------------------------------------*/

	/*-- 使用可能か判定 ----------------------------------------*/

	vsh_info = getVertexShaderInfo(type) ;
	if( !vsh_info->act ){ return ; }
	/*----------------------------------------------------------*/

	/*-- USAGE作成 ---------------------------------------------*/

	if( !DG_CheckVertexShaderSoftProcessing() ){ usage = 0 ; }
	else{ usage = D3DUSAGE_SOFTWAREPROCESSING ; }
	/*----------------------------------------------------------*/

	/*-- ハンドル作成 ------------------------------------------*/

	shader->vsh_type = type ;

	buffer = (vsh_info->buff)->lpVtbl->GetBufferPointer(vsh_info->buff) ;
	hr = IDirect3DDevice8_CreateVertexShader( g_pd3dDevice,  pDecl,
				buffer, &shader->handle, usage) ;

	if( FAILED(	hr ) )
	{
#ifdef DEBUG_MODE
		char	strbuff[256] ;
		sprintf(strbuff, "can't create vertex shader[%d:%s]", type, _vsh_name_tbl[type]) ;
		dbgErrMessPuts(strbuff, hr) ;
#endif
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/
		return ;
	}
	/*----------------------------------------------------------*/
}

/* 頂点シェーダープログラムの選択 */
void DG_SelectVertexShader( DG_VERTEXSHADER *shader )
{
	if( shader->handle ){ DG_SetVertexShader(shader->handle) ; }
}

/* 頂点シェーダープログラムの解放 */
void DG_KillVertexShader( DG_VERTEXSHADER *shader )
{
	/*-- ハンドル解放 ------------------------------------------*/

	if( shader->handle ){ DG_DeleteVertexShader(shader->handle) ; }

	shader->vsh_type = VSHT_NULL ;
	shader->handle   = (DWORD)NULL ;
	/*----------------------------------------------------------*/
}

