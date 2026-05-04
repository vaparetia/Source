/*********************************************

	エフェクト　物理シム・マント　のランチャー関数

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
	int     *model_name ;
	int     model_name_size ;
	int     sample_num ;
	void    *object ;
	int     objnum ;
	FVECTOR *pos ;
	SVECTOR *rot ;
	int     boundmodel_name ;
	float   oval_param ;
	int     force_disp_flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	IVECTOR d2 ;
	FVECTOR d5 ;
	SVECTOR d6 ;
	int     d7 ;
	float   d8 ;
} Data ;

void *NewWavingClothModelW_called_3010Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWavingClothModelW_called( int ,int *,int ,int ,void *,int ,FVECTOR *,SVECTOR *,int ,float ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	work.model_name = &data->d1 ;
	work.model_name_size = ( int ) 1.000000  ;
	work.sample_num = ( int ) 3.000000  ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.objnum = link->vy ;
	work.pos = &data->d5 ;
	work.rot = &data->d6 ;
	work.boundmodel_name = data->d7 ;
	work.oval_param = data->d8 ;
	work.force_disp_flag = ( int ) 1.000000  ;

	return NewWavingClothModelW_called( work.name,
	                                    work.model_name,
	                                    work.model_name_size,
	                                    work.sample_num,
	                                    work.object,
	                                    work.objnum,
	                                    work.pos,
	                                    work.rot,
	                                    work.boundmodel_name,
	                                    work.oval_param,
	                                    work.force_disp_flag
) ;
}


/*********************************************

	エフェクト　物理シム・マント　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
