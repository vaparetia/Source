//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   パーサー
	1999/07/08 K.Uehara
	$Id: parse.c,v 1.1.1.3 2002/11/19 11:42:42 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	"libgcl.h"


/* 可変長ブロックサイズ取得 */
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
		*size = GCL_GetUShort( top + 1 );
		return top + 1 + 2;
	  case 0x0F:
		*size = GCL_Get3Bytes( top + 1 );
		return top + 1 + 3;
	  default:
		*size = code;
		return top + 1;
	}
}

/* 最大２バイトのサイズ取得 */
unsigned char *GCL_GetShortSize( unsigned char *top, int *size )
{
	if( *top & 0x80 ){
		*size = ( GCL_GetUShort( top ) & 0x7FFF );
		return top + 2;
	} else {
		*size = *top;
		return top + 1;
	}
}

/* ---------------------------------------------------------------------- */
/*
	GCL値取得ルーチン
*/
//BP_CAMERA - extra vars to track script parsing
static char* gBP_ScriptEndValue = NULL;   // Tracks end value
static int   gBP_ScriptDynamicValue = 0;  // Tracks whether value is dynamic variable
//BP_CAMERA - extra vars to track script parsing

char *GCL_GetNextValue( char *top, int *type_p, int *value_p )
{
	int type, tag;
	unsigned char *p;

	p = ( unsigned char * )top;
	type = *p;

	tag = GCL_TAG( type );

	if( ( tag & GCL_VALUE ) == GCL_VALUE ){
		/* MINIMUM VALUE */
		*type_p = GCL_INT;
		*value_p = ( *p & ~GCL_VALUE ) - 1;
		p ++;
	} else if( tag == GCL_CONST ){
		/* 値設定系 */
		*type_p = type;
		p++;

      //BP_CAMERA - track dynamic values that maybe different between languages
      switch( type )
      {
      case GCL_PROCID:
      case GCL_STRID2:
      case GCL_SYMBOL:
      case GCL_STRID:
      case GCL_STRING:
      case GCL_STRRES:
         gBP_ScriptDynamicValue = 1;
         break;
      }
      //BP_CAMERA - track dynamic values that maybe different between languages

		switch( type ){
		  case GCL_END:
        gBP_ScriptEndValue = p;//BP_CAMERA - track end value
			p = NULL;
			break;
		  case GCL_SHORT:
			/* short value */
			*value_p = GCL_GetShort( p );
			p += 2;
			break;
		  case GCL_STRID2:
		  case GCL_INT:
		  case GCL_SYMBOL:
			/* int value */
			*value_p = GCL_GetLong( p );
			p += 4;
			break;
		  case GCL_STRID:
		  case GCL_PROCID:
			/* 24Bit value */
			*value_p = GCL_GetStrCode( p );
			p += STRCODE_SIZE;
			break;
		  case GCL_BYTE:
		  case GCL_BOOL:
		  case GCL_CHAR:
			/* byte value */
			*value_p = ( unsigned char )GCL_GetByte( p );
			p++;
			break;
		  case GCL_STRING:
			/* string value */
			*value_p = ( int )( p + 1 );	/* 文字列の開始アドレス */
			p += *p + 1;
			break;
		  case GCL_STRRES:
			/* string resource */
			*value_p = ( int )GCL_GetStringResource( GCL_GetUShort( p ) );
			/* 文字列の開始アドレス */
			*type_p = GCL_STRING;			/* 文字列扱い */
			p += 2;
			break;
		  default:
			printf( "GCL:WRONG CODE %x\n", type );
			HANGUP();
		}
	} else {
		/* 制御構造系 */
		int size;
		unsigned char *next;

		*type_p = tag;

      //BP_CAMERA - track dynamic values that maybe different between languages
      switch( tag )
      {
      case GCL_VAR:
      case GCL_ARRAY:
         gBP_ScriptDynamicValue = 1;
         break;
      }
      //BP_CAMERA - track dynamic values that maybe different between languages

		switch( tag ){
		  case GCL_VAR:
		  case GCL_ARRAY:
			// 変数・配列
			return GCL_GetVar( top, type_p, value_p );

		  case GCL_ARG:
			// 引数
			if( ( *p & 0x0F ) == 0x0F ){
				*value_p = GCL_GetArgs( *( p + 1 ) + 0x0F );
				p++;
			} else {
				*value_p = GCL_GetArgs( *p & 0x0F );
			}
			*type_p = GCL_INT;
			p ++;
			break;
		  case GCL_LOCAL:
			// ローカル変数
			*value_p = GCL_GetLocalArgs( *p & 0x0F );
			*type_p = GCL_INT;
			p ++;
			break;

		  case GCL_BLOCK:
			// 実行ブロック
			next = GCL_GetBlockSize( p, &size );
			*value_p = ( int )next;				// アドレスを返す
			p = next + size;
			break;
		  case GCL_EXPR:
			// 式
			next = GCL_GetBlockSize( p, &size );
			*value_p = GCL_Expr( next, NULL );
			p = next + size;
			break;
		  case GCL_OPTION:
			// オプション
			next = GCL_GetBlockSize( p, &size );
			*type_p |= ( *next << 16 );			// オプション文字
			*value_p = ( int )( next + 1 );		// アドレスを返す
			p = next + size;
			break;
		  default:
			printf( "GCL:WRONG CODE %x\n", type );
			HANGUP();
		}
	}
	return p;
}

/* ---------------------------------------------------------------------- */
/*
	PROC, BLOCKの呼出し引数処理
*/

static void GCL_InitArgStack( void )
{
	GCL_WORK.argstack_p = GCL_WORK.argbuffer;
}

void *GCL_SetArgStack( GCL_ARGS *args, int local_args_num )
{
	int *org;

	if( args == NULL ){
		return NULL;
	}
D( printf( "set args %d\n", args->argc ); );
	org = GCL_WORK.argstack_p;

	GCL_WORK.argstack_p = GCL_WORK.argstack_p + local_args_num;
	*GCL_WORK.argstack_p = ( int )args;
	GCL_WORK.argstack_p++;

	GCL_ASSERT( GCL_WORK.argstack_p <= GCL_WORK.argbuffer + GCL_MAX_ARGS_BUF );

	return org;
}

void GCL_UnsetArgStack( void *stack )
{
	if( stack != NULL ){
		GCL_WORK.argstack_p = stack;
	}
}

int GCL_GetArgs( int argno )
{
D( printf( "GetArgs %d Value %d Total %d\n", argno, argstack_p[ -1 - argno ], argstack_p[ -1 ] ); );
	if( argno == 0 ){
//printf( "GET STATUS %d\n", GCL_Status );
		return GCL_Status;
	}
	GCL_ASSERT( GCL_WORK.argstack_p > GCL_WORK.argbuffer );
	GCL_ASSERT( argno <= ( ( GCL_ARGS * )GCL_WORK.argstack_p[ -1 ] )->argc );

	return ( ( GCL_ARGS * )GCL_WORK.argstack_p[ -1 ] )->argv[ argno - 1 ];
}

int GCL_GetLocalArgs( int argno )
{
	GCL_ASSERT( GCL_WORK.argstack_p > GCL_WORK.argbuffer );

	return *( GCL_WORK.argstack_p - 1 - argno );
}

void GCL_SetLocalArgs( int argno, int value )
{
	GCL_ASSERT( GCL_WORK.argstack_p > GCL_WORK.argbuffer );

	*( GCL_WORK.argstack_p - 1 - argno ) = value;
}

/* ---------------------------------------------------------------------- */
/*
	コマンドラインの先頭を記録するスタック
*/

static void GCL_InitCommandLineBuffer( void )
{
	GCL_WORK.commandline_p = GCL_WORK.commandlines;
}

void GCL_SetCommandLine( char *argtop )
{
	*( GCL_WORK.commandline_p ++ ) = argtop;
	GCL_ASSERT( GCL_WORK.commandline_p <= GCL_WORK.commandlines + GCL_MAX_COMMAND_NEST );
}

void GCL_UnsetCommandLine( void )
{
	GCL_WORK.commandline_p --;
	GCL_ASSERT( GCL_WORK.commandline_p >= GCL_WORK.commandlines );
}

/* ---------------------------------------------------------------------- */
/*
	値取得用ユーティリティ関数
*/

void GCL_SetArgTop( char *top )
{
//printf( "ARGTOP %X\n", top );
	GCL_NextStrPtr = top;
}

char *GCL_GetOption( char c )
{
	char *p;
	p = GCL_WORK.commandline_p[ -1 ];		/* カレント */

	for( ;; ){
		int type, value;

		p = GCL_GetNextValue( p, &type, &value );
		if( type == GCL_END ) return NULL;
		if( GCL_TAG( type ) == GCL_OPTION ){
			if( ( type >> 16 ) == c ){
				GCL_NextStrPtr = ( char * )value;
				return ( char * )value;
			}
		}
	}
}

int GCL_GetNextOption( void )
{
	char *p;

	p = GCL_NextStrPtr;
	if ( p == NULL || *p == GCL_END ) return 0 ;

	for( ;; ){
		int type, value;

		p = GCL_GetNextValue( p, &type, &value );
		if( type == GCL_END ) return 0;

		if( GCL_TAG( type ) == GCL_OPTION ){
			GCL_NextStrPtr = ( char * )value;
			return ( type >> 16 );
		}
	}
}

int GCL_GetInt( char *ptr )
{
	int type, value;

	GCL_ASSERT( ptr != NULL );

	GCL_NextStrPtr = GCL_GetNextValue( ptr, &type, &value );

	return value;
}

int GCL_GetIV( char *ptr, int *vec )
{
	int i, type, value;

	GCL_ASSERT( ptr != NULL );

	for( i = 0; i < 3; i++ ){
		ptr = GCL_GetNextValue( ptr, &type, &value );
		vec[ i ] = value;
	}
	GCL_NextStrPtr = ptr;

	return 0;
}

int GCL_GetSV( char *ptr, short *vec )
{
	int i, type, value;

	GCL_ASSERT( ptr != NULL );

	for( i = 0; i < 3; i++ ){
		ptr = GCL_GetNextValue( ptr, &type, &value );
		vec[ i ] = value;
	}
	GCL_NextStrPtr = ptr;

	return 0;
}

char *GCL_GetString( char *ptr )
{
	int type, value;

	if( ptr == NULL ) return NULL;

	if( ( GCL_NextStrPtr = GCL_GetNextValue( ptr, &type, &value ) ) == NULL ){
		return NULL;
	}
	//GCL_ASSERT( type == GCL_STRING );

	return ( char * )value;
}

void *GCL_GetResource_LE( char *ptr )
{
	return ( void * )GCL_GetString( ptr );
}

int GCL_GetResourceNo( char *ptr )
{
	int no;

	GCL_ASSERT( *ptr == GCL_STRRES );
	no = GCL_GetShort( ptr + 1 );

	GCL_NextStrPtr = ptr + 3;

	return no;
}

void GCL_GetNextVarRef( GCL_VAR_REF *ref )
{
	char *top;
	top = GCL_NextStr();
	GCL_ASSERT( top != NULL );

	GCL_NextStrPtr = GCL_GetVarRef( top, ref );
}

char *GCL_SkipCommand( char *ptr )
{
	for( ;; ){
		int type, value;
		ptr = GCL_GetNextValue( ptr, &type, &value );
		if( type == GCL_END ){
			GCL_NextStrPtr = NULL;
			return ptr;
		}
	}
}

//BP_CAMERA - functions to access command bytes
void GCL_GetCommandBytes( const char** start, const char** end )
{
   // Search for end of line pointer
   char* p = GCL_WORK.commandline_p[ -1 ];
   *start = p;
   gBP_ScriptEndValue = NULL;
   for( ;; )
   {
      // GCL_GetNextValue will set "gBP_ScriptEndValue" at end command
      int type, value;
      p = GCL_GetNextValue( p, &type, &value );
      if( type == GCL_END )
      {
         break;
      }
   }

   // Should always find end value
   XASSERT( gBP_ScriptEndValue != NULL, "Should always find end of script line!" );
   *end = gBP_ScriptEndValue;
}

void GCL_GetOptionBytes( const char** start, const char** end )
{
   static char  gBP_ScriptStaticData[1024];

   char* output = gBP_ScriptStaticData;
   char* arg_curr, *opt_curr;
   char* arg_next, *opt_next;
   int arg_type, arg_value;
   int opt_type, opt_value;

   // Loop over all args
   arg_curr = GCL_WORK.commandline_p[ -1 ];
   while(arg_curr)
   {
      // Is next arg an option?
      arg_next = GCL_GetNextValue( arg_curr, &arg_type, &arg_value );
      if( GCL_TAG(arg_type) == GCL_OPTION )
      {
         // Output static option type
         opt_curr = (char*)arg_value;
         while( arg_curr < opt_curr )
         {
            *output++ = *arg_curr++;
         }

         // Parse option values
         while( (opt_curr) && (opt_curr < arg_next) )
         {
            // Get option value
            gBP_ScriptDynamicValue = 0;
            opt_next = GCL_GetNextValue( opt_curr, &opt_type, &opt_value );

            // Output if static data
            if( (opt_next) && (gBP_ScriptDynamicValue == 0) )
            {
               while( (opt_curr < opt_next) && (opt_curr < arg_next) )
               {
                  *output++ = *opt_curr++;
               }
            }

            // Next option
            opt_curr = opt_next;
         }
      }

      // Next arg
      arg_curr = arg_next;         
   }

   // Set output
   *start = &gBP_ScriptStaticData[0];
   *end   = output - 1;
}
//BP_CAMERA - functions to access command bytes

char *GCL_NextStr( void )
{
	if( GCL_NextStrPtr == NULL ) return NULL;
	if( *GCL_NextStrPtr == GCL_END || GCL_TAG( *GCL_NextStrPtr ) == GCL_OPTION ){
		return NULL;
	}
	return GCL_NextStrPtr;
}

int GCL_GetNextInt( void )
{
	return GCL_GetInt( GCL_NextStr() );
}

int GCL_GetNextIV( int *vec )
{
	return GCL_GetIV( GCL_NextStr(), ( int * )vec );
}

int GCL_GetNextSV( short *vec )
{
	return GCL_GetSV( GCL_NextStr(), ( short * )vec );
}

char *GCL_GetNextString( void )
{
	return GCL_GetString( GCL_NextStr() );
}

void *GCL_GetNextResource_LE( void )
{
	return ( void * )GCL_GetString( GCL_NextStr() );
}

int GCL_GetNextResourceNo( void )
{
	return GCL_GetResourceNo( GCL_NextStr() );
}

int GCL_GetOptionValue( char c, int default_value )
{
	if( GCL_GetOption( c ) != NULL ){
		return GCL_GetInt( GCL_NextStr() );
	} else {
		return default_value;
	}
}

/* ----------------------- initialize ---------------------- */

void GCL_ParseInit( void )
{
	GCL_InitArgStack();
	GCL_InitCommandLineBuffer();
}
