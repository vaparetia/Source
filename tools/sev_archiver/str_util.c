/*



 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR_SEPARATE '/'

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

int StringEqual( char *str1, char *str2 )
{
	char	c1, c2 ;
	while ( *str1 != '\0' && *str2 != '\0' ){
		c1 = *str1++ ;
		c2 = *str2++ ;
		if ( c1 > 'a' && c1 < 'z' ) c1 = c1 - ( 'a' - 'A' );
		if ( c2 > 'a' && c2 < 'z' ) c2 = c2 - ( 'a' - 'A' );
		if ( c1 != c2 ) return (0);
	}
	if ( *str1 != *str2 ) return ( 0 );
	return (1);
}

int StringLastDelimiter( char *delimiter, char *str )
{
	int		delimiter_len ;
	int		last_delimiter = 0 ;
	char	*tmp_str ;

	delimiter_len = strlen( delimiter );
	tmp_str = str ;
	while ( tmp_str != NULL ){
		if ( ( tmp_str = strstr( tmp_str, delimiter ) ) == NULL ) break ;
		last_delimiter = (int)tmp_str - (int)str ;
		tmp_str++ ;
		if ( *tmp_str == '\0' ) break ;
	}

	return last_delimiter ;
}

void StringDeleteCComment( char *str )
{
	char	*top, *end ;

	/* Cコメント削除 */
	if ( ( top = strstr( str, "/*" ) ) != NULL ){
		if ( ( end = strstr( str, "*/" ) ) != NULL ){
			end += 2 ;
			while ( *end != '\0' ){
				*top++ = *end++ ;
			}
			*top = '\0' ;
		}
	}

	/* C++コメント削除 */
	if ( ( top = strstr( str, "//" ) ) != NULL ){
		*top = '\0' ;
	}

}

int IsCheckCharactor( char c, char *list )
{
	for ( ; *list != '\0' ; list++ ){
		if ( c == *list ) return ( 1 );
	}
	return ( 0 );
}

int IsCheckAlphabet( char c )
{
	if ( (  c >= 'a' && c <= 'z' ) || (  c >= 'A' && c <= 'Z' ) ) return ( 1 );
	return ( 0 );
}

int IsCheckNumber( char c )
{
	if ( c >= '0' && c <= '9' ) return ( 1 );
	return ( 0 );
}

int IsCheckCVariable( char *str )
{
	int		res = 0, top_flag = 0 ;

	for ( ; *str != '\0' ; str++ ){
		if ( IsCheckCharactor( *str, "\t ," ) ){
			top_flag = 0 ;
			continue ;
		}
		if ( top_flag == 0 ){
			if ( IsCheckAlphabet( *str ) || *str == "_" ){
				top_flag = 1 ;
			} else {
				top_flag = 2 ;
			}
		} else {
			if ( IsCheckAlphabet( *str ) || IsCheckNumber( *str ) || *str == "_" ){
			} else {
				if ( top_flag == 1 && IsCheckCharactor( *str, "({" ) == 0 ){
					res = 1 ;
					return ( res );
				}
				top_flag = 0 ;
			}
		}
	}
	return ( res );
}

/* ---------------------------------------------------------------- */
/* 文字列コードの取得 */
unsigned short MGS_GetStrCode( char *str )
{
	unsigned short	code = 0 ;
	while ( *str != 0 && *str != '.' ){
		code = ( (code<<5) | ( code>>11) ) & 0xffff ;
		if ( *str == ':' ) code = - *str ;
		if ( *str == '\\' ) code = - *str ;
		if ( *str == '/' ) code = - *str ;
		code = ( code ) + (unsigned char)*str++ ;
	}
	return ( code & 0xffff );
}

#define BIT_LEN		24
unsigned int MGS_GetStrCode2( char *string )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;

	p = ( unsigned char * )string;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	while( ( c = *( p++ ) ) != '\0' ){
		if ( c == ':' ) continue ;
		if ( c == '\\' ) continue ;
		if ( c == '/' ) continue ;
		if ( c == '.' ) break ;
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c ;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	return id;
}

