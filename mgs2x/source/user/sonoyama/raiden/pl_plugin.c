//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_plugin.c
   プレイヤー、プラグインから使用する関数

   2000/06/12 M.Sonoyama
   $Id: pl_plugin.c,v 1.1.1.3 2002/11/19 11:50:59 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
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
#include	"g_other.h"
#include	"raiden.h"

/* プラグインの登録 */
void	PL_AddPlugin( this, no, init, action )
PL_PluginSet	*this ;
int		no ;
PL_PLUGIN_INIT	init ;
PL_ACTION	action ;
{
    PL_PluginSet	*list ;

#ifdef DEBUG_MODE
	/* 二重登録のチェック */
	list = PL_PluginList.next ;
	while( list != NULL ) {
		if ( list == this ) {
			printf( "plugin func has already linked! : %x\n", this ) ;
			ASSERT( 0 ) ;
		}
		list = list->next ;
	}
#endif
    this->plugin_no = no ;
    this->init = init ;
    this->action = action ;

    list = &PL_PluginList ;
    this->next = list->next ;
    list->next = this ;
}

/* ポーリング関数の登録 */
void	PL_AddPollingFunc( this, func )
PL_PollingSet		*this ;
PL_POLLING_FUNC		func ;
{
	PL_PollingSet 	*list ;

#ifdef DEBUG_MODE
	/* 二重登録のチェック */
	list = PL_PollingList.next ;
	while( list != NULL ) {
		if ( list == this ) {
			printf( "polling func has already linked! : %x\n", this ) ;
			ASSERT( 0 ) ;
		}
		list = list->next ;
	}
#endif
	this->func = func ;
	list = &PL_PollingList ;
	this->next = list->next ;
	list->next = this ;
}

/* ポーリング関数の削除 */
void 	PL_RemovePollingFunc( this )
PL_PollingSet		*this ;
{
	PL_PollingSet 	*pre, *list ;

	pre = NULL ;
	list = &PL_PollingList ;
	while( list != NULL ) {
		if ( list == this ) {
			ASSERT( pre != NULL ) ;
			pre->next = this->next ;
			break ;
		}
		pre = list ;
		list = list->next ;
	}
}

/*----------------------------------------------------------------*/


