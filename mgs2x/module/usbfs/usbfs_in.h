/*
	usbfs_in.h
		usbfs internal header

	2000/05/01	K.Uehara
	$Id: usbfs_in.h,v 1.4 2001/02/14 01:20:41 usr01475 Exp $
*/

// in rusbfs.c
int init_rusbfs( void ); // EEとの通信初期化

// in dumpdesc.c
extern void dump_usb_desc( int dev_id );	// デバッグ用

// in usbfs.c
int usbReadSet( int fd, int size );
int usbReadSync( char *buffer, int size, int last );
int usbReadEnd( void );

// for rusbfs.c EEとのリモート通信用

enum {
	USBFS_OK = 0,
	USBFS_INIT = 1,
	USBFS_OPEN = 2,
	USBFS_CLOSE = 3,
	USBFS_READ = 4,
	USBFS_WRITE = 5,
	USBFS_SEEK = 6,
	USBFS_DOPEN = 7,
	USBFS_DREAD = 8,
	USBFS_DCLOSE = 9,
	USBFS_GETSTAT = 0x0a,
	USBFS_PUTS = 0x10,
};


