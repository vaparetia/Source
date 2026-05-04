/*********************************************

	エフェクト　レイ_滲み血メッセージ　のランチャー関数

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
	int     name ;
	int     joint_num ;
	float   x ;
	float   y ;
	float   z ;
	float   range ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	FVECTOR d2 ;
	float   d3 ;
} Data ;

void *PDRAY_OozeBloodAddDemo_d008Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *PDRAY_OozeBloodAddDemo( int ,int ,float ,float ,float ,float  ) ;

	FVECTOR * pos ;

	work.name = data->d0 ;
	work.joint_num = data->d1 ;
	pos = &data->d2 ;
	work.x = pos->vx ;
	work.y = pos->vy ;
	work.z = pos->vz ;
	work.range = data->d3 ;

	return PDRAY_OozeBloodAddDemo( work.name,
	                               work.joint_num,
	                               work.x,
	                               work.y,
	                               work.z,
	                               work.range
) ;
}


/*********************************************

	エフェクト　レイ_滲み血メッセージ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
