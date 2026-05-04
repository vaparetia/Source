/*
	prog_utl.c
	各種ユーティリティルーチン

	2000/01/22 K.Takabe
	$Id: prog_utl.c,v 1.1 2001/07/14 11:04:57 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include "prog_utl.h"

/* ================================================================ */

/* ０クリアされたメモリを確保 */
void *Malloc0( size_t mem_size )
{
	void	*ptr ;
	ptr = malloc( mem_size );
	memset( ptr, 0, mem_size );
	return ( ptr );
}

/* メモリを解放 */
void Free( void *ptr )
{
	if ( ptr != NULL ) free( ptr );
}

/* 指定されたバッファへオフセットとサイズを指定して書き込み */
void WriteData( void *buffer, int offset, void *data, size_t size )
{
	memcpy( (void*)( (int)buffer + offset ), data, size );
}

/* １qword境界に揃えた場合のサイズを求める */
size_t AlignSize16( size_t type_size, size_t data_count )
{
	return ( ( ( type_size * data_count ) + 15 ) & 0xfffffff0 );
}

/* 指定したパス,名前,拡張子からファイルパスを生成 */
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

/* 大文字小文字を区別しないstrcmp関数 */
int StrCmpi( char *str1, char *str2 )
{
	char	c1, c2 ;
	int		res ;

	res = 0 ;
	while ( !( *str1 == '\0' && *str2 == '\0' ) ){
		c1 = *str1++ ;
		c2 = *str2++ ;
		if ( c1 >= 'A' && c1 <= 'Z' ) c1 = c1 + 'a' - 'A' ;
		if ( c2 >= 'A' && c2 <= 'Z' ) c2 = c2 + 'a' - 'A' ;
		if ( c1 == c2 ) continue ;
		break ;
	}
	res = c1 - c2 ;
	return ( res );
}
