/*
	conv2.c
	convert program

	2000/01/12 K.Takabe
	$Id: conv2.c,v 1.1 2000/01/18 05:02:44 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "fmt_kms.h"

#define DIR_SEPARATE	'/'

/* ================================================================ */
static MakeFileName( char *filepath, char *path, char *name, char *ext )
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

#define BIT_LEN		24
unsigned int MGS_GetStrCode2( char *input )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;
	char	filename[256], *string ;

	MakeFileName( filename, NULL, input, NULL );
	string = filename ;

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

/* ================================================================ */
/*
	Main Routine
*/
int Convert_SetTRICODE( char *input_filename, char *tri_filename )
{
	KMS_DEF	*def ;
	FILE	*fp ;
	void	*mem ;
	int		size ;

	if ( ( fp = fopen( input_filename, "r+b" ) ) == NULL ){
		fprintf(stderr,"file open error!!(%s)\n", input_filename );
		return (-1);
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	mem = malloc( size );

	fread( mem, 1, size, fp );

	def = mem ;
	def->models[0].pad2 = MGS_GetStrCode2( tri_filename ) ;
	printf("write code in kms file ( %d : <%s> )\n", def->models[0].pad2, tri_filename );

	/* ΩÒ§≠Ã·§∑ */
	fseek( fp, 0, SEEK_SET );
	fwrite( mem, 1, size, fp );

	free( mem );
	fclose( fp );

	return (0);
}


