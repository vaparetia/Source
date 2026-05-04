//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 弾丸　のランチャー関数

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
	int     id ;
	float   rot_y ;
	float   speed ;
	FVECTOR *from ;
	FVECTOR *to ;
	int     flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	float   d2 ;
	FVECTOR d3 ;
	FVECTOR d4 ;
	int     d5 ;
} Data ;

void *NewDemoBulletCall_6005Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoBulletCall( int ,float ,float ,FVECTOR *,FVECTOR *,int  ) ;


	work.id = data->d0 ;
	work.rot_y = data->d1 ;
	work.speed = data->d2 ;
	work.from = &data->d3 ;
	work.to = &data->d4 ;
	work.flag = data->d5 ;

	return NewDemoBulletCall( work.id,
	                          work.rot_y,
	                          work.speed,
	                          work.from,
	                          work.to,
	                          work.flag
) ;
}


/*********************************************

	エフェクト　効果 弾丸　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
