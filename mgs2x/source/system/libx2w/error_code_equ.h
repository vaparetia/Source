/*--------------------------------------------------------------*/
/*	error_code_equ.h											*/
/*					Error Code Equ								*/
/*--------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/*	ID テーブル													*/
/*--------------------------------------------------------------*/
#ifdef	ERROR_CODE_EQU

	ERROR_CODE_EQU(	X2WERR_ID_CANT_CREATE_D3D8		)	// 00
	ERROR_CODE_EQU(	X2WERR_ID_CANT_CREATE_D3DEVICE8	)	// 01
	ERROR_CODE_EQU(	X2WERR_ID_DISPLAY_FORMAT		)	// 02
	ERROR_CODE_EQU(	X2WERR_ID_OTHER_FATALERROR		)	// 03
	ERROR_CODE_EQU(	X2WERR_ID_SET_DISPLAY_16BIT		)	// 04
	ERROR_CODE_EQU(	X2WERR_ID_SET_DISPLAY_32BIT		)	// 05
	ERROR_CODE_EQU(	X2WERR_ID_ASK_RETRY_RESET_DEVICE)	// 06
	ERROR_CODE_EQU(	X2WERR_ID_ASK_RETRY_FILEREAD	)	// 07
	ERROR_CODE_EQU(	X2WERR_ID_ASK_ERROR_EXIT		)	// 08
	ERROR_CODE_EQU(	X2WERR_ID_ERROR_ABORT			)	// 09
	ERROR_CODE_EQU(	X2WERR_ID_OUTOFMEMORY			)	// 10
	ERROR_CODE_EQU(	X2WERR_ID_OUTOFVIDEOMEMORY		)	// 11
	ERROR_CODE_EQU(	X2WERR_ID_INVALIDCALL			)	// 12
	ERROR_CODE_EQU(	X2WERR_ID_CANT_CREATE_OBJECT	)	// 13
	ERROR_CODE_EQU(	X2WERR_ID_CHECKDISK_FAILED		)	// 14

#endif	// ERROR_CODE_EQU

/*--------------------------------------------------------------*/
/*	HRESULTコード対応表											*/
/*--------------------------------------------------------------*/

#ifdef	HRESULT_TO_ERROR_CODE_EQU

	HRESULT_TO_ERROR_CODE_EQU(	E_OUTOFMEMORY,				X2WERR_ID_OUTOFMEMORY		)
	HRESULT_TO_ERROR_CODE_EQU(	D3DERR_OUTOFVIDEOMEMORY,	X2WERR_ID_OUTOFVIDEOMEMORY	)
	HRESULT_TO_ERROR_CODE_EQU(	D3DERR_INVALIDCALL,			X2WERR_ID_INVALIDCALL		)

#endif	// HRESULT_TO_ERROR_CODE_EQU
