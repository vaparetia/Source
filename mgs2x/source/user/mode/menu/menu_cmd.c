//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	menu_cmd.c
	メニュー関連補助ルーチン

	2001/07/15	K.Takabe
	$Id: menu_cmd.c,v 1.1.1.3 2002/11/19 11:45:13 Yoshizawa1 Exp $
*/
/*

command 装備メニュー制御[NewEquipMenuControl] \
	-mic_mode $b:マイク説明状態（１で拡張説明を有効に）

command 装備自動選択セット[NewSetAutoSelect] \
	-item $w:選択アイテム装備 \
	-weapon $w:選択武器装備
// このコマンドでセットした後で装備メニューウィンドウを開くと
// 自動的にカーソルが指定した装備に移動する

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
/* エイムズマイク説明モードの変更コマンド */
int NewEquipMenuControl( void )
{
	int		val ;

	/* エイムズマイクの説明モード設定 */
	if ( GCL_GetOption( 'm' ) != NULL ){
		val = GCL_GetNextInt();
		if ( val ){
			MENU_StatusFlag |= MENU_STATUS_MIC_EXPLAIN2 ;
		} else {
			MENU_StatusFlag &= ~MENU_STATUS_MIC_EXPLAIN2 ;
		}
	}
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* アイテムの自動選択指定 */
int NewSetAutoSelect( void )
{
	if ( GCL_GetOption( 'i' ) ){
		MENU_AutoSelectItem = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'w' ) ){
		MENU_AutoSelectWeapon = GCL_GetNextInt() ;
	}
	return ( 0 );
}



