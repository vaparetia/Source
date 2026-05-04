//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gameinit.c
		ゲーム関係のイニシャライズ
		init ステージ読み込み後に、シナリオから呼ばれる

	1999/09/01 K.Uehara
	$Id: gameinit.c,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mgs_type.h"

#ifdef KP_XBOX
#include "mgsx_type.h"
#endif

#include "game.h"
#include "font.h"
#include "libgcl.h"

#include "resource.h"
#include "debugmenu.h"
#include "BP_EndianSupport.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

void	*NewFontInit( int name, int map )
{
	void *font;
	if( ( font = GV_GetCache( GV_CacheID( 3685445/*"keepfont"*/, 'r' ) ) ) != NULL )
   {
		font_resident_load_set( font );
	}

	return font ;
}

void *NewGameInit( int name, int map )
{
	// ゲームライブラリの初期化
	GM_StartMenuPrimManager();

#ifdef DEBUG_MODE

	if( GCL_GetOptionValue( 'd', 1 ) ){
		GM_DebugModeEnable = 1;
		GM_StartDebugViewer();
	} else {
		GM_DebugModeEnable = 0;
	}

#endif

	// フォントデータの初期化
	NewFontInit( name, map ) ;
#if 0
	{
		void *font;
		if( ( font = GV_GetCache( GV_CacheID( 3685445/*"keepfont"*/, 'r' ) ) ) != NULL ){
			font_resident_load_set( font );
		}
	}
#endif

	return ( void * )1;
}

static void change_font_code( char *string )
{
	unsigned char *p = ( unsigned char * )string;

	while( *p != '\0' ){
		if( *p & 0x80 ){
			/* 漢字コードの1バイト目 */
			int code;
			code = ( *p << 8 ) | *( p + 1 );
			if( FONT_TYPE( code & ~FONT_OP_MASK ) == FONT_TYPE_GCL ){
				code = code - FONT_CODE_TOP( FONT_TYPE_GCL ) + FONT_CODE_TOP( FONT_TYPE_MENU );
			}
			*p = code >> 8;
			*( p + 1 ) = ( code & 0xFF );
			p += 2;
		} else {
			p++;
		}
	}
}

static void set_gcl_param( void )
{
	while( GCL_NextStr() != NULL ){
		int ref_id, res_id;

		res_id = GCL_GetNextResourceNo();
		ref_id = GCL_GetNextInt();
		GM_SetResourceId( ref_id, res_id );
	}
}

int NewResidentResourceSet( void )
{
	GCL_ENVIRONMENT gclenv;
	GCL_STRING_RESOURCE *resinfo;		// コピー元
	GCL_STRING_RESOURCE *dest;			// コピー先
	unsigned char *fres_top;


	/* リソースを常駐部に転送する */

	/* リソース情報の取得 */
	GCL_SaveCurrentEnvironment( &gclenv );
	resinfo = &gclenv.res;

	/* リソース情報を常駐領域に転送 */
	{
      int size = GCL_GetLong( resinfo->block_top );
      printf( "RESOURCE SIZE = %X\n", size );
      dest = GV_AllocResidentMemory( sizeof(GCL_STRING_RESOURCE) + size, 0 );	// ID = 0 で初期化

      /* ヘッダ部分の初期化 */
      {
         //fres_top = ( unsigned char * )dest;

         dest->block_top = ((char*)dest) + sizeof(GCL_STRING_RESOURCE);
         dest->resource_table = (int*)(dest->block_top + ((char*)resinfo->resource_table - resinfo->block_top));
         dest->string_table = dest->block_top + ((char*)resinfo->string_table - resinfo->block_top);
         dest->font_data = dest->block_top + ((char*)resinfo->font_data - resinfo->block_top);
         dest->bp_string_data = resinfo->bp_string_data ? (dest->block_top + ((char*)resinfo->bp_string_data - resinfo->block_top)) : NULL;
      }

		memcpy( dest->block_top, resinfo->block_top, size );
	}

	/* 文字列コードの再変換 ( font.h を参照 )*/
	{
		int *ofs;
		int i, num;

      num = ( (int)dest->string_table - (int)dest->resource_table ) / sizeof( int );
		ofs = dest->resource_table;

		for( i = 0; i < num; i++ ){
			if( BP_LE_SwapSInt(*ofs) & 0x80000000 ){
				/* 最上位が立っているものは文字列 */
				change_font_code( dest->string_table + ( BP_LE_SwapSInt(*ofs) & 0x7FFFFFFF ) );
			}
			ofs ++;
		}
	}

	/* 常駐部のフォントをMENUフォントとして設定 */
	/* 先頭はフォントサイズ */
	font_set_top_addr( FONT_TYPE_MENU, dest->font_data + sizeof( int ) );

	/* 常駐リソース管理関数に登録 */

	GM_SetResourceInfo( dest );

	if( GCL_GetOption( 's' ) != NULL ){
		set_gcl_param();
	}
	if( GCL_GetOption( 'b' ) != NULL ){
		set_gcl_param();
	}
	
	return 0;
}

int NewResidentAreaSet( void )
{
	int id, size;

	id = GCL_GetNextInt();
	size = GCL_GetNextInt();

	id = id | ( 0x1F << 24 );	// rawデータとして登録
printf( "ALLOC RESIDENT ID %X SIZE %d\n", id, size );
	GV_AllocResidentMemory( size, id );

#if 1 //def KP_XBOX
	return 0;
#endif	
}
