/*
	usbfs.h
		usbfs クライアント
		PCをサーバーにして,USB経由でファイルを読み込む。

	2000/05/01	K.Uehara
	$Id: usbfs.h,v 1.4 2001/02/14 01:20:41 usr01475 Exp $
*/

/* in usbfs.c */

// 基本的にsceOpen系の関数と同等。

#ifdef __cplusplus
extern "C" {
#endif

// EEからはこちらで呼び出す。IOPでは、内部使用。
int usbInit( char *id );	// id は5バイト以内の文字列
int usbOpen( char *filename, int mode );
int usbClose( int fd );
int usbRead( int fd, void *buffer, int size );
int usbWrite( int fd, void *buffer, int size );
int usbLseek( int fd, int offset, int where );

int usbDopen( char *filename );
int usbDread( int fd, void *buf );
int usbDclose( int fd );
int usbGetStat( char *filename, void *buf );

int usbPuts( char *mes );
int usbPrintf( char *format, ... );	// EEのみ

#if defined( IOP )
// usbfs.irxではusb系に、nousbfs.irxではsce系の関数を呼び出す。
int pcInit( char *id );	// id は5バイト以内の文字列
int pcOpen( char *filename, int mode );
int pcClose( int fd );
int pcRead( int fd, void *buffer, int size );
int pcWrite( int fd, void *buffer, int size );
int pcLseek( int fd, int offset, int where );
#endif

#ifdef __cplusplus
}
#endif

typedef struct {
	int p1;
	int p2;
	int p3;
	int p4;
} USBFS_PARAM;

