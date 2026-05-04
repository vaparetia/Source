/*--------------------------------------------------------------*/
/*	error_code.c												*/
/*					Error Code									*/
/*--------------------------------------------------------------*/
#define	__ERROR_CODE_C__

#include <stdio.h>
#include <stdlib.h>

#include <xtl.h>
#include <dxerr8.h>

#include "libgv.h"
#include "error_code.h"

/*--------------------------------------------------------------*/
/*	Typedef														*/
/*--------------------------------------------------------------*/
enum
{
	LANG_JPN,
	LANG_ENG,
	LANG_GER,
	LANG_ITA,
	LANG_FRA,
	LANG_SPA,

	MAX_LANG_NUM,
} ;

typedef	struct	ERROR_CODE_MESS_
{
	char	*str[MAX_LANG_NUM] ;
} ERROR_CODE_MESS ;

typedef	struct	ERROR_CODE_INFO_
{
	ERROR_CODE_MESS	*mess ;
} ERROR_CODE_INFO ;

typedef	struct	HRESULT_TO_ERROR_CODE_TBL_
{
	HRESULT	hr ;
	DWORD	err ;
} HRESULT_TO_ERROR_CODE_TBL ;

/*--------------------------------------------------------------*/
/*	文字列														*/
/*--------------------------------------------------------------*/
#include "error_code_str.h"

/*--------------------------------------------------------------*/
/*	テーブル作成												*/
/*--------------------------------------------------------------*/
#undef ERROR_CODE_EQU
#define	ERROR_CODE_EQU(id_)	{ (ERROR_CODE_MESS *)&(id_ ## _mess) },
static	ERROR_CODE_INFO	_info_tbl[MAX_X2WERR_ID] =
{
#include "error_code_equ.h"
} ;
#undef ERROR_CODE_EQU

/*--------------------------------------------------------------*/
/*	HRESULT to ERROR ID テーブル作成							*/
/*--------------------------------------------------------------*/
#undef	HRESULT_TO_ERROR_CODE_EQU
#define	HRESULT_TO_ERROR_CODE_EQU(hr_, err_)	{ (hr_), (err_) },
static	HRESULT_TO_ERROR_CODE_TBL	_hr2err_tbl[MAX_X2WERR_ID] =
{
#include "error_code_equ.h"
} ;
#undef HRESULT_TO_ERROR_CODE_EQU

#define	MAX_HR2ERR_TBL_NUM	(sizeof(_hr2err_tbl)/sizeof(_hr2err_tbl[0]))

/*--------------------------------------------------------------*/
/*	XC_LANGUAGE_ --> LANG_変換テーブル							*/
/*--------------------------------------------------------------*/
static	DWORD	_vclang2lang[] =
{
	0,			// NULL
	LANG_JPN,	// XC_LANGUAGE_JAPANESE = 1
	LANG_GER,	// XC_LANGUAGE_GERMAN
	LANG_FRA,	// XC_LANGUAGE_FRENCH
	LANG_SPA,	// XC_LANGUAGE_SPANISH
	LANG_ITA,	// XC_LANGUAGE_ITALIAN
	LANG_ENG,	// XC_LANGUAGE_ENGLISH
} ;

/*--------------------------------------------------------------*/
/*	X2W_GetErrorIDMess	エラーID文字列取得						*/
/*--------------------------------------------------------------*/
char	*X2W_GetErrorIDMess(char *str, int str_size, int id)
{
	ERROR_CODE_INFO	*info ;
	ERROR_CODE_MESS	*mess ;
	char			*src_str ;
	int				src_len ;
	int				len ;

#if FALSE
	/*-- エラーコード付加 --------------------------------------*/

	sprintf(str, "message code[%04d]\n\n", id) ;
	str_size -= strlen(str) ;
	/*----------------------------------------------------------*/
#else
	str[0] = 0 ;
#endif

	/*-- 文字列設定 --------------------------------------------*/

	info = &_info_tbl[id] ;
	mess = info->mess ;
	if( mess )
	{
		DWORD	lang_id ;

		lang_id = _vclang2lang[XGetLanguage()] ;				// 言語設定取得
		ASSERT( lang_id < MAX_LANG_NUM ) ;

		if( lang_id >= MAX_LANG_NUM ){ lang_id = LANG_ENG ; }	// 安全策

		src_str = mess->str[lang_id] ;		// 言語切り替え
		if( src_str )
		{
			src_len = strlen(src_str) ;
			if( src_len > (str_size - 1) ){ src_len = (str_size - 1) ; }

			len = strlen(str) ;
			memcpy(str + len, src_str, src_len) ;	// 文字列末にくっつける

			str[len + src_len] = 0 ;	// 終端
		}

	}
	/*----------------------------------------------------------*/
	return(str) ;
}

/*--------------------------------------------------------------*/
/*	X2W_GetErrorIDMess	エラーID文字列取得						*/
/*--------------------------------------------------------------*/
int		X2W_GetErrorIDFromHResult(HRESULT hr)
{
	int							i ;
	HRESULT_TO_ERROR_CODE_TBL	*tbl ;

	tbl = _hr2err_tbl ;
	for(i=MAX_HR2ERR_TBL_NUM; i>0; i--, tbl++)
	{
		if( tbl->hr == hr ){ break ; }
	}
	if( !i ){ return(-1) ; }

	return(tbl->err) ;
}

/*-- End Of File --*/
