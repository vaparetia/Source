//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ray_consol.c
	レイのコンソール

	2000/01/25 T.Shibata

	$Id: put_light.c,v 1.1.1.3 2002/11/19 11:48:38 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

typedef struct {
	GV_ACT_EX		actor;
	int				name;
	int				put_name;
	int				light_flags;
} Work;

enum {
	MESG_PLIGHT_INV = 0,
	MESG_PLIGHT_VIS,
	MESG_PLIGHT_SPE,
	MESG_PLIGHT_DIE,
};

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case MESG_PLIGHT_INV:
			work->light_flags = 0;
			break;
		  case MESG_PLIGHT_VIS:
			work->light_flags = 1;
			break;
		  case MESG_PLIGHT_SPE:
			work->light_flags = 2;
		  case MESG_PLIGHT_DIE:
			work->light_flags = -1;
			printf("終了\n");
			return 1;
			break;
		  default:
			printf("put_light.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return 0;
}

static int CheckControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name == name ) return (0) ;
	}
	return (1);
}


static void Act(Work *work)
{
	if(CheckMesgParam( work )||CheckControl( work->put_name )){
		//終了
		work->light_flags = -1;
		GV_DestroyActor( work ) ;
	}
}

static void Die(Work *work)
{
}

static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		//if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	return ( NULL );
}


extern void *NewCircleLight(FMATRIX *world,int *flag,int mode,int *map);

static int GetResources( Work *work )
{
	CONTROL		*control;
	int			put_name = work->put_name = GCL_GetOptionValue( 'p', 0 );
	int			mode = GCL_GetOptionValue( 'm', 0 );
	FMATRIX		*put_mat;
	void		*son;
	control = SearchControl( put_name );
	if(!control){ return -1; }
	if(mode&0x8000)
		put_mat = &control->object->objs->objs[10].world;
	else
		put_mat = &control->object->objs->objs[6].world;

	son = NewCircleLight(put_mat,&work->light_flags,mode,&control->map);

	GV_SetActorChild( work, son );

	return 0;
}

void *NewPutLight_doll( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewActor(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = name;
		//work->object = object;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
	}

	return (void *)work ;
}

