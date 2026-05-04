
#include <stdio.h>
#include <stdlib.h>

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
	if ( *str1 != *str2 ) return (0);
	return (1);
}
