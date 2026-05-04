//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dbgobj.c
   デバッグ・テスト用プログラム
   
   2000/02/16 M.Sonoyama
   $Id: dbgobj.c,v 1.1.1.3 2002/11/19 11:50:39 Yoshizawa1 Exp $
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

/*------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)

/*------------------------------------------------------------*/

extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; 

typedef struct _Work {
    GV_ACT		actor ;
	FVECTOR		mov ;
	HZX_D_CALLBACK	callback ;
	void			*hzd ;
	int				time ;
} Work ;

/*------------------------------------------------------------*/

/* 動的ハザードコールバック */
/* 位置を更新する */
static	void	HazardCallback( hzd, p1, p2, flag )
void			*hzd ;
void			*p1, *p2 ;
int				flag ;
{
	FVECTOR		*mov ;
	Work		*work ;

	work = ( Work * )p1 ;
	if ( flag & HZX_CALLBACK_FLAG_DESTROY ) {
		/* くっつき先がなくなった */
		work->hzd = NULL ;
		GV_DestroyActor( work ) ;
		return ;
	}
	/* 位置を更新 */
	mov = ( FVECTOR * )p2 ;
	HZX_CallbackUpdatePos( hzd, mov, mov ) ; /* 位置を更新する便利関数 */
}

/*------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
//	AN_Test_Eye2( &work->mov, 2 ) ;
//	if ( ++ work->time == 150 ) GV_DestroyActor ( work ) ;
}

static	void	Die( work )
Work		*work ;
{
	if ( work->hzd != NULL )
		HZX_RemoveDynamicCallback( work->hzd, &work->callback ) ;
}

/*------------------------------------------------------------*/

static	int	GetResources( work, mov, seg, type )
Work		*work ;
FVECTOR		*mov ;
void		*seg ;
int			type ;
{
	int				atr ;
	void			*hzd ;

	work->hzd = NULL ;
	DG_COPY_VEC( &work->mov, mov ) ;
	if ( type == HZX_TYPE_SEGMENT ) {	/* 壁 */
		atr = ( ( HZX_SEG * )seg )->attribute ;
		if ( atr & HZX_SEG_DYNAMIC ) {	/* 動的壁チェック */
			hzd = ( ( HZX_SEG * )seg )->ptr ;
		} else {
			return 0 ;
		}
	} else {							/* 床 */
		atr = ( ( HZX_FLR * )seg )->attribute ;
		if ( atr & HZX_FLOOR_DYNAMIC ) {	/* 動的床チェック */
			hzd = ( ( HZX_FLR * )seg )->ptr ;
		} else {
			return 0 ;
		}
	}
	/* コールバック登録 */
	HZX_SetDynamicCallback( hzd, &work->callback, HazardCallback, work, &work->mov ) ;
	work->hzd = hzd ;
    return 0 ;
}

/* 動的壁弾痕テスト */
/* 
   mov 		: 弾痕位置
   seg		: HZX_SEG * or HZX_FLR *
   type		: 壁 or 床
*/
void	*NewDebugScar( FVECTOR *mov, void *seg, int type ) 
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, mov, seg, type ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}
