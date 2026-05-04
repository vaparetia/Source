/*--------------------------------------------------------------*/
/*	error_util.h												*/
/*					Error Utility								*/
/*--------------------------------------------------------------*/
#ifndef	__ERROR_UTIL_H__
#define	__ERROR_UTIL_H__

#ifdef	__ERROR_UTIL_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------*/
/*	広域変数													*/
/*--------------------------------------------------------------*/
EXTERN	BOOL	X2W_ErrorDisplayStatus ;

/*--------------------------------------------------------------*/
/*	Message Box Type											*/
/*--------------------------------------------------------------*/
	/* 確認・警告用 */
#define	X2W_MB_WARNING	(MB_OK					\
						| MB_ICONEXCLAMATION	\
						| MB_SYSTEMMODAL		\
						| MB_SETFOREGROUND)

#define	X2W_MB_ERROR	X2W_MB_WARNING

	/* YES/NO選択用 */
#define	X2W_MB_YESNO	(MB_YESNO				\
						| MB_ICONQUESTION		\
						| MB_SYSTEMMODAL		\
						| MB_SETFOREGROUND)

	/* 再試行/キャンセル選択用 */
#define	X2W_MB_RETRYCANCEL	(MB_RETRYCANCEL			\
							| MB_ICONEXCLAMATION	\
							| MB_SYSTEMMODAL		\
							| MB_SETFOREGROUND)

	/* OK/CANCEL択用 */
#define	X2W_MB_OKCANCEL	(MB_OKCANCEL			\
						| MB_ICONQUESTION		\
						| MB_SYSTEMMODAL		\
						| MB_SETFOREGROUND)


/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/

extern	int	X2W_ErrorPrintf(DWORD type, char *fmt, ...) ;
extern	int	X2W_ErrorPuts(DWORD type, char *str) ;
extern	int	X2W_ErrorPrintfID(DWORD type, int err_id, ...) ;
extern	int	X2W_ErrorPutsID(DWORD type, int err_id) ;
extern	int	X2W_ErrorPutsHResult(DWORD type, HRESULT hr) ;

#define	X2W_GetErrorDisplayStatus() (X2W_ErrorDisplayStatus)	// 表示状態取得

extern	void	X2W_ErrorAbort(void) ;	// 異常終了手続き(内部で終了)

extern	char	*X2W_ErrorGetErrorMess(char *str, DWORD size, HRESULT hr) ;
extern	char	*X2W_ErrorGetLastErrorMess(char *str, DWORD size) ;

extern	void	InitErrorLog(void) ;
extern	void	ReleaseErrorLog(void) ;
extern	void	ErrorLogPrintf(char *fmt, ...) ;
extern	void	ErrorLogPuts(char *str) ;
extern	void	ErrorLogFlush(void) ;

#ifdef __cplusplus
}
#endif

#undef	EXTERN
#endif /* !defined(__ERROR_UTIL_H__) */

/*-- End Of File --*/

