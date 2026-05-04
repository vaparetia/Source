//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	mdl_p_am.c
	モデルアニメ
	1999/10/12 S.Okajima
	$Id: mdl_p_am.c,v 1.1.1.3 2002/11/19 11:47:08 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"

typedef	struct	{
	DG_OBJS		*objs ;
	int			time;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;

	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos;
	SVECTOR		rot;

	int			now_data_num;
	int			time_count;

	int			repeat;

	int			data_num;
	Unit		unit[0];
} Work ;


static	void Act( Work *work )
{
	if( work->time_count >= work->unit[ work->now_data_num ].time-1 ){
		work->time_count=0;

		DG_DequeueObjs( work->unit[ work->now_data_num ].objs );
		work->now_data_num++;

//printf("work->now_data_num:%d\n",work->now_data_num);
		if( work->now_data_num >= work->data_num ){
			if( work->repeat == 1 ){
				work->now_data_num=0;
			}else{
				work->now_data_num = work->data_num-1;
//				GV_DestroyActor( work ) ;
//				return;
			}
		}

		DG_QueueObjs( work->unit[ work->now_data_num ].objs );

	}else{
		work->time_count++;
	}

#if 0
//printf("work->time_count  :%d:%d:\n",work->time_count, work->unit[ work->now_data_num ].time-1);
	if( work->now_data_num < work->data_num-1 ){
		if( work->time_count >= work->unit[ work->now_data_num ].time-1 ){
			work->time_count=0;
			DG_DequeueObjs( work->unit[ work->now_data_num ].objs );
			work->now_data_num++;
			DG_QueueObjs( work->unit[ work->now_data_num ].objs );
		}else{
			work->time_count++;
		}
	}
#endif

	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_PutObjs( work->unit[ work->now_data_num ].objs );
//	DG_GetLightMatrix( (FVECTOR*)work->unit[ work->now_data_num ].objs->world.m[3], work->lights );

}

static	void Die( Work *work )
{
	int	i;
	DG_DequeueObjs( work->unit[ work->now_data_num ].objs );
	for( i=0; i<work->data_num; i++ ){
		DG_FreeObjs( work->unit[ i ].objs );
	}
}

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	int	i;
	int	data1;
	int	data2;
	DG_DEF		*def ;

	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->pos.vx = (float)GCL_GetNextInt() ;
		work->pos.vy = (float)GCL_GetNextInt() ;
		work->pos.vz = (float)GCL_GetNextInt() ;
	}

	DG_GetLightMatrix( &work->pos, work->lights );

	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->rot.vx = (float)GCL_GetNextInt() ;
		work->rot.vy = (float)GCL_GetNextInt() ;
		work->rot.vz = (float)GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 'd' ) != NULL ) {
		for( i=0; i<work->data_num; i++ ){
			if( GCL_NextStr() != NULL ){
				data1=GCL_GetNextInt();
				data2=GCL_GetNextInt();
//printf("qqq:%d %d %d \n",data1,data2,7625400 /*"test1"*/);
				def = (DG_DEF*)GV_GetCache( GV_CacheID( data1, 'k' ) ) ;
				work->unit[i].objs = DG_MakeObjs( def, DG_FLAG_SHADE | DG_FLAG_ONEPIECE, 0 );
				work->unit[i].time = data2 ;



				DG_SetLightMatrix( work->unit[i].objs, work->lights );

				work->lights[1].m[3][0]=128;
				work->lights[1].m[3][1]=128;
				work->lights[1].m[3][2]=128;

/*
				if(trans){
					for( j=0; j<def->n_models; j++ ){
						def->models[j].type |= DG_TYPE_TRANS;
					}
				}
*/

			}else{
				break;
			}
		}
		if( work->data_num != i ){
			printf("-- More Data Pls. --");
			printf("-- More Data Pls. --");
			printf("-- More Data Pls. --");
			printf("-- More Data Pls. --");
			printf("-- More Data Pls. --");
		}
	}

	if ( GCL_GetOption( 'i' ) != NULL ) {
		work->repeat = GCL_GetNextInt() ;
	}else{
		work->repeat = 0;
	}

}

static	int GetResources( Work *work, int data_num )
{

	work->data_num=data_num;

	GetOptionValue( work );

	DG_QueueObjs( work->unit[0].objs );

	work->time_count=0;
	work->now_data_num=0;

	return 0 ;
}

void *NewModelPatAnimation( void )
{
	Work		*work ;
	int			buf_size;
	int			data_num=0;

	OPERATOR() ;

	if ( GCL_GetOption( 'n' ) != NULL ) {
		data_num = GCL_GetNextInt() ;
	}
	if( data_num < 0 ) data_num=1;


	buf_size = sizeof( Work ) + sizeof( Unit ) * data_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, data_num ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
