//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　人体 モーションモデル　のランチャー関数

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
	int     where ;
	int     dir ;
	FVECTOR *pos ;
	int     model ;
	int     motion ;
	int     mot_num ;
	int     status ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d2 ;
	FVECTOR d3 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
} Data ;

void *NewSigPutModel2_100eLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSigPutModel2( int ,int ,int ,FVECTOR *,int ,int ,int ,int  ) ;


	work.name = data->d0 ;
	work.where = DM_GetCurrentMap(  ) ;
	work.dir = ((data->d2 * ( int ) 4096.000000 ) / ( int ) 360.000000 ) ;
	work.pos = &data->d3 ;
	work.model = data->d4 ;
	work.motion = data->d5 ;
	work.mot_num = data->d6 ;
	work.status = data->d7 ;

	return NewSigPutModel2( work.name,
	                        work.where,
	                        work.dir,
	                        work.pos,
	                        work.model,
	                        work.motion,
	                        work.mot_num,
	                        work.status
) ;
}


/*********************************************

	エフェクト　人体 モーションモデル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
