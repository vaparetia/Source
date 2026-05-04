//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
     control_headmark2.c
     ヘッドマーク管理2
     2000/06/15/ T.Shibata
     
	 $Id: control_headmark2.c,v 1.1.1.3 2002/11/19 11:48:52 Yoshizawa1 Exp $
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

#include	"headmark.h"

typedef struct
{
    GV_ACT_EX   actor ;
	FVECTOR		pos;
	FVECTOR		size;

    FMATRIX		*world ;
    int    		*type ;
    void   		*headmark ;
    void   		*headmark1 ;

    void   		*headmark_move ;
	TARGET		*trg;
	int			pre_mark;
	CONTROL		*ctrl;

//	OBJECT 		*body;

} Work ;

extern void	*AN_HeadMark( FMATRIX *world, int mark );
extern void	*AN_HeadMarkWithTrgt( FMATRIX *world, int mark, TARGET *trg, FVECTOR*,FVECTOR* );
extern  void *AN_Zzz(  FMATRIX *world ) ;
extern  void *NewPiyori(FMATRIX *world, int* ) ;
extern  void *NewPiyori_Anes(FMATRIX *world) ;
extern  void *New_Zzz(FMATRIX *world) ;
extern void *NewAbsentMind( FMATRIX *world );
extern  void *NewSleep(FMATRIX *world, int* ) ;
extern void *NewDiscovery(FMATRIX *world);

static  int ReceiveSignal( void *workp, int signal, int value )
{
    Work *work = workp ;
    int  head_addr ;
    int  move_addr ;
    int  head1_addr ;

    head_addr = (int)work->headmark ;
    head1_addr = (int)work->headmark1 ;
    move_addr = (int)work->headmark_move ;
	
    switch(signal){
	  case GV_SIGNAL_DIE:
		if(head_addr == value) work->headmark = NULL ;
		if(head1_addr == value) work->headmark1 = NULL ;
		if(move_addr == value) work->headmark_move = NULL ;
		break ;
	  case GV_SIGNAL_KILL:
		GV_DestroyActor( work );
		break;
	  default :
		return GV_DefaultSignalFunc( workp, signal, value );
    }

	return 0;
}

static  void  Act(Work *work)
{
	int  type = *work->type;
#if 0
	if(GV_Time%60==0){
		extern void *NewDiscovery(FMATRIX *world);
		NewDiscovery(work->world);
	}
#endif	

	if(!(type & HMK2_TYPE_CALL)){
		return ;
	}

printf("headmark :  aaaaaaaaaaaaaaaaaaaaaaa  type[%x]\n",type ) ;

	*work->type &= ~(HMK2_TYPE_CALL);
	
	if(type & 0x40000000){
		if(work->headmark != NULL) GV_DestroyOtherActor(work->headmark) ;
		if(work->headmark1 != NULL) GV_DestroyOtherActor(work->headmark1) ;
		if(work->headmark_move != NULL) GV_DestroyOtherActor(work->headmark_move) ;
		work->headmark = NULL;
		work->headmark_move = NULL;
		work->headmark1 = NULL;
		return ;
	}

	if( type & HMK2_TYPE_MARK){
		// ?,! マークのとき 
		if(work->headmark != NULL) GV_DestroyOtherActor(work->headmark);
		if(work->headmark1 != NULL) GV_DestroyOtherActor(work->headmark1);

		if((type & HMK2_TYPE_CHECK) == 0x0c){
			if(work->pre_mark & 1) type = HMK2_TYPE_BREAK_QT;
			else                   type = HMK2_TYPE_BREAK_AT;
		}

		printf("type = %x\n",type );

		if(type & HMK2_TYPE_TRGT){
		    DG_COPY_VEC(&work->pos,&DG_ZeroVector);
			DG_COPY_VEC(&work->size,&DG_ZeroVector);
			
			work->headmark = AN_HeadMarkWithTrgt( work->world,(type & HMK2_TYPE_CHECK),
												  work->trg, &work->pos, &work->size );

			work->headmark1 = AN_HeadMark( work->world,(type & HMK2_TYPE_CHECK) | 0x4000);
			
		}else{
			work->headmark = AN_HeadMark( work->world,(type & HMK2_TYPE_CHECK));
			work->headmark1 = AN_HeadMark( work->world,(type & HMK2_TYPE_CHECK) | 0x4000);
		}
		GV_SetActorChild( work, work->headmark ) ;
		GV_SetActorChild( work, work->headmark1 ) ;

		work->pre_mark = (type & HMK2_TYPE_CHECK);
	}else{
		//value チェンジ	
		if( type & 0x0800 ){
			GV_CallChildSignalFunc( work, 0, (type & 0x00ff) );
			printf("value = %d\n",(type & 0x00ff));
			return;
		}

		// ぴよぴよとかのとき
		if(work->headmark_move != NULL) GV_DestroyOtherActor(work->headmark_move);

		switch(type & HMK2_TYPE_CHECK){
		case 0:
			work->headmark_move = New_Zzz(work->world);
			break;
		case 1:
			work->headmark_move = NewPiyori(work->world, &work->ctrl->map);
			break ;
		case 2: 
			work->headmark_move = NewSleep(work->world, &work->ctrl->map);
			break ;
		case 3:
			work->headmark_move = NewAbsentMind(work->world);
			break ;
		case 4:
			work->headmark_move = NewDiscovery(work->world);
			break ;
		  default:
			work->headmark_move = NULL ;
			break ;
		}
		if(work->headmark_move != NULL) GV_SetActorChild(work, work->headmark_move) ;

		work->pre_mark = (type & HMK2_TYPE_CHECK);
	}

}

static  void  Die(Work *work)
{
//	if(work->headmark != NULL) GV_DestroyOtherActor(work->headmark);
//	if(work->headmark_move != NULL) GV_DestroyOtherActor(work->headmark_move);
//	if(work->headmark1 != NULL) GV_DestroyOtherActor(work->headmark_move);
}

static  int GetResources( Work *work, FMATRIX *world, int *type, TARGET *trg )
{
	work->world = world ;
	work->type  = type ;

	work->headmark = NULL ;
	work->headmark1 = NULL ;
	work->headmark_move = NULL ;
	work->trg = trg;
//	work->body = body;

	DG_COPY_VEC(&work->pos,&DG_ZeroVector);
	DG_COPY_VEC(&work->size,&DG_ZeroVector);

	/* シグナル設定 */
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

   return 0 ;
}

void  *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *trg, CONTROL *cntrl )
{
	Work *work ;
//printf("\n\nptrg = %p\n\n",trg);
	work = (Work *)GV_NewEffect(GV_ACTOR_AFTER,sizeof( Work ) ) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;

		work->ctrl = cntrl;
		if(GetResources( work, world, type, trg ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
    return (void *)work ;
}
