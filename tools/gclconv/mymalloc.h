/*
	mymalloc.h
*/

#ifndef __MYMALLOC_H__

// NULLを返す代わりにエラー終了するmalloc
void *my_malloc( int size );
void *my_realloc( void *ptr, int size );
void my_free( void *ptr );

// 細かいブロックをまとめて管理する。個別にFreeできない。

typedef struct _mltag MLTAG;

void *tMalloc( MLTAG *tag, int size );

MLTAG *tMalloc_Create( int page_size );
void tMalloc_Destroy( MLTAG *tag );

// ユーティリティ
void *Malloc( int size );
void init_Malloc( void );
void free_Malloc( void );

#ifndef __MYMALLOC_INTERNAL__
#define malloc my_malloc
#define free my_free
#endif

#endif // __MYMALLOC_H__

