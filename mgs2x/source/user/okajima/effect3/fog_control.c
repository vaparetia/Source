//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fog_control.c
	フォグコントローラー
	2001/05/11 S.Okajima
	$Id: fog_control.c,v 1.1.1.3 2002/11/19 11:47:31 Yoshizawa1 Exp $
*/


#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#endif


#include	"gameheader.h"
#include	"libmt.h"

extern void DG_SetClipParam( float ,float );
extern int DM_FrameSkip;

typedef	struct	{
	GV_ACT_EX	actor ;

	CVECTOR		col;

	float		float_near;
	float		float_far;
	float		up_far;
	float		down_far;
	int			far_clip;
} Data ;

typedef	struct	{
	GV_ACT_EX	actor ;

	int			name;
	int			where;

	int			far_clip;

	int			insert_time;
	int			phase0;
	int			phase1;
	int			phase2;

	CVECTOR		col;
	float		float_near;
	float		float_far;

	int			time;
	int			time_max;

	int			num;
	int			num_max;

	Data		data_insert;

	Data		data_keep;

	Data		data;
	Data		data_dest[0];
} Work ;

Work *OK_FOG_CONTROL_WORK = NULL;

/*----------------------------------------------------------------*/
static void CheckMesgParam( Work *work )
{
	Data	*data;
	Data	*data_keep;
	GV_MSG *msg;
	int mes_num;
	int num;
	int	time;


	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
//printf("num:%d\n",num);
		switch( num ){
		  case 0:

			if( work->num == msg->message[1] ) break;
			work->num = msg->message[1];
			if( work->num < 0 ){
				printf("ERR:ERR:ERR:ERR:ERR:ERR:ERR:fog_control.c:\n");
				work->num = 0;
			}

			data_keep = &work->data_keep;
			data      = &work->data     ;

			data->col.r    = data_keep->col.r;
			data->col.g    = data_keep->col.g;
			data->col.b    = data_keep->col.b;

			data->float_near     = data_keep->float_near;
			data->float_far      = data_keep->float_far;

			data->up_far   = data_keep->up_far;
			data->down_far = data_keep->down_far;

			time           = msg->message[2];

			time = DIRECT_TICK( time );
			if( time < 1 ) time = 1;
			work->time       = time;
			work->time_max   = time;
			if( work->time_max <= 0 ) work->time_max=1;
//printf("%d::%d::\n",work->num,time);
			break;
		  default:
			break;
		}
		msg--;
	}
}


static	void Act( Work *work )
{
	int		num;
	Data	insert;
	Data	*data_insert;
	Data	*data;
	Data	*data_keep;
	Data	*data_dest;
	int		time0;
	int		time1;
	int		time_max;
	FVECTOR	fvtemp;
	float	ftemp;
	float	calc_far;

	CheckMesgParam( work );

	work->time -= DM_FrameSkip ;
	work->time--;
	if( work->time < 0 ){
		work->time = 0;
//		GV_DestroyActor( work ) ;
//		return ;
	}


	data_keep = &work->data_keep;

	num = work->num;
	data      =&work->data;
	data_dest = work->data_dest;
	data_dest+= num;

	num = data_dest->col.r + (data->col.r - data_dest->col.r) * work->time / work->time_max;
	if( num > 255 ){
		num = 255;
	}else if( num < 0 ){
		num = 0;
	}
	data_keep->col.r = num;

	num = data_dest->col.g + (data->col.g - data_dest->col.g) * work->time / work->time_max;
	if( num > 255 ){
		num = 255;
	}else if( num < 0 ){
		num = 0;
	}
	data_keep->col.g = num;

	num = data_dest->col.b + (data->col.b - data_dest->col.b) * work->time / work->time_max;
	if( num > 255 ){
		num = 255;
	}else if( num < 0 ){
		num = 0;
	}
	data_keep->col.b = num;

	data_keep->float_near = data_dest->float_near + (data->float_near - data_dest->float_near) * (float)work->time / (float)work->time_max;
	data_keep->float_far  = data_dest->float_far  + (data->float_far  - data_dest->float_far ) * (float)work->time / (float)work->time_max;
	if( data_keep->float_near == data_keep->float_far ) data_keep->float_far = data_keep->float_near + 1.0f;

	data_keep->up_far   = data_dest->up_far   + (data->up_far   - data_dest->up_far  ) * (float)work->time / (float)work->time_max;
	data_keep->down_far = data_dest->down_far + (data->down_far - data_dest->down_far) * (float)work->time / (float)work->time_max;



	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 1.0f;
	fvtemp.vw = 1.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	if( fvtemp.vy > 0.0f ){
		calc_far = data_keep->up_far   * fvtemp.vy  +  data_keep->float_far * (1.0f - fvtemp.vy);
//if(GV_Time%120==0)printf("a:%f %f %f %f\n",data_keep->up_far,data_keep->far,fvtemp.vy,calc_far);
	}else{
		fvtemp.vy =-fvtemp.vy;
		calc_far = data_keep->down_far * fvtemp.vy  +  data_keep->float_far * (1.0f - fvtemp.vy);
//if(GV_Time%120==0)printf("b:%f %f %f %f\n",data_keep->down_far,data_keep->far,fvtemp.vy,calc_far);
	}



	data_insert = &work->data_insert;
	insert.col.r = 0;
	insert.col.g = 0;
	insert.col.b = 0;
	insert.float_near  = 0.0f;
	insert.float_far   = 0.0f;
	if( work->insert_time > 0 ){

		work->insert_time-= DM_FrameSkip ;
		work->insert_time--;

		time0 = work->insert_time - work->phase1 - work->phase2;
		if( time0 > 0 ){
			time1 = work->phase0 - time0;
			time_max = work->phase0;
			data_keep->col.r = (data_keep->col.r * time0 + data_insert->col.r * time1) / time_max;
			data_keep->col.g = (data_keep->col.g * time0 + data_insert->col.g * time1) / time_max;
			data_keep->col.b = (data_keep->col.b * time0 + data_insert->col.b * time1) / time_max;
			data_keep->float_near  = (data_keep->float_near  * time0 + data_insert->float_near  * time1) / time_max;
			data_keep->float_far   = (data_keep->float_far   * time0 + data_insert->float_far   * time1) / time_max;
		}else{
			time0 = work->insert_time - work->phase2;
			if( time0 > 0 ){
				time_max = work->phase1;
				data_keep->col.r = data_insert->col.r;
				data_keep->col.g = data_insert->col.g;
				data_keep->col.b = data_insert->col.b;
				data_keep->float_near  = data_insert->float_near;
				data_keep->float_far   = data_insert->float_far;
			}else{
				time0 = work->insert_time;
				if( time0 > 0 ){
					time1 = work->phase2 - time0;
					time_max = work->phase2;
					data_keep->col.r = (data_keep->col.r * time1 + data_insert->col.r * time0) / time_max;
					data_keep->col.g = (data_keep->col.g * time1 + data_insert->col.g * time0) / time_max;
					data_keep->col.b = (data_keep->col.b * time1 + data_insert->col.b * time0) / time_max;
					data_keep->float_near  = (data_keep->float_near  * time1 + data_insert->float_near  * time0) / time_max;
					data_keep->float_far   = (data_keep->float_far   * time1 + data_insert->float_far   * time0) / time_max;
				}else{
					work->insert_time = 0;
				}
			}
		}
	}




//printf("%4d %4d %4d::%6.1f %6.1f\n",data_keep->col.r, data_keep->col.g, data_keep->col.b , data_keep->float_near, data_keep->float_far);


	DG_SetFogColor( data_keep->col.r,data_keep->col.g, data_keep->col.b );
	DG_SetFogParam( data_keep->float_near, calc_far );
	if( data->far_clip ){
		ftemp = DG_MAX( calc_far,  data_keep->float_near );
		ftemp = DG_MAX( ftemp, 0.0f ) + 1000.0f;
		DG_SetClipParam( DRAW_NEAR_CLIP, ftemp );
	}

}

/*----------------------------------------------------------------*/
//       phase1
//       __________________________
//      ●●●●●●●●●●●●●●
//     ●                            ●
//    ●                                ●
//   ●                                    ●
//   ~~~~                          ~~~~~~~~~
//   phase0                        phase2
//
int	OK_FogDataInsert( CVECTOR col, float float_near, float float_far, int phase0, int phase1, int phase2 )
{
	Work	*work;
	Data	*data_insert;

	if( OK_FOG_CONTROL_WORK==NULL ) return 0;
	work = OK_FOG_CONTROL_WORK;


	data_insert = &work->data_insert;


	work->insert_time = phase0 + phase1 + phase2;

	data_insert->col  = col;
	data_insert->float_near = float_near;
	data_insert->float_far  = float_far;

	work->phase0      = phase0;
	work->phase1      = phase1;
	work->phase2      = phase2;

	return 1;
}


/*----------------------------------------------------------------*/
static	void Die( Work *work )
{
	OK_FOG_CONTROL_WORK = NULL;
}


static	int GetResources( Work *work )
{
	Data	*data;
	int		i;

	work->far_clip = 0;
	if ( GCL_GetOption( 'z' ) != NULL ) {
		work->far_clip = 1;
	}


	work->insert_time = 0;

	work->time     = 0;
	work->time_max = 1;

	work->num = 0;

	work->data.col.r = DG_FogColor.r;
	work->data.col.g = DG_FogColor.g;
	work->data.col.b = DG_FogColor.b;
	work->data.float_near  = (255.0f - DG_FogParam2) / DG_FogParam1;
	work->data.float_far   = -DG_FogParam2 / DG_FogParam1;

	data = work->data_dest;
	if ( GCL_GetOption( 'd' ) != NULL ) {
		for( i=0; i<work->num_max; i++ ){
			data->col.r    = GCL_GetNextInt();
			data->col.g    = GCL_GetNextInt();
			data->col.b    = GCL_GetNextInt();

			data->float_near     = (float)GCL_GetNextInt();
			data->float_far      = (float)GCL_GetNextInt();

			data->up_far   = (float)GCL_GetNextInt();
			data->down_far = (float)GCL_GetNextInt();

//			data->far_clip = GCL_GetNextInt();
			data++;
		}
	}


	return 0;
}

void *NewFogControl( int name, int where )
{
	Work	*work ;
	int		num;
	int		buf_size;

	if( OK_FOG_CONTROL_WORK == NULL ){

		if ( GCL_GetOption( 'n' ) != NULL ) {
			num     = GCL_GetNextInt();
			if( num <= 0  ) return NULL;
		}else{
			return NULL;
		}

		buf_size = sizeof( Work ) + sizeof( Data ) * num;

		work = OK_FOG_CONTROL_WORK = (Work *)GV_NewActor( GV_ACTOR_AFTER2, buf_size ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			work->name  = name;
			work->where = where;

			work->num_max = num;

			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)OK_FOG_CONTROL_WORK ;
	}else{
		return (void *)OK_FOG_CONTROL_WORK ;
	}
}
