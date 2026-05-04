//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	n_sight.c
	暗視ゴーグル
	
	2000/01/13 Y.Korekado
	$Id: n_sight.c,v 1.1.1.3 2002/11/19 11:44:04 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewNightSight( body, sw )

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"

extern	void *NewOpticalCamouflage( DG_OBJS *objs, int flag ) ;

/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)

/*----------------------------------------------------------------*/
typedef	struct n_sight_Work	{
	GV_ACT_EX	actor ;
	OBJECT		*body ;
	OBJECT		n_sight ;
	DG_DEF 		*open ;
	DG_DEF 		*close ;

	int			*flag ;
	int			ef_flag ;

	int			status ;
	u_int		count ;
} Work ;

enum {
	DIE=-1,
	INVISIBLE,
	OPEN,
	CLOSE,
	CLOSE_OFF
} ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */
/*----------------------------------------------------------------*/
static	int SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->n_sight.objs->flag |= DG_FLAG_INVISIBLE ;
		work->ef_flag = INVISIBLE ;
		return 0 ;
	} else {
		work->n_sight.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->ef_flag = *work->flag ;
      switch ( work->ef_flag ) {
			case INVISIBLE :
					work->n_sight.objs->flag |= DG_FLAG_INVISIBLE ;
					return 0 ;
			case OPEN :
				if ( work->status == CLOSE ) {
					DG_ChangeModelObj( work->n_sight.objs, work->n_sight.objs->objs, work->open, work->open->models ) ;
/** cigeno add **/
/** bound_box set  **/
/* モデル切り替え後の新しいバウンドをセット*/
					work->n_sight.objs->bound_min = work->n_sight.objs->objs->bound_min ;
					work->n_sight.objs->bound_max = work->n_sight.objs->objs->bound_max ;
/****************/
					work->status = OPEN ;
				}
				break ;
			case CLOSE :
				if ( work->status == OPEN ) {
					DG_ChangeModelObj( work->n_sight.objs, work->n_sight.objs->objs, work->close, work->close->models ) ;
/** cigeno add **/
/** bound_box set  **/
					work->n_sight.objs->bound_min = work->n_sight.objs->objs->bound_min ;
					work->n_sight.objs->bound_max = work->n_sight.objs->objs->bound_max ;
/****************/
					work->status = CLOSE ;
				}
				break ;
		}
	}
#if 0
/* debug バウンド表示 */
	NewBoundingBoxView_1( work->n_sight.objs->world, 
							&work->n_sight.objs->bound_min,
							&work->n_sight.objs->bound_max, 0x8000ffff );
#endif
	return 1 ;
}


static	void	Act( work )
Work		*work ;
{
	if ( *work->flag < 0 ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	if ( SetInvisible( work ) ) {
		DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
		DG_PutObjs( work->n_sight.objs ) ;
	}
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( work->body->objs->flag & DG_FLAG_OPTCMF ) {
			work->n_sight.objs->flag |= DG_FLAG_OPTCMF ;
		} else {
			work->n_sight.objs->flag &= ~DG_FLAG_OPTCMF ;
		}
	}
}

static	void	Die( work )
Work		*work ;
{
	extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;
	
	if ( !(GV_IsStageDestroy( work )) ) {
		DG_DisconnectObjs( work->body->objs, work->n_sight.objs ) ;
	}
    GM_FreeObject( &work->n_sight ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, body, open, close, flag )
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
int		open ;
int		close ;
int		*flag ;
{
	extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;
	extern	void	*InfaredCamera_Light(FMATRIX *world,int *mode,int *group_id );
	FMATRIX		*world ;

	work->body = body ;
	work->flag = flag ;
	world = &work->body->objs->objs[HUMAN21_ATAMA].world ;

	GM_InitObject( &work->n_sight, open, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	work->open = work->n_sight.objs->def ;
	work->close = (DG_DEF*) GV_GetCache( GV_CacheID( close, 'k' ) );

	DG_SetPos( world ) ;
	DG_PutObjs( work->n_sight.objs ) ;
	DG_SetLightMatrix( work->n_sight.objs, body->objs->light );

	DG_ConnectObjs( body->objs, work->n_sight.objs ) ;

	work->status = OPEN ;
	work->count = 0 ;

	SetInvisible( work ) ;
	GV_SetActorChild( work, 
		InfaredCamera_Light( world, &(work->ef_flag), &work->body->map_name) ) ;

	if ( body->objs->flag & DG_FLAG_OPTCMF ) {
		GV_SetActorChild( work, NewOpticalCamouflage( work->n_sight.objs, 0 ) ) ;
	}

	return 0 ;
}

void	*NewNightSight( body, open, close, flag )
OBJECT	*body ;	/* ボディーオブジェクト */
int		open ;
int		close ;
int		*flag ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, body, open, close, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
