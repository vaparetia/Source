/*
	SARユーティリティ

 */

#include <stdio.h>
#include <stdlib.h>

#include "fmt_sar.h"

extern void MakeFileName( char *filepath, char *path, char *name, char *ext );
extern int StringEqual( char *str1, char *str2 );
extern int StringLastDelimiter( char *delimiter, char *str );
extern void StringDeleteCComment( char *str );
extern int IsCheckCVariable( char *str );
extern int IsCheckCharactor( char c, char *list );
extern int IsCheckAlphabet( char c );
extern int IsCheckNumber( char c );


/* ---------------------------------------------------------------- */
typedef struct {
	int		id ;
	char	name[256] ;
} CodeList ;
static int			n_se = 0 ;
static CodeList		se_list[4096] ;

int SAR_LoadSeCodeList( char *filename )
{
	FILE	*fp ;
	char	buffer[512], *ptr, *name ;

	if ( ( fp = fopen( filename, "rt" ) ) == NULL ){
		printf("file open error!!(%s)\n", filename );
		return ( 1 ) ;
	}

	while ( feof( fp ) == 0 ){
		int		n ;
		fgets( buffer, 512, fp );
		if ( buffer[0] == '#' ) continue ;
		if ( buffer[0] < ' ' && buffer[0] != '\t' ) continue ;
		ptr = buffer ;
		name = se_list[ n_se ].name ;
		while ( *ptr != '\0' && *ptr <= ' ' ) ptr++ ;
		while ( *ptr != '\0' && *ptr > ' ' && *ptr != ',' ){
			*name++ = *ptr++ ;
		}
		*name = '\0' ;

		while ( *ptr != '\0' && ptr[0] != '0' && ptr[1] != 'x' ) ptr++ ;
		sscanf( ptr, "0x%x", &n );
		name = se_list[ n_se ].id = n ;

		/*printf("%d : %s,0x%04x\n", n_se, se_list[ n_se ].name, se_list[ n_se ].id );*/
		n_se++ ;
	}
}

int SAR_LoadSeCodeList2( char *filename )
{
	FILE	*fp ;
	char	buffer[512], *ptr, *name, tmp_buffer[512] ;

	if ( ( fp = fopen( filename, "rt" ) ) == NULL ){
		printf("file open error!!(%s)\n", filename );
		return ( 1 ) ;
	}

	while ( feof( fp ) == 0 ){
		int		n ;
		fgets( buffer, 512, fp );

		strcpy( tmp_buffer, buffer );
		StringDeleteCComment( tmp_buffer );
		if ( IsCheckCVariable( tmp_buffer ) == 0 ) continue ;

		n = StringLastDelimiter( "//", buffer ) ;
		if ( n == 0 ) continue ;
		ptr = &buffer[ n + 2 ] ;
		name = se_list[ n_se ].name ;
		while ( *ptr != '\0' && *ptr <= ' ' ) ptr++ ;
		while ( *ptr != '\0' && *ptr > ' ' && *ptr != ',' ){
			*name++ = *ptr++ ;
		}
		*name = '\0' ;

		while ( *ptr != '\0' && IsCheckNumber( *ptr ) == 0 ) ptr++ ;
		sscanf( ptr, "%d", &n );
		name = se_list[ n_se ].id = n ;

		/*printf("%d : %s,%4d\n", n_se, se_list[ n_se ].name, se_list[ n_se ].id );*/
		n_se++ ;
	}
}

int GetSeCode( char *name )
{
	int		i ;
	for ( i = 0 ; i < n_se ; i++ ){
		if ( StringEqual( name, se_list[ i ].name ) ){
			//printf("find se name (%s,%d)\n", se_list[ i ].name, se_list[ i ].id );
			return ( se_list[i].id );
		}
	}
	//printf("not search se name!!\n");
	return ( 0 );
}

/* ---------------------------------------------------------------- */
int SAR_LoadSAR( char *filename, SAR_HEADER *header, SAR_LIST *info, char *sar_data )
{
	FILE	*fp ;
	int		size ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("SAR file open error!! (%s)\n", filename );
		return ( -1 );
	}

	fread( header, sizeof(SAR_HEADER), 1, fp );
	fread( info, sizeof(SAR_LIST), header->n_datas, fp );
	fread( sar_data, sizeof(char), header->data_size, fp );

	fclose( fp );
	return ( 0 );
}

int SAR_SaveSAR( char *filename, SAR_HEADER *header, SAR_LIST *info, char *sar_data )
{
	FILE	*fp ;
	int		size ;

	if ( ( fp = fopen( filename, "wb" ) ) == NULL ){
		printf("SAR file open error!! (%s)\n", filename );
		return ( -1 );
	}

	fwrite( header, sizeof(SAR_HEADER), 1, fp );
	fwrite( info, sizeof(SAR_LIST), header->n_datas, fp );
	fwrite( sar_data, sizeof(char), header->data_size, fp );

	fclose( fp );
	return ( 0 );
}


/* ---------------------------------------------------------------- */
static int LoadText( FILE *fp, char *buff )
{
	while ( 1 ){
		*buff = fgetc( fp ) ;
		if ( *buff == '\n' || feof( fp ) ){
			*buff = '\0' ;
			return ( 0 );
		}
		buff++ ;
	}

	return ( 0 );
}
static char* GetDataTop( char *ptr )
{
	while ( *ptr != '{' && *ptr != '\0' ) ptr++ ;
	if ( *ptr != '\0' ) ptr++ ;
	return ptr ;
}
static char* GetData_Buff( char *ptr, char *buff )
{
	while ( *ptr != '}' && *ptr != ',' && *ptr != '\0' ){
		*buff++ = *ptr++ ;
	}
	*buff = '\0' ;
	if ( *ptr != '\0' ) ptr++ ;
	return ( ptr ) ;
}
static char* GetData_String( char *ptr, char *buff )
{
	char tmp[256], *c ;
	ptr = GetData_Buff( ptr, tmp );
	c = tmp ;
	while ( *c == ' ' ) c++ ;
	while ( *c != '\0' ){
		if ( *c == ' ' ) *c = '\0' ;
		*buff++ = *c++ ;
	}
	*buff = '\0' ;
	return ( ptr );
}
static char* GetData_Int( char *ptr, int *data )
{
	char tmp[256] ;
	ptr = GetData_Buff( ptr, tmp );
	sscanf( tmp, "%d", data );
	return ( ptr );
}
static int GetData_Param( char *buff, char *keyword, int *data )
{
	int		flag ;
	while ( *buff != '\0' ){
		if ( *buff == '=' ){
			buff++ ;
			while ( *buff == ' ' ) buff++ ;
			sscanf( buff, "%d", data );
			return ( 1 );
		}
		buff++ ;
	}
	*data = 0 ;
	return ( 0 );
}

int SAR_LoadSEV( char *filename, char *datas, int *local_header_size )
{
	FILE	*fp ;
	int		i, count, base_tick ;
	char		buff[1024], data[256], name[256], *ptr ;
	SEV_LOCAL_HEADER *header ;
	SEV_ELEMENT	*element ;
	int		size ;

	*local_header_size = 0 ;
	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf("SEV file not found (%s)\n", filename );
		return ( 0 );
	}

	header = (SEV_LOCAL_HEADER*)datas ;

	// ヘッダーの読み出し
	LoadText( fp, buff );
	if  ( !StringEqual( buff, "SEV file " ) ){
		fclose( fp );
		printf("Not SEV file format !!!(%s)\n", filename );
		return ( 0 );
	}
	LoadText( fp, buff );
	GetData_Param( buff, "MotionTick", &base_tick );

	// 効果音ファイルの読み出し
	LoadText( fp, buff );
	GetData_Param( buff, "SE_DATA", &header->n_data );
	for ( i = 0 ; i < header->n_data ; i++ ){
		int		param ;
		LoadText( fp, buff );
		ptr = GetDataTop( buff );
		ptr = GetData_String( ptr, name ); header->params[i].se_code = GetSeCode( name ) ;
		ptr = GetData_Int( ptr, &param ); header->params[i].mode = param ;
		ptr = GetData_Int( ptr, &param ); header->params[i].length = param ;
		ptr = GetData_Int( ptr, &param ); header->params[i].joint = param ;
		ptr = GetData_Int( ptr, &param ); header->params[i].flags = param ;
#if 0
		printf("se:%s(0x%02x),%d,%d,%d,%08x\n", name,
			   header->params[i].se_code, header->params[i].mode, header->params[i].joint,
			   header->params[i].length, header->params[i].flags );
#endif
	}

	// イベント情報の読み出し
	LoadText( fp, buff );
	GetData_Param( buff, "EVENT_DATA", &count );
	element = (SEV_ELEMENT*)&header->params[ header->n_data ] ;
	for ( i = 0 ; i < count ; i++ ){
		int		frame, id ;
		LoadText( fp, buff );
		ptr = GetDataTop( buff );
		ptr = GetData_Int( ptr, &frame );
		ptr = GetData_Int( ptr, &id );
		element->time = base_tick * frame ;
		element->next_offset = 4 ;
		element->id = id ;
#if 0
		printf("data:frame = %d(%d), id = %d\n", element->time, frame, id );
#endif
		element++ ;
	}
	/* 終端コード入力 */
	element->time = 0xffff ;
	element->next_offset = 0 ;
	element->id = 0 ;
	count++ ;

	fclose( fp );

	size = sizeof(SEV_LOCAL_HEADER) + sizeof(SEV_PARAM) * header->n_data + sizeof(SEV_ELEMENT) * count ;
	*local_header_size = sizeof(SEV_LOCAL_HEADER) + sizeof(SEV_PARAM) * header->n_data ;
	return ( size );
}

/* ---------------------------------------------------------------- */
