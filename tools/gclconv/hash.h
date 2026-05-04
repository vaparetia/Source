/*
	汎用ハッシュライブラリヘッダ
*/


typedef struct {
	void *data;
} HASH_ENTRY;

typedef HASH_ENTRY HASH_RESULT;

#define HASH_VALUE( _ent ) ( (_ent)->data )

typedef struct _hash_info HASH_INFO;

#define HASH_KEY_STRING 1
#define HASH_KEY_INT	2

#define HASH_OK			0
#define HASH_FAIL		1

#define HASH_DEFAULT_SIZE	256

HASH_INFO *hash_create( int hash_size, int flag );
void hash_destroy( HASH_INFO *info );

HASH_ENTRY *hash_search_string( HASH_INFO *info, char *key );
	// 見つからなければNULL
HASH_ENTRY *hash_entry_string( HASH_INFO *info, char *key, void *data );
	// key, dataは staticなものでなければならない。
	// 登録できたらNULL, 出来なかったら同じＩＤのTAGを返す。

HASH_ENTRY *hash_search_id( HASH_INFO *info, int id );
	// 見つからなければNULL
HASH_ENTRY *hash_entry_id( HASH_INFO *info, int id, void *data );
	// 登録できたらNULL, 出来なかったら同じＩＤのTAGを返す。


