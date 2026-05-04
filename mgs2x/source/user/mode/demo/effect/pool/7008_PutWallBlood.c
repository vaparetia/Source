/*********************************************

	エフェクト　血 壁血（起動）　のランチャー関数

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
	FVECTOR *pos ;
	SVECTOR *rot ;
	int     white ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	SVECTOR d1 ;
	int     d2 ;
} Data ;

void *PutWallBlood_7008Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *PutWallBlood( FVECTOR *,SVECTOR *,int  ) ;


	work.pos = &data->d0 ;
	work.rot = &data->d1 ;
	work.white = data->d2 ;

	return PutWallBlood( work.pos,
	                     work.rot,
	                     work.white
) ;
}


/*********************************************

	エフェクト　血 壁血（起動）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
