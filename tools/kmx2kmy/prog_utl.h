/*
	prog_utl.h
	各種ユーティリティルーチン

	2000/01/22 K.Takabe
	$Id: prog_utl.h,v 1.1 2002/06/05 05:23:18 usr02774 Exp $
*/

#ifndef __PROG_UTL_H__
#define __PROG_UTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DIR_SEPARATE	'/'

extern void *Malloc0( size_t mem_size );
extern void Free( void *ptr );
extern void WriteData( void *buffer, int offset, void *data, size_t size );
extern size_t AlignSize16( size_t type_size, size_t data_count );
extern void MakeFileName( char *filepath, char *path, char *name, char *ext );
int StrCmpi( char *str1, char *str2 );


#ifdef __cplusplus
}
#endif

#endif
