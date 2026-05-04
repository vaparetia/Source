//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	wind_local.c
	ローカル風
	2000/02/14 S.Okajima
	$Id: wind_local.c,v 1.1.1.3 2002/11/19 11:47:16 Yoshizawa1 Exp $

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

extern void *OK_LOCAL_WIND2_WORK;
extern int OK_GetLocalWind2( FVECTOR *pos, FVECTOR *output );

extern FVECTOR	G_wind;
extern SVECTOR	G_wind_rot;
extern int		G_wind_intense;
extern int		G_wind_intense_max;
extern int		G_wind_sw;
extern FMATRIX	G_wind_matrix;

typedef	struct	{
	FVECTOR		boundary0;
	FVECTOR		boundary1;
	float		rot_y;
	float		rot_rand;
	SVECTOR		local_rot;
	float		intense;
	float		temp_intense;
} Unit ;

typedef	struct	{
	GV_ACT_EX	actor;
	int		name;
	int		where;

	int		go_flag;
	int		unit_num;
	Unit	unit[0];
} Work ;
Work	*OK_LOCAL_WIND_WORK = NULL;


//----------------------------------------------------------
/* 返り値 1:成功 */
int OK_SetDataLocalWind( FVECTOR *boundary0, FVECTOR *boundary1, float intense, SVECTOR *rot )
{
	Work	*work;

	if( OK_LOCAL_WIND_WORK!=NULL ){
		work = OK_LOCAL_WIND_WORK;

		DG_COPY_VEC( &work->unit->boundary0, boundary0 );
		DG_COPY_VEC( &work->unit->boundary1, boundary1 );
		work->unit->intense      = intense;
		work->unit->temp_intense = intense;
		work->unit->local_rot.vx = rot->vx;
		work->unit->local_rot.vy = rot->vy;
		work->unit->local_rot.vz = 0;
		return 1;
	}else{
		return 0;
	}
}





static void Act( Work *work )
{
	int	i;
	Unit	*unit;
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

	if( work->go_flag ){
		unit = work->unit;
		for(i=0; i<work->unit_num; i++){
			unit->temp_intense = unit->intense;
			unit->local_rot.vx = (short)( unit->rot_rand * rnd() );
			unit->local_rot.vy = (short)( unit->rot_rand * rnd() + unit->rot_y );
			unit->local_rot.vz = 0;
			unit++;
		}
	}else{
		unit = work->unit;
		for(i=0; i<work->unit_num; i++){
			unit->temp_intense = 0.0f;
			unit++;
		}
	}
}

static void Die( Work *work )
{
	OK_LOCAL_WIND_WORK = NULL;
}

//----------------------------------------------------------
/* 入力：*pos    検査座標 非破壊  */
/* 出力：*output 検査座標における風ベクトル（バンダリにヒットしなかったらグローバルの風を返す） */
/* 返り値 1:成功（常に成功に変更 2000.6/29） */
int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output )
{
	int		i;
	Work	*work;
	Unit	*unit;

	if( OK_LOCAL_WIND2_WORK!=NULL ){
		if( OK_GetLocalWind2( pos, output ) ){
			return 1;
		}
	}

	if( OK_LOCAL_WIND_WORK!=NULL ){
		work = OK_LOCAL_WIND_WORK;
		unit = work->unit;
		for(i=0; i<work->unit_num; i++){
			if( vu0_CheckBoundingBox( pos, &unit->boundary0, &unit->boundary1 ) ){
				output->vx=0.0f;
				output->vy=0.0f;
				output->vz = unit->temp_intense*(1.0f + rnd()) * 0.5f;
				DG_SetPos2( &DG_ZeroVector, &unit->local_rot );
				DG_RotVector( output, output, 1 );
				return 1;
			}
			unit++;
		}
	}
	output->vx = G_wind.vx;
	output->vy = G_wind.vy;
	output->vz = G_wind.vz;
	return 0;
}
//----------------------------------------------------------
/* 入力：*pos    検査座標 非破壊  */
/* 出力：*output 検査座標における風ベクトル（バンダリにヒットしなかったらグローバルの風を返す） */
/* 返り値 1:成功（常に成功に変更 2000.6/29） */
int OK_GetLocalWind_SH( FVECTOR *pos, FVECTOR *output )
{
	int		i;
	Work	*work;
	Unit	*unit;

	if( OK_LOCAL_WIND2_WORK!=NULL ){
		if( OK_GetLocalWind2( pos, output ) ){
			return 1;
		}
	}

	if( OK_LOCAL_WIND_WORK!=NULL ){
		work = OK_LOCAL_WIND_WORK;
		unit = work->unit;
		for(i=0; i<work->unit_num; i++){
			if( vu0_CheckBoundingBox( pos, &unit->boundary0, &unit->boundary1 ) ){
				float ret;
				output->vx=0.0f;
				output->vy=0.0f;
				output->vz = ret =unit->temp_intense*(1.0f + rnd()) * 0.5f;
				DG_SetPos2( &DG_ZeroVector, &unit->local_rot );
				DG_RotVector( output, output, 1 );
				return (int)ret;
			}
			unit++;
		}
	}
	output->vx = G_wind.vx;
	output->vy = G_wind.vy;
	output->vz = G_wind.vz;
	return G_wind_intense;
}
//----------------------------------------------------------
/* 入力：*pos    検査座標 非破壊  */
/* 出力：*output 検査座標における風の回転 */
void OK_GetLocalWindDirection( FVECTOR *pos, SVECTOR *output )
{
	int		i;
	Work	*work;
	Unit	*unit;
	FVECTOR	fvtemp;

	if( OK_LOCAL_WIND2_WORK!=NULL ){
		if( OK_GetLocalWind2( pos, &fvtemp ) ){
			OK_DirVecXY( &DG_ZeroVector, &fvtemp, output );
			return;
		}
	}


	if( OK_LOCAL_WIND_WORK!=NULL ){
		work = OK_LOCAL_WIND_WORK;
		unit = work->unit;
		for(i=0; i<work->unit_num; i++){
			if( vu0_CheckBoundingBox( pos, &unit->boundary0, &unit->boundary1 ) ){
				output->vx = unit->local_rot.vx;
				output->vy = unit->local_rot.vy;
				output->vz = unit->local_rot.vz;
			}
			unit++;
		}
	}
	output->vx = G_wind_rot.vx;
	output->vy = G_wind_rot.vy;
	output->vz = G_wind_rot.vz;
}
//----------------------------------------------------------

static	void	GetOptionValue( Work *work )
{
	int	i;
	Unit	*unit;
	float	ftemp;

	if ( GCL_GetOption( 'd' ) != NULL ) {
		unit=work->unit;
		for( i=0; i<work->unit_num; i++ ){
			unit->boundary0.vx = (float)GCL_GetNextInt() ;
			unit->boundary0.vy = (float)GCL_GetNextInt() ;
			unit->boundary0.vz = (float)GCL_GetNextInt() ;
			unit->boundary1.vx = (float)GCL_GetNextInt() ;
			unit->boundary1.vy = (float)GCL_GetNextInt() ;
			unit->boundary1.vz = (float)GCL_GetNextInt() ;
			unit->rot_y        = (float)GCL_GetNextInt() ;
			unit->rot_rand     = (float)GCL_GetNextInt() ;
			unit->intense      = (float)GCL_GetNextInt() ;
			unit->temp_intense = unit->intense;

			if( unit->boundary0.vx > unit->boundary1.vx ){
				ftemp = unit->boundary0.vx;
				unit->boundary0.vx = unit->boundary1.vx;
				unit->boundary1.vx = ftemp;
			}
			if( unit->boundary0.vy > unit->boundary1.vy ){
				ftemp = unit->boundary0.vy;
				unit->boundary0.vy = unit->boundary1.vy;
				unit->boundary1.vy = ftemp;
			}
			if( unit->boundary0.vz > unit->boundary1.vz ){
				ftemp = unit->boundary0.vz;
				unit->boundary0.vz = unit->boundary1.vz;
				unit->boundary1.vz = ftemp;
			}
			unit++;
		}
	}else{
		ASSERT(0)
	}
}

static int GetResources( Work *work )
{
	work->go_flag = 1;
	GetOptionValue( work ) ;
	return (0);
}

void *NewLocalWind( int name, int where )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;

	OPERATOR() ;


	unit_num=0;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt() ;
	}

	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_PREV2, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;
		work->where = where;

		OK_LOCAL_WIND_WORK = work;
		work->unit_num=unit_num;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* 
   2000/08/19 T.Morita Added
 */
void *NewLocalWind_Demo( FVECTOR *center, FVECTOR *size, float rot_y, float rot_rand, float intense, int name )
{
	Work		*work ;
	float	ftemp;
	FVECTOR h_size ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_PREV2, sizeof( Work ) + sizeof( Unit ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name ;
		work->where = GM_CurrentMap ;

		work->go_flag  = 1 ;
		work->unit_num = 1 ;

		OK_LOCAL_WIND_WORK = work ;

		_sceVu0ScaleVector( &h_size, size, 0.5f ) ;
		_sceVu0AddVector( &work->unit->boundary0, center, &h_size ) ;
		_sceVu0SubVector( &work->unit->boundary1, center, &h_size ) ;
		work->unit->rot_y        = rot_y ;
		work->unit->rot_rand     = rot_rand ;
		work->unit->intense      = intense ;
		work->unit->temp_intense = work->unit->intense ;

		if( work->unit->boundary0.vx > work->unit->boundary1.vx ){
		    ftemp = work->unit->boundary0.vx;
		    work->unit->boundary0.vx = work->unit->boundary1.vx;
		    work->unit->boundary1.vx = ftemp;
		}
		if( work->unit->boundary0.vy > work->unit->boundary1.vy ){
		    ftemp = work->unit->boundary0.vy;
		    work->unit->boundary0.vy = work->unit->boundary1.vy;
		    work->unit->boundary1.vy = ftemp;
		}
		if( work->unit->boundary0.vz > work->unit->boundary1.vz ){
		    ftemp = work->unit->boundary0.vz;
		    work->unit->boundary0.vz = work->unit->boundary1.vz;
		    work->unit->boundary1.vz = ftemp;
		}
	}
	return (void *)work ;
}

