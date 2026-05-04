/*
	汎用ハッシュライブラリ
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "mymalloc.h"

//#define SHOW_PERF		// profileを行う

/*
   my_mallocはNULLならErrorを出すタイプのmalloc
*/

#define HASH_TAG_BUFFER_EXPAND_SIZE		1024

typedef struct _hash_tag {
	struct _hash_tag *hi;
	struct _hash_tag *low;
	int id;
	void *key;
	HASH_ENTRY ent;
} HASH_TAG;

typedef struct _hash_tag_buffer {
	struct _hash_tag_buffer *next;
	int num;
	int now_p;
	HASH_TAG tags[ 0 ];
} HASH_TAG_BUFFER;

struct _hash_info {
	int hash_table_size;
	int table_size_bit;
	int flag;
#ifdef SHOW_PERF
	int entry_num;
	int search_count;
	int max_search_level;
	int search_level_sum;
#endif
	HASH_TAG_BUFFER *buf;
	HASH_TAG **table;
};	 // HASH_INFO

/* ---------------------------------------------------------------------- */
/*
	下請け
*/

static HASH_TAG_BUFFER *new_tag_buf( int tagnum )
{
	HASH_TAG_BUFFER *buf;

	buf = my_malloc( sizeof( HASH_TAG_BUFFER )
						  + sizeof( HASH_TAG ) * HASH_TAG_BUFFER_EXPAND_SIZE );
	buf->next = NULL;
	buf->num = tagnum;
	buf->now_p = 0;

	return buf;
}

static void free_tag_buf( HASH_TAG_BUFFER *root )
{
	HASH_TAG_BUFFER *buf = root;

	while( buf != NULL ){
		HASH_TAG_BUFFER *next;

		next = buf->next;
		my_free( buf );
		buf = next;
	}
}

static HASH_TAG *new_tag( HASH_INFO *info, int id, char *key, void *data )
{
	HASH_TAG *tag;
	HASH_TAG_BUFFER *buf;

	buf = info->buf;

	if( buf->now_p >= buf->num ){
		// やばいので拡張

		info->buf = new_tag_buf( HASH_TAG_BUFFER_EXPAND_SIZE );
		info->buf->next = buf;
		buf = info->buf;
	}
	tag = &( buf->tags[ buf->now_p ] );
	buf->now_p ++;

	memset( tag, 0, sizeof( *tag ) );

	tag->id = id;
	tag->key = key;
	tag->ent.data = data;

	return tag;
}

/* ---------------------------------------------------------------------- */
/*
	作成
*/

HASH_INFO *hash_create( int size, int flag )
{
	HASH_INFO *info;
	int m;

	for( m = 0; m < 16; m++ ){		// 高々１６ビットに制限しておく。
		if( ( 1 << m ) >= size ){
			break;
		}
	}
	info = my_malloc( sizeof( HASH_INFO ) );

	memset( info, 0, sizeof( *info ) );
	
	info->hash_table_size = size;
	info->flag = flag;

	info->buf = new_tag_buf( HASH_TAG_BUFFER_EXPAND_SIZE );
	info->table_size_bit = m;
	info->table = my_malloc( sizeof( HASH_TAG * ) * ( 1 << m ) );

	memset( info->table, 0, sizeof( HASH_TAG * ) * ( 1 << m ) );

	return info;
}

void hash_destroy( HASH_INFO *info )
{
#ifdef SHOW_PERF
printf( "NUM = %d AV = %f MAX = %d\n", info->entry_num
	   , ( float )info->search_level_sum / info->search_count
	   , info->max_search_level );
{
	int i;
	int count = 0;
	for( i = 0; i < ( 1 << info->table_size_bit ); i++ ){
		if( info->table[ i ] == NULL ){
			count++;
		}
	}
	printf( "NULL COUNT = %d/%d\n", count, ( 1 << info->table_size_bit ) );
}
#endif
	my_free( info->table );
	free_tag_buf( info->buf );

	my_free( info );
}

/* ---------------------------------------------------------------------- */
/*
	登録
*/

static int id2hash( HASH_INFO *info, int id )
{
	int h;
	id = id + ( id >> info->table_size_bit );
	h = ( id & ( ( 1 << info->table_size_bit ) - 1 ) );
	return h;
}

static int str2id( char *str )
{
	int i;
	unsigned int id;
	unsigned char *p = ( unsigned char * )str;

	// 適当に先頭16バイトから生成する。

	id = 0;
	for( i = 0; i < 8; i++ ){
		if( p[ i ] == '\0' ) break;
		id = ( id << 4 ) + p[ i ];
	}
	return id;
}

static int string_cmp( HASH_TAG *tag, int id, char *key )
{
	int c;

	c = tag->id - id;
	if( c == 0 ){
		// さらにチェック
		c = strcmp( tag->key, key );
	}
	return c;
}

static int norm_cmp( HASH_TAG *tag, int id, char *key )
{
	return tag->id - id;
}

static HASH_ENTRY *entry( HASH_INFO *info, int id, char *key, void *data
					   , int (*cmp)( HASH_TAG *tag, int id, char *key ) )
{
	HASH_TAG *tag;
	int h;

	h = id2hash( info, id );

	if( ( tag = info->table[ h ] ) != NULL ){
		for( ;; ){
			int c;

			c = ( *cmp )( tag, id, key );

			if( c < 0 ){
				if( tag->low != NULL ){
					tag = tag->low;
				} else {
					// ここにヒット
					tag->low = new_tag( info, id, key, data );
					break;		// 登録できた
				}
			} else if( c > 0 ){
				if( tag->hi != NULL ){
					tag = tag->hi;
				} else {
					// ここにヒット
					tag->hi = new_tag( info, id, key, data );
					break;		// 登録できた
				}
			} else {
				// あたり
				return &( tag->ent );
			}
		}
	} else {
		info->table[ h ] = new_tag( info, id, key, data );
	}
#ifdef SHOW_PERF
	info->entry_num ++;
#endif
	return NULL;
}

HASH_ENTRY *hash_entry_string( HASH_INFO *info, char *key, void *data )
{
	int id;

	/* stringからHASH IDを作成 */
	id = str2id( key );

	return entry( info, id, key, data, string_cmp );
}

HASH_ENTRY *hash_entry_id( HASH_INFO *info, int id, void *data )
{
	return entry( info, id, NULL, data, norm_cmp );
}

/* ---------------------------------------------------------------------- */
/*
	検索
*/

static HASH_ENTRY *search( HASH_INFO *info, int id, char *key
					   , int (*cmp)( HASH_TAG *tag, int id, char *key ) )
{
	HASH_TAG *tag;
	int h;
#ifdef SHOW_PERF
	int count = 0;
#endif
	h = id2hash( info, id );

	if( ( tag = info->table[ h ] ) != NULL ){
		while( tag != NULL ){
			int c;

			c = ( *cmp )( tag, id, key );

			if( c < 0 ){
				tag = tag->low;
			} else if( c > 0 ){
				tag = tag->hi;
			} else {
				// あたり
#ifdef SHOW_PERF
				if( count > info->max_search_level ){
					info->max_search_level = count;
				}
				info->search_level_sum += count;
				info->search_count ++;
#endif
				return &( tag->ent );
			}
#ifdef SHOW_PERF
			count++;
#endif
		}
	}
	return NULL;
}

HASH_ENTRY *hash_search_string( HASH_INFO *info, char *key )
{
	int id;

	/* stringからHASH IDを作成 */
	id = str2id( key );

	return search( info, id, key, string_cmp );
}

HASH_ENTRY *hash_search_id( HASH_INFO *info, int id )
{
	return search( info, id, NULL, norm_cmp );
}
