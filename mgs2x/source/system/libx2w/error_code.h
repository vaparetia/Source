/*--------------------------------------------------------------*/
/*	error_code.h												*/
/*					Error Code									*/
/*--------------------------------------------------------------*/
#ifndef	__ERROR_CODE_H__
#define	__ERROR_CODE_H__

#ifdef	__ERROR_CODE_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------*/
/*	エラーコード												*/
/*--------------------------------------------------------------*/
#undef ERROR_CODE_EQU
#define	ERROR_CODE_EQU(id_)	id_,
enum
{
#include "error_code_equ.h"
	MAX_X2WERR_ID,
} ;
#undef ERROR_CODE_EQU

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/
extern	char	*X2W_GetErrorIDMess(char *str, int str_size, int id) ;
extern	int		X2W_GetErrorIDFromHResult(HRESULT hr) ;

#ifdef __cplusplus
}
#endif
#undef	EXTERN
#endif /* !defined(__ERROR_CODE_H__) */

/*-- End Of File --*/

