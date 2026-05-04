/*********************************************

	エフェクト　全画面 環境光　のランチャー関数

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
	IVECTOR *rgb ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *DM_ChangeAmbient_0006Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_ChangeAmbient( IVECTOR * ) ;


	work.rgb = &data->d0 ;

	return DM_ChangeAmbient( work.rgb
) ;
}


/*********************************************

	エフェクト　全画面 環境光　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
