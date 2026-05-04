/*
	nousbfs.c
		USBFSのダミーモジュール

	2000/05/09 K.Uehara
	$Id: nousbfs.c,v 1.4 2001/02/14 01:20:41 usr01475 Exp $
*/

#include <stdio.h>
#include <kernel.h>
#include "usbfs.h"

ModuleInfo Module = { "KCEJ_NO_USB", 0x0101 };

int usbInit( char *id )
{
	return 0;
}

int usbOpen( char *filename, int mode )
{
	return open( filename, mode );
}

int usbClose( int fd )
{
	return close( fd );
}

int usbRead( int fd, void *buffer, int size )
{
	return read( fd, buffer, size );
}

int usbWrite( int fd, void *buffer, int size )
{
	return write( fd, buffer, size );
}

int usbLseek( int fd, int offset, int where )
{
	return lseek( fd, offset, where );
}

int usbfs_start( void )
{
    extern libhead usbfs_entry; /* ライブラリ名_entry を参照 */

    if( RegisterLibraryEntries(&usbfs_entry) != 0 ) {
		/* 既に同名の常駐ライブラリがいるので登録に失敗 */
		return NO_RESIDENT_END; /* 終了してメモリから退去 */
    }

	printf( "USB FILE DO NOT USE\n" );

	return RESIDENT_END;
}
