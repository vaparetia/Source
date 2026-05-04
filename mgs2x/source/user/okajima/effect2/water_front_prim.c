//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	water_front_prim.c
	濁流先頭

	2001/01/23 S.Okajima
	$Id: water_front_prim.c,v 1.1.1.3 2002/11/19 11:47:27 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplush2( FVECTOR *center, SVECTOR *rot, float length );
extern int	OK_SplushManFlag;
/*----------------------------------------------------------------*/
#define	SP_MIN			( 30.0f)
#define	SP_MAX			(200.0f)

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x2000 / 16 / N_VERTS)
//#define N_LOOPS		(8)
#define N_LOOPS		(4)
#define	TOTAL_PARTS		(N_LOOPS*N_PRIMS)
#define	TOTAL_VERTS		(N_LOOPS*N_PRIMS*N_VERTS)

//#define N_MUL		(4)
#define N_MUL		(2)
#define N_MUL_VERTS	(N_VERTS*N_MUL)
#define N_MUL_PRIMS	(N_PRIMS/N_MUL)
#define N_MUL_PARTS	(N_LOOPS*N_MUL_PRIMS)



#define SIZE_MIN	(10.0f)

//#define SPEED	(50.0f)
//#define SPEED	(55.0f)
#define SPEED	(100.0f)

#define	NODE_NUM	(32)

#define	ANGLE_ADD_RND	(TPI/32.0f)
#define	ANGLE_ADD_MIN	(TPI/16.0f)

#define	ANGLE_RADIUS	(100.0f)

#define	DECAY_AT_WALL		(0.5f)
#define	UPPER_FORCE_RATIO	(1.0f)

#define	EFFECT_COUNT_MAX	(20)

typedef	struct	{
	float	ner_edge_x;
	float	ner_edge_z;
	float	far_edge_x;
	float	far_edge_z;
	float	force_x;
	float	force_z;
} Data;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			init_flag[N_MUL_PARTS] ;

	void	*func_check;

	CVECTOR		col;
	int		pattern;
	int		activate_num;

	FVECTOR center_pos;
	FVECTOR direction;
	FVECTOR next_direction;
	FVECTOR side_width;
	FVECTOR side_radius0;
	FVECTOR side_radius1;
	FVECTOR side0_pos;
	FVECTOR side1_pos;
	FVECTOR	step;
	FMATRIX side_mat;
	int		max_node_num;
	int		now_node;
	int		next_node;
	int		next_next_node;
	float	height;
	float	bottom;
	float	side_limit;

	Data	data;

	float	next_side_margine;
	float	size;
	FVECTOR node[NODE_NUM];

	FVECTOR vec[TOTAL_VERTS];

	int		dir_flag;

	int		splush_total_count;

	int		effect_count0;

	float	node_distance;
	float	node_multiple;
	float	max_height;

	SVECTOR	rot0;
	SVECTOR	rot1;
	SVECTOR	rot2;
	SVECTOR	rot3;
	SVECTOR	rot0_add;
	SVECTOR	rot1_add;
	SVECTOR	rot2_add;
	SVECTOR	rot3_add;

	FVECTOR	drop_center;

	DG_PRIM2	*prim ;
} Work ;

static void CheckMesgParam( Work *work )
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
		  case -1:
			OK_SplushManFlag = 0;
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static void Splush_X( FVECTOR *pos, FVECTOR *vec )
{
	SVECTOR svtemp;
	float	ftemp;

	ftemp = DG_FABS( vec->vy )*4.0f;
	if( ftemp < SP_MIN ){
		return;
	}else if( ftemp > SP_MAX ){
		ftemp = SP_MAX;
	}

	svtemp.vx =-( 512 + ((irnd()>>8)&511) );
	svtemp.vz = 0;
	if( vec->vx > 0.0f ){
		svtemp.vy = 1024;
	}else{
		svtemp.vy =-1024;
	}
	OK_PutSplush2( pos, &svtemp, ftemp );
}

/*----------------------------------------------------------------*/
static void Splush_Z( FVECTOR *pos, FVECTOR *vec )
{
	SVECTOR svtemp;
	float	ftemp;

	ftemp = DG_FABS( vec->vy )*4.0f;
	if( ftemp < SP_MIN ){
		return;
	}else if( ftemp > SP_MAX ){
		ftemp = SP_MAX;
	}

	svtemp.vx =-( 512 + ((irnd()>>8)&511) );
	svtemp.vz = 0;
	if( vec->vz > 0.0f ){
		svtemp.vy = 0;
	}else{
		svtemp.vy = 2048;
	}
	OK_PutSplush2( pos, &svtemp, ftemp );
}

/*----------------------------------------------------------------*/
static inline void DataSet_X( FVECTOR *vec, float direction )
{
	vec->vx = -vec->vx * DECAY_AT_WALL * rnd();
	vec->vy =  DG_FABS( vec->vy ) * rnd();
	vec->vz+=  direction * rnd();
}

static inline void DataSet_Z( FVECTOR *vec, float direction )
{
	vec->vz = -vec->vz * DECAY_AT_WALL * rnd();
	vec->vy =  DG_FABS( vec->vy ) * rnd();
	vec->vx+=  direction * rnd();
}

/*----------------------------------------------------------------*/
static void PosUpdate_ONE( FVECTOR *sc_pos, FVECTOR *sc_vec, Data *data )
{
	float ner_diff_x;
	float ner_diff_z;

	ner_diff_x = sc_pos->vx - data->ner_edge_x;
	ner_diff_z = sc_pos->vz - data->ner_edge_z;
	if( (ner_diff_x > 0.0f)
	 && (ner_diff_z < 0.0f) ){
		if( DG_FABS( ner_diff_x ) < DG_FABS( ner_diff_z ) ){
			sc_pos->vx = data->ner_edge_x;
			DataSet_X( sc_vec, data->force_x );
		}else{
			sc_pos->vz = data->ner_edge_z;
			DataSet_Z( sc_vec, data->force_z );
		}
	}else if( sc_pos->vx < data->far_edge_x ){
		sc_pos->vx = data->far_edge_x;
		DataSet_X( sc_vec, data->force_x );
		Splush_X( sc_pos, sc_vec );
	}else if( sc_pos->vz > data->far_edge_z ){
		sc_pos->vz = data->far_edge_z;
		DataSet_Z( sc_vec, data->force_z );
		Splush_Z( sc_pos, sc_vec );
	}
}
static void PosUpdate_TWO( FVECTOR *sc_pos, FVECTOR *sc_vec, Data *data )
{
	float ner_diff_x;
	float ner_diff_z;

	ner_diff_x = sc_pos->vx - data->ner_edge_x;
	ner_diff_z = sc_pos->vz - data->ner_edge_z;
	if( (ner_diff_x < 0.0f)
	 && (ner_diff_z < 0.0f) ){
		if( DG_FABS( ner_diff_x ) < DG_FABS( ner_diff_z ) ){
			sc_pos->vx = data->ner_edge_x;
			DataSet_X( sc_vec, data->force_x );
		}else{
			sc_pos->vz = data->ner_edge_z;
			DataSet_Z( sc_vec, data->force_z );
		}
	}else if( sc_pos->vx > data->far_edge_x ){
		sc_pos->vx = data->far_edge_x;
		DataSet_X( sc_vec, data->force_x );
		Splush_X( sc_pos, sc_vec );
	}else if( sc_pos->vz > data->far_edge_z ){
		sc_pos->vz = data->far_edge_z;
		DataSet_Z( sc_vec, data->force_z );
		Splush_Z( sc_pos, sc_vec );
	}
}
static void PosUpdate_THREE( FVECTOR *sc_pos, FVECTOR *sc_vec, Data *data )
{
	float ner_diff_x;
	float ner_diff_z;

	ner_diff_x = sc_pos->vx - data->ner_edge_x;
	ner_diff_z = sc_pos->vz - data->ner_edge_z;
	if( (ner_diff_x < 0.0f)
	 && (ner_diff_z > 0.0f) ){
		if( DG_FABS( ner_diff_x ) < DG_FABS( ner_diff_z ) ){
			sc_pos->vx = data->ner_edge_x;
			DataSet_X( sc_vec, data->force_x );
		}else{
			sc_pos->vz = data->ner_edge_z;
			DataSet_Z( sc_vec, data->force_z );
		}
	}else if( sc_pos->vx > data->far_edge_x ){
		sc_pos->vx = data->far_edge_x;
		DataSet_X( sc_vec, data->force_x );
		Splush_X( sc_pos, sc_vec );
	}else if( sc_pos->vz < data->far_edge_z ){
		sc_pos->vz = data->far_edge_z;
		DataSet_Z( sc_vec, data->force_z );
		Splush_Z( sc_pos, sc_vec );
	}
}
static void PosUpdate_FOUR( FVECTOR *sc_pos, FVECTOR *sc_vec, Data *data )
{
	float ner_diff_x;
	float ner_diff_z;

	ner_diff_x = sc_pos->vx - data->ner_edge_x;
	ner_diff_z = sc_pos->vz - data->ner_edge_z;
	if( (ner_diff_x > 0.0f)
	 && (ner_diff_z > 0.0f) ){
		if( DG_FABS( ner_diff_x ) < DG_FABS( ner_diff_z ) ){
			sc_pos->vx = data->ner_edge_x;
			DataSet_X( sc_vec, data->force_x );
		}else{
			sc_pos->vz = data->ner_edge_z;
			DataSet_Z( sc_vec, data->force_z );
		}
	}else if( sc_pos->vx < data->far_edge_x ){
		sc_pos->vx = data->far_edge_x;
		DataSet_X( sc_vec, data->force_x );
		Splush_X( sc_pos, sc_vec );
	}else if( sc_pos->vz < data->far_edge_z ){
		sc_pos->vz = data->far_edge_z;
		DataSet_Z( sc_vec, data->force_z );
		Splush_Z( sc_pos, sc_vec );
	}
}


/*----------------------------------------------------------------*/
static void PartInitial( Work *work, int now, int next )
{
	SVECTOR	rot;
	FVECTOR	next_node;
	float	now_X_min;
	float	now_X_max;
	float	now_Z_min;
	float	now_Z_max;
	float	nxt_X_min;
	float	nxt_X_max;
	float	nxt_Z_min;
	float	nxt_Z_max;
	float	now_width;
	float	next_width;
	int	i;

	DG_COPY_VEC( &work->center_pos, &work->node[now] );
	DG_COPY_VEC( &next_node,        &work->node[next] );

	_sceVu0SubVector( &work->direction,      &next_node, &work->center_pos ) ;
	work->node_distance = GV_VecLen3F( &work->direction );
	work->node_multiple = 0.0f;

	i = work->next_next_node;
	_sceVu0SubVector( &work->next_direction,   &work->node[i], &next_node ) ;
	_sceVu0Normalize( &work->next_direction,   &work->next_direction );

	//ＸＺ平面上での直交ベクトル
	_sceVu0Normalize( &work->direction, &work->direction );
	work->side_width.vx =-work->direction.vz;
	work->side_width.vy = 0;
	work->side_width.vz = work->direction.vx;
	_sceVu0ScaleVector( &work->side_width, &work->side_width, work->center_pos.vw );
	_sceVu0ScaleVector( &work->step, &work->direction, SPEED );

	OK_DirVecXY( &DG_ZeroVector, &work->direction, &rot );
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_GetPos( &work->side_mat );

	work->next_side_margine = next_node.vw;

	now_width  = work->center_pos.vw;
	next_width = next_node.vw;

	now_X_min = work->center_pos.vx - now_width;
	now_X_max = work->center_pos.vx + now_width;
	now_Z_min = work->center_pos.vz - now_width;
	now_Z_max = work->center_pos.vz + now_width;
	nxt_X_min = next_node.vx  - next_width;
	nxt_X_max = next_node.vx  + next_width;
	nxt_Z_min = next_node.vz  - next_width;
	nxt_Z_max = next_node.vz  + next_width;

	if( DG_FABS( work->direction.vx ) > DG_FABS( work->direction.vz ) ){
		if( work->direction.vx > 0.0f ){
			if( work->next_direction.vz > 0.0f ){
				work->func_check = PosUpdate_THREE;
				work->data.ner_edge_x = nxt_X_min;
				work->data.far_edge_x = nxt_X_max;
				work->data.ner_edge_z = now_Z_max;
				work->data.far_edge_z = now_Z_min;

			}else{
				work->func_check = PosUpdate_TWO;
				work->data.ner_edge_x = nxt_X_min;
				work->data.far_edge_x = nxt_X_max;
				work->data.ner_edge_z = now_Z_min;
				work->data.far_edge_z = now_Z_max;
			}
		}else{
			if( work->next_direction.vz > 0.0f ){
				work->func_check = PosUpdate_FOUR;
				work->data.ner_edge_x = nxt_X_max;
				work->data.far_edge_x = nxt_X_min;
				work->data.ner_edge_z = now_Z_max;
				work->data.far_edge_z = now_Z_min;
			}else{
				work->func_check = PosUpdate_ONE;
				work->data.ner_edge_x = nxt_X_max;
				work->data.far_edge_x = nxt_X_min;
				work->data.ner_edge_z = now_Z_min;
				work->data.far_edge_z = now_Z_max;
			}
		}
	}else{
		if( work->direction.vz > 0.0f ){
			if( work->next_direction.vx > 0.0f ){
				work->func_check = PosUpdate_ONE;
				work->data.ner_edge_x = now_X_max;
				work->data.far_edge_x = now_X_min;
				work->data.ner_edge_z = nxt_Z_min;
				work->data.far_edge_z = nxt_Z_max;
			}else{
				work->func_check = PosUpdate_TWO;
				work->data.ner_edge_x = now_X_min;
				work->data.far_edge_x = now_X_max;
				work->data.ner_edge_z = nxt_Z_min;
				work->data.far_edge_z = nxt_Z_max;
			}
		}else{
			if( work->next_direction.vx > 0.0f ){
				work->func_check = PosUpdate_FOUR;
				work->data.ner_edge_x = now_X_max;
				work->data.far_edge_x = now_X_min;
				work->data.ner_edge_z = nxt_Z_max;
				work->data.far_edge_z = nxt_Z_min;
			}else{
				work->func_check = PosUpdate_THREE;
				work->data.ner_edge_x = now_X_min;
				work->data.far_edge_x = now_X_max;
				work->data.ner_edge_z = nxt_Z_max;
				work->data.far_edge_z = nxt_Z_min;
			}
		}
	}

	_sceVu0ScaleVector( &work->next_direction, &work->next_direction, SPEED*2.0f );
	work->effect_count0 = 0;
}

/*----------------------------------------------------------------*/
static	int CalcNodeData( Work *work )
{
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;

	if( (work->node_multiple-4.0f)*SPEED  >  work->node_distance + work->next_side_margine){
//	if( work->node_multiple*SPEED  >  work->node_distance + work->next_side_margine){
		work->now_node = work->next_node;
		work->next_node++;
		if( work->next_node >= work->max_node_num ){
			work->now_node  = 0;
			work->next_node = 1;
		}
		work->next_next_node = work->next_node + 1;
		PartInitial( work, work->now_node, work->next_node );
	}

	work->rot0.vx+= work->rot0_add.vx;
//	work->rot0.vy+= work->rot0_add.vy;
//	work->rot0.vz+= work->rot0_add.vz;
	work->rot1.vx+= work->rot1_add.vx;
//	work->rot1.vy+= work->rot1_add.vy;
//	work->rot1.vz+= work->rot1_add.vz;
	work->rot2.vx+= work->rot2_add.vx;
//	work->rot2.vy+= work->rot2_add.vy;
//	work->rot2.vz+= work->rot2_add.vz;
	work->rot3.vx+= work->rot3_add.vx;
//	work->rot3.vy+= work->rot3_add.vy;
//	work->rot3.vz+= work->rot3_add.vz;

	DG_SetPos( &work->side_mat );
	DG_RotatePos( &work->rot0 );
	DG_RotVector( &work->side_radius0, &work->side0_pos, 1 );
	DG_SetPos( &work->side_mat );
	DG_RotatePos( &work->rot1 );
	DG_RotVector( &work->side_radius1, &fvtemp0, 1 );
	_sceVu0AddVector( &fvtemp1, &work->center_pos, &work->side_width );
	_sceVu0AddVector( &fvtemp1, &fvtemp1, &fvtemp0 );
	_sceVu0AddVector( &work->side0_pos, &work->side0_pos, &fvtemp1 );

	DG_SetPos( &work->side_mat );
	DG_RotatePos( &work->rot2 );
	DG_RotVector( &work->side_radius0, &work->side1_pos, 1 );
	DG_SetPos( &work->side_mat );
	DG_RotatePos( &work->rot3 );
	DG_RotVector( &work->side_radius1, &fvtemp0, 1 );
	_sceVu0SubVector( &fvtemp1, &work->center_pos, &work->side_width );
	_sceVu0AddVector( &fvtemp1, &fvtemp1, &fvtemp0 );
	_sceVu0AddVector( &work->side1_pos, &work->side1_pos, &fvtemp1 );

	work->max_height = work->bottom + (float)((int)(work->height)*work->effect_count0/EFFECT_COUNT_MAX);
//	work->max_height = work->bottom + work->height;

	work->side0_pos.vy+= work->height;
	work->side0_pos.vy = ( work->side0_pos.vy < work->max_height )? work->side0_pos.vy: work->max_height;
	work->side1_pos.vy+= work->height;
	work->side1_pos.vy = ( work->side1_pos.vy < work->max_height )? work->side1_pos.vy: work->max_height;
	work->effect_count0++;

	_sceVu0AddVector( &work->center_pos, &work->center_pos, &work->step ) ;
	work->node_multiple += 1.0f;

	return 1;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	void ( *func_check )( FVECTOR *sc_pos, FVECTOR *sc_vec, Data *data );
	Data	*data;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR	*sc_pos;
	FVECTOR	*vec;
	FVECTOR	*sc_vec;
	FVECTOR	point;
	FVECTOR diff;
	int	clock;
	int	i,j,k;
	int		dir_flag;
	int		max_alpha;
	float	side_limit;
	float	bottom;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGBWH	*uvrgb;
	int		num;
	int		alpha;
	int		col_count;
	int	*pitemp;


	CheckMesgParam( work );


	if( !CalcNodeData( work ) ){
		GV_DestroyActor( work ) ;
		return;
	}


//AN_Test_Eye2( &work->center_pos, 2 );
//AN_Test_Eye2( &work->side0_pos, 2 );
//AN_Test_Eye2( &work->side1_pos, 2 );

	work->splush_total_count = 0;


	dir_flag = work->dir_flag;
	side_limit = work->side_limit;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	func_check = work->func_check;
	data = &work->data;

	data->force_x = work->next_direction.vz;
	data->force_z = work->next_direction.vx;

	bottom    = work->bottom;
	max_alpha = work->col.cd * 2 / 3;
	col_count = N_MUL_PARTS - work->activate_num  + 1;
	uvrgb        = prim->uvrgb[  clock];
	pos          = prim->pos[  clock];
	pos_before   = prim->pos[1-clock];
	vec = work->vec;
	pitemp = work->init_flag;
	for ( j=0 ; j < N_LOOPS ; j++ ){
		if( (*pitemp)!=0 ){
			OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			OK_Mem_Scr( SCR_TMP, vec,        sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;

			sc_pos = SCR_POS;
			sc_vec = SCR_TMP;
			for ( i = 0 ; i < N_MUL_PRIMS ; i++ ){
				if( col_count > N_MUL_PARTS ) col_count = 0;
				alpha = max_alpha*col_count/N_MUL_PARTS;
				col_count++;
				for ( k = 0 ; k < N_MUL_VERTS ; k++ ){

					( *func_check )( sc_pos, sc_vec, data );

					sc_pos->vx += sc_vec->vx;
					sc_pos->vy += sc_vec->vy;
					sc_pos->vz += sc_vec->vz;

					if( sc_pos->vy < bottom ){
						sc_pos->vy = bottom + work->size;
						sc_vec->vy =-sc_vec->vy*(0.75f);
					}
					sc_vec->vy += P_GRAVITY;
					uvrgb->a = alpha;

					sc_vec++;
					sc_pos++;
					uvrgb++;
				}
			}
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			OK_Scr_Mem( vec, SCR_TMP, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		}else{
			uvrgb  += N_MUL_VERTS*N_MUL_PRIMS;
		}
		pos        += N_VERTS*N_PRIMS;
		pos_before += N_VERTS*N_PRIMS;
		vec        += N_VERTS*N_PRIMS;
	}

	_sceVu0SubVector( &diff, &work->side1_pos, &work->side0_pos ) ;
	_sceVu0ScaleVector( &diff, &diff, 1.0f/N_MUL_VERTS );

	DG_COPY_VEC( &point, &work->side0_pos );

	num    = work->activate_num * N_MUL_VERTS;

	vec    = work->vec;
	pos    = prim->pos[clock];
//	uvrgb  = prim->uvrgb[clock];
	vec   += num;
	pos   += num;
	uvrgb += num;

	pitemp = work->init_flag;
	pitemp+= work->activate_num;
	(*pitemp)=1;

	work->activate_num++;
	work->activate_num = (work->activate_num < N_MUL_PARTS)? work->activate_num: 0;

	sc_pos = SCR_POS;
	sc_vec = SCR_TMP;
	for ( j=0 ; j < N_MUL_VERTS ; j++ ){
#if 0
		angle = TPI*rnd();
//		size = SIZE_MIN + work->size*rnd();
		size = work->size*(0.8f + rnd()*0.2f);
		uvrgb->w  = (int)(cosf( angle ) * size);
		uvrgb->h  = (int)(sinf( angle ) * size);

//		uvrgb->w  = 0;
//		uvrgb->h  = 0;
#endif

		DG_COPY_VEC( sc_pos, &point );
		_sceVu0AddVector( &point, &point, &diff ) ;
//		_sceVu0ScaleVector( sc_vec, &work->direction, rnd()*SPEED*4.0f + SPEED*0.1f );
		_sceVu0ScaleVector( sc_vec, &work->direction, rnd()*SPEED*3.0f + SPEED );
		sc_vec->vx+=frnd()*SPEED*0.25f;
		sc_vec->vy+= rnd()*SPEED;
		sc_vec->vz+=frnd()*SPEED*0.25f;
		sc_pos++;
		sc_vec++;
//		uvrgb++;
	}
	OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_MUL_VERTS ) ;
	OK_Scr_Mem( vec, SCR_TMP, sizeof(FVECTOR), N_MUL_VERTS ) ;








	_sceVu0SubVector( &diff, &work->side1_pos, &work->side0_pos ) ;
	_sceVu0ScaleVector( &diff, &diff, 0.5f );
	_sceVu0AddVector( &work->drop_center, &work->side0_pos, &diff ) ;


//	AN_Test_Eye2( &work->drop_center, 2 );


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR	*base;
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	float	angle;
	float	size;

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );

	switch( work->pattern ){
	  case -1:
	  case 0:
	  case 1:
	  default:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	  case 2:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	}


	base = work->node;

	pos0       = prim->pos[0];
	pos1       = prim->pos[1];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS*N_VERTS ; i++ ){
#if 1
		angle = TPI*rnd();
//		size = SIZE_MIN + work->size*rnd();
		size = work->size*(0.8f + rnd()*0.2f);
		uvrgbwh1->w  = uvrgbwh0->w  = (int)(cosf( angle ) * size);
		uvrgbwh1->h  = uvrgbwh0->h  = (int)(sinf( angle ) * size);
#else
		uvrgbwh1->w  = uvrgbwh0->w  = 0;
		uvrgbwh1->h  = uvrgbwh0->h  = 0;
#endif
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
		uvrgbwh1->r  = uvrgbwh0->r  = work->col.r ;
		uvrgbwh1->g  = uvrgbwh0->g  = work->col.g ;
		uvrgbwh1->b  = uvrgbwh0->b  = work->col.b ;
//		uvrgbwh1->a  = uvrgbwh0->a  = work->col.cd ;
		uvrgbwh1->a  = uvrgbwh0->a  = 0 ;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;

		DG_COPY_VEC( pos0, base );
		DG_COPY_VEC( pos1, base );
		pos0++;
		pos1++;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int	i;
	int	itemp;
	int	*pitemp;

	if ( GCL_GetOption( 'n' ) != NULL ){
		for( i = 0; GCL_NextStr() != NULL; i++ ){
			work->node[i].vx = (float)GCL_GetNextInt() ;
			work->node[i].vy = (float)GCL_GetNextInt() ;
			work->node[i].vz = (float)GCL_GetNextInt() ;
			work->node[i].vw = (float)GCL_GetNextInt() ; // 幅
			if(i>=NODE_NUM-1) break;
		}
		work->max_node_num = i;
	}else{
		return -1;
	}

	work->bottom = work->node[0].vy;

	if ( GCL_GetOption( 'h' ) != NULL ){
		work->height = (float)GCL_GetNextInt() ;
	}else{
		return -1;
	}

	work->pattern = 0;
	if ( GCL_GetOption( 'p' ) != NULL ){
		work->pattern = (float)GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 's' ) != NULL ){
		work->size = (float)GCL_GetNextInt() ;
		if( work->pattern==1 ) work->size*= 2.0f;
	}else{
		return -1;
	}

	work->side_radius0.vx = 0.0f;
	work->side_radius0.vy = 0.0f;
	work->side_radius0.vz = work->height * 0.5f;
	work->side_radius1.vx = 0.0f;
	work->side_radius1.vy = 0.0f;
	work->side_radius1.vz = work->height * 0.3f;

	work->now_node       = 0;
	work->next_node      = 1;
	work->next_next_node = 2;
	PartInitial( work, 0, 1 );

	work->rot0.vx = (irnd()>>8)&4095;
	work->rot0.vy = 0;
	work->rot0.vz = 0;
	work->rot0_add.vx = ((irnd()>>8)&31)+32;
	work->rot0_add.vy = 0;
	work->rot0_add.vz = 0;

	work->rot1.vx = (irnd()>>8)&4095;
	work->rot1.vy = 0;
	work->rot1.vz = 0;
	work->rot1_add.vx = work->rot0_add.vx*2 + ((irnd()>>8)&7) + 16;
	work->rot1_add.vy = 0;
	work->rot1_add.vz = 0;

	work->rot2.vx = (irnd()>>8)&4095;
	work->rot2.vy = 0;
	work->rot2.vz = 0;
	work->rot2_add.vx = ((irnd()>>8)&15)+32;
	work->rot2_add.vy = 0;
	work->rot2_add.vz = 0;

	work->rot3.vx = (irnd()>>8)&4095;
	work->rot3.vy = 0;
	work->rot3.vz = 0;
	work->rot3_add.vx = work->rot2_add.vx*2 + ((irnd()>>8)&7) + 16;
	work->rot3_add.vy = 0;
	work->rot3_add.vz = 0;

	work->activate_num = 0;


	switch( work->pattern ){
	  case -1:
		tex = DG_GetTexture( 8617636 /*"col128_alp"*/ );
		work->col.r = 255;
		work->col.g = 255;
		work->col.b = 255;
		work->col.cd= 255;
		break;
	  case 0:
	  default:
		tex = DG_GetTexture( 15638432 /*"blood_1bw_msk"*/ );
		work->col.r = 255;
		work->col.g = 255;
		work->col.b = 255;
		work->col.cd= 96;
		break;
	  case 1:
		tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
		work->col.r = 255;
		work->col.g = 255;
		work->col.b = 255;
		work->col.cd= 255;
		break;
	  case 2:
		tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
		work->col.r = 255;
		work->col.g = 255;
		work->col.b = 255;
		work->col.cd= 128;
		break;
	}

	work->col.r = (u_char)DG_ColorMatrix.m[3][0];
	work->col.g = (u_char)DG_ColorMatrix.m[3][1];
	work->col.b = (u_char)DG_ColorMatrix.m[3][2];
	itemp = (work->col.r > work->col.g)? work->col.r : work->col.g;
	itemp = (itemp       > work->col.b)? itemp       : work->col.b;
	if ( itemp==0 ) itemp = 1 ;
	work->col.r = 127 + 128 * work->col.r / itemp;
	work->col.g = 127 + 128 * work->col.g / itemp;
	work->col.b = 127 + 128 * work->col.b / itemp;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);


	pitemp = work->init_flag;
	for( i=0; i<N_MUL_PARTS; i++ ){
		(*pitemp)=0;
		pitemp++;
	}


	{
		extern void *NewRainCameraProg( FVECTOR *pos );
		GV_SetActorChild( work, NewRainCameraProg( &work->drop_center ) );
	}

	return 0 ;
}

void *NewPassingSplash( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
