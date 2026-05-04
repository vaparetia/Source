//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cache.c
		ロードデータ登録と、初期化ルーチンの呼び出し。

	1999/05/31 K.Uehara
	$Id: cache.c,v 1.1.1.3 2002/11/19 11:42:43 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<string.h>

#include	"libgv.h"
#include	"libgv.cnf"

#define SET_SPEC( _id, _spec )	( (_id) | ( (_spec) << 24 ) )
#define GET_SPEC( _id )			( (_id & ~GV_CACHEID_RESIDENT) >> 24 )

#define NEVER_USED	(0)

/*
	ロード初期化ルーチン
*/

typedef struct
{
   int loaderCallResident;
   GV_LOADFUNC func;
} BP_SLoaderFunction;
static BP_SLoaderFunction Loaders[ MAX_LOADERS ];

/*
	データキャッシュリスト
*/

typedef struct {
	int id;
	void *buf;
   int swapped;
} CACHE;

static CACHE Caches[ MAX_CACHES ];
static CACHE *EmptyCache;

/* --------------------------------------------------------- */

/*
	キャッシュの検索
		すでに存在していれば、そのポインタ
		無い場合にはNULLを返し,いれるべき場所をEmptyCacheにセットする
*/

static CACHE *FindCache( int id )
{
	CACHE *cp, *empty;
	int mod, this;
	int i, j;

	empty = NULL;
	mod = id % MAX_CACHES;
	cp = Caches + mod;
	j = MAX_CACHES - mod;

	for( i = MAX_CACHES; i > 0; --i ){
		this = ( 0x1FFFFFFF & cp->id );
		if( this == NEVER_USED ){
			if( empty == NULL ) empty = cp;
			break;
		}
		if( this == id ){
			return cp;
		}
		cp ++;
		if( --j == 0 ) cp = Caches;
	}

#ifdef DEBUG_MODE
	if ( i == 0 ) {
	    printf( "CACHE FULL( Too many DATA ) !!!!!!\n" ) ;
	    ASSERT( 0 ) ;
	}
#endif
	EmptyCache = empty;
	return NULL;
}

/* --------------------------------------------------------- */

int GV_CacheID( int root_id, int spec )
{
	spec -= 'a';
	ASSERT( spec >= 0 && spec < 26 );
	return SET_SPEC( root_id, spec );
}

int GV_CacheID2( char *root_name, int spec )
{
	return GV_CacheID( GV_StrCode( root_name ), spec );
}

int GV_CacheID3( char *name )
{
	char root[ 48 ];
#if 1
	char *p;
	strcpy( root, name );
	p = strrchr( root, '.' );
	if( p == NULL ){
		return GV_CacheID2( root, 0 );
	} else {
		*p = '\0';
		return GV_CacheID2( root, p[ 1 ] );
	}
#else
	char c, *cp, *cp2;

	cp = name;
	cp2 = root;
	do {
		if( ( c = *( cp ++ ) ) == '.' ){
			*cp2 = '\0';
			break;
		}
		*( cp2 ++ ) = c;
	} while( c != '\0' );

	return GV_CacheID2( root, cp[ 0 ] );
#endif
}

/* --------------------------------------------------------- */

void *GV_GetCache( int id )
{
	CACHE *cp;

	if( ( cp = FindCache( id ) ) != NULL ) return cp->buf;
	printf( "GetCache:not found %d.%c\n", id & 0x00FFFFFF, 'a' + ( ( unsigned int )id >> 24 ) );
	return NULL;
}

void *GV_GetCacheMarkSwapped( int id, int *pWasSwapped )
{
   CACHE *cp;

   if( ( cp = FindCache( id ) ) != NULL ) 
   {
      *pWasSwapped = cp->swapped;
      cp->swapped = 1;
      return cp->buf;
   }
   printf( "GetCacheMarkSwapped:not found %d.%c\n", id & 0x00FFFFFF, 'a' + ( ( unsigned int )id >> 24 ) );
   *pWasSwapped = 0;
   return NULL;
}

CACHE *GV_SetCache( int id, void *buf )
{
	CACHE *cp;

	if( ( cp = FindCache( id ) ) == NULL ){
		if( ( cp = EmptyCache ) == NULL ){
			return NULL;
		}
	} else {
		if( !( cp->id & GV_CACHEID_RESIDENT ) ){
			return NULL;
		}
		printf( "%d:OVERWRITE RESIDENT DATA\n", id );
	}
	cp->id = id;
	cp->buf = buf;
   cp->swapped = 0;

	return cp;
}

void GV_SetLoader( int spec, GV_LOADFUNC init )
{
   int type;

   if( spec & LOADER_CALL_RESIDENT )
   {
	   spec &= ~LOADER_CALL_RESIDENT;
      type = spec - 'a';
      ASSERT( type >= 0 && type < MAX_LOADERS );
      Loaders[ type ].func = init;
      Loaders[ type ].loaderCallResident = 1;
	}
   else
   {
      type = spec - 'a';
      ASSERT( type >= 0 && type < MAX_LOADERS );
      Loaders[ type ].func = init;
      Loaders[ type ].loaderCallResident = 0;
   }
}

/* キャッシュのすげかえをする */
/* ちゃんと用が済んだら戻すこと！ */
void	GV_ExchangeCache( int id1, int id2 )
{
	CACHE	*c1, *c2 ;
	void	*buftmp ;
   int swaptmp;

	c1 = FindCache( id1 ) ;
	c2 = FindCache( id2 ) ;
	if ( c1 == NULL || c2 == NULL ) return ;
	buftmp = c1->buf ;
	c1->buf = c2->buf ;
	c2->buf = buftmp ;

   swaptmp = c1->swapped;
   c1->swapped = c2->swapped;
   c2->swapped = swaptmp;
}

/* --------------------------------------------------------- */

void GV_ResetLoader( void )
{
	int i;

	for( i = 0; i < MAX_LOADERS; ++ i )
   {
      Loaders[i].func = NULL;
      Loaders[i].loaderCallResident = 0;
	}
}

void GV_InitCacheSystem( void )
{
	CACHE *cp;
	int i;

	/* キャッシュリストのクリア */

	cp = Caches;
	for( i = MAX_CACHES; i > 0; --i ){
		cp->id = NEVER_USED;
		cp++;
	}
}

void GV_FreeCacheSystem( void )
{
	CACHE *cp;
	int i;

	/* キャッシュリストのクリア */

	cp = Caches;
	for( i = MAX_CACHES; i > 0; --i ){
		cp->id = NEVER_USED;
		cp++;
	}
}

/* --------------------------------------------------------- */

int GV_LoadInit( void *data, int name, int cache_mode )
	 // cache_mode : 0 no cache
	 // cache_mode : 1 cache
	 // cache_mode : 2 resident init
	 // cache_mode : 3 resident re-initialize
{
	BP_SLoaderFunction init;
	CACHE *cp = NULL;
	int id, code;

	id = name;

	if( cache_mode != 0 && ( GET_SPEC( id ) != 't' - 'a'
							&& GET_SPEC( id ) != 'x' - 'a' ) ){
		// キャッシュ登録
#if 0
		if( FindCache( id ) != NULL || ( cp = EmptyCache ) == NULL ){
			printf( "ID CONFLICT %X\n", id );
			return -1;
		}
		cp->id = id;
		cp->buf = data;
      cp->swapped = 0;
#else
		if( ( cp = GV_SetCache( id, data ) ) == NULL ){
			printf( "ID CONFLICT %X\n", id );
			return -1;
		}
#endif
	}
	if( GET_SPEC( id ) > MAX_LOADERS ){
		return 1;
	}

	switch( cache_mode ){
	  case GV_INIT_NOCACHE:
		// 初期化
      init = Loaders[ GET_SPEC( id ) ];
		if( init.func != NULL ){
			if( ( code = ( *init.func )( data, id, cache_mode ) ) <= 0 ) return code;
		}
		return 1;
	  case GV_INIT_CACHE:
	  case GV_INIT_RESIDENT:
      init = Loaders[ GET_SPEC( id ) ];
		if( init.func != NULL ){
			if( ( code = ( *init.func )( data, id, cache_mode ) ) <= 0 ){
				if( cp != NULL ){
					cp->id = NEVER_USED;
				}
				return code;
			}
		}
		if( cp != NULL && cache_mode == GV_INIT_RESIDENT ){
			cp->id |= GV_CACHEID_RESIDENT;
		}
		break;
	  case GV_INIT_RESIDENT_AGAIN:
		if( cp != NULL ){
			cp->id |= GV_CACHEID_RESIDENT;
		}
      init = Loaders[ GET_SPEC( id ) ];
		if( init.func != NULL ){
			if( init.loaderCallResident ){
				if( ( code = ( *init.func )( data, id | GV_CACHEID_RESIDENT, cache_mode ) ) <= 0 ){
					if( cp != NULL ){
						cp->id = NEVER_USED;
					}
					return code;
				}
			}
		}
		break;
	}
	return 1;
}
