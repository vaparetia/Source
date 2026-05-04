/*********************************************

	エフェクト　血 カメラ血（起動）　のランチャー関数

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
	FVECTOR *pos_pers ;
	int     white ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	int     d1 ;
} Data ;

void *PutCameraBlood_7006Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *PutCameraBlood( FVECTOR *,int  ) ;


	work.pos_pers = &data->d0 ;
	work.white = data->d1 ;

	return PutCameraBlood( work.pos_pers,
	                       work.white
) ;
}


/*********************************************

	エフェクト　血 カメラ血（起動）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
