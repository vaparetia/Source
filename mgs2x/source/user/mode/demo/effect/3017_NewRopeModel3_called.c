//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　物理シム・壊れた手錠　のランチャー関数

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
	IVECTOR d1 ;
	FVECTOR d4 ;
	SVECTOR d5 ;
} Data ;

#include "BP_EndianSupport.h"

void *NewRopeModel3_called_3017Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRopeModel3_called( int ,int ,void *,int ,FVECTOR *,SVECTOR *,float ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

   BP_LE_SwapSIntArray_Inp(&data->d5, 2);
   BP_LE_SwapSShortArray_Inp(&data->d5, 4);

	work.model_name = data->d0 ;
	work.sample_num = ( int ) 4.000000  ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.objnum = link->vy ;
	work.pos = &data->d4 ;
	work.rot = &data->d5 ;
	work.oval_param = ( float ) 1.000000  ;
	work.collision_flag = ( int ) 0.000000  ;
	work.visible_flag = ( int ) 1.000000  ;
	work.mode = ( int ) 1.000000  ;
	work.bound_model = ( int ) 0.000000  ;

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

	エフェクト　物理シム・壊れた手錠　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
