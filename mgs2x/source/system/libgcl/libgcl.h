/*
	GCL ライブラリヘッダ
	1999/07/08 K.Uehara
	$Id: libgcl.h,v 1.1.1.3 2002/11/19 11:42:42 Yoshizawa1 Exp $
*/

#ifndef __LIBGCL__H__
#define __LIBGCL__H__

#include "libgv.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------*/
/*
   コンフィグレーション
*/

extern int GCL_VariableVersion;			// variable.symから来るバージョン

#define GCL_SCENERIO		1574688			// デフォルトで読み込まれるシナリオ

/* 変数領域サイズ */

#define MAX_VAR_BUF			( 7 * 1024 )	// 通常変数
#define MAX_LOCAL_VAR_BUF	( 512 )			// ローカル変数

/* パースコンフィグレーション */

#define GCL_MAX_COMMAND_NEST	24			// コマンドライン先頭のスタック
#define GCL_MAX_ARGS_BUF		32			// 引数バッファのサイズ

/* 動作の変更 */

//#define NO_VARIABLE_SHIFT			// 変数領域のシフトを行わない（領域サイズが減る)
//#define STRCODE16					// 16ビット文字列IDにする
//#define UNUSE_PROC_SELECT			// PROC SELECTを使用しない

#ifdef DEBUG_MODE
#define GCL_DEBUG_MODE 1
#define GCL_DEBUG_MAX_CALL_STACK 64		// PROC, BLOCKのコールスタック記録用
#endif

// リンク変数は, linkvar.hで宣言。サイズは MAX_LINKVARBUF

/*----------------------------------------------------------------*/

/* ---- 環境 ----- */

/* スクリプト管理テーブル */

typedef struct {
	char *proc_table;
	int proc_num;
	char *proc_body;
	char *script_body;
} GCL_SCRIPT;

/* フォント管理テーブル */

typedef struct {
	char *block_top;
	int *resource_table;
	char *string_table;
	char *font_data;
   char *bp_string_data;
} GCL_STRING_RESOURCE;

/* 一時的に環境を保存する */

typedef struct {
	GCL_SCRIPT			script;
	GCL_STRING_RESOURCE res;
} GCL_ENVIRONMENT;

/* 解釈時に使用するワーク */

typedef struct {
	char *next_str_ptr;		// 次に解釈すべきコード
	int status;				// コマンド終了コード
#ifndef UNUSE_PROC_SELECT
	int proc_select_no;		// 言語対応
#endif
	// コマンドラインスタック
	char **commandline_p;
	char *commandlines[ GCL_MAX_COMMAND_NEST ];
	// 引数＆ローカル変数スタック
	int *argstack_p;
	int argbuffer[ GCL_MAX_ARGS_BUF ];
#ifdef GCL_DEBUG_MODE
	int block_nest;
	int *call_stack_p;
	int call_stack[ GCL_DEBUG_MAX_CALL_STACK ];
#endif
} GCL_WORK_T;

extern GCL_WORK_T gcl_work;		// in gcl_init.c
#define GCL_WORK (gcl_work)

/* ---- 構造体 ---- */

typedef int GCL_COMMANDFUNC( char *top );

#define GCL_OK		0
#define GCL_RETURN	1
#define GCL_ERROR	-1

typedef struct {
	int id;
	GCL_COMMANDFUNC	*func;
} GCL_COMMANDLIST;

typedef struct _gcl_commanddef {
	struct _gcl_commanddef *next;
	int n_commlist;
	GCL_COMMANDLIST *commlist;
} GCL_COMMANDDEF;

#define GCL_COMMANDS( a )	{ NULL, sizeof( a ) / sizeof( GCL_COMMANDLIST ), a }

typedef struct {
	u_short argc;
	int *argv;	/* 新バージョンでは引数リストは int の配列 */
} GCL_ARGS;

typedef struct _gcl_var_ref {
	unsigned char code[4];
	unsigned short max;
	unsigned short offset;
} GCL_VAR_REF;

/* ----- 型 ------ */

#define GCL_END		0x00

#define GCL_SHORT	0x01		/* 16ビット符合付き整数 */
#define GCL_BYTE	0x02		/* 8ビット符合なし整数 */
#define GCL_CHAR	0x03		/* 8ビット文字 */
#define GCL_BOOL	0x04		/* 1ビットフラグ */
#define GCL_VECTOR	0x05		/* 32ビット * 3 ベクトル */
#define GCL_STRID	0x06		/* 24ビット文字列ID */
#define GCL_STRING	0x07		/* 文字列。パスカル風ストリング */
#define GCL_PROCID	0x08		/* 関数ID */
#define GCL_INT		0x09		/* 32ビット符合付き整数 */
#define GCL_SYMBOL	0x0a		/* 外部ファイル解決シンボル（int） */
#define GCL_SYMBOL2	0x0b		/* t:シンボルで解決できないときは-1 */
#define GCL_SYMBOL3	0x0c		/* t:シンボルで解決できないときはERROR */
#define GCL_STRID2	0x0d		/* 24ビット文字列ID + 8bit NUM*/
#define GCL_STRRES	0x0e		/* リソース参照文字列 */

#define GCL_TYPEMASK	0x0f	/* タイプ情報を取得するためのマスク */
#define GCL_TYPE( a )	( (a) & GCL_TYPEMASK )

#define GCL_CONST	0x00		/* 定数 */
#define GCL_VAR		0x10		/* 変数 (このあとに24ビットでオフセットがつく) */
#define GCL_ARRAY	0x20		/* 配列 */

#define GCL_EXPR	0x30		/* 式 */
//#define GCL_OP		0x31		/* 演算子 */

#define GCL_ARG		0x40		/* 引数文字 */
#define GCL_OPTION	0x50		/* オプション文字 */
#define GCL_COMMAND	0x60		/* コマンド */
#define GCL_PROC	0x70		/* ユーザー関数 */
#define GCL_BLOCK	0x80		/* 実行ブロック */
#define GCL_LOCAL	0x90		/* ローカル変数 : 0x90そのものは拡張プリフィックス */

#define GCL_VALUE	0xC0		/* 小さい値のヘッダ */
#define GCL_OP		0xA0		/* 演算子コード */
#define GCL_OP_MASK	0xE0		/* 演算子マスク */

#define GCL_WILD	0xF0		/* ワイルドカード文字 (フォーマット用) */
#define GCL_REPEAT	0xE0		/* 繰り返し (フォーマット用) */

#define GCL_TAGMASK	0xF0		/* TAG情報を取得するためのマスク */
#define GCL_TAG( a )	( (a) & GCL_TAGMASK )

/*
   変数の情報フォーマット
   変数は基本的にbyte, shortのみ

   情報は24ビットで、
   ( type << 20 ) | ( bit << 16 ) | ( offset );
   typeはグローバル、ローカル、リンク
*/

#define GCL_VAR_TYPE_GLOBAL	0x000000		/* 通常変数 */
#define GCL_VAR_TYPE_LOCAL	0x100000		/* 未使用 */
#define GCL_VAR_TYPE_SYSTEM	0x400000		/* システム変数 */
#define GCL_VAR_TYPE_LINK	0x800000		/* リンク用領域 */

/* 組み込み関数 */

EXTERN_INLINE int GCL_GetVarRefType( GCL_VAR_REF *ref )
{
	return ref->code[ 0 ];
}

EXTERN_INLINE int GCL_GetLong( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	return ( p[ 3 ] << 24 ) | ( p[ 2 ] << 16 ) | ( p[ 1 ] << 8 ) | ( p[ 0 ] );
}

EXTERN_INLINE int GCL_GetVarCode( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	/* なぜかBigEndian */
	return ( p[ 0 ] << 24 ) | ( p[ 1 ] << 16 ) | ( p[ 2 ] << 8 ) | ( p[ 3 ] );
}

EXTERN_INLINE int GCL_Get3Bytes( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	return ( ( p[ 2 ] << 16 ) | ( p[ 1 ] << 8 ) | ( p[ 0 ] ) );
}

EXTERN_INLINE int GCL_GetShort( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	return ( signed short )( ( p[ 1 ] << 8 ) | ( p[ 0 ] ) );
}

EXTERN_INLINE int GCL_GetUShort( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	return ( unsigned short )( ( p[ 1 ] << 8 ) | ( p[ 0 ] ) );
}

EXTERN_INLINE unsigned char GCL_GetByte( char *ptr )
{
	return *( unsigned char * )ptr;
}

/* 文字列IDのバイト数が変わったらここを変更 */

#ifdef STRCODE16
// 16ビット文字列ID

#define STRCODE_SIZE	2		/* 文字列IDは３バイト */

EXTERN_INLINE int GCL_GetStrCode( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	return ( ( p[ 1 ] << 8 ) | ( p[ 0 ] ) );
}

#else
// 24ビット文字列ID

#define STRCODE_SIZE	3		/* 文字列IDは３バイト */

EXTERN_INLINE int GCL_GetStrCode( char *ptr )
{
	unsigned char *p;
	p = ( unsigned char * )ptr;
	return ( ( p[ 2 ] << 16 ) | ( p[ 1 ] << 8 ) | ( p[ 0 ] ) );
}

#endif // STRCODE16

#define GCL_MAX_ARGS		8
#define GCL_PROC_MAX_ARGS	16

#define GCL_VAR_BUF

/* ------------------------------------ */
/* デバッグ用 */
#ifdef DEBUG_MODE

#define GCL_ASSERT( c )	if ( !(c) ) GCL_Assert( __FILE__, __LINE__ ) ;

void GCL_Assert( char *file, int line );

extern int GCL_DEBUG_CurrentProcID;
extern int GCL_DEBUG_CurrentCommandID;

#else
#define GCL_ASSERT( c )		ASSERT( c )
#endif

/* ------------------------------------ */

extern GCL_ARGS	gcl_null_args;	// 定数: in command.c

// 後方互換のため、defineしておく。
/* 最後に実行したブロックの返り値 in command.c */
#define GCL_Status		GCL_WORK.status

/* 次の値を取得するポインタ */
#define GCL_NextStrPtr	GCL_WORK.next_str_ptr

/* ------------------------------------ */

/* gcl_init.c */

void GCL_Initialize( void );
void GCL_ResetSystem( void );
void GCL_ChangeSenerioCode( int code );

/* command.c */

void GCL_ResetCommList( void );
int GCL_AddCommMulti( GCL_COMMANDDEF *def );
int GCL_DelCommMulti( GCL_COMMANDDEF *def );
int GCL_LoadScript( char *datatop );

void GCL_SetProcSelectNo( int no );
void GCL_ExecScript( void );
int GCL_ExecProc( int proc_id, GCL_ARGS *arg );

int GCL_ExecBlock( char *top, GCL_ARGS *args );
int GCL_ExecBlockBody( char *top, GCL_ARGS *args, int local_num );
int GCL_ExecProcBlock( char *top, GCL_ARGS *args, int local_num );

char *GCL_GetStringResource( int id );
char *GCL_LookupBPStringByKPString(char* pKPString);
char *GCL_GetFontDataTop( void );
void GCL_SaveCurrentEnvironment( GCL_ENVIRONMENT *gclenv );
void GCL_RestoreEnvironment( GCL_ENVIRONMENT *gclenv );

void *GCL_SearchId( void *buffer, unsigned int id, int n );

/* basic.c */

void GCL_InitBasicCommands( void );

/* parse.c */

void GCL_ParseInit( void );

char *GCL_GetNextValue( char *top, int *type_p, int *value_p );

void GCL_SetArgTop( char *top );

void *GCL_SetArgStack( GCL_ARGS *args, int local_args_num );
void GCL_UnsetArgStack( void *stack );
int GCL_GetArgs( int argno );
int GCL_GetLocalArgs( int argno );
void GCL_SetLocalArgs( int argno, int value );

void GCL_SetCommandLine( char *argtop );
void GCL_UnsetCommandLine( void );

char *GCL_GetOption( char c );

unsigned char *GCL_GetBlockSize( unsigned char *top, int *size );
unsigned char *GCL_GetShortSize( unsigned char *top, int *size );

// 値取得系
int GCL_GetInt( char *ptr );
int GCL_GetIV( char *ptr, int *vec );
int GCL_GetSV( char *ptr, short *vec );
char *GCL_GetString( char *ptr );
void *GCL_GetResource_LE( char *ptr );
char *GCL_SkipCommand( char *ptr );

//BP_CAMERA - functions to access command bytes
void GCL_GetCommandBytes( const char** start, const char** end );
void GCL_GetOptionBytes( const char** start, const char** end );
//BP_CAMERA - functions to access command bytes

char *GCL_NextStr( void );
#define GCL_SetNextStr( _ptr )	GCL_SetArgTop( _ptr )

// コンビニエンス(こちらを使うことを推奨)
int GCL_GetNextInt( void );
int GCL_GetNextIV( int *vec );
int GCL_GetNextSV( short *vec );
char *GCL_GetNextString( void );
void *GCL_GetNextResource_LE( void );
void GCL_GetNextVarRef( GCL_VAR_REF *ref );
int GCL_GetNextResourceNo( void );

int GCL_GetOptionValue( char c, int default_value );

int GCL_GetNextOption( void );

/* expr.c */

int GCL_Expr( char *data, int *value );

/* variable.c */

void GCL_InitVar( void );
void GCL_InitClearVar( void );

char *GCL_GetVar( char *top, int *type_p, int *value_p );
char *GCL_SetVar( char *top, int value );

char *GCL_GetVarRef( char *top, GCL_VAR_REF *ref );
void GCL_SetVarRef( GCL_VAR_REF *ref, int offset, int value );
int GCL_ReadVarRef( GCL_VAR_REF *ref, int offset );

void GCL_SaveVar( void );
void GCL_RestoreVar( void );
void GCL_ResetLocalVar( void );
void GCL_SaveLinkVar( void *ptr, int size ) ;
void *GCL_GetSavedLinkVar( void *ptr ) ;

int GCL_ReadSavedVar( char *top );
char *GCL_VarSaveBuffer( char *top );

void *GCL_GetLinkvarSaveAreaTop( void );
void *GCL_GetVarSaveAreaTop( void );

/* 旧バージョンとの互換 */

#define GCL_ARG_GLOBAL	&( gcl_null_args )

#ifdef __cplusplus
}
#endif

#endif

#if 0
#define D( a )	a
#else
#define D( a )
#endif
