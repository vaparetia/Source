//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	サブメモリブロック管理ライブラリ
		空きリストのみしか管理しない
		ヘッダサイズを消費しない
		解放にはそのブロック取得時のサイズが必要

	2000/07/04 K.Uehara
	$Id: memlist.c,v 1.1.1.3 2002/11/19 11:42:44 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"libgv.h"
#include	"memlist.h"

#ifdef	PSX2
typedef char byte;
#endif

#define MINIMUM_SIZE	16

void GV_MlBufferInit( MEMLIST *list, char *name, void *top, void *bottom, int align )
{
	list->name = name;
	list->top = top;
	list->now_bottom = list->bottom = bottom;
	list->empty.next = NULL;
	list->empty.size = 0;
	list->align = align;

	/*
		バッファ全体をFREEブロックに
	*/
	{
		MEMTAG *t;
		t = ( MEMTAG * )top;

		t->next = NULL;
		t->size = (byte*)bottom - (byte*)top;
		
		list->now_bottom = top;
		list->empty.next = top;
		list->empty.size = 0;
	}
}

void *GV_MlMalloc( MEMLIST *list, int size )
{
	MEMTAG *t, *prev;
	int align, minsize;
	void *p;

	/* sizeは、alignで正規化 */
	align = list->align;
	size = ( ( size + align - 1 ) / align ) * align;
	minsize = ( align < MINIMUM_SIZE ) ? MINIMUM_SIZE: align;

	for( prev = t = &list->empty; t != NULL; t = t->next ){
		if( t->size == size || ( unsigned int )( t->size - size ) < (u_int)minsize ){
			/* ちょうどいいサイズのブロックがあった */
			size = t->size;
			prev->next = t->next;
			*( int * )t = size;
			return t;
		}
		if( ( int )( t->size - minsize ) > size ){
			/* 空ブロックの後ろから必要な大きさだけとる */
			p = ( char * )t + t->size - size;
			t->size -= size;
			*( int * )p = size;
			return p;
		}
		prev = t;
	}

#if 0
	/* 空きリストの中に充分な大きさのブロックがなかったので、ヒープを拡張する */
	p = ( char * )list->now_bottom - size;
	list->now_bottom = ( void * )p;
	if( p < list->top ){
		printf( "MlMalloc:NO MEMORY %d\n", size );
		return NULL;
	}
	*( int * )p = size;

	return p;
#else
	printf( "MlMalloc:NO MEMORY %d\n", size );
	return NULL;
#endif
}

void GV_MlFree( MEMLIST *list, void *ptr, int size )
{
	MEMTAG *t, *prev, *bfp;

	bfp = &list->empty;

	for( prev = t = &list->empty; t != NULL; t = t->next ){
		if( ( char * )ptr + size == ( char * )t ){
			/* tとptrは隣接しているので、tのブロックにptrを含める */
			MEMTAG *ft;
			ft = ( MEMTAG * )ptr;
			ft->next = t->next;
			ft->size = t->size + size;
			prev->next = ft;

			/* その次の空きブロックも隣接していたら統合 */
			if( ft->next != NULL && ( char * )ft->next + ft->next->size == ( char * )ptr ){
				ft->next->size += ft->size;
				prev->next = ft->next;
			}
			return;
		}
		if( ( char * )t + t->size == ( char * )ptr ){
			/* 空き領域とptrの領域が隣接している */
			t->size += size;
			return;
		}
#ifdef DEBUG_MODE
		/* 整合性チェック */
		{
			char *fend;
			fend = ( char * )ptr + size;
			if( ( char * )t < ( char * )ptr && ( char * )t + t->size > ( char * )ptr ){
				printf( "MlFree:Wrong PTR\n" );
				HANGUP();
				return;
			}
			if( ( char * )t < ( char * )fend && ( char * )t + t->size > ( char * )fend ){
				printf( "MlFree:Wrong SIZE\n" );
				HANGUP();
				return;
			}
		}
#endif // DEBUG_MODE
		if( ( char * )ptr < ( char * )t ){
			bfp = prev;
		}
		prev = t;
	}
	{
		/* フリーブロックの真中の解放 */
		MEMTAG *f;

		f = ptr;

		for( t = &list->empty; t != NULL; t = t->next ){
			if( t->next < f ){
				bfp = t;
				break;
			}
		}
		f->size = size;
		f->next = bfp->next;
		bfp->next = f;
	}
}


#ifdef DEBUG_MODE
void GV_MlPrintDebug( MEMLIST *list )
{
	MEMTAG *p;
	void *now;

	now = list->bottom;
	p = list->empty.next;

	printf( "ML:--ML %s [%08X-%08X]--\n", list->name, list->top, list->bottom );
	for( ; p != NULL; p = p->next ){
		if( p->size <= 0 || ( p->next != NULL && ( ( void * )p->next > list->bottom
												   || ( void * )p->next < list->top ) ) ){
			printf( "ML:MEMLIST ERROR\n" );
			HANGUP();
		}
		if( ( byte* )p + p->size < ( byte* )now ){
			void *top;
			int size;
			top = (( byte * )p + p->size);
			size = ( byte* )now - ( byte* )p - p->size;
			printf( "used %08X-%08X size %d\n", top, ( byte* )top + size - 1, size );
		}
		printf( "ML:FREE %08X-%08X size %d\n", p, ( byte* )p + p->size - 1, p->size );
		now = p;
	}
	if( now > list->now_bottom ){
		void *top;
		int size;
		top = list->now_bottom;
		size = ( char * )now - ( char * )top;
		printf( "ML:USED %08X-%08X size %d\n", top, ( byte* )top + size - 1, size );
	}
	printf( "ML:TOP  %08X USED %d FREE %d\n", list->now_bottom, ( byte* )list->bottom - ( byte* )list->now_bottom
			, ( byte* )list->now_bottom - ( byte* )list->top );
}

#endif // DEBUG_MODE

