/*
	fscd.c
		CDファイル読み書き用ファイルシステム

	2000/06/06	K.Uehara
	$Id: fscd.c,v 1.1.1.3 2002/11/19 11:42:39 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sifdev.h>

#include "cdbios.h"
#include "libfs.h"

/* ---------------------------------------------------------------------- */
/*
	互換用ダミー関数
*/

int  pcOpen(char *filename, int flag)
{
	return -1;
}

int  pcClose(int fd)
{
	return -1;
}

int  pcRead(int fd, void *buf, int nbyte)
{
	return -1;
}

int  pcWrite(int fd, void *buf, int nbyte)
{
	return -1;
}

int  pcLseek(int fd, unsigned int offset, int whence)
{
	return -1;
}

/* ---------------------------------------------------------------------- */
/*
	初期化
*/

void FS_CdInit( void )
{
	cdbios_init();
}

/* ---------------------------------------------------------------------- */
/*
	他のファイル読み込み用
*/

void FS_LoadFileRequest( int fileno, int offset, int size, void *buffer )
{
   BP_BREAK;
	while( cdbios_get_status() != 0 );

	SyncDCache( buffer, buffer + size );

	cdbios_read( buffer, FS_GET_FILE_POS( fileno ) + offset, size );
}

int  FS_LoadFileSync( void )
{
	return cdbios_get_status();
}
