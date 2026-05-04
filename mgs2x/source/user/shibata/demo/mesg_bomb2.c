//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mesg_bomb2.c
	デモ用メッセージ起動エフェクトコールキャラ
	
	2000/02/13 T.Shibata
	
	$Id: mesg_bomb2.c,v 1.1.1.3 2002/11/19 11:48:32 Yoshizawa1 Exp $

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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
#include "../../mode/demo/eft_con.h"

typedef	struct	{
	GV_ACT_EX	actor;
	FMATRIX		world;
	int			name;
	int			con_name;
	//CONTROL		*control;
	FVECTOR		*mov;
	SVECTOR		*rot;
	float		temp0;
	float		temp1;
} Work;

enum {
	BOMB_EFT = 0,
	FLYING_SMK,
	BOMB_GAS_EFT,
	CRUSH_FORCE,
	REC,
	SPARK,
	BLOOD_MIST,
	SPHERE_SPLUSH,
	WAVE_SPLUSH,
	BLOOD_STRIP,
	POINT_TIDAL,
	BLOOD_WATER,
	BUBBLE_MANY,
};

//Ｃ４爆発
extern void *NewBombEffect( FVECTOR *pos, int mode );
//壁這う煙
extern void *NewFlyingSmoke( FVECTOR *pos, FVECTOR *vector, int life );
//中煙
extern void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot );
//ライン飛び散り
extern void *NewCrushWithForce( FVECTOR *pos );
//跳弾
extern void *NewSpark( FMATRIX *world );
//火花
extern void *NewTs_Spark( FVECTOR *center, FVECTOR *vec, float pow );
//血煙
extern void *AN_Blood_Mist( FVECTOR *pos );
//ドッポーン
extern void *NewSphereSplush( FVECTOR *pos, float radius, float intense, int );
//波涛
extern void *NewWaveSplash_Demo( FMATRIX *root, float scale, int base_alp );
//任意血
extern void *NewBloodStrip( FMATRIX *world, float *intense, float *width, int life );
//ポイント浸水
extern void *NewPointTidal( FMATRIX *world, float *intense, float *width, int life );
//水中血
extern void *NewBloodWater_demo( FVECTOR *pos, FVECTOR *vec, float pow, int mode );
//追従泡
extern void *NewBubbleMany( FVECTOR *center, int life, int );

#if 0
static CONTROL* SearchControl( int name, int map )
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
	printf("ray_console.c: search faild control !!\n");
	return ( NULL );
}

static int CheckControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return (0) ;
	}
	return (1);
}
#endif

static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num,temp;
	FVECTOR		fvtemp;
	FMATRIX		world;
	float		force;

	if( !DM_GetEftControl( work->con_name ) ){
		GV_DestroyActor( work );
	}

#if 0
	{
#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
		FMATRIX temp;
		DG_SetPos2( work->mov, work->rot );
		DG_GetPos( &temp );
		HZX_ViewMatrix( &temp, 500.0F );
		//printf("x %5d:y %5d:z %5d\n",work->control->rot.vx,work->control->rot.vy,work->control->rot.vz);
		//printf("x %5d:y %5d:z %5d\n",work->control->turn.vx,work->control->turn.vy,work->control->turn.vz);
		//PRINT_PFVEC(0,(FVECTOR*)temp.m[0]);
		//PRINT_PFVEC(1,(FVECTOR*)temp.m[1]);
		//PRINT_PFVEC(2,(FVECTOR*)temp.m[2]);
	}
#endif
	DG_SetPos2( work->mov, work->rot );
	DG_GetPos( &work->world );
	
	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case BOMB_EFT:
			NewBombEffect( work->mov, msg->message[1] );
			break;
		  case FLYING_SMK:
			force = (float)msg->message[2];
			DG_SetPos2( &DG_ZeroVector, work->rot );
#ifdef BP_PS2
			DG_RotVector( &(FVECTOR){0.0f,0.0f,1.0f,0.0f}, &fvtemp, 1 );
#else
			{
				FVECTOR tmp = {0.0f,0.0f,1.0f,0.0f} ;
				DG_RotVector( &tmp, &fvtemp, 1 );
			}
#endif
			_sceVu0ScaleVector( &fvtemp, &fvtemp, force );
			NewFlyingSmoke( work->mov, &fvtemp, msg->message[1] );
			break;
		  case BOMB_GAS_EFT:
			NewBombGasEffect( work->mov, work->rot );
			break;
		  case CRUSH_FORCE:
			NewCrushWithForce( work->mov );
			break;
		  case REC:
			DG_SetPos2( work->mov, work->rot );
			DG_GetPos( &world );
			NewSpark( &world );
			break;
		  case SPARK:
			DG_SetPos2( &DG_ZeroVector, work->rot );
#ifdef BP_PS2
			DG_RotVector( &(FVECTOR){0.0f,0.0f,1.0f,0.0f}, &fvtemp, 1 );
#else
			{
				FVECTOR tmp = {0.0f,0.0f,1.0f,0.0f} ;
				DG_RotVector( &tmp, &fvtemp, 1 );
			}
#endif
			NewTs_Spark( work->mov, &fvtemp, (float)msg->message[1] );
			break;
		  case BLOOD_MIST:
			AN_Blood_Mist( work->mov );
			break;
		  case SPHERE_SPLUSH:
			NewSphereSplush( work->mov, (float)msg->message[1], (float)msg->message[2], 0 );
			break;
		  case WAVE_SPLUSH:
			DG_SetPos2( work->mov, work->rot );
			DG_GetPos( &world );
			NewWaveSplash_Demo( &world, (float)msg->message[1], msg->message[2] );
			break;
		  case BLOOD_STRIP:
			work->temp0 = (float)msg->message[1];
			work->temp1 = (float)msg->message[2];
			NewBloodStrip( &work->world, &work->temp0, &work->temp1, msg->message[3] );
			break;
		  case POINT_TIDAL:
			work->temp0 = (float)msg->message[1];
			work->temp1 = (float)msg->message[2];
			NewPointTidal( &work->world, &work->temp0, &work->temp1, msg->message[3] );
			break;
		  case BLOOD_WATER:
			NewBloodWater_demo( work->mov, (FVECTOR*)work->world.m[2], (float)msg->message[1], 0 );
			break;
		  case BUBBLE_MANY:
			temp = msg->message[2]<<24;
			temp |= msg->message[3]<<16;
			temp |= msg->message[4]<<8;
			NewBubbleMany( work->mov, msg->message[1], temp );
			break;
		  default:
			printf("mesg_bomb.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	//GV_WaitMessage( work, work->name );
}

static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
//	work->control = SearchControl( work->con_name, 0);
//	if(!work->control) return -1;

	EFTCONTROL *ctrl = DM_GetEftControl( work->con_name );
	if(!ctrl) return -1;

	work->mov = &ctrl->mov;
	work->rot = &ctrl->rot;	
	return 0;
}

void *NewMesgBomb2( int name, int con_name )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ));

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->con_name = con_name;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
	}

	return (void *)work ;
}
