//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	wind_man.c
	風制御
	1999/08/19 S.Okajima
	$Id: wind_man.c,v 1.1.1.3 2002/11/19 11:47:16 Yoshizawa1 Exp $

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

#define	DECAY_TIME	(90.0f)

extern float	OK_slow_param;

#if 0 /*-->ok_init.cに実体を移行しました。 yano2002.03.28*/
FVECTOR	G_wind    = { 0.0f, 0.0f, 0.0f, 0.0f };	/* 風速（単位フレームに移動する距離） */
SVECTOR	G_wind_rot= { 0, 0, 0, 0 };	/* 風向 */
int		G_wind_intense=0;	/* 強さ */
int		G_wind_intense_max=1;	/* 強さ */
int		G_wind_sw=0;							/* 突風発生中に立つ */
FMATRIX	G_wind_matrix;
#else
extern FVECTOR	G_wind ;	/* 風速（単位フレームに移動する距離） */
extern SVECTOR	G_wind_rot;	/* 風向 */
extern int		G_wind_intense ;	/* 強さ */
extern int		G_wind_intense_max ;	/* 強さ */
extern int		G_wind_sw ;			/* 突風発生中に立つ */
extern FMATRIX	G_wind_matrix;
#endif

enum {
	WIND_MODE_NORMAL=0,
	WIND_MODE_FORCE
} ;

enum {
	WIND_MODE_SUB1=0,
	WIND_MODE_SUB2,
	WIND_MODE_SUB3,
	WIND_MODE_SUB4,
	WIND_MODE_SUB5,
	WIND_MODE_SUB6,
	WIND_MODE_SUB7,
	WIND_MODE_SUB8
} ;



typedef	struct	{
	GV_ACT_EX	actor;

	int		name;
	int		where;

	int		go_flag;

	FVECTOR	before_direction;	/* １フレーム前 合成風 方向 */

	FVECTOR	m_direction;		/* 合成風 方向 */
	FVECTOR	m_intense;			/* 合成風 風速 */
	float	before_intense;			/* 合成風 風速 */

	FVECTOR	direction;			/* 通常風 方向 */
	FVECTOR	force_direction;	/* 突風   方向 */
	FVECTOR	randam_width;
	FVECTOR	intense_max;			/* 通常風 単位フレームに吹く距離 */
	float	force_intense_max;		/* 突風   単位フレームに吹く距離 */
	float	randam_width_max;
	float	force_wind_frequency;
	float	force_wind_count;
	float	force_wind_continue_max;
	int		randam_width_count;

	int		mode1;
	int		mode2;

	float	force_wind_temp_rot;

	int		f_count;
} Work ;

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	int			buf[ 3 ] ;

	work->intense_max=DG_ZeroVector;
	if ( GCL_GetOption( 'i' ) != NULL ) {
		work->intense_max.vx = 0.0f ;
		work->intense_max.vy = -(float)GCL_GetNextInt() ;
		work->intense_max.vz = 0.0f ;
		work->intense_max.vw = 0.0f ;
	}

	work->force_intense_max=0.0f;
	if ( GCL_GetOption( 'j' ) != NULL ) {
		work->force_intense_max = (float)GCL_GetNextInt() ;
	}

	work->direction=DG_ZeroVector;
	if ( GCL_GetOption( 'r' ) != NULL ) {
		GCL_GetNextIV( buf ) ;
		work->direction.vx=(float)(buf[0] & 4095);
		work->direction.vy=(float)(buf[1] & 4095);
		work->direction.vz=(float)(buf[2] & 4095);
	}

	work->randam_width_max=0.0f;
	if ( GCL_GetOption( 'a' ) != NULL ) {
		work->randam_width_max = (float)GCL_GetNextInt() ;
	}

	work->force_wind_frequency=100.0f;
	if ( GCL_GetOption( 'b' ) != NULL ) {
		work->force_wind_frequency = 1.0f / (float)GCL_GetNextInt() ;
	}

	work->force_wind_continue_max=100.0f;
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->force_wind_continue_max = (float)GCL_GetNextInt() ;
	}

}

/* 通常風 */
static int WindNormal( Work *work )
{
//TEST_Printf( "   :%d",work->mode2 );

	switch( work->mode2 ){
		case WIND_MODE_SUB1:
			work->randam_width_count=30;
			work->randam_width.vx = frnd() * work->randam_width_max;
			work->randam_width.vy = frnd() * work->randam_width_max;
			work->randam_width.vz = frnd() * work->randam_width_max;
			work->mode2=WIND_MODE_SUB2;
		  break;
		case WIND_MODE_SUB2:
			work->randam_width_count--;
			if( work->randam_width_count < 0 ) work->mode2=WIND_MODE_SUB1;
		  break;
		default:
		  break;
	}


	_sceVu0ScaleVector( &work->randam_width, &work->randam_width, 0.99f );
	_sceVu0AddVector( &work->m_direction, &work->direction, &work->randam_width ) ;
	DG_COPY_VEC( &work->m_intense, &work->intense_max );

	if( rnd() < work->force_wind_frequency ){
		work->force_wind_count=rnd() * work->force_wind_continue_max*0.5f + work->force_wind_continue_max*0.5f + DECAY_TIME;
		work->force_direction.vy=work->direction.vy+(float)(irnd()%1024-512);
		work->mode2=WIND_MODE_SUB1;
		work->force_wind_temp_rot=(float)(irnd()%32+4)*0.125f;	/* 微妙 */
		if(irnd()%2){
			work->force_wind_temp_rot *= (-1.0f);
		}
		return WIND_MODE_FORCE;
	}

	return WIND_MODE_NORMAL;
}

/* 突風 */
static int WindForce( Work *work )
{
	FVECTOR	fvtemp;

//TEST_Printf( "   :%d",work->mode2 );

	fvtemp=DG_ZeroVector;
	work->force_wind_count--;
	switch( work->mode2 ){
		case WIND_MODE_SUB1:
//TEST_Locate( 200 ,10, 0 );
//TEST_Printf( "%d\n",(int)work->force_wind_count );
			G_wind_sw=1;
			fvtemp.vy=-work->force_intense_max;
			work->force_direction.vy=work->force_direction.vy+work->force_wind_temp_rot;
			if( work->force_wind_count <= DECAY_TIME ) work->mode2=WIND_MODE_SUB2;
		  break;
		case WIND_MODE_SUB2:
			G_wind_sw=1;
			fvtemp.vy=-work->force_intense_max * work->force_wind_count / DECAY_TIME;
			work->force_direction.vy=work->force_direction.vy+work->force_wind_temp_rot;
			if( work->force_wind_count <= 0 ){
				work->mode2=WIND_MODE_SUB1;
				return WIND_MODE_NORMAL;
			}
		  break;
		default:
		  break;
	}

	_sceVu0AddVector( &work->m_direction, &work->direction, &work->force_direction ) ;
	_sceVu0AddVector( &work->m_intense, &fvtemp, &work->intense_max ) ;

	return WIND_MODE_FORCE;
}

static void Act( Work *work )
{
	SVECTOR	svtemp2;
	float	ftemp0;
	float	ftemp1;
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
		  default:
			work->go_flag = 1;
			break;
		}
		msg--;
	}

	if( work->go_flag ){
		G_wind_sw=0;
		switch( work->mode1 ){
			case WIND_MODE_NORMAL:
				work->mode1 = WindNormal( work );
			  break;
			case WIND_MODE_FORCE:
				work->mode1 = WindForce( work );
			  break;
			default:
			  break;
		}

		vu0_FVtoSV0( &work->before_direction, &G_wind_rot );
#ifdef KP_XBOX //yano
		if( work->m_direction.vy < 1024.0f ){/*PS2版をエミュレート*/
			work->m_direction.vy = 4095.0f;
		}
#endif
		vu0_FVtoSV0( &work->m_direction, &svtemp2 );
		OK_DirectionSmoother( &G_wind_rot, &svtemp2, 0.9f + 0.1f*(1.0f-OK_slow_param) );
		vu0_SV0toFV( &G_wind_rot, &work->before_direction );
		DG_SetPos2( &DG_ZeroVector, &G_wind_rot );
		DG_GetPos( &G_wind_matrix );
		DG_PutVector( &work->m_intense, &G_wind, 1 );

		G_wind_intense_max = (int)work->force_intense_max;
#if 0
		G_wind_intense=(int)work->m_intense.vy;
#else
		ftemp0 = ( sinf( work->f_count * PI * 0.02f ) + sinf( work->f_count * PI * 0.0090001f) ) * 0.5f;
		ftemp0 = ( ftemp0 > 0.0f )? ftemp0: -ftemp0;
		ftemp1 = 0.6f * OK_slow_param * OK_slow_param * OK_slow_param;
		G_wind_intense=(int)( (work->m_intense.vy*ftemp0) * ftemp1
		                   +  (float)G_wind_intense * (1.0f - ftemp1) );
		G_wind_intense = ( G_wind_intense > G_wind_intense_max )? G_wind_intense_max: G_wind_intense;
#endif

		if( G_wind_intense<0 ) G_wind_intense = -G_wind_intense;

	}else{
		DG_COPY_VEC( &G_wind, &DG_ZeroVector );
		G_wind_rot.vx = 0;
		G_wind_rot.vy = 0;
		G_wind_rot.vz = 0;
		G_wind_intense=0;	/* 強さ */
		G_wind_intense_max=1;	/* 強さ */
		G_wind_sw=0;							/* 突風発生中に立つ */
		DG_COPY_MAT( &G_wind_matrix, &DG_UnitMatrix );
	}


#if 0

	{
		FVECTOR	len_vec;
		FVECTOR	view_pos;
		len_vec.vx = 0.0f;
		len_vec.vy = -G_wind_intense;
		len_vec.vz = 0.0f;
		len_vec.vw = 1.0f;
		view_pos.vx = GM_PlayerPosition.vx;
		view_pos.vy = GM_PlayerPosition.vy + 1000.0f;
		view_pos.vz = GM_PlayerPosition.vz;
		DG_SetPos2( &view_pos, &G_wind_rot );
		DG_PutVector( &len_vec, &len_vec, 1 );
		AN_Test_Eye2( &len_vec, 3 );
		AN_Test_Eye3( &view_pos );

		printf("%d %d\n",G_wind_intense,G_wind_intense_max);

#if 0
		{
			SVECTOR	rot;
			extern void OK_GetLocalWindDirection( FVECTOR *pos, SVECTOR *output );
			OK_GetLocalWindDirection( &GM_PlayerPosition, &rot );
			printf("%d %d %d\n",rot.vx,rot.vy,rot.vz);
		}
#endif

	}
#endif


	work->f_count += 1.0f;


}

static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	SVECTOR	svtemp;

	work->f_count = 0.0f;
	work->go_flag = 1;

//	vu0_RandInit(1.573082904243469238);

	GetOptionValue( work ) ;

	vu0_Ldv1( &work->direction );
	vu0_Stv0( &work->m_direction );
	vu0_Stv0( &work->before_direction );

	work->mode1=WIND_MODE_NORMAL;
	work->mode2=WIND_MODE_SUB1;

	work->randam_width_count=0;
	work->force_wind_count=0.0f;

	vu0_FVtoSV0( &work->direction, &svtemp );
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_PutVector( &work->intense_max, &G_wind, 1 );

	return (0);
}

void *NewWindManager( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_PREV, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->where = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

