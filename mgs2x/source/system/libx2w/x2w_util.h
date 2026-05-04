/*--------------------------------------------------------------*/
/*	x2w_util.h													*/
/*					Utility										*/
/*--------------------------------------------------------------*/
#ifndef	__X2W_UTIL_H__
#define	__X2W_UTIL_H__

#include <dxerr8.h>

#ifdef	__XBOX2WIN_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------*/
/*	macro														*/
/*--------------------------------------------------------------*/

#ifdef DEBUG_MODE
#define	dbgErrMessPuts(str, hr)		DXTRACE_ERR((str), (hr))
#else
#define	dbgErrMessPuts(str, hr)
#endif

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/

extern void	X2W_InisOSStatus(void) ;
extern void	X2W_ReleaseOSStatus(void) ;

extern BOOL	X2W_OverlappedReadFileEnable(void) ;	// 非同期ReadFileが使用できるか検査

extern BOOL	X2W_OSIsWin32System(void) ;		// Windows 95/98/ME系列判定
extern BOOL	X2W_OSIsWinNTSystem(void) ;		// Windows NT/2000/XP系列判定

extern void	X2W_NextProcess(void) ;	// 次のProcess起動

extern BOOL	X2W_InsertDiskCheck(void) ;	// Disk挿入検査(違法複製対策)

#ifdef __cplusplus
}
#endif

#undef	EXTERN
#endif /* !defined(__X2W_UTIL_H__) */

/*-- End Of File --*/

