//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	automenu.c
	自動メニュー選択マネージャーキャラ（装備自動選択セットの連続実行キャラ）

	2001/07/30	K.Takabe
	$Id: automenu.c,v 1.1.1.3 2002/11/19 11:45:12 Yoshizawa1 Exp $
*/
/*

chara	自動メニュー選択マネージャー[NewAutoMenuSelectManagerSet] $s:name \
	-pad $w:検出パッド指定（デフォルトはＲ３） \
	-num $w:最大指定数 \
	-command $b:タイプ（０：アイテム、１：武器） $w:装備 ...2
// 指定した検出パッドが押された時点で装備メニューの自動選択が発動します
// 但しメニューを開くこと自体はユーザー（パッドデモ）側で行ってください
// -commandは-numオプションで指定した数だけ必ず指定してください

*/

/* ---------------------------------------------------------------- */
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
#include	"def_dma.h"
#include	"libdg.cnf"
#include	"dmapack.h"
#include	"font.h"
#include	"sprite_2d.h"
#include "menu.h"

/* ---------------------------------------------------------------- */
typedef struct {
	int		type ;	/* 0:item , 1:weapon */
	int		num ;	/* 装備番号 */
} Command ;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	int			name ;
	int			where ;

	int			pad ;
	int			n_command ;
	int			max_command ;
	Command		commands[0] ;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	if ( work->n_command >= work->max_command ){
		GV_DestroyActor( work );
		return ;
	}
	if ( GV_PadData[ 0 ].press & work->pad ){
		if ( work->commands[ work->n_command ].type == 0 ){
			/* アイテム */
			printf("auto select item : %d\n", work->commands[ work->n_command ].num );
			MENU_AutoSelectItem = work->commands[ work->n_command ].num ;
		} else {
			/* 武器 */
			printf("auto select weapon : %d\n", work->commands[ work->n_command ].num );
			MENU_AutoSelectWeapon = work->commands[ work->n_command ].num ;
		}
		work->n_command++ ;
	}
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	int			i ;

	work->name = name ;
	work->where = where ;

	work->pad = PAD_AR ;
	if ( GCL_GetOption( 'p' ) ){
		work->pad = GCL_GetNextInt();
	}

	if ( !GCL_GetOption( 'c' ) ) return ( -1 );
	for ( i = 0 ; i < work->max_command ; i++ ){
		work->commands[ i ].type = GCL_GetNextInt();
		work->commands[ i ].num = GCL_GetNextInt();
		printf("%s: %d %d\n", __FILE__, work->commands[ i ].type, work->commands[ i ].num );
	}

	return (0);
}
/* ---------------------------------------------------------------- */
/* アイテムの自動選択指定 */
void *NewAutoMenuSelectManagerSet( int name, int where )
{
	Work		*work ;
	int			max = 1 ;

	if ( !GCL_GetOption( 'n' ) ) return ( NULL );
	max = GCL_GetNextInt();

	OPERATOR() ;
	/* 装備メニューよりも後に実行されるようにプライオリティを調整 */
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
 									sizeof( Work ) + sizeof(Command) * max, PLAYER_MENU_ACTOR_PRIO + 1 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		work->max_command = max ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}



