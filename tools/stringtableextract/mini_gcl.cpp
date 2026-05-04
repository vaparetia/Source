/*
	@file
	gcl コマンド管理ルーチン

	1999/07/08 K.Uehara
	$Id: command.c,v 1.12 2004/04/29 04:14:11 usr01475 Exp $
*/

//#include	"sys_common.h"

#ifndef MGS_VERSION
#pragma error Must define MGS_VERSION
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include	"libgcl.h"

extern bool gbOriginalReleaseData;

int GCL_VariableVersion;

static GCL_SCRIPT current_script;
static GCL_STRING_RESOURCE current_strres;

GCL_ARGS gcl_null_args = { 0, NULL };

typedef struct {
#if MGS_VERSION == 2
   unsigned int id;
#endif
	unsigned int offset;
} GCL_PROC_TABLE;

#ifdef GCL_DEBUG_MODE

#ifdef _KPXBOX
#define cprintf printf
#endif

/* ---------------------------------------------------------------------- */
/*
	デバッグ用ログとり関数
*/

static int get_script_offset( void *ptr )
{
	int offset;
	if( ptr == NULL ) return 0;

	if( ( offset = ( char * )ptr - current_script.script_body ) < 0 ){
		offset = ( ( char * )ptr - current_script.proc_body ) | 0x80000000;
	}
	return offset;
}

static void GCL_DEBUG_Reset( void )
{
	GCL_WORK.block_nest = 0;
	GCL_WORK.call_stack_p = GCL_WORK.call_stack;
}

static void GCL_DEBUG_NestIn( void )
{
	GCL_WORK.block_nest ++;
}

static void GCL_DEBUG_NestOut( void )
{
	GCL_WORK.block_nest --;
}

static void GCL_DEBUG_InBlock( void *ptr, void *arg, int local_arg )
{
	int offset;

	offset = get_script_offset( ptr );

	if( arg != NULL ){
		// ARGが積まれる
		offset |= ( local_arg + 1 ) << 24;		// 最大16
	}
	ASSERT( GCL_WORK.call_stack_p - GCL_WORK.call_stack <= GCL_DEBUG_MAX_CALL_STACK );

	*( GCL_WORK.call_stack_p ++ ) = offset;
}

static void GCL_DEBUG_OutBlock( void )
{
	GCL_WORK.call_stack_p --;
}

/**
	GCLスタックのバックトレースを表示する。
*/
void GCL_DEBUG_DumpLog( void )
{
	if( GCL_WORK.block_nest > 0 ){
		int *p;
		int *asp;

		asp = GCL_WORK.argstack_p;
		
		cprintf( "----- GCL LOG nest = %d -----\n", GCL_WORK.block_nest );
		cprintf( "NOW   : %08X\n", get_script_offset( GCL_WORK.next_str_ptr ) & 0x00FFFFFF );
		for( p = GCL_WORK.call_stack_p - 1; p >= GCL_WORK.call_stack; p -- ){
			cprintf( "%s : %08X "
					, ( *p & 0x80000000 ) ? "PROC " : "BLOCK"
					, ( *p & 0x00FFFFFF ) );

			if( asp > GCL_WORK.argbuffer ){
				GCL_ARGS *arg;
				int n = ( ( *p >> 24 ) & 0x1F ) - 1;

				arg = ( GCL_ARGS * )( asp[ - 1 ] );

				if( arg->argc > 0 ){
					int i;
					cprintf( "( " );
					for( i = 0; i < arg->argc; i++ ){
						cprintf( "%X(%d) ", arg->argv[ i ], arg->argv[ i ] );
					}
					cprintf( ") " );
				}
				if( n > 0 ){
					int i;
					cprintf( "[ " );
					for( i = 0; i < n; i++ ){
						cprintf( "%X(%d) ", *( asp - 2 - i ), *( asp - 2 - i ) );
					}
					cprintf( "]" );
				}
			}
			cprintf( "\n" );
			asp -= ( ( *p >> 24 ) & 0x1F );
		}
		cprintf( "----------------\n" );
	}
#if 0
	{
		int i;
		GCL_PROC_TABLE *pt = current_script.proc_table;
		for( i = 0; i < current_script.proc_num; i++ ){
			cprintf( "ID:%08X : OFS %08X\n", pt[ i ].id, pt[ i ].offset );
		}
	}
#endif
}
#else

/* 消し去る */
#define GCL_DEBUG_Reset()
#define GCL_DEBUG_NestIn()
#define GCL_DEBUG_NestOut()
#define GCL_DEBUG_InBlock( _ptr, _arg, _local_arg )
#define GCL_DEBUG_OutBlock()

#ifdef DEBUG_MODE
void GCL_DEBUG_DumpLog( void )
{
}
#endif

#endif // GCL_DEBUG_MODE

/* ------------------ コマンドリスト関連 ------------------ */

static GCL_COMMANDDEF *commdef;

/**
	コマンドリストを初期化。
*/
void GCL_ResetCommList( void )
{
	commdef = NULL;
}

/**
	コマンドを登録。
	@param def	コマンドリスト
*/
int GCL_AddCommMulti( GCL_COMMANDDEF *def )
{
//D( printf( "AddCommMulti\n" ); );

	def->next = commdef;
	commdef = def;

#if 0
{
	int i;
	GCL_COMMANDLIST *cl;
	cl = def->commlist;
	for( i = def->n_commlist; i > 0; i-- ){
		printf( "COM %d ADR %X\n", cl->id, cl->func );
		cl++;
	}
}
#endif

	return 0;
}

/**
	コマンドリストをコマンドから削除。
	@param def	コマンドリスト
*/
int GCL_DelCommMulti( GCL_COMMANDDEF *def )
{
	GCL_COMMANDDEF *p, *prev;

	if( commdef == def ){
		commdef = def->next;
	}
	p = prev = commdef;
	for( ; p != NULL; prev = p, p = p->next ){
		if( p == def ){
			prev->next = p->next;
			return 0;
		}
	}
	return -1;
}

#if 0

/* コマンドリストから、指定IDのコマンドを検索する */

static GCL_COMMANDLIST *FindCommand( int id )
{
	GCL_COMMANDDEF *def;
D( printf( "FindCommand %X\n", id ); );
	for( def = commdef; def != NULL; def = def->next ){
		int i;
		GCL_COMMANDLIST *cl;
		cl = def->commlist;
		for( i = def->n_commlist; i > 0; i-- ){
			if( cl->id == id ) return cl;
			cl++;
		}
	}
	printf( "command not found %x\n", id );
	HANGUP();
	return NULL;
}

/**
	コマンドを呼び出す。
	@param ptr	コマンド先頭アドレス
*/
int GCL_Command( char *ptr )
{
	/* コマンド呼び出し */
	int id;
	GCL_COMMANDLIST *cl;
	int ret;
	unsigned char *p = ( unsigned char * )ptr;
	unsigned char *next;
	int ofs;

	id = GCL_GetStrCode( (char*)p );
	p += STRCODE_SIZE;

	cl = FindCommand( id );

	next = GCL_GetShortSize( p, &ofs );

	GCL_SetCommandLine( next + ofs );	/* オプション以外の部分をスキップ */
	GCL_SetArgTop( next );				/* コマンドライン先頭を設定 */

	GV_CALL_BEGIN();
	GV_CALL_SET( cl->func );

D( printf( "call %x\n", cl->func ); );
	ret = ( *cl->func )( next );		/* コマンドライン先頭からパース */

	GV_CALL_END();

	GCL_UnsetCommandLine();

	return ret;
}
#endif
/* -------------------- PROC 関連 ------------------- */

#ifndef UNUSE_PROC_SELECT
/**
	言語対応。procプリフィックスを有効にする。

	@param no	有効にするprocプリフィックスナンバー。
*/
void GCL_SetProcSelectNo( int no )
{
	GCL_WORK.proc_select_no = no;
}
#endif

/* ユーザー定義関数( PROC )の初期設定 */

static char *set_proc_table( char *proc_table, int *proc_num )
{
	GCL_PROC_TABLE *pt;
	int n = 0;

	pt = ( GCL_PROC_TABLE * )proc_table;
#if MGS_VERSION == 3
	while( *( int * )pt != 0xFFFFFFFF )
#else
   while( *( int * )pt != 0 )
#endif
   {
		pt++;
		n++;
	}
	*proc_num = n;
	return ( char * )( pt + 1 );
}

#define OFFSET_LOCAL_NUM( ofs ) ( ( ((unsigned int )(ofs)) >> 24 ) & 0xF )
#define OFFSET_PREFIX( ofs )	( ( ((unsigned int )(ofs)) >> 28 ) & 0xF )
#define OFFSET_CODE( ofs )		( (ofs) & 0x00FFFFFF )

#if 0
static char *get_proc_block( int id, int *local_arg_num )
{
	GCL_PROC_TABLE *pt;
	int offset;
	char *res;

	if( id == 0 ){
		// NULL proc
		return 0;
	}

	offset = ( id & 0x7FFF ) - 1;	// 1始まり

	XASSERT( offset >= 0 && offset <= current_script.proc_num, "Wrong Proc ID %X\n", id );

	pt = ( GCL_PROC_TABLE * )current_script.proc_table + offset;
#ifndef UNUSE_PROC_SELECT
	if( id & 0x8000 ){
		/* prefixつき */
		int prev = 0;
		for( ;; ){
			int pref = OFFSET_PREFIX( BP_LE_SwapUInt(pt->offset) );
			if( pref == GCL_WORK.proc_select_no ){
				break;
			}
			XASSERT( pref > prev, "wrong lang code %d\n", GCL_WORK.proc_select_no );
			prev = pref;
			pt++;
			XASSERT( BP_LE_SwapUInt(pt->offset) != -1, "wrong lang code %d\n", GCL_WORK.proc_select_no );
		}
	}
#endif

	*local_arg_num = OFFSET_LOCAL_NUM( BP_LE_SwapUInt(pt->offset) );

	res = current_script.proc_body + OFFSET_CODE( BP_LE_SwapUInt(pt->offset) );

	return res;
}

static int GCL_ExecProcBlock( char *top, GCL_ARGS *args, int local_num );

/**
	Procを呼び出す。

	@param proc_id  呼び出すprocのid。
	@param arg		procに渡す引数。GCL_ARGS構造体で渡す。ない場合はNULL。
	@return return で返ってきた値。ない場合は０。
*/
int GCL_ExecProc( int proc_id, GCL_ARGS *arg )
{
	int result;

#ifdef DEBUG_MODE
	int save_id;
	save_id = GCL_DEBUG_CurrentProcID;
	GCL_DEBUG_CurrentProcID = proc_id;
#endif
	{
		char *p;
		int size;
		int local;

		p = get_proc_block( proc_id, &local );
		p = GCL_GetBlockSize( p, &size );
		result = GCL_ExecProcBlock( p, arg, local );
	}
#ifdef DEBUG_MODE
	GCL_DEBUG_CurrentProcID = save_id;
#endif
	return result;
}

static int GCL_Proc( char *p )
{
	/* PROC コマンド呼び出し */
	/* ARGSをintのバッファに積んで、ExecBlockを再帰呼び出し */

	int argbuf[ GCL_PROC_MAX_ARGS ];
	int i, id;
	GCL_ARGS arg;

	/* 先頭は ID */
	id = GCL_GetShort( p );
	p += 2;

D( printf( "id %X\n", id ); );

	for( i = 0; ; i++ ){
		int type, value;

		p = GCL_GetNextValue( p, &type, &value );
		if( type == GCL_END ) break;
		if( i >= GCL_PROC_MAX_ARGS ){
			printf( "TOO MANY ARGS PROC %d\n", GCL_PROC_MAX_ARGS );
			HANGUP();
		}
		argbuf[ i ] = value;
	}
	arg.argc = i;
	arg.argv = argbuf;

	return GCL_ExecProc( id, &arg );
}
#endif

/* -------------------- PROC,COMMAND 共通のbsearch ------------------- */
/*
	GCL_PROC_TABLEの構造は、charaの検索テーブルも同じ
*/

/**
	{ int id, int value } 型の構造体用のbsearch。
	command, procの検索に使用。
	@param buffer	データ先頭
	@param id		検索するID
	@param n		データ数
*/
#if 0
void *GCL_SearchId( void *buffer, unsigned int id, int n )
{
	typedef struct {
		unsigned int id;
		int value;
	} IdTable;
	
	IdTable *pt = buffer;
	int low, high;
	int m = 0;

	low = 0;
	high = n - 1;

	while( low < high ){
		m = ( low + high ) / 2;
		if( pt[ m ].id < id ){
			low = m + 1;
		} else {
			high = m;
		}
	}
	if( pt[ low ].id == id ){
		return ( void * )&pt[ low ];
	}
	return NULL;
}
#endif
/* ------------------ 文字列リソース関連 -----------------*/

/**
	GCLのリソースの取得
	@param id リソースID
*/
char *GCL_GetStringResource( int id )
{
	GCL_STRING_RESOURCE *cfr;
	int offset;

	cfr = &current_strres;
	offset = GCL_GetLong( ( char * )( cfr->resource_table + id ) );
	return cfr->string_table + ( offset & 0x7FFFFFFF );
}

/* ------------------ スクリプトの初期設定 -----------------*/

/*
	暗号化関連
*/

static unsigned int rand_seed = 12;

static void my_srand( int seed )
{
	rand_seed = seed;
}

static int my_rand( void )
{
	rand_seed = 2100005341UL * rand_seed + 3321;
	return ( rand_seed >> 15 );
}

#if MGS_VERSION==3
static void decode_buffer( int seed, void *buffer, int size )
{
	unsigned char *p = (unsigned char*)buffer;
	int i;

	my_srand( seed );
	for( i = 0; i < size; i++ ){
		int c;
		c = ( my_rand() & 0xFF );
		*p ^= c;
		p++;
	}
}
#endif

static int GCL_LoadScript_MGS2_SOL( char * const datatop )
{
#if MGS_VERSION==2
	GCL_SCRIPT *cur;
	int num;
	char *next = datatop;

	cur = &current_script;

	GCL_VariableVersion = GCL_GetLong( next );
D( printf( "var version %X\n", GCL_VariableVersion ) );
	next += sizeof( int );
   int procAndTableSize = GCL_GetLong( next );
   next += sizeof( int );

   int scriptBodySize = GCL_GetLong( next + procAndTableSize );
   cur->script_body = next + procAndTableSize + sizeof( int );
   cur->proc_table = next;
	next = set_proc_table( next, &num );
	cur->proc_num = num;
//   int procSize = GCL_GetLong( next );
   cur->proc_body = next + sizeof( int );
   next = cur->script_body + scriptBodySize;

   //Last section start is 16-byte aligned
   while( ( next - datatop ) & 0xf )
   {
      ++next;
   }

	{
		/* フォント設定 */
		char *fres_top = next;
		GCL_STRING_RESOURCE *cfr = &current_strres;

		cfr->block_top = fres_top;
		cfr->resource_table = ( int * )( GCL_GetLong( fres_top + 4 ) + fres_top );
		cfr->string_table = GCL_GetLong( fres_top + 8 ) + fres_top;
		cfr->font_data = GCL_GetLong( fres_top + 12 ) + fres_top;

		next = fres_top + GCL_GetLong( fres_top );
	}

	GCL_DEBUG_Reset();

	return 0;
#else
   printf("Error!  Original release data only supported for MGS2.\n");
   throw false;
#endif
}

/**
	GCX データのセットアップ。
	@param datatop	gcxデータの先頭
*/
int GCL_LoadScript( char *datatop )
{
   if( gbOriginalReleaseData )
   {
      return GCL_LoadScript_MGS2_SOL( datatop );
   }

	GCL_SCRIPT *cur;
	int num;
	char *next;

	cur = &current_script;

	GCL_VariableVersion = GCL_GetLong( datatop );
D( printf( "var version %X\n", GCL_VariableVersion ) );
	datatop += sizeof( int );

	next = set_proc_table( datatop, &num );
	cur->proc_table = datatop;
	cur->proc_num = num;
	{
		/* フォント設定 */
		char *fres_top = next;
		GCL_STRING_RESOURCE *cfr = &current_strres;

		cfr->block_top = fres_top;
		cfr->resource_table = ( int * )( GCL_GetLong( fres_top + 4 ) + fres_top );
		cfr->string_table = GCL_GetLong( fres_top + 8 ) + fres_top;
		cfr->font_data = GCL_GetLong( fres_top + 12 ) + fres_top;

		next = fres_top + GCL_GetLong( fres_top );

#if MGS_VRSION == 3
      int seed;
		if( ( seed = GCL_GetLong( fres_top + 16 ) ) != 0 ){
			// 暗号化を解除。
			decode_buffer( seed, cfr->string_table, cfr->font_data - cfr->string_table );
			memset( fres_top + 16, 0, sizeof( int ) );
		}
#endif
	}
	cur->proc_body = next + sizeof( int );
	cur->script_body = next + sizeof( int ) + GCL_GetLong( next ) + sizeof( int );

	GCL_DEBUG_Reset();

	return 0;
}

/**
	gcx中のフォントデータの先頭を返す。
*/
char *GCL_GetFontDataTop( void )
{
	return current_strres.font_data + sizeof( int );
}

char *GCL_GetFontDataEnd( void )
{
   return current_strres.block_top + GCL_GetLong(current_strres.block_top);
}

/**
	現在のGCL実行環境のセーブ。
	違うGCXを一時的にロードする時に使う。

	@param gclenv 実行環境
*/
void GCL_SaveCurrentEnvironment( GCL_ENVIRONMENT *gclenv )
{
	gclenv->script = current_script;
	gclenv->res = current_strres;
}

/**
	セーブした環境を戻す。

	@param gclenv 実行環境
*/
void GCL_RestoreEnvironment( GCL_ENVIRONMENT *gclenv )
{
	current_script = gclenv->script;
	current_strres = gclenv->res;
}

/* ------------------ スクリプトの実行 -------------------*/

unsigned char *GCL_GetBlockSize( unsigned char *top, int *size )
{
   /* top は GCL_TAGのあるアドレス */
   int code;
   D( printf( "GetBlock %02X\n", *top ) );
   code = ( *top & 0x0F );

   switch( code ){
     case 0x0D:
        *size = *( top + 1 );
        return top + 1 + 1;
     case 0x0E:
        *size = GCL_GetUShort( (char*)top + 1 );
        return top + 1 + 2;
     case 0x0F:
        *size = GCL_Get3Bytes( (char*)top + 1 );
        return top + 1 + 3;
     default:
        *size = code;
        return top + 1;
   }
}

#if 0
static inline int _GCL_ExecBlockBody( char *top )
{
	unsigned char *p;
	unsigned char *next;
	int size;

	/*
	   コマンドを実行する。引数リストは呼び出し側で設定。
	   BLOCKコマンドの後の実体データのポインタを渡す。
	*/
	GCL_DEBUG_NestIn();

D( printf( "ExecBlock\n" ); );
	p = top;
	while( p != NULL ){
		switch( GCL_TAG( *p ) ){
		  case GCL_COMMAND:
D( printf( "Command\n" ); );
			next = GCL_GetBlockSize( p, &size );
			if( GCL_Command( next ) == GCL_RETURN ){
D( printf( "RETURN\n" ); );
				return GCL_RETURN;
			}
			p = next + size;
			break;
		  case GCL_PROC:
D( printf( "Proc\n" ); );
			next = GCL_GetBlockSize( p, &size );
			GCL_Status = GCL_Proc( next );
			p = next + size;
			break;
		  case GCL_EXPR:
			next = GCL_GetBlockSize( p, &size );
			GCL_Status = GCL_Expr( next );
			p = next + size;
			break;
		  case GCL_END:
D( printf( "End\n" ); );
			/* ブロックの終了 */
			goto END;
		  default:
			printf( "SCRIPT COMMAND ERROR %x\n", *p );
			HANGUP();
		}
	}
	printf( "ERROR in script\n" );
	HANGUP();
END:
	GCL_DEBUG_NestOut();
	return 0;
}

/**
	ExecBlockの本体。内部使用。

	@param top		ブロックの先頭
	@param args		引数
	@param local_num	そのブロックで使うローカル変数の数
*/
int GCL_ExecBlockBody( char *top, GCL_ARGS *args, int local_num )
{
	int res;
	void *org_stack;

	/* 引数をスタックに積む。引数は以下の関数で $1 などの形で参照できる */
	org_stack = GCL_SetArgStack( args, local_num );

	GCL_DEBUG_InBlock( top, ( void * )args, local_num );

	/* ブロック実行本体 */
	res = _GCL_ExecBlockBody( top );

	GCL_DEBUG_OutBlock();

	/* 引数スタックを解放 */
	GCL_UnsetArgStack( org_stack );

	return res;
}

/**
	ブロックを実行する。

	@param top		ブロックの先頭アドレス
	@param args		procに渡す引数。GCL_ARGS構造体で渡す。ない場合はNULL。
	@return return で返ってきた値。ない場合は０。
*/
int GCL_ExecBlock( char *top, GCL_ARGS *args )
{
	if( GCL_ExecBlockBody( top, args, 0 ) == GCL_RETURN ){
		return GCL_Status;
	}
	GCL_Status = 0;
	return 0;	// デフォルトの値は０
}

static int GCL_ExecProcBlock( char *top, GCL_ARGS *args, int local_num )
{
	if( args == NULL ){
		args = &gcl_null_args;
	}
	if( GCL_ExecBlockBody( top, args, local_num ) == GCL_RETURN ){
		return GCL_Status;
	}
	GCL_Status = 0;
	return 0;	// デフォルトの値は０
}

/**
	GCL_LoadScript()で読み込んだスクリプトの実行。
*/

void GCL_ExecScript( void )
{
	unsigned char *datatop;
	unsigned char *next;
	int size;

	datatop = current_script.script_body;
D( printf( "datatop = %X\n", datatop ) );
	if( GCL_TAG( *datatop ) != GCL_BLOCK ){
		printf( "NOT SCRIPT DATA %X !!\n", *datatop );
		HANGUP();
	}
	next = GCL_GetBlockSize( datatop, &size );
	GCL_ExecBlock( next, &gcl_null_args );
}
#endif



