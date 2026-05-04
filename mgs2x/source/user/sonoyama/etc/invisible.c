//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   multi.c
   マルチウェイトモデル表示用
   
   2000/04/06 M.Sonoyama
   $Id: invisible.c,v 1.1.1.3 2002/11/19 11:50:43 Yoshizawa1 Exp $
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


typedef	struct	{
    GV_ACT		actor ;
	CONTROL		*ctrl ;
    OBJECT		*body ;
    int			evmflag ;
    int			flag ;
} Work ;

typedef	struct	{
    GV_ACT	actor ;
    OBJECT	*body ;
    int		*evmflag ;	
    int		*flag ;
} Work2 ;


static Work  *invis_work1 = NULL ;
static Work2 *invis_work2 = NULL ;


static	void	Die( Work2 *work )
{
	int i ;

	if ( work->body ) {
		/* オブジェのフラグを元に戻す */
		work->body->objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->body->objs->flag |= *work->flag ;

		/* 各関節すべてのオブジェのフラグを元に戻す */
		i = work->body->objs->n_models ;
		while( --i>=0 ) {
			work->body->objs->objs[i].flag &= ~DG_FLAG_INVISIBLE ;
			work->body->objs->objs[i].flag |= *work->flag ;
		}

		/* EVMオブジェのフラグを元に戻す */
		if ( work->body->evmobj ) {
			work->evmflag = work->body->evmobj->flag ;
			work->body->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
		}
	}
	if ( invis_work1 ) {
		GV_DestroyActor( invis_work1 ) ;
	}
	invis_work1 = NULL ;
	invis_work2 = NULL ;
}

static	void	Act( Work *work )
{
	int i ;

	/* オブジェのフラグを不可視にする */
    work->flag = work->body->objs->flag & DG_FLAG_INVISIBLE ;
	DG_InvisibleObjs( work->body->objs ) ;
	
	/* 各関節すべてのオブジェのフラグを不可視にする */
	i = work->body->objs->n_models ;
	while( --i>=0 ) {
		work->body->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
	}

	/* EVMオブジェのフラグを不可視にする */
	if ( work->body->evmobj ) {
		work->evmflag = work->body->evmobj->flag ;
		work->body->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	}
}

static	void	Act2( Work2	*work )
{
	int i ;

	/* オブジェのフラグを元に戻す */
    work->body->objs->flag &= ~DG_FLAG_INVISIBLE ;
    work->body->objs->flag |= *work->flag ;

	/* 各関節すべてのオブジェのフラグを元に戻す */
	i = work->body->objs->n_models ;
	while( --i>=0 ) {
		work->body->objs->objs[i].flag &= ~DG_FLAG_INVISIBLE ;
		work->body->objs->objs[i].flag |= *work->flag ;
	}

	/* EVMオブジェのフラグを元に戻す */
	if ( work->body->evmobj ) {
		work->body->evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
		work->body->evmobj->flag |= *work->evmflag ;
	}
}


/*
   以下２つの関数は、プログラム中からも、シナリオからも呼んでよし

*/
int NewKillInvisibleChara()
{
	if ( invis_work1 && invis_work2 ){
		GV_DestroyActor( invis_work2 ) ;
		invis_work2 = NULL ;
		return 1 ;
	}
	return 0 ;
}


int NewInvisibleChara()
{
    Work		*work = NULL /* BP ADDED = NULL because of check at bottom */;
    Work2		*work2 = NULL /* BP ADDED = NULL because of check at bottom */;

	if ( invis_work1 && invis_work2 )
	  return 0 ;
	if ( GM_PlayerControl == NULL )
	  return 0 ;
	if ( GM_PlayerBody == NULL )
	  return 0 ;

    work = (Work *)GV_CreateActor( GV_ACTOR_PLAYER,
								  GV_CLASS_OBJECT,
								  sizeof( Work ),
								  PLAYER_INVISIBLE_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, NULL ) ;
		work->ctrl = GM_PlayerControl ;
		work->body = GM_PlayerBody    ;

		work2 = (Work2 *)GV_CreateActor( GV_ACTOR_PREV2,
										GV_CLASS_OBJECT, 
										sizeof( Work2 ),
										PLAYER_INVISIBLE_ACTOR2_PRIO ) ;
		if ( work2 != NULL ) {
			GV_SetActor( &work2->actor, Act2, Die ) ;
			work2->flag = &work->flag ;
			work2->evmflag = &work->evmflag ;
			work2->body = GM_PlayerBody ;

			invis_work1 = work  ;
			invis_work2 = work2 ;
		}
    }
    return (work && work2) ? 1 : 0 ;
}
