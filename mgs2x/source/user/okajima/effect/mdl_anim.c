//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mdl_anim.c
	モデルアニメ
	1999/10/12 S.Okajima
	$Id: mdl_anim.c,v 1.1.1.3 2002/11/19 11:47:07 Yoshizawa1 Exp $
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
	int			time;
	DG_OBJS		*objs ;
	SVECTOR		rot;
	FVECTOR		pos;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_OBJS		*objs ;
	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos;
	SVECTOR		rot;

	int			now_data_num;
	int			time_count;
	int			repeat;
	int			model;

	int			data_num;
	Unit		unit[0];
} Work ;


static	void Act( Work *work )
{

//printf("work->time_count  :%d:%d:\n",work->time_count, work->unit[ work->now_data_num ].time-1);
	if( work->time_count >= work->unit[ work->now_data_num ].time-1 ){
		work->time_count=0;
		work->now_data_num++;

//printf("work->now_data_num:%d\n",work->now_data_num);
		if( work->now_data_num >= work->data_num ){
			if( work->repeat == 1 ){
				work->now_data_num=0;
			}else{
				GV_DestroyActor( work ) ;
				return;
			}
		}

		if( work->now_data_num == 0 ){
			work->pos = work->unit[0].pos;
			work->rot = work->unit[0].rot;
		}else{
			_sceVu0AddVector( &work->pos, &work->pos, &work->unit[work->now_data_num].pos );
			work->rot.vx += work->unit[work->now_data_num].rot.vx;
			work->rot.vy += work->unit[work->now_data_num].rot.vy;
			work->rot.vz += work->unit[work->now_data_num].rot.vz;

//printf("%6d:%6d:%6d:\n",(int)work->rot.vx,(int)work->rot.vy,(int)work->rot.vz);

		}

	}else{
		work->time_count++;
	}


	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_PutObjs( work->objs );
	DG_GetLightMatrix( (FVECTOR*)work->objs->world.m[3], work->lights );

}

static	void Die( Work *work )
{
	DG_DequeueObjs( work->objs );
	DG_FreeObjs( work->objs );
}

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	int	i;

	if ( GCL_GetOption( 'k' ) != NULL ) {
		work->model = GCL_GetNextInt() ;
	}else{
		printf("NO MODEL!!");
	}

	if ( GCL_GetOption( 'd' ) != NULL ) {
		for( i=0; i<work->data_num; i++ ){
			if( GCL_NextStr() != NULL ){
				work->unit[i].time   = GCL_GetNextInt() ;
				work->unit[i].pos.vx = (float)GCL_GetNextInt() ;
				work->unit[i].pos.vy = (float)GCL_GetNextInt() ;
				work->unit[i].pos.vz = (float)GCL_GetNextInt() ;
				work->unit[i].rot.vx = (short)GCL_GetNextInt() ;
				work->unit[i].rot.vy = (short)GCL_GetNextInt() ;
				work->unit[i].rot.vz = (short)GCL_GetNextInt() ;
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

	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->repeat = GCL_GetNextInt() ;
	}else{
		work->repeat = 0;
	}
}

static	int GetResources( Work *work, int data_num )
{
	DG_DEF		*def ;

	work->data_num=data_num;

	GetOptionValue( work );

	work->pos = work->unit[0].pos;
	work->rot = work->unit[0].rot;

//	def = (DG_DEF*)GV_GetCache( GV_CacheID( 7473930 /*"m4a_nm"*/, 'k' ) ) ;
	def = (DG_DEF*)GV_GetCache( GV_CacheID( work->model, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );
	DG_QueueObjs( work->objs );
	DG_SetLightMatrix( work->objs, work->lights );

	work->time_count=0;
	work->now_data_num=0;

	return 0 ;
}

void *NewModelPosAnimation( void )
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
