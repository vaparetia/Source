//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_fog_set.c
	フォグセット
	2000/04/09 S.Okajima
	$Id: d_fog_set.c,v 1.1.1.3 2002/11/19 11:46:52 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"libmt.h"

extern void DG_SetClipParam( float ,float );
extern int DM_FrameSkip;

typedef	struct	d_fog_set_Work{
	GV_ACT_EX		actor ;

	int			col_r_dest;
	int			col_r;
	int			col_g_dest;
	int			col_g;
	int			col_b_dest;
	int			col_b;
	int			col_a_dest;
	int			col_a;

	float		float_near;
	float		near_dest;
	float		float_far;
	float		far_dest;

	int			time;
	int			time_max;

	int			far_clip_flag;

} Work ;

Work *OK_SET_FOG_WORK = NULL;
extern int OK_fog_unable_flag;

void OK_FogStatusSet( int unable_flag ){
	if( unable_flag ){
		OK_fog_unable_flag = 1;
	}else{
		OK_fog_unable_flag = 0;
	}
}

static	void Act( Work *work )
{
	int	col_r;
	int	col_g;
	int	col_b;
	float	float_near;
	float	float_far;

	if( OK_fog_unable_flag ) return;

	work->time -= DM_FrameSkip ;
	work->time--;
	if( work->time < 0 ){
		work->time = 0;
//printf("ok_:fog destroy\n");
//		GV_DestroyActor( work ) ;
//		return ;
	}
	col_r = work->col_r_dest + (work->col_r - work->col_r_dest) * work->time / work->time_max;
	col_g = work->col_g_dest + (work->col_g - work->col_g_dest) * work->time / work->time_max;
	col_b = work->col_b_dest + (work->col_b - work->col_b_dest) * work->time / work->time_max;
//printf("ok_:%d::%d %d::%d %d\n",col_b,work->col_b_dest,work->col_b,work->time,work->time_max);

	float_near = work->near_dest + (work->float_near - work->near_dest) * (float)work->time / (float)work->time_max;
	float_far  = work->far_dest  + (work->float_far  - work->far_dest ) * (float)work->time / (float)work->time_max;
//	if( DG_FABS( float_near ) - DG_FABS( float_far )   >   1.0f ) float_far = float_near + 1.0f;
	if( float_near == float_far ) float_far = float_near + 1.0f;

	DG_SetFogColor( col_r, col_g, col_b );
	DG_SetFogParam( float_near, float_far );
//	DG_SetClipParam( DRAW_NEAR_CLIP, far*1.2f );


	if( work->time < 0 ){
		GV_DestroyActor( work ) ;
	}

}

static	void Die( Work *work )
{
	OK_SET_FOG_WORK = NULL;
}

static void Initialize( Work *work )
{
	work->col_r_dest = DG_FogColor.r;
	work->col_g_dest = DG_FogColor.g;
	work->col_b_dest = DG_FogColor.b;

	work->col_r = DG_FogColor.r;
	work->col_g = DG_FogColor.g;
	work->col_b = DG_FogColor.b;

	work->time            = 0;
	work->time_max        = 1;
}

static void SetData( Work *work, int col_r, int col_g, int col_b, float float_near, float float_far, int time )
{
	work->col_r = DG_FogColor.r;
	work->col_g = DG_FogColor.g;
	work->col_b = DG_FogColor.b;
	work->float_near  = (255.0f - DG_FogParam2) / DG_FogParam1;
	work->float_far   = -DG_FogParam2 / DG_FogParam1;

	work->col_r_dest = col_r;
	if( work->col_r_dest > 255 ) work->col_r_dest = 255;
	work->col_g_dest = col_g;
	if( work->col_g_dest > 255 ) work->col_g_dest = 255;
	work->col_b_dest = col_b;
	if( work->col_b_dest > 255 ) work->col_b_dest = 255;
	work->near_dest = float_near;
	work->far_dest = float_far;


	time = DIRECT_TICK( time );
	if( time <= 0 ) time = 1;
	work->time       = time;
	work->time_max   = time;
	if( work->time_max <= 0 ) work->time_max=1;
}

static	int GetResources( Work *work, int col_r, int col_g, int col_b, float float_near, float float_far, int time )
{
	Initialize( work );
	SetData( work, col_r, col_g, col_b, float_near, float_far, time );
	return 0;
}

/*
int col_r  :目標カラーＲ成分
int col_g  :目標カラーＧ成分
int col_b  :目標カラーＢ成分
float float_near :目標ニア  値
float float_far  :目標ファー値
int time   :変化時間（フレーム指定）
*/
void *NewFogSet_Demo( int col_r, int col_g, int col_b, float float_near, float float_far, int time )
{
	Work		*work ;

	if( OK_SET_FOG_WORK == NULL ){
		work = OK_SET_FOG_WORK = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			if ( GetResources( work, col_r, col_g, col_b, float_near, float_far, time ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)work ;
	}else{
		SetData( OK_SET_FOG_WORK, col_r, col_g, col_b, float_near, float_far, time  );
		return (void *)OK_SET_FOG_WORK ;
	}
}
