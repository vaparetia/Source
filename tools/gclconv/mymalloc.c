/*
	gclconv

		malloc, free関連
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

#define __MYMALLOC_INTERNAL__
#include "mymalloc.h"

void *my_malloc( int size )
{
	void *ptr;
	if( ( ptr = malloc ( size ) ) == NULL ){
		ERROR( "メモリがありません\n" );
	}
	return ptr;
}

void *my_realloc( void *ptr, int size )
{
	if( ( ptr = realloc( ptr, size ) ) == NULL ){
		ERROR( "メモリを拡張できません\n" );
	}
	return ptr;
}

void my_free( void *ptr )
{
	free( ptr );
}


/* ---------------------------------------------------------------------- */
/*
	小さいバッファを確保する
*/

#define MIN_PAGE_SIZE	1024

typedef struct _mlpage {
	struct _mlpage *next;
	int size;
	int now_p;
	char buffer[ 0 ];
} MLPAGE;

struct _mltag {
	struct _mlpage *page;
	int slice_size;
};	// MLTAG

static void *new_page( int size )
{
	MLPAGE *page;

	page = my_malloc( size + sizeof( MLPAGE ) );

	page->next = NULL;
	page->size = size;
	page->now_p = 0;

	return page;
}

MLTAG *tMalloc_Create( int page_size )
{
	MLTAG *tag;

	tag = my_malloc( sizeof( MLTAG ) );

	tag->slice_size = page_size;
	tag->page = new_page( page_size );

	return tag;
}

void *tMalloc( MLTAG *tag, int size )
{
	void *ptr;
	MLPAGE *page;

	size = ( size + 3 ) & ~3;		/* sizeof( int ) にnormalize */

	page = tag->page;

	if( size < page->size - page->now_p ){
		ptr = page->buffer + page->now_p;
		page->now_p += size;
	} else {
		int min;
		MLPAGE *pnew;
		min = ( tag->slice_size < MIN_PAGE_SIZE ) ? tag->slice_size : MIN_PAGE_SIZE;
		if( size > min ){
			// 結構大きいので、独立したページにする。
			// それは、現行のページの後ろにもっていく
			pnew = new_page( size );
			pnew->next = page->next;
			page->next = pnew;
			ptr = page->buffer;
		} else {
			// 新規にページを作って、それを割り当て対象にする。
			pnew = new_page( tag->slice_size );
			pnew->next = page;
			tag->page = pnew;
			return tMalloc( tag, size );
		}
	}
	return ptr;
}

void tMalloc_Destroy( MLTAG *tag )
{
	MLPAGE *page;

	page = tag->page;

	while( page != NULL ){
		MLPAGE *next;

		next = page->next;
		my_free( page );
		page = next;
	}

	my_free( tag );
}

/*
   グローバル
*/

#if 0

#define MALLOC_BUFFER_SIZE	(256*1024)

static MLTAG *global_tag;

void init_Malloc( void )
{
	global_tag = tMalloc_Create( MALLOC_BUFFER_SIZE );
}

void free_Malloc( void )
{
	tMalloc_Destroy( global_tag );
}

void *Malloc( int size )
{
	return tMalloc( global_tag, size );
}


#else // 旧バージョン

static void *alloc_ptr_top;
static void *alloc_ptr;
static int alloc_rest_size;

#define MALLOC_BUFFER_SIZE	(256*1024)

void *Malloc( int size )
{
	void *ptr;

	size = ( size + 3 ) & ~3;		/* sizeof( int ) にnormalize */

	if( alloc_rest_size < size ){
		void *p;
		p = my_malloc( MALLOC_BUFFER_SIZE );
		*( void ** )p = alloc_ptr_top;
		alloc_ptr_top = p;
		p = ( char * )p + sizeof( void ** );
		alloc_rest_size = MALLOC_BUFFER_SIZE - sizeof( void ** );
		alloc_ptr = (char*)alloc_ptr_top + sizeof( void ** );
	}
	alloc_rest_size -= size;
	ptr = alloc_ptr;
	alloc_ptr = ( char * )alloc_ptr + size;

	return ptr;
}

void init_Malloc( void )
{
	alloc_ptr_top = NULL;
	alloc_rest_size = 0;
}

void free_Malloc( void )
{
	void *p, *next;

	for( next = p = alloc_ptr_top; next != NULL; p = next ){
		next = *( void ** )p;
		free( p );
	}
}
#endif
