//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	resman.c
		ローカルリソース管理

	2001/06/12 K.Kano
	$Id: resman.c,v 1.2 2002/12/05 18:42:00 takaki Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include "font.h"


#if 0 //多言語対応

#ifdef NTSC
/* 日本、アメリカ用 */
#define MAX_RESOURCES 32
#endif

#ifdef AREA_EU_BP_IGNORE	//	#ifdef PAL
/* ヨーロッパ用 */
#define MAX_RESOURCES 128
#endif

#else
/* 多言語 */
#define MAX_RESOURCES 128
#endif


typedef struct {
	int ref_id;			// 参照用ID
	short res_id;		// リソースID
} RESOURCE_TABLE;

static RESOURCE_TABLE table[ MAX_RESOURCES ];

GCL_STRING_RESOURCE *gGlobalResInfo=NULL;
static int resource_num = 0;

void BP_ClearLocalResourceInfo()
{
   //Reset ptr at the start of every stage because the script data to which it pointed before
   //is now something else.  ComLocalResourceSet() will be used in the stage's script
   //to set it valid if appropriate.
   gGlobalResInfo = NULL;
}

static void SetResourceInfo( GCL_STRING_RESOURCE *res )
{
	if(gGlobalResInfo==NULL) gGlobalResInfo = res;
}

static void SetResourceId( int ref_id, int res_id )
{
	int n=resource_num;
	int i;

	/* 重複チェック */
	for(i=0;i<n;i++){
		// printf("Check : res_id , ref_id = %d , %d\n",table[i].res_id,table[i].ref_id);

		if(table[i].ref_id==0 || table[i].ref_id==1){
			/* 0と1は、StrCodeでは存在しない数値なので、上書きする
			   リスタート対策！ */

			table[i].ref_id=ref_id;
			table[i].res_id=res_id;
			return;
		}
		if(table[i].ref_id==ref_id){
			table[i].res_id=res_id;

			// printf("Overwrite : res_id , ref_id = %d , %d\n",res_id,ref_id);

			return;
		}
	}

	table[n].ref_id = ref_id;
	table[n].res_id = res_id;

	// printf("res_id , ref_id = %d , %d\n",res_id,ref_id);

	resource_num ++;
	ASSERT( resource_num < MAX_RESOURCES );
}

void *GetLocalResource( int ref_id, int offset )
{
	int i;

	// printf("Get : ref_id = %d,%d\n",ref_id,offset);

	for( i = 0; i < resource_num; i++ ){
		if( table[ i ].ref_id == ref_id ){
			int ofs;
			int id;
			id = table[ i ].res_id + offset;

			ofs = GCL_GetLong( ( char * )( gGlobalResInfo->resource_table + id ) );
			return gGlobalResInfo->string_table + ( ofs & 0x7FFFFFFF );
		}
	}
	ASSERT( FALSE );
	return NULL;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


#if 0

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

#endif

static void set_gcl_param( void )
{
	while( GCL_NextStr() != NULL ){
		int ref_id, res_id;

		ref_id = GCL_GetNextInt();
		res_id = GCL_GetNextResourceNo();

		SetResourceId( ref_id, res_id );
	}
}

int ComLocalResourceSet( void )
{
	static GCL_ENVIRONMENT gclenv;
	GCL_STRING_RESOURCE *src;		// コピー元
	GCL_STRING_RESOURCE *dest;			// コピー先
	// unsigned char *fres_top;


	/* リソースを常駐部に転送する */

	/* リソース情報の取得 */
	GCL_SaveCurrentEnvironment( &gclenv );
	src = &gclenv.res;

#if 0
	/* リソース情報を常駐領域に転送 */
	{
		int size;

		size = GCL_GetLong( src->block_top );
printf( "RESOURCE SIZE = %X\n", size );

		dest = GV_Malloc(size);	// ID = 0 で初期化

		memcpy( dest, src->block_top, size );
	}

	/* ヘッダ部分の初期化 */
	{
		fres_top = ( unsigned char * )dest;

		printf("%d %d %d %d\n",
			   dest->block_top,dest->resource_table,dest->string_table,dest->font_data);

		dest->block_top = ( char * )dest;
		dest->resource_table = ( int * )( GCL_GetLong( fres_top + 4 ) + fres_top );
		dest->string_table = GCL_GetLong( fres_top + 8 ) + fres_top;
		dest->font_data = GCL_GetLong( fres_top + 12 ) + fres_top;
	}
#else
	dest=src;
#endif


#if 0
	/* 文字列コードの再変換 ( font.h を参照 )*/
	{
		int *ofs;
		int i, num;

		num = ( GCL_GetLong( fres_top + 8 ) - GCL_GetLong( fres_top + 4 ) ) / sizeof( int );
		ofs = dest->resource_table;

		for( i = 0; i < num; i++ ){
			if( *ofs & 0x80000000 ){
				/* 最上位が立っているものは文字列 */
				change_font_code( dest->string_table + ( *ofs & 0x7FFFFFFF ) );
			}
			ofs ++;
		}
	}
#endif

#if 0
	/* 常駐部のフォントをMENUフォントとして設定 */
	/* 先頭はフォントサイズ */
	font_set_top_addr( FONT_TYPE_MENU, dest->font_data + sizeof( int ) );
#endif

	/* 常駐リソース管理関数に登録 */

	SetResourceInfo( dest );

	if( GCL_GetOption( 's' ) != NULL ){
		set_gcl_param();
	}
	if( GCL_GetOption( 'b' ) != NULL ){
		set_gcl_param();
	}
	
	return 0;
}
