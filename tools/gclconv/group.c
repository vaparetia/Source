/*
	プロトタイプ実行前チェック用グループ処理
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mymalloc.h"
#include "gclconv.h"

typedef struct _group_tag{
	struct _group_tag *next;
	long value;
	char string[ 0 ];
} GROUP_TAG;

typedef struct _group {
	struct _group *next;
	GROUP_TAG *ref_tag;		/* 参照タグ */
	GROUP_TAG *chk_tag;		/* バッティングチェックタグ */
	char group_name[ 0 ];
} GROUP;

static GROUP *top = NULL;

void init_group( void )
{
	top = NULL;
}

GROUP *new_group( char *name )
{
	GROUP *new;

	new = Malloc( sizeof( GROUP ) + strlen( name ) + 1 );
	new->next = top;
	strcpy( new->group_name, name );
	new->ref_tag = NULL;
	new->chk_tag = NULL;

	top = new;

	return new;
}

void add_group_ref_tag( GROUP *gp, char *word )
{
	GROUP_TAG *tp;

	tp = Malloc( sizeof( GROUP_TAG ) + strlen( word ) + 1 );
	strcpy( tp->string, word );
	tp->value = get_strcode( word );
	tp->next = gp->ref_tag;
	gp->ref_tag = tp;
}

void add_group_chk_tag( GROUP *gp, char *word )
{
	GROUP_TAG *tp;

	tp = Malloc( sizeof( GROUP_TAG ) + strlen( word ) + 1 );
	strcpy( tp->string, word );
	tp->value = get_strcode( word );
	tp->next = gp->chk_tag;
	gp->chk_tag = tp;
}

static GROUP *search_group( char *groupname )
{
	GROUP *tag;

	for( tag = top; tag != NULL; tag = tag->next ){
		if( strcmp( tag->group_name, groupname ) == 0 ){
			return tag;
		}
	}
	return NULL;
}

static int search_tag( GROUP_TAG *top, char *word )
{
	GROUP_TAG *tp;
	int code;
	code = get_strcode( word );
	for( tp = top; tp != NULL; tp = tp->next ){
		if( strcmp( tp->string, word ) == 0 ){
			return 1;
		}
		if( tp->value == code ){
			ERROR( "文字列IDが同じです%s:%s\n", tp->string, word );
		}
	}
	return 0;
}

/* ------------------------------------------------------------ */
/*
	外とのインターフェースルーチン
*/

void analyze_group( void )
{
	WORDBUF word;
	GROUP *gp;

	/* すでに group タグは読み込まれている */

	get_word_ex( word );

	if( search_group( word ) != NULL ){
		ERROR( "group %s が再定義されたか、すでに使用されています\n", word );
	}

	gp = new_group( word );

	get_word_cl( word );
	if( word[ 0 ] != '{' ){
		ERROR( "{ がありません\n" );
	}

	for( ;; ){
		get_word_cl( word );
		if( word[ 0 ] == '}' ){
			break;
		}
		if( word[ 0 ] == ',' ){
			continue;
		}
		if( search_tag( gp->ref_tag, word ) ){
			WARNING( "再定義です\n" );
		}
		add_group_ref_tag( gp, word );
	}
}

int check_group( char *group, char *tag )
{
	GROUP *gp;

PRINTF( "CHECK_GROUP %s %s\n", group, tag );
	if( ( gp = search_group( group ) ) == NULL ){
		/* 新規に作成する */
PRINTF( "NEW GROUP\n" );
		gp = new_group( group );
	}
	if( gp->ref_tag != NULL ){
		/* group 定義されている */
PRINTF( "REF:\n" );
		if( ! search_tag( gp->ref_tag, tag ) ){
			ERROR( "Group定義されていない文字コードです\n" );
		}
	} else {
PRINTF( "CHK:\n" );
		if( ! search_tag( gp->chk_tag, tag ) ){
			add_group_chk_tag( gp, tag );
		}
	}
	return 0;
}
