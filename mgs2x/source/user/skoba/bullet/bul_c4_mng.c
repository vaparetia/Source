//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_c4_mng.c
   £Ã£´´ÉÍý¼Ô

   2001/03/07	M.Sonoyama
   $Id: bul_c4_mng.c,v 1.1.1.3 2002/11/19 11:50:00 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"bul_c4.h"


C4_MNG_WORK		*C4MngWork = NULL ;
extern int PL_PAD_PUNCH ; /* raiden/pl_pad.c */

static	void	C4MngAct( C4_MNG_WORK *work )
{
	Work		*list, *this ;
	int			n_msg ;
	GV_MSG		*msg ;

	/* ¥Ñ¥Ã¥É¤ÇÇúÈ¯¤¹¤Ù¤­£Ã£´¤ò¸¡º÷ */
	if ( GM_N_C4Bombs == 0 ) return ;
	if ( ( ( GV_PadData[ 0 ].press & PL_PAD_PUNCH ) ||
		 ( ( n_msg = GV_ReceiveMessage( GV_StrCode( "£Ã£´" ), &msg ) ) > 0 ) ) &&
		!PL_AttackDisable() &&  
		!( GM_CheckPlayerStatus( PLAYER_PAD_OFF | PLAYER_FORCE ) ) &&
		!( GM_ItemTypes[ PL_GetPlayerItem() ] & IT_TYPE_SUBJECT ) &&
		!( GV_PadData[ 0 ].flag & GV_PAD_RELEASE ) ) {
		list = work->list.next ;
		while( list != NULL ) {
			/* ÀèÆ¬¤«¤é¸¡º÷ */
			this = list ;
			list = list->next ;
			if ( ( this->flag & FLAG_PLAYER ) &&
				 !( this->flag & ( FLAG_FREEZE | FLAG_DESTROY | FLAG_SWITCH ) ) ) {
				this->flag |= FLAG_SWITCH ;
				break ;
			}
		}
	}
#if 0
	/* ÊÑ¿ô¥¹¥¤¥Ã¥Á¤ÇÇúÈ¯¤¹¤ë */
	if ( GM_C4_Blast || GM_C4_BlastAll ) {
		list = work->list.next ;
		while( list != NULL ) {
			this = list ;
			list = list->next ;
			if ( !( this->flag & FLAG_KAITAI ) &&
				 !( this->flag & ( FLAG_FREEZE | FLAG_DESTROY ) ) ) {
				this->flag |= FLAG_SWITCH ;
				if ( GM_C4_BlastAll == 0 ) break ;
			}
		}		
		GM_C4_Blast = 0 ;
		GM_C4_BlastAll = 0 ;
	}
#endif
}

static	void	C4MngDie( C4_MNG_WORK *work )
{
	C4MngWork = NULL ;
}

/* ¥Þ¥Í¡¼¥¸¥ã¡¼µ¯Æ° */
/* £Ã£´ËÜÂÎ¤¬£Á£Æ£Ô£Å£Ò¤Ê¤Î¤Ç¡¢£Ð£Ì£Á£Ù£Å£Ò¥ì¥Ù¥ë¤ÇÆ°¤«¤¹ */
void		*PL_C4Manager( void )
{
	C4_MNG_WORK		*work ;

	work = ( C4_MNG_WORK * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
										    sizeof( C4_MNG_WORK ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, C4MngAct, C4MngDie ) ;

	work->n_bombs = 0 ;
	work->n_incremental = 0 ;
	work->list.next = NULL ;

	C4MngWork = work ;
	return work ;
}

