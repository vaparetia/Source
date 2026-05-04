/*--------------------------------------------------------------*/
/*	wpshadertbl.c												*/
/*					Pixel Shader Table							*/
/*--------------------------------------------------------------*/
#define	__WPSHADER_TBL_C__

#include <stdio.h>

#include "xtl.h"
#include "libfs.h"
#include "libgv.h"

#include "wpshadertbl.h"
#include "wpshh.h"

/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
typedef	struct	PShCodeTbl_
{
	char	*str ;
	DWORD	size ;

	char	*name ;
} PShCodeTbl ;

/*--------------------------------------------------------------*/
/*	Pixel Shader name 展開										*/
/*--------------------------------------------------------------*/
#undef	PSH_TYPE_EQU
#define	PSH_TYPE_EQU(name_)	#name_ ,
char	*_psh_name_tbl[MAX_PSHT] =
{
	"NULL",
#include "wpshadertbl.h"
} ;
#undef	PSH_TYPE_EQU

/*--------------------------------------------------------------*/
/*	Pixel Shader Code Table展開								*/
/*--------------------------------------------------------------*/
#define	PSH_TYPE_EQU(name_)	{ (char *)vshh_##name_ , sizeof(vshh_##name_), #name_ },
static	PShCodeTbl	_psh_code_tbl[MAX_PSHT] =
{
	{ NULL, 0, NULL },
#include "wpshadertbl.h"
} ;
#undef	PSH_TYPE_EQU

/*--------------------------------------------------------------*/
/*	PShInfoInit		初期化										*/
/*--------------------------------------------------------------*/
void	PShInfoInit(void)
{
	int				i ;
	PShInfo			*psh ;
	HRESULT			hr ;
	PShCodeTbl		*tbl ;
	ID3DXBuffer		*err_str_buff ;

	/*-- メモリ初期化 ------------------------------------------*/

	PShInfoDest() ;
	memset(_psh_tbl, 0x00, sizeof(_psh_tbl)) ;
	/*----------------------------------------------------------*/

	/*-- Pixel Shader作成 -------------------------------------*/

	tbl = _psh_code_tbl ;
	psh = _psh_tbl ;
	for(i=MAX_PSHT; i>0; i--, tbl++, psh++)
	{
		if( !tbl->str ){ continue ; }

		/*-- Pixel Shader作成 ---------------------------------*/

		err_str_buff = NULL ;
		hr = D3DXAssembleShader(tbl->str, tbl->size - 1, 0, NULL, &psh->buff, &err_str_buff) ;
		if( !FAILED(hr) )
		{
			psh->act = TRUE ;
		}
		else
		{
			char	strbuff[256] ;

			sprintf(strbuff, "%s:%d", tbl->name, tbl->size) ;
			dbgErrMessPuts(strbuff, hr) ;	// アセンブル失敗(未対応形式？)

			dbgErrMessPuts(err_str_buff->lpVtbl->GetBufferPointer(err_str_buff), hr) ;
										// アセンブル失敗(未対応形式？)

			dbgErrMessPuts(tbl->str, hr) ;
		}

		if( err_str_buff ){ err_str_buff->lpVtbl->Release(err_str_buff) ; }
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	PShInfoDest		終了処理									*/
/*--------------------------------------------------------------*/
void	PShInfoDest(void)
{
	int				i ;
	PShInfo			*psh ;

	psh = _psh_tbl ;
	for(i=MAX_PSHT; i>0; i--, psh++)
	{
		/*-- 解放 ----------------------------------------------*/

		if( psh->buff ){ (psh->buff)->lpVtbl->Release(psh->buff) ; }	// Release
		/*------------------------------------------------------*/
	}

	memset(_psh_tbl, 0x00, sizeof(_psh_tbl)) ;
}

/*-- End Of File --*/
