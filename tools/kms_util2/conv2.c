/*
	conv2.c
	convert program

	2000/01/12 K.Takabe
	$Id: conv2.c,v 1.8 2002/06/05 02:40:42 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt_kms.h"
#include "prog_utl.h"


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

/* ポインタの復元処理 */
static void KmsDataCheck( KMS_DEF *def )
{
	KMS_MDL		*mdl ;
	KMS_MDLPACK	*pack ;
	int			i, j, n_models, tri_code ;

	mdl = def->models ;
	for ( i = def->n_x_models ; i > 0 ; i--, mdl++ ){
		pack = (KMS_MDLPACK*)( (int)mdl->packs + (int)def ) ;
		for ( j = mdl->n_packs ; j > 0 ; j--, pack++ ){
			pack->pad = 0 ;
		}
	}
}

/* ================================================================ */
/*
	Main Routine
*/
/* kms2 file version */
int Convert_SetTRICODE_Kms2( char *input_filename, char *tri_filename )
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
	if ( DG_GetMdlFormat( def ) != MGS_MODEL_NORM && DG_GetMdlFormat( def ) != MGS_MODEL_MULTITEX ){
		printf("format error!!(%d)\n", DG_GetMdlFormat( def ) );
		free( mem );
		fclose( fp );
		return (1);
	}
	//KmsDataCheck( def );

	def->texture = MGS_GetStrCode2( tri_filename ) ;
	printf("write code in kms file ( %d : <%s> )\n", def->texture , tri_filename );

	/* 書き戻し */
	fseek( fp, 0, SEEK_SET );
	fwrite( mem, 1, size, fp );

	free( mem );
	fclose( fp );

	return (0);
}

/* evm file version */
int Convert_SetTRICODE_Evm( char *input_filename, char *tri_filename )
{
	EVM_DEF	*def ;
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
#if 0
	if ( DG_GetMdlFormat( def ) != MGS_MODEL_NORM && DG_GetMdlFormat( def ) != MGS_MODEL_MULTITEX ){
		printf("format error!!(%d)\n", DG_GetMdlFormat( def ) );
		free( mem );
		fclose( fp );
		return (1);
	}
#endif

	def->texture = MGS_GetStrCode2( tri_filename ) ;
	printf("write code in evm file ( %d : <%s> )\n", def->texture , tri_filename );

	/* 書き戻し */
	fseek( fp, 0, SEEK_SET );
	fwrite( mem, 1, size, fp );

	free( mem );
	fclose( fp );

	return (0);
}

int Convert_SetTRICODE( char *input_filename, char *tri_filename )
{
	char	ext[256] ;

	MakeFileName( ext, "", "", input_filename );

	if ( StrCmpi( ext, ".kms" ) == 0 ||
		 StrCmpi( ext, ".kmx") == 0 ||
		 StrCmpi( ext, ".kmy") == 0 ){
		return Convert_SetTRICODE_Kms2( input_filename, tri_filename ) ;
	} else if ( StrCmpi( ext, ".evm" ) == 0 ||
				StrCmpi( ext, ".evx" ) == 0 ||
				StrCmpi( ext, ".evy" ) == 0 ) {
		return Convert_SetTRICODE_Evm( input_filename, tri_filename ) ;
	}
}

