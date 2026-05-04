/*
   MDU_util.h

   データ変換関連ユーティリティヘッダ

   by M.Sonoyama 1999 Aug.～
   $Id: MDU_util.h,v 1.13 2002/04/10 02:21:43 usr04098 Exp $

   Konami Computer Entertainment Japan West   
*/

/*----------------------------------------------------------------*/
#include <sys/types.h>
#ifdef WIN32
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
typedef unsigned int u_int;
#else
#include <unistd.h>
#endif

//#define	MEM_CHECK

#ifdef MEM_CHECK
extern	void	*MDU_Alloc( int ) ;
extern	void	*MDU_Calloc( int, int ) ;
extern	void	*MDU_Realloc( void *, int ) ;
extern	void	MDU_Free( void * ) ;
void		MDU_MemDump( void ) ;
#else

#if 1

#define	MDU_Alloc( s )	malloc( s ) 
#define	MDU_Realloc( a, s )	realloc( a, s )

#else

#define ALLOC_LIMIT	256

static inline void *MDU_Alloc(long s)
{
    if(s<ALLOC_LIMIT) s=ALLOC_LIMIT;
    return malloc(s);
}
static inline void *MDU_Realloc(void *a,long s)
{
    if(s<ALLOC_LIMIT) s=ALLOC_LIMIT;
    return realloc(a,s);
}
#endif

#define	MDU_Calloc( n, s )	calloc( n, s )
#define	MDU_Free( a )		free( a )
#define MDU_MemDump()

#endif

extern	void	MDU_FreadEndianLong( FILE *, long *, int ) ;
extern	void	MDU_FreadEndianShort( FILE *, short *, int ) ;
extern	void	MDU_FwriteEndianLong( FILE *, long *, int, int ) ;
extern	void	MDU_FwriteEndianShort( FILE *, short *, int, int ) ;
#ifdef IS_BIG_ENDIAN
extern	void	MDU_EndianLong( u_long *, int ) ;
extern	void	MDU_EndianShort( u_short *, int ) ;
#else
#define	MDU_EndianLong( ptr, n ) 
#define	MDU_EndianShort( ptr, n ) 
#endif
extern	void	MDU_EndianLongForce( u_long *, int ) ;
extern	void	MDU_EndianShortForce( u_short *, int ) ;

#define	MDU_DAR_MODE_BROKEN	(0)
#define	MDU_DAR_MODE_KEEP	(1)

/*----------------------------------------------------------------*/

extern	int	MDU_FindString( char *, char * ) ;
extern	char	*MDU_GetExtension( char * ) ;
extern	void	MDU_GetRoot( char *, char * ) ;
extern	void	MDU_ChangeExtension( char *, char * ) ;
extern	char	*MDU_TailName( char * ) ;
extern	int	MDU_GetStrCode( char * ) ;
extern	int	MDU_GetFileSize( char * ) ;

extern	int	MDU_GetSentenceElem( FILE *fp, char **elem, int *n_elems ) ;

/*----------------------------------------------------------------*/

extern	void	MDU_ParseOption( int, char ** ) ;
extern	char	**MDU_GetOption( char ) ;
extern	char	*MDU_GetNextValue( void ) ;
extern	int	MDU_GetNextInt( void ) ;

/*----------------------------------------------------------------*/

extern	void	*MDU_AllocVoid( int ) ;
extern	u_long	*MDU_AllocUlong( int ) ;
extern	long	*MDU_AllocLong( int ) ;
extern	u_short	*MDU_AllocUshort( int ) ;
extern	short	*MDU_AllocShort( int ) ;
extern	int	*MDU_AllocInt( int ) ;
extern	u_char *MDU_AllocUchar( int ) ;
extern	signed char	*MDU_AllocChar( int ) ;
extern	void	**MDU_AllocVoid2( int, int ) ;
extern	u_long	**MDU_AllocUlong2( int, int ) ;
extern	long	**MDU_AllocLong2( int, int ) ;
extern	u_short	**MDU_AllocUshort2( int, int ) ;
extern	short	**MDU_AllocShort2( int, int ) ;
extern	int	**MDU_AllocInt2( int, int ) ;
extern	u_char	**MDU_AllocUchar2( int, int ) ;
extern	signed char	**MDU_AllocChar2( int, int ) ;

extern	void		MDU_FreeChar2( char **, int ) ;

/*----------------------------------------------------------------*/

extern	int	MaxInt( int, int ) ;
extern	int	MinInt( int, int ) ;


/*----------------------------------------------------------------*/

int IsTransTexture(char *name);
int IsOverlayTexture(char *name);
int IsEffectTexture(char *name);
