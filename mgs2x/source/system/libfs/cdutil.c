//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cdutil.c
		CD系のユーティリティ
	現在はXBOXのみで使用。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <libcdvd.h>
#include <sifrpc.h>
#include <sifdev.h>

#endif

#include "libgv.h"
#include "libfs.h"
#include "cdbios.h"

extern void fs_str_toupper( char *str );

#ifdef PSX2
#define CD_PATH		"cdrom0:\\MGS2\\"
#define HD_PATH		"pfs0:/mgs2/"
#define DEV_PATH	"host0:./"
#endif

#ifdef KP_XBOX
#define CD_PATH		"D:\\MGS2\\"
#define HD_PATH		"D:\\"
#define DEV_PATH	".\\"
#define fs_str_toupper( _a )
#endif

void FS_CdFileSetup( FS_FILE_INFO *finfo )
{
#if 0 //BP_PS2
printf( "CdFileSetup\n" );
	for( ; finfo->name != NULL; finfo ++ ){
		char buf[ 64 ];

		switch( finfo->mode ){
		  case FS_FILE_MODE_CD:
			sprintf( buf, "%s%s;1", CD_PATH, finfo->name );
			fs_str_toupper( buf + 7 );		// cdrom0は小文字
#ifdef PSX2
			buf[ 5 ] = '0' + finfo->layer;
#endif
			break;
		  case FS_FILE_MODE_HDU:
			sprintf( buf, "%s%s", HD_PATH, finfo->name );
			break;
		  case FS_FILE_MODE_DEV:
			sprintf( buf, "%s%s", DEV_PATH, finfo->name );
			break;
		}
		finfo->pos = cdbios_get_filepos( buf );
printf( "FILE %12s POS %08X\n", finfo->name, finfo->pos );
	}
#endif
}

