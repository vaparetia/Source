/*
	prog_utl.c
	各種ユーティリティルーチン

	2000/01/22 K.Takabe
	$Id: prog_utl.c,v 1.1 2000/01/22 10:33:00 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include "prog_utl.h"

/* ================================================================ */
void *Malloc0( size_t mem_size )
{
	void	*ptr ;
	ptr = malloc( mem_size );
	memset( ptr, 0, mem_size );
	return ( ptr );
}
void Free( void *ptr )
{
	if ( ptr != NULL ) free( ptr );
}
void WriteData( void *buffer, int offset, void *data, size_t size )
{
	memcpy( (void*)( (int)buffer + offset ), data, size );
}
size_t AlignSize16( size_t type_size, size_t data_count )
{
	return ( ( ( type_size * data_count ) + 15 ) & 0xfffffff0 );
}
void MakeFileName( char *filepath, char *path, char *name, char *ext )
{
	char	*tmp_ptr, *ptr ;

	if ( path != NULL ){
		tmp_ptr = path ;
		ptr = path ;
		while ( *ptr != '\0' ){
			if ( *ptr == DIR_SEPARATE ){
				tmp_ptr = ptr + 1 ;
			}
			ptr++ ;
		}
		for ( ptr = path ; ptr != tmp_ptr ; ) *filepath++ = *ptr++ ;
	}

	if ( name != NULL ){
		tmp_ptr = name ;
		ptr = name ;
		while ( *ptr != '\0' ){
			if ( *ptr == DIR_SEPARATE ){
				tmp_ptr = ptr + 1 ;
			}
			ptr++ ;
		}
		for ( ptr = tmp_ptr ; ( *ptr != '\0' ) && ( *ptr != '.') ; ) *filepath++ = *ptr++ ;
	}

	if ( ext != NULL ){
		*filepath++ = '.' ;
		tmp_ptr = ext ;
		ptr = ext ;
		while ( *ptr != '\0' ){
			if ( *ptr == '.' ){
				tmp_ptr = ptr + 1 ;
			}
			ptr++ ;
		}
		for ( ptr = tmp_ptr ; *ptr != '\0' ; ) *filepath++ = *ptr++ ;
	}

	*filepath = '\0' ;
}

