//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　物理シム 新髪の毛ＥＶＭ２　のランチャー関数

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
	int     model_name ;
	int     sample_num ;
	void    *object ;
	FVECTOR *pos ;
	SVECTOR *rot ;
	float   oval_param ;
	int     visible_flag ;
	int     light_flag ;
	int     bound_model ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
   int bp_pad[1];
	IVECTOR d3 ;
	FVECTOR d6 ;
	SVECTOR d7 ;
	float   d8 ;
	int     d9 ;
	int     d10 ;
} Data ;

#include "BP_EndianSupport.h"

void *NewEvmHairModel_Demo2_3009Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewEvmHairModel_Demo2( int ,int ,int ,void *,FVECTOR *,SVECTOR *,float ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

   BP_LE_SwapSIntArray_Inp(&data->d7, 2);
   BP_LE_SwapSShortArray_Inp(&data->d7, 4);

	work.name = data->d0 ;
	work.model_name = data->d1 ;
	work.sample_num = data->d2 ;
	link = &data->d3 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.pos = &data->d6 ;
	work.rot = &data->d7 ;
	work.oval_param = data->d8 ;
	work.visible_flag = ( int ) 1.000000  ;
	work.light_flag = data->d9 ;
	work.bound_model = data->d10 ;

	return NewEvmHairModel_Demo2( work.name,
	                              work.model_name,
	                              work.sample_num,
	                              work.object,
	                              work.pos,
	                              work.rot,
	                              work.oval_param,
	                              work.visible_flag,
	                              work.light_flag,
	                              work.bound_model
) ;
}


/*********************************************

	エフェクト　物理シム 新髪の毛ＥＶＭ２　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
