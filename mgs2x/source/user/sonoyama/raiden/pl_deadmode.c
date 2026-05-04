//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_deadmode.c
   プレイヤー／特殊ゲームオーバーアクションリンク
   
   2001/04/23 M.Sonoyama
   $Id: pl_deadmode.c,v 1.1.1.3 2002/11/19 11:50:58 Yoshizawa1 Exp $
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

#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_inline.h"

/* 特殊ゲームオーバーアクション登録 */
void		PL_AddDeadAction( PL_DeadMode *this, PL_ACTION action, PL_DEADMODE_COND_FUNC cond )
{
#ifdef DEBUG_MODE
	/* 二重登録のチェック */
	{
		PL_DeadMode	*list ;
		
		list = PL_DeadModeList.next ;
		while( list != NULL ) {
			if ( list == this ) {
				printf( "error : this player dead action is already exist.\n" ) ;
				ASSERT( 0 ) ;
			}
			list = list->next ;
		}
	}
#endif	
	this->action = action ;
	this->cond = cond ;
	this->next = PL_DeadModeList.next ;
	PL_DeadModeList.next = this ;
}

/* 特殊ゲームオーバーアクション削除 */
void		PL_RemoveDeadAction( PL_DeadMode *this )
{
	PL_DeadMode		*list ;

	list = &PL_DeadModeList ;
	while( list->next != NULL ) {
		if ( list->next == this ) {
			list->next = this->next ;
			return ;
		}
		list = list->next ;
	}
}
