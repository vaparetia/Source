/*
  sceMc* -> Xbox むりやりエミュレート
  
  2002/03/15 M.Kobayashi

  $Id: sceemux.h,v 1.1.1.3 2002/11/19 11:43:19 Yoshizawa1 Exp $
 */

#ifndef	__SCEEMUX_H__
#define	__SCEEMUX_H__


#ifdef __cplusplus
extern "C" {
#endif

extern void	sceMcEmulateInit( void );
extern void	sceMcEmulateStop( void );

	
extern int sceMcGetInfo( int slot, int port, int *type, int *free, int *format );	// slot<->port 入れ替え
extern int sceMcFormat( int slot, int port );
extern int sceMcUnformat( int slot, int port );
extern int sceMcSync( int mode, int *pcmd, int *presult );
extern int sceMcChdir( int slot, int port, const char* path, char* pwd );
extern int sceMcMkdir( int slot, int port, const char* name );
extern int sceMcOpen( int slot, int port, const char* name, int mode );
extern int sceMcClose( int fd );
extern int sceMcSeek( int fd, int offset, int mode );
extern int	sceMcRead( int fd, void* pBuf, int size );
extern int sceMcWrite( int fd, void* pBuf, int size );
extern int sceMcRename( int slot, int port, const char* org, const char* dst );
extern int sceMcDelete( int slot, int port, const char* name );
extern int sceMcSetFileInfo ( int slot, int port, const char* name, const char* info, unsigned valid );
extern int sceMcGetDir( int slot, int port, const char* name, unsigned mode, int maxent, sceMcTblGetDir* table );
	


	
inline int sceMcInit( void ) { sceMcEmulateInit(); return 0; }
inline int sceMcGetSlotMax( int port ) { return 4; }
	

#ifdef __cplusplus
}
#endif

#endif	//__SCEEMUX_H__
