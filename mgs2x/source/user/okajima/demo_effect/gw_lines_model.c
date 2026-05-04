//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gw_lines_model.c
	ジョージワシントン橋のワイヤー保護ライン
	2000/08/10 S.Okajima
	$Id: gw_lines_model.c,v 1.1.1.3 2002/11/19 11:46:56 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define		WIRE_MODEL	12079660 /*"d00a_wire_cm"*/ 

#define	LINE	(4)
#define	MAX_WIRE	(37)
#define	WIRE_SEGMENTS	(90)
//#define	WIRE_SEGMENTS	(32)

#define CENTER_SHIFT (10000.0f)
#define F_NEAR (50000.0f)
#define F_FAR  (150000.0f)
#define MAX_ALPHA (128)

extern int  DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

/* ---------------------------------------------------------------- */
typedef	struct	{
	int			queue_flag;
	DG_COMDL	*comdl ;
} Unit ;

typedef	struct	{
	FVECTOR		pos[MAX_WIRE];
	Unit		unit[MAX_WIRE];
} WIRE_SET ;

typedef	struct	{
	GV_ACT_EX		actor ;

	WIRE_SET	wire_set[LINE];
} Work ;


static void SetCoModels( Unit *unit )
{
	if( unit->queue_flag == 0 ){
		unit->queue_flag = 1;
		DG_QueueComdlObjs( unit->comdl );
	}
}

static void DeleteCoModels( Unit *unit )
{
	if( unit->queue_flag == 1 ){
		unit->queue_flag = 0;
		DG_DequeueComdlObjs( unit->comdl );
	}
}

static void ComodelScaleMatrix( FMATRIX *out_mat, FMATRIX *in_mat, FVECTOR *scale)
{
	DG_COPY_VEC((FVECTOR*)&out_mat->m[3], (FVECTOR*)&in_mat->m[3]) ;

	out_mat->m[0][0] = in_mat->m[0][0] * scale->vx;
	out_mat->m[1][0] = in_mat->m[1][0] * scale->vx;
	out_mat->m[2][0] = in_mat->m[2][0] * scale->vx;	

	out_mat->m[0][1] = in_mat->m[0][1] * scale->vy;
	out_mat->m[1][1] = in_mat->m[1][1] * scale->vy;
	out_mat->m[2][1] = in_mat->m[2][1] * scale->vy;

	out_mat->m[0][2] = in_mat->m[0][2] * scale->vz;
	out_mat->m[1][2] = in_mat->m[1][2] * scale->vz;
	out_mat->m[2][2] = in_mat->m[2][2] * scale->vz;
}

/* ---------------------------------------------------------------- */
static int CheckNear( FVECTOR *pos0, FVECTOR *pos1, float distance )
{
	if( pos0->vx > pos1->vx ){
		if( pos0->vx - pos1->vx > distance) return 0;
	}else{
		if( pos1->vx - pos0->vx > distance) return 0;
	}
/*
	if( pos0->vy > pos1->vy ){
		if( pos0->vy - pos1->vy > distance) return 0;
	}else{
		if( pos1->vy - pos0->vy > distance) return 0;
	}
*/
	if( pos0->vz > pos1->vz ){
		if( pos0->vz - pos1->vz > distance) return 0;
	}else{
		if( pos1->vz - pos0->vz > distance) return 0;
	}

//	printf("a0:%f %f %f\n",pos0->vx,pos0->vy,pos0->vz);
//	printf("a1:%f %f %f\n",pos1->vx,pos1->vy,pos1->vz);

	return 1;

}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	Unit	*unit;
	int		i,j,k;
	float	checker;
	FVECTOR	*pos;
	FVECTOR	*origin_pos;
	FVECTOR	cam_pos;
	FVECTOR	local_pos[MAX_WIRE];
	DG_COMDL_POS	*co_pos;
	int		alpha;
	WIRE_SET	*wire_set;

	wire_set = work->wire_set;

	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	DG_SetPos( &DG_Chanls->eye_pers );
	for( k=0; k<LINE; k++ ){
		/* 座標を透視変換する */
		DG_PutVector( wire_set->pos, local_pos, MAX_WIRE );

		origin_pos = wire_set->pos;
		pos = local_pos;
		unit = wire_set->unit;
		for( i=0; i<MAX_WIRE; i++ ){


			pos->vw = ( pos->vw > 0.0f )? pos->vw: -pos->vw;


			checker = pos->vx/pos->vw;
			checker = ( checker > 0.0f )? checker: -checker;
			if( pos->vz < pos->vw
			 && pos->vw < F_FAR
			 && checker < 8.0f	//画面外２倍までみる
			 ){	// 画面方向にあり、距離内。加えて画面クリップ
				SetCoModels( unit );
				if( pos->vw < F_NEAR ){
					alpha = MAX_ALPHA;
				}else{
					alpha = (int)( (float)MAX_ALPHA - (float)MAX_ALPHA * 
					( pos->vw - F_NEAR ) / ( F_FAR - F_NEAR ) );
				}
				co_pos = unit->comdl->pos;
				for( j=0; j<WIRE_SEGMENTS; j++ ){
					/* 整数型なので注意！ */
					co_pos->color.vw = alpha ;
					co_pos++;
				}
//			}else if( pos->vz > pos->vw  &&  pos->vw > -F_FAR ){	// 画面手前
//			}else if( (pos->vz > pos->vw) && CheckNear( origin_pos, &cam_pos, F_NEAR ) ){	// 画面手前
			}else if( CheckNear( origin_pos, &cam_pos, F_NEAR ) ){	// 画面手前
//			}else if(0){	// 画面手前
				SetCoModels( unit );
				co_pos = unit->comdl->pos;
				for( j=0; j<WIRE_SEGMENTS; j++ ){
					/* 整数型なので注意！ */
					co_pos->color.vw = MAX_ALPHA ;
					co_pos++;
				}
			}else{
				DeleteCoModels( unit );
#if 0
				co_pos = unit->comdl->pos;
				for( j=0; j<WIRE_SEGMENTS; j++ ){
					/* 整数型なので注意！ */
					co_pos->color.vw = 0 ;
					co_pos++;
				}
#endif
			}
			origin_pos++;
			pos++;
			unit++;
		}
		wire_set++;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	Unit	*unit;
	int		i,k;
	WIRE_SET	*wire_set;

	wire_set = work->wire_set;
	for( k=0; k<LINE; k++ ){
		unit = wire_set->unit;
		for( i=0; i<MAX_WIRE; i++ ){
			DeleteCoModels( unit );
			DG_FreeComdl( unit->comdl );
			unit++;
		}
		wire_set++;
	}
}

static FVECTOR terminal0  = {   10000.0f, 170000.0f, 14250.0f, 0.0f };
static float floor_height = 66000.0f;
static float pos_z[LINE] = { 14250.0f, 17750.0f, 46250.0f, 49750.0f };

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	DG_DEF		*def;
	Unit		*unit;
	float		ftemp;
	FVECTOR		fvtemp1;
	FVECTOR		*pos;
	SVECTOR		svtemp;
	FMATRIX		mat;
	float		len;
	int			i,j,k;
	WIRE_SET	*wire_set;


	def = (DG_DEF*)GV_GetCache( GV_CacheID( WIRE_MODEL, 'k' ) ) ;

	svtemp.vx = 0;
	svtemp.vy = 0;
	svtemp.vz = 0;

	wire_set = work->wire_set;
	for( k=0; k<LINE; k++ ){
		pos = wire_set->pos;
		unit = wire_set->unit;
		for( i=0; i<MAX_WIRE; i++ ){
			ftemp = (float)i*(-20000.0f);
			fvtemp1.vx = terminal0.vx + ftemp;
			fvtemp1.vy = floor_height;
			fvtemp1.vz = pos_z[k];

			DG_SetPos2( &fvtemp1, &svtemp );
			DG_GetPos( &mat );
			DG_COPY_VEC( pos, (FVECTOR *)mat.m[3] );

			ftemp = (float)(i-MAX_WIRE/2);
			ftemp *= ftemp;

#if 0
			fvtemp1.vx = 15.0f;
			fvtemp1.vy = (ftemp * 2.197f + 280.0f) / (float)WIRE_SEGMENTS;
			fvtemp1.vz = 15.0f;
#else
			fvtemp1.vx = 20.0f;
			fvtemp1.vy = (ftemp * 2.197f + 280.0f) / (float)WIRE_SEGMENTS;
			fvtemp1.vz = 20.0f;
#endif

			ComodelScaleMatrix( &mat, &mat, &fvtemp1 );
			len = fvtemp1.vy*100.0f;	//長さ

			pos->vy += len*0.5f;

			unit->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS|DG_COMDL_NOFOG, WIRE_SEGMENTS, 0 );
			if( unit->comdl==NULL ) return -1;

			for( j=0; j<WIRE_SEGMENTS; j++ ){
				/* 整数型なので注意！ */
				unit->comdl->pos[j].color.vx = 128 ;
				unit->comdl->pos[j].color.vy = 128 ;
				unit->comdl->pos[j].color.vz = 128 ;
				unit->comdl->pos[j].color.vw = 128 ;
				DG_COPY_MAT( &unit->comdl->pos[j].world, &mat );
				unit->comdl->pos[j].world.m[3][1] += len*(float)j;
//if(j==WIRE_SEGMENTS-1)printf("%6d %6d %6d\n",(int)mat.m[3][0],(int)(unit->comdl->pos[j].world.m[3][1] + len),(int)mat.m[3][2]);
			}

			unit->comdl->flag &= ~DG_COMDL_INVISIBLE;
			unit->queue_flag = 0;
//			DG_QueueComdlObjs( unit->comdl );

			unit++;
			pos++;
		}
		wire_set++;
	}


	return (0);
}

/* ---------------------------------------------------------------- */
void *NewGWB_LinesModel( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

