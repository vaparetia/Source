/*
   MDU_util.c
   
   データ変換関連ユーティリティ
   
   by M.Sonoyama 1999 Aug.～
   $Id: MDU_util.c,v 1.14 2000/10/13 06:24:05 usr02011 Exp $
   
   Konami Computer Entertainment Japan West   
   */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>

#include	"MDU_util.h"

#include	"fmt_kms.h"


/*----------------------------------------------------------------*/


/* メモリ管理ユーティリティ */
#ifdef MEM_CHECK
#define	MAX_MEM_BLOCKS	(100000)
static	int	MemoryBlock[ MAX_MEM_BLOCKS ] ;
static	int	MemBlockSize[ MAX_MEM_BLOCKS ] ;
static	int	N_MemBlocks = 0 ;
static	int	N_Allocs = 0 ;
static	int	N_Frees = 0 ;
static	int	N_FreeFalses = 0 ;

void	MDU_InitMemorySystem( void )
{
	N_MemBlocks = 0 ;
	N_Allocs = 0 ;
	N_Frees = 0 ;
	N_FreeFalses = 0 ;
}

void		*MDU_Alloc( size )
int		size ;
{
    void	*buf ;

    if ( N_MemBlocks == MAX_MEM_BLOCKS ) {
		printf( "memory block full\n" ) ;
		return NULL ;
    }
    buf = malloc( size ) ;
    if ( buf == NULL ) {
		printf( "cannot alloc memory\n" ) ;
		return NULL ;
    }
    MemoryBlock[ N_MemBlocks ] = ( u_int )buf ;
    MemBlockSize[ N_MemBlocks ] = size ;
    N_MemBlocks ++ ;
    N_Allocs ++ ;
    return buf ;
}

void		*MDU_Calloc( n, size )
int		n, size ;
{
    void	*buf ;

    if ( N_MemBlocks == MAX_MEM_BLOCKS ) {
		printf( "memory block full\n" ) ;
		return NULL ;
    }
    buf = calloc( n, size ) ;
    if ( buf == NULL ) {
		printf( "cannot alloc memory\n" ) ;
		return NULL ;
    }
    MemoryBlock[ N_MemBlocks ] = ( u_int )buf ;
    N_MemBlocks ++ ;
    N_Allocs ++ ;
    return buf ;
}

void		*MDU_Realloc( addr, size )
void		*addr ;
int		size ;
{
    int		i, loc, c, min ;
    void	*buf ;

    buf = MDU_Alloc( size ) ;
    if ( buf == NULL ) {
		printf( "cannot reallocate memory\n" ) ;
		return NULL ;
    }

    loc = ( int )addr ;
    c = 0 ;
    for ( i = 0; i < N_MemBlocks; i ++ ) {
		if ( MemoryBlock[ i ] == loc ) {
			c = 1 ;
			if ( size > MemBlockSize[ i ] ) min = MemBlockSize[ i ] ;
			else			    min = size ;
			memcpy( buf, addr, min ) ;
			free( addr ) ;
			break ;
		}
    }    
    MemoryBlock[ i ] = buf ;
    MemBlockSize[ i ] = size ;
    return buf ;
}

void		MDU_Free( addr )
void		*addr ;
{
    int		loc, i, c ;

    if ( N_MemBlocks == 0 ) return ;

    loc = ( u_int )addr ;
    c = 0 ;
    for ( i = 0; i < N_MemBlocks; i ++ ) {
		if ( c == 0 && MemoryBlock[ i ] == loc ) {
			free( addr ) ;
			c = 1 ;
			continue ;
		} else if ( c ) {
			MemoryBlock[ i - 1 ] = MemoryBlock[ i ] ;
			MemBlockSize[ i - 1 ] = MemBlockSize[ i ] ;
		}
    }
    if ( c ) {
		N_MemBlocks -- ;
    } else {
		N_FreeFalses ++ ;
		printf("Free Failed\n");
		exit(0);
    }
    N_Frees ++ ;
}

void		MDU_MemDump( void ) 
{
    printf( "%d mem blocks used\n", N_MemBlocks ) ;
    printf( "%d allocs \n", N_Allocs ) ;
    printf( "%d frees \n", N_Frees ) ;
    printf( "%d free falses\n", N_FreeFalses ) ;
}

void		MDU_MemDebugClear( void )
{
    N_Allocs = 0 ;
    N_Frees = 0 ;
    N_FreeFalses = 0 ;
}

int		MDU_InsideMemBlock( addr, ptr )
void		*addr ;
void		*ptr ;
{
    int		i, p, ad ;

    p = ( int )ptr ;
    ad = ( int )addr ;
    for ( i = 0; i < N_MemBlocks; i ++ ) {
		if ( ad == MemoryBlock[ i ] ) {
			if ( p >= ad && p < ad + MemBlockSize[ i ] ) return 1 ;
			return 0 ;
		}
    }
    return -1 ;
}
#else

#endif

/*----------------------------------------------------------------*/

/* エンディアンユーティリティ */
#ifdef IS_BIG_ENDIAN
void		MDU_EndianLong( ptr, n )
u_long		*ptr ;
int		n ;
{
    u_long	val, byte0, byte1, byte2, byte3 ;

    if ( ptr == NULL ) return ;
    while ( -- n >= 0 ) {
		val = *ptr ;
		byte0 = ( 0x000000ff & val ) << 24 ;
		byte1 = ( 0x0000ff00 & val ) << 8 ;
		byte2 = ( 0x00ff0000 & val ) >> 8 ;
		byte3 = ( 0xff000000 & val ) >> 24 ;
		*( ptr ++ ) = byte0 | byte1 | byte2 | byte3 ;
    }
}

void		MDU_EndianShort( ptr, n )
u_short		*ptr ;
int		n ;
{
    u_short		val, byte0, byte1 ;

    if ( ptr == NULL ) return ;
    while ( -- n >= 0 ) {
		val = *ptr ;
		byte0 = ( 0x00ff & val ) << 8 ;
		byte1 = ( 0xff00 & val ) >> 8 ;
		*( ptr ++ ) = byte0 | byte1 ;
    }
}

#endif /* IS_BIG_ENDIAN */

void		MDU_EndianLongForce( ptr, n )
u_long		*ptr ;
int		n ;
{
    u_long	val, byte0, byte1, byte2, byte3 ;

    if ( ptr == NULL ) return ;
    while ( -- n >= 0 ) {
		val = *ptr ;
		byte0 = ( 0x000000ff & val ) << 24 ;
		byte1 = ( 0x0000ff00 & val ) << 8 ;
		byte2 = ( 0x00ff0000 & val ) >> 8 ;
		byte3 = ( 0xff000000 & val ) >> 24 ;
		*( ptr ++ ) = byte0 | byte1 | byte2 | byte3 ;
    }
}

void		MDU_EndianShortForce( ptr, n )
u_short		*ptr ;
int		n ;
{
    u_short		val, byte0, byte1 ;

    if ( ptr == NULL ) return ;
    while ( -- n >= 0 ) {
		val = *ptr ;
		byte0 = ( 0x00ff & val ) << 8 ;
		byte1 = ( 0xff00 & val ) >> 8 ;
		*( ptr ++ ) = byte0 | byte1 ;
    }
}

/*	ファイルから読み込んでエンディアン変換( long )	*/
void 	MDU_FreadEndianLong( fp, buf_l, num )
FILE		*fp ;
long		*buf_l ;
int		num ;
{
    fread( buf_l, sizeof( long ), num, fp ) ;
    MDU_EndianLong( buf_l, num ) ;
}

/*	ファイルから読み込んでエンディアン変換( short )	*/
void	MDU_FreadEndianShort( fp, buf_s, num )
FILE		*fp ;
short		*buf_s ;
int		num ;
{
    fread( buf_s, sizeof( short ), num, fp ) ;
    MDU_EndianShort( buf_s, num ) ;
}

/*	エンディアン変換してファイル書き込み( long )	*/
void 	MDU_FwriteEndianLong( fp, buf_l, num, mode )
FILE		*fp ;
long		*buf_l ;
int		num ;
int		mode ;
{
    long	*ltmp ;

    if ( mode == MDU_DAR_MODE_KEEP ) {
		ltmp = ( long * )MDU_Alloc( sizeof( long ) * num ) ;
		memcpy( ltmp, buf_l, sizeof( long ) * num ) ;
    } else {
		ltmp = buf_l ;
    }

    MDU_EndianLong( ltmp, num ) ;
    fwrite( ltmp, sizeof( long ), num, fp ) ;
    if ( mode == 1 ) MDU_Free( ltmp ) ;
}

/*	エンディアン変換してファイル書き込み( short )	*/
void	MDU_FwriteEndianShort( fp, buf_s, num, mode )
FILE		*fp ;
short		*buf_s ;
int		num ;
int		mode ;
{
    short	*stmp ;
    
    if ( mode == MDU_DAR_MODE_KEEP ) {
		stmp = ( short * )MDU_Alloc( sizeof( short ) * num ) ;
		memcpy( stmp, buf_s, sizeof( short ) * num ) ;
    } else {
		stmp = buf_s ;
    }

    MDU_EndianShort( stmp, num ) ;
    fwrite( stmp, sizeof( short ), num, fp ) ;
    if ( mode == 1 ) MDU_Free( stmp ) ;
}

/*----------------------------------------------------------------*/

/* ファイル名操作ユーティリティ */

/* ファイル名に指定文字列が含まれるかチェック */
int		MDU_FindString( str, chk )
char		*str, *chk ;
{
    char       	*cp ;
    int		len, len2, n ;

    cp = str ; n = 0 ;
    len = strlen( chk ) ;
    len2 = strlen( str ) ;
    if ( len > len2 ) return -1 ;
    while( *cp != '\0' ) {
		if ( len2 < len ) return -1 ;
		if ( strncmp( cp, chk, len ) == 0 ) return n ;
		n ++ ; cp ++ ; len2 -- ;
    }
    return -1 ;
}

/* ファイル名の拡張子を取得 */
char		*MDU_GetExtension( name )
char		*name ;
{
    char	c, *cp ;

    cp = name ;
    while ( ( c = *( cp ++ ) ) != '\0' ) {
		if ( c == '.' ) name = cp ;
    }
    return name ;
}

/* ファイル名の拡張子を除いた部分を取得 */
void		MDU_GetRoot( name, root )
char		*name, *root ;
{
    char	c, *cp ;
    char	*r ;

    cp = name ;
    r = root ;
    while ( ( c = *( cp ++ ) ) != '\0' ) {
		if ( c == '.' ) break ;
		*( r ++ ) = c ;
    }
    *r = '\0' ;
}

/* 拡張子を変える */
void		MDU_ChangeExtension( name, ext )
char		*name, *ext ;
{
    char	c, *cp ;
#if 0
    cp = name ;
    while ( ( c = *( cp ++ ) ) != '\0' ) {
		if ( c == '.' ) break ;
    }
#endif
    cp = strrchr( name, '.' ) ;
    if ( cp == NULL ) return ;
    cp ++ ;
    strcpy( cp, ext ) ;
}

/* ファイル名からディレクトリを除いた部分を取得 */
char		*MDU_TailName( filename )
char		*filename ;
{
    char	*tail ;
    char	c, *cp ;
	
    tail = cp = filename ;
    while ( ( c = *( cp ++ ) ) != '\0' ) {
		if ( c == '/' ) tail = cp ;
    }
    return tail ;
}

/* 文字列コードを取得 */
int	MDU_GetStrCodeOld( str )
char		*str ;
{
    u_char		c ;
    u_short		id ;
    char *p;

	//    p = str;
    p = MDU_TailName( str ) ;
    id = 0 ;
    while ( ( c = (u_char)*( p ++ ) ) != '\0' ) {
		if ( c == '.' ) break ;
		id = ( id << 5 ) | ( id >> 11 ) ;
		id += c ;
    }
    return id ;
}

#define BIT_LEN		24
int 	MDU_GetStrCodeNew( char *string )
{
    unsigned char c ;
    unsigned char *p ;
    unsigned int id, mask ;

	//    p = ( unsigned char * )string ;
    p = MDU_TailName( string ) ;
    id = 0 ;
    mask = ( 1 << BIT_LEN ) - 1 ;

    while( ( c = *( p++ ) ) != '\0' ) {
		if ( c == '.' ) break ;
		id = ( id << 5 ) | ( id >> ( BIT_LEN - 5 ) ) ;
		id += c ;
		id &= mask ;
    }
    if( id == 0 ) id = 1 ;		/* 0 になってしまったら１にする */
    return id;
}

#define BIT_LEN		24
int 	MDU_GetStrCode( char *string )
{
    unsigned char c ;
    unsigned char *p ;
    unsigned int id, mask ;

    /* しばらく旧バージョンで */
	//    return MDU_GetStrCodeOld( string ) ;

	//    p = ( unsigned char * )string ;
    p = MDU_TailName( string ) ;
    id = 0 ;
    mask = ( 1 << BIT_LEN ) - 1 ;

    while( ( c = *( p++ ) ) != '\0' ) {
		if ( c == '.' ) break ;
		id = ( id << 5 ) | ( id >> ( BIT_LEN - 5 ) ) ;
		id += c ;
		id &= mask ;
    }
    if( id == 0 ) id = 1 ;		/* 0 になってしまったら１にする */
    return id;
}

/* ファイルサイズを取得 */
int	MDU_GetFileSize( name )
char	*name ;
{
    int		size ;
    FILE	*fp ;
    
    size = 0 ;
    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) return -1 ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) ;
    fclose( fp ) ;
    return size ;
}

int		   MDU_GetSentenceElem( FILE *fp, char **elem, int *n_elems )
{
	char		buf[ 256 ] ;
	int			n, i ;
	char		*ptr ;

	if ( fgets( buf, 256, fp ) == NULL ) return -1 ;

	n = 0 ;
	ptr = buf ;
	while( 1 ) {
		if ( ptr[ 0 ] == '\n' || ptr[ 0 ] == '\0' ||
			n >= 16 ) break ;
		for ( i = 0; *ptr != ' ' && *ptr != '\t' && *ptr != '\n' &&
			  *ptr != '\0'; i ++ ) {
			elem[ n ][ i ] = *ptr ;
			ptr ++ ;
		}
		if ( i != 0 ) {
			elem[ n ][ i ] = '\0' ;	
			n ++ ;
		} else {
			ptr ++ ;
		}
	}
	*n_elems = n ;
	return 0 ;
}

/*----------------------------------------------------------------*/

/* オプション解釈ユーティリティ */

static	char	**Argv ;
static	char	**ArgPtr ;
static	int	Argc, Argn ;

/* 文字列をパーサーにかけることを明示 */
void	MDU_ParseOption( argc, argv )
int	argc ;
char	**argv ;
{
    Argc = argc ;
    Argn = 0 ;
    ArgPtr = Argv = argv ;
}

/* オプション開始ポインタを返す */
char	**MDU_GetOption( opt ) 
char	opt ;
{
    int		i ;
    char	**ptr, *p ;

    ptr = Argv ;
    Argn = 0 ;
    for ( i = 0; i < Argc; i ++ ) {
		p = *ptr ;
		if ( p[ 0 ] == '-' ) {
			if ( p[ 1 ] == opt ) {
				ArgPtr = ptr ;
				Argn = i ;
				return ptr ;
			}
		} 
		ptr ++ ;
    }
    return NULL ;
}

/* 現在のポインタの値を取得 */
char	*MDU_GetNextValue( void )
{
    char	*ptr ;

    ArgPtr ++ ;
    Argn ++ ;
    ptr = *ArgPtr ;
    if ( Argn == Argc || 
		( ptr[ 0 ] == '-' && strlen( ptr ) != 1 ) ) return NULL ;
    return ptr ;
}

/* 現在のポインタの値を整数にして取得 */
int	MDU_GetNextInt( void )
{
    char	*ptr ;

    ptr = MDU_GetNextValue() ;
    if ( ptr == NULL ) return -1 ;
    return atoi( ptr ) ;
}

/*----------------------------------------------------------------*/

/* テクスチャが半透明がどうか */
int IsTransTexture(char *name)
{
#if 1
    if(MDU_FindString( name, "_add" ) >= 0 ||
       MDU_FindString( name, "_sub" ) >= 0 ||
       MDU_FindString( name, "_hlf" ) >= 0 ||
       MDU_FindString( name, "_alp" ) >= 0 ||
       MDU_FindString( name, "_bld" ) >= 0 ) {

		return 1 ;
    }
#else
    if ( !strcmp( name, "rai_hair_fro1msk" ) ||
		!strcmp( name, "rai_hair_fro2msk" ) ) return 1 ;
#endif
    return 0 ;
}

/* テクスチャがオーバーレイかどうか */
int IsOverlayTexture(char *name)
{
    if ( MDU_FindString( name, "_ovl" ) >= 0 ) return 1;
    return 0;
}

/* テクスチャがエフェクト用かどうか */
int IsEffectTexture(char *name)
{
    if(MDU_FindString(name,"_emap")>= 0) return DG_PACKFLAG_EMAP;
    if(MDU_FindString(name,"_smap")>= 0) return DG_PACKFLAG_SMAP;
    if(MDU_FindString(name,"_bmap")>= 0) return DG_PACKFLAG_BMAP;
    return 0 ;
}

/*----------------------------------------------------------------*/


/* おまけ */

/* 最大値 */
int	MaxInt( a, b )
int	a, b ;
{
    return ( a > b ) ? a : b ;
}

/* 最小値 */
int	MinInt( a, b )
int	a, b ;
{
    return ( a < b ) ? a : b ;
}

/* 領域確保関数 */

void	*MDU_AllocVoid( int num )
{
    return ( void * )MDU_Alloc( num ) ;
}

u_long	*MDU_AllocUlong( int num )
{
    u_long	*tmp ;

    tmp = ( u_long * )calloc( num, sizeof( u_long ) ) ;
    return tmp ;
}

long	*MDU_AllocLong( int num )
{
    long	*tmp ;

    tmp = ( long * )calloc( num, sizeof( long ) ) ;
    return tmp ;
}

u_short	*MDU_AllocUshort( int num )
{
    u_short	*tmp ;

    tmp = ( u_short * )MDU_Alloc( sizeof( u_short ) * num ) ;
    return tmp ;
}

short	*MDU_AllocShort( int num )
{
    short	*tmp ;

    tmp = ( short * )MDU_Alloc( sizeof( short ) * num ) ;
    return tmp ;
}

int	*MDU_AllocInt( int num ) 
{
    int		*tmp ;

    tmp = ( int * )MDU_Alloc( sizeof( int ) * num ) ;
    return tmp ;
}

u_char *MDU_AllocUchar( int num )
{
    u_char	*tmp ;

    tmp = ( u_char * )MDU_Alloc( num ) ;
    return tmp ;
}

signed char	*MDU_AllocChar( int num )
{
    signed char *tmp ;

    tmp = ( signed char * )MDU_Alloc( num ) ;
    return tmp ;
}

/*	2次	*/
void	**MDU_AllocVoid2( m, n )
int	m, n ;
{
    void	**tmp ;
    int		i ;

    tmp = ( void ** )MDU_Alloc( sizeof( void * ) * m ) ;
    for ( i = 0; i < m; i ++ ) tmp[ i ] = MDU_AllocVoid( n ) ;
    return tmp ;
}

u_long	**MDU_AllocUlong2( m, n )
int	m, n ;
{
    u_long	**tmp ;
    int		i ;
    
    tmp = ( u_long ** )MDU_Alloc( sizeof( u_long * ) * m ) ;
    for ( i = 0; i < m; i ++ ) {
		tmp[ i ] = MDU_AllocUlong( n ) ;
    }
    return tmp ;
}

long	**MDU_AllocLong2( m, n )
int	m, n ;
{
    long	**tmp ;
    int		i ;
    
    tmp = ( long ** )MDU_Alloc( sizeof( long * ) * m ) ;
    for ( i = 0; i < m; i ++ ) {
		tmp[ i ] = MDU_AllocLong( n ) ;
    }
    return tmp ;
}

u_short	**MDU_AllocUshort2( m, n )
int	m, n ;
{
    u_short	**tmp ;
    int		i ;

    tmp = ( u_short ** )MDU_Alloc( sizeof( u_short * ) * m ) ;
    for ( i = 0; i < m; i ++ ) {
		tmp[ i ] = MDU_AllocUshort( n ) ;
    }
    return tmp ;
}

short	**MDU_AllocShort2( m, n )
int	m, n ;
{
    short	**tmp ;
    int		i ;

    tmp = ( short ** )MDU_Alloc( sizeof( short * ) * m ) ;
    for ( i = 0; i < m; i ++ ) {
		tmp[ i ] = MDU_AllocShort( n ) ;
    }
    return tmp ;
}		

int	**MDU_AllocInt2( m, n )
int	m, n ;
{
    int		**tmp ;
    int		i ;

    tmp = ( int ** )MDU_Alloc( sizeof( int * ) * m ) ;
    for ( i = 0; i < m; i ++ ) tmp[ i ] = MDU_AllocInt( n ) ;
    return tmp ;
}

u_char	**MDU_AllocUchar2( m, n )
int	m, n ;
{
    u_char	**tmp ;
    int		i ;

    tmp = ( u_char ** )MDU_Alloc( sizeof( u_char * ) * m ) ;
    for ( i = 0; i < m; i ++ ) tmp[ i ] = MDU_AllocUchar( n ) ;
    return tmp ;
}		

signed char	**MDU_AllocChar2( m, n )
int		m, n ;
{
    signed char	**tmp ;
    int		i ;

    tmp = ( signed char ** )MDU_Alloc( sizeof( signed char * ) * m ) ;
    for ( i = 0; i < m; i ++ ) tmp[ i ] = MDU_AllocChar( n ) ;
    return tmp ;
}

void	MDU_FreeChar2( char **p, int m )
{
	int			i ;

	for ( i = 0; i < m; i ++ ) {
		if ( p[ i ] != NULL ) MDU_Free( p[ i ] ) ;
	}
	MDU_Free( p ) ;
}
