/*********************************************

	エフェクト　装備品 レインコート　のランチャー関数

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
	void    *body ;
	int     *sg_name ;
	int     sg_num ;
	int     mt_name ;
	float   wind_min ;
	float   wind_max ;
	float   frame_min ;
	float   frame_max ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
	int     d8 ;
	int     d9 ;
	int     d10 ;
	int     d11 ;
	int     d12 ;
	float   d13 ;
	float   d14 ;
	float   d15 ;
	float   d16 ;
} Data ;

void *NewRaincoat_called_1005Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRaincoat_called( void *,int *,int ,int ,float ,float ,float ,float  ) ;

	IVECTOR * link ;
	void    * handle ;
	int     * nm2 ;
	int     * nm3 ;
	int     * nm4 ;
	int     * nm5 ;
	int     * nm6 ;
	int     * nm7 ;
	int     * nm8 ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	work.sg_name = &data->d3 ;
	nm2 = &data->d4 ;
	nm3 = &data->d5 ;
	nm4 = &data->d6 ;
	nm5 = &data->d7 ;
	nm6 = &data->d8 ;
	nm7 = &data->d9 ;
	nm8 = &data->d10 ;
	work.sg_num = data->d11 ;
	work.mt_name = data->d12 ;
	work.wind_min = data->d13 ;
	work.wind_max = data->d14 ;
	work.frame_min = data->d15 ;
	work.frame_max = data->d16 ;

	return NewRaincoat_called( work.body,
	                           work.sg_name,
	                           work.sg_num,
	                           work.mt_name,
	                           work.wind_min,
	                           work.wind_max,
	                           work.frame_min,
	                           work.frame_max
) ;
}


/*********************************************

	エフェクト　装備品 レインコート　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
