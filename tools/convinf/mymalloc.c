/*
	gclconv

		malloc, free関連
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ERROR( char *str )
{
	printf( "%s", str );
	exit( 1 );
}

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
		alloc_ptr = alloc_ptr_top + sizeof( void ** );
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
