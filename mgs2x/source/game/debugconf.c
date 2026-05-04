//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	debugmenu.c
		デバッグメニューコンフィグファイル読み込み

	2001/03/13	K.Uehara
	$Id: debugconf.c,v 1.1.1.3 2002/11/19 11:41:47 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <sifdev.h>
#include <eeregs.h>
#include <libgraph.h>
#endif

#include "libgv.h"
#include "libfs.h"
#include "libdg.h"
#include "libgcl.h"
#include "libdg.cnf"

#include "game.h"

#define _DEBUG_MENU_BODY_

#include "debugmenu.h"

/* ---------------------------------------------------------------------- */
/*
	初期化テーブル読み込み
		HDDからブートされた時のみ cdrom.img/debug.cnfから値を読み込む
*/

#define MAX_TABLES 256

static GM_DEBUG_MENU_TABLES tables[ MAX_TABLES ];
static int tables_p = 0;

static int Getc( int fd )
{
	u_char buf[ 2 ];

#ifdef PSX2
	if( pcRead( fd, buf, 1 ) < 1 ){
		return -1;
	}
#else
	return -1; /*まだです2002.02.01*/
#endif

	return buf[ 0 ];
}

static int get_line( int fd, char *linebuf )
{
	char *p = linebuf;
	for( ;; ){
		int c;

		c = Getc( fd );
		if( c == -1 ){
			return 0;
		}
		if( c == '\r' || c == '\n' ){
			break;
		}
		*( p ++ ) = c;
	}
	*( p ++ ) = '\0';

	return 1;
}

static char *get_word( char *buf, char *src )
{
	char *p = buf;

	while( isspace( *src ) && *src != '\0' ){
		src ++;
	}
	if( *src == '\0' ) return NULL;

	if( *src == '\"' ){
		src ++;
		while( *src != '\"' && *src != '\0' ){
			*( p ++ ) = *( src ++ );
		}
		src ++;
	} else {
		while( !isspace( *src ) && *src != '\0' ){
			*( p ++ ) = *( src ++ );
		}
	}
	*p = '\0';

	return src;
}

void GM_LoadDebugMenuConf( void )
{
	int fd;
	char linebuf[ 256 ];

#ifdef PSX2
	if( ( fd = pcOpen( "host0:./debug.cnf", SCE_RDONLY ) ) < 0 ){
		printf( "NO READ DEBUG CONF\n" );
		return;
	}
#else
	return ; /*まだです2002.02.01*/
#endif

	tables_p = 0;

	while( get_line( fd, linebuf ) ){
		char *p;
		char buf[ 128 ];
		int key1, key2, key, value;

		p = linebuf;
		if( ( p = get_word( buf, p ) ) == NULL ){
			continue;
		}
		if( buf[ 0 ] == '#' || buf[ 0 ] == ';' ){
			// コメント
			continue;
		}

		key1 = GV_StrCode( buf );
		if( ( p = get_word( buf, p ) ) == NULL ){
			printf( "debug.cnf:NO CLASS\n" );
			return;
		}
		key2 = GV_StrCode( buf );
		if( ( p = get_word( buf, p ) ) == NULL ){
			printf( "debug.cnf:NO VALUE\n" );
			return;
		}

		key = ( ( key1 + key2 ) & 0x00FFFFFF );
		value = strtol( buf, NULL, 0 );

		tables[ tables_p ].value = value;
		tables[ tables_p ].strid = key;
		tables_p++;
		if( tables_p >= MAX_TABLES ){
			printf( "TABLE OVER\n" );
			break;
		}
	}
	{
		void *tp;
		int size;
		size = tables_p * sizeof( GM_DEBUG_MENU_TABLES );
		tp = GV_AllocResidentMemory( size, 0 );
		memcpy( tp, tables, size );
		GM_SetDebugMenuTables( tp, tables_p );
	}
}
