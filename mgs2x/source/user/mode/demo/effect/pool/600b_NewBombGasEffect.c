/*********************************************

	エフェクト　効果 中煙　のランチャー関数

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
	SVECTOR *pole_rot ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	SVECTOR d1 ;
} Data ;

void *NewBombGasEffect_600bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBombGasEffect( FVECTOR *,SVECTOR * ) ;


	work.pos = &data->d0 ;
	work.pole_rot = &data->d1 ;

	return NewBombGasEffect( work.pos,
	                         work.pole_rot
) ;
}


/*********************************************

	エフェクト　効果 中煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
