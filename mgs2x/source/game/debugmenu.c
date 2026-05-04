//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	debugmenu.c
		デバッグメニュー操作

	2001/02/19	K.Uehara
	$Id: debugmenu.c,v 1.1.1.3 2002/11/19 11:41:47 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <string.h>
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
	表示コンフィグ
*/

#define DM_LINE		15
#define DM_X0		16
#define DM_Y0		32

#define CLASS_X		64
#define MENU_X		(CLASS_X+8*8)
#define ITEM_X		(MENU_X+8*16)
#define YSTEP		16

#define FIRST_REPEAT	(10*2)
#define SECOND_REPEAT	(1*2)

/* ---------------------------------------------------------------------- */
/*
	管理構造体
*/

typedef struct {
	unsigned int value:8;
	unsigned int strid:24;
} TABLES;

typedef struct {
	int status;
	int count;
	int prev_dir;
	int tick;
	GM_DEBUG_MENU *base;		// リストトップ
	GM_DEBUG_MENU *top;			// 表示トップ
	GM_DEBUG_MENU *cur;			// カレント
	GM_DEBUG_MENU_TABLES *table;
	int table_num;
} Work;

enum {
	STATE_HIDE = 0,
	STATE_SHOW = 1,
};

/*
	システムは常駐する
*/

static Work debugmenu_work;

/* ---------------------------------------------------------------------- */
/*
	設定
*/

#define TYPE( p )	( (p)->type & GM_DEBUG_MENU_TYPE_MASK )

static int get_value( GM_DEBUG_MENU *p )
{
	if( TYPE( p ) == GM_DEBUG_MENU_FLAG ){
		return *( p->target );
	} else if( TYPE( p ) == GM_DEBUG_MENU_FUNC ){
		return ( *( int (*)( int ) )p->target )( -1 );
	}
	return 0;
}

static void set_value( GM_DEBUG_MENU *p, int value )
{
	int mask = p->mask;
	if( TYPE( p ) == GM_DEBUG_MENU_FLAG ){
		*( p->target ) = ( *( p->target ) & ~mask ) | ( value & mask );
	} else if( TYPE( p ) == GM_DEBUG_MENU_FUNC ){
		int target;
	 	target = ( *( int (*)( int ) )p->target )( -1 );
		value = ( target & ~mask ) | ( value & mask );
		( *( int (*)( int ) )p->target )( value );
	}
}

static void setup( Work *work )
{
	// 変数から値を読み込み設定する。
	GM_DEBUG_MENU *p;

	for( p = work->base; p != NULL; p = p->next ){
		int v;
		int i;
		v = get_value( p );
		if( p->values != NULL ){
			for( i = 0; i < p->max; i++ ){
				if( ( ( p->values[ i ] & p->mask ) == ( v & p->mask ) ) ){
					p->current = i;
					break;
				}
			}
		} else {
			p->current = v;
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	表示用
*/

static void draw( Work *work )
{
	int i;
	GM_DEBUG_MENU *p;
	char *class;
	int x0, y0;

	x0 = DM_X0;
	y0 = DM_Y0;

	DEBUG_Locate( x0, y0, 0 );
	p = work->top;

	class = NULL;
	for( i = 0; i < DM_LINE && p != NULL; i++, p = p->next ){
		int value;
		if( p == work->cur ){
			DEBUG_Color( 200, 100, 100, 128 );
		} else {
			DEBUG_Color( 200, 200, 200, 128 );
		}

		if( class == NULL || strcmp( class, p->class ) != 0 ){
			DEBUG_Locate( CLASS_X, y0, 0 );
			DEBUG_Printf( "%s", p->class );
			class = p->class;
		}
		DEBUG_Locate( MENU_X, y0, 0 );
		DEBUG_Printf( "%s", p->menu );

		DEBUG_Locate( ITEM_X, y0, 0 );

		if( p->values != NULL ){
			value = p->values[ p->current ];
		} else {
			value = get_value( p );
		}
		if( p->items != NULL ){
			char *mes;
			mes = ( p->current == 0xFF ) ? "???" : p->items[ p->current ];
			DEBUG_Printf( "%s", mes );
		} else {
			if( p->type & GM_DEBUG_MENU_DEC ){
				DEBUG_Printf( "%d", value );
			} else {
				DEBUG_Printf( "%X", value );
			}
		}
		y0 = y0 + YSTEP;
	}
	if( i == DM_LINE && p != NULL && ( GV_Time % 60 < 40 ) ){
		DEBUG_Color( 200, 200, 200, 128 );
		DEBUG_Locate( CLASS_X, y0, 0 );
		DEBUG_Printf( "v" );
	}
	
	DEBUG_ResetColor();
}

/* ---------------------------------------------------------------------- */
/*
	操作用
*/

static void updown( Work *work, int dir )
{
	// 一度スクラッチパッドに載せる
	GM_DEBUG_MENU **list = ( GM_DEBUG_MENU ** )SCRPAD_ADDR;
	GM_DEBUG_MENU *p, **lp;
	int n, tpn, crn, btn;

	lp = list;
	n = 0;
	tpn = crn = 0;
	for( p = work->base; p != NULL; p = p->next, lp++, n++ ){
		if( p == work->top ){
			tpn = n;
		}
		if( p == work->cur ){
			crn = n;
		}
		*lp = p;
	}
	btn = n;
	*lp = NULL;

	if( dir > 0 ){
		// 一つ下
		if( crn < btn - 1 ){
			crn ++;
			work->cur = list[ crn ];
			if( tpn + DM_LINE <= crn ){
				tpn = crn - DM_LINE + 1;
				work->top = list[ tpn ];
			}
		}
	} else if( dir < 0 ){
		// 一つ上
		if( crn > 0 ){
			crn --;
			work->cur = list[ crn ];
			if( tpn > crn ){
				tpn = crn;
				work->top = list[ tpn ];
			}
		}
	}
}

static void selectfunc( Work *work, int dir )
{
	GM_DEBUG_MENU *cur = work->cur;
	int value;

	if( dir == -1 ){
		if( cur->current > 0 ){
			cur->current --;
		} else {
			cur->current = cur->max - 1;
		}
	} else {
		if( cur->max - 1 > cur->current ){
			cur->current ++;
		} else {
			cur->current = 0;
		}
	}
	// 値をここで設定
	if( cur->values != NULL ){
		value = cur->values[ cur->current ];
	} else {
		value = cur->current;
	}
	set_value( cur, value );
}

static void operate( Work *work, GV_PAD *pad )
{
	int button;

	button = pad->status;

	if( button & ( PAD_U | PAD_D | PAD_L | PAD_R ) ){
		int dir, ofs;

		dir = ( button & PAD_U ) ? -1 : 1;
		if( button & PAD_U ){
			dir = 0x10;
		} else if( button & PAD_D ){
			dir = 0x11;
		} else if( button & PAD_L ){
			dir = 0x20;
		} else if( button & PAD_R ){
			dir = 0x21;
		}
		ofs = ( dir & 1 ) ? 1 : -1;

		if( work->prev_dir == dir ){
			work->count --;
			if( work->count < 0 ){
				if( dir & 0x10 ){
					updown( work, ofs );
				} else {
					selectfunc( work, ofs );
				}
				work->count = SECOND_REPEAT;
			}
		} else {
			if( dir & 0x10 ){
				updown( work, ofs );
			} else {
				selectfunc( work, ofs );
			}
			work->count = FIRST_REPEAT;
			work->prev_dir = dir;
		}
	} else {
		work->prev_dir = 0;
	}
}

void GM_ActDebugMenu( GV_PAD *pad )
{
	Work *work = &debugmenu_work;

	GM_DebugPrint_Off = 0 ;/* 本当は、menupri.c の Act でやっていたのだが
							  Actorでなくなってしまったため、とりあえずこ
							  こで初期化
							  T.Morita Added 2002.02.06 */

	switch( work->status ){
	  case STATE_HIDE:
		if( pad->press & PAD_L1 ){
			// SHOWへ移行
			if( work->top == NULL ){
				// 初期設定
				work->top = work->cur = work->base;
			}
			setup( work );
			work->status = STATE_SHOW;
		}
		break;
	  case STATE_SHOW:
		if( pad->press & PAD_L1 ){
			// HIDEへ移行
			work->status = STATE_HIDE;
			break;
		}
		if( work->tick + 1 < GV_Time ){
			setup( work );
		}
		work->tick = GV_Time;
		operate( work, pad );
		draw( work );

		GM_DebugPrint_Off = 1;

		break;
	}

}

/* ---------------------------------------------------------------------- */
/*
	登録処理用
*/

void GM_SetDebugMenuTables( GM_DEBUG_MENU_TABLES *table, int table_num )
{
	debugmenu_work.table = table;
	debugmenu_work.table_num = table_num;
}

void GM_AddDebugMenu( GM_DEBUG_MENU *menu )
{
	Work *work = &debugmenu_work;
	GM_DEBUG_MENU *p, *pos;

	/* 整合性チェック */

	pos = NULL;
	menu->strid = ( ( GV_StrCode( menu->class ) + GV_StrCode( menu->menu ) ) & 0x00FFFFFF );
	for( p = work->base; p != NULL; p = p->next ){
		if( menu == p ){
			/* すでに登録済み */
			return;
		}
		if( strcmp( p->class, menu->class ) <= 0 ){
			pos = p;
		}
		if( menu->strid == p->strid ){
			/* 同じID */
			printf( "%s:%s and %s:%s are same ID\n"
					, menu->class, menu->menu, p->class, p->menu );
			HANGUP();
		}
	}
	/* 登録 */
	if( pos == NULL ){
		// 先頭
		menu->next = work->base;
		work->base = menu;
	} else {
		// POSのうしろ
		menu->next = pos->next;
		pos->next = menu;
	}
	if( menu->mask == 0 ){
		menu->mask = 0xFFFFFFFF;
	}
	/* debug.cnfのキーを検索して該当する場合は値をセットする */
	{
		int i;
		for( i = 0; i < work->table_num; i++ ){
			if( work->table[ i ].strid == menu->strid ){
				int value;
				value = work->table[ i ].value;
				set_value( menu, value );
			}
		}
	}
}

void GM_ResetDebugMenu( void )
{
	Work *work = &debugmenu_work;

	work->status = 0;
	work->base = NULL;
	work->top = NULL;
	work->cur = NULL;
}
