/*********************************************

	エフェクト　システム クリッピング調整　のランチャー関数

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
	float   near_para ;
	float   far_para ;
} Work ;

typedef struct data_t {
	float   d0 ;
	float   d1 ;
} Data ;

void *DM_SetClipParam_fff02Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_SetClipParam( float ,float  ) ;


	work.near_para = data->d0 ;
	work.far_para = data->d1 ;

	return DM_SetClipParam( work.near_para,
	                        work.far_para
) ;
}


/*********************************************

	エフェクト　システム クリッピング調整　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
