/*
   MDU_darFile.c
   darfile関数
   
   by M.Sonoyama 1999 Aug.～
   $Id: MDU_darFile.c,v 1.5 2000/08/30 02:17:35 usr02011 Exp $
   
   Konami Computer Entertainment Japan West   
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>

#include	"MDU_util.h"

#define MAX_FILE_NAME_LEN	(256)
#define	MAX_DARS		(16)

typedef	struct {
    char	name[ MAX_FILE_NAME_LEN ] ; /* 名前 */
    int		size ;			    /* サイズ */
    void	*buf ;			    /* ファイル中身へのポインタ */
} DAR_PART ;

typedef	struct {
    char	name[ MAX_FILE_NAME_LEN ] ;
    int		n_files ;			/* 実際に格納しているファイル数 */
    DAR_PART	*parts ;		/* 各ファイルへのポインタ */
} DAR_FILE ;

static	DAR_FILE	DarFiles[ MAX_DARS ] ;
static	int		Used[ MAX_DARS ] ;
static	int		CurrentDarFile ;
static	DAR_FILE	*DarFile ;
static	int		N_Filess[ MAX_DARS ] ;
static	int		*N_Files ; /* 確保した領域数 */

static	int	VerboseMode ;
static	int	No_WriteMode ;
static	FILE *DarFP ;

static	void	*Dar_ReadOneFile2( FILE *, char *, int * ) ;
static	void	Dar_WriteOneFile2( FILE *, char *, int, void * ) ;

/* DarFile 構造体全初期化 */
void	MDU_DarInitFileAll( void ) 
{
    int		i ;

    for ( i = 0; i < MAX_DARS; i ++ ) {
		DarFiles[ i ].n_files = 0 ;
		N_Filess[ i ] = 0 ;
		Used[ i ] = 0 ;
		DarFiles[ i ].parts = NULL ;
    }	
    CurrentDarFile = -1 ;
    VerboseMode = No_WriteMode = 0 ;
}

void	MDU_DarChangeMode( a, b )
int	a, b ;
{
    VerboseMode = a ;
    No_WriteMode = b ;
}

/* 空いている DarFile を検索 */
static 	int	FindDarFile( void )
{
    int		i ;

    for ( i = 0; i < MAX_DARS; i ++ ) {
		if ( Used[ i ] == 0 ) return i ;
    }
    return -1 ;
}

/* カレントDarFile構造体をセット */
static	int	SetCurrentDarFile( n )
int		n ;
{
    if ( n < 0 || n >= MAX_DARS ) return -1 ;
    if ( Used[ n ] == 0 ) return -1 ;
    DarFile = &( DarFiles[ n ] ) ;
    N_Files = &( N_Filess[ n ] ) ;
    CurrentDarFile = n ;
    return 0 ;
}

/* DarFile 構造体初期化 */
static	int	InitDarFile( void )
{
    int		n ;

    n = FindDarFile() ;
    if ( n < 0 ) {
		fprintf( stderr, "too many darfiles.\n" ) ;
		return -1 ;
    }
    Used[ n ] = 1 ;
    SetCurrentDarFile( n ) ;
    DarFile->n_files = 0 ;
    *N_Files = 0 ;
    DarFile->parts = NULL ;
    return n ;
}

/* DarFile 構造体領域確保 */
static	int	AllocDarFile( name, n_files )
char	*name ;
int	n_files ;
{
    int		i ;
    DAR_PART	*parts ;

    DarFile->parts = parts 
		= ( DAR_PART * )MDU_Alloc( sizeof( DAR_PART ) * n_files ) ;
    if ( parts == NULL ) return -1 ;
    strcpy( DarFile->name, name ) ;
    DarFile->n_files = 0 ;
    *N_Files = n_files ;
    for ( i = 0; i < n_files; i ++ ) {
		parts->size = 0 ;
		parts->buf = NULL ;
		parts ++ ;
    }
    return 0 ;
}

/* DarFile 構造体解放 */
static	void	FreeDarFile( void )
{
    int		i ;
    DAR_PART	*parts ;

    parts = DarFile->parts ;
    for ( i = 0; i < *N_Files; i ++ ) {
		if ( parts->buf != NULL ) MDU_Free( parts->buf ) ; 
		parts ++ ;
    }
    MDU_Free( DarFile->parts ) ;
    DarFile->n_files = 0 ;
    *N_Files = 0 ;
    DarFile->parts = NULL ;
    Used[ CurrentDarFile ] = 0 ;
    CurrentDarFile = -1 ;
}

/* 空いているDAR_PARTを検索 */
static	DAR_PART	*FindEmptyDarPart( void )
{
    DAR_PART		*parts ;
    int			i ;

    parts = DarFile->parts ;
    for ( i = 0; i < *N_Files; i ++ ) {
		if ( parts->buf == NULL ) return parts ;
		parts ++ ;
    }
    return NULL ;
}

/* DAR_PART を名前検索 */
static	DAR_PART	*FindDarPart( name )
char		*name ;
{
    DAR_PART		*parts ;
    int			i ;

    parts = DarFile->parts ;
    for ( i = 0; i < *N_Files; i ++ ) {
		if ( parts->buf != NULL ) {
			if ( strcmp( parts->name, name ) == 0 ) return parts ;
		}
		parts ++ ;
    }
    return NULL ;
}

/* DarFile 構造体に登録 */
static	int	AddDarFile( fp, n_files )
FILE		*fp ;
int		n_files ;
{
    DAR_PART		*parts ;
    
    while( -- n_files >= 0 ) {
		parts = FindEmptyDarPart() ;
		if ( parts == NULL ) return -1 ;
		parts->buf = Dar_ReadOneFile2( fp, parts->name, &( parts->size ) ) ;
		if ( parts->buf == NULL ) return -1 ;
		DarFile->n_files ++ ;
    }
    return 0 ;
}

/* DarFile 構造体から削除 */
static	void	DeleteFromDarFile( n_files, files )
int		n_files ;
char		**files ;
{
    int		i ;
    DAR_PART	*parts ;

    for ( i = 0; i < n_files; i ++ ) {
		parts = FindDarPart( files[ i ] ) ;
		if ( parts == NULL ) continue ;
		if ( VerboseMode )
			fprintf( stdout, "%s is deleted\n", files[ i ] ) ;
		MDU_Free( parts->buf ) ;
		parts->buf = NULL ;
		DarFile->n_files -- ;
    }
}

/* DarFile 構造体をファイルに書き込み */
static	int	WriteFromDarFile( darfile )
char		*darfile ;
{
    FILE	*fp ;
    DAR_PART	*parts ;
    long	n_files ;

    fp = fopen( darfile, "w" ) ;
    if ( fp == NULL ) return - 1 ;
    n_files = ( long )DarFile->n_files ;
    MDU_FwriteEndianLong( fp, &n_files, 1, MDU_DAR_MODE_KEEP ) ;
    parts = DarFile->parts ;
    n_files = *N_Files ;
    while( -- n_files >= 0 ) {
		if ( parts->buf != NULL ) {
			Dar_WriteOneFile2( fp, parts->name, parts->size, parts->buf ) ;
		}
		parts ++ ;
    }
    fclose( fp ) ;
    return 0 ;
}

/* DarFile 構造体に読み込み */
static	int	ReadToDarFile( darfile )
char	*darfile ;
{
    FILE	*fp ;
    long	n_files ;

    fp = fopen( darfile, "r" ) ;
    if ( fp == NULL ) return -1 ;
    MDU_FreadEndianLong( fp, &n_files, 1 ) ;
    if ( AllocDarFile( darfile, n_files ) < 0 ) return -1 ;
    if ( AddDarFile( fp, n_files ) < 0 ) return -1 ;
    fclose( fp ) ;
    return 0 ;
}

/*---------------------------------------------------------------*/

/*	ファイルサイズを取得 	*/
static	u_long	_GetFileSize( fp )
FILE		*fp ;
{
    u_long	where, size ;

    where = ftell( fp ) ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) - where ;
    fseek( fp, where, SEEK_SET ) ;
    return size ;
}

/*	名前を書き込む 		*/
static	void	WriteString( fp, str, max )
FILE		*fp ;
char		*str ;
int		max ;
{
    int		len ;
    
    len = strlen( str ) + 1 ;
    if ( len > max ) {
		len = max ;
    }
	str[ len ] = '\0' ;
    fwrite( str, 1, len, fp ) ;
}

/*	Write Padding Skip	*/
static	void	WritePaddingSkip( fp )
FILE		*fp ;
{	
    long	pad ;
    char	tmp ;
    
    tmp = 0 ;
    pad = 3 & ( 4 - ( 3 & ftell( fp ) ) ) ;
    while( -- pad >= 0 ) {
		fwrite( &tmp, 1, 1, fp ) ;
    }
}

/*	Write Padding Skip１６	*/
static	void	WritePaddingSkip16( fp )
FILE		*fp ;
{	
    long	pad ;
    char	tmp ;
#if 1
    tmp = 0 ;
    pad = 15 & ( 16 - ( 15 & ftell( fp ) ) ) ;
    while( -- pad >= 0 ) {
		fwrite( &tmp, 1, 1, fp ) ;
    }
#endif
}

/*	1ファイル dar に書き込み 	*/
static	int	WriteOneFile( darfp, fp, name )
FILE		*darfp, *fp ;
char		*name ;
{
    u_long	size ;
    char	buf[ 1024 ], name2[ 256 ] ;

    size = _GetFileSize( fp ) ;
    strcpy( name2, MDU_TailName( name ) ) ;
    if ( VerboseMode ) fprintf( stdout, "%s ( size %ld )\n", name2, size ) ;
    WriteString( darfp, name2, MAX_FILE_NAME_LEN ) ;
    WritePaddingSkip( darfp ) ;
    MDU_FwriteEndianLong( darfp, &size, 1, MDU_DAR_MODE_KEEP ) ;
    WritePaddingSkip16( darfp ) ;
    while( size >= 1024 ) {
		fread( buf, 1, 1024, fp ) ;
		fwrite( buf, 1, 1024, darfp ) ;
		size -= 1024 ;
    }
    if ( size > 0 ) {
		fread( buf, 1, size, fp ) ;
		fwrite( buf, 1, size, darfp ) ;
    }
    return 0 ;
}

/* 	１ファイル dar に書き込み２	*/
static	void	Dar_WriteOneFile2( fp, name, size, buf )
FILE		*fp ;
char		*name ;
int		size ;
void		*buf ;
{
    char	name2[ 256 ] ;
#if 0
    if ( VerboseMode ) fprintf( stdout, "%s ( size %d )\n", name, size ) ;
#endif
    strcpy( name2, MDU_TailName( name ) ) ;
    WriteString( fp, name2, MAX_FILE_NAME_LEN ) ;
    WritePaddingSkip( fp ) ;
    MDU_FwriteEndianLong( fp, ( long * )( &size ), 1, MDU_DAR_MODE_KEEP ) ;
    WritePaddingSkip16( fp ) ;
    fwrite( buf, 1, size, fp ) ;
    putc( 0, fp ) ;
}

/*	Archive の作成 	   */
int	MDU_DarCreateArchive( darfile, n_files, files )
char		*darfile ;
int		n_files ;
char		**files ;
{
    char	*name ;
    FILE	*darfp, *fp ;

    if ( VerboseMode ) fprintf( stdout, "create archive %s\n", darfile ) ;
    DarFP = darfp = fopen( darfile, "wb" ) ;
    if ( darfp == NULL ) {
		printf( "cannot open file : %s\n", darfile ) ;
		return -1 ;
    }
    MDU_FwriteEndianLong( darfp, ( long * )( &n_files ), 1, MDU_DAR_MODE_KEEP ) ;
    while( -- n_files >= 0 ) {
		name = *( files ++ ) ;
		fp = fopen( name, "rb" ) ;
		if ( fp == NULL ) {
			printf( "cannot open file : %s\n", name ) ;
			return -1 ;
		}
		WriteOneFile( darfp, fp, name ) ;
		putc( 0, darfp ) ;
		fclose( fp ) ;
    }
    fclose( darfp ) ;
    return 0 ;
}

/*	Archive に追加	*/
int	MDU_DarAppendArchive( darfile, n_files, files )
char		*darfile ;
int		n_files ;
char		**files ;
{
    int		all ;
    char	*name ;
    FILE	*darfp, *fp ;

    if ( VerboseMode ) fprintf( stdout, "append archive %s\n", darfile ) ;
    darfp = fopen( darfile, "rb+" ) ;
    if ( darfp == NULL ) return -1 ;
    MDU_FreadEndianLong( darfp, ( long * )( &all ), MDU_DAR_MODE_KEEP ) ;
    fseek( darfp, 0, SEEK_END ) ;
    while( -- n_files >= 0 ) {
		name = *( files ++ ) ;
		fp = fopen( name, "rb" ) ;
		if ( fp == NULL ) {
			fprintf( stderr, "cannot open file %s\n", name ) ;
			continue ;
		}
		WriteOneFile( darfp, fp, name ) ;
		putc( 0, darfp ) ;
		fclose( fp ) ;
		all ++ ;
    }
    fseek( darfp, 0, SEEK_SET ) ;
    MDU_FwriteEndianLong( darfp, ( long * )( &all ), 1, MDU_DAR_MODE_BROKEN ) ;
    fclose( darfp ) ;
    return 0 ;
}

/*---------------------------------------------------------------*/

/*	名前を読み込む		*/
static	int	ReadString( fp, str, max ) 
FILE		*fp ;
char		*str ;
int		max ;
{
    int		len ;
    char	c ;

    len = 0 ;
    while( !feof( fp ) ) {
		*( str ++ ) = c = fgetc( fp ) ;
		if ( c == '\0' ) break ;
		if ( ++ len >= max ) return -1 ;
    } 
    return len ;
}

/*	Read Padding Skip	*/
static	void	ReadPaddingSkip( fp )
FILE		*fp ;
{
    long	pad ;

    pad = 3 & ( 4 - ( 3 & ftell( fp ) ) ) ;
    fseek( fp, pad, SEEK_CUR ) ;
}

/*	Read Padding Skip１６	*/
static	void	ReadPaddingSkip16( fp )
FILE		*fp ;
{
#if 1
    long	pad ;

    pad = 15 & ( 16 - ( 15 & ftell( fp ) ) ) ;
    fseek( fp, pad, SEEK_CUR ) ;
#endif
}

/*	1ファイルdarから読み込んで書きだし   */
static	int	ReadOneFile( darfp, fp, size )
FILE		*darfp, *fp ;
int		size ;
{
    char	buf[ 1024 ] ;

    while( size >= 1024 ) {
		fread( buf, 1, 1024, darfp ) ;
		fwrite( buf, 1, 1024, fp ) ;
		size -= 1024 ;
    }
    if ( size > 0 ) {
		fread( buf, 1, size, darfp ) ;
		fwrite( buf, 1, size, fp ) ;
    }
    return 0 ;
}

/*	１ファイルdarから読み込み２	*/
static	void	*Dar_ReadOneFile2( fp, name, size )
FILE		*fp ;
char		*name ;
int		*size ;
{	
    int		sz ;
    void	*buf ;

    if ( ReadString( fp, name, MAX_FILE_NAME_LEN ) < 0 ) return NULL ;
    ReadPaddingSkip( fp ) ;
    MDU_FreadEndianLong( fp, ( long * )( &sz ), 1 ) ;
    ReadPaddingSkip16( fp ) ;
    if ( ( buf = MDU_AllocVoid( sz ) ) == NULL ) return NULL ;
    fread( buf, 1, sz, fp ) ;
    fseek( fp, 1, SEEK_CUR ) ;    
    *size = sz ;
    return buf ;
}

/*	Archive から 抽出   */
int	MDU_DarExtractArchive( dir, darfile )
char		*dir ;
char		*darfile ;
{
    int			size, n_files ;
    char		name[ MAX_FILE_NAME_LEN ], str[ 256 ] ;
    char		full[ MAX_FILE_NAME_LEN + 256 ] ;
    FILE		*darfp, *fp ;

    darfp = fopen( darfile, "rb" ) ;
    if ( darfp == NULL ) return -1 ;
    MDU_FreadEndianLong( darfp, ( u_long * )( &n_files ), 1 ) ;
    if ( VerboseMode ) fprintf( stdout, "%d files exist \n", n_files ) ;
    while( -- n_files >= 0 ) {
		if ( ReadString( darfp, name, MAX_FILE_NAME_LEN ) < 0 ) return -1 ;
		ReadPaddingSkip( darfp ) ;
		MDU_FreadEndianLong( darfp, ( u_long * )( &size ), 1 ) ;
		ReadPaddingSkip16( darfp ) ;
		if ( VerboseMode ) fprintf( stdout, "%s ( size %d )\n", name, size ) ;
		sprintf( str, "%s", name ) ;
		if ( No_WriteMode ) {
			fseek( darfp, size, SEEK_CUR ) ;
			goto dar_load_skip ;
		}
		strcpy( full, dir ) ;
		strcat( full, name ) ;
		fp = fopen( full, "wb" ) ;
		if ( fp == NULL ) return -1 ;
		ReadOneFile( darfp, fp, size ) ;
		fclose( fp ) ;
		dar_load_skip :
        fseek( darfp, 1, SEEK_CUR ) ;
    }
    fclose( darfp ) ;
    return 0 ;
}

/*	DarFile 中に含まれるかどうかチェック 	*/
static	int	InsideDar( name, n_files, files )
char		*name ;
int		n_files ;
char		**files ;
{
    int		i ;

    for ( i = 0; i < n_files; i ++ ) {
		if ( strcmp( name, files[ i ] ) == 0 ) break ;
    }
    if ( i == n_files ) return 0 ;
    return 1 ;
}

/*	Archive から 特定のファイルを抽出	*/
int		MDU_DarExtractArchive2( dir, darfile, n_files, files )
char		*dir ;
char		*darfile ;
int		n_files ;
char		**files ;
{
    int			size, all, remain ;
    char		name[ MAX_FILE_NAME_LEN ], str[ 256 ] ;
    char		full[ MAX_FILE_NAME_LEN + 256 ] ;
    FILE		*darfp, *fp ;

    remain = n_files ;
    darfp = fopen( darfile, "rb" ) ;
    if ( darfp == NULL ) return -1 ;
    MDU_FreadEndianLong( darfp, ( u_long * )( &all ), 1 ) ;
    while( -- all >= 0 ) {
		if ( ReadString( darfp, name, MAX_FILE_NAME_LEN ) < 0 ) return -1 ;
		ReadPaddingSkip( darfp ) ;
		MDU_FreadEndianLong( darfp, ( u_long * )( &size ), 1 ) ;
		ReadPaddingSkip16( darfp ) ;
		if ( InsideDar( name, n_files, files ) == 0 ) {
			fseek( darfp, size, SEEK_CUR ) ;
			goto dar_load_skip2 ;
		}
		if ( VerboseMode ) fprintf( stdout, "%s ( size %d )\n", name, size ) ;
		sprintf( str, "%s", name ) ;
		if ( No_WriteMode ) {
			fseek( darfp, size, SEEK_CUR ) ;
			goto dar_load_skip2 ;
		}
		strcpy( full, dir ) ;
		strcat( full, name ) ;
		fp = fopen( full, "wb" ) ;
		if ( fp == NULL ) return -1 ;
		ReadOneFile( darfp, fp, size ) ;
		fclose( fp ) ;
		if ( -- remain == 0 ) break ;
		dar_load_skip2 :
        fseek( darfp, 1, SEEK_CUR ) ;
    }
    fclose( darfp ) ;
    return 0 ;
}

/*     	Archive から特定のファイルを削除	*/
int	MDU_DarDeleteArchive( darfile, n_files, files )
char		*darfile ;
int		n_files ;
char		**files ;
{
    if ( InitDarFile() < 0 ) return -1 ;
    if ( ReadToDarFile( darfile ) < 0 ) return -1 ;
    DeleteFromDarFile( n_files, files ) ;
    if ( WriteFromDarFile( darfile ) < 0 ) return -1 ;
    FreeDarFile() ;
    return 0 ;    
}
