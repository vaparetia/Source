//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	wind_local2.c
	ローカル風
	2000/02/14 S.Okajima
	$Id: wind_local2.c,v 1.1.1.3 2002/11/19 11:47:16 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
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
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define	DECAY_TIME	(90.0f)

extern FVECTOR	G_wind;

typedef	struct	{
	GV_ACT_EX	actor;
	int			name;

	int			go_flag;

	FVECTOR		boundary0;
	FVECTOR		boundary1;
	FMATRIX		world;
	float		intense;
	float		temp_intense;

	EFTCONTROL	*ctrl;
} Work ;
Work	*OK_LOCAL_WIND2_WORK = NULL;

int		OK_LOCAL_WIND2_FLAG = 1;

static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;


	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			work->go_flag = 0;
			break;
		  case 1:
			work->go_flag = 1;
			break;
		  default:
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
			break ;
		}
		msg--;
	}

	if( OK_LOCAL_WIND2_FLAG!=1 ){
		return;
	}


	if( work->go_flag ){
		work->temp_intense = work->intense;
	}else{
		work->temp_intense = 0.0f;
	}
}

static void Die( Work *work )
{
	OK_LOCAL_WIND2_WORK = NULL;
	OK_LOCAL_WIND2_FLAG = 1;
}

static int GetResources( Work *work, int con_name )
{
	work->ctrl = DM_GetEftControl( con_name );

	if( !work->ctrl ) return -1;

	DG_SetPos2( &work->ctrl->mov, &work->ctrl->rot );
	DG_GetPos( &work->world );

	work->go_flag  = 1 ;

	return 0;
}

//----------------------------------------------------------
/* 入力：*pos    検査座標 非破壊  */
/* 出力：*output 検査座標における風ベクトル（バンダリにヒットしなかったらグローバルの風を返す） */
/* 返り値 1:成功（常に成功に変更 2000.6/29） */
int OK_GetLocalWind2( FVECTOR *pos, FVECTOR *output )
{
	Work	*work;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;

	if( OK_LOCAL_WIND2_WORK!=NULL ){
		work = OK_LOCAL_WIND2_WORK;

		_sceVu0AddVector( &fvtemp0, &work->ctrl->mov, &work->boundary0 ) ;
		_sceVu0AddVector( &fvtemp1, &work->ctrl->mov, &work->boundary1 ) ;

		if( vu0_CheckBoundingBox( pos, &fvtemp0, &fvtemp1 ) ){
			output->vx=0.0f;
			output->vy=0.0f;
			output->vz = work->temp_intense*(1.0f + rnd()) * 0.5f;
			DG_SetPos2( &DG_ZeroVector, &work->ctrl->rot );
			DG_RotVector( output, output, 1 );
			return 1;
		}
	}
	output->vx = G_wind.vx;
	output->vy = G_wind.vy;
	output->vz = G_wind.vz;
	return 0;
}

void *NewLocalWind2_Demo( int con_name, FVECTOR *size, float intense, int name )
{
	Work		*work ;
	float	ftemp;
	FVECTOR h_size ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_PREV2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		OK_LOCAL_WIND2_WORK = work;

		work->name  = name ;

		_sceVu0ScaleVector( &h_size, size, 0.5f ) ;
		_sceVu0AddVector( &work->boundary0, &DG_ZeroVector, &h_size ) ;
		_sceVu0SubVector( &work->boundary1, &DG_ZeroVector, &h_size ) ;
		work->intense      = intense ;
		work->temp_intense = work->intense ;

		if( work->boundary0.vx > work->boundary1.vx ){
		    ftemp = work->boundary0.vx;
		    work->boundary0.vx = work->boundary1.vx;
		    work->boundary1.vx = ftemp;
		}
		if( work->boundary0.vy > work->boundary1.vy ){
		    ftemp = work->boundary0.vy;
		    work->boundary0.vy = work->boundary1.vy;
		    work->boundary1.vy = ftemp;
		}
		if( work->boundary0.vz > work->boundary1.vz ){
		    ftemp = work->boundary0.vz;
		    work->boundary0.vz = work->boundary1.vz;
		    work->boundary1.vz = ftemp;
		}

		if ( GetResources( work, con_name ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}


	}
	return (void *)work ;
}

