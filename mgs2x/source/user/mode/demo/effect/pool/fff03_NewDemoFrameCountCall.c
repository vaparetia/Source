/*********************************************

	エフェクト　システム ３０フレーム再生　のランチャー関数

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
	int     start ;
	int     end ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
} Data ;

void *NewDemoFrameCountCall_fff03Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoFrameCountCall( int ,int ,int  ) ;


	work.name = data->d0 ;
	work.start = data->d1 ;
	work.end = data->d2 ;

	return NewDemoFrameCountCall( work.name,
	                              work.start,
	                              work.end
) ;
}


/*********************************************

	エフェクト　システム ３０フレーム再生　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
