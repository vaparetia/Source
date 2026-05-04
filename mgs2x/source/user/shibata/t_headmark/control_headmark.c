//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
     ヘッドマーク管理
     
     1999/12/14  H.TANAKA

*/


#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"gameheader.h"
#include	"def_dma.h"

typedef struct
{
    GV_ACT_EX   actor ;

    FMATRIX  *world ;
    int      *type ;
    void     *headmark ;
    void     *headmark_move ;
} Work ;

extern  void *AN_HeadMark( FMATRIX *world, int mark ) ;
extern  void *AN_Zzz(  FMATRIX *world ) ;
extern  void *NewPiyori(FMATRIX *world) ;
extern  void *NewPiyori_Anes(FMATRIX *world) ;
extern  void *New_Zzz(FMATRIX *world) ;


static  int ReceiveSignal( void *workp, int signal, int value )
{
    Work *work = workp ;
    int  head_addr ;
    int  move_addr ;

    head_addr = (int)work->headmark ;
    move_addr = (int)work->headmark_move ;
    switch(signal)
    {
        case GV_SIGNAL_DIE :
	    if(head_addr == value)
		work->headmark = NULL ;
	    if(move_addr == value)
		work->headmark_move = NULL ;
	    break ;
	    case GV_SIGNAL_KILL:
	    	GV_DestroyActor( work );
	    	break;

	default :
	    break ;
    }
	return 0;
}

static  void  Act(Work *work)
{
	int  mode ;

	if((*work->type & 0x8000) == 0){
		return ;
	}else{
		*work->type &= 0x7fff ;
		mode = *work->type ;
		/* ?,! マークのとき */
		if( (((0 <= mode ) && ( mode < 6))) || ((mode&0xff) == 9) ){
			/* 何か別の物が動いていた場合 */
			if(work->headmark != NULL) GV_DestroyActor(work->headmark);
			work->headmark = AN_HeadMark(work->world,mode) ;
			GV_SetActorChild( work, work->headmark ) ;
		}else if(( 6 <= mode ) && ( mode < 9)){ /* ピヨピヨ系のとき */
			/* 何か別の物が動いていた場合 */
			if(work->headmark_move != NULL) GV_DestroyActor(work->headmark_move) ;
			switch(mode){
				case 6:
					work->headmark_move = New_Zzz(work->world) ;
					break;
				case 7:
					work->headmark_move = NewPiyori(work->world) ;
					break ;
				case 8: 
					work->headmark_move = NewPiyori_Anes(work->world) ;
					break ;
				default:
					work->headmark_move = NULL ;
					break ;
			}
			if(work->headmark_move != NULL) GV_SetActorChild(work, work->headmark_move) ;
		}else{
			if(work->headmark != NULL) GV_DestroyActor(work->headmark) ;
			if(work->headmark_move != NULL) GV_DestroyActor(work->headmark_move) ;
			work->headmark = NULL ;
			work->headmark_move = NULL  ;
		}
	}
}

static  void  Die(Work *work)
{
}

static  int GetResources( Work *work, FMATRIX *world, int *type )
{
	work->world = world ;
	work->type  = type ;

	work->headmark = NULL ;
	work->headmark_move = NULL ;

	/* シグナル設定 */
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

   return 0 ;
}

void  *NewControl_Headmark( FMATRIX *world, int *type )
{
	Work *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_AFTER,sizeof( Work ) ) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work, world, type ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
    return (void *)work ;
}
