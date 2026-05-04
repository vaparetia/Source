/*********************************************

	エフェクト　水飛沫 モーション連動　のランチャー関数

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
	void    *objs ;
	float   step_limit ;
	int     life ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d3 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewSplashMotion_Demo_0100Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSplashMotion_Demo( void *,float ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	int      max ;
	int      min ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.step_limit = data->d3 ;
	max = data->d4 ;
	min = data->d5 ;
	work.life = (max - min) ;

	return NewSplashMotion_Demo( work.objs,
	                             work.step_limit,
	                             work.life
) ;
}


/*********************************************

	エフェクト　水飛沫 モーション連動　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
