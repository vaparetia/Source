/*
	サブ／ローカルメモリ管理用ルーチン
		空きリストのみしか管理しない
		ヘッダサイズを消費しない
		解放にはそのブロック取得時のサイズが必要

	2000/07/04	K.Uehara
	$Id: memlist.h,v 1.1.1.3 2002/11/19 11:42:44 Yoshizawa1 Exp $
*/

#ifndef __GV_ML__
#define __GV_ML__

// 空きリスト管理用
typedef struct _memtag {
	struct _memtag *next;
	int size;
} MEMTAG;

// メモリ領域管理用
typedef struct _memlist {
	char *name;				// デバッグ用
	void *top;
	void *bottom;
	void *now_bottom;
	int align;
	MEMTAG empty;
} MEMLIST;

void GV_MlBufferInit( MEMLIST *list, char *name, void *top, void *bottom, int align );
void *GV_MlMalloc( MEMLIST *list, int size );
void GV_MlFree( MEMLIST *list, void *free, int size );

#define GV_MlGetSize( _p )	( *( int * )_p )

#ifdef DEBUG_MODE
void GV_MlPrintDebug( MEMLIST *list );
#endif // DEBUG_MODE

#endif // __GV_ML__
