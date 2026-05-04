/*********************************************

	エフェクト　物理シム・バンダナ風の強い場所用　のランチャー関数

	Created by DEMO Composer.
 *********************************************/


#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include "../libdemo.h"



typedef struct work_t {
	int     model_name ;
	int     sample_num ;
	void    *object ;
	int     objnum ;
	FVECTOR *pos ;
	SVECTOR *rot ;
	float   oval_param ;
	int     collision_flag ;
	int     visible_flag ;
	int     mode ;
	int     bound_model ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	IVECTOR d2 ;
	FVECTOR d5 ;
	SVECTOR d6 ;
	float   d7 ;
	int     d8 ;
} Data ;

void *NewRopeModel3_called_3018Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRopeModel3_called( int ,int ,void *,int ,FVECTOR *,SVECTOR *,float ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.model_name = data->d0 ;
	work.sample_num = (data->d1 + ( int ) 7.000000 ) ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.objnum = link->vy ;
	work.pos = &data->d5 ;
	work.rot = &data->d6 ;
	work.oval_param = data->d7 ;
	work.collision_flag = ( int ) 1.000000  ;
	work.visible_flag = ( int ) 1.000000  ;
	work.mode = ( int ) 0.000000  ;
	work.bound_model = data->d8 ;

	return NewRopeModel3_called( work.model_name,
	                             work.sample_num,
	                             work.object,
	                             work.objnum,
	                             work.pos,
	                             work.rot,
	                             work.oval_param,
	                             work.collision_flag,
	                             work.visible_flag,
	                             work.mode,
	                             work.bound_model
) ;
}


/*********************************************

	エフェクト　物理シム・バンダナ風の強い場所用　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
