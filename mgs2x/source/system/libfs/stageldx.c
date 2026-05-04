/*
	stageld.c
		HD ファイルシステム用ステージデータロードルーチン

	# 本来はGMが担当すべきモジュールだが、HD、CDの切り分けを
	# 行うために、FSに置く。

	1999/05/28 K.Uehara
	$Id: stageldx.c,v 1.1.1.3 2002/11/19 11:42:41 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "libgv.h"
#include "libfs.h"

/* 管理構造体 */

typedef struct {
	char *dirname;		/* ステージデータ格納ディレクトリ名 */
	int numfile;		/* そのステージデータ中のファイル数 */
	int nowfile;		/* 現在読み込み中のファイル番号 */

	char *filename;		/* 現在読み込み中のファイル名(NULLの時あり) */
	int size;			/* ファイルサイズ */
	int rest;			/* 残り */
	/*
		プロセス状態
	*/
	int			load_stat ;		/* 処理フェーズ			*/
	/*
		data.cnf 処理用
	*/
	char		*load_conf ;		/* data.cnf のバッファアドレス	*/
	char		*load_next ;		/* data.cnf の現在位置		*/
	int			aligned;			/* 128 Byte Align かどうか(qar) */
	short		cache_mode ;		/* キャッシュモード（０～２）	*/
	short		sound_mode ;		/* サウンドファイル初期化モード */
	/*
		読み込み中のデータ
	*/
	void		*load_data ;		/* 読み込んだデータのアドレス	*/
	char		load_name[ 64 ];	/* ファイル名バッファ */
	/*
		.dar ファイル初期化用
	*/
	void		*dar_next ;		/* .dar ファイル処理位置	*/
	int			dar_unit ;		/* .dar ファイル残りユニット数	*/
} FS_STAGE_INFO;

/*----------------------------------------------------------------*/

	/*
		処理フェーズ
	*/
enum	{
	WAIT_CONF,	/* data.cnf 待ち	*/
	LOAD_DATA,	/* ロード要求出し	*/
	WAIT_DATA,	/* ロード完了待ち	*/
	INIT_DAR,	/* .dar 初期化処理	*/
	INIT_QAR	/* .qar 初期化処理	*/
} ;

	/*
		cache_mode
	*/
enum {
	NOCACHE = 0,		/* ロード後メモリを解放する */
	CACHE = 1,			/* ロード後もメモリを保持 */
	RESIDENT = 2,		/* 常駐 */
	RESIDENT_REINIT = 3,/* 常駐の再読み込み */
};

/*----------------------------------------------------------------*/

int fs_loaded_file_size;
int FS_FileSize;

/*----------------------------------------------------------------*/

	/*
		拡張子チェック
	*/

char *FS_GetFileExt( char *name )
{
	char		c, *exp ;

	exp = name ;
	while ( ( c = *( name ++ ) ) != '\0' ) {
		if ( c == '.' ) exp = name ;
	}
	return exp;
}

int	FS_CheckFileExt( char *name, char *ext )
{
printf( "EXT CHeck%s %s\n", name, ext ) ;

	return ( strcmp( FS_GetFileExt( name ), ext ) == 0 );
}

	/*
		data.cnf ファイルを解釈して
		・キャッシュモードを変更
		・次のロード要求を発効
	*/
static char *get_word( char *now, char *buf )
{
	char c, *cp1, *cp2;

	/*
		１単語読み込む
	*/
	*( cp2 = buf ) = '\0';
	if( ( cp1 = now ) == NULL ) return NULL;
	do {
		if ( ( c = *( cp1 ++ ) ) == '\0' ) return NULL;
	} while ( isspace( ( int )c ) ) ;
	do {
		*( cp2 ++ ) = c ;
		if ( ( c = *( cp1 ++ ) ) == '\0' ) break ;
	} while ( !isspace( ( int )c ) ) ;
	*cp2 = '\0' ;
	return ( c == '\0' ) ? NULL : cp1;
}

	/*
		data.cnf ファイルを解釈して
		・キャッシュモードを変更
		・次のロード要求を発効
	*/
static	int	LoadNext( FS_STAGE_INFO *work )
{
	if( work->load_next == NULL ) return 0;

	for ( ; ; ) {
		work->load_next = get_word( work->load_next, work->load_name );

		switch( work->load_name[ 0 ] ){
		  case '\0':
			/* データの終り */
			return 0;
		  case '.':
			/*
			   '.' ではじまる単語ならば、キャッシュモードを変更
			*/
			switch ( work->load_name[ 1 ] ) {
			  case 'r' :		/* resident モード	*/
				work->cache_mode = RESIDENT ;
				break ;
			  case 'c' :		/* cache モード		*/
				work->cache_mode = CACHE ;
				break ;
			  case 'n' :		/* nocache モード	*/
				work->cache_mode = NOCACHE ;
				break ;
			}
			break;
		  default:
			/*
				ロード要求を発効する
			*/
			work->load_data = NULL;
			work->filename = work->load_name;
			if( work->filename[ 0 ] == '*' || work->filename[ 0 ] == '@' ){
				work->filename++;
			}

			if ( ( work->size = FS_LoadRequest( work->filename ) ) < 0 ) {
				/* ファイルがない */
				HANGUP();
				return -1 ;
			}
			if( work->load_name[ 0 ] == '@' ){
				/* quad word align */
				work->load_data = GV_AllocMemory( GV_NORMAL_MEMORY
												  , &work->load_data, work->size, 128 );
				work->aligned = 128;
			} else {
				work->load_data = GV_MallocLoad( work->size );
				work->aligned = 0;
			}
			
			FS_LoadSet( work->load_data, work->size );

			work->rest = work->size;
			work->nowfile ++;
			return 1 ;
		}
	}
}

static int get_file_total( char *datacnf_top )
{
	char *p = datacnf_top;
	char buffer[ 64 ];
	int count = 0;

	while( ( p = get_word( p, buffer ) ) != NULL ){
		if( buffer[ 0 ] != '.' ){
			count++;
		}
	}
	return count;
}

/*----------------------------------------------------------------*/

static int LoadInit( void *data, int id, FS_STAGE_INFO *work, int size )
{
	if( work->cache_mode == RESIDENT ){
		void *ptr;

		if( work->aligned != 0 ){
			ptr = GV_AllocResidentMemoryAligned( size, id, work->aligned );
		} else {
			ptr = GV_AllocResidentMemory( size, id );
		}
		memcpy( ptr, data, size );
		data = ptr;
	}
	FS_FileSize = size;	// テクスチャのイニシャライズでファイルサイズが必要
	return GV_LoadInit( data, id, work->cache_mode );
}

static	int	Act( FS_STAGE_INFO *work )
{
	u_char*	pu8Buf ;
	int		stat, unit ;

	switch ( work->load_stat ) {
	  case WAIT_CONF :
		/*
		   data.cnf 待ち
		   */
		stat = FS_LoadSync() ;
		if ( stat > 0 ) break ;

		/* data.cnf 読み終わった */
		*( work->load_conf + fs_loaded_file_size ) = '\0';
		work->load_stat = LOAD_DATA ;
		work->load_next = work->load_conf ;
		work->cache_mode = CACHE ;
		work->numfile = get_file_total( work->load_conf );
		work->nowfile = 0;
		break ;
	  case LOAD_DATA :
		/*
		   ロード要求出し
		   */
		stat = LoadNext( work ) ;
		if ( stat <= 0 ) {
			if( stat < 0 ){
				WARNING( "FILE READ ERROR %d!!\n", stat );
				HANGUP();
			}
			return 0;
		}
		work->load_stat = WAIT_DATA ;
		break ;
	  case WAIT_DATA :
		/*
		   ロード完了待ち
		   */
		stat = FS_LoadSync() ;
		work->rest = stat;

		if ( stat > 0 ) break ;
		if ( FS_CheckFileExt( work->filename, "dar" ) ) {
			/*
			   .dar ファイルならば、INIT_DAR モードへ移行
			*/
			pu8Buf = (u_char*)work->load_data ;
			work->dar_unit = *(u_int*)pu8Buf;
			pu8Buf = (u_char*)((u_int*)pu8Buf + 1) ;
			work->dar_next = (void*)pu8Buf;
			work->load_stat = INIT_DAR ;
		} else if( FS_CheckFileExt( work->filename, "qar" ) ) {
			/*
			   .qar ファイルならば、INIT_QAR モードへ移行
			*/
			pu8Buf = (u_char*)work->load_data ;
			work->dar_unit = work->size;	// サイズ情報に読み変える
			work->dar_next = pu8Buf ;
			work->load_stat = INIT_QAR ;
		} else {
			/*
			   通常ファイルならば、ただちに初期化処理
			*/
			fs_loaded_file_size = work->size;
printf( "loadinit %X %X\n", work->load_data, GV_CacheID3( work->filename ) );
			stat = LoadInit( work->load_data, GV_CacheID3( work->filename ),
							   work, work->size ) ;
			ASSERT( stat > 0 ) ;
			if ( work->cache_mode == NOCACHE ){
				/* no cache */
				GV_Free( work->load_data ) ;
			}
			work->load_stat = LOAD_DATA ;
		}
		break ;
	  case INIT_DAR :
		/*
			.dar 初期化処理
		*/
		pu8Buf = (u_char*)work->dar_next ;
		unit = work->dar_unit ;
		do {
			char		*cp ;
			int		size ;
			int     id ;

			work->dar_next = (void*)pu8Buf ;
			work->dar_unit = unit ;
			cp = work->load_name ;
			while ( ( *( cp ++ ) = *(char *)( pu8Buf ++ ) ) != '\0' ) ;
			pu8Buf += ( 3 & ( 4 - ( 3 & (u_int)pu8Buf ) ) ) ;	//align
			size = *(u_int *)pu8Buf ;
//			printf( "Load align %d\n", 15 & ( 16 - ( 15 & (u_int)pu8Buf ) ) ) ;
			
			pu8Buf += sizeof( u_int ) ;
			pu8Buf += ( 15 & ( 16 - ( 15 & (u_int)pu8Buf ) ) ) ;
			/*
				返り値が０の場合は次のターンにリトライする
			*/
			id = GV_CacheID3( work->load_name ) ;
printf( "[%s] : size %d id(%x) Addr%x \n", work->load_name, size, id, pu8Buf ) ;
#if 1
			stat = LoadInit( pu8Buf, id, work, size ) ;
			if ( stat == 0 ) return 1;
			if ( stat < 0 ) {
				/* エラー */
				printf( "INIT_ERROR in %s !!\n", work->load_name );
				HANGUP();
				return 0;
			}
#endif
			pu8Buf += size + 1 ;
		} while ( -- unit > 0 ) ;

		if ( work->cache_mode == NOCACHE ){
			/* no cache */
			GV_Free( work->load_data ) ;
		}
		work->load_stat = LOAD_DATA ;
		break ;
	  case INIT_QAR:
		/*
			.qar 初期化処理
		*/
		{
			char *buf;
			char *table;
			int i, num;
			typedef struct {
				int id;
				int size;
			} FILEINFO;
			FILEINFO *info;
printf( "INIT QAR\n" );
			buf = (char*)work->dar_next ;
			table = ( buf + *( int * )( buf + work->dar_unit - sizeof( int ) ) );
			num = *( short * )( table + 0 );
			info = ( FILEINFO * )( table + 4 );

			for( i = 0; i < num; i++ ){
printf( "INIT ID = %X %d\n", info->id, info->size );
				stat = LoadInit( buf, info->id, work, info->size );
				if( stat == 0 ) return 1;
				if( stat < 0 ){
					printf( "INIT_ERROR in %s !!\n", work->load_name );
					HANGUP();
					return 0;
				}
				buf += ( ( info->size + 127 ) / 128 ) * 128;
				info ++;
			}
		}
		work->load_stat = LOAD_DATA ;
		break;
	}
	return 1;
}

/* ---------------------------- 外部呼び出し関数 ------------------------- */

void *FS_LoadStageRequest( char *dirname )
{
	FS_STAGE_INFO *info;
	void *ptr;
	int size;
printf( "--LOAD STAGE %s\n", dirname );
	info = (FS_STAGE_INFO*)GV_MallocLoad( sizeof( FS_STAGE_INFO ) );
	
	ptr = NULL;
	info->load_stat = WAIT_CONF;
	info->load_conf = NULL;

	info->nowfile = -1;
	info->filename = info->load_name;

	FS_ChangeDirectory( dirname );

	if( ( size = FS_LoadRequest( "data.cnf" ) ) < 0 ){
		return NULL;
	}

	info->load_conf = (char*)GV_MallocLoad( size + 1 );
//	*( info->load_conf + size + 1 ) = '\0';
//	*( info->load_conf + size ) = '\0';
	FS_LoadSet( info->load_conf, size );
	fs_loaded_file_size = size;

	return info;
}

int FS_LoadStageSync( void *info )
{
	return Act( ( FS_STAGE_INFO * )info );
}

void FS_LoadStageComplete( void *info )
{
	GV_Free( info );
}

void FS_LoadStagePreseek( char *dirname )
{
	// ダミー
}

void FS_LoadSoundPak( int code )
{
	sd_set_cli( 0xFE800000 | code );
}

/* ---------------------------------------------------------------------- */
/*
	リンク用ダミー関数
*/

void FS_HdStageFileInit( void )
{
}

