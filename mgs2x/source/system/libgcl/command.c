//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	コマンド管理ルーチン

	1999/07/08 K.Uehara
	$Id: command.c,v 1.6 2003/01/09 13:13:49 takaki Exp $
*/
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	"libgcl.h"
#include	"font.h"
#include "BP_EndianSupport.h"

#ifdef KP_XBOX
#include <xtl.h>
#endif	// KP_XBOX

int GCL_VariableVersion;

static GCL_SCRIPT current_script;
static GCL_STRING_RESOURCE current_strres;

GCL_ARGS gcl_null_args = { 0, NULL };

typedef struct {
   // BP - These are values that are little-endian
	unsigned int le_id;
	unsigned int le_offset;
} GCL_PROC_TABLE;

typedef struct {
   unsigned int id;
   int value;
} CHARA_ID_TABLE;

static void *_GCL_SearchProcBlockId( void *buffer, unsigned int id, int n );

#ifdef GCL_DEBUG_MODE

#ifdef KP_XBOX
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
	*( GCL_WORK.call_stack_p -- );
}

// GCLスタックのバックトレース

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

/* コマンドリストの登録 */

void GCL_ResetCommList( void )
{
	commdef = NULL;
}

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

int GCL_Command( char *ptr )
{
	/* コマンド呼び出し */
	int id;
	GCL_COMMANDLIST *cl;
	int ret;
	unsigned char *p = ( unsigned char * )ptr;
	unsigned char *next;
	int ofs;

	id = GCL_GetStrCode( p );
	p += STRCODE_SIZE;

	cl = FindCommand( id );

	next = GCL_GetShortSize( p, &ofs );

	GCL_SetCommandLine( next + ofs );	/* オプション以外の部分をスキップ */
	GCL_SetArgTop( next );				/* コマンドライン先頭を設定 */

D( printf( "call %x\n", cl->func ); );
	ret = ( *cl->func )( next );		/* コマンドライン先頭からパース */

	GCL_UnsetCommandLine();

	return ret;
}

/* -------------------- PROC 関連 ------------------- */

#ifndef UNUSE_PROC_SELECT
void GCL_SetProcSelectNo( int no )
{
	GCL_WORK.proc_select_no = ( no << 24 );
}
#endif

/* ユーザー定義関数( PROC )の初期設定 */

static char *set_proc_table( char *proc_table, int *proc_num )
{
	GCL_PROC_TABLE *pt;
	int n = 0;

	pt = ( GCL_PROC_TABLE * )proc_table;
	while( *( int * )pt != 0 ){
D( printf( "PROC %X %X\n", BP_LE_SwapUInt( pt->id ), BP_LE_SwapUInt( pt->offset ) ); );
		pt++;
		n++;
	}
	*proc_num = n;
	return ( char * )( pt + 1 );
}

static char *_get_proc_block( int id, int *local_arg_num )
{
	// 多国語対応
	GCL_PROC_TABLE *pt;

	pt = ( GCL_PROC_TABLE * )current_script.proc_table;

	/* バイナリサーチ */
	pt = _GCL_SearchProcBlockId( current_script.proc_table, id, current_script.proc_num );
	if( pt != NULL ){
		void *res;
      unsigned int pt_offset = BP_LE_SwapUInt( pt->le_offset );

		*local_arg_num = ( ( unsigned int )pt_offset >> 24 );

		res = ( pt_offset & 0x00FFFFFF ) + current_script.proc_body;

		return res;
	}
	return NULL;
}

static char *get_proc_block( int id, int *local_arg_num )
{
	char *res;

	if( ( res = _get_proc_block( id, local_arg_num ) ) != NULL ){
		return res;
	}
#ifndef UNUSE_PROC_SELECT
	if( ( res = _get_proc_block( id | GCL_WORK.proc_select_no, local_arg_num ) ) != NULL ){
		return res;
	}
	printf( "LANG = %X\n", GCL_WORK.proc_select_no );
#endif
	printf( "PROC %X NOT FOUND. \n", id );

	HANGUP();
	return NULL;
}

#if 0
int GCL_ForceExecProc( int proc_id, GCL_ARGS *arg )
{
	return GCL_ExecBlock( get_proc_block( proc_id ) + 5, arg );
}
#endif

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
#ifndef KP_WINDOWS
		p = GCL_GetBlockSize( p, &size );
		result = GCL_ExecProcBlock( p, arg, local );
#else
		if( !p ){
			ASSERT(0) ;
			result = 0 ;		// 安全策
		} else {
			p = GCL_GetBlockSize( p, &size );
			result = GCL_ExecProcBlock( p, arg, local );
		}
#endif
	}
#ifdef DEBUG_MODE
	GCL_DEBUG_CurrentProcID = save_id;
#endif
	return result;
}

int GCL_Proc( char *p )
{
	/* PROC コマンド呼び出し */
	/* ARGSをintのバッファに積んで、ExecBlockを再帰呼び出し */

	int argbuf[ GCL_PROC_MAX_ARGS ];
	int i, id;
	GCL_ARGS arg;

	/* 先頭は ID */
	id = GCL_GetStrCode( p );
	p += STRCODE_SIZE;
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

/* -------------------- PROC,COMMAND 共通のbsearch ------------------- */
/*
	GCL_PROC_TABLEの構造は、charaの検索テーブルも同じ
*/

void *GCL_SearchId( void *buffer, unsigned int id, int n )
{
	CHARA_ID_TABLE *pt = buffer;
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

static void *_GCL_SearchProcBlockId( void *buffer, unsigned int id, int n )
{
   // BP - Very similar to GCL_SearchId, except proc blocks have non-swapped tables to look through
   GCL_PROC_TABLE *pt = buffer;
   int low, high;
   int m = 0;

   low = 0;
   high = n - 1;

   while( low < high ){
      m = ( low + high ) / 2;
      if( BP_LE_SwapUInt( pt[ m ].le_id ) < id ){
         low = m + 1;
      } else {
         high = m;
      }
   }
   if( BP_LE_SwapUInt( pt[ low ].le_id ) == id ){
      return ( void * )&pt[ low ];
   }
   return NULL;
}

/* ------------------ 文字列リソース関連 -----------------*/

char *GCL_GetStringResource( int id )
{
	GCL_STRING_RESOURCE *cfr;
	int offset;

	cfr = &current_strres;
	offset = GCL_GetLong( ( char * )( cfr->resource_table + id ) );
	return cfr->string_table + ( offset & 0x7FFFFFFF );
}


static char *GCL_LookupBPStringByKPString_InResourceTable(char* pKPString, GCL_STRING_RESOURCE *cfr)
{
   int i;
   int foundIndex = -1;
   int* pStringOffsets = cfr->resource_table;
   int const numStrings = (int*)cfr->string_table - cfr->resource_table;

   //Must have BP string data to do string lookup!
   if(cfr->bp_string_data == NULL)
   {
      BP_MARCO_BREAK;
      return pKPString;
   }

   for( i = 0; i < numStrings; ++i )
   {
      unsigned int stringOffset = GCL_GetLong( pStringOffsets+i );
      if( stringOffset & 0x80000000 )
      {
         int const offset = stringOffset & 0x7FFFFFFF;
         char* pString = cfr->string_table + offset;

         if( strcmp(pString, pKPString) == 0 )
         {
            foundIndex = i;
            break;
         }
      }
   }

   if( foundIndex >= 0 )
   {
      // first four bytes is number of strings
      int* bpStringOffset = (int*)(cfr->bp_string_data + 4);

      return cfr->bp_string_data + BP_LE_SwapSInt( bpStringOffset[foundIndex] );
   }

   return NULL;
}

extern GCL_STRING_RESOURCE *gGlobalResInfo;
extern GCL_STRING_RESOURCE *gGlobalResInfo2;

extern char* BP_GetOverrideString(char* inputString);

char *GCL_LookupBPStringByKPString(char* pKPString)
{
   char* result = NULL;

   if( pKPString )
   {
      if( strlen(pKPString) == 0 )
         return "";

      // Look up in current loaded string resource first
      result = GCL_LookupBPStringByKPString_InResourceTable(pKPString, &current_strres);

      // If that lookup fails look in global "resource" (resman.c)
      if( gGlobalResInfo && result == NULL )
         result = GCL_LookupBPStringByKPString_InResourceTable(pKPString, gGlobalResInfo);

      // If that lookup fails look in "other" global "resource" (resource.c)
      if( gGlobalResInfo2 && result == NULL )
         result = GCL_LookupBPStringByKPString_InResourceTable(pKPString, gGlobalResInfo2);
   }

   if( result == NULL )
      result = pKPString;

   return BP_GetOverrideString(result);
}

/* ------------------ スクリプトの初期設定 -----------------*/

int GCL_LoadScript( char *datatop )
{
   int bpFourCC, is_bp_gcl;

	GCL_SCRIPT *cur;
	int num;
	char *next;

	cur = &current_script;

   printf( "GCL_LoadScript: %8.8x\n", datatop );

   // Determine if this asset is a BP GCL file.
   bpFourCC = GCL_GetLong(datatop);

   if( bpFourCC == 'BGCL' )
   {
      is_bp_gcl = 1;
      datatop += 4;
   }
   else
   {
      is_bp_gcl = 0;
      if( bpFourCC != 'BGCL' )
         BP_TODO_BREAK; //TODO: convert this script file!
   }

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

      if( is_bp_gcl )
         cfr->bp_string_data = fres_top + GCL_GetLong(fres_top + 16);
      else
         cfr->bp_string_data = NULL;

	}
	cur->proc_body = next + sizeof( int );
	cur->script_body = next + sizeof( int ) + GCL_GetLong( next ) + sizeof( int );

	GCL_DEBUG_Reset();

	return 0;
}

char *GCL_GetFontDataTop( void )
{
	return current_strres.font_data + sizeof( int );
}

static void DumpScript( GCL_SCRIPT const *script )
{
   printf( "%8.8x %d %8.8x %8.8x", script->proc_table, script->proc_num, script->proc_body, script->script_body );
}

void GCL_SaveCurrentEnvironment( GCL_ENVIRONMENT *gclenv )
{
   printf( "GCL_SaveCurrent: ");
   DumpScript( &current_script );
   printf( "\n" );

	gclenv->script = current_script;
	gclenv->res = current_strres;
}

void GCL_RestoreEnvironment( GCL_ENVIRONMENT *gclenv )
{
   printf( "GCL_Restore: cnt: ");
   DumpScript( &current_script );

   current_script = gclenv->script;
	current_strres = gclenv->res;

   printf( " new " );
   DumpScript( &current_script );
   printf( "\n" );
}

/* ------------------ スクリプトの実行 -------------------*/

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
			GCL_Status = GCL_Expr( next, NULL );
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

int GCL_ExecBlock( char *top, GCL_ARGS *args )
{
	if( GCL_ExecBlockBody( top, args, 0 ) == GCL_RETURN ){
		return GCL_Status;
	}
	GCL_Status = 0;
	return 0;	// デフォルトの値は０
}

int GCL_ExecProcBlock( char *top, GCL_ARGS *args, int local_num )
{
	if( GCL_ExecBlockBody( top, args, local_num ) == GCL_RETURN ){
		return GCL_Status;
	}
	GCL_Status = 0;
	return 0;	// デフォルトの値は０
}

void GCL_ExecScript( void )
{
	unsigned char *datatop;
	unsigned char *next;
	int size;
	datatop = current_script.script_body;
#ifdef KP_WINDOWS
	if( !datatop )
	{
		dbgErrMessPuts("Script is Empty!!\n", 0) ;
		return ;
	}
#endif
D( printf( "datatop = %X\n", datatop ) );
	if( GCL_TAG( *datatop ) != GCL_BLOCK ){
		printf( "NOT SCRIPT DATA %X !!\n", *datatop );
		HANGUP();
	}
	next = GCL_GetBlockSize( datatop, &size );
	GCL_ExecBlock( next, &gcl_null_args );
}



