//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙_汎用立ち昇る煙　のランチャー関数

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
	FMATRIX *world ;
	float   init_speed ;
	float   rising_speed ;
	float   init_size ;
	float   last_size ;
	int     alpha ;
	int     num ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	float   d6 ;
	int     d7 ;
	int     d8 ;
} Data ;

void *NewDemoRisingSmoke_7107Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoRisingSmoke( FMATRIX *,float ,float ,float ,float ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.init_speed = data->d3 ;
	work.rising_speed = data->d4 ;
	work.init_size = data->d5 ;
	work.last_size = data->d6 ;
	work.alpha = data->d7 ;
	work.num = data->d8 ;

	return NewDemoRisingSmoke( work.world,
	                           work.init_speed,
	                           work.rising_speed,
	                           work.init_size,
	                           work.last_size,
	                           work.alpha,
	                           work.num
) ;
}


/*********************************************

	エフェクト　煙_汎用立ち昇る煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
