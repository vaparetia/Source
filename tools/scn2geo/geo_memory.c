/*
   geo_memory.c

   メモリ管理
   
   2002/11/5 T.Morita

   $Id: geo_memory.c,v 1.5 2002/12/06 09:52:08 usr04098 Exp $
   */
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<math.h>
#include	<sys/types.h> 

#include	"scn2geo.h"



#define MAX_CHANK_TEMP_SIZE 1024*1024*10



static void *mem_block_core = NULL ;
static void *mem_block      = NULL ;
static int   mem_size       = 0    ;

/* メモリシステムの起動 */
void  mem_startup( void )
{
    mem_block_core = malloc( MAX_CHANK_TEMP_SIZE ) ;
    if ( mem_block_core == NULL ) {
	printf( "No memory for memsys size%d\n", MAX_CHANK_TEMP_SIZE ) ;
	return ;
    }
    bzero( mem_block_core, MAX_CHANK_TEMP_SIZE ) ;
    mem_block = (void*)((u_int)mem_block_core & 0xffffff00) ;
}

/* メモリシステムの終了 */
void  mem_shutdown( void )
{
return ;
    if ( mem_block_core ){
	free( mem_block_core ) ;
    }
}



/* アラインメント */
static void size_alignment( int align_bit )
{
    int mask = (1 << align_bit) - 1 ;

    mem_size = (mem_size + mask) & ~mask ;
}

void *mem_alloc_align( int size, int align_bit )
{
    void *addr ;

    /* サイズ0のMALLOCは禁止 */
    if ( size == 0 ) {
	printf( "geo_core.c : Do not malloc size 0!!!!\n" ) ;
	return NULL ;
    }

    /* チャンクにメモリが残っているかどうか */
    if ( mem_size + size > MAX_CHANK_TEMP_SIZE ){
	printf( "No chank memory for size%d (%d/%d)\n", size,
		mem_size, MAX_CHANK_TEMP_SIZE ) ;
	return NULL ;
    }

    /* この順序にしないと アラインメント操作が正確に反映されない */
    /* アラインメントを!scnする */
    size_alignment( align_bit ) ;

    /* アドレス計算をする */
    addr = (void*)((u_int)mem_block + mem_size) ;

    /* サイズを更新する */
    mem_size += size ;

    printf( "chank_malloc : %x sz%d c_size%d\n", addr, size, mem_size ) ;
    return addr ;
}

void *mem_alloc( int size )
{
    return mem_alloc_align( size, 4 ) ;
}

int mem_tell_align( int align_bit )
{
    size_alignment( align_bit ) ;

    return mem_size ;
}

int mem_tell()
{
    return mem_tell_align( 4 ) ;
}

void *mem_get_mem_align( int align_bit )
{
    size_alignment( align_bit ) ;

    return (void*)((u_int)mem_block + mem_size) ;
}

void *mem_get_mem()
{
    return mem_get_mem_align( 4 ) ;
}


/*  アラインメント機能を持ったメモリーを確保する */
typedef struct mem_tag_t
{
    void *ptr  ;
    int   size ;
} GEO_MEM ;

void *mem_malloc_align( int size, int align_bit )
{
    int     mask = (1 << align_bit) - 1 ;
    GEO_MEM tag ;
    void   *ptr ;

    /* タグを作る */
    tag.size = (size + sizeof(GEO_MEM) + 2*mask + 1) & ~mask ;
    tag.ptr  = malloc( tag.size ) ;
    if ( tag.ptr == NULL ) {
		printf( "geo_memory.c : no memory for mem_malloc(%d).\n", tag.size ) ;
		return NULL ;
    }
    bzero( tag.ptr, tag.size ) ; /* クリア */

    /* 実際に渡すポインタを設定 */
    ptr = (void *)(((u_int)tag.ptr + sizeof(GEO_MEM) + mask) & ~mask) ;
    *((GEO_MEM *)ptr-1) = tag ;


    /* ここから ちょっと不安なのでチェック用 */
    if ( (u_int)((GEO_MEM *)ptr-1) < (u_int)tag.ptr ){
		printf( "mem_malloc atama ha %x %d\n", tag.ptr, tag.size ) ;
		return NULL ;
    }
    if ( (u_int)ptr+size > (u_int)tag.ptr+tag.size ){
		printf( "mem_malloc ketu dashi %x %d\n", tag.ptr, tag.size ) ;
		return NULL ;
    }

	//printf( "mem_malloc tag_pt%x tag_siz%d ptr%x size%d\n", tag.ptr, tag.size, ptr, size ) ;

    return ptr ;
}

void *mem_malloc( int size )
{
    return mem_malloc_align( size, 4 ) ;
}

void mem_free( void *ptr )
{
    GEO_MEM *tag = ptr ;

	tag-- ;
	//printf( "mem_free tag_pt%x tag_siz%d ptr%x size%d\n", tag->ptr, tag->size, ptr ) ;

    free( tag->ptr ) ;
}


void mem_dump( void *ptr )
{
    GEO_MEM *tag = ptr ;

	if ( ptr == NULL ){
		printf( "mem_dump : ptr = NULL\n" ) ;
	} else {
		tag-- ;		
		printf( "mem_dump : tag_pt%x tag_siz%d ptr%x\n", tag->ptr, tag->size, ptr ) ;
	}
}

