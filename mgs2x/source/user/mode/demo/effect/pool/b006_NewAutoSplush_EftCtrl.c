/*********************************************

	エフェクト　水効果_球水飛沫ダミー　のランチャー関数

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
	int     con_name ;
	float   rad ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
} Data ;

void *NewAutoSplush_EftCtrl_b006Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewAutoSplush_EftCtrl( int ,float  ) ;


	work.con_name = data->d0 ;
	work.rad = data->d1 ;

	return NewAutoSplush_EftCtrl( work.con_name,
	                              work.rad
) ;
}


/*********************************************

	エフェクト　水効果_球水飛沫ダミー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
