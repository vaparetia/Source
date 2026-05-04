//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	resource.c
	常駐リソース管理ルーチン

	2000/10/14	K.Uehara
	$Id: resource.c,v 1.1.1.3 2002/11/19 11:41:55 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include "BP_LocalizedTextByEnum.h"

#define MAX_RESOURCES 16

typedef struct {
	short ref_id;		// 参照用ID
	short res_id;		// リソースID
} RESIDENT_RESOURCE_TABLE;

static RESIDENT_RESOURCE_TABLE table[ MAX_RESOURCES ];

GCL_STRING_RESOURCE *gGlobalResInfo2;
static int resource_num = 0;

void GM_InitResource( void )
{
	resource_num = 0;	// 初期設定
}

void GM_SetResourceInfo( GCL_STRING_RESOURCE *res )
{
	gGlobalResInfo2 = res;
}

void GM_SetResourceId( int ref_id, int res_id )
{
	int n = resource_num;

	table[ n ].ref_id = ref_id;
	table[ n ].res_id = res_id;

	resource_num ++;
	ASSERT( resource_num < MAX_RESOURCES );
}

void *GM_GetResource( int ref_id, int offset )
{
   if( offset < BP_LOCALIZEDTEXT_ENUM_START )
   {
      int i;

      for( i = 0; i < resource_num; i++ ){
         if( table[ i ].ref_id == ref_id ){
            int ofs;
            int id;
            id = table[ i ].res_id + offset;

            ofs = GCL_GetLong( ( char * )( gGlobalResInfo2->resource_table + id ) );
            return gGlobalResInfo2->string_table + ( ofs & 0x7FFFFFFF );
         }
      }
      ASSERT( FALSE );
      return NULL;
   }
   else
   {
      return BP_GetStringForEnum(offset);
   }
}
